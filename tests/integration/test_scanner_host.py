"""End-to-end tests: the component on ESPHome's host platform against a fake scanner.

Run with `uv run pytest -m integration`.  The first run compiles the host firmware
(a few minutes); set M5SIM_PROGRAM to a prebuilt binary to skip compilation.
"""

from __future__ import annotations

import os
import pty
import queue
import re
import subprocess
import sys
import threading
import time
from pathlib import Path
from typing import TYPE_CHECKING

import pytest

from .fake_scanner import FakeScanner

if TYPE_CHECKING:
    from collections.abc import Iterator

pytestmark = pytest.mark.integration

HERE = Path(__file__).parent
# Must match `port` in host_scanner.yaml.  ESPHome requires a two-level device path
# (/dir/name) that exists at compile time; the tests point it at a pty at runtime.
PORT = Path("/tmp/m5stack-barcode-sim")  # noqa: S108
ANSI = re.compile(r"\x1b\[[0-9;]*m")


@pytest.fixture(scope="session")
def program() -> Path:
    """Compile the host firmware once per session and return the executable."""
    if prebuilt := os.environ.get("M5SIM_PROGRAM"):
        return Path(prebuilt)
    PORT.unlink(missing_ok=True)
    PORT.symlink_to("/dev/null")
    try:
        result = subprocess.run(
            [
                sys.executable,
                "-m",
                "esphome",
                "compile",
                str(HERE / "host_scanner.yaml"),
            ],
            capture_output=True,
            text=True,
            check=False,
        )
    finally:
        PORT.unlink()
    output = result.stdout + result.stderr
    match = re.search(r"Successfully compiled program to path '([^']+)'", output)
    if result.returncode != 0 or match is None:
        pytest.fail(f"Host firmware build failed:\n{output[-5000:]}")
    return Path(match.group(1))


class Firmware:
    """A running host firmware whose log lines can be awaited."""

    def __init__(self, proc: subprocess.Popen[bytes], output_fd: int) -> None:
        """Start collecting the process output from the pty at `output_fd`."""
        self.proc = proc
        self.lines: list[str] = []
        self._queue: queue.Queue[str] = queue.Queue()
        self._output = os.fdopen(output_fd, "rb", buffering=0)
        threading.Thread(target=self._reader, daemon=True).start()

    def _reader(self) -> None:
        pending = b""
        while True:
            try:
                chunk = self._output.read(4096)
            except OSError:  # EIO once the firmware exits and the pty closes
                return
            if not chunk:
                return
            pending += chunk
            *lines, pending = pending.split(b"\n")
            for line in lines:
                text = line.decode(errors="replace").rstrip()
                self._queue.put(ANSI.sub("", text))

    def wait_for(self, pattern: str, timeout: float = 10) -> re.Match[str]:
        """Return the first new log line matching `pattern`, or fail after `timeout`."""
        regex = re.compile(pattern)
        deadline = time.monotonic() + timeout
        while (remaining := deadline - time.monotonic()) > 0:
            try:
                line = self._queue.get(timeout=remaining)
            except queue.Empty:
                break
            self.lines.append(line)
            if match := regex.search(line):
                return match
        log = "\n".join(self.lines[-60:])
        pytest.fail(f"Timed out waiting for {pattern!r}. Recent log:\n{log}")
        raise AssertionError  # unreachable; pytest.fail raises

    def assert_absent(self, pattern: str, duration: float) -> None:
        """Fail if a log line matching `pattern` appears within `duration`."""
        regex = re.compile(pattern)
        deadline = time.monotonic() + duration
        while (remaining := deadline - time.monotonic()) > 0:
            try:
                line = self._queue.get(timeout=remaining)
            except queue.Empty:
                return
            self.lines.append(line)
            assert not regex.search(line), f"Unexpected log line: {line}"


@pytest.fixture
def run(
    program: Path,
    tmp_path: Path,
) -> Iterator[callable]:
    """Start the fake scanner and the firmware; yield a starter function."""
    started: list[tuple[FakeScanner, subprocess.Popen[bytes]]] = []

    def _start(  # noqa: PLR0913 - keyword-only scenario options
        *,
        mode: str = "host",
        terminator: str = "crlf",
        start_scan: bool = False,
        start_reply: str = "scan",
        nak_params: frozenset[int] = frozenset(),
        extra_env: dict[str, str] | None = None,
    ) -> tuple[FakeScanner, Firmware]:
        scanner = FakeScanner(PORT)
        scanner.start_reply = start_reply
        scanner.nak_params = set(nak_params)
        scanner.__enter__()
        env = {
            **os.environ,
            "HOME": str(tmp_path),  # fresh preferences for every test
            "SIM_MODE": mode,
            "SIM_TERMINATOR": terminator,
        }
        if start_scan:
            env["SIM_START"] = "1"
        env.update(extra_env or {})
        # A pty (not a pipe) keeps the firmware's stdout line-buffered.
        output_fd, firmware_tty = pty.openpty()
        proc = subprocess.Popen(
            [str(program)],
            env=env,
            cwd=tmp_path,
            stdin=subprocess.DEVNULL,
            stdout=firmware_tty,
            stderr=firmware_tty,
        )
        os.close(firmware_tty)
        started.append((scanner, proc))
        firmware = Firmware(proc, output_fd)
        firmware.wait_for(r"READY", timeout=15)
        return scanner, firmware

    yield _start
    for scanner, proc in started:
        proc.kill()
        proc.wait()
        scanner.__exit__()


def test_version_is_published(run: callable) -> None:
    """GET_VERSION's unframed response is parsed into the version sensor."""
    _, firmware = run()
    assert any("VERSION[2.2.18]" in line for line in firmware.lines)


def test_host_scan_publishes_barcode(run: callable) -> None:
    """A HOST-mode scan publishes the barcode with the terminator stripped."""
    _, firmware = run(start_scan=True)
    firmware.wait_for(r"SCANNING\[1\]")
    # The scan ends (scanning sensor off) just before the barcode is published.
    firmware.wait_for(r"SCANNING\[0\]")
    firmware.wait_for(r"BARCODE\[HELLO-123\]")


@pytest.mark.parametrize("terminator", ["none", "cr", "tab", "crcr", "crlfcrlf"])
def test_ack_and_barcode_in_one_read(run: callable, terminator: str) -> None:
    """Barcode bytes arriving in the same read as the start ACK are not lost."""
    _, firmware = run(start_scan=True, start_reply="burst", terminator=terminator)
    firmware.wait_for(r"BARCODE\[HELLO-123\]")
    firmware.assert_absent(r"SCAN_TIMEOUT", duration=1.5)


def test_scan_timeout(run: callable) -> None:
    """on_scan_timeout fires once scan_duration elapses after the start ACK."""
    _, firmware = run(start_scan=True, start_reply="ack")
    firmware.wait_for(r"SCANNING\[1\]")
    firmware.wait_for(r"SCANNING\[0\]", timeout=5)
    firmware.wait_for(r"SCAN_TIMEOUT")


@pytest.mark.parametrize(
    ("mode", "terminator"),
    [("continuous", "crlf"), ("continuous", "none"), ("auto_sense", "tab")],
)
def test_continuous_modes_publish_barcodes(
    run: callable,
    mode: str,
    terminator: str,
) -> None:
    """Barcodes output without a start command are published in non-host modes."""
    scanner, firmware = run(mode=mode, terminator=terminator)
    # The scan state follows the ACKed mode, just before the select publishes it.
    firmware.wait_for(r"SCANNING\[1\]")
    firmware.wait_for(rf"MODE\[{mode}\]")
    scanner.emit_barcode(b"FIRST")
    firmware.wait_for(r"BARCODE\[FIRST\]")
    scanner.emit_barcode(b"SECOND")
    firmware.wait_for(r"BARCODE\[SECOND\]")


def test_oversized_frame_is_dropped(run: callable) -> None:
    """An oversized frame is dropped whole and the next barcode still arrives."""
    scanner, firmware = run(mode="continuous")
    firmware.wait_for(r"MODE\[continuous\]")
    scanner.emit_barcode(b"X" * 700)
    firmware.wait_for(r"Dropped oversized frame")
    scanner.emit_barcode(b"NEXT")
    firmware.wait_for(r"BARCODE\[NEXT\]")


def test_long_barcode_is_truncated_to_ha_state_limit(run: callable) -> None:
    """Barcodes longer than HA's 255-character state limit are truncated."""
    scanner, firmware = run(mode="continuous")
    firmware.wait_for(r"MODE\[continuous\]")
    scanner.emit_barcode(b"A" * 300)
    match = firmware.wait_for(r"BARCODE\[(A+)\]")
    assert len(match.group(1)) == 255  # noqa: PLR2004


PARAM_VOLUME = 0x8C
VOLUME_HIGH = 0x00


def test_setting_reverted_before_ack_ends_at_last_value(run: callable) -> None:
    """Changing a setting and changing it back before the ACK applies the last value."""
    scanner, firmware = run(extra_env={"SIM_TOGGLE": "1"})
    firmware.wait_for(r"TOGGLED")
    firmware.assert_absent(r"VOLUME\[high\]", duration=2)
    volume_commands = [f[6] for f in scanner.received if f[5] == PARAM_VOLUME]
    assert VOLUME_HIGH not in volume_commands


def test_rejected_setting_fails_fast_and_keeps_state(run: callable) -> None:
    """A NAKed command is dropped at once (no 2 s timeout and retry)."""
    scanner, firmware = run(
        nak_params=frozenset({PARAM_VOLUME}),
        extra_env={"SIM_SET_VOLUME": "1"},
    )
    firmware.wait_for(r"rejected", timeout=1)
    firmware.assert_absent(r"VOLUME\[high\]|timed out", duration=2.5)
    # Sent exactly once: no retry after the NAK.
    high = [f for f in scanner.received if f[5] == PARAM_VOLUME and f[6] == VOLUME_HIGH]
    assert len(high) == 1


@pytest.mark.parametrize(
    ("payload", "expected"),
    [
        # 150 two-byte characters (300 bytes): truncation must not split a character.
        ("é".encode() * 150, "é" * 127),
        # Latin-1 / invalid UTF-8 byte: replaced, not the whole barcode dropped.
        (b"caf\xe9", "caf\ufffd"),
        # Overlong encoding of '/' and a UTF-16 surrogate: protobuf rejects both.
        (b"A\xc0\xafB\xed\xa0\x80C", "A\ufffd\ufffdB\ufffd\ufffd\ufffdC"),
    ],
    ids=["multibyte-truncation", "latin1", "overlong-surrogate"],
)
def test_barcodes_are_published_as_valid_utf8(
    run: callable,
    payload: bytes,
    expected: str,
) -> None:
    """Barcodes reach HA as valid UTF-8 no longer than HA's state limit."""
    scanner, firmware = run(mode="continuous")
    firmware.wait_for(r"MODE\[continuous\]")
    scanner.emit_barcode(payload)
    match = firmware.wait_for(r"BARCODE\[(.*)\]")
    assert match.group(1) == expected
