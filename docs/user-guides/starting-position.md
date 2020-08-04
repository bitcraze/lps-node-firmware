---
title: Crazyflie initial positioning
page_id: starting-position
---


When starting the Crazyflie in the LPS it should be positioned according
to the image below. For optimal performance, always start the Crazyflie
facing the positive X-axis.

The orientation of the Crazyflie can not be measured directly using the
LPS system, the only information available regarding yaw is to detect
changes in position after a motion. The correct yaw should be found
eventually also when starting in a different orientation, but it will
take a long time and the Crazyflie will behave in an odd way during the
process.

![crazyflie lps starting position](/docs/images/crazyflie_isometric_drawing_2.png){:.align-center
width="600"}
