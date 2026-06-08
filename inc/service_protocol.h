#ifndef __SERVICE_PROTOCOL_H__
#define __SERVICE_PROTOCOL_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SERVICE_PROTOCOL_VERSION 1

#define SERVICE_PACKET_REQUEST 0x40
#define SERVICE_PACKET_REPLY 0x41

#define SERVICE_COMMAND_GET_CONFIG 0x01
#define SERVICE_COMMAND_SET_POSITION 0x02
#define SERVICE_COMMAND_SET_RADIO_MODE 0x03
#define SERVICE_COMMAND_SET_TX_POWER 0x04
#define SERVICE_COMMAND_SET_UWB_CHANNEL 0x05

#define SERVICE_STATUS_OK 0x00
#define SERVICE_STATUS_UNSUPPORTED_VERSION 0x01
#define SERVICE_STATUS_UNKNOWN_COMMAND 0x02
#define SERVICE_STATUS_BAD_LENGTH 0x03
#define SERVICE_STATUS_BAD_VALUE 0x04
#define SERVICE_STATUS_NOT_TARGET 0x05
#define SERVICE_STATUS_WRITE_FAILED 0x06
#define SERVICE_STATUS_BUSY_RESET_PENDING 0x07

#define SERVICE_REPLY_FLAG_ACTIVE 0x01
#define SERVICE_REPLY_FLAG_WRITTEN 0x02
#define SERVICE_REPLY_FLAG_RESET_PENDING 0x04

#define SERVICE_REQUEST_HEADER_SIZE 8
#define SERVICE_REPLY_HEADER_SIZE 9

typedef struct {
  uint8_t type;
  uint8_t protocolVersion;
  uint8_t controllerId;
  uint8_t targetId;
  uint16_t requestId;
  uint8_t commandId;
  uint8_t payloadLength;
} __attribute__((packed)) serviceRequestHeader_t;

typedef struct {
  uint8_t type;
  uint8_t protocolVersion;
  uint8_t targetId;
  uint8_t controllerId;
  uint16_t requestId;
  uint8_t status;
  uint8_t flags;
  uint8_t payloadLength;
} __attribute__((packed)) serviceReplyHeader_t;

typedef struct {
  float position[3];
} __attribute__((packed)) serviceSetPositionPayload_t;

typedef struct {
  uint8_t radioMode;
} __attribute__((packed)) serviceSetRadioModePayload_t;

typedef struct {
  uint8_t enableSmartPower;
  uint8_t forceTxPower;
  uint32_t txPower;
} __attribute__((packed)) serviceSetTxPowerPayload_t;

typedef struct {
  uint8_t channel;
} __attribute__((packed)) serviceSetUwbChannelPayload_t;

typedef struct {
  uint8_t nodeId;
  uint8_t mode;
  uint8_t positionEnabled;
  float position[3];
  uint8_t smartPower;
  uint8_t forceTxPower;
  uint32_t txPower;
  uint8_t lowBitrate;
  uint8_t longPreamble;
  uint8_t channel;
  uint8_t serviceProtocolVersion;
} __attribute__((packed)) serviceConfigPayload_t;

bool serviceProtocolIsRequestLengthValid(const serviceRequestHeader_t *request, size_t frameLength);
void serviceProtocolInitReply(serviceReplyHeader_t *reply, const serviceRequestHeader_t *request, uint8_t status, uint8_t flags, uint8_t payloadLength);

#endif // __SERVICE_PROTOCOL_H__
