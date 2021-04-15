#include "usbd_cdc_if.h"
#include "usbcomm.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

USBD_HandleTypeDef  *hUsbDevice_0;

extern USBD_HandleTypeDef hUsbDeviceFS;

extern QueueHandle_t rxq;

static int isL4 = L4_ENABLED;

uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  if (isL4) {
    return CDC_Transmit_FS_l4();
  } else {
    return CDC_Transmit_FS_f0();
  }
}

bool CDC_IsUsbInit(void)
{
  if (isL4) {
    return CDC_IsUsbInit_l4();
  } else {
    return CDC_IsUsbInit_f0();
  }
}

int CDC_Write(char* buffer, int len)
{
   if (isL4) {
    return CDC_Write_l4();
  } else {
    return CDC_Write_f0();
  }
}

int CDC_Read(char *buffer, int len) {
     if (isL4) {
    return CDC_Read_l4();
  } else {
    return CDC_Read_f0();
  }
}
