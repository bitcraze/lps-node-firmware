#include "service_protocol.h"

static bool expectedPayloadLength(uint8_t commandId, uint8_t *payloadLength) {
  switch (commandId) {
    case SERVICE_COMMAND_GET_CONFIG:
      *payloadLength = 0;
      return true;
    case SERVICE_COMMAND_SET_POSITION:
      *payloadLength = sizeof(serviceSetPositionPayload_t);
      return true;
    case SERVICE_COMMAND_SET_RADIO_MODE:
      *payloadLength = sizeof(serviceSetRadioModePayload_t);
      return true;
    case SERVICE_COMMAND_SET_TX_POWER:
      *payloadLength = sizeof(serviceSetTxPowerPayload_t);
      return true;
    case SERVICE_COMMAND_SET_UWB_CHANNEL:
      *payloadLength = sizeof(serviceSetUwbChannelPayload_t);
      return true;
    default:
      return false;
  }
}

bool serviceProtocolIsRequestLengthValid(const serviceRequestHeader_t *request, size_t frameLength) {
  uint8_t payloadLength;

  if (request == NULL) {
    return false;
  }

  if (!expectedPayloadLength(request->commandId, &payloadLength)) {
    return false;
  }

  if (request->payloadLength != payloadLength) {
    return false;
  }

  return frameLength == (size_t)SERVICE_REQUEST_HEADER_SIZE + payloadLength;
}

void serviceProtocolInitReply(serviceReplyHeader_t *reply, const serviceRequestHeader_t *request, uint8_t status, uint8_t flags, uint8_t payloadLength) {
  reply->type = SERVICE_PACKET_REPLY;
  reply->protocolVersion = SERVICE_PROTOCOL_VERSION;
  reply->targetId = request->targetId;
  reply->controllerId = request->controllerId;
  reply->requestId = request->requestId;
  reply->status = status;
  reply->flags = flags;
  reply->payloadLength = payloadLength;
}
