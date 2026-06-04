from __future__ import annotations

from collections.abc import Callable, Iterable
import re
import time
from typing import Any

import serial
from serial.tools import list_ports as serial_list_ports_module

from .protocol import (
    ControllerStatus,
    ProtocolParseError,
    ServiceError,
    ServiceOk,
    parse_shell_line,
    parse_status_line,
)


class PortSelectionError(RuntimeError):
    """Raised when the serial port cannot be selected automatically."""


class SerialLinkError(RuntimeError):
    """Raised for serial protocol and timeout errors."""


PortListFn = Callable[[], Iterable[Any]]
SerialFactory = Callable[..., Any]


_SERVICE_MODE_HINT = (
    "connected node is not in Service Controller mode; "
    "run 'lps-service controller setup --port <port>' and reset the node"
)


def _is_loco_node_port(port: Any) -> bool:
    haystack = " ".join(
        str(getattr(port, name, "") or "")
        for name in ("description", "manufacturer", "product", "hwid")
    ).lower()
    return "loco positioning node" in haystack or "bitcraze" in haystack


def _default_list_ports() -> list[Any]:
    return list(serial_list_ports_module.comports())


def select_port(explicit_port: str | None, list_ports: PortListFn | None = None) -> str:
    if explicit_port is not None:
        return explicit_port

    provider = list_ports if list_ports is not None else _default_list_ports
    candidates = [port for port in provider() if _is_loco_node_port(port)]

    if not candidates:
        raise PortSelectionError("no Loco Positioning Node serial port found; pass --port")

    if len(candidates) > 1:
        devices = ", ".join(str(getattr(port, "device", port)) for port in candidates)
        raise PortSelectionError(
            f"multiple Loco Positioning Node serial ports found: {devices}; pass --port"
        )

    return str(getattr(candidates[0], "device", candidates[0]))


class ServiceSerialClient:
    def __init__(
        self,
        port: str,
        *,
        baud: int = 115200,
        timeout: float = 1.0,
        serial_factory: SerialFactory | None = None,
        sleep_fn: Callable[[float], None] = time.sleep,
        monotonic_fn: Callable[[], float] = time.monotonic,
    ) -> None:
        self.port = port
        self.baud = baud
        self.timeout = timeout
        self._serial_factory = serial_factory or serial.Serial
        self._sleep = sleep_fn
        self._monotonic = monotonic_fn
        self._serial: Any | None = None
        self._service_shell_active = False

    def open(self) -> None:
        self._serial = self._serial_factory(self.port, self.baud, timeout=0)
        self._service_shell_active = False

    def close(self) -> None:
        if self._serial is not None:
            if self._service_shell_active:
                try:
                    self._write(b"exit\r")
                except Exception:
                    pass
                self._service_shell_active = False
            self._serial.close()
            self._serial = None

    def __enter__(self) -> ServiceSerialClient:
        self.open()
        return self

    def __exit__(self, exc_type: object, exc: object, tb: object) -> None:
        self.close()

    @property
    def serial(self) -> Any:
        if self._serial is None:
            raise SerialLinkError("serial port is not open")
        return self._serial

    def _write(self, data: bytes) -> None:
        self.serial.write(data)

    def _read_byte_until(self, deadline: float) -> bytes:
        while self._monotonic() < deadline:
            data = self.serial.read(1)
            if data:
                return data
            self._sleep(0.001)
        return b""

    def _read_line(self, deadline: float) -> str:
        data = bytearray()
        while self._monotonic() < deadline:
            byte = self._read_byte_until(deadline)
            if byte == b"":
                break
            if byte == b"\n":
                return data.decode(errors="replace").strip("\r")
            data.extend(byte)

        if data:
            partial = data.decode(errors="replace").strip("\r")
            raise SerialLinkError(
                f"timeout waiting for serial line; partial line: {partial!r}"
            )
        raise SerialLinkError("timeout waiting for serial line")

    def enter_service_shell(self) -> None:
        self._write(b"\x1bsvc\n")
        deadline = self._monotonic() + self.timeout

        while self._monotonic() < deadline:
            line = self._read_line(deadline).strip()
            if line.startswith("SVC READY"):
                self._service_shell_active = True
                return

            shell_line = _extract_shell_reply(line)
            if shell_line is None:
                continue

            try:
                parsed = parse_shell_line(shell_line)
            except ProtocolParseError as exc:
                raise SerialLinkError(
                    f"malformed service reply: {shell_line} ({exc})"
                ) from exc

            if isinstance(parsed, ServiceError) and parsed.code == "wrong_mode":
                raise SerialLinkError(_SERVICE_MODE_HINT)
            if isinstance(parsed, ServiceError):
                if parsed.code == "bad_value" and parsed.values.get("field") == "line":
                    self._confirm_active_service_shell(deadline)
                    self._service_shell_active = True
                    return
                raise SerialLinkError(f"service shell rejected entry: {shell_line}")

        raise SerialLinkError("timeout entering service shell")

    def _confirm_active_service_shell(self, deadline: float) -> None:
        self._write(b"help\r")

        while self._monotonic() < deadline:
            line = self._read_line(deadline).strip()
            shell_line = _extract_shell_reply(line)
            if shell_line is None:
                continue

            try:
                parsed = parse_shell_line(shell_line)
            except ProtocolParseError as exc:
                raise SerialLinkError(
                    f"malformed service reply: {shell_line} ({exc})"
                ) from exc

            if isinstance(parsed, ServiceOk) and "commands" in parsed.values:
                self._service_shell_active = True
                return
            if isinstance(parsed, ServiceError):
                raise SerialLinkError(f"service shell rejected active-shell check: {shell_line}")

        raise SerialLinkError("timeout confirming active service shell")

    def command(self, command: str) -> ServiceOk:
        self._write(command.encode("ascii") + b"\r")
        deadline = self._monotonic() + self.timeout

        while self._monotonic() < deadline:
            line = self._read_line(deadline).strip()
            shell_line = _extract_shell_reply(line)
            if shell_line is None:
                continue

            try:
                parsed = parse_shell_line(shell_line)
            except ProtocolParseError as exc:
                raise SerialLinkError(
                    f"malformed service reply: {shell_line} ({exc})"
                ) from exc

            if isinstance(parsed, ServiceError):
                raise SerialLinkError(f"service command failed: {shell_line}")
            return parsed

        raise SerialLinkError(f"timeout waiting for reply to {command!r}")

    def controller_status(self) -> ControllerStatus:
        self._write(b"?")
        deadline = self._monotonic() + self.timeout

        while self._monotonic() < deadline:
            line = self._read_line(deadline).strip()
            if line.startswith("STATUS "):
                return parse_status_line(line)

        raise SerialLinkError("timeout waiting for controller status")

    def controller_setup(self) -> None:
        self._write(b"m")
        self._sleep(0.05)
        self._write(b"5")
        deadline = self._monotonic() + self.timeout

        while self._monotonic() < deadline:
            line = self._read_line(deadline)
            if "New device mode:" in line and "Service Controller" in line:
                return

        raise SerialLinkError("timeout waiting for service-controller setup confirmation")

    def controller_radio_status(self) -> int:
        self._write(b"r")
        deadline = self._monotonic() + self.timeout
        radio_mode: int | None = None

        while self._monotonic() < deadline:
            line = self._read_line(deadline)
            lowered = line.lower()
            if "current mode is" in lowered:
                low_bitrate = "low bitrate" in lowered
                long_preamble = "long preamble" in lowered
                radio_mode = (1 if low_bitrate else 0) | (2 if long_preamble else 0)
                break

        if radio_mode is None:
            raise SerialLinkError("timeout waiting for radio-mode status")

        self._write(b"x")
        while self._monotonic() < deadline:
            line = self._read_line(deadline)
            if "Incorrect mode" in line:
                return radio_mode

        raise SerialLinkError("timeout leaving radio-mode menu")

    def controller_radio_set(self, radio_mode: int) -> None:
        if radio_mode < 0 or radio_mode > 3:
            raise ValueError("radio mode must be in range 0..3")

        self._write(b"r")
        self._sleep(0.05)
        self._write(str(radio_mode).encode("ascii"))
        deadline = self._monotonic() + self.timeout

        while self._monotonic() < deadline:
            line = self._read_line(deadline)
            if "New radio mode:" in line:
                return

        raise SerialLinkError("timeout waiting for radio-mode setup confirmation")

    def wait_for_service_shell(self, total_timeout: float = 30.0) -> None:
        deadline = self._monotonic() + total_timeout
        last_error: Exception | None = None
        self.close()

        while self._monotonic() < deadline:
            try:
                self.open()
                self.enter_service_shell()
                return
            except Exception as exc:
                last_error = exc
                self.close()
                self._sleep(0.5)

        detail = f": {last_error}" if last_error is not None else ""
        raise SerialLinkError(f"timeout waiting for service shell after reset{detail}")


def _extract_shell_reply(line: str) -> str | None:
    match = re.search(r"(?<!\S)(?:OK|ERR)(?=\s|$)", line)
    if match is None:
        return None

    return line[match.start():]
