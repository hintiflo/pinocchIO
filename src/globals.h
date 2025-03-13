/*
 * globals.h
 *
 *  Created on: 23.02.2023
 *      Author: r.hif
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <stdio.h>


// define current board version to compile for
#define PCBv02

#define ID_UNIQUE_ADDRESS   0x1FFFF7E8
#define ID_GetUnique32(x)   ((x >= 0 && x < 3) ? (*(uint32_t *) (ID_UNIQUE_ADDRESS + 4 * (x))) : 0)

#define UNIFIED_PORTPIN_FUNC
#define GENERAL_PULLUD_FUNC
#define GENERAL_INITIRQ_FUNC

#define MAX_EXTI_CNT 20


// Counts milliseconds
volatile uint32_t systick_count;

// void rstSysTick(){		systick_count = 0;	}
// uint32_t getSysTick(){		return systick_count;	}


#endif /* GLOBALS_H_ */
