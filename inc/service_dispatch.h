#ifndef __SERVICE_DISPATCH_H__
#define __SERVICE_DISPATCH_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "service_protocol.h"

typedef struct {
  uint8_t nodeId;
  bool (*readConfig)(serviceConfigPayload_t *snapshot);
  bool (*writePosition)(const float position[3]);
  bool (*writeRadioMode)(uint8_t radioMode);
  bool (*writeTxPower)(const serviceSetTxPowerPayload_t *payload);
} serviceDispatchConfig_t;

size_t serviceDispatchHandleRequest(const serviceDispatchConfig_t *config, const uint8_t *requestFrame, size_t requestLength, uint8_t *replyFrame, size_t replyCapacity, bool *resetRequired);

#endif // __SERVICE_DISPATCH_H__
