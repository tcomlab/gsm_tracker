#ifndef USB_H
#define USB_H

#include "main.h"
#include "usb_cdc.h"

#define MSG_SIZE 128

void AT91F_USB_Open(void);
void usb_process(void);

#endif
