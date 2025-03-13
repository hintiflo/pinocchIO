/**
 * @file	hal.h
 * @author	Florian Hinterleitner
 * @date	08.01.2023
 * @brief	provides access to all digital IOs
 * 
 */



#ifndef HAL_M010_H_
#define HAL_M010_H_

// #include "stdbool.h"
 #include "globals.h"


typedef enum
{	PULLUP,
	PULLDOWN,
	NOPULL
}PullState;

typedef enum
{	RISING_EDGE,
	FALLING_EDGE,
	RISE_FALL_EDGE,
	NOIRQ
}IrqState;

typedef struct
{	GPIO_TypeDef * GPIOx;
	int pinNr;
}gpioPin;




/* public prototypes */
bool initPortPinInput(GPIO_TypeDef * GPIOx, int Pin, bool pull, bool up);
bool initPortPinOutput(GPIO_TypeDef * GPIOx, int Pin, bool pushpull, int speed);

bool deinitPortPin(GPIO_TypeDef * GPIOx, int Pin);
gpioPin getGpioPinFromPortPin(char port, int pinNr);

void setRisingCycles(int extiIdx)	;
void setFallingCycles(int extiIdx)	;
void clrRisingCycles(int extiIdx)	;
void clrFallingCycles(int extiIdx)	;
int getRisingCycles(int extiIdx)	;
int getFallingCycles(int extiIdx)	;

void setRisingEdges(int extiIdx);
void setFallingEdges(int extiIdx);
void clrRisingEdges(int extiIdx);
void clrFallingEdges(int extiIdx);
int getRisingEdges(int extiIdx);
int getFallingEdges(int extiIdx);

void setRisingEdge(int extiIdx)	;
void setFallingEdge(int extiIdx);
void clrRisingEdge(int extiIdx)	;
void clrFallingEdge(int extiIdx);
bool getRisingEdge(int extiIdx)	;
bool getFallingEdge(int extiIdx);



/**
 * @brief	higher HAL
 * 			intended for high level use, indicating PortA, B or C
 */

//	manipulate whole port in one command
void setPort(int port, int pin);
void clrPort(int port, int pin);
bool getPort(int port, int pin);

//	manipulate singel pins
void setPin(char port, int pin);
void clrPin(char port, int pin);
bool getPin(char port, int pin);

bool initOutput(char port,  int Pin);
bool initInput(char port, int Pin);
bool initPull(char port, int Pin, bool up);
bool deinitPull(char port, int Pin);			// @brief deactivate PullUps/downs and (make the pin an output?)
														// OOOR, solve this shit directly at initPin???

bool initExtIrq(char port, int pinNr, bool risingEdge, bool fallingEdge);
void deinitExtIrq(char port, int pinNr);

bool getIoDirection(char port, int pinNr);
PullState getPullState(char port, int pinNr);
IrqState  getExtIrqState(char port, int pinNr);



/**
 * @brief	lower HAL
 * 			intended for low-level and HAL-internal use indicating GPIOA, GPIOB or GPIOC
 */
void setPinLO(GPIO_TypeDef * GPIOx, int Pin);
void clrPinLO(GPIO_TypeDef * GPIOx, int Pin);
bool getPinLO(GPIO_TypeDef * GPIOx, int Pin);

bool getIoDirectionLO(GPIO_TypeDef * GPIOx, int Pin);
PullState getPullStateLO(GPIO_TypeDef * GPIOx, int Pin);

IRQn_Type  getExtiIdx(int exti);
int getExtiCrIdx(int exti);
int getExtiCrPxIdx(int exti);
int getExtiCrPBIdx(int exti);


bool initExtIrqLO(GPIO_TypeDef * GPIOx, int Pin, bool risingEdge, bool fallingEdge);
void deinitExtIrqLO(GPIO_TypeDef * GPIOx, int Pin);
IrqState getExtIrqStateLO(GPIO_TypeDef * GPIOx, int Pin);
bool initOutputLO(GPIO_TypeDef * GPIOx, int Pin);
bool initInputLO(GPIO_TypeDef * GPIOx, int Pin);
bool initPullLO(GPIO_TypeDef * GPIOx, int Pin, bool up);
bool deinitPullLO(GPIO_TypeDef * GPIOx, int Pin);			// @brief deactivate PullUps/downs and (make the pin an output?)
														// OOOR, solve this shit directly at initPin???



/* \public prototypes */

// for bitwise access in a 16Bit - var
// union REG16BIT {
//      uint16_t allBits;
//      struct {
// 		uint8_t bit0    :1;
// 		uint8_t bit1    :1;
// 		uint8_t bit2    :1;
// 		uint8_t bit3    :1;
// 		uint8_t bit4    :1;
// 		uint8_t bit5    :1;
// 		uint8_t bit6    :1;
// 		uint8_t bit7    :1;
// 		uint8_t bit8    :1;
// 		uint8_t bit9    :1;
// 		uint8_t bit10    :1;
// 		uint8_t bit11    :1;
// 		uint8_t bit12    :1;
// 		uint8_t bit13    :1;
// 		uint8_t bit14    :1;
// 		uint8_t bit15    :1;
//      };
//  };
//
// union REG16BIT wordReg;
//
// wordReg.allBits = word;
// sprintf(buf, "switched: %d%d%d%d %d%d%d%d %d%d%d%d %d%d%d%d \n"
// 		, wordReg.bit0, wordReg.bit1, wordReg.bit2, wordReg.bit3
// 		, wordReg.bit4, wordReg.bit5, wordReg.bit6, wordReg.bit7
// 		, wordReg.bit8, wordReg.bit9, wordReg.bit10, wordReg.bit11
// 		, wordReg.bit12, wordReg.bit13, wordReg.bit14, wordReg.bit15
// );
//
//
// sscanf(buf, "%*s %c%c%c%c %c%c%c%c %c%c%c%c %c%c%c%c\n",
// 			&bitbuf[0] , &bitbuf[1] ,  &bitbuf[2] , &bitbuf[3] , &bitbuf[4] , &bitbuf[5] , &bitbuf[6] , &bitbuf[7] ,
// 			&bitbuf[8] , &bitbuf[9] , &bitbuf[10] , &bitbuf[11] , &bitbuf[12] , &bitbuf[13] , &bitbuf[14] , &bitbuf[15]	);
//
// 			wordReg.bit0  = bitbuf[0] ;	wordReg.bit1  = bitbuf[1] ;	wordReg.bit2  = bitbuf[2] ;	wordReg.bit3  = bitbuf[3] ;
// 			wordReg.bit4  = bitbuf[4] ;	wordReg.bit5  = bitbuf[5] ;	wordReg.bit6  = bitbuf[6] ;	wordReg.bit7  = bitbuf[7] ;
// 			wordReg.bit8  = bitbuf[8] ;	wordReg.bit9  = bitbuf[9] ;	wordReg.bit10 = bitbuf[10];	wordReg.bit11 = bitbuf[11];
// 			wordReg.bit12 = bitbuf[12];	wordReg.bit13 = bitbuf[13];	wordReg.bit14 = bitbuf[14];	wordReg.bit15 = bitbuf[15];
//
// setPatGen16(wordReg.allBits	, 0);






#ifndef UNIFIED_PORTPIN_FUNC
void setPort2_0();
void clrPort2_0();
bool getPort2_0();

void setPort2_1();
void clrPort2_1();
bool getPort2_1();


void setPort1_0();
void clrPort1_0();
bool getPort1_0();


void setPort1_1()	;
void clrPort1_1()	;
bool getPort1_1()	;


void setPort1_2()	;
void clrPort1_2()	;
bool getPort1_2()	;


void setPort1_3()	;
void clrPort1_3()	;
bool getPort1_3()	;


void setPort1_4()	;
void clrPort1_4()	;
bool getPort1_4()	;


void setPort1_5()	;
void clrPort1_5()	;
bool getPort1_5()	;


void setPort1_6()	;
void clrPort1_6()	;
bool getPort1_6()	;


void setPort1_7()	;
void clrPort1_7()	;
bool getPort1_7()	;


void setPort1_8()	;
void clrPort1_8()	;
bool getPort1_8()	;


void setPort1_9()	;
void clrPort1_9()	;
bool getPort1_9()	;


void setPort1_10()	;
void clrPort1_10()	;
bool getPort1_10()	;


void setPort1_11()	;
void clrPort1_11()	;
bool getPort1_11()	;


void setPort1_12()	;
void clrPort1_12()	;
bool getPort1_12()	;


void setPort1_13()	;
void clrPort1_13()	;
bool getPort1_13()	;


void setPort1_14()	;
void clrPort1_14()	;
bool getPort1_14()	;


void setPort1_15()	;
void clrPort1_15()	;
bool getPort1_15()	;

#endif // UNIFIED_PORTPIN_FUNC




/**
 * @brief	activate Ports A, B and C and set debug LED as output
 *
 * ...by supplying the Ports with a clock source setting PortC13 as output
 *
 */



void initDbgLed();
void setDbgLED();
void clrDbgLED();

// static bool getDbgLED()		
// void deinitDbgLed();

void initRelays();
void setRelay0();
void clrRelay0();
bool getRelay0();
void setRelay1();
void clrRelay1();
bool getRelay1();
void deinitRelays();

void initPortA();
void deinitPortA();
void initPortB();
void deinitPortB();
void initPortC();
void deinitPortC();

void initDACs();
void deinitDACs();

bool writeDacRaw(int channel, int decval);
bool writeDacPercent(int channel, float percent);
int  readDacRaw(int channel);
float readDacPercent(int channel);

bool writeDacVolts(int channel, float voltage);
float  readDacVolts(int channel);


int		dacVoltToRaw(float voltage);
float	dacRawToVolt(int dacval);


void initHal();
void deinitHal();


#endif /* HAL_M010_H_ */


