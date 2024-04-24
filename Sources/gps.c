/*
 * gps.c
 *
 *  Created on: 04.01.2010
 *      Author: Taras
 *      COMPLITE!!!!!
 */
#include "main.h"
#include "gps.h"
//#include "usart.h"
//#include "aic.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "system.h"
#include "crc.h"
//----------------------------------------------------------------------------------------------
//#define NMEA_RAW_SIZE 128
char nmea_raw[256];
#define PLENGTH 16
extern SDeviceStatus Device;
//-----------------------------------------------------------------------------------------------
void gps_irq_read_data(void)
{
	unsigned int status;
	static int counter = 0;
	char data;
	status = AT91C_BASE_US1->US_CSR;
	if (status & AT91C_US_RXRDY)
	{
		data = AT91C_BASE_US1->US_RHR;
		if (data == '\n')
			Device.gps_data_redy = 1;
		if (data == '$')
			counter = 0;
		if (counter >= sizeof(nmea_raw))
			counter = 0;
		nmea_raw[counter++] = data;
	}
}
//----------------------------------------------------------------------------------------------
char gps_calcchecksum(const char * s) // to на всякий случай
{
	s++;
	char c = *s++;
	while (*s != '*' && *s != '\0')
		c ^= *s++;
	return c;
}
//-----------------------------------------------------------------------------------------------
static void convert_data(double lat, double lon, char ew, char ns)
{
	double drob, M1, M2, M3;
	int celaya, tmp;
	double test1, test2;
	celaya = (int) lat;
	drob = lat - celaya;
	M1 = drob / 60;
	tmp = 0;
	M2 = 0;
	while (celaya >= 1000)
	{
		celaya = celaya - 1000;
		tmp++;
	}
	M2 = tmp * 10;
	tmp = 0;
	while (celaya >= 100)
	{
		celaya = celaya - 100;
		tmp++;
	}
	M2 = M2 + tmp;
	M3 = 0;
	tmp = 0;
	while (celaya >= 10)
	{
		celaya = celaya - 10;
		tmp++;
	}
	M3 = tmp * 10;
	tmp = 0;
	while (celaya >= 1)
	{
		celaya = celaya - 1;
		tmp++;
	}
	M3 = M3 + tmp;
	M3 = M3 / 60;
	test1 = M2 + M3 + M1;
	Device.gps.lat = test1;// M2 + M3 + M1;
	if (ns == 'S')
		Device.gps.lat = Device.gps.lat * (-1);
	celaya = (int) lon;
	drob = lon - celaya;
	M1 = drob / 60;
	tmp = 0;
	M2 = 0;
	while (celaya >= 1000)
	{
		celaya = celaya - 1000;
		tmp++;
	}
	M2 = tmp * 10;
	tmp = 0;
	while (celaya >= 100)
	{
		celaya = celaya - 100;
		tmp++;
	}
	M2 = M2 + tmp;
	M3 = 0;
	tmp = 0;
	while (celaya >= 10)
	{
		celaya = celaya - 10;
		tmp++;
	}
	M3 = tmp * 10;
	tmp = 0;
	while (celaya >= 1)
	{
		celaya = celaya - 1;
		tmp++;
	}
	M3 = M3 + tmp;
	M3 = M3 / 60;
	test2 = M2 + M3 + M1;
	Device.gps.lon = test2;
	if (ew == 'W')
		Device.gps.lon = Device.gps.lon * (-1);
}
//-----------------------------------------------------------------------------------------------
void gps_sheluder(void)
{
	Device.gps_data_redy = 0;
	char ns, ew;
	double lat, lon;
	int i = 0;
	int j = 0;
	int field = 0;
	char done_flag = 0;
	char temp[PLENGTH];
        
        if (strstr(nmea_raw,"GPRMC") != 0)
        {
	  while (!done_flag)
	  {
		while (nmea_raw[i] != ',')
		{
			temp[j] = nmea_raw[i];
			i++;
			j++;
		}
		temp[j] = '\0';
		j = 0;
		i++;

		switch (field)
		{
		case 0:
			break;
		case 1:
			strncpy(Device.gps.GEO_time, temp, 6);
			break;
		case 2:
			//Device.gps.status = *temp;
			break;
		case 3:
			lat = atof(temp);
			break;
		case 4:
			ns = *temp;
			break;
		case 5:
			lon = atof(temp);
			break;
		case 6:
			ew = *temp;
			break;
		case 7:
			Device.gps.speed = (atof(temp) * 1.852);
			break;
		case 8:
			Device.gps.cog = atof(temp);
			break;
		case 9:
			memcpy(Device.gps.GEO_time + 6, temp, 6);
			break;
		default:
			done_flag = 1;
			convert_data(lat, lon, ew, ns);
			Device.gps.CRC = CRC((unsigned char*)&Device.gps,sizeof(Device.gps)-2);
			break;
		}
		field++;
	  }
	}
        else
        {
        if (strstr(nmea_raw,"$GPGGA") != 0)
        {
          while (!done_flag)
	  {
		while (nmea_raw[i] != ',')
		{
			temp[j] = nmea_raw[i];
			i++;
			j++;
		}
		temp[j] = '\0';
		j = 0;
		i++;

		switch (field)
		{
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			Device.gps.status = (char)atof(temp);
			break;
		case 7:
			Device.io.satelites = (unsigned char)atof(temp);
			break;
		case 8:
			break;
		case 9:
			break;
		default:
			done_flag = 1;
			break;
		}
		field++;
	  }
        }
          
          
        }
}
//-----------------------------------------------------------------------------------
// TODO: Зделать высмыкивание данных о количестве спутников

