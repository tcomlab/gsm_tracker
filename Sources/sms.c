/*
 * sms.c
 *
 *  Created on: 03.07.2010
 *      Author: Admin
 */

#include "main.h"
#include "system.h"
#include "gsm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//-------------------------------------------------------------------------------
extern SDeviceStatus Device;
//-------------------------------------------------------------------------------
void sms_requvest_sheluder(void)
{
	char tmp_string[32];
	Device.gsm.sms_data_flag = 0;
	int sms_in_modem = 0;
	sscanf(Device.gsm.sms_data + 2, "+CMTI: \"SM\",%d", &sms_in_modem);
	while (sms_in_modem)
	{
		sprintf(tmp_string, "AT+CMGR=%d\r", sms_in_modem);
		SendATWResp(tmp_string,"");
		if (Device.setting.shared_relay_control)
		{
			if (strstr(Device.gsm.at_data, Device.setting.relay1_on_phraze))
				RELAY1_ON;
			if (strstr(Device.gsm.at_data, Device.setting.relay1_off_phraze))
				RELAY1_OFF;
			if (strstr(Device.gsm.at_data, Device.setting.relay2_on_phraze))
				RELAY2_ON;
			if (strstr(Device.gsm.at_data, Device.setting.relay2_off_phraze))
				RELAY2_OFF;
		}
		else
		{
			if (strstr(Device.gsm.at_data, Device.setting.director_namber))
			{
				if (strstr(Device.gsm.at_data, Device.setting.relay1_on_phraze))
					RELAY1_ON;
				if (strstr(Device.gsm.at_data, Device.setting.relay1_off_phraze))
					RELAY1_OFF;
				if (strstr(Device.gsm.at_data, Device.setting.relay2_on_phraze))
					RELAY2_ON;
				if (strstr(Device.gsm.at_data, Device.setting.relay2_off_phraze))
					RELAY2_OFF;
			}
		}
		sprintf(tmp_string, "AT+CMGD=%d\r", sms_in_modem);
		SendATWResp(tmp_string,"");
		sms_in_modem--;
	}
}
//-------------------------------------------------------------------------------
void usd_sheluder(void)
{
	//unsigned char tmp[70];
	Device.gsm.usd_data_flag = 0;
	memcpy(Device.usd_ballans_mesage, Device.gsm.usd_call_data + 10, 64);
	Device.gsm.wait_csd_flag = 1;
	//tmp[0] = 0x14;
	//memcpy(tmp + 1, Device.usd_ballans_mesage, 64);
	//if (Device.gsm.connecting == 3)
	//	send_data_server(tmp, 68);
}
//-------------------------------------------------------------------------------
int send_sms(char *number, char *message)
{
	char tmp[512];
	sprintf(tmp, "AT+CMGS=\"%s\"", number);
	if (SendATWResp(Device.gsm.at_data, ">"))
	{
		memset(tmp, 0, 512);
		sprintf(tmp, "%s \x1A",message);
		SendATWResp(tmp,"");
		return 1;
	}
	return 0;
}
//-------------------------------------------------------------------------------
