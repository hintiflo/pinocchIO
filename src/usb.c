// Virtual COM port implementation
// Originally published by W.S. at Nuvoton, improved by members of microcontroller.net
#include "usb.h"

// ==========================================================================================================
// Configurable stuff
// ==========================================================================================================

// For devices with 2x16 bits/word access schema
// (e.g. STM32L0x2, STM32L0x3, STM32F04x, STM32F072, STM32F078, STM32F303xD, STM32F303xE)
// #define UMEM_SHIFT 0
// #define UMEM_FAKEWIDTH uint16_t

// For devices with 1x16 bits/word access schema
// (e.g. STM32F103, STM32F302, STM32F303xB, STM32F303xBxC)
#define UMEM_SHIFT 1
#define UMEM_FAKEWIDTH uint32_t

// The name of the IRQ handler must match startup_stm32.s
#define NAME_OF_USB_IRQ_HANDLER USB_LP_CAN_RX0_IRQHandler

// Take the number from the reference manual of your µC.
#define USB_IRQ_NUMBER 20

// Vendor and product ID of the USB device
#define VID 1155
#define PID 22336

// Vendor descriptor string
const uint8_t VendorStringDescriptor[] =
{
    16,       // bLength
    3,        // bDescriptorType string
    'N', 0,
    'u', 0,
    'v', 0,
    'o', 0,
    't', 0,
    'o', 0,
    'n', 0
};

// Product descriptor string
const uint8_t ProductStringDescriptor[] =
{
    32,       // bLength
    3,        // bDescriptorType
    'U', 0,
    'S', 0,
    'B', 0,
    ' ', 0,
    'V', 0,
    'i', 0,
    'r', 0,
    't', 0,
    'u', 0,
    'a', 0,
    'l', 0,
    ' ', 0,
    'C', 0,
    'O', 0,
    'M', 0
};

// Serial number string
const uint8_t StringSerial[] =
{
    26,       // bLength
    3,        // bDescriptorType
    'N', 0,
    'T', 0,
    '2', 0,
    '0', 0,
    '0', 0,
    '9', 0,
    '1', 0,
    '0', 0,
    '1', 0,
    '4', 0,
    '0', 0,
    '0', 0
};

// ==========================================================================================================
// For debugging
// ==========================================================================================================

// Enable trace messages
#define ENABLE_TRACING 0

// The following function is used to output trace messages on the SWO pin
#if ENABLE_TRACING
    #include <stdio.h>

    // Cortex-M ITM registers
    #define ITM_PORT0_8   (*(volatile char *)(0xE0000000UL))
    #define ITM_PORT0_32  (*(volatile unsigned long *)(0xE0000000UL))
    #define ITM_TER       (*(volatile unsigned long *)(0xE0000E00UL))
    #define ITM_TCR       (*(volatile unsigned long *)(0xE0000E80UL))

    static void trace(char *ptr)
    {
        while (*ptr)
        {
            if (((ITM_TCR & 1UL) != 0UL) &&   // ITM enabled
                ((ITM_TER & 1UL) != 0UL))     // ITM and port 0 enabled
            {
                while (ITM_PORT0_32 == 0UL)
                {
                    __asm__ volatile("NOP");
                }
                ITM_PORT0_8 = *ptr;
            }
            ptr++;
        }
    }
#else
    #define trace(msg) // do nothing
#endif


// ==========================================================================================================
// Constants
// ==========================================================================================================

// Cortex-M NVIC Register
#define NVIC_ISER     (*(volatile uint32_t (*) [16])(0xE000E100))
#define NVIC_ICER     (*(volatile uint32_t (*) [16])(0xE000E180))

// USB device registers
// All USB registers are 16 bit width but they must be accessed with 32 bit instructions.
#define USB_BASE      0x40005C00
#define USB_EpRegs(x) (*(volatile uint32_t *)(0x40005C00 + 4*(x)))
#define USB_EP0R      (*(volatile uint32_t *)(0x40005C00))
#define USB_EP1R      (*(volatile uint32_t *)(0x40005C04))
#define USB_EP2R      (*(volatile uint32_t *)(0x40005C08))
#define USB_EP3R      (*(volatile uint32_t *)(0x40005C0C))
#define USB_EP4R      (*(volatile uint32_t *)(0x40005C10))
#define USB_EP5R      (*(volatile uint32_t *)(0x40005C14))
#define USB_EP6R      (*(volatile uint32_t *)(0x40005C18))
#define USB_EP7R      (*(volatile uint32_t *)(0x40005C1C))

#define USB_CNTR      (*(volatile uint32_t *)(0x40005C40))
#define USB_ISTR      (*(volatile uint32_t *)(0x40005C44))
#define USB_FNR       (*(volatile uint32_t *)(0x40005C48))
#define USB_DADDR     (*(volatile uint32_t *)(0x40005C4C))
#define USB_BTABLE    (*(volatile uint32_t *)(0x40005C50))

// Bits in USB_CNTR
#define  FRES         (1<<0)
#define  PDWN         (1<<1)
#define  LP_MODE      (1<<2)
#define  FSUSP        (1<<3)
#define  RESUME       (1<<4)

#define  ESOFM        (1<<8)
#define  SOFM         (1<<9)
#define  RESETM       (1<<10)
#define  SUSPM        (1<<11)
#define  WKUPM        (1<<12)
#define  ERRM         (1<<13)
#define  PMAOVRM      (1<<14)
#define  CTRM         (1<<15)

// Bits in USB_ISTR
#define  DIR          (1<<4)
#define  ESOF         (1<<8)
#define  SOF          (1<<9)
#define  RESET        (1<<10)
#define  SUSP         (1<<11)
#define  WKUP         (1<<12)
#define  ERR          (1<<13)
#define  PMAOVR       (1<<14)
#define  CTR          (1<<15)

// Bits in den USB_EPnR
#define  CTR_RX       (1<<15)
#define  DTOG_RX      (1<<14)
#define  STAT_RX      (3<<12)
#define  SETUP        (1<<11)
#define  EP_TYPE      (3<<9)
#define  EP_KIND      (1<<8)
#define  CTR_TX       (1<<7)
#define  DTOG_TX      (1<<6)
#define  STAT_TX      (3<<4)
#define  MASK_EA      (15)

// EndPoint register Mask (No Toggle Fields)
#define EP_NoToggleBits  (CTR_RX|SETUP|EP_TYPE|EP_KIND|CTR_TX|MASK_EA)

// ******* assignment of physical endpoints 0..7 ********************
#define  logEpCtrl      0
#define  logEpBulkIn    1
#define  logEpBulkOut   2
#define  logEpInt       3

// for statt, unstall, ...
#define  phys_In        0x80
#define  physEpCtrlIn   (0 + 0x80)
#define  physEpCtrlOut  0
#define  physEpBulkIn   (1 + 0x80)
#define  physEpBulkOut  2
#define  physEpIntIn    (3 + 0x80)
#define  physEpIntOut   3

/*
 Layout of the USB-RAM 1x16 bits/word access schema
 ======================================================
 (does not apply to devices with 2x16 bits/word access schema)

 From the CPU point of view the memory starts is in range 0x40006000 bis 0x400063FF, 1024 bytes.
 However, only a half of it is implemented, so there are only 512 bytes usable.

 Example:
 If the text to send is "Hello-World"
 0x40006000: 48 65 00 00 6C 6C 00 00 6F 2D 00 00 57 6F 00 00 72 6C 00 00 64 ...
              H  e        l  l        o  -        W  o        r  l        d ...

 At offset 0:
 Control_In   64 Bytes
 Control_Out  64 Bytes
 Bulk_In_A    64 Bytes (double buffered)
 Bulk_In_B    64 Bytes
 Bulk_Out_A   64 Bytes (double buffered)
 Bulk_Out_B   64 Bytes
 Int_In        8 Bytes (not used)
 Int_Out       8 Bytes (not used)
 -----------------------
 sum         400 Bytes

 After that comes the EpTable (which USB_BTABLE points to) with 4 entries (Control, BulkIn, BuklOut, Int)
 each with 4 double words = 64 bytes, so we have 400+64 bytes in total.
*/

#define USB_RAM       0x40006000

#define EpCtrlMaxLen  64
#define EpCtrlLenId   ((1<<15)|(1<<10))

#define EpBulkMaxLen  64
#define EpBulkLenId   ((1<<15)|(1<<10))

#define EpIntMaxLen    8
#define EpIntLenId    (4<<10)

// EP0 = control
#define Ep0TxOffset   0      // 64 Bytes at   0
#define Ep0RxOffset   64     // 64 Bytes at  64

// EP1 = Bulk-IN
#define Ep1TxAOffset  128    // 64 Bytes at 128
#define Ep1TxBOffset  192    // 64 Bytes at 192

// EP2 = Bulk-OUT
#define Ep2RxAOffset  256    // 64 Bytes at 256
#define Ep2RxBOffset  320    // 64 Bytes at 320

// EP3 = Int (unused)
#define Ep3TxOffset   384    // 8 Bytes at 384
#define Ep3RxOffset   292    // 8 Bytes at 392

// EP-table
#define EpTableOffset 400    // 64 Bytes at 400

#define EPControlTxBuffer (USB_RAM + (Ep0TxOffset<<UMEM_SHIFT))
#define EPControlRxBuffer (USB_RAM + (Ep0RxOffset<<UMEM_SHIFT))

#define EP1TxABuffer      (USB_RAM + (Ep1TxAOffset<<UMEM_SHIFT))
#define EP1TxBBuffer      (USB_RAM + (Ep1TxBOffset<<UMEM_SHIFT))

#define EP2RxABuffer      (USB_RAM + (Ep2RxAOffset<<UMEM_SHIFT))
#define EP2RxBBuffer      (USB_RAM + (Ep2RxBOffset<<UMEM_SHIFT))

#define EP3TxBuffer       (USB_RAM + (Ep3TxOffset<<UMEM_SHIFT))
#define EP3RxBuffer       (USB_RAM + (Ep3RxOffset<<UMEM_SHIFT))

struct TEpTableEntry
{
    UMEM_FAKEWIDTH TxOffset;
    UMEM_FAKEWIDTH TxCount;
    UMEM_FAKEWIDTH RxOffset;
    UMEM_FAKEWIDTH RxCount;
};

#define EpTable   ((struct TEpTableEntry *) (USB_RAM + (EpTableOffset<<UMEM_SHIFT)))

// Codes of the standard bRequest's in the setup package
#define GET_STATUS          0x00
#define CLEAR_FEATURE       0x01
#define SET_FEATURE         0x03
#define SET_ADDRESS         0x05
#define GET_DESCRIPTOR      0x06
#define SET_DESCRIPTOR      0x07
#define GET_CONFIGURATION   0x08
#define SET_CONFIGURATION   0x09
#define GET_INTERFACE       0x0A
#define SET_INTERFACE       0x0B
#define SYNC_FRAME          0x0C

// additional bRequest codes for virtual COM port
#define SET_LINE_CODE               0x20    // 7 byte package with baud rate, ...
#define GET_LINE_CODE               0x21    // 7 byte package with baud rate, ..
#define SET_CONTROL_LINE_STATE      0x22    // 2 bit  DTR and RTS
#define SEND_BREAK                  0x23    // not unused

// structure of the setup package
struct TSetupPaket
{
    uint8_t bmRequestType;  // see above
    uint8_t bRequest;       // see request table in USB documentation
    uint16_t wValue;        // depends on the request
    uint16_t wIndex;        // depends on the request
    uint16_t wLength;       // number of bytes in case of data storage
};

// structure of the command- and org-data block
struct TCommand
{
    struct TSetupPaket SetupPacket;  // the last setup package
    long TransferLen;                // number of bytes to send to the host
    long PacketLen;                  // how large the package to the host may be
    uint8_t* TransferPtr;            // points to the bytes to send
    bool RemoteWakeup;
    bool SelfPowered;
    uint8_t Configuration;
};

// Line coding structure
// 0-3 BaudRate     Data terminal rate (baudrate), in bits per second
// 4   bCharFormat  Stop bits: 0=1 Stop bit, 1=1.5 Stop bits, 2=2 Stop bits
// 5   bParityType  Parity:    0=None, 1=Odd, 2=Even, 3=Mark, 4=Space
// 6   bDataBits    Data bits: 5, 6, 7, 8, 16
struct T_LineCoding
{
    uint32_t BaudRate;   // Baud rate
    uint8_t Stopbits;    // stop bit
    uint8_t ParityType;  // parity
    uint8_t DataBits;    // data bits
};

// ==========================================================================================================
// Variables
// ==========================================================================================================

// Transmit buffer
volatile char UsbTxBuf[USB_TXLEN];
volatile int txr=0;
volatile int txw=0;

// Receive buffer
volatile char UsbRxBuf[USB_RXLEN];
volatile int rxr=0;
volatile int rxw=0;

// Status flags
volatile bool receiving = false;
volatile bool transmitting = false;
volatile bool suspended = false;
volatile bool configurationSet = false;

// counter of 1ms interrupts, stops if suspended
volatile uint32_t heartbeat = 0;

struct TCommand CMD;
struct T_LineCoding LineCoding;
uint16_t Dtr_Rts;
volatile uint8_t DeviceAddress=0;

// ==========================================================================================================
// Implementation
// ==========================================================================================================

// functions to start the virtual COM port:

void Class_Start(void)
{
    LineCoding.BaudRate = 9600;
    LineCoding.DataBits = 8;
    LineCoding.Stopbits = 0;
    LineCoding.ParityType = 0;
    Dtr_Rts = 0;
    receiving = true;
    transmitting = false;
}

bool Class_Compare(uint16_t aValue) // always true, not needed here
{
    return true;
}

// the device descriptors
#define LEN_DEVICE         18
#define DESC_DEVICE         1

const uint8_t DeviceDescriptor[LEN_DEVICE] =
{
    LEN_DEVICE,        // bLength
    DESC_DEVICE,       // bDescriptorType
    0x00, 0x02,        // bcdUSB
    0x02,              // bDeviceClass
    0x00,              // bDeviceSubClass
    0x00,              // bDeviceProtocol
    EpCtrlMaxLen,      // bMaxPacketSize0
    VID & 0xFF,        // Vendor  ID LO
    VID >> 8,          // Vendor  ID HI
    PID & 0xFF,        // Product ID LO
    PID >> 8,          // Product ID HI
    0x00, 0x01,        // bcdDevice
    0x01,              // iManufacturer
    0x02,              // iProduct
    0x03,              // iSerialNumber
    0x01               // bNumConfigurations
};

#define LEN_CONFIG          9
#define DESC_CONFIG         2

#define LEN_INTERFACE       9
#define DESC_INTERFACE      4

#define LEN_ENDPOINT        7
#define DESC_ENDPOINT       5

const uint8_t ConfigDescriptor[0x43] =
{
                              // CONFIG descriptor
    LEN_CONFIG,               // bLength
    DESC_CONFIG,              // bDescriptorType
    sizeof(ConfigDescriptor),
    0x00,                     // wTotalLength
    0x02,                     // bNumInterfaces
    0x01,                     // bConfigurationValue
    0x00,                     // iConfiguration
    0xC0,                     // bmAttributes
    0x32,                     // MaxPower

                              // INTERFACE descriptor
    LEN_INTERFACE,            // bLength
    DESC_INTERFACE,           // bDescriptorType
    0x00,                     // bInterfaceNumber
    0x00,                     // bAlternateSetting
    0x01,                     // bNumEndpoints
    0x02,                     // bInterfaceClass
    0x02,                     // bInterfaceSubClass
    0x01,                     // bInterfaceProtocol
    0x00,                     // iInterface

                              // Communication Class Specified INTERFACE descriptor
    0x05,                     // Size of the descriptor, in Bytes
    0x24,                     // CS_INTERFACE descriptor type
    0x00,                     // Header functional descriptor subtype
    0x10, 0x01,               // Communication device compliant to the communication spec. ver. 1.10

                              // Communication Class Specified INTERFACE descriptor
    0x05,                     // Size of the descriptor, in Bytes
    0x24,                     // CS_INTERFACE descriptor type
    0x01,                     // Call management functional descriptor
    0x00,                     // BIT0: Whether device handle call management itself.
                              // BIT1: Whether device can send/receive call
                              // management information over a Data Class Interface 0
    0x01,                     // Interface number of data class interface optionally used for call management

                              // Communication Class Specified INTERFACE descriptor
    0x04,                     // Size of the descriptor, in Bytes
    0x24,                     // CS_INTERFACE descriptor type
    0x02,                     // Abstract control management functional descriptor subtype
    0x00,                     // bmCapabilities

                              // Communication Class Specified INTERFACE descriptor
    0x05,                     // bLength
    0x24,                     // bDescriptorType: CS_INTERFACE descriptor type
    0x06,                     // bDescriptorSubType
    0x00,                     // bMasterInterface
    0x01,                     // bSlaveInterface0

                              // ENDPOINT descriptor for interrupt
    LEN_ENDPOINT,             // bLength
    DESC_ENDPOINT,            // bDescriptorType
    0x80 + logEpInt,          // bEndpointAddress
    3,                        // Attribute: Interrupt
    EpIntMaxLen, 0x00,        // wMaxPacketSize
    0x01,                     // bInterval

                              // INTERFACE descriptor
    LEN_INTERFACE,            // bLength
    DESC_INTERFACE,           // bDescriptorType
    0x01,                     // bInterfaceNumber
    0x00,                     // bAlternateSetting
    0x02,                     // bNumEndpoints
    0x0A,                     // bInterfaceClass
    0x00,                     // bInterfaceSubClass
    0x00,                     // bInterfaceProtocol
    0x00,                     // iInterface

                              // ENDPOINT descriptor for Bulk IN
    LEN_ENDPOINT,             // bLength
    DESC_ENDPOINT,            // bDescriptorType
    0x80 + logEpBulkIn,       // bEndpointAddress
    2,                        // Attribute: Bulk
    EpBulkMaxLen, 0x00,       // wMaxPacketSize
    0,                        // bInterval try 2ms

                              // ENDPOINT descriptor for Bulk OUT
    LEN_ENDPOINT,             // bLength
    DESC_ENDPOINT,            // bDescriptorType
    logEpBulkOut,             // bEndpointAddress
    2,                        // Attribute: Bulk
    EpBulkMaxLen, 0x00,       // wMaxPacketSize
    0                         // bInterval try 2ms
};

#define DESC_STRING         3

const uint8_t StringLang[] =
{
    4,                        // bLength
    DESC_STRING,              // bDescriptorType
    0x09, 0x04                // Language ID: USA(0x0409)
};


// to send a null byte or empty package
const uint8_t always0 = 0;

// helper functions

void EnableUsbIRQ(void)
{
    NVIC_ISER[USB_IRQ_NUMBER/32] = ((uint32_t) 1) << (USB_IRQ_NUMBER % 32);
}

void DisableUsbIRQ(void)
{
    NVIC_ICER[USB_IRQ_NUMBER/32] = ((uint32_t) 1) << (USB_IRQ_NUMBER % 32);
}

void Stall(int physEpNum)
{
    trace("stall\n");
    uint32_t D, S, Maske;
    int logEpNum;
    logEpNum = physEpNum & 0x0F;

    if (logEpNum == physEpNum)
    {
        Maske = EP_NoToggleBits | STAT_RX; // without STAT_TX and no DTOG_x
        S = 1 << 12;
    }
    else
    {
        Maske = EP_NoToggleBits | STAT_TX; // without STAT_RX and no DTOG_x
        S = 1 << 4;
    }
    D = USB_EpRegs(logEpNum);
    USB_EpRegs(logEpNum) = (D ^ S) & Maske;
}

void UnStall(int physEpNum)
{
    trace("unstall\n");
    uint32_t D,Maske;
    uint32_t status;
    if ((physEpNum & 0x80)==0)
    {
        // it is an Out Endpoint
        status = (USB_EpRegs(physEpNum & 0x07) >> 12) & 0x03;
        Maske  = EP_NoToggleBits;
        if (status==1) // Out stalled?
        {
            // toggle bit 13:12  01 -> 11
            D = USB_EpRegs(physEpNum & 0x07) & Maske;
            USB_EpRegs(physEpNum & 0x07) = (D | (2 << 12));
        }
    }
    else
    {
        // it is an In Endpoint
        status = (USB_EpRegs(physEpNum & 0x07) >> 4) & 0x03;
        Maske  = EP_NoToggleBits;
        if (status==1) //In stalled?
        {
            // toggle bit 5:4  01 -> 10
            D = USB_EpRegs(physEpNum & 0x07) & Maske;
            USB_EpRegs(physEpNum & 0x07) = (D | (3 << 4));
        }
    }
}

void StallLogEP(int logEpNum)
{
    Stall(logEpNum);
    Stall(logEpNum | phys_In);
}


// prepare endpoint for receiving, set STAT_RX to 11 via toggle
void ClearBuffer(int logEpNum)
{
    #if ENABLE_TRACING
        char buf[30];
        sprintf(buf,"clrBuf logEpNum=%i\n",logEpNum);
        trace(buf);
    #endif
    uint32_t D, Maske;
    Maske = EP_NoToggleBits | STAT_RX; // without STAT_TX and no DTOG_x
    D = USB_EpRegs(logEpNum);
    USB_EpRegs(logEpNum) = (D ^ STAT_RX) & Maske;
}

// prepare endpoint for sending, set STAT_TX to 11 via toggle
void ValidateBuffer(int logEpNum)
{
    #if ENABLE_TRACING
        char buf[30];
        sprintf(buf,"validateBuf logEpNum=%i\n",logEpNum);
        trace(buf);
    #endif
    uint32_t D, Maske;
    Maske = EP_NoToggleBits | STAT_TX; // without STAT_RX and no DTOG_x
    D = USB_EpRegs(logEpNum);
    USB_EpRegs(logEpNum) = (D ^ STAT_TX) & Maske;
}

bool USB_SetAddress(uint8_t adr)
{
    #if ENABLE_TRACING
        char buf[30];
        sprintf(buf,"setAddr adr=%i\n",adr);
        trace(buf);
    #endif
    USB_DADDR = 0x80 | adr;
    return true;
}

bool USB_ConfigDevice(bool obConf)
{
    return true;  // nothing to do in this USB core
}

// setup physical endpoints, e.g. after a reset command

void InitEndpoints(void)
{
    trace("InitEndpoints\n");
    USB_CNTR = 1;          // disable reset and int
    CMD.Configuration = 0; // nothing before CONFIGURED
    CMD.TransferLen = 0;   // nothing to transfer
    CMD.PacketLen = 0;     // nothing to transfer
    CMD.TransferPtr = 0;
    USB_CNTR = 0;          // all int off

    suspended = false;
    configurationSet = false;
    transmitting = false;
    receiving = false;

    // EP0 = Control, IN and OUT
    EpTable[0].TxOffset = Ep0TxOffset;
    EpTable[0].TxCount = 0;
    EpTable[0].RxOffset = Ep0RxOffset;
    EpTable[0].RxCount = EpCtrlLenId;

    // EP1 = Bulk IN (only IN)
    EpTable[1].TxOffset = Ep1TxAOffset;
    EpTable[1].TxCount = 0;
    EpTable[1].RxOffset = Ep1TxBOffset; // here 2nd tx buffer
    EpTable[1].RxCount = EpBulkLenId;

    // EP2 = Bulk OUT (only OUT)
    EpTable[2].TxOffset = Ep2RxAOffset;
    EpTable[2].TxCount = EpBulkLenId;
    EpTable[2].RxOffset = Ep2RxBOffset;
    EpTable[2].RxCount = EpBulkLenId;

    // EP3 = Int, IN and OUT
    EpTable[3].TxOffset = Ep3TxOffset;
    EpTable[3].TxCount = EpIntLenId;
    EpTable[3].RxOffset = Ep3RxOffset;
    EpTable[3].RxCount = EpIntLenId;

    USB_BTABLE = EpTableOffset;

    USB_EP0R =
        (3 << 12) |              // STAT_RX = 3, rx enabled
        (2 << 4) |               // STAT_TX = 2, send nak
        (1 << 9) |               // EP_TYPE = 1, control
        logEpCtrl;

    USB_EP1R =
        (0 << 12) |              // STAT_RX = 0, rx disabled
        (2 << 4) |               // STAT_TX = 2, send nak
        (0 << 9) |               // EP_TYPE = 0, bulk
        logEpBulkIn;

    USB_EP2R =
        (3 << 12) |              // STAT_RX = 3, rx enabled
        (0 << 4) |               // STAT_TX = 0, tx disabled
        (0 << 9) |               // EP_TYPE = 0, bulk
        logEpBulkOut;

    USB_EP3R =
        (3 << 12) |              // STAT_RX = 3, rx enabled
        (2 << 4) |               // STAT_TX = 2, send nak
        (3 << 9) |               // EP_TYPE = 0, bulk
        logEpInt;

    USB_ISTR = 0;                // remove pending interrupts
    USB_CNTR =
        CTRM |                   // Int after ACK packages in or out
        RESETM |                 // Int after reset
        SUSPM | WKUPM | ESOFM |
        SOFM;                    // Int every 1ms frame
    USB_SetAddress(0);
}

// for small delays
void Nop(volatile uint32_t count)
{
    while (count)
    {
        __asm__ volatile ("NOP");
        count--;
    }
}


int ReadControlBlock(uint8_t* PBuffer, int maxlen)
{
    int count, i, n;
    UMEM_FAKEWIDTH D;
    UMEM_FAKEWIDTH* P;

    count = EpTable[0].RxCount & 0x3FF;
    #if ENABLE_TRACING
        char buf[40];
        sprintf(buf,"rdCtrlBlock maxlen=%i, count=%i\n",maxlen,count);
        trace(buf);
    #endif
    if (count > maxlen)
    {
        count = maxlen;
    }

    if (count)
    {
        P = (UMEM_FAKEWIDTH*) EPControlRxBuffer;
        n = 2;
        i = count;
        D = *P++;
        while (i > 0)
        {
            *PBuffer = D & 0xFF;
            D = D >> 8;
            --n;
            if (!n)
            {
                D = *P++;
                n = 2;
            }
            --i;
            ++PBuffer;
        }
    }
    ClearBuffer(logEpCtrl);
    return count;
}

int WriteControlBlock(uint8_t* PBuffer, int count)
{
    #if ENABLE_TRACING
        char buf[30];
        sprintf(buf,"wrCtrlBlock count=%i\n",count);
        trace(buf);
    #endif
    UMEM_FAKEWIDTH A, L;
    UMEM_FAKEWIDTH* P;
    int i, n;

    if (count > EpCtrlMaxLen)
    {
        count = EpCtrlMaxLen;
    }
    EpTable[0].TxCount = count;

    if (count)
    {
        A = 0;
        i = 0;
        n = 0;
        P = (UMEM_FAKEWIDTH*) EPControlTxBuffer;
        while (i < count)
        {
            L = *PBuffer++;
            A = A | (L << n);
            n += 8;
            if (n > 8)
            {
                n = 0;
                *P++ = A;
                A = 0;
            }
            ++i;
        }
        if (n)
        {
            *P = A;
        }
    }
    ValidateBuffer(logEpCtrl);
    // Nop(100); // Add extra delay if enumerations fails
    return count;
}

void ACK(void)
{
    WriteControlBlock((uint8_t*) &always0, 0);
}

// identify request type in setup package (Standard, Class, Vendor)
bool IsStandardRequest(void)
{
    return (CMD.SetupPacket.bmRequestType & 0x60) == 0;
}

bool IsClassRequest(void)
{
    return (CMD.SetupPacket.bmRequestType & 0x60) == 0x20;
}

bool IsVendorRequest(void)
{
    return (CMD.SetupPacket.bmRequestType & 0x60) == 0x40;
}

// start control block control transfer to the host
void DescriptorBlockwiseIn(void)
{
    int i, j;
    uint8_t* Q;

    if ((CMD.SetupPacket.bmRequestType & 0x80) == 0)
    {
        trace("bmRequestType & 0x80 ==0\n");
        return;
    }
    i = CMD.TransferLen;
    if (i > CMD.PacketLen)
    {
        i = CMD.PacketLen;
    }
    Q = CMD.TransferPtr; // source
    j = WriteControlBlock(Q, i);
    CMD.TransferPtr = Q + j;               // pointer to the rest
    CMD.TransferLen = CMD.TransferLen - j; // rest number of bytes
    if (CMD.TransferLen < 0)
    {
        CMD.TransferLen = 0;
    }
}

// Functions for processing incoming requests:

// USB-Request "SET FEATURE" and "CLEAR FEATURE"
void DoSetClearFeature(bool value)
{
    int Feature;
    int FuerWen;
    int EP;

    Feature = CMD.SetupPacket.wValue;
    FuerWen = CMD.SetupPacket.bmRequestType;
    EP = CMD.SetupPacket.wIndex;

    #if ENABLE_TRACING
        char buf[30];
        sprintf(buf,"doSetClearFeature for %02x\n",FuerWen);
        trace(buf);
    #endif

    switch (FuerWen)
    {
        case 0: // for device
            trace("forDevice\n");
            if (Feature == 1)
            {
                CMD.RemoteWakeup = value;
            }
            break;

        case 1: // for interface
            trace("forInterface\n");
            break;

        case 2: // for endpoint
            trace("forEndPoint\n");
            if (Feature == 0) //Feature Stall
            {
                switch(EP)
                {
                   case 0x81: //bulk in
                   case 0x02: //bulk out
                   case 0x83: //int in
                        if (value)
                        {
                            Stall(EP);
                        }
                        else
                        {
                            UnStall(EP);
                        }
                        ACK();
                        break;
                   default:
                        StallLogEP(logEpCtrl);
                }
            }
            else StallLogEP(logEpCtrl);
            break;

        default:
            trace("forElse\n");
            StallLogEP(logEpCtrl); // send nak
    }
}

// USB-Request "GET STATUS"
void DoGetStatus(void)
{
    uint8_t Buf[4];
    int FuerWen;
    int EP;
    FuerWen = CMD.SetupPacket.bmRequestType;
    EP = CMD.SetupPacket.wIndex;

    Buf[0] = 0;
    Buf[1] = 0;
    #if ENABLE_TRACING
        char buf[30];
        sprintf(buf,"doGetStatus for %02x\n",FuerWen);
        trace(buf);
    #endif

    switch (FuerWen)
    {
        case 0x80: // for device
            trace("forDevice\n");
            if (CMD.RemoteWakeup)
            {
                Buf[0] |= 2;
            }
            if (CMD.SelfPowered)
            {
                Buf[0] |= 1;
            }
            break;

        case 0x81: // for interface
            trace("forInterface\n");
            break;

        case 0x82: // for endpoint
            trace("forEndpoint\n");
            switch (EP)
            {
               // commented out because only required for bulk + int eps
               // case 0x80: //ctrl in
               // case 0x00: //ctrl out

               case 0x02: //bulk out stall condition
                    if (((USB_EpRegs(2) >> 12) & 0x03) ==1)
                    {
                        Buf[0] = 1;
                    }
                    break;

               case 0x81: //bulk in stall condition
                    if (((USB_EpRegs(1) >>  4) & 0x03) ==1)
                    {
                        Buf[0] = 1;
                    }
                    break;

               case 0x83: //interrupt in stall condition
                    if (((USB_EpRegs(3) >>  4) & 0x03) ==1)
                    {
                        Buf[0] = 1;
                    }
                    break;

               default:
                    trace("unsuported ep for OnGetStatus\n");
                    StallLogEP(logEpCtrl); // unknown, stall
                    return;
            }
            break;

        default:
            trace("forElse\n");
            StallLogEP(logEpCtrl); // send nak
            return;
    }

    //Buf[1] = 0;
    CMD.PacketLen = EpCtrlMaxLen;
    CMD.TransferLen = 2;
    CMD.TransferPtr = Buf;
    DescriptorBlockwiseIn();
}

// Send descriptors to the host
void DoGetDescriptor(void)
{
    uint16_t Type, Index;
    int aLen;
    const uint8_t* P;

    Type = CMD.SetupPacket.wValue >> 8;
    Index = CMD.SetupPacket.wValue & 0xFF;
    aLen = -1;
    P = 0;

    #if ENABLE_TRACING
        char buf[30];
        sprintf(buf,"doGetDescr type %04x\n",Type);
        trace(buf);
    #endif

    switch (Type)
    {
        case DESC_DEVICE: // Get Device Descriptor
            trace("descDevice\n");
            {
                aLen = LEN_DEVICE;
                P = DeviceDescriptor;
                break;
            }

        case DESC_CONFIG: // Get Configuration Descriptor
            trace("descConfig\n");
            {
                aLen = ConfigDescriptor[3]; // Total length is WORD
                aLen = (aLen << 8) | ConfigDescriptor[2];
                P = ConfigDescriptor;
                break;
            }

        case DESC_STRING: // Get String Descriptor
        {
            trace("descString\n");
            switch (Index)
            {
                case 0:
                    aLen = 4;
                    P = StringLang;
                    break;
                case 1:
                    aLen = VendorStringDescriptor[0];
                    P = VendorStringDescriptor;
                    break;
                case 2:
                    aLen = ProductStringDescriptor[0];
                    P = ProductStringDescriptor;
                    break;
                case 3:
                    aLen = StringSerial[0];
                    P = StringSerial;
                    break;
                default:
                    StallLogEP(logEpCtrl); // unknown, stall
                    aLen = -1;
            }
            break;
        }
        default:
            trace("descElse\n");
            {
                StallLogEP(logEpCtrl); // unknown, stall
                aLen = -1;
            }
    }

    if (aLen < 0)
        return;

    // do not send more that the host wants
    if (aLen > CMD.SetupPacket.wLength)
    {
        aLen = CMD.SetupPacket.wLength;
    }
    CMD.PacketLen = EpCtrlMaxLen;
    CMD.TransferLen = aLen;
    CMD.TransferPtr = (uint8_t*) P;
    DescriptorBlockwiseIn();
}

// received an adderss
void DoSetAddress(void)
{
    ACK();
    DeviceAddress=CMD.SetupPacket.wValue;
}

// USB-Request "SET CONFIGURATION"
void DoSetConfiguration(void)
{
    bool haveConfig;

    haveConfig = Class_Compare(CMD.SetupPacket.wValue);
    if (CMD.SetupPacket.wValue == 0)
    {
        CMD.Configuration = CMD.SetupPacket.wValue & 0xFF;
        configurationSet = false;
    }
    else if (haveConfig)
    {
        USB_ConfigDevice(true);
        Class_Start();
        CMD.Configuration = CMD.SetupPacket.wValue & 0xFF;
        configurationSet = true;
        ACK();
    }
    else
    {
        CMD.Configuration = 0;
        configurationSet = false;
        Stall(0);
    }
}

// CDC specific functions:

// "SET LINE CODING"
void VCOM_SetLineCoding(void)
{
    ACK();
    // prepare to receive 7 bytes from host ???
}

// data output for CDC specific USB-Request "SET LINE CODING"
void SetLineCodingDataOut(void)
{
    // read 7 bytes from USB buffer
    ReadControlBlock((uint8_t*) &LineCoding, 7);
    ACK();
}

// remember DTR and RTS from host
void VCOM_Read_DTR_RTS(void)
{
    Dtr_Rts = CMD.SetupPacket.wValue >> 8;
    ACK();
}

// CDC specific USB-Request "GET LINE CODING"
void VCOM_GetLineCoding(void)
{
    CMD.PacketLen = EpCtrlMaxLen;
    CMD.TransferLen = 7;
    CMD.TransferPtr = (uint8_t*) &LineCoding;
    DescriptorBlockwiseIn(); // send 7 bytes to the host
}

// Setup event

void OnSetup(uint16_t EpCtrlStatus)
{
    ReadControlBlock(&CMD.SetupPacket.bmRequestType, 8);

    if (IsStandardRequest()) // if type=standard
    {
        trace("IsStandardRequest\n");
        switch (CMD.SetupPacket.bRequest)
        {
            case SET_ADDRESS:
                trace("SET_ADDRESS\n");
                DoSetAddress();
                return;

            case CLEAR_FEATURE:
                trace("CLEAR_FEATURE\n");
                DoSetClearFeature(false);
                return;

            case SET_FEATURE:
                trace("SET_FEATURE\n");
                DoSetClearFeature(true);
                return;

            case GET_CONFIGURATION:
                trace("GET_CONFIGURATION\n");
                CMD.PacketLen = EpCtrlMaxLen;
                CMD.TransferLen = 1;
                CMD.TransferPtr = (uint8_t*) &CMD.Configuration;
                DescriptorBlockwiseIn();
                return;

            case GET_STATUS:
                trace("GET_STATUS\n");
                DoGetStatus();
                return;

            case GET_INTERFACE:
                trace("GET_INTERFACE\n");
                CMD.TransferLen = 1;
                CMD.TransferPtr = (uint8_t*) &always0;
                DescriptorBlockwiseIn();
                return;

            case SET_INTERFACE:
                trace("SET_INTERFACE\n");
                Class_Start();
                ACK();
                return;

            case GET_DESCRIPTOR:
                trace("GET_DESCRIPTOR\n");
                DoGetDescriptor();
                return;

            case SET_CONFIGURATION:
                trace("SET_CONFIGURATION\n");
                DoSetConfiguration();
                return;
        }
        // everything else gets responded with stall, see below
    }

    if (IsClassRequest()) // if type=class
    {
        trace("IsClassRequest\n");
        switch (CMD.SetupPacket.bRequest)
        {
            case SET_LINE_CODE:
                trace("SET_LINE_CODE\n");
                VCOM_SetLineCoding();
                return;

            case GET_LINE_CODE:
                trace("GET_LINE_CODE\n");
                VCOM_GetLineCoding();
                return;

            case SET_CONTROL_LINE_STATE:
                trace("SET_CONTROL_LINE_STATE\n");
                VCOM_Read_DTR_RTS();
                return;
        }
    }

    if (IsVendorRequest()) // if type=vendor
    {
        trace("IsVendorRequest\n");
    }

    // for all other cases responded with stall
    trace("Stall\n");
    Stall(0);
}

// endpoint interrupts

void OnEpCtrlOut(uint16_t EpCtrlStatus) // Control-EP OUT
{
    uint8_t tbuf[EpCtrlMaxLen];

    if (IsStandardRequest()) // if type=standard
    {
        // expect empty ACK from host but could be also bRequest=7 = SET_DESCRIPTOR (never seen that)
        trace("IsStandardRequest\n");
        ReadControlBlock(tbuf, EpCtrlMaxLen);
        return;
    }

    if (IsClassRequest()) // if type=class
    {
        trace("IsClassRequest\n");
        switch (CMD.SetupPacket.bRequest)
        {
            case SET_LINE_CODE:
                trace("SET_LINE_CODE\n");
                SetLineCodingDataOut();
                ACK(); // TODO: check if really needed, SetLineCodingDataOut() calls ACK() already
                break;

            default:
                trace("default\n");
                ACK();
        }
        return;
    }

    // we do not check for vendor request here
    ACK();
}

void OnEpCtrlIn(uint16_t EpCtrlStatus) // Control-EP IN
{
    if (IsStandardRequest()) // if type=standard
    {
        trace("IsStandardRequest\n");
        switch (CMD.SetupPacket.bRequest)
        {
            case GET_DESCRIPTOR:
                trace("GET_DESCRIPTOR\n");
                if (CMD.TransferLen > 0)
                {
                    DescriptorBlockwiseIn();
                }
                break;
            case GET_LINE_CODE:
                trace("GET_LINE_CODE\n");
                ACK();
                break;
            default:
                trace("default\n");
                ACK();
        }
        return;
    }
}

// BULK IN and OUT Interrupts

void EpBulkBeginTransmit(void)
{
    int i, n;
    UMEM_FAKEWIDTH L, A;
    UMEM_FAKEWIDTH* P;

    P = (UMEM_FAKEWIDTH*) EP1TxABuffer;
    i = txw - txr;
    if (i < 0)
    {
        i += USB_TXLEN; // i = number of bytes to send
    }
    if (i > EpBulkMaxLen)
    {
        i = EpBulkMaxLen;
    }

    // the last package shall be less than 64 bytes
    // otherwise it gets lost sometimes.
    else if (i==EpBulkMaxLen)
    {
      i=EpBulkMaxLen-2;
    }

    A = 0;
    n = 0;
    EpTable[1].TxCount = (i & 0x3FF) | EpBulkLenId;
    transmitting = true;

    while (i)
    {
        L = UsbTxBuf[txr];
        txr = (txr + 1) & (USB_TXLEN - 1);
        A = A | (L << n);
        n += 8;
        if (n > 8)
        {
            *P++ = A;
            n = 0;
            A = 0;
        }
        --i;
    }
    if (n)
    {
        *P = A; // output remaining bytes, if any
    }
    ValidateBuffer(logEpBulkIn);
}

// EP1 = Bulk-EP IN
void OnEpBulkIn(void)
{
    if (txr == txw)
    {
        transmitting = false;
    }
    else
    {
        EpBulkBeginTransmit();
    }
}

// EP2 = Bulk-EP OUT
void OnEpBulkOut(void)
{
    int i, n, hdroom, avail;
    UMEM_FAKEWIDTH D;
    char c;
    UMEM_FAKEWIDTH* P;

    // select bulk EP and calculate number of bytes to read
    avail = EpTable[2].RxCount & 0x3FF;

    i = rxw - rxr;
    if (i < 0)
    {
        i += USB_RXLEN;
    }
    hdroom = USB_RXLEN - i;
    if (hdroom <= avail)
    {
        receiving = false;
        return;
    }

    P = (UMEM_FAKEWIDTH*) EP2RxBBuffer;
    n = 2;
    i = avail;
    D = *P++; // load 2 bytes
    while (i > 0)
    {
        c = D & 0xFF; // LSB first
        UsbRxBuf[rxw] = c;
        rxw = (rxw + 1) & (USB_RXLEN - 1);
        D = D >> 8;
        --n;
        if (!n)
        {
            D = *P++;
            n = 2;
        }
        --i;
    }
    if (hdroom - avail >= EpBulkMaxLen)
    {
        ClearBuffer(logEpBulkOut); // we read it
    }
    else
    {
        receiving = false;
    }
}

// Int-EP IN
void OnEpIntIn(void)
{
    // do nothing
}

// Int-EP IN
void OnEpIntOut(void)
{
    // do nothing
}

// USB-Interrupt-Handlers

void NAME_OF_USB_IRQ_HANDLER(void)
{
    //trace("irq\n");
    uint32_t I;
    int EpNum;
    uint16_t EpStatus;

    I = USB_ISTR; // Interrupt-Status

    if (I & PMAOVR) // internal Timeout...
    {
        trace("PMAOVR\n");
        USB_ISTR = ~PMAOVR; // clear int
    }

    if (I & ERR) // data transfer error
    {
        trace("ERR\n");
        USB_ISTR = ~ERR; // clear int
    }

    if (I & WKUP) // Suspend-->Resume
    {
        trace("WKUP\n");
        USB_CNTR &= ~(FSUSP | LP_MODE);
        USB_ISTR = ~WKUP; // clear int
        suspended = false;
    }

    if (I & SUSP) // after 3 ms Pause --> Suspend
    {
        trace("SUSP\n");
        USB_ISTR = ~SUSP; // clear int
        USB_CNTR |= (FSUSP | LP_MODE);
        suspended = true;
    }

    if (I & RESET) // Bus Reset
    {
        trace("RESET\n");
        USB_ISTR = ~RESET; // clear int
        InitEndpoints();
        return;
    }

    if (I & SOF) // Start of Frame, comes every 1 ms
    {
        //trace("SOF\n");
        USB_ISTR = ~SOF; // clear int
        heartbeat++;
        if (suspended)
        {
            trace("SOF -> unsuspend\n");
            suspended = false;
        }
    }

    if (I & ESOF) // if a SOF package is missing
    {
        //trace("ESOF\n");
        USB_ISTR = ~ESOF; // clear int
        if (!suspended)
        {
            trace("ESOF -> suspend\n");
            suspended = true;
        }
    }

    // Endpoint Interrupts
    if (I & CTR)
    {
        trace("CTR ");
        USB_ISTR = ~CTR; // clear int
        EpNum = I & MASK_EA; // which EP?
        EpStatus = USB_EpRegs(EpNum); // read EP status

        if (I & DIR) // OUT, also package received
        {
            trace("out\n");
            USB_EpRegs(EpNum) = EpStatus & ~CTR_RX & EP_NoToggleBits;
            if (EpNum == logEpCtrl)
            {
                trace("logEpCtrl\n");
                if (EpStatus & SETUP)
                {
                    trace("SETUP\n");
                    OnSetup(EpStatus); // Handle the Setup-package
                }
                else
                {
                    trace("EpCtrlOut\n");
                    OnEpCtrlOut(EpStatus); // class specific
                }
            }
            else if (EpNum == logEpBulkOut)
            {
                trace("logEpBulkOut\n");
                OnEpBulkOut();
            }
            else if (EpNum == logEpInt)
            {
                trace("logEpInt\n");
                OnEpIntOut();
            }
        }
        else // IN, also package sent
        {
            // Apply new device address
            if (DeviceAddress)
            {
                USB_SetAddress(DeviceAddress);
                DeviceAddress=0;
            }

            trace("in\n");
            USB_EpRegs(EpNum) = EpStatus & ~CTR_TX & EP_NoToggleBits;
            if (EpNum == logEpCtrl)
            {
                trace("logEpCtrl\n");
                OnEpCtrlIn(EpStatus);
            }
            else if (EpNum == logEpBulkIn)
            {
                trace("logEpBulkIn\n");
                OnEpBulkIn();
            }
            else if (EpNum == logEpInt)
            {
                trace("logEpInt\n");
                OnEpIntIn();
            }
        }
    }
}

// ==========================================================================================================
// Application interface
// ==========================================================================================================
// Called by the application outside interrupt context


// USB-Setup
// Clock must be set up already before
void UsbSetup(void)
{
    trace("setup\n");
    uint32_t* P;

    P = (uint32_t*) USB_RAM; // clear RAM
    while ((uint32_t) P < (USB_RAM + 1024))
    {
        *P++ = 0;
    }

    CMD.Configuration = 0;
    configurationSet = false;
    suspended = false;

    Class_Start();            // setup LineCoding-Block with defaults
    USB_CNTR = 3;             // Powerdown+Reset
    Nop(100);                 // wait
    USB_CNTR = 1;             // Reset
    USB_ISTR = 0;             // clear spurious Ints
    Nop(1000);                // wait
    EnableUsbIRQ();
    InitEndpoints();
}

// true if there receive buffer contains at least one character
bool UsbRxAvail(void)
{
    bool res = rxr != rxw;
    return res;
}

// Check if the receive buffer contains the given character.
bool UsbRxBufferContains(char c)
{
    int i = rxr;
    while (i != rxw)
    {
        if (UsbRxBuf[i]==c)
        {
            return true;
        }
        i = (i + 1) & (USB_RXLEN - 1);
    }
    return false;
}

// Wait until a character has been received.
// Returns true on success, aborts on USB suspend.
bool UsbRxWait(uint32_t timeout_ms)
{
    uint32_t start=heartbeat;
    while(1)
    {
        if (rxr != rxw)
        {
            // success
            return true;
        }

        if (!configurationSet || suspended)
        {
            // USB is inactive
            return false;
        }

        if (heartbeat-start <= timeout_ms)
        {
            // timeout
            return false;
        }

        __asm__ volatile ("wfi"); // wait
    }
}


// Receive a character, wait if necessary.
// Returns false if nothing received, aborts on timeout and USB suspend.
bool UsbGetChar_noWait(char* c)
{
    if (rxr != rxw)
    {
        // read from buffer
        *c = UsbRxBuf[rxr];
        rxr = (rxr + 1) & (USB_RXLEN - 1);

        // trigger receiving
        if (!receiving && !suspended && configurationSet)
        {
            DisableUsbIRQ();
            int i, hdroom;

            i = rxw - rxr;
            if (i < 0)
            {
                i += USB_RXLEN;
            }
            hdroom = USB_RXLEN - i;

            if (hdroom > EpBulkMaxLen)
            {
                receiving = true;
                ClearBuffer(logEpBulkOut);
            }
            EnableUsbIRQ();
        }
        return true;
    }
    else
    {
        return false;
    }
}

// Receive a character with timeout.
// Returns true on success, aborts on USB suspend.
bool UsbGetChar(char* c, uint32_t timeout_ms)
{
    if (UsbRxWait(timeout_ms))
    {
        return UsbGetChar_noWait(c);
    }
    else
    {
        return false;
    }
}


// Receive a string until the given size limit, terminator or timeout is reached.
// The received terminator is included in the returned string and also counted.
// Returns number of received characters, aborts on USB suspend.
// If the requested terminator is not '\0', then this gets appended to the string automatically but not counted.
int UsbGetStr(char* buffer, int bufSize, char terminator, uint32_t timeout_ms)
{
    // If the termination character is not '\0' then reserve space for the additional string terminator.
    if(terminator!='\0')
    {
        bufSize--;
    }

    uint32_t start=heartbeat;
    int count=0;
    while(count<bufSize)
    {
        // wait for received character
        while(1)
        {
            if (rxr != rxw)
            {
                // ready read received byte, stop waiting
                break;
            }

            if (!configurationSet || suspended)
            {
                // USB is inactive, abort
                goto done;
            }

            if (heartbeat-start <= timeout_ms)
            {
                // timeout, abort
                goto done;
            }

            __asm__ volatile ("wfi"); // wait
        }

        // get the received character
        char c;
        if (UsbGetChar_noWait(&c))
        {
            buffer[count]=c;
            count++;

            // Stop loop when the terminator has been received
            if (c==terminator)
            {
                goto done;
            }
        }
    }

    done:
    if(terminator!='\0')
    {
        buffer[count]='\0';
    }
    return count;
}

// Receive bytes until the given size limit or timeout is reached.
// Returns number of received bytes, aborts on USB suspend.
int UsbGetBytes(void* buffer, int bufSize, uint32_t timeout_ms)
{
    uint32_t start=heartbeat;
    int count=0;
    while(count<bufSize)
    {
        // wait for received byte
        while(1)
        {
            if (rxr != rxw)
            {
                // ready read received byte, stop waiting
                break;
            }

            if (!configurationSet || suspended)
            {
                // USB is inactive, abort
                goto done;
            }

            if (heartbeat-start <= timeout_ms)
            {
                // timeout, abort
                goto done;
            }

            __asm__ volatile ("wfi"); // wait
        }

        // get the received byte
        char c;
        if (UsbGetChar_noWait(&c))
        {
            ((char*)buffer)[count]=c;
            count++;
        }
    }

    done:
    return count;
}


// true if the USB connection is established and not suspended
bool UsbActive (void)
{
    bool res = configurationSet && !suspended;
    return res;
}

// true if at least one character can be sent
bool UsbTxReady(void)
{
    bool res = configurationSet && !suspended && ((txw + 1) & (USB_TXLEN - 1)) != txr;
    return res;
}

// true if the send buffer is totally empty
bool UsbTxEmpty(void)
{
    bool res = (txw == txr);
    return res;
}

// query free space in send buffer
int UsbTxFree(void)
{
    int i;
    i = txw - txr;
    if (i < 0)
    {
        i = i + USB_TXLEN;
    }
    return USB_TXLEN - i;
}

// Trigger sending the remaining characters from the send buffer (asynchronously, not blocking)
void UsbTxFlush (void)
{
    if (!transmitting)
    {
        DisableUsbIRQ();
        EpBulkBeginTransmit();
        EnableUsbIRQ();
    }
}

// Wait until a character can be sent.
// Returns true on success, aborts on USB suspend.
bool UsbTxWait(uint32_t timeout_ms)
{
    uint32_t start=heartbeat;
    while(1)
    {
        if (!configurationSet || suspended)
        {
            // USB is inactive
            return false;
        }

        if (((txw + 1) & (USB_TXLEN - 1)) != txr)
        {
            // ready to send
            return true;
        }

        if (heartbeat-start > timeout_ms)
        {
            // timeout
            return false;
        }

        __asm__ volatile ("wfi"); // wait
    }
}

// Send a character to the host (via send buffer).
// Returns false if the buffer is full.
bool UsbSendChar_noWait(char c)
{
    // check space
    int i = (txw + 1) & (USB_TXLEN - 1);
    if (i == txr)
    {
        return false;
    }

    // write into the buffer
    UsbTxBuf[txw] = c;
    txw = i;

    // Comment in the following condition to delay sending until the buffer is full
    // Then you may call UsbTxFlush() to trigger sending

    // if (((txw + 1) & (txLen - 1)) == txr)
    {
        // trigger sending
        if (!transmitting)
        {
            DisableUsbIRQ();
            EpBulkBeginTransmit();
            EnableUsbIRQ();
        }
    }
    return true;
}

// Send a character to the host (via send buffer).
// Waits if the buffer is full, returns false on timeout or USB suspend.
bool UsbSendChar(char c, uint32_t timeout_ms)
{
    if (UsbTxWait(timeout_ms))
    {
        return UsbSendChar_noWait(c);
    }
    else
    {
        return false;
    }
}

// Send a string to the host.
// Waits if buffer is full, aborts on timeout or USB suspend.
// Returns the number of characters that have been sent.
int UsbSendStr(char* str, uint32_t timeout_ms)
{
    uint32_t start=heartbeat;
    int count=0;
    while (*str)
    {
        // Wait until there is space in the send buffer
        while(1)
        {
            if (!configurationSet || suspended)
            {
                // USB is inactive, abort
                return count;
            }

            if (((txw + 1) & (USB_TXLEN - 1)) != txr)
            {
                // ready to send, stop waiting
                break;
            }

            if (heartbeat-start > timeout_ms)
            {
                // timeout, abort
                return count;
            }

            __asm__ volatile ("wfi"); // wait
        }

        // send the character
        char c = *str;
        if (UsbSendChar_noWait(c))
        {
            str++;
            count++;
        }
    }
    return count;
}

// Send some bytes to the host.
// Waits if buffer is full, aborts on timeout or USB suspend.
int UsbSendBytes(void* bytes, int size, uint32_t timeout_ms)
{
    uint32_t start=heartbeat;
    int count=0;
    while (count<size)
    {
        // Wait until there is space in the send buffer
        while(1)
        {
            if (!configurationSet || suspended)
            {
                // USB is inactive, abort
                return count;
            }

            if (((txw + 1) & (USB_TXLEN - 1)) != txr)
            {
                // ready to send, stop waiting
                break;
            }

            if (heartbeat-start > timeout_ms)
            {
                // timeout, abort
                return count;
            }

            __asm__ volatile ("wfi"); // wait
        }

        // send the byte
        char c = *(char*)bytes;
        if (UsbSendChar_noWait(c))
        {
            bytes++;
            count++;
        }
    }
    return count;
}

// Clear the receive buffer, remaining data get lost.
void UsbRxClear(void)
{
    DisableUsbIRQ();
    rxr=0;
    rxw=0;
    EnableUsbIRQ();
}

// Clear the transfer buffer, remaining data get lost.
void UsbTxClear(void)
{
    DisableUsbIRQ();
    txr=0;
    txw=0;
    EnableUsbIRQ();
}
