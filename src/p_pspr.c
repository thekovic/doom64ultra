/* P_pspr.c */

#include "doomdef.h"
#include "p_local.h"

extern void P_Thrust (player_t *player, angle_t angle, fixed_t move);

#define LOWERSPEED      FRACUNIT*7
#define RAISESPEED      FRACUNIT*7

#define WEAPONX         (0)*FRACUNIT
#define WEAPONBOTTOM    (96)*FRACUNIT   //old 128
#define WEAPONTOP       (0)*FRACUNIT    //old 32

#define RECOILPITCH     0x2AA8000

#define BFGCELLS        40          /* plasma cells for a bfg attack */

#define LASERRANGE          (4096*FRACUNIT)
#define LASERAIMHEIGHT      (40*FRACUNIT)
#define LASERDISTANCE       (30)

extern int ArtifactLookupTable[8];

/*============================================================================= */

int     ticremainder[MAXPLAYERS]; // 800A5E70

/*
==================
=
= P_SetupPsprites
=
= Called at start of level for each player
==================
*/

void P_SetupPsprites (int curplayer) // 8001B0D0
{
    int i;
    player_t *player;

    ticremainder[curplayer] = 0;
    player = &players[curplayer];

    /* remove all psprites */

    for (i=0 ; i<NUMPSPRITES ; i++)
    {
        player->psprites[i].state = NULL;
        player->psprites[i].alpha = 255;
    }

    /* spawn the gun */
    player->pendingweapon = player->readyweapon;
    P_BringUpWeapon (player);
}

/*
==================
=
= P_MovePsprites
=
= Called every tic by player thinking routine
==================
*/

void P_MovePsprites (player_t *player) // 8001B14C
{
    int         i;
    pspdef_t    *psp;
    state_t     *state;

    ticremainder[0] += vblsinframe[0];

    while (ticremainder[0] >= 2)
    {
        ticremainder[0] -= 2;

        psp = &player->psprites[0];
        for (i=0 ; i<NUMPSPRITES ; i++, psp++)
        {
            if ( (state = psp->state) != 0)     /* a null state means not active */
            {
            /* drop tic count and possibly change state */
                if (psp->tics != -1)    /* a -1 tic count never changes */
                {
                    psp->tics--;
                    if (!psp->tics)
                        P_SetPsprite (player, i, psp->state->nextstate);
                }
            }
        }
    }

    player->psprites[ps_flash].sx = player->psprites[ps_flashalpha].sx = player->psprites[ps_weapon].sx;
    player->psprites[ps_flash].sy = player->psprites[ps_flashalpha].sy = player->psprites[ps_weapon].sy;
}

/*============================================================================= */

/*
=================
=
= P_RecursiveSound
=
= If a monster yells at a player, it will alert other monsters to the player
=
=================
*/

mobj_t *soundtarget; // 800A5E74

void P_RecursiveSound (sector_t *sec, int soundblocks) // 8001B254
{
    int         i;
    line_t      *check;
    sector_t    *other;
    sector_t    *front, *back;

    /* wake up all monsters in this sector */
    if (sec->validcount == validcount && sec->soundtraversed <= soundblocks+1)
        return;     /* already flooded */

    sec->validcount = validcount;
    sec->soundtraversed = soundblocks+1;
    sec->soundtarget = soundtarget;

    for (i=0 ;i<sec->linecount ; i++)
    {
        check = sec->lines[i];
        back = check->backsector;
        if (!back)
            continue;       /* single sided */

        front = check->frontsector;
        if (front->floorheight >= back->ceilingheight ||
            front->ceilingheight <= back->floorheight ||
            back->floorheight == back->ceilingheight) // [d64] added this check
            continue;       /* closed door */

        if ( front == sec)
            other = back;
        else
            other = front;

        if (check->flags & ML_SOUNDBLOCK)
        {
            if (!soundblocks)
                P_RecursiveSound (other, 1);
        }
        else
            P_RecursiveSound (other, soundblocks);
    }
}


/*
=================
=
= P_NoiseAlert
=
=================
*/

void P_NoiseAlert (player_t *player) // 8001B3A4
{
    sector_t    *sec;

    sec = player->mo->subsector->sector;

    if (player->lastsoundsector == (void *)sec)
        return;     /* don't bother doing it again here */

    soundtarget = player->mo;
    player->lastsoundsector = (void *)sec;

    validcount++;
    P_RecursiveSound (sec, 0);
}


/*
==================
=
= P_SetPsprite
=
==================
*/

void P_SetPsprite (player_t *player, int position, statenum_t stnum) // 8001B3FC
{
    pspdef_t *psp;
    state_t  *state;

    psp = &player->psprites[position];

    do
    {
        if (!stnum)
        {
            psp->state = NULL;
            break;      /* object removed itself */
        }
        state = &states[stnum];
        psp->state = state;
        psp->tics = state->tics;  /* could be 0 */

        /* call action routine */
        if (state->action)
        {
            ((void (*)(void *, void *)) state->action) (player, psp);
            if (!psp->state)
                break;
        }
        stnum = psp->state->nextstate;
    } while (!psp->tics);   /* an initial state of 0 could cycle through */
}


/*
===============================================================================

                        PSPRITE ACTIONS

===============================================================================
*/

/*
===============================================================================

PSPRITE ACTIONS

===============================================================================
*/

weaponinfo_t    weaponinfo[NUMWEAPONS] = // 8005AD80
{
    {   /* saw */
/* ammo         */  am_noammo,
/* upstate      */  S_SAWUP,
/* downstate    */  S_SAWDOWN,
/* readystate   */  S_SAW,
/* atkstate     */  S_SAW1,
/* flashstate   */  S_NULL
    },

    {   /* fist */
/* ammo         */  am_noammo,
/* upstate      */  S_PUNCHUP,
/* downstate    */  S_PUNCHDOWN,
/* readystate   */  S_PUNCH,
/* atkstate     */  S_PUNCH1,
/* flashstate   */  S_NULL
    },

    {   /* pistol */
/* ammo         */  am_clip,
/* upstate      */  S_PISTOLUP,
/* downstate    */  S_PISTOLDOWN,
/* readystate   */  S_PISTOL,
/* atkstate     */  S_PISTOL2,
/* flashstate   */  S_PISTOLFLASH
    },

    {   /* shotgun */
/* ammo         */  am_shell,
/* upstate      */  S_SGUNUP,
/* downstate    */  S_SGUNDOWN,
/* readystate   */  S_SGUN,
/* atkstate     */  S_SGUN2,
/* flashstate   */  S_SGUNFLASH
    },

    {   /* super shotgun */
/* ammo         */  am_shell,
/* upstate      */  S_DSGUNUP,
/* downstate    */  S_DSGUNDOWN,
/* readystate   */  S_DSGUN,
/* atkstate     */  S_DSGUN1,
/* flashstate   */  S_DSGUNFLASH
    },

    {   /* chaingun */
/* ammo         */  am_clip,
/* upstate      */  S_CHAINUP,
/* downstate    */  S_CHAINDOWN,
/* readystate   */  S_CHAIN,
/* atkstate     */  S_CHAIN1,
/* flashstate   */  S_CHAINFLASH1
    },

    {   /* missile */
/* ammo         */  am_misl,
/* upstate      */  S_MISSILEUP,
/* downstate    */  S_MISSILEDOWN,
/* readystate   */  S_MISSILE,
/* atkstate     */  S_MISSILE1,
/* flashstate   */  S_MISSILEFLASH1
    },

    {   /* plasma */
/* ammo         */  am_cell,
/* upstate      */  S_PLASMAANIM,
/* downstate    */  S_PLASMADOWN,
/* readystate   */  S_PLASMA,
/* atkstate     */  S_PLASMA1,
/* flashstate   */  S_NULL
    },

    {   /* bfg */
/* ammo         */  am_cell,
/* upstate      */  S_BFGUP,
/* downstate    */  S_BFGDOWN,
/* readystate   */  S_BFG,
/* atkstate     */  S_BFG1,
/* flashstate   */  S_BFGFLASH1
    },

    {   /* laser rifle */
/* ammo         */  am_cell,
/* upstate      */  S_LASERUP,
/* downstate    */  S_LASERDOWN,
/* readystate   */  S_LASER,
/* atkstate     */  S_LASER1,
/* flashstate   */  S_LASERFLASH
    }
};


/*
================
=
= P_BringUpWeapon
=
= Starts bringing the pending weapon up from the bottom of the screen
= Uses player
================
*/

void P_BringUpWeapon (player_t *player) // 8001B4BC
{
    statenum_t  new;

    if (player->pendingweapon == wp_nochange)
        player->pendingweapon = player->readyweapon;

    if (player->pendingweapon == wp_chainsaw) {
        S_StartSound(player->mo, sfx_sawup);
    }
    else if (player->pendingweapon == wp_plasma) {
        S_StartSound(player->mo, sfx_electric);
    }

    new = weaponinfo[player->pendingweapon].upstate;

    player->pendingweapon = wp_nochange;
    player->psprites[ps_weapon].sx = WEAPONX;
    player->psprites[ps_weapon].sy = WEAPONBOTTOM;
    player->psprites[ps_flashalpha].alpha = 255;
    P_SetPsprite (player, ps_weapon, new);
}

/*
================
=
= P_DropWeapon
=
= Player died, so put the weapon away
================
*/

void P_DropWeapon (player_t *player) // 8001B580
{
    P_SetPsprite (player, ps_weapon, weaponinfo[player->readyweapon].downstate);
}

/*
================
=
= P_CheckAmmo
=
= returns true if there is enough ammo to shoot
= if not, selects the next weapon to use
================
*/

boolean P_CheckAmmo (player_t *player) // 8001B5BC
{
    ammotype_t  ammo;
    int         count;

    ammo = weaponinfo[player->readyweapon].ammo;

    if (player->readyweapon == wp_bfg)
        count = BFGCELLS;
    else if (player->readyweapon == wp_laser)
    {
      count = ArtifactLookupTable[player->artifacts];
      if (count == 0)
        count = 1;
    }
    else if (player->readyweapon == wp_supershotgun)
        count = 2;  // Double barrel.
    else
        count = 1;

    if (ammo == am_noammo || player->ammo[ammo] >= count)
        return true;

    /* out of ammo, pick a weapon to change to */
    do
    {
        if (player->weaponowned[wp_plasma] && player->ammo[am_cell])
            player->pendingweapon = wp_plasma;
        else if (player->weaponowned[wp_supershotgun] && player->ammo[am_shell] > 2)
            player->pendingweapon = wp_supershotgun;
        else if (player->weaponowned[wp_chaingun] && player->ammo[am_clip])
            player->pendingweapon = wp_chaingun;
        else if (player->weaponowned[wp_shotgun] && player->ammo[am_shell])
            player->pendingweapon = wp_shotgun;
        else if (player->ammo[am_clip])
            player->pendingweapon = wp_pistol;
        else if (player->weaponowned[wp_chainsaw])
            player->pendingweapon = wp_chainsaw;
        else if (player->weaponowned[wp_missile] && player->ammo[am_misl])
            player->pendingweapon = wp_missile;
        else if (player->weaponowned[wp_bfg] && player->ammo[am_cell]>40)
            player->pendingweapon = wp_bfg;
        else
            player->pendingweapon = wp_fist;
    } while (player->pendingweapon == wp_nochange);

    P_SetPsprite (player, ps_weapon, weaponinfo[player->readyweapon].downstate);
    P_UpdateWeaponWheel(player);

    return false;
}


/*
================
=
= P_FireWeapon
=
================
*/

void P_FireWeapon (player_t *player) // 8001B7CC
{
    statenum_t  new;

    if (!P_CheckAmmo (player))
        return;

    P_SetMobjState (player->mo, S_PLAY_ATK1);

    player->psprites[ps_weapon].sx = WEAPONX;
    player->psprites[ps_weapon].sy = WEAPONTOP;
    new = weaponinfo[player->readyweapon].atkstate;
    P_SetPsprite (player, ps_weapon, new);
    P_NoiseAlert (player);
}

/*
=================
=
= A_WeaponReady
=
= The player can fire the weapon or change to another weapon at this time
=
=================
*/

SEC_GAME void A_WeaponReady (player_t *player, pspdef_t *psp) // 8001B83C
{
    int         angle;

    /* */
    /* check for change */
    /*  if player is dead, put the weapon away */
    /* */
    if (player->pendingweapon != wp_nochange || !player->health)
    {
        /* change weapon (pending weapon should allready be validated) */
        P_DropWeapon(player);
        return;
    }

    /* */
    /* check for fire */
    /* */
    /* the missile launcher and bfg do not auto fire */
    if (players[0].attackdown)
    {
        P_FireWeapon (player);
        return;
    }

    /* */
    /* bob the weapon based on movement speed */
    /* */
    //angle = (64*gamevbls)&(FINEANGLES-1);
    angle = (64*ticon)&(FINEANGLES-1); // PsxDoom/D64 use ticon no gamevbls
    psp->sx = WEAPONX + FixedMul(player->bob, finecosine(angle));
    angle &= FINEANGLES/2-1;
    psp->sy = WEAPONTOP + FixedMul(player->bob, finesine(angle));
}


/*
=================
=
= A_ReFire
=
= The player can re fire the weapon without lowering it entirely
=
=================
*/

SEC_GAME void A_ReFire (player_t *player, pspdef_t *psp) // 8001B91C
{
    /* */
    /* check for fire (if a weaponchange is pending, let it go through instead) */
    /* */
    if (players[0].attackdown && player->pendingweapon == wp_nochange && player->health)
    {
        player->refire++;
        P_FireWeapon (player);
    }
    else
    {
        player->refire = 0;
        P_CheckAmmo (player);
    }
}

/*
=================
=
= A_CheckReload
=
=================
*/

SEC_GAME void A_CheckReload(player_t *player, pspdef_t *psp) // 8001B9A0
{
    P_CheckAmmo(player);
}

/*
=================
=
= A_Lower
=
=================
*/

SEC_GAME void A_Lower (player_t *player, pspdef_t *psp) // 8001B9C0
{
    if (customskill.player_switch_speed) // [Immorpher] double weapon lower speed on nightmare
    {
        psp->sy += 2*LOWERSPEED;
    }
    else
    {
        psp->sy += LOWERSPEED;
    }
    if (psp->sy < WEAPONBOTTOM )
        return;

    /* */
    /* [d64] stop plasma buzz */
    /* */
    if (player->readyweapon == wp_plasma)
        S_StopSound(NULL, sfx_electric);

    /* */
    /* [d64] clear flash graphic drawer */
    /* */
    P_SetPsprite(player, ps_flash, S_NULL);

    if (player->playerstate == PST_DEAD)
    {
        psp->sy = WEAPONBOTTOM;
        return;     /* don't bring weapon back up */
    }

    /* */
    /* The old weapon has been lowered off the screen, so change the weapon */
    /* and start raising it */
    /* */
    if (!player->health)
    {   /* player is dead, so keep the weapon off screen */
        P_SetPsprite (player,  ps_weapon, S_NULL);
        return;
    }

    player->readyweapon = player->pendingweapon;

    P_BringUpWeapon (player);
}


/*
=================
=
= A_Raise
=
=================
*/

SEC_GAME void A_Raise (player_t *player, pspdef_t *psp) // 8001BA84
{
    statenum_t  new;

    if (customskill.player_switch_speed) // [Immorpher] double weapon raise speed on nightmare
    {
        psp->sy -= 2*RAISESPEED;
    }
    else
    {
        psp->sy -= RAISESPEED;
    }

    if (psp->sy > WEAPONTOP )
        return;

    psp->sy = WEAPONTOP;

    /* */
    /* the weapon has been raised all the way, so change to the ready state */
    /* */
    new = weaponinfo[player->readyweapon].readystate;

    P_SetPsprite (player, ps_weapon, new);
}


/*
================
=
= A_GunFlash
=
=================
*/

SEC_GAME void A_GunFlash (player_t *player, pspdef_t *psp) // 8001BAD8
{
    /* [d64] set alpha on flash frame */
    if(player->readyweapon == wp_missile)
        player->psprites[ps_flashalpha].alpha = 100;

    P_SetPsprite (player,ps_flashalpha,weaponinfo[player->readyweapon].flashstate);
}


/*
===============================================================================

                        WEAPON ATTACKS

===============================================================================
*/


/*
==================
=
= A_Punch
=
==================
*/

SEC_GAME void A_Punch (player_t *player, pspdef_t *psp) // 8001BB2C
{
    angle_t     angle;
    int         damage;

    //damage = ((P_Random ()&7)*3)+3;
    damage = ((P_Random()&7)+1)*3;
    if (player->powers[pw_strength])
        damage *= 10;
    angle = player->mo->angle;
    angle += (angle_t)(P_Random()-P_Random())<<18;
    P_LineAttack (player->mo, angle, 0, MELEERANGE, P_AimSlope(player), damage);
    /* turn to face target */
    if (hittarget.type == ht_thing)
    {
        I_RumbleShot(0, 5);
        S_StartSound(player->mo, sfx_punch);
        player->mo->angle = R_PointToAngle2 (player->mo->x, player->mo->y, hittarget.thing->x, hittarget.thing->y);
    }
}

/*
==================
=
= A_Saw
=
==================
*/

SEC_GAME void A_Saw (player_t *player, pspdef_t *psp) // 8001BC1C
{
    angle_t     angle;
    int         damage;
    int         rnd1, rnd2;

    //damage = ((P_Random ()&7)*3)+3;
    damage = ((P_Random()&7)+1)*3;
    angle = player->mo->angle;
    rnd1 = P_Random();
    rnd2 = P_Random();
    angle += (angle_t)(rnd2-rnd1)<<18;
    /* use meleerange + 1 se the puff doesn't skip the flash */
    P_LineAttack (player->mo, angle, 0, MELEERANGE+1, P_AimSlope(player), damage);
    if (hittarget.type != ht_none)
        I_RumbleShot(0, 3);
    else
        I_RumbleShot(0, 2);
    if (hittarget.type != ht_thing)
    {
        S_StartSound (player->mo, sfx_saw1);
        return;
    }
    S_StartSound (player->mo, sfx_saw2);

    /* turn to face target */
    angle = R_PointToAngle2 (player->mo->x, player->mo->y, hittarget.thing->x, hittarget.thing->y);
    if (angle - player->mo->angle > ANG180)
    {
        if (angle - player->mo->angle < -ANG90/20)
            player->mo->angle = angle + ANG90/21;
        else
            player->mo->angle -= ANG90/20;
    }
    else
    {
        if (angle - player->mo->angle > ANG90/20)
            player->mo->angle = angle - ANG90/21;
        else
            player->mo->angle += ANG90/20;
    }
    player->mo->flags |= MF_JUSTATTACKED;
}

/*
==================
=
= A_ChainSawReady
=
==================
*/

SEC_GAME void A_ChainSawReady(player_t *player, pspdef_t *psp) // 8001BDA8
{
    S_StartSound(player->mo, sfx_sawidle);
    A_WeaponReady(player, psp);
}

/*
==================
=
= A_FireMissile
=
==================
*/

SEC_GAME void A_FireMissile (player_t *player, pspdef_t *psp) // 8001BDE4
{
    player->ammo[weaponinfo[player->readyweapon].ammo]--;

    player->recoilpitch = RECOILPITCH;

    if(player->onground)
        P_Thrust(player, player->mo->angle + ANG180, FRACUNIT);

    P_SpawnPlayerMissile (player->mo, MT_PROJ_ROCKET);

    I_RumbleShot(0, 8);
}


/*
==================
=
= A_FireBFG
=
==================
*/

SEC_GAME void A_FireBFG (player_t *player, pspdef_t *psp) // 8001BE78
{
    player->ammo[weaponinfo[player->readyweapon].ammo] -= BFGCELLS;
    P_SpawnPlayerMissile (player->mo, MT_PROJ_BFG);
    I_RumbleShot(0, 6);
}


/*
==================
=
= A_PlasmaAnimate
=
==================
*/
int pls_animpic = 0; // 8005AE70

SEC_GAME void A_PlasmaAnimate(player_t *player, pspdef_t *psp) // 8001BED8
{
    P_SetPsprite(player, ps_flash, pls_animpic + S_PLASMATUBE1);

    if (++pls_animpic >= 3)
        pls_animpic = 0;
}

/*
==================
=
= A_FirePlasma
=
==================
*/

SEC_GAME void A_FirePlasma (player_t *player, pspdef_t *psp) // 8001BF2C
{
    player->ammo[weaponinfo[player->readyweapon].ammo]--;
    P_SetPsprite (player,ps_flash,S_NULL);
    P_SpawnPlayerMissile (player->mo, MT_PROJ_PLASMA);
    I_RumbleShot(0, 2);
}

/*
===============
=
= P_BulletSlope
=
===============
*/

fixed_t P_BulletSlope(player_t *player) // 8001BF88
{
    angle_t an;
    mobj_t *mo;
    fixed_t bulletslope;

    if (!player->config->autoaim)
        goto done;

    // see which target is to be aimed at
    mo = player->mo;
    an = mo->angle;
    bulletslope = P_AimLineAttack(mo, an, 0, 16 * 64 * FRACUNIT);

    if (hittarget.type == ht_thing)
        return bulletslope;

    an += 1 << 26;
    bulletslope = P_AimLineAttack(mo, an, 0, 16 * 64 * FRACUNIT);
    if (hittarget.type == ht_thing)
        return bulletslope;

    an -= 2 << 26;
    bulletslope = P_AimLineAttack(mo, an, 0, 16 * 64 * FRACUNIT);

    if (hittarget.type == ht_thing)
        return bulletslope;
done:
    return P_AimSlope(player);
}

/*
===============
=
= P_GunShot
=
===============
*/

void P_GunShot (mobj_t *mo, boolean accurate, fixed_t bulletslope) // 8001C024
{
    angle_t     angle;
    int         damage;
    int         rnd1, rnd2;

    damage = ((P_Random ()&3)*4)+4;
    angle = mo->angle;
    if (!accurate)
    {
        rnd1 = P_Random();
        rnd2 = P_Random();
        angle += (rnd2-rnd1)<<18;
    }

    P_LineAttack (mo, angle, 0, MISSILERANGE, bulletslope, damage);
}

/*
==================
=
= A_FirePistol
=
==================
*/

SEC_GAME void A_FirePistol (player_t *player, pspdef_t *psp) // 8001C0B4
{
    S_StartSound (player->mo, sfx_pistol);
    player->ammo[weaponinfo[player->readyweapon].ammo]--;
    P_SetPsprite (player,ps_flashalpha,weaponinfo[player->readyweapon].flashstate);

    P_GunShot (player->mo, !player->refire, P_BulletSlope(player));
}

/*
==================
=
= A_FireShotgun
=
==================
*/

SEC_GAME void A_FireShotgun (player_t *player, pspdef_t *psp) // 8001C138
{
    int         i, bulletslope;

    S_StartSound (player->mo, sfx_shotgun);
    player->ammo[weaponinfo[player->readyweapon].ammo]--;
    player->recoilpitch = RECOILPITCH;

    P_SetPsprite (player,ps_flashalpha,weaponinfo[player->readyweapon].flashstate);
    bulletslope = P_BulletSlope(player);

    for (i=0 ; i<7 ; i++)
    {
        P_GunShot(player->mo, false, bulletslope);
    }
    I_RumbleShot(0, 5);
}

/*
==================
=
= A_FireShotgun2
=
==================
*/

SEC_GAME void A_FireShotgun2(player_t *player, pspdef_t *psp) // 8001C210
{
    angle_t     angle;
    int         damage;
    int         i;
    int         bulletslope;

    S_StartSound(player->mo, sfx_sht2fire);
    P_SetMobjState(player->mo, S_PLAY_ATK2);

    player->ammo[weaponinfo[player->readyweapon].ammo] -= 2;
    player->recoilpitch = RECOILPITCH;

    if(player->onground)
        P_Thrust(player, player->mo->angle + ANG180, FRACUNIT);

    P_SetPsprite(player, ps_flashalpha, weaponinfo[player->readyweapon].flashstate);
    bulletslope = P_BulletSlope(player); // [Immorpher] find auto aim slope and other things for shooting, required for damage

    for (i = 0; i<20; i++)
    {
        //damage = ((P_Random() % 3) * 5) + 5;
        damage = 5 * (P_Random() % 3 + 1);
        angle = player->mo->angle;
        angle += (P_Random() - P_Random()) << 19;
        P_LineAttack(player->mo, angle, 0, MISSILERANGE, bulletslope + ((P_Random() - P_Random()) << 5), damage);
    }
    I_RumbleShot(0, 7);
}

/*
==================
=
= A_CockSgun
=
==================
*/
#if 0 //No Used In PSX Doom/ Doom64
SEC_GAME void A_CockSgun (player_t *player, pspdef_t *psp)
{
    S_StartSound (player->mo, sfx_sgcock);
}
#endif // 0

/*
==================
=
= A_FireCGun
=
==================
*/

SEC_GAME void A_FireCGun (player_t *player, pspdef_t *psp) // 8001C3F8
{
    int ammo;
    int rand;

    ammo = player->ammo[weaponinfo[player->readyweapon].ammo];

    if (!ammo)
        return;

    S_StartSound (player->mo, sfx_pistol);

    player->ammo[weaponinfo[player->readyweapon].ammo]--;

    /* randomize sx */
    rand = (((P_Random() & 1) << 1) - 1);
    psp->sx = (rand * FRACUNIT);

    /* randomize sy */
    rand = ((((ammo - 1) & 1) << 1) - 1);
    psp->sy = WEAPONTOP - (rand * (2*FRACUNIT));

    player->recoilpitch = RECOILPITCH;

    P_SetPsprite (player,ps_flashalpha,weaponinfo[player->readyweapon].flashstate + psp->state - &states[S_CHAIN1]);

    player->psprites[ps_flashalpha].alpha = 160;

    P_GunShot (player->mo, !player->refire, P_BulletSlope(player));
    I_RumbleShot(0, 3);
}

/*
================
=
= A_BFGFlash
=
=================
*/

SEC_GAME void A_BFGFlash(mobj_t* actor) // 8001C548
{
    players[0].bfgcount = 100;
    actor->alpha = 170;
    I_RumbleAmbient(0, 1);
}

/*
================
=
= A_BFGSpray
=
= Spawn a BFG explosion on every monster in view
=
=================
*/

SEC_GAME void A_BFGSpray (mobj_t *mo) // 8001C560
{
    int         i, j, damage;
    angle_t     an;
    int         alpha;

    alpha = 0;

    /* offset angles from its attack angle */
    for (i=0 ; i<40 ; i++)
    {
        an = mo->angle - ANG90/2 + ANG90/40*i;
        /* mo->target is the originator (player) of the missile */
        P_AimLineAttack (mo->target, an, 0, 16*64*FRACUNIT);
        if (hittarget.type != ht_thing)
            continue;
        P_SpawnMobj (hittarget.thing->x, hittarget.thing->y, hittarget.thing->z + (hittarget.thing->height>>1), MT_BFGSPREAD);
        damage = 0;
        for (j=0;j<15;j++)
        {
            damage += (P_Random()&7) + 1;
        }
        P_DamageMobj (hittarget.thing, mo->target,mo->target, damage);
    }

    alpha = mo->alpha * 3;
    if (alpha < 0)
        alpha += 3;

    mo->alpha = alpha >> 2;
    I_RumbleAmbient(0, -1);
}

/*
================
=
= A_BFGsound
=
=================
*/

SEC_GAME void A_BFGsound (player_t *player, pspdef_t *psp) // 8001C698
{
    S_StartSound (player->mo, sfx_bfg);
}

#if 0
/*
================
=
= A_OpenShotgun2
=
=================
*/

SEC_GAME void A_OpenShotgun2(player_t *player, pspdef_t *psp)//L80021AFC()
{
    S_StartSound(player->mo, sfx_dbopn);
}
#endif // 0

/*
================
=
= A_LoadShotgun2
=
=================
*/

SEC_GAME void A_LoadShotgun2(player_t *player, pspdef_t *psp) // 8001C6C0
{
    S_StartSound(player->mo, sfx_sht2load1);
}

/*
================
=
= A_CloseShotgun2
=
=================
*/

SEC_GAME void A_CloseShotgun2(player_t *player, pspdef_t *psp) // 8001C6E8
{
    S_StartSound(player->mo, sfx_sht2load2);
    //A_ReFire(player, psp);
}

/*
================
=
= P_LaserCrossBSP
=
=================
*/

void P_LaserCrossBSP(int bspnum, laserdata_t *laser) // 8001C710
{
    node_t* node;
    int ds1, ds2;
    int s1, s2;
    int dist;
    int frac;
    mobj_t *marker;
    laserdata_t *childlaser;
    fixed_t x, y, z;
    fixed_t x1, y1, z1;
    fixed_t x2, y2, z2;
    fixed_t nx, ny, ndx, ndy;

    while(!(bspnum & NF_SUBSECTOR))
    {
        node = &nodes[bspnum];

        x1 = laser->x1;
        y1 = laser->y1;
        z1 = laser->z1;
        x2 = laser->x2;
        y2 = laser->y2;
        z2 = laser->z2;

        nx = node->line.x;
        ny = node->line.y;
        ndx = (node->line.dx >> FRACBITS);
        ndy = (node->line.dy >> FRACBITS);

        /* traverse nodes */
        ds1 = (((x1 - nx) >> FRACBITS) * ndy) - (((y1 - ny) >> FRACBITS) * ndx);
        ds2 = (((x2 - nx) >> FRACBITS) * ndy) - (((y2 - ny) >> FRACBITS) * ndx);

        s1 = (ds1 < 0);
        s2 = (ds2 < 0);

        /* did the two laser points cross the node? */
        if(s1 == s2)
        {
            bspnum = node->children[s1];
            continue;
        }

        /* new child laser */
        childlaser = (laserdata_t *)Z_Malloc( sizeof(*childlaser), PU_LEVSPEC, 0);

        /* copy laser pointer */
        *childlaser = *laser;

        /* get the intercepting point of the laser and node */
        frac = FixedDiv(ds1, ds1 - ds2);

        x = (((x2 - x1) >> FRACBITS) * frac) + x1;
        y = (((y2 - y1) >> FRACBITS) * frac) + y1;
        z = (((z2 - z1) >> FRACBITS) * frac) + z1;

        /* update endpoint of current laser to intercept point */
        laser->x2 = x;
        laser->y2 = y;
        laser->z2 = z;

        /* childlaser begins at intercept point */
        childlaser->x1 = x;
        childlaser->y1 = y;
        childlaser->z1 = z;

        /* update distmax */
        dist = (laser->distmax * frac) >> FRACBITS;

        childlaser->distmax = laser->distmax - dist;
        laser->distmax = dist;

        /* point to child laser */
        laser->next = childlaser;

        /* traverse child nodes */
        P_LaserCrossBSP(node->children[s1], laser);

        laser = childlaser;
        bspnum = node->children[s2];
    }

    /* subsector was hit, spawn a marker between the two laser points */
    x = (laser->x1 + laser->x2) >> 1;
    y = (laser->y1 + laser->y2) >> 1;
    z = (laser->z1 + laser->z2) >> 1;

    marker = P_SpawnMobj(x, y, z, MT_LASERMARKER);

    /* have marker point to which laser it belongs to */
    marker->extradata = laser;
    laser->marker = marker;
}

/*
================
=
= T_LaserThinker
=
=================
*/

void T_LaserThinker(laser_t *laser) // 8001C9B8
{
    fade_t *fade;
    laserdata_t *pThisLaser;

    pThisLaser = laser->laserdata;
    pThisLaser ->dist += 64;

    /* laser reached its destination? */
    if(pThisLaser->dist >= pThisLaser->distmax)
    {
        /* reached the end? */
        if (!pThisLaser->next)
        {
            P_RemoveThinker(&laser->thinker);

            if (laser->marker->alpha == 0)
            {
                P_RemoveMobj(laser->marker);
            }
            else
            {
                /* fade out the laser puff */
                fade = Z_Malloc (sizeof(*fade), PU_LEVSPEC, 0);
                P_AddThinker (&fade->thinker);
                fade->thinker.function = T_FadeThinker;
                fade->amount = -24;
                fade->destAlpha = 0;
                fade->flagReserve = 0;
                fade->mobj = laser->marker;
            }
        }
        else
        {
            laser->laserdata = pThisLaser->next;
        }

        P_RemoveMobj(pThisLaser->marker);
        Z_Free(pThisLaser);
    }
    else
    {
        /* update laser's location */
        pThisLaser->x1 += (pThisLaser->slopex * 32);
        pThisLaser->y1 += (pThisLaser->slopey * 32);
        pThisLaser->z1 += (pThisLaser->slopez * 32);
    }
}

extern const fixed_t crouchease[];

/*
================
=
= A_FireLaser
=
=================
*/
SEC_GAME void A_FireLaser(player_t *player, pspdef_t *psp) // 8001CAC0
{
    angle_t     angleoffs;
    angle_t     spread = 0;
    mobj_t      *mobj;
    int         lasercount;
    int         i;
    fixed_t     slopex, slopey, slopez;
    fixed_t     x, y, z;
    fixed_t     x1, y1, z1;
    fixed_t     x2, y2, z2;
    byte        type;
    laserdata_t *laser_data;
    laser_t     *laser;
    fixed_t     laserfrac;
    int         damage;
    int         autoaim;
    int         shootheight;
    int         laserheight; // [Immorpher] Adjust laser shoot height based on player view
    int         laserspacing; // [Immorpher] Adjust laser shoot distance based on player view

    mobj = player->mo;

    type = ArtifactLookupTable[player->artifacts];

    /* setup laser type */
    switch(type)
    {
    case 1:     /* Rapid fire / single shot */
        psp->tics = 5;
        lasercount = 1;
        angleoffs = mobj->angle;
        break;
    case 2:     /* Rapid fire / double shot */
        psp->tics = 4;
        lasercount = 2;
        spread = 0x16C0000;
        angleoffs = mobj->angle + 0xFF4A0000;
        break;
    case 3:     /* Spread shot */
        psp->tics = 4;
        lasercount = 3;
        spread = 0x2220000 + (0x2220000 * (player->refire & 3));
        angleoffs = mobj->angle - spread;
        break;
    default:    /* Normal shot */
        lasercount = 1;
        angleoffs = mobj->angle;
        break;
    }


    shootheight = LASERAIMHEIGHT >> FRACBITS;
    if (player->crouchtimer)
        shootheight = FixedMul (shootheight, crouchease[player->crouchtimer]);
    laserheight = (shootheight + (((shootheight >> 1) * finesine(player->pitch >> ANGLETOFINESHIFT)) >> FRACBITS)) << FRACBITS;
    laserspacing = (LASERDISTANCE*finecosine(player->pitch >> ANGLETOFINESHIFT)) >> FRACBITS;

    x1 = mobj->x + (finecosine(mobj->angle >> ANGLETOFINESHIFT) * laserspacing);
    y1 = mobj->y + (finesine(mobj->angle >> ANGLETOFINESHIFT) * laserspacing);
    z1 = mobj->z + laserheight;

    autoaim = player->config->autoaim;
    spawnpuff = false;
    if (autoaim)
    {
        slopez = P_AimLineAttack(mobj, mobj->angle, laserheight, LASERRANGE);
        autoaim = hittarget.type == ht_thing;
    }
    else
    {
        slopez = P_AimSlope(player);
    }

    /* setup laser beams */
    for(i = 0; i < lasercount; i++)
    {
        slopex = finecosine(angleoffs >> ANGLETOFINESHIFT);
        slopey = finesine(angleoffs >> ANGLETOFINESHIFT);

        damage = ((P_Random() & 7) * 10) + 10;

        P_LineAttack(mobj, angleoffs, laserheight, LASERRANGE, slopez, damage);
        angleoffs += spread;

        if (hittarget.type != ht_none)
        {
            laserfrac = (hittarget.frac << (FRACBITS - 4)) - (4 << FRACBITS);
            x2 = hittarget.x;
            y2 = hittarget.y;
            z2 = hittarget.z;
        }
        else
        {
            laserfrac = (2048*FRACUNIT);
            x2 = mobj->x + FixedMul(slopex, laserfrac);
            y2 = mobj->y + FixedMul(slopey, laserfrac);
            z2 = z1 + FixedMul(slopez, laserfrac);
        }

        z = (z2 - z1) >> FRACBITS;
        x = (x2 - x1) >> FRACBITS;
        y = (y2 - y1) >> FRACBITS;

        /* setup laser */
        laser_data = (laserdata_t *)Z_Malloc( sizeof(*laser_data), PU_LEVSPEC, 0);

        /* setup laser head point */
        laser_data->x1 = x1;
        laser_data->y1 = y1;
        laser_data->z1 = z1;

        /* setup laser tail point */
        laser_data->x2 = x2;
        laser_data->y2 = y2;
        laser_data->z2 = z2;

        /* setup movement slope */
        laser_data->slopex = slopex;
        laser_data->slopey = slopey;
        laser_data->slopez = slopez;

        /* setup distance info */
        laser_data->dist = 0;
        laser_data->distmax = (fixed_t) D_sqrtf((float)((x * x) + (y * y) + (z * z)));

        laser_data->next = NULL;

        P_LaserCrossBSP(numnodes-1, laser_data);

        /* setup laser puff */
        laser = (laser_t *)Z_Malloc( sizeof(*laser), PU_LEVSPEC, 0);
        P_AddThinker (&laser->thinker);
        laser->thinker.function = T_LaserThinker;
        laser->laserdata = laser_data;
        laser->marker = P_SpawnMobj(x2, y2, z2, MT_PROJ_LASER);

        // [nova] hide puff when hitting nothing
        if (hittarget.type == ht_none || hittarget.hitsky)
            laser->marker->alpha = 0;

        player->ammo[weaponinfo[player->readyweapon].ammo]--;
    }

    spawnpuff = true;

    P_SetPsprite(player, ps_flashalpha, weaponinfo[player->readyweapon].flashstate);
    S_StartSound(player->mo, sfx_laser);
}
