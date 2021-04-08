#ifndef __USB_COMM_H__
#define __USB_COMM_H

#include <stdbool.h>

#define RX_Q_SIZE 10
#define TX_Q_SIZE 512
#define Q_ITEM_SIZE (sizeof(uint8_t))

bool usbcommInit(void);

int usbcommRead(char* buffer, size_t len);

void usbcommWrite(char const *data, int len);

void usbcommSetSystemStarted(bool init);

void usbcommPrintWelcomeMessage();

#endif // __USB_COMM_H__
