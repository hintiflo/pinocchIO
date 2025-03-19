/**
 * @file	scpi.h
 * @author	Florian Hinterleitner
 * @date	08.01.2023
 * @brief	provides translation of incoming scpi messages into according IDs and contains outgoing scpi messages
 *
 */

#ifndef SCPI_H_
#define SCPI_H_


#include "globals.h"
// #define UNIFIED_SCPI_VECTOR

// #define MAX_SCPI_IN_STRELN 100
#define MAX_SCPI_IN_STRELN 35

#ifdef	UNIFIED_SCPI_VECTOR

	#define MAX_SCPI_IN_STRCNT 127
	#define SCPI_IN_STRCNT 		25 + 1
#endif	// UNIFIED_SCPI_VECTOR
#ifndef	UNIFIED_SCPI_VECTOR
	#define SCPI_IN_STRCNT 		(sizeof(scpiRxLongMsgs)/sizeof(scpiRxLongMsgs[0]))
#endif	// UNIFIED_SCPI_VECTOR

#define MAX_SCPI_OUT_STRELN MAX_SCPI_IN_STRELN





// #define 	NUMBER_OF_SCPI_COMMANDS	(sizeof(rSCPIlong)/sizeof(rSCPIlong[0]))


int getScpiIdfromMsg(char * scpiCmd);
//char * getScpiTok1(char * scpiMsg);
//int getScpiSubsys(char * scpiMsg);
static float getFloatFromStr(char * source);
static int getIntFromStr(char * source);

	#ifndef	UNIFIED_SCPI_VECTOR
		static const char scpiRxLongMsgs[][MAX_SCPI_IN_STRELN] =
		{
			"*CLS"			,		// Clear Status Command
			"*ESE" 			,		// Standard Event Status Enable Command
			"*ESE?" 		,		// Standard Event Status Enable Query
			"*ESR?" 		,		// Standard Event Status Register Query
			"*IDN?" 		,		// Identification Query
			"*OPC" 			,		// Operation Complete Command
			"*OPC?" 		,		// Operation Complete Query
			"*RST" 			,		// Reset Command
			"*SRE" 			,		// Service Request Enable Command
			"*SRE?" 		,		// Service Request Enable Query
			"*STB?" 		,	// Read Status Byte Query
			"*TST?" 		,	// Self-Test Query
			"*WAI" 			,	// Wait-to-Continue Command
			"Relay0 on" 	,
			"Relay0 off" 	,
			"Relay0?" 		,
			"Relay1 on" 	,
			"Relay1 off" 	,
			"Relay1?" 		,
		#ifdef PCBv02
			"PortC:0 ON",
			"PortC:0 OFF",
			"PortC:0?",
			"PortC:1 ON",
			"PortC:1 OFF",
			"PortC:1?",
		#endif // PCBv02

			"PortB:0 Output" 		,
			"PortB:0 Input" 		,
			"PortB:0 PullDown" 		,
			"PortB:0 PullUp" 		,
			"PortB:0 noPull" 		,
			"PortB:0 RisingEdge" 	,
			"PortB:0 FallingEdge" 	,
			"PortB:0 noIRQ" 		,
			"PortB:0 pull?" 		,
			"PortB:0 IRQ?" 			,
			"PortB:0 state?" 		,
			"PortB:0 direction?"	,

			"PortB:1 Output" 		,
			"PortB:1 Input" 		,
			"PortB:1 PullDown" 		,
			"PortB:1 PullUp" 		,
			"PortB:1 noPull" 		,
			"PortB:1 RisingEdge" 	,
			"PortB:1 FallingEdge" 	,
			"PortB:1 noIRQ" 		,
			"PortB:1 pull?" 		,
			"PortB:1 IRQ?" 			,
			"PortB:1 state?" 		,
			"PortB:1 direction?"	,

			"PortB:0 on" 			,
			"PortB:0 off" 			,
			"PortB:0?" 				,
			"PortB:1 on" 			,
			"PortB:1 off" 			,
			"PortB:1?" 				,

			"PortA:0 on" 		,
			"PortA:0 off" 		,
			"PortA:0?" 			,
			"PortA:1 on" 		,
			"PortA:1 off" 		,
			"PortA:1?" 			,
			"PortA:2 on" 		,
			"PortA:2 off" 		,
			"PortA:2?" 			,
			"PortA:3 on" 		,
			"PortA:3 off" 		,
			"PortA:3?" 			,
			"PortA:4 on" 		,
			"PortA:4 off" 		,
			"PortA:4?" 			,
			"PortA:5 on" 		,
			"PortA:5 off" 		,
			"PortA:5?" 			,
			"PortA:6 on" 		,
			"PortA:6 off" 		,
			"PortA:6?" 			,
			"PortA:7 on" 		,
			"PortA:7 off" 		,
			"PortA:7?" 			,
			"PortA:8 on" 		,
			"PortA:8 off" 		,
			"PortA:8?" 			,
			"PortA:9 on" 		,
			"PortA:9 off" 		,
			"PortA:9?" 			,
			"PortA:10 on" 		,
			"PortA:10 off" 		,
			"PortA:10?" 		,
			"PortA:11 on" 		,
			"PortA:11 off" 		,
			"PortA:11?" 		,
			"PortA:12 on" 		,
			"PortA:12 off" 		,
			"PortA:12?" 		,
			"PortA:13 on" 		,
			"PortA:13 off" 		,
			"PortA:13?" 		,
			"PortA:14 on" 		,
			"PortA:14 off" 		,
			"PortA:14?" 		,
			"PortA:15 on" 		,
			"PortA:15 off" 		,
			"PortA:15?" 		,

			"VOUTA:VOLT"   		,
			"VOUTB:VOLT"   		,
			"VOUTA:VOLT?"   	,
			"VOUTB:VOLT?"   	,

			"TimerA:start"			,
			"TimerA:stop"			,
			"TimerA?"				,
			"TimerA:Frequency"		,          	// float 1/period
			"TimerA:Period"			,          	// float period
			"TimerA:Frequency?"		,
			"TimerA:Period?"		,

			"TimerB:start"			,
			"TimerB:stop"			,
			"TimerB?"				,
			"TimerB:Frequency"		,          	// float 1/period
			"TimerB:Period"			,          	// float period
			"TimerB:Frequency?"		,
			"TimerB:Period?"		,

			"TimerC:start"			,
			"TimerC:stop"			,
			"TimerC?"				,
			"TimerC:Frequency"		,          	// float 1/period
			"TimerC:Period"			,          	// float period
			"TimerC:Frequency?"		,
			"TimerC:Period?"		,

			"KeepAlive:period",
			"KeepAlive on",
			"KeepAlive off",
			"KeepAlive update",
			"KeepAlive?",

			"Debug:ReadRegister",
			"Debug:BLINK",
			"Debug:CPUID",
			"Debug:VoltToRaw",
			"Debug:RawToVolt",


		};


		static const char scpiRxShortMsgs[][MAX_SCPI_IN_STRELN] =
		{
				"*CLS"			,		// Clear Status Command
				"*ESE"			,		// Standard Event Status Enable Command
				"*ESE?"			,		// Standard Event Status Enable Query
				"*ESR?"			,		// Standard Event Status Register Query
				"*IDN?"			,		// Identification Query
				"*OPC"			,		// Operation Complete Command
				"*OPC?"			,		// Operation Complete Query
				"*RST"			,		// Reset Command
				"*SRE"			,		// Service Request Enable Command
				"*SRE?"			,		// Service Request Enable Query
				"*STB?"			,		// Read Status Byte Query
				"*TST?"			,		// Self-Test Query
				"*WAI"			,		// Wait-to-Continue Command
				"REL0 ON"		,
				"REL0 OFF"		,
				"REL0?"			,
				"REL1 ON"		,
				"REL1 OFF"		,
				"REL1?"			,
			#ifdef PCBv02
				"PC:0 ON",
				"PC:0 OFF",
				"PC:0?",
				"PC:1 ON",
				"PC:1 OFF",
				"PC:1?",
			#endif // PCBv02

				"PB:0 Out" 	 ,
				"PB:0 In" 	 ,
				"PB:0 PD" 	 ,
				"PB:0 PU" 	 ,
				"PB:0 noPull",
				"PB:0 Rise"  ,
				"PB:0 Fall"  ,
				"PB:0 noIRQ" ,
				"PB:0 pull?" ,
				"PB:0 IRQ?"  ,
				"PB:0 state?",
				"PB:0 dir?",

				"PB:1 Out" 	 ,
				"PB:1 In" 	 ,
				"PB:1 PD" 	 ,
				"PB:1 PU" 	 ,
				"PB:1 noPull",
				"PB:1 Rise"  ,
				"PB:1 Fall"  ,
				"PB:1 noIRQ" ,
				"PB:1 pull?" ,
				"PB:1 IRQ?"  ,
				"PB:1 state?",
				"PB:1 dir?"		,

				"PB:0 on" 		,
				"PB:0 off" 		,
				"PB:0?" 		,

				"PB:1 on" 		,
				"PB:1 off" 		,
				"PB:1?" 		,

				"PA:0 on" 		,
				"PA:0 off" 		,
				"PA:0?" 		,
				"PA:1 on" 		,
				"PA:1 off" 		,
				"PA:1?" 		,
				"PA:2 on" 		,
				"PA:2 off" 		,
				"PA:2?" 		,
				"PA:3 on" 		,
				"PA:3 off" 		,
				"PA:3?" 		,
				"PA:4 on" 		,
				"PA:4 off" 		,
				"PA:4?" 		,
				"PA:5 on" 		,
				"PA:5 off" 		,
				"PA:5?" 		,
				"PA:6 on" 		,
				"PA:6 off" 		,
				"PA:6?" 		,
				"PA:7 on" 		,
				"PA:7 off" 		,
				"PA:7?" 		,
				"PA:8 on" 		,
				"PA:8 off" 		,
				"PA:8?" 		,
				"PA:9 on" 		,
				"PA:9 off" 		,
				"PA:9?" 		,
				"PA:10 on" 		,
				"PA:10 off" 	,
				"PA:10?" 		,
				"PA:11 on" 		,
				"PA:11 off" 	,
				"PA:11?" 		,
				"PA:12 on" 		,
				"PA:12 off" 	,
				"PA:12?" 		,
				"PA:13 on" 		,
				"PA:13 off" 	,
				"PA:13?" 		,
				"PA:14 on" 		,
				"PA:14 off" 	,
				"PA:14?" 		,
				"PA:15 on" 		,
				"PA:15 off" 	,
				"PA:15?" 		,

				"VOUTA:VOLT"   ,
				"VOUTB:VOLT"   ,
				"VOUTA:VOLT?"   ,
				"VOUTB:VOLT?"   ,

				"TimA:start"	,
				"TimA:stop"		,
				"TimA?"			,
				"TimA:Freq"		,	// float 1/period
				"TimA:Peri"		,	// float period
				"TimA:Freq?"	,
				"TimA:Peri?"	,

				"TimB:start"			,
				"TimB:stop"			,
				"TimB?"				,
				"TimB:Freq"		,          	// float 1/period
				"TimB:Peri"		,          	// float period
				"TimB:Freq?"	,
				"TimB:Peri?"		,

				"TimC:start"			,
				"TimC:stop"			,
				"TimC?"				,
				"TimC:Freq"		,          	// float 1/period
				"TimC:Peri"		,          	// float period
				"TimC:Freq?"	,
				"TimC:Peri?"		,

				"KA:peri",
				"KA on",
				"KA off",
				"KA up",
				"KA?",

				"DBG:READREG",
				"dbg:blink",
				"DBG:CPUID",
				"DBG:VoltToRaw",
				"DBG:RawToVolt",

		};
	#endif	// UNIFIED_SCPI_VECTOR


/**
 * @brief	enum containing the IDs, equivalent to the msgs in 'scpiRxMsgs'
 */
typedef enum
{
			RX_IDNq,
			RX_RST,

			RX_REL0_ON,
			RX_REL0_OFF,
			RX_REL0q,
			RX_REL1_ON,
			RX_REL1_OFF,
			RX_REL1q,

			RX_VOUTA_VOLT,		// float voltage
			RX_VOUTB_VOLT,		// float voltage
			RX_VOUTA_VOLTq,
			RX_VOUTB_VOLTq,

			RX_KA_peri,
			RX_KA_on,
			RX_KA_off,
			RX_KA_up,
			RX_KAq,

			RX_DBG_BLINK,
			RX_DBG_CPUID,

			RX_DBG_dacVoltToRaw,
			RX_DBG_dacRawToVolt,

}scpiRxIDs;






static const char scpiTxMsgs[][MAX_SCPI_OUT_STRELN] =
{
	"smartIO v0.4, analog out option",
	" invalid command",
	" parameter invalid",
	" on",
	" off",
	" parameter set",

	" started",
	" not started",
	" stopped",
	" not stopped",

	" reset",
	" notReset",

	" period",
	" frequency",
	" overflow",

	" input",
	" output",
 	" RisingEdge",
	" FallingEdge",
	" Rising&FallingEdge",
	" noIRQ",

	" PullUp",
	" PullDown",
	" noPull",

	" Scpi ID",
	" Sub-Sys ID",
	" Register Value",
};


/**
 * @brief	enum contaings the IDs, equivalent to the msgs in 'scpiRxMsgs'
 */
typedef enum
{
	TX_IDNq,
	TX_ERR,
	TX_PARAM_ERR,
	TX_ON,
	TX_OFF,
	TX_PARAM_OK,

	TX_STARTED,
	TX_NOT_STARTED,
	TX_STOPPED,
	TX_NOT_STOPPED,
	TX_RESET,
	TX_NOT_RESET,
	TX_PERIOD,
	TX_FREQUENCY,
	TX_OVERFLOW,

	TX_INPUT,
	TX_OUTPUT,
	TX_RISINGEDGE,
	TX_FALLINGEDGE,
	TX_RISE_FALL_EDGE,
	TX_NOIRQ,

	TX_PULLUP,
	TX_PULLDOWN,
	TX_NOPULL,


	TX_SCPI_ID,
	TX_SUBSYS_ID,
	TX_REGVAL,


}scpiTxIDs;



int getScpiIdfromMsg(char * scpiCmd)
{
	int scpiID = -1;

	for(int idx = 0; idx < SCPI_IN_STRCNT; idx ++)
	{
			if ( 0 == strncasecmp(scpiCmd, scpiRxShortMsgs[idx], strlen(scpiRxShortMsgs[idx]) ) )
					scpiID = idx;
			else if ( 0 == strncasecmp(scpiCmd, scpiRxLongMsgs[idx], strlen(scpiRxLongMsgs[idx]) ) )
				scpiID = idx;
			//	else
			//		scpiID = -1;
	}

	return scpiID;
}

#define SCPI_SUBSYS_CNT 		(sizeof(subSyses)/sizeof(subSyses[0]))



static const char subSyses[][35] =
{
			"Relay0" 		,
			"Relay1" 		,
		#ifdef PCBv02
			"PortA" 		,
			"PortB" 		,
			"PortC" 		,
		#endif // PCBv02
			"Port2" 		,
			"Port1" 		,
			"ExtIrq"	,
			"TimerA"		,
			"TimerB"		,
			"TimerC"		,
			"Keepalive"		,
			"Debug"			,
};

typedef enum
{
			SYSID_RELAY0 	,
			SYSID_RELAY1 	,
		#ifdef PCBv02
			SYSID_PORTA		,
			SYSID_PORTB		,
			SYSID_PORTC		,
		#endif // PCBv02
			SYSID_PORT2 	,
			SYSID_PORT1 	,
			SYSID_EXTI 	,
			SYSID_TIMERA	,
			SYSID_TIMERB	,
			SYSID_TIMERC	,
			SYSID_KEEPALIVE	,
			SYSID_DEBUG		,
}scpiSubsysIDs;



static float getFloatFromStr(char * source)
{	float result;
	sscanf( source, "%*s%f", &result);

	/*	rounding to 3 decimal points ... not working
		char roundBuf[10];
		sprintf(roundBuf, "%2.3f", result);
		sscanf( roundBuf, "%f", &result);
	 */

	return result;
}

static int getIntFromStr(char * source)
{	int result;
	sscanf( source, "%*s%d", &result);
	//	char * endPtr;
	//	result = strtol(schtring, &endPtr, 10);
	return result;
}


//int getScpiSubsys(char * scpiMsg)
//{
//	char * locScpiMsg;
//	locScpiMsg = strdup(scpiMsg);
//
//	int subSys = -1;
//	char * tok1;
//	char delimiter1[] = " :";
//	tok1 = strtok(locScpiMsg, delimiter1);
//	// printf("tok1 = %s \n", tok1);
//
//	int idx;
//	for (idx = 0; idx < SCPI_SUBSYS_CNT; idx++)
//		if (0 == strncasecmp(tok1, subSyses[idx], strlen(subSyses[idx])))
//			subSys = idx;
//	// free(locScpiMsg);
//	return subSys;
//}
//
//
//char * getScpiTok1(char * scpiMsg)
//{
//	return strtok(scpiMsg, ":");
//}



#endif /* SCPI_H_ */
