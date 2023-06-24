/* DoomDef.h */

#define	VINT	int

/* ULTRA64 LIBRARIES */
#include <ultra64.h>
#include "ultratypes.h"
#include <PR/ramrom.h>	/* needed for argument passing into the app */
#include <assert.h>
#include <libaudio.h>
#include <stdarg.h>

#include "i_main.h"

/* TEST DEBUG */
#include "graph.h"

/* WESS SYSTEM */
#include "soundhw.h"
#include "seqload.h"
#include "wessapi.h"
#include "wessint_s.h"

/*-----------*/
/* SYSTEM IO */
/*-----------*/

extern u16 cfb[2][SCREEN_WD*SCREEN_HT]; // 8036A000

/*============================================================================= */

/* Fixes and Version Update Here*/

// NEWS Updates
// Nightmare Mode Originally Activated in the project [GEC] Master Edition.

// FIXES
#define FIX_LINEDEFS_DELETION   1   // Fixes for the 'linedef deletion' bug. From PsyDoom

/*============================================================================= */

/* all external data is defined here */
#include "doomdata.h"

/* header generated by multigen utility */
#include "doominfo.h"//"info.h"

#define MAXCHAR ((char)0x7f)
#define MAXSHORT ((short)0x7fff)
#define MAXINT	((int)0x7fffffff)	/* max pos 32-bit int */
#define MAXLONG ((long)0x7fffffff)

#define MINCHAR ((char)0x80)
#define MINSHORT ((short)0x8000)
#define MININT 	((int)0x80000000)	/* max negative 32-bit integer */
#define MINLONG ((long)0x80000000)

#ifndef NULL
#define	NULL	0
#endif

extern void R_RenderFilter(void);

extern int D_vsprintf(char *string, const char *format, va_list args);

/* c_convert.c  */
extern int LightGetHSV(int r,int g,int b); // 800020BC
extern int LightGetRGB(int h,int s,int v); // 8000248C

/*
===============================================================================

						GLOBAL TYPES

===============================================================================
*/

#define MAXPLAYERS	1   		/* D64 has one player */
#define TICRATE		30			/* number of tics / second */

#define	FRACBITS		16
#define	FRACUNIT		(1<<FRACBITS)

typedef int fixed_t;

#define	ANG45	0x20000000
#define	ANG90	0x40000000
#define	ANG180	0x80000000
#define	ANG270	0xc0000000
#define ANG5	0x38e0000   // (ANG90/18)
#define ANG1	0xb60000    // (ANG45/45)
typedef unsigned angle_t;

#define	FINEANGLES			8192
#define	FINEMASK			(FINEANGLES-1)
#define	ANGLETOFINESHIFT	19	/* 0x100000000 to 0x2000 */

//extern	fixed_t		finesine(5*FINEANGLES/4);
//extern	fixed_t		*finecosine;
/*
extern fixed_t D_abs(fixed_t x);
extern fixed_t finesine(int x);
extern fixed_t finecosine(int x);
extern angle_t tantoangle(int x);
*/
static inline fixed_t FixedDiv2(fixed_t a, fixed_t b)
{
    fixed_t flo;

    asm volatile(
    ".set noreorder\n\t"
    ".set nomacro\n\t"
    "dsll   %1, %1, 16\n\t"
    "ddiv   $0, %1, %2\n\t"
    "mflo   %0\n\t"
    ".set macro\n\t"
    ".set reorder"
    : "=r" (flo)
    : "r" (a), "r" (b)
    );

    return (fixed_t) flo;
}

static inline fixed_t FixedMul(fixed_t a, fixed_t b)
{
    fixed_t flo;

    asm volatile(
    ".set noreorder\n\t"
    ".set nomacro\n\t"
    "dmult   %1, %2\n\t"
    "mflo    %0\n\t"
    "dsra    %0, %0, 16\n\t"
    ".set macro\n\t"
    ".set reorder"
    : "=r" (flo)
    : "r" (a), "r" (b)
    );

    return (fixed_t) flo;
}

static inline fixed_t D_abs(fixed_t x)
{
    fixed_t _s = x >> 31;
    return (x ^ _s) - _s;
}

static inline fixed_t finesine(int x)
{
    x = x << 19;
    if ((x ^ (x << 1)) < 0)
        x = (1 << 31) - x;
    x = x >> 17;
    return x * (98304 - ((x * x) >> 11)) >> 13;
}

static inline fixed_t finecosine(int x) {
    return finesine(x + 2048);
}

static inline angle_t tantoangle(int x) {
    return ((angle_t)((-47*((x)*(x))) + (359628*(x)) - 3150270));
}

typedef enum
{
	sk_baby,
	sk_easy,
	sk_medium,
	sk_hard,
	sk_nightmare
} skill_t;

typedef enum
{
	ga_nothing,
	ga_died,
	ga_completed,
	ga_secretexit,// no used
	ga_warped,
	ga_exitdemo,
	//News
	//ga_recorddemo,// no used
	ga_timeout,
	ga_restart,
	ga_exit
} gameaction_t;

#define LASTLEVEL 34
#define TOTALMAPS 33

#ifdef DEVWARP
#define DEVWARP_ENABLED 1
#else
#define DEVWARP_ENABLED 0
#endif

/* */
/* library replacements */
/* */

void D_memmove(void *dest, void *src);
void D_memset (void *dest, int val, int count);
void D_memcpy (void *dest, void *src, int count);
void D_strncpy (char *dest, char *src, int maxcount);
int D_strncasecmp (char *s1, char *s2, int len);
void D_strupr(char *s);
int D_strlen(char *s);

/*
===============================================================================

							MAPOBJ DATA

===============================================================================
*/

struct mobj_s;

/* think_t is a function pointer to a routine to handle an actor */
typedef void (*think_t) ();

/* a latecall is a function that needs to be called after p_base is done */
typedef void (*latecall_t) (struct mobj_s *mo);

typedef struct thinker_s
{
	struct		thinker_s	*prev, *next;
	think_t		function;
} thinker_t;

struct player_s;

typedef struct mobj_s
{
/* info for drawing */
	fixed_t			x,y,z;

	struct subsector_s	*subsector;

	int				flags;
	struct player_s	*player;		/* only valid if type == MT_PLAYER */

	struct mobj_s	*prev, *next;
	struct mobj_s	*snext, *sprev;	/* links in sector (if needed) */
	struct mobj_s	*bnext, *bprev;	/* links in blocks (if needed) */

	struct mobj_s	*target;		/* thing being chased/attacked (or NULL) */
	struct mobj_s   *tracer;        /* Thing being chased/attacked for tracers. */

	angle_t			angle;
	VINT			sprite;				/* used to find patch_t and flip value */
	VINT			frame;				/* might be ord with FF_FULLBRIGHT */
	fixed_t			floorz, ceilingz;	/* closest together of contacted secs */
	fixed_t			radius, height;		/* for movement checking */
	fixed_t			momx, momy, momz;	/* momentums */

	mobjtype_t		type;
	mobjinfo_t		*info;				/* &mobjinfo[mobj->type] */
	VINT			tics;				/* state tic counter	 */
	state_t			*state;

	VINT			health;
	VINT			movedir;		/* 0-7 */
	VINT			movecount;		/* when 0, select a new dir */

									/* also the originator for missiles */
	VINT			reactiontime;	/* if non 0, don't attack yet */
									/* used by player to freeze a bit after */
									/* teleporting */
	VINT			threshold;		/* if >0, the target will be chased */
									/* no matter what (even if shot) */

    int             alpha;          /* [D64] alpha value */

	void            *extradata;     /* for latecall functions */

	latecall_t		latecall;		/* set in p_base if more work needed */

	int             tid;            /* [D64] tid value */
} mobj_t;

/* each sector has a degenmobj_t in it's center for sound origin purposes */
struct subsector_s;
typedef struct
{
	fixed_t			x,y,z;
    struct subsector_s	*subsec;	// Psx Doom / Doom 64 New
} degenmobj_t;

typedef struct laserdata_s
{
    fixed_t     x1, y1, z1;
    fixed_t     x2, y2, z2;
    fixed_t     slopex, slopey, slopez;
    fixed_t     distmax, dist;
    mobj_t      *marker;
    struct laserdata_s *next;
} laserdata_t;

typedef struct laser_s
{
    thinker_t   thinker;
    laserdata_t *laserdata;
    mobj_t      *marker;
} laser_t;

/* */
/* frame flags */
/* */
#define	FF_FULLBRIGHT	0x8000		/* flag in thing->frame */
#define FF_FRAMEMASK	0x7fff

/* */
/* mobj flags */
/* */
#define	MF_SPECIAL		1			/* call P_SpecialThing when touched */
#define	MF_SOLID		2
#define	MF_SHOOTABLE	4
#define	MF_NOSECTOR		8			/* don't use the sector links */
									/* (invisible but touchable)  */
#define	MF_NOBLOCKMAP	16			/* don't use the blocklinks  */
									/* (inert but displayable) */
#define	MF_AMBUSH		32
#define	MF_JUSTHIT		64			/* try to attack right back */
#define	MF_JUSTATTACKED	128			/* take at least one step before attacking */
#define	MF_SPAWNCEILING	256			/* hang from ceiling instead of floor */
//#define	MF_NOGRAVITY	512			/* don't apply gravity every tic */
#define	MF_GRAVITY	512			    /* apply gravity every tic */

/* movement flags */
#define	MF_DROPOFF		0x400		/* allow jumps from high places */
#define	MF_PICKUP		0x800		/* for players to pick up items */
#define	MF_NOCLIP		0x1000		/* player cheat */
#define	MF_TRIGDEATH	0x2000		/* [d64] trigger line special on death */
#define	MF_FLOAT		0x4000		/* allow moves to any height, no gravity */
#define	MF_TELEPORT		0x8000		/* don't cross lines or look at heights */
#define MF_MISSILE		0x10000		/* don't hit same species, explode on block */

#define	MF_DROPPED		0x20000		/* dropped by a demon, not level spawned */
#define	MF_TRIGTOUCH	0x40000		/* [d64] trigger line special on touch/pickup */
#define	MF_NOBLOOD		0x80000		/* don't bleed when shot (use puff) */
#define	MF_CORPSE		0x100000	/* don't stop moving halfway off a step */
#define	MF_INFLOAT		0x200000	/* floating to a height for a move, don't */
									/* auto float to target's height */
#define	MF_COUNTKILL	0x400000	/* count towards intermission kill total */
#define	MF_COUNTITEM	0x800000	/* count towards intermission item total */

#define	MF_SKULLFLY		0x1000000	/* skull in flight */
#define	MF_NOTDMATCH	0x2000000	/* don't spawn in death match (key cards) */

#define	MF_SEETARGET	0x4000000	/* is target visible? */

/* Doom 64 New Flags */
#define	MF_COUNTSECRET  0x8000000   /* [d64] Count as secret when picked up (for intermissions) */
#define	MF_RENDERLASER  0x10000000  /* [d64] Exclusive to MT_LASERMARKER only */
#define	MF_SHADOW       0x40000000  /* temporary player invisibility powerup. */
#define	MF_NOINFIGHTING 0x80000000  /* [d64] Do not switch targets */

//(val << 0 < 0) 0x80000000
//(val << 1 < 0) 0x40000000
//(val << 2 < 0) 0x20000000
//(val << 3 < 0) 0x10000000
//(val << 4 < 0) 0x8000000
//(val << 5 < 0) 0x4000000
//(val << 6 < 0) 0x2000000
//(val << 7 < 0) 0x1000000
//(val << 8 < 0) 0x800000
//(val << 9 < 0) 0x400000
//(val << a < 0) 0x200000
//(val << b < 0) 0x100000
//(val << c < 0) 0x80000
//(val << d < 0) 0x40000
//(val << e < 0) 0x20000
//(val << f < 0) 0x10000

/* Exclusive Psx Doom Flags */
//#define	MF_BLENDMASK1	0x10000000
//#define	MF_BLENDMASK2	0x20000000
//#define	MF_BLENDMASK3	0x40000000
//#define	MF_ALL_BLEND_MASKS  (MF_BLENDMASK1 | MF_BLENDMASK2 | MF_BLENDMASK3)

/*============================================================================= */
typedef enum
{
	PST_LIVE,			/* playing */
	PST_DEAD,			/* dead on the ground */
	PST_REBORN			/* ready to restart */
} playerstate_t;


/* psprites are scaled shapes directly on the view screen */
/* coordinates are given for a 320*200 view screen */
typedef enum
{
	ps_weapon,
	ps_flash,
	ps_flashalpha, // New Doom64
	NUMPSPRITES
} psprnum_t;

typedef struct
{
	state_t	*state;		/* a NULL state means not active */
	int		tics;
	int     alpha;
	fixed_t	sx, sy;
} pspdef_t;

typedef enum
{
	it_bluecard,
	it_yellowcard,
	it_redcard,
	it_blueskull,
	it_yellowskull,
	it_redskull,
	NUMCARDS
} card_t;

typedef enum
{
	wp_chainsaw,
	wp_fist,
	wp_pistol,
	wp_shotgun,
	wp_supershotgun,// [psx]
	wp_chaingun,
	wp_missile,
	wp_plasma,
	wp_bfg,
	wp_laser,       // [d64]
	NUMWEAPONS,
	wp_nochange
} weapontype_t;

typedef enum
{
	am_clip,		/* pistol / chaingun */
	am_shell,		/* shotgun */
	am_cell,		/* BFG / plasma / #$&%*/
	am_misl,		/* missile launcher */
	NUMAMMO,
	am_noammo		/* chainsaw / fist */
} ammotype_t;

typedef enum
{
    ART_FAST = 1,
    ART_DOUBLE = 2,
    ART_TRIPLE = 3,
} artifacts_t;

typedef struct
{
	ammotype_t	ammo;
	int			upstate;
	int			downstate;
	int			readystate;
	int			atkstate;
	int			flashstate;
} weaponinfo_t;

extern	weaponinfo_t	weaponinfo[NUMWEAPONS]; // 8005AD80

typedef enum
{
	pw_invulnerability,
	pw_strength,
	pw_invisibility,
	pw_ironfeet,
	pw_allmap,
	pw_infrared,
	NUMPOWERS
} powertype_t;

#define	INVULNTICS		(30*30)
#define	INVISTICS		(60*30)
#define	INFRATICS		(120*30)
#define	IRONTICS		(60*30)
#define	STRTICS		    (3*30)

#define	MSGTICS		    (5*30)

/*
================
=
= player_t
=
================
*/

typedef struct player_s
{
	mobj_t		*mo;
	playerstate_t	playerstate;

	fixed_t		forwardmove, sidemove;	/* built from ticbuttons */
	angle_t		angleturn;				/* built from ticbuttons */

	fixed_t		viewz;					/* focal origin above r.z */
	fixed_t		viewheight;				/* base height above floor for viewz */
	fixed_t		deltaviewheight;		/* squat speed */
	fixed_t		bob;					/* bounded/scaled total momentum */
	fixed_t     recoilpitch;            /* [D64] new*/

	int			health;					/* only used between levels, mo->health */
										/* is used during levels	 */
	int			armorpoints, armortype;	/* armor type is 0-2 */

	int			powers[NUMPOWERS];		/* invinc and invis are tic counters	 */
	boolean		cards[NUMCARDS];
	int         artifacts;              /* [d64]*/
	boolean		backpack;
	int			frags;					/* kills of other player */
	weapontype_t	readyweapon;
	weapontype_t	pendingweapon;		/* wp_nochange if not changing */
	boolean		weaponowned[NUMWEAPONS];
	int			ammo[NUMAMMO];
	int			maxammo[NUMAMMO];
	int			attackdown, usedown;	/* true if button down last tic */
	int			cheats;					/* bit flags */

	int			refire;					/* refired shots are less accurate */

	int			killcount, itemcount, secretcount;		/* for intermission */
	char		*message;				/* hint messages */
	char		*message1;				// [Immorpher] additional message levels
	char		*message2;				// [Immorpher] additional message levels
	char		*message3;				// [Immorpher] additional message levels
	int         messagetic;             /* messages tic countdown*/
	int         messagetic1;            // [Immorpher] message tic buffer
	int         messagetic2;            // [Immorpher] message tic buffer
	int         messagetic3;            // [Immorpher] message tic buffer
	unsigned int		messagecolor;			// [Immorpher] message color
	unsigned int		messagecolor1;			// [Immorpher] message color 1
	unsigned int		messagecolor2;			// [Immorpher] message color 2
	unsigned int		messagecolor3;			// [Immorpher] message color 3
	int			damagecount, bonuscount;/* for screen flashing */
	int			bfgcount;               /* for bfg screen flashing */
	mobj_t		*attacker;				/* who did damage (NULL for floors) */
	int			extralight;				/* so gun flashes light up areas */
	pspdef_t	psprites[NUMPSPRITES];	/* view sprites (gun, etc) */

	void		*lastsoundsector;		/* don't flood noise every time */

	int			automapx, automapy, automapscale, automapflags;

	int			turnheld;				/* for accelerative turning */
	int         onground;               /* [d64] */
} player_t;

#define CF_NOCLIP       1       // no use
#define CF_GODMODE      2
#define CF_ALLMAP       4
#define CF_DEBUG        64
#define CF_TEX_TEST     0x200
#define CF_SCREENSHOT 	0x400    // Original 0x1000
#define CF_LOCKMOSTERS  0x800
#define CF_WALLBLOCKING 0x1000
#define CF_WEAPONS      0x2000
#define CF_HEALTH       0x4000
#define CF_ALLKEYS      0x8000
#define CF_MACROPEEK    0x10000

#define CF_NOCOLORS     0x20000    // [GEC] NEW CHEAT CODE
#define CF_FULLBRIGHT   0x40000    // [GEC] NEW CHEAT CODE
#define CF_GAMMA		0x80000    // [Immorpher] NEW CHEAT CODE

#define	AF_LINES		1				/* automap active on lines mode */
#define	AF_SUBSEC		2               /* automap active on subsector mode */
#define	AF_FOLLOW		4

/*
===============================================================================

					GLOBAL VARIABLES

===============================================================================
*/

/*================================== */

extern int gamevbls;		            // 80063130 /* may not really be vbls in multiplayer */
extern int gametic;		                // 80063134
extern int ticsinframe;                 // 80063138 /* how many tics since last drawer */
extern int ticon;			            // 8006313C
extern int lastticon;                   // 80063140
extern int vblsinframe[MAXPLAYERS];	    // 80063144 /* range from 4 to 8 */
extern int ticbuttons[MAXPLAYERS];		// 80063148
extern int oldticbuttons[MAXPLAYERS];	// 8006314C

extern	boolean		gamepaused;

extern	int DrawerStatus;

//extern	int		maxlevel;			/* highest level selectable in menu (1-25) */

int MiniLoop ( void (*start)(void),  void (*stop)()
		,  int (*ticker)(void), void (*drawer)(void) );

int	G_Ticker (void);
void G_Drawer (void);
void G_RunGame (void);

/*================================== */


extern	gameaction_t	gameaction;

#define	SBARHEIGHT	32			/* status bar height at bottom of screen */

typedef enum
{
	gt_single,
	gt_coop,
	gt_deathmatch
} gametype_t;

//extern	gametype_t	netgame;

//extern	boolean		playeringame[MAXPLAYERS];
//extern	int			consoleplayer;		/* player taking events and displaying */
//extern	int			displayplayer;
extern	player_t	players[MAXPLAYERS];

extern	skill_t		gameskill;
extern	int			gamemap;
extern	int			nextmap;
extern	int			totalkills, totalitems, totalsecret;	/* for intermission *///80077d4c,80077d58,80077E18

//extern	mapthing_t	deathmatchstarts[10], *deathmatch_p;    //80097e4c,80077e8c
extern	mapthing_t	playerstarts[MAXPLAYERS];//800a8c60

/*
===============================================================================

					GLOBAL FUNCTIONS

===============================================================================
*/

short BigShort(short dat);
short LittleShort(short dat);
long LongSwap(long dat);

fixed_t	FixedMul (fixed_t a, fixed_t b);
fixed_t FixedDiv (fixed_t a, fixed_t b);
fixed_t FixedDiv2(fixed_t a, fixed_t b);

char *PrintFixed(char *buf, fixed_t d);

//extern fixed_t FixedMul2 (fixed_t a, fixed_t b);// ASM MIPS CODE
//extern fixed_t FixedDiv3 (fixed_t a, fixed_t b);// ASM MIPS CODE

#ifdef __BIG_ENDIAN__

#define	LONGSWAP(x)   (x)
#define	LITTLESHORT(x)  (x)
#define	BIGSHORT(x)     (x)

#else

#define	LONGSWAP(x)     LongSwap(x)
#define	LITTLESHORT(x)  LittleShort(x)
#define	BIGSHORT(x)     BigShort(x)

#endif // __BIG_ENDIAN__

/*----------- */
/*MEMORY ZONE */
/*----------- */
/* tags < 8 are not overwritten until freed */
#define	PU_STATIC		1			/* static entire execution time */
#define	PU_LEVEL		2			/* static until level exited */
#define	PU_LEVSPEC		4			/* a special thinker in a level */
/* tags >= 8 are purgable whenever needed */
#define	PU_PURGELEVEL	8
#define	PU_CACHE		16

#define	ZONEID	0x1d4a

typedef struct memblock_s
{
	int		size;           /* including the header and possibly tiny fragments */
	void    **user;         /* NULL if a free block */
	int     tag;            /* purgelevel */
	int     id;             /* should be ZONEID */
	int		lockframe;		/* don't purge on the same frame */
	struct memblock_s   *next;
	struct memblock_s	*prev;
	void    *gfxcache;      /* New on Doom64 */
} memblock_t;

typedef struct
{
	int		size;				/* total bytes malloced, including header */
	memblock_t	*rover;
	memblock_t	*rover2;        /* New on Doom64 */
	memblock_t	*rover3;        /* New on Doom64 */
	memblock_t	blocklist;		/* start / end cap for linked list */
} memzone_t;

extern	memzone_t	*mainzone;

void	Z_Init (void);
memzone_t *Z_InitZone (byte *base, int size);

void    Z_SetAllocBase(memzone_t *mainzone);
void 	*Z_Malloc2 (memzone_t *mainzone, int size, int tag, void *ptr);
void	*Z_Alloc2(memzone_t *mainzone, int size, int tag, void *user); // PsxDoom / Doom64
void    Z_Free2 (memzone_t *mainzone,void *ptr);

#define Z_Malloc(x,y,z) Z_Malloc2(mainzone,x,y,z)
#define Z_Alloc(x,y,z) Z_Alloc2(mainzone,x,y,z)
#define Z_Free(x) Z_Free2(mainzone,x)

void	Z_FreeTags(memzone_t *mainzone, int tag);
void    Z_Touch(void *ptr);
void	Z_CheckZone (memzone_t *mainzone);
void	Z_ChangeTag (void *ptr, int tag);
int 	Z_FreeMemory (memzone_t *mainzone);
void    Z_DumpHeap(memzone_t *mainzone);

/*------- */
/*WADFILE */
/*------- */

// New Doom64
typedef enum
{
	dec_none,
	dec_jag,
	dec_d64
} decodetype;

typedef struct
{
	int			filepos;					/* also texture_t * for comp lumps */
	int			size;
	char		name[8];
} lumpinfo_t;

typedef struct {
    void       *cache;
} lumpcache_t;

extern	lumpinfo_t	*lumpinfo;		/* points directly to rom image */
//extern	int			numlumps;
//extern	lumpcache_t	*lumpcache;
//extern	byte		*mapfileptr;	// psxdoom/d64
//extern	int			mapnumlumps;	// psxdoom/d64
//extern	lumpinfo_t  *maplump;		// psxdoom/d64

void	W_Init (void);

int     W_CheckNumForName (char *name, int hibit1, int hibit2);
int		W_GetNumForName (char *name);

int		W_LumpLength (int lump);
void	W_ReadLump (int lump, void *dest, decodetype dectype);

void	*W_CacheLumpNum (int lump, int tag, decodetype dectype);
void	*W_CacheLumpName (char *name, int tag, decodetype dectype);

void	W_OpenMapWad(int mapnum);
void    W_FreeMapLump(void);
int		W_MapLumpLength(int lump);
int		W_MapGetNumForName(char *name);
void	*W_GetMapLump(int lump);

/*---------*/
/* DECODES */
/*---------*/
void	DecodeD64(unsigned char *input, unsigned char *output);
void	DecodeJaguar(unsigned char *input, unsigned char *output);

/*------------*/
/* BASE LEVEL */
/*------------*/

/*--------*/
/* D_MAIN */
/*--------*/

void D_DoomMain (void);

/*------*/
/* GAME */
/*------*/

extern	boolean	demoplayback, demorecording;
extern	int		*demo_p, *demobuffer;

void G_InitNew (skill_t skill, int map, gametype_t gametype);
void G_InitSkill(skill_t skill); // [Immorpher] skill initialize
void G_CompleteLevel (void);
void G_RecordDemo (void);
int G_PlayDemoPtr (int skill, int map);

/*------*/
/* PLAY */
/*------*/

mobj_t *P_SpawnMapThing (mapthing_t *mthing);
void P_SetupLevel (int map, skill_t skill);
void P_Init (void);

void P_Start (void);
void P_Stop (int exit);
int P_Ticker (void);
void P_Drawer (void);

/*---------*/
/* IN_MAIN */
/*---------*/

void IN_Start (void);
void IN_Stop (void);
int IN_Ticker (void);
void IN_Drawer (void);

/*--------*/
/* M_MAIN */
/*--------*/

typedef void(*menufunc_t)(void);

typedef struct
{
	int	casepos;
	int x;
	int y;
} menuitem_t;

typedef struct
{
	menuitem_t *menu_item;
	int item_lines;
	menufunc_t menu_call;
	int cursor_pos;
} menudata_t;

extern menudata_t MenuData[8];      // 800A54F0
extern menuitem_t Menu_Game[5];     // 8005AAA4
extern int MenuAnimationTic;        // 800a5570
extern int cursorpos;               // 800A5574
extern int m_vframe1;               // 800A5578
extern menuitem_t *MenuItem;        // 800A5578
extern int itemlines;               // 800A5580
extern menufunc_t MenuCall;         // 800A5584

extern int linepos;                 // 800A5588
extern int text_alpha_change_value; // 800A558C
extern int MusicID;                 // 800A5590
extern int m_actualmap;             // 800A5594
extern int last_ticon;              // 800a5598

extern skill_t startskill;          // 800A55A0
extern int startmap;                // 800A55A4
extern int EnableExpPak;            // 800A55A8

//-----------------------------------------

extern int MenuIdx;                 // 8005A7A4
extern int text_alpha;              // 8005A7A8
extern int ConfgNumb;               // 8005A7AC
extern int Display_X;               // 8005A7B0
extern int Display_Y;               // 8005A7B4
extern boolean enable_messages;     // 8005A7B8
extern int HUDopacity;    			// [Immorpher] HUD 0pacity options
extern int SfxVolume;               // 8005A7C0
extern int MusVolume;               // 8005A7C4
extern int brightness;              // 8005A7C8
extern int M_SENSITIVITY;           // 8005A7CC
extern boolean FeaturesUnlocked;    // 8005A7D0
extern int MotionBob;				// [Immorpher] Motion Bob
extern int VideoFilter;				// [GEC & Immorpher] VideoFilter
extern boolean antialiasing;     	// [Immorpher] Anti-aliasing
extern boolean interlacing;     	// [Immorpher] Interlacing
extern boolean DitherFilter;     	// [Immorpher] Dither Filter
extern int ColorDither;     		// [Immorpher] Color Dither
extern int FlashBrightness;     	// [Immorpher] Strobe brightness adjustment
extern boolean Autorun;     		// [Immorpher] Autorun
extern boolean runintroduction; 	// [Immorpher] New introduction text
extern boolean StoryText; 			// [Immorpher] Enable story text
extern boolean MapStats; 			// [Immorpher] Enable automap statistics
extern int HUDmargin; 				// [Immorpher] HUD margin options
extern boolean ColoredHUD;     		// [Immorpher] Colored hud

int M_RunTitle(void); // 80007630

int M_ControllerPak(void); // 80007724
int M_ButtonResponder(int buttons); // 80007960

void M_AlphaInStart(void); // 800079E0
void M_AlphaOutStart(void); // 800079F8
int M_AlphaInOutTicker(void); // 80007A14
void M_FadeInStart(void); // 80007AB4
void M_FadeOutStart(int exitmode); // 80007AEC

void M_SaveMenuData(void); // 80007B2C
void M_RestoreMenuData(boolean alpha_in); // 80007BB8
void M_MenuGameDrawer(void); // 80007C48
int M_MenuTicker(void); // 80007E0C
void M_MenuClearCall(void); // 80008E6C

void M_MenuTitleDrawer(void); // 80008E7C
void M_FeaturesDrawer(void); // 800091C0
void M_VolumeDrawer(void); // 800095B4
void M_MovementDrawer(void); // 80009738
void M_VideoDrawer(void); // 80009884
void M_DisplayDrawer(void); // [Immorpher] new menu
void M_StatusHUDDrawer(void); // [Immorpher] new menu
void M_DefaultsDrawer(void); // [Immorpher] new menu
void M_CreditsDrawer(void); // [Immorpher] new menu

void M_DrawBackground(int x, int y, int color, char *name); // 80009A68
void M_DrawOverlay(int x, int y, int w, int h, int color); // 80009F58

int M_ScreenTicker(void); // 8000A0F8

void M_ControllerPakDrawer(void); // 8000A3E4

void M_SavePakStart(void); // 8000A6E8
void M_SavePakStop(void); // 8000A7B4
int M_SavePakTicker(void); // 8000A804
void M_SavePakDrawer(void); // 8000AB44

void M_LoadPakStart(void); // 8000AEEC
void M_LoadPakStop(void); // 8000AF8C
int M_LoadPakTicker(void); // 8000AFE4
void M_LoadPakDrawer(void); // 8000B270

int M_CenterDisplayTicker(void); // 8000B4C4
void M_CenterDisplayDrawer(void); // 8000B604

int M_ControlPadTicker(void); // 8000B694
void M_ControlPadDrawer(void); // 8000B988

/*----------*/
/* PASSWORD */
/*----------*/

extern char *passwordChar;      // 8005AC60
extern byte Passwordbuff[16];   // 800A55B0
extern boolean doPassword;      // 8005ACB8
extern int CurPasswordSlot;     // 8005ACBC

void M_EncodePassword(byte *buff);//8000BC10
int M_DecodePassword(byte *inbuff, int *levelnum, int *skill, player_t *player); // 8000C194
void M_PasswordStart(void); // 8000C710
void M_PasswordStop(void); // 8000C744
int M_PasswordTicker(void); // 8000C774
void M_PasswordDrawer(void); // 8000CAF0

/*--------*/
/* F_MAIN */
/*--------*/

void F_StartIntermission (void);
void F_StopIntermission (void);
int F_TickerIntermission (void);
void F_DrawerIntermission (void);

void F_Start (void);
void F_Stop (void);
int F_Ticker (void);
void F_Drawer (void);

void BufferedDrawSprite(int type, state_t *state, int rotframe, int color, int xpos, int ypos);

/*---------*/
/* AM_MAIN */
/*---------*/

void AM_Start (void);
void AM_Control (player_t *player);
void AM_Drawer (void);

/*-----------*/
/* D_SCREENS */
/*-----------*/

int D_RunDemo(char *name, skill_t skill, int map); // 8002B2D0
int D_TitleMap(void);           // 8002B358
int D_WarningTicker(void);      // 8002B3E8
void D_DrawWarning(void);       // 8002B430
int D_LegalTicker(void);        // 8002B5F8
void D_DrawLegal(void);         // 8002B644
int D_NoPakTicker(void);        // 8002B7A0
void D_DrawNoPak(void);         // 8002B7F4
void D_SplashScreen(void);      // 8002B988
int D_Credits(void);            // 8002BA34
int D_CreditTicker(void);       // 8002BA88
void D_CreditDrawer(void);      // 8002BBE4
void D_OpenControllerPak(void); // 8002BE28

/*--------*/
/* REFRESH */
/*--------*/

void R_RenderPlayerView (void);
void R_Init (void);
angle_t R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
struct subsector_s *R_PointInSubsector (fixed_t x, fixed_t y);

/*------*/
/* MISC */
/*------*/
typedef struct
{
	char *	name;
	int		MusicSeq;
} mapinfo_t;

extern mapinfo_t MapInfo[];

extern unsigned char rndtable[256];
int M_Random (void);
int P_Random (void);
int I_Random (void); // [Immorpher] new random table
void M_ClearRandom (void);
void M_ClearBox (fixed_t *box);
void M_AddToBox (fixed_t *box, fixed_t x, fixed_t y);

/*---------*/
/* S_SOUND */
/*---------*/

/* header generated by Dave's sound utility */
#include "sounds.h"

void S_Init(void);
void S_SetSoundVolume(int volume);
void S_SetMusicVolume(int volume);
void S_StartMusic(int mus_seq);
void S_StopMusic(void);
void S_PauseSound(void);
void S_ResumeSound(void);
void S_StopSound(mobj_t *origin,int seqnum);
void S_StopAll(void);
int S_SoundStatus(int seqnum);
void S_StartSound(mobj_t *origin, int sound_id);
int S_AdjustSoundParams(mobj_t *listener, mobj_t *origin, int* vol, int* pan);

/*--------*/
/* I_MAIN */
/*--------*/

/* vbi_wait_thread messages */
enum VID_MSG {
	VID_MSG_RSP = 666,
	VID_MSG_RDP = 667,
	VID_MSG_PRENMI = 668,
	VID_MSG_VBI = 669,
	VID_MSG_KICKSTART = 670,
};

extern OSTask *vid_task;   // 800A5244
extern u32 vid_side;       // 800A5248

extern u8 gamepad_bit_pattern; // 800A5260 // one bit for each controller

// Controller Pak
extern OSPfsState FileState[16];    // 800A52D8
extern s32 File_Num;   // 800A54D8
extern s32 Pak_Size;   // 800A54DC
extern u8 *Pak_Data;   // 800A54E0
extern s32 Pak_Memory; // 800A54E4

extern char Pak_Table[256]; // 8005A620
extern char Game_Name[16]; // 8005A790

extern boolean disabledrawing; // 8005A720
extern s32 vsync;              // 8005A724
extern s32 drawsync2;          // 8005A728
extern s32 drawsync1;          // 8005A72C
extern u32 NextFrameIdx;       // 8005A730
extern s32 FilesUsed;          // 8005A740

#define MAX_GFX 5120
#define MAX_VTX 3072
#define MAX_MTX 4

extern Gfx *GFX1;	// 800A4A00
extern Gfx *GFX2;	// 800A4A04

extern Vtx *VTX1;	// 800A4A08
extern Vtx *VTX2;	// 800A4A0C

extern Mtx *MTX1;	// 800A4A10
extern Mtx *MTX2;	// 800A4A14

void I_Start(void);  // 80005620
void I_IdleGameThread(void *arg); // 8000567C
void I_Main(void *arg); // 80005710
void I_SystemTicker(void *arg); // 80005730
void I_Init(void); // 80005C50

void I_Error(char *error, ...) __attribute__ ((format (printf, 1, 2)));
int I_GetControllerData(void); // 800060D0

void I_CheckGFX(void); // 800060E8
void I_ClearFrame(void); // 8000637C
void I_DrawFrame(void);  // 80006570
void I_GetScreenGrab(void); // 800066C0

void I_MoveDisplay(int x,int y); // 80006790

int I_CheckControllerPak(void); // 800070B0
int I_DeletePakFile(int filenumb); // 80007224
int I_SavePakFile(int filenumb, int flag, byte *data, int size); // 80007308
int I_ReadPakFile(void); // 800073B8
int I_CreatePakFile(void); // 800074D4

void I_WIPE_MeltScreen(void); // 80006964
void I_WIPE_FadeOutScreen(void); // 80006D34

/*---------*/
/* Doom 64 */
/*---------*/

/* DOOM 64 CUSTOM COMBINES */
#define G_CC_D64COMB01 TEXEL0, 0, SHADE, 0, 0, 0, 0, 1
#define G_CC_D64COMB02 0, 0, 0, SHADE, 0, 0, 0, 1
#define G_CC_D64COMB03 0, 0, 0, TEXEL0, TEXEL0, 0, PRIMITIVE, 0
#define G_CC_D64COMB04 TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0
#define G_CC_D64COMB05 1, 0, PRIMITIVE, 0, 1, 0, PRIMITIVE, 0
#define G_CC_D64COMB06 TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, 0
#define G_CC_D64COMB07 1, 0, PRIM_LOD_FRAC, TEXEL0, TEXEL0, 0, PRIMITIVE, 0
#define G_CC_D64COMB08 COMBINED, 0, SHADE, ENVIRONMENT, 0, 0, 0, COMBINED
#define G_CC_D64COMB09 TEXEL0, 0, PRIM_LOD_FRAC, ENVIRONMENT, 0, 0, 0, 1
#define G_CC_D64COMB10 TEXEL0, 0, PRIM_LOD_FRAC, ENVIRONMENT, 0, 0, 0, TEXEL0

#define G_CC_D64COMB11 PRIMITIVE, 0, TEXEL0, SHADE, 0, 0, 0, 0
#define G_CC_D64COMB12 COMBINED, 0, PRIM_LOD_FRAC, ENVIRONMENT, 0, 0, 0, 0

#define G_CC_D64COMB13 TEXEL0, 0, SHADE, 0, 0, 0, 0, 0
#define G_CC_D64COMB14 COMBINED, 0, PRIM_LOD_FRAC, ENVIRONMENT, 0, 0, 0, 0

#define G_CC_D64COMB15 1, 0, SHADE, 0, 0, 0, 0, 1
#define G_CC_D64COMB16 0, 0, 0, COMBINED, 0, 0, 0, COMBINED

#define G_CC_D64COMB17 1, 0, PRIM_LOD_FRAC, TEXEL0, TEXEL0, 0, PRIMITIVE, 0
#define G_CC_D64COMB18 COMBINED, 0, PRIMITIVE, ENVIRONMENT, 0, 0, 0, COMBINED

#define G_CC_D64COMB19 1, 0, TEXEL0, PRIMITIVE, 0, 0, 0, PRIMITIVE

/* DOOM 64 CUSTOM RENDER MODES */
#define	RM_XLU_SURF_CLAMP(clk)					\
	IM_RD | CVG_DST_CLAMP | FORCE_BL | ZMODE_OPA |		\
	GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)

#define	G_RM_XLU_SURF_CLAMP			RM_XLU_SURF_CLAMP(1)
#define	G_RM_XLU_SURF2_CLAMP		RM_XLU_SURF_CLAMP(2)


#define	RM_XLU_SURF_ADD(clk)					\
	IM_RD | CVG_DST_SAVE | FORCE_BL | ZMODE_OPA |		\
	GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1)

#define	G_RM_XLU_SURF_ADD		RM_XLU_SURF_ADD(1)
#define	G_RM_XLU_SURF2_ADD		RM_XLU_SURF_ADD(2)

#define	gDPSetPrimColorD64(pkt, m, l, rgba)				\
{									\
	Gfx *_g = (Gfx *)(pkt);						\
									\
	_g->words.w0 =	(_SHIFTL(G_SETPRIMCOLOR, 24, 8) | 		\
			 _SHIFTL(m, 8, 8) | _SHIFTL(l, 0, 8));		\
	_g->words.w1 =  (unsigned int)(rgba);		\
}

#define	gDPSetEnvColorD64(pkt, rgba)					\
            gDPSetColor(pkt, G_SETENVCOLOR, rgba)

#define	gDPSetFogColorD64(pkt, rgba)					\
            gDPSetColor(pkt, G_SETFOGCOLOR, rgba)

#define PACKRGBA(r,g,b,a)       (((r)<<24)|((g)<<16)|((b)<<8)|(a))

/*
U_JPAD  0x08000000
L_JPAD  0x02000000
R_JPAD  0x01000000
D_JPAD  0x04000000

C_UP    0x00080000
C_LEFT  0x00020000
C_RIGHT 0x00040000
C_DOWN  0x00010000

START_BUTTON   0x10000000
A_BUTTON       0x80000000
B_BUTTON       0x40000000

L_TRIG  0x00200000
R_TRIG  0x00100000
Z_TRIG  0x20000000

A_UP    0x53    MAX
A_LEFT  0xAD00  MAX
A_RIGHT 0x5300  MAX
A_DOWN  0xAD    MAX

#define CONT_A      0x8000 // 1
#define CONT_B      0x4000 // 2
#define CONT_G	    0x2000 // 4
#define CONT_START  0x1000 // 8
#define CONT_UP     0x0800 // 16
#define CONT_DOWN   0x0400 // 32
#define CONT_LEFT   0x0200 // 64
#define CONT_RIGHT  0x0100 // 128
//0x0080 256 -> NO_USED
//0x0040 512 -> NO_USED
#define CONT_L      0x0020 // 1024
#define CONT_R      0x0010 // 2048
#define CONT_E      0x0008 // 4096
#define CONT_D      0x0004 // 8192
#define CONT_C      0x0002 // 16384
#define CONT_F      0x0001 // 32768

#define A_BUTTON	CONT_A
#define B_BUTTON	CONT_B
#define L_TRIG		CONT_L
#define R_TRIG		CONT_R
#define Z_TRIG		CONT_G
#define START_BUTTON	CONT_START
#define U_JPAD		CONT_UP
#define L_JPAD		CONT_LEFT
#define R_JPAD		CONT_RIGHT
#define D_JPAD		CONT_DOWN
#define U_CBUTTONS	CONT_E
#define L_CBUTTONS	CONT_C
#define R_CBUTTONS	CONT_F
#define D_CBUTTONS	CONT_D

Swap Values
{
    R_JPAD 1
    L_JPAD 2
    D_JPAD 4
    U_JPAD 8

    START_BUTTON 16
    Z_TRIG 32
    B_BUTTON 64
    A_BUTTON 128

    R_CBUTTONS 256
    L_CBUTTONS 512
    D_CBUTTONS 1024
    U_CBUTTONS 2048

    R_TRIG 4096
    L_TRIG 8192

    Normal|Swap|Result

    0x2f000000  (47)    (Z_TRIG|U_JPAD|D_JPAD|L_JPAD|R_JPAD)
    0xe03f0000  (16352) (L_TRIG|R_TRIG|U_CBUTTONS|D_CBUTTONS|L_CBUTTONS|R_CBUTTONS|A_BUTTON|B_BUTTON|Z_TRIG)
    0xf0000     (3840)  (U_CBUTTONS|D_CBUTTONS|L_CBUTTONS|R_CBUTTONS)

    0xa000000   (10)    (U_JPAD|L_JPAD)
    0x5000000   (5)     (D_JPAD|R_JPAD)

    0xef3f0000  (16367) (L_TRIG|R_TRIG|U_CBUTTONS|D_CBUTTONS|L_CBUTTONS|R_CBUTTONS|A_BUTTON|B_BUTTON|Z_TRIG|U_JPAD|D_JPAD|L_JPAD|R_JPAD)

    0x30000     (768)   (L_CBUTTONS|R_CBUTTONS)

    0xef300000  (12527) (L_TRIG|R_TRIG|A_BUTTON|B_BUTTON|Z_TRIG|U_JPAD|D_JPAD|L_JPAD|R_JPAD)

    0xe03d0000  (15840) (L_TRIG|R_TRIG|U_CBUTTONS|D_CBUTTONS|R_CBUTTONS|A_BUTTON|B_BUTTON|Z_TRIG)
}

#define A_BUTTON	CONT_A
#define B_BUTTON	CONT_B
#define Z_TRIG		CONT_G
#define START_BUTTON	CONT_START
#define U_JPAD		CONT_UP
#define D_JPAD		CONT_DOWN
#define L_JPAD		CONT_LEFT
#define R_JPAD		CONT_RIGHT
#define L_TRIG		CONT_L
#define R_TRIG		CONT_R
#define U_CBUTTONS	CONT_E
#define D_CBUTTONS	CONT_D
#define L_CBUTTONS	CONT_C
#define R_CBUTTONS	CONT_F

//((int)(buttons << 0) < 0) A_BUTTON || CONT_A
//((int)(buttons << 1) < 0) B_BUTTON || CONT_B
//((int)(buttons << 2) < 0) Z_TRIG || CONT_G
//((int)(buttons << 3) < 0) START_BUTTON || CONT_START
//((int)(buttons << 4) < 0) U_JPAD || CONT_UP
//((int)(buttons << 5) < 0) D_JPAD || CONT_DOWN
//((int)(buttons << 6) < 0) L_JPAD || CONT_LEFT
//((int)(buttons << 7) < 0) R_JPAD || CONT_RIGHT
//((int)(buttons << 8) < 0) NO_USED
//((int)(buttons << 9) < 0) NO_USED
//((int)(buttons << a) < 0) L_TRIG || CONT_L
//((int)(buttons << b) < 0) R_TRIG || CONT_R
//((int)(buttons << c) < 0) U_CBUTTONS || CONT_E
//((int)(buttons << d) < 0) D_CBUTTONS || CONT_D
//((int)(buttons << e) < 0) L_CBUTTONS || CONT_C
//((int)(buttons << f) < 0) R_CBUTTONS || CONT_F
*/

/*
(val << 0 < 0) 0x80000000 PAD_A
(val << 1 < 0) 0x40000000 PAD_B
(val << 2 < 0) 0x20000000 PAD_Z_TRIG
(val << 3 < 0) 0x10000000 PAD_START
(val << 4 < 0) 0x08000000 PAD_UP
(val << 5 < 0) 0x04000000 PAD_DOWN
(val << 6 < 0) 0x02000000 PAD_LEFT
(val << 7 < 0) 0x01000000 PAD_RIGHT
(val << 8 < 0) 0x00800000 NO_USED
(val << 9 < 0) 0x00400000 NO_USED
(val << a < 0) 0x00200000 PAD_L_TRIG
(val << b < 0) 0x00100000 PAD_R_TRIG
(val << c < 0) 0x00080000 PAD_UP_C
(val << d < 0) 0x00040000 PAD_DOWN_C
(val << e < 0) 0x00020000 PAD_LEFT_C
(val << f < 0) 0x00010000 PAD_RIGHT_C
*/

/* CONTROL PAD */
#define PAD_RIGHT       0x01000000
#define PAD_LEFT        0x02000000
#define PAD_DOWN        0x04000000
#define PAD_UP          0x08000000
#define PAD_START       0x10000000
#define PAD_Z_TRIG      0x20000000
#define PAD_B           0x40000000
#define PAD_A           0x80000000
#define PAD_RIGHT_C     0x00010000
#define PAD_LEFT_C      0x00020000
#define PAD_DOWN_C      0x00040000
#define PAD_UP_C        0x00080000
#define PAD_R_TRIG      0x00100000

#define PAD_L_TRIG      0x00200000

#define ALL_JPAD        (PAD_UP|PAD_DOWN|PAD_LEFT|PAD_RIGHT)
#define ALL_CBUTTONS    (PAD_UP_C|PAD_DOWN_C|PAD_LEFT_C|PAD_RIGHT_C)
#define ALL_BUTTONS     (PAD_L_TRIG|PAD_R_TRIG|PAD_UP_C|PAD_DOWN_C|PAD_LEFT_C|PAD_RIGHT_C|PAD_A|PAD_B|PAD_Z_TRIG)
#define ALL_TRIG        (PAD_L_TRIG|PAD_R_TRIG|PAD_Z_TRIG)

typedef struct
{
	unsigned int BT_RIGHT;
	unsigned int BT_LEFT;
	unsigned int BT_FORWARD;
	unsigned int BT_BACK;
	unsigned int BT_ATTACK;
	unsigned int BT_USE;
	unsigned int BT_MAP;
	unsigned int BT_SPEED;
	unsigned int BT_STRAFE;
	unsigned int BT_STRAFELEFT;
	unsigned int BT_STRAFERIGHT;
	unsigned int BT_WEAPONBACKWARD;
	unsigned int BT_WEAPONFORWARD;
} buttons_t;

extern buttons_t *BT_DATA[MAXPLAYERS]; // 800A559C

typedef struct
{
	short compressed;
	short numpal;
	short width;
	short height;
} gfxN64_t;

typedef struct
{
	short id;
	short numpal;
	short wshift;
	short hshift;
} textureN64_t;

typedef struct
{
	unsigned short  tiles;      // 0
	short           compressed; // 2
	unsigned short  cmpsize;    // 4
	short           xoffs;      // 6
	short           yoffs;      // 8
	unsigned short  width;      // 10
	unsigned short  height;     // 12
	unsigned short  tileheight; // 14
} spriteN64_t;
