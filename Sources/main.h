/*
 * GPS GSM TRACKER PROJECT Ver:0.01
 * Author Syvash Taras aka TCom
 * www.esl.net.ua
 * esl electronic develop group
 * 04.01.2010
 *
 */

#ifndef MAIN_H
#define MAIN_H

#include "AT91SAM7S256.h"
//#define NULLDEV (AT91C_PIO_PA24)
//------------------------------------------------------------------------------
// Clocks
//------------------------------------------------------------------------------
/// Frequency of the board main oscillator, in Hz.
#define BOARD_MAINOSC           18432000
/// Master clock frequency (when using board_lowlevel.c), in Hz.
#define BOARD_MCK               48000000
//------------------------------------------------------------------------------
#define STATUS_LED1_ON     AT91C_BASE_PIOA->PIO_SODR = STATUS_LED1;
#define STATUS_LED1_OFF    AT91C_BASE_PIOA->PIO_CODR = STATUS_LED1;

#define STATUS_LED2_ON     AT91C_BASE_PIOA->PIO_SODR = STATUS_LED2;
#define STATUS_LED2_OFF    AT91C_BASE_PIOA->PIO_CODR = STATUS_LED2;

#define STATUS_LED3_ON     AT91C_BASE_PIOA->PIO_SODR = STATUS_LED3;
#define STATUS_LED3_OFF    AT91C_BASE_PIOA->PIO_CODR = STATUS_LED3;

#define RELAY1_ON          AT91C_BASE_PIOA->PIO_SODR = RELAY1;
#define RELAY1_OFF         AT91C_BASE_PIOA->PIO_CODR = RELAY1;

#define RELAY2_ON          AT91C_BASE_PIOA->PIO_SODR = RELAY2;
#define RELAY2_OFF         AT91C_BASE_PIOA->PIO_CODR = RELAY2;

#define USB_HOSTUP         AT91C_BASE_PIOA->PIO_CODR = USB_PULL_UP;
#define USB_HOSTDOWN       AT91C_BASE_PIOA->PIO_SODR = USB_PULL_UP;

#define GSM_MOFF           AT91C_BASE_PIOA->PIO_SODR = GSM_POWER_SUPPLY;
#define GSM_MON            AT91C_BASE_PIOA->PIO_CODR = GSM_POWER_SUPPLY;

#define GPS_MOFF           AT91C_BASE_PIOA->PIO_SODR = GPS_POWER_SUPPLY;
#define GPS_MON            AT91C_BASE_PIOA->PIO_CODR = GPS_POWER_SUPPLY;

#define DF_CS_active           AT91C_BASE_PIOA->PIO_CODR = AT91C_PA11_NPCS0;
#define DF_CS_inactive         AT91C_BASE_PIOA->PIO_SODR = AT91C_PA11_NPCS0;

#define RELAYF1  (1<<0) // Relay 1 state
#define RELAYF2  (1<<1) // Relay 2 state
#define BATTCHR  (1<<2) // Battery charge now
#define USBPLUG  (1<<3) // USB Connect to host
#define BORTSYP  (1<<4) // DC input OK
#define FLTPOWER (1<<5) // Power fault
//------------------------------------------------------------------------------
// Pins
//------------------------------------------------------------------------------
#define STATUS_LED1        AT91C_PIO_PA0
#define STATUS_LED2        AT91C_PIO_PA1
#define STATUS_LED3        AT91C_PIO_PA2
#define GPS_POWER_SUPPLY   AT91C_PIO_PA3
#define GSM_POWER_SUPPLY   AT91C_PIO_PA4
#define GSM_RX_DATA        AT91C_PIO_PA5
#define GSM_TX_DATA        AT91C_PIO_PA6
#define FLT                AT91C_PIO_PA7
#define UOK                AT91C_PIO_PA8
#define CAN_KLINE_RX       AT91C_PIO_PA9
#define CAN_KLINE_TX       AT91C_PIO_PA10
#define DF_CS              AT91C_PIO_PA11
#define DF_MISO            AT91C_PIO_PA12
#define DF_MOSI            AT91C_PIO_PA13
#define DF_CLK             AT91C_PIO_PA14
#define RELAY1             AT91C_PIO_PA15
#define RELAY2             AT91C_PIO_PA16
#define GPS_DATA           AT91C_PIO_PA21
#define GSM_STATUS         AT91C_PIO_PA22
#define GSM_POWER_KEY      AT91C_PIO_PA23
#define DOK                AT91C_PIO_PA24
#define CHRG               AT91C_PIO_PA25
#define KLINE_ENABLE       AT91C_PIO_PA26
#define nc1                AT91C_PIO_PA27
#define nc2                AT91C_PIO_PA28
#define nc3                AT91C_PIO_PA29
#define nc4                AT91C_PIO_PA30
#define USB_PULL_UP        AT91C_PIO_PA31
//-----------------------------------------------------------------------------
#endif
