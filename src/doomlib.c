/* doomlib.c  */

#include "doomdef.h"

#define WORDMASK    7

/*
====================
=
= D_memmove
=
====================
*/

void D_memmove(void *dest, const void *src) // 800019F0
{
    byte p;
    byte *p1;
    byte *p2;

    p = *(byte *)src;
    p1 = (byte *)src;
    p2 = (byte *)dest;

    *p2++ = *p1++;

    while (p != '\0')
    {
        p = *p1;
        *p2++ = *p1++;
    }
}

/*
====================
=
= D_memset
=
====================
*/

void D_memset(void *dest, int val, int count) // 80001A20
{
    byte    *p;
    int     *lp;
    int     v;

    /* round up to nearest word */
    p = dest;
    while ((int)p & WORDMASK)
    {
        if (--count < 0)
            return;
        *p++ = (char)val;
    }

    /* write 8 bytes at a time */
    lp = (int *)p;
    v = (int)(val << 24) | (val << 16) | (val << 8) | val;
    while (count >= 8)
    {
        lp[0] = lp[1] = v;
        lp += 2;
        count -= 8;
    }

    /* finish up */
    p = (byte *)lp;
    while (count--)
        *p++ = (char)val;
}

/*
====================
=
= D_memcpy
=
====================
*/

/*
void D_memcpy(void *dest, const void *src, int count) // 80001ACC
{
    byte    *d, *s;
    int     *ld, *ls;
    int     stopcnt;

    ld = (int *)dest;
    ls = (int *)src;

    if ((((int)ld | (int)ls | count) & 7))
    {
        d = (byte *)dest;
        s = (byte *)src;
        while (count--)
            *d++ = *s++;
    }
    else
    {
        if (count == 0)
            return;

        if(-(count & 31))
        {
            stopcnt = -(count & 31) + count;
            while (stopcnt != count)
            {
                ld[0] = ls[0];
                ld[1] = ls[1];
                ld += 2;
                ls += 2;
                count -= 8;
            }

            if (count == 0)
                return;
        }

        while (count)
        {
            ld[0] = ls[0];
            ld[1] = ls[1];
            ld[2] = ls[2];
            ld[3] = ls[3];
            ld[4] = ls[4];
            ld[5] = ls[5];
            ld[6] = ls[6];
            ld[7] = ls[7];
            ld += 8;
            ls += 8;
            count -= 32;
        }
    }
}
*/

/*
====================
=
= D_strncpy
=
====================
*/

void D_strncpy(char *dest, const char *src, int maxcount) // 8000lBB0
{
    byte    *p1, *p2;
    p1 = (byte *)dest;
    p2 = (byte *)src;
    while (maxcount--)
        if (!(*p1++ = *p2++))
            return;
}

/*
====================
=
= D_strncasecmp
=
====================
*/

int D_strncmp(const char *s1, const char *s2, int len) // 80001BEC
{
    while (*s1 && *s2)
    {
        if (*s1 != *s2)
            return *s1 < *s2 ? -1 : 1;
        s1++;
        s2++;
        if (!--len)
            return 0;
    }
    if (*s1 != *s2)
        return *s1 < *s2 ? -1 : 1;
    return 0;
}

/*
====================
=
= D_strupr
=
====================
*/

void D_strupr(char *s) // 80001C74
{
    char    c;

    while ((c = *s) != 0)
    {
        if (c >= 'a' && c <= 'z')
            c -= 'a' - 'A';
        *s++ = c;
    }
}

f32 D_rsqrtf(f32 input)
{
    s32 fac, mov;
    f32 ret, tmp, sum;
    asm volatile(
            "li      %[fac], 0x5f3759df         \n\t"
            "mfc1    %[mov], %[input]           \n\t"
            "srl     %[mov], %[mov], 1          \n\t"
            "subu    %[mov], %[fac], %[mov]     \n\t"
            "mtc1    %[mov], %[ret]             \n\t"
            "mul.s   %[sum], %[ret], %[ret]     \n\t"
            "lui     %[mov], 0xbf00             \n\t"
            "mtc1    %[mov], %[tmp]             \n\t"
            "mul.s   %[sum], %[sum], %[tmp]     \n\t"
            "lui     %[mov], 0x3fc0             \n\t"
            "mul.s   %[sum], %[sum], %[input]   \n\t"
            "mtc1    %[mov], %[tmp]             \n\t"
            "add.s   %[sum], %[sum], %[tmp]     \n\t"
            "mul.s   %[ret], %[ret], %[sum]     \n\t"
            : [ret] "=f" (ret),
              [fac] "=&r" (fac),
              [mov] "=&r" (mov),
              [tmp] "=&f" (tmp),
              [sum] "=&f" (sum)
            : [input] "f" (input));
    return ret;
}

/*
====================
=
= D_strlen
=
====================
*/

int D_strlen(const char *s) // 80001CC0
{
    int len = 0;

    while(*(s++))
        len++;

    return len;
}

// GCC 32/64-bit integer arithmetic support for 32-bit systems that can't link
// to libgcc.

// Function prototypes and descriptions are taken from
// https://gcc.gnu.org/onlinedocs/gccint/Integer-library-routines.html.

// This file may be #include'd by another file, so we try not to pollute the
// namespace and we don't import any headers.

// All functions must be resolvable by the linker and therefore can't be inline
// or static, even if they're #included into the file where they'll be used.

// For best performance we try to avoid branching. This makes the code a little
// weird in places.

// See https://github.com/glitchub/arith64 for more information.
// This software is released as-is into the public domain, as described at
// https://unlicense.org. Do whatever you like with it.

typedef union
{
    u64 u64;
    s64 s64;
    struct
    {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        u32 hi; u32 lo;
#else
        u32 lo; u32 hi;
#endif
    } u32;
    struct
    {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        s32 hi; s32 lo;
#else
        s32 lo; s32 hi;
#endif
    } s32;
} arith64_word;

// extract hi and lo 32-bit words from 64-bit value
#define arith64_hi(n) (arith64_word){.u64=n}.u32.hi
#define arith64_lo(n) (arith64_word){.u64=n}.u32.lo

// Negate a if b is negative, via invert and increment.
#define arith64_neg(a, b) (((a) ^ ((((s64)(b)) >= 0) - 1)) + (((s64)(b)) < 0))
#define arith64_abs(a) arith64_neg(a, a)

// Return the result of shifting a left by b bits.
__attribute__((used))
s64 __ashldi3(s64 a, int b)
{
    arith64_word w = {.s64 = a};

    b &= 63;

    if (b >= 32)
    {
        w.u32.hi = w.u32.lo << (b - 32);
        w.u32.lo = 0;
    } else if (b)
    {
        w.u32.hi = (w.u32.lo >> (32 - b)) | (w.u32.hi << b);
        w.u32.lo <<= b;
    }
    return w.s64;
}


int __clzdi2(u64 a)
{
    int b, n = 0;
    b = !(a & 0xffffffff00000000ULL) << 5; n += b; a <<= b;
    b = !(a & 0xffff000000000000ULL) << 4; n += b; a <<= b;
    b = !(a & 0xff00000000000000ULL) << 3; n += b; a <<= b;
    b = !(a & 0xf000000000000000ULL) << 2; n += b; a <<= b;
    b = !(a & 0xc000000000000000ULL) << 1; n += b; a <<= b;
    return n + !(a & 0x8000000000000000ULL);
}

// Calculate both the quotient and remainder of the unsigned division of a and
// b. The return value is the quotient, and the remainder is placed in variable
// pointed to by c (if it's not NULL).
u64 __divmoddi4(u64 a, u64 b, u64 *c)
{
    if (b > a)                                  // divisor > numerator?
    {
        if (c) *c = a;                          // remainder = numerator
        return 0;                               // quotient = 0
    }
    if (!arith64_hi(b))                         // divisor is 32-bit
    {
        if (b == 0)                             // divide by 0
        {
            volatile char x = 0; x = 1 / x;     // force an exception
        }
        if (b == 1)                             // divide by 1
        {
            if (c) *c = 0;                      // remainder = 0
            return a;                           // quotient = numerator
        }
        if (!arith64_hi(a))                     // numerator is also 32-bit
        {
            if (c)                              // use generic 32-bit operators
                *c = arith64_lo(a) % arith64_lo(b);
            return arith64_lo(a) / arith64_lo(b);
        }
    }

    // let's do long division
    char bits = __clzdi2(b) - __clzdi2(a) + 1;  // number of bits to iterate (a and b are non-zero)
    u64 rem = a >> bits;                   // init remainder
    a <<= 64 - bits;                            // shift numerator to the high bit
    u64 wrap = 0;                          // start with wrap = 0
    while (bits-- > 0)                          // for each bit
    {
        rem = (rem << 1) | (a >> 63);           // shift numerator MSB to remainder LSB
        a = (a << 1) | (wrap & 1);              // shift out the numerator, shift in wrap
        wrap = ((s64)(b - rem - 1) >> 63);  // wrap = (b > rem) ? 0 : 0xffffffffffffffff (via sign extension)
        rem -= b & wrap;                        // if (wrap) rem -= b
    }
    if (c) *c = rem;                            // maybe set remainder
    return (a << 1) | (wrap & 1);               // return the quotient
}

// Return the quotient of the signed division of a and b.
__attribute__((used))
s64 __divdi3(s64 a, s64 b)
{
    u64 q = __divmoddi4(arith64_abs(a), arith64_abs(b), (void *)0);
    return arith64_neg(q, a^b); // negate q if a and b signs are different
}

// Return the result of logically shifting a right by b bits.
__attribute__((used))
u64 __lshrdi3(u64 a, int b)
{
    arith64_word w = {.u64 = a};

    b &= 63;

    if (b >= 32)
    {
        w.u32.lo = w.u32.hi >> (b - 32);
        w.u32.hi = 0;
    } else if (b)
    {
        w.u32.lo = (w.u32.hi << (32 - b)) | (w.u32.lo >> b);
        w.u32.hi >>= b;
    }
    return w.u64;
}

// Return the remainder of the signed division of a and b.
__attribute__((used))
s64 __moddi3(s64 a, s64 b)
{
    u64 r;
    __divmoddi4(arith64_abs(a), arith64_abs(b), &r);
    return arith64_neg(r, a); // negate remainder if numerator is negative
}

// Return the quotient of the unsigned division of a and b.
__attribute__((used))
u64 __udivdi3(u64 a, u64 b)
{
    return __divmoddi4(a, b, (void *)0);
}

// Return the remainder of the unsigned division of a and b.
__attribute__((used))
u64 __umoddi3(u64 a, u64 b)
{
    u64 r;
    __divmoddi4(a, b, &r);
    return r;
}
