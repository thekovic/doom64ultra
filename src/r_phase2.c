
//Renderer phase 2 - Sky Rendering Routines

#include "doomdef.h"
#include "r_local.h"

#define FIRESKY_WIDTH   64
#define FIRESKY_HEIGHT  64

typedef enum
{
    SKF_CLOUD       = 1,
    SKF_THUNDER     = 2,
    SKF_MOUNTAIN    = 4,
} skyflags_e;

Vtx SkyCloudVertex[4] = // 8005B1D0
{
    { .v = { {-3808, 180,  3808}, 0, {(0 << 6), (0 << 6)}, {0, 0, 0, 0xff} } },
    { .v = { { 3808, 180,  3808}, 0, {(0 << 6), (0 << 6)}, {0, 0, 0, 0xff} } },
    { .v = { { 3808, 180, -3808}, 0, {(0 << 6), (0 << 6)}, {0, 0, 0, 0xff} } },
    { .v = { {-3808, 180, -3808}, 0, {(0 << 6), (0 << 6)}, {0, 0, 0, 0xff} } },
};

Vtx SkyFireVertex[4] = // 8005B210
{
    { .v = { { -160, 120, 0}, 0, {(0 << 6), (0  << 6)}, {0, 0, 0, 0xff} } },
    { .v = { {  160, 120, 0}, 0, {(0 << 6), (0  << 6)}, {0, 0, 0, 0xff} } },
    { .v = { {  160,   0, 0}, 0, {(0 << 6), (64 << 6)}, {0, 0, 0, 0xff} } },
    { .v = { { -160,   0, 0}, 0, {(0 << 6), (64 << 6)}, {0, 0, 0, 0xff} } },
};

fixed_t     FogNear;            // 800A8120
int         FogColor;           // 800A8124
skyfunc_t   R_RenderSKY;        // 800A8130
byte        *SkyFireData[2];    // 800A8140 // Fire data double buffer
byte        *SkyCloudData;      // 800A8148
int         Skyfadeback;        // 800A814C
int         FireSide = 0;       // 800A8150
int         SkyCloudOffsetX;    // 800A8154
int         SkyCloudOffsetY;    // 800A8158
int         ThunderCounter;     // 800A815C
int         LightningCounter;   // 800A8160
int         SkyPicSpace;        // 800A8164
int         SkyPicMount;        // 800A8168
int         SkyCloudColor;      // 800A816C
int         SkyVoidColor;       // 800A8170
int         SkyFlags;           // 800A8174

void R_RenderSpaceSky(void) HOT;
void R_RenderCloudSky(void) HOT;
void R_RenderVoidSky(void) HOT;
void R_RenderEvilSky(void) HOT;
void R_RenderClouds(void) HOT;
void R_RenderSkyPic(int lump, int yoffset, boolean repeat) HOT;
void R_RenderFireSky(void) HOT;
void R_CloudThunder(void) HOT;

void R_FreeSky(void)
{
    if (R_RenderSKY == R_RenderFireSky)
    {
        Z_Free(SkyFireData[0]);
        Z_Free(SkyFireData[1]);
    }
    else if (R_RenderSKY == R_RenderCloudSky)
    {
        Z_Free(SkyCloudData);
    }
}

void R_SetupSky(void) // 80025060
{
    byte *data;

    FogNear = 985;
    FogColor = PACKRGBA(0,0,0,0);
    R_RenderSKY = NULL;
    SkyFlags = 0;
    SkyCloudOffsetX = 0;
    SkyCloudOffsetY = 0;
    ThunderCounter = 180;
    LightningCounter = 0;
    FireSide = 0;
    Skyfadeback = 0;
    SkyPicSpace = W_GetNumForName("SPACE");

    switch(skytexture)
    {
        case 1:
        case 10:
            SkyFlags = (SKF_CLOUD|SKF_THUNDER);
            R_RenderSKY = R_RenderCloudSky;

            SkyCloudData = (byte *)W_CacheLumpName("CLOUD", PU_STATIC, dec_jag, 0);
            SkyCloudColor = PACKRGBA(176,128,255,255); // 0xb080ffff

            if (skytexture == 10)
            {
                SkyPicMount = W_GetNumForName("MOUNTC");
                SkyFlags |= SKF_MOUNTAIN;
            }

            *(int*)SkyCloudVertex[0].v.cn = PACKRGBA(0,0,0,255); // 0xff;
            *(int*)SkyCloudVertex[1].v.cn = PACKRGBA(0,0,0,255); // 0xff;
            *(int*)SkyCloudVertex[2].v.cn = PACKRGBA(0,0,21,255); // 0x15ff;
            *(int*)SkyCloudVertex[3].v.cn = PACKRGBA(0,0,21,255); // 0x15ff;
            break;

        case 2:
            SkyFlags = SKF_CLOUD;
            R_RenderSKY = R_RenderCloudSky;

            SkyCloudData = (byte *)W_CacheLumpName("CLOUD", PU_STATIC, dec_jag, 0);
            SkyCloudColor = PACKRGBA(255,48,48,255); // 0xff3030ff;

            *(int*)SkyCloudVertex[0].v.cn = PACKRGBA(16,0,0,255); // 0x100000ff;
            *(int*)SkyCloudVertex[1].v.cn = PACKRGBA(16,0,0,255); // 0x100000ff;
            *(int*)SkyCloudVertex[2].v.cn = PACKRGBA(16,0,0,255); // 0x100000ff;
            *(int*)SkyCloudVertex[3].v.cn = PACKRGBA(16,0,0,255); // 0x100000ff;
            break;

        case 3:
        case 5:
            SkyFlags = SKF_CLOUD;
            R_RenderSKY = R_RenderCloudSky;

            SkyCloudData = (byte *)W_CacheLumpName("CLOUD", PU_STATIC, dec_jag, 0);
            SkyCloudColor = PACKRGBA(208,112,64,255); // 0xd07040ff;

            if (skytexture == 3)
            {
                SkyPicMount = W_GetNumForName("MOUNTB");
                SkyFlags |= SKF_MOUNTAIN;
            }
            else
            {
                FogNear = 975;
                FogColor = PACKRGBA(48,16,8,0); // 0x30100800;
            }

            *(int*)SkyCloudVertex[0].v.cn = PACKRGBA(0,0,0,255); // 0xff;
            *(int*)SkyCloudVertex[1].v.cn = PACKRGBA(0,0,0,255); // 0xff;
            *(int*)SkyCloudVertex[2].v.cn = PACKRGBA(64,16,0,255); // 0x401000ff;
            *(int*)SkyCloudVertex[3].v.cn = PACKRGBA(64,16,0,255); // 0x401000ff;
            break;

        case 4:
        case 9:
            R_RenderSKY = R_RenderFireSky;

            data = W_CacheLumpName("FIRE", PU_LEVEL, dec_jag, -1);
            SkyFireData[0] = (data + 8);
            SkyFireData[1] = Z_Malloc((FIRESKY_WIDTH*FIRESKY_HEIGHT), PU_LEVEL, NULL);

            D_memcpy(SkyFireData[1], SkyFireData[0],(FIRESKY_WIDTH*FIRESKY_HEIGHT));

            if (skytexture == 4)
            {
                *(int*)SkyFireVertex[0].v.cn = PACKRGBA(255,0,0,255); // 0xff0000ff;
                *(int*)SkyFireVertex[1].v.cn = PACKRGBA(255,0,0,255); // 0xff0000ff;
                *(int*)SkyFireVertex[2].v.cn = PACKRGBA(255,96,0,255); // 0xff6000ff;
                *(int*)SkyFireVertex[3].v.cn = PACKRGBA(255,96,0,255); // 0xff6000ff;
            }
            else
            {
                *(int*)SkyFireVertex[0].v.cn = PACKRGBA(0,255,0,255); // 0xff00ff;
                *(int*)SkyFireVertex[1].v.cn = PACKRGBA(0,255,0,255); // 0xff00ff;
                *(int*)SkyFireVertex[2].v.cn = PACKRGBA(112,112,0,255); // 0x707000ff;
                *(int*)SkyFireVertex[3].v.cn = PACKRGBA(112,112,0,255); // 0x707000ff;
            }
            break;

        case 6:
            R_RenderSKY = R_RenderSpaceSky;
            break;

        case 7:
            FogNear = 995;
            R_RenderSKY = R_RenderEvilSky;
            break;

        case 8:
            R_RenderSKY = R_RenderVoidSky;
            FogNear = 975;
            FogColor = PACKRGBA(0,64,64,0); // 0x404000;
            SkyVoidColor = PACKRGBA(0,56,56,0); // 0x383800;
            break;

        case 11:
            R_RenderSKY = R_RenderSpaceSky;
            SkyPicMount = W_GetNumForName("MOUNTA");
            SkyFlags |= SKF_MOUNTAIN;
            break;
    }
}

static int R_ProjectSkyHorizon(void)
{
    /* extremely shortened version of matrix transform, just calculate the y
     * value from the horizon at far plane and then generate all offsets from that */
    fixed_t y, w;

    w = -viewpitchcos * 3808;                            // rotate z
    y = FixedMul(viewvcot, -viewpitchsin * 3808);        // scale by vfov
    y = ((FixedDiv(y, w) * (YResolution/2)) >> FRACBITS);  // convert to screen space

    return y;
}

void R_RenderSpaceSky(void) // 80025440
{
    int pitchoffset;

    pitchoffset = R_ProjectSkyHorizon();
    gDPSetAlphaCompare(GFX1++, G_AC_NONE);
    gDPSetCombineMode(GFX1++, G_CC_D64COMB09, G_CC_D64COMB09);
    gDPSetRenderMode(GFX1++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gDPSetPrimColor(GFX1++, 0, (lights[255].rgba >> 8), 0, 0, 0, 255);

    R_RenderSkyPic(SkyPicSpace, pitchoffset - YResolution/2, true);

    if (SkyFlags & SKF_MOUNTAIN)
    {
        gDPPipeSync(GFX1++);
        gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);
        gDPSetCombineMode(GFX1++, G_CC_D64COMB10, G_CC_D64COMB10);
        gDPSetRenderMode(GFX1++, G_RM_TEX_EDGE, G_RM_TEX_EDGE2);

        R_RenderSkyPic(SkyPicMount, pitchoffset + YResolution/2 + 50, false);
    }
}

void R_RenderCloudSky(void) // 800255B8
{
    int pitchoffset;
    int cloudoff = 50<<(hudyshift-2);

    pitchoffset = R_ProjectSkyHorizon();
    pitchoffset += YResolution/2 - cloudoff;

    if (SkyFlags & SKF_MOUNTAIN)
        pitchoffset += cloudoff<<1;     // don't clear where the mountain would be drawn

    if (pitchoffset < YResolution)
    {
        u32 color = FlashEnvColor;

        gDPSetCycleType(GFX1++, G_CYC_FILL);
        gDPSetRenderMode(GFX1++,G_RM_NOOP,G_RM_NOOP2);
        if (VideoSettings.BitDepth == BITDEPTH_16)
        {
            color = RGBATO5551(color);
            color |= (color << 16);
        }
        gDPSetFillColor(GFX1++, color);
        gDPFillRectangle(GFX1++, 0, MAX(pitchoffset, 0), XResolution-1, YResolution-1);
    }

    if (SkyFlags & SKF_CLOUD)
        R_RenderClouds();

    if (SkyFlags & SKF_THUNDER)
        R_CloudThunder();

    if (SkyFlags & SKF_MOUNTAIN)
    {
        gDPPipeSync(GFX1++);
        gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);
        gDPSetCombineMode(GFX1++, G_CC_D64COMB10, G_CC_D64COMB10);
        gDPSetRenderMode(GFX1++, G_RM_TEX_EDGE, G_RM_TEX_EDGE2);

        R_RenderSkyPic(SkyPicMount, pitchoffset, false);
    }
}

void R_RenderVoidSky(void) // 800256B4
{
    u32 color = SkyVoidColor;

    color = C_MultColor(color, lights[255].rgba >> 8);
    color = C_AddColors(color, FlashEnvColor & 0xffffff00);
    gDPSetCycleType(GFX1++, G_CYC_FILL);
    gDPSetRenderMode(GFX1++,G_RM_NOOP,G_RM_NOOP2);

    // Fill borders with SkyVoidColor
    if (VideoSettings.BitDepth == BITDEPTH_16)
    {
        color = RGBATO5551(color);
        color |= (color << 16);
    }
    gDPSetFillColor(GFX1++, color);
    gDPFillRectangle(GFX1++, 0, 0, XResolution-1, YResolution-1);

    gDPPipeSync(GFX1++);
    gDPSetCycleType(GFX1++, G_CYC_2CYCLE);
}

void R_RenderEvilSky(void) // 80025738
{
    int color;
    int pitchoffset;

    gDPSetPrimColor(GFX1++, 0, ((lights[255].rgba >> 8)  - Skyfadeback), 0, 0, 0, 255);
    gDPSetAlphaCompare(GFX1++, G_AC_NONE);
    gDPSetCombineMode(GFX1++, G_CC_D64COMB09, G_CC_D64COMB09);
    gDPSetRenderMode(GFX1++,G_RM_OPA_SURF,G_RM_OPA_SURF2);

    pitchoffset = R_ProjectSkyHorizon();
    R_RenderSkyPic(SkyPicSpace, pitchoffset - YResolution/2, true);

    if (Skyfadeback)
    {
        Skyfadeback += 4;

        if (Skyfadeback > 255)
            Skyfadeback = 255;

        if (Skyfadeback > 128)
            color = 128;
        else
            color = Skyfadeback;

        M_DrawBackground(63, SCREEN_HT/2-95, color, "EVIL");
    }
}

extern Mtx R_ModelMatrix;
extern Mtx R_ProjectionMatrix;

void R_RenderClouds(void) // 80025878
{
    int x, y;

    gDPSetCycleType(GFX1++, G_CYC_2CYCLE);
    gDPSetTexturePersp(GFX1++, G_TP_PERSP);
    gDPSetTextureLUT(GFX1++, G_TT_NONE);
    gDPSetCombineMode(GFX1++, G_CC_D64COMB11, G_CC_D64COMB12);
    gDPSetRenderMode(GFX1++,G_RM_OPA_SURF,G_RM_OPA_SURF2);

    gDPSetPrimColorD64(GFX1++, 0, (lights[255].rgba >> 8), SkyCloudColor);

    R_RotateCameraMatrix();

    if (!gamepaused)
    {
        SkyCloudOffsetX -= 1;
        SkyCloudOffsetY += 2;
    }

    x = SkyCloudOffsetX;
    y = SkyCloudOffsetY;

    D_memcpy(VTX1, SkyCloudVertex, sizeof(Vtx)*4);

    VTX1[0].v.tc[0] = x;
    VTX1[1].v.tc[0] = x + (64 << 8);
    VTX1[2].v.tc[0] = x + (64 << 8);
    VTX1[3].v.tc[0] = x;

    VTX1[0].v.tc[1] = y;
    VTX1[1].v.tc[1] = y;
    VTX1[2].v.tc[1] = y + (128 << 8);
    VTX1[3].v.tc[1] = y + (128 << 8);

    gDPSetTextureImage(GFX1++, G_IM_FMT_I, G_IM_SIZ_16b , 1, (SkyCloudData+8));
    gDPSetTile(GFX1++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, G_TX_WRAP, 6, 0, G_TX_WRAP, 6, 0);

    gDPLoadSync(GFX1++);
    gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, (((64 * 64) -1) >> 1), 0);

    gDPPipeSync(GFX1++);
    gDPSetTile(GFX1++, G_IM_FMT_I, G_IM_SIZ_8b, 8, 0, G_TX_RENDERTILE, 0, G_TX_WRAP, 6, 0, G_TX_WRAP, 6, 0);
    gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0, (63 << 2), (63 << 2));
    gSPTexture(GFX1++, 0xe000, 0xe000, 0, G_TX_RENDERTILE, G_ON);

    gSPVertex(GFX1++, VTX1, 4, 0);
    gSP2Triangles(GFX1++, 0, 2, 1, 0, 0, 3, 2, 0);
    DEBUG_COUNTER(LastVisTriangles += 2);

    VTX1 += 4;

    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(&R_ModelMatrix), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
}

void R_RenderSkyPic(int lump, int yoffset, boolean repeat) // 80025BDC
{
    byte *data;
    byte *src;
    byte *paldata;
    int i;

    int tileh;
    int yl, yh = 0, nextyh;
    int ang;
    int lrs;
    int spriteh;
    int dsdx;

    data = W_CacheLumpNum(lump, PU_CACHE, dec_jag, sizeof(spriteN64_t));

    ang = (((XResolution/2*-viewinvhcot)>>FRACBITS) - (viewangle >> 22)) & 255;
    tileh = ((spriteN64_t*)data)->tileheight;
    dsdx = viewinvhcot >> 4 >> hudxshift;

    src = data + sizeof(spriteN64_t);
    paldata = (src + ((spriteN64_t*)data)->cmpsize);

    gDPSetCycleType(GFX1++, G_CYC_1CYCLE);

    gDPSetTexturePersp(GFX1++, G_TP_NONE);
    gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
    gDPSetBlendColor(GFX1++, 0, 0, 0, 0);

    // Load Palette Data (256 colors)
    gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1, paldata);

    gDPTileSync(GFX1++);
    gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

    gDPLoadSync(GFX1++);
    gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 255);

    gDPPipeSync(GFX1++);

    spriteh = ((spriteN64_t*)data)->height << 2;
    lrs = (((tileh << 8) + 1) >> 1) - 1;
    yl = (yoffset << 2) - spriteh;
    if (repeat)
        while (yl > 0)
            yl -= spriteh;

    for(i = 0; i < ((spriteN64_t*)data)->tiles; i++)
    {
        nextyh = yh + (tileh << hudyshift);

        if (!repeat && yl + spriteh <= 0)
            continue;

        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 8, 0);

        gDPLoadSync(GFX1++);
        gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, lrs, 0);

        gDPPipeSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b, 32, 0, G_TX_RENDERTILE, 0, 0, 0, 0, 0, 8, 0);
        gDPSetTileSize(GFX1++, 0, 0, 0, ((256-1) << 2), ((tileh - 1) << 2));

        for (int tyl = yl; tyl < (YResolution<<2); tyl += spriteh)
        {
            int ly = tyl + nextyh;
            if (ly > 0)
            {
                int uy = tyl + yh;
                int t = 0;

                if (uy < 0)
                {
                    t = (-uy) << 3;
                    uy = 0;
                }

                gSPTextureRectangle(GFX1++, 0, uy,
                                            SCREEN_WD << hudxshift, ly,
                                            G_TX_RENDERTILE,
                                            (ang << 5), t,
                                            dsdx, (1 << 12 >> hudyshift));
            }
            if (!repeat)
                break;
        }
        yh = nextyh;
        src += (tileh << 8);
    }

    gDPPipeSync(GFX1++);
    gDPSetCycleType(GFX1++, G_CYC_2CYCLE);
}

void R_SpreadFire(byte *src, int seed)
{
    int x, y, randIdx;
    byte *srcoffset, *tmpSrc, pixel;

    src += FIRESKY_WIDTH;

    for (x = 0; x < FIRESKY_WIDTH; x++)
    {
        srcoffset = (src + x);

        for (y = 1; y < FIRESKY_HEIGHT; y++)
        {
            pixel = *srcoffset;
            if (pixel != 0)
            {
                randIdx = rndtable[seed];
                seed = (seed + 2) & 0xff;

                tmpSrc = &src[(x - (randIdx & 3) + 1) & (FIRESKY_WIDTH-1)];
                *(tmpSrc - FIRESKY_WIDTH) = pixel - ((randIdx & 1) << 4);
            }
            else
            {
                *(srcoffset - FIRESKY_WIDTH) = 0;
            }

            src += FIRESKY_WIDTH;
            srcoffset += FIRESKY_WIDTH;
        }

        src -= ((FIRESKY_WIDTH*FIRESKY_HEIGHT) - FIRESKY_WIDTH);
    }
}

void R_RenderFireSky(void) // 80025F68
{
    byte *buff;
    int ang, t, t2;
    u32 color;
    int pitchoffset, topoffset;

    gDPSetCycleType(GFX1++, G_CYC_FILL);
    gDPSetRenderMode(GFX1++,G_RM_NOOP,G_RM_NOOP2);

    pitchoffset = R_ProjectSkyHorizon();
    pitchoffset += YResolution/2;
    topoffset = pitchoffset - 120;
    if (topoffset > 0)
    {
        color = FlashEnvColor;
        if (VideoSettings.BitDepth == BITDEPTH_16)
        {
            color = RGBATO5551(color);
            color |= (color << 16);
        }
        gDPSetFillColor(GFX1++, color);
        gDPFillRectangle(GFX1++, 0, 0, XResolution-1, topoffset-1);
    }
    if (pitchoffset < YResolution)
    {
        color = *(int*)SkyFireVertex[2].v.cn;
        color = C_MultColor(color, lights[255].rgba >> 8);
        color = C_AddColors(color, FlashEnvColor & 0xffffff00);
        if (VideoSettings.BitDepth == BITDEPTH_16)
        {
            color = RGBATO5551(color);
            color |= (color << 16);
        }
        gDPSetFillColor(GFX1++, color);
        gDPFillRectangle(GFX1++, 0, MAX(pitchoffset, 0), XResolution-1, YResolution-1);
    }
    pitchoffset -= YResolution/2;

    gDPSetCycleType(GFX1++, G_CYC_2CYCLE);
    gDPSetTexturePersp(GFX1++, G_TP_PERSP);
    gDPSetTextureLUT(GFX1++, G_TT_NONE);
    gDPSetCombineMode(GFX1++, G_CC_D64COMB13, G_CC_D64COMB14);
    gDPSetRenderMode(GFX1++,G_RM_OPA_SURF,G_RM_OPA_SURF2);
    gDPSetPrimColor(GFX1++, 0, (lights[255].rgba >> 8), 0, 0, 0, 255);

    if (((gamevbls < gametic) && (gametic & 1)) && (!gamepaused))
    {
        buff = SkyFireData[FireSide];
        D_memcpy(buff, SkyFireData[FireSide ^ 1], (FIRESKY_WIDTH*FIRESKY_HEIGHT));

        R_SpreadFire(buff, M_Random());

        FireSide ^= 1;
    }
    else
    {
        buff = SkyFireData[FireSide ^ 1];
    }

    D_memcpy(VTX1, SkyFireVertex, sizeof(Vtx)*4);

    // somwehere around 2.375 the T coord can wrap, so just clamp it
    ang = ((SCREEN_WD / 2 * -MIN(viewinvhcot, 0x26000))>>FRACBITS) - (viewangle >> 22);
    t = ((ang & 255) << 5);
    t2 = t + ((SCREEN_WD * MIN(viewinvhcot, 0x26000))>>11);

    for (int i = 0; i < 4; i++)
        VTX1[i].v.ob[1] -= pitchoffset;

    VTX1[0].v.tc[0] = t;
    VTX1[1].v.tc[0] = t2;
    VTX1[2].v.tc[0] = t2;
    VTX1[3].v.tc[0] = t;

    gDPSetTextureImage(GFX1++, G_IM_FMT_I, G_IM_SIZ_16b , 1, buff);
    gDPSetTile(GFX1++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 6, 0);

    gDPLoadSync(GFX1++);
    gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, (((64 * 64) -1) >> 1), 256);

    gDPPipeSync(GFX1++);
    gDPSetTile(GFX1++, G_IM_FMT_I, G_IM_SIZ_8b, 8, 0, G_TX_RENDERTILE, 0, 0, 0, 0, 0, 6, 0);
    gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0, (63 << 2), (63 << 2));
    gSPTexture(GFX1++, (1024 << 6)-1, (512 << 6), 0, G_TX_RENDERTILE, G_ON);

    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(MTX1), G_MTX_PROJECTION| G_MTX_LOAD | G_MTX_NOPUSH);
    guOrtho(MTX1, -160.f, 160.f, -120.f, 120.f, 0.f, 1.f, 1.f);
    MTX1++;

    gSPVertex(GFX1++, VTX1, 4, 0);
    gSP2Triangles(GFX1++, 0, 2, 1, 0, 0, 3, 2, 0);
    DEBUG_COUNTER(LastVisTriangles += 2);

    VTX1 += 4;

    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(&R_ProjectionMatrix), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
}

void R_CloudThunder(void) // 80026418
{
    int rand;
    if (!(gamepaused))
    {
        ThunderCounter -= vblsinframe[0];

        if(ThunderCounter <= 0)
        {
            if (LightningCounter == 0)
            {
                S_StartSound(NULL, sfx_thndrlow + (M_Random() & 1));
            }
            else if(!(LightningCounter < 6))    // Reset loop after 6 lightning flickers
            {
                rand = (M_Random() & 7);
                ThunderCounter = (((rand << 4) - rand) << 2) + 60;
                LightningCounter = 0;
                return;
            }

            if ((LightningCounter & 1) == 0)
            {
                *(int*)SkyCloudVertex[0].v.cn += PACKRGBA(17,17,17,0); // 0x11111100;
                *(int*)SkyCloudVertex[1].v.cn += PACKRGBA(17,17,17,0); // 0x11111100;
                *(int*)SkyCloudVertex[2].v.cn += PACKRGBA(17,17,17,0); // 0x11111100;
                *(int*)SkyCloudVertex[3].v.cn += PACKRGBA(17,17,17,0); // 0x11111100;
            }
            else
            {
                *(int*)SkyCloudVertex[0].v.cn -= PACKRGBA(17,17,17,0); // 0x11111100;
                *(int*)SkyCloudVertex[1].v.cn -= PACKRGBA(17,17,17,0); // 0x11111100;
                *(int*)SkyCloudVertex[2].v.cn -= PACKRGBA(17,17,17,0); // 0x11111100;
                *(int*)SkyCloudVertex[3].v.cn -= PACKRGBA(17,17,17,0); // 0x11111100;
            }

            ThunderCounter = (M_Random() & 7) + 1; // Do short delay loops for lightning flickers
            LightningCounter += 1;
        }
    }
}

