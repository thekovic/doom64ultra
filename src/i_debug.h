#pragma once

#include "config.h"

#if !defined(LOGGING) && (!defined(NDEBUG) || defined(USB))
#define LOGGING
#endif

/*********************************
         Debug Functions
*********************************/

#ifdef LOGGING

/* Initializes the logger and debug thread if enabled at build time. */
extern void I_InitDebugging();

/* Waits for gdb connection if required, and prints debug startup messages. */
extern void L_Init();

/* Prints a message with known length to the developer's command prompt.
 * The message must be null terminated.
 * @param msg The buffer to print.
 * @param len Length of characters in msg, should not include null terminator. */

extern void (*D_print)(const char *msg, unsigned long len);

/* Prints a formatted message to the developer's command prompt.
 * Supports up to 256 characters. */

extern void D_printf(const char* message, ...) __attribute__((format(printf, 1, 2)));

#else /* LOGGING */
#define I_InitDebugging()
#define L_Init()
#define D_print(msg, len)
#define D_printf(...)
#endif /* LOGGING */

/* Prints a statically allocated string. Argument must be a string literal, or
 * a char[]. Argument cannot be a pointer. */
#define D_printstatic(s) D_print((s), sizeof(s) - 1)
