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
/* uwb_tdoa_anchor3.c: Uwb TDOA anchor V3, randomized tx times and ids */

/*
 * This anchor algorithm is transmitting messages at random times to remove the
 * dependency to anchor 0 in TDoA V2. Ids a also randomized to support dynamic
 * setups with varying number of anchors.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "uwb.h"
#include "libdw1000.h"
#include "mac.h"

#include "cfg.h"
#include "lpp.h"

#define debug(...) printf(__VA_ARGS__)

// Still using modulo 2 calculation for slots
// TODO: If A0 is the TDMA master it could transmit slots parameters and frame
//       start so that we would not be limited to modulo 2 anymore
#define NSLOTS 8
#define TDMA_SLOT_BITS 26 // 26: 2ms timeslot
#define TDMA_NSLOT_BITS 3

#define TDMA_FRAME_BITS (TDMA_SLOT_BITS + TDMA_NSLOT_BITS)
#define TDMA_SLOT_LEN (1ull<<(TDMA_SLOT_BITS+1))
#define TDMA_FRAME_LEN (1ull<<(TDMA_FRAME_BITS+1))

#define TDMA_LAST_FRAME(NOW) ( NOW & ~(TDMA_FRAME_LEN-1) )

// Time length of the preamble
#define PREAMBLE_LENGTH_S ( 128 * 1017.63e-9 )
#define PREAMBLE_LENGTH (uint64_t)( PREAMBLE_LENGTH_S * 499.2e6 * 128 )

// Guard length to account for clock drift and time of flight
#define TDMA_GUARD_LENGTH_S ( 1e-6 )
#define TDMA_GUARD_LENGTH (uint64_t)( TDMA_GUARD_LENGTH_S * 499.2e6 * 128 )

// Timeout for receiving a packet in a timeslot
#define RECEIVE_TIMEOUT 300

// Timeout for receiving a service packet after we TX ours
#define RECEIVE_SERVICE_TIMEOUT 800

#define TS_TX_SIZE 4

// Useful constants
static const uint8_t base_address[] = {0,0,0,0,0,0,0xcf,0xbc};

// FSM states
enum state_e {
  syncTdmaState = 0, // Anchors 1 to 5 starts here and rise up to synchronizedState
  syncTimeState,
  synchronizedState, // Anchor 0 is always here!
};

enum slotState_e {
  slotRxDone,
  slotTxDone,
};

// This context struct contains all the requied global values of the algorithm
static struct ctx_s {
  int anchorId;
  enum state_e state;
  enum slotState_e slotState;

  // Current and next TDMA slot
  int slot;
  int nextSlot;

  // Current packet id and tx timestamps
  uint8_t pid;

  // TDMA start of frame in local clock
  dwTime_t tdmaFrameStart;

  // list of timestamps and ids for last frame.
  uint8_t packetIds[NSLOTS];
  uint32_t rxTimestamps[NSLOTS];
  uint32_t txTimestamps[NSLOTS];

  uint16_t distances[NSLOTS];
} ctx;

// Packet formats
#define PACKET_TYPE_TDOA3 0x30
#define TDOA_3_REMOTE_COUNT_MAX 7

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


typedef struct {
  uint8_t type;
  uint8_t pid[NSLOTS];  // Packet id of the timestamps
  uint8_t timestamps[NSLOTS][TS_TX_SIZE];  // Relevant time for anchors
  uint16_t distances[NSLOTS];
} __attribute__((packed)) rangePacket2_t;

#define FULL_RANGE_PACKET_SIZE (sizeof(rangePacketHeader3_t) + TDOA_3_REMOTE_COUNT_MAX * sizeof(remoteAnchorDataFull_t))

// Current implementation always sends TDOA_3_REMOTE_COUNT_MAX remote anchor data
#define LPP_HEADER FULL_RANGE_PACKET_SIZE
#define LPP_TYPE (LPP_HEADER + 1)
#define LPP_PAYLOAD (LPP_HEADER + 2)

/* Adjust time for schedule transfer by DW1000 radio. Set 9 LSB to 0 */
static uint32_t adjustTxRxTime(dwTime_t *time)
{
  uint32_t added = (1<<9) - (time->low32 & ((1<<9)-1));

  time->low32 = (time->low32 & ~((1<<9)-1)) + (1<<9);

  return added;
}

/* Calculate the transmit time for a given timeslot in the current frame */
static dwTime_t transmitTimeForSlot(int slot)
{
  dwTime_t transmitTime = { .full = 0 };

  // Calculate start of the slot
  transmitTime.full = ctx.tdmaFrameStart.full + slot*TDMA_SLOT_LEN;
  // Add guard and preamble time
  transmitTime.full += TDMA_GUARD_LENGTH;
  transmitTime.full += PREAMBLE_LENGTH;

  // DW1000 can only schedule time with 9 LSB at 0, adjust for it
  adjustTxRxTime(&transmitTime);

  return transmitTime;
}

static void handleFailedRx(dwDevice_t *dev)
{

  ctx.rxTimestamps[ctx.slot] = 0;
  ctx.distances[ctx.slot] = 0;

  // Failed TDMA sync, keeps track of the number of fail so that the TDMA
  // watchdog can take decision as of TDMA resynchronisation
  if (ctx.slot == 0) {
    ctx.state = syncTdmaState;
  }
}

static void calculateDistance(int slot, int newId, int remotePid, uint32_t remoteTx, uint32_t remoteRx, uint32_t ts)
{
  // Check that the 2 last packets are consecutive packets and that our last packet is in beteen
  if ((ctx.packetIds[slot] == ((newId-1) & 0x07f)) && remotePid == ctx.packetIds[ctx.anchorId]) {
    double tround1 = remoteRx - ctx.txTimestamps[ctx.slot];
    double treply1 = ctx.txTimestamps[ctx.anchorId] - ctx.rxTimestamps[ctx.slot];
    double tround2 = ts - ctx.txTimestamps[ctx.anchorId];
    double treply2 = remoteTx - remoteRx;

    uint32_t distance = ((tround2 * tround1)-(treply1 * treply2)) / (2*(treply1 + tround2));
    ctx.distances[slot] = distance & 0xfffful;
  } else {
    ctx.distances[slot] = 0;
  }
}

static void convertTdoa3ToTdoa2(const uint8_t slot, const rangePacket3_t* range3, rangePacket2_t* range2)
{
  memset(range2, 0, sizeof(rangePacket2_t));

  range2->pid[slot] = range3->header.seq;
  memcpy(&range2->timestamps[slot], &range3->header.txTimeStamp, TS_TX_SIZE);

  const void* anchorDataPtr = &range3->remoteAnchorData;
  for (uint8_t i = 0; i < range3->header.remoteCount; i++) {
    remoteAnchorDataFull_t* anchorData = (remoteAnchorDataFull_t*)anchorDataPtr;

    // TODO krri For now assume id < 8
    uint8_t id = anchorData->id;
    uint8_t seq = anchorData->seq & 0x7f;
    bool hasDistance = ((anchorData->seq & 0x80) != 0);

    range2->pid[id] = seq;
    memcpy(&range2->timestamps[id], &anchorData->rxTimeStamp, TS_TX_SIZE);

    if (hasDistance) {
      range2->distances[id] = anchorData->distance;
      anchorDataPtr += sizeof(remoteAnchorDataFull_t);
    } else {
      anchorDataPtr += sizeof(remoteAnchorDataShort_t);
    }
  }
}

static void handleRxPacket(dwDevice_t *dev)
{
  static packet_t rxPacket;
  dwTime_t rxTime = { .full = 0 };

  dwGetRawReceiveTimestamp(dev, &rxTime);
  dwCorrectTimestamp(dev, &rxTime);

  int dataLength = dwGetDataLength(dev);
  rxPacket.payload[0] = 0;
  dwGetData(dev, (uint8_t*)&rxPacket, dataLength);

  if (dataLength == 0 || rxPacket.payload[0] != PACKET_TYPE_TDOA3 || rxPacket.sourceAddress[0] != ctx.slot) {
    handleFailedRx(dev);
    return;
  }
  rangePacket3_t* rangePacket3 = (rangePacket3_t *)rxPacket.payload;

  rangePacket2_t rangePacketMem;
  rangePacket2_t* rangePacket = &rangePacketMem;
  convertTdoa3ToTdoa2(ctx.slot, rangePacket3, rangePacket);

  uint32_t remoteTx;
  memcpy(&remoteTx, rangePacket->timestamps[ctx.slot], 4);
  uint32_t remoteRx;
  memcpy(&remoteRx, rangePacket->timestamps[ctx.anchorId], 4);

  calculateDistance(ctx.slot, rangePacket->pid[ctx.slot], rangePacket->pid[ctx.anchorId],
                    remoteTx, remoteRx, rxTime.low32);

  ctx.packetIds[ctx.slot] = rangePacket->pid[ctx.slot];
  ctx.rxTimestamps[ctx.slot] = rxTime.low32;
  memcpy(&ctx.txTimestamps[ctx.slot], &rangePacket->timestamps[ctx.slot], 4);

  // Resync TDMA and save useful anchor 0 information
  if (ctx.slot == 0) {
    // Resync local frame start to packet from anchor 0
    dwTime_t pkTxTime = { .full = 0 };
    memcpy(&pkTxTime, rangePacket->timestamps[ctx.slot], TS_TX_SIZE);
    ctx.tdmaFrameStart.full = rxTime.full - (pkTxTime.full - TDMA_LAST_FRAME(pkTxTime.full));

    //TODO: Save relevant data to calculate masterTime
  }
}

static void handleServicePacket(dwDevice_t *dev)
{
  static packet_t servicePacket;

  int dataLength = dwGetDataLength(dev);
  servicePacket.payload[0] = 0;
  dwGetData(dev, (uint8_t*)&servicePacket, dataLength);

  if (servicePacket.payload[0] == SHORT_LPP) {
    lppHandleShortPacket(&servicePacket.payload[1], dataLength - MAC802154_HEADER_LENGTH - 1);
  }
}

// Setup the radio to receive a packet in the next timeslot
static void setupRx(dwDevice_t *dev)
{
  dwTime_t receiveTime = { .full = 0 };

  // Calculate start of the slot
  receiveTime.full = ctx.tdmaFrameStart.full + ctx.nextSlot*TDMA_SLOT_LEN;

  dwSetReceiveWaitTimeout(dev, RECEIVE_TIMEOUT);
  dwWriteSystemConfigurationRegister(dev);

  dwNewReceive(dev);
  dwSetDefaults(dev);
  dwSetTxRxTime(dev, receiveTime);
  dwStartReceive(dev);
}


static int populateTxData(rangePacket3_t *rangePacket)
{
  // rangePacket->header.type already populated
  rangePacket->header.seq = ctx.packetIds[ctx.anchorId];
  memcpy(&rangePacket->header.txTimeStamp, &ctx.txTimestamps[ctx.anchorId], TS_TX_SIZE);
  rangePacket->header.remoteCount = NSLOTS - 1;

  remoteAnchorDataFull_t* anchorData = (remoteAnchorDataFull_t*) &rangePacket->remoteAnchorData;
  for (uint8_t i = 0; i < NSLOTS; i++) {
    if (i != ctx.anchorId) {
      anchorData->id = i;
      anchorData->seq = ctx.packetIds[i] | 0x80;
      memcpy(&anchorData->rxTimeStamp, &ctx.rxTimestamps[i], TS_TX_SIZE);
      anchorData->distance = ctx.distances[i];

      anchorData++;
    }
  }

  return FULL_RANGE_PACKET_SIZE;
}

// Set TX data in the radio TX buffer
static void setTxData(dwDevice_t *dev)
{
  static packet_t txPacket;
  static bool firstEntry = true;
  static int lppLength = 0;

  if (firstEntry) {

    memcpy(txPacket.sourceAddress, base_address, 8);
    txPacket.sourceAddress[0] = ctx.anchorId;
    memcpy(txPacket.destAddress, base_address, 8);
    txPacket.destAddress[0] = 0xff;

    txPacket.payload[0] = PACKET_TYPE_TDOA3;

    firstEntry = false;
  }

  uwbConfig_t *uwbConfig = uwbGetConfig();

  int rangePacketSize = populateTxData((rangePacket3_t *)txPacket.payload);

  // LPP anchor position is currently sent in all packets
  if (uwbConfig->positionEnabled) {
    txPacket.payload[LPP_HEADER] = SHORT_LPP;
    txPacket.payload[LPP_TYPE] = LPP_SHORT_ANCHOR_POSITION;

    struct lppShortAnchorPosition_s *pos = (struct lppShortAnchorPosition_s*) &txPacket.payload[LPP_PAYLOAD];
    memcpy(pos->position, uwbConfig->position, 3*sizeof(float));

    lppLength = 2 + sizeof(struct lppShortAnchorPosition_s);
  }

  dwSetData(dev, (uint8_t*)&txPacket, MAC802154_HEADER_LENGTH + rangePacketSize + lppLength);
}

// Setup the radio to send a packet in the next timeslot
static void setupTx(dwDevice_t *dev)
{
  ctx.packetIds[ctx.anchorId] = ctx.pid;
  ctx.pid = (ctx.pid + 1) & 0x7f;

  dwTime_t txTime = transmitTimeForSlot(ctx.nextSlot);
  ctx.txTimestamps[ctx.anchorId] = txTime.low32;

  dwSetReceiveWaitTimeout(dev, RECEIVE_SERVICE_TIMEOUT);
  dwWriteSystemConfigurationRegister(dev);

  dwNewTransmit(dev);
  dwSetDefaults(dev);
  setTxData(dev);
  dwSetTxRxTime(dev, txTime);

  dwWaitForResponse(dev, true);
  dwStartTransmit(dev);
}

// Increment the slot variables and, if required, switch tdmaStartFrame to next
// frame state time
static void updateSlot()
{
  ctx.slot = ctx.nextSlot;
  ctx.nextSlot = ctx.nextSlot + 1;
  if (ctx.nextSlot >= NSLOTS) {
    ctx.nextSlot = 0;
  }

  // If the next slot is 0, the next schedule has to be in the next frame!
  if (ctx.nextSlot == 0) {
    ctx.tdmaFrameStart.full += TDMA_FRAME_LEN;
  }
}

// slotStep is called once per timeslot as long as TDMA is synched and setup
// the next timeslot action
static uint32_t slotStep(dwDevice_t *dev, uwbEvent_t event)
{
  switch (ctx.slotState) {
    case slotRxDone:
      if (event == eventPacketReceived) {
        handleRxPacket(dev);
      } else {
        handleFailedRx(dev);
      }

      // Quickly setup transfer to next slot
      if (ctx.nextSlot == ctx.anchorId) {
        setupTx(dev);
        ctx.slotState = slotTxDone;
        updateSlot();
      } else {
        setupRx(dev);
        ctx.slotState = slotRxDone;
        updateSlot();
      }

      break;
    case slotTxDone:
    // We try to receive an LPP packet after sending our packet.
    // After this is done, we setup the next receive.
      if (event == eventPacketReceived || event == eventReceiveTimeout) {
        if (event == eventPacketReceived) {
          debug("Received service packet!\r\n");
          handleServicePacket(dev);
          // The service packet handling time desynchronized us, lets resynch
          ctx.state = syncTdmaState;
          return 0;
        }
        setupRx(dev);
        ctx.slotState = slotRxDone;
        updateSlot();
      }
      break;
  }

  return MAX_TIMEOUT;
}

// Initialize/reset the agorithm
static void tdoa3Init(uwbConfig_t * config, dwDevice_t *dev)
{
  ctx.anchorId = config->address[0];
  ctx.state = syncTdmaState;
  ctx.slot = NSLOTS-1;
  ctx.nextSlot = 0;
  memset(ctx.txTimestamps, 0, sizeof(ctx.txTimestamps));
  memset(ctx.rxTimestamps, 0, sizeof(ctx.rxTimestamps));
}

// Called for each DW radio event
static uint32_t tdoa3UwbEvent(dwDevice_t *dev, uwbEvent_t event)
{
  if (ctx.state == synchronizedState) {
    return slotStep(dev, event);
  } else {
    if (ctx.anchorId == 0) {
      dwGetSystemTimestamp(dev, &ctx.tdmaFrameStart);
      ctx.tdmaFrameStart.full = TDMA_LAST_FRAME(ctx.tdmaFrameStart.full) + 2*TDMA_FRAME_LEN;
      ctx.state = synchronizedState;
      setupTx(dev);

      ctx.slotState = slotTxDone;
      updateSlot();
    } else {
      switch (event) {
        case eventPacketReceived: {
            static packet_t rxPacket;
            dwTime_t rxTime = { .full = 0 };
            dwGetReceiveTimestamp(dev, &rxTime);
            int dataLength = dwGetDataLength(dev);
            dwGetData(dev, (uint8_t*)&rxPacket, dataLength);

            if (rxPacket.sourceAddress[0] == 0 && rxPacket.payload[0] == PACKET_TYPE_TDOA3) {
              rangePacket3_t * rangePacket = (rangePacket3_t *)rxPacket.payload;

              // Resync local frame start to packet from anchor 0
              dwTime_t pkTxTime = { .full = 0 };
              memcpy(&pkTxTime, &rangePacket->header.txTimeStamp, TS_TX_SIZE);
              ctx.tdmaFrameStart.full = rxTime.full - (pkTxTime.full - TDMA_LAST_FRAME(pkTxTime.full));

              ctx.tdmaFrameStart.full += TDMA_FRAME_LEN;

              setupTx(dev);
              ctx.slotState = slotRxDone;
              ctx.state = synchronizedState;
              updateSlot();
            } else {
              // Start the receiver waiting for a packet from anchor 0
              dwIdle(dev);
              dwSetReceiveWaitTimeout(dev, RECEIVE_TIMEOUT);
              dwWriteSystemConfigurationRegister(dev);

              dwNewReceive(dev);
              dwSetDefaults(dev);
              dwStartReceive(dev);
            }
          }
          break;
        default:
          // Start the receiver waiting for a packet from anchor 0
          dwIdle(dev);
          dwSetReceiveWaitTimeout(dev, RECEIVE_TIMEOUT);
          dwWriteSystemConfigurationRegister(dev);

          dwNewReceive(dev);
          dwSetDefaults(dev);
          dwStartReceive(dev);
          break;
      }
    }
  }

  return MAX_TIMEOUT;
}

uwbAlgorithm_t uwbTdoa3Algorithm = {
  .init = tdoa3Init,
  .onEvent = tdoa3UwbEvent,
};
