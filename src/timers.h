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

void tim1SetRegUpdating();
bool tim1GetRegUpdating();
void tim1ClrRegUpdating();

void tim1SetRanout();
bool tim1GetRanout();
void tim1ClrRanout();

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

void TIM1_UP_IRQHandler(void)
{

	tim1SetRanout();


	// keepaliveSetRanout();
	TIM1 -> SR = 0;
}


#endif /* TIMERS_H_ */
