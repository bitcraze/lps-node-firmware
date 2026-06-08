import pytest

from lps_service_cli.model import (
    ModelError,
    apply_commands_for_node,
    dump_document,
    node_record_from_get_values,
)
from lps_service_cli.yaml_io import dumps_yaml, loads_yaml


def sample_get_values() -> dict[str, object]:
    return {
        "req": 4660,
        "target": 5,
        "node": 5,
        "mode": 4,
        "pos_enabled": True,
        "x": 1.25,
        "y": 2.5,
        "z": 3.75,
        "smart_power": True,
        "force_tx_power": False,
        "tx_power": "0x07274767",
        "radio": 1,
        "channel": 5,
        "low_bitrate": True,
        "long_preamble": False,
        "version": 1,
    }


def test_get_values_map_to_settings_and_status_without_duplication() -> None:
    record = node_record_from_get_values(5, sample_get_values())

    assert record == {
        "id": 5,
        "settings": {
            "position": {"enabled": True, "x": 1.25, "y": 2.5, "z": 3.75},
            "radio": 1,
            "channel": 5,
            "power": "default",
        },
        "status": {
            "mode": 4,
            "low_bitrate": True,
            "long_preamble": False,
            "firmware_service_protocol_version": 1,
        },
    }


def test_get_values_uses_reported_node_id_when_present() -> None:
    values = sample_get_values()
    values["node"] = 9

    record = node_record_from_get_values(5, values)

    assert record["id"] == 9


def test_non_default_power_maps_to_raw_settings() -> None:
    values = sample_get_values()
    values["smart_power"] = False
    values["force_tx_power"] = True
    values["tx_power"] = "0x1f1f1f1f"

    record = node_record_from_get_values(5, values)

    assert record["settings"]["smart_power"] is False
    assert record["settings"]["force_tx_power"] is True
    assert record["settings"]["tx_power"] == "0x1f1f1f1f"
    assert "power" not in record["settings"]


def test_yaml_roundtrip_preserves_document() -> None:
    document = dump_document(
        records=[node_record_from_get_values(5, sample_get_values())],
        port="/dev/ttyACM0",
        dumped_at="2026-06-04T12:34:56Z",
    )

    text = dumps_yaml(document)
    loaded = loads_yaml(text)

    assert loaded == document
    assert "settings:" in text
    assert "status:" in text


def test_apply_uses_settings_only_and_ignores_status() -> None:
    record = {
        "id": 7,
        "settings": {
            "position": {"enabled": True, "x": 1.0, "y": 2.0, "z": 3.0},
            "radio": 2,
            "channel": 7,
            "power_db": 10.5,
        },
        "status": {"mode": 4, "radio": 0, "power": "ignored"},
    }

    assert apply_commands_for_node(record) == [
        "set pos 7 1.0 2.0 3.0",
        "set radio 7 2",
        "set channel 7 7",
        "set power 7 10.5",
    ]


@pytest.mark.parametrize("node_id", [999, -1])
def test_apply_rejects_node_id_outside_byte_range(node_id: int) -> None:
    record = {
        "id": node_id,
        "settings": {"radio": 1},
    }

    with pytest.raises(ModelError, match="0\\.\\.255"):
        apply_commands_for_node(record)


@pytest.mark.parametrize("coordinate", [float("nan"), float("inf")])
def test_apply_rejects_non_finite_position_coordinate(coordinate: float) -> None:
    record = {
        "id": 7,
        "settings": {
            "position": {"enabled": True, "x": coordinate, "y": 2.0, "z": 3.0},
        },
    }

    with pytest.raises(ModelError, match="finite"):
        apply_commands_for_node(record)


def test_get_values_rejects_reported_node_id_outside_byte_range() -> None:
    values = sample_get_values()
    values["node"] = 999

    with pytest.raises(ModelError, match="0\\.\\.255"):
        node_record_from_get_values(5, values)


def test_apply_rejects_raw_power_fields() -> None:
    record = {
        "id": 7,
        "settings": {"tx_power": "0x1f1f1f1f"},
    }

    with pytest.raises(ModelError):
        apply_commands_for_node(record)


@pytest.mark.parametrize("channel", [0, 6, 8])
def test_apply_rejects_invalid_channel(channel: int) -> None:
    record = {
        "id": 7,
        "settings": {"channel": channel},
    }

    with pytest.raises(ModelError, match="channel"):
        apply_commands_for_node(record)
