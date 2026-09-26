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

from .fake_scanner import (
    OP_SETTING,
    OP_START,
    OP_STOP,
    VERSION_RESPONSE,
    FakeScanner,
)

if TYPE_CHECKING:
    from collections.abc import Iterator

pytestmark = pytest.mark.integration

HERE = Path(__file__).parent
# Must match `port` in host_scanner.yaml.  ESPHome requires a two-level device path
# (/dir/name) that exists at compile time; the tests point it at a pty at runtime.
PORT = Path("/tmp/m5stack-barcode-sim")  # noqa: S108
ANSI = re.compile(r"\x1b\[[0-9;]*m")
# Parameter byte (frame[5]) of the buzzer volume setting command, and its "high" value
PARAM_VOLUME = 0x8C
VOLUME_HIGH = 0x00
PARAM_F2 = 0xF2
F2_MUTE = 0x0C
PARAM_MODE = 0x8A
MODE_FIELD = 0  # operation_mode in the stored settings
MODE_CONTINUOUS = 3  # its option index


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

    def wait_for(
        self,
        pattern: str,
        timeout: float = 10,
        *,
        from_start: bool = False,
    ) -> re.Match[str]:
        """Return the first new log line matching `pattern`, or fail after `timeout`.

        With `from_start`, lines logged before this call (e.g. during boot) also count.
        """
        regex = re.compile(pattern)
        if from_start:
            for line in self.lines:
                if match := regex.search(line):
                    return match
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
        version_response: bytes | None = VERSION_RESPONSE,
        extra_env: dict[str, str] | None = None,
    ) -> tuple[FakeScanner, Firmware]:
        scanner = FakeScanner(PORT)
        scanner.start_reply = start_reply
        scanner.nak_params = set(nak_params)
        scanner.version_response = version_response
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

    _start.started = started
    yield _start
    _stop_all(started)


def _stop_all(started: list[tuple[FakeScanner, subprocess.Popen[bytes]]]) -> None:
    while started:
        scanner, proc = started.pop()
        # SIGTERM: a clean shutdown, which also writes the coverage data
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait()
        scanner.__exit__()
        assert not scanner.ignored, (
            f"commands sent without a wake-up: {scanner.ignored}"
        )


@pytest.fixture
def restart(run: callable) -> callable:
    """Stop every running firmware; the next `run()` reuses the same preferences."""
    return lambda: _stop_all(run.started)


def test_version_is_published(run: callable) -> None:
    """GET_VERSION's unframed response is parsed into the version sensor."""
    _, firmware = run()
    assert any("VERSION[2.2.18]" in line for line in firmware.lines)


@pytest.mark.parametrize(
    ("response", "version"),
    [
        (b"\x58\xa4\x00\x00Firmware version:2.2.18   \xe4D", "2.2.18"),
        # No "Firmware version:": the last value in the response
        (b"\x58\xa4\x00\x00Model:X1 Rev: 3.1.4 \xe4D", "3.1.4"),
        # The key's value wins over later values
        (b"\x58\xa4Firmware version:1.2.3\x01\x02Build:zzz\xe4D", "1.2.3"),
    ],
    ids=["trailing-spaces", "no-key", "key-then-more"],
)
def test_version_response_variants(
    run: callable, response: bytes, version: str
) -> None:
    """The version sensor gets the firmware version from other response layouts."""
    _, firmware = run(version_response=response)
    firmware.wait_for(rf"VERSION\[{re.escape(version)}\]", from_start=True)


def test_unanswered_version_request_warns(run: callable) -> None:
    """A scanner that never answers GET_VERSION does not block the queue."""
    _, firmware = run(version_response=None)
    firmware.wait_for(r"Version request timed out", from_start=True)
    firmware.wait_for(r"set Warning flag: Scanner not responding", from_start=True)
    assert not any("VERSION[" in line for line in firmware.lines)


def test_restored_settings_are_published_once(run: callable, tmp_path: Path) -> None:
    """Settings restored at boot are published once, not on every loop."""
    write_prefs(
        tmp_path,
        "m5stack_barcode_settings",
        bytes([3]) + with_volume(YAML_SETTINGS, VOLUME_HIGH) + YAML_SETTINGS,
    )
    _, firmware = run()
    firmware.assert_absent(r"VOLUME\[low\]", duration=1.5)
    assert sum("VOLUME[high]" in line for line in firmware.lines) == 1


def test_host_scan_publishes_barcode(run: callable) -> None:
    """A HOST-mode scan publishes the barcode with the terminator stripped."""
    _, firmware = run(start_scan=True)
    firmware.wait_for(r"SCANNING\[1\]")
    # The scan ends (scanning sensor off) just before the barcode is published.
    firmware.wait_for(r"SCANNING\[0\]")
    firmware.wait_for(r"SENSOR\[HELLO-123\]")
    firmware.wait_for(r"BARCODE\[HELLO-123\]")
    firmware.wait_for(r"EVENT\[scan_successful\]")
    firmware.assert_absent(r"SCAN_TIMEOUT", duration=1.5)  # scan_duration is 1 s


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
    firmware.assert_absent(r"SCAN_TIMEOUT", duration=1.5)


def test_unlimited_scan_does_not_time_out(run: callable) -> None:
    """With scan_duration: unlimited a scan runs until stopped."""
    _, firmware = run(
        start_scan=True, start_reply="ack", extra_env={"SIM_DURATION": "unlimited"}
    )
    firmware.wait_for(r"SCANNING\[1\]")
    firmware.assert_absent(r"SCAN_TIMEOUT|SCANNING\[0\]", duration=2.5)


def test_leaving_host_mode_cancels_the_scan_timeout(run: callable) -> None:
    """Switching to continuous mode mid-scan ends the host scan without a timeout."""
    _, firmware = run(
        start_scan=True,
        start_reply="ack",
        extra_env={"SIM_CONTINUOUS_DURING_SCAN": "1"},
    )
    firmware.wait_for(r"MODE\[continuous\]")
    firmware.assert_absent(r"SCAN_TIMEOUT|SCANNING\[0\]", duration=2)


def test_config_barcode_is_applied_not_published(run: callable) -> None:
    """A configuration barcode the scanner passes on is applied over UART and synced."""
    scanner, firmware = run(mode="continuous")
    firmware.wait_for(r"MODE\[continuous\]")
    scanner.emit_barcode(b"^#SC^2050801")  # buzzer volume: middle
    firmware.wait_for(r"VOLUME\[medium\]")
    assert any(f[5] == PARAM_VOLUME and f[6] == 1 for f in scanner.received)
    scanner.emit_barcode(b"^#SC^3030010")  # communication mode: serial (not managed)
    firmware.wait_for(r"Ignoring configuration barcode \^#SC\^3030010")
    scanner.emit_barcode(b"AFTER")
    firmware.wait_for(r"BARCODE\[AFTER\]")
    assert not any("BARCODE[^#SC^" in line for line in firmware.lines)


def test_stop_ends_the_scan(run: callable) -> None:
    """Stop ends a running scan at once and cancels its timeout."""
    scanner, firmware = run(
        start_scan=True, start_reply="ack", extra_env={"SIM_STOP": "1"}
    )
    firmware.wait_for(r"SCANNING\[1\]")
    firmware.wait_for(r"SCANNING\[0\]", timeout=1)
    firmware.assert_absent(r"SCAN_TIMEOUT", duration=2)  # scan_duration is 1 s
    assert any(f[1] == OP_STOP for f in scanner.received)


def test_invalid_action_value_is_ignored(run: callable) -> None:
    """A lambda returning an unknown option key changes nothing."""
    scanner, firmware = run(extra_env={"SIM_BAD_VALUE": "1"})
    firmware.wait_for(r"Ignoring invalid buzzer_volume value 'loud'", from_start=True)
    firmware.wait_for(r"Ignoring invalid value 7 for setting 5", from_start=True)
    firmware.assert_absent(r"VOLUME\[high", duration=0.5)
    # Only the YAML value sent at boot (there are no stored settings)
    assert volume_commands(scanner) == [VOLUME_LOW]


def test_switch_entity_follows_the_scanner(run: callable) -> None:
    """A two-state setting's switch shows the ACKed value."""
    scanner, firmware = run(extra_env={"SIM_SOUND": "1"})
    firmware.wait_for(r"SOUND\[1\]", from_start=True)
    # Sound is a mute switch: "enabled" sends mute off (F2 0C 00).
    assert any(
        f[5] == PARAM_F2 and f[6] == F2_MUTE and f[7] == 0 for f in scanner.received
    )


def test_rejected_start_ends_the_scan(run: callable) -> None:
    """A start the scanner NAKs (e.g. its mode was changed by a config barcode)."""
    _, firmware = run(start_scan=True, start_reply="nak")
    firmware.wait_for(r"SCANNING\[1\]")
    firmware.wait_for(r"SCANNING\[0\]", timeout=1)
    firmware.assert_absent(r"SCAN_TIMEOUT|BARCODE\[", duration=1.5)


def test_unanswered_start_ends_the_scan_and_warns(run: callable) -> None:
    """A scanner that stops answering ends the scan and sets the warning status."""
    scanner, firmware = run(
        start_scan=True,
        start_reply="silent",
        extra_env={"SIM_LATE_VOLUME": "1"},
    )
    firmware.wait_for(r"SCANNING\[1\]")
    firmware.wait_for(r"No reply \(attempt 1/2\), retrying", timeout=3)
    firmware.wait_for(r"set Warning flag: Scanner not responding", timeout=3)
    firmware.wait_for(r"SCANNING\[0\]", timeout=1)
    starts = [f for f in scanner.received if f[1] == OP_START]
    assert len(starts) == 2  # noqa: PLR2004 - sent once, retried once
    # The next command the scanner answers clears the warning.
    firmware.wait_for(r"VOLUME\[high\]", timeout=5)
    firmware.wait_for(r"cleared Warning flag", from_start=True)


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
    firmware.assert_absent(r"VOLUME\[high\]|No reply", duration=2.5)
    # Sent exactly once: no retry after the NAK.
    high = [f for f in scanner.received if f[5] == PARAM_VOLUME and f[6] == VOLUME_HIGH]
    assert len(high) == 1
    assert not any("BARCODE[" in line for line in firmware.lines)


@pytest.mark.parametrize(
    ("payload", "expected"),
    [
        # 150 two-byte characters (300 bytes): truncation must not split a character.
        ("é".encode() * 150, "é" * 127),
        # Latin-1 / invalid UTF-8 byte: replaced, not the whole barcode dropped.
        (b"caf\xe9", "caf\ufffd"),
        # Overlong encoding of '/' and a UTF-16 surrogate: protobuf rejects both.
        (b"A\xc0\xafB\xed\xa0\x80C", "A\ufffd\ufffdB\ufffd\ufffd\ufffdC"),
        # Four-byte characters, and one cut off at the end of the barcode.
        ("\U0001f600ok".encode() + b"\xf0\x9f\x98", "\U0001f600ok\ufffd"),
        # Valid characters at the edges of each lead byte range
        ((s := "é\u07ff\u0800\ud7ff\ue000\ufb01€\U00010000\U00100000").encode(), s),
        # A lone continuation byte, a lead byte without its continuation, an
        # overlong three-byte form and a code point above U+10FFFF
        (
            b"a\x80b\xc3Ac\xe0\x80\x80d\xf4\x90\x80\x80",
            "a\ufffdb\ufffdAc" + "\ufffd" * 3 + "d" + "\ufffd" * 4,
        ),
    ],
    ids=[
        "multibyte-truncation",
        "latin1",
        "overlong-surrogate",
        "four-byte",
        "range-edges",
        "invalid-sequences",
    ],
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


# Values of host_scanner.yaml's m5stack_barcode options (option indices, in SettingId
# order).  The on_boot actions change some of them at runtime.
YAML_SETTINGS = bytes([0, 0, 0, 0, 0, 2, 1, 0, 1, 2, 3, 3, 3, 1, 0])
VOLUME_FIELD = 5  # buzzer_volume in ScannerSettings
VOLUME_MEDIUM = 0x01
VOLUME_LOW = 0x02


def fnv1_hash(text: str) -> int:
    """Hash a preference key the way ESPHome does (FNV-1, 32 bit)."""
    value = 2166136261
    for char in text.encode():
        value = (value * 16777619) & 0xFFFFFFFF
        value ^= char
    return value


def prefs_path(home: Path) -> Path:
    """Where ESPHome's host platform stores this firmware's preferences."""
    return home / ".esphome" / "prefs" / "m5stack-barcode-sim.prefs"


def write_prefs(home: Path, key: str, data: bytes) -> None:
    """Store a preference the way ESPHome's host platform does."""
    path = prefs_path(home)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(fnv1_hash(key).to_bytes(4, "little") + bytes([len(data)]) + data)


def read_prefs(home: Path, key: str) -> bytes | None:
    """Return a stored preference, parsing the host format (key, length, data)."""
    try:
        raw = prefs_path(home).read_bytes()
    except FileNotFoundError:
        return None
    wanted = fnv1_hash(key)
    offset = 0
    while offset + 5 <= len(raw):
        record_key = int.from_bytes(raw[offset : offset + 4], "little")
        length = raw[offset + 4]
        data = raw[offset + 5 : offset + 5 + length]
        if record_key == wanted:
            return data
        offset += 5 + length
    return None


def wait_for_stored(home: Path, field: int, value: int, timeout: float = 10) -> None:
    """Wait until a stored applied setting (by SettingId) is `value` on disk."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        prefs = read_prefs(home, "m5stack_barcode_settings")
        # version byte, then the `applied` settings
        if prefs is not None and prefs[1 + field] == value:
            return
        time.sleep(0.05)
    pytest.fail(f"setting {field} = {value} was not persisted within {timeout} s")


def wait_for_stored_volume(home: Path, volume: int) -> None:
    """Wait until the stored applied buzzer_volume is `volume` (flushed to disk)."""
    wait_for_stored(home, VOLUME_FIELD, volume)


def with_volume(settings: bytes, volume: int) -> bytes:
    """Return `settings` with buzzer_volume replaced."""
    return settings[:VOLUME_FIELD] + bytes([volume]) + settings[VOLUME_FIELD + 1 :]


F2_TERMINATOR = 0x05
PARAM_SCAN_DURATION = 0x88


def setting_frames(scanner: FakeScanner) -> list[bytes]:
    """Return the setting commands the scanner received."""
    return [f for f in scanner.received if f[1] == OP_SETTING]


def volume_commands(scanner: FakeScanner) -> list[int]:
    """Values of the buzzer volume commands the scanner received."""
    return [f[6] for f in scanner.received if f[5] == PARAM_VOLUME]


def test_runtime_setting_change_survives_restart(
    run: callable,
    restart: callable,
    tmp_path: Path,
) -> None:
    """A setting changed at runtime (e.g. from HA) is kept after a reboot."""
    _, firmware = run(extra_env={"SIM_SET_VOLUME": "1"})
    firmware.wait_for(r"VOLUME\[high\]")
    # The change is published before the preferences are flushed to disk.
    wait_for_stored_volume(tmp_path, VOLUME_HIGH)
    restart()
    scanner, firmware = run()
    firmware.wait_for(r"VOLUME\[high\]", from_start=True)
    firmware.assert_absent(r"VOLUME\[low\]", duration=1)
    assert volume_commands(scanner) == []


def test_yaml_edit_is_applied(run: callable, tmp_path: Path) -> None:
    """A YAML value that changed since the last boot overrides the stored value."""
    # Stored: the scanner has 'high'; the YAML used to say 'medium' (now 'low').
    applied = with_volume(YAML_SETTINGS, VOLUME_HIGH)
    baseline = with_volume(YAML_SETTINGS, VOLUME_MEDIUM)
    write_prefs(tmp_path, "m5stack_barcode_settings", bytes([3]) + applied + baseline)
    scanner, firmware = run()
    firmware.wait_for(r"VOLUME\[low\]", from_start=True)
    assert volume_commands(scanner) == [VOLUME_LOW]
    firmware.wait_for(
        r"scan_duration set to 1s", from_start=True
    )  # the last one queued
    # Settings whose YAML value did not change were restored, not re-sent: only the
    # volume and what the on_boot actions change (terminator, scan duration) are sent.
    sent = {(f[5], f[6]) if f[5] == PARAM_F2 else f[5] for f in setting_frames(scanner)}
    assert sent == {PARAM_VOLUME, (PARAM_F2, F2_TERMINATOR), PARAM_SCAN_DURATION}


def test_legacy_preferences_are_migrated(
    run: callable,
    restart: callable,
    tmp_path: Path,
) -> None:
    """Settings stored by older firmware are kept, including runtime changes."""
    legacy = bytes([2]) + with_volume(YAML_SETTINGS, VOLUME_HIGH)
    write_prefs(tmp_path, "m5stack_barcode", legacy)
    scanner, firmware = run()
    firmware.wait_for(r"Migrating scanner settings", from_start=True)
    firmware.wait_for(r"VOLUME\[high\]", from_start=True)
    assert volume_commands(scanner) == []
    # Migrated once: stored in the current format, the old entry cleared.
    wait_for_stored_volume(tmp_path, VOLUME_HIGH)
    assert read_prefs(tmp_path, "m5stack_barcode")[0] == 0
    restart()
    scanner, firmware = run()
    firmware.wait_for(r"Restoring scanner settings from flash", from_start=True)
    firmware.wait_for(r"VOLUME\[high\]", from_start=True)
    assert volume_commands(scanner) == []


@pytest.mark.parametrize(
    ("key", "stored"),
    [
        # buzzer_volume 3 is one past its last option
        (
            "m5stack_barcode_settings",
            bytes([3]) + with_volume(YAML_SETTINGS, 3) + YAML_SETTINGS,
        ),
        (
            "m5stack_barcode_settings",
            bytes([9]) + with_volume(YAML_SETTINGS, 0) + YAML_SETTINGS,
        ),
        ("m5stack_barcode", bytes([1]) + with_volume(YAML_SETTINGS, 0)),
    ],
    ids=["value-out-of-range", "unknown-version", "legacy-unknown-version"],
)
def test_invalid_stored_settings_are_ignored(
    run: callable,
    tmp_path: Path,
    key: str,
    stored: bytes,
) -> None:
    """Stored settings that are out of range or of an unknown layout are not used."""
    write_prefs(tmp_path, key, stored)
    scanner, firmware = run()
    firmware.wait_for(r"No stored scanner settings", from_start=True)
    firmware.wait_for(r"VOLUME\[low\]", from_start=True)
    firmware.wait_for(
        r"scan_duration set to 1s", from_start=True
    )  # the last one queued
    assert len(setting_frames(scanner)) >= 15  # noqa: PLR2004 - every setting sent


def test_mode_is_restored_at_boot(
    run: callable,
    restart: callable,
    tmp_path: Path,
) -> None:
    """Continuous mode restored from flash is scanning from boot, without a command."""
    _, firmware = run(mode="continuous")
    firmware.wait_for(r"MODE\[continuous\]")
    wait_for_stored(tmp_path, MODE_FIELD, MODE_CONTINUOUS)
    restart()
    scanner, firmware = run(mode="continuous")
    firmware.wait_for(r"SCANNING\[1\]", from_start=True)
    assert not any(f[5] == PARAM_MODE for f in setting_frames(scanner))


def test_factory_reset_reapplies_the_yaml_values(
    run: callable,
    restart: callable,
    tmp_path: Path,
) -> None:
    """A factory reset clears the stored settings; the next boot sends every value."""
    _, firmware = run(extra_env={"SIM_SET_VOLUME": "1"})
    firmware.wait_for(r"VOLUME\[high\]")
    wait_for_stored_volume(tmp_path, VOLUME_HIGH)
    restart()
    _, firmware = run(extra_env={"SIM_FACTORY_RESET": "1"})
    firmware.wait_for(r"rebooting to re-apply YAML settings")
    assert firmware.proc.wait(timeout=10) == 0  # the host platform's reboot exits
    assert read_prefs(tmp_path, "m5stack_barcode_settings")[0] == 0
    restart()
    scanner, firmware = run()
    firmware.wait_for(r"No stored scanner settings", from_start=True)
    firmware.wait_for(r"VOLUME\[low\]", from_start=True)
    firmware.wait_for(
        r"scan_duration set to 1s", from_start=True
    )  # the last one queued
    assert len(setting_frames(scanner)) >= 15  # noqa: PLR2004 - every setting
