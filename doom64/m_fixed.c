
/* m_fixed.c -- fixed point implementation */

#include "i_main.h"
#include "doomdef.h"
#include "p_spec.h"
#include "r_local.h"

fixed_t finesine(int x)
{
    // original has qA = 12 (output range [-4095,4095])
    // we need 16 (output range [-65535,65535])
//    static const int qN = 13, qA= 16, qP= 15, qR= 2*qN-qP, qS= qN+qP+1-qA;

    // scale the input range
    // this makes it work as a replacement for the old finesine table
//    x <<= 2;
//    x = (x << (30 - qN));

    x = x << 19;

    if ((x ^ (x << 1)) < 0)
        x = (1 << 31) - x;

//    x = x >> (30 - qN);
    x = x >> 17;

//    return x * ((3 << qP) - ((x * x) >> qR)) >> qS;
    return x * (98304 - ((x * x) >> 11)) >> 13;
}

fixed_t finecosine(int x) {
    return finesine(x + 2048);
}

angle_t tantoangle(int x) {
    return ((angle_t)((-47*((x)*(x))) + (359628*(x)) - 3150270));
}

/*
===============
=
= FixedDiv
=
===============
*/
fixed_t FixedDiv2(register fixed_t a, register fixed_t b)//L8003EEF0()
{
	register unsigned        c;
	register unsigned        bit;
	register int             sign;

	sign = a^b;

	if (a <= 0)
		a = -a;

	if (b <= 0)
		b = -b;

	bit = 0x10000;
	do
	{
		b <<= 1;
		bit <<= 1;
	} while (b < a);

	c = 0;
	do
	{
		if (a >= b)
		{
			a -= b;
			c |= bit;
		}
		a <<= 1;
		bit >>= 1;
	} while (bit && a);

	if (sign < 0)
		c = -c;

	return c;
}
#if 1
fixed_t FixedDiv(fixed_t a, fixed_t b) // 80002BF8
{
    fixed_t     aa, bb;
    unsigned    c;
    int         sign;

    sign = a^b;

    if (a < 0)
        aa = -a;
    else
        aa = a;

    if (b < 0)
        bb = -b;
    else
        bb = b;

    if ((unsigned)(aa >> 14) >= bb)
    {
        if (sign < 0)
            c = MININT;
        else
            c = MAXINT;
    }
    else
        c = (fixed_t) FixedDiv2(a, b);

    return c;
}
#endif

/*
===============
=
= FixedMul
=
===============
*/
#if 1
fixed_t FixedMul2(fixed_t a, fixed_t b) // 800044D0
{
    s64 result = ((s64) a * (s64) b) >> 16;

    return (fixed_t) result;
}
#endif
#if 0
s64 FixedMul2(s64 a, s64 b) // 800044D0
{
    register s64 flo;

    //asm(".set noreorder");
    asm("dmult  $4, $5");
    asm("mflo   $3");
    asm("dsra   $3, 16");
    asm("move   %0, $3":"=r" (flo):);

    return (fixed_t) flo;

    /*
    dmult   $4, $5
    mflo    $2
    dsra    $2, $2, 16
    jr      $31
    nop
    */
}
#endif // 0

/*
===============
=
= FixedDiv2
=
===============
*/

fixed_t FixedDiv3(fixed_t a, fixed_t b) // 800044E4
{
    s64 result = ((s64) a << 16) / (s64)b;

    return (fixed_t) result;
}
