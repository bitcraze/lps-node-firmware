---
title: TDoA 3 set up
page_id: tdoa3_setup
---


TDoA 3 is part of the official Crazyflie and LPS Node firmware releases
since 2018.10.

The Crazyflie will automatically detect that the positioning system is
using TDoA3 and switch to the correct mode. If you want to force the
Crazyflie to use TDoA3 on startup, use the LPS\_TDOA3\_ENABLE=1 compile
option

### The LPS Nodes

-   Configure the Nodes as anchors, and set the mode to TDoA 3
-   Set anchor positions, see below.

Setting anchor positions
------------------------

Anchor positions can be set by the python client or the tool in the
lps-node-firmware repo. You might have to move the Crazyflie close to
all anchors to transfer the positions from the Crazyflie.

All the anchors work in the same coordinate system, so you need to
choose a common origin for your system. Try to expand your system step
by step, testing it at each step to catch problem when they start to
happen.

2D positioning
--------------

TDoA 3 supports 2D positioning as well. In this mode all anchors can be
placed in the same plane. The tag (LPS deck) is expected to move in the
XY-plane at a fixed Z.

Recompile the Crazyflie firmware using the LPS\_2D\_POSITION\_HEIGHT
compile flag set to the desired Z. For instance
`make "EXTRA_CFLAGS=-DLPS_2D_POSITION_HEIGHT=1.2"` if the tag will be
located at 1.2 meters height.

Longer range and lower bitrate
------------------------------

Note: this is experimental functionality that is not fully tested or
might not work as expected.

In this mode the UWB bitrate is lower and the data of messages can be
read at a longer distance. The packet rate will be reduced though.

Recompile the Crazyflie firmware with the LPS\_LONGER\_RANGE compile
flag.

Example: `make "EXTRA_CFLAGS=-DLPS_LONGER_RANGE"`

There is a setting for the bitrate in the Anchor configuration when
connecting using USB.

Longer preamble
---------------

Note: this is experimental functionality that is not fully tested or
might not work as expected.

It is possible to use a longer preamble, this will also reduce packet
rate. There is no compile flag for this setting, it requires some code
modifications in the Crazyflie and the anchors. The preamble is used by
the anchor radios to detect messages and a longer preamble increase the
probability and thus the useful distance. Note that the radios might
detect the preamble but fail at reading the data of the packet (see
lower bitrate above). The \"SFD\" LED on the anchor indicates if a
preamble has been detected by the radio.

### The Crazyflie

-   Find the src/deck/drivers/src/locodeck.c file
-   Locate the call to dwEnableMode()
-   Change the second argument to MODE\_LONGDATA\_MID\_ACCURACY
-   Compile and flash

### The LPS Nodes

-   There is a configuration setting for the preamble length

Sniffing
--------

Sniffing the system is useful when you want to start digging into the
radio messages exchanged between the anchor. It is a debug
functionality.

If an anchor is configured to act as a sniffer, it can be used to listen
to the UWB messages in the system. To set an anchor to sniffer mode,
connect via USB and configure it to sniffer mode.

### Basic sniffing

Connect the sniffer via USB and check in /dev which port that it uses.
Use this as the first argument The second argument is the output format,
use yaml example:
`python3 tools/sniffer/sniffer_binary.py /dev/tty.usbmodem1421 yaml`

### TDoA 3 parsing

Pipe the yaml into the TDoA 3 decoder to see TDoA 3 data unpacked

example:
`python3 tools/sniffer/sniffer_binary.py /dev/tty.usbmodem1421 yaml | python3 tools/sniffer/tdoa3_decoder.py`

This will show all data that the sniffer receives. To limit the output
it is possible to filter which anchors to show. Add one or more ids at
the end to only show data from only these anchors.

example:
`python3 tools/sniffer/sniffer_binary.py /dev/tty.usbmodem1421 yaml | python3 tools/sniffer/tdoa3_decoder.py 2 3`

### Time of flight and anchor to anchor distance

By piping the TDoA 3 data into the TOF tool, the measured anchor to
anchor distances will be displayed. By adding the \'m\' argument the
output will be in meters instead of UWB clock ticks.

example:
`python3 tools/sniffer/sniffer_binary.py /dev/tty.usbmodem1421 yaml | python3 tools/sniffer/tdoa3_decoder.py | python3 tools/sniffer/tdoa3_tof.py m`
