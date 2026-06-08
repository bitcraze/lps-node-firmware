#include "service_dispatch.h"

#include <string.h>

static bool isKnownCommand(uint8_t commandId) {
  switch (commandId) {
    case SERVICE_COMMAND_GET_CONFIG:
    case SERVICE_COMMAND_SET_POSITION:
    case SERVICE_COMMAND_SET_RADIO_MODE:
    case SERVICE_COMMAND_SET_TX_POWER:
    case SERVICE_COMMAND_SET_UWB_CHANNEL:
      return true;
    default:
      return false;
  }
}

static size_t writeReplyHeader(uint8_t *replyFrame, const serviceRequestHeader_t *request, uint8_t status, uint8_t flags, uint8_t payloadLength) {
  serviceReplyHeader_t reply;

  serviceProtocolInitReply(&reply, request, status, flags, payloadLength);
  memcpy(replyFrame, &reply, sizeof(reply));

  return sizeof(reply);
}

static size_t writeHeaderOnlyReply(uint8_t *replyFrame, const serviceRequestHeader_t *request, uint8_t status) {
  return writeReplyHeader(replyFrame, request, status, 0, 0);
}

static bool hasReplyCapacity(size_t replyCapacity, size_t payloadLength) {
  return replyCapacity >= sizeof(serviceReplyHeader_t) + payloadLength;
}

static bool isValidUwbChannel(uint8_t channel) {
  switch (channel) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 7:
      return true;
    default:
      return false;
  }
}

size_t serviceDispatchHandleRequest(const serviceDispatchConfig_t *config, const uint8_t *requestFrame, size_t requestLength, uint8_t *replyFrame, size_t replyCapacity, bool *resetRequired) {
  serviceRequestHeader_t request;

  if (resetRequired != NULL) {
    *resetRequired = false;
  }

  if (config == NULL || requestFrame == NULL || replyFrame == NULL) {
    return 0;
  }

  if (requestLength < sizeof(serviceRequestHeader_t) || replyCapacity < sizeof(serviceReplyHeader_t)) {
    return 0;
  }

  memcpy(&request, requestFrame, sizeof(request));

  if (request.type != SERVICE_PACKET_REQUEST) {
    return writeHeaderOnlyReply(replyFrame, &request, SERVICE_STATUS_BAD_VALUE);
  }

  if (request.targetId != config->nodeId) {
    return writeHeaderOnlyReply(replyFrame, &request, SERVICE_STATUS_NOT_TARGET);
  }

  if (request.protocolVersion != SERVICE_PROTOCOL_VERSION) {
    return writeHeaderOnlyReply(replyFrame, &request, SERVICE_STATUS_UNSUPPORTED_VERSION);
  }

  if (!isKnownCommand(request.commandId)) {
    return writeHeaderOnlyReply(replyFrame, &request, SERVICE_STATUS_UNKNOWN_COMMAND);
  }

  if (!serviceProtocolIsRequestLengthValid(&request, requestLength)) {
    return writeHeaderOnlyReply(replyFrame, &request, SERVICE_STATUS_BAD_LENGTH);
  }

  switch (request.commandId) {
    case SERVICE_COMMAND_GET_CONFIG: {
      serviceConfigPayload_t snapshot = { 0 };

      if (!hasReplyCapacity(replyCapacity, sizeof(snapshot))) {
        return 0;
      }

      if (config->readConfig == NULL || !config->readConfig(&snapshot)) {
        return writeHeaderOnlyReply(replyFrame, &request, SERVICE_STATUS_WRITE_FAILED);
      }

      writeReplyHeader(replyFrame, &request, SERVICE_STATUS_OK, SERVICE_REPLY_FLAG_ACTIVE, sizeof(snapshot));
      memcpy(replyFrame + sizeof(serviceReplyHeader_t), &snapshot, sizeof(snapshot));
      return sizeof(serviceReplyHeader_t) + sizeof(snapshot);
    }

    case SERVICE_COMMAND_SET_POSITION: {
      float position[3];

      memcpy(position, requestFrame + sizeof(request), sizeof(position));

      if (config->writePosition == NULL || !config->writePosition(position)) {
        return writeHeaderOnlyReply(replyFrame, &request, SERVICE_STATUS_WRITE_FAILED);
      }

      return writeReplyHeader(replyFrame, &request, SERVICE_STATUS_OK, SERVICE_REPLY_FLAG_ACTIVE, 0);
    }

    case SERVICE_COMMAND_SET_RADIO_MODE: {
      serviceSetRadioModePayload_t payload;

      memcpy(&payload, requestFrame + sizeof(request), sizeof(payload));

      if (payload.radioMode > 3) {
        return writeHeaderOnlyReply(replyFrame, &request, SERVICE_STATUS_BAD_VALUE);
      }

      if (config->writeRadioMode == NULL || !config->writeRadioMode(payload.radioMode)) {
        return writeHeaderOnlyReply(replyFrame, &request, SERVICE_STATUS_WRITE_FAILED);
      }

      if (resetRequired != NULL) {
        *resetRequired = true;
      }

      return writeReplyHeader(replyFrame, &request, SERVICE_STATUS_OK, SERVICE_REPLY_FLAG_WRITTEN | SERVICE_REPLY_FLAG_RESET_PENDING, 0);
    }

    case SERVICE_COMMAND_SET_TX_POWER: {
      serviceSetTxPowerPayload_t payload;

      memcpy(&payload, requestFrame + sizeof(request), sizeof(payload));

      if (config->writeTxPower == NULL || !config->writeTxPower(&payload)) {
        return writeHeaderOnlyReply(replyFrame, &request, SERVICE_STATUS_WRITE_FAILED);
      }

      if (resetRequired != NULL) {
        *resetRequired = true;
      }

      return writeReplyHeader(replyFrame, &request, SERVICE_STATUS_OK, SERVICE_REPLY_FLAG_WRITTEN | SERVICE_REPLY_FLAG_RESET_PENDING, 0);
    }

    case SERVICE_COMMAND_SET_UWB_CHANNEL: {
      serviceSetUwbChannelPayload_t payload;

      memcpy(&payload, requestFrame + sizeof(request), sizeof(payload));

      if (!isValidUwbChannel(payload.channel)) {
        return writeHeaderOnlyReply(replyFrame, &request, SERVICE_STATUS_BAD_VALUE);
      }

      if (config->writeUwbChannel == NULL || !config->writeUwbChannel(payload.channel)) {
        return writeHeaderOnlyReply(replyFrame, &request, SERVICE_STATUS_WRITE_FAILED);
      }

      if (resetRequired != NULL) {
        *resetRequired = true;
      }

      return writeReplyHeader(replyFrame, &request, SERVICE_STATUS_OK, SERVICE_REPLY_FLAG_WRITTEN | SERVICE_REPLY_FLAG_RESET_PENDING, 0);
    }

    default:
      return writeHeaderOnlyReply(replyFrame, &request, SERVICE_STATUS_UNKNOWN_COMMAND);
  }
}
