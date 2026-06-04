#include "unity.h"
#include "service_protocol.h"

void setUp(void) {}
void tearDown(void) {}

void test_serviceRequestHeaderShouldHaveStablePackedSize(void) {
  TEST_ASSERT_EQUAL_UINT(sizeof(serviceRequestHeader_t), SERVICE_REQUEST_HEADER_SIZE);
}

void test_serviceReplyHeaderShouldHaveStablePackedSize(void) {
  TEST_ASSERT_EQUAL_UINT(sizeof(serviceReplyHeader_t), SERVICE_REPLY_HEADER_SIZE);
}

void test_serviceProtocolShouldRejectWrongPayloadLength(void) {
  serviceRequestHeader_t request = {
    .type = SERVICE_PACKET_REQUEST,
    .protocolVersion = SERVICE_PROTOCOL_VERSION,
    .controllerId = 0xfe,
    .targetId = 3,
    .requestId = 42,
    .commandId = SERVICE_COMMAND_SET_POSITION,
    .payloadLength = 3,
  };

  TEST_ASSERT_FALSE(serviceProtocolIsRequestLengthValid(&request, sizeof(request) + request.payloadLength));
}

void test_serviceProtocolShouldAcceptPositionPayloadLength(void) {
  serviceRequestHeader_t request = {
    .type = SERVICE_PACKET_REQUEST,
    .protocolVersion = SERVICE_PROTOCOL_VERSION,
    .controllerId = 0xfe,
    .targetId = 3,
    .requestId = 42,
    .commandId = SERVICE_COMMAND_SET_POSITION,
    .payloadLength = sizeof(serviceSetPositionPayload_t),
  };

  TEST_ASSERT_TRUE(serviceProtocolIsRequestLengthValid(&request, sizeof(request) + request.payloadLength));
}

void test_serviceProtocolShouldInitializeReplyForRequest(void) {
  serviceRequestHeader_t request = {
    .type = SERVICE_PACKET_REQUEST,
    .protocolVersion = SERVICE_PROTOCOL_VERSION,
    .controllerId = 0xfe,
    .targetId = 3,
    .requestId = 42,
    .commandId = SERVICE_COMMAND_GET_CONFIG,
    .payloadLength = 0,
  };
  serviceReplyHeader_t reply;

  serviceProtocolInitReply(&reply, &request, SERVICE_STATUS_OK, SERVICE_REPLY_FLAG_ACTIVE, 0);

  TEST_ASSERT_EQUAL_UINT8(SERVICE_PACKET_REPLY, reply.type);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_PROTOCOL_VERSION, reply.protocolVersion);
  TEST_ASSERT_EQUAL_UINT8(3, reply.targetId);
  TEST_ASSERT_EQUAL_UINT8(0xfe, reply.controllerId);
  TEST_ASSERT_EQUAL_UINT16(42, reply.requestId);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_STATUS_OK, reply.status);
  TEST_ASSERT_EQUAL_UINT8(SERVICE_REPLY_FLAG_ACTIVE, reply.flags);
  TEST_ASSERT_EQUAL_UINT8(0, reply.payloadLength);
}
