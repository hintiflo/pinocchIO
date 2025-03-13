
/**
 * @file	usbCDC.h
 * @author	Florian Hinterleitner
 * @date	21.01.2023
 * @brief	provides buffering/queueing of incoming and outgoing USB messages and wraps the Nuvotron-usb layer
 * 
 */

#include "usb.h"
#include "stdbool.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

// char usbRxBuf[USB_RXLEN][100];
char usbTxBuf[USB_TXLEN] = "";

#define STD_TX_TIMEOUT 10		// milli seconds
#define STD_RX_TIMEOUT STD_TX_TIMEOUT

// #define USB_ENQUEUEING


#define usbEnq			usbCdcTxEnqueueStr	// short hand alias for long ass function name
#define usbEnqStrs		usbCdcTxEnqueueStrs



/**
 * @brief	provides active status of the USB connection
 * @retval	true if the USB connection is established and not suspended
 */
bool usbCdcActive(){	return UsbActive();	}


/**
 * @brief	Start the USB core
 * USB-Setup
 * Clock must be set up already before
 * @retval	TRUE on success, FALSE otherwise
 */
bool usbCdcInit()
{	UsbSetup();		//
	return usbCdcActive();
}

/**
 * @brief	provides status of the RX buffer
 * @retval	true if the receive buffer contains at least one character
 */
bool usbCdcRxAvail() {	return UsbRxAvail();	}

/**
 * @brief	Check if the receive buffer contains the given character.
 * @retval	true if the receive buffer contains the given character
 */
bool usbCdcRxBufferContains(char c){	return UsbRxBufferContains(c);	}

bool usbCdcRxWait(uint32_t timeout_ms){	return UsbRxWait(timeout_ms);	}		// Wait until a character has been received. Returns true on success, aborts on USB suspend.
bool usbCdcGetChar_noWait(char* c){		return UsbGetChar_noWait(c);	}		// Get a received character from the buffer. Returns false if there was nothing.

// Receive a string until the given size limit, terminator or timeout is reached.
// The received terminator is included in the returned string and also counted.
// Returns number of received characters, aborts on USB suspend.
// If the requested terminator is not '\0', then this gets appended to the string automatically but not counted.
int  usbCdcGetStr(char* buffer, int bufSize, char terminator, uint32_t timeout_ms){	return  UsbGetStr(buffer, bufSize, terminator, timeout_ms);	}
bool usbCdcGetChar(char* c, uint32_t timeout_ms){	return UsbGetChar(c, timeout_ms);	}	// Receive a character, wait if necessary. Returns false if nothing received, aborts on timeout and USB suspend.
int  usbCdcGetBytes(void* buffer, int bufSize, uint32_t timeout_ms){	return  UsbGetBytes(buffer, bufSize, timeout_ms);	}	// Receive bytes until the given size limit or timeout is reached. Returns number of received bytes, aborts on USB suspend.
void usbCdcRxClear(){	UsbRxClear();	}								// Clear the receive buffer, remaining data get lost.
bool usbCdcTxReady(){	return UsbTxReady();	}						// true if at least one character can be sent
bool usbCdcTxEmpty(){	return UsbTxEmpty();	}						// true if the send buffer is totally empty
int  usbCdcTxFree(){	 return  UsbTxFree();	}						// Query free space in send buffer

// Wait until a character can be sent.
// Returns true on success, aborts on USB suspend.
bool usbCdcTxWait(uint32_t timeout_ms){	return UsbTxWait(timeout_ms);	}

// Send a character to the host (via send buffer).
// Returns false if the buffer is full.
bool usbCdcSendChar_noWait(char c){	return UsbSendChar_noWait(c);	}

// Send a character to the host (via send buffer).
// Waits if the buffer is full, returns false on timeout or USB suspend.
bool usbCdcSendChar(char c, uint32_t timeout_ms){	return UsbSendChar(c, timeout_ms);	}

/**
 * @brief	Send a string to the host.
 * 			Waits if buffer is full, aborts on timeout or USB suspend.
 * @retval	number of transmitted chars
 */
int	 usbCdcSendStr(char* str, uint32_t timeout_ms){	return	 UsbSendStr(str, timeout_ms);	}

int	 usbCdcSendBytes(void* bytes, int size, uint32_t timeout_ms){	return  UsbSendBytes(bytes, size, timeout_ms);	}	// Send some bytes to the host.  Waits if buffer is full, aborts on timeout or USB suspend.
void usbCdcTxFlush(){	UsbTxFlush();	}								// Trigger sending the remaining characters from the send buffer (asynchronously, not blocking)
void usbCdcTxClear(){	UsbTxClear();}									// Trigger sending the remaining characters from the send buffer (asynchronously, not blocking)

/*
 * @brief	take a given number 'num' of strings, assemble into one string and append a float value
 *
 * float value printed in scientific notation with three decimal digits
 *
 * */
bool usbCdcTxEnqueueStrsFloat(float flott, int num, ...);

/*
 * @brief	take a given number 'num' of strings, assemble into one string and append an integer value
 *
 * */
bool usbCdcTxEnqueueStrsInt(int intt, int num, ...);

bool usbCdcTxEnqueueStrsInt(int intt, int num, ...)
{
	va_list valist;
	va_start(valist, num);
	for(int idx=0; idx<num; idx++)
		strcat(usbTxBuf, va_arg(valist, const char * ));
	va_end(valist);

	char intBuf[25];
	sprintf(intBuf, " %d", intt);

	strcat(usbTxBuf, intBuf);
	strcat(usbTxBuf, "\n");

	int sentChars = usbCdcSendStr(usbTxBuf, STD_TX_TIMEOUT);
	if( strlen(usbTxBuf) == sentChars )
	{	usbTxBuf[0] = 0;
		memset(usbTxBuf, 0, strlen(usbTxBuf));
		return true;
	}else

	return false;
}


/// \todo move to scpi module, also all other string manipulations
bool usbCdcTxEnqueueStrsFloat(float flott, int num, ...) // ...variable number of strings, then a float
{
	va_list valist;
	va_start(valist, num);
	for(int idx=0; idx<num; idx++)
		strcat(usbTxBuf, va_arg(valist, const char * ));
	va_end(valist);

	char floatBuf[25];
	sprintf(floatBuf, " %2.3e", flott);

	strcat(usbTxBuf, floatBuf);
	strcat(usbTxBuf, "\n");

	int sentChars = usbCdcSendStr(usbTxBuf, STD_TX_TIMEOUT);
	if( strlen(usbTxBuf) == sentChars )
	{	usbTxBuf[0] = 0;
		memset(usbTxBuf, 0, strlen(usbTxBuf));
		return true;
	}else

	return false;
}

bool usbCdcTxEnqueueStrs(int num, ...); // ...varying number of strings

/*
 * brief	take a given number 'num' of strings, assemble into one string and
 *
 *
 *
 *
 * */
bool usbCdcTxEnqueueStrs(int num, ...)
{

	va_list valist;
	va_start(valist, num);

	for(int idx=0; idx<num; idx++)
		strcat(usbTxBuf, va_arg(valist, const char * ));
	va_end(valist);

	strcat(usbTxBuf, "\n");

	int sentChars = usbCdcSendStr(usbTxBuf, STD_TX_TIMEOUT);
	if( strlen(usbTxBuf) == sentChars )
	{	usbTxBuf[0] = 0;
		memset(usbTxBuf, 0, strlen(usbTxBuf));
		return true;
	}else

	return false;
}


/**
 * @brief	enqueues strings for transmission
 * @retval	number of transmitted chars
 */
bool usbCdcTxEnqueueStr(const char * str)
{
	#ifdef USB_ENQUEUEING
		return strcat(usbTxBuf, str);
	#endif // USB_ENQUEUEING

	#ifndef USB_ENQUEUEING
		// strcat(usbTxBuf, "\n");
		sprintf(usbTxBuf, "%s \n", str);
		//		return	 strlen(usbTxBuf) == usbCdcSendStr(usbTxBuf, STD_TX_TIMEOUT);
		int sentChars = usbCdcSendStr(usbTxBuf, STD_TX_TIMEOUT);
		if( strlen(usbTxBuf) == sentChars )
		{	usbTxBuf[0] = 0;
			memset(usbTxBuf, 0, strlen(usbTxBuf));
			return true;
		}else
			return false;
	#endif // USB_ENQUEUEING

}

#ifndef USB_ENQUEUEING
	#define usbTxFlush()	;					// empty placeholder for later func
#endif // USB_ENQUEUEING

#ifdef USB_ENQUEUEING
	bool usbTxFlush()
	{
		strcat(usbTxBuf, " \n");
		int sentChars = usbCdcSendStr(usbTxBuf, STD_TX_TIMEOUT);
		if( strlen(usbTxBuf) == sentChars )
		{
			usbTxBuf[0] = 0;
			memset(usbTxBuf, 0, strlen(usbTxBuf));
			return true;
		}
		else
			return false;
	}
#endif // USB_ENQUEUEING

bool usbCdcRxDequeue(char* buffer, int bufSize, char terminator)
{
	int res = 0;
	res = usbCdcGetStr(buffer, bufSize, terminator, STD_RX_TIMEOUT);
	if(res > 0)
		return true;
	else
		return false;
}

