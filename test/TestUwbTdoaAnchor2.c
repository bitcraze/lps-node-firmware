// Unit tests for uwb_tdoa_anchor2.c
//
// The source file is included directly to get access to the static algorithm
// context. The module has no header.
//
// The radio (libdw1000) is mocked, and the functions that matter are stubbed
// with callbacks that form a small fake radio. The fake radio is driven by a
// simulated system with anchor 0 and the anchor under test.

#include <string.h>

#include "unity.h"

#include "mock_libdw1000.h"
#include "mock_uwb.h"
#include "mock_lpp.h"

#include "uwb_tdoa_anchor2.c"

#define FIRST_ANCHOR_ID 1
#define LAST_ANCHOR_ID (NSLOTS - 1)

#define FRAMES_TO_RUN 5

// The local clock of the anchor under test runs at the same rate as the clock
// of anchor 0, but with an offset. The time of flight is 0.
#define CLOCK_OFFSET ((uint64_t)123456789)

// Time from the start of a slot to the transmission in that slot
#define TX_TIME_IN_SLOT ((((uint64_t)TDMA_GUARD_LENGTH + PREAMBLE_LENGTH) & ~(uint64_t)0x1ff) + 0x200)

// The anchor under test boots in the middle of a frame
#define BOOT_TIME (TDMA_FRAME_LEN + 3 * TDMA_SLOT_LEN + TDMA_SLOT_LEN / 2)

#define MAX_TX_COUNT 32
#define MAX_IMMEDIATE_TIMEOUTS 10

// Simulated system ------------------------------------------------------------

typedef enum {
  opNone = 0,
  opTx,
  opRx,
} radioOpType_t;

typedef struct {
  radioOpType_t type;
  bool isDelayed;
  uint64_t time;  // Scheduled time in local clock, only valid if isDelayed
} radioOp_t;

static struct {
  // Current time and end of the last run, in the clock of anchor 0
  uint64_t now;
  uint64_t runEnd;

  // Radio operation that is being set up, and the one that has been started
  radioOp_t setupOp;
  radioOp_t pendingOp;

  // Received packet presented by the radio
  packet_t rxPacket;
  unsigned int rxPacketLength;
  uint64_t rxTime;  // Local clock

  uint8_t anchor0Pid;

  // Number of coming slot-0 packets from anchor 0 that are lost
  int slot0PacketsToDrop;
  // Send an LPP service packet in the response window after the next TX
  bool sendServicePacket;

  // Slots of the transmissions of the anchor under test
  int txSlots[MAX_TX_COUNT];
  int txCount;
  // Number of times the anchor synchronized while waiting for anchor 0
  int syncListenCount;
} sim;

static uwbConfig_t config;
static dwDevice_t dev;

// Fake radio ------------------------------------------------------------------

static void fakeNewTransmit(dwDevice_t* dev, int numCalls) {
  (void)dev; (void)numCalls;
  sim.setupOp.type = opTx;
  sim.setupOp.isDelayed = false;
}

static void fakeNewReceive(dwDevice_t* dev, int numCalls) {
  (void)dev; (void)numCalls;
  sim.setupOp.type = opRx;
  sim.setupOp.isDelayed = false;
}

static void fakeSetTxRxTime(dwDevice_t* dev, const dwTime_t futureTime, int numCalls) {
  (void)dev; (void)numCalls;
  sim.setupOp.isDelayed = true;
  sim.setupOp.time = futureTime.full;
}

static void fakeStartTransmit(dwDevice_t* dev, int numCalls) {
  (void)dev; (void)numCalls;
  sim.pendingOp = sim.setupOp;
}

static void fakeStartReceive(dwDevice_t* dev, int numCalls) {
  (void)dev; (void)numCalls;
  sim.pendingOp = sim.setupOp;
}

static void fakeGetData(dwDevice_t* dev, uint8_t data[], unsigned int n, int numCalls) {
  (void)dev; (void)numCalls;
  memcpy(data, &sim.rxPacket, n);
}

static unsigned int fakeGetDataLength(dwDevice_t* dev, int numCalls) {
  (void)dev; (void)numCalls;
  return sim.rxPacketLength;
}

static void fakeGetReceiveTimestamp(dwDevice_t* dev, dwTime_t* time, int numCalls) {
  (void)dev; (void)numCalls;
  time->full = sim.rxTime;
}

static void setUpMocks() {
  dwNewTransmit_StubWithCallback(fakeNewTransmit);
  dwNewReceive_StubWithCallback(fakeNewReceive);
  dwSetTxRxTime_StubWithCallback(fakeSetTxRxTime);
  dwStartTransmit_StubWithCallback(fakeStartTransmit);
  dwStartReceive_StubWithCallback(fakeStartReceive);
  dwGetData_StubWithCallback(fakeGetData);
  dwGetDataLength_StubWithCallback(fakeGetDataLength);
  dwGetReceiveTimestamp_StubWithCallback(fakeGetReceiveTimestamp);
  dwGetRawReceiveTimestamp_StubWithCallback(fakeGetReceiveTimestamp);

  dwCorrectTimestamp_Ignore();
  dwIdle_Ignore();
  dwSetDefaults_Ignore();
  dwSetData_Ignore();
  dwSetReceiveWaitTimeout_Ignore();
  dwWriteSystemConfigurationRegister_Ignore();
  dwWaitForResponse_Ignore();

  uwbGetConfig_IgnoreAndReturn(&config);
  lppHandleShortPacket_Ignore();
}

// Simulation ------------------------------------------------------------------

// Slot in the frame of anchor 0 for a time in local clock
static int slotOfLocalTime(uint64_t localTime) {
  return ((localTime - CLOCK_OFFSET) % TDMA_FRAME_LEN) / TDMA_SLOT_LEN;
}

// Time of the next transmission of anchor 0 after the current time
static uint64_t nextAnchor0TxTime() {
  uint64_t txTime = TDMA_LAST_FRAME(sim.now) + TX_TIME_IN_SLOT;
  if (txTime <= sim.now) {
    txTime += TDMA_FRAME_LEN;
  }
  return txTime;
}

// Time of the event that completes the pending radio operation
static uint64_t pendingOpTime() {
  if (sim.pendingOp.isDelayed) {
    return sim.pendingOp.time - CLOCK_OFFSET;
  }
  return nextAnchor0TxTime();
}

// Send an event to the anchor under test. Like uwbTask() in src/uwb.c, a
// returned timeout of 0 gives an immediate eventTimeout.
static void sendEvent(uwbEvent_t event) {
  uint32_t timeout = uwbTdoa2Algorithm.onEvent(&dev, event);

  int immediateTimeouts = 0;
  while (timeout == 0) {
    immediateTimeouts++;
    TEST_ASSERT_TRUE_MESSAGE(immediateTimeouts <= MAX_IMMEDIATE_TIMEOUTS, "Endless immediate timeouts");
    timeout = uwbTdoa2Algorithm.onEvent(&dev, eventTimeout);
  }
}

// The TDOA2 packet anchor 0 transmits in slot 0, transmitted and received at
// the current time
static void receiveAnchor0Packet() {
  memset(&sim.rxPacket, 0, sizeof(sim.rxPacket));
  sim.rxPacket.sourceAddress[0] = 0;

  rangePacket_t* rangePacket = (rangePacket_t*)sim.rxPacket.payload;
  rangePacket->type = PACKET_TYPE_TDOA2;
  rangePacket->pid[0] = sim.anchor0Pid++;
  uint32_t txTime = (uint32_t)sim.now;
  memcpy(rangePacket->timestamps[0], &txTime, TS_TX_SIZE);

  sim.rxPacketLength = MAC802154_HEADER_LENGTH + sizeof(rangePacket_t);
  sim.rxTime = sim.now + CLOCK_OFFSET;

  sendEvent(eventPacketReceived);
}

static void receiveServicePacket() {
  memset(&sim.rxPacket, 0, sizeof(sim.rxPacket));
  sim.rxPacket.sourceAddress[0] = 0xff;
  sim.rxPacket.payload[0] = SHORT_LPP;
  sim.rxPacket.payload[1] = LPP_SHORT_ANCHOR_POSITION;

  sim.rxPacketLength = MAC802154_HEADER_LENGTH + 2 + sizeof(struct lppShortAnchorPosition_s);
  sim.rxTime = sim.now + CLOCK_OFFSET;

  sendEvent(eventPacketReceived);
}

// Complete the pending radio operation and send the resulting events
static void step() {
  radioOp_t op = sim.pendingOp;
  TEST_ASSERT_NOT_EQUAL_MESSAGE(opNone, op.type, "No radio operation is pending");

  sim.now = pendingOpTime();
  sim.pendingOp.type = opNone;

  if (op.type == opTx) {
    TEST_ASSERT_TRUE_MESSAGE(sim.txCount < MAX_TX_COUNT, "Too many transmissions");
    sim.txSlots[sim.txCount++] = slotOfLocalTime(op.time);

    sendEvent(eventPacketSent);

    // Response window after the TX, unless a new radio operation was started
    if (sim.pendingOp.type == opNone) {
      if (sim.sendServicePacket) {
        sim.sendServicePacket = false;
        receiveServicePacket();
      } else {
        sendEvent(eventReceiveTimeout);
      }
    }
  } else if (!op.isDelayed) {
    // The anchor is waiting for sync and receives the next packet from anchor 0
    sim.syncListenCount++;
    receiveAnchor0Packet();
  } else if (slotOfLocalTime(op.time) == 0) {
    // The RX starts at the start of the slot, the packet from anchor 0 comes
    // later in the slot. A lost packet is gone when the RX times out.
    sim.now += TX_TIME_IN_SLOT;
    if (sim.slot0PacketsToDrop > 0) {
      sim.slot0PacketsToDrop--;
      sendEvent(eventReceiveTimeout);
    } else {
      receiveAnchor0Packet();
    }
  } else {
    // No other anchors are present
    sendEvent(eventReceiveTimeout);
  }
}

static void runFrames(int frames) {
  sim.runEnd += frames * TDMA_FRAME_LEN;
  while (pendingOpTime() < sim.runEnd) {
    step();
  }
}

static void boot(int anchorId) {
  memset(&ctx, 0, sizeof(ctx));
  memset(&sim, 0, sizeof(sim));
  sim.now = BOOT_TIME;
  sim.runEnd = BOOT_TIME;

  memset(&config, 0, sizeof(config));
  config.address[0] = anchorId;

  uwbTdoa2Algorithm.init(&config, &dev);
  sendEvent(eventTimeout);
}

// Boot and run until the anchor is synchronized and transmits in its own slot
static void bootAndSynchronize(int anchorId) {
  boot(anchorId);
  runFrames(FRAMES_TO_RUN);

  // Verify the fixture
  TEST_ASSERT_EQUAL_INT(synchronizedState, ctx.state);

  sim.txCount = 0;
  sim.syncListenCount = 0;
}

static void assertAllTxInSlot(int anchorId) {
  char message[40];

  snprintf(message, sizeof(message), "Anchor %d, no transmissions", anchorId);
  TEST_ASSERT_TRUE_MESSAGE(sim.txCount > 0, message);

  for (int i = 0; i < sim.txCount; i++) {
    snprintf(message, sizeof(message), "Anchor %d, transmission %d", anchorId, i);
    TEST_ASSERT_EQUAL_INT_MESSAGE(anchorId, sim.txSlots[i], message);
  }
}

static char* anchorMessage(int anchorId) {
  static char message[40];
  snprintf(message, sizeof(message), "Anchor %d", anchorId);
  return message;
}

// Tests -----------------------------------------------------------------------

void setUp(void) {
  setUpMocks();
}

void tearDown(void) {
}

void test_anchorOnlyTransmitsInOwnSlotAfterBoot() {
  for (int anchorId = FIRST_ANCHOR_ID; anchorId <= LAST_ANCHOR_ID; anchorId++) {
    // Fixture
    boot(anchorId);

    // Test
    runFrames(FRAMES_TO_RUN);

    // Assert
    assertAllTxInSlot(anchorId);
  }
}

void test_anchorSynchronizesOnceAfterBoot() {
  for (int anchorId = FIRST_ANCHOR_ID; anchorId <= LAST_ANCHOR_ID; anchorId++) {
    // Fixture
    boot(anchorId);

    // Test
    runFrames(FRAMES_TO_RUN);

    // Assert
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, sim.syncListenCount, anchorMessage(anchorId));
  }
}

void test_anchorOnlyTransmitsInOwnSlotAfterResyncOnMissedSlot0Packet() {
  for (int anchorId = FIRST_ANCHOR_ID; anchorId <= LAST_ANCHOR_ID; anchorId++) {
    // Fixture
    bootAndSynchronize(anchorId);

    // Test
    sim.slot0PacketsToDrop = 1;
    runFrames(FRAMES_TO_RUN);

    // Assert
    assertAllTxInSlot(anchorId);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, sim.syncListenCount, anchorMessage(anchorId));
  }
}

void test_anchorOnlyTransmitsInOwnSlotAfterResyncOnServicePacket() {
  for (int anchorId = FIRST_ANCHOR_ID; anchorId <= LAST_ANCHOR_ID; anchorId++) {
    // Fixture
    bootAndSynchronize(anchorId);

    // Test
    sim.sendServicePacket = true;
    runFrames(FRAMES_TO_RUN);

    // Assert
    assertAllTxInSlot(anchorId);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, sim.syncListenCount, anchorMessage(anchorId));
  }
}
