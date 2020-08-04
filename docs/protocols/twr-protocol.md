---
title: Two Way Ranging Protocol
page_id: twr-protocol
---

The two way ranging protocol allows a tag to measure its distance to an
anchor. The protocol contains 4 messages, two are sent by the tag and two by
the anchor. The message sequence is as follow: ![TOA](/docs/images/toa.png)

The time of flight is calculated using the formula:
![Formula TOA](/docs/images/formula_toa2.png){:width="400"}

TWR packet IDs

 |  ID    |  Description|
 |  ------|  ----------------------------------------|
|   0x01  |  [TWR\_POLL](#twr_poll)|
|   0x02  |  [TWR\_ANSWER](#twr_answer)|
|   0x03  |  [TWR\_FINAL](#twr_final)|
|   0x04   | [TWR\_REPORT](#twr_report)|

## TWR\_POLL

Format:

| Payload byte | Name | Note                                                                                 |
| ------------ | ---- | ------------------------------------------------------------------------------------ |
| 0            | SEQ  | 1 Byte sequence number, is repeated in subsequent packet of the same TWR transaction |

## TWR\_ANSWER

Format:

| Payload byte | Name                   | Note                                                     |
| ------------ | ---------------------- | -------------------------------------------------------- |
| 0            | SEQ                    | 1 Byte sequence number, matches the POLL sequence number |
| 1..          | LPP management message | *Optional* LPP management data                           |

The optional management data can be any LPP management packet deemed
useful for the tag. Currently this is used to send the anchor position
from the anchor to the tag using the [LPP Short anchor position packet](/docs/protocols/lpp-short-packets-protocol.md)

## TWR\_FINAL

Format:

| Payload byte | Name | Note                                                     |
| ------------ | ---- | -------------------------------------------------------- |
| 0            | SEQ  | 1 Byte sequence number, matches the POLL sequence number |

## TWR\_REPORT

Format:

| Payload byte | Name    | Note                                                     |
| ------------ | ------- | -------------------------------------------------------- |
| 0            |  SEQ    | 1 Byte sequence number, matches the POLL sequence number |
| 1..          |  Report | Report structure, see format bellow                      |

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
