#include <stdio.h>
#include "stm32f1xx.h"
#include "usbCDC.h"
#include <strings.h>
#include <string.h>
#include "debugUnit.h"
#include "hal-m010.h"
#include "scpi.h"
#include "keepalive.h"
#include "timers.h"
#include "globals.h"

#define USB_MAX_TX_LEN USB_TXLEN
// #define TIM_ENC


#define processorName STM32F103RET6
// 						   not C8Tx

// The current clock frequency
uint32_t SystemCoreClock=8000000;

static void rstMain();

// Interrupt handler
void SysTick_Handler(void)
{
    systick_count++;

    // gehoert in eine func gepackt und -> hal-m010.c verschifft
    for(int idx = 0; idx <= (MAX_EXTI_CNT-1); idx++)	// loop over all possible EXTIs
    {
		int pin = 0;
		(9 == idx) ? pin = 0 : pin;
		(10 == idx) ? pin = 1 : pin;

		int nCycles = 5;	// 5 SystTick cycles
		if(getRisingEdges(idx) > 0 )
		{
			if (getRisingCycles(idx) <= nCycles && getPin('B', pin))
				setRisingCycles(idx);
			if( getRisingCycles(idx)> nCycles)
			{
				setRisingEdge(idx);				clrRisingEdges(idx);
			}
		}
		else
		{
			clrRisingCycles(idx);
		}

		if(getFallingEdges(idx) > 0 )
		{
			if (getFallingCycles(idx) <= nCycles && !getPin('B', pin))
				setFallingCycles(idx);
			if( getFallingCycles(idx) > nCycles)
			{
				setFallingEdge(idx);
				clrFallingEdges(idx);
			}
		}
		else
		{	clrFallingCycles(idx);
		}
    }
}

// Delay some milliseconds
void delay_ms(int ms)
{
    uint32_t start=systick_count;
    while (systick_count-start<ms);
}

// Change system clock to 48Mhz using 8Mhz crystal
void init_clock()
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

void init_io()
{
    // Enable USB
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USBEN);

	// obso: is now in HAL
		// Enable Port A, B and C
		// SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN + RCC_APB2ENR_IOPBEN + RCC_APB2ENR_IOPCEN);

}

#ifdef PAT_GEN
	void initPatGen16(){}

	void deinitPatGen16(){}

	void setPatGen16(uint16_t word, uint8_t rest){}


	static void demoPatGen16()
	{	// py to gen 16bit bin-table:
		// for idx in range(2**16):
		//     format(i, "#018b")
	}
#endif // PAT_GEN


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

// Redirect standard output to the USB port, e.g. from puts() or printf()
int _write(int file, char *ptr, int len)
{
    return UsbSendBytes(ptr, len, 10);
}

int main()
{
    /*** Initializations	***/
		init_clock();
		init_io();

		systick_count=0;
		// rstSysTick();

		// Initialize system timer
		SysTick_Config(SystemCoreClock/1000);

		initHal();
		initTimerA();
		initTimerB();
		initTimerC();
		initKeepalive();

		usbCdcInit();

		keepaliveClrRanout();


    /*** \Initializations	***/


		initDACs();

		// writeDacRaw(2, 50);
		writeDacVolts(1, 0.00);
		writeDacVolts(2, 0.00);


	#ifdef DEBUG
//		 presumed sum of cpuIDs = -472074265

		volatile int dbgSumCpuIDs = ID_GetUnique32(0) + ID_GetUnique32(1) + ID_GetUnique32(2) + ID_GetUnique32(3);
		volatile int dbgAddress = 1073809408;



		/*** Testing ground		***/
			// setTIM1(1);
			// startTIM1();
			//		des is woi a scheiss, die Zeile:
			initPatGen16();
			// uint8_t sCount1[50] = {'\0'};
			// uint8_t sCount2[50] = {'\0'};
			startTimerA();
			startTimerB();
			startTimerC();

			// demoPatGen16();
			//	delay_ms(1000);
			setRelay0();
			//	delay_ms(1000);
			setRelay0();
			//	delay_ms(1000);
			clrRelay0();

			//	uint8_t MSG[50] = {'\0'};
			//    HAL_Init();
			//    SystemClock_Config();

	    /*** \Testing ground		***/
		ITM_SendStr(scpiTxMsgs[TX_IDNq]);
	#endif // DEBUG

	rstMain();
    blinkDbgLed();

    while (1)
    {	char usbInBuf[USB_RXLEN] = "";

        if (usbCdcRxBufferContains('\n'))
        {   usbCdcRxDequeue(usbInBuf,sizeof(usbInBuf),'\n');
            int scpiID = getScpiIdfromMsg(usbInBuf);

            switch(scpiID)
            {
            	case	RX_IDNq:		usbEnq( scpiTxMsgs[TX_IDNq] );
            							// ITM_SendStr(scpiTxMsgs[0]);
            		break;
             	case	RX_RST:			rstMain();
            							// ITM_SendStr(scpiTxMsgs[0]);
            		break;
            	case	RX_REL0_ON:		setRelay0();         		break;
				case	RX_REL0_OFF:	clrRelay0();         		break;
				case	RX_REL0q: 		usbEnqStrs(2, subSyses[SYSID_RELAY0], getRelay0() ? scpiTxMsgs[TX_ON] : scpiTxMsgs[TX_OFF] );
					break;
				case	RX_REL1_ON: 	setRelay1();         		break;
				case	RX_REL1_OFF: 	clrRelay1();         		break;
				case	RX_REL1q:		usbEnqStrs(2, subSyses[SYSID_RELAY1], getRelay1() ? scpiTxMsgs[TX_ON] : scpiTxMsgs[TX_OFF] );
					break;

			#ifdef PCBv02
				case	RX_PC_0_ON:		setPin('C', 0);					break;
				case	RX_PC_0_OFF:	clrPin('C', 0);					break;
				case	RX_PC_0q:		usbEnqStrs(3, subSyses[SYSID_PORTC], ":0", getPin('C', 0) ? scpiTxMsgs[TX_ON] : scpiTxMsgs[TX_OFF]) ;		break;
				case	RX_PC_1_ON:		setPin('C', 1);					break;
				case	RX_PC_1_OFF:	clrPin('C', 1);					break;
				case	RX_PC_1q: 		usbEnqStrs(3, subSyses[SYSID_PORTC], ":1", getPin('C', 1) ? scpiTxMsgs[TX_ON] : scpiTxMsgs[TX_OFF]) ;		break;

				// GPIOA9 => PortB0
				case	RX_PB_0_OUT:	deinitExtIrq('B', 0);	initOutput('B', 0);		break;	// initPinOutput
				case	RX_PB_0_IN:		deinitExtIrq('B', 0);	initInput('B', 0);		break;	// initPinInput
				case	RX_PB_0_PULLU:	initPull('B', 0, true);			break;
				case	RX_PB_0_PULLD:	initPull('B', 0, false);			break;
				case	RX_PB_0_NOPULL:	deinitPull('B', 0);				break;
				case	RX_PB_0_RISE:	initInput('B', 0);	initExtIrq('B', 0, true, false);	break;
				case	RX_PB_0_FALL:	initInput('B', 0);	initExtIrq('B', 0, false, true);	break;
				case	RX_PB_0_NOIRQ:	deinitExtIrq('B', 0);
										initOutput('B', 0);	// set pin to output and 0
										clrPin('B', 0);
					break;
				case	RX_PB_0_PULLq:	{	if ( PULLUP == getPullState('B', 0) )				usbEnqStrs(3, subSyses[SYSID_PORTB], ":0",	scpiTxMsgs[TX_PULLUP]);
											if ( PULLDOWN == getPullState('B', 0) )			usbEnqStrs(3, subSyses[SYSID_PORTB], ":0",	scpiTxMsgs[TX_PULLDOWN]);
											if ( NOPULL == getPullState('B', 0) )				usbEnqStrs(3, subSyses[SYSID_PORTB], ":0",	scpiTxMsgs[TX_NOPULL]);
										}
					break;
				case	RX_PB_0_IRQq:		if(	RISING_EDGE		== getExtIrqState('B', 0) )	usbEnqStrs(3, subSyses[SYSID_PORTB], ":0",	scpiTxMsgs[TX_RISINGEDGE]);
											if(	FALLING_EDGE 	== getExtIrqState('B', 0) )	usbEnqStrs(3, subSyses[SYSID_PORTB], ":0",	scpiTxMsgs[TX_FALLINGEDGE]);
											if(	RISE_FALL_EDGE 	== getExtIrqState('B', 0) )	usbEnqStrs(3, subSyses[SYSID_PORTB], ":0",	scpiTxMsgs[TX_RISE_FALL_EDGE]);
											if(	NOIRQ 			== getExtIrqState('B', 0) )	usbEnqStrs(3, subSyses[SYSID_PORTB], ":0",	scpiTxMsgs[TX_NOIRQ]);
					break;
				case	RX_PB_0_STATEq:	{	char usbTxState[ USB_MAX_TX_LEN ] = "";
											strcat(usbTxState, subSyses[SYSID_PORTB]);
											strcat(usbTxState, ":0 State:\n");

											strcat(usbTxState, " Direction:");
											strcat(usbTxState, getIoDirection('B', 0) ? scpiTxMsgs[TX_INPUT] :	scpiTxMsgs[TX_OUTPUT]);
											strcat(usbTxState, "\n Level:");
											strcat(usbTxState, (getPin('B', 0) ? scpiTxMsgs[TX_ON] :	scpiTxMsgs[TX_OFF] ) );
											usbEnq(usbTxState);
										}
					break;
				case	RX_PB_0_DIRq:	{	if(getIoDirection('B', 0))
												usbEnqStrs(3, subSyses[SYSID_PORTB], ":0",	scpiTxMsgs[TX_INPUT]) ;
											else
												usbEnqStrs(3, subSyses[SYSID_PORTB], ":0",	scpiTxMsgs[TX_OUTPUT]) ;
										}
					break;

				// GPIOA10 => PortB1
				case	RX_PB_1_OUT:	deinitExtIrq('B', 1);	initOutput('B', 1);					break;
				case	RX_PB_1_IN:		deinitExtIrq('B', 1);	initInput('B', 1);					break;
				case	RX_PB_1_PULLU:	initPull('B', 1, true);			break;
				case	RX_PB_1_PULLD:	initPull('B', 1, false);		break;
				case	RX_PB_1_NOPULL:	deinitPull('B', 1);				break;
				case	RX_PB_1_RISE:	initInput('B', 1);		initExtIrq('B', 1, true, false);	break;
				case	RX_PB_1_FALL:	initInput('B', 1);		initExtIrq('B', 1, false, true);	break;
				case	RX_PB_1_NOIRQ:	deinitExtIrq('B', 1);
										initOutput('B', 1);	// set pin to output and 0
										clrPin('B', 1);
					break;
				case	RX_PB_1_PULLq:		if ( PULLUP == getPullState('B', 1) )				usbEnqStrs(3, subSyses[SYSID_PORTB], ":1",	scpiTxMsgs[TX_PULLUP]) ;
											if ( PULLDOWN == getPullState('B', 1) )			usbEnqStrs(3, subSyses[SYSID_PORTB], ":1",	scpiTxMsgs[TX_PULLDOWN]) ;
											if ( NOPULL == getPullState('B', 1) )				usbEnqStrs(3, subSyses[SYSID_PORTB], ":1",	scpiTxMsgs[TX_NOPULL]) ;
					break;
				case	RX_PB_1_IRQq:		if(	RISING_EDGE		== getExtIrqState('B', 1) )	usbEnqStrs(3, subSyses[SYSID_PORTB], ":1",	scpiTxMsgs[TX_RISINGEDGE]);
											if(	FALLING_EDGE 	== getExtIrqState('B', 1) )	usbEnqStrs(3, subSyses[SYSID_PORTB], ":1",	scpiTxMsgs[TX_FALLINGEDGE]);
											if(	RISE_FALL_EDGE 	== getExtIrqState('B', 1) )	usbEnqStrs(3, subSyses[SYSID_PORTB], ":1",	scpiTxMsgs[TX_RISE_FALL_EDGE]);
											if(	NOIRQ 			== getExtIrqState('B', 1) )	usbEnqStrs(3, subSyses[SYSID_PORTB], ":1",	scpiTxMsgs[TX_NOIRQ]);
					break;
				case	RX_PB_1_STATEq:	{	char usbTxState[ USB_MAX_TX_LEN ] = "";
											strcat(usbTxState, subSyses[SYSID_PORTB]);
											strcat(usbTxState, ":1 State:\n");
											strcat(usbTxState, " Direction:");
											strcat(usbTxState, getIoDirection('B', 1) ? scpiTxMsgs[TX_INPUT] :	scpiTxMsgs[TX_OUTPUT]);
											strcat(usbTxState, "\n Level:");
											strcat(usbTxState, (getPin('B', 1) ? scpiTxMsgs[TX_ON] : scpiTxMsgs[TX_OFF] ) );
											usbEnq(usbTxState);
										}
					break;
				case	RX_PB_1_DIRq:		if(getIoDirection('B', 1))
												usbEnqStrs(3, subSyses[SYSID_PORTB], ":1",	scpiTxMsgs[TX_INPUT]) ;
											else
												usbEnqStrs(3, subSyses[SYSID_PORTB], ":1",	scpiTxMsgs[TX_OUTPUT]) ;
					break;
				case	RX_PB_0_ON: 		setPin('B', 0);         														break;
				case	RX_PB_0_OFF: 		clrPin('B', 0);         														break;
				case	RX_PB_0q: 			usbEnqStrs(3, subSyses[SYSID_PORTB], ":0", getPin('B', 0) ? scpiTxMsgs[TX_ON] : scpiTxMsgs[TX_OFF]) ;		break;
				case	RX_PB_1_ON: 		setPin('B', 1);         														break;
				case	RX_PB_1_OFF: 		clrPin('B', 1);         														break;
				case	RX_PB_1q: 			usbEnqStrs(3, subSyses[SYSID_PORTB], ":1", getPin('B', 1) ? scpiTxMsgs[TX_ON] : scpiTxMsgs[TX_OFF]) ;		break;
			#endif // PCBv02


				case	RX_PORTA_0_ON: 		setPin('A',0);         														break;
				case	RX_PORTA_0_OFF: 	clrPin('A',0);         														break;
				case	RX_PORTA_0_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":0", getPin('A',0)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_1_ON: 		setPin('A',1);         														break;
				case	RX_PORTA_1_OFF: 	clrPin('A',1);         														break;
				case	RX_PORTA_1_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":1", getPin('A',1)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_2_ON: 		setPin('A',2);         														break;
				case	RX_PORTA_2_OFF: 	clrPin('A',2);         														break;
				case	RX_PORTA_2_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":2", getPin('A',2)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_3_ON: 		setPin('A',3);         														break;
				case	RX_PORTA_3_OFF: 	clrPin('A',3);         														break;
				case	RX_PORTA_3_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":3", getPin('A',3)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_4_ON:		setPinLO(GPIOC, 4);															break;
				case	RX_PORTA_4_OFF: 	clrPinLO(GPIOC, 4);         												break;
				case	RX_PORTA_4_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":4", getPinLO(GPIOC, 4)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_5_ON: 		setPinLO(GPIOC, 5);     														break;
				case	RX_PORTA_5_OFF: 	clrPinLO(GPIOC, 5);     														break;
				case	RX_PORTA_5_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":5", getPinLO(GPIOC, 5)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_6_ON: 		setPin('A',6);         														break;
				case	RX_PORTA_6_OFF: 	clrPin('A',6);         														break;
				case	RX_PORTA_6_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":6", getPin('A',6)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_7_ON: 		setPin('A',7);         														break;
				case	RX_PORTA_7_OFF: 	clrPin('A',7);         														break;
				case	RX_PORTA_7_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":7", getPin('A',7)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_8_ON: 		setPin('A',8);         														break;
				case	RX_PORTA_8_OFF: 	clrPin('A',8);         														break;
				case	RX_PORTA_8_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":8", getPin('A',8)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_9_ON: 		setPin('A',9);         														break;
				case	RX_PORTA_9_OFF: 	clrPin('A',9);         														break;
				case	RX_PORTA_9_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":9", getPin('A',9)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_10_ON: 	setPin('A',10);         														break;
				case	RX_PORTA_10_OFF: 	clrPin('A',10);         														break;
				case	RX_PORTA_10_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":10", getPin('A',10)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_11_ON: 	setPin('A',11);         														break;
				case	RX_PORTA_11_OFF: 	clrPin('A',11);         														break;
				case	RX_PORTA_11_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":11", getPin('A',11)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_12_ON: 	setPin('A',12);         														break;
				case	RX_PORTA_12_OFF: 	clrPin('A',12);         														break;
				case	RX_PORTA_12_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":12", getPin('A',12)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_13_ON: 	setPin('A',13);         														break;
				case	RX_PORTA_13_OFF: 	clrPin('A',13);         														break;
				case	RX_PORTA_13_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":13", getPin('A',13)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_14_ON: 	setPin('A',14);         														break;
				case	RX_PORTA_14_OFF: 	clrPin('A',14);         														break;
				case	RX_PORTA_14_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":14", getPin('A',14)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;
				case	RX_PORTA_15_ON: 	setPin('A',15);         														break;
				case	RX_PORTA_15_OFF: 	clrPin('A',15);         														break;
				case	RX_PORTA_15_q: 		usbEnqStrs(3, subSyses[SYSID_PORTA], ":15", getPin('A',15)?scpiTxMsgs[TX_ON]:scpiTxMsgs[TX_OFF] );		break;

			// VOUT / DACs
//				case	RX_VOUTA_VOLT:		writeDacPercent(1, getFloatFromStr(usbInBuf)) ? usbEnq(scpiTxMsgs[TX_PARAM_OK ]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR ]);	break;
//				case	RX_VOUTB_VOLT:		writeDacPercent(2, getFloatFromStr(usbInBuf)) ? usbEnq(scpiTxMsgs[TX_PARAM_OK ]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR ]);	break;
				case	RX_VOUTA_VOLT:		writeDacVolts(1, getFloatFromStr(usbInBuf)) ? usbEnq(scpiTxMsgs[TX_PARAM_OK ]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR ]);	break;
				case	RX_VOUTB_VOLT:		writeDacVolts(2, getFloatFromStr(usbInBuf)) ? usbEnq(scpiTxMsgs[TX_PARAM_OK ]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR ]);	break;

//				case	RX_VOUTA_VOLTq:		usbCdcTxEnqueueStrsFloat(readDacPercent(1), 1, "VOUTA:")		;			break;
//				case	RX_VOUTB_VOLTq:		usbCdcTxEnqueueStrsFloat(readDacPercent(2), 1, "VOUTB:")		;			break;

				case	RX_VOUTA_VOLTq:		usbCdcTxEnqueueStrsFloat(readDacVolts(1), 1, "VOUTA:")		;			break;
				case	RX_VOUTB_VOLTq:		usbCdcTxEnqueueStrsFloat(readDacVolts(2), 1, "VOUTB:")		;			break;

			// TimerA
				case	RX_TIMA_START:	startTimerA()		?	usbEnq(scpiTxMsgs[TX_STARTED]) : usbEnq(scpiTxMsgs[TX_NOT_STARTED]);					break;
				case	RX_TIMA_STOP:	stopTimerA()		?	usbEnq(scpiTxMsgs[TX_STOPPED]) : usbEnq(scpiTxMsgs[TX_NOT_STOPPED]);					break;
				case	RX_TIMA_FREQ:	setTimerA(1/getFloatFromStr(usbInBuf))?	usbEnq(scpiTxMsgs[TX_PARAM_OK]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR]);		break;
				case	RX_TIMA_PERI:	setTimerA(getFloatFromStr(usbInBuf))?	usbEnq(scpiTxMsgs[TX_PARAM_OK]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR]);		break;
				case	RX_TIMA_FREQq:	usbCdcTxEnqueueStrsFloat(1/(2*getTimerPeriod((TIM2->PSC), (TIM2->ARR))), 2, subSyses[SYSID_TIMERA], scpiTxMsgs[TX_FREQUENCY]);
					break;
				case	RX_TIMA_PERIq:	usbCdcTxEnqueueStrsFloat(2*getTimerPeriod((TIM2->PSC), (TIM2->ARR)), 2, subSyses[SYSID_TIMERA], scpiTxMsgs[TX_PERIOD]);
					break;
				case	RX_TIMAq:		getTimerState(TIM2)	?	usbEnq(scpiTxMsgs[TX_ON]) : usbEnq(scpiTxMsgs[TX_OFF]);
					break;

			// TimerB
				case	RX_TIMB_START:  startTimerB()		?	usbEnq(scpiTxMsgs[TX_STARTED]) : usbEnq(scpiTxMsgs[TX_NOT_STARTED]);					break;
				case	RX_TIMB_STOP:   stopTimerB()		?	usbEnq(scpiTxMsgs[TX_STOPPED]) : usbEnq(scpiTxMsgs[TX_NOT_STOPPED]);					break;
				case	RX_TIMB_FREQ:   setTimerB(1/getFloatFromStr(usbInBuf))?	usbEnq(scpiTxMsgs[TX_PARAM_OK]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR]);		break;
				case	RX_TIMB_PERI:   setTimerB(getFloatFromStr(usbInBuf))?	usbEnq(scpiTxMsgs[TX_PARAM_OK]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR]);		break;
				case	RX_TIMB_FREQq:  usbCdcTxEnqueueStrsFloat(1/(2*getTimerPeriod((TIM3->PSC), (TIM3->ARR))), 2, subSyses[SYSID_TIMERB], scpiTxMsgs[TX_FREQUENCY]);
					break;
				case	RX_TIMB_PERIq:	usbCdcTxEnqueueStrsFloat(2*getTimerPeriod((TIM3->PSC), (TIM3->ARR)), 2, subSyses[SYSID_TIMERB], scpiTxMsgs[TX_PERIOD]);
					break;
				case	RX_TIMBq:		getTimerState(TIM3) ?	usbEnq(scpiTxMsgs[TX_ON]) : usbEnq(scpiTxMsgs[TX_OFF]);
					break;	//

			// TimerC
				case	RX_TIMC_START:  startTimerC()		?	usbEnq(scpiTxMsgs[TX_STARTED]) : usbEnq(scpiTxMsgs[TX_NOT_STARTED]);					break;
				case	RX_TIMC_STOP:   stopTimerC()		?	usbEnq(scpiTxMsgs[TX_STOPPED]) : usbEnq(scpiTxMsgs[TX_NOT_STOPPED]);					break;
				case	RX_TIMC_FREQ:	setTimerC(1/getFloatFromStr(usbInBuf))?	usbEnq(scpiTxMsgs[TX_PARAM_OK]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR]);		break;
				case	RX_TIMC_PERI:   setTimerC(getFloatFromStr(usbInBuf))?	usbEnq(scpiTxMsgs[TX_PARAM_OK]) : usbEnq(scpiTxMsgs[TX_PARAM_ERR]);		break;
				case	RX_TIMC_FREQq:  usbCdcTxEnqueueStrsFloat(1/(2*getTimerPeriod((TIM4->PSC), (TIM4->ARR))), 2, subSyses[SYSID_TIMERC], scpiTxMsgs[TX_FREQUENCY]);
					break;
				case	RX_TIMC_PERIq:	usbCdcTxEnqueueStrsFloat(2*getTimerPeriod((TIM4->PSC), (TIM4->ARR)), 2, subSyses[SYSID_TIMERC], scpiTxMsgs[TX_PERIOD]);
					break;
				case	RX_TIMCq:   	getTimerState(TIM4)	?	usbEnq(scpiTxMsgs[TX_ON]) : usbEnq(scpiTxMsgs[TX_OFF]);									break;

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
				case	RX_DBG_READREG:	{	int address = getIntFromStr(usbInBuf);
											int val = *(int*)(address);
											// volatile int value1 = *(int*)0x40010800;	yes
											// volatile int value2 = *(int*)1073809408; yes
											// volatile int dbgRegval = READ_REG(dbgAddress); nope

											// usbEnqStrs(val, 1,  scpiTxMsgs[TX_REGVAL]);
											usbCdcTxEnqueueStrsInt(val, 1,  scpiTxMsgs[TX_REGVAL]);
										}
					break;
				case	RX_DBG_BLINK:
						blinkDbgLed();
					break;

				case	RX_DBG_CPUID:
					{	// char cpuIdBuf[MAX_SCPI_OUT_STRELN];
						// sprintf(cpuIdBuf, "Processor IDs: %lu, %lu, %lu, %lu \n", ID_GetUnique32(0),ID_GetUnique32(1),ID_GetUnique32(2),ID_GetUnique32(3));
						usbCdcTxEnqueueStrsInt( ID_GetUnique32(0) + ID_GetUnique32(1) + ID_GetUnique32(2) + ID_GetUnique32(3), 1, "sum of CPU IDs: ") ;
					}
					break;

				case	RX_DBG_dacVoltToRaw:    usbCdcTxEnqueueStrsInt( dacVoltToRaw(getFloatFromStr(usbInBuf)), 1, "roh: " );         		break;
				case	RX_DBG_dacRawToVolt:    usbCdcTxEnqueueStrsFloat( dacRawToVolt(getIntFromStr(usbInBuf)), 1, "volts: ");         		break;


			// \Debug
				// case	:   ;         		break;
				default:				usbEnq(scpiTxMsgs[TX_ERR]);													break;
            }	// \switch

			#ifdef DEBUG
				usbCdcTxEnqueueStrsInt(getScpiSubsys(usbInBuf), 1, scpiTxMsgs[TX_SUBSYS_ID]);
				usbCdcTxEnqueueStrsInt(scpiID, 1, scpiTxMsgs[TX_SCPI_ID]);
				usbEnq(usbInBuf);
			#endif // DEBUG

        }

        if(keepaliveGetRanout())
        {
        	keepaliveClrRanout();
        	keepaliveStop();
        	rstMain();
        	usbEnqStrs(2 , subSyses[SYSID_KEEPALIVE], scpiTxMsgs[TX_OVERFLOW]);
        }

        // collect IRQ events and report them upstream
        for(int idx = 0; idx <= (MAX_EXTI_CNT-1); idx++)	// loop over all possible EXTIs
        {
            if(getRisingEdge(idx) )
            {	usbCdcTxEnqueueStrsInt(idx, 3, scpiTxMsgs[TX_RISINGEDGE], " on ", subSyses[SYSID_EXTI]);
            	clrRisingEdge(idx);
            }

            if(getFallingEdge(idx))
            {	usbCdcTxEnqueueStrsInt(idx, 3, scpiTxMsgs[TX_FALLINGEDGE], " on ", subSyses[SYSID_EXTI]);
             	clrFallingEdge(idx);
            }
        }

		// invalidate usbInBuf;
		usbInBuf[0] = 0;
		usbTxFlush();

    }	// \while
    // if eternity should fail:

    deinitTimerA();
    deinitTimerB();
    deinitTimerC();
    deinitKeepalive();
    deinitHal();
}


static void rstMain()
{

	// startTimerA();
	// startTimerB();
	// startTimerC();

	stopTimerA();
	stopTimerB();
	stopTimerC();

	// keepaliveStart();
	keepaliveStop();


	writeDacVolts(1, 0.0);
	writeDacVolts(2, 0.0);
	
	clrRelay0();
	clrRelay1();

	clrPin('C', 0);
	clrPin('C', 1);

	clrPin('B', 0);
	clrPin('B', 1);

	// NOPE, this would lead to a deadlock:
	// keepaliveSetPeriod(1.2);

	clrPin('A', 0);
	clrPin('A', 1);
	clrPin('A', 2);
	clrPin('A', 3);
	clrPin('A', 4);
	clrPin('A', 5);
	clrPin('A', 6);
	clrPin('A', 7);
	clrPin('A', 8);
	clrPin('A', 9);
	clrPin('A', 10);
	clrPin('A', 11);
	clrPin('A', 12);
	clrPin('A', 13);
	clrPin('A', 14);
	clrPin('A', 15);


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
