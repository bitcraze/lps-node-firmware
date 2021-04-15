#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"


USBD_HandleTypeDef hUsbDeviceFS;
extern USBD_DescriptorsTypeDef FS_Desc;

static int isL4 = L4_ENABLED;

void MX_USB_DEVICE_Init(void)
{
  if (isL4) {
    MX_USB_DEVICE_Init_l4();
  } else {
    MX_USB_DEVICE_Init_f0();
  }
}
