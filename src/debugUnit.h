/**
 * @file	debugUnit.h
 * @author	Florian Hinterleitner
 * @date	07.01.2023
 * @brief 
 * 
 */

#ifndef DEBUGUNIT_H_
#define DEBUGUNIT_H_

/*	maybe useful for debuggering
	typedef struct
	{
	  __IO uint32_t IDCODE;
	  __IO uint32_t CR;
	}DBGMCU_TypeDef;
 * */



// Output a trace message
void ITM_SendStr(char *ptr)
{   while (*ptr)
    {   ITM_SendChar(*ptr);
        ptr++;
    }
}


#endif /* DEBUGUNIT_H_ */
