from __future__ import annotations

from dataclasses import dataclass

import pytest

from lps_service_cli.protocol import ControllerStatus, ServiceOk
from lps_service_cli.serial_link import (
    PortSelectionError,
    SerialLinkError,
    ServiceSerialClient,
    select_port,
)


@dataclass
class FakePort:
    device: str
    description: str
    manufacturer: str | None = None
    product: str | None = None


class FakeSerial:
    def __init__(self, responses: list[bytes]) -> None:
        self.responses = bytearray(b"".join(responses))
        self.writes: list[bytes] = []
        self.is_open = True

    def write(self, data: bytes) -> int:
        self.writes.append(data)
        return len(data)

    def read(self, size: int = 1) -> bytes:
        if not self.responses:
            return b""
        data = self.responses[:size]
        del self.responses[:size]
        return bytes(data)

    def close(self) -> None:
        self.is_open = False


def test_select_port_uses_explicit_port() -> None:
    assert select_port("/dev/ttyACM9", list_ports=lambda: []) == "/dev/ttyACM9"


def test_select_port_autodetects_loco_node() -> None:
    ports = [FakePort("/dev/ttyACM0", "Loco Positioning Node")]

    assert select_port(None, list_ports=lambda: ports) == "/dev/ttyACM0"


def test_select_port_autodetects_bitcraze_metadata() -> None:
    ports = [FakePort("/dev/ttyACM0", "USB Serial", manufacturer="Bitcraze")]

    assert select_port(None, list_ports=lambda: ports) == "/dev/ttyACM0"


def test_select_port_rejects_no_candidates() -> None:
    with pytest.raises(PortSelectionError, match="--port"):
        select_port(None, list_ports=lambda: [FakePort("/dev/ttyUSB0", "Other")])


def test_select_port_rejects_multiple_candidates() -> None:
    ports = [
        FakePort("/dev/ttyACM0", "Loco Positioning Node"),
        FakePort("/dev/ttyACM1", "USB Serial", product="Bitcraze LPS Node"),
    ]

    with pytest.raises(PortSelectionError, match="multiple.*ttyACM0.*ttyACM1"):
        select_port(None, list_ports=lambda: ports)


def test_context_manager_opens_and_closes_serial() -> None:
    fake = FakeSerial([])

    with ServiceSerialClient("/dev/fake", serial_factory=lambda *args, **kwargs: fake) as client:
        assert client.serial is fake
        assert fake.is_open is True

    assert fake.is_open is False


def test_close_exits_active_service_shell() -> None:
    fake = FakeSerial([b"SVC READY version=1\r\nsvc> "])
    client = ServiceSerialClient("/dev/fake", serial_factory=lambda *args, **kwargs: fake)

    client.open()
    client.enter_service_shell()
    client.close()

    assert fake.writes == [b"\x1bsvc\n", b"exit\r"]
    assert fake.is_open is False


def test_enter_service_shell_sends_escape_and_detects_ready() -> None:
    fake = FakeSerial([b"SVC READY version=1\r\nsvc> "])
    client = ServiceSerialClient("/dev/fake", serial_factory=lambda *args, **kwargs: fake)

    client.open()
    client.enter_service_shell()

    assert fake.writes == [b"\x1bsvc\n"]


def test_enter_service_shell_recovers_when_shell_is_already_active() -> None:
    fake = FakeSerial([
        b"ERR code=bad_value field=line\r\nsvc> ",
        b"OK commands=help,local,radio,get,set,exit\r\nsvc> ",
    ])
    client = ServiceSerialClient("/dev/fake", serial_factory=lambda *args, **kwargs: fake)

    client.open()
    client.enter_service_shell()

    assert fake.writes == [b"\x1bsvc\n", b"help\r"]


def test_enter_service_shell_reports_wrong_mode() -> None:
    fake = FakeSerial([b"ERR code=wrong_mode need=service\r\n"])
    client = ServiceSerialClient("/dev/fake", serial_factory=lambda *args, **kwargs: fake)

    client.open()
    with pytest.raises(SerialLinkError, match="Service Controller mode"):
        client.enter_service_shell()


def test_service_command_returns_parsed_ok_line() -> None:
    fake = FakeSerial([
        b"SVC READY version=1\r\nsvc> ",
        b"OK req=1 target=5 status=active reset=0\r\nsvc> ",
    ])
    client = ServiceSerialClient("/dev/fake", serial_factory=lambda *args, **kwargs: fake)

    client.open()
    client.enter_service_shell()
    result = client.command("set radio 5 1")

    assert isinstance(result, ServiceOk)
    assert result.values["target"] == 5
    assert fake.writes[-1] == b"set radio 5 1\r"


def test_service_command_handles_prompt_prefix_before_ok() -> None:
    fake = FakeSerial([
        b"svc> OK req=1 target=5 status=active reset=0\r\nsvc> ",
    ])
    client = ServiceSerialClient("/dev/fake", serial_factory=lambda *args, **kwargs: fake)

    client.open()
    result = client.command("get 5")

    assert isinstance(result, ServiceOk)
    assert result.values == {"req": 1, "target": 5, "status": "active", "reset": 0}


def test_service_command_raises_on_service_err() -> None:
    fake = FakeSerial([b"ERR code=timeout target=5\r\nsvc> "])
    client = ServiceSerialClient("/dev/fake", serial_factory=lambda *args, **kwargs: fake)

    client.open()
    with pytest.raises(SerialLinkError, match="timeout"):
        client.command("get 5")


def test_service_command_rejects_partial_ok_without_newline() -> None:
    fake = FakeSerial([b"OK req=1 target=5"])
    client = ServiceSerialClient(
        "/dev/fake",
        timeout=0.01,
        serial_factory=lambda *args, **kwargs: fake,
    )

    client.open()
    with pytest.raises(SerialLinkError, match="timeout|partial"):
        client.command("get 5")


def test_service_command_reports_malformed_ok_reply() -> None:
    fake = FakeSerial([b"OK badtoken\r\n"])
    client = ServiceSerialClient(
        "/dev/fake",
        timeout=0.01,
        serial_factory=lambda *args, **kwargs: fake,
    )

    client.open()
    with pytest.raises(SerialLinkError, match="malformed service reply|protocol"):
        client.command("get 5")


def test_service_command_reports_bare_ok_reply_as_malformed() -> None:
    fake = FakeSerial([b"OK\r\n"])
    client = ServiceSerialClient(
        "/dev/fake",
        timeout=0.01,
        serial_factory=lambda *args, **kwargs: fake,
    )

    client.open()
    with pytest.raises(SerialLinkError, match="malformed"):
        client.command("get 5")


def test_service_command_reports_malformed_err_reply() -> None:
    fake = FakeSerial([b"ERR badtoken\r\n"])
    client = ServiceSerialClient(
        "/dev/fake",
        timeout=0.01,
        serial_factory=lambda *args, **kwargs: fake,
    )

    client.open()
    with pytest.raises(SerialLinkError, match="malformed service reply|protocol"):
        client.command("get 5")


def test_service_command_reports_bare_err_reply_as_malformed() -> None:
    fake = FakeSerial([b"ERR\r\n"])
    client = ServiceSerialClient(
        "/dev/fake",
        timeout=0.01,
        serial_factory=lambda *args, **kwargs: fake,
    )

    client.open()
    with pytest.raises(SerialLinkError, match="malformed"):
        client.command("get 5")


def test_service_command_reports_prompt_prefixed_bare_ok_reply_as_malformed() -> None:
    fake = FakeSerial([b"svc> OK\r\n"])
    client = ServiceSerialClient(
        "/dev/fake",
        timeout=0.01,
        serial_factory=lambda *args, **kwargs: fake,
    )

    client.open()
    with pytest.raises(SerialLinkError, match="malformed"):
        client.command("get 5")


def test_enter_service_shell_reports_bare_err_reply_as_malformed() -> None:
    fake = FakeSerial([b"ERR\r\n"])
    client = ServiceSerialClient(
        "/dev/fake",
        timeout=0.01,
        serial_factory=lambda *args, **kwargs: fake,
    )

    client.open()
    with pytest.raises(SerialLinkError, match="malformed"):
        client.enter_service_shell()


def test_controller_status_reads_stable_status_line() -> None:
    fake = FakeSerial([
        b'STATUS id=3 mode=5 mode_name="Service Controller" radio=0 low_bitrate=0 long_preamble=0\r\n'
    ])
    client = ServiceSerialClient("/dev/fake", serial_factory=lambda *args, **kwargs: fake)

    client.open()
    status = client.controller_status()

    assert status == ControllerStatus(
        node_id=3,
        mode=5,
        mode_name="Service Controller",
        radio=0,
        low_bitrate=False,
        long_preamble=False,
    )
    assert fake.writes == [b"?"]


def test_controller_setup_sends_menu_mode_sequence() -> None:
    fake = FakeSerial([b"New device mode: Service Controller\r\n"])
    client = ServiceSerialClient(
        "/dev/fake",
        serial_factory=lambda *args, **kwargs: fake,
        sleep_fn=lambda _: None,
    )

    client.open()
    client.controller_setup()

    assert fake.writes == [b"m", b"5"]


def test_controller_radio_set_sends_menu_radio_sequence() -> None:
    fake = FakeSerial([b"New radio mode: low bitrate, normal preamble\r\n"])
    client = ServiceSerialClient(
        "/dev/fake",
        serial_factory=lambda *args, **kwargs: fake,
        sleep_fn=lambda _: None,
    )

    client.open()
    client.controller_radio_set(1)

    assert fake.writes == [b"r", b"1"]


def test_controller_radio_status_reads_persistent_menu_radio_mode() -> None:
    fake = FakeSerial([
        b"-------------------\r\n",
        b"Current mode is low bitrate, long preamble\r\n",
        b"Available radio modes:\r\n",
        b"Type 0-9 to choose new mode...\r\n",
        b"Incorrect mode 'x'\r\n",
    ])
    client = ServiceSerialClient(
        "/dev/fake",
        serial_factory=lambda *args, **kwargs: fake,
        sleep_fn=lambda _: None,
    )

    client.open()
    radio_mode = client.controller_radio_status()

    assert radio_mode == 3
    assert fake.writes == [b"r", b"x"]


def test_controller_radio_set_rejects_invalid_mode() -> None:
    fake = FakeSerial([])
    client = ServiceSerialClient("/dev/fake", serial_factory=lambda *args, **kwargs: fake)

    client.open()
    with pytest.raises(ValueError, match="radio mode"):
        client.controller_radio_set(4)

    assert fake.writes == []


def test_wait_for_service_shell_retries_until_ready() -> None:
    first = FakeSerial([b"ERR code=wrong_mode need=service\r\n"])
    second = FakeSerial([b"SVC READY version=1\r\nsvc> "])
    serials = [first, second]

    def serial_factory(*args: object, **kwargs: object) -> FakeSerial:
        return serials.pop(0)

    client = ServiceSerialClient(
        "/dev/fake",
        timeout=0.01,
        serial_factory=serial_factory,
        sleep_fn=lambda _: None,
    )

    client.wait_for_service_shell(total_timeout=1.0)

    assert first.writes == [b"\x1bsvc\n"]
    assert first.is_open is False
    assert second.writes == [b"\x1bsvc\n"]


def test_wait_for_service_shell_times_out_when_ready_never_arrives() -> None:
    now = 0.0
    opened_serials: list[FakeSerial] = []

    def monotonic() -> float:
        return now

    def sleep(duration: float) -> None:
        nonlocal now
        now += duration

    def serial_factory(*args: object, **kwargs: object) -> FakeSerial:
        fake = FakeSerial([])
        opened_serials.append(fake)
        return fake

    client = ServiceSerialClient(
        "/dev/fake",
        timeout=0.01,
        serial_factory=serial_factory,
        sleep_fn=sleep,
        monotonic_fn=monotonic,
    )

    with pytest.raises(SerialLinkError, match="timeout"):
        client.wait_for_service_shell(total_timeout=0.05)

    assert opened_serials
    assert opened_serials[0].writes == [b"\x1bsvc\n"]
    assert all(fake.is_open is False for fake in opened_serials)
