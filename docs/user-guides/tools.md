---
title: LPS configuration tool
page_id: tools
---



 The [LPS configuration
tool](https://github.com/bitcraze/lps-tools) is used for configuring the
[Loco Node](https://store.bitcraze.io/collections/positioning/products/loco-positioning-node) via USB. It\'s used for:

-   Firmware update
-   Positioning mode
-   Address configuration

To use the LPS tool start the program and follow the step by step
instructions.


![LPStools start dialog](/docs/images/lpstools_start_dialog.png){:.align-right width="400"}

## Service-message CLI

The `lps-service` CLI in `tools/lps-service-cli/` manages node settings through the service-message serial shell.

It can get and set one node, dump a range of nodes to YAML, apply settings from YAML, set one setting on a range of nodes, and set the local Service Controller radio mode. Node settings include position, radio bitrate/preamble mode, UWB channel, and TX power.

When applying YAML, raw power dump fields (`tx_power`, `smart_power`, or `force_tx_power`) must first be converted to `power: default` or `power_db`; see the CLI README for details.

```bash
cd tools/lps-service-cli
uv run lps-service --port /dev/ttyACM0 controller status
uv run lps-service --port /dev/ttyACM0 controller radio set 1  # reset/restart controller after this
uv run lps-service --port /dev/ttyACM0 dump 1..12 -o anchors.yaml
uv run lps-service --port /dev/ttyACM0 set 1..12 --power default
uv run lps-service --port /dev/ttyACM0 set 1..12 --channel 5
uv run lps-service --port /dev/ttyACM0 apply anchors.yaml
```
