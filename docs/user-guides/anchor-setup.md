---
title: Preparing the LPS nodes
page_id: anchor-setup
---


Use the [LPS configuration tool](/docs/user-guides/tools.md)
to update the firmware and set the addresses of the nodes from 0-5. Note
the address on the nodes.

The nodes should be placed at least 15 cm away from walls or ceiling. In
our reference set-up we accomplish this by using
[these](https://github.com/bitcraze/bitcraze-mechanics/blob/master/LPS-anchor-stand/anchor-stand.stl)
3D printed stands.

![loco reference distance](/docs/images/locorefdist.png){:width="400"}

Power the anchors with 5V using either the micro-USB connector, the
terminal or the DJ-jack.

![dwm setup](/docs/images/dwm-setup.jpg)

Note; If you\'re not following our reference set-up for placing the
nodes try to space them out at lest 2 meters apart and at different
heights (see [Dilution Of
Precision](https://en.wikipedia.org/wiki/Dilution_of_precision_(GPS))).

The position in X/Y/Z of the anchor UWB antenna needs to be measured. To
do so chose an origin (in a square/rectangular room one of the corners
is a good choice), choose your axis to be a [right-handed
coordinates](https://en.wikipedia.org/wiki/Right-hand_rule). Finally you
can measure the X/Y/Z position of the anchors in this coordinate system.
Using a corner as origin means that you can use a laser-ranger against
the walls to get a direct reading of the coordinates.

Use the LPS tab of the Crazyflie client to store the physical anchor
positions in the anchors. The communication from the client to the
anchors is bridged through a Crazyflie, so you must have a Crazyflie
with an Loco Positioning deck connected to the client.
