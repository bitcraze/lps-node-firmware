// USB communication wrapper

#include <stdbool.h>
#include <stdint.h>

#include "usbd_conf.h"
#include "usbd_cdc_if.h"

#define INITBUFFER_LEN 1024

static char initBuffer[INITBUFFER_LEN];
static int initPtr = 0;

static bool isInit = false;

int usbcommRead(char* buffer, size_t len)
{
  return CDC_Read(buffer, len);
}

void usbcommWrite(char *data, int len)
{
  if (isInit) {
    if (USBD_IsSerialConnected()) {
      CDC_Write(data, len);
    }
  } else {
    for(int i=0; initPtr<INITBUFFER_LEN && i<len; i++,initPtr++) {
      initBuffer[initPtr] = data[i];
    }
  }
}

void usbcommSetSystemStarted(bool init)
{
  isInit = init;
}

void usbcommPrintWelcomeMessage()
{
  static bool wasConnected;

  if (isInit) {
    if (wasConnected != USBD_IsSerialConnected()) {
      if (USBD_IsSerialConnected()) {
        CDC_Write(initBuffer, initPtr);
      }
    }
    wasConnected = USBD_IsSerialConnected();
  }
}
