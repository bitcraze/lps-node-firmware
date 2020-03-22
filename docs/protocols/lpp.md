---
title: Loco Positioning Protocol
page_id: lpp
---

The Loco Positioning Protocol (LPP) defines the messages send on the
Loco Positioning System UWB radio to configure and use the system. There

is two main classes of messages:
* messages used for ranging and positioning
* messages used for system management

The ranging messages are sent and received directly by one of the ranging
algorithms in the Crazyflie and/or the Nodes. The management message are sent
from the Crazyflie to a Node, but are usually initiated by a client or manangement
tool to perform system configuration of the Nodes.

## Ranging protocols

All ranging packet share the same format:

| byte | value   | note                     |
| ---- | ------  | ------------------------ |
| 0    | ID      | Packet ID                |
| 1..  | Payload | Format is defined per ID |
