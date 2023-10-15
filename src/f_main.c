/* f_main.c -- finale */

#include "doomdef.h"
#include "st_main.h"

extern gameaction_t P_CheckCheats (void);

#define T_NULL          ""

// [Immorpher] New introduction text adapted from Doom 64 reloaded!
#define C_INTRO_TXT01   "Surviving the poison of evil has"
#define C_INTRO_TXT02   "pushed sanity to an edge. The"
#define C_INTRO_TXT03   "doctors' treatments are of little"
#define C_INTRO_TXT04   "help. Nightmares of so many demons"
#define C_INTRO_TXT05   "pouring through will not stop."
#define C_INTRO_TXT06   " "
#define C_INTRO_TXT07   "The outpost at Phobos was left dead"
#define C_INTRO_TXT08   "in a nuclear haze. However, hell"
#define C_INTRO_TXT09   "has reached back in. Something has"
#define C_INTRO_TXT10   "distorted reality and resurrected"
#define C_INTRO_TXT11   "decaying carnage back into hideous"
#define C_INTRO_TXT12   "living tissue."
#define C_INTRO_TXT13   " "
#define C_INTRO_TXT14   "The mutations are devastating. The"
#define C_INTRO_TXT15   "Demons have returned even stronger"
#define C_INTRO_TXT16   "and more vicious than before. As"
#define C_INTRO_TXT17   "the only one who has survived the"
#define C_INTRO_TXT18   "horror, the mission is clear..."
#define C_INTRO_TXT19   " "
#define C_INTRO_TXT20   "MERCILESS EXTERMINATION!"

#define C_END1_TXT01    "You cackle as the"
#define C_END1_TXT02    "familiarity of the"
#define C_END1_TXT03    "situation occurs to you."
#define C_END1_TXT04    "The gateway to the demons"
#define C_END1_TXT05    "domain was too accessible."
#define C_END1_TXT06    "You realize the demons mock"
#define C_END1_TXT07    "you with their invitation."
#define C_END1_TXT08    "It does not matter..."
#define C_END1_TXT09    "The demons spawn like rats"
#define C_END1_TXT10    "and you have the grade AAA"
#define C_END1_TXT11    "U.A.C. poison they crave."
#define C_END1_TXT12    "Your bloodthirsty scream"
#define C_END1_TXT13    "shatters the teleport haze."
#define C_END1_TXT14    "Once again you find yourself"
#define C_END1_TXT15    "amidst..."

#define C_END2_TXT01    "The vast silence reminds"
#define C_END2_TXT02    "you of the military morgue."
#define C_END2_TXT03    " "
#define C_END2_TXT04    "You knew the installation"
#define C_END2_TXT05    "had a classified level."
#define C_END2_TXT06    " "
#define C_END2_TXT07    "The U.A.C. had some good"
#define C_END2_TXT08    "reason to hide this place."
#define C_END2_TXT09    " "
#define C_END2_TXT10    "You wonder what it"
#define C_END2_TXT11    "could be..."

#define C_END3_TXT01    "You smile."
#define C_END3_TXT02    " "
#define C_END3_TXT03    "What strange place have"
#define C_END3_TXT04    "you stumbled upon?"
#define C_END3_TXT05    " "
#define C_END3_TXT06    "The demons did not expect"
#define C_END3_TXT07    "you to survive this far."
#define C_END3_TXT08    "You feel their demonic"
#define C_END3_TXT09    "presence waiting for you..."
#define C_END3_TXT10    " "
#define C_END3_TXT11    "Let them taste their guts!"

#define C_END4_TXT01    "You wretch as a strange"
#define C_END4_TXT02    "acrid odor assaults you."
#define C_END4_TXT03    " "
#define C_END4_TXT04    "Death and demon carcass!"
#define C_END4_TXT05    " "
#define C_END4_TXT06    "No nightmare could have"
#define C_END4_TXT07    "prepared you for this."
#define C_END4_TXT08    " "
#define C_END4_TXT09    "You realize that this"
#define C_END4_TXT10    "place was not meant for"
#define C_END4_TXT11    "living humans."

#define C_END5_TXT01    "Congratulations!"
#define C_END5_TXT02    "You found..."
#define C_END5_TXT03    " "
#define C_END5_TXT04    "HECTIC"
#define C_END5_TXT05    " "
#define C_END5_TXT06    "Only the best will reap"
#define C_END5_TXT07    "its rewards."

#define C_END6_TXT01    "Finally..."
#define C_END6_TXT02    "The mother of all demons"
#define C_END6_TXT03    "is dead!"
#define C_END6_TXT04    " "
#define C_END6_TXT05    "The blood pours from"
#define C_END6_TXT06    "your eyes as you stand"
#define C_END6_TXT07    "in defiance."
#define C_END6_TXT08    " "
#define C_END6_TXT09    "As the only marine to"
#define C_END6_TXT10    "endure the slaughter-"
#define C_END6_TXT11    "you decide to remain"
#define C_END6_TXT12    "in Hell and ensure no"
#define C_END6_TXT13    "demon ever rises again."
#define C_END6_TXT14    " "
#define C_END6_TXT15    "The End."

char *introcluster[] =   // [Immorpher] new intro text adapted from Doom 64 Manual and Doom 64 Reloaded
{
    C_INTRO_TXT01,
    C_INTRO_TXT02,
    C_INTRO_TXT03,
    C_INTRO_TXT04,
    C_INTRO_TXT05,
    C_INTRO_TXT06,
    C_INTRO_TXT07,
    C_INTRO_TXT08,
    C_INTRO_TXT09,
    C_INTRO_TXT10,
    C_INTRO_TXT11,
    C_INTRO_TXT12,
    C_INTRO_TXT13,
    C_INTRO_TXT14,
    C_INTRO_TXT15,
    C_INTRO_TXT16,
    C_INTRO_TXT17,
    C_INTRO_TXT18,
    C_INTRO_TXT19,
    C_INTRO_TXT20,
    T_NULL
};

char *endcluster1[] =   // 8005A2C0
{
    C_END1_TXT01,
    C_END1_TXT02,
    C_END1_TXT03,
    C_END1_TXT04,
    C_END1_TXT05,
    C_END1_TXT06,
    C_END1_TXT07,
    C_END1_TXT08,
    C_END1_TXT09,
    C_END1_TXT10,
    C_END1_TXT11,
    C_END1_TXT12,
    C_END1_TXT13,
    C_END1_TXT14,
    C_END1_TXT15,
    T_NULL
};

char *endcluster2[] =   // 8005A300
{
    C_END2_TXT01,
    C_END2_TXT02,
    C_END2_TXT03,
    C_END2_TXT04,
    C_END2_TXT05,
    C_END2_TXT06,
    C_END2_TXT07,
    C_END2_TXT08,
    C_END2_TXT09,
    C_END2_TXT10,
    C_END2_TXT11,
    T_NULL
};

char *endcluster3[] =   // 8005A330
{
    C_END3_TXT01,
    C_END3_TXT02,
    C_END3_TXT03,
    C_END3_TXT04,
    C_END3_TXT05,
    C_END3_TXT06,
    C_END3_TXT07,
    C_END3_TXT08,
    C_END3_TXT09,
    C_END3_TXT10,
    C_END3_TXT11,
    T_NULL
};

char *endcluster4[] =   // 8005A360
{
    C_END4_TXT01,
    C_END4_TXT02,
    C_END4_TXT03,
    C_END4_TXT04,
    C_END4_TXT05,
    C_END4_TXT06,
    C_END4_TXT07,
    C_END4_TXT08,
    C_END4_TXT09,
    C_END4_TXT10,
    C_END4_TXT11,
    T_NULL
};

char *endcluster5[] =   // 8005A390
{
    C_END5_TXT01,
    C_END5_TXT02,
    C_END5_TXT03,
    C_END5_TXT04,
    C_END5_TXT05,
    C_END5_TXT06,
    C_END5_TXT07,
    T_NULL
};

char *endcluster6[] =   // 8005A3B0
{
    C_END6_TXT01,
    C_END6_TXT02,
    C_END6_TXT03,
    C_END6_TXT04,
    C_END6_TXT05,
    C_END6_TXT06,
    C_END6_TXT07,
    C_END6_TXT08,
    C_END6_TXT09,
    C_END6_TXT10,
    C_END6_TXT11,
    C_END6_TXT12,
    C_END6_TXT13,
    C_END6_TXT14,
    C_END6_TXT15,
    T_NULL
};

//
// Character cast strings F_FINALE.C
//
#define CC_ZOMBIE   "Zombieman"
#define CC_SHOTGUN  "Shotgun Guy"
//#define CC_HEAVY  "Heavy Weapon Dude" // Enemy Removed
#define CC_IMP      "Imp"
#define CC_NIMP     "Nightmare Imp" // New Enemy on Doom64
#define CC_DEMON    "Bull Demon"
#define CC_SPECT    "Spectre"   // New Enemy on Doom64
#define CC_LOST     "Lost Soul"
#define CC_CACO     "Cacodemon"
#define CC_HELL     "Hell Knight"
#define CC_BARON    "Baron Of Hell"
#define CC_ARACH    "Arachnotron"
#define CC_PAIN     "Pain Elemental"
//#define CC_REVEN  "Revenant"  // Enemy Removed
#define CC_MANCU    "Mancubus"
//#define CC_ARCH   "Arch-Vile" // Enemy Removed
//#define CC_SPIDER "The Spider Mastermind" // Enemy Removed
#define CC_CYBER    "The Cyberdemon"
#define CC_MOTHER   "Mother Demon"
#define CC_HERO     "Our Hero"

//
// Final DOOM 2 animation
// Casting by id Software.
// in order of appearance
//
typedef struct
{
    char        *name;
    mobjtype_t  type;
} castinfo_t;

static castinfo_t   castorder[] = // 8005A3F0
{
    { CC_ZOMBIE, MT_POSSESSED1 },// MT_POSSESSED
    { CC_SHOTGUN, MT_POSSESSED2 },// MT_SHOTGUY
    //{ CC_HEAVY, MT_CHAINGUY },
    { CC_IMP, MT_IMP1 },// MT_TROOP
    { CC_NIMP, MT_IMP2 },// MT_TROOP2
    { CC_DEMON, MT_DEMON1 },// MT_SERGEANT
    { CC_SPECT, MT_DEMON2 },// MT_SERGEANT2
    { CC_LOST, MT_SKULL },// MT_SKULL
    { CC_CACO, MT_CACODEMON },// MT_HEAD
    { CC_HELL, MT_BRUISER2 },// MT_KNIGHT
    { CC_BARON, MT_BRUISER1 },// MT_BRUISER
    { CC_ARACH, MT_BABY },// MT_BABY
    { CC_PAIN, MT_PAIN },// MT_PAIN
    //{ CC_REVEN, MT_UNDEAD },
    { CC_MANCU, MT_MANCUBUS },// MT_FATSO
    //{ CC_ARCH, MT_VILE },
    //{ CC_SPIDER, MT_SPIDER },
    { CC_CYBER, MT_CYBORG },// MT_CYBORG
    { CC_MOTHER, MT_RESURRECTOR },// MT_CYBORG
    { CC_HERO, MT_PLAYER },// MT_PLAYER
    { NULL, 0 }
};

typedef enum
{
    F_STAGE_FADEIN_BACKGROUD,
    F_STAGE_DRAWTEXT,
    F_STAGE_SCROLLTEXT,
    F_STAGE_FADEOUT_BACKGROUD,
    F_STAGE_CAST
} finalestage_t;

static int textypos;            // 800631F0
static int textline;            // 800631F4
static char **text;             // 800631F8
static int textalpha;           // 800631FC

static boolean speed = false;

/*
=================
=
= F_StartIntermission
=
=================
*/

void F_StartIntermission(void) // 80002CD0
{
    if (nextmap == 1)
    {
        text = introcluster;
        textypos = 20;
    }
    else if ((gamemap == 8) && (nextmap == 9))
    {
        text = endcluster1;
        textypos = 15;
    }
    else if ((gamemap == 4) && (nextmap == 29))
    {
        text = endcluster2;
        textypos = 43;
    }
    else if ((gamemap == 12) && (nextmap == 30))
    {
        text = endcluster3;
        textypos = 43;
    }
    else if ((gamemap == 18) && (nextmap == 31))
    {
        text = endcluster4;
        textypos = 43;
    }
    else if ((gamemap == 1) && (nextmap == 32))
    {
        text = endcluster5;
        textypos = 71;
    }

    DrawerStatus = 2;
    textline = 0;
    textalpha = 0;
    S_StartMusic(mus_title); // [Immorpher] Play menu music for intermission
    speed = false;
}

/*
=================
=
= F_StopIntermission
=
=================
*/

void F_StopIntermission(int exit) // 80002E14
{
    S_StopMusic(); // [Immorpher] stop intermission music
    gamepaused = false;
    DrawerStatus = 0;
    I_WIPE_FadeOutScreen();
}

/*
=================
=
= F_TickerIntermission
=
=================
*/

int F_TickerIntermission(void) // 80002E44
{
    unsigned int buttons, oldbuttons, exit;

    gameaction = ga_nothing;
    P_CheckCheats();

    exit = gameaction;
    if (!gamepaused)
    {
        buttons = allticbuttons & 0xffff0000;
        oldbuttons = alloldticbuttons & 0xffff0000;

        exit = ga_nothing;

        if ((buttons != oldbuttons) && (buttons & (PAD_A|PAD_B)))
        {
            speed = true;
        }

        if(*text[textline])
        {
            textalpha += !speed ? 8 : 16;
            if (textalpha > 255)
            {
                textalpha = 0;
                textline++;
            }
        }
        else if ((buttons != oldbuttons) && (buttons & (PAD_A|PAD_B)))
        {
            exit = ga_exit;
        }

         // [Immorpher] Speed up text intermission by pressing buttons
        if (buttons & (PAD_A|PAD_B))
        {
            textalpha += 256;
        }
    }

    return exit;
}

/*
=================
=
= F_DrawerIntermission
=
=================
*/

void F_DrawerIntermission(void) // 80002F14
{
    int i, ypos;
    I_ClearFrame();
    I_ClearFB(0x000000ff);

    M_DrawBackground(63, 25, 128, "EVIL");

    ypos = textypos;
    for(i = 0; i < textline; i++)
    {
        if(runintroduction) {
            ST_Message(20, ypos, text[i], 0xc0c0c0ff);
            ypos += 10;
        } else {
            ST_DrawString(-1, ypos, text[i], 0xc0c0c0ff);
            ypos += 14;
        }
    }

    if(runintroduction) {
        ST_Message(20, ypos, text[i], textalpha | PACKRGBA(192, 192*textalpha/255, 192*textalpha/255, 0));
    } else {
        ST_DrawString(-1, ypos, text[i], textalpha | PACKRGBA(192, 192*textalpha/255, 192*textalpha/255, 0));
    }

    if (MenuCall)
    {
        M_DrawOverlay(0, 0, XResolution, YResolution, 96);
        MenuCall();
    }

    I_DrawFrame();
}

static finalestage_t    finalestage;    // 80063200
static int              castnum;        // 80063204
static int              casttics;       // 80063208
static state_t         *caststate;      // 8006320C
static boolean          castdeath;      // 80063210
static int              castframes;     // 80063214
static int              castonmelee;    // 80063218
static int              castrotation;   // 8006321C
static int              castfadein;     // 80063220
static int              fadeinout;      // 80063224

/*
=================
=
= F_Start/Cast_Start
=
=================
*/

void F_Start(void) // 8000313C
{
    DrawerStatus = 3;
    finalestage = F_STAGE_FADEIN_BACKGROUD;
    fadeinout = 0;
    textypos = 15;
    textline = 0;
    textalpha = 0;
    castnum = 0;
    caststate = &states[mobjinfo[castorder[castnum].type].seestate];
    casttics = states[mobjinfo[castorder[castnum].type].seestate].tics;
    castdeath = false;
    castframes = 0;
    castonmelee = 0;
    castrotation = 0;
    castfadein = 0;
    speed = false;

    S_StartMusic(mus_final);
}

/*
=================
=
= F_Stop/Cast_Stop
=
=================
*/

void F_Stop(int exit) // 80003220
{
    gamepaused = false;
    DrawerStatus = 0;
    S_StopMusic();
    I_WIPE_FadeOutScreen();
}

/*
=================
=
= F_Ticker/Cast_Ticker
=
=================
*/

int F_Ticker(void) // 80003258
{
    unsigned int buttons, oldbuttons;
    int st, sfx;

    buttons = allticbuttons = M_ButtonResponder(allticbuttons);
    oldbuttons = alloldticbuttons & 0xffff0000;

    gameaction = ga_nothing;
    P_CheckCheats();

    if (gamepaused != 0)
    {
        return gameaction;
    }

    if ((buttons != oldbuttons) && (buttons & ALL_BUTTONS))
    {
        speed = true;
    }

    switch(finalestage)
    {
        case F_STAGE_FADEIN_BACKGROUD:
            fadeinout += 6;
            if (fadeinout > 160)
            {
                fadeinout = 160;
                finalestage = F_STAGE_DRAWTEXT;
            }
            break;

        case F_STAGE_DRAWTEXT:
            if (*endcluster6[textline])
            {
                textalpha += !speed ? 8 : 16;
                if (textalpha > 255)
                {
                    textalpha = 0;
                    textline++;
                }
            }
            else
            {
                finalestage = F_STAGE_SCROLLTEXT;
            }
            break;

        case F_STAGE_SCROLLTEXT:
            textypos -= 1;
            if (textypos < -200)
            {
                finalestage = F_STAGE_FADEOUT_BACKGROUD;
            }
            break;

        case F_STAGE_FADEOUT_BACKGROUD:
            fadeinout -= 6;
            if (fadeinout < 0)
            {
                fadeinout = 0;
                finalestage = F_STAGE_CAST;
            }
            break;

        case F_STAGE_CAST:
            fadeinout += 6;
            if (fadeinout > 128)
            {
                fadeinout = 128;
            }

            if (castdeath == false)
            {
                if (buttons != oldbuttons)
                {
                    if (buttons & PAD_LEFT)
                    {
                        castrotation += 1;
                        if (castrotation > 7) {
                            castrotation = 0;
                        }
                    }
                    else if (buttons & PAD_RIGHT)
                    {
                        castrotation -= 1;
                        if (castrotation < 0) {
                            castrotation = 7;
                        }
                    }
                    else if (buttons & (ALL_CBUTTONS|ALL_TRIG|PAD_A|PAD_B))
                    {
                        S_StartSound(NULL, sfx_shotgun); // sfx_shotgn

                        /* go into death frame */
                        if (mobjinfo[castorder[castnum].type].deathsound)
                            S_StartSound(NULL, mobjinfo[castorder[castnum].type].deathsound);

                        caststate = &states[mobjinfo[castorder[castnum].type].deathstate];
                        castframes = 0;
                        castdeath = true;

                        if(castorder[castnum].type == MT_CYBORG) {
                            casttics = 10;
                        }
                        else {
                            casttics = caststate->tics;
                        }

                    }
                }
            }

            if (gametic > gamevbls)
            {
                if (castfadein < 192) {
                    castfadein += 2;
                }

                /* advance state*/
                if (--casttics > 0)
                    return ga_nothing;  /* not time to change state yet */

                if (castdeath && caststate->nextstate == S_NULL)
                {
                    /* switch from deathstate to next monster */
                    castrotation = 0;
                    castnum++;
                    castfadein = 0;
                    castdeath = false;

                    if (castorder[castnum].name == NULL)
                        castnum = 0;

                    if (mobjinfo[castorder[castnum].type].seesound)
                        S_StartSound(NULL, mobjinfo[castorder[castnum].type].seesound);

                    caststate = &states[mobjinfo[castorder[castnum].type].seestate];
                    castframes = 0;
                }

                st = caststate->nextstate;
                caststate = &states[st];

                if (castdeath == false)
                {
                    castframes++;

                    if (castframes == 12)
                    {   /* go into attack frame */
                        if (castonmelee)
                            caststate = &states[mobjinfo[castorder[castnum].type].meleestate];
                        else
                            caststate = &states[mobjinfo[castorder[castnum].type].missilestate];

                        castonmelee ^= 1;

                        if (caststate == &states[S_NULL])
                        {
                            if (castonmelee)
                                caststate = &states[mobjinfo[castorder[castnum].type].meleestate];
                            else
                                caststate = &states[mobjinfo[castorder[castnum].type].missilestate];
                        }
                    }

                    if (((castframes == 20) && (castorder[castnum].type == MT_MANCUBUS)) ||
                          castframes == 24 || caststate == &states[S_PLAY])
                    {
                        caststate = &states[mobjinfo[castorder[castnum].type].seestate];
                        castframes = 0;
                    }
                }

                casttics = caststate->tics;
                if (casttics == -1)
                    casttics = TICRATE;

                /* sound hacks.... */
                st = ((int)caststate - (int)states) / sizeof(state_t);
                switch (st)
                {
                    case S_PLAY_ATK2:
                        sfx = sfx_sht2fire; // sfx_dshtgn
                        break;

                    case S_SARG_ATK2:
                        sfx = sfx_sargatk; // sfx_sgtatk
                        break;

                    case S_FATT_ATK2:
                    case S_FATT_ATK4:
                    case S_FATT_ATK6:
                        sfx = sfx_bdmissile; // sfx_firsht
                        break;

                    case S_POSS_ATK2:
                        sfx = sfx_pistol;
                        break;

                    case S_SPOS_ATK2:
                        sfx = sfx_shotgun; // sfx_shotgn
                        break;

                    case S_TROO_MELEE1:
                        sfx = sfx_scratch; // sfx_claw
                        break;

                    case S_TROO_ATK1:
                    case S_HEAD_ATK1:
                    case S_BOSS_ATK1:
                    case S_BOS2_ATK1:
                        sfx = sfx_bdmissile; // sfx_firsht
                        break;

                    case S_SKULL_ATK2:
                        sfx = sfx_skullatk; // sfx_sklatk
                        break;

                    case S_BSPI_ATK2:
                        sfx = sfx_plasma; // sfx_plasma
                        break;

                    case S_CYBER_ATK1:
                    case S_CYBER_ATK3:
                    case S_CYBER_ATK5:
                        sfx = sfx_missile; // sfx_rlaunc
                        break;

                    case S_PAIN_ATK1:
                        sfx = sfx_skullatk; // sfx_sklatk
                        break;

                    //case S_VILE_ATK2:
                        //sfx = sfx_vilatk;
                        //break;

                    //case S_SKEL_FIST2:
                        //sfx = sfx_skeswg;
                        //break;

                    //case S_SKEL_FIST4:
                        //sfx = sfx_skepch;
                        //break;

                    //case S_SKEL_MISS2:
                        //sfx = sfx_skeatk;
                        //break;

                    //case S_CPOS_ATK2:
                    //case S_CPOS_ATK3:
                    //case S_CPOS_ATK4:
                        //sfx = sfx_pistol;
                        //break;

                    //case S_SPID_ATK2:
                    //case S_SPID_ATK3:
                        //sfx = sfx_pistol;
                        //break;

                    default:
                        sfx = 0;
                        break;
                }

                if (sfx)
                    S_StartSound(NULL, sfx);
            }

            break;

        default:
            break;
    }

    return ga_nothing;
}

/*
=================
=
= F_Drawer/Cast_Drawer
=
=================
*/
void F_Drawer(void) // 800039DC
{
    int i, type, alpha, ypos;

    I_ClearFrame();
    I_ClearFB(0x000000ff);

    switch(finalestage)
    {
        case F_STAGE_FADEIN_BACKGROUD:
        case F_STAGE_FADEOUT_BACKGROUD:
            M_DrawBackground(0, 0, fadeinout, "FINAL");
            break;

        case F_STAGE_DRAWTEXT:
        case F_STAGE_SCROLLTEXT:
            M_DrawBackground(0, 0, fadeinout, "FINAL");

            ypos = textypos;
            for(i = 0; i < textline; i++)
            {
                if (ypos >= 16)
                    alpha = 0xff;
                else if (ypos > 0)
                    alpha = ypos<<4;
                else
                    alpha = 0;
                ST_DrawString(-1, ypos, endcluster6[i], alpha | 0xc0c0c000);
                ypos += 14;
            }

            ST_DrawString(-1, ypos, endcluster6[i], textalpha | 0xc0c0c000);
            break;

        case F_STAGE_CAST:
            M_DrawBackground(63, 25, fadeinout, "EVIL");

            type = castorder[castnum].type;

            if (type == MT_DEMON2){
                alpha = 48;
            }
            else {
                alpha = mobjinfo[type].alpha;
            }

            F_DrawSprite(type, caststate, castrotation,
                         PACKRGBA(castfadein, castfadein, castfadein, alpha), 160, 190, FRACUNIT, 0);

            ST_DrawString(-1, 208, castorder[castnum].name, 0xC00000ff);
            break;

        default:
            break;
    }

    if (MenuCall)
    {
        M_DrawOverlay(0, 0, XResolution, YResolution, 96);
        MenuCall();
    }

    ST_DrawDebug();

    I_DrawFrame();
}
