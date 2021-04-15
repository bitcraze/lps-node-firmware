// USB communication wrapper

#include <stdbool.h>
#include <stdint.h>

// #include "usbd_conf.h"
// #include "usbd_cdc_if.h"
// #include "usbcomm.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define INITBUFFER_LEN 1024

// QueueHandle_t rxq;
// static StaticQueue_t rxqBuffer;
// static uint8_t rxqStorage[ RX_Q_SIZE * Q_ITEM_SIZE ];

// static char initBuffer[INITBUFFER_LEN];
// static int initPtr = 0;

static bool isInit = false;

bool usbcommInit(void)
{
  // rxq = xQueueCreateStatic(RX_Q_SIZE, Q_ITEM_SIZE, rxqStorage, &rxqBuffer);

  // if (rxq)
  // {
  //   isInit = true;
  // }

  return isInit;
}

int usbcommRead(char* buffer, size_t len)
{
  return 2; // CDC_Read(buffer, len);
}

void usbcommWrite(char const *data, int len)
{
  // if (CDC_IsUsbInit()) {
  //   if (USBD_IsSerialConnected()) {
  //     CDC_Write(data, len);
  //   }
  // } else {
  //   for(int i=0; initPtr<INITBUFFER_LEN && i<len; i++,initPtr++) {
  //     initBuffer[initPtr] = data[i];
  //   }
  // }
}

void usbcommSetSystemStarted(bool init)
{
  isInit = init;
}

void usbcommPrintWelcomeMessage()
{
  // static bool wasConnected;

  // if (isInit) {
  //   if (wasConnected != USBD_IsSerialConnected()) {
  //     if (USBD_IsSerialConnected()) {
  //       CDC_Write(initBuffer, initPtr);
  //     }
  //   }
  //   wasConnected = USBD_IsSerialConnected();
  // }
}
