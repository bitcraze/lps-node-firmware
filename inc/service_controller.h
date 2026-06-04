#ifndef __SERVICE_CONTROLLER_H__
#define __SERVICE_CONTROLLER_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SERVICE_CONTROLLER_MAX_FRAME_SIZE 64
#define SERVICE_CONTROLLER_ATTEMPTS 3
#define SERVICE_CONTROLLER_TIMEOUT_MS 200

typedef enum {
  serviceControllerResultOk,
  serviceControllerResultWrongMode,
  serviceControllerResultBusy,
  serviceControllerResultTimeout,
} serviceControllerResult_t;

bool serviceControllerIsReady(void);
// If replyPayload is not NULL, replyPayloadLength is in/out: initialize it to
// the replyPayload buffer capacity before calling. On return it contains the
// actual reply payload length. If the reply does not fit, the payload is not
// copied and serviceControllerResultBusy is returned.
serviceControllerResult_t serviceControllerTransaction(uint8_t targetId, uint8_t commandId, const uint8_t *payload, uint8_t payloadLength, uint8_t *replyPayload, uint8_t *replyPayloadLength, uint8_t *replyStatus, uint8_t *replyFlags, uint16_t *requestId);

#endif // __SERVICE_CONTROLLER_H__
