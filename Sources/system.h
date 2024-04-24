/*
 * system.h
 *
 *  Created on: 11.04.2010
 *      Author: Admin
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#pragma pack(push, 2)
typedef struct TDeviceSetting
{
	unsigned char settingID; // 0x11 ID setting
	char apn[30]; // GPRS Access point
	char apnuser[30]; // GPRS Access user point name
	char apnpswd[30]; // GPRS Access point password
	unsigned int send_geo_data_to_server_time; // Send data interval GEO second
	unsigned int send_io_data_to_server_time; // Interval send data IO second
	char main_server_ip[20]; // IP main server
	int main_server_port; // Server port
	char led_status;
	char usd_cash[8];
	char director_namber[18];
	char relay1_on_phraze[12];
	char relay1_off_phraze[12];
	char relay2_on_phraze[12];
	char relay2_off_phraze[12];
	char shared_relay_control;
	unsigned short CRC;
} DeviceSetting;
#pragma pack(pop)

//#pragma pack(push, 1)
typedef struct SystemIO
{
	unsigned char IDIO;
	unsigned short ch1;
	unsigned short ch2;
	unsigned short ch3;
	unsigned short ch4;
	unsigned short ch5;
	unsigned short ch6;
	unsigned short vbort;
	unsigned short vbat;
	unsigned char flags;
	char IO_time[12];
	char satelites;
	char csq;
	unsigned short CRC;
} SIO;
//#pragma pack(pop)

#pragma pack(push,1)
typedef struct SystemGPS
{
	unsigned char IDGPS;
	char GEO_time[12]; // 13 time of GPS data query hhmmss.sss
	char status; // 1 A=valid V=invalid
	double lat; // 8 lattitude ddmm.mmmm
	double lon; // 8 longintude dddmm.mmmm
	double speed; // 8 speed over ground knots
	double cog; // 8 course over ground degrees
	unsigned short CRC;
} SGPS;
#pragma pack(pop)
//------------------------------------------------------------------------------------
typedef struct SystemGSM
{
	unsigned char IDGSM;
	unsigned char sms_data_flag;
	unsigned char usd_data_flag;
	unsigned char call_data_flag;
	unsigned char gprs_data_flag;
	unsigned char at_data_flag;
	unsigned char call_ready;
	unsigned char gprs_active;
	char gprs_data[256];
	char sms_data[128];
	char usd_call_data[128];
	char at_data[128];
	unsigned char wait_csd_flag;
} SGSM;
//-----------------------------------------------------------
//unsigned char modem_status; - DEIFNITIONS
#define NO_REDY 0
#define GPRS_OK 1

//-----------------------------------------------------------
typedef struct Database
{
 unsigned short current_block; // номер следующего пустого блока
 unsigned short rec_index;
}SDB;
//-----------------------------------------------------------
//#pragma pack(push,8)
typedef struct SystemDevice
{
	DeviceSetting setting;
	SGSM gsm;
	SGPS gps;
	SIO io;
	char IMEI[16];
	SDB Database;
	unsigned int sys_time;
	unsigned int sys_time_msec;
	unsigned char gps_data_redy;
	unsigned char power_status;
	char usd_ballans_mesage[512];
} SDeviceStatus;
//#pragma pack(pop)
//------------------------------------------------------------------------------------
#define SERVER_DATA_RX 0x01
#define SMS_DATA_RX    0x02
#define INCOMING_CALL  0x03
#define INCOMING_USD   0x04
#define AT_CMD         0x05
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
#endif /* SYSTEM_H_ */
