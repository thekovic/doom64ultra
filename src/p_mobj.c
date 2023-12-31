/* P_mobj.c */

#include "doomdef.h"
#include "p_local.h"
#include "config.h"

void G_PlayerReborn (int player);

extern void ST_InitEveryLevel(void);
extern void ST_UpdateFlash(void);

extern mobj_t *cameratarget;    // 800A5D70
extern mapthing_t *spawnlist;   // 800A5D74
extern int spawncount;          // 800A5D78
extern mapthing_t  *nightmarerespawnlist;
extern int nightmarerespawncount;

/*
===============
=
= P_SpawnMobj
=
===============
*/

mobj_t *P_SpawnMobj (fixed_t x, fixed_t y, fixed_t z, mobjtype_t type) // 80018a20
{
    mobj_t      *mobj;
    state_t     *st;
    mobjinfo_t  *info;

    mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);

    bzero(mobj,  sizeof (*mobj));
    info = &mobjinfo[type];

    mobj->type = type;
    mobj->info = info;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->flags = info->flags;
    mobj->health = info->spawnhealth;
    mobj->reactiontime = info->reactiontime;
    mobj->alpha = info->alpha;

    /* do not set the state with P_SetMobjState, because action routines can't */
    /* be called yet */
    st = &states[info->spawnstate];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    /* set subsector and/or block links */
    P_SetThingPosition (mobj);

    mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;
    if (z == ONFLOORZ)
        mobj->z = mobj->floorz;
    else if (z == ONCEILINGZ)
        mobj->z = mobj->ceilingz - mobj->info->height;
    else
        mobj->z = z;

    /* */
    /* link into the mobj list */
    /* */
    mobjhead.prev->next = mobj;
    mobj->next = (void*) &mobjhead;
    mobj->prev = mobjhead.prev;
    mobjhead.prev = mobj;

    if((mobj->flags & MF_COUNTKILL) != 0)
        totalkills++;

    if((mobj->flags & MF_COUNTITEM) != 0)
        totalitems++;

    return mobj;
}

/*
=================
=
= P_SpawnMapThing
=
= The fields of the mapthing should already be in host byte order
==================
*/

extern mobj_t*  checkthing;
extern int      testflags;
extern fixed_t  testradius;
extern fixed_t  testx;
extern fixed_t  testy;

extern boolean PB_CheckPosition(void);

mobj_t *P_SpawnMapThing (mapthing_t *mthing) // 80018C24
{
    int         i, bit = 0;
    mobj_t      *mobj;
    fixed_t     x,y,z;
    mobj_t      tmp_mobj;

    if (mthing->type == MAXPLAYERS)
    {
        playerstarts[0] = *mthing;
        return NULL;
    }

    bit = (1 << customskill.monster_counts) & 0x7;

    if (!(mthing->options & bit) )
        return NULL;

    /* find which type to spawn */
    for (i = 0; i < NUMMOBJTYPES; i++)
    {
        if (mthing->type == mobjinfo[i].doomednum)
            break;
    }

    if (i==NUMMOBJTYPES)
    {
        //I_Error ("P_SpawnMapThing: Unknown type %d at (%d, %d)",mthing->type , mthing->x, mthing->y);
        return NULL;
    }

    //
    // [d64] check if spawn is valid
    //
    if((mobjinfo[i].flags & MF_SOLID) != 0)
    {
        checkthing = &tmp_mobj;
        testflags = mobjinfo[i].flags;
        testradius = mobjinfo[i].radius;
        testx = mthing->x << FRACBITS;
        testy = mthing->y << FRACBITS;

        if(!PB_CheckPosition())
            return NULL;
    }

    // [d64]: queue mobj for spawning later
    if(mthing->options & MTF_SPAWN)
    {
        mthing->options &= ~MTF_SPAWN;
        D_memcpy(&spawnlist[spawncount], mthing, sizeof(mapthing_t));
        spawncount += 1;

        return NULL;
    }

    /* spawn it */
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    if (mobjinfo[i].flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else
        z = ONFLOORZ;

    mobj = P_SpawnMobj (x,y,z, i);
    mobj->z += (mthing->z << FRACBITS);
    mobj->angle = ANG45 * (mthing->angle/45);
    mobj->tid = mthing->tid;

    if (customskill.monster_respawns
            && (mobjinfo[i].flags & MF_COUNTKILL))
    {
        if ((mthing >= spawnlist && mthing < &spawnlist[spawncount])
                || (mthing >= nightmarerespawnlist && mthing < &nightmarerespawnlist[nightmarerespawncount]))
        {
            mobj->spawnpoint = mthing;
        }
        else
        {
            D_memcpy(&nightmarerespawnlist[nightmarerespawncount], mthing, sizeof(mapthing_t));
            mobj->spawnpoint = &nightmarerespawnlist[nightmarerespawncount];
            nightmarerespawncount += 1;
        }
    }

    if (mobj->tics > 0)
        mobj->tics = 1 + (P_Random () % mobj->tics);

    if (mthing->options & MTF_AMBUSH)
        mobj->flags |= MF_AMBUSH;

    if (mthing->options & MTF_ONTOUCH)
        mobj->flags |= MF_TRIGTOUCH;

    if (mthing->options & MTF_ONDEATH)
        mobj->flags |= MF_TRIGDEATH;

    if (mthing->options & MTF_NIGHTMARE) {
        mobj->health *= 2;
        mobj->flags |= MF_NIGHTMARE;
    }

    if (mthing->options & MTF_SECRET)
    {
        mobj->flags |= MF_COUNTSECRET;
        totalsecret++;
    }

    if ((mthing->options & MTF_NOINFIGHTING) || customskill.monster_infighting == infighting_never) // [Immorpher] No infighting on merciless difficulty!
        mobj->flags |= MF_NOINFIGHTING;

    return mobj;
}

/*
============
=
= P_SpawnPlayer
=
= Called when a player is spawned on the level
= Most of the player structure stays unchanged between levels
============
*/

void P_SpawnPlayer(/*mapthing_t *mthing*/) // 80018F94
{
    player_t    *p;
    fixed_t     x,y,z;
    mobj_t      *mobj;
    int levelnum;
    int skill;

    //if (!playeringame[mthing->type-1])
        //return;                       /* not playing */

    p = &players[0];

    if (p->playerstate == PST_REBORN)
        G_PlayerReborn(0);
    else
    {
        p->killcount = 0;
        p->secretcount = 0;
        p->itemcount = 0;
    }

    x = playerstarts[0].x << FRACBITS;
    y = playerstarts[0].y << FRACBITS;

    z = ONFLOORZ;
    mobj = P_SpawnMobj (x,y,z, MT_PLAYER);

    mobj->angle  = ANG45 * (playerstarts[0].angle/45);
    mobj->player = p;
    mobj->health = p->health;
    mobj->tid    = playerstarts[0].tid;
    mobj->z      = mobj->z + (playerstarts[0].z << FRACBITS);
    p->mo = mobj;
    p->playerstate = PST_LIVE;
    p->refire = 0;
    p->messages[0] = NULL;
    p->messages[1] = NULL;  // [Immorpher] reset messages
    p->messages[2] = NULL;  // [Immorpher] reset messages
    p->messagetics[0] = 0;
    p->messagetics[1] = 0;  // [Immorpher] reset messages
    p->messagetics[2] = 0;  // [Immorpher] reset messages
    p->damagecount = 0;
    p->bonuscount = 0;
    p->bfgcount = 0;
    p->viewheight = VIEWHEIGHT;
    p->automapscale = 850;
    p->viewz = mobj->z + VIEWHEIGHT;
    p->falltimer = 0;
    p->controls = &CurrentControls[0];
    p->controls2 = &CurrentControls2[0];
    p->config = &playerconfigs[0];
    p->pitch = 0;
    p->addfov = 0;
    p->weaponwheelalpha = 0;
    cameratarget = p->mo;

    // don't do anything immediately
    playerbuttons[0] = oldplayerbuttons[0] = BB_USE;

#ifdef DEVCHEATS
    if (p->cheats & (CF_WALLBLOCKING|CF_NOCLIP))
        p->mo->flags |= MF_NOCLIP;
    if (p->cheats & CF_WEAPONS)
        P_GiveAllWeapons(p);
    if (p->cheats & CF_ALLKEYS)
        P_GiveAllKeys(p);
    if (p->cheats & CF_ARTIFACTS)
        p->artifacts |= 1 | 2 | 4;
#endif

    P_SetupPsprites (0);        /* setup gun psprite     */

    if (doPassword != 0)
    {
        M_DecodePassword(Passwordbuff, &levelnum, &skill, p);
        doPassword = false;
    }
    else if (doLoadSave)
    {
        I_LoadProgress(&LevelSaveBuffer);
        doLoadSave = false;
    }

    P_UpdateWeaponWheel(p);
    p->weaponwheelpos = p->weaponwheeltarget;

    ST_InitEveryLevel();
    ST_UpdateFlash(); // ST_doPaletteStuff();
}

/*
===============
=
= P_RemoveMobj
=
===============
*/

void P_RemoveMobj (mobj_t *mobj) // 80019130
{
    /* unlink from sector and block lists */
    P_UnsetThingPosition (mobj);

    /* unlink from mobj list */
    mobj->next->prev = mobj->prev;
    mobj->prev->next = mobj->next;
    Z_Free (mobj);
}

/*
================
=
= P_SetMobjState
=
= Returns true if the mobj is still present
================
*/

boolean P_SetMobjState (mobj_t *mobj, statenum_t state) // 80019184
{
    state_t *st;

    if (state == S_NULL)
    {
        mobj->state = NULL;
        P_RemoveMobj (mobj);
        return false;
    }

    st = &states[state];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    if (st->action)     /* call action functions when the state is set */
        st->action (mobj);

    mobj->latecall = NULL;  /* make sure it doesn't come back to life... */

    return true;
}

/*============================================================================= */

/*
===============================================================================

                        GAME SPAWN FUNCTIONS

===============================================================================
*/

/*
================
=
= P_SpawnPuff
=
================
*/

extern fixed_t attackrange; // 800A5704

void P_SpawnPuff (fixed_t x, fixed_t y, fixed_t z) // 80019218
{
    mobj_t  *th;
    int rnd1, rnd2;

    rnd1 = P_Random();
    rnd2 = P_Random();

    z += ((rnd2-rnd1)<<10);
    th = P_SpawnMobj (x,y,z, MT_SMOKE_SMALL);
    th->momz = FRACUNIT;
    th->tics -= P_Random()&1;
    if (th->tics < 1)
        th->tics = 1;

/* don't make punches spark on the wall */
    if (attackrange == MELEERANGE)
        P_SetMobjState (th, S_SSMOKE3);
}


/*
================
=
= P_SpawnBlood
=
================
*/

void P_SpawnBlood (fixed_t x, fixed_t y, fixed_t z, int damage, mobj_t *source) // 800192B8
{
    mobj_t  *th;
    int i;

    for(i = 0; i < 3; i++)
    {
        x += ((P_Random()-P_Random())<<12);
        y += ((P_Random()-P_Random())<<12);
        z += ((P_Random()-P_Random())<<11);
        th = P_SpawnMobj (x,y,z, MT_BLOOD);
        P_SetBloodColor(th, source);
        th->momz = FRACUNIT*2;
        th->tics -= P_Random()&1;
        if (th->tics<1)
            th->tics = 1;
        if (damage <= 12 && damage >= 9)
            P_SetMobjState (th, S_BLOOD2);
        else if (damage < 9)
            P_SetMobjState (th, S_BLOOD3);
    }
}

void P_SetBloodColor (mobj_t *blood, mobj_t *source)
{
    if (blood != source)
        blood->alpha = MAX(source->alpha, 128);

    switch (source->type)
    {
    case MT_BRUISER1:
        blood->extradata = (void*)1;
        break;
    case MT_BRUISER2:
        blood->extradata = (void*)2;
        break;
    case MT_IMP2:
        blood->extradata = (void*)3;
        break;
    default:
        blood->extradata = (void*)0;
        break;
    }
}


// Certain functions assume that a mobj_t pointer is non-NULL,
// causing a crash in some situations where it is NULL.  Vanilla
// Doom did not crash because of the lack of proper memory
// protection. This function substitutes NULL pointers for
// pointers to a dummy mobj, to avoid a crash.

mobj_t *P_SubstNullMobj(mobj_t *mobj)
{
    if (mobj == NULL)
    {
        static mobj_t dummy_mobj;

        dummy_mobj.x = 0;
        dummy_mobj.y = 0;
        dummy_mobj.z = 0;
        dummy_mobj.flags = 0;

        mobj = &dummy_mobj;
    }

    return mobj;
}

/*
================
=
= P_SpawnMissile
=
================
*/

mobj_t *P_SpawnMissile (mobj_t *source, mobj_t *dest, fixed_t xoffs, fixed_t yoffs, fixed_t heightoffs, mobjtype_t type) // 80019410
{
    mobj_t      *th;
    angle_t     an = 0;
    int         dist;
    int         speed;
    fixed_t     x, y, z;
    int         rnd1, rnd2;
    int         vertspread; // [Immorpher] Vertical spread for merciless

    x = source->x + xoffs;
    y = source->y + yoffs;
    z = source->z + heightoffs;
    vertspread = 1; // Set to default 1 which is center of destination height

    th = P_SpawnMobj (x, y, z, type);
    if (th->info->seesound)
        S_StartSound (source, th->info->seesound);
    th->target = source;        /* where it came from */

        if ((type == MT_PROJ_BABY) || (type == MT_PROJ_DART)) /* no aim projectile */
            an = source->angle;
        else if (dest)
            an = R_PointToAngle2(x, y, dest->x, dest->y);

    if (dest && (dest->flags & MF_SHADOW))
        {
        rnd1 = P_Random();
        rnd2 = P_Random();
        an += ((rnd2 - rnd1) << 20);
        }

    if (customskill.monster_random_aim && type != MT_PROJ_DART) { // [Immorpher] randomize projectiles a bit for merciless

        vertspread = I_Random() % 3; // [Immorpher] Randomize vertical

        if (P_Random() <= 96) { // [Immorpher] Randomize horizontal
            rnd1 = I_Random();
            rnd2 = P_Random();
            an += (5*(rnd2 - rnd1)/4 << 20);
        }
    }

    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    speed = th->info->speed;

    if (source && source->flags & MF_NIGHTMARE)
    {
        th->flags |= MF_NIGHTMARE;
        speed *= 2;
    }

    th->momx = speed * finecosine(an);
    th->momy = speed * finesine(an);
    if (dest)
    {
        dist = P_AproxDistance (dest->x - x, dest->y - y);
        dist = dist / (th->info->speed << FRACBITS);
        if (dist < 1)
            dist = 1;
        th->momz = ((dest->z + (dest->height >> vertspread)) - z) / dist;
    }

    if (!P_TryMove(th, th->x, th->y))
        P_ExplodeMissile (th);

    return th;
}


extern const fixed_t crouchease[];
/*
================
=
= P_SpawnPlayerMissile
=
= Tries to aim at a nearby monster
================
*/

void P_SpawnPlayerMissile (mobj_t *source, mobjtype_t type) // 80019668
{
    mobj_t      *th;
    angle_t     an;
    fixed_t     x,y,z, slope = 0;
    int         speed;
    fixed_t     missileheight;
    int         offset;
    int         pitch = source->player->pitch;
    int         autoaim = source->player->config->autoaim;

    // [d64] adjust offset and height based on missile
    if(type == MT_PROJ_ROCKET)  // [Immorpher] Removed frac unit from here for calculations
    {
        missileheight = 42;
        offset = 30;
    }
    else if(type == MT_PROJ_PLASMA)
    {
        missileheight = 32;
        offset = 40;
    }
    else if(type == MT_PROJ_BFG)
    {
        missileheight = 32;
        offset = 30;
    }
    else
    {
        missileheight = 32;
        offset = 0;
    }

    if (source->player->crouchtimer)
        missileheight = FixedMul (missileheight, crouchease[source->player->crouchtimer]);

    missileheight = (missileheight + (((missileheight >> 1) * finesine(pitch >> ANGLETOFINESHIFT)) >> FRACBITS)) << FRACBITS;
    offset = (offset*finecosine(pitch >> ANGLETOFINESHIFT)) >> FRACBITS;

    /* */
    /* see which target is to be aimed at */
    /* */
    hittarget.type = ht_none;
    an = source->angle;
    if (autoaim)
    {
        slope = P_AimLineAttack (source, an, missileheight, 16*64*FRACUNIT);
        if (hittarget.type != ht_thing)
        {
            an += 1<<26;
            slope = P_AimLineAttack (source, an, missileheight, 16*64*FRACUNIT);
            if (hittarget.type != ht_thing)
            {
                an -= 2<<26;
                slope = P_AimLineAttack (source, an, missileheight, 16*64*FRACUNIT);
            }
            if (hittarget.type != ht_thing)
            {
                an = source->angle;
                slope = 0;
            }
        }
    }

    if (!autoaim || hittarget.type != ht_thing)
        slope = P_AimSlope(source->player);

    x = source->x;
    y = source->y;
    z = source->z;

    th = P_SpawnMobj (x,y,z+missileheight, type);
    if (th->info->seesound)
        S_StartSound (source, th->info->seesound);
    th->target = source;
    th->angle = an;

    speed = th->info->speed;

    th->momx = speed * finecosine(an>>ANGLETOFINESHIFT);
    th->momy = speed * finesine(an>>ANGLETOFINESHIFT);
    th->momz = speed * slope;

    x = source->x + (offset * finecosine(an>>ANGLETOFINESHIFT));
    y = source->y + (offset * finesine(an>>ANGLETOFINESHIFT));

    // [d64]: checking against very close lines?
    if((hittarget.type == ht_line && hittarget.frac <= 0xC80) || !P_TryMove(th, x, y))
        P_ExplodeMissile(th);
}


/*
===================
=
= P_ExplodeMissile
=
===================
*/

void P_ExplodeMissile (mobj_t *mo) // 800198B8
{
        if(!P_SetMobjState(mo, mobjinfo[mo->type].deathstate))
            return;

    mo->momx = mo->momy = mo->momz = 0;

    mo->tics -= P_Random()&1;
    if (mo->tics < 1)
        mo->tics = 1;

    mo->flags &= ~MF_MISSILE;
    if (mo->info->deathsound)
    {
        S_StopSound(mo, 0);
        S_StartSound(mo, mo->info->deathsound);
    }
}
