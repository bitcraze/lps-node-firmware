---
title: ROS offboard positioning with LPS
page_id: ros
---

### ROS

The ROS module for positioning with the LPS is available on Github:
<https://github.com/bitcraze/lps-ros>. It is developed and tested on ROS
Kinetic running on Ubuntu 16.04. As of now ROS is the only software
environment in which the system has been tested.

To get started from a newly installed Ubuntu 16.04:

1.  Install ROS (full desktop):
    <http://wiki.ros.org/kinetic/Installation/Ubuntu>
2.  Install the ros joystick node with
    `sudo apt-get install ros-kinetic-joy`
3.  Create your workspace:
    <http://wiki.ros.org/ROS/Tutorials/InstallingandConfiguringROSEnvironment#Create_a_ROS_Workspace>
4.  Clone <https://github.com/bitcraze/lps-ros> and
    <https://github.com/whoenig/crazyflie_ros> in the workspace src
    folder
5.  Run *catkin\_make* to build the packages
6.  Source the workspace *devel/setup.bash* again
7.  Run *rosdep install bitcraze\_lps\_estimator* to install
    dependencies

The positions of the anchors has to be entered in a configuration file.
Create a file at *catkin\_ws/src/lps-ros/data/anchor\_pos.yaml* and
enter your anchor coordinates. An example is present in the data folder:

``` {.yaml}
n_anchors: 6
anchor0_pos: [  0,   0, 1.85]
anchor1_pos: [  0,   2, 1.85]
anchor2_pos: [  4,   2, 1.85]
anchor3_pos: [  4,   0, 1.85]
anchor4_pos: [1.3,   1, 1.85]
anchor5_pos: [2.6,   1, 1.85]
```

The positions are in meters, \[x, y, z\], from the origin.

When this file is configured, connect an XBox 360 (or compatible)
gamepad, start the Crazyflie 2.0 and launch:

``` {.bash}
$ roslaunch bitcraze_lps_estimator dwm_loc_ekf_hover.launch uri:=radio://0/80/250K x:=3 y:=2.3 z:=1.0
```

Change the URI with your copter radio configuration. You will see the
anchor position in RED so that you can verify there position make sense.
The Crazyflie position will be shown in green, before flying you can
verify that the position is roughly correct.

The X,Y,Z parameter will be the hovering point, make sure they make
sense in your setup and it is better to place the Crazyflie just under
this first setpoint to start with. The gamepad mapping is (this is for
an X-Box gamepad):


  |**Button**      | **Action**|
  |----------------| -------------------------------------------|
 | X               | Take-off|
 | A               | Land|
|  B               | Emergency stop (this is the red button ;)|
|  Left joystick   | X/Y position|
|  Right joystick  | Altitude/Yaw setpoint|
|  ----------------| -------------------------------------------|

Note that the YAW setpoint is not handled yet.
