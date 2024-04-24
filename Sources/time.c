/*
 * io.c
 *
 *  Created on: 01.02.2010
 *      Author: Admin
 */
#include "system.h"
#include "main.h"
//--------------------------------------------------------------------------------------------------
extern SDeviceStatus Device;
//---------------------------------------------------------------------------------------------------
#define TIMER_PRESCALER 95
void timer0_c_irq_handler(void) // Секундный таймер
{
	static int time = TIMER_PRESCALER;
	AT91PS_TC TC_pt = AT91C_BASE_TC0;
	unsigned int dummy;
	dummy = TC_pt->TC_SR;
	dummy = dummy;
	Device.sys_time_msec++;
	if (--time == 0)
	{
		time = TIMER_PRESCALER;
		Device.sys_time++;
	}
}
// --------------------------------------------------------------------------------------------
