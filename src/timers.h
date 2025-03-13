/**
 * @file	timers.h
 * @author	Florian Hinterleitner
 * @date	28.01.2023
 * @brief	provides access to all timers
 *
 * mapping:
 * 	TrigA ->	A0	TIM2_CH1_ETR
 * 	TrigB ->	A6	TIM3_CH1
 * 	TrigC ->	B8	TIM4_CH3
 *
 * 	TimerA ->	TIM2 CH1
 * 	TimerB ->	TIM3 CH1
 * 	TimerC ->	TIM4 CH3
 *
 *	C:\Users\r.hif\STM32\LFP-Treiber_src\Src\main.c
	C:\Users\r.hif\STM32\Taschenlaser\src\main.c
	STM-HAL: C:\Users\r.hif\STM32\TOPTICA_ImagingExtender_MCU\Src\main.c
	STM-HAL: C:\Users\r.hif\STM32\TransAlternator\Src\main.c
 *
 *
 *
 */
#ifndef TIMERS_H_
#define TIMERS_H_

#define	TIMS_MANUAL_IRQ

#define	TRIGA_MIN_PERIOD 0.00001
#define	TRIGA_MAX_PERIOD 10	// seconds
// #define TIM_ENC

#ifdef TIM_ENC
 	#define TIM_EncoderMode_TI1                ((uint16_t)0x0001)
 	#define TIM_EncoderMode_TI2                ((uint16_t)0x0002)
 	#define TIM_EncoderMode_TI12               ((uint16_t)0x0003)
 	#define  TIM_ICPolarity_Rising             ((uint16_t)0x0000)
 	#define  TIM_ICPolarity_Falling            ((uint16_t)0x0002)
 	#define  TIM_ICPolarity_BothEdge           ((uint16_t)0x000A)    //0b1010


 void TIM_EncoderInterfaceConfig(TIM_TypeDef* TIMx, uint16_t TIM_EncoderMode,
                                 uint16_t TIM_IC1Polarity, uint16_t TIM_IC2Polarity);

		 TIM_ENCODERMODE_TI1;

			uint8_t MSG[50] = {'\0'};
		    HAL_Init();
		    SystemClock_Config();

		 HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);


		 TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising,  TIM_ICPolarity_Rising);
		 TIM2->CR1 |= TIM_CR1_CEN;	    // TIM_Cmd(TIM2, ENABLE);
		 disable: TIMx->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));

	void TIM_EncoderInterfaceConfig(TIM_TypeDef* TIMx, uint16_t TIM_EncoderMode,
									uint16_t TIM_IC1Polarity, uint16_t TIM_IC2Polarity)
	{
		uint16_t tmpsmcr = 0;
		uint16_t tmpccmr1 = 0;
		uint16_t tmpccer = 0;

		tmpsmcr = TIMx->SMCR;
		tmpccmr1 = TIMx->CCMR1;
		tmpccer = TIMx->CCER;
		tmpsmcr &= (uint16_t)(~((uint16_t)TIM_SMCR_SMS));  //0x0007    //Close from mode
		tmpsmcr |= TIM_EncoderMode; //0x0003
		tmpccmr1 &= (uint16_t)(((uint16_t)~((uint16_t)TIM_CCMR1_CC1S)) & (uint16_t)(~((uint16_t)TIM_CCMR1_CC2S)));
		tmpccmr1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;
		tmpccer &= (uint16_t)(((uint16_t)~((uint16_t)TIM_CCER_CC1P)) & ((uint16_t)~((uint16_t)TIM_CCER_CC2P)));
		tmpccer |= (uint16_t)(TIM_IC1Polarity | (uint16_t)(TIM_IC2Polarity << (uint16_t)4));
		TIMx->SMCR = tmpsmcr;
		TIMx->CCMR1 = tmpccmr1;
		TIMx->CCER = tmpccer;

	}

#endif // TIM_ENC



uint32_t 	getSysClk();
uint32_t 	getP1TimClk();
// uint32_t getP2TimClk();
float 		getTimerPeriod(uint32_t psc, uint32_t arr);
bool		getTimerState(TIM_TypeDef * TIMx);
uint32_t 	getTimerPSC(float period);
uint32_t 	getTimerARR(float period, uint32_t psc);

bool initTIM(TIM_TypeDef * TIMx, int psc, int arr);
bool setTIM(TIM_TypeDef * TIMx, float period);
bool startTIM(TIM_TypeDef * TIMx);
bool stopTIM(TIM_TypeDef * TIMx);
bool resetTIM( TIM_TypeDef * TIMx );
void updateEventTIM(TIM_TypeDef * TIMx);
bool deinitTIM1();
//	bool setTimerPolHi(TIM_TypeDef * TIMx);
//	bool setTimerPolLo(TIM_TypeDef * TIMx);

// void TIM1_IRQHandler(void);	//timer 1 interrupt service routine
// void TIM1_BRK_IRQHandler(void);                 /* TIM1 Break interrupt                             */
// void TIM1_UP_IRQHandler(void);                  /* TIM1 Update interrupt                            */
// void TIM1_TRG_COM_IRQHandler(void);             /* TIM1 Trigger and Commutation interrupts          */
// void TIM1_CC_IRQHandler(void);                  /* TIM1 Capture Compare interrupt                   */
// obsolete, not existing:
// void TIM1_IRQHandler(void)	//timer 1 interrupt service routine



void tim1SetRegUpdating();
bool tim1GetRegUpdating();
void tim1ClrRegUpdating();

void tim1SetRanout();
bool tim1GetRanout();
void tim1ClrRanout();


static bool initTimerA();
static bool setTimerA(float period);
static bool startTimerA();
static bool stopTimerA();
static bool deinitTimerA();

static bool initTimerB();
static bool setTimerB(float period);
static bool startTimerB();
static bool stopTimerB();
static bool deinitTimerB();

static bool initTimerC();
static bool setTimerC(float period)	;
static bool startTimerC();
static bool stopTimerC();
static bool deinitTimerC();



#ifdef	TIMS_MANUAL_IRQ
	void TIM2_IRQHandler(void);	//timer 2 interrupt service routine
	void TIM3_IRQHandler(void);	//timer 2 interrupt service routine
	void TIM4_IRQHandler(void);	//timer 2 interrupt service routine
#endif	// TIMS_MANUAL_IRQ


bool deinitTIM1()
{	// NVIC_DisableIRQ( TIM1_BRK_IRQn);
	NVIC_DisableIRQ( TIM1_UP_IRQn);
	return true;
}




static bool startTimerA()			{	return startTIM(TIM2);				}
static bool stopTimerA()
{	stopTIM(TIM2);
	//	clrPort1_0();
	clrPort(1, 0);
	return true;
}

static bool setTimerA(float period) {	return setTIM(TIM2, period);		}
static bool initTimerA()			{	return initTIM(TIM2, 1000, 1000);	}
static bool deinitTimerA()			{	return true;						}
static bool startTimerB()			{	return startTIM(TIM3);				}
static bool stopTimerB()
{	stopTIM(TIM3);
	//	clrPort1_6();
	clrPort(1, 6);
	return true;
}

static bool setTimerB(float period)	{	return setTIM(TIM3, period);		}
static bool initTimerB()			{	return initTIM(TIM3, 1000, 1000);	}		// config TIM3 to 200Hz
static bool deinitTimerB(){	return true;}

static bool startTimerC()			{	return startTIM(TIM4);				}
static bool stopTimerC()
{	stopTIM(TIM4);
	// clrPort1_8();
	clrPort(1, 8);
	return true;
}
static bool setTimerC(float period)	{	return setTIM(TIM4, period);		}
static bool initTimerC()			{	return initTIM(TIM4, 4800-1, 5-1);	}
static bool deinitTimerC()			{	return true;}


void TIM1_UP_IRQHandler(void)
{

	tim1SetRanout();


	// keepaliveSetRanout();
	TIM1 -> SR = 0;
}

#ifdef	TIMS_MANUAL_IRQ

	void TIM2_IRQHandler(void)	//timer 2 interrupt service routine
	{	TIM2 -> SR = 0;
		//	getPort1_0()	? clrPort1_0() : setPort1_0();
		getPort(1,0)	? clrPort(1,0) : setPort(1,0);
	}

	void TIM3_IRQHandler(void)	//timer 2 interrupt service routine
	{	TIM3 -> SR = 0;
		// getPort1_6()	? clrPort1_6() : setPort1_6();
		getPort(1,6)	? clrPort(1,6) : setPort(1,6);

	}

	void TIM4_IRQHandler(void)	//timer 2 interrupt service routine
	{	TIM4 -> SR = 0;
		// getPort1_8()	? clrPort1_8() : setPort1_8();
		getPort(1,8)	? clrPort(1,8) : setPort(1,8);
	}

#endif	// TIMS_MANUAL_IRQ

#endif /* TIMERS_H_ */
