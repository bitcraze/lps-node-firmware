---
title: TDoA 3 Long Range
page_id: tdoa3_long_range
---
In the TDoA3 mode the system can be configured in Long Range mode to extend the distance that the Crazyflie can
receive data from the anchors. We have not done any systematic measurements for the maximum distance but we believe
it is at least around 30 - 40 meters.

## How it works

The bitrate of the UWB messages is reduced which increases the distance at which messages can be received. Since each
packet takes a longer time to transmit, the packet rate is also reduced. This increases the noise in the estimated
position a bit compared to "normal" TDoA3.

## How to configure

Both the anchors and the Crazyflie must be configured in the Long Range mode. You can read about
[how to configure the Crazyflie here](https://www.bitcraze.io/documentation/repository/crazyflie-firmware/master/userguides/configuration/tdoa3_long_range_config).

The anchors must be configured with the radio mode: `low bitrate, normal preamble`, [see the low level configuration section](/docs/development/anchor-low-level-config)

The key sequence to use in the configuration is:
1. `r` - the UWB radio settings menu
2. `1` - low bitrate, normal preamble

You might also want to [increase the TX power](../configure-tx-power).

## Limitations

TDoA3 Long Range is supported from version 2022.09.

Long Range only works with TDoA3

If the system is very "flat", the estimate in Z will not be good, the reason is simple geometry.
For good 3D position estimation we recommend that the ratio between the length|width in relation to the height should
not be larger than 3. This is not an absolute requirement but we think it is a good guideline.
When using Long Range mode this can become a problem if the size of the system is increased without moving the
upper anchors higher. One solution is to add more anchors to the system in the ceiling and/or on the ground to create
smaller "cells" with a good ratio between width and height.
