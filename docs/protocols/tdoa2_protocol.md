---
title: TDoA Anchor protocol V2
page_id: tdoa2_protocol
---


This page describes the TDoA anchor protocol V2.

System architecture
-------------------

The protocol is designed to be used with 8 anchors. In a TDoA system the
tag will achieve much better positioning inside the anchor convex hull,
so using 8 anchor allows to get a parallelepiped convex hull.

The anchors are addresses from 0 to 7.

Packet scheduling
-----------------

The protocol is using TDMA to transmit the packets. The anchor 0 is the
TDMA master, other anchors are synchronized on it, this means that if
Anchor0 fails or cannot be received by an anchor, the anchor will stop
transmitting and listen for Anchor0.

The time slots length are 2ms and there is 8 time slots per frame. The
frame length is 16ms. Each anchor is allowed to transmit in the the
timeslot corresponding to its address. Anchors transmits early in the
timeslot.

Packets format
--------------

There is only one packet format. This packet is sent at the begining of
each time-slot by the anchor allowed to transmit in the timeslot.

``` {.c}
typedef struct rangePacket_s {
  uint8_t type;
  uint8_t seqs[8];  // Packet sequence number of the timestamps
  uint32_t timestamps[8];  // Relevant time for anchors
  uint16_t distances[8];
} __attribute__((packed)) rangePacket_t;
```

Assuming the current anchors is of ID **anchor\_id**

* **type** is 0x22
* **seqs\[i\]**
   * **If i == anchor_id** Sequence number of the current packet
   * **If i != anchor_id** Sequence number of the latest receive packet from anchor **i**
* **timestamps[i]**
   * **If i == anchor_id** Transmit time of the current packet expressed in current anchor clock
   * **If i != anchor_id** Receive time of the latest receive packet from anchor **i** expressed in current anchor clock
* **distances[i]**
   * **If i == anchor_id** Reserved for future use
   * **If i != anchor_id** Time of flight between current anchor and anchor **i** in radio timer tick expressed in current anchor clock

Tag implementation
------------------

A TDoA tag is simply an UWB sniffer. A sniffer can use the data in the
packets together with the packet receive timestamps to calculate its
position.

In order to save some electric power, a tag implementation can choose to
track the TDMA timeslots and only listen when packets are transmitted.
Though this is not a requirement.

Calculating TDoA
----------------

[Please see TDoA positioning principles](/docs/functional-areas/tdoa_principles.md)
