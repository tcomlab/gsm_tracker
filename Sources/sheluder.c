/*
 * heluder.c
 *
 *  Created on: 10.04.2010
 *      Author: Admin
 */
//---------------------------------------------------------------------------------
#include "main.h"
#include "gsm.h"
#include "gps.h"
#include "stdio.h"
#include "system.h"
#include <intrinsics.h>
#include <stdlib.h>
#include <string.h>
#include "usb.h"
#include "sms.h"
#include "db.h"
#include "crc.h"
//---------------------------------------------------------------------------------
#define BATT_ALERT    0
#define BATT_LOW      1
#define BATT_NORMAL   2
#define BATT_GOOD     3
//---------------------------------------------------------------------------------
#define REDY_TO_CONNECT    0 // Готов к соединению
#define WAIT_SERVER_ANSWER 1 // Ожидаем ответа сервера
#define SEND_LOGIN         2 // Отсылаем логин информацию
#define WAIT_LOGIN_RESULT  3 // Ожидаем результата авторизации
#define READY_TO_TRANSFER  4 // Готов для передачи данных на сервер
//---------------------------------------------------------------------------------
extern SDeviceStatus Device;
//#define IMEI_EMULATED "356895035537219"
char tmp_buf[128];
//---------------------------------------------------------------------------------
extern void sms_requvest_sheluder(void);
extern void usd_sheluder(void);
extern void save_settings_in_flash(void);
extern void disable_io_modem(void);
extern void enable_io_modem(void);
//---------------------------------------------------------------------------------
void led_indications(unsigned int led_blink_time) // Индикация
{
	static unsigned int local_timer_led1 = 0;
	static unsigned char togle = 0;
	static unsigned int local_led_blink_time=0;
	if (led_blink_time > 0) local_led_blink_time = led_blink_time;
    //------------------------------------------------------------
	if (Device.setting.led_status == 0)
	{
		STATUS_LED1_OFF;
		STATUS_LED2_OFF;
		STATUS_LED3_OFF;
		return; // если не позволено мигать то уходим
	}
	//------------------------------------------------------------
	if (Device.power_status  < BATT_NORMAL)
	{
		if (Device.sys_time_msec > (local_timer_led1 + 30))
		{
			local_timer_led1 = Device.sys_time_msec++;

			if (togle == 0)
			{
				STATUS_LED1_ON;
				STATUS_LED2_ON;
				STATUS_LED3_ON;
				togle = 1;
			}
			else if (togle == 1)
			{
				STATUS_LED1_OFF;
				STATUS_LED2_OFF;
				STATUS_LED3_OFF;
				togle = 0;
			}
		}
        return;
	}
	//------------------------------------------------------------
	if (Device.gps.status > 0)
	{
		STATUS_LED3_ON;
	}
	else
	{
		STATUS_LED3_OFF;
	}
	//------------------------------------------------------------
	if (Device.sys_time_msec > (local_timer_led1 + local_led_blink_time))
	{
		local_timer_led1 = Device.sys_time_msec++;
		;
		if (togle == 0)
		{
			STATUS_LED1_ON;
			togle = 1;
		}
		else if (togle == 1)
		{
			STATUS_LED1_OFF;
			togle = 0;
		}
	}
	//------------------------------------------------------------
}
//---------------------------------------------------------------------------------
void data_transfer(void)
{
	unsigned char tmp[258];
	memset(tmp, 0, 255);
	static unsigned int time_to_send_geodata = 0;
	static unsigned int time_to_send_iodata = 0;
	//------------------------------------------------------------
	if (Device.sys_time > (time_to_send_geodata
			+ Device.setting.send_geo_data_to_server_time))
	{
		time_to_send_geodata = Device.sys_time;
		STATUS_LED2_ON;
		if (Device.gps.status == 0)
		{
			printf("NO data avalible satelites, no send geodata\r\n");
			return;
		}
		Device.gps.CRC = CRC((unsigned char *) &Device.gps, sizeof(Device.gps)
				- 2);
		send_data_server((unsigned char *) &Device.gps, sizeof(Device.gps));
		STATUS_LED2_OFF;
		return;
	}
	//---------------------------------------------------------------------------
	if (Device.sys_time > (time_to_send_iodata
			+ Device.setting.send_io_data_to_server_time))
	{
		time_to_send_iodata = Device.sys_time;
		STATUS_LED2_ON;
		memcpy(&Device.io.IO_time, &Device.gps.GEO_time,
				sizeof(Device.gps.GEO_time));
		Device.io.CRC
				= CRC((unsigned char *) &Device.io, sizeof(Device.io) - 2);
		send_data_server((unsigned char *) &Device.io, sizeof(Device.io));
		STATUS_LED2_OFF;
		return;
	}
	//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------------
void call_sheluder(void)
{
	Device.gsm.call_data_flag = 0;
	SendATWResp("AT+CLCC\r", "OK");
	if (strstr(Device.gsm.at_data, Device.setting.director_namber) != 0)
	{
		SendATWResp("ATA\r", "OK");
		SendATWResp("AT+VTS=1\r", "OK");
		RELAY1_ON;
	}
	SendATWResp("ATH\r", "OK");
}
//--------------------------------------------------------------------------------
void batt_control(void)
{
	static unsigned int local_timer = 0;
	if (Device.sys_time > (local_timer + 5))
	{
		local_timer = Device.sys_time;
		AT91C_BASE_ADC->ADC_CR = 0x2; // Start conversion
		while (!(AT91C_BASE_ADC->ADC_SR & AT91C_ADC_EOC0))
			; // Wait ADC0 conwersion
		Device.io.ch5 = 1023 - AT91C_BASE_ADC->ADC_CDR0;
		while (!(AT91C_BASE_ADC->ADC_SR & AT91C_ADC_EOC1))
			; // Wait ADC1 conwersion
		Device.io.ch6 = 1023 - AT91C_BASE_ADC->ADC_CDR1;
		while (!(AT91C_BASE_ADC->ADC_SR & AT91C_ADC_EOC2))
			; // Wait ADC2 conwersion
		Device.io.vbat = AT91C_BASE_ADC->ADC_CDR2;
		while (!(AT91C_BASE_ADC->ADC_SR & AT91C_ADC_EOC3))
			; // Wait ADC3 conwersion
		Device.io.vbort = AT91C_BASE_ADC->ADC_CDR3;
		while (!(AT91C_BASE_ADC->ADC_SR & AT91C_ADC_EOC4))
			; // Wait ADC4 conwersion
		Device.io.ch1 = 1023 - AT91C_BASE_ADC->ADC_CDR4;
		while (!(AT91C_BASE_ADC->ADC_SR & AT91C_ADC_EOC5))
			; // Wait ADC5 conwersion
		Device.io.ch2 = 1023 - AT91C_BASE_ADC->ADC_CDR5;
		while (!(AT91C_BASE_ADC->ADC_SR & AT91C_ADC_EOC6))
			; // Wait ADC6 conwersion
		Device.io.ch3 = 1023 - AT91C_BASE_ADC->ADC_CDR6;
		while (!(AT91C_BASE_ADC->ADC_SR & AT91C_ADC_EOC7))
			; // Wait ADC7 conwersion
		Device.io.ch4 = 1023 - AT91C_BASE_ADC->ADC_CDR7;
		//------------------------------------------
		if (!(AT91C_BASE_PIOA->PIO_PDSR & (DOK)))
			Device.io.flags |= BORTSYP;
		else
			Device.io.flags &= ~BORTSYP;
		//-------------------------------------------
		if (!(AT91C_BASE_PIOA->PIO_PDSR & (FLT)))
			Device.io.flags |= FLTPOWER;
		else
			Device.io.flags &= ~FLTPOWER;
		//-------------------------------------------
		if (AT91C_BASE_PIOA->PIO_PDSR & (CHRG))
			Device.io.flags &= ~BATTCHR;
		else
			Device.io.flags |= BATTCHR;
		//-------------------------------------------
		if (AT91C_BASE_PIOA->PIO_PDSR & (RELAY1))
			Device.io.flags |= RELAYF1;
		else
			Device.io.flags &= ~RELAYF1;
		//-------------------------------------------
		if (AT91C_BASE_PIOA->PIO_PDSR & (RELAY2))
			Device.io.flags |= RELAYF2;
		else
			Device.io.flags &= ~RELAYF2;
		//-------------------------------------------
		// 0.0032258064516129032258064516129032
		if (Device.io.vbat < 565 )
		{
			Device.power_status = BATT_LOW;
		}
		else
		if (Device.io.vbat > 575 )
		{
			Device.power_status = BATT_NORMAL;
		}
	}
}
//----------------------------------------------------------------------------------
#define NORMAL_TASK       0
#define SERVER_DATA_TASK  1
#define AT_NO_WAITE_TASK  2
#define RESET_TASK        3

void task_data(unsigned char mode)
{
	static unsigned int login_timeout = 0;
	static unsigned int timeout_connection = 0;
	static unsigned char connect_automat = 0;
	static unsigned int global_errore_counter=0;
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++
	if (global_errore_counter >= 5)
	{
		GSM_MOFF;
		global_errore_counter = 0;
		printf("GSM errore counter overflow \r\n");
		mode = RESET_TASK;
	}
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++
	if (mode == RESET_TASK)
	{
		Device.gsm.call_ready = 0; // Тушим флаг активности модема
		connect_automat = 0;
		//printf("connect_automat = 0;{1}\r\n");
		login_timeout = 0;
		global_errore_counter=0;
		Device.gsm.gprs_active = 0;
		return;
	}
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++
	if (mode == SERVER_DATA_TASK)
	{
		if (strstr(Device.gsm.gprs_data, "CONS") != 0) //
		{
			connect_automat = SEND_LOGIN;
			printf("connect_automat = SEND_LOGIN;{2}\r\n");
			goto end_func;
		}

		if (strstr(Device.gsm.gprs_data, "ASUC") != 0)
		{
			connect_automat = READY_TO_TRANSFER;
			printf("connect_automat = READY_TO_TRANSFER;{3}\r\n");
			goto end_func;
		}

		if (strstr(Device.gsm.gprs_data, "GEOK") != 0)
		{
			global_errore_counter=0;
			printf("\r\n :Save geo data ok\r\n");
			goto end_func;
		}

		if (strstr(Device.gsm.gprs_data, "IOOK") != 0)
		{
			printf("\r\n :Save io data ok\r\n");
			//error_pounter = 0;
			goto end_func;
		}

		if (strstr(Device.gsm.gprs_data, "CSOK") != 0)
		{
			printf("\r\n :Save cash data ok\r\n");
			//error_pounter = 0;
			goto end_func;
		}
	end_func:
		Device.gsm.gprs_data_flag = 0;
		memset(Device.gsm.gprs_data, 0, sizeof(Device.gsm.gprs_data));
		return;
	}
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++
	if (mode == AT_NO_WAITE_TASK)
	{
		if (strstr(Device.gsm.at_data, "+PDP: DEACT") != 0)
		{
			connect_automat = REDY_TO_CONNECT;
			printf("connect_automat = REDY_TO_CONNECT;{4}\r\n");
			global_errore_counter++;
			goto end;
		}

		if (strstr(Device.gsm.at_data, "CONNECT FAIL") != 0)
		{
			connect_automat = REDY_TO_CONNECT;
			printf("connect_automat = REDY_TO_CONNECT;{5}\r\n");
			goto end;
		}
		if (strstr(Device.gsm.at_data, "CONNECT OK") != 0)
		{

			goto end;
		}

		if (strstr(Device.gsm.at_data, "CLOSED") != 0)
		{
			connect_automat = REDY_TO_CONNECT;
			printf("connect_automat = REDY_TO_CONNECT;{6}\r\n");
			goto end;
		}

		if (strstr(Device.gsm.at_data, "Call Ready") != 0)
		{
			Device.gsm.call_ready = 1;
			goto end;
		}

		if (strstr(Device.gsm.at_data, "UNDER-VOLTAGE WARNNING") != 0)
		{
			goto end;
		}
		//if (strstr(at_data, "SEND OK")) {
		//	send_ok = 1;
		//	goto end;
		//}
		end: memset(Device.gsm.at_data, 0, sizeof(Device.gsm.at_data));
		Device.gsm.at_data_flag = 0;
		return;
	}
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (mode == NORMAL_TASK)
    {
	// ++++++++++++++++++++++++++++++++++++++++++++++++++++
    	if (Device.gsm.at_data_flag)   return;
    	if (Device.gsm.gprs_data_flag) return;
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++
    	switch (connect_automat)
    	{
    		case REDY_TO_CONNECT:
    			switch (tcp_status())
    			{
    			case IP_INITIAL:
    				printf("try connect to server\r\n");
    				connect_to_server(Device.setting.main_server_ip,
    						Device.setting.main_server_port);
    				timeout_connection = Device.sys_time;
    				connect_automat = WAIT_SERVER_ANSWER;
    				printf("connect_automat = WAIT_SERVER_ANSWER;{7}\r\n");
    				break;

    			case CONNECT_OK:
    				SendATWResp("AT+CIPCLOSE\r", "OK");
    				global_errore_counter++;
    				break;

    			case IP_CLOSE:
    				SendATWResp("AT+CIPSHUT\r", "OK");
    				global_errore_counter++;
    				break;

    			case PDP_DEACT_PDP:
    				SendATWResp("AT+CIPCLOSE\r", "OK");
    				SendATWResp("AT+CIPSHUT\r", "OK");
    				global_errore_counter++;
    				break;

    			default: // Боимся не понятного
    				SendATWResp("AT+CIPSHUT\r", "OK");
    				global_errore_counter++;
    				break;
    			}
    			return;
     //--------------------------------------------------------------
    		case WAIT_SERVER_ANSWER:
    			if (Device.sys_time > (timeout_connection + 40)) // 40 sec
    			{
    				timeout_connection = Device.sys_time;
    				connect_automat = REDY_TO_CONNECT;
    				printf("connect_automat = REDY_TO_CONNECT;{8}\r\n");
    			}
    			return;
	 //--------------------------------------------------------------
    		case SEND_LOGIN:

    			switch (tcp_status())
    			{
    			case CONNECT_OK:
    			case 0xFF:
    				memset(tmp_buf, 0, sizeof(tmp_buf));
    				Device.IMEI[15] = '\0';
    				sprintf(tmp_buf,"R%s",Device.IMEI);
    				send_data_server((unsigned char *)tmp_buf, 20);
    				break;

    			default:
    				connect_automat = REDY_TO_CONNECT;
    				printf("connect_automat = REDY_TO_CONNECT;{9}\r\n");
    				return;
    			}
    			connect_automat = WAIT_LOGIN_RESULT;
    			printf("connect_automat = WAIT_LOGIN_RESULT;{10}\r\n");
    			login_timeout = 0;
    			return;
	 //--------------------------------------------------------------
    		case WAIT_LOGIN_RESULT:
    			printf("wait login result.. \r\n");
    			if (login_timeout >= 4)
    			{
    				connect_automat = SEND_LOGIN;
    			    printf("connect_automat = SEND_LOGIN;{11}\r\n");
    			}
    			else
    				login_timeout++;
    			return;
	 //--------------------------------------------------------------
    		case READY_TO_TRANSFER:
    			switch (tcp_status())
    			{
    			case CONNECT_OK:
    				data_transfer();
    				break;

    			case IP_CLOSE:
    				SendATWResp("AT+CIPSHUT\r", "OK");
    				global_errore_counter++;
    				break;
    			default:
    				break;
    			}
    			return;
    	}
    return;
    //--------------------------------------------------------------
    }
}
//--------------------------------------------------------------------------------
void transfer_data_to_data_base(void)
{
	return; // TODO: Save data to dataflash mc
}
//--------------------------------------------------------------------------------
void modem_watcher(void)
{
	static volatile unsigned char modem_automat = 0;
	static unsigned int local_timer = 0;
	static unsigned int timeout_wait_call_ready = 0;
	static unsigned int timeout_modem_dont_up = 0;

	if (Device.power_status  < BATT_NORMAL)
	{
		GSM_MOFF;
		task_data(RESET_TASK);
		disable_io_modem();
	    return;
	}

	if ((AT91C_BASE_PIOA->PIO_PDSR & GSM_STATUS))
	{
		task_data(RESET_TASK);
		switch (modem_automat)
		{
		case 0:
			disable_io_modem();
			GSM_MON;
			local_timer = Device.sys_time;
			modem_automat = 1;
			printf("Modem power on, power key press\r\n");
			AT91C_BASE_PIOA->PIO_SODR = GSM_POWER_KEY;
			return;

		case 1: // Ето получилось из-за того что модем несмог включится
			if (Device.sys_time > (timeout_modem_dont_up + 15))
			{
				timeout_modem_dont_up = Device.sys_time;
				modem_automat = 0;
				printf("Modem dont up STATUP PIN\r\n");
				return;
			}
			return;

		case 5:
			GSM_MOFF;
			printf("Modem status crash, re-up system\r\n");
			task_data(RESET_TASK);
			disable_io_modem();
                        modem_automat = 0;
			return;
		default:
			modem_automat = 0;
			return;
		}
	}
	else
	{
		switch (modem_automat)
		{
			//--------------------------------------------------
			case 0:
			return;
			//--------------------------------------------------
			case 1:
				if ((local_timer + 2) < Device.sys_time)
				{
					AT91C_BASE_PIOA->PIO_CODR = GSM_POWER_KEY;
					printf("power key pop\r\n");
					modem_automat = 2;
					return;
				}
			return;
			//--------------------------------------------------
			case 2:
				modem_automat = 3;
				enable_io_modem();
				timeout_wait_call_ready = Device.sys_time;
			return;
			//--------------------------------------------------
			case 3:
				if (Device.gsm.call_ready == 0)
				{
					if (Device.sys_time > (timeout_wait_call_ready + 20))
					{
						timeout_wait_call_ready = Device.sys_time;
						GSM_MOFF;
						printf("Modem dont sent Call Ready\r\n");
						task_data(RESET_TASK);
						modem_automat = 0;
						disable_io_modem();
					}
				return; // Ожидаем когда модем скажет кол реди
				}
				local_timer = Device.sys_time;
				printf("Modem on, init system\r\n");
				SendATWResp("ATE0\r", "OK");
#ifdef IMEI_EMULATED
				strcpy(Device.IMEI,IMEI_EMULATED);
#else
				get_imei(Device.IMEI);
#endif
				SendATWResp("AT+CIPHEAD=1\r", "OK");
				SendATWResp("AT+CMGF=1\r", "OK");
				SetAPN(Device.setting.apn);
				modem_automat = 4;
			return;
			//-----------------------------------------------
			case 4:
				if ((local_timer + 4) < Device.sys_time)
				{
					local_timer = Device.sys_time;
					led_indications(20);
					if (gsm_networt_avalible())
					{
						Device.gsm.gprs_active = enable_gprs();
						modem_automat = 5;
						return;
					}
				}
			return;
			//------------------------------------------------
			case 5:
				led_indications(200);
			return;
			//------------------------------------------------
			default:
			//modem_automat = 0 ;
			return;
		}
	}
}
//--------------------------------------------------------------------------------
void system_start(void)
{
	unsigned int cash_timer = 0;
	unsigned int csq_timer = 0;
	unsigned int check_time = 0;
	printf("%c[2J", 27);
	printf("-- Compiled: %s %s --\n\r", __DATE__, __TIME__);
	while (1)
	{
		STATUS_LED2_OFF;
		*AT91C_WDTC_WDCR = 0xA5000001;
		//--------------------------------------------------------------
		batt_control();
		//--------------------------------------------------------------
		modem_watcher();
		//--------------------------------------------------------------
		if (Device.gsm.gprs_active == 1)
		{
			if (Device.sys_time > (check_time + 2))
			{
				check_time = Device.sys_time;
				task_data(NORMAL_TASK);
			}
		}
		else
		transfer_data_to_data_base();
		//--------------------------------------------------------------
		//if (!(AT91C_BASE_PIOA->PIO_PDSR & (UOK)))
		//{
		//	USB_HOSTUP;
		//	usb_process();
		//}
		//else
		//{
		//	USB_HOSTDOWN;
		//}
		//--------------------------------------------------------------
		led_indications(0); /// Работа с седодиодами
		//--------------------------------------------------------------
		if (Device.gsm.at_data_flag)
					task_data(AT_NO_WAITE_TASK);
		//--------------------------------------------------------------
		if (Device.gps_data_redy)
					gps_sheluder();
		//--------------------------------------------------------------
		if (Device.gsm.sms_data_flag)
					sms_requvest_sheluder();
		//--------------------------------------------------------------
		if (Device.gsm.call_ready)
		{
			if (Device.gsm.gprs_data_flag)
				task_data(SERVER_DATA_TASK);
		//--------------------------------------------------------------
			if (Device.gsm.call_data_flag)
				call_sheluder();
		//--------------------------------------------------------------
			if (Device.gsm.usd_data_flag)
				usd_sheluder();
		//--------------------------------------------------------------
			if (Device.sys_time > (cash_timer + 500))
			{
				cash_timer = Device.sys_time;
				sprintf(tmp_buf, "at+cusd=1,\"%s\"\r", Device.setting.usd_cash);
				ReqCSD(tmp_buf);
			}
		//--------------------------------------------------------------
			if (Device.sys_time > (csq_timer + 200))
			{
				csq_timer = Device.sys_time;
				get_csq(&(Device.io.csq));
			}
		//--------------------------------------------------------------
		}
		//--------------------------------------------------------------
	}
}
