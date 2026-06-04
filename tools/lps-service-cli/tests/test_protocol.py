import pytest

from lps_service_cli.protocol import (
    ControllerStatus,
    ProtocolParseError,
    ServiceError,
    ServiceOk,
    parse_shell_line,
    parse_status_line,
)


def test_parse_ok_line_with_typed_values() -> None:
    parsed = parse_shell_line(
        "OK req=4660 target=5 node=5 mode=4 pos_enabled=1 "
        "x=1.250 y=-2.500 z=3.750 smart_power=1 force_tx_power=0 "
        "tx_power=0x07274767 radio=1 low_bitrate=1 long_preamble=0 version=1"
    )

    assert isinstance(parsed, ServiceOk)
    assert parsed.values["req"] == 4660
    assert parsed.values["target"] == 5
    assert parsed.values["x"] == 1.25
    assert parsed.values["y"] == -2.5
    assert parsed.values["tx_power"] == "0x07274767"
    assert parsed.values["smart_power"] is True
    assert parsed.values["force_tx_power"] is False


def test_parse_err_line() -> None:
    parsed = parse_shell_line("ERR code=timeout target=7 attempts=3")

    assert isinstance(parsed, ServiceError)
    assert parsed.code == "timeout"
    assert parsed.values["target"] == 7
    assert parsed.values["attempts"] == 3


def test_parse_status_line() -> None:
    parsed = parse_status_line(
        'STATUS id=3 mode=5 mode_name="Service Controller" radio=0 low_bitrate=0 long_preamble=0'
    )

    assert parsed == ControllerStatus(
        node_id=3,
        mode=5,
        mode_name="Service Controller",
        radio=0,
        low_bitrate=False,
        long_preamble=False,
    )


def test_rejects_unknown_shell_line() -> None:
    with pytest.raises(ProtocolParseError, match="unsupported"):
        parse_shell_line("svc> ")


def test_rejects_malformed_quotes_in_ok_line() -> None:
    with pytest.raises(ProtocolParseError):
        parse_shell_line('OK mode_name="unterminated')


def test_rejects_missing_required_status_field() -> None:
    with pytest.raises(ProtocolParseError):
        parse_status_line(
            'STATUS id=3 mode_name="Service Controller" radio=0 low_bitrate=0 long_preamble=0'
        )


def test_rejects_invalid_status_integer_field() -> None:
    with pytest.raises(ProtocolParseError):
        parse_status_line(
            'STATUS id=3 mode=abc mode_name="Service Controller" radio=0 low_bitrate=0 long_preamble=0'
        )


def test_rejects_decimal_status_integer_field() -> None:
    with pytest.raises(ProtocolParseError):
        parse_status_line(
            'STATUS id=3 mode=5.9 mode_name="Service Controller" radio=0 low_bitrate=0 long_preamble=0'
        )


def test_rejects_invalid_boolean_field() -> None:
    with pytest.raises(ProtocolParseError):
        parse_shell_line("OK smart_power=2")


def test_rejects_malformed_ok_token_without_separator() -> None:
    with pytest.raises(ProtocolParseError):
        parse_shell_line("OK malformed")


def test_rejects_invalid_hex_field() -> None:
    with pytest.raises(ProtocolParseError):
        parse_shell_line("OK tx_power=0xnothex")
