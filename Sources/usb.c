//---------------------------------------------------------------------------------------------
#include "usb.h"
#include <stdio.h>
#include <string.h>
#include "gsm.h"
#include "system.h"
#include "crc.h"
#include "usb_cdc.h"
//---------------------------------------------------------------------------------------------
// USB command implementation
#pragma pack(push, 1)
struct DEV_CONFIG
{
	char apn[30];
	char user[30];
	char pwd[30];
	short interval_geo;
	short interval_io;
	unsigned char led;
	char software_version[10];
};
#pragma pack(pop)
//---------------------------------------------------------------------------------------------
extern struct _AT91S_CDC pCDC;
extern SDeviceStatus Device;
extern void save_settings_in_flash(void);
//---------------------------------------------------------------------------------------------
//void AT91F_USB_Open(void)
//{
//	AT91C_BASE_CKGR->CKGR_PLLR |= AT91C_CKGR_USBDIV_1;
//	AT91C_BASE_PMC->PMC_SCER = AT91C_PMC_UDP;
//	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_UDP);
//	AT91F_CDC_Open(&pCDC, AT91C_BASE_UDP);
//}
//---------------------------------------------------------------------------------------------
void usb_process(void)
{
	unsigned int length = 0;
    char data[280];
	*AT91C_WDTC_WDCR = 0xA5000001;
	if (!pCDC.IsConfigured(&pCDC))
		return;
	length = pCDC.Read(&pCDC, data, 255);
	if (length > 0) // sending config console packet
	{
          printf("USB Data recived size=%d \r\n", length);
          pCDC.Write(&pCDC,"brake down", 10);
//		// -------------------------------------------------------
//		if (strncmp(data, "GET_SETTING", 11) == 0)
//		{
//			Device.setting.CRC = CRC((unsigned char*) &Device.setting,
//					sizeof(Device.setting) - 2);
//			pCDC.Write(&pCDC, (char*) &Device.setting, 255);
//                        return;
//		}
//		// -------------------------------------------------------
//		if (strncmp(data, "STORE", 5) == 0)
//		{
//			memcpy(&Device.setting, data + 5, sizeof(Device.setting));
//			if (Device.setting.CRC == CRC(
//					(unsigned char*) &Device.setting, sizeof(Device.setting)
//							- 2))
//			{
//				save_settings_in_flash();
//				pCDC.Write(&pCDC, "SAVED", 255);
//                                return;
//			}
//			else
//			{
//				pCDC.Write(&pCDC, "SAVEERORCRC", 255);
//                                return;
//			}
//		}
//		// -------------------------------------------------------
//		if (strncmp(data, "GET_IO", 6) == 0)
//		{
//			Device.io.CRC = CRC((unsigned char*) &Device.io,
//					sizeof(Device.io) - 2);
//			pCDC.Write(&pCDC, (char *) &Device.io, 255);
//                        return;
//		}
//		pCDC.Write(&pCDC, "ERRORE ", 7);
	}
}
//---------------------------------------------------------------------------------------------
