from __future__ import annotations

import math
import operator
from collections.abc import Mapping, Sequence
from typing import Any


SERVICE_PROTOCOL_VERSION = 1


class ModelError(ValueError):
    """Raised when service node settings cannot be converted or applied."""


def _required(values: Mapping[str, object], key: str) -> object:
    try:
        return values[key]
    except KeyError as exc:
        raise ModelError(f"missing required field: {key}") from exc


def _required_bool(values: Mapping[str, object], key: str) -> bool:
    value = _required(values, key)
    if not isinstance(value, bool):
        raise ModelError(f"expected boolean for {key}")
    return value


def _required_int(values: Mapping[str, object], key: str) -> int:
    value = _required(values, key)
    if not isinstance(value, int) or isinstance(value, bool):
        raise ModelError(f"expected integer for {key}")
    return value


def _required_float(values: Mapping[str, object], key: str) -> float:
    value = _required(values, key)
    if not isinstance(value, int | float) or isinstance(value, bool):
        raise ModelError(f"expected number for {key}")
    return float(value)


def _required_finite_float(values: Mapping[str, object], key: str) -> float:
    value = _required_float(values, key)
    if not math.isfinite(value):
        raise ModelError(f"expected finite number for {key}")
    return value


def _validate_node_id(value: object, key: str) -> int:
    if isinstance(value, bool):
        raise ModelError(f"expected integer for {key}")
    try:
        node_id = operator.index(value)  # type: ignore[arg-type]
    except TypeError as exc:
        raise ModelError(f"expected integer for {key}") from exc
    if node_id < 0 or node_id > 255:
        raise ModelError(f"{key} outside valid range 0..255")
    return node_id


def node_record_from_get_values(requested_id: int, values: Mapping[str, object]) -> dict[str, object]:
    """Convert parsed service ``get`` values to a YAML node record."""
    settings: dict[str, object] = {
        "position": {
            "enabled": _required_bool(values, "pos_enabled"),
            "x": _required_float(values, "x"),
            "y": _required_float(values, "y"),
            "z": _required_float(values, "z"),
        },
        "radio": _required_int(values, "radio"),
    }

    smart_power = _required_bool(values, "smart_power")
    force_tx_power = _required_bool(values, "force_tx_power")
    if smart_power and not force_tx_power:
        settings["power"] = "default"
    else:
        settings["smart_power"] = smart_power
        settings["force_tx_power"] = force_tx_power
        settings["tx_power"] = _required(values, "tx_power")

    return {
        "id": _validate_node_id(values.get("node", requested_id), "node"),
        "settings": settings,
        "status": {
            "mode": _required_int(values, "mode"),
            "low_bitrate": _required_bool(values, "low_bitrate"),
            "long_preamble": _required_bool(values, "long_preamble"),
            "firmware_service_protocol_version": _required_int(values, "version"),
        },
    }


def dump_document(
    records: Sequence[Mapping[str, object]], port: str, dumped_at: str
) -> dict[str, object]:
    """Build the top-level YAML dump document."""
    return {
        "service_protocol_version": SERVICE_PROTOCOL_VERSION,
        "source": {
            "port": port,
            "dumped_at": dumped_at,
        },
        "nodes": list(records),
    }


def _record_mapping(record: Mapping[str, object], key: str) -> Mapping[str, object]:
    value = record.get(key)
    if not isinstance(value, Mapping):
        raise ModelError(f"expected mapping for {key}")
    return value


def _record_int(record: Mapping[str, object], key: str) -> int:
    value = record.get(key)
    if not isinstance(value, int) or isinstance(value, bool):
        raise ModelError(f"expected integer for {key}")
    return value


def _record_node_id(record: Mapping[str, object], key: str) -> int:
    return _validate_node_id(record.get(key), key)


def _validate_radio(value: object) -> int:
    if not isinstance(value, int) or isinstance(value, bool):
        raise ModelError("expected integer for radio")
    if value < 0 or value > 3:
        raise ModelError("radio outside valid range 0..3")
    return value


def _validate_power_db(value: object) -> int | float:
    if not isinstance(value, int | float) or isinstance(value, bool):
        raise ModelError("expected number for power_db")

    numeric = float(value)
    if numeric < 0.5 or numeric > 33.5:
        raise ModelError("power_db outside valid range 0.5..33.5")
    if not (numeric * 2).is_integer():
        raise ModelError("power_db must use 0.5 dB increments")

    return value


def _power_command(node_id: int, settings: Mapping[str, object]) -> str | None:
    raw_fields = {"tx_power", "smart_power", "force_tx_power"}
    present_raw_fields = raw_fields.intersection(settings)
    present_supported_fields = {"power", "power_db"}.intersection(settings)

    form_count = len(present_supported_fields) + (1 if present_raw_fields else 0)
    if form_count > 1:
        raise ModelError("multiple power settings specified")

    if present_raw_fields:
        fields = ", ".join(sorted(present_raw_fields))
        raise ModelError(f"raw power fields cannot be applied by service shell: {fields}")

    if "power" in settings:
        power = settings["power"]
        if power != "default":
            raise ModelError("unsupported power setting")
        return f"set power {node_id} default"

    if "power_db" in settings:
        power_db = _validate_power_db(settings["power_db"])
        return f"set power {node_id} {power_db}"

    return None


def apply_commands_for_node(record: Mapping[str, object]) -> list[str]:
    """Build service shell commands from node settings, ignoring status fields."""
    node_id = _record_node_id(record, "id")
    settings = _record_mapping(record, "settings")

    commands: list[str] = []

    if "position" in settings:
        position = settings["position"]
        if not isinstance(position, Mapping):
            raise ModelError("expected mapping for position")
        enabled = position.get("enabled")
        if not isinstance(enabled, bool):
            raise ModelError("expected boolean for position.enabled")
        if enabled:
            x = _required_finite_float(position, "x")
            y = _required_finite_float(position, "y")
            z = _required_finite_float(position, "z")
            commands.append(f"set pos {node_id} {x} {y} {z}")

    if "radio" in settings:
        radio = _validate_radio(settings["radio"])
        commands.append(f"set radio {node_id} {radio}")

    power_command = _power_command(node_id, settings)
    if power_command is not None:
        commands.append(power_command)

    return commands
