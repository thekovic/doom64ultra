/* doomlib.c  */

#include "doomdef.h"
#include "i_debug.h"

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
    void *orig = dest;
    void *end = dest + count;
    if ((((u32) dest) & 7) == 0)
    {
        void *alignend = (void*)(((u32) dest) + (count & ~31));
        if (alignend > dest)
        {
            u64 val64;
            val = val & 0xff;
            asm volatile(
            ".set noreorder                     \n\t"
            ".set nomacro                       \n\t"
            ".set gp=64                         \n\t"
            "dsll %[val64], %[val], 8           \n\t"
            "or   %[val64], %[val64], %[val]    \n\t"
            "dsll %[val], %[val64], 16          \n\t"
            "or   %[val], %[val64], %[val]      \n\t"
            "dsll %[val64], %[val], 32          \n\t"
            "or   %[val64], %[val64], %[val]    \n\t"
            "1:                                 \n\t"
            "sd      %[val64], 0(%[dest])       \n\t"
            "sd      %[val64], 8(%[dest])       \n\t"
            "sd      %[val64], 16(%[dest])      \n\t"
            "sd      %[val64], 24(%[dest])      \n\t"
            "addiu   %[dest], %[dest], 32       \n\t"
            "bnel    %[dest], %[end], 1b        \n\t"
            "nop                                \n\t"
            ".set gp=32                         \n\t"
            ".set macro                         \n\t"
            ".set reorder                       \n\t"
            : [val64] "=&r" (val64), [val] "+&r" (val), [dest] "+&r" (dest)
            : [end] "r" (alignend)
               : "memory");
        }
    }
    if ((((u32) dest) & 3) == 0)
    {
        void *alignend = (void*)(((u32) end) & ~3);
        if (alignend > dest)
        {
            val = val & 0xff;
            val = (val<<8)|val;
            val = (val<<16)|val;
            do
            {
                *(u32*)dest = val;
                dest += sizeof(u32);
            }
            while (dest != alignend);
        }
    }
    while (dest != end)
    {
        *(u8*)dest = val;
        dest++;
    }
    if ((u32)dest-(u32)orig != count)
        D_printf("memset %d %ld %08lx -> %08lx\n", count, (u32)dest-(u32)orig, (u32)orig, (u32)dest);
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

asm(
    ".set gp=64                     \n\t"
    ".set noreorder                 \n\t"
    ".set nomacro                   \n\t"

    ".globl  __udivdi3              \n\t"
    ".ent    __udivdi3              \n\t"
"__udivdi3:                         \n\t"
    ".frame  $sp, 0, $ra            \n\t"
    "dsll    $a0, $a0, 32           \n\t"
    "dsll    $a1, $a1, 32           \n\t"
    "dsrl    $a1, $a1, 32           \n\t"
    "daddu   $a1, $a0, $a1          \n\t"
    "dsll    $a2, $a2, 32           \n\t"
    "dsll    $a3, $a3, 32           \n\t"
    "dsrl    $a3, $a3, 32           \n\t"
    "daddu   $a3, $a2, $a3          \n\t"
    "ddivu   $zero, $a1, $a3        \n\t"
    "mflo    $v1                    \n\t"
    "dsrl    $v0, $v1, 32           \n\t"
    "dsll    $v1, $v1, 32           \n\t"
    "jr      $ra                    \n\t"
    "dsrl    $v1, $v1, 32           \n\t"
    ".end    __udivdi3              \n\t"

    ".set macro                     \n\t"
    ".set reorder                   \n\t"
    ".set gp=32                     \n\t"
);

#ifndef NDEBUG
asm(
    ".set gp=64                     \n\t"
    ".set noreorder                 \n\t"
    ".set nomacro                   \n\t"

    ".globl  __umoddi3              \n\t"
    ".ent    __umoddi3              \n\t"
"__umoddi3:                         \n\t"
    ".frame  $sp, 0, $ra            \n\t"
    "dsll    $a0, $a0, 32           \n\t"
    "dsll    $a1, $a1, 32           \n\t"
    "dsrl    $a1, $a1, 32           \n\t"
    "daddu   $a1, $a0, $a1          \n\t"
    "dsll    $a2, $a2, 32           \n\t"
    "dsll    $a3, $a3, 32           \n\t"
    "dsrl    $a3, $a3, 32           \n\t"
    "daddu   $a3, $a2, $a3          \n\t"
    "ddivu   $zero, $a1, $a3        \n\t"
    "mfhi    $v1                    \n\t"
    "dsrl    $v0, $v1, 32           \n\t"
    "dsll    $v1, $v1, 32           \n\t"
    "jr      $ra                    \n\t"
    "dsrl    $v1, $v1, 32           \n\t"
    ".end    __umoddi3              \n\t"

    ".globl  __divdi3               \n\t"
    ".ent    __divdi3               \n\t"
"__divdi3:                          \n\t"
    ".frame  $sp, 0, $ra            \n\t"
    "dsll    $a0, $a0, 32           \n\t"
    "dsll    $a1, $a1, 32           \n\t"
    "dsrl    $a1, $a1, 32           \n\t"
    "daddu   $a1, $a0, $a1          \n\t"
    "dsll    $a2, $a2, 32           \n\t"
    "dsll    $a3, $a3, 32           \n\t"
    "dsrl    $a3, $a3, 32           \n\t"
    "daddu   $a3, $a2, $a3          \n\t"
    "ddiv    $zero, $a1, $a3        \n\t"
    "mflo    $v1                    \n\t"
    "dsrl    $v0, $v1, 32           \n\t"
    "dsll    $v1, $v1, 32           \n\t"
    "jr      $ra                    \n\t"
    "dsrl    $v1, $v1, 32           \n\t"
    ".end    __divdi3               \n\t"

    ".globl  __moddi3               \n\t"
    ".ent    __moddi3               \n\t"
"__moddi3:                          \n\t"
    ".frame  $sp, 0, $ra            \n\t"
    "dsll    $a0, $a0, 32           \n\t"
    "dsll    $a1, $a1, 32           \n\t"
    "dsrl    $a1, $a1, 32           \n\t"
    "daddu   $a1, $a0, $a1          \n\t"
    "dsll    $a2, $a2, 32           \n\t"
    "dsll    $a3, $a3, 32           \n\t"
    "dsrl    $a3, $a3, 32           \n\t"
    "daddu   $a3, $a2, $a3          \n\t"
    "ddiv    $zero, $a1, $a3        \n\t"
    "mfhi    $v1                    \n\t"
    "dsrl    $v0, $v1, 32           \n\t"
    "dsll    $v1, $v1, 32           \n\t"
    "jr      $ra                    \n\t"
    "dsrl    $v1, $v1, 32           \n\t"
    ".end    __moddi3               \n\t"

    ".set macro                     \n\t"
    ".set reorder                   \n\t"
    ".set gp=32                     \n\t"
);
#endif
