#include <stdio.h>
#include "stm32f1xx.h"
#include "usbCDC.h"
#include <strings.h>
#include <string.h>
#include "hal-m010.h"
#include "scpi.h"
#include "keepalive.h"
#include "timers.h"
#include "globals.h"

#define USB_MAX_TX_LEN USB_TXLEN

#define processorName STM32F103RET6
// 						   not C8Tx

// The current clock frequency
uint32_t SystemCoreClock=8000000;

static void rstMain();
static void blinkDbgLed();
static void init_clock();
static void init_io();


int main()
{
    /*** Initializations	***/
	init_clock();
	init_io();

	systick_count=0;

	// Initialize system timer
	SysTick_Config(SystemCoreClock/1000);

	initHal();
	initKeepalive();
	usbCdcInit();
	keepaliveClrRanout();
	initDACs();
	rstMain();
    blinkDbgLed();


    /*** \Initializations	***/


    while (1)
    {	char usbInBuf[USB_RXLEN] = "";

        if (usbCdcRxBufferContains('\n'))
        {   usbCdcRxDequeue(usbInBuf,sizeof(usbInBuf),'\n');
            int scpiID = getScpiIdfromMsg(usbInBuf);

            switch(scpiID)
            {
            	case	RX_IDNq:		usbEnq( scpiTxMsgs[TX_IDNq] );          							break;
             	case	RX_RST:			rstMain();										            		break;
            	case	RX_REL0_ON:		setRelay0();         												break;
				case	RX_REL0_OFF:	clrRelay0();    					     							break;
				case	RX_REL0q: 		usbEnqStrs(2, subSyses[SYSID_RELAY0], getRelay0() ? scpiTxMsgs[TX_ON] : scpiTxMsgs[TX_OFF] );
					break;
				case	RX_REL1_ON: 	setRelay1();         												break;
				case	RX_REL1_OFF: 	clrRelay1();      					 						  		break;
				case	RX_REL1q:		usbEnqStrs(2, subSyses[SYSID_RELAY1], getRelay1() ? scpiTxMsgs[TX_ON] : scpiTxMsgs[TX_OFF] );
					break;

			// VOUT / DACs
				case	RX_VOUTA_VOLT:		writeDacVolts(1, getFloatFromStr(usbInBuf)) ? usbEnq(scpiTxMsgs[TX_PARAM_OK ]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR ]);	break;
				case	RX_VOUTB_VOLT:		writeDacVolts(2, getFloatFromStr(usbInBuf)) ? usbEnq(scpiTxMsgs[TX_PARAM_OK ]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR ]);	break;

				case	RX_VOUTA_VOLTq:		usbCdcTxEnqueueStrsFloat(readDacVolts(1), 1, "VOUTA:");			break;
				case	RX_VOUTB_VOLTq:		usbCdcTxEnqueueStrsFloat(readDacVolts(2), 1, "VOUTB:");			break;

				// TimerA
				case	RX_TIMA_FREQ:	setTimerA(1/getFloatFromStr(usbInBuf))?	usbEnq(scpiTxMsgs[TX_PARAM_OK]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR]);		break;
				case	RX_TIMA_FREQq:	usbCdcTxEnqueueStrsFloat(1/(2*getTimerPeriod((TIM2->PSC), (TIM2->ARR))), 2, subSyses[SYSID_TIMERA], scpiTxMsgs[TX_FREQUENCY]);
					break;

			// Keepalive
				case	RX_KA_peri:		keepaliveSetPeriod(getFloatFromStr(usbInBuf)) ? usbEnq(scpiTxMsgs[TX_PARAM_OK]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR]);
					break;
				case	RX_KA_on:   	keepaliveStart() ? usbEnqStrs(2, subSyses[SYSID_KEEPALIVE], scpiTxMsgs[TX_STARTED]) : usbEnqStrs(2, subSyses[SYSID_KEEPALIVE], scpiTxMsgs[TX_NOT_STARTED]);
					break;
				case	RX_KA_off:   	keepaliveStop();
										usbEnqStrs(2, subSyses[SYSID_KEEPALIVE], scpiTxMsgs[TX_STOPPED]);
					break;
				case	RX_KA_up:   	keepaliveReset() ? usbEnqStrs(2, subSyses[SYSID_KEEPALIVE], scpiTxMsgs[TX_RESET]) : usbEnqStrs(2, subSyses[SYSID_KEEPALIVE], scpiTxMsgs[TX_NOT_RESET]);
					break;
				case	RX_KAq:			usbCdcTxEnqueueStrsFloat(getTimerPeriod(TIM1->PSC, TIM1->ARR), 3, subSyses[SYSID_KEEPALIVE], keepaliveGetState() ? scpiTxMsgs[TX_ON] : scpiTxMsgs[TX_OFF], scpiTxMsgs[TX_PERIOD]);
					break;
			// \Keepalive

			// Debug
				case	RX_DBG_BLINK:	blinkDbgLed();														break;

				case	RX_DBG_CPUID:
						usbCdcTxEnqueueStrsInt( ID_GetUnique32(0) + ID_GetUnique32(1) + ID_GetUnique32(2) + ID_GetUnique32(3), 1, "sum of CPU IDs: ") ;
					break;

				case	RX_DBG_dacVoltToRaw:    usbCdcTxEnqueueStrsInt( dacVoltToRaw(getFloatFromStr(usbInBuf)), 1, "raw: " );  break;
				case	RX_DBG_dacRawToVolt:    usbCdcTxEnqueueStrsFloat( dacRawToVolt(getIntFromStr(usbInBuf)), 1, "volts: "); break;

			// \Debug
				// case	:   ;         		break;
				default:				usbEnq(scpiTxMsgs[TX_ERR]);													break;
            }	// \switch

        }

        if(keepaliveGetRanout())
        {
        	keepaliveClrRanout();
        	keepaliveStop();
        	rstMain();
        	usbEnqStrs(2 , subSyses[SYSID_KEEPALIVE], scpiTxMsgs[TX_OVERFLOW]);
        }

		// invalidate usbInBuf;
		usbInBuf[0] = 0;
		usbTxFlush();

    }	// \while
    // if eternity should fail:

    deinitKeepalive();
    deinitHal();
}


static void rstMain()
{
	keepaliveStop();

	writeDacVolts(1, 0.0);
	writeDacVolts(2, 0.0);
	
	clrRelay0();
	clrRelay1();

}

// Interrupt handler
void SysTick_Handler(void)	{	systick_count++;	}

// Delay some milliseconds
void delay_ms(int ms)
{
    uint32_t start=systick_count;
    while (systick_count-start<ms);
}

// Change system clock to 48Mhz using 8Mhz crystal
static void init_clock()
{   SET_BIT(RCC->CR, RCC_CR_HSION);
    while(!READ_BIT(RCC->CR, RCC_CR_HSIRDY)) {}
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_HSI);
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_HSI) {}
    CLEAR_BIT(RCC->CR, RCC_CR_PLLON);
    while(READ_BIT(RCC->CR, RCC_CR_PLLRDY)) {}
    MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_0);
    SET_BIT(RCC->CR, RCC_CR_HSEON);
    while(!READ_BIT(RCC->CR, RCC_CR_HSERDY)) {}
    WRITE_REG(RCC->CFGR, RCC_CFGR_PLLSRC + RCC_CFGR_PLLMULL6 + RCC_CFGR_PPRE1_DIV2);
    SET_BIT(RCC->CR, RCC_CR_PLLON);
    while(!READ_BIT(RCC->CR, RCC_CR_PLLRDY)) {}
    MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);
    SystemCoreClock=48000000;
    MODIFY_REG(RCC->CFGR, RCC_CFGR_USBPRE, RCC_CFGR_USBPRE);
    // Enable clock for alternate functions
    // SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);
}

static void init_io()
{
    // Enable USB
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USBEN);
}



// Redirect standard output to the USB port, e.g. from puts() or printf()
int _write(int file, char *ptr, int len)
{
    return UsbSendBytes(ptr, len, 10);
}



static void blinkDbgLed()
{
	setDbgLED();	delay_ms(200);	clrDbgLED();	delay_ms(200);
	setDbgLED();	delay_ms(200);	clrDbgLED();	delay_ms(200);

	setDbgLED();	delay_ms(75);	clrDbgLED();	delay_ms(75);
	setDbgLED();	delay_ms(75);	clrDbgLED();	delay_ms(75);
	setDbgLED();	delay_ms(75);	clrDbgLED();	delay_ms(75);
	setDbgLED();	// delay_ms(100);	clrDbgLED();	delay_ms(100);
	setDbgLED();

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}
