#include "r_local.h"

void F_DrawSprite(int type, state_t *state, int rotframe, int color, int xpos, int ypos, fixed_t scale, int translation) // 80003D1C
{
    spriteframe_t   *sprframe;
    int              lump;

    u16 *paldata;
    byte *src;
    spriteN64_t *sprite;

    int xoffs;
    int tiles;
    int tilew;
    int width;
    int width2;
    fixed_t invscale;
    fixed_t yscale;
    fixed_t invyscale;

    int dsdx;
    int dtdy;
    int spos;
    int x1;
    int y1;
    int xh;
    int yh;

    sprframe = &sprites[state->sprite].spriteframes[state->frame & FF_FRAMEMASK];
    lump = sprframe->lump[rotframe];
    sprite = W_CacheLumpNum(lump, PU_CACHE, dec_jag, sizeof(spriteN64_t));
    tiles = sprite->tiles;

    if (tiles <= 0)
        return;

    invscale = scale;
    yscale = scale;
    if (VideoSettings.ScreenAspect)
        scale = FixedMul(scale, aspectscale[VideoSettings.ScreenAspect]);

    src = (byte *) &sprite[1];
    xoffs = (sprite->xoffs * scale) >> (FRACBITS - hudxshift);
    width = (sprite->width * scale) >> (FRACBITS - hudxshift);

    xpos <<= hudxshift;

    if (!sprframe->flip[rotframe])
    {
        x1 = xpos - xoffs;
        xh = x1 + width;
        spos = 0;
        dsdx = 1;
    }
    else
    {
        xh = xpos + xoffs;
        x1 = xh - width;
        spos = (sprite->width - 1) << 5;
        dsdx = -1;
    }

    if (xh <= 0 || x1 > (SCREEN_WD<<hudxshift))
        return;

    if (osTvType == OS_TV_PAL)
        yscale = FixedMul(yscale, 0x13333);

    y1 = ((ypos << FRACBITS) - sprite->yoffs * yscale) >> (FRACBITS - hudyshift);

    int height = sprite->height;
    int tileh = sprite->tileheight;
    int screenheight = SCREEN_HT<<hudyshift;

    if (y1 + ((height * yscale) >> (FRACBITS - hudyshift)) <= 0 || y1 > screenheight)
        return;

    if (invscale != FRACUNIT)
        invscale = FixedDiv2(FRACUNIT, invscale);
    invyscale = invscale;
    if (VideoSettings.ScreenAspect)
        invscale = FixedMul(invscale, invaspectscale[VideoSettings.ScreenAspect]);
    if (osTvType == OS_TV_PAL)
        invyscale = FixedMul(invyscale, 0xd555);

    if (x1 < 0)
    {
        int soff = (-x1 * invscale) >> (FRACBITS-5+hudxshift);
        if (spos)
            spos -= soff;
        else
            spos = soff;
        x1 = 0;
    }
    xh = MIN(xh, SCREEN_WD<<hudxshift);

    dsdx = (dsdx * invscale) >> (FRACBITS-10-2+hudxshift);
    dtdy = invyscale >> (FRACBITS-10-2+hudyshift);

    R_RenderModes(rm_hudsprite);

    gDPSetPrimColorD64(GFX1++, 0, 0, color);

    if ((color & 255) < 255)
    {
        gDPSetRenderMode(GFX1++, G_RM_XLU_SURF_CLAMP, G_RM_XLU_SURF2_CLAMP);
    }
    else
    {
        gDPSetRenderMode(GFX1++, G_RM_TEX_EDGE, G_RM_TEX_EDGE2);
    }

    if (sprite->compressed < 0)
    {
        width2 = ALIGN(sprite->width, 8);
        tilew = tileh * width2;

        if (lump >= firstplayerlump && lump <= lastplayerlump && translation >= 0)
            paldata = playerpalettes[translation];
        else if (sprite->cmpsize & 1)
            paldata = W_CacheLumpNum(((mobjinfo[type].palette + lump) -
                                    (sprite->cmpsize >> 1)), PU_CACHE, dec_jag, 0) + 8;
        else
            paldata = (u16 *) (src + sprite->cmpsize);

        /* Load Palette Data (256 colors) */
        gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1, paldata);

        gDPTileSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 255);
    }
    else
    {
        width2 = ALIGN(sprite->width, 16);
        tilew = tileh * width2;

        if (tilew < 0)
            tilew = tilew + 1;

        tilew >>= 1;

        register const bool isblood = lump >= bloodlump && lump < bloodlump + 4;
        if (isblood && Settings.GreenBlood)
            paldata = bloodpalettes[lump - bloodlump][1];
        else if (isblood && translation >= 0)
            paldata = bloodpalettes[lump - bloodlump][translation];
        else if (lump == giblump && translation >= 0)
            paldata = bloodpalettes[4][translation];
        else
            paldata = (u16*) (src + sprite->cmpsize);

        /* Load Palette Data (16 colors) */
        gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1, paldata);

        gDPTileSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 15);
    }

    gDPPipeSync(GFX1++);

    int uy1, t, uyh;

    for (int tilecnt = 0; tilecnt < tiles; tilecnt++)
    {
        int tpos = MIN(tileh, height);
        yh = y1 + ((tpos * yscale) >> (FRACBITS - hudyshift));

        if (y1 >= screenheight)
            break;

        if (yh > 0)
        {
            gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
            gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);
            gDPLoadSync(GFX1++);

            if (sprite->compressed < 0)
            {
                /* Load Image Data (8bit) */
                gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, ((width2 * tpos + 1) >> 1) - 1, 0);
                gDPPipeSync(GFX1++);
                gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b, ((width2 + 7) >> 3), 0,
                           G_TX_RENDERTILE, 0, 0, 0, 0, 0, 0, 0);
            }
            else
            {
                /* Load Image Data (4bit) */
                gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, ((width2 * tpos + 3) >> 2) - 1, 0);
                gDPPipeSync(GFX1++);
                gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_4b, (((width2>>1) + 7) >> 3), 0,
                           G_TX_RENDERTILE, 0, 0, 0, 0, 0, 0, 0);
            }

            gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0, ((width2 - 1) << 2), (tpos - 1) << 2);

            if (y1 < 0)
            {
                t = (-y1 * invyscale) >> (FRACBITS-5+hudyshift);
                uy1 = 0;
            }
            else
            {
                t = 0 << 5;
                uy1 = y1;
            }
            uyh = MIN(yh, screenheight);

            gSPTextureRectangle(GFX1++, x1, uy1, xh, uyh, G_TX_RENDERTILE,
                                spos, t, dsdx, dtdy);
        }

        height -= tpos;
        src += tilew;
        y1 = yh;
    }

    DEBUG_COUNTER(LastVisThings += 1);

    globallump = -1;
}
