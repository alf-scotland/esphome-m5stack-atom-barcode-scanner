"""Emulates the M5Stack Atom QR scanner's UART protocol on a pseudo-terminal.

Implements the parts of ATOM_QRCODE_CMD_EN.pdf the component relies on: the 0x00
wake-up byte (a sleeping scanner ignores a command that does not follow it by at least
50 ms; the fake treats the scanner as asleep before every command), the
04 D0 00 00 FF 2C ACK for every setting command and for start/stop decoding in host
mode, the NAK for start/stop outside it, the unframed GET_VERSION response and unframed
barcode output followed by the configured terminator. Output is paced like a 9600 baud
line (about 1 ms per byte).
"""

from __future__ import annotations

import os
import pty
import queue
import threading
import time
import tty
from typing import TYPE_CHECKING, Self

if TYPE_CHECKING:
    from pathlib import Path

ACK = bytes([0x04, 0xD0, 0x00, 0x00, 0xFF, 0x2C])
# NAK: the reply to start/stop decoding outside host mode (PDF item 3)
NAK = bytes([0x05, 0xD1, 0x00, 0x00, 0x06, 0xFF, 0x24])
TERMINATORS = {0: b"", 1: b"\r\n", 2: b"\r", 3: b"\t", 4: b"\r\r", 5: b"\r\n\r\n"}
MODES = {0x08: "host", 0x00: "level", 0x02: "pulse", 0x04: "continuous"}
VERSION_RESPONSE = (
    b"\x58\xa4\x00\x00Product Name:SE630 Product ID:0001 "
    b"Hardware version:1.0 Firmware version:2.2.18\xe4D"
)
OP_SETTING = 0xC6
OP_VERSION = 0xA3
OP_START = 0xE4
OP_STOP = 0xE5
PARAM_MODE = 0x8A
PARAM_F2 = 0xF2
F2_TERMINATOR = 0x05
BYTE_TIME = 10 / 9600  # 8N1 at 9600 baud
WAKE_DELAY = 0.045  # 50 ms per the PDF, less a little for scheduling jitter


class FakeScanner:
    """A scanner on the master side of a pty; the firmware opens `link`."""

    def __init__(self, link: Path) -> None:
        """Create the pty and point `link` at its slave side."""
        self._master, slave = pty.openpty()
        tty.setraw(slave)
        self.link = link
        if link.is_symlink():
            link.unlink()
        link.symlink_to(os.ttyname(slave))
        self._slave = slave
        self.mode = "host"
        self.terminator = b""
        # What a host-mode start does: "scan" (ACK, then the barcode), "burst" (ACK
        # and barcode in a single write), "ack" (ACK only; the scan times out), "nak"
        # (rejected, as outside host mode) or "silent" (no reply at all).
        self.start_reply = "scan"
        self.barcode = b"HELLO-123"
        # Parameter bytes (frame[5]) of setting commands to NAK instead of ACK.
        self.nak_params: set[int] = set()
        # Reply to GET_VERSION (None: no reply at all).
        self.version_response: bytes | None = VERSION_RESPONSE
        self.received: list[bytes] = []
        # Frames ignored because they did not follow a wake-up byte by WAKE_DELAY.
        self.ignored: list[bytes] = []
        self._woken_at: float | None = None
        self._stop = threading.Event()
        self._thread = threading.Thread(target=self._run, daemon=True)
        # Output goes through a writer thread, so pacing and reply delays never hold up
        # reading (the wake-up timing is measured on arrival).
        self._out: queue.Queue[tuple[bytes | float, bool | None]] = queue.Queue()
        self._writer = threading.Thread(target=self._write, daemon=True)

    def __enter__(self) -> Self:
        """Start serving the protocol."""
        self._thread.start()
        self._writer.start()
        return self

    def __exit__(self, *exc: object) -> None:
        """Stop serving and release the pty."""
        self._stop.set()
        self._thread.join(timeout=2)
        self._writer.join(timeout=2)
        os.close(self._master)
        os.close(self._slave)
        self.link.unlink(missing_ok=True)

    def emit(self, data: bytes, *, paced: bool = True) -> None:
        """Queue bytes for the firmware: at line speed, or at once if not `paced`."""
        self._out.put((data, paced))

    def pause(self, seconds: float) -> None:
        """Queue a pause in the output."""
        self._out.put((seconds, None))

    def _write(self) -> None:
        while not self._stop.is_set():
            try:
                item, paced = self._out.get(timeout=0.05)
            except queue.Empty:
                continue
            if paced is None:
                time.sleep(item)
            elif not paced:
                os.write(self._master, item)
            else:
                for byte in item:
                    os.write(self._master, bytes([byte]))
                    time.sleep(BYTE_TIME)

    def emit_barcode(self, barcode: bytes) -> None:
        """Send a barcode followed by the currently configured terminator."""
        self.emit(barcode + self.terminator)

    def _handle(self, frame: bytes) -> None:
        self.received.append(frame)
        op = frame[1]
        if op == OP_SETTING:
            self._handle_setting(frame)
        elif op == OP_VERSION and self.version_response is not None:
            self.pause(0.07)
            self.emit(self.version_response)
        elif op in (OP_START, OP_STOP):
            self._handle_start_stop(op)

    def _handle_setting(self, frame: bytes) -> None:
        if frame[5] in self.nak_params:
            self.emit(NAK)
            return
        if frame[5] == PARAM_MODE:
            self.mode = MODES.get(frame[6], "auto_sense")
        elif frame[5] == PARAM_F2 and frame[6] == F2_TERMINATOR:
            self.terminator = TERMINATORS[frame[7]]
        self.emit(ACK)

    def _handle_start_stop(self, op: int) -> None:
        start = op == OP_START
        if start and self.start_reply == "silent":
            return
        if self.mode != "host" or (start and self.start_reply == "nak"):
            self.emit(NAK)
        elif start and self.start_reply == "burst":
            self.emit(ACK + self.barcode + self.terminator, paced=False)
        else:
            self.emit(ACK)
            if start and self.start_reply == "scan":
                self.pause(0.3)
                self.emit_barcode(self.barcode)

    def _run(self) -> None:
        os.set_blocking(self._master, False)
        buf = b""
        while not self._stop.is_set():
            try:
                buf += os.read(self._master, 256)
            except (BlockingIOError, OSError):
                time.sleep(0.002)
            buf = self._wake(buf)
            # Every command frame is: length byte, <length> bytes, 2-byte checksum.
            while buf and len(buf) >= buf[0] + 2:
                frame, buf = buf[: buf[0] + 2], buf[buf[0] + 2 :]
                woken_at, self._woken_at = self._woken_at, None
                if woken_at is None or time.monotonic() - woken_at < WAKE_DELAY:
                    self.ignored.append(frame)
                else:
                    self._handle(frame)
                buf = self._wake(buf)

    def _wake(self, buf: bytes) -> bytes:
        """Consume leading wake-up bytes, noting when the scanner woke."""
        if buf.startswith(b"\x00"):
            self._woken_at = time.monotonic()
        return buf.lstrip(b"\x00")
