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
/* uwb_sniffer.c: Uwb sniffer implementation */

/* ------------------ modify the uwb_sniffer.c code for TDoA3 ------------------ */
#include "uwb.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "cfg.h"
#include "led.h"

#include "libdw1000.h"

#include "dwOps.h"
#include "mac.h"

// add header
#include "task.h"
#include "lpp.h"


  //------------------------------- useful constents ----------------------------------- //
  #define REMOTE_TX_MAX_COUNT 8
  #define ID_COUNT 256
  #define ANCHOR_STORAGE_COUNT 16
  #define ANTENNA_OFFSET 154.6   // In meters
  #define ANTENNA_DELAY  ((ANTENNA_OFFSET*499.2e6*128)/299792458.0) // In radio tick
  #define MIN_TOF ANTENNA_DELAY
  #define ID_COUNT 256
  #define ID_WITHOUT_CONTEXT 0xff
  #define ID_INVALID 0xff

  #define MAX_CLOCK_DEVIATION_SPEC 10e-6
  #define CLOCK_CORRECTION_SPEC_MIN (1.0d - MAX_CLOCK_DEVIATION_SPEC * 2)
  #define CLOCK_CORRECTION_SPEC_MAX (1.0d + MAX_CLOCK_DEVIATION_SPEC * 2) 

  #define CLOCK_CORRECTION_ACCEPTED_NOISE 0.03e-6
  #define CLOCK_CORRECTION_FILTER 0.1d
  #define CLOCK_CORRECTION_BUCKET_MAX 4

  // for computing tof ranging distance
  #define ANTENNA_OFFSET  154.6
  #define LOCODECK_TS_FREQ  499.2e6 * 128
  #define SPEED_OF_LIGHT  299792458.0
// -------------------------------- necessary defination ------------------------------ //
// Packet formats
#define PACKET_TYPE_TDOA3 0x30

// Anchor context
typedef struct {
  uint8_t id;
  bool isUsed;
  uint8_t seqNr;
  uint32_t rxTimeStamp;
  uint32_t txTimeStamp;
  uint16_t distance;
  uint32_t distanceUpdateTime;
  bool isDataGoodForTransmission;

  double clockCorrection;
  int clockCorrectionBucket;
} anchorContext_t;

typedef struct {
  uint8_t type;
  uint8_t seq;
  uint32_t txTimeStamp;
  uint8_t remoteCount;
} __attribute__((packed)) rangePacketHeader3_t;

typedef struct {
  uint8_t id;
  uint8_t seq;
  uint32_t rxTimeStamp;
  uint16_t distance;
} __attribute__((packed)) remoteAnchorDataFull_t;

typedef struct {
  uint8_t id;
  uint8_t seq;
  uint32_t rxTimeStamp;
} __attribute__((packed)) remoteAnchorDataShort_t;

typedef struct {
  rangePacketHeader3_t header;
  uint8_t remoteAnchorData;
} __attribute__((packed)) rangePacket3_t;



// This context struct contains all the required global values of the algorithm
static struct ctx_s {
  int anchorId;
  // Information about latest transmitted packet
  uint8_t seqNr;
  uint32_t txTime; // In UWB clock ticks

  // Next transmit time in system clock ticks
  uint32_t nextTxTick;
  int averageTxDelay; // ms

  // List of ids to transmit in remote data section
  uint8_t remoteTxId[REMOTE_TX_MAX_COUNT];
  uint8_t remoteTxIdCount;

  // The list of anchors to transmit and store is updated at regular intervals
  uint32_t nextAnchorListUpdate;

  // Remote anchor data
  uint8_t anchorCtxLookup[ID_COUNT];
  anchorContext_t anchorCtx[ANCHOR_STORAGE_COUNT];
  uint8_t anchorRxCount[ID_COUNT];
} ctx;


// --------------------------------------------------------------------------------- //


static void setupRx(dwDevice_t *dev)
{
  dwNewReceive(dev);
  dwSetDefaults(dev);
  dwStartReceive(dev);
}


static uint32_t tdoa3SnifferOnEvent(dwDevice_t *dev, uwbEvent_t event){
   
  static dwTime_t arrival;
  static packet_t rxPacket;

  if (event == eventPacketReceived) {

    int dataLength = dwGetDataLength(dev);
    dwGetRawReceiveTimestamp(dev, &arrival);
    dwGetData(dev, (uint8_t*)&rxPacket, dataLength);

    // Reset the radio into receive mode 
    setupRx(dev);

    // the anchor ID that send the radio signal
    uint8_t remoteAnchorId = rxPacket.sourceAddress[0]; 
    const rangePacket3_t* rangePacket = (rangePacket3_t *)rxPacket.payload;
    const void* anchorDataPtr = &rangePacket->remoteAnchorData;

    remoteAnchorDataFull_t* anchorData = (remoteAnchorDataFull_t*)anchorDataPtr;
    // the anchor ID that receives the radio signal
    const uint8_t id = anchorData->id;
    // printf("id in the remoteAnchorDatafull: %d \r\n", (int)id);
    bool hasDistance = ((anchorData->seq & 0x80) != 0);
    if (hasDistance) {
        uint16_t tof = anchorData->distance;
    //  M_PER_TICK = SPEED_OF_LIGHT / LOCODECK_TS_FREQ
    //  precompute value
        double M_PER_TICK = 0.0046917639786157855; 
    // printf("M_PER_TICK: %.19f\r\n", M_PER_TICK);
        double ranging = tof * M_PER_TICK - ANTENNA_OFFSET; 
        printf("tof ranging distance from anchor %d to anchor %d: %lf \r\n", (int) remoteAnchorId,  (int)id, ranging);
    }

  } else {
    // Reset the radio into receive mode 
    setupRx(dev);
  }

  return MAX_TIMEOUT;
}

// original sniffer Event code
static uint32_t twrAnchorOnEvent(dwDevice_t *dev, uwbEvent_t event)
{
  static dwTime_t arrival;
  static packet_t rxPacket;

  if (event == eventPacketReceived) {
    int dataLength = dwGetDataLength(dev);
    dwGetRawReceiveTimestamp(dev, &arrival);
    dwGetData(dev, (uint8_t*)&rxPacket, dataLength);

    dwNewReceive(dev);
    dwSetDefaults(dev);
    dwStartReceive(dev);

    if (cfgIsBinaryMode()) {
      write(STDOUT_FILENO, "\xbc", 1);  // Write a header to show it's in sniffer mode
      write(STDOUT_FILENO, &arrival.full, 5);
      write(STDOUT_FILENO, &rxPacket.sourceAddress[0], 1);
      write(STDOUT_FILENO, &rxPacket.destAddress[0], 1);
      dataLength -= MAC802154_HEADER_LENGTH;
      write(STDOUT_FILENO, &dataLength, 2);
      write(STDOUT_FILENO, rxPacket.payload, dataLength);  // This is the data
      write(STDOUT_FILENO, &dataLength, 2);  // Length repeated for sync detection
    } else {
      printf("From %02x to %02x @%02x%08x: ", rxPacket.sourceAddress[0],
                                            rxPacket.destAddress[0],
                                            (unsigned int) arrival.high8,
                                            (unsigned int) arrival.low32);
      for (int i=0; i<(dataLength - MAC802154_HEADER_LENGTH); i++) {
        printf("%02x", rxPacket.payload[i]);
      }
      printf("\r\n");
    }

  } else {
    dwNewReceive(dev);
    dwSetDefaults(dev);
    dwStartReceive(dev);
  }

  return MAX_TIMEOUT;
}

static void SnifferInit(uwbConfig_t * newconfig, dwDevice_t *dev)
{
  // Set the LED for anchor mode
  ledBlink(ledMode, false);
}

uwbAlgorithm_t uwbSnifferAlgorithm = {
  .init = SnifferInit,
  .onEvent = tdoa3SnifferOnEvent,
//   .onEvent = twrAnchorOnEvent,
};
