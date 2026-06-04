#include "service_controller.h"

#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "mac.h"
#include "service_protocol.h"
#include "uwb.h"

#define SERVICE_CONTROLLER_POLL_TIMEOUT_MS 10

static const uint8_t baseAddress[] = {0,0,0,0,0,0,0xcf,0xbc};

typedef struct {
  bool ready;
  uint8_t controllerId;
  uint16_t nextRequestId;

  SemaphoreHandle_t transactionMutex;
  SemaphoreHandle_t replySemaphore;

  volatile bool requestPending;
  volatile bool awaitingReply;
  uint16_t activeRequestId;
  uint8_t activeTargetId;

  packet_t requestPacket;
  uint8_t requestFrameLength;

  uint8_t replyPayload[SERVICE_CONTROLLER_MAX_FRAME_SIZE - SERVICE_REPLY_HEADER_SIZE];
  uint8_t replyPayloadLength;
  uint8_t replyStatus;
  uint8_t replyFlags;
  uint16_t replyRequestId;
} serviceControllerContext_t;

static serviceControllerContext_t ctx;

static void setupRx(dwDevice_t *dev)
{
  dwNewReceive(dev);
  dwSetDefaults(dev);
  dwStartReceive(dev);
}

static void setupTx(dwDevice_t *dev)
{
  ctx.requestPending = false;

  dwIdle(dev);
  dwNewTransmit(dev);
  dwSetDefaults(dev);
  dwSetData(dev, (uint8_t*)&ctx.requestPacket, MAC802154_HEADER_LENGTH + ctx.requestFrameLength);
  dwWaitForResponse(dev, true);
  dwStartTransmit(dev);
}

static void wakeUwbTask(void)
{
  TaskHandle_t uwbTask = xTaskGetHandle("uwb");

  if (uwbTask != NULL) {
    xTaskAbortDelay(uwbTask);
  }
}

static uint16_t allocateRequestId(void)
{
  uint16_t requestId = ctx.nextRequestId++;

  if (ctx.nextRequestId == 0) {
    ctx.nextRequestId = 1;
  }

  return requestId;
}

static bool buildRequestFrame(uint8_t targetId, uint16_t requestId, uint8_t commandId, const uint8_t *payload, uint8_t payloadLength)
{
  if (payloadLength > SERVICE_CONTROLLER_MAX_FRAME_SIZE - SERVICE_REQUEST_HEADER_SIZE) {
    return false;
  }

  if (payloadLength > 0 && payload == NULL) {
    return false;
  }

  MAC80215_PACKET_INIT(ctx.requestPacket, MAC802154_TYPE_DATA);
  ctx.requestPacket.pan = 0xbccf;
  memcpy(ctx.requestPacket.sourceAddress, baseAddress, sizeof(ctx.requestPacket.sourceAddress));
  ctx.requestPacket.sourceAddress[0] = ctx.controllerId;
  memcpy(ctx.requestPacket.destAddress, baseAddress, sizeof(ctx.requestPacket.destAddress));
  ctx.requestPacket.destAddress[0] = targetId;

  serviceRequestHeader_t *request = (serviceRequestHeader_t *)ctx.requestPacket.payload;
  request->type = SERVICE_PACKET_REQUEST;
  request->protocolVersion = SERVICE_PROTOCOL_VERSION;
  request->controllerId = ctx.controllerId;
  request->targetId = targetId;
  request->requestId = requestId;
  request->commandId = commandId;
  request->payloadLength = payloadLength;

  if (payloadLength > 0) {
    memcpy(&ctx.requestPacket.payload[SERVICE_REQUEST_HEADER_SIZE], payload, payloadLength);
  }

  ctx.requestFrameLength = SERVICE_REQUEST_HEADER_SIZE + payloadLength;
  return true;
}

static bool isMatchingReply(const packet_t *packet, int dataLength)
{
  if (!ctx.awaitingReply) {
    return false;
  }

  if (dataLength < MAC802154_HEADER_LENGTH + SERVICE_REPLY_HEADER_SIZE) {
    return false;
  }

  if (packet->destAddress[0] != ctx.controllerId) {
    return false;
  }

  if (packet->sourceAddress[0] != ctx.activeTargetId) {
    return false;
  }

  const serviceReplyHeader_t *reply = (const serviceReplyHeader_t *)packet->payload;

  if (reply->type != SERVICE_PACKET_REPLY) {
    return false;
  }

  if (reply->protocolVersion != SERVICE_PROTOCOL_VERSION) {
    return false;
  }

  if (reply->controllerId != ctx.controllerId) {
    return false;
  }

  if (reply->targetId != ctx.activeTargetId) {
    return false;
  }

  if (reply->requestId != ctx.activeRequestId) {
    return false;
  }

  if (reply->payloadLength > SERVICE_CONTROLLER_MAX_FRAME_SIZE - SERVICE_REPLY_HEADER_SIZE) {
    return false;
  }

  return dataLength == MAC802154_HEADER_LENGTH + SERVICE_REPLY_HEADER_SIZE + reply->payloadLength;
}

static void handleReply(const packet_t *packet)
{
  const serviceReplyHeader_t *reply = (const serviceReplyHeader_t *)packet->payload;

  ctx.replyPayloadLength = reply->payloadLength;
  ctx.replyStatus = reply->status;
  ctx.replyFlags = reply->flags;
  ctx.replyRequestId = reply->requestId;

  if (reply->payloadLength > 0) {
    memcpy(ctx.replyPayload, &packet->payload[SERVICE_REPLY_HEADER_SIZE], reply->payloadLength);
  }

  ctx.awaitingReply = false;
  ctx.requestPending = false;
  xSemaphoreGive(ctx.replySemaphore);
}

static void handleRxPacket(dwDevice_t *dev)
{
  static packet_t rxPacket;
  int dataLength = dwGetDataLength(dev);

  if (dataLength <= 0 || dataLength > MAC802154_HEADER_LENGTH + SERVICE_CONTROLLER_MAX_FRAME_SIZE) {
    return;
  }

  rxPacket.payload[0] = 0;
  dwGetData(dev, (uint8_t*)&rxPacket, dataLength);

  if (isMatchingReply(&rxPacket, dataLength)) {
    handleReply(&rxPacket);
  }
}

bool serviceControllerIsReady(void)
{
  return ctx.ready;
}

serviceControllerResult_t serviceControllerTransaction(uint8_t targetId, uint8_t commandId, const uint8_t *payload, uint8_t payloadLength, uint8_t *replyPayload, uint8_t *replyPayloadLength, uint8_t *replyStatus, uint8_t *replyFlags, uint16_t *requestId)
{
  serviceControllerResult_t result = serviceControllerResultTimeout;
  uint8_t replyPayloadCapacity = 0;

  if (!ctx.ready) {
    return serviceControllerResultWrongMode;
  }

  if (replyPayload != NULL && replyPayloadLength == NULL) {
    return serviceControllerResultBusy;
  }

  if (replyPayload != NULL && replyPayloadLength != NULL) {
    replyPayloadCapacity = *replyPayloadLength;
  }

  if (xSemaphoreTake(ctx.transactionMutex, 0) != pdTRUE) {
    return serviceControllerResultBusy;
  }

  uint16_t activeRequestId = allocateRequestId();

  if (requestId != NULL) {
    *requestId = activeRequestId;
  }

  if (!buildRequestFrame(targetId, activeRequestId, commandId, payload, payloadLength)) {
    xSemaphoreGive(ctx.transactionMutex);
    return serviceControllerResultBusy;
  }

  ctx.activeRequestId = activeRequestId;
  ctx.activeTargetId = targetId;

  for (int attempt = 0; attempt < SERVICE_CONTROLLER_ATTEMPTS; attempt++) {
    xSemaphoreTake(ctx.replySemaphore, 0);

    ctx.awaitingReply = true;
    ctx.requestPending = true;
    wakeUwbTask();

    if (xSemaphoreTake(ctx.replySemaphore, pdMS_TO_TICKS(SERVICE_CONTROLLER_TIMEOUT_MS)) == pdTRUE) {
      if (replyPayloadLength != NULL) {
        *replyPayloadLength = ctx.replyPayloadLength;
      }

      if (replyPayload != NULL && replyPayloadLength != NULL && ctx.replyPayloadLength > replyPayloadCapacity) {
        // No dedicated result exists for caller buffer overflow; report Busy
        // and leave the caller buffer untouched.
        result = serviceControllerResultBusy;
        break;
      }

      if (replyPayload != NULL && ctx.replyPayloadLength > 0) {
        memcpy(replyPayload, ctx.replyPayload, ctx.replyPayloadLength);
      }
      if (replyStatus != NULL) {
        *replyStatus = ctx.replyStatus;
      }
      if (replyFlags != NULL) {
        *replyFlags = ctx.replyFlags;
      }
      if (requestId != NULL) {
        *requestId = ctx.replyRequestId;
      }

      result = serviceControllerResultOk;
      break;
    }

    ctx.awaitingReply = false;
    ctx.requestPending = false;
  }

  xSemaphoreGive(ctx.transactionMutex);

  return result;
}

static void serviceControllerInit(uwbConfig_t *config, dwDevice_t *dev)
{
  static StaticSemaphore_t transactionMutexBuffer;
  static StaticSemaphore_t replySemaphoreBuffer;

  memset(&ctx, 0, sizeof(ctx));

  ctx.controllerId = config->address[0];
  ctx.nextRequestId = 1;
  ctx.transactionMutex = xSemaphoreCreateMutexStatic(&transactionMutexBuffer);
  ctx.replySemaphore = xSemaphoreCreateBinaryStatic(&replySemaphoreBuffer);
  ctx.ready = true;

  setupRx(dev);
}

static uint32_t serviceControllerOnEvent(dwDevice_t *dev, uwbEvent_t event)
{
  switch (event) {
    case eventPacketReceived:
      handleRxPacket(dev);
      break;
    case eventPacketSent:
      setupRx(dev);
      return SERVICE_CONTROLLER_POLL_TIMEOUT_MS;
    case eventTimeout:
    case eventReceiveTimeout:
    case eventReceiveFailed:
      break;
    default:
      configASSERT(false);
      break;
  }

  if (ctx.requestPending) {
    setupTx(dev);
  } else {
    setupRx(dev);
  }

  return SERVICE_CONTROLLER_POLL_TIMEOUT_MS;
}

uwbAlgorithm_t uwbServiceControllerAlgorithm = {
  .init = serviceControllerInit,
  .onEvent = serviceControllerOnEvent,
};
