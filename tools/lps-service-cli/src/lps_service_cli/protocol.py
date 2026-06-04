from __future__ import annotations

from dataclasses import dataclass
import shlex
import string
from typing import Any


class ProtocolParseError(ValueError):
    """Raised when a service-shell line cannot be parsed."""


@dataclass(frozen=True)
class ServiceOk:
    values: dict[str, Any]


@dataclass(frozen=True)
class ServiceError:
    code: str
    values: dict[str, Any]


@dataclass(frozen=True)
class ControllerStatus:
    node_id: int
    mode: int
    mode_name: str
    radio: int
    low_bitrate: bool
    long_preamble: bool


def _convert_value(key: str, value: str) -> Any:
    if key in {"smart_power", "force_tx_power", "pos_enabled", "low_bitrate", "long_preamble"}:
        if value not in {"0", "1"}:
            raise ProtocolParseError(f"expected boolean 0/1 for {key}")
        return value == "1"

    if value.startswith("0x") or value.startswith("0X"):
        hex_digits = value[2:]
        if hex_digits == "" or any(digit not in string.hexdigits for digit in hex_digits):
            raise ProtocolParseError(f"expected valid hex value for {key}")
        return "0x" + hex_digits.lower().zfill(8)

    try:
        return int(value)
    except ValueError:
        pass

    try:
        return float(value)
    except ValueError:
        return value


def _split_tokens(line: str) -> list[str]:
    try:
        return shlex.split(line)
    except ValueError as exc:
        raise ProtocolParseError(str(exc)) from exc


def _required_value(values: dict[str, Any], key: str) -> Any:
    try:
        return values[key]
    except KeyError as exc:
        raise ProtocolParseError(f"missing required field: {key}") from exc


def _required_int(values: dict[str, Any], key: str) -> int:
    value = _required_value(values, key)
    if not isinstance(value, int) or isinstance(value, bool):
        raise ProtocolParseError(f"expected integer for {key}")
    return value


def _required_bool(values: dict[str, Any], key: str) -> bool:
    value = _required_value(values, key)
    if not isinstance(value, bool):
        raise ProtocolParseError(f"expected boolean for {key}")
    return value


def _parse_key_values(tokens: list[str]) -> dict[str, Any]:
    values: dict[str, Any] = {}
    for token in tokens:
        if "=" not in token:
            raise ProtocolParseError(f"expected key=value token, got '{token}'")
        key, value = token.split("=", 1)
        if key == "":
            raise ProtocolParseError("empty key")
        values[key] = _convert_value(key, value)
    return values


def parse_shell_line(line: str) -> ServiceOk | ServiceError:
    stripped = line.strip()
    if stripped.startswith("OK "):
        return ServiceOk(_parse_key_values(_split_tokens(stripped)[1:]))

    if stripped.startswith("ERR "):
        values = _parse_key_values(_split_tokens(stripped)[1:])
        code = values.get("code")
        if not isinstance(code, str):
            raise ProtocolParseError("ERR line missing string code")
        return ServiceError(code=code, values=values)

    raise ProtocolParseError(f"unsupported service shell line: {line!r}")


def parse_status_line(line: str) -> ControllerStatus:
    stripped = line.strip()
    if not stripped.startswith("STATUS "):
        raise ProtocolParseError(f"unsupported status line: {line!r}")

    values = _parse_key_values(_split_tokens(stripped)[1:])
    return ControllerStatus(
        node_id=_required_int(values, "id"),
        mode=_required_int(values, "mode"),
        mode_name=str(_required_value(values, "mode_name")),
        radio=_required_int(values, "radio"),
        low_bitrate=_required_bool(values, "low_bitrate"),
        long_preamble=_required_bool(values, "long_preamble"),
    )
