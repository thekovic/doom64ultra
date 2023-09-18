/* P_local.h */

#pragma once

#include "r_local.h"
#include "a_local.h"

#define	FLOATSPEED		(FRACUNIT*4)

#define	GRAVITY			(FRACUNIT*4)    //like JagDoom
#define	MAXMOVE			(16*FRACUNIT)


#define	MAXHEALTH			100
#define	VIEWHEIGHT			(56*FRACUNIT) //  D64 change to 41

/* mapblocks are used to check movement against lines and things */
#define MAPBLOCKUNITS	128
#define	MAPBLOCKSIZE	(MAPBLOCKUNITS*FRACUNIT)
#define	MAPBLOCKSHIFT	(FRACBITS+7)
#define	MAPBMASK		(MAPBLOCKSIZE-1)
#define	MAPBTOFRAC		(MAPBLOCKSHIFT-FRACBITS)


/* player radius for movement checking */
#define	PLAYERRADIUS	16*FRACUNIT

/* MAXRADIUS is for precalculated sector block boxes */
/* the spider demon is larger, but we don't have any moving sectors */
/* nearby */
#define	MAXRADIUS		32*FRACUNIT


#define	USERANGE		(70*FRACUNIT)
#define	MELEERANGE		(80*FRACUNIT)
#define	MISSILERANGE	(32*64*FRACUNIT)


typedef enum
{
	DI_EAST,
	DI_NORTHEAST,
	DI_NORTH,
	DI_NORTHWEST,
	DI_WEST,
	DI_SOUTHWEST,
	DI_SOUTH,
	DI_SOUTHEAST,
	DI_NODIR,
	NUMDIRS
} dirtype_t;

#define	BASETHRESHOLD	90		/* follow a player exlusively for 3 seconds */



/*
===============================================================================

							P_TICK

===============================================================================
*/

extern	thinker_t	thinkercap;	/* both the head and tail of the thinker list */

void P_InitThinkers (void);
void P_AddThinker (thinker_t *thinker) HOT;
void P_RemoveThinker (thinker_t *thinker) HOT;

/*
===============================================================================

							P_PSPR

===============================================================================
*/

void P_SetupPsprites (int curplayer) SEC_GAME; //(player_t *curplayer);
void P_MovePsprites (player_t *curplayer) SEC_GAME;

void P_SetPsprite (player_t *player, int position, statenum_t stnum) SEC_GAME;
void P_BringUpWeapon (player_t *player) SEC_GAME;
void P_DropWeapon (player_t *player) SEC_GAME;

/*
===============================================================================

							P_USER

===============================================================================
*/

void	P_PlayerThink (player_t *player) SEC_GAME;


/*
===============================================================================

							P_MOBJ

===============================================================================
*/

typedef struct {
    mobj_t *prev, *next;
} mobjhead_t;

extern	mobjhead_t	mobjhead;

DEBUG_COUNTER(extern int activethinkers);
DEBUG_COUNTER(extern int activemobjs);

#define ONFLOORZ	MININT
#define	ONCEILINGZ	MAXINT

mobj_t *P_SpawnMobj (fixed_t x, fixed_t y, fixed_t z, mobjtype_t type) HOT;

void 	P_RemoveMobj (mobj_t *th) HOT;
mobj_t* P_SubstNullMobj (mobj_t* th) SEC_GAME;
boolean	P_SetMobjState (mobj_t *mobj, statenum_t state) SEC_GAME;
void 	P_MobjThinker (mobj_t *mobj) HOT;

void	P_SpawnPuff (fixed_t x, fixed_t y, fixed_t z) SEC_GAME;
void 	P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, int damage) SEC_GAME;
//mobj_t *P_SpawnMissile (mobj_t *source, mobj_t *dest, mobjtype_t type);
mobj_t *P_SpawnMissile (mobj_t *source, mobj_t *dest, fixed_t xoffs, fixed_t yoffs, fixed_t heightoffs, mobjtype_t type) HOT;

void	P_SpawnPlayerMissile (mobj_t *source, mobjtype_t type) HOT;

void	P_RunMobjBase (void) HOT;//P_RunMobjBase2 (void);
void	P_RunMobjExtra (void) HOT;

void L_SkullBash (mobj_t *mo) SEC_GAME;
void L_MissileHit (mobj_t *mo) SEC_GAME;
void L_CrossSpecial (mobj_t *mo) SEC_GAME;

void P_ExplodeMissile (mobj_t *mo) SEC_GAME;

/*
===============================================================================

							P_ENEMY

===============================================================================
*/

void A_MissileExplode (mobj_t *mo) SEC_GAME;
void A_SkullBash (mobj_t *mo) SEC_GAME;

/*
===============================================================================

							P_MAPUTL

===============================================================================
*/

/*typedef struct
{
	fixed_t	x,y, dx, dy;
} divline_t;*/

typedef struct
{
	fixed_t     frac;
    boolean     isaline;
    union {
		line_t  *line;
		mobj_t  *thing;
	} d;//8
} intercept_t;

typedef boolean(*traverser_t)(intercept_t *in);


fixed_t P_AproxDistance (fixed_t dx, fixed_t dy) HOT;
int 	P_PointOnLineSide (fixed_t x, fixed_t y, line_t *line) HOT;
int 	P_PointOnDivlineSide (fixed_t x, fixed_t y, divline_t *line) HOT;
void 	P_MakeDivline (line_t *li, divline_t *dl) HOT;
fixed_t P_InterceptVector (divline_t *v2, divline_t *v1) HOT;
int 	P_BoxOnLineSide (fixed_t *tmbox, line_t *ld) HOT;
boolean P_CheckUseHeight(line_t *line) SEC_GAME;

extern	fixed_t opentop, openbottom, openrange;//,,800A5748
extern	fixed_t	lowfloor;
void 	P_LineOpening (line_t *linedef) SEC_GAME;

boolean P_BlockLinesIterator (int x, int y, boolean(*func)(line_t*) ) HOT;
boolean P_BlockThingsIterator (int x, int y, boolean(*func)(mobj_t*) ) HOT;

extern	divline_t 	trace;  // 800A5D58

#define PT_ADDLINES         1
#define PT_ADDTHINGS        2
#define PT_EARLYOUT         4

#define MAXINTERCEPTS       128

boolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2, int flags, boolean(*trav)(intercept_t *)) HOT;
boolean PIT_AddLineIntercepts(line_t* ld) HOT; // 80018574
boolean PIT_AddThingIntercepts(mobj_t* thing) HOT; // 8001860C
fixed_t P_InterceptLine(line_t *line, divline_t *trace) HOT; // 8001872C
boolean P_TraverseIntercepts(traverser_t func, fixed_t maxfrac) HOT;

/*
===============================================================================

							P_MAP

===============================================================================
*/

extern	boolean		floatok;				/* if true, move would be ok if */  //80077ea8
extern	fixed_t		tmfloorz, tmceilingz;	/* within tmfloorz - tmceilingz */  //80078010, 80077d30

extern	line_t	*specialline;//80077dc8
extern	mobj_t	*movething;


boolean P_CheckPosition (mobj_t *thing, fixed_t x, fixed_t y) HOT;
boolean P_TryMove (mobj_t *thing, fixed_t x, fixed_t y) HOT;
boolean P_CheckSight (mobj_t *t1, mobj_t *t2) HOT;
void 	P_UseLines (player_t *player) SEC_GAME;

boolean P_ChangeSector (sector_t *sector, boolean crunch) SEC_GAME;

fixed_t P_AimSlope (player_t *player) HOT;

typedef enum
{
    ht_none,
    ht_line,
    ht_thing,
    ht_floor,
    ht_ceiling,
} hittype_e;

typedef struct
{
    fixed_t x, y, z;
    hittype_e type;
    boolean   hitsky;
    union {
        line_t   *line;
        mobj_t   *thing;
        sector_t *sector;
    };
    int frac;
} hit_t;

extern boolean spawnpuff;
extern hit_t hittarget;

fixed_t P_AimLineAttack (mobj_t *t1, angle_t angle, fixed_t zheight, fixed_t distance) SEC_GAME;

void P_LineAttack (mobj_t *t1, angle_t angle, fixed_t zheight, fixed_t distance, fixed_t slope, int damage) SEC_GAME;
void P_RadiusAttack (mobj_t *spot, mobj_t *source, int damage) SEC_GAME;

/*
===============================================================================

							P_SETUP

===============================================================================
*/

extern	byte		*rejectmatrix;			/* for fast sight rejection */
extern	short		*blockmaplump;		/* offsets in blockmap are from here */
extern	short		*blockmap;
extern	int			bmapwidth, bmapheight;	/* in mapblocks */
extern	fixed_t		bmaporgx, bmaporgy;		/* origin of block map */
extern	mobj_t		**blocklinks;			/* for thing chains */

/*
===============================================================================

							P_INTER

===============================================================================
*/

extern	int		maxammo[NUMAMMO];
extern	int		clipammo[NUMAMMO];

void P_TouchSpecialThing (mobj_t *special, mobj_t *toucher) SEC_GAME;

void P_DamageMobj (mobj_t *target, mobj_t *inflictor, mobj_t *source, int damage) HOT;

void P_GiveAllWeapons (player_t *player) SEC_GAME;
void P_GiveAllKeys (player_t *player) SEC_GAME;

#include "p_spec.h"

/*
===============================================================================

							P_MOVE

===============================================================================
*/

//PSX NEW
#define MAXTHINGSPEC 8
extern line_t  *thingspec[8];
extern int		numthingspec;//80077ee8

extern mobj_t  *tmthing;
extern fixed_t  tmx, tmy;
extern boolean  checkposonly;

void	P_TryMove2(void) HOT;
int     PM_PointOnLineSide(fixed_t x, fixed_t y, line_t *line) SEC_GAME;
void 	P_UnsetThingPosition (mobj_t *thing) HOT;
void	P_SetThingPosition (mobj_t *thing) HOT;
void	PM_CheckPosition(void) HOT;
boolean PM_BoxCrossLine(line_t *ld) HOT;
boolean PIT_CheckLine(line_t *ld) HOT;
boolean PIT_CheckThing(mobj_t *thing) HOT;


/*
===============================================================================

							P_SHOOT

===============================================================================
*/

void P_Shoot2(void) SEC_GAME;
boolean PA_DoIntercept(void *value, boolean isline, int frac) SEC_GAME;
boolean	PA_ShootLine(line_t *li, fixed_t interceptfrac) SEC_GAME;
boolean PA_ShootThing(mobj_t *th, fixed_t interceptfrac) SEC_GAME;
fixed_t PA_SightCrossLine(line_t *line) SEC_GAME;
boolean PA_CrossSubsector(subsector_t *sub) SEC_GAME;
int PA_DivlineSide(fixed_t x, fixed_t y, divline_t *line) SEC_GAME;
boolean PA_CrossBSPNode(int bspnum) SEC_GAME;

/*
===============================================================================

							P_SIGHT

===============================================================================
*/

void P_CheckSights(void) HOT;
boolean P_CheckSight(mobj_t *t1, mobj_t *t2) HOT;
boolean PS_CrossBSPNode(int bspnum) HOT;
boolean PS_CrossSubsector(subsector_t *sub) HOT;
fixed_t PS_SightCrossLine (line_t *line) HOT;

