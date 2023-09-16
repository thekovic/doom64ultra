/* st_main.c -- status bar */

#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"
#include "p_spec.h"
#include "r_local.h"

extern void P_RefreshBrightness(void);

static sbflash_t flashCards[6];    // 800A8180
boolean tryopen[6]; // 800A81E0

byte *sfontlump;     // 800A81F8
byte *statuslump;   // 800A81FC
int sumbolslump;    // 800A8204

typedef struct
{
    int      x;
    int      y;
    int      w;
    int      h;
} symboldata_t;

static const symboldata_t symboldata[] = // 8005B260
{
    {120, 14,  13, 13}, // 0
    {134, 14,   9, 13}, // 1
    {144, 14,  14, 13}, // 2
    {159, 14,  14, 13}, // 3
    {174, 14,  16, 13}, // 4
    {191, 14,  13, 13}, // 5
    {205, 14,  13, 13}, // 6
    {219, 14,  14, 13}, // 7
    {234, 14,  14, 13}, // 8
    {  0, 29,  13, 13}, // 9
    { 67, 28,  14, 13}, // -
    { 36, 28,  15, 14}, // %
    { 28, 28,   7, 14}, // !
    { 14, 29,   6, 13}, // .
    { 52, 28,  13, 13}, // ?
    { 21, 29,   6, 13}, // :
    {  0,  0,  13, 13}, // A
    { 14,  0,  13, 13}, // B
    { 28,  0,  13, 13}, // C
    { 42,  0,  14, 13}, // D
    { 57,  0,  14, 13}, // E
    { 72,  0,  14, 13}, // F
    { 87,  0,  15, 13}, // G
    {103,  0,  15, 13}, // H
    {119,  0,   6, 13}, // I
    {126,  0,  13, 13}, // J
    {140,  0,  14, 13}, // K
    {155,  0,  11, 13}, // L
    {167,  0,  15, 13}, // M
    {183,  0,  16, 13}, // N
    {200,  0,  15, 13}, // O
    {216,  0,  13, 13}, // P
    {230,  0,  15, 13}, // Q
    {246,  0,  13, 13}, // R
    {  0, 14,  14, 13}, // S
    { 15, 14,  14, 13}, // T
    { 30, 14,  13, 13}, // U
    { 44, 14,  15, 13}, // V
    { 60, 14,  15, 13}, // W
    { 76, 14,  15, 13}, // X
    { 92, 14,  13, 13}, // Y
    {106, 14,  13, 13}, // Z
    { 83, 31,  10, 11}, // a
    { 93, 31,  10, 11}, // b
    {103, 31,  11, 11}, // c
    {114, 31,  11, 11}, // d
    {125, 31,  11, 11}, // e
    {136, 31,  11, 11}, // f
    {147, 31,  12, 11}, // g
    {159, 31,  12, 11}, // h
    {171, 31,   4, 11}, // i
    {175, 31,  10, 11}, // j
    {185, 31,  11, 11}, // k
    {196, 31,   9, 11}, // l
    {205, 31,  12, 11}, // m
    {217, 31,  13, 11}, // n
    {230, 31,  12, 11}, // o
    {242, 31,  11, 11}, // p
    {  0, 43,  12, 11}, // q
    { 12, 43,  11, 11}, // r
    { 23, 43,  11, 11}, // s
    { 34, 43,  10, 11}, // t
    { 44, 43,  11, 11}, // u
    { 55, 43,  12, 11}, // v
    { 67, 43,  13, 11}, // w
    { 80, 43,  13, 11}, // x
    { 93, 43,  10, 11}, // y
    {103, 43,  11, 11}, // z
    {  0, 95, 108, 11}, // Slider bar
    {108, 95,   6, 11}, // Slider gem
    {  0, 54,  32, 26}, // Skull 1
    { 32, 54,  32, 26}, // Skull 2
    { 64, 54,  32, 26}, // Skull 3
    { 96, 54,  32, 26}, // Skull 4
    {128, 54,  32, 26}, // Skull 5
    {160, 54,  32, 26}, // Skull 6
    {192, 54,  32, 26}, // Skull 7
    {224, 54,  32, 26}, // Skull 8
    {134, 97,   7, 11}, // Right arrow
    {114, 95,  20, 18}, // Select box
    {105, 80,  15, 15}, // Dpad left
    {120, 80,  15, 15}, // Dpad right
    {135, 80,  15, 15}, // Dpad up
    {150, 80,  15, 15}, // Dpad down
    { 45, 80,  15, 15}, // C left button
    { 60, 80,  15, 15}, // C right button
    { 75, 80,  15, 15}, // C up button
    { 90, 80,  15, 15}, // C down button
    {165, 80,  15, 15}, // L button
    {180, 80,  15, 15}, // R button
    {  0, 80,  15, 15}, // A button
    { 15, 80,  15, 15}, // B btton
    {195, 80,  15, 15}, // Z button
    { 30, 80,  15, 15}, // Start button
    {156, 96,  13, 13}, // Down arrow
    {143, 96,  13, 13}, // Up arrow
    {169, 96,   7, 13}, // Left arrow
    {134, 96,   7, 13}, // Right arrow
};

static int card_x[6] = {78, 89, 100, 78, 89, 100};      // 8005b870

void ST_Init(void) // 80029BA0
{
  sfontlump = (byte *)W_CacheLumpName("SFONT",PU_STATIC,dec_jag);
  statuslump = (byte *)W_CacheLumpName("STATUS",PU_STATIC,dec_jag);
  sumbolslump = W_GetNumForName("SYMBOLS");
}

void ST_InitEveryLevel(void) // 80029C00
{
    infraredFactor = 0;
    quakeviewy = 0;
    quakeviewx = 0;
    camviewpitch = 0;
    flashCards[0].active = false;
    flashCards[1].active = false;
    flashCards[2].active = false;
    flashCards[3].active = false;
    flashCards[4].active = false;
    flashCards[5].active = false;
    tryopen[0] = false;
    tryopen[1] = false;
    tryopen[2] = false;
    tryopen[3] = false;
    tryopen[4] = false;
    tryopen[5] = false;

}

/*
====================
=
= ST_Ticker
=
====================
*/

void ST_Ticker (void) // 80029C88
{
	player_t    *player;
	int		    ind;

	player = &players[0];

    /* */
	/* Countdown time for the message */
	/* */
    player->messagetic--;
    player->messagetic1--; // [Immorpher] decriment message buffer
    player->messagetic2--; // [Immorpher] decriment message buffer
    player->messagetic3--; // [Immorpher] decriment message buffer

	/* */
	/* Tried to open a CARD or SKULL door? */
	/* */
	for (ind = 0; ind < NUMCARDS; ind++)
	{
		/* CHECK FOR INITIALIZATION */
		if (tryopen[ind])
		{
			tryopen[ind] = false;
			flashCards[ind].active = true;
			flashCards[ind].delay = FLASHDELAY-1;
			flashCards[ind].times = FLASHTIMES+1;
			flashCards[ind].doDraw = false;
		}
		/* MIGHT AS WELL DO TICKING IN THE SAME LOOP! */
		else if (flashCards[ind].active && !--flashCards[ind].delay)
		{
			flashCards[ind].delay = FLASHDELAY-1;
			flashCards[ind].doDraw ^= 1;
			if (!--flashCards[ind].times)
				flashCards[ind].active = false;
			if (flashCards[ind].doDraw && flashCards[ind].active)
				S_StartSound(NULL,sfx_itemup);
		}
	}

	/* */
	/* Do flashes from damage/items */
	/* */
	if (cameratarget == player->mo)
    {
        ST_UpdateFlash(); // ST_doPaletteStuff();
	}
}


/*
====================
=
= ST_Drawer
=
====================
*/

#define DEBUGLINES 16
#define DEBUGLINELEN (SCREEN_WD/8+1)
static char *debugbuf = NULL;
static int debugX = 8, debugY = 8;//80077E5C|uGp00000a4c, 80077E68|uGp00000a58
static int debugcnt = 0;
static int debugstart = 0;
static int debug = 0;

extern memzone_t	*mainzone;

void ST_DrawDebug (void)
{
    char buf[20];

    if(debug)
    {
        for(int i = 0; i < debugcnt; i++)
        {
            int index = (i+debugstart)%DEBUGLINES;
            ST_Message(debugX, (i*8) + debugY, &debugbuf[index*DEBUGLINELEN],0x00ff00a0);
        }
    }
    switch (ShowDebugCounters)
    {
    case 1:
        sprintf(buf, "FPS %d", LastFrameCycles ? fround(46875000.0 / (f64) LastFrameCycles) : 0);
        ST_Message(16, SCREEN_HT-56, buf, 0x00ff00a0);
        break;
    case 2:
        // total time taken to complete tick/drawing
        sprintf(buf, "CPU %lu", (u32)OS_CYCLES_TO_USEC(LastCpuCycles));
        ST_Message(16, SCREEN_HT-88, buf, 0x00ff00a0);
        // time for GFX RSP
        sprintf(buf, "RSP %lu", (u32)OS_CYCLES_TO_USEC(LastGfxRspCycles));
        ST_Message(16, SCREEN_HT-80, buf, 0x00ff00a0);
        // time for RSP + RDP
        sprintf(buf, "RDP %lu", (u32)OS_CYCLES_TO_USEC(LastRdpCycles));
        ST_Message(16, SCREEN_HT-72, buf, 0x00ff00a0);
        // time for Audio RSP
        sprintf(buf, "ASP %lu", (u32)OS_CYCLES_TO_USEC(LastAudioRspCycles));
        ST_Message(16, SCREEN_HT-64, buf, 0x00ff00a0);
        // total time taken to complete tick/drawing/RCP
        sprintf(buf, "FRM %lu", (u32)OS_CYCLES_TO_USEC(LastFrameCycles));
        ST_Message(16, SCREEN_HT-56, buf, 0x00ff00a0);
        break;
#ifndef NDEBUG
    case 3:
        // time to tick all mobjs and thinkers
        sprintf(buf, "WRL %lu", (u32)OS_CYCLES_TO_USEC(LastWorldCycles));
        ST_Message(16, SCREEN_HT-96, buf, 0x00ff00a0);
        // time to run audio on CPU
        sprintf(buf, "AUD %lu", (u32)OS_CYCLES_TO_USEC(LastAudioCycles));
        ST_Message(16, SCREEN_HT-88, buf, 0x00ff00a0);
        // time to traverse BSP
        sprintf(buf, "BSP %lu", (u32)OS_CYCLES_TO_USEC(LastBspCycles));
        ST_Message(16, SCREEN_HT-80, buf, 0x00ff00a0);
        // time to build RSP command lists
        sprintf(buf, "RND %lu", (u32)OS_CYCLES_TO_USEC(LastPhase3Cycles));
        ST_Message(16, SCREEN_HT-72, buf, 0x00ff00a0);
        // total time taken to complete tick/drawing
        sprintf(buf, "CPU %lu", (u32)OS_CYCLES_TO_USEC(LastCpuCycles));
        ST_Message(16, SCREEN_HT-64, buf, 0x00ff00a0);
        // total time taken to complete tick/drawing/RCP
        sprintf(buf, "FRM %lu", (u32)OS_CYCLES_TO_USEC(LastFrameCycles));
        ST_Message(16, SCREEN_HT-56, buf, 0x00ff00a0);
        break;
    case 4:
        // number of rendered RSP commands
        sprintf(buf, "GFX %lu", ((int)((int)GFX1 - (int)GFX2) / sizeof(Gfx)) + GfxIndex);
        ST_Message(16, SCREEN_HT-72, buf, 0x00ff00a0);
        // number of rendered vertexes
        sprintf(buf, "VTX %lu", ((int)((int)VTX1 - (int)VTX2) / sizeof(Vtx)) + VtxIndex);
        ST_Message(16, SCREEN_HT-64, buf, 0x00ff00a0);
        // number of rendered triangles
        sprintf(buf, "TRI %lu", LastVisTriangles);
        ST_Message(16, SCREEN_HT-56, buf, 0x00ff00a0);
        break;
    case 5:
        // number of rendered subsectors
        sprintf(buf, "SUBS %lu", LastVisSubsectors);
        ST_Message(16, SCREEN_HT-80, buf, 0x00ff00a0);
        // number of rendered leafs
        sprintf(buf, "LEAF %lu", LastVisLeaves);
        ST_Message(16, SCREEN_HT-72, buf, 0x00ff00a0);
        // number of rendered segs
        sprintf(buf, "SEGS %lu", LastVisSegs);
        ST_Message(16, SCREEN_HT-64, buf, 0x00ff00a0);
        // number of rendered things
        sprintf(buf, "THNG %lu", LastVisThings);
        ST_Message(16, SCREEN_HT-56, buf, 0x00ff00a0);
        break;
    case 6:
        // macro id [ index in macro ]
        sprintf(buf, "MACRO ");
        if (activemacroidx >= 0 && activemacro)
            sprintf(&buf[6], "%d[%d]", activemacroidx, activemacro - macros[activemacroidx]);
        else
            sprintf(&buf[6], "-");
        ST_Message(16, SCREEN_HT-72, buf, 0x00ff00a0);
        // current number of thinkers
        sprintf(buf, "THINK %d", activethinkers);
        ST_Message(16, SCREEN_HT-64, buf, 0x00ff00a0);
        // current number of mobjs
        sprintf(buf, "MOBJS %d", activemobjs);
        ST_Message(16, SCREEN_HT-56, buf, 0x00ff00a0);
        break;
    case 7:
        // memory used by PU_LEVEL and PU_LEVSPEC
        sprintf(buf, "LEV %lu", LevelMem);
        ST_Message(16, SCREEN_HT-72, buf, 0x00ff00a0);
        // non-cached memory usage
        sprintf(buf, "USE %7lu : %lu", UsedMem, mainzone->size - UsedMem);
        ST_Message(16, SCREEN_HT-64, buf, 0x00ff00a0);
        // all memory usage incl. PU_CACHE
        sprintf(buf, "OCC %7lu : %lu", OccupiedMem, mainzone->size - OccupiedMem);
        ST_Message(16, SCREEN_HT-56, buf, 0x00ff00a0);
        break;
#endif
    default:
        break;
    }
}

void ST_Drawer (void) // 80029DC0
{
    byte        *src;
    player_t    *player;
    int ind, ms_alpha;

    player = &players[0];

    /* */
    /* Draw Text Message */
    /* */
    if ((enable_messages) && player->messagetic > 0) // [Immorpher] only display messages and calculate if global tic is active
    {
        if (player->messagetic != player->messagetic1) // [Immorpher] new global tic indicates new message to add
        {	// Sequentially shift messages to lower states
            player->message3 = player->message2;
            player->messagetic3 = player->messagetic2;
            player->messagecolor3 = player->messagecolor2;

            player->message2 = player->message1;
            player->messagetic2 = player->messagetic1;
            player->messagecolor2 = player->messagecolor1;

            player->message1 = player->message;
            player->messagetic1 = player->messagetic;
            player->messagecolor1 = player->messagecolor;
        }

        if (player->messagetic1 > 0) // display message 1
        {
            ms_alpha = player->messagetic1 << 3; // set message alpha
            if (ms_alpha >= 196)
                ms_alpha = 196;

            ST_Message(2+HUDmargin, HUDmargin, player->message1, ms_alpha | player->messagecolor1); // display message
        }

        if (player->messagetic2 > 0) // display message 2
        {
            ms_alpha = player->messagetic2 << 3; // set message alpha
            if (ms_alpha >= 196)
                ms_alpha = 196;

            ST_Message(2+HUDmargin, 10+HUDmargin, player->message2, ms_alpha | player->messagecolor2); // display message
        }

        if (player->messagetic3 > 0) // display message 3
        {
            ms_alpha = player->messagetic3 << 3; // set message alpha
            if (ms_alpha >= 196)
                ms_alpha = 196;

            ST_Message(2+HUDmargin, 20+HUDmargin, player->message3, ms_alpha | player->messagecolor3); // display message
        }
    }

    if (HUDopacity){
        int crosshair, color, stat;
        weapontype_t weapon;
        ammotype_t ammotype;

        I_CheckGFX();

        if (globallump != (int)sfontlump)
        {
            gDPPipeSync(GFX1++);
            gDPSetCycleType(GFX1++, G_CYC_1CYCLE);
            gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
            gDPSetTexturePersp(GFX1++, G_TP_NONE);
            gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);
            gDPSetBlendColor(GFX1++, 0, 0, 0, 0);
            gDPSetCombineMode(GFX1++, G_CC_D64COMB04, G_CC_D64COMB04);
            gDPSetRenderMode(GFX1++, G_RM_XLU_SURF_CLAMP, G_RM_XLU_SURF2_CLAMP);
        }

        src = statuslump+sizeof(spriteN64_t);

        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, 639, 0);

        gDPPipeSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b, 10, 0, G_TX_RENDERTILE, 0, 0, 0, 0, 0, 0, 0);
        gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0, (79 << 2), (15 << 2));

        gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1, src + ((spriteN64_t*)statuslump)->cmpsize);

        gDPTileSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 255);

        gDPPipeSync(GFX1++);

        /* */
        /* Gray color */
        /* */
        gDPSetPrimColor(GFX1++, 0, 0, 128, 128, 128, HUDopacity);

        /* */
        /* Health */
        /* */
        gSPTextureRectangle(GFX1++, ((2+HUDmargin) << hudxshift), ((SCREEN_HT - 22 - HUDmargin) << hudyshift),
                                    ((42 + HUDmargin) << hudxshift), ((SCREEN_HT - 16 - HUDmargin)  << hudyshift),
                                    G_TX_RENDERTILE,
                                    (0 << 5), (0 << 5),
                                    (1 << 12 >> hudxshift), (1 << 12 >> hudyshift));

        /* */
        /* Armor */
        /* */
        gSPTextureRectangle(GFX1++, ((SCREEN_WD - 40 - HUDmargin) << hudxshift), ((SCREEN_HT - 22 - HUDmargin) << hudyshift),
                                    ((SCREEN_WD - 4 - HUDmargin) << hudxshift), ((SCREEN_HT - 16 - HUDmargin) << hudyshift),
                                    G_TX_RENDERTILE,
                                    (40 << 5), (0 << 5),
                                    (1 << 12 >> hudxshift), (1 << 12 >> hudyshift));

        stat = player->health;
        if (!ColoredHUD) {
            color = PACKRGBA(224,0,0,HUDopacity);
        } else if (stat <= 67) {
            color = PACKRGBA(224-96*stat/67,128*stat/67,0, HUDopacity);
        } else if (stat <= 133) {
            color = PACKRGBA(256-256*stat/133,128,64*stat/133-32, HUDopacity);
        } else {
            color = PACKRGBA(0, 256-192*stat/200,288*stat/200-160, HUDopacity);
        }

        // Crosshair (use health color to draw it)
        crosshair = player->config->crosshair;
        if (crosshair > 0)
        {
            register int cx = XResolution<<1;
            register int cy = YResolution<<1;
            gDPSetPrimColorD64(GFX1++, 0, 0, color);
            if (crosshair == 1) { // Dot
                gSPTextureRectangle(GFX1++,
                        cx-(1<<hudxshift), cy,
                        cx, cy+(1<<hudyshift),
                        G_TX_RENDERTILE,
                        (39 << 5), (5 << 5),
                        0, 0);
            } else if (crosshair == 2) { // Cross
                gSPTextureRectangle(GFX1++,
                        cx-(1<<hudxshift), cy-(1<<hudyshift),
                        cx, cy+(2<<hudyshift),
                        G_TX_RENDERTILE,
                        (40 << 5), (3 << 5),
                        (1 << 10), (1 << 10)); // vertical
                gSPTextureRectangle(GFX1++,
                        cx-(2<<hudxshift), cy,
                        cx+(1<<hudxshift), cy+(1<<hudyshift),
                        G_TX_RENDERTILE,
                        (23 << 5), (5 << 5),
                        (1 << 10), (1 << 10)); // horizontal
            } else if (crosshair == 3) { // Vertical
                gSPTextureRectangle(GFX1++,
                        cx-(3<<hudxshift), cy,
                        cx+(2<<hudxshift), cy+(1<<hudyshift),
                        G_TX_RENDERTILE,
                        (55 << 5), (4 << 5),
                        (1 << 10), (1 << 10)); // horizontal
                gSPTextureRectangle(GFX1++, cx-(1<<hudxshift),
                        cy-(1<<hudyshift), cx, cy+(2<<hudyshift),
                        G_TX_RENDERTILE,
                        (40 << 5), (3 << 5),
                        (1 << 10), (1 << 10)); // vertical
            }
        }

        /* */
        /* White color */
        /* */
        gDPSetPrimColor(GFX1++, 0, 0, 255, 255, 255, HUDopacity);

        /* */
        /* Cards & skulls */
        /* */
        for (ind = 0; ind < NUMCARDS; ind++)
        {
            if (player->cards[ind] || (flashCards[ind].active && flashCards[ind].doDraw))
            {
                register int cx = card_x[ind] << hudxshift;
                /* */
                /* Draw Keys Graphics */
                /* */
                gSPTextureRectangle(GFX1++, cx, ((SCREEN_HT-10-HUDmargin) << hudyshift),
                                            cx+(9 << hudxshift), ((SCREEN_HT-HUDmargin) << hudyshift),
                                            G_TX_RENDERTILE,
                                            ((ind * 9) << 5), (6 << 5),
                                            (1 << 12 >> hudxshift), (1 << 12 >> hudyshift));
            }
        }

        /* */
        /* Health */
        /* */
        ST_DrawNumber(22+HUDmargin, SCREEN_HT-13-HUDmargin, player->health, 0, color);

        /* */
        /* Ammo */
        /* */
        weapon = player->pendingweapon;

        if (weapon == wp_nochange)
            weapon = player->readyweapon;

        ammotype = weaponinfo[weapon].ammo;

        if (ammotype != am_noammo)
        {
            stat = player->ammo[ammotype];
            if (stat < 0)
                stat = 0;

            if (!ColoredHUD) {
                color = PACKRGBA(224,0,0,HUDopacity);
            } else if (ammotype == am_clip) {
                color = PACKRGBA(96,96,128,HUDopacity);
            } else if (ammotype == am_shell) {
                color = PACKRGBA(196,32,0,HUDopacity);
            } else if (ammotype == am_cell) {
                color = PACKRGBA(0,96,128,HUDopacity);
            } else {
                color = PACKRGBA(164,96,0,HUDopacity);
            }
            ST_DrawNumber(SCREEN_WD/2, SCREEN_HT-13-HUDmargin, stat, 0, color);
        }

        /* */
        /* Armor */
        /* */
        stat = player->armorpoints;
        if (!ColoredHUD || stat == 0) {
            color = PACKRGBA(224,0,0,HUDopacity);
        } else if (player->armortype == 1) {
            color = PACKRGBA(0,128,64,HUDopacity);
        } else {
            color = PACKRGBA(0,64,128,HUDopacity);
        }
        ST_DrawNumber(SCREEN_WD-22-HUDmargin, SCREEN_HT-13-HUDmargin, stat, 0, color);
    }
    ST_DrawDebug();
}

#define ST_FONTWHSIZE 8

void ST_Message(int x,int y,const char *text,int color) // 8002A36C
{
    byte *src;
    byte c;
    int s,t;
    int xpos, ypos;

    if (globallump != (int)sfontlump)
    {
        gDPPipeSync(GFX1++);

        gDPSetCycleType(GFX1++, G_CYC_1CYCLE);
        gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
        gDPSetTexturePersp(GFX1++, G_TP_NONE);

        gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);
        gDPSetBlendColor(GFX1++, 0, 0, 0, 0);

        gDPSetCombineMode(GFX1++, G_CC_D64COMB04, G_CC_D64COMB04);
        gDPSetRenderMode(GFX1++, G_RM_XLU_SURF_CLAMP, G_RM_XLU_SURF2_CLAMP);

        src = sfontlump+sizeof(spriteN64_t);

        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);

        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, 1023, 0);

        gDPPipeSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_4b, 16, 0, G_TX_RENDERTILE, 0, 0, 0, 0, 0, 0, 0);
        gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0, (255 << 2), (15 << 2));

        gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1, (src+0x800));

        gDPTileSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 15);

        gDPPipeSync(GFX1++);

        globallump = (int)sfontlump;
    }

    gDPSetPrimColorD64(GFX1++, 0, 0, color);

    if (x == -1)
        x = (SCREEN_WD>>1) - ((D_strlen(text) * ST_FONTWHSIZE) >> 1);

    ypos = y;
    xpos = x;
    while (*text)
    {
        c = *text;

        if (c == '\n')
        {
            ypos += (ST_FONTWHSIZE+1);
            xpos = x;
        }
        else
        {
            if(c >= 'a' && c <= 'z')
                c -= (26 + 6);

            if (c >= '!' && c <= '_')
            {
                if ((c - '!') < 32)
                    t = 0;
                else
                    t = ST_FONTWHSIZE;

                s = ((c - '!') & ~32) * ST_FONTWHSIZE;

                gSPTextureRectangle(GFX1++,
                                    (xpos << hudxshift), (ypos << hudyshift),
                                    ((xpos + ST_FONTWHSIZE) << hudxshift), ((ypos + ST_FONTWHSIZE) << hudyshift),
                                    G_TX_RENDERTILE,
                                    (s << 5), (t << 5),
                                    (1 << 12 >> hudxshift), (1 << 12 >> hudyshift));
            }
            xpos += ST_FONTWHSIZE;
        }
        text++;
    }
}

void ST_DrawNumber(int x, int y, int value, int mode, int color) // 8002A79C
{
    int index, width, i;
    int number [16];

    width = 0;
    for (index = 0; index < 16; index++)
    {
        number[index] = value % 10;
        width += symboldata[number[index]].w;

        value /= 10;
		if (!value) break;
    }

    switch(mode)
	{
	    case 0:			/* Center */
	        x -= (width / 2);
        case 1:			/* Right */
            while (index >= 0)
            {
                ST_DrawSymbol(x, y, number[index], color);
                x += symboldata[number[index]].w;
                index--;
            }
            break;
        case 2:			/* Left */
            i = 0;
            while (index >= 0)
            {
                x -= symboldata[number[i]].w;
                ST_DrawSymbol(x, y, number[i], color);
                i++;
                index--;
            }
            break;
        default:
            break;
	}
}

void ST_DrawString(int x, int y, const char *text, int color) // 8002A930
{
    byte c;
    int xpos, ypos, index;

    I_CheckGFX();

    xpos = x;
    if(xpos <= -1) /* Get Center Text Position */
        xpos = ST_GetCenterTextX((byte*)text);

    while (*text)
    {
        c = *text;
        ypos = y;

        if(c >= 'A' && c <= 'Z')
        {
            index = (c - 'A') + 16;
        }
        else if(c >= 'a' && c <= 'z')
        {
            index = (c - 'a') + 42;
            ypos = y + 2;
        }
        else if(c >= '0' && c <= '9')
        {
            index = (c - '0') + 0;
        }
        else if (c == '!')
        {
            index = 12;
            ypos = y - 1;
        }
        else if (c == '-')
        {
            index = 10;
        }
        else if (c == '.')
        {
            index = 13;
        }
        else if (c == ':')
        {
            index = 15;
        }
        else if (c == '?')
        {
            index = 14;
        }
        else if (c == '%')
        {
            index = 11;
        }
        else if(c >= FIRST_SYMBOL && c <= LAST_SYMBOL)
        {
            index = (c - '0');
        }
        else
        {
            xpos += 6; /* space */
            text++;
            continue;
        }

        ST_DrawSymbol(xpos, ypos, index, color);
        xpos += symboldata[index].w;

        text++;
    }
}

int ST_GetCenterTextX(byte *text) // 8002AAF4
{
    byte c;
    int xpos, index;

    xpos = 0;
    while (*text)
    {
        c = *text;

        if(c >= 'A' && c <= 'Z')
        {
            index = (c - 'A') + 16;
        }
        else if(c >= 'a' && c <= 'z')
        {
            index = (c - 'a') + 42;
        }
        else if(c >= '0' && c <= '9')
        {
            index = (c - '0') + 0;
        }
        else if (c == '!')
        {
            index = 12;
        }
        else if (c == '-')
        {
            index = 10;
        }
        else if (c == '.')
        {
            index = 13;
        }
        else if (c == ':')
        {
            index = 15;
        }
        else if (c == '?')
        {
            index = 14;
        }
        else if (c == '%')
        {
            index = 11;
        }
        else if(c >= FIRST_SYMBOL && c <= LAST_SYMBOL)
        {
            index = (c - '0');
        }
        else
        {
            xpos += 6; /* space */
            text++;
            continue;
        }

        xpos += symboldata[index].w;

        text++;
    }

    return (320 - xpos) / 2;
}

#define ST_MAXDMGCOUNT  144
#define ST_MAXSTRCOUNT  32
#define ST_MAXBONCOUNT  100

void ST_UpdateFlash(void) // 8002AC30
{
    player_t *plyr;
	int		cnt;
	int		bzc;
	int		bnc;


	plyr = &players[0];

	if ((plyr->powers[pw_infrared] < 120) && infraredFactor)
    {
        infraredFactor -= 4;
        if (infraredFactor < 0) {
            infraredFactor = 0;
        }

        P_RefreshBrightness();
    }

    /* invulnerability flash (white) */
	if (plyr->powers[pw_invulnerability] >= 61 || plyr->powers[pw_invulnerability] & 8)
	{
		FlashEnvColor = PACKRGBA(128, 128, 128, 255);
	}
	/* bfg flash (green)*/
	else if(plyr->bfgcount)
    {
        FlashEnvColor = PACKRGBA(0, plyr->bfgcount, 0, 255);
    }
	else
	{
	    /* damage and strength flash (red) */
	    cnt = plyr->damagecount;

	    if (cnt)
        {
            if((cnt + 16) > ST_MAXDMGCOUNT)
                cnt = ST_MAXDMGCOUNT;
        }

        if (plyr->powers[pw_strength] <= ST_MAXSTRCOUNT)
        {
            /* slowly fade the berzerk out */
            bzc = plyr->powers[pw_strength];

            if (bzc == 1)
                bzc = 0;
        }
        else
        {
            bzc = ST_MAXSTRCOUNT;
        }

        if ((cnt != 0) || (bzc != 0))
        {
            if (bzc < cnt)
            {
                FlashEnvColor = PACKRGBA(cnt, 0, 0, 255);
            }
            else
            {
                FlashEnvColor = PACKRGBA(bzc, 0, 0, 255);
            }
        }
        /* suit flash (green/yellow) */
        else if(plyr->powers[pw_ironfeet] >= 61 || plyr->powers[pw_ironfeet] & 8)
        {
            FlashEnvColor = PACKRGBA(0, 32, 4, 255);
        }
        /* bonus flash (yellow) */
        else if (plyr->bonuscount)
        {
            cnt = FlashBrightness*((plyr->bonuscount + 7) >> 3)/32;

            if (cnt > ST_MAXBONCOUNT)
                cnt = ST_MAXBONCOUNT;

            bnc = ((cnt << 2) + cnt) << 1;

            FlashEnvColor = PACKRGBA(bnc, bnc, cnt, 255);
        }
        else
        {
            FlashEnvColor = PACKRGBA(0, 0, 0, 255); /* Default Flash */
        }
	}
}

void ST_DrawSymbol(int xpos, int ypos, int index, int color) // 8002ADEC
{
    const symboldata_t *symbol;
    byte *data;
    int offset;

    data = W_CacheLumpNum(sumbolslump, PU_CACHE, dec_jag);

    if (sumbolslump != globallump)
    {
        gDPPipeSync(GFX1++);
        gDPSetCycleType(GFX1++, G_CYC_1CYCLE);

        gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
        gDPSetTexturePersp(GFX1++, G_TP_NONE);

        gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);
        gDPSetBlendColor(GFX1++, 0, 0, 0, 0);

        gDPSetCombineMode(GFX1++, G_CC_D64COMB04, G_CC_D64COMB04);
        gDPSetRenderMode(GFX1++, G_RM_XLU_SURF_CLAMP, G_RM_XLU_SURF2_CLAMP);

        // Load Palette Data
        offset = (((gfxN64_t*)data)->width * ((gfxN64_t*)data)->height);
        offset = ALIGN(offset, 8);
        gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b ,
                           1, data + offset + sizeof(gfxN64_t));

        gDPTileSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 255);

        gDPPipeSync(GFX1++);

        globallump = sumbolslump;
    }

    gDPSetPrimColorD64(GFX1++, 0, 0, color);

    // Load Image Data
    gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b ,
                       ((gfxN64_t*)data)->width, data + sizeof(gfxN64_t));

    symbol = &symboldata[index];

    // Clip Rectangle From Image
    gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b,
                (symbol->w + 8) / 8, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

    gDPLoadSync(GFX1++);
    gDPLoadTile(GFX1++, G_TX_LOADTILE,
                (symbol->x << 2), (symbol->y << 2),
                ((symbol->x + symbol->w) << 2), ((symbol->y + symbol->h) << 2));

    gDPPipeSync(GFX1++);
    gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b,
                (symbol->w + 8) / 8, 0, G_TX_RENDERTILE, 0, 0, 0, 0, 0, 0, 0);

    gDPSetTileSize(GFX1++, G_TX_RENDERTILE,
                 (symbol->x << 2), (symbol->y << 2),
                 ((symbol->x + symbol->w) << 2), ((symbol->y + symbol->h) << 2));

    gSPTextureRectangle(GFX1++,
                (xpos << hudxshift), (ypos << hudyshift),
                ((xpos + symbol->w) << hudxshift), ((ypos + symbol->h) << hudyshift),
                G_TX_RENDERTILE,
                (symbol->x << 5), (symbol->y << 5),
                (1 << 12 >> hudxshift), (1 << 12 >> hudyshift));
}

#include "stdarg.h"

void ST_EnableDebug(void)
{
    debug = true;
    if (!debugbuf)
        debugbuf = Z_Malloc(DEBUGLINES * DEBUGLINELEN, PU_STATIC, NULL);
}

void ST_DisableDebug(void)
{
    debug = false;
}

void ST_DebugSetPrintPos(int x, int y)
{
	debugX = x;
	debugY = y;
}

void ST_DebugPrint(const char *text, ...)
{
    if(debug)
    {
        int index, count;
        va_list args;
        char buf[256];
        char *ptr;

        if (debugcnt < DEBUGLINES)
            index = debugcnt;
        else
            index = debugstart;
        ptr = &debugbuf[index*DEBUGLINELEN];

        va_start (args, text);
        count = D_vsprintf (buf, text, args);
        va_end (args);

        if (count < 0)
            return;

        count = MIN(count, DEBUGLINELEN - 1);
        D_memcpy(ptr, buf, count);
        ptr[count] = '\0';

        if (debugcnt < DEBUGLINES)
            debugcnt += 1;
        else
            debugstart = debugstart < DEBUGLINES - 1 ?  debugstart + 1 : 0;
    }
}

void ST_DebugClear(void)
{
    if (debug)
    {
        debugcnt = 0;
        debugstart = 0;
        for (int i = 0; i < DEBUGLINES; i++)
            debugbuf[i * DEBUGLINELEN] = '\0';
    }
}
