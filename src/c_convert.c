/* c_convert.c  */

/*-----------------------------------*/
/* Color Converter RGB2HSV & HSV2RGB */
/*-----------------------------------*/

#include "doomdef.h"

/*
===================
=
= C_LightGetHSV
= Set HSV values based on given RGB
=
===================
*/

int C_LightGetHSV(int r,int g,int b) // 800020BC
{
    unsigned char min, max;
    unsigned char h_, s_, v_;

    min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    max = r > g ? (r > b ? r : b) : (g > b ? g : b);

    v_ = max;
    if (v_ == 0)
    {
        return 0;
    }

    s_ = (255 * (long)(max - min)) / v_;
    if (s_ == 0)
    {
        return v_ & 0x000000FF;
    }

    if (max == r)
    {
        h_ = 0 + 43 * (g - b) / (max - min);
    }
    else if (max == g)
    {
        h_ = 85 + 43 * (b - r) / (max - min);
    }
    else
    {
        h_ = 171 + 43 * (r - g) / (max - min);
    }

    return (((h_ << 16) | (s_ << 8) | v_) & 0x00FFFFFF);
}

/*
===================
=
= C_LightGetRGB
= Set RGB values based on given HSV
=
===================
*/

int C_LightGetRGB(int h,int s,int v) // 8000248C
{
    unsigned char r,g,b;
    unsigned char region, remainder, p, q, t;

    if (s == 0)
    {
        return (((v&0xff) << 16) | ((v&0xff) << 8) | (v&0xff)) & 0x00FFFFFF;
    }

    region = h / 43;
    remainder = (h - (region * 43)) * 6;

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
        {
            r = v; g = t; b = p;
            break;
        }
        case 1:
        {
            r = q; g = v; b = p;
            break;
        }
        case 2:
        {
            r = p; g = v; b = t;
            break;
        }
        case 3:
        {
            r = p; g = q; b = v;
            break;
        }
        case 4:
        {
            r = t; g = p; b = v;
            break;
        }
        default:
        {
            r = v; g = p; b = q;
            break;
        }
    }

    return (((r&0xff) << 16) | ((g&0xff) << 8) | (b&0xff)) & 0x00FFFFFF;
}

u32 C_AddColors(u32 c1, u32 c2)
{
    u32 e1, e2, er, r = 0;

    for (unsigned i = 0; i <= 24; i += 8)
    {
        e1 = (((unsigned) c1) >> i) & 0xff;
        e2 = (((unsigned) c2) >> i) & 0xff;
        er = e1 + e2;
        er = MIN(er, 0xff);
        r |= er << i;
    }

    return r;
}

u32 C_LerpColors(u32 a, u32 b, u32 fac)
{
    fac = MIN(fac, 256);
    u32 nfac = 256 - fac;
    return (((((a>>24) * nfac + (b>>24) * fac))&0xff00)<<16)
        | ((((((a>>16)&0xff) * nfac + ((b>>16)&0xff) * fac))&0xff00)<<8)
        | (((((a>>8)&0xff) * nfac + ((b>>8)&0xff) * fac))&0xff00)
        | (((((a&0xff) * nfac + (b&0xff) * fac))&0xff00)>>8);
}

u32 C_MultColor(u32 c, u8 fac)
{
    u32 v, r = c & 0xff;
    for (unsigned i = 8; i <= 24; i += 8)
    {
        v = (((c>>i)&0xff)*((u32)fac))>>8;
        v = MIN(v, 0xff);
        r |= (v<<i);
    }
    return r;
}
