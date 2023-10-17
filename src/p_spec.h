/* P_spec.h */

#pragma once

#include "r_local.h"

/*
===============================================================================

                            P_SPEC

===============================================================================
*/

/* */
/*  Aim Camera */
/* */
typedef struct
{
    thinker_t   thinker;
    mobj_t      *viewmobj;
} aimcamera_t;

/* */
/*  move Camera */
/* */
typedef struct
{
    thinker_t   thinker;
    fixed_t     x;
    fixed_t     y;
    fixed_t     z;
    fixed_t     slopex;
    fixed_t     slopey;
    fixed_t     slopez;
    int         tic;
    int         current;
} movecamera_t;

/* */
/*  Fade Mobj in/out */
/* */
typedef struct
{
    thinker_t   thinker;
    mobj_t      *mobj;
    int         amount;
    int         destAlpha;
    int         flagReserve;
} fade_t;

/* */
/*  Fade Bright */
/* */
typedef struct
{
    thinker_t   thinker;
    int         factor;
} fadebright_t;

/* */
/*  Mobj Exp */
/* */
typedef struct
{
    thinker_t   thinker;
    int         delay;
    int         lifetime;
    int         delaydefault;
    mobj_t      *mobj;
} mobjexp_t;


/* */
/*  Quake */
/* */
typedef struct
{
    thinker_t   thinker;
    int         tics;
} quake_t;


/* */
/*  Animating textures and planes */
/* */
typedef struct
{
    int     basepic;
    int     picstart;
    int     picend;
    int     current;
    int     frame;
    int     tics;
    int     delay;
    int     delaycnt;
    boolean isreverse;
} anim_t;

/* */
/*  psx doom / doom64 exit delay */
/* */
typedef struct
{
    thinker_t  thinker;
    int tics;
    void(*finishfunc)(void);
}delay_t;

/* */
/*  source animation definition */
/* */
typedef struct
{
    int     delay;
    char    startname[9];
    int     frames;
    int     speed;
    boolean isreverse;
    boolean ispalcycle;
} animdef_t;

#define MAXANIMS        15  //[d64] is 15
extern  anim_t  anims[MAXANIMS], *lastanim;

extern card_t   MapBlueKeyType;
extern card_t   MapRedKeyType;
extern card_t   MapYellowKeyType;

/* */
/*  Animating line specials */
/* */
extern  line_t  **linespeciallist;
extern  int     numlinespecials;

/* */
/*  Animating sector specials */
/* */
extern  sector_t    **sectorspeciallist;
extern  int         numsectorspecials;

/*  Define values for map objects */
#define MO_TELEPORTMAN      14


/* at game start */
//void  P_InitPicAnims (void);

/* at map load */
void    P_SpawnSpecials (void) SEC_GAME;

/* every tic */
void    P_UpdateSpecials (void) SEC_GAME;

/* when needed */
boolean P_UseSpecialLine (line_t *line, mobj_t *thing) SEC_GAME;
//void  P_ShootSpecialLine ( mobj_t *thing, line_t *line);
//void P_CrossSpecialLine (line_t *line,mobj_t *thing);

void    P_PlayerInSpecialSector (player_t *player, sector_t *sec) SEC_GAME;

int     twoSided(int sector,int line) SEC_GAME;
sector_t *getSector(int currentSector,int line,int side) SEC_GAME;
side_t  *getSide(int currentSector,int line, int side) SEC_GAME;
fixed_t P_FindLowestFloorSurrounding(sector_t *sec) SEC_GAME;
fixed_t P_FindHighestFloorSurrounding(sector_t *sec) SEC_GAME;
fixed_t P_FindNextHighestFloor(sector_t *sec,int currentheight) SEC_GAME;
fixed_t P_FindLowestCeilingSurrounding(sector_t *sec) SEC_GAME;
fixed_t P_FindHighestCeilingSurrounding(sector_t *sec) SEC_GAME;
int     P_FindSectorFromLineTag(int tag,int start) SEC_GAME;
int     P_FindMinSurroundingLight(sector_t *sector,int max) SEC_GAME;
sector_t *getNextSector(line_t *line,sector_t *sec) SEC_GAME;

int P_FindLightFromLightTag(int tag,int start) SEC_GAME;
boolean P_ActivateLineByTag(int tag,mobj_t *thing) SEC_GAME;

/* */
/*  SPECIAL */
/* */
int EV_DoDonut(line_t *line);

/*
===============================================================================

                            P_LIGHTS

===============================================================================
*/

typedef struct
{
    thinker_t   thinker;
    sector_t    *sector;
    int         count;
    int         special;
} fireflicker_t;

typedef struct
{
    thinker_t   thinker;
    sector_t    *sector;
    int         count;
    int         special;
} lightflash_t;

typedef struct
{
    thinker_t   thinker;
    sector_t    *sector;
    int         count;
    int         maxlight;
    int         darktime;
    int         brighttime;
    int         special;
} strobe_t;

typedef enum
{
    PULSENORMAL,
    PULSESLOW,
    PULSERANDOM
} glowtype_e;

/*#define GLOWSPEED           2
#define STROBEBRIGHT        1
#define SUPERFAST           10
#define FASTDARK            15
#define SLOWDARK            30*/

typedef struct
{
    thinker_t   thinker;
    sector_t    *sector;
    glowtype_e  type;
    int         count;
    int         direction;
    int         minlight;
    int         maxlight;
    int         special;
} glow_t;

typedef struct
{
    thinker_t   thinker;
    sector_t    *sector;
    sector_t    *headsector;
    int         count;
    int         start;
    int         index;
    int         special;
} sequenceglow_t;

#define GLOWSPEED       2
#define STROBEBRIGHT    3
#define STROBEBRIGHT2   1
#define TURBODARK       4
#define FASTDARK        15
#define SLOWDARK        30

void    T_FireFlicker(fireflicker_t *flick) SEC_GAME;
void    P_SpawnFireFlicker(sector_t *sector) SEC_GAME;
void    T_LightFlash (lightflash_t *flash) SEC_GAME;
void    P_SpawnLightFlash (sector_t *sector) SEC_GAME;
void    T_StrobeFlash (strobe_t *flash) SEC_GAME;
void    P_SpawnStrobeFlash (sector_t *sector, int fastOrSlow) SEC_GAME;
void    P_SpawnStrobeAltFlash(sector_t *sector, int fastOrSlow) SEC_GAME;
int     EV_StartLightStrobing(line_t *line) SEC_GAME;
void    T_Glow(glow_t *g) SEC_GAME;
void    P_SpawnGlowingLight(sector_t *sector, glowtype_e type) SEC_GAME;


typedef enum
{
    mods_flags,
    mods_special,
    mods_flats,
    mods_lights,
} modifysector_t;

#define LIGHT_FLOOR     0
#define LIGHT_CEILING   1
#define LIGHT_THING     2
#define LIGHT_UPRWALL   3
#define LIGHT_LWRWALL   4

int P_ModifySectorColor(line_t* line, int index, int type) SEC_GAME;

#define SEQUENCELIGHTMAX 48

void T_SequenceGlow(sequenceglow_t *seq_g) SEC_GAME;
void P_SpawnSequenceLight(sector_t* sector, boolean first) SEC_GAME;

typedef struct
{
    thinker_t   thinker;
    sector_t    *sector;
    int         dest;
    int         src;
    int         r;
    int         g;
    int         b;
    int         inc;
} lightmorph_t;

void P_UpdateLightThinker(int destlight, int srclight) SEC_GAME;
void T_LightMorph(lightmorph_t *lt) SEC_GAME;
int P_ChangeLightByTag(int tag1, int tag2) SEC_GAME;
int P_DoSectorLightChange(int tag1,int tag2) SEC_GAME;

typedef struct
{
    thinker_t   thinker;
    sector_t    *sector;
    sector_t    *combiner;
    int         special;
} combine_t;
void P_CombineLightSpecials(sector_t *sector) SEC_GAME;

/*
===============================================================================

                            P_SWITCH

===============================================================================
*/
typedef struct
{
    char    name1[9];
    char    name2[9];
} switchlist_t;

typedef enum
{
    top,
    middle,
    bottom
} bwhere_e;

typedef struct
{
    side_t      *side;
    bwhere_e    where;
    int         btexture;
    int         btimer;
    line_t      *line;
} button_t;

#define MAXSWITCHES 50      /* max # of wall switches in a level */
#define MAXBUTTONS  16      /* 4 players, 4 buttons each at once, max. */
#define BUTTONTIME  30      /* 1 second */

extern  button_t    buttonlist[MAXBUTTONS];

void    P_ChangeSwitchTexture(line_t *line,int useAgain) SEC_GAME;
void    P_InitSwitchList(void) SEC_GAME;
void    P_StartSwitchSound(line_t *line, int sound_id) SEC_GAME;


/*
===============================================================================

                            P_PLATS

===============================================================================
*/
typedef enum
{
    down = -1,
    waiting = 0,
    up = 1,
    in_stasis = 2
} plat_e;

typedef enum
{
    perpetualRaise,
    raiseAndChange,
    raiseToNearestAndChange,
    downWaitUpStay,
    blazeDWUS,
    upWaitDownStay,
    blazeUWDS,
    customDownUp,
    customDownUpFast,
    customUpDown,
    customUpDownFast
} plattype_e;

typedef struct
{
    thinker_t   thinker;
    sector_t    *sector;
    fixed_t     speed;
    fixed_t     low;
    fixed_t     high;
    int         wait;
    int         count;
    plat_e      status;
    plat_e      oldstatus;
    boolean     crush;
    int         tag;
    plattype_e  type;
} plat_t;

#define PLATWAIT    3           /* seconds */
#define PLATSPEED   (FRACUNIT*2)
#define MAXPLATS    30

extern  plat_t  *activeplats[MAXPLATS];

void    T_PlatRaise(plat_t  *plat) SEC_GAME;
int     EV_DoPlat(line_t *line,plattype_e type,int amount) SEC_GAME;
void    P_AddActivePlat(plat_t *plat) SEC_GAME;
void    P_RemoveActivePlat(plat_t *plat) SEC_GAME;
int     EV_StopPlat(line_t *line) SEC_GAME;
void    P_ActivateInStasis(int tag) SEC_GAME;

/*
===============================================================================

                            P_DOORS

===============================================================================
*/
typedef enum
{
    Normal,
    Close30ThenOpen,
    DoorClose,
    DoorOpen,
    RaiseIn5Mins,
    BlazeRaise,
    BlazeOpen,
    BlazeClose
} vldoor_e;

typedef struct
{
    thinker_t   thinker;
    vldoor_e    type;
    sector_t    *sector;
    fixed_t     topheight;
    fixed_t     bottomheight;   // D64 new
    fixed_t     initceiling;    // D64 new
    fixed_t     speed;
    int         direction;      /* 1 = up, 0 = waiting at top, -1 = down */
    int         topwait;        /* tics to wait at the top */
                                /* (keep in case a door going down is reset) */
    int         topcountdown;   /* when it reaches 0, start going down */
} vldoor_t;

#define VDOORSPEED  FRACUNIT*2
#define VDOORWAIT   120

void    EV_VerticalDoor (line_t *line, mobj_t *thing) SEC_GAME;
boolean P_CheckKeyLock(line_t *line, mobj_t *thing) SEC_GAME;  // Psx Doom New
int     EV_DoDoor (line_t *line, vldoor_e  type) SEC_GAME;
void    T_VerticalDoor (vldoor_t *door) SEC_GAME;
void    P_SpawnDoorCloseIn30 (sector_t *sec) SEC_GAME;
void    P_SpawnDoorRaiseIn5Mins (sector_t *sec, int secnum) SEC_GAME;

/*
===============================================================================

                            P_CEILNG

===============================================================================
*/
typedef enum
{
    lowerToFloor,
    raiseToHighest,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise,
    silentCrushAndRaise,
    customCeiling,
    crushAndRaiseOnce,
    customCeilingToHeight,
    crushSlowTrapOnce,
    lowerToFloorSlow
} ceiling_e;

typedef struct
{
    thinker_t   thinker;
    ceiling_e   type;
    sector_t    *sector;
    fixed_t     bottomheight, topheight;
    fixed_t     speed;
    boolean     crush;
    int         direction;      /* 1 = up, 0 = waiting, -1 = down */
    int         tag;            /* ID */
    int         olddirection;
    boolean     instant;
} ceiling_t;

#define CEILSPEED       FRACUNIT*2
#define MAXCEILINGS     30

extern  ceiling_t   *activeceilings[MAXCEILINGS];

int     EV_DoCeiling (line_t *line, ceiling_e  type, fixed_t speed) SEC_GAME;
void    T_MoveCeiling (ceiling_t *ceiling) SEC_GAME;
void    P_AddActiveCeiling(ceiling_t *c) SEC_GAME;
void    P_RemoveActiveCeiling(ceiling_t *c) SEC_GAME;
int     EV_CeilingCrushStop(line_t  *line) SEC_GAME;
void    P_ActivateInStasisCeiling(line_t *line) SEC_GAME;

/*
===============================================================================

                            P_FLOOR

===============================================================================
*/
typedef enum
{
    lowerFloor,         /* lower floor to highest surrounding floor */
    lowerFloorToLowest, /* lower floor to lowest surrounding floor */
    turboLower,         /* lower floor to highest surrounding floor VERY FAST */
    raiseFloor,         /* raise floor to lowest surrounding CEILING */
    raiseFloorToNearest,    /* raise floor to next highest surrounding floor */
    raiseToTexture,     /* raise floor to shortest height texture around it */
    lowerAndChange,     /* lower floor to lowest surrounding floor and change */
                        /* floorpic */
    raiseFloor24,
    raiseFloor24AndChange,
    raiseFloorCrush,
    raiseFloorTurbo,        // [d64]: unused
    customFloor,
    customFloorToHeight,
    turboLower16Above,
    turboLower32Above,
    turboLower64Above,
    lower16AboveSlow
} floor_e;

typedef enum
{
    build8, // slowly build by 8
    turbo16 // quickly build by 16
} stair_e;

typedef struct
{
    thinker_t   thinker;
    floor_e     type;
    boolean     crush;
    sector_t    *sector;
    int         direction;
    int         newspecial;
    short       texture;
    fixed_t     floordestheight;
    fixed_t     speed;
    boolean     instant;
} floormove_t;

typedef struct
{
    thinker_t   thinker;
    sector_t    *sector;
    fixed_t     ceildest;
    fixed_t     flrdest;
    int         ceildir;
    int         flrdir;
} splitmove_t;

#define FLOORSPEED  FRACUNIT*3

typedef enum
{
    ok,
    crushed,
    pastdest,
    stop
} result_e;

result_e    T_MovePlane(sector_t *sector,fixed_t speed,
            fixed_t dest,boolean crush,int floorOrCeiling,int direction) SEC_GAME;

int     EV_BuildStairs(line_t *line, stair_e type) SEC_GAME;
int     EV_DoFloor(line_t *line,floor_e floortype,fixed_t speed) SEC_GAME;
int     EV_SplitSector(line_t *line, boolean sync) SEC_GAME;// New D64
void    T_MoveFloor(floormove_t *floor) SEC_GAME;
void    T_MoveSplitPlane(splitmove_t *split) SEC_GAME;// New D64

/*
===============================================================================

                            P_TELEPT

===============================================================================
*/
int     EV_Teleport( line_t *line,mobj_t *thing ) SEC_GAME;
int     EV_SilentTeleport( line_t *line,mobj_t *thing ) SEC_GAME;

/*
===============================================================================

                            P_MISC

===============================================================================
*/

void T_AimCamera(aimcamera_t *camera) SEC_GAME; // 8000DE60
int P_SetAimCamera(line_t *line, boolean aim) SEC_GAME; // 8000DF20
int EV_SpawnTrapMissile(line_t *line, mobj_t *target, mobjtype_t type) SEC_GAME; // 8000E02C
void P_SpawnDelayTimer(int tics, void(*action)()) SEC_GAME; // 8000E1CC
void T_CountdownTimer(delay_t *timer) SEC_GAME; // 8000E1CC
void P_ExitLevel(void) SEC_GAME; // 8000E220
void P_SecretExitLevel(int map) SEC_GAME; // 8000E25C
int P_ModifyLineFlags(line_t *line, int tag) SEC_GAME; // 8000E6BC
int P_ModifyLineData(line_t *line, int tag) SEC_GAME; // 8000E780
int P_ModifyLineTexture(line_t *line, int tag) SEC_GAME; // 8000E82C
int P_ModifySector(line_t *line, int tag, int type) SEC_GAME; // 8000E928
void T_FadeThinker(fade_t *fade) SEC_GAME; // 8000EACC
int EV_SpawnMobjTemplate(int tag, bool silent) SEC_GAME; // 8000EB8C
int EV_FadeOutMobj(int tag) SEC_GAME; // 8000ED08
void T_Quake(quake_t *quake) SEC_GAME; // 8000EDE8
void T_Combine(combine_t *combine) SEC_GAME;
void T_LaserThinker(laser_t *laser) SEC_GAME;
void P_SpawnQuake(int tics) SEC_GAME; // 8000EE7C
int P_RandomLineTrigger(line_t *line,mobj_t *thing) SEC_GAME; // 8000EEE0
void T_MoveCamera(movecamera_t *camera) SEC_GAME; // 8000F014
void P_SetMovingCamera(line_t *line) SEC_GAME; // 8000f2f8
void P_RefreshBrightness(void) SEC_GAME; // 8000f410
void P_SetLightFactor(int lightfactor) SEC_GAME; // 8000F458
void T_FadeInBrightness(fadebright_t *fb) SEC_GAME; // 8000f610
int P_ModifyMobjFlags(int tid, int flags) SEC_GAME; // 8000F674
int P_AlertTaggedMobj(int tid, mobj_t *activator) SEC_GAME; // 8000F6C4
void T_MobjExplode(mobjexp_t *exp) SEC_GAME; // 8000F76C

/*
===============================================================================

                            P_MACROS

===============================================================================
*/

typedef struct
{
    int tag;
    mobj_t *activator;
} macroactivator_t;

/* MACRO Variables */
extern macro_t     *activemacro;       // 800A6094
extern mobj_t      *macroactivator;    // 800A6098
extern line_t      macrotempline;      // 800A60A0
extern line_t      *macroline;         // 800A60EC
extern thinker_t   *macrothinker;      // 800A60F0
extern int         macrointeger;       // 800A60F4
extern macro_t     *restartmacro;      // 800A60F8
extern int         macrocounter;       // 800A60FC
extern macroactivator_t macroqueue[4]; // 800A6100
extern int         macroidx1;          // 800A6120
extern int         macroidx2;          // 800A6124
DEBUG_COUNTER(extern int activemacroidx);

int P_StartMacro(int macroindex, line_t *line, mobj_t *thing) SEC_GAME; // 80021088
int P_SuspendMacro(void) SEC_GAME; // 80021148
void P_ToggleMacros(int tag, boolean toggleon) SEC_GAME; // 80021214
void P_RunMacros(void) SEC_GAME; // 8002126C
void P_RestartMacro(line_t *line, int id) SEC_GAME; // 80021384
