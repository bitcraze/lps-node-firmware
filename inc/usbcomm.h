#ifndef __USB_COMM_H__
#define __USB_COMM_H

#include <stdbool.h>

int usbcommRead(char* buffer, size_t len);

void usbcommWrite(char const *data, int len);

void usbcommSetSystemStarted(bool init);

void usbcommPrintWelcomeMessage();

#endif // __USB_COMM_H__
