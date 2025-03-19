/*
 * keepalive.h
 *
 *  Created on: 08.02.2023
 *      Author: Florian Hinterleitner
 */

#ifndef KEEPALIVE_H_
#define KEEPALIVE_H_

#include "timers.h"
#include "globals.h"

#define STD_KEEPALIVE_PERIOD 1 // second

bool initKeepalive();
bool keepaliveSetPeriod(float period);
bool keepaliveStart();
void keepaliveStop();
bool keepaliveReset();
bool keepaliveGetState();
void deinitKeepalive();

void keepaliveSetRanout();
bool keepaliveGetRanout();
void keepaliveClrRanout();

void keepaliveDelayMs(int ms)
{
    uint32_t ka_start=systick_count;
    while (systick_count-ka_start<ms);
}




bool initKeepalive()
{	keepaliveClrRanout();
	tim1SetRegUpdating();

	int psc = getTimerPSC(STD_KEEPALIVE_PERIOD);
	int arr = getTimerARR(STD_KEEPALIVE_PERIOD, psc);
	initTIM(TIM1, psc, arr);
    setTIM(TIM1, STD_KEEPALIVE_PERIOD);
    resetTIM(TIM1);
    updateEventTIM(TIM1);
    keepaliveDelayMs(2);
    // delay_ms(2);
    tim1ClrRegUpdating();
    return true;
}

/*
 * @brief	set keepalive timeout in seconds
 *
 * */

bool keepaliveSetPeriod(float period)
{	tim1SetRegUpdating();
	bool resetted = setTIM(TIM1, period);
	resetTIM(TIM1);
    updateEventTIM(TIM1);
    keepaliveDelayMs(2);
    tim1ClrRegUpdating();
    return resetted;
}

bool keepaliveStart()
{
	return startTIM(TIM1);
}

void keepaliveStop()
{
	stopTIM(TIM1);
}

bool keepaliveReset()				{	return resetTIM(TIM1);				}
bool keepaliveGetState()			{	return getTimerState(TIM1);	}
void keepaliveSetRanout()
{

tim1SetRanout();
//	keealiveRanOut = true;
}
bool keepaliveGetRanout()
{

return tim1GetRanout();
//	return keealiveRanOut;
}
void keepaliveClrRanout()
{

tim1ClrRanout();

//	keealiveRanOut = false;
}




void deinitKeepalive()				{	deinitTIM1();				}


#endif /* KEEPALIVE_H_ */
