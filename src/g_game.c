/* G_game.c  */

#include "doomdef.h"
#include "p_local.h"
#include "p_saveg.h"
#include "config.h"
#include "i_usb.h"
#include "st_main.h"
#ifdef USB
#include "i_debug.h"
#endif

#ifdef DEVWARP
#define DEVWARP_ENABLED 1
#else
#define DEVWARP_ENABLED 0
#endif

void G_PlayerReborn (int player);

void G_DoReborn (int playernum);

void G_DoLoadLevel (void);


gameaction_t    gameaction;                 // 80063230
SDATA customskill_t   customskill;
int             gamemap;                    // 80063238
int             nextmap;                    // 8006323C /* the map to go to after the stats */

//boolean         playeringame[MAXPLAYERS]; //
SDATA player_t        players[MAXPLAYERS];        // 80063240

int             consoleplayer;          /* player taking events and displaying  */
int             displayplayer;          /* view being displayed  */
int             totalkills, totalitems, totalsecret;    /* for intermission  */

SDATA playerconfig_t  playerconfigs[MAXPLAYERS];

//char            demoname[32];
boolean         demorecording;          // 800633A4
boolean         demoplayback;           // 800633A8
int             *demo_p = NULL, *demobuffer = NULL;   // 8005A180, 8005a184
int             demosize;

//mapthing_t    deathmatchstarts[10], *deathmatch_p;    // 80097e4c, 80077E8C
mapthing_t  playerstarts[MAXPLAYERS];   // 800a8c60

/*
==============
=
= G_DoLoadLevel
=
==============
*/

void G_DoLoadLevel (void) // 80004530
{
    if (customskill.pistol_start || ((gameaction == ga_restart) || (gameaction == ga_warped)) || (players[0].playerstate == PST_DEAD))
        players[0].playerstate = PST_REBORN;

    P_SetupLevel(gamemap);
    P_FinishSetupLevel();
    I_SaveProgress(&LevelSaveBuffer);
    I_CheckRumblePak();
    gameaction = ga_nothing;
}


/*
==============================================================================

                        PLAYER STRUCTURE FUNCTIONS

also see P_SpawnPlayer in P_Mobj
==============================================================================
*/

/*
====================
=
= G_PlayerFinishLevel
=
= Can when a player completes a level
====================
*/

void G_PlayerFinishLevel (int player) // 80004598
{
    player_t *p;

    p = &players[player];

    bzero(p->powers,  sizeof (p->powers));
    bzero(p->cards,  sizeof (p->cards));
    p->mo->flags &= ~MF_SHADOW; /* cancel invisibility  */
    p->damagecount = 0;                     /* no palette changes  */
    p->bonuscount = 0;
    p->bfgcount = 0;
    p->automapflags = 0;
    p->messagetics[0] = 0;
    p->messagetics[1] = 0;  // [Immorpher] Clear messages
    p->messagetics[2] = 0;  // [Immorpher] Clear messages
}

/*
====================
=
= G_PlayerReborn
=
= Called after a player dies
= almost everything is cleared and initialized
====================
*/

int globalcheats = 0; // [GEC]

void G_PlayerReborn (int player) // 80004630
{
    player_t *p;

    p = &players[player];
    bzero(p,  sizeof(*p));

    p->attackdown = false;
    p->playerstate = PST_LIVE;
    p->health = MAXHEALTH;
    p->readyweapon = p->pendingweapon = wp_pistol;
    p->weaponowned[wp_fist] = true;
    p->weaponowned[wp_pistol] = true;
    p->ammo[am_clip] = 50;
    p->maxammo[am_clip] = maxammo[am_clip];
    p->maxammo[am_shell] = maxammo[am_shell];
    p->maxammo[am_cell] = maxammo[am_cell];
    p->maxammo[am_misl] = maxammo[am_misl];
    p->weaponwheelsize = p->weaponwheelpos = p->weaponwheeltarget = WHEEL_WEAPON_SIZE;

    p->cheats |= globalcheats; // [GEC] Apply global cheat codes
#ifdef DEVCHEATS
    p->cheats |= (DEVCHEATS);
#endif
}

/*============================================================================  */

/*
====================
=
= G_CompleteLevel
=
====================
*/

void G_CompleteLevel (void) // 800046E4
{
    gameaction = ga_completed;
}

/*
====================
=
= G_InitNew
=
====================
*/

mobj_t emptymobj; // 80063158

void G_InitNew (customskill_t skill, int map, gametype_t gametype) // 800046F4
{
    //printf ("G_InitNew, skill %d, map %d\n", skill, map);

    /* free all tags except the PU_STATIC tag */
    Z_FreeTags(mainzone, ~PU_STATIC); // (PU_LEVEL | PU_LEVSPEC | PU_CACHE)

    M_ClearRandom ();

/* force players to be initialized upon first level load          */
    players[0].playerstate = PST_REBORN;

/* these may be reset by I_NetSetup */
    customskill = skill;
    gamemap = map;

    bzero(&emptymobj,  sizeof(emptymobj));
    players[0].mo = &emptymobj; /* for net consistancy checks */

    demorecording = false;
    demoplayback = false;

    G_InitSkill (skill);
}

void G_InitSkill (customskill_t skill) // [Immorpher] initialize skill
{
    if (skill.monster_speed >= 1)
    {
        // Faster enemies
        states[S_SARG_ATK1].tics = 4;
        states[S_SARG_ATK2].tics = 4;
        states[S_SARG_ATK3].tics = 4;
        mobjinfo[MT_DEMON1].speed = 17;
        mobjinfo[MT_DEMON2].speed = 17;
        // Faster Projectiles
        mobjinfo[MT_PROJ_BRUISER1].speed = 20; // MT_BRUISERSHOT
        mobjinfo[MT_PROJ_BRUISER2].speed = 20; // MT_BRUISERSHOT2
        mobjinfo[MT_PROJ_HEAD].speed = 30; // MT_HEADSHOT value like Doom 64 Ex
        mobjinfo[MT_PROJ_IMP1].speed = 20; // MT_TROOPSHOT
        mobjinfo[MT_PROJ_IMP2].speed = 35; // MT_TROOPSHOT2 value like Doom 64 Ex

        if (skill.monster_speed >= 2)
        {
            mobjinfo[MT_DEMON1].speed = 24;
            mobjinfo[MT_DEMON2].speed = 24;
            mobjinfo[MT_MANCUBUS].speed = 14;
            mobjinfo[MT_POSSESSED1].speed = 14;
            mobjinfo[MT_POSSESSED2].speed = 14;
            mobjinfo[MT_IMP1].speed = 14;
            mobjinfo[MT_IMP2].speed = 20;
            mobjinfo[MT_CACODEMON].speed = 14;
            mobjinfo[MT_BRUISER1].speed = 14;
            mobjinfo[MT_BRUISER2].speed = 14;
            mobjinfo[MT_SKULL].speed = 14;
            mobjinfo[MT_BABY].speed = 18;
            mobjinfo[MT_CYBORG].speed = 20;
            mobjinfo[MT_PAIN].speed = 12;
            mobjinfo[MT_RESURRECTOR].speed = 32;
        }
    }
    else
    {
        // Restore enemy speeds
        states[S_SARG_ATK1].tics = 8;
        states[S_SARG_ATK2].tics = 8;
        states[S_SARG_ATK3].tics = 8;
        mobjinfo[MT_DEMON1].speed = 12;
        mobjinfo[MT_DEMON2].speed = 12;
        mobjinfo[MT_MANCUBUS].speed = 8;
        mobjinfo[MT_POSSESSED1].speed = 8;
        mobjinfo[MT_POSSESSED2].speed = 8;
        mobjinfo[MT_IMP1].speed = 8;
        mobjinfo[MT_IMP2].speed = 16;
        mobjinfo[MT_CACODEMON].speed = 8;
        mobjinfo[MT_BRUISER1].speed = 8;
        mobjinfo[MT_BRUISER2].speed = 8;
        mobjinfo[MT_SKULL].speed = 8;
        mobjinfo[MT_BABY].speed = 12;
        mobjinfo[MT_CYBORG].speed = 16;
        mobjinfo[MT_PAIN].speed = 8;
        mobjinfo[MT_RESURRECTOR].speed = 30;
        // Restore projectile speeds
        mobjinfo[MT_PROJ_BRUISER1].speed = 15; // MT_BRUISERSHOT
        mobjinfo[MT_PROJ_BRUISER2].speed = 15; // MT_BRUISERSHOT2
        mobjinfo[MT_PROJ_HEAD].speed = 20; // MT_HEADSHOT
        mobjinfo[MT_PROJ_IMP1].speed = 10; // MT_TROOPSHOT
        mobjinfo[MT_PROJ_IMP2].speed = 20; // MT_TROOPSHOT2
    }

    if (skill.monster_shrink >= 1)
    {
        // [Immorpher] Thinner enemies
        mobjinfo[MT_DEMON1].radius = 38*FRACUNIT;
        mobjinfo[MT_DEMON1].height = 81*FRACUNIT;
        mobjinfo[MT_DEMON2].radius = 38*FRACUNIT;
        mobjinfo[MT_DEMON2].height = 81*FRACUNIT;
        mobjinfo[MT_MANCUBUS].radius = 54*FRACUNIT;
        mobjinfo[MT_MANCUBUS].height = 105*FRACUNIT;
        mobjinfo[MT_POSSESSED1].radius = 19*FRACUNIT;
        mobjinfo[MT_POSSESSED1].height = 79*FRACUNIT;
        mobjinfo[MT_POSSESSED2].radius = 19*FRACUNIT;
        mobjinfo[MT_POSSESSED2].height = 79*FRACUNIT;
        mobjinfo[MT_IMP1].radius = 27*FRACUNIT;
        mobjinfo[MT_IMP1].height = 86*FRACUNIT;
        mobjinfo[MT_IMP2].radius = 27*FRACUNIT;
        mobjinfo[MT_IMP2].height = 86*FRACUNIT;
        mobjinfo[MT_CACODEMON].radius = 40*FRACUNIT;
        mobjinfo[MT_CACODEMON].height = 88*FRACUNIT;
        mobjinfo[MT_BRUISER1].radius = 24*FRACUNIT;
        mobjinfo[MT_BRUISER1].height = 98*FRACUNIT;
        mobjinfo[MT_BRUISER2].radius = 24*FRACUNIT;
        mobjinfo[MT_BRUISER2].height = 98*FRACUNIT;
        mobjinfo[MT_SKULL].radius = 26*FRACUNIT;
        mobjinfo[MT_SKULL].height = 62*FRACUNIT;
        mobjinfo[MT_BABY].radius = 62*FRACUNIT;
        mobjinfo[MT_BABY].height = 78*FRACUNIT;
        mobjinfo[MT_CYBORG].radius = 59*FRACUNIT;
        mobjinfo[MT_CYBORG].height = 168*FRACUNIT;
        mobjinfo[MT_PAIN].radius = 55*FRACUNIT;
        mobjinfo[MT_PAIN].height = 98*FRACUNIT;
        mobjinfo[MT_RESURRECTOR].radius = 71*FRACUNIT;
        mobjinfo[MT_RESURRECTOR].height = 148*FRACUNIT;
    }
    else
    {
        // Restore Enemy Size
        mobjinfo[MT_DEMON1].radius = 44*FRACUNIT;
        mobjinfo[MT_DEMON1].height = 100*FRACUNIT;
        mobjinfo[MT_DEMON2].radius = 50*FRACUNIT;
        mobjinfo[MT_DEMON2].height = 100*FRACUNIT;
        mobjinfo[MT_MANCUBUS].radius = 60*FRACUNIT;
        mobjinfo[MT_MANCUBUS].height = 108*FRACUNIT;
        mobjinfo[MT_POSSESSED1].radius = 32*FRACUNIT;
        mobjinfo[MT_POSSESSED1].height = 87*FRACUNIT;
        mobjinfo[MT_POSSESSED2].radius = 32*FRACUNIT;
        mobjinfo[MT_POSSESSED2].height = 87*FRACUNIT;
        mobjinfo[MT_IMP1].radius = 42*FRACUNIT;
        mobjinfo[MT_IMP1].height = 94*FRACUNIT;
        mobjinfo[MT_IMP2].radius = 42*FRACUNIT;
        mobjinfo[MT_IMP2].height = 94*FRACUNIT;
        mobjinfo[MT_CACODEMON].radius = 55*FRACUNIT;
        mobjinfo[MT_CACODEMON].height = 90*FRACUNIT;
        mobjinfo[MT_BRUISER1].radius = 24*FRACUNIT;
        mobjinfo[MT_BRUISER1].height = 100*FRACUNIT;
        mobjinfo[MT_BRUISER2].radius = 24*FRACUNIT;
        mobjinfo[MT_BRUISER2].height = 100*FRACUNIT;
        mobjinfo[MT_SKULL].radius = 28*FRACUNIT;
        mobjinfo[MT_SKULL].height = 64*FRACUNIT;
        mobjinfo[MT_BABY].radius = 64*FRACUNIT;
        mobjinfo[MT_BABY].height = 80*FRACUNIT;
        mobjinfo[MT_CYBORG].radius = 70*FRACUNIT;
        mobjinfo[MT_CYBORG].height = 170*FRACUNIT;
        mobjinfo[MT_PAIN].radius = 60*FRACUNIT;
        mobjinfo[MT_PAIN].height = 112*FRACUNIT;
        mobjinfo[MT_RESURRECTOR].radius = 80*FRACUNIT;
        mobjinfo[MT_RESURRECTOR].height = 150*FRACUNIT;
    }

    if (skill.monster_reduced_pain >= 1)
    {
        // Less pain
        mobjinfo[MT_DEMON1].painchance = 90;
        mobjinfo[MT_DEMON2].painchance = 90;
        mobjinfo[MT_MANCUBUS].painchance = 40;
        mobjinfo[MT_POSSESSED1].painchance = 100;
        mobjinfo[MT_POSSESSED2].painchance = 95;
        mobjinfo[MT_IMP1].painchance = 100;
        mobjinfo[MT_IMP2].painchance = 64;
        mobjinfo[MT_CACODEMON].painchance = 64;
        mobjinfo[MT_BRUISER1].painchance = 25;
        mobjinfo[MT_BRUISER2].painchance = 25;
        mobjinfo[MT_SKULL].painchance = 128;
        mobjinfo[MT_BABY].painchance = 64;
        mobjinfo[MT_CYBORG].painchance = 10;
        mobjinfo[MT_PAIN].painchance = 64;
        mobjinfo[MT_RESURRECTOR].painchance = 25;
    }
    else
    {
        // Restore pains
        mobjinfo[MT_DEMON1].painchance = 180;
        mobjinfo[MT_DEMON2].painchance = 180;
        mobjinfo[MT_MANCUBUS].painchance = 80;
        mobjinfo[MT_POSSESSED1].painchance = 200;
        mobjinfo[MT_POSSESSED2].painchance = 170;
        mobjinfo[MT_IMP1].painchance = 200;
        mobjinfo[MT_IMP2].painchance = 128;
        mobjinfo[MT_CACODEMON].painchance = 128;
        mobjinfo[MT_BRUISER1].painchance = 50;
        mobjinfo[MT_BRUISER2].painchance = 50;
        mobjinfo[MT_SKULL].painchance = 256;
        mobjinfo[MT_BABY].painchance = 128;
        mobjinfo[MT_CYBORG].painchance = 20;
        mobjinfo[MT_PAIN].painchance = 128;
        mobjinfo[MT_RESURRECTOR].painchance = 50;
    }
}
/*============================================================================  */

void M_QuickLoadFailed(void);
/*
=================
=
= G_RunGame
=
= The game should allready have been initialized or laoded
=================
*/

void G_RunGame (void) // 80004794
{
    while (1)
    {
        /* load a level */
        if (gameaction == ga_loadquicksave)
        {
            if (!I_QuickLoad())
            {
                M_QuickLoadFailed();
                gameaction = ga_nothing;
                return;
            }
        }
        else
        {
            G_DoLoadLevel ();

            if(!DEVWARP_ENABLED && runintroduction && Settings.StoryText == true) { // [Immorpher] run introduction text screen
                MiniLoop(F_StartIntermission, F_StopIntermission, F_TickerIntermission, F_DrawerIntermission);
                runintroduction = false; // [Immorpher] only run it once!
            }
        }

        //D_printf("RUN P_Start\n");
        /* run a level until death or completion */
        MiniLoop (P_Start, P_Stop, P_Ticker, P_Drawer);

        if (gameaction == ga_recorddemo)
            G_RecordDemo();

        if(gameaction == ga_warped || gameaction == ga_loadquicksave)
            continue; /* skip intermission */

        if ((gameaction == ga_died) || (gameaction == ga_restart))
            continue;           /* died, so restart the level */

        if (gameaction == ga_exitdemo)
            return;

        /* run a stats intermission - [Immorpher] Removed Hectic exception */
        MiniLoop(IN_Start, IN_Stop, IN_Ticker, IN_Drawer);

#ifdef USB
        /* back to title screen from a custom map */
        if (gamemap == 0)
            return;
#endif

        if((((gamemap ==  8) && (nextmap ==  9)) ||
           ((gamemap ==  4) && (nextmap == 29)) ||
           ((gamemap == 12) && (nextmap == 30)) ||
           ((gamemap == 18) && (nextmap == 31)) ||
           ((gamemap ==  1) && (nextmap == 32))) && Settings.StoryText == true)
        {
            /* run the intermission if needed */
            MiniLoop(F_StartIntermission, F_StopIntermission, F_TickerIntermission, F_DrawerIntermission);

            if(gameaction == ga_warped || gameaction == ga_loadquicksave)
                continue; /* skip intermission */

            if(gameaction == ga_restart)
                continue;

            if (gameaction == ga_exitdemo)
                return;
        }
        else
        {
            if (nextmap >= LASTLEVEL)
            {
                /* run the finale if needed */
                MiniLoop(F_Start, F_Stop, F_Ticker, F_Drawer);

                if(gameaction == ga_warped || gameaction == ga_loadquicksave)
                    continue; /* skip intermission */

                if(gameaction == ga_restart)
                    continue;
                else
                    return;
            }
        }

        /* Set Next Level */
        gamemap = nextmap;
    }
}

int G_PlayDemoPtr (customskill_t skill, int map) // 800049D0
{
    int     exit;
    controls_t controls[MAXPLAYERS];
    playerconfig_t configs[MAXPLAYERS];
    gametype_t gametype;

    demobuffer = demo_p;

    for (int i = 0; i < MAXPLAYERS; i++)
    {
        /* copy key configuration */
        D_memcpy(&controls[i], &CurrentControls[i], sizeof(controls_t));
        /* copy additional player settings */
        D_memcpy(&configs[i], &playerconfigs[i], sizeof(playerconfig_t));
    }

    // [nova] new demo format
    if (*((int *)demobuffer) == DEMO_MAGIC)
    {
        const demoheader_t *header = (void*)demobuffer;

        demobuffer += (sizeof(demoheader_t) >> 2);
        skill = header->skill;
        map = header->map;
        gametype = header->gametype;
        //playersingame[0] = header->player1;
        //playersingame[1] = header->player2;
        //playersingame[2] = header->player3;
        //playersingame[3] = header->player4;
        for (int i = 0; i < MAXPLAYERS; i++)
        {
            const savedplayerconfig_t *config;

            //if (!playersingame[i])
                //continue;

            config = (void*)demobuffer;
            P_UnArchivePlayerConfig(i, config);
            R_UpdatePlayerPalette(0);
            playerconfigs[i].crosshair = 0;
            demobuffer += (sizeof(savedplayerconfig_t) >> 2);
        }
    }
    else
    {
        /* skip all old demos except titlemap due to desyncs */
        if (map != 33)
            return ga_exit;

        /* set new key configuration */
        bzero(&CurrentControls[0].buttons, sizeof CurrentControls[0].buttons);
        D_memcpy(&CurrentControls[0].buttons, demobuffer, sizeof(int)*13);
        CurrentControls[0].stick = STICK_TURN & STICK_MOVE;
        /* set new player settings  */
        playerconfigs[0].looksensitivity = demobuffer[13];
        playerconfigs[0].movesensitivity = 0;
        playerconfigs[0].autorun = false;
        playerconfigs[0].autoaim = true;
        playerconfigs[0].verticallook = 1;
        playerconfigs[0].crosshair = 0;
        playerconfigs[0].lookspring = false;
        /* skip analog and key configuration */
        demobuffer += 14;
        gametype = gt_single;
    }

    /* play demo game */
    G_InitNew (skill, map, gametype);
    G_DoLoadLevel ();
    demoplayback = true;
    exit = MiniLoop (P_Start, P_Stop, P_Ticker, P_Drawer);
    demoplayback = false;

    for (int i = 0; i < MAXPLAYERS; i++)
    {
        /* restore key configuration */
        D_memcpy(&CurrentControls[i], &controls[i], sizeof(controls_t));
        /* restore player settings */
        D_memcpy(&playerconfigs[i], &configs[i], sizeof(playerconfig_t));
    }

    /* free all tags except the PU_STATIC tag */
    Z_FreeTags(mainzone, ~PU_STATIC); // (PU_LEVEL | PU_LEVSPEC | PU_CACHE)

    return exit;
}

void G_RecordDemo (void)//80013D0C
{
    int exit;
    int demolen;

    demosize = 0x8000;
    demo_p = demobuffer = Z_Malloc (demosize, PU_STATIC, NULL);

    demoheader_t *header = (void*) demobuffer;
    demobuffer += (sizeof *header) >> 2;

    header->magic = DEMO_MAGIC;
    header->version = 0;
    header->map = startmap;
    header->skill = startskill;
    header->gametype = gt_single;
    header->player1 = 1;
    //header->player1 = playersingame[0];
    //header->player2 = playersingame[1];
    //header->player3 = playersingame[2];
    //header->player4 = playersingame[3];
    for (int i = 0; i < MAXPLAYERS; i++)
    {
        savedplayerconfig_t *config;

        //if (!playersingame[i])
            //continue;

        config = (void*) demobuffer;
        P_ArchivePlayerConfig(i, config);
        demobuffer += (sizeof *config) >> 2;
    }

    G_InitNew (startskill, startmap, gt_single);
    G_DoLoadLevel ();
    demorecording = true;
    MiniLoop (P_Start, P_Stop, P_Ticker, P_Drawer);
    demorecording = false;
    demolen = (demobuffer - demo_p) * sizeof(int);

#ifdef USB
    I_USBSendFile(demo_p, demolen);
#endif

    I_SaveDemo(demo_p, demolen);

    ST_EnableDebug();
    ST_DebugPrint("Playing Demo");
    if (IsEmulator)
        ST_DebugPrint("RAM Pos 0x%08lx Len 0x%x", (u32) demo_p, demolen);

    do {
        exit = G_PlayDemoPtr (startskill, startmap);
    } while (exit != ga_exitdemo);

    ST_DebugClear();
    ST_DisableDebug();
    Z_Free(demo_p);

    gameaction = exit;
}
