#include <string.h>

#include "unity.h"
#include "service_dispatch.h"
#include "service_protocol.h"

static serviceConfigPayload_t readConfigSnapshot;
static bool readConfigResult;
static bool writePositionResult;
static bool writeRadioModeResult;
static bool writeTxPowerResult;
static bool readConfigCalled;
static bool writePositionCalled;
static bool writeRadioModeCalled;
static bool writeTxPowerCalled;
static float writtenPosition[3];
static uint8_t writtenRadioMode;
static serviceSetTxPowerPayload_t writtenTxPower;

static bool readConfig(serviceConfigPayload_t *snapshot) {
  readConfigCalled = true;
  *snapshot = readConfigSnapshot;
  return readConfigResult;
}

static bool writePosition(const float position[3]) {
  writePositionCalled = true;
  memcpy(writtenPosition, position, sizeof(writtenPosition));
  return writePositionResult;
}

static bool writeRadioMode(uint8_t radioMode) {
  writeRadioModeCalled = true;
  writtenRadioMode = radioMode;
  return writeRadioModeResult;
}

static bool writeTxPower(const serviceSetTxPowerPayload_t *payload) {
  writeTxPowerCalled = true;
  writtenTxPower = *payload;
  return writeTxPowerResult;
}

static serviceDispatchConfig_t defaultConfig(void) {
  serviceDispatchConfig_t config = {
    .nodeId = 7,
    .readConfig = readConfig,
    .writePosition = writePosition,
    .writeRadioMode = writeRadioMode,
    .writeTxPower = writeTxPower,
  };

  return config;
}

static size_t makeRequest(uint8_t *frame, uint8_t targetId, uint8_t commandId, const void *payload, uint8_t payloadLength) {
  serviceRequestHeader_t request = {
    .type = SERVICE_PACKET_REQUEST,
    .protocolVersion = SERVICE_PROTOCOL_VERSION,
    .controllerId = 0xfe,
    .targetId = targetId,
    .requestId = 0x1234,
    .commandId = commandId,
    .payloadLength = payloadLength,
  };

  memcpy(frame, &request, sizeof(request));
  if (payloadLength > 0) {
    memcpy(frame + sizeof(request), payload, payloadLength);
  }

  return sizeof(request) + payloadLength;
}

static serviceReplyHeader_t replyHeader(const uint8_t *frame) {
  serviceReplyHeader_t reply;
  memcpy(&reply, frame, sizeof(reply));
  return reply;
}

void setUp(void) {
  memset(&readConfigSnapshot, 0, sizeof(readConfigSnapshot));
  memset(writtenPosition, 0, sizeof(writtenPosition));
  memset(&writtenTxPower, 0, sizeof(writtenTxPower));
  readConfigResult = true;
  writePositionResult = true;
  writeRadioModeResult = true;
  writeTxPowerResult = true;
  readConfigCalled = false;
  writePositionCalled = false;
  writeRadioModeCalled = false;
  writeTxPowerCalled = false;
  writtenRadioMode = 0;
}

void tearDown(void) {}

void test_serviceDispatchShouldReturnNotTargetForDifferentTargetId(void) {
  serviceDispatchConfig_t config = defaultConfig();
  uint8_t requestFrame[SERVICE_REQUEST_HEADER_SIZE];
  uint8_t replyFrame[SERVICE_REPLY_HEADER_SIZE];
  bool resetRequired = true;

  size_t requestLength = makeRequest(requestFrame, 9, SERVICE_COMMAND_GET_CONFIG, NULL, 0);
  size_t replyLength = serviceDispatchHandleRequest(&config, requestFrame, requestLength, replyFrame, sizeof(replyFrame), &resetRequired);
  serviceReplyHeader_t reply = replyHeader(replyFrame);

  TEST_ASSERT_EQUAL_UINT(sizeof(serviceReplyHeader_t), replyLength);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_STATUS_NOT_TARGET, reply.status);
  TEST_ASSERT_EQUAL_UINT8(0, reply.flags);
  TEST_ASSERT_EQUAL_UINT8(0, reply.payloadLength);
  TEST_ASSERT_FALSE(resetRequired);
  TEST_ASSERT_FALSE(readConfigCalled);
}

void test_serviceDispatchShouldRejectNonRequestFrameWithoutCallback(void) {
  serviceDispatchConfig_t config = defaultConfig();
  uint8_t requestFrame[SERVICE_REQUEST_HEADER_SIZE];
  uint8_t replyFrame[SERVICE_REPLY_HEADER_SIZE + sizeof(serviceConfigPayload_t)];
  bool resetRequired = true;

  size_t requestLength = makeRequest(requestFrame, 7, SERVICE_COMMAND_GET_CONFIG, NULL, 0);
  requestFrame[0] = SERVICE_PACKET_REPLY;

  size_t replyLength = serviceDispatchHandleRequest(&config, requestFrame, requestLength, replyFrame, sizeof(replyFrame), &resetRequired);
  serviceReplyHeader_t reply = replyHeader(replyFrame);

  TEST_ASSERT_EQUAL_UINT(sizeof(serviceReplyHeader_t), replyLength);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_STATUS_BAD_VALUE, reply.status);
  TEST_ASSERT_EQUAL_UINT8(0, reply.flags);
  TEST_ASSERT_EQUAL_UINT8(0, reply.payloadLength);
  TEST_ASSERT_FALSE(resetRequired);
  TEST_ASSERT_FALSE(readConfigCalled);
}

void test_serviceDispatchShouldReturnConfigPayloadForGetConfig(void) {
  serviceDispatchConfig_t config = defaultConfig();
  uint8_t requestFrame[SERVICE_REQUEST_HEADER_SIZE];
  uint8_t replyFrame[SERVICE_REPLY_HEADER_SIZE + sizeof(serviceConfigPayload_t)];
  serviceConfigPayload_t replyPayload;
  bool resetRequired = true;

  readConfigSnapshot.nodeId = 7;
  readConfigSnapshot.mode = 2;
  readConfigSnapshot.positionEnabled = 1;
  readConfigSnapshot.position[0] = 1.5f;
  readConfigSnapshot.position[1] = -2.25f;
  readConfigSnapshot.position[2] = 3.75f;
  readConfigSnapshot.smartPower = 1;
  readConfigSnapshot.forceTxPower = 0;
  readConfigSnapshot.txPower = 0x12345678;
  readConfigSnapshot.lowBitrate = 1;
  readConfigSnapshot.longPreamble = 0;
  readConfigSnapshot.serviceProtocolVersion = SERVICE_PROTOCOL_VERSION;

  size_t requestLength = makeRequest(requestFrame, 7, SERVICE_COMMAND_GET_CONFIG, NULL, 0);
  size_t replyLength = serviceDispatchHandleRequest(&config, requestFrame, requestLength, replyFrame, sizeof(replyFrame), &resetRequired);
  serviceReplyHeader_t reply = replyHeader(replyFrame);
  memcpy(&replyPayload, replyFrame + sizeof(reply), sizeof(replyPayload));

  TEST_ASSERT_EQUAL_UINT(sizeof(serviceReplyHeader_t) + sizeof(serviceConfigPayload_t), replyLength);
  TEST_ASSERT_TRUE(readConfigCalled);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_STATUS_OK, reply.status);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_REPLY_FLAG_ACTIVE, reply.flags);
  TEST_ASSERT_EQUAL_UINT8(sizeof(serviceConfigPayload_t), reply.payloadLength);
  TEST_ASSERT_EQUAL_MEMORY(&readConfigSnapshot, &replyPayload, sizeof(replyPayload));
  TEST_ASSERT_FALSE(resetRequired);
}

void test_serviceDispatchShouldWritePositionAndReturnActiveWithoutReset(void) {
  serviceDispatchConfig_t config = defaultConfig();
  const float expectedPosition[3] = { 4.0f, 5.5f, -6.25f };
  serviceSetPositionPayload_t payload = {
    .position = { 4.0f, 5.5f, -6.25f },
  };
  uint8_t requestFrame[SERVICE_REQUEST_HEADER_SIZE + sizeof(payload)];
  uint8_t replyFrame[SERVICE_REPLY_HEADER_SIZE];
  bool resetRequired = true;

  size_t requestLength = makeRequest(requestFrame, 7, SERVICE_COMMAND_SET_POSITION, &payload, sizeof(payload));
  size_t replyLength = serviceDispatchHandleRequest(&config, requestFrame, requestLength, replyFrame, sizeof(replyFrame), &resetRequired);
  serviceReplyHeader_t reply = replyHeader(replyFrame);

  TEST_ASSERT_EQUAL_UINT(sizeof(serviceReplyHeader_t), replyLength);
  TEST_ASSERT_TRUE(writePositionCalled);
  TEST_ASSERT_EQUAL_FLOAT_ARRAY(expectedPosition, writtenPosition, 3);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_STATUS_OK, reply.status);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_REPLY_FLAG_ACTIVE, reply.flags);
  TEST_ASSERT_EQUAL_UINT8(0, reply.payloadLength);
  TEST_ASSERT_FALSE(resetRequired);
}

void test_serviceDispatchShouldWriteRadioModeAndRequireReset(void) {
  serviceDispatchConfig_t config = defaultConfig();
  serviceSetRadioModePayload_t payload = {
    .radioMode = 3,
  };
  uint8_t requestFrame[SERVICE_REQUEST_HEADER_SIZE + sizeof(payload)];
  uint8_t replyFrame[SERVICE_REPLY_HEADER_SIZE];
  bool resetRequired = false;

  size_t requestLength = makeRequest(requestFrame, 7, SERVICE_COMMAND_SET_RADIO_MODE, &payload, sizeof(payload));
  size_t replyLength = serviceDispatchHandleRequest(&config, requestFrame, requestLength, replyFrame, sizeof(replyFrame), &resetRequired);
  serviceReplyHeader_t reply = replyHeader(replyFrame);

  TEST_ASSERT_EQUAL_UINT(sizeof(serviceReplyHeader_t), replyLength);
  TEST_ASSERT_TRUE(writeRadioModeCalled);
  TEST_ASSERT_EQUAL_UINT8(3, writtenRadioMode);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_STATUS_OK, reply.status);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_REPLY_FLAG_WRITTEN | SERVICE_REPLY_FLAG_RESET_PENDING, reply.flags);
  TEST_ASSERT_EQUAL_UINT8(0, reply.payloadLength);
  TEST_ASSERT_TRUE(resetRequired);
}

void test_serviceDispatchShouldWriteTxPowerAndRequireReset(void) {
  serviceDispatchConfig_t config = defaultConfig();
  serviceSetTxPowerPayload_t payload = {
    .enableSmartPower = 1,
    .forceTxPower = 1,
    .txPower = 0x4a5b6c7d,
  };
  uint8_t requestFrame[SERVICE_REQUEST_HEADER_SIZE + sizeof(payload)];
  uint8_t replyFrame[SERVICE_REPLY_HEADER_SIZE];
  bool resetRequired = false;

  size_t requestLength = makeRequest(requestFrame, 7, SERVICE_COMMAND_SET_TX_POWER, &payload, sizeof(payload));
  size_t replyLength = serviceDispatchHandleRequest(&config, requestFrame, requestLength, replyFrame, sizeof(replyFrame), &resetRequired);
  serviceReplyHeader_t reply = replyHeader(replyFrame);

  TEST_ASSERT_EQUAL_UINT(sizeof(serviceReplyHeader_t), replyLength);
  TEST_ASSERT_TRUE(writeTxPowerCalled);
  TEST_ASSERT_EQUAL_UINT8(payload.enableSmartPower, writtenTxPower.enableSmartPower);
  TEST_ASSERT_EQUAL_UINT8(payload.forceTxPower, writtenTxPower.forceTxPower);
  TEST_ASSERT_EQUAL_UINT32(payload.txPower, writtenTxPower.txPower);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_STATUS_OK, reply.status);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_REPLY_FLAG_WRITTEN | SERVICE_REPLY_FLAG_RESET_PENDING, reply.flags);
  TEST_ASSERT_EQUAL_UINT8(0, reply.payloadLength);
  TEST_ASSERT_TRUE(resetRequired);
}

void test_serviceDispatchShouldRejectInvalidRadioModeWithoutCallback(void) {
  serviceDispatchConfig_t config = defaultConfig();
  serviceSetRadioModePayload_t payload = {
    .radioMode = 4,
  };
  uint8_t requestFrame[SERVICE_REQUEST_HEADER_SIZE + sizeof(payload)];
  uint8_t replyFrame[SERVICE_REPLY_HEADER_SIZE];
  bool resetRequired = true;

  size_t requestLength = makeRequest(requestFrame, 7, SERVICE_COMMAND_SET_RADIO_MODE, &payload, sizeof(payload));
  size_t replyLength = serviceDispatchHandleRequest(&config, requestFrame, requestLength, replyFrame, sizeof(replyFrame), &resetRequired);
  serviceReplyHeader_t reply = replyHeader(replyFrame);

  TEST_ASSERT_EQUAL_UINT(sizeof(serviceReplyHeader_t), replyLength);
  TEST_ASSERT_FALSE(writeRadioModeCalled);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_STATUS_BAD_VALUE, reply.status);
  TEST_ASSERT_EQUAL_UINT8(0, reply.flags);
  TEST_ASSERT_EQUAL_UINT8(0, reply.payloadLength);
  TEST_ASSERT_FALSE(resetRequired);
}
