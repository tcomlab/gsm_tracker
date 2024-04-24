/*
 * gsm.c
 *
 *  Created on: 17.02.2010
 *      Author: Admin
 */
//-----------------------------------------------------------------------------------
#include "gsm.h"
#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "system.h"
//-----------------------------------------------------------------------------------
char rx_buffer[512];
//-----------------------------------------------------------------------------------
extern SDeviceStatus Device;
//---------------------------------------------------------
extern void enable_io_modem(void);
extern void disable_io_modem(void);
//-----------------------------------------------------------------------------------
/*
 * Очистка буфера ответов АТ комманд
 */
//-------------------------------------------
void irq_read_modem(void) // Функция читает в буфер данные и работает автономно
{
	unsigned int status;
	static unsigned int counter = 0;
	status = AT91C_BASE_US0->US_CSR;
	if (status & AT91C_US_RXRDY) {
		if (counter >= sizeof(rx_buffer))
			counter = 0;
		rx_buffer[counter++] = AT91C_BASE_US0->US_RHR;
	}
	if (status & AT91C_US_TIMEOUT) {
		printf("GSM_BUF:%s",rx_buffer);
		if (strstr(rx_buffer, "+IPD")) {
			memset(Device.gsm.gprs_data, 0, sizeof(Device.gsm.gprs_data));
			memcpy(Device.gsm.gprs_data, rx_buffer,
					sizeof(Device.gsm.gprs_data));
			Device.gsm.gprs_data_flag = 1;
			goto end;
		}
		if (strstr(rx_buffer, "+CMTI:")) {
			memset(Device.gsm.sms_data, 0, sizeof(Device.gsm.sms_data));
			Device.gsm.sms_data_flag = 1;
			memcpy(Device.gsm.sms_data, rx_buffer, sizeof(Device.gsm.sms_data));
			goto end;
		}
		if (strstr(rx_buffer, "RING")) {
			memset(Device.gsm.usd_call_data, 0,
					sizeof(Device.gsm.usd_call_data));
			Device.gsm.call_data_flag = 1;
			memcpy(Device.gsm.usd_call_data, rx_buffer,
					sizeof(Device.gsm.usd_call_data));
			goto end;
		}
		if (strstr(rx_buffer, "+CUSD:")) {
			memset(Device.gsm.usd_call_data, 0,
					sizeof(Device.gsm.usd_call_data));
			Device.gsm.usd_data_flag = 1;
			memcpy(Device.gsm.usd_call_data, rx_buffer,
					sizeof(Device.gsm.usd_call_data));
			goto end;
		}
		Device.gsm.at_data_flag = 1;
		memset(Device.gsm.at_data, 0, sizeof(Device.gsm.at_data));
		memcpy(Device.gsm.at_data, rx_buffer, sizeof(Device.gsm.at_data));
end:
		memset(rx_buffer, 0, sizeof(rx_buffer));
		counter = 0;
		AT91C_BASE_US0->US_CR = AT91C_US_STTTO;
		return;
	}
}
//-----------------------------------------------------------------------------------
unsigned char SendATWResp (char *cmd,char *resp)
{
	printf("AT CMD: %s \r\n", cmd);
	memset(Device.gsm.at_data, 0, sizeof(Device.gsm.at_data));
	unsigned int local_timeout=0;

	while (!(*(cmd) == '\r'))
	{
	  *AT91C_WDTC_WDCR = 0xA5000001;
	  local_timeout = Device.sys_time;
	  while ((AT91C_BASE_US0->US_CSR & AT91C_US_TXEMPTY) == 0)
	  {
		  if (Device.sys_time > (local_timeout + 3)) // таймайт в секундах
		  {
                    *AT91C_WDTC_WDCR = 0xA5000001;
		  			printf("AT91C_BASE_US0 timeout\r\n");
		  			return 10; // Если можем ничего неоветил выбрасываемся по таймаутом
		  }
	  };
	  AT91C_BASE_US0->US_THR = *cmd;
	  cmd++;
	}
	Device.gsm.at_data_flag = 0;
	while ((AT91C_BASE_US0->US_CSR & AT91C_US_TXEMPTY) == 0)
		;
	AT91C_BASE_US0->US_THR = '\r';
	while ((AT91C_BASE_US0->US_CSR & AT91C_US_TXEMPTY) == 0)
		;
	AT91C_BASE_US0->US_THR = '\n';

	local_timeout = Device.sys_time;
	while(!Device.gsm.at_data_flag)
	{
		if (Device.sys_time > (local_timeout + 5)) // таймайт в секундах
		{
                  *AT91C_WDTC_WDCR = 0xA5000001;
			printf("function timeout\r\n");
			return 10; // Если можем ничего неоветил выбрасываемся по таймаутом
		}
	}
	if (strstr(Device.gsm.at_data,resp) != 0)
	{
		  return 1;
	}
	else
	{
		printf("Error. Response %s \r\n",resp);
		return 0;
	}

}
//------------------------------------------------------------------------------------
void SetAPN (char *APN)
{
	char tmpstring[256];
	sprintf(tmpstring, "AT+CIPCSGP=1,\"%s\"\r", APN);
	SendATWResp(tmpstring,"OK");
}
//-----------------------------------------------------------------------------------
/*
 *  Чтение IMEI дентификатора GSM модуля
 *  Возвращяет 1 при успешном выполнении
 */
int get_imei(char * IMEI)
{
	if (SendATWResp("AT+GSN\r","OK") == 1)
	{
		memcpy(IMEI, Device.gsm.at_data + 2, 15);
		return 1;
	}
	return 0;
}
//-----------------------------------------------------------------------------------
/*
 *  Функция чтения уровня сигнала в GSM/GPRS модуле
 *  Возвращяет 1 при успешном выполнении
 */
int get_csq(char * CSQ)
{
	char tmp[4];
	if (SendATWResp("AT+CSQ\r", "OK") == 1)
	{
		memcpy(tmp, Device.gsm.at_data + 7, 3);
		*CSQ = atoi(tmp);
		return 1;
	}
	return 0;
}
//-----------------------------------------------------------------------------------
int enable_gprs(void)
{
	if (SendATWResp("AT+CGATT=1\r", "OK") == 1)
	{
		return 1;
	}
	return 0;
}
//-----------------------------------------------------------------------------------
/*
 * Определение состояния TCP/IP стека GSM модуля
 */
int tcp_status(void)
{
        SendATWResp("AT+CIPSTATUS\r","OK");
	if (strstr(Device.gsm.at_data, "STATE: IP INITIAL"))
		return IP_INITIAL;
	if (strstr(Device.gsm.at_data, "STATE: IP START"))
		return IP_START;
	if (strstr(Device.gsm.at_data, "STATE: IP CONFIG"))
		return IP_CONFIG;
	if (strstr(Device.gsm.at_data, "STATE: IP IND"))
		return IP_IND;
	if (strstr(Device.gsm.at_data, "STATE: IP GPRSACT"))
		return IP_GPRSACT;
	if (strstr(Device.gsm.at_data, "STATE: IP STATUS"))
		return IP_STATUS;
	if (strstr(Device.gsm.at_data, "STATE: TCP CONNECTING"))
		return TCP_UDP_CONNECTING;
	if (strstr(Device.gsm.at_data, "STATE: IP CLOSE"))
		return IP_CLOSE;
	if (strstr(Device.gsm.at_data, "STATE: CONNECT OK"))
		return CONNECT_OK;
	if (strstr(Device.gsm.at_data, "STATE: PDP DEACT"))
		return PDP_DEACT_PDP;
	return 0xFF;
}
//-----------------------------------------------------------------------------------
/*
 * 
 */
int send_data_server(unsigned char *buf, int size)
{
	char tmp_buf[32];
	int send_data_count = 0;
	memset(Device.gsm.at_data,0,sizeof(Device.gsm.at_data));
	sprintf(tmp_buf, "AT+CIPSEND=%d\r", size);
	if (SendATWResp(tmp_buf,">") != 1)
	{
		printf("Data transfer error not received > \r\n");
		return 0;
	}

	for (send_data_count = 0; send_data_count < size+1; send_data_count++)
	{
			while ((AT91C_BASE_US0->US_CSR & AT91C_US_TXEMPTY) == 0);
			AT91C_BASE_US0->US_THR = *buf++;
	}
	return 1;
}
//-----------------------------------------------------------------------------------
int connect_to_server(char * Server, unsigned int Port)
{
	char tmp_buff[128];
	sprintf(tmp_buff, "AT+CIPSTART=\"tcp\",\"%s\",\"%d\"\r", Server, Port);

	if (SendATWResp(tmp_buff,"OK") == 1)
		return 1;
	else
	    return 0;
}
//-----------------------------------------------------------------------------------
unsigned char gsm_networt_avalible(void)
{
	if (SendATWResp("AT+CREG?\r","+CREG: 0,1") == 1)
		return  1;
	else
		return 0;
}
//-----------------------------------------------------------------------------------
void ReqCSD(char * byte)
{
	unsigned int local_timeout=0;
	printf("AT ReqCSD:%s \r\n", byte);

	while (!(*(byte) == '\r'))
	{
		local_timeout = Device.sys_time;
		while ((AT91C_BASE_US0->US_CSR & AT91C_US_TXEMPTY) == 0)
		{
			if (Device.sys_time > (local_timeout + 5)) // таймайт в секундах
			{
				printf(" ReqCSD function timeout\r\n");
				return ; // Если можем ничего неоветил выбрасываемся по таймаутом
			}
		};
		AT91C_BASE_US0->US_THR = *byte;
		byte++;
	}
	Device.gsm.at_data_flag = 0;
	while ((AT91C_BASE_US0->US_CSR & AT91C_US_TXEMPTY) == 0)
		;
	AT91C_BASE_US0->US_THR = '\r';
	while ((AT91C_BASE_US0->US_CSR & AT91C_US_TXEMPTY) == 0)
		;
	AT91C_BASE_US0->US_THR = '\n';
	Device.gsm.wait_csd_flag = 8;
}
//-----------------------------------------------------------------------------------
