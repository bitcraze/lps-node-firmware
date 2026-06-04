# LPS Service CLI

Command line tool for LPS service-message settings over the serial service shell.

Run commands from this directory:

```bash
cd tools/lps-service-cli
uv run lps-service --help
```

## Serial port

Pass the serial port for the connected Loco Positioning node or controller with `--port`:

```bash
uv run lps-service --port /dev/ttyACM0 controller status
```

If `--port` is omitted, the CLI tries to auto-select a serial port.

## Controller setup

The service-message shell is available when the connected device is configured as a Service Controller. Check the current controller state with:

```bash
uv run lps-service --port /dev/ttyACM0 controller status
```

Configure Service Controller mode with:

```bash
uv run lps-service --port /dev/ttyACM0 controller setup
```

`controller setup` writes the mode setting. Reset or restart the controller after setup before using service-message commands. You can also ask the CLI to wait for the service shell after the reset:

```bash
uv run lps-service --port /dev/ttyACM0 controller setup --wait
```

Show or set the local Service Controller radio mode independently of remote anchor settings:

```bash
uv run lps-service --port /dev/ttyACM0 controller radio status
uv run lps-service --port /dev/ttyACM0 controller radio set 1
```

`controller radio set` writes the local radio-mode configuration. Reset or restart the controller for the new radio mode to apply.

Radio mode values are:

- `0`: normal bitrate, normal preamble
- `1`: low bitrate, normal preamble
- `2`: normal bitrate, long preamble
- `3`: low bitrate, long preamble

## Get one node

Dump one node as YAML:

```bash
uv run lps-service --port /dev/ttyACM0 get 1
```

## Set node settings

Set one setting on one node or a node range. Selectors can be a single node such as `1` or a range such as `1..12`.

Set radio:

```bash
uv run lps-service --port /dev/ttyACM0 set 1 --radio 2
```

Set default power handling:

```bash
uv run lps-service --port /dev/ttyACM0 set 1 --power default
```

Set explicit transmit power in dB:

```bash
uv run lps-service --port /dev/ttyACM0 set 1 --power-db 12.5
```

Set position:

```bash
uv run lps-service --port /dev/ttyACM0 set 1 --position 1.0 2.0 0.5
```

Set one setting on a range:

```bash
uv run lps-service --port /dev/ttyACM0 set 1..12 --power default
```

## Dump nodes to YAML

Dump to the console:

```bash
uv run lps-service --port /dev/ttyACM0 dump 1..12
```

Write the dump to a file:

```bash
uv run lps-service --port /dev/ttyACM0 dump 1..12 -o anchors.yaml
```

## Apply YAML

Apply settings from a YAML file:

```bash
uv run lps-service --port /dev/ttyACM0 apply anchors.yaml
```

`apply` uses `nodes[].id` to select each target node and reads only `nodes[].settings` for values to write. It ignores `status` and `source` metadata; those fields are included in dumps for diagnostics and inventory only.

Dumps containing raw power fields (`tx_power`, `smart_power`, or `force_tx_power`) cannot currently be applied directly. Convert those settings to `power: default` or `power_db` before running `apply`.

Example YAML document with a settings/status split:

```yaml
service_protocol_version: 1
source:
  port: /dev/ttyACM0
  dumped_at: "2026-06-04T12:00:00Z"
nodes:
  - id: 1
    settings:
      position:
        enabled: true
        x: 1.0
        y: 2.0
        z: 0.5
      radio: 2
      power: default
    status:
      mode: 1
      low_bitrate: false
      long_preamble: false
      firmware_service_protocol_version: 1
  - id: 2
    settings:
      position:
        enabled: true
        x: 2.0
        y: 2.0
        z: 0.5
      radio: 2
      power_db: 12.5
    status:
      mode: 1
      low_bitrate: false
      long_preamble: false
      firmware_service_protocol_version: 1
```
