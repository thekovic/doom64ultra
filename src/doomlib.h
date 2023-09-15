#pragma once

#include <ultratypes.h>
#include <stdarg.h>
#include <stdbool.h>

#define DATA __attribute__((section(".data")))
#define SDATA __attribute__((section(".sdata")))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))
#define SEC_TEXT __attribute__((section(".text")))
#define SEC_GAME __attribute__((section(".text.game")))
#define SEC_MENU __attribute__((section(".text.menu")))
#define SEC_STARTUP __attribute__((section(".text.startup")))
#define INLINE_NEVER __attribute__((noinline))
#define INLINE_ALWAYS __attribute__((always_inline)) inline
#define NO_RETURN __attribute__((noreturn))

#define MAXCHAR ((char)0x7f)
#define MAXSHORT ((short)0x7fff)
#define MAXINT	((int)0x7fffffff)	/* max pos 32-bit int */
#define MAXLONG ((long)0x7fffffff)

#define MINCHAR ((char)0x80)
#define MINSHORT ((short)0x8000)
#define MININT 	((int)0x80000000)	/* max negative 32-bit integer */
#define MINLONG ((long)0x80000000)

#ifndef NULL
#define	NULL	0
#endif

typedef unsigned char byte;
typedef bool boolean;

/* */
/* library replacements */
/* */

extern int D_vsprintf(char *string, const char *format, va_list args);
extern int D_sprintf(char* dst, const char* fmt, ...) __attribute__((format (printf, 2, 3)));

#define sprintf D_sprintf

extern void bzero(void *, int);
extern int bcmp(void *, void *, int);
#define memcmp bcmp

void D_memmove(void *dest, const void *src);
void D_memset (void *dest, int val, int count);
void D_memcpy (void *dest, const void *src, int count);
void D_strncpy (char *dest, const char *src, int maxcount);
int D_strncmp (const char *s1, const char *s2, int len);
void D_strupr(char *s);
int D_strlen(const char *s);

void memcpy (void *dest, const void *src, int count);
#define D_memcpy memcpy

#define MEMORY_BARRIER() asm volatile ("" : : : "memory")
