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


