/* D_screens.c */

#include "config.h"
#include "doomdef.h"
#include "st_main.h"

extern byte        *SkyFireData[2];    // 800A8140 // Fire data double buffer
extern int         FireSide;           // 800A8150
#define FIRESKY_WIDTH   64
#define FIRESKY_HEIGHT  64

void R_SpreadFire(byte *src, int seed);

int D_RunDemo(char *name, customskill_t skill, int map) // 8002B2D0
{
  int lump;
  int exit;

  lump = W_GetNumForName(name);
  demosize = W_LumpLength(lump);
  demo_p = W_GetLump(lump, dec_d64, -1);
  exit = G_PlayDemoPtr(skill, map);
  Z_Free(demo_p);

  return exit;
}

int D_TitleMap(void) // 8002B358
{
  int exit;

  D_OpenControllerPak();

  demosize = 16000;
  demo_p = Z_Alloc(demosize, PU_STATIC, NULL);
  bzero(demo_p,  demosize);
  D_memcpy(demo_p, ControllerPresets[1].ctrl.buttons, sizeof(int)*13);
  exit = G_PlayDemoPtr(SkillPresets[2].skill, 33);
  Z_Free(demo_p);

  return exit;
}

#define G_CC_SPLASHFIRE TEXEL0, 0, ENVIRONMENT, 0, 0, 0, 0, 0

void D_DrawSplashFire(void)
{
    byte *buff;
    int color;

    if (gamevbls < gametic)
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

    color = C_LightGetRGB(((*&vsync - 20) >> 1) & 0xff, 255, 96) << 8;

    gDPSetCycleType(GFX1++, G_CYC_1CYCLE);
    gDPSetTexturePersp(GFX1++, G_TP_NONE);
    gDPSetTextureLUT(GFX1++, G_TT_NONE);
    gDPSetCombineMode(GFX1++, G_CC_SPLASHFIRE, G_CC_SPLASHFIRE);
    gDPSetRenderMode(GFX1++,G_RM_OPA_SURF,G_RM_OPA_SURF2);

    gDPSetTextureImage(GFX1++, G_IM_FMT_I, G_IM_SIZ_16b , 1, buff);
    gDPSetTile(GFX1++, G_IM_FMT_I, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 6, 0);

    gDPLoadSync(GFX1++);
    gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, (((64 * 64) -1) >> 1), 256);

    gDPPipeSync(GFX1++);
    gDPSetTile(GFX1++, G_IM_FMT_I, G_IM_SIZ_8b, 8, 0, G_TX_RENDERTILE, 0, 0, 0, 0, 0, 6, 0);
    gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0, (63 << 2), (63 << 2));
    gSPTexture(GFX1++, (1024 << 6)-1, (512 << 6), 0, G_TX_RENDERTILE, G_ON);

    gDPSetEnvColorD64(GFX1++, C_MultColor(color, text_alpha));
    gSPTextureRectangle(GFX1++, 0, (SCREEN_HT-FIRESKY_HEIGHT+1)<<hudyshift,
                        SCREEN_WD << hudxshift, SCREEN_HT<<hudyshift,
                        G_TX_RENDERTILE,
                        0, 0,
                        (1 << 12 >> hudxshift), (1 << 12 >> hudyshift));
}

void D_DrawUltraTitle(int y)
{
    ST_DrawString(-1,  y, "Doom 64 Ultra", text_alpha | 0xc080c000);
    ST_Message(-1,  y+16, "version " D64ULTRA_VERSION, text_alpha | 0x8080ff00);
}

#if !defined(DEVWARP) && !defined(SKIP_INTRO)
int D_WarningTicker(void) // 8002B3E8
{
    if ((gamevbls < gametic) && !(gametic & 7))
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;
    return 0;
}

void D_DrawWarning(void) // 8002B430
{
    I_ClearFrame();
    I_ClearFB(0x000000ff);

    D_DrawSplashFire();

    if (MenuAnimationTic & 1)
        ST_DrawString(-1,  30, "WARNING!", 0xc00000ff);

    ST_DrawString(-1,  60, "nintendo 64 controller", 0xffffffff);
    ST_DrawString(-1,  80, "is not connected.", 0xffffffff);
    ST_DrawString(-1, 120, "please turn off your", 0xffffffff);
    ST_DrawString(-1, 140, "nintendo 64 system.", 0xffffffff);
    ST_DrawString(-1, 180, "plug in your nintendo 64", 0xffffffff);
    ST_DrawString(-1, 200, "controller and turn it on.", 0xffffffff);

    I_DrawFrame();
}

static bool D_SkipPressed(void)
{
    if ((allticbuttons & PAD_A) && !(alloldticbuttons & PAD_A))
        return true;
    if ((allticbuttons & PAD_B) && !(alloldticbuttons & PAD_B))
        return true;
    return false;
}

int D_UltraTicker(void)
{
    if ((ticon - last_ticon) >= 16*TICRATE)
        return ga_exit;
    if ((ticon - last_ticon) >= 15 && D_SkipPressed())
        return ga_exit;
    return ga_nothing;
}

#if REGION == REGION_JP
#define SPLASH_TITLE_POS 20
#define SPLASH_TEXT_POS  72
#else
#define SPLASH_TITLE_POS 32
#define SPLASH_TEXT_POS  76
#endif

void D_DrawUltra(void)
{
    static const char *DISCLAIMER[] = {
        "doom 64 ultra is a free and",
        "open source fan-made modification",
        "created for educational purposes.",
        NULL,
        "it is not intended to be bought",
        "or sold for profit.",
        NULL,
        "visit the home page for more",
        "information and updates:",
        "https://d64u.github.io",
    };

    I_ClearFrame();

    I_ClearFB(0x000000ff);

    D_DrawSplashFire();
    D_DrawUltraTitle(SPLASH_TITLE_POS);

    for (int i = 0; i < ARRAYLEN(DISCLAIMER); i++)
        if (DISCLAIMER[i])
            ST_Message(-1, SPLASH_TEXT_POS + (i*10), DISCLAIMER[i], 0xffffffff);

    if (FilesUsed > -1) {
        ST_DrawString(-1, SCREEN_HT-40, "hold \x8d to manage pak", text_alpha | 0xffffff00);
    }

    I_DrawFrame();
}

int D_LegalTicker(void) // 8002B5F8
{
    if (D_SkipPressed())
        last_ticon = -(5*TICRATE);
    if ((ticon - last_ticon) >= (5*TICRATE))
    {
        text_alpha -= 8;
        if (text_alpha < 0)
        {
            text_alpha = 0;
            return ga_exit;
        }
    }
    return ga_nothing;
}

void D_DrawLegal(void) // 8002B644
{
    I_ClearFrame();

    I_ClearFB(0x000000ff);

    D_DrawSplashFire();
    D_DrawUltraTitle(SPLASH_TITLE_POS);

#if REGION == REGION_JP
    M_DrawBackground(35, 50, text_alpha, "JPLEGAL");
#elif REGION == REGION_EU
    M_DrawBackground(35, 66, text_alpha, "PLLEGAL");
#else
    M_DrawBackground(27, 74, text_alpha, "USLEGAL");
#endif

    if (FilesUsed > -1) {
        ST_DrawString(-1, SCREEN_HT-40, "hold \x8d to manage pak", text_alpha | 0xffffff00);
    }

    I_DrawFrame();
}

int D_NoPakTicker(void) // 8002B7A0
{
    if ((ticon - last_ticon) >= 15 && D_SkipPressed())
        return ga_exit;
    if ((ticon - last_ticon) >= (8*TICRATE))
        return ga_exit;

    return ga_nothing;
}

void D_DrawNoPak(void) // 8002B7F4
{
    I_ClearFrame();
    I_ClearFB(0x000000ff);

    D_DrawSplashFire();

#if REGION == REGION_JP
    M_DrawBackground(22, 88, 0xff, "JPCPAK");
#else
    ST_DrawString(-1,  40, "no controller pak.", 0xffffffff);
    ST_DrawString(-1,  60, "your game cannot", 0xffffffff);
    ST_DrawString(-1,  80, "be saved.", 0xffffffff);
    ST_DrawString(-1, 120, "please turn off your", 0xffffffff);
    ST_DrawString(-1, 140, "nintendo 64 system", 0xffffffff);
    ST_DrawString(-1, 160, "before inserting a", 0xffffffff);
    ST_DrawString(-1, 180, "controller pak.", 0xffffffff);
#endif

    I_DrawFrame();
}

#ifdef REQUIRE_EXPANSION_PAK
void D_DrawNoMemory(void)
{
    I_ClearFrame();
    I_ClearFB(0x000000ff);

    D_DrawSplashFire();

    ST_DrawString(-1,  20, "no expansion pak.", 0xffffffff);
    ST_DrawString(-1,  40, "complex levels outside", 0xffffffff);
    ST_DrawString(-1,  60, "the original campaign", 0xffffffff);
    ST_DrawString(-1,  80, "may not be rendered", 0xffffffff);
    ST_DrawString(-1, 100, "properly or crash.", 0xffffffff);
    ST_DrawString(-1, 140, "please turn off your", 0xffffffff);
    ST_DrawString(-1, 160, "nintendo 64 system", 0xffffffff);
    ST_DrawString(-1, 180, "before inserting an", 0xffffffff);
    ST_DrawString(-1, 200, "expansion pak.", 0xffffffff);

    I_DrawFrame();
}
#endif

void D_SplashScreen(void) // 8002B988
{
    SkyFireData[0] = Z_Malloc(FIRESKY_WIDTH*FIRESKY_HEIGHT*2, PU_STATIC, NULL);
    SkyFireData[1] = &SkyFireData[0][FIRESKY_WIDTH*FIRESKY_HEIGHT];
    bzero(SkyFireData[0], FIRESKY_WIDTH*FIRESKY_HEIGHT*2);
    D_memset(SkyFireData[0]+(FIRESKY_WIDTH-1)*FIRESKY_HEIGHT, 0xf0, FIRESKY_WIDTH);
    D_memset(SkyFireData[1]+(FIRESKY_WIDTH-1)*FIRESKY_HEIGHT, 0xf0, FIRESKY_WIDTH);

    /* */
    /* Check if the n64 control is connected */
    /* if not connected, it will show the Warning screen */
    /* */
    if (((*&gamepad_bit_pattern) & 1) == 0) {
        MiniLoop(NULL, NULL,D_WarningTicker,D_DrawWarning);
    }

    /* */
    /* Check if the n64 controller Pak is connected */
    /* */
    I_CheckControllerPak();

    /* */
    /* if not connected, it will show the NoPak screen */
    /* */
    if (!SramPresent && FilesUsed < 0) {
        last_ticon = 0;
        MiniLoop(NULL, NULL, D_NoPakTicker, D_DrawNoPak);
    }

#ifdef REQUIRE_EXPANSION_PAK
    /* */
    /* Check if expansion pak is connected if >8MB memory */
    /* */
    if (!HAS_EXPANSION_PAK())
    {
        MiniLoop(NULL, NULL, D_NoPakTicker, D_DrawNoMemory);
    }
#endif

    text_alpha = 0xff;
    last_ticon = 0;
    MiniLoop(NULL, NULL, D_UltraTicker, D_DrawUltra);

    /* */
    /* show the legals screen */
    /* */

    last_ticon = 0;
    MiniLoop(NULL, NULL, D_LegalTicker, D_DrawLegal);

    Z_Free(SkyFireData[0]);
}
#endif

static int cred_step;   // 800B2210
static int cred1_alpha; // 800B2214
static int cred2_alpha; // 800B2218
static int cred_next;   // 800B2218

int D_Credits(void) // 8002BA34
{
    int exit;

    cred_next = 0;
    cred1_alpha = 0;
    cred2_alpha = 0;
    cred_step = 0;
    exit = MiniLoop(NULL, NULL, D_CreditTicker, D_CreditDrawer);

    return exit;
}

int D_CreditTicker(void) // 8002BA88
{
    if (allticbuttons & 0xffff0000)
        return ga_exit;

    if (cred_next < 4)
    {
        if (cred_step == 0)
        {
            cred1_alpha += 8;
            if (cred1_alpha >= 255)
            {
                cred1_alpha = 0xff;
                cred_step = 1;
            }
        }
        else if (cred_step == 1)
        {
            cred2_alpha += 8;
            if (cred2_alpha >= 255)
            {
                cred2_alpha = 0xff;
                last_ticon = ticon;
                cred_step = 2;
            }
        }
        else if (cred_step == 2)
        {
            if ((ticon - last_ticon) >= (6*TICRATE))
                cred_step = 3;
        }
        else
        {
            cred1_alpha -= 8;
            cred2_alpha -= 8;
            if (cred1_alpha < 0)
            {
                cred_next += 1;
                cred1_alpha = 0;
                cred2_alpha = 0;
                cred_step = 0;
            }
        }
    }
    else if (cred_next == 4)
        return ga_exitdemo;

    return ga_nothing;
}

void D_CreditDrawer(void) // 8002BBE4
{
    int color = 0;

    I_ClearFrame();

    if (cred_next == 2)
        color = (cred1_alpha * 16) / 255;
    else if ((cred_next == 3) || (cred_next == 4))
        color = (cred1_alpha * 30) / 255;

    I_ClearFB(PACKRGBA(color, 0, 0, 255));

    if (cred_next == 0)
    {
        text_alpha = cred1_alpha;
        M_ModCredits1Drawer();
    }
    else if (cred_next == 1)
    {
        text_alpha = cred1_alpha;
        M_ModCredits2Drawer();
    }
    else if (cred_next == 2)
    {
        M_DrawBackground(68, 21, cred1_alpha, "IDCRED1");
        M_DrawBackground(32, 41, cred2_alpha, "IDCRED2");
    }
    else if ((cred_next == 3) || (cred_next == 4))
    {
        M_DrawBackground(22, 82, cred1_alpha, "WMSCRED1");
        M_DrawBackground(29, 28, cred2_alpha, "WMSCRED2");
    }

    I_DrawFrame();
}

void D_OpenControllerPak(void) // 8002BE28
{
    unsigned int oldbuttons;

    oldbuttons = I_GetControllerData(0);

    if (((oldbuttons & 0xffff0000) == PAD_START) && (I_CheckControllerPak() == 0))
    {
        MenuCall = M_ManagePakDrawer;
        linepos = 0;
        cursorpos = 0;

        MiniLoop(M_FadeInStart, M_MenuClearCall, M_ManagePakTicker, M_MenuGameDrawer);
        I_WIPE_FadeOutScreen();
    }
    return;
}
