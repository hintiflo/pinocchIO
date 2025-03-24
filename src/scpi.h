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
#define MAX_SCPI_IN_STRELN 35
#define SCPI_IN_STRCNT 		(sizeof(scpiRxLongMsgs)/sizeof(scpiRxLongMsgs[0]))
#define MAX_SCPI_OUT_STRELN MAX_SCPI_IN_STRELN

int getScpiIdfromMsg(char * scpiCmd);
static float getFloatFromStr(char * source);
static int getIntFromStr(char * source);

static const char scpiRxLongMsgs[][MAX_SCPI_IN_STRELN] =
{
	"*IDN?" 		,		// Identification Query
	"*RST" 			,		// Reset Command

	"Relay0 on" 	,
	"Relay0 off" 	,
	"Relay0?" 		,
	"Relay1 on" 	,
	"Relay1 off" 	,
	"Relay1?" 		,

	"VOUTA:VOLT"   		,
	"VOUTB:VOLT"   		,
	"VOUTA:VOLT?"   	,
	"VOUTB:VOLT?"   	,

	"TimerA:Frequency"		,          	// float 1/period
	"TimerA:Frequency?"		,

	"KeepAlive:period",
	"KeepAlive on",
	"KeepAlive off",
	"KeepAlive update",
	"KeepAlive?",

	"Debug:BLINK",
	"Debug:CPUID",
	"Debug:VoltToRaw",
	"Debug:RawToVolt",


};


static const char scpiRxShortMsgs[][MAX_SCPI_IN_STRELN] =
{
		"*IDN?"			,		// Identification Query
		"*RST"			,		// Reset Command
		"REL0 ON"		,
		"REL0 OFF"		,
		"REL0?"			,
		"REL1 ON"		,
		"REL1 OFF"		,
		"REL1?"			,

		"VOUTA:VOLT"   ,
		"VOUTB:VOLT"   ,
		"VOUTA:VOLT?"   ,
		"VOUTB:VOLT?"   ,

		"TimA:Freq"		,	// float 1/period
		"TimA:Freq?"		,


		"KA:peri",
		"KA on",
		"KA off",
		"KA up",
		"KA?",

		"dbg:blink",
		"DBG:CPUID",
		"DBG:VoltToRaw",
		"DBG:RawToVolt",

};


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

			RX_TIMA_FREQ,
			RX_TIMA_FREQq,

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
{	int scpiID = -1;
	for(int idx = 0; idx < SCPI_IN_STRCNT; idx ++)
	{
			if ( 0 == strncasecmp(scpiCmd, scpiRxShortMsgs[idx], strlen(scpiRxShortMsgs[idx]) ) )
					scpiID = idx;
			else if ( 0 == strncasecmp(scpiCmd, scpiRxLongMsgs[idx], strlen(scpiRxLongMsgs[idx]) ) )
				scpiID = idx;
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
	return result;
}

static int getIntFromStr(char * source)
{	int result;
	sscanf( source, "%*s%d", &result);
	return result;
}

#endif /* SCPI_H_ */
