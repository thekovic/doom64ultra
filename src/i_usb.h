#pragma once

#include "config.h"

/*********************************
         DataType macros
*********************************/

// Cart definitions
#define CART_NONE      0
#define CART_64DRIVE   1
#define CART_EVERDRIVE 2
#define CART_SC64      3

// Data types defintions
#define DATATYPE_TEXT       0x01
#define DATATYPE_RAWBINARY  0x02
#define DATATYPE_HEADER     0x03
#define DATATYPE_SCREENSHOT 0x04
#define DATATYPE_HEARTBEAT  0x05

#define USB_OP_QUICKLOAD 0x1
#define USB_OP_QUICKSAVE 0x2
#define USB_OP_DUMPHEAP  0x4
#define USB_OP_LOADMAP   0x8

// Globals
extern signed char IsEmulator;
extern signed char FlashCart;
extern unsigned long PendingUSBOperations;

/*********************************
          USB Functions
*********************************/

/*==============================
    I_InitFlashCart
    Initializes the USB buffers and pointers
    @return 1 if the USB initialization was successful, 0 if not
==============================*/

extern void I_InitFlashCart(void);


#ifdef USB

// Function pointers
extern void (*UsbFuncWriteStart)(int datatype, int size);
extern void (*UsbFuncWritePart)(const void* data, int size);
extern void (*UsbFuncWriteEnd)(int datatype, int size);

/* Check the USB for incoming commands and run them, should be called by main
 * thread every frame. */

int I_DispatchUSBCommands(void);

/* Dumps a binary file through USB
 * @param The file to dump
 * @param The size of the file */

void I_USBSendFile(void* file, int size);

/* Returns the size of the next token in the current command. */

int I_CmdNextTokenSize(void);

/* Stores the next part of the incoming command into the provided buffer and
 * advances the parser to the next token. Make sure the buffer can fit the
 * amount of data from I_CmdNextTokenSize! If you pass NULL, it skips this
 * token. */

void I_CmdGetNextToken(void* buffer);

/* Consumes all command tokens so the USB thread can continue. */
void I_CmdSkipAllTokens(void);

#endif /* USB */
