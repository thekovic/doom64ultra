#include "doomdef.h"
#include "p_local.h"
#include "st_main.h"

extern void G_PlayerFinishLevel (int player);

boolean     gamepaused = false; // 800A6270

SDATA u32 playerbuttons[MAXPLAYERS];
SDATA u32 oldplayerbuttons[MAXPLAYERS];

/*
===============================================================================

                                THINKERS

All thinkers should be allocated by Z_Malloc so they can be operated on uniformly.  The actual
structures will vary in size, but the first element must be thinker_t.

Mobjs are similar to thinkers, but kept seperate for more optimal list
processing
===============================================================================
*/

SDATA thinker_t thinkercap = { /* both the head and tail of the thinker list */    //80096378
    .prev = &thinkercap, .next = &thinkercap };
SDATA mobjhead_t mobjhead = {  /* head and tail of mobj list */                    //800A8C74,
    .prev = (void*) &mobjhead, .next = (void*) &mobjhead };
DEBUG_COUNTER(SDATA int activethinkers = 0);

/*
===============
=
= P_InitThinkers
=
===============
*/
#if 0
void P_InitThinkers (void)
{
    thinkercap.prev = thinkercap.next  = &thinkercap;
    mobjhead.next = mobjhead.prev = &mobjhead;
}
#endif // 0

/*
===============
=
= P_AddThinker
=
= Adds a new thinker at the end of the list
=
===============
*/

void P_AddThinker (thinker_t *thinker) // 80021770
{
    thinkercap.prev->next = thinker;
    thinker->next = &thinkercap;
    thinker->prev = thinkercap.prev;
    thinkercap.prev = thinker;
}

/*
===============
=
= P_RemoveThinker
=
= Deallocation is lazy -- it will not actually be freed until its
= thinking turn comes up
=
===============
*/

void P_RemoveThinker (thinker_t *thinker) // 8002179C
{
    thinker->function = (think_t)-1;

    if (thinker == macrothinker) { // [D64] New lines
        macrothinker = NULL;
    }
}

/*
===============
=
= P_RunThinkers
=
===============
*/

void P_RunThinkers (void) // 800217C8
{
    thinker_t   *currentthinker, *next;

    DEBUG_COUNTER(activethinkers = 0);

    currentthinker = thinkercap.next;
    if (thinkercap.next != &thinkercap)
    {
        while (currentthinker != &thinkercap)
        {
            if (currentthinker->function == (think_t)-1)
            {   // time to remove it
                next = currentthinker->next;
                currentthinker->next->prev = currentthinker->prev;
                currentthinker->prev->next = currentthinker->next;
                Z_Free (currentthinker);
            }
            else
            {
                if (currentthinker->function)
                {
                    ((void (*)(void *)) currentthinker->function) (currentthinker);
                }
                DEBUG_COUNTER(activethinkers++);
                // get next pointer after in case the thinker added another one
                next = currentthinker->next;
            }
            currentthinker = next;
        }
    }
}

/*
===================
=
= P_RunMobjLate
=
= Run stuff that doesn't happen every tick
===================
*/
#if 0
void P_RunMobjLate (void)
{
    mobj_t  *mo;
    mobj_t  *next;

    for (mo=mobjhead.next ; mo != (void*) &mobjhead ; mo=next)
    {
        next = mo->next;    /* in case mo is removed this time */
        if (mo->latecall)
        {
            mo->latecall(mo);
        }
    }
}
#endif // 0

u8 pausecursorpos = 0;

/*
==============
=
= P_CheckCheats
=
==============
*/

gameaction_t P_CheckCheats (void) // 8002187C
{
    unsigned int buttons;
    int exit;

    buttons = allticbuttons & 0xffff0000;

    if (!gamepaused)
    {
        if ((buttons & PAD_START) && !(alloldticbuttons & PAD_START))
        {
            gamepaused = true;

            S_PauseSound();

            lastticon = ticon;

            cursorpos = pausecursorpos;
            M_PauseMenu();
        }

        return ga_nothing;
    }

    exit = M_MenuTicker();

    if (exit)
    {
        pausecursorpos = cursorpos;
        M_MenuClearCall(ga_nothing);
    }

    if (exit == ga_warped || exit == ga_recorddemo || exit == ga_restart || exit == ga_exitdemo || exit == ga_loadquicksave)
    {
        gameaction = exit;
    }
    else if (exit == ga_exit)
    {
        gamepaused = false;
        S_ResumeSound();
        ticon = lastticon;
        ticsinframe = lastticon >> 2;
    }

    return exit;
}

void G_DoReborn (int playernum);//extern

static inline u32 P_GetButtons(player_t *player, int buttons, int buttons2)
{
    u32 b = 0;
    controls_t *controls = player->controls;
    controls2_t *controls2 = player->controls2;

    for (u32 i = 0; i < NUMBUTTONS; i++)
    {
        if (controls->buttons[i] & buttons)
            b |= 1<<i;
    }
    if (controls2)
    {
        if ((buttons2 & PAD_A) && controls2->a != BT_NONE)
            b |= 1<<controls2->a;
        if ((buttons2 & PAD_B) && controls2->b != BT_NONE)
            b |= 1<<controls2->b;
        if ((buttons2 & PAD_Z_TRIG) && controls2->z != BT_NONE)
            b |= 1<<controls2->z;
    }

    return b;
}

/*
=================
=
= P_Ticker
=
=================
*/

//extern functions
void P_CheckSights (void);
void P_RunMobjBase (void);

int P_Ticker (void)//80021A00
{
    player_t *pl;
    gameaction_t menuaction;

    gameaction = ga_nothing;

    //
    // check for pause and cheats
    //
    menuaction = P_CheckCheats();

    DEBUG_CYCLES_START(world_start);

    if ((!gamepaused) && (gamevbls < gametic))
    {
        P_RunThinkers();
        P_CheckSights();
        P_RunMobjBase();

        P_UpdateSpecials();
        P_RunMacros();

        ST_Ticker(); // update status bar
    }

    //ST_DebugPrint("%d",Z_FreeMemory (mainzone));

    //
    // run player actions
    //
    pl = players;

    if (pl->playerstate == PST_REBORN)
    {
        if (customskill.permadeath)
            gameaction = ga_exitdemo;
        else
            gameaction = ga_died;
    }


    if (menuaction != ga_exit)
        oldplayerbuttons[0] = playerbuttons[0];
    playerbuttons[0] = P_GetButtons(pl, ticbuttons[0], ticbuttons[1]);
    if (menuaction == ga_exit) // avoid button presses triggering again in the game
        oldplayerbuttons[0] = playerbuttons[0];

    AM_Control(pl);
    P_PlayerThink(pl);

#ifndef NDEBUG
    if (!gamepaused)
        DEBUG_CYCLES_END(world_start, LastWorldCycles);
    else
        LastWorldCycles = 0;
#endif

    return gameaction; // may have been set to ga_died, ga_completed, or ga_secretexit
}

/*
=============
=
= P_Drawer
=
= draw current display
=============
*/

extern Mtx R_ProjectionMatrix;  // 800A68B8
extern Mtx R_ModelMatrix;       // 8005b0C8

void P_Drawer (void) // 80021AC8
{
    I_ClearFrame();

    gMoveWd(GFX1++, G_MW_CLIP, G_MWO_CLIP_RNX, 1);
    gMoveWd(GFX1++, G_MW_CLIP, G_MWO_CLIP_RNY, 1);
    gMoveWd(GFX1++, G_MW_CLIP, G_MWO_CLIP_RPX, 65535);
    gMoveWd(GFX1++, G_MW_CLIP, G_MWO_CLIP_RPY, 65535);
    gMoveWd(GFX1++, G_MW_PERSPNORM, G_MWO_MATRIX_XX_XY_I, 68);

    // create a projection matrix
    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(&R_ProjectionMatrix), G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);

    // create a model matrix
    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(&R_ModelMatrix), G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);

    if (players[0].automapflags & (AF_LINES|AF_SUBSEC))
    {
        AM_Drawer();
    }
    else
    {
        R_RenderPlayerView();
        //ST_DebugPrint("x %d || y %d", players[0].mo->x >> 16, players[0].mo->y >> 16);

        if (demoplayback)
            ST_DrawDebug();
        else
            ST_Drawer();
    }

    if (MenuCall)
    {
        M_DrawOverlay(0, 0, XResolution, YResolution, 96);
        MenuCall();
    }

    I_DrawFrame();
}

extern void T_FadeInBrightness(fadebright_t *fb);
extern int start_time;  // 80063390
extern int end_time;    // 80063394

void P_Start (void) // 80021C50
{
    fadebright_t *fb;

    DrawerStatus = 1;

    if (gamemap == 33)  /* Add by default God Mode in player  */
        players[0].cheats |= CF_GODMODE;

    gamepaused = false;
    validcount = 1;

    AM_Start();

    if (gameaction == ga_loadquicksave)
    {
        MusicID = MapInfo[gamemap].MusicSeq-(NUMSFX-1);
        S_StartMusic(MapInfo[gamemap].MusicSeq);
        P_RefreshBrightness();

        gameaction = ga_nothing;
    }
    else
    {
        M_ClearRandom();

        /* do a nice little fade in effect */
        fb = Z_Malloc(sizeof(*fb), PU_LEVSPEC, 0);
        P_AddThinker(&fb->thinker);
        fb->thinker.function = T_FadeInBrightness;
        fb->factor = 0;

        /* autoactivate line specials */
        P_ActivateLineByTag(999, players[0].mo);

        start_time = ticon;

        MusicID = MapInfo[gamemap].MusicSeq-(NUMSFX-1);
        S_StartMusic(MapInfo[gamemap].MusicSeq);
    }
}

void P_Stop (int exit) // 80021D58
{
    /* [d64] stop plasma buzz */
    S_StopSound(0, sfx_electric);

    end_time = ticon;
    gamepaused = false;
    DrawerStatus = 0;

    G_PlayerFinishLevel(0);

    /* free all tags except the PU_STATIC tag */
    Z_FreeTags(mainzone, ~PU_STATIC); // (PU_LEVEL | PU_LEVSPEC | PU_CACHE)

    if ((gamemap != 33) || (exit == ga_exit))
        S_StopMusic();

    if ((demoplayback) && (exit == ga_exitdemo))
        I_WIPE_FadeOutScreen();
    else if (exit != ga_loadquicksave)
        I_WIPE_MeltScreen();

    S_StopAll();
    I_StopRumble();
}

