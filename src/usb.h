// Virtual COM port implementation
// Originally published by W.S. at Nuvoton, improved by members of microcontroller.net
#ifndef __USB_H__
#define __USB_H__

#include <stdint.h>
#include <stdbool.h>

// Buffer size for sending characters, must be 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, ...
#define USB_TXLEN 256

// Buffer size for received characters, must be 128, 256, 512, 1024, 2048, ...
#define USB_RXLEN 256



// ==========================================================================================================
// Description of the USB driver
// ==========================================================================================================
/*
            Scope of usb.c
       ___________________________
      /                           \
 ______         ________         ___________             ___________         ________         ______
|      |       |        |       |           |           |           |       |        |       |      |
|      |--->---| Tx buf |--->---|           |           |           |--->---| Rx buf |--->---|      |
| Your |       |________|       | STM32 USB |           | PC USB    |       |________|       | PC   |
| app  |        ________        | interface |===========| interface |        ________        | app  |
|      |       |        |       |           |           |           |       |        |       |      |
|      |---<---| Rx buf |---<---|           |           |           |---<---| Tx buf |---<---|      |
|______|       |________|       |___________|           |___________|       |________|       |______|
   |                                 |                         |                                 |
   |                                 |                         |                                 |
   +---------------------------->----+                         +----<----------------------------+
         Control commands                                                Control commands


The USB communication is driven by the PC. The PC tells when it wants to receive or send data and how
much (up to 64 bytes per package). In addition the PC sends a SOF package every 1 ms to check if the
connection is alive.

When the SOF packages are missing, then the USB interface goes into suspend mode. Then the app may read
remaining characters from the Rx buffer but sending is blocked (except function UsbSendChar_noWait(),
which can fill the Tx buffer anyway).

Whenever the µC receives a package from the PC, an interrupt occurs. So the whole communication is driven
by interrupts. There is no need for a background thread. The ISR must respond every package, or the PC
will drop the connection (after some few repetitions). You have to avoid blocking interrupts for more
than one 1ms.

The application interface provides many functions which can wait until a timeout of some milliseconds.
Please note that this timeout depends on counting the regular SOF packages from PC. So the actual time
may be a little longer than requested, specially when the connection drops.

When the connection drops, the buffers are not cleared. You can re-establish the connection, then the
data transfer continues without loosing a single character.

There is no way for the STM32 to find out if an application on the PC side has opened the virtual COM port.
If the virtual COM port is not opened, the Tx buffer will fill up quickly. But you can check the USB
connection with function UsbActive().

When the Rx buffer of the STM32 is full, then the PC cannot send more data to the STM32. In this case,
the Tx buffer on the PC side will fill up. It's up to the PC application what happens then.

For the other direction where the STM32 send data to the PC, a timeout may occur when the communication
gets stuck. The API reports back how many characters have been written into the buffer. Your application
must decide what to do in that case (discard the remaining characters or try to repeat).

See configurable settings in usb.c
*/

// ==========================================================================================================
// Application interface
// ==========================================================================================================

// Start the USB core
void UsbSetup(void);

// true if the USB connection is established and not suspended
bool UsbActive(void);

// true if there receive buffer contains at least one character
bool UsbRxAvail(void);

// Check if the receive buffer contains the given character.
bool UsbRxBufferContains(char c);

// Wait until a character has been received.
// Returns true on success, aborts on USB suspend.
bool UsbRxWait(uint32_t timeout_ms);

// Get a received character from the buffer.
// Returns false if there was nothing.
bool UsbGetChar_noWait(char* c);

// Receive a character, wait if necessary.
// Returns false if nothing received, aborts on timeout and USB suspend.
bool UsbGetChar(char* c, uint32_t timeout_ms);

// Receive a string until the given size limit, terminator or timeout is reached.
// The received terminator is included in the returned string and also counted.
// Returns number of received characters, aborts on USB suspend.
// If the requested terminator is not '\0', then this gets appended to the string automatically but not counted.
int UsbGetStr(char* buffer, int bufSize, char terminator, uint32_t timeout_ms);

// Receive bytes until the given size limit or timeout is reached.
// Returns number of received bytes, aborts on USB suspend.
int UsbGetBytes(void* buffer, int bufSize, uint32_t timeout_ms);

// Clear the receive buffer, remaining data get lost.
void UsbRxClear(void);

// true if at least one character can be sent
bool UsbTxReady(void);

// true if the send buffer is totally empty
bool UsbTxEmpty(void);

// Query free space in send buffer
int UsbTxFree(void);

// Wait until a character can be sent.
// Returns true on success, aborts on USB suspend.
bool UsbTxWait(uint32_t timeout_ms);

// Send a character to the host (via send buffer).
// Returns false if the buffer is full.
bool UsbSendChar_noWait(char c);

// Send a character to the host (via send buffer).
// Waits if the buffer is full, returns false on timeout or USB suspend.
bool UsbSendChar(char c, uint32_t timeout_ms);

// Send a string to the host.
// Waits if buffer is full, aborts on timeout or USB suspend.
// Returns the number of characters that have been sent.
int UsbSendStr(char* str, uint32_t timeout_ms);

// Send some bytes to the host.
// Waits if buffer is full, aborts on timeout or USB suspend.
int UsbSendBytes(void* bytes, int size, uint32_t timeout_ms);

// Trigger sending the remaining characters from the send buffer (asynchronously, not blocking)
void UsbTxFlush(void);

// Clear the transfer buffer, remaining data get lost.
void UsbTxClear(void);


#endif
