---
title: TDoA 3 implementation
page_id: tdoa3_implementation
---


This page contains an overview of some of the features in the TDoA 3
implementation. For details, please see the code in the Crazyflie
firmware and the LPS node firmware.

### Clock correction

The different clocks in the system are drifting and it is important to
keep track of the drift of other clocks, we call this clock correction.
The clock drift is caused by temperature changes in the clock crystals
and is a slow process, and this is a property that we explore to detect
errors.

We try to measure the ratio of the frequency of the remote clock VS the
local clock. This is done by looking at two packets from the remote
anchor and dividing the delta of the receive timestamps (local clock)
with the delta of the transmit timestamps (the remote clock). The
assumption is that the distance to the remote anchor has note changed
and that the time of flight of the radio waves therefore is constant.
This is not entirely true for a moving Tag, but if the packet rate is
high enough, the error will be small and can be ignored.

There are many other error sources (packet loss, reflections, delays
through objects and packet collisions for instance) that will effect the
clock correction significantly though but since we know that the clock
drift is changing slowly, we can remove any abrupt changes. There is a
mechanism that discards any values that are \"too far away\" from the
current estimated clock correction. The accepted values are low pass
filtered to smooth the estimated clock correction.

Finally there is also a \"leaky bucket\" (a counter) that is used to
reset the clock correction mechanism if there have been too many
outliers lately. This might indicate that the current clock correction
is wrong and we have to start over.

### Storage

TDoA 3 is designed to handle a dynamic number of anchors. Since we must
store data about anchors in the system, both in the anchors as well as
in the Tags, there is dynamic storage functionality. It has a limited
number of slots for anchor data and the anchor id is used as a key to
access the data. This sets a limit on how many remote anchors that can
be handled by an Anchor or Tag. The assumption is that there should not
be more than 15-20 anchors visible to a device at one time.

The algorithms used to decide if the data of an anchor should be
replaced by a new anchor differs a bit between Anchors and Tags. Anchors
are assumed to have a static position and should only see the same
anchors over time, while a Tag is moving around in space and will be
exposed to new anchors.

### Error management

The first line of defence is the outlier detection in the clock
correction step. It keeps the clock correction tidy but also indicates
whether a packet is suspicious or not. Data in suspicious packets is not
used in the system.

The second line of defence is to look at sequence numbers. If sequence
numbers are not consecutive in received packets, special action has to
be taken to handle it. It is for instance important when calculating
TDoA to make sure we use timestamps from the same packets in all steps.

The third line of defence is to check that the TDoA value we have
calculated is physically possible. The TDoA value can never be greater
than the distance between the two anchors used for the measurement and
value that are larger are discarded.

The Final check is to verify that a TDoA value fits the current
estimated position. If the TDoA value is too far away it is likely that
there has been an error and the value is discarded, if it is deemed to
be good it is fed into the position estimator for further processing. In
this step there is also a mechanism that tries to determine if there are
\"too many\" outliers, which could indicate that the estimated position
is wrong. This makes the outlier detector less picky and accepts larger
errors (letting more TDoA samples through) to enable the position
estimator to converge again.
