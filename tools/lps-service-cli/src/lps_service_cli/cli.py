from __future__ import annotations

import argparse
import math
import os
import sys
from collections.abc import Callable, Mapping, Sequence
from datetime import UTC, datetime
from pathlib import Path
from typing import Any

import yaml

from .model import apply_commands_for_node, dump_document, node_record_from_get_values
from .ranges import NodeSelectorError, parse_node_selector
from .serial_link import (
    PortSelectionError,
    SerialLinkError,
    ServiceSerialClient,
    select_port,
)
from .yaml_io import dumps_yaml, read_yaml_file, write_yaml_file


ClientFactory = Callable[..., Any]


class CliArgumentParser(argparse.ArgumentParser):
    def __init__(self, *args: Any, **kwargs: Any) -> None:
        kwargs.setdefault("formatter_class", argparse.RawDescriptionHelpFormatter)
        super().__init__(*args, **kwargs)

    def error(self, message: str) -> None:
        self.print_usage(sys.stderr)
        self.exit(1, f"{self.prog}: ERROR: {message}\n")


def _default_client_factory(port: str, *, baud: int, timeout: float) -> ServiceSerialClient:
    return ServiceSerialClient(port, baud=baud, timeout=timeout)


def _dumped_at() -> str:
    return datetime.now(UTC).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def _validate_radio(value: int) -> int:
    if value < 0 or value > 3:
        raise ValueError("radio must be in range 0..3")
    return value


def _validate_channel(value: int) -> int:
    if value not in {1, 2, 3, 4, 5, 7}:
        raise ValueError("channel must be one of 1,2,3,4,5,7")
    return value


def _validate_power_db(value: float) -> float:
    if value < 0.5 or value > 33.5:
        raise ValueError("power-db must be in range 0.5..33.5")
    if not math.isclose(value * 2.0, round(value * 2.0), rel_tol=0.0, abs_tol=1e-9):
        raise ValueError("power-db must use 0.5 dB increments")
    return value


def _validate_position(values: Sequence[float]) -> tuple[float, float, float]:
    if len(values) != 3:
        raise ValueError("position requires X Y Z")
    x, y, z = values
    if not all(math.isfinite(value) for value in (x, y, z)):
        raise ValueError("position coordinates must be finite")
    return x, y, z


def _client_port(client: Any, fallback: str) -> str:
    return str(getattr(client, "port", fallback))


def _print_ok(command: str) -> None:
    print(f"OK {command}")


def _print_error(message: object) -> None:
    print(f"ERROR: {message}", file=sys.stderr)


def _print_node_error(node_id: object, message: object) -> None:
    print(f"ERROR node={node_id}: {message}", file=sys.stderr)


def _serial_call(args: argparse.Namespace, action: Callable[[], Any]) -> Any:
    attempts = max(1, args.retries)
    for attempt in range(attempts):
        try:
            return action()
        except SerialLinkError:
            if attempt == attempts - 1:
                raise
    raise AssertionError("unreachable retry loop state")


def _set_command_for_node(args: argparse.Namespace, node_id: int) -> str:
    if args.position is not None:
        x, y, z = _validate_position(args.position)
        return f"set pos {node_id} {x} {y} {z}"
    if args.radio is not None:
        radio = _validate_radio(args.radio)
        return f"set radio {node_id} {radio}"
    if args.channel is not None:
        channel = _validate_channel(args.channel)
        return f"set channel {node_id} {channel}"
    if args.power is not None:
        return f"set power {node_id} default"
    if args.power_db is not None:
        power_db = _validate_power_db(args.power_db)
        return f"set power {node_id} {power_db}"
    raise ValueError("exactly one setting must be specified")


def _read_nodes(document: Mapping[str, object]) -> list[Mapping[str, object]]:
    nodes = document["nodes"]
    if not isinstance(nodes, list):
        raise ValueError("YAML document nodes must be a list")

    records: list[Mapping[str, object]] = []
    for node in nodes:
        if not isinstance(node, Mapping):
            raise ValueError("YAML node records must be mappings")
        records.append(node)
    return records


def _cmd_get(args: argparse.Namespace, client: Any, port: str) -> int:
    node_id = args.node_id_value
    _serial_call(args, client.enter_service_shell)
    reply = _serial_call(args, lambda: client.command(f"get {node_id}"))
    record = node_record_from_get_values(node_id, reply.values)
    document = dump_document([record], port=_client_port(client, port), dumped_at=_dumped_at())
    print(dumps_yaml(document), end="")
    return 0


def _cmd_set(args: argparse.Namespace, client: Any, _port: str) -> int:
    _serial_call(args, client.enter_service_shell)

    failed = args.local_validation_failed
    for node_id, command in args.set_commands:
        try:
            _serial_call(args, lambda command=command: client.command(command))
        except SerialLinkError as exc:
            if not args.continue_on_error:
                raise
            failed = True
            _print_node_error(node_id, exc)
            continue
        _print_ok(command)

    return 1 if failed else 0


def _cmd_dump(args: argparse.Namespace, client: Any, port: str) -> int:
    _serial_call(args, client.enter_service_shell)

    records: list[Mapping[str, object]] = []
    failed = False
    for node_id in args.node_ids:
        try:
            reply = _serial_call(args, lambda node_id=node_id: client.command(f"get {node_id}"))
            records.append(node_record_from_get_values(node_id, reply.values))
        except (SerialLinkError, ValueError) as exc:
            failed = True
            _print_node_error(node_id, exc)

    document = dump_document(records, port=_client_port(client, port), dumped_at=_dumped_at())
    if args.output is None:
        print(dumps_yaml(document), end="")
    else:
        write_yaml_file(args.output, document)

    return 1 if failed else 0


def _cmd_apply(args: argparse.Namespace, client: Any, _port: str) -> int:
    _serial_call(args, client.enter_service_shell)

    failed = args.local_validation_failed
    for node_id, commands in args.apply_plans:
        for command in commands:
            try:
                _serial_call(args, lambda command=command: client.command(command))
            except SerialLinkError as exc:
                if not args.continue_on_error:
                    raise
                failed = True
                _print_node_error(node_id, exc)
                break
            _print_ok(command)

    return 1 if failed else 0


def _cmd_controller_status(_args: argparse.Namespace, client: Any, _port: str) -> int:
    status = _serial_call(_args, client.controller_status)
    service_controller = status.mode == 5
    print(f"service_controller: {'yes' if service_controller else 'no'}")
    print(f"mode_name: {status.mode_name}")
    print(f"mode: {status.mode}")
    print(f"node_id: {status.node_id}")
    print(f"radio: {status.radio}")
    return 0


def _radio_mode_text(radio_mode: int) -> str:
    bitrate = "low bitrate" if (radio_mode & 1) else "normal bitrate"
    preamble = "long preamble" if (radio_mode & 2) else "normal preamble"
    return f"{bitrate}, {preamble}"


def _cmd_controller_setup(args: argparse.Namespace, client: Any, _port: str) -> int:
    _serial_call(args, client.controller_setup)
    print("OK controller setup: Service Controller mode written; reset/restart required")
    if args.wait:
        _serial_call(args, lambda: client.wait_for_service_shell(total_timeout=30.0))
        _print_ok("service shell ready")
    return 0


def _cmd_controller_radio_status(args: argparse.Namespace, client: Any, _port: str) -> int:
    radio_mode = _serial_call(args, client.controller_radio_status)
    print(f"radio: {radio_mode}")
    print(f"radio_mode: {_radio_mode_text(radio_mode)}")
    print("applies_after_reset: yes")
    return 0


def _cmd_controller_radio_set(args: argparse.Namespace, client: Any, _port: str) -> int:
    radio_mode = _validate_radio(args.radio_mode)
    _serial_call(args, lambda: client.controller_radio_set(radio_mode))
    print(
        f"OK controller radio mode {radio_mode}: {_radio_mode_text(radio_mode)}; "
        "reset/restart required"
    )
    return 0


def _add_global_options(parser: argparse.ArgumentParser, *, defaults: bool) -> None:
    default = {} if defaults else {"default": argparse.SUPPRESS}
    parser.add_argument("--port", help="serial port to use, for example /dev/ttyACM0", **default)
    parser.add_argument(
        "--baud",
        type=int,
        help="positive baud rate (default: 115200)",
        **({"default": 115200} if defaults else default),
    )
    parser.add_argument(
        "--timeout",
        type=float,
        help="positive timeout in seconds for serial replies (default: 1.0)",
        **({"default": 1.0} if defaults else default),
    )
    parser.add_argument(
        "--retries",
        type=int,
        help="serial command retries, >=1 (default: 3)",
        **({"default": 3} if defaults else default),
    )
    parser.add_argument(
        "--verbose", action="store_true", help="show selected port and diagnostics", **default
    )


def build_parser() -> argparse.ArgumentParser:
    parser = CliArgumentParser(
        prog="lps-service",
        description="Configure LPS service-message settings over the serial service shell.",
    )
    _add_global_options(parser, defaults=True)

    subparsers = parser.add_subparsers(
        dest="command", required=True, parser_class=CliArgumentParser
    )

    get_parser = subparsers.add_parser(
        "get",
        help="dump one node as YAML",
        description="Dump one node as YAML.",
        epilog="NODE_ID: node ID 0..255.",
    )
    _add_global_options(get_parser, defaults=False)
    get_parser.add_argument("node_id", metavar="NODE_ID", help="node ID 0..255")
    get_parser.set_defaults(handler=_cmd_get)

    set_parser = subparsers.add_parser(
        "set",
        help="set one setting on selected nodes",
        description="Set one writable setting on selected nodes.",
        epilog="""
SELECTOR: N or A..B, node IDs 0..255.

Radio mode values:
  0=normal bitrate + normal preamble
  1=low bitrate + normal preamble
  2=normal bitrate + long preamble
  3=low bitrate + long preamble

Channel values:
  --channel N           UWB channel, one of 1,2,3,4,5,7

Power values:
  --power default       enable default/smart power
  --power-db DB         force TX power, 0.5..33.5 dB in 0.5 dB steps

Position values:
  --position X Y Z      finite coordinates in meters
""",
    )
    _add_global_options(set_parser, defaults=False)
    set_parser.add_argument("selector", metavar="SELECTOR", help="N or A..B, node IDs 0..255")
    setting_group = set_parser.add_mutually_exclusive_group(required=True)
    setting_group.add_argument(
        "--position",
        nargs=3,
        type=float,
        metavar=("X", "Y", "Z"),
        help="finite coordinates in meters",
    )
    setting_group.add_argument("--radio", type=int, metavar="N", help="radio mode 0..3")
    setting_group.add_argument(
        "--channel", type=int, metavar="N", help="UWB channel, one of 1,2,3,4,5,7"
    )
    setting_group.add_argument("--power", choices=["default"], help="default/smart power")
    setting_group.add_argument(
        "--power-db",
        type=float,
        dest="power_db",
        metavar="DB",
        help="force TX power, 0.5..33.5 dB in 0.5 dB steps",
    )
    set_parser.add_argument("--continue-on-error", action="store_true")
    set_parser.set_defaults(handler=_cmd_set)

    dump_parser = subparsers.add_parser(
        "dump",
        help="dump selected nodes as YAML",
        description="Dump selected nodes as YAML.",
        epilog="SELECTOR: N or A..B, node IDs 0..255.",
    )
    _add_global_options(dump_parser, defaults=False)
    dump_parser.add_argument("selector", metavar="SELECTOR", help="N or A..B, node IDs 0..255")
    dump_parser.add_argument("-o", "--output", metavar="FILE", type=Path, help="write YAML to FILE")
    dump_parser.set_defaults(handler=_cmd_dump)

    apply_parser = subparsers.add_parser(
        "apply",
        help="apply node settings from YAML",
        description="Apply node settings from YAML.",
        epilog="FILE: YAML dump/apply file. Uses nodes[].id and nodes[].settings only.",
    )
    _add_global_options(apply_parser, defaults=False)
    apply_parser.add_argument("file", metavar="FILE", type=Path, help="YAML file to apply")
    apply_parser.add_argument("--continue-on-error", action="store_true")
    apply_parser.set_defaults(handler=_cmd_apply)

    controller_parser = subparsers.add_parser("controller", help="controller mode commands")
    _add_global_options(controller_parser, defaults=False)
    controller_subparsers = controller_parser.add_subparsers(
        dest="controller_command", required=True, parser_class=CliArgumentParser
    )

    status_parser = controller_subparsers.add_parser("status", help="show controller status")
    _add_global_options(status_parser, defaults=False)
    status_parser.set_defaults(handler=_cmd_controller_status)

    setup_parser = controller_subparsers.add_parser("setup", help="set Service Controller mode")
    _add_global_options(setup_parser, defaults=False)
    setup_parser.add_argument("--wait", action="store_true", help="wait for service shell after reset")
    setup_parser.set_defaults(handler=_cmd_controller_setup)

    radio_parser = controller_subparsers.add_parser(
        "radio",
        help="local controller radio mode commands",
        description="Show or set the local service-controller radio mode.",
        epilog="""
Radio mode values:
  0=normal bitrate + normal preamble
  1=low bitrate + normal preamble
  2=normal bitrate + long preamble
  3=low bitrate + long preamble

Changes are written to local config and require reset/restart to apply.
""",
    )
    _add_global_options(radio_parser, defaults=False)
    radio_subparsers = radio_parser.add_subparsers(
        dest="controller_radio_command", required=True, parser_class=CliArgumentParser
    )

    radio_status_parser = radio_subparsers.add_parser("status", help="show local radio mode")
    _add_global_options(radio_status_parser, defaults=False)
    radio_status_parser.set_defaults(handler=_cmd_controller_radio_status)

    radio_set_parser = radio_subparsers.add_parser("set", help="set local radio mode")
    _add_global_options(radio_set_parser, defaults=False)
    radio_set_parser.add_argument("radio_mode", metavar="N", type=int, help="radio mode 0..3")
    radio_set_parser.set_defaults(handler=_cmd_controller_radio_set)

    return parser


def _preflight_get(args: argparse.Namespace) -> int | None:
    node_ids = parse_node_selector(args.node_id)
    if len(node_ids) != 1:
        raise ValueError("get requires exactly one node id")
    args.node_id_value = node_ids[0]
    return None


def _preflight_set(args: argparse.Namespace) -> int | None:
    failed = False
    commands: list[tuple[int, str]] = []
    for node_id in parse_node_selector(args.selector):
        try:
            commands.append((node_id, _set_command_for_node(args, node_id)))
        except ValueError as exc:
            if not args.continue_on_error:
                raise
            failed = True
            _print_node_error(node_id, exc)

    args.local_validation_failed = failed
    args.set_commands = commands
    if not commands:
        return 1 if failed else 0
    return None


def _validate_output_path(path: Path) -> None:
    parent = path.parent
    if not parent.exists():
        raise ValueError(f"output parent directory does not exist: {parent}")
    if not parent.is_dir():
        raise ValueError(f"output parent path is not a directory: {parent}")
    if path.exists() and path.is_dir():
        raise ValueError(f"output path is a directory: {path}")

    fd = os.open(path, os.O_WRONLY | os.O_CREAT, 0o666)
    os.close(fd)


def _preflight_dump(args: argparse.Namespace) -> int | None:
    args.node_ids = parse_node_selector(args.selector)
    if args.output is not None:
        _validate_output_path(args.output)
    return None


def _preflight_apply(args: argparse.Namespace) -> int | None:
    document = read_yaml_file(args.file)
    records = _read_nodes(document)

    failed = False
    plans: list[tuple[object, list[str]]] = []
    for record in records:
        node_id = record.get("id", "?")
        try:
            commands = apply_commands_for_node(record)
        except (ValueError, KeyError) as exc:
            if not args.continue_on_error:
                raise
            failed = True
            _print_node_error(node_id, exc)
            continue
        if commands:
            plans.append((node_id, commands))

    args.local_validation_failed = failed
    args.apply_plans = plans
    if not plans:
        return 1 if failed else 0
    return None


def _preflight_args(args: argparse.Namespace) -> int | None:
    if args.baud < 1:
        raise ValueError("baud must be a positive integer")
    if not math.isfinite(args.timeout) or args.timeout <= 0.0:
        raise ValueError("timeout must be a positive finite number")
    if args.retries < 1:
        raise ValueError("retries must be at least 1")

    if args.command == "get":
        return _preflight_get(args)
    if args.command == "set":
        return _preflight_set(args)
    if args.command == "dump":
        return _preflight_dump(args)
    if args.command == "apply":
        return _preflight_apply(args)
    if (
        args.command == "controller"
        and getattr(args, "controller_command", None) == "radio"
        and getattr(args, "controller_radio_command", None) == "set"
    ):
        _validate_radio(args.radio_mode)
    return None


def run(argv: Sequence[str] | None = None, *, client_factory: ClientFactory = _default_client_factory) -> int:
    parser = build_parser()
    try:
        args = parser.parse_args(argv)
    except SystemExit as exc:
        return int(exc.code) if isinstance(exc.code, int) else 1

    try:
        preflight_exit = _preflight_args(args)
        if preflight_exit is not None:
            return preflight_exit

        port = select_port(args.port)
        if args.verbose:
            print(f"DEBUG port={port}", file=sys.stderr)
        with client_factory(port, baud=args.baud, timeout=args.timeout) as client:
            return int(args.handler(args, client, port))
    except (
        PortSelectionError,
        SerialLinkError,
        NodeSelectorError,
        ValueError,
        KeyError,
        OSError,
        yaml.YAMLError,
    ) as exc:
        _print_error(exc)
        return 1


def main(argv: Sequence[str] | None = None) -> int:
    return run(argv)


if __name__ == "__main__":
    raise SystemExit(main())
