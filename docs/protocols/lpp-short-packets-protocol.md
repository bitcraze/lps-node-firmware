---
title: LPP Short packets protocol
page_id: lpp-short-packets-protocol
---

The LPP short packets are management packets, short enough to be sent
using a single Crazyflie CRTP packet. This means that the payload part
of an LPP packet should be smaller than 28 Bytes. This also makes these
packets well suited to be transferred as extra data appended to a
ranging packet (this is for example used for the anchor position
packet).

LPP Short packet format:

| byte  | value    | note                     |
| ----- | -------- | -------------------------|
| 0     | 0xF0     | LPP short                |
| 1     | LPPS\_ID | LPP Short Packet ID      |
| 2..17 | Payload  | Format is defined per ID |

LPP Short packet IDs:

| LPPS\_ID | Description                              |
| -------- | ---------------------------------------- |
| 0x01     | [Anchor position](#0x01-anchor-position) |

## 0x01: Anchor position

Format:

``` {.c}
struct lppShortAnchorPosition_t {
  float x;
  float y;
  float z;
} __attribute__((packed));
```

When sent using *unicast* to an anchor, the anchor will save the (x,y,z)
position as its new position.

When appended to a ranging message, the tag uses the (x,y,z) position as
the anchor position.
