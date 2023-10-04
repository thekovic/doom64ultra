/***************************************************************
                            usb.c

Allows USB communication between an N64 flashcart and the PC
using UNFLoader.
https://github.com/buu342/N64-UNFLoader
***************************************************************/

#include "i_usb.h"
#include "i_debug.h"
#include "doomdef.h"
#ifdef USB
#include <os_internal.h>
#include <internal/osint.h>
#endif

// Settings
#define DEBUG_ADDRESS_SIZE 8*1024*1024 // Max size of USB I/O. The bigger this value, the more ROM you lose!

/*********************************
           Data macros
*********************************/

// Input/Output buffer size. Always keep it at 512
#define BUFFER_SIZE 512

// USB Memory location
#define DEBUG_ADDRESS (0x04000000 - DEBUG_ADDRESS_SIZE) // Put the debug area at the 64MB - DEBUG_ADDRESS_SIZE area in ROM space

// Data header related
#define USBHEADER_CREATE(type, left) ((((type)<<24) | ((left) & 0x00FFFFFF)))

// Protocol related
#define USBPROTOCOL_VERSION 2
#define HEARTBEAT_VERSION   1

// Data types defintions
#define DATATYPE_TEXT       0x01
#define DATATYPE_RAWBINARY  0x02
#define DATATYPE_HEADER     0x03
#define DATATYPE_SCREENSHOT 0x04
#define DATATYPE_HEARTBEAT  0x05
#define DATATYPE_RDBPACKET  0x06


/*********************************
        Convenience macros
*********************************/

// Use these to conveniently read the header from usb_poll()
#define USBHEADER_GETTYPE(header) (((header) & 0xFF000000) >> 24)
#define USBHEADER_GETSIZE(header) (((header) & 0x00FFFFFF))


/*********************************
          64Drive macros
*********************************/

#define D64_COMMAND_TIMEOUT       1000
#define D64_WRITE_TIMEOUT         1000

#define D64_BASE                  0x10000000
#define D64_REGS_BASE             0x18000000

#define D64_REG_STATUS            (D64_REGS_BASE + 0x0200)
#define D64_REG_COMMAND           (D64_REGS_BASE + 0x0208)

#define D64_REG_MAGIC             (D64_REGS_BASE + 0x02EC)

#define D64_REG_USBCOMSTAT        (D64_REGS_BASE + 0x0400)
#define D64_REG_USBP0R0           (D64_REGS_BASE + 0x0404)
#define D64_REG_USBP1R1           (D64_REGS_BASE + 0x0408)

#define D64_CI_BUSY               0x1000

#define D64_MAGIC                 0x55444556

#define D64_CI_ENABLE_ROMWR       0xF0
#define D64_CI_DISABLE_ROMWR      0xF1

#define D64_CUI_ARM               0x0A
#define D64_CUI_DISARM            0x0F
#define D64_CUI_WRITE             0x08

#define D64_CUI_ARM_MASK          0x0F
#define D64_CUI_ARM_IDLE          0x00
#define D64_CUI_ARM_UNARMED_DATA  0x02

#define D64_CUI_WRITE_MASK        0xF0
#define D64_CUI_WRITE_IDLE        0x00
#define D64_CUI_WRITE_BUSY        0xF0


/*********************************
         EverDrive macros
*********************************/

#define ED_TIMEOUT        1000

#define ED_BASE           0x10000000
#define ED_BASE_ADDRESS   0x1F800000

#define ED_REG_USBCFG     (ED_BASE_ADDRESS | 0x0004)
#define ED_REG_VERSION    (ED_BASE_ADDRESS | 0x0014)
#define ED_REG_USBDAT     (ED_BASE_ADDRESS | 0x0400)
#define ED_REG_SYSCFG     (ED_BASE_ADDRESS | 0x8000)
#define ED_REG_KEY        (ED_BASE_ADDRESS | 0x8004)

#define ED_USBMODE_RDNOP  0xC400
#define ED_USBMODE_RD     0xC600
#define ED_USBMODE_WRNOP  0xC000
#define ED_USBMODE_WR     0xC200

#define ED_USBSTAT_ACT    0x0200
#define ED_USBSTAT_RXF    0x0400
#define ED_USBSTAT_TXE    0x0800
#define ED_USBSTAT_POWER  0x1000
#define ED_USBSTAT_BUSY   0x2000

#define ED_REGKEY         0xAA55

#define ED25_VERSION      0xED640007
#define ED3_VERSION       0xED640008
#define ED7_VERSION       0xED640013


/*********************************
            SC64 macros
*********************************/

#define SC64_WRITE_TIMEOUT          1000

#define SC64_BASE                   0x10000000
#define SC64_REGS_BASE              0x1FFF0000

#define SC64_REG_SR_CMD             (SC64_REGS_BASE + 0x00)
#define SC64_REG_DATA_0             (SC64_REGS_BASE + 0x04)
#define SC64_REG_DATA_1             (SC64_REGS_BASE + 0x08)
#define SC64_REG_IDENTIFIER         (SC64_REGS_BASE + 0x0C)
#define SC64_REG_KEY                (SC64_REGS_BASE + 0x10)

#define SC64_SR_CMD_ERROR           (1 << 30)
#define SC64_SR_CMD_BUSY            (1 << 31)

#define SC64_V2_IDENTIFIER          0x53437632

#define SC64_KEY_RESET              0x00000000
#define SC64_KEY_UNLOCK_1           0x5F554E4C
#define SC64_KEY_UNLOCK_2           0x4F434B5F

#define SC64_CMD_CONFIG_SET         'C'
#define SC64_CMD_USB_WRITE_STATUS   'U'
#define SC64_CMD_USB_WRITE          'M'
#define SC64_CMD_USB_READ_STATUS    'u'
#define SC64_CMD_USB_READ           'm'

#define SC64_CFG_ROM_WRITE_ENABLE   1

#define SC64_USB_WRITE_STATUS_BUSY  (1 << 31)
#define SC64_USB_READ_STATUS_BUSY   (1 << 31)


/*********************************
       Function Prototypes
*********************************/

static void I_FindCartUSB(void);

#ifdef USB
static void usb_sendheartbeat(void);

static void usb_64drive_write(int datatype, const void* data, int size);
static void usb_64drive_write_start(int datatype, int size);
static void usb_64drive_write_part(const void* data, int size);
static void usb_64drive_write_end(int datatype, int size);
static u32  usb_64drive_poll(void);
static void usb_64drive_read(void);

static void usb_everdrive_write(int datatype, const void* data, int size);
static void usb_everdrive_write_start(int datatype, int size);
static void usb_everdrive_write_part(const void* data, int size);
static void usb_everdrive_write_end(int datatype, int size);
static u32  usb_everdrive_poll(void);
static void usb_everdrive_read(void);

static void usb_sc64_write(int datatype, const void* data, int size);
static void usb_sc64_write_start(int datatype, int size);
static void usb_sc64_write_part(const void* data, int size);
static void usb_sc64_write_end(int datatype, int size);
static u32  usb_sc64_poll(void);
static void usb_sc64_read(void);
#endif


/*********************************
             Globals
*********************************/

s8 FlashCart = CART_NONE;
s8 IsEmulator = 0;

#ifdef USB
// Function pointers
void (*UsbFuncWrite)(int datatype, const void* data, int size);
void (*UsbFuncWriteStart)(int datatype, int size);
void (*UsbFuncWritePart)(const void* data, int size);
void (*UsbFuncWriteEnd)(int datatype, int size);
u32  (*UsbFuncPoll)(void);
void (*UsbFuncRead)(void);

// USB globals
static u8 usb_buffer_align[BUFFER_SIZE] ALIGNED(16);
static u8* usb_buffer;
static u32 write_address;
static char usb_didtimeout = FALSE;
static int usb_datatype = 0;
static int usb_datasize = 0;
static int usb_dataleft = 0;
static int usb_readblock = -1;

OSMesg      dmaMessageBuf;
OSIoMesg    dmaIOMessageBuf;
OSMesgQueue dmaMessageQ;

// USB thread definitions
#define USB_THREAD_ID    14
#define USB_THREAD_PRI   140
#define USB_THREAD_STACK 0x2000

#define MSG_READ        0x11
#define MSG_WRITE       0x12
#define MSG_WRITE_START 0x13
#define MSG_WRITE_PART  0x14
#define MSG_WRITE_END   0x15

#define USBERROR_NONE    0
#define USBERROR_NOTTEXT 1
#define USBERROR_UNKNOWN 2
#define USBERROR_TOOMUCH 3
#define USBERROR_CUSTOM  4

#define COMMAND_TOKENS 10

/*********************************
             Structs
*********************************/

// Thread message struct
typedef struct
{
    int msgtype;
    int datatype;
    void* buff;
    int size;
} usbMesg;

// Debug command struct
struct
{
    volatile int   current;
    volatile int   totaltokens;
    volatile int   incoming_start[COMMAND_TOKENS];
    volatile int   incoming_size[COMMAND_TOKENS];
    const char* volatile error;
} CommandParser;


/*********************************
        Function Prototypes
*********************************/

// Threads
static void I_USBThread(void *arg);

// USB thread globals
static OSMesgQueue usbMessageQ;
static OSMesg      usbMessageBuf;
static OSThread    usbThread;
static vu64        usbThreadStack[USB_THREAD_STACK/sizeof(u64)];

#endif /* USB */

/*********************************
      I/O Wrapper Functions
*********************************/

/*==============================
    usb_io_read
    Reads a 32-bit value from a
    given address using the PI.
    @param  The address to read from
    @return The 4 byte value that was read
==============================*/

static INLINE_ALWAYS u32 usb_io_read(u32 pi_address)
{
    u32 value;
    osPiReadIo(pi_address, &value);
    return value;
}


/*==============================
    usb_io_write
    Writes a 32-bit value to a
    given address using the PI.
    @param  The address to write to
    @param  The 4 byte value to write
==============================*/

static INLINE_ALWAYS void usb_io_write(u32 pi_address, u32 value)
{
    osPiWriteIo(pi_address, value);
}


#ifdef USB
/*==============================
    usb_dma_read
    Reads arbitrarily sized data from a
    given address using DMA.
    @param  The buffer to read into
    @param  The address to read from
    @param  The size of the data to read
==============================*/

static inline void usb_dma_read(void *ram_address, u32 pi_address, size_t size)
{
    osWritebackDCache(ram_address, size);
    osInvalDCache(ram_address, size);
    osPiStartDma(&dmaIOMessageBuf, OS_MESG_PRI_NORMAL, OS_READ, pi_address, ram_address, size, &dmaMessageQ);
    osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
}


/*==============================
    usb_dma_write
    writes arbitrarily sized data to a
    given address using DMA.
    @param  The buffer to read from
    @param  The address to write to
    @param  The size of the data to write
==============================*/

static inline void usb_dma_write(void *ram_address, u32 pi_address, size_t size)
{
    osWritebackDCache(ram_address, size);
    osPiStartDma(&dmaIOMessageBuf, OS_MESG_PRI_NORMAL, OS_WRITE, pi_address, ram_address, size, &dmaMessageQ);
    osRecvMesg(&dmaMessageQ, NULL, OS_MESG_BLOCK);
}


/*********************************
         Timeout helpers
*********************************/

/*==============================
    usb_timeout_start
    Returns current value of COUNT coprocessor 0 register
    @return C0_COUNT value
==============================*/

static INLINE_ALWAYS u32 usb_timeout_start(void)
{
    return osGetCount();
}


/*==============================
    usb_timeout_check
    Checks if timeout occurred
    @param Starting value obtained from usb_timeout_start
    @param Timeout duration specified in milliseconds
    @return TRUE if timeout occurred, otherwise FALSE
==============================*/

static char usb_timeout_check(u32 start_ticks, u32 duration)
{
    u64 current_ticks = (u64)osGetCount();
    u64 timeout_ticks = OS_USEC_TO_CYCLES((u64)duration * 1000);
    if (current_ticks < start_ticks)
        current_ticks += 0x100000000ULL;
    if (current_ticks >= (start_ticks + timeout_ticks))
        return TRUE;
    return FALSE;
}
#endif


/*********************************
          USB functions
*********************************/

/*==============================
    I_InitFlashCart
    Initializes the USB buffers and pointers
    @returns 1 if the USB initialization was successful, 0 if not
==============================*/

SEC_STARTUP void I_InitFlashCart(void)
{
#ifdef USB
    // Initialize the debug related globals
    usb_buffer = (u8*)OS_DCACHE_ROUNDUP_ADDR(usb_buffer_align);
    bzero(usb_buffer,  BUFFER_SIZE);

    osCreateMesgQueue(&dmaMessageQ, &dmaMessageBuf, 1);
#endif

    // Find the flashcart
    I_FindCartUSB();

#ifdef USB
    // Set the function pointers based on the flashcart
    switch (FlashCart)
    {
        case CART_64DRIVE:
            UsbFuncWrite      = usb_64drive_write;
            UsbFuncWriteStart = usb_64drive_write_start;
            UsbFuncWritePart  = usb_64drive_write_part;
            UsbFuncWriteEnd   = usb_64drive_write_end;
            UsbFuncPoll       = usb_64drive_poll;
            UsbFuncRead       = usb_64drive_read;
            break;
        case CART_EVERDRIVE:
            UsbFuncWrite      = usb_everdrive_write;
            UsbFuncWriteStart = usb_everdrive_write_start;
            UsbFuncWritePart  = usb_everdrive_write_part;
            UsbFuncWriteEnd   = usb_everdrive_write_end;
            UsbFuncPoll       = usb_everdrive_poll;
            UsbFuncRead       = usb_everdrive_read;
            break;
        case CART_SC64:
            UsbFuncWrite      = usb_sc64_write;
            UsbFuncWriteStart = usb_sc64_write_start;
            UsbFuncWritePart  = usb_sc64_write_part;
            UsbFuncWriteEnd   = usb_sc64_write_end;
            UsbFuncPoll       = usb_sc64_poll;
            UsbFuncRead       = usb_sc64_read;
            break;
        default:
            return;
    }

    // Create the message queue for the USB message
    osCreateMesgQueue(&usbMessageQ, &usbMessageBuf, 1);

#ifdef DEBUG_MEM
    usbThreadStack[0] = STACK_GUARD;
#endif
    // Initialize the USB thread
    osCreateThread(&usbThread, USB_THREAD_ID, I_USBThread, 0,
                    (void*)(usbThreadStack+USB_THREAD_STACK/sizeof(u64)),
                    USB_THREAD_PRI);
    osStartThread(&usbThread);
#endif
}


/*==============================
    usb_findcart
    Checks if the game is running on a 64Drive, EverDrive or a SC64.
==============================*/

static void I_FindCartUSB(void)
{
    u32 buff;

    // Before we do anything, check that we are using an emulator
    // Check the RDP clock register.
    // Always zero on emulators
    if (IO_READ(0xA4100010) == 0) // DPC_CLOCK_REG in Libultra
    {
        IsEmulator = 1;
        return;
    }

    // Fallback, harder emulator check.
    // The VI has an interesting quirk where its values are mirrored every 0x40 bytes
    // It's unlikely that emulators handle this, so we'll write to the VI_TEST_ADDR register and readback 0x40 bytes from its address
    // If they don't match, we probably have an emulator
    buff = IO_READ(0xA4400038);
    IO_WRITE(0xA4400038, 0x6ABCDEF9);
    if (IO_READ(0xA4400038) != IO_READ(0xA4400078))
    {
        IO_WRITE(0xA4400038, buff);
        IsEmulator = 1;
        return;
    }
    IO_WRITE(0xA4400038, buff);

    // Read the cartridge and check if we have a 64Drive.
    if (usb_io_read(D64_REG_MAGIC) == D64_MAGIC)
    {
        FlashCart = CART_64DRIVE;
        return;
    }

    // Since we didn't find a 64Drive let's assume we have an EverDrive
    // Write the key to unlock the registers, then read the version register
    usb_io_write(ED_REG_KEY, ED_REGKEY);
    buff = usb_io_read(ED_REG_VERSION);

    // EverDrive 2.5 not compatible
    if (buff == ED25_VERSION)
        return;

    // Check if we have an EverDrive
    if (buff == ED7_VERSION || buff == ED3_VERSION)
    {
        // Set the USB mode
        usb_io_write(ED_REG_SYSCFG, 0);
        usb_io_write(ED_REG_USBCFG, ED_USBMODE_RDNOP);

        // Set the cart to EverDrive
        FlashCart = CART_EVERDRIVE;
        return;
    }

    // Since we didn't find an EverDrive either let's assume we have a SC64
    // Write the key sequence to unlock the registers, then read the identifier register
    usb_io_write(SC64_REG_KEY, SC64_KEY_RESET);
    usb_io_write(SC64_REG_KEY, SC64_KEY_UNLOCK_1);
    usb_io_write(SC64_REG_KEY, SC64_KEY_UNLOCK_2);

    // Check if we have a SC64
    if (usb_io_read(SC64_REG_IDENTIFIER) == SC64_V2_IDENTIFIER)
    {
        // Set the cart to SC64
        FlashCart = CART_SC64;
        return;
    }
}

#ifdef USB

/*==============================
    usb_write
    Writes data to the USB.
    Will not write if there is data to read from USB
    @param The DATATYPE that is being sent
    @param A buffer with the data to send
    @param The size of the data being sent
==============================*/

static void usb_write(int datatype, const void* data, int size)
{
    // If there's data to read first, stop
    if (usb_dataleft != 0)
        return;

    // Call the correct write function
    UsbFuncWrite(datatype, data, size);
}

static void usb_write_start(int datatype, int size)
{
    // If there's data to read first, stop
    if (usb_dataleft != 0)
        return;

    // Call the correct write function
    UsbFuncWriteStart(datatype, size);
}

static void usb_write_part(const void* data, int size)
{
    // If there's data to read first, stop
    if (usb_dataleft != 0)
        return;

    // Call the correct write function
    UsbFuncWritePart(data, size);
}

static void usb_write_end(int datatype, int size)
{
    // If there's data to read first, stop
    if (usb_dataleft != 0)
        return;

    // Call the correct write function
    UsbFuncWriteEnd(datatype, size);
}

/*==============================
    usb_poll
    Returns the header of data being received via USB
    The first byte contains the data type, the next 3 the number of bytes left to read
    @return The data header, or 0
==============================*/

static u32 usb_poll(void)
{
    // If we're out of USB data to read, we don't need the header info anymore
    if (usb_dataleft <= 0)
    {
        usb_dataleft = 0;
        usb_datatype = 0;
        usb_datasize = 0;
        usb_readblock = -1;
    }

    // If there's still data that needs to be read, return the header with the data left
    if (usb_dataleft != 0)
        return USBHEADER_CREATE(usb_datatype, usb_dataleft);

    // Call the correct read function
    return UsbFuncPoll();
}


/*==============================
    usb_read
    Reads bytes from USB into the provided buffer
    @param The buffer to put the read data in
    @param The number of bytes to read
==============================*/

static void usb_read(void* buffer, int nbytes)
{
    int read = 0;
    int left = nbytes;
    int offset = usb_datasize-usb_dataleft;
    int copystart = offset%BUFFER_SIZE;
    int block = BUFFER_SIZE-copystart;
    int blockoffset = (offset/BUFFER_SIZE)*BUFFER_SIZE;

    // If there's no data to read, stop
    if (usb_dataleft == 0)
        return;

    // Read chunks from ROM
    while (left > 0)
    {
        // Ensure we don't read too much data
        if (left > usb_dataleft)
            left = usb_dataleft;
        if (block > left)
            block = left;

        // Call the read function if we're reading a new block
        if (usb_readblock != blockoffset)
        {
            usb_readblock = blockoffset;
            UsbFuncRead();
        }

        // Copy from the USB buffer to the supplied buffer
        D_memcpy((void*)((u32)buffer+read), usb_buffer+copystart, block);

        // Increment/decrement all our counters
        read += block;
        left -= block;
        usb_dataleft -= block;
        blockoffset += BUFFER_SIZE;
        block = BUFFER_SIZE;
        copystart = 0;
    }
}


/*==============================
    usb_skip
    Skips a USB read by the specified amount of bytes
    @param The number of bytes to skip
==============================*/

static void usb_skip(int nbytes)
{
    // Subtract the amount of bytes to skip to the data pointers
    usb_dataleft -= nbytes;
    if (usb_dataleft < 0)
        usb_dataleft = 0;
}


/*==============================
    usb_rewind
    Rewinds a USB read by the specified amount of bytes
    @param The number of bytes to rewind
==============================*/

static void usb_rewind(int nbytes)
{
    // Add the amount of bytes to rewind to the data pointers
    usb_dataleft += nbytes;
    if (usb_dataleft > usb_datasize)
        usb_dataleft = usb_datasize;
}


/*==============================
    usb_purge
    Purges the incoming USB data
==============================*/

static void usb_purge(void)
{
    usb_dataleft = 0;
    usb_datatype = 0;
    usb_datasize = 0;
    usb_readblock = -1;
}


/*==============================
    usb_timedout
    Checks if the USB timed out recently
    @return 1 if the USB timed out, 0 if not
==============================*/

static char usb_timedout()
{
    return usb_didtimeout;
}


/*==============================
    usb_sendheartbeat
    Sends a heartbeat packet to the PC
    This is done once automatically at initialization,
    but can be called manually to ensure that the
    host side tool is aware of the current USB protocol
    version.
==============================*/

static void usb_sendheartbeat(void)
{
    u8 buffer[4];

    // First two bytes describe the USB library protocol version
    buffer[0] = (u8)(((USBPROTOCOL_VERSION)>>8)&0xFF);
    buffer[1] = (u8)(((USBPROTOCOL_VERSION))&0xFF);

    // Next two bytes describe the heartbeat packet version
    buffer[2] = (u8)(((HEARTBEAT_VERSION)>>8)&0xFF);
    buffer[3] = (u8)(((HEARTBEAT_VERSION))&0xFF);

    // Send through USB
    usb_write(DATATYPE_HEARTBEAT, buffer, sizeof(buffer)/sizeof(buffer[0]));
}


/*********************************
        64Drive functions
*********************************/

/*==============================
    usb_64drive_wait
    Wait until the 64Drive CI is ready
    @return FALSE if success or TRUE if failure
==============================*/

static char usb_64drive_wait(void)
{
    u32 timeout;

    // Wait until the cartridge interface is ready
    timeout = usb_timeout_start();
    do
    {
        // Took too long, abort
        if (usb_timeout_check(timeout, D64_COMMAND_TIMEOUT))
        {
            usb_didtimeout = TRUE;
            return TRUE;
        }
    }
    while(usb_io_read(D64_REG_STATUS) & D64_CI_BUSY);

    // Success
    usb_didtimeout = FALSE;
    return FALSE;
}


/*==============================
    usb_64drive_set_writable
    Set the CARTROM write mode on the 64Drive
    @param A boolean with whether to enable or disable
==============================*/

static void usb_64drive_set_writable(u32 enable)
{
    // Wait until CI is not busy
    usb_64drive_wait();

    // Send enable/disable CARTROM writes command
    usb_io_write(D64_REG_COMMAND, enable ? D64_CI_ENABLE_ROMWR : D64_CI_DISABLE_ROMWR);

    // Wait until operation is finished
    usb_64drive_wait();
}


/*==============================
    usb_64drive_cui_write
    Writes data from buffer in the 64drive through USB
    @param Data type
    @param Offset in CARTROM memory space
    @param Transfer size
==============================*/

static void usb_64drive_cui_write(u8 datatype, u32 offset, u32 size)
{
    u32 timeout;

    // Start USB write
    usb_io_write(D64_REG_USBP0R0, offset >> 1);
    usb_io_write(D64_REG_USBP1R1, USBHEADER_CREATE(datatype, ALIGN(size, 4))); // Align size to 32-bits due to bugs in the firmware
    usb_io_write(D64_REG_USBCOMSTAT, D64_CUI_WRITE);

    // Spin until the write buffer is free
    timeout = usb_timeout_start();
    do
    {
        // Took too long, abort
        if (usb_timeout_check(timeout, D64_WRITE_TIMEOUT))
        {
            usb_didtimeout = TRUE;
            return;
        }
    }
    while((usb_io_read(D64_REG_USBCOMSTAT) & D64_CUI_WRITE_MASK) != D64_CUI_WRITE_IDLE);
}


/*==============================
    usb_64drive_cui_poll
    Checks if there is data waiting to be read from USB FIFO
    @return TRUE if data is waiting, FALSE if otherwise
==============================*/

static char usb_64drive_cui_poll(void)
{
    // Check if we have data waiting in buffer
    if ((usb_io_read(D64_REG_USBCOMSTAT) & D64_CUI_ARM_MASK) == D64_CUI_ARM_UNARMED_DATA)
        return TRUE;
    return FALSE;
}


/*==============================
    usb_64drive_cui_read
    Reads data from USB FIFO to buffer in the 64drive
    @param  Offset in CARTROM memory space
    @return USB header (datatype + size)
==============================*/

static u32 usb_64drive_cui_read(u32 offset)
{
    u32 header;
    u32 left;
    u32 datatype;
    u32 size;

    // Arm USB FIFO with 8 byte sized transfer
    usb_io_write(D64_REG_USBP0R0, offset >> 1);
    usb_io_write(D64_REG_USBP1R1, 8);
    usb_io_write(D64_REG_USBCOMSTAT, D64_CUI_ARM);

    // Wait until data is received
    while ((usb_io_read(D64_REG_USBCOMSTAT) & D64_CUI_ARM_MASK) != D64_CUI_ARM_UNARMED_DATA)
        ;

    // Get datatype and bytes remaining
    header = usb_io_read(D64_REG_USBP0R0);
    left = usb_io_read(D64_REG_USBP1R1) & 0x00FFFFFF;
    datatype = header & 0xFF000000;
    size = header & 0x00FFFFFF;

    // Determine if we need to read more data
    if (left > 0)
    {
        // Arm USB FIFO with known transfer size
        usb_io_write(D64_REG_USBP0R0, (offset + 8) >> 1);
        usb_io_write(D64_REG_USBP1R1, left);
        usb_io_write(D64_REG_USBCOMSTAT, D64_CUI_ARM);

        // Wait until data is received
        while ((usb_io_read(D64_REG_USBCOMSTAT) & D64_CUI_ARM_MASK) != D64_CUI_ARM_UNARMED_DATA)
            ;

        // Calculate total transfer length
        size += left;
    }

    // Disarm USB FIFO
    usb_io_write(D64_REG_USBCOMSTAT, D64_CUI_DISARM);

    // Wait until USB FIFO is disarmed
    while ((usb_io_read(D64_REG_USBCOMSTAT) & D64_CUI_ARM_MASK) != D64_CUI_ARM_IDLE)
        ;

    // Due to a 64drive bug, we need to ignore the last 512 bytes of the transfer if it's larger than 512 bytes
    if (size > 512)
        size -= 512;

    // Return data header (datatype and size)
    return (datatype | size);
}


/*==============================
    usb_64drive_write
    Sends data through USB from the 64Drive
    Will not write if there is data to read from USB
    @param The DATATYPE that is being sent
    @param A buffer with the data to send
    @param The size of the data being sent
==============================*/

static void usb_64drive_write(int datatype, const void* data, int size)
{
    s32 left = size;
    u32 pi_address = D64_BASE + DEBUG_ADDRESS;

    // Return if previous transfer timed out
    if ((usb_io_read(D64_REG_USBCOMSTAT) & D64_CUI_WRITE_MASK) == D64_CUI_WRITE_BUSY)
    {
        usb_didtimeout = TRUE;
        return;
    }

    // Set the cartridge to write mode
    usb_64drive_set_writable(TRUE);

    // Write data to SDRAM until we've finished
    while (left > 0)
    {
        // Calculate transfer size
        u32 block = MIN(left, BUFFER_SIZE);

        // Copy data to PI DMA aligned buffer
        D_memcpy(usb_buffer, data, block);

        // Pad the buffer with zeroes if it wasn't 4 byte aligned
        while (block%4)
            usb_buffer[block++] = 0;

        // Copy block of data from RDRAM to SDRAM
        usb_dma_write(usb_buffer, pi_address, ALIGN(block, 2));

        // Update pointers and variables
        data = (void*)((u32)data + block);
        left -= block;
        pi_address += block;
    }

    // Disable write mode
    usb_64drive_set_writable(FALSE);

    // Send the data through USB
    usb_64drive_cui_write(datatype, DEBUG_ADDRESS, size);
    usb_didtimeout = FALSE;
}

static void usb_64drive_write_start(int datatype, int size)
{
    write_address = D64_BASE + DEBUG_ADDRESS;

    // Return if previous transfer timed out
    if ((usb_io_read(D64_REG_USBCOMSTAT) & D64_CUI_WRITE_MASK) == D64_CUI_WRITE_BUSY)
    {
        usb_didtimeout = TRUE;
        return;
    }

    // Set the cartridge to write mode
    usb_64drive_set_writable(TRUE);

}

static void usb_64drive_write_part(const void* data, int size)
{
    s32 left = size;

    if (usb_didtimeout)
        return;

    // Write data to SDRAM until we've finished
    while (left > 0)
    {
        // Calculate transfer size
        u32 block = MIN(left, BUFFER_SIZE);

        // Copy data to PI DMA aligned buffer
        D_memcpy(usb_buffer, data, block);

        // Pad the buffer with zeroes if it wasn't 4 byte aligned
        while (block%4)
            usb_buffer[block++] = 0;

        // Copy block of data from RDRAM to SDRAM
        usb_dma_write(usb_buffer, write_address, block);

        // Update pointers and variables
        data = (void*)((u32)data + block);
        left -= block;
        write_address += block;
    }
}

static void usb_64drive_write_end(int datatype, int size)
{
    if (usb_didtimeout)
        return;

    // Disable write mode
    usb_64drive_set_writable(FALSE);

    // Send the data through USB
    usb_64drive_cui_write(datatype, DEBUG_ADDRESS, size);
    usb_didtimeout = FALSE;
}


/*==============================
    usb_64drive_poll
    Returns the header of data being received via USB on the 64Drive
    The first byte contains the data type, the next 3 the number of bytes left to read
    @return The data header, or 0
==============================*/

static u32 usb_64drive_poll(void)
{
    u32 header;

    // If there's data to service
    if (usb_64drive_cui_poll())
    {
        // Read data to the buffer in 64drive SDRAM memory
        header = usb_64drive_cui_read(DEBUG_ADDRESS);

        // Get the data header
        usb_datatype = USBHEADER_GETTYPE(header);
        usb_dataleft = USBHEADER_GETSIZE(header);
        usb_datasize = usb_dataleft;
        usb_readblock = -1;

        // Return the data header
        return USBHEADER_CREATE(usb_datatype, usb_datasize);
    }

    // Return 0 if there's no data
    return 0;
}


/*==============================
    usb_64drive_read
    Reads bytes from the 64Drive ROM into the global buffer with the block offset
==============================*/

static void usb_64drive_read(void)
{
    // Set up DMA transfer between RDRAM and the PI
    usb_dma_read(usb_buffer, D64_BASE + DEBUG_ADDRESS + usb_readblock, BUFFER_SIZE);
}


/*********************************
       EverDrive functions
*********************************/

/*==============================
    usb_everdrive_usbbusy
    Spins until the USB is no longer busy
    @return FALSE on success, TRUE on failure
==============================*/

static char usb_everdrive_usbbusy(void)
{
    u32 val;
    u32 timeout = usb_timeout_start();
    do
    {
        val = usb_io_read(ED_REG_USBCFG);
        if (usb_timeout_check(timeout, ED_TIMEOUT))
        {
            usb_io_write(ED_REG_USBCFG, ED_USBMODE_RDNOP);
            usb_didtimeout = TRUE;
            return TRUE;
        }
    }
    while ((val & ED_USBSTAT_ACT) != 0);
    return FALSE;
}


/*==============================
    usb_everdrive_canread
    Checks if the EverDrive's USB can read
    @return TRUE if it can read, FALSE if not
==============================*/

static char usb_everdrive_canread(void)
{
    u32 val;
    u32 status = ED_USBSTAT_POWER;

    // Read the USB register and check its status
    val = usb_io_read(ED_REG_USBCFG);
    status = val & (ED_USBSTAT_POWER | ED_USBSTAT_RXF);
    if (status == ED_USBSTAT_POWER)
        return TRUE;
    return FALSE;
}


/*==============================
    usb_everdrive_readusb
    Reads from the EverDrive USB buffer
    @param The buffer to put the read data in
    @param The number of bytes to read
==============================*/

static void usb_everdrive_readusb(void* buffer, int size)
{
    u16 block, addr;

    while (size)
    {
        // Get the block size
        block = BUFFER_SIZE;
        if (block > size)
            block = size;
        addr = BUFFER_SIZE - block;

        // Request to read from the USB
        usb_io_write(ED_REG_USBCFG, ED_USBMODE_RD | addr);

        // Wait for the FPGA to transfer the data to its internal buffer, or stop on timeout
        if (usb_everdrive_usbbusy())
            return;

        // Read from the internal buffer and store it in our buffer
        usb_dma_read(buffer, ED_REG_USBDAT + addr, block);
        buffer = (char*)buffer + block;
        size -= block;
    }
}

/*==============================
    usb_everdrive_write
    Sends data through USB from the EverDrive
    Will not write if there is data to read from USB
    @param The DATATYPE that is being sent
    @param A buffer with the data to send
    @param The size of the data being sent
==============================*/

static void usb_everdrive_write(int datatype, const void* data, int size)
{
    char wrotecmp = 0;
    char cmp[] = {'C', 'M', 'P', 'H'};
    int read = 0;
    int left = size;
    int offset = 8;
    u32 header = (size & 0x00FFFFFF) | (datatype << 24);

    // Put in the DMA header along with length and type information in the global buffer
    usb_buffer[0] = 'D';
    usb_buffer[1] = 'M';
    usb_buffer[2] = 'A';
    usb_buffer[3] = '@';
    usb_buffer[4] = (header >> 24) & 0xFF;
    usb_buffer[5] = (header >> 16) & 0xFF;
    usb_buffer[6] = (header >> 8)  & 0xFF;
    usb_buffer[7] = header & 0xFF;

    // Write data to USB until we've finished
    while (left > 0)
    {
        int block = left;
        int blocksend, baddr;
        if (block+offset > BUFFER_SIZE)
            block = BUFFER_SIZE-offset;

        // Copy the data to the next available spots in the global buffer
        D_memcpy(usb_buffer+offset, (void*)((char*)data+read), block);

        // Restart the loop to write the CMP signal if we've finished
        if (!wrotecmp && read+block >= size)
        {
            left = 4;
            offset = block+offset;
            data = cmp;
            wrotecmp = 1;
            read = 0;
            continue;
        }

        // Ensure the data is 2 byte aligned and the block address is correct
        blocksend = ALIGN((block+offset), 2);
        baddr = BUFFER_SIZE - blocksend;

        // Set USB to write mode and send data through USB
        usb_io_write(ED_REG_USBCFG, ED_USBMODE_WRNOP);
        usb_dma_write(usb_buffer, ED_REG_USBDAT + baddr, blocksend);

        // Set USB to write mode with the new address and wait for USB to end (or stop if it times out)
        usb_io_write(ED_REG_USBCFG, ED_USBMODE_WR | baddr);
        if (usb_everdrive_usbbusy())
        {
            usb_didtimeout = TRUE;
            return;
        }

        // Keep track of what we've read so far
        left -= block;
        read += block;
        offset = 0;
    }
    usb_didtimeout = FALSE;
}

static void usb_everdrive_write_start(int datatype, int size)
{
    u32 header = (size & 0x00FFFFFF) | (datatype << 24);
    u8 buf[8];

    // Put in the DMA header along with length and type information in the global buffer
    buf[0] = 'D';
    buf[1] = 'M';
    buf[2] = 'A';
    buf[3] = '@';
    buf[4] = (header >> 24) & 0xFF;
    buf[5] = (header >> 16) & 0xFF;
    buf[6] = (header >> 8)  & 0xFF;
    buf[7] = header & 0xFF;

    usb_everdrive_write_part(buf, sizeof buf);
}

static void usb_everdrive_write_part(const void* data, int size)
{
    int read = 0;
    int left = size;

    if (usb_didtimeout)
        return;

    // Write data to USB until we've finished
    while (left > 0)
    {
        int block = left;
        int blocksend, baddr;
        if (block > BUFFER_SIZE)
            block = BUFFER_SIZE;

        // Copy the data to the next available spots in the global buffer
        D_memcpy(usb_buffer, (void*)((char*)data+read), block);

        // Ensure the data is 2 byte aligned and the block address is correct
        blocksend = ALIGN(block, 2);
        baddr = BUFFER_SIZE - blocksend;

        // Set USB to write mode and send data through USB
        usb_io_write(ED_REG_USBCFG, ED_USBMODE_WRNOP);
        usb_dma_write(usb_buffer, ED_REG_USBDAT + baddr, blocksend);

        // Set USB to write mode with the new address and wait for USB to end (or stop if it times out)
        usb_io_write(ED_REG_USBCFG, ED_USBMODE_WR | baddr);
        if (usb_everdrive_usbbusy())
        {
            usb_didtimeout = TRUE;
            return;
        }

        // Keep track of what we've read so far
        left -= block;
        read += block;
    }
    usb_didtimeout = FALSE;
}

static void usb_everdrive_write_end(int datatype, int size)
{
    static const char cmp[] = {'C', 'M', 'P', 'H'};

    usb_everdrive_write_part(cmp, sizeof cmp);
}

/*==============================
    usb_everdrive_poll
    Returns the header of data being received via USB on the EverDrive
    The first byte contains the data type, the next 3 the number of bytes left to read
    @return The data header, or 0
==============================*/

static u32 usb_everdrive_poll(void)
{
    int len;
    int offset = 0;
    unsigned char  buffaligned[32];
    unsigned char* buff = (unsigned char*)OS_DCACHE_ROUNDUP_ADDR(buffaligned);

    // Wait for the USB to be ready
    if (usb_everdrive_usbbusy())
        return 0;

    // Check if the USB is ready to be read
    if (!usb_everdrive_canread())
        return 0;

    // Read the first 8 bytes that are being received and check if they're valid
    usb_everdrive_readusb(buff, 8);
    if (buff[0] != 'D' || buff[1] != 'M' || buff[2] != 'A' || buff[3] != '@')
        return 0;

    // Store information about the incoming data
    usb_datatype = buff[4];
    usb_datasize = (buff[5] << 16) | (buff[6] << 8) | (buff[7] << 0);
    usb_dataleft = usb_datasize;
    usb_readblock = -1;

    // Get the aligned data size. Must be 2 byte aligned
    len = ALIGN(usb_datasize, 2);

    // While there's data to service
    while (len > 0)
    {
        u32 bytes_do = BUFFER_SIZE;
        if (len < BUFFER_SIZE)
            bytes_do = len;

        // Read a chunk from USB and store it into our temp buffer
        usb_everdrive_readusb(usb_buffer, bytes_do);

        // Copy received block to ROM
        usb_dma_write(usb_buffer, ED_BASE + DEBUG_ADDRESS + offset, bytes_do);
        offset += bytes_do;
        len -= bytes_do;
    }

    // Read the CMP Signal
    if (usb_everdrive_usbbusy())
        return 0;
    usb_everdrive_readusb(buff, 4);
    if (buff[0] != 'C' || buff[1] != 'M' || buff[2] != 'P' || buff[3] != 'H')
    {
        // Something went wrong with the data
        usb_datatype = 0;
        usb_datasize = 0;
        usb_dataleft = 0;
        usb_readblock = -1;
        return 0;
    }

    // Return the data header
    return USBHEADER_CREATE(usb_datatype, usb_datasize);
}


/*==============================
    usb_everdrive_read
    Reads bytes from the EverDrive ROM into the global buffer with the block offset
==============================*/

static void usb_everdrive_read(void)
{
    // Set up DMA transfer between RDRAM and the PI
    usb_dma_read(usb_buffer, ED_BASE + DEBUG_ADDRESS + usb_readblock, BUFFER_SIZE);
}


/*********************************
       SC64 functions
*********************************/

/*==============================
    usb_sc64_execute_cmd
    Executes specified command in SC64 controller
    @param  Command ID to execute
    @param  2 element array of 32 bit arguments to pass with command, use NULL when argument values are not needed
    @param  2 element array of 32 bit values to read command result, use NULL when result values are not needed
    @return TRUE if there was error during command execution, otherwise FALSE
==============================*/

static char usb_sc64_execute_cmd(u8 cmd, u32 *args, u32 *result)
{
    u32 sr;

    // Write arguments if provided
    if (args != NULL)
    {
        usb_io_write(SC64_REG_DATA_0, args[0]);
        usb_io_write(SC64_REG_DATA_1, args[1]);
    }

    // Start execution
    usb_io_write(SC64_REG_SR_CMD, cmd);

    // Wait for completion
    do
    {
        sr = usb_io_read(SC64_REG_SR_CMD);
    }
    while (sr & SC64_SR_CMD_BUSY);

    // Read result if provided
    if (result != NULL)
    {
        result[0] = usb_io_read(SC64_REG_DATA_0);
        result[1] = usb_io_read(SC64_REG_DATA_1);
    }

    // Return error status
    if (sr & SC64_SR_CMD_ERROR)
        return TRUE;
    return FALSE;
}


/*==============================
    usb_sc64_set_writable
    Enable ROM (SDRAM) writes in SC64
    @param  A boolean with whether to enable or disable
    @return Previous value of setting
==============================*/

static u32 usb_sc64_set_writable(u32 enable)
{
    u32 args[2];
    u32 result[2];

    args[0] = SC64_CFG_ROM_WRITE_ENABLE;
    args[1] = enable;
    if (usb_sc64_execute_cmd(SC64_CMD_CONFIG_SET, args, result))
        return 0;

    return result[1];
}

/*==============================
    usb_sc64_write
    Sends data through USB from the SC64
    @param The DATATYPE that is being sent
    @param A buffer with the data to send
    @param The size of the data being sent
==============================*/

static void usb_sc64_write(int datatype, const void* data, int size)
{
    u32 left = size;
    u32 pi_address = SC64_BASE + DEBUG_ADDRESS;
    u32 writable_restore;
    u32 timeout;
    u32 args[2];
    u32 result[2];

    // Return if previous transfer timed out
    usb_sc64_execute_cmd(SC64_CMD_USB_WRITE_STATUS, NULL, result);
    if (result[0] & SC64_USB_WRITE_STATUS_BUSY)
    {
        usb_didtimeout = TRUE;
        return;
    }

    // Enable SDRAM writes and get previous setting
    writable_restore = usb_sc64_set_writable(TRUE);

    while (left > 0)
    {
        // Calculate transfer size
        u32 block = MIN(left, BUFFER_SIZE);

        // Copy data to PI DMA aligned buffer
        D_memcpy(usb_buffer, data, block);

        // Copy block of data from RDRAM to SDRAM
        usb_dma_write(usb_buffer, pi_address, ALIGN(block, 2));

        // Update pointers and variables
        data = (void*)((u32)data + block);
        left -= block;
        pi_address += block;
    }

    // Restore previous SDRAM writable setting
    usb_sc64_set_writable(writable_restore);

    // Start sending data from buffer in SDRAM
    args[0] = SC64_BASE + DEBUG_ADDRESS;
    args[1] = USBHEADER_CREATE(datatype, size);
    if (usb_sc64_execute_cmd(SC64_CMD_USB_WRITE, args, NULL))
    {
        usb_didtimeout = TRUE;
        return; // Return if USB write was unsuccessful
    }

    // Wait for transfer to end
    timeout = usb_timeout_start();
    do
    {
        // Took too long, abort
        if (usb_timeout_check(timeout, SC64_WRITE_TIMEOUT))
        {
            usb_didtimeout = TRUE;
            return;
        }
        usb_sc64_execute_cmd(SC64_CMD_USB_WRITE_STATUS, NULL, result);
    }
    while (result[0] & SC64_USB_WRITE_STATUS_BUSY);
    usb_didtimeout = FALSE;
}

static void usb_sc64_write_start(int datatype, int size)
{
    u32 result[2];

    // Return if previous transfer timed out
    usb_sc64_execute_cmd(SC64_CMD_USB_WRITE_STATUS, NULL, result);
    if (result[0] & SC64_USB_WRITE_STATUS_BUSY)
    {
        usb_didtimeout = TRUE;
        return;
    }
}

static void usb_sc64_write_part(const void* data, int size)
{
    u32 left = size;
    u32 pi_address = SC64_BASE + DEBUG_ADDRESS;
    u32 writable_restore;

    if (usb_didtimeout)
        return;

    // Enable SDRAM writes and get previous setting
    writable_restore = usb_sc64_set_writable(TRUE);

    while (left > 0)
    {
        // Calculate transfer size
        u32 block = MIN(left, BUFFER_SIZE);

        // Copy data to PI DMA aligned buffer
        D_memcpy(usb_buffer, data, block);

        // Copy block of data from RDRAM to SDRAM
        usb_dma_write(usb_buffer, pi_address, ALIGN(block, 2));

        // Update pointers and variables
        data = (void*)((u32)data + block);
        left -= block;
        pi_address += block;
    }

    // Restore previous SDRAM writable setting
    usb_sc64_set_writable(writable_restore);
}

static void usb_sc64_write_end(int datatype, int size)
{
    u32 timeout;
    u32 args[2];
    u32 result[2];

    if (usb_didtimeout)
        return;

    // Start sending data from buffer in SDRAM
    args[0] = SC64_BASE + DEBUG_ADDRESS;
    args[1] = USBHEADER_CREATE(datatype, size);
    if (usb_sc64_execute_cmd(SC64_CMD_USB_WRITE, args, NULL))
    {
        usb_didtimeout = TRUE;
        return; // Return if USB write was unsuccessful
    }

    // Wait for transfer to end
    timeout = usb_timeout_start();
    do
    {
        // Took too long, abort
        if (usb_timeout_check(timeout, SC64_WRITE_TIMEOUT))
        {
            usb_didtimeout = TRUE;
            return;
        }
        usb_sc64_execute_cmd(SC64_CMD_USB_WRITE_STATUS, NULL, result);
    }
    while (result[0] & SC64_USB_WRITE_STATUS_BUSY);
    usb_didtimeout = FALSE;
}


/*==============================
    usb_sc64_poll
    Returns the header of data being received via USB on the SC64
    The first byte contains the data type, the next 3 the number of bytes left to read
    @return The data header, or 0
==============================*/

static u32 usb_sc64_poll(void)
{
    u8 datatype;
    u32 size;
    u32 args[2];
    u32 result[2];

    // Get read status and extract packet info
    usb_sc64_execute_cmd(SC64_CMD_USB_READ_STATUS, NULL, result);
    datatype = result[0] & 0xFF;
    size = result[1] & 0xFFFFFF;

    // Return 0 if there's no data
    if (size == 0)
        return 0;

    // Fill USB read data variables
    usb_datatype = datatype;
    usb_dataleft = size;
    usb_datasize = usb_dataleft;
    usb_readblock = -1;

    // Start receiving data to buffer in SDRAM
    args[0] = SC64_BASE + DEBUG_ADDRESS;
    args[1] = size;
    if (usb_sc64_execute_cmd(SC64_CMD_USB_READ, args, NULL))
        return 0; // Return 0 if USB read was unsuccessful

    // Wait for completion
    do
    {
        usb_sc64_execute_cmd(SC64_CMD_USB_READ_STATUS, NULL, result);
    }
    while (result[0] & SC64_USB_READ_STATUS_BUSY);

    // Return USB header
    return USBHEADER_CREATE(datatype, size);
}


/*==============================
    usb_sc64_read
    Reads bytes from the SC64 SDRAM into the global buffer with the block offset
==============================*/

static void usb_sc64_read(void)
{
    // Set up DMA transfer between RDRAM and the PI
    usb_dma_read(usb_buffer, SC64_BASE + DEBUG_ADDRESS + usb_readblock, BUFFER_SIZE);
}

void I_USBQuickSave(void);

void I_PollUSBCommands(void)
{
    usbMesg msg;

    /* Force USB thread to unblock, discarding any unparsed commands from the
     * previous frame */
    I_CmdSkipAllTokens();

    msg.msgtype = MSG_READ;
    osSendMesg(&usbMessageQ, (OSMesg)&msg, OS_MESG_BLOCK);
}

int I_CmdNextTokenSize(void)
{
    // If we're out of commands to read, return 0
    if (*&CommandParser.current == *&CommandParser.totaltokens)
        return 0;

    // Otherwise, return the amount of data to read
    return *&CommandParser.incoming_size[*&CommandParser.current];
}

void I_CmdGetNextToken(void* buffer)
{
    u8 curr = *&CommandParser.current;

    // Skip this command if no buffer exists
    if (buffer == NULL)
    {
        *&CommandParser.current = curr + 1;
        return;
    }

    int total = *&CommandParser.totaltokens;

    // If we're out of commands to read, do nothing
    if (curr == total)
        return;

    // Read from the correct offset
    usb_skip(CommandParser.incoming_start[curr]);
    usb_read(buffer, CommandParser.incoming_size[curr]);
    usb_rewind(CommandParser.incoming_size[curr]+CommandParser.incoming_start[curr]);
    *&CommandParser.current = curr + 1;

    if (curr == total)
        I_CmdSkipAllTokens();
}

void I_CmdSkipAllTokens(void)
{
    *&CommandParser.current = *&CommandParser.totaltokens;
    usb_purge();
    osSetThreadPri(&usbThread, USB_THREAD_PRI);
}

static char CommandBuffer[256];

static void I_CmdSetup(void)
{
    int i;
    int datasize = USBHEADER_GETSIZE(usb_poll());
    int dataleft = datasize;
    int filesize = 0;
    char filestep = 0;

    // Initialize the starting offsets at -1
    for (int i = 0; i < COMMAND_TOKENS; i++)
        CommandParser.incoming_start[i] = -1;

    // Read data from USB in blocks
    while (dataleft > 0)
    {
        int readsize = sizeof CommandBuffer;
        if (readsize > dataleft)
            readsize = dataleft;

        // Read a block from USB
        bzero(CommandBuffer,  sizeof CommandBuffer);
        usb_read(CommandBuffer, readsize);

        // Parse the block
        for (i=0; i<readsize && dataleft > 0; i++)
        {
            // If we're not reading a file
            int offset = datasize-dataleft;
            u8 tok = *&CommandParser.totaltokens;

            // Decide what to do based on the current character
            switch (CommandBuffer[i])
            {
                case ' ':
                case '\0':
                    if (filestep < 2)
                    {
                        if (CommandParser.incoming_start[tok] != -1)
                        {
                            CommandParser.incoming_size[tok] = offset-CommandParser.incoming_start[tok];
                            (*&CommandParser.totaltokens)++;
                        }

                        if (CommandBuffer[i] == '\0')
                            dataleft = 0;
                        break;
                    }
                case '@':
                    filestep++;
                    if (filestep < 3)
                        break;
                default:
                    // Decide what to do based on the file handle
                    if (filestep == 0 && CommandParser.incoming_start[tok] == -1)
                    {
                        // Store the data offsets and sizes in the global command buffers
                        CommandParser.incoming_start[tok] = offset;
                    }
                    else if (filestep == 1)
                    {
                        // Get the filesize
                        filesize = filesize*10 + CommandBuffer[i]-'0';
                    }
                    else if (filestep > 1)
                    {
                        // Store the file offsets and sizes in the global command buffers
                        CommandParser.incoming_start[tok] = offset;
                        CommandParser.incoming_size[tok] = filesize;
                        (*&CommandParser.totaltokens)++;

                        // Skip a bunch of bytes
                        if ((readsize-i)-filesize < 0)
                            usb_skip(filesize-(readsize-i));
                        dataleft -= filesize;
                        i += filesize;
                        filesize = 0;
                        filestep = 0;
                    }
                    break;
            }
            dataleft--;
        }
    }

    // Rewind the USB fully
    usb_rewind(datasize);
}

static boolean I_RunUSBCommand(void);

static void I_USBTicker(void)
{
    char errortype = USBERROR_NONE;
    usbMesg* threadMsg = NULL;
    const char *error = NULL;

    // Wait for a USB message to arrive
    osRecvMesg(&usbMessageQ, (OSMesg *)&threadMsg, OS_MESG_BLOCK);

    // put it back if another thread is still parsing
    if (*&CommandParser.current != *&CommandParser.totaltokens)
    {
        osSetThreadPri(&usbThread, OS_PRIORITY_IDLE);
        osJamMesg(&usbMessageQ, (OSMesg *)&threadMsg, OS_MESG_NOBLOCK);
        return;
    }

    // Ensure there's no data in the USB (which handles MSG_READ)
    while (usb_poll() != 0)
    {
        int header = usb_poll();

#ifdef USB_GDB
        if (USBHEADER_GETTYPE(header) == DATATYPE_RDBPACKET)
        {
            extern void I_TakeRDBPacket(void);
            I_TakeRDBPacket();
        }
        else
#endif
        if (USBHEADER_GETTYPE(header) != DATATYPE_TEXT)
        {
            errortype = USBERROR_NOTTEXT;
            usb_purge();
            break;
        }

        // Initialize the command trackers
        *&CommandParser.totaltokens = 0;
        *&CommandParser.current = 0;
        *&CommandParser.error = NULL;

        // Break the USB command into parts
        I_CmdSetup();

        // Ensure we don't read past our buffer
        if (I_CmdNextTokenSize() > sizeof CommandBuffer)
        {
            errortype = USBERROR_TOOMUCH;
            usb_purge();
            break;
        }

        // Read from the USB to retrieve the command name
        I_CmdGetNextToken(CommandBuffer);

        if (!I_RunUSBCommand())
            errortype = USBERROR_UNKNOWN;
        else if ((error = *&CommandParser.error) != NULL)
            errortype = USBERROR_CUSTOM;

        usb_purge();
    }

    // Spit out an error if there was one during the command parsing
    if (errortype != USBERROR_NONE)
    {
        switch (errortype)
        {
            case USBERROR_NOTTEXT:
                usb_write(DATATYPE_TEXT, "Error: USB data was not text\n", 29+1);
                break;
            case USBERROR_UNKNOWN:
                usb_write(DATATYPE_TEXT, "Error: Unknown command\n", 23+1);
                break;
            case USBERROR_TOOMUCH:
                usb_write(DATATYPE_TEXT, "Error: Command too large\n", 25+1);
                break;
            case USBERROR_CUSTOM:
                usb_write(DATATYPE_TEXT, error, D_strlen(error)+1);
                usb_write(DATATYPE_TEXT, "\n", 1+1);
                break;
        }
        errortype = USBERROR_NONE;
    }

    if (threadMsg)
    {
        // Handle the other USB messages
        switch (threadMsg->msgtype)
        {
            case MSG_WRITE:
                if (usb_timedout())
                    usb_sendheartbeat();
                usb_write(threadMsg->datatype, threadMsg->buff, threadMsg->size);
                break;
            case MSG_WRITE_START:
                if (usb_timedout())
                    usb_sendheartbeat();
                usb_write_start(threadMsg->datatype, threadMsg->size);
                break;
            case MSG_WRITE_PART:
                if (usb_timedout())
                    usb_sendheartbeat();
                usb_write_part(threadMsg->buff, threadMsg->size);
                break;
            case MSG_WRITE_END:
                if (usb_timedout())
                    usb_sendheartbeat();
                usb_write_end(threadMsg->datatype, threadMsg->size);
                break;
        }
    }
}

static NO_RETURN void I_USBThread(void *arg)
{
    SET_GP();

    // Send a heartbeat
    usb_sendheartbeat();

    *&CommandParser.totaltokens = 0;
    *&CommandParser.current = 0;

    // Thread loop
    while (1)
    {
        I_USBTicker();
        I_CheckStack(usbThreadStack, "USB");
    }
}

void I_USBPrint(const char* message, u32 len)
{
    usbMesg msg;
    // Send the printf to the usb thread
    msg.msgtype = MSG_WRITE;
    msg.datatype = DATATYPE_TEXT;
    msg.buff = (void*) message;
    msg.size = len + 1;
    osSendMesg(&usbMessageQ, (OSMesg)&msg, OS_MESG_BLOCK);
}

#ifdef USB_GDB
void I_USBSendRDB(const u8* packet, u32 len)
{
    usbMesg msg;
    // Send the packet to the usb thread
    msg.msgtype = MSG_WRITE;
    msg.datatype = DATATYPE_RDBPACKET;
    msg.buff = (void*) packet;
    msg.size = len;
    osSendMesg(&usbMessageQ, (OSMesg)&msg, OS_MESG_BLOCK);
}

void I_USBWriteRDB(const u8* packet, u32 len)
{
    usb_write(DATATYPE_RDBPACKET, packet, len);
}
#endif /* USB_GDB */

void I_USBSendFile(void* file, int size)
{
    usbMesg msg;

    if (FlashCart == CART_NONE)
        return;

    // Send the binary file to the usb thread
    msg.msgtype = MSG_WRITE;
    msg.datatype = DATATYPE_RAWBINARY;
    msg.buff = file;
    msg.size = size;

    osSendMesg(&usbMessageQ, (OSMesg)&msg, OS_MESG_BLOCK);
}

void I_USBSendStart(int size)
{
    usbMesg msg;

    if (FlashCart == CART_NONE)
        return;

    // Send the binary file to the usb thread
    msg.msgtype = MSG_WRITE_START;
    msg.datatype = DATATYPE_RAWBINARY;
    msg.buff = NULL;
    msg.size = size;

    osSendMesg(&usbMessageQ, (OSMesg)&msg, OS_MESG_BLOCK);
}

void I_USBSendPart(const void *buf, int size)
{
    usbMesg msg;

    assert(!(size & 1));

    if (FlashCart == CART_NONE)
        return;

    // Send the binary file to the usb thread
    msg.msgtype = MSG_WRITE_PART;
    msg.datatype = DATATYPE_RAWBINARY;
    msg.buff = (void *) buf;
    msg.size = size;

    osSendMesg(&usbMessageQ, (OSMesg)&msg, OS_MESG_BLOCK);
}

void I_USBSendEnd(int size)
{
    usbMesg msg;

    if (FlashCart == CART_NONE)
        return;

    // Send the binary file to the usb thread
    msg.msgtype = MSG_WRITE_END;
    msg.datatype = DATATYPE_RAWBINARY;
    msg.buff = NULL;
    msg.size = size;

    osSendMesg(&usbMessageQ, (OSMesg)&msg, OS_MESG_BLOCK);
}

static void I_USBWriteImage(void *frame, u32 w, u32 h, u8 depth)
{
    int data[4];

    // Create the data header to send
    data[0] = DATATYPE_SCREENSHOT;
    data[1] = depth;
    data[2] = w;
    data[3] = h;

    usb_write(DATATYPE_HEADER, data, sizeof data);
    usb_write(DATATYPE_SCREENSHOT, frame, depth * w * h);
}

static void I_USBWriteScreenshot(void)
{
    int mask = __osDisableInt();
    void *frame;
    u32 depth;

    // wait for rdp to finish writing
    while (__osSpDeviceBusy() || __osDpDeviceBusy());

    frame = CFB(vid_side);
    depth = VideoSettings.BitDepth == BITDEPTH_32 ? 4 : 2;
    osInvalDCache(frame, depth * XResolution * YResolution);
    I_USBWriteImage(CFB(vid_side), XResolution, YResolution, depth);

    __osRestoreInt(mask);
}

const char USB_HELP[] =
    "Available USB commands\n----------------------\n"
    "q\tQuicksave to USB\n"
    "Q\tQuickload from USB\n"
    "m\tLoads a custom map WAD\n"
    "s\tTake screenshot\n"
    "h\tDump heap\n"
    "?\tShow this message\n"
    "\n";

void I_USBPrintHelp(void)
{
    D_printstatic(USB_HELP);
}

static volatile u32 QueuedUSBOperations = 0;
u32 PendingUSBOperations = 0;

/* Execute a command from the USB serial port. Every function dispatched here
 * must be safe to run on either the USB thread or Debug thread. */
static boolean I_RunUSBCommand(void)
{
    char cmd;

    if (CommandBuffer[0] == '\0' || CommandBuffer[1] != '\0')
        cmd = '\0'; // fallthrough to error handler below
    else
        cmd = CommandBuffer[0];

    switch (cmd)
    {
    case '?':
        I_CmdSkipAllTokens();
        usb_write(DATATYPE_TEXT, USB_HELP, sizeof(USB_HELP));
        break;
    case 'q':
        I_CmdSkipAllTokens();
        *&QueuedUSBOperations |= USB_OP_QUICKSAVE;
        break;
    case 'Q':
        *&QueuedUSBOperations |= USB_OP_QUICKLOAD;
        break;
    case 'm':
        *&QueuedUSBOperations |= USB_OP_LOADMAP;
        break;
    case 'h':
        I_CmdSkipAllTokens();
        *&QueuedUSBOperations |= USB_OP_DUMPHEAP;
        break;
    case 's':
        I_CmdSkipAllTokens();
        I_USBWriteScreenshot();
        break;
    default:
        {
            I_CmdSkipAllTokens();

            const char ERROR[] = "Unknown command\n";
            usb_write(DATATYPE_TEXT, ERROR, sizeof(ERROR));
            usb_write(DATATYPE_TEXT, USB_HELP, sizeof(USB_HELP));
        }
        return false;
    }

    return true;
}

// called from the main thread
int I_DispatchUSBCommands(void)
{
    // Ensure flash cart is present
    if (!FlashCart)
        return ga_nothing;

    I_PollUSBCommands();

    PendingUSBOperations = *&QueuedUSBOperations;
    *&QueuedUSBOperations = 0;

    if (!PendingUSBOperations)
        return ga_nothing;

    if (PendingUSBOperations & USB_OP_DUMPHEAP)
    {
        Z_DumpHeap(mainzone);
        PendingUSBOperations &= ~USB_OP_DUMPHEAP;
    }

    if (PendingUSBOperations & USB_OP_QUICKSAVE)
        I_USBQuickSave();

    if (PendingUSBOperations & USB_OP_QUICKLOAD)
        return ga_loadquicksave;

    if (PendingUSBOperations & USB_OP_LOADMAP)
        return ga_warped;

    return ga_nothing;
}

#endif /* USB */
