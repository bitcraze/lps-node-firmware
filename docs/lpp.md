---
title: Loco Positioning Protocol
page_id: lpp
---

The Loco Positioning Protocol (LPP) defines the messages send on the
Loco Positioning System UWB radio to configure and use the system. There
is two main class of messages: the messages used for ranging and
positioning and the messages used for management.

The ranging messages are sent and received directly by the ranging
algorithms in the Crazyflie and in the Nodes. The management message are
sent by management tools like the Crazyflie client.

Ranging protocols
-----------------

All ranging packet share the same format:

 | byte  |  value     | note| 
|   ------|  --------- | --------------------------| 
|   0     |  ID       |  Packet ID| 
|   1..   |  Payload  |  Format is defined per ID| 

### Two Way Ranging

The two way ranging protocol allows a tag to measure its distance to an
anchor. The protocol contains 4 messages, two are sent by the tag two by
the anchor. The message sequence is as follow: ![](/images/toa.png)

The time of flight is calculated using the formula:
![](/images/formula_toa2.png){:width="400"}

TWR packet IDs

 |  ID    |  Description| 
 |  ------|  ----------------------------------------| 
|   0x01  |  [TWR\_POLL](/doc/lps/lpp#TWR_POLL)| 
|   0x02  |  [TWR\_ANSWER](/doc/lps/lpp#TWR_ANSWER)| 
|   0x03  |  [TWR\_FINAL](/doc/lps/lpp#TWR_FINAL)| 
|   0x04   | [TWR\_REPORT](/doc/lps/lpp#TWR_REPORT)| 

#### TWR\_POLL

Format:

|   Payload byte  |  Name  |  Note| 
|  ---------------| -------| ----------| 
|   0             |  SEQ   |  1 Byte sequence number, is repeated in subsequent packet of the same TWR transaction| 

#### TWR\_ANSWER

Format:

|   Payload byte |   Name                   |   Note| 
 |  -------------- | ------------------------|  -----------------------------------------| 
|   0             |  SEQ                      1 Byte sequence number, matches the POLL sequence number| 
 |  1..          |   LPP management message   *Optional* LPP management data| 

The optional management data can be any LPP management packet deemed
useful for the tag. Currently this is used to send the anchor position
from the anchor to the tag using the [LPP Short anchor position
packet](/doc/lps/lpp#x01anchor_position)

#### TWR\_FINAL

Format:

|   Payload byte  |  Name  |  Note| 
 |  --------------|  ------|  ----------------------------------------------------------| 
|   0             |  SEQ   |  1 Byte sequence number, matches the POLL sequence number| 

#### TWR\_REPORT

Format:

|   Payload byte  |  Name    |  Note| 
|   -------------- | --------|  ----------------------------------------------------------| 
|   0             |  SEQ    |   1 Byte sequence number, matches the POLL sequence number| 
|   1..           |  Report |   Report structure, see format bellow| 

Report structure format:

``` {.c}
typedef struct {
  uint8_t pollRx[5];      // R1 timestamp
  uint8_t answerTx[5];    // T2 timestamp
  uint8_t finalRx[5];     // R3 timestamp

  float pressure;         // Pressure measurement at the anchor
  float temperature;      // Temperature of the pressure sensor
  float asl;              // Above Sea Level altitude
  uint8_t pressure_ok;    // Not 0 if the pressure information is correct
} __attribute__((packed)) lpsTwrTagReportPayload_t;
```

Management protocols
--------------------

### LPP Short packets

The LPP short packets are management packet short enough to be sent
using a single Crazyflie CRTP packet. This means that the payload part
of an LPP packet should be smaller than 28 Bytes. This makes these
packet also well suited to be transferred as extra data appended to a
ranging packet (this is for example used for the anchor position
packet).

LPP Short packet format:

|   byte   |  value   |    note| 
|   ------- | ----------|  --------------------------| 
|   0      |  0xF0      |  LPP short| 
|   1      |  LPPS\_ID  |  LPP Short Packet ID| 
|   2..17   | Payload   |  Format is defined per ID| 

LPP Short packet IDs:

|   LPPS\_ID  |  Description| 
|   ---------- | ----------------------------------------------------| 
|   0x01      |  [Anchor position](/doc/lps/lpp#x01anchor_position)| 

#### 0x01: Anchor position

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
