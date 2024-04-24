/*
 * db.c
 *
 *  Created on: 25.10.2010
 *      Author: Admin
 *
 *   ратенький дескриптор устройки базы данных
 *  база данных состоит из отресков - кластеров
 *  каждый кластер шаблонизирован
 *  дл€ определени€ какой шаблон использовать
 *  нужна найти сивол начала кластера
 *  =2 байта будет тип пакета
 *  исхолд€ из типа пакета добавить длинну и получить конец кластера который заканчиваетс€ символом
 *  окончани€ кластера
 *
 *
 */


#include "db.h"
#include "system.h"
#include "at45.h"
#include <stdio.h>
#include <string.h>
#include "main.h"
//---------------------------------------------------------------------------
typedef struct DBRecort
{
  unsigned short index;
  unsigned char  len;
  unsigned char data[252];
}DBR;

//---------------------------------------------------------------------------
extern SDeviceStatus Device;
//unsigned char DBRXBuffer[1024];

unsigned int byte_counter=0;
//---------------------------------------------------------------------------
void save_geo_in_db(SGPS *gps)
{

}
//---------------------------------------------------------------------------
void save_io_in_db(SIO *io)
{

}
//---------------------------------------------------------------------------
void format_flash (void)
{
	int i,b,blink=200;
	for (i=0;i<8191;i++)
	{
	  for (b=0;b<512;b++)Buffer_Write_Byte(2,b,0xFF);
	  wait_DF_busy();
	  Buffer_To_Page(2,i);
	  if (blink>5) {STATUS_LED1_ON;}
	  if (blink>10) {STATUS_LED1_OFF;blink=0;}
	  blink++;
	}
}
//---------------------------------------------------------------------------
unsigned short search_last_index(void)
{
	//format_flash();
	unsigned short index = 0, i = 0, b = 0, buf_switch = 0,
			claster_pointer = 0, fault_serch = 0;
	unsigned char buffer[1024];
	
	for (i = 1; i < 8191; i++)
	{
		STATUS_LED1_ON;
		if (fault_serch > 10)
			return index;
                wait_DF_busy();
		Page_To_Buffer(i,1);
		wait_DF_busy();
		for (b = 0; b < 512; b++)
		{
			if (buf_switch == 0)
				buffer[b] = Buffer_Read_Byte(1, b);
			if (buf_switch == 1)
				buffer[b + 512] = Buffer_Read_Byte(1, b);
		}
		if (buf_switch == 0)
			buf_switch = 1;
		else if (buf_switch == 1)
			buf_switch = 0;
                STATUS_LED1_OFF;
		//--------------------------------------------
//		for (b = 0; b < 2048; b++)
//		{
//			if (claster_pointer == 1024)
//				claster_pointer = 0;
//			else
//				claster_pointer++;
//
//			if (buffer[claster_pointer] == 0xA0) // »щем начало кластера
//			{
//				// »шем айди накета данных
//				if ((buffer[claster_pointer + 3] != 0x20)
//						&& (buffer[claster_pointer + 3] != 0x21))
//				{
//					fault_serch++;
//					break;
//				}
//
//				if ((buffer[claster_pointer + 3] == 0x20)
//						&& (buffer[claster_pointer + 3 + sizeof(SGPS) + 1]
//								== 0xA1))
//				{
//					// ¬от она запись о геоданных
//					index = buffer[claster_pointer + 1];
//				}
//
//				if ((buffer[claster_pointer + 3] == 0x21)
//						&& (buffer[claster_pointer + 3 + sizeof(SIO) + 1]
//								== 0xA1))
//				{
//					// ¬от она запись о входах
//					index = buffer[claster_pointer + 1];
//				}
//			}
//		}
		//--------------------------------------------
	}
	return 0;
}
//---------------------------------------------------------------------------
// ƒанна€ функци€ находит крайний блок базы данных и индекс таблицы
//
void init_database(void)
{
	unsigned short index = 0;

	index = search_last_index();

}
//---------------------------------------------------------------------------
