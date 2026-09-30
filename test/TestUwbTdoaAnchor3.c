// Unit tests for uwb_tdoa_anchor3.c
//
// The source file is included directly to get access to the static functions
// and the static algorithm context. The radio (libdw1000), the FreeRTOS tick
// and the configuration are replaced by fakes in this file.

#include "unity.h"

#include "uwb_tdoa_anchor3.c"

// Radio clock: 499.2 MHz * 128 = 63.8976 GHz, i.e. 63897600 ticks per ms
#define RADIO_TICKS_PER_MS ((uint64_t)63897600)
// The 32 bit timestamps in the TDoA3 protocol wrap after 2^32 radio ticks (~67.2 ms)
#define TIMESTAMP_32_BIT_WRAP_MS (((uint64_t)1 << 32) / RADIO_TICKS_PER_MS)

#define ANCHOR_ID 1
#define REMOTE_ANCHOR_ID 2

// The clock of the remote anchor runs at the same rate but with an offset
#define REMOTE_CLOCK_OFFSET ((uint64_t)123456789)

#define REMOTE_TX_INTERVAL_MS 20

// Fakes ---------------------------------------------------------------------

static TickType_t fakeTick;
static uint64_t fakeRadioTime;

static packet_t fakeRxPacket;
static unsigned int fakeRxPacketLength;

static packet_t fakeTxPacket;
static unsigned int fakeTxPacketLength;

static uwbConfig_t fakeConfig;
static dwDevice_t fakeDev;

TickType_t xTaskGetTickCount(void) {
  return fakeTick;
}

void dwGetRawReceiveTimestamp(dwDevice_t* dev, dwTime_t* time) {
  (void)dev;
  time->full = fakeRadioTime;
}

void dwCorrectTimestamp(dwDevice_t* dev, dwTime_t* timestamp) {
  (void)dev;
  (void)timestamp;
}

void dwGetSystemTimestamp(dwDevice_t* dev, dwTime_t* time) {
  (void)dev;
  time->full = fakeRadioTime;
}

unsigned int dwGetDataLength(dwDevice_t* dev) {
  (void)dev;
  return fakeRxPacketLength;
}

void dwGetData(dwDevice_t* dev, uint8_t data[], unsigned int n) {
  (void)dev;
  memcpy(data, &fakeRxPacket, n);
}

void dwSetData(dwDevice_t* dev, uint8_t data[], unsigned int n) {
  (void)dev;
  memcpy(&fakeTxPacket, data, n);
  fakeTxPacketLength = n;
}

void dwIdle(dwDevice_t* dev) { (void)dev; }
void dwNewReceive(dwDevice_t* dev) { (void)dev; }
void dwStartReceive(dwDevice_t* dev) { (void)dev; }
void dwNewTransmit(dwDevice_t* dev) { (void)dev; }
void dwStartTransmit(dwDevice_t* dev) { (void)dev; }
void dwSetDefaults(dwDevice_t* dev) { (void)dev; }
void dwSetTxRxTime(dwDevice_t* dev, const dwTime_t futureTime) { (void)dev; (void)futureTime; }

void lppHandleShortPacket(char *data, size_t length) {
  (void)data;
  (void)length;
}

struct uwbConfig_s * uwbGetConfig() {
  return &fakeConfig;
}

// Helpers -------------------------------------------------------------------

static void setTime(uint32_t ms) {
  fakeTick = ms;
  fakeRadioTime = ms * RADIO_TICKS_PER_MS;
}

// Deliver a TDoA3 packet from the remote anchor, transmitted and received at time ms
static void receivePacketFromRemoteAnchor(uint32_t ms, uint8_t seqNr) {
  setTime(ms);

  memset(&fakeRxPacket, 0, sizeof(fakeRxPacket));
  fakeRxPacket.sourceAddress[0] = REMOTE_ANCHOR_ID;

  rangePacket3_t* rangePacket = (rangePacket3_t*)fakeRxPacket.payload;
  rangePacket->header.type = PACKET_TYPE_TDOA3;
  rangePacket->header.seq = seqNr;
  rangePacket->header.txTimeStamp = (uint32_t)(fakeRadioTime + REMOTE_CLOCK_OFFSET);
  rangePacket->header.remoteCount = 0;

  fakeRxPacketLength = MAC802154_HEADER_LENGTH + sizeof(rangePacketHeader3_t);

  uwbTdoa3Algorithm.onEvent(&fakeDev, eventPacketReceived);
}

// Receive packets from the remote anchor until the anchor under test considers
// the remote data good for transmission. Returns the time of the last reception.
static uint32_t receiveUntilRemoteDataIsGood() {
  uint32_t ms = 1;
  for (uint8_t seqNr = 0; seqNr < 4; seqNr++) {
    receivePacketFromRemoteAnchor(ms, seqNr);
    ms += REMOTE_TX_INTERVAL_MS;
  }
  ms -= REMOTE_TX_INTERVAL_MS;

  // Verify the fixture
  anchorContext_t* anchorCtx = getContext(REMOTE_ANCHOR_ID);
  TEST_ASSERT_NOT_NULL(anchorCtx);
  TEST_ASSERT_TRUE(anchorCtx->isDataGoodForTransmission);

  return ms;
}

// Make the anchor under test transmit a packet at time ms
static void transmitAt(uint32_t ms) {
  setTime(ms);
  ctx.nextTxTick = 0;
  fakeTxPacketLength = 0;

  uwbTdoa3Algorithm.onEvent(&fakeDev, eventTimeout);

  TEST_ASSERT_NOT_EQUAL(0, fakeTxPacketLength);
}

// Find remote anchor data in the transmitted packet
static bool findRemoteDataInTxPacket(uint8_t id, uint32_t* rxTimeStamp) {
  const rangePacket3_t* rangePacket = (rangePacket3_t*)fakeTxPacket.payload;
  const uint8_t* anchorDataPtr = &rangePacket->remoteAnchorData;

  for (uint8_t i = 0; i < rangePacket->header.remoteCount; i++) {
    const remoteAnchorDataFull_t* anchorData = (remoteAnchorDataFull_t*)anchorDataPtr;
    if (anchorData->id == id) {
      *rxTimeStamp = anchorData->rxTimeStamp;
      return true;
    }

    bool hasDistance = ((anchorData->seq & 0x80) != 0);
    if (hasDistance) {
      anchorDataPtr += sizeof(remoteAnchorDataFull_t);
    } else {
      anchorDataPtr += sizeof(remoteAnchorDataShort_t);
    }
  }

  return false;
}

// Tests ---------------------------------------------------------------------

void setUp(void) {
  memset(&ctx, 0, sizeof(ctx));
  memset(&fakeConfig, 0, sizeof(fakeConfig));
  fakeConfig.address[0] = ANCHOR_ID;
  fakeTxPacketLength = 0;
  setTime(0);

  uwbTdoa3Algorithm.init(&fakeConfig, &fakeDev);
}

void tearDown(void) {
}

void testThatRecentRemoteDataIsTransmitted() {
  // Fixture
  uint32_t lastRxMs = receiveUntilRemoteDataIsGood();
  uint32_t expectedRxTimeStamp = (uint32_t)(lastRxMs * RADIO_TICKS_PER_MS);

  // Test
  transmitAt(lastRxMs + 10);

  // Assert
  uint32_t actualRxTimeStamp = 0;
  TEST_ASSERT_TRUE(findRemoteDataInTxPacket(REMOTE_ANCHOR_ID, &actualRxTimeStamp));
  TEST_ASSERT_EQUAL_UINT32(expectedRxTimeStamp, actualRxTimeStamp);
}

void testThatRemoteDataOlderThan32BitTimestampWrapIsNotTransmitted() {
  // Fixture
  uint32_t lastRxMs = receiveUntilRemoteDataIsGood();

  // Test
  // The remote anchor goes silent (packet loss), our next transmission is more
  // than one 32 bit timestamp wrap later. The receiver can not tell that the
  // rx timestamp is from an earlier wrap.
  transmitAt(lastRxMs + 100);
  TEST_ASSERT_TRUE(100 > TIMESTAMP_32_BIT_WRAP_MS);

  // Assert
  uint32_t actualRxTimeStamp = 0;
  TEST_ASSERT_FALSE_MESSAGE(findRemoteDataInTxPacket(REMOTE_ANCHOR_ID, &actualRxTimeStamp),
    "Remote rx timestamp that is 100 ms old (> 67 ms) was transmitted");
}
