---
title: TDoA2 VS TDoA3
page_id: tdoa2-vs-tdoa3
---


This page describe the basic differences between TDoA2 and TDoA3.

When we designed TDoA2 the main goal was to be able to position multiple
Crazyflies simultaneously. To solve the problem we had to go from Two
way ranging to a [TDoA solution](/docs/functional-areas/tdoa_principles.md). As the name
TDoA2 implies, there was a first iteration that has been scrapped and
TDoA2 was the first official version. After some time we found
limitations in the TDoA2 that we wanted to solve and this resulted in
TDoA3.

The main basic difference between TDoA2 and TDoA3 is that TDoA2 uses
scheduled transmission times while TDoA3 is randomized. In TDoA2 there
are 8 transmission time slots, one for each anchor, when the anchors
transmit in a round-robbin fashion. Anchor 0 is used as a master and the
other anchors synchronizes their transmission schedules to the
transmissions of the master. The strict schedule ensures that no two
anchors transmit at the same time and that we do not get packet
collisions. In TDoA3 transmission times are randomized and the idea is
to accept and handle a limited number of packet collisions.

The implications of these fundamentally different design principles are:

Single point of failure
-----------------------

In TDoA2 anchor 0 is the master that the other anchors synchronize to.
If an anchor does not get packets from anchor 0 it stops transmitting
which means that anchor 0 is a single point of failure; if Anchor 0 dies
the entire system stops. In TDoA3 on the other hand, each anchor
transmits at random times and is not dependant on other anchors for
synchronization.

System size
-----------

Since the anchors use anchor 0 as a master in TDoA2, all anchors must be
in range of anchor 0 which sets a limitation on the maximum size of the
system. It also limits how the anchors can be placed, where anchor 0
typically has to be placed in the center of a large system. In TDoA3
this is not a requirement, and anchors can be placed more freely. The
system can be expanded over larger spaces and also in multiple rooms or
floors.

Nr of anchors
-------------

Since TDoA2 uses a time slotted scheme with 8 slots, this also sets the
maximum number of anchors that can be supported. In essence it can cover
a box shaped space, for instance a room. In TDoA3 there is no such
limitation and more anchors can be added to the system as long as they
use different ids. More anchors can be used to cover larger spaces,
spaces that are not box shaped or that spans multiple rooms.

Precision
---------

TDoa2 avoids packet collisions by using a schedule while TDoA3 relies on
the ability to handle them. When two anchors transmit at the same time
there are a few possible outcomes on the receiver side:

1.  One of the packets is picked by the receiver and the other one is
    discarded
2.  Same as 1. but the precision of the receive time is degraded
3.  Both packets are discarded

The end result is that the precision of TDoA2 is slightly better than
TDoA3.

Logging
-------

In TDoA2 there are log variables set up for TDoA values between anchor
0-1 (in tdoa.d0-1), 1-2 (in tdoa.d1-2), 2-3, 3-4, 4-5, 5-6, 6-7 and 7-0
since these are the most common TDoA pairs due to the round-robbin
schedule. In TDoA3 there is only one variable for TDoA values
(tdoa3.tdoa). The anchors used for the value is set by the parameters
tdoa3.logId and tdoa3,logOthrId. So setting tdoa3.logId=7 and
tdoa3,logOthrId=23 would populate the log variable tdoa3.tdoa with the
TDoA value between anchor 7 and 23.

Client support
--------------

TDoA2 is fully supported in the python client. There is basic support
for TDoA3, but with limitations. For instance, anchor positions are set
by transmitting settings through the Crazyflie via the UWB radio, and it
is possible to design a TDoA3 system that is larger than what can be
reached from one position.
