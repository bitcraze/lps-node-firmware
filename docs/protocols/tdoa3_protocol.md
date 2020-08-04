---
title: TDoA Anchor protocol V3
page_id: tdoa3_protocol
---



Design goals
------------

The protocol is designed to be used with a large number of anchors
covering a big area. Each anchor has a limited reach and covers only a
small area, but by adding more anchors the system can cover an
(virtually) unlimited area. At any point in the covered area, 5-10
anchors should be within range of a Tag.

Each anchor knows its own position in a global coordinate system and
broadcasts this information in time stamped packets that are used by one
or more Tags to estimate their position.

There is no synchronization between anchors and they have their own
clocks. There is no master in the system and all anchors are equal.

Packet scheduling
-----------------

Packets are transmitted at random times and collisions are handled by
the receiving party.

Packet format
-------------

{% ditaa --alt "TDoA3 protocol structure" %}
                    +--------+----------------------------------+----------+
Packet    :         | Header | remoteCount x Remote anchor data | LPP data |
                    +--------+----------------------------------+----------+

                                  7 bytes
                     <------------------------------------------->
                      8 bits   8 bits     32 bits       8 bits
                     <------> <------> <-----------> <----------->
                    +--------+--------+-------------+-------------+
Header    :         |  type  |  seq   | txTimeStamp | remoteCount |
                    +--------+--------+-------------+-------------+

                                            6 or 8 bytes
                     <---------------------------------------------------------->
                      8 bits      1 bit      7 bits     32 bits     0 or 16 bits
                     <------> <-----------> <------> <-----------> <------------>
                    +--------+-------------+--------+-------------+--------------+
Remote anchor data: |   id   | hasDistance |  seq   | rxTimeStamp |   distance   |
                    +--------+-------------+--------+-------------+--------------+
{% endditaa %}

-   **Header**
    -   **type:** 0x30
    -   **seq:** the sequence number of this packet 0 - 127
    -   **txTimeStamp:** transmission time of this packet
    -   **remoteCount:** the number of slots in the Remote Anchor Data
        section that are populated with data, max 8
-   **Remote anchor data**
    -   **id:** the id of the remote anchor
    -   **hasDistance:** 1 = distance data is included in the remote
        anchor data, total length is 8 bytes, otherwise it is 6 bytes.
    -   **seq:** sequence number of latest packet received from the
        remote anchor 0 - 127
    -   **rxTimeStamp:** receive time stamp of latest packet from the
        remote anchor
    -   **distance:** time of flight between the current anchor and the
        remote anchor in radio timer ticks
-   **LPP data:** See [LPP short packet description](/docs/protocols/lpp-short-packets-protocol.md)

Tag implementation
------------------

A TDoA tag is simply an UWB sniffer. A sniffer can use the data in the
packets together with the timestamps when packets are received to
calculate its position.

Calculating TDoA
----------------

[Please see TDoA positioning principles](/docs/functional-areas/tdoa_principles.md)
