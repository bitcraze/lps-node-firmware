from __future__ import annotations

import errno
import os
from pathlib import Path
from typing import Any

import yaml

from lps_service_cli import cli
from lps_service_cli.protocol import ControllerStatus, ServiceOk
from lps_service_cli.serial_link import SerialLinkError


def sample_get_values(node_id: int) -> dict[str, object]:
    return {
        "node": node_id,
        "mode": 4,
        "pos_enabled": True,
        "x": 1.0 + node_id,
        "y": 2.0 + node_id,
        "z": 3.0 + node_id,
        "smart_power": True,
        "force_tx_power": False,
        "tx_power": "0x07274767",
        "radio": 1,
        "low_bitrate": True,
        "long_preamble": False,
        "version": 1,
    }


class FakeClient:
    def __init__(
        self,
        port: str = "/dev/fake",
        *,
        fail_commands: set[str] | None = None,
        controller_status_result: ControllerStatus | None = None,
        **kwargs: Any,
    ) -> None:
        self.port = port
        self.kwargs = kwargs
        self.fail_commands = fail_commands or set()
        self.controller_status_result = controller_status_result
        self.entered_service_shell = 0
        self.commands: list[str] = []
        self.closed = False
        self.controller_status_called = 0
        self.controller_setup_called = 0
        self.controller_radio_set_calls: list[int] = []
        self.controller_radio_status_result = 0
        self.wait_calls: list[float] = []

    def __enter__(self) -> "FakeClient":
        return self

    def __exit__(self, exc_type: object, exc: object, tb: object) -> None:
        self.closed = True

    def enter_service_shell(self) -> None:
        self.entered_service_shell += 1

    def command(self, command: str) -> ServiceOk:
        self.commands.append(command)
        if command in self.fail_commands:
            raise SerialLinkError(f"fake failure for {command}")
        if command.startswith("get "):
            node_id = int(command.split()[1])
            return ServiceOk(sample_get_values(node_id))
        return ServiceOk({})

    def controller_status(self) -> ControllerStatus:
        self.controller_status_called += 1
        if self.controller_status_result is not None:
            return self.controller_status_result
        return ControllerStatus(
            node_id=3,
            mode=5,
            mode_name="Service Controller",
            radio=0,
            low_bitrate=False,
            long_preamble=False,
        )

    def controller_setup(self) -> None:
        self.controller_setup_called += 1

    def controller_radio_status(self) -> int:
        return self.controller_radio_status_result

    def controller_radio_set(self, radio: int) -> None:
        self.controller_radio_set_calls.append(radio)
        self.controller_radio_status_result = radio

    def wait_for_service_shell(self, total_timeout: float = 30.0) -> None:
        self.wait_calls.append(total_timeout)


class FakeFactory:
    def __init__(
        self,
        *,
        fail_commands: set[str] | None = None,
        controller_status_result: ControllerStatus | None = None,
    ) -> None:
        self.fail_commands = fail_commands
        self.controller_status_result = controller_status_result
        self.clients: list[FakeClient] = []

    def __call__(self, port: str, **kwargs: Any) -> FakeClient:
        client = FakeClient(
            port,
            fail_commands=self.fail_commands,
            controller_status_result=self.controller_status_result,
            **kwargs,
        )
        self.clients.append(client)
        return client

    @property
    def client(self) -> FakeClient:
        assert self.clients
        return self.clients[-1]


def test_get_prints_yaml_record_and_sends_get(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(["--port", "/dev/fake", "get", "5"], client_factory=factory)

    assert exit_code == 0
    assert factory.client.entered_service_shell == 1
    assert factory.client.commands == ["get 5"]
    document = yaml.safe_load(capsys.readouterr().out)
    assert document["source"]["port"] == "/dev/fake"
    assert document["nodes"][0]["id"] == 5
    assert document["nodes"][0]["settings"]["position"]["x"] == 6.0


def test_get_accepts_global_port_after_subcommand(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(["get", "5", "--port", "/dev/ttyACM0"], client_factory=factory)

    assert exit_code == 0
    assert factory.client.entered_service_shell == 1
    assert factory.client.commands == ["get 5"]
    document = yaml.safe_load(capsys.readouterr().out)
    assert document["source"]["port"] == "/dev/ttyACM0"
    assert document["nodes"][0]["id"] == 5


def test_get_verbose_after_subcommand_prints_selected_port(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["get", "5", "--port", "/dev/ttyACM0", "--verbose"], client_factory=factory
    )

    assert exit_code == 0
    assert factory.client.commands == ["get 5"]
    assert "DEBUG port=/dev/ttyACM0" in capsys.readouterr().err


def test_set_accepts_global_port_after_command_options(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["set", "1..2", "--power", "default", "--port", "/dev/ttyACM0"],
        client_factory=factory,
    )

    assert exit_code == 0
    assert factory.client.commands == ["set power 1 default", "set power 2 default"]
    assert capsys.readouterr().out.count("OK") == 2


def test_set_range_power_db_sends_command_for_each_node(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["--port", "/dev/fake", "set", "1..3", "--power-db", "10.5"],
        client_factory=factory,
    )

    assert exit_code == 0
    assert factory.client.commands == [
        "set power 1 10.5",
        "set power 2 10.5",
        "set power 3 10.5",
    ]
    assert capsys.readouterr().out.count("OK") == 3


def test_dump_range_writes_yaml_file(tmp_path: Path) -> None:
    factory = FakeFactory()
    output = tmp_path / "anchors.yaml"

    exit_code = cli.run(
        ["--port", "/dev/fake", "dump", "1..2", "-o", str(output)],
        client_factory=factory,
    )

    assert exit_code == 0
    assert factory.client.commands == ["get 1", "get 2"]
    document = yaml.safe_load(output.read_text(encoding="utf-8"))
    assert [node["id"] for node in document["nodes"]] == [1, 2]


def test_dump_missing_output_parent_returns_error_without_opening_serial(
    capsys: Any, tmp_path: Path
) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        [
            "--port",
            "/dev/ttyACM0",
            "dump",
            "1",
            "-o",
            str(tmp_path / "missing" / "anchors.yaml"),
        ],
        client_factory=factory,
    )

    assert exit_code == 1
    assert factory.clients == []
    assert "ERROR" in capsys.readouterr().err


def test_dump_unwritable_existing_output_returns_error_without_opening_serial(
    capsys: Any, monkeypatch: Any, tmp_path: Path
) -> None:
    factory = FakeFactory()
    output = tmp_path / "anchors.yaml"
    output.write_text("existing content\n", encoding="utf-8")
    output.chmod(0o400)
    real_os_open = os.open

    def deny_output_open(
        path: str | bytes | os.PathLike[str] | os.PathLike[bytes],
        flags: int,
        mode: int = 0o777,
        *,
        dir_fd: int | None = None,
    ) -> int:
        if Path(path) == output:
            raise PermissionError(errno.EACCES, "Permission denied", str(path))
        if dir_fd is None:
            return real_os_open(path, flags, mode)
        return real_os_open(path, flags, mode, dir_fd=dir_fd)

    monkeypatch.setattr(os, "open", deny_output_open)

    try:
        exit_code = cli.run(
            ["--port", "/dev/ttyACM0", "dump", "1", "-o", str(output)],
            client_factory=factory,
        )
    finally:
        output.chmod(0o600)

    assert exit_code == 1
    assert factory.clients == []
    assert "ERROR" in capsys.readouterr().err


def test_apply_ignores_status_and_sends_only_settings_commands(tmp_path: Path) -> None:
    factory = FakeFactory()
    document = {
        "nodes": [
            {
                "id": 7,
                "settings": {
                    "position": {"enabled": True, "x": 1.0, "y": 2.0, "z": 3.0},
                    "radio": 2,
                    "power_db": 10.5,
                },
                "status": {"mode": 4, "radio": 0, "power": "ignored"},
            }
        ]
    }
    path = tmp_path / "anchors.yaml"
    path.write_text(yaml.safe_dump(document), encoding="utf-8")

    exit_code = cli.run(["--port", "/dev/fake", "apply", str(path)], client_factory=factory)

    assert exit_code == 0
    assert factory.client.commands == [
        "set pos 7 1.0 2.0 3.0",
        "set radio 7 2",
        "set power 7 10.5",
    ]


def test_apply_missing_file_returns_error_without_opening_serial(
    capsys: Any, tmp_path: Path
) -> None:
    factory = FakeFactory()
    missing = tmp_path / "missing.yaml"

    exit_code = cli.run(["--port", "/dev/fake", "apply", str(missing)], client_factory=factory)

    assert exit_code == 1
    assert factory.clients == []
    assert "ERROR" in capsys.readouterr().err


def test_apply_continue_on_error_skips_failed_node_remaining_commands(
    capsys: Any, tmp_path: Path
) -> None:
    factory = FakeFactory(fail_commands={"set radio 1 2"})
    document = {
        "nodes": [
            {"id": 1, "settings": {"radio": 2, "power": "default"}},
            {"id": 2, "settings": {"radio": 3}},
        ]
    }
    path = tmp_path / "anchors.yaml"
    path.write_text(yaml.safe_dump(document), encoding="utf-8")

    exit_code = cli.run(
        [
            "--port",
            "/dev/fake",
            "--retries",
            "1",
            "apply",
            str(path),
            "--continue-on-error",
        ],
        client_factory=factory,
    )

    assert exit_code == 1
    assert factory.client.commands == ["set radio 1 2", "set radio 2 3"]
    assert "ERROR" in capsys.readouterr().err


def test_apply_stops_on_first_failed_command_without_continue_on_error(
    capsys: Any, tmp_path: Path
) -> None:
    factory = FakeFactory(fail_commands={"set radio 1 2"})
    document = {
        "nodes": [
            {"id": 1, "settings": {"radio": 2, "power": "default"}},
            {"id": 2, "settings": {"radio": 3}},
        ]
    }
    path = tmp_path / "anchors.yaml"
    path.write_text(yaml.safe_dump(document), encoding="utf-8")

    exit_code = cli.run(
        ["--port", "/dev/fake", "--retries", "1", "apply", str(path)],
        client_factory=factory,
    )

    assert exit_code == 1
    assert factory.client.commands == ["set radio 1 2"]
    assert "ERROR" in capsys.readouterr().err


def test_dump_continues_after_node_failure_and_returns_nonzero(capsys: Any) -> None:
    factory = FakeFactory(fail_commands={"get 2"})

    exit_code = cli.run(
        ["--port", "/dev/fake", "--retries", "1", "dump", "1..3"],
        client_factory=factory,
    )

    assert exit_code == 1
    assert factory.client.commands == ["get 1", "get 2", "get 3"]
    captured = capsys.readouterr()
    document = yaml.safe_load(captured.out)
    assert [node["id"] for node in document["nodes"]] == [1, 3]
    assert "ERROR node=2" in captured.err


def test_set_stops_on_first_failure_without_continue_on_error(capsys: Any) -> None:
    factory = FakeFactory(fail_commands={"set power 2 10.5"})

    exit_code = cli.run(
        ["--port", "/dev/fake", "--retries", "1", "set", "1..3", "--power-db", "10.5"],
        client_factory=factory,
    )

    assert exit_code == 1
    assert factory.client.commands == ["set power 1 10.5", "set power 2 10.5"]
    assert "ERROR" in capsys.readouterr().err


def test_set_continues_after_failure_with_continue_on_error(capsys: Any) -> None:
    factory = FakeFactory(fail_commands={"set power 2 10.5"})

    exit_code = cli.run(
        [
            "--port",
            "/dev/fake",
            "--retries",
            "1",
            "set",
            "1..3",
            "--power-db",
            "10.5",
            "--continue-on-error",
        ],
        client_factory=factory,
    )

    assert exit_code == 1
    assert factory.client.commands == [
        "set power 1 10.5",
        "set power 2 10.5",
        "set power 3 10.5",
    ]
    assert "ERROR" in capsys.readouterr().err


def test_invalid_radio_returns_error_without_opening_serial(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["--port", "/dev/fake", "set", "1", "--radio", "4"],
        client_factory=factory,
    )

    assert exit_code == 1
    assert factory.clients == []
    assert "ERROR" in capsys.readouterr().err


def test_invalid_dump_selector_returns_error_without_opening_serial(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["--port", "/dev/fake", "dump", "5..3"],
        client_factory=factory,
    )

    assert exit_code == 1
    assert factory.clients == []
    assert "ERROR" in capsys.readouterr().err


def test_invalid_power_db_increment_returns_error_without_opening_serial(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["--port", "/dev/fake", "set", "1", "--power-db", "10.25"],
        client_factory=factory,
    )

    assert exit_code == 1
    assert factory.clients == []
    assert "ERROR" in capsys.readouterr().err


def test_argparse_usage_error_returns_error(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(["set", "1"], client_factory=factory)

    assert exit_code == 1
    assert "ERROR" in capsys.readouterr().err


def test_invalid_baud_returns_error_without_opening_serial(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["--port", "/dev/ttyACM0", "--baud", "0", "get", "1"], client_factory=factory
    )

    assert exit_code == 1
    assert factory.clients == []
    assert "ERROR" in capsys.readouterr().err


def test_invalid_timeout_returns_error_without_opening_serial(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["--port", "/dev/ttyACM0", "--timeout", "0", "get", "1"], client_factory=factory
    )

    assert exit_code == 1
    assert factory.clients == []
    assert "ERROR" in capsys.readouterr().err


def test_invalid_retries_returns_error_without_opening_serial(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["--port", "/dev/ttyACM0", "--retries", "0", "get", "1"], client_factory=factory
    )

    assert exit_code == 1
    assert factory.clients == []
    assert "ERROR" in capsys.readouterr().err


def test_controller_status_does_not_enter_service_shell_and_prints_service_controller(
    capsys: Any,
) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["--port", "/dev/fake", "controller", "status"], client_factory=factory
    )

    assert exit_code == 0
    assert factory.client.entered_service_shell == 0
    assert factory.client.controller_status_called == 1
    out = capsys.readouterr().out
    assert "service_controller: yes" in out
    assert "Service Controller" in out


def test_controller_status_accepts_global_port_after_subcommand(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["controller", "status", "--port", "/dev/ttyACM0"], client_factory=factory
    )

    assert exit_code == 0
    assert factory.client.entered_service_shell == 0
    assert factory.client.controller_status_called == 1
    out = capsys.readouterr().out
    assert "service_controller: yes" in out


def test_controller_status_uses_mode_five_for_service_controller(capsys: Any) -> None:
    factory = FakeFactory(
        controller_status_result=ControllerStatus(
            node_id=3,
            mode=5,
            mode_name="Unexpected Name",
            radio=0,
            low_bitrate=False,
            long_preamble=False,
        )
    )

    exit_code = cli.run(
        ["--port", "/dev/ttyACM0", "controller", "status"], client_factory=factory
    )

    assert exit_code == 0
    out = capsys.readouterr().out
    assert "service_controller: yes" in out
    assert "mode: 5" in out


def test_controller_setup_is_explicit_and_calls_controller_setup(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["--port", "/dev/fake", "controller", "setup"], client_factory=factory
    )

    assert exit_code == 0
    assert factory.client.entered_service_shell == 0
    assert factory.client.controller_setup_called == 1
    assert factory.client.wait_calls == []
    out = capsys.readouterr().out.lower()
    assert "service controller mode" in out
    assert "reset" in out or "restart" in out


def test_controller_setup_wait_calls_setup_then_wait_for_shell() -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["--port", "/dev/fake", "controller", "setup", "--wait"],
        client_factory=factory,
    )

    assert exit_code == 0
    assert factory.client.controller_setup_called == 1
    assert factory.client.wait_calls == [30.0]


def test_controller_radio_status_prints_decoded_local_radio(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["--port", "/dev/fake", "controller", "radio", "status"],
        client_factory=factory,
    )

    assert exit_code == 0
    assert factory.client.controller_status_called == 0
    out = capsys.readouterr().out
    assert "radio: 0" in out
    assert "normal bitrate" in out
    assert "normal preamble" in out


def test_controller_radio_set_calls_local_radio_set(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["--port", "/dev/fake", "controller", "radio", "set", "1"],
        client_factory=factory,
    )

    assert exit_code == 0
    assert factory.client.controller_radio_set_calls == [1]
    out = capsys.readouterr().out
    assert "OK controller radio mode 1" in out
    assert "reset/restart required" in out


def test_controller_radio_set_rejects_invalid_mode_without_opening_serial(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(
        ["--port", "/dev/fake", "controller", "radio", "set", "4"],
        client_factory=factory,
    )

    assert exit_code == 1
    assert factory.clients == []
    assert "ERROR" in capsys.readouterr().err


def test_controller_radio_help_documents_values(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(["controller", "radio", "--help"], client_factory=factory)

    assert exit_code == 0
    out = capsys.readouterr().out
    assert "radio mode" in out
    assert "0=normal bitrate + normal preamble" in out
    assert "3=low bitrate + long preamble" in out
    assert "reset/restart" in out


def test_help_smoke_lists_commands(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(["--help"], client_factory=factory)

    assert exit_code == 0
    out = capsys.readouterr().out
    assert "get" in out
    assert "set" in out
    assert "dump" in out
    assert "apply" in out
    assert "controller" in out


def test_set_help_documents_radio_and_power_ranges(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(["set", "--help"], client_factory=factory)

    assert exit_code == 0
    out = capsys.readouterr().out
    assert "radio mode" in out
    assert "0=normal bitrate + normal preamble" in out
    assert "3=low bitrate + long preamble" in out
    assert "0.5..33.5 dB" in out
    assert "0.5 dB steps" in out
    assert "finite coordinates in meters" in out


def test_selector_help_documents_node_id_range(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(["dump", "--help"], client_factory=factory)

    assert exit_code == 0
    out = capsys.readouterr().out
    assert "SELECTOR" in out
    assert "N or A..B" in out
    assert "node IDs 0..255" in out


def test_global_help_documents_serial_option_ranges(capsys: Any) -> None:
    factory = FakeFactory()

    exit_code = cli.run(["--help"], client_factory=factory)

    assert exit_code == 0
    out = capsys.readouterr().out
    assert "positive baud rate" in out
    assert "timeout in seconds" in out
    assert "retries, >=1" in out
