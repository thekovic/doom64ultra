/* ISC License
 * Copyright (c) 2018 Andrew Reece
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* https://github.com/azmr/blit-fonts */
#pragma once

#define blit32_WIDTH  5
#define blit32_HEIGHT 6
#define blit32_ADVANCE (blit32_WIDTH + 1)
#define blit32_DESCENDER 2
#define blit32_BASELINE_OFFSET (blit32_DESCENDER + 1)
#define blit32_ROW_ADVANCE (blit32_HEIGHT + blit32_BASELINE_OFFSET)
#define blit32_EXTRA_BITS(x) (((x) >> 30) & 3)

typedef unsigned long blit32_glyph;
typedef unsigned short blit_pixel;

enum {blit_Clip, blit_Wrap};
#define blit_inline static __attribute__((always_inline)) inline
#define blit_NUM_GLYPHS 95
#define blit_IndexFromASCII(ascii) (ascii - ' ')
#define blit_ASCIIFromIndex(index) (index + ' ')

static blit32_glyph blit32_Glyphs[blit_NUM_GLYPHS] =
{
    /* all chars up to 32 are non-printable */
    0x00000000,0x08021084,0x0000294a,0x15f52bea,0x08fa38be,0x33a22e60,0x2e94d8a6,0x00001084,
    0x10421088,0x04421082,0x00a23880,0x00471000,0x04420000,0x00070000,0x0c600000,0x02222200,
    0x1d3ad72e,0x3e4214c4,0x3e22222e,0x1d18320f,0x210fc888,0x1d183c3f,0x1d17844c,0x0222221f,
    0x1d18ba2e,0x210f463e,0x0c6018c0,0x04401000,0x10411100,0x00e03800,0x04441040,0x0802322e,
    0x3c1ef62e,0x231fc544,0x1f18be2f,0x3c10862e,0x1f18c62f,0x3e10bc3f,0x0210bc3f,0x1d1c843e,
    0x2318fe31,0x3e42109f,0x0c94211f,0x23149d31,0x3e108421,0x231ad6bb,0x239cd671,0x1d18c62e,
    0x0217c62f,0x30eac62e,0x2297c62f,0x1d141a2e,0x0842109f,0x1d18c631,0x08454631,0x375ad631,
    0x22a21151,0x08421151,0x3e22221f,0x1842108c,0x20820820,0x0c421086,0x00004544,0xbe000000,
    0x00000082,0x1c97b000,0x0e949c21,0x1c10b800,0x1c94b908,0x3c1fc5c0,0x42211c4c,0x4e87252e,
    0x12949c21,0x0c210040,0x8c421004,0x12519521,0x0c210842,0x235aac00,0x12949c00,0x0c949800,
    0x4213a526,0x7087252e,0x02149800,0x0e837000,0x0c213c42,0x0e94a400,0x0464a400,0x155ac400,
    0x36426c00,0x4e872529,0x1e223c00,0x1843188c,0x08421084,0x0c463086,0x0006d800,
};

/* StartX/Y refers to the top left corner of the glyph's bounding box */
static int blit32_TextNExplicit(blit_pixel *Buffer, blit_pixel Value, int Scale, int BufWidth, int BufHeight, int Wrap, int StartX, int StartY, int StrLen, const char *String)
{
    int IsNegative = BufWidth < 0;
    int DrawDir = IsNegative ? -1 : 1;
    int AbsBufWidth = DrawDir * BufWidth;
    unsigned int LinesPrinted = 1;
    int i, x, y;
    StrLen = StrLen >= 0 ? StrLen : 0x7FFFFFFF; /* if negative, wrap to max int */
    for(i = 0, x = StartX, y = StartY; String[i] && i < StrLen; ++i)
    {
        char c = String[i];
        int EndY = y + (blit32_HEIGHT + blit32_DESCENDER) * Scale * DrawDir;
        int BufUnderflow  = EndY < 0 || y < 0;
        int BufOverflow   = BufHeight && (EndY >= BufHeight || y >= BufHeight);
        int BufYMinExceed = BufUnderflow || (IsNegative && BufOverflow);
        int BufYMaxExceed = BufOverflow  || (IsNegative && BufUnderflow);
        int BufXMinExceed = x < 0;
        int EndX = c == '\n' || c == '\r' ? StartX            :
            c == '\t' ? x + 4 * Scale * blit32_ADVANCE :
            c == '\b' ?       x - Scale * blit32_WIDTH :
            /* normal char */ x + Scale * blit32_WIDTH;
        int BufXMaxExceed = EndX > AbsBufWidth;

        if(BufYMaxExceed) { break; } /* no point adding extra undrawable lines */
        else if((! BufYMinExceed && (! BufXMaxExceed || Wrap)) || c == '\n')
        {
            if(BufXMaxExceed && c != '\n') { c = '\n'; --i; } /* new line and redo on-screen checks */
            else if(BufXMinExceed)         { c = ' '; }       /* skip past character without drawing */
            switch(c)
            {
                default:                                                 /* normal character */
                    {
                        unsigned int glY, pxY, glX, pxX;
                        blit32_glyph Glyph = blit32_Glyphs[blit_IndexFromASCII(c)];
                        unsigned int OffsetY = y + blit32_EXTRA_BITS(Glyph) * Scale * DrawDir;
                        blit_pixel *Pixel, *Row = Buffer + OffsetY * AbsBufWidth + x;
                        for(glY = 0; glY < blit32_HEIGHT; ++glY)
                            for(pxY = Scale; pxY--; Row += BufWidth)
                                for(glX = 0, Pixel = Row; glX < blit32_WIDTH; ++glX)
                                {
                                    unsigned int Shift = glY * blit32_WIDTH + glX;
                                    unsigned int PixelDrawn = (Glyph >> Shift) & 1;
                                    if(PixelDrawn) for(pxX = Scale; pxX--; *Pixel++ = Value);
                                    else { Pixel += Scale; }
                                }
                    } /* fallthrough */
                case  ' ': x +=           Scale * blit32_ADVANCE; break; /* space: no need to touch pixels */
                case '\b': x -=           Scale * blit32_ADVANCE; break; /* non-destructive backspace */
                case '\t': x +=       4 * Scale * blit32_ADVANCE; break; /* tab: add 4 spaces */
                case '\n': y += DrawDir * Scale * blit32_ROW_ADVANCE; ++LinesPrinted; /* new line; fallthrough */
                case '\r': x  = StartX;                        break; /* carriage return */
            }
        }
    }
    return LinesPrinted;
}

blit_inline int blit32_TextExplicit(blit_pixel *Buffer, blit_pixel Value, int Scale, int BufWidth, int BufHeight, int Wrap, int StartX, int StartY, const char *String)
{ return blit32_TextNExplicit(Buffer, Value, Scale, BufWidth, BufHeight, Wrap, StartX, StartY, -1, String); }
