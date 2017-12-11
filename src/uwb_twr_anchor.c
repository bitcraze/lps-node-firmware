/*
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * LPS node firmware.
 *
 * Copyright 2016, Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
/* uwb_twr_anchor.c: Uwb two way ranging anchor implementation */

#include "uwb.h"

#include <string.h>
#include <stdio.h>

#include "cfg.h"
#include "led.h"

#include "lpp.h"

#include "libdw1000.h"

#include "dwOps.h"
#include "mac.h"

uint8_t base_address[] = {0,0,0,0,0,0,0xcf,0xbc};

// System configuration
static struct uwbConfig_s config;

// The four packets for ranging
#define POLL 0x01   // Poll is initiated by the tag
#define ANSWER 0x02
#define FINAL 0x03
#define REPORT 0x04 // Report contains all measurement from the anchor

typedef struct {
  uint8_t pollRx[5];
  uint8_t answerTx[5];
  uint8_t finalRx[5];

  float pressure;
  float temperature;
  float asl;
  uint8_t pressure_ok;
} __attribute__((packed)) reportPayload_t;

typedef union timestamp_u {
  uint8_t raw[5];
  uint64_t full;
  struct {
    uint32_t low32;
    uint8_t high8;
  } __attribute__((packed));
  struct {
    uint8_t low8;
    uint32_t high32;
  } __attribute__((packed));
} timestamp_t;

// Timestamps for ranging
dwTime_t poll_tx;
dwTime_t poll_rx;
dwTime_t answer_tx;
dwTime_t answer_rx;
dwTime_t final_tx;
dwTime_t final_rx;

uint32_t rangingTick;

float pressure, temperature, asl;
bool pressure_ok;

const double C = 299792458.0;       // Speed of light
const double tsfreq = 499.2e6 * 128;  // Timestamp counter frequency

#define ANTENNA_OFFSET 154.6   // In meter
#define ANTENNA_DELAY  (ANTENNA_OFFSET*499.2e6*128)/299792458.0 // In radio tick

static packet_t rxPacket;
static packet_t txPacket;
static volatile uint8_t curr_tag = 0;

// #define printf(...)
#define debug(...) // printf(__VA_ARGS__)

static void txcallback(dwDevice_t *dev)
{
  dwTime_t departure;
  dwGetTransmitTimestamp(dev, &departure);
  departure.full += (ANTENNA_DELAY/2);

  debug("TXCallback: ");

  switch (txPacket.payload[0]) {
    case ANSWER:
    debug("ANSWER to %02x at %04x\r\n", txPacket.destAddress[0], (unsigned int)departure.low32);
      answer_tx = departure;
      break;
    case REPORT:
      debug("REPORT\r\n");
      break;
  }
}

#define TYPE 0
#define SEQ 1
#define LPP_HEADER 2
#define LPP_TYPE 3
#define LPP_PAYLOAD 4

static void rxcallback(dwDevice_t *dev) {
  dwTime_t arival = { .full=0 };
  int dataLength = dwGetDataLength(dev);

  if (dataLength == 0) return;

  bzero(&rxPacket, MAC802154_HEADER_LENGTH);

  debug("RXCallback(%d): ", dataLength);

  dwGetData(dev, (uint8_t*)&rxPacket, dataLength);

  if (memcmp(rxPacket.destAddress, config.address, 8)) {
    debug("Not for me! for %02x with %02x\r\n", rxPacket.destAddress[0], rxPacket.payload[0]);
    dwNewReceive(dev);
    dwSetDefaults(dev);
    dwStartReceive(dev);
    return;
  }

  memcpy(txPacket.destAddress, rxPacket.sourceAddress, 8);
  memcpy(txPacket.sourceAddress, rxPacket.destAddress, 8);

  switch(rxPacket.payload[TYPE]) {
    // Anchor received messages
    case POLL:
      debug("POLL from %02x at %04x\r\n", rxPacket.sourceAddress[0], (unsigned int)arival.low32);
      rangingTick = HAL_GetTick();
      ledBlink(ledRanging, true);

      curr_tag = rxPacket.sourceAddress[0];

      int payloadLength = 2;
      txPacket.payload[TYPE] = ANSWER;
      txPacket.payload[SEQ] = rxPacket.payload[SEQ];

      uwbConfig_t *uwbConfig = uwbGetConfig();
      if (uwbConfig->positionEnabled) {
        txPacket.payload[LPP_HEADER] = SHORT_LPP;
        txPacket.payload[LPP_TYPE] = LPP_SHORT_ANCHOR_POSITION;

        struct lppShortAnchorPosition_s *pos = (struct lppShortAnchorPosition_s*) &txPacket.payload[LPP_PAYLOAD];
        memcpy(pos->position, uwbConfig->position, 3*sizeof(float));

        payloadLength += 2 + sizeof(struct lppShortAnchorPosition_s);
      }

      dwNewTransmit(dev);
      dwSetDefaults(dev);
      dwSetData(dev, (uint8_t*)&txPacket, MAC802154_HEADER_LENGTH+payloadLength);

      dwWaitForResponse(dev, true);
      dwStartTransmit(dev);

      dwGetReceiveTimestamp(dev, &arival);
      arival.full -= (ANTENNA_DELAY/2);
      poll_rx = arival;
      break;
    case FINAL:
    {
      if (curr_tag == rxPacket.sourceAddress[0]) {
        reportPayload_t *report = (reportPayload_t *)(txPacket.payload+2);

        debug("FINAL\r\n");

        dwGetReceiveTimestamp(dev, &arival);
        arival.full -= (ANTENNA_DELAY/2);
        final_rx = arival;

        txPacket.payload[TYPE] = REPORT;
        txPacket.payload[SEQ] = rxPacket.payload[SEQ];
        memcpy(&report->pollRx, &poll_rx, 5);
        memcpy(&report->answerTx, &answer_tx, 5);
        memcpy(&report->finalRx, &final_rx, 5);
        report->pressure = pressure;
        report->temperature = temperature;
        report->asl = asl;
        report->pressure_ok = pressure_ok;

        dwNewTransmit(dev);
        dwSetDefaults(dev);
        dwSetData(dev, (uint8_t*)&txPacket, MAC802154_HEADER_LENGTH+2+sizeof(reportPayload_t));

        dwWaitForResponse(dev, true);
        dwStartTransmit(dev);
      } else {
        dwNewReceive(dev);
        dwSetDefaults(dev);
        dwStartReceive(dev);
      }

      break;
    }
    case SHORT_LPP:
    {
      if(curr_tag == rxPacket.sourceAddress[0] && dataLength-MAC802154_HEADER_LENGTH > 1) {
        lppHandleShortPacket(&rxPacket.payload[1], dataLength-MAC802154_HEADER_LENGTH-1);
      }

      dwNewReceive(dev);
      dwSetDefaults(dev);
      dwStartReceive(dev);

      break;
    }
  }
}

static uint32_t twrAnchorOnEvent(dwDevice_t *dev, uwbEvent_t event)
{
  switch(event) {
    case eventPacketReceived:
      rxcallback(dev);
      break;
    case eventPacketSent:
      txcallback(dev);
      break;
    case eventTimeout:
    case eventReceiveFailed:
      dwNewReceive(dev);
      dwSetDefaults(dev);
      dwStartReceive(dev);
      break;
    default:
      configASSERT(false);
  }

  return MAX_TIMEOUT;
}

static void twrAnchorInit(uwbConfig_t * newconfig, dwDevice_t *dev)
{
  // Set the LED for anchor mode
  ledOn(ledMode);

  config = *newconfig;

  // Initialize the packet in the TX buffer
  MAC80215_PACKET_INIT(txPacket, MAC802154_TYPE_DATA);
  txPacket.pan = 0xbccf;

  // onEvent is going to be called with eventTimeout which will start receiving
}

uwbAlgorithm_t uwbTwrAnchorAlgorithm = {
  .init = twrAnchorInit,
  .onEvent = twrAnchorOnEvent,
};
