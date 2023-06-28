
//Renderer phase 2 - Sky Rendering Routines

#include "doomdef.h"
#include "r_local.h"

#define FIRESKY_WIDTH   64
#define FIRESKY_HEIGHT  64

#define VINVFOV 0x9c399       /* ANGMAX/atan(3/4) */

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
	{ .v = { { -160, 120, -160}, 0, {(0 << 6), (0  << 6)}, {0, 0, 0, 0xff} } },
	{ .v = { {  160, 120, -160}, 0, {(0 << 6), (0  << 6)}, {0, 0, 0, 0xff} } },
	{ .v = { {  160,   0, -160}, 0, {(0 << 6), (64 << 6)}, {0, 0, 0, 0xff} } },
	{ .v = { { -160,   0, -160}, 0, {(0 << 6), (64 << 6)}, {0, 0, 0, 0xff} } },
};

fixed_t     FogNear;            // 800A8120
int         FogColor;           // 800A8124
skyfunc_t   R_RenderSKY;        // 800A8130
byte        *SkyFireData[2];    // 800A8140 // Fire data double buffer
byte        *SkyCloudData;      // 800A8148
int         Skyfadeback;        // 800A814C
int         FireSide;           // 800A8150
int         SkyCloudOffsetX;    // 800A8154
int         SkyCloudOffsetY;    // 800A8158
int         ThunderCounter;     // 800A815C
int         LightningCounter;   // 800A8160
int         SkyPicSpace;        // 800A8164
int         SkyPicMount;        // 800A8168
int         SkyCloudColor;      // 800A816C
int         SkyVoidColor;       // 800A8170
int         SkyFlags;           // 800A8174

void R_RenderSpaceSky(void);
void R_RenderCloudSky(void);
void R_RenderVoidSky(void);
void R_RenderEvilSky(void);
void R_RenderClouds(void);
void R_RenderSkyPic(int lump, int yoffset, boolean repeat);
void R_RenderFireSky(void);
void R_CloudThunder(void);

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

            SkyCloudData = (byte *)W_CacheLumpName("CLOUD", PU_STATIC, dec_jag);
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

            SkyCloudData = (byte *)W_CacheLumpName("CLOUD", PU_STATIC, dec_jag);
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

            SkyCloudData = (byte *)W_CacheLumpName("CLOUD", PU_STATIC, dec_jag);
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

            data = W_CacheLumpName("FIRE", PU_LEVEL, dec_jag);
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

void R_RenderSpaceSky(void) // 80025440
{
    gDPSetAlphaCompare(GFX1++, G_AC_NONE);
    gDPSetCombineMode(GFX1++, G_CC_D64COMB09, G_CC_D64COMB09);
    gDPSetRenderMode(GFX1++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gDPSetPrimColor(GFX1++, 0, (lights[255].rgba >> 8), 0, 0, 0, 255);

    R_RenderSkyPic(SkyPicSpace, 128, true);

    if (SkyFlags & SKF_MOUNTAIN)
    {
        gDPPipeSync(GFX1++);
        gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);
        gDPSetCombineMode(GFX1++, G_CC_D64COMB10, G_CC_D64COMB10);
        gDPSetRenderMode(GFX1++, G_RM_TEX_EDGE, G_RM_TEX_EDGE2);

        R_RenderSkyPic(SkyPicMount, 170, false);
    }
}

void R_RenderCloudSky(void) // 800255B8
{
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

        R_RenderSkyPic(SkyPicMount, 170, false);
    }
}

#define RGBATO551(c) (((((u32)(c))&0xf8000000)>>16) | (((c)&0xf80000)>>13) | (((c)&0xf800)>>10) | (((c)&0xff) > 0))

static u32 R_AddColors(u32 c1, u32 c2)
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

static u32 R_MultColor(u32 c, u8 fac)
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

void R_RenderVoidSky(void) // 800256B4
{
    int color = SkyVoidColor;

    color = R_MultColor(color, lights[255].rgba >> 8);
    color = R_AddColors(color, FlashEnvColor & 0xffffff00);
    color = RGBATO551(color);
    gDPSetCycleType(GFX1++, G_CYC_FILL);
    gDPSetRenderMode(GFX1++,G_RM_NOOP,G_RM_NOOP2);

    // Fill borders with SkyVoidColor
    gDPSetFillColor(GFX1++, (color << 16) | color);
    gDPFillRectangle(GFX1++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);

    gDPPipeSync(GFX1++);
    gDPSetCycleType(GFX1++, G_CYC_2CYCLE);
}

void R_RenderEvilSky(void) // 80025738
{
    int color;

    gDPSetPrimColor(GFX1++, 0, ((lights[255].rgba >> 8)  - Skyfadeback), 0, 0, 0, 255);
    gDPSetAlphaCompare(GFX1++, G_AC_NONE);
    gDPSetCombineMode(GFX1++, G_CC_D64COMB09, G_CC_D64COMB09);
    gDPSetRenderMode(GFX1++,G_RM_OPA_SURF,G_RM_OPA_SURF2);

    R_RenderSkyPic(SkyPicSpace, 128, true);

    if (Skyfadeback)
    {
        Skyfadeback += 4;

        if (Skyfadeback > 255)
            Skyfadeback = 255;

        if (Skyfadeback > 128)
            color = 128;
        else
            color = Skyfadeback;

        M_DrawBackground(63, 25, color, "EVIL");
    }
}

extern Mtx R_ModelMatrix;

void R_RenderClouds(void) // 80025878
{
    int x, y;
    int pitchoffset;

    pitchoffset = FixedMul(((int)viewpitch) >> 16, VINVFOV * (SCREEN_HT/2)) >> 16;
    if (pitchoffset < 136)
    {
        int yoff = 100+pitchoffset;
        int color = FlashEnvColor;

        if (yoff < 0)
            yoff = 0;
        gDPSetCycleType(GFX1++, G_CYC_FILL);
        gDPSetRenderMode(GFX1++,G_RM_NOOP,G_RM_NOOP2);
        color = RGBATO551(color);
        gDPSetFillColor(GFX1++, (color << 16) | color);
        gDPFillRectangle(GFX1++, 0, yoff, SCREEN_WD-1, SCREEN_HT-1);
    }

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
    gSP1Triangle(GFX1++, 0, 2, 1, 0);
    gSP1Triangle(GFX1++, 0, 3, 2, 0);

    VTX1 += 4;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(&R_ModelMatrix), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
#pragma GCC diagnostic pop
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
    int pitchoffset;
    int spriteh;

    data = W_CacheLumpNum(lump, PU_CACHE, dec_jag);

    ang = ((0 - (viewangle >> 22)) & 255);
    tileh = ((spriteN64_t*)data)->tileheight;

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
    pitchoffset = FixedMul(((int)viewpitch) >> 16, VINVFOV * (SCREEN_HT/2)) >> 14;
    lrs = (((tileh << 8) + 1) >> 1) - 1;
    yl = (yoffset << 2) - spriteh;
    yl += pitchoffset;
    if (repeat)
        while (yl > 0)
            yl -= spriteh;

    for(i = 0; i < ((spriteN64_t*)data)->tiles; i++)
    {
        nextyh = yh + (tileh << 2);

        if (!repeat && yl + nextyh <= 0)
            continue;

        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 8, 0);

        gDPLoadSync(GFX1++);
        gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, lrs, 0);

        gDPPipeSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b, 32, 0, G_TX_RENDERTILE, 0, 0, 0, 0, 0, 8, 0);
        gDPSetTileSize(GFX1++, 0, 0, 0, ((256-1) << 2), ((tileh - 1) << 2));

        for (int tyl = yl; tyl < (SCREEN_HT<<2); tyl += spriteh)
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

                gSPTextureRectangle(GFX1++, (0 << 2), uy,
                                            (320 << 2), ly,
                                            G_TX_RENDERTILE,
                                            (ang << 5), t,
                                            (1 << 10), (1 << 10));
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

void R_RenderFireSky(void) // 80025F68
{
    byte *buff;
    byte *src, *srcoffset, *tmpSrc;
    int width, height, rand;
    int pixel, randIdx;
    int ang, t;
    int pitchoffset;
    int color;

    pitchoffset = FixedMul(((int)viewpitch) >> 16, VINVFOV * (SCREEN_HT/2)) >> 16;

    gDPSetCycleType(GFX1++, G_CYC_FILL);
    gDPSetRenderMode(GFX1++,G_RM_NOOP,G_RM_NOOP2);

    if (pitchoffset > 0)
    {
        color = FlashEnvColor;
        color = RGBATO551(color);
        gDPSetFillColor(GFX1++, (color << 16) | color);
        gDPFillRectangle(GFX1++, 0, 0, SCREEN_WD-1, pitchoffset-1);
    }
    if (pitchoffset < 120)
    {
        color = *(int*)SkyFireVertex[2].v.cn;
        color = R_MultColor(color, lights[255].rgba >> 8);
        color = R_AddColors(color, FlashEnvColor & 0xffffff00);
        color = RGBATO551(color);
        gDPSetFillColor(GFX1++, (color << 16) | color);
        gDPFillRectangle(GFX1++, 0, pitchoffset + 120, SCREEN_WD-1, SCREEN_HT-1);

    }

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

        rand = (M_Random() & 0xff);
        width = 0;
        src = (buff + FIRESKY_WIDTH);

        do // width
        {
            height = 2;
            srcoffset = (src + width);

            // R_SpreadFire
            pixel = *(byte*)srcoffset;
            if (pixel != 0)
            {
                randIdx = rndtable[rand];
                rand = ((rand + 2) & 0xff);

                tmpSrc = (src + (((width - (randIdx & 3)) + 1) & (FIRESKY_WIDTH-1)));
                *(byte*)(tmpSrc - FIRESKY_WIDTH) = pixel - ((randIdx & 1) << 4);
            }
            else
            {
                *(byte*)(srcoffset - FIRESKY_WIDTH) = 0;
            }

            src += FIRESKY_WIDTH;
            srcoffset += FIRESKY_WIDTH;

            do // height
            {
                height += 2;

                // R_SpreadFire
                pixel = *(byte*)srcoffset;
                if (pixel != 0)
                {
                    randIdx = rndtable[rand];
                    rand = ((rand + 2) & 0xff);

                    tmpSrc = (src + (((width - (randIdx & 3)) + 1) & (FIRESKY_WIDTH-1)));
                    *(byte*)(tmpSrc - FIRESKY_WIDTH) = pixel - ((randIdx & 1) << 4);
                }
                else
                {
                    *(byte*)(srcoffset - FIRESKY_WIDTH) = 0;
                }

                src += FIRESKY_WIDTH;
                srcoffset += FIRESKY_WIDTH;

                // R_SpreadFire
                pixel = *(byte*)srcoffset;
                if (pixel != 0)
                {
                    randIdx = rndtable[rand];
                    rand = ((rand + 2) & 0xff);

                    tmpSrc = (src + (((width - (randIdx & 3)) + 1) & (FIRESKY_WIDTH-1)));
                    *(byte*)(tmpSrc - FIRESKY_WIDTH) = pixel - ((randIdx & 1) << 4);
                }
                else
                {
                    *(byte*)(srcoffset - FIRESKY_WIDTH) = 0;
                }

                src += FIRESKY_WIDTH;
                srcoffset += FIRESKY_WIDTH;

            } while (height != FIRESKY_HEIGHT);

            src -= ((FIRESKY_WIDTH*FIRESKY_HEIGHT) - FIRESKY_WIDTH);
            width++;

        } while (width != FIRESKY_WIDTH);

        FireSide ^= 1;
    }
    else
    {
        buff = SkyFireData[FireSide ^ 1];
    }

    D_memcpy(VTX1, SkyFireVertex, sizeof(Vtx)*4);

    ang = (viewangle >> 22);
    t = ((-ang & 255) << 5);

    for (int i = 0; i < 4; i++)
        VTX1[i].v.ob[1] -= pitchoffset;

    VTX1[0].v.tc[0] = t;
    VTX1[1].v.tc[0] = t + 0x2800;
    VTX1[2].v.tc[0] = t + 0x2800;
    VTX1[3].v.tc[0] = t;

    gDPSetTextureImage(GFX1++, G_IM_FMT_I, G_IM_SIZ_16b , 1, buff);
    gDPSetTile(GFX1++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 6, 0);

    gDPLoadSync(GFX1++);
    gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, (((64 * 64) -1) >> 1), 256);

    gDPPipeSync(GFX1++);
    gDPSetTile(GFX1++, G_IM_FMT_I, G_IM_SIZ_8b, 8, 0, G_TX_RENDERTILE, 0, 0, 0, 0, 0, 6, 0);
    gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0, (63 << 2), (63 << 2));
    gSPTexture(GFX1++, (1024 << 6)-1, (512 << 6), 0, G_TX_RENDERTILE, G_ON);

    gSPVertex(GFX1++, VTX1, 4, 0);
    gSP1Triangle(GFX1++, 0, 2, 1, 0);
    gSP1Triangle(GFX1++, 0, 3, 2, 0);

    VTX1 += 4;
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

