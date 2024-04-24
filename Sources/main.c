/*
 * cGuardDevice v1.03.01
 */
//#include "timer.h"
#include "main.h"
#include "gps.h"
#include <intrinsics.h>
#include "gsm.h"
#include "usb.h"
#include "at45.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "at91flash.h"
//#include "usart.h"
//#include "aic.h"
#include "system.h"

SDeviceStatus Device;

#pragma location=0x11000
__no_init DeviceSetting SettingDeviceFlash;

extern void system_start(void);
extern void clock_init(void);
extern void irq_read_modem(void);
extern void gps_irq_read_data(void);
extern void timer0_c_irq_handler(void);
//------------------------------------------------------------------------------
/*
extern void Undefined_Handler(void);
extern void SWI_Handler(void);
extern void Prefetch_Handler(void);
extern void Abort_Handler(void);
extern void FIQ_Handler(void);

void Undefined_Handler(void)
{
  TRACE_FATAL("Undefined_Handler\r\n");
}
void SWI_Handler(void)
{
  TRACE_FATAL("SWI_Handler\r\n");
}
void Prefetch_Handler(void)
{
  TRACE_FATAL("Prefetch_Handler\r\n");
}
void Abort_Handler(void)
{
  TRACE_FATAL("Abort_Handler\r\n");
}
void FIQ_Handler(void)
{
  TRACE_FATAL("FIQ_Handler\r\n");
}
void defaultIrqHandler( void )
{
  TRACE_FATAL("defaultIrqHandler\r\n");
}
void defaultFiqHandler( void )
{
  TRACE_FATAL("defaultFiqHandler\r\n");
}
void defaultSpuriousHandler( void )
{
  TRACE_FATAL("defaultSpuriousHandler\r\n");
}
*/
//------------------------------------------------------------------------------
#define SPI_SPEED 1000000
#define DATAFLASH_TCSS (0xf << 16)      /* 250ns 15/60000000 */
#define DATAFLASH_TCHS (0x1 << 24)      /* 250ns 32*1/60000000 */
#define AT91C_SPI_CLK 14976000 
#define AT91C_MASTER_CLOCK 48000000

void nardware_init(void) {
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_PIOA) | (1 << AT91C_ID_US1) | (1
			<< AT91C_ID_US0) | (1 << AT91C_ID_ADC) | (1 << AT91C_ID_SPI);//|(1 << AT91C_ID_SYS);
	//--------------------------------------------------------------------
	// Configure PIO
	AT91C_BASE_PIOA->PIO_PER = STATUS_LED1 | STATUS_LED2 | STATUS_LED3
			| GSM_POWER_KEY | GSM_STATUS | RELAY1 | RELAY2 | USB_PULL_UP | DOK
			| FLT | UOK | CHRG | AT91C_PA5_RXD0 | AT91C_PA6_TXD0
			| GPS_POWER_SUPPLY | GSM_POWER_SUPPLY | KLINE_ENABLE
			| AT91C_PA11_NPCS0;
	//|AT91C_PA13_MOSI|AT91C_PA12_MISO|AT91C_PA14_SPCK|AT91C_PA11_NPCS0;

	AT91C_BASE_PIOA->PIO_PPUDR = STATUS_LED1 | STATUS_LED2 | STATUS_LED3
			| GSM_POWER_KEY | GSM_STATUS | RELAY1 | RELAY2 | AT91C_PA5_RXD0
			|AT91C_PA6_TXD0 | AT91C_PIO_PA17 | AT91C_PIO_PA18 | AT91C_PIO_PA19
			|AT91C_PIO_PA20 | USB_PULL_UP | GPS_POWER_SUPPLY | GSM_POWER_SUPPLY
			| KLINE_ENABLE;

	AT91C_BASE_PIOA->PIO_OER = STATUS_LED1 | STATUS_LED2 | STATUS_LED3
			| GSM_POWER_KEY | RELAY1 | RELAY2 | AT91C_PA5_RXD0 | AT91C_PA6_TXD0
			| USB_PULL_UP | GPS_POWER_SUPPLY | GSM_POWER_SUPPLY | KLINE_ENABLE
			| AT91C_PA11_NPCS0;
	;
	//----------------------------------------------------------------------------
	// Configure UART1 GPS Chanel
	AT91C_BASE_PIOA->PIO_PDR = AT91C_PA21_RXD1;
	AT91C_BASE_US1->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RXDIS
			| AT91C_US_TXDIS;
	// Configure mode
	AT91C_BASE_US1->US_MR = (AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE);
	// Configure baudrate
	// Asynchronous, no oversampling
	if (((0 & AT91C_US_SYNC) == 0) && ((0 & AT91C_US_OVER) == 0)) {
		AT91C_BASE_US1->US_BRGR = (BOARD_MCK / 9600) / 16;
	}
	AT91C_BASE_US1->US_IDR = 0xFFFFFF;
	AT91C_BASE_US1->US_IER = AT91C_US_RXRDY;
	// Disable the interrupt first
	AT91C_BASE_AIC->AIC_IDCR = 1 << AT91C_ID_US1;
	// Configure mode and handler
	AT91C_BASE_AIC->AIC_SMR[AT91C_ID_US1] = 0;
	AT91C_BASE_AIC->AIC_SVR[AT91C_ID_US1] = (unsigned int) gps_irq_read_data;
	// Clear interrupt
	AT91C_BASE_AIC->AIC_ICCR = 1 << AT91C_ID_US1;
	AT91C_BASE_AIC->AIC_IECR = 1 << AT91C_ID_US1;
	AT91C_BASE_US1->US_CR = AT91C_US_RXEN;
	//----------------------------------------------------------------------------
	// Configure ADC
	AT91C_BASE_ADC->ADC_CR = 0x2; // set Start Bit
	AT91C_BASE_ADC->ADC_MR = 0x3f00; // set mode register, 10bit chanel, MCK/128
	AT91C_BASE_ADC->ADC_CHER = 0xFF; // enable chanel 0-7
	//-----------------------------------------------------------------------------
	// Configure SPI
	AT91C_BASE_SPI->SPI_CR = AT91C_SPI_SPIDIS | AT91C_SPI_SWRST;
	AT91C_BASE_SPI->SPI_CR = AT91C_SPI_SWRST;
	AT91C_BASE_SPI->SPI_CR = AT91C_SPI_SWRST;
	AT91C_BASE_PIOA->PIO_ASR = AT91C_PA12_MISO | AT91C_PA13_MOSI
			| AT91C_PA14_SPCK;// | AT91C_PA11_NPCS0;
	AT91C_BASE_PIOA->PIO_PDR = AT91C_PA12_MISO | AT91C_PA13_MOSI
			| AT91C_PA14_SPCK;// | AT91C_PA11_NPCS0;
	AT91C_BASE_PIOA->PIO_PPUER = AT91C_PA12_MISO | AT91C_PA11_NPCS0;
	AT91C_BASE_SPI->SPI_MR = 0;
	AT91C_BASE_SPI->SPI_MR = AT91C_SPI_MSTR | AT91C_SPI_MODFDIS
			| AT91C_SPI_PS_FIXED;
	AT91C_SPI_CSR[0] = 0;
	AT91C_SPI_CSR[0] = AT91C_SPI_NCPHA | (2 << 8) | AT91C_SPI_BITS_8;// | (AT91C_SPI_DLYBS & 0x100000) ;
	DF_CS_inactive;
	AT91C_BASE_SPI->SPI_CR = AT91C_SPI_SPIEN;
	//-------------------------------------------------------------------------
	// Configure DBGU shell
	AT91C_BASE_PIOA->PIO_ASR = AT91C_PA9_DRXD | AT91C_PA10_DTXD;
	AT91C_BASE_PIOA->PIO_PDR = AT91C_PA9_DRXD | AT91C_PA10_DTXD;
	AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RSTRX | AT91C_US_RSTTX;
	AT91C_BASE_DBGU->DBGU_IDR = 0xFFFFFFFF;
	AT91C_BASE_DBGU->DBGU_BRGR = BOARD_MCK / (115200 * 16);
	AT91C_BASE_DBGU->DBGU_MR = (AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE);
	AT91C_BASE_DBGU->DBGU_PTCR = AT91C_PDC_RXTDIS | AT91C_PDC_TXTDIS;
	AT91C_BASE_DBGU->DBGU_CR = AT91C_US_RXEN | AT91C_US_TXEN;
	//AT91C_BASE_DBGU->DBGU_IER = AT91C_US_RXBUFF;
	//-------------------------------------------------------------------------
	// System clock init
	unsigned int dummy;
	AT91C_BASE_AIC->AIC_IDCR = 1 << AT91C_ID_TC0;
	AT91C_BASE_TC0->TC_IDR = AT91C_TC_CPCS;
	AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC0);
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
	AT91C_BASE_TC0->TC_IDR = 0xFFFFFFFF;
	dummy = AT91C_BASE_TC0->TC_SR;
	dummy = dummy;
	AT91C_BASE_TC0->TC_CMR = 0x01;
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKEN;
	AT91C_BASE_AIC->AIC_IDCR = 0x1 << AT91C_ID_TC0;
	AT91C_BASE_AIC->AIC_SVR[AT91C_ID_TC0] = (unsigned int) timer0_c_irq_handler;
	AT91C_BASE_AIC->AIC_SMR[AT91C_ID_TC0] = AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL
			| 1;
	AT91C_BASE_AIC->AIC_ICCR = 0x1 << AT91C_ID_TC0;
	AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS; //  IRQ enable CPC
	AT91C_BASE_AIC->AIC_IECR = 0x1 << AT91C_ID_TC0;
	AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG;
}
//----------------------------------------------------------------------------
void enable_io_modem(void) {
	//GSM_MON;
	AT91C_BASE_US0->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RXDIS
			| AT91C_US_TXDIS;
	// Configure mode
	AT91C_BASE_US0->US_MR = (AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE);
	// Configure baudrate
	// Asynchronous, no oversampling
	if (((0 & AT91C_US_SYNC) == 0) && ((0 & AT91C_US_OVER) == 0)) {
		AT91C_BASE_US0->US_BRGR = (BOARD_MCK / 115200) / 16;
	}
	AT91C_BASE_US0->US_CR = AT91C_US_RXEN;
	AT91C_BASE_US0->US_CR = AT91C_US_TXEN;
	AT91C_BASE_US0->US_IER = AT91C_US_RXRDY | AT91C_US_TIMEOUT;
	;
	// Disable the interrupt first
	AT91C_BASE_AIC->AIC_IDCR = 1 << AT91C_ID_US0;
	// Configure mode and handler
	AT91C_BASE_AIC->AIC_SMR[AT91C_ID_US0] = 0;
	AT91C_BASE_AIC->AIC_SVR[AT91C_ID_US0] = (unsigned int) irq_read_modem;
	// Clear interrupt
	AT91C_BASE_AIC->AIC_ICCR = 1 << AT91C_ID_US0;
	AT91C_BASE_AIC->AIC_IECR = 1 << AT91C_ID_US0;
	AT91C_BASE_PIOA->PIO_PPUDR = AT91C_PA5_RXD0 | AT91C_PA6_TXD0;
	AT91C_BASE_PIOA->PIO_OER = AT91C_PA5_RXD0 | AT91C_PA6_TXD0;
	AT91C_BASE_PIOA->PIO_PER = AT91C_PA5_RXD0 | AT91C_PA6_TXD0;
	AT91C_BASE_PIOA->PIO_PDR = AT91C_PA5_RXD0 | AT91C_PA6_TXD0;
	AT91C_BASE_US0->US_RTOR = 6000;
}
//----------------------------------------------------------------------------
void disable_io_modem(void) {
	AT91C_BASE_PIOA->PIO_PPUDR = AT91C_PA5_RXD0 | AT91C_PA6_TXD0;
	AT91C_BASE_PIOA->PIO_ODR = AT91C_PA5_RXD0 | AT91C_PA6_TXD0;
	AT91C_BASE_AIC->AIC_IDCR = 1 << AT91C_ID_US0;
	AT91C_BASE_US0->US_CR = AT91C_US_RXDIS;
	AT91C_BASE_US0->US_CR = AT91C_US_TXDIS;
	AT91C_BASE_US0->US_IDR = 0xFFFFFF;
	//GSM_MOFF;
}
//----------------------------------------------------------------------------
void save_settings_in_flash(void) {
	unsigned char tmpbuf[sizeof(Device.setting)];
	unsigned long adreesSetting = ((unsigned long) &SettingDeviceFlash);
	at91flashGetLock(adreesSetting);
	at91flashSetLock(adreesSetting, 0);
	memcpy(tmpbuf, &Device.setting, sizeof(Device.setting));
	at91flashWrite(adreesSetting, tmpbuf, sizeof(Device.setting));
}
//----------------------------------------------------------------------------
//#define UA
///#define RU

void set_setting_to_default(void) {
#ifdef RU
	strcpy(Device.setting.apn, "internet.beeline.ru");
	strcpy(Device.setting.apnuser, "beeline");
	strcpy(Device.setting.apnpswd, "beeline");
	strcpy(Device.setting.main_server_ip, "78.108.90.197");
        //strcpy(Device.setting.main_server_ip,"91.192.132.146");
        strcpy(Device.setting.usd_cash, "#102#");
        strcpy(Device.setting.director_namber, "9051845317");
#endif
#ifdef UA
	strcpy(Device.setting.apn, "internet");
	strcpy(Device.setting.apnuser, "");
	strcpy(Device.setting.apnpswd, "");
        strcpy(Device.setting.main_server_ip, "78.108.90.197"); 
	//strcpy(Device.setting.main_server_ip,"91.192.132.146");
	strcpy(Device.setting.usd_cash, "*101#");
        strcpy(Device.setting.director_namber, "0505745157");
#endif
	Device.setting.settingID = 0x11;
	Device.setting.main_server_port = 5700;
	Device.setting.send_geo_data_to_server_time = 5;
	Device.setting.send_io_data_to_server_time = 120;
	Device.setting.led_status = 1;
	strcpy(Device.setting.relay1_on_phraze, "Relay1on");
	strcpy(Device.setting.relay1_off_phraze, "Relay1off");
	strcpy(Device.setting.relay2_on_phraze, "Relay2on");
	strcpy(Device.setting.relay2_off_phraze, "Relay2off");
	Device.setting.shared_relay_control = 0;
	save_settings_in_flash();
}
//----------------------------------------------------------------------------
void watchdogEnable(int millis) {
	int period = (millis * 256) / 1000;
	AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDRSTEN | // enable reset on timeout
			AT91C_WDTC_WDDBGHLT | // respect debug mode
			AT91C_WDTC_WDIDLEHLT | // respect idle mode
			((period << 16) & AT91C_WDTC_WDD) | // delta is as wide as the period, so we can restart anytime
			(period & AT91C_WDTC_WDV); // set the period
}
//----------------------------------------------------------------------------
void main() {
    __disable_interrupt();
    at91flashInit();
    memset(&Device, 0, sizeof(Device));// DeviceStatus->PowerState = 0;
#ifdef DEBUG

#else
   // if (SettingDeviceFlash.settingID == 0xFF)
#endif
    set_setting_to_default();
    memcpy(&Device.setting, &SettingDeviceFlash, sizeof(Device.setting));

#ifdef DEBUG
     AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDDIS;
#else    
#define WDT_TIME 5
    
    WDIDLEHLT = 1               в нерабочем режиме WDT не работает
    WDDBGHLT  = 1               при отладке WDT не работает
    WDD = WDT_TIME * 256        5 секунд
    WDDIS = 0                   WDT разрешен
    WDRPROC = 0                 полный сброс - и процессора и периферии
    WDRSTEN = 1                 разрешение сброса от WDT
    WDFIEN = 0                  запрет прерывания от WDT
    WDV = WDT_TIME * 256        5 секунд
    AT91C_BASE_WDTC->WDTC_WDMR =  AT91C_WDTC_WDIDLEHLT
                                | AT91C_WDTC_WDDBGHLT
                                | ((WDT_TIME*256)<<16)
                                | AT91C_WDTC_WDRSTEN
                                | (WDT_TIME*256);
#endif
    
	*AT91C_WDTC_WDCR = 0xA5000001;
	Device.gps.IDGPS = 0x20;
	Device.io.IDIO = 0x21;
	nardware_init();
	USB_HOSTDOWN;
	AT91F_USB_Open();
	disable_io_modem();
	GSM_MOFF;
	GPS_MON;
	STATUS_LED1_OFF;
	STATUS_LED2_OFF;
	STATUS_LED3_OFF;
	__enable_interrupt();
	system_start();
} //099 303 24 86
//--------------------------------------------------------------------------


