/**
 * @file	hal-m010.c
 * @author	Florian Hinterleitner
 * @date	08.01.2023
 * @brief	provides access to all digital IOs
 *
 */
// #include "hal-m010.h"
#include "globals.h"
#include "stm32f1xx.h"
#include "stdbool.h"
#include "hal-m010.h"
// #include "debugUnit.h"

#define MAX_DAC_VAL			( 4096-1  )
#define MIN_DAC_VOLT		-10.000
#define MAX_DAC_VOLT		10.000


void initDbgLed()	{   MODIFY_REG(GPIOC->CRH, GPIO_CRH_CNF13 + GPIO_CRH_MODE13, GPIO_CRH_MODE13_0);	}
void setDbgLED()	{	WRITE_REG(GPIOC->BSRR, GPIO_BSRR_BS13);	}
void clrDbgLED()	{	WRITE_REG(GPIOC->BSRR, GPIO_BSRR_BR13);	}

void initRelays()
{	//	PB0 , 1
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF0 + GPIO_CRL_MODE0, GPIO_CRL_MODE0_0);
	MODIFY_REG(GPIOB->CRL, GPIO_CRL_CNF1 + GPIO_CRL_MODE1, GPIO_CRL_MODE1_0);

	GPIOB->BRR = 0b0000000000000011; // clear PB0 , 1
}

void setRelay0()	{	SET_BIT(GPIOB->BSRR, GPIO_BSRR_BS0);				}
void clrRelay0()	{	SET_BIT(GPIOB->BRR, GPIO_BRR_BR0);					}
bool getRelay0()	{	return READ_BIT(GPIOB->IDR, GPIO_IDR_IDR0) != 0;	}
void setRelay1()	{	SET_BIT(GPIOB->BSRR, GPIO_BSRR_BS1);				}
void clrRelay1()	{	SET_BIT(GPIOB->BRR, GPIO_BRR_BR1);					}
bool getRelay1()	{	return (READ_BIT(GPIOB->IDR, GPIO_IDR_IDR1) != 0);	}


// VOUTA = PA4 -> DAC_OUT1
// VOUTB = PA5 -> DAC_OUT2
void initDACs()
{	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_DACEN);	// enable DAC Clock

	SET_BIT(DAC->CR, DAC_CR_EN1);
	SET_BIT(DAC->CR, DAC_CR_EN2);

	// ensure trigger is turned off and holding regs directly pushed to our regs:
	CLEAR_BIT(DAC->CR, DAC_CR_TEN1);
	CLEAR_BIT(DAC->CR, DAC_CR_TEN2);

	// ensure output buffer is on
	CLEAR_BIT(DAC->CR, DAC_CR_BOFF1);
	CLEAR_BIT(DAC->CR, DAC_CR_BOFF2);

}


void deinitDACs()
{	CLEAR_BIT(RCC->APB1ENR, RCC_APB1ENR_DACEN);	// disable DAC Clock
}

/*
 * @brief		write raw values to DAC
 * @param[in]	channel chooses between CH1 and CH2
 * @param[in]	dacval	value, between 0 and (2^12)-1,
 * 				to be written into the DAC, corresponds with its analouge voltage
 * @return		'true' if correct inputs were supplied, otherwise 'false'.
 *
 */

bool writeDacRaw(int channel, int dacval)
{	if ( 0 <= dacval && dacval <= MAX_DAC_VAL )
	{	if ( 1 == channel )
		{		DAC->DHR12R1 = dacval;
			//	DAC->DOR1;
		}
		else if ( 2 == channel )
		{		DAC->DHR12R2 = dacval;
			//	DAC->DOR2;
		}
		else
			return false;
		return true;
	}else{
		return false;
	}
}


bool writeDacPercent(int channel, float percent)
{
	if ( 0 <= percent && percent <= 100 )
	{	writeDacRaw(channel, percent*(MAX_DAC_VAL)/100.0);
		return true;
	}else	// error case
		return false;
}


int  readDacRaw(int channel)
{	if ( 1 == channel )
		return	DAC->DOR1;
	else if ( 2 == channel )
		return DAC->DOR2;
	else
		return false;
}

float readDacPercent(int channel)
{	return 100.0*readDacRaw(channel)/(MAX_DAC_VAL);
}

bool writeDacVolts(int channel, float voltage)
{
	if ( MIN_DAC_VOLT <= voltage && voltage <= MAX_DAC_VOLT )
	{	writeDacRaw(channel, dacVoltToRaw(voltage));
		return true;
	}else	// error case
		return false;
}

/*
 * @brief	calculate the raw integer equivalent of a given DAC Voltage
 * 			implements y = k*x + d
 * */
int		dacVoltToRaw(float voltage)
{	return	(MAX_DAC_VAL/2 - voltage*MAX_DAC_VAL/(MAX_DAC_VOLT - MIN_DAC_VOLT));
}

/*
 * @brief	calculate the analogue voltage equivalent of a given DAC raw value
 * */
float	dacRawToVolt(int dacval)
{	return - (dacval - MAX_DAC_VAL/2)*(MAX_DAC_VOLT - MIN_DAC_VOLT)/MAX_DAC_VAL;
}

float  readDacVolts(int channel)
{	return	dacRawToVolt(readDacRaw(channel));
}

void initHal()
{
    // Enable clocks for Port A, B and C
    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN + RCC_APB2ENR_IOPBEN + RCC_APB2ENR_IOPCEN);

	initDbgLed();
	initRelays();

}

