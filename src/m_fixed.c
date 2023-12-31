
/* m_fixed.c -- fixed point implementation */

#include "i_main.h"
#include "doomdef.h"
#include "p_spec.h"
#include "r_local.h"

/*
===============
=
= FixedDiv
=
===============
*/
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
    {
        c = (fixed_t) FixedDiv2(a, b);
    }

    return c;
}

char *PrintFixed(char *buf, fixed_t d)
{
    int whole, part;
    char *s = d < 0 ? "-" : "";

    d = D_abs(d);
    whole = d >> FRACBITS;
    part = (d & 0xffff)*10000/FRACUNIT;
    sprintf(buf, "%s%d.%04d", s, whole, part);
    return buf;
}
