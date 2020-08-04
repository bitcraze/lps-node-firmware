---
title: Configure LPS positioning mode wirelessly
page_id: configure-mode
---


You can switch the system working mode between TWR, TDoA2 and TDoA3
using a Crazyflie with the loco positioning deck and the Crazyflie
client. This page explains the procedure to do so.

Switching mode
--------------

Using the client, connect a Crazyflie with a LocoDeck installed. When
connected open the Loco Positioning System tab:

![cfclient switch anchor mode](/docs/images/cfclient_switch_anchor_mode.png){:width="600"}

1.  Make sure your Crazyflie has good connection and clear line of sight
    with all the anchors. Ideally place it in the center of the flying
    space. Make sure that the status of all the anchor is a solid green
    in the tab.
2.  Using the radio button (2), select the mode on which the system is
    currently running. TWR or TDoA.
3.  Click the switch buttons (3). The status (1) of all the anchors
    should switch red.
4.  Using the radio button (2), select the new mode, the anchor
    status (1) should switch back to green. Your Crazyflie is now
    getting its position using the new mode!

### How it works

1.  The client sends messages to the Crazyflie via the Crazyradio with
    information of the mode switch for each anchor
2.  The Crazyflie passes the messages on to the anchors via the LPS
    (UWB) radio
3.  The Anchors receive the messages, update their configuration and
    re-boots for the new configuration to be active

The communication from Crazyflie to anchor is somewhat unreliable and
there is no back channel that verifies that the mode switch has been
executed. Please make sure all anchors actually changed mode by checking
that all the anchor status boxes are green. In a physically large
system, the LPS radio may not reach all anchors and this method will not
work at all.

### Trouble shooting

If some of the status boxes do not go back to green after the switching
procedure, it indicates that the anchors did not change mode and you
have a system with mixed modes. A system with mixed modes will not work.

#### I want my system in TDoA mode but some anchors are left in TWR mode

1.  Select TWR mode using the radio button (2).
2.  Click on the switch button (3). The status (1) of all the anchors
    still in TWR mode should turn red. You can click on the button (3)
    multiple if required.
3.  Select TDoA mode using the radio button (2). All anchors should now
    be in TDoA modes.

#### I want my system in TWR mode but some anchors are left in TDoA mode

To change anchors from TDoA to TWR it is very important that the master
anchor, anchor 0, is still in TDoA mode. If it is not the case you can
switch the system back to TDoA before attempting to switch to TWR again.

#### I have a system in mixed modes and I can not get it right

There is always the option to connect each anchor via a USB cable to a
computer and change the mode manually.
