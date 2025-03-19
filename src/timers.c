/*
 * timers.c
 *
 *  Created on: 17.02.2023
 *      Author: r.hif
 */


#include "stm32f1xx.h"
#include "stdbool.h"
//#include "timers.h"

#define	TIM_MIN_PERIOD 0.00001
#define	TIM_MAX_PERIOD 10	// seconds


#define	TIMS_MANUAL_ISR

uint32_t getSysClk()	{	return SystemCoreClock;	}
uint32_t getP1TimClk()	{	return getSysClk();		}

uint32_t getTimerPSC(float period)
{	uint32_t psc = 0;

	// implicit sanity-check: 4us > period > 50s leaves psc to '0'
	if(   4.00E-06 	<= period && period <  40.00E-06)	psc =  1 ;  //  t max  83.33E-09  min t,    2.73E-03
	if(  40.00E-06 	<= period && period <  10.00E-03)	psc =  9 ;  //  t max 416.67E-09  min t,   13.65E-03
	if(  10.00E-03 	<= period && period < 100.00E-03)	psc =  99 ;  //  t max   4.17E-06  min t,  136.53E-03
	if( 100.00E-03	<= period && period <   1.00E-00 )	psc =  999 ;  //  t max  41.67E-06  min t,    1.37E+00
	if(   1.00E-00	<= period && period <  10.00E-00)	psc =  9999 ;  //  t max 416.67E-06  min t,   13.65E+00
	if(  10.00E-00 	<= period && period <= 50.00E-00)	psc =  65535 ;  //  t max   2.73E-03  min t,   89.48E+00

	return psc;
}

uint32_t getTimerARR(float period, uint32_t psc)
{	uint32_t	arr = 0,
				TCLK = getP1TimClk();
	float periodCLK = period*TCLK;
	float periodCLKbyPSC = periodCLK / (psc+1.0);
	arr = (uint32_t) ( periodCLKbyPSC  ) -1.0;
	return arr;
}

float getTimerPeriod(uint32_t psc, uint32_t arr)
{	return 	(float)((psc+1)*(arr+1))/(getP1TimClk());
}

bool	getTimerState(TIM_TypeDef * TIMx)
{	return READ_BIT(TIMx->CR1, TIM_CR1_CEN);
}

bool initTIM(TIM_TypeDef * TIMx, int psc, int arr)
{
	if( TIM1 == TIMx )
	{	RCC -> APB2ENR |= RCC_APB2ENR_TIM1EN;       // enable TIM2 clock
		NVIC_EnableIRQ( TIM1_UP_IRQn);
	}

	TIMx -> PSC = psc;							// set prescaler
	TIMx -> DIER |= TIM_DIER_UIE;				// enable update interrupt
	TIMx -> ARR = arr;							// set auto reload register
	TIMx -> CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;	// autoreload on, counter enabled
	TIMx -> EGR = 1;                  			// trigger update event to reload timer registers
	CLEAR_BIT(TIMx->CR1, TIM_CR1_CEN);
	SET_BIT(TIMx->EGR, TIM_EGR_UG); // ...update event

	return true;
}

bool stopTIM(TIM_TypeDef * TIMx)
{	CLEAR_BIT(TIMx->CR1, TIM_CR1_CEN);
	CLEAR_REG(TIMx->CNT);
	return false == READ_BIT(TIMx->CR1, TIM_CR1_CEN);
}

bool startTIM(TIM_TypeDef * TIMx)
{	stopTIM(TIMx);
	SET_BIT(TIMx->CR1, TIM_CR1_CEN);
	return 	true == READ_BIT(TIMx->CR1, TIM_CR1_CEN);
}

bool resetTIM(TIM_TypeDef * TIMx)
{	CLEAR_REG(TIMx->CNT);
	return 0x00 == TIMx->CNT;
}


void updateEventTIM(TIM_TypeDef * TIMx)
{	SET_BIT(TIMx->EGR, TIM_EGR_UG); // ...update event
}


bool setTIM(TIM_TypeDef * TIMx, float period)
{
	if( TIM_MIN_PERIOD <= period && period <= TIM_MAX_PERIOD )//
	{
		int psc = getTimerPSC(period);
		int arr = getTimerARR(period, psc);

		TIMx -> PSC = psc;
		TIMx -> ARR = arr;

		return true;
	}else{
		return false;
	}
}

bool tim1RegUpdating = false;
void tim1SetRegUpdating()			{	tim1RegUpdating = true;		}
bool tim1GetRegUpdating()			{	return tim1RegUpdating;		}
void tim1ClrRegUpdating()			{	tim1RegUpdating = false;	}

bool tim1RanOut = false;
void tim1SetRanout()
{	if( !tim1GetRegUpdating() )
	tim1RanOut = true;
}
bool tim1GetRanout()			{	return tim1RanOut;		}
void tim1ClrRanout()			{	tim1RanOut = false;	}
