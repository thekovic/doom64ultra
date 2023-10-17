/* P_user.c */

#include "doomdef.h"
#include "p_local.h"

#define MAXMOCKTIME     900 // [Immorpher] Reduced this by half for the fun!
int deathmocktics; // 800A56A0

// Bonus mock texts from the Doom 64 community!
#define MK_TXT01t1  "MOTHER DEMON: HAHAHAHA!"
#define MK_TXT02t1  "MOTHER DEMON: YOU SHOULDN'T HAVE\nDONE THAT."
#define MK_TXT03t1  "MIDWAY: TRY AN EASIER LEVEL..."
#define MK_TXT04t1  "MIDWAY: WOW! LOOK AT THOSE DEMON\nFEET."
#define MK_TXT05t1  "MIDWAY: I PLAY DOOM AND I CAN'T GET\nUP."
#define MK_TXT06t1  "DEMON: OUCH! THAT HAD TO HURT."
#define MK_TXT07t1  "DEMON: LOOK AT ME! I'M FLAT!"
#define MK_TXT08t1  "MIDWAY: THANKS FOR PLAYING!"
#define MK_TXT09t1  "MOTHER DEMON: YOU LAZY @&$#!"
#define MK_TXT10t1  "MOTHER DEMON: HAVE YOU HAD ENOUGH?"
#define MK_TXT11t1  "MIDWAY: THE DEMONS GAVE YOU THE\nBOOT!"
#define MK_TXT12t1  "MIDWAY: THE LEAD DEMON VANQUISHED\nYOU!"
#define MK_TXT13t1  "IMMORPHER: WELCOME TO THE IQ64\nCLUB! MIGHT AS WELL JOIN THE GOOFS\nON THE DOOM 64 DISCORD SERVER!"
#define MK_TXT14t1  "IRL RANDOM HAJILE: HMMM, THAT'S ONE\nDOOMED SPACE MARINE."
#define MK_TXT15t1  "RETROTOUR: NO RUSH. JUST RESTART\nWHEN YOU'RE READY."
#define MK_TXT16t1  "SCD: GO BACK TO QUAKE, YA MORON!"
#define MK_TXT17t1  "WHINNY: I BET YOU STILL SLEEP WITH\nYOUR NIGHT LIGHT, WAAAAH!!!"
#define MK_TXT18t1  "UMLAUT: IF YOU'D LIKE TO JOIN US,\nYOU'RE ALWAYS WELCOME, DEAD OR\nALIVE. MUAHAHAHA..."
#define MK_TXT19t1  "TAUFAN99: NOW YOU KNOW THIS IS NO\nSUPERHERO SHOW."
#define MK_TXT20t1  "QUASIOTTER: EVERY TIME YOU DIE,\nA PUPPY IS BORN. THE DEMONS ARE\nJUST TRYING TO MAKE MORE PUPPIES."
#define MK_TXT21t1  "GEC: HOT OR COLD, WHICH SIDE DO YOU\nWANT TO BE ON?"
#define MK_TXT22t1  "WOLF MCBEARD: IT'S OK, I WON'T TELL\nANYONE YOU CALL ME DADDY."
#define MK_TXT23t1  "POOPSTER: NO MORE SEQUELS FOR\nYOU.... YET."
#define MK_TXT24t1  "WHYBMONOTACRAB: IF ONLY YOU COULD\nTALK TO THESE DEMONS, THEN PERHAPS\nYOU COULD TRY AND MAKE FRIENDS WITH\nTHEM, FORM ALLIANCES..."
#define MK_TXT25t1  "CAPTAIN CALEB: YOU'VE HAD OVER\nTWENTY YEARS, AND YOU STILL SUCK\nAT THIS GAME?"
#define MK_TXT26t1  "DUKE: WHAT ARE YOU WAITING FOR,\nCHRISTMAS?"
#define MK_TXT27t1  "COLLECTONIAN: UGH, YOU FORGOT\nAGAIN!? LAST TIME: ROCKETS ARE\nFOR KILLING DEMONS, NOT YOURSELF!"
#define MK_TXT28t1  "NEVANDER: THAT LOOKED LIKE IT HURT.\nWELL, WHAT ARE YOU WAITING FOR?\nTRY AGAIN AND KICK THEIR ASSES!"
#define MK_TXT29t1  "SCWIBA: YO IMMORPHER, I SAID NO\nMORE DOOM 64! WHY AM I EVEN IN\nTHIS MOD?!"
#define MK_TXT30t1  "GRAV: YOU KNOW YOU CAN BEAT THIS\nGAME IN 30 MINUTES RIGHT? WHAT'S\nTAKING SO LONG?"
#define MK_TXT31t1  "HARDCORE_GAMER: THIS IS WHAT\nHAPPENS WHEN YOU'VE HAD TOO\nMANY CORONAS."
#define MK_TXT32t1  "ISANN KEKET: ON YOUR FEET, MARINE.\nSLAYERS NEVER SLEEP!"
#define MK_TXT33t1  "AMUSED BRIAN: WELL THAT DIDN'T GO\nACCORDING TO PLAN..."
#define MK_TXT34t1  "SECTOR666: IF YOU HIDE ON THE FLOOR\nFOREVER AT LEAST YOU WON'T DIE...\nWHOOPS, TOO LATE..."
#define MK_TXT35t1  "DEXIAZ: IT'S OFFICIAL! YOU SUCK AT\nPLAYING MAPS!"
#define MK_TXT36t1  "Z0K: IF YOU DROPPED SOME WEAPONS,\nYOU COULD HAVE DODGED, FATSO!"
#define MK_TXT37t1  "PUZZLEWELL: THEY'LL BURY YOU IN A\nLUNCHBOX (WAIT WRONG GAME...)."
#define MK_TXT38t1  "KMXEXII: I CAN ONLY HOPE THAT THIS\nDEATH WAS IN SOME WAY LOST SOUL OR\nPAIN ELEMENTAL RELATED."
#define MK_TXT39t1  "ENDLESS: ENDLESSLY DYING IS NOT\nA GOOD STRATEGY."
#define MK_TXT40t1  "ERROR: BACK TO THE DRAWING BOARD...\nTOO BAD YOU CAN'T DRAW."
#define MK_TXT41t1  "BLUETHUNDER: SO... YOU'RE GONNA DIE\nNOW... SERIOUSLY?!!"
#define MK_TXT42t1  "HYACSHO: MAYBE YOU SHOULD PLAY\nDOOM 1993 INSTEAD?"
#define MK_TXT43t1  "NOWANDTHEN64: OF ALL THE ATTEMPTS\nTHAT HAVE BEEN MADE AT THIS GAME,\nTHAT WAS CERTAINLY ONE OF THEM."
#define MK_TXT44t1  "SIXHUNDREDSIXTEEN: SURE LET THEM\nKILL YOU! WHAT A %#$@ING DISASTER..."
#define MK_TXT45t1  "MIKE_C: DON'T BLAME THE CONTROLLER,\nYOU BIG WUSS!"
#define MK_TXT46t1  "GLENN PLANT: THE RISING SUN WILL\nEVENTUALLY SET, A NEWBORN'S LIFE\nWILL FADE. FROM SUN TO MOON, MOON\nTO SUN. GIVE PEACEFUL REST TO THE\nLIVING DEAD."
#define MK_TXT47t1  "MOTHERLOAD: YOUR GREAT GRANDFATHER\nWAS IN A REAL WAR AND YOU'RE TOO\nMUCH OF A $#@&! TO BEAT THIS LEVEL."
#define MK_TXT48t1  "GERARDO: WHAT?? FEELING SLEEPY NOW?\nYOU CONSIDER YOURSELF SLAYER AND\nYOU WANT TO REST? SHAME ON YOU!"
#define MK_TXT49t1  "HEADSHOT: YOU'VE HUMOURED THE\nDEMONS GREATLY WITH YOUR\nARROGANCE AND CONTEMPT..."
#define MK_TXT50t1  "ZELLLOOO: WE HAVE PINKY,\nBUT WHERE'S THE BRAIN?"
#define MK_TXT51t1  "PEACHES: THIS ACTION TO DEFY THE\nEVIL ONE IS NOT WITHOUT RISK.\nCONFRONTATION IS OFTEN BEST\nAVOIDED..."

const char *mockstrings1[] =   // 8005A290
{
    MK_TXT01t1, MK_TXT02t1, MK_TXT03t1, MK_TXT04t1,
    MK_TXT05t1, MK_TXT06t1, MK_TXT07t1, MK_TXT08t1,
    MK_TXT09t1, MK_TXT10t1, MK_TXT11t1, MK_TXT12t1,
    MK_TXT13t1, MK_TXT14t1, MK_TXT15t1, MK_TXT16t1,
    MK_TXT17t1, MK_TXT18t1, MK_TXT19t1, MK_TXT20t1,
    MK_TXT21t1, MK_TXT22t1, MK_TXT23t1, MK_TXT24t1,
    MK_TXT25t1, MK_TXT26t1, MK_TXT27t1, MK_TXT28t1,
    MK_TXT29t1, MK_TXT30t1, MK_TXT31t1, MK_TXT32t1,
    MK_TXT33t1, MK_TXT34t1, MK_TXT35t1, MK_TXT36t1,
    MK_TXT37t1, MK_TXT38t1, MK_TXT39t1, MK_TXT40t1,
    MK_TXT41t1, MK_TXT42t1, MK_TXT43t1, MK_TXT44t1,
    MK_TXT45t1, MK_TXT46t1, MK_TXT47t1, MK_TXT48t1,
    MK_TXT49t1, MK_TXT50t1, MK_TXT51t1,
};

#define MAXPLAYERSPEED 0x16000

static const fixed_t       movesteps[3] = {MAXPLAYERSPEED*7/11, MAXPLAYERSPEED, MAXPLAYERSPEED/2}; // 8005B060

#define SLOWTURNTICS    10
const fixed_t           angleturn[] =       // 8005B070
    {50,50,83,83,100,116,133,150,150,166,
    133,133,150,166,166,200,200,216,216,233}; // fastangleturn

const fixed_t           crouchease[] = {65536, 61440, 53951, 32768};
/*============================================================================= */

mobj_t          *slidething;    //80077D04, pmGp000008f4
extern  fixed_t slidex, slidey; //80077dbc || 80077dc0
extern  line_t  *specialline;   //80077dc8

void P_SlideMove (mobj_t *mo);

#if 0
void P_PlayerMove (mobj_t *mo)//L80029CB8()
{
    fixed_t     momx, momy;
    line_t      *latchedline;
    fixed_t     latchedx, latchedy;

    //momx = vblsinframe[playernum] * (mo->momx>>2);
    //momy = vblsinframe[playernum] * (mo->momy>>2);

    // Change on Final Doom
    momx = mo->momx;
    momy = mo->momy;

    slidething = mo;

    P_SlideMove ();

    latchedline = (line_t *)specialline;
    latchedx = slidex;
    latchedy = slidey;

    if ((latchedx == mo->x) && (latchedy == mo->y))
        goto stairstep;

    if (P_TryMove (mo, latchedx, latchedy))
        goto dospecial;

stairstep:

    if (momx > MAXMOVE)
        momx = MAXMOVE;
    else if (momx < -MAXMOVE)
        momx = -MAXMOVE;

    if (momy > MAXMOVE)
        momy = MAXMOVE;
    else if (momy < -MAXMOVE)
        momy = -MAXMOVE;

    /* something fucked up in slidemove, so stairstep */

    if (P_TryMove (mo, mo->x, mo->y + momy))
    {
        mo->momx = 0;
        mo->momy = momy;
        goto dospecial;
    }

    if (P_TryMove (mo, mo->x + momx, mo->y))
    {
        mo->momx = momx;
        mo->momy = 0;
        goto dospecial;
    }

    mo->momx = mo->momy = 0;

dospecial:
    if (latchedline)
        P_CrossSpecialLine (latchedline, mo);
}
#endif // 0

/*
===================
=
= P_PlayerXYMovement
=
===================
*/

#define STOPSPEED       0x1000
#define FRICTION        0xd200  //Jag 0xd240

SEC_GAME void P_PlayerXYMovement (mobj_t *mo) // 80021E20
{
    /* */
    /* try to slide along a blocked move */
    /* */
        if (!P_TryMove(mo, mo->x + mo->momx, mo->y + mo->momy))
            P_SlideMove(mo);

    /* */
    /* slow down */
    /* */
    if (mo->z > mo->floorz && !(mo->player->cheats & CF_FLYMODE))
        return;     /* no friction when airborne */

    if (mo->flags & MF_CORPSE)
        if (mo->floorz != mo->subsector->sector->floorheight)
            return;         /* don't stop halfway off a step */

    if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED &&
        mo->momy > -STOPSPEED && mo->momy < STOPSPEED)
    {
        mo->momx = 0;
        mo->momy = 0;
    }
    else
    {
        mo->momx = FixedMul(mo->momx, FRICTION);
        mo->momy = FixedMul(mo->momy, FRICTION);
    }
}


/*
===============
=
= P_PlayerZMovement
=
===============
*/

SEC_GAME void P_PlayerZMovement (mobj_t *mo) // 80021f38
{
    /* */
    /* check for smooth step up */
    /* */
    if (mo->z < mo->floorz)
    {
        int viewheight = FixedMul (VIEWHEIGHT, crouchease[mo->player->crouchtimer]);

        mo->player->viewheight -= (mo->floorz - mo->z);
        mo->player->deltaviewheight = (viewheight - mo->player->viewheight) >> 2;
    }

    /* */
    /* adjust height */
    /* */
    mo->z += mo->momz;

    /* */
    /* clip movement */
    /* */
    if (mo->z <= mo->floorz)
    {   /* hit the floor */
        if (mo->momz < 0)
        {
            if (mo->momz < -(GRAVITY*2))    /* squat down */
            {
                mo->player->deltaviewheight = mo->momz>>3;
                if (!(mo->player->cheats & CF_FLYMODE))
                    S_StartSound (mo, sfx_oof);
            }
            mo->momz = 0;
        }
        mo->z = mo->floorz;
    }
    else if ((mo->player->cheats & CF_FLYMODE) && mo->player->playerstate != PST_DEAD)
    {
        if (mo->momz > -STOPSPEED && mo->momz < STOPSPEED)
            mo->momz = 0;
        else
            mo->momz = FixedMul(mo->momz, FRICTION);
    }
    else
    {
        if (mo->momz == 0)
            mo->momz = -(GRAVITY/2);
        else
            mo->momz -= (GRAVITY/4);
    }

    if (mo->z + mo->height > mo->ceilingz)
    {   /* hit the ceiling */
        if (mo->momz > 0)
            mo->momz = 0;
        mo->z = mo->ceilingz - mo->height;
    }
}


/*
================
=
= P_PlayerMobjThink
=
================
*/

SEC_GAME void P_PlayerMobjThink (mobj_t *mobj) // 80022060
{
    state_t *st;
    int     state;

    /* */
    /* momentum movement */
    /* */
    if (mobj->momx || mobj->momy)
        P_PlayerXYMovement (mobj);

    if ( (mobj->z != mobj->floorz) || mobj->momz)
        P_PlayerZMovement (mobj);

    mobj->player->onground = (mobj->z <= mobj->floorz);

    // [nova] - timer for jump grace period
    if (mobj->player->onground)
        mobj->player->falltimer = 0;
    else if (mobj->player->falltimer < 30)
        mobj->player->falltimer += 1;

    /* */
    /* cycle through states, calling action functions at transitions */
    /* */
    if (mobj->tics == -1)
        return;             /* never cycle */

    mobj->tics--;

    if (mobj->tics > 0)
        return;             /* not time to cycle yet */

    state = mobj->state->nextstate;
    st = &states[state];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;
}

/*============================================================================= */

#define DEADZONE 7
#define XBM 68
#define XCM 52
#define YBM 70
#define YCM 54

static SEC_GAME u8 P_AdjustStickMode(u8 mode, int pbuttons)
{
    if (pbuttons & BB_LOOK)
        return STICK_TURN | STICK_VLOOK;
    else if (pbuttons & BB_STRAFE)
        return (mode & ~STICK_TURN) | STICK_STRAFE;
    return mode;
}

/* Returns the (x, y) coordinates of the control stick or mouse in signed 1.14
 * fixed point, with x/y in the upper/lower halfword. Possibly clamps the value
 * according to move sensitivity. This is done in a separate function because
 * the x/y must both be known in order to clamp. */
static SEC_GAME int P_GetStick(u8 mode, int buttons, int movesens, bool mouse)
{
    int x, y;

    x = STICK_X(buttons);
    y = STICK_Y(buttons);

    if (mouse)
    {
        x <<= 7;
        y <<= 7;

        /* handle mouse move sensitivity here */
        if (mode & STICK_STRAFE)
            x += (x * movesens) / 100;
        if (mode & STICK_MOVE)
            y += (y * movesens) / 100;

        goto done;
    }

    if (D_abs(x) < DEADZONE)
        x = 0;
    if (D_abs(y) < DEADZONE)
        y = 0;

    if (mode & (STICK_MOVE|STICK_STRAFE))
    {
        x = MAX(-127, x);
        y = MAX(-127, y);

        if (x || y)
        {
            f32 dist;
            f32 fx, fy;

            if (x > 0)
                x -= DEADZONE;
            else if (x < 0)
                x += DEADZONE;

            if (y > 0)
                y -= DEADZONE;
            else if (y < 0)
                y += DEADZONE;

            dist = 1.0f / (f32) (120 - movesens);

            /* n64 controllers typically report lower X values, so scale X up
             * by a small amount of XBM/YBM or (70/68) for the value here */
            fx = ((f32) x) * 1.029411765f * dist;
            fy = ((f32) y) * dist;

            dist = fx * fx + fy * fy;
            if (dist > 1.f)
            {
                dist = D_rsqrtf(dist);
                fx *= dist;
                fy *= dist;
            }

            if (mode & STICK_STRAFE)
                x = D_truncf(fx * (f32) (1<<14));
            if (mode & STICK_MOVE)
                y = D_truncf(fy * (f32) (1<<14));
        }
    }

    if (mode & STICK_TURN)
        x <<= 7;
    if (mode & STICK_VLOOK)
        y <<= 7;

done:
    return ((((u32) x) & 0xffff) << 16) | (((u32) y) & 0xffff);
}

/*
 * Returns true if a button was pressed, or if (shift | shiftbutton) is
 * pressed when button is unbound */
static SEC_GAME boolean P_ButtonOrShift(int button, int shift, int shiftbutton, int buttons, int oldbuttons)
{
    return button
        ? (buttons & button) && !(oldbuttons & button)
        : (buttons & shift) && (buttons & shiftbutton) && !(oldbuttons & shiftbutton);
}

/*
====================
=
= P_BuildMove
=
====================
*/

#define JUMPTHRUST      0x90000
#define JUMPGRACE       5

typedef struct {
    int     pitchmove;
    fixed_t forwardmove;
    fixed_t sidemove;
    angle_t angleturn;
    u8 crouch;
    u8 crouchheld;
    u8 jump;
    u8 jumpheld;
} buildmove_t;

SEC_GAME void P_BuildMove (player_t *player, buildmove_t *move) // 80022154
{
    int             speed, movespeed;
    int             buttons, oldbuttons;
    mobj_t         *mo;
    playerconfig_t *config;
    int             playerindex;
    int             aircontrol;
    fixed_t         stickmove, stickstrafe, stickturn, stickvlook;

    playerindex = player - players;
    aircontrol = player->cheats & CF_FLYMODE;
    config = player->config;

    buttons = playerbuttons[playerindex];
    oldbuttons = oldplayerbuttons[playerindex];

    move->forwardmove = move->sidemove = move->pitchmove = move->angleturn = 0;

    /* check for button held by passing oldbuttons = 0 */
    move->crouchheld = P_ButtonOrShift(BB_CROUCH, BB_LOOK, BB_LOOKDOWN, buttons, 0);
    move->jumpheld = P_ButtonOrShift(BB_JUMP, BB_LOOK, BB_LOOKUP, buttons, 0);
    move->crouch =  move->crouchheld && !move->jumpheld;
    move->jump =   !move->crouchheld &&  P_ButtonOrShift(BB_JUMP, BB_LOOK, BB_LOOKUP, buttons, oldbuttons);

    if(config->autorun) // [Immorpher] New autorun option
        speed = (buttons & BB_SPEED) < 1;
    else
        speed = (buttons & BB_SPEED) > 0;

    movespeed = (move->crouchheld && !aircontrol) ? 2 : speed;
    stickmove = stickstrafe = stickturn = stickvlook = 0;

    {
        int stick, x, y, mousebits;
        u32 stickval;

        mousebits = *&mouse_bit_pattern;
        stick = P_AdjustStickMode(player->controls->stick, buttons);
        stickval = P_GetStick(stick, ticbuttons[playerindex],
                              config->movesensitivity,
                              mousebits & (1<<playerindex));
        x = ((int) (stickval & 0xffff0000)) >> 14;
        y = ((int)(stickval << 16)) >> 14;
        if (stick & STICK_TURN)
            stickturn = x;
        if (stick & STICK_STRAFE)
            stickstrafe = x;
        if (stick & STICK_MOVE)
            stickmove = y;
        if (stick & STICK_VLOOK)
            stickvlook = y;

        if (player->controls2)
        {
            stick = P_AdjustStickMode(player->controls2->stick, buttons);
            stickval = P_GetStick(stick, ticbuttons[playerindex+1],
                                  config->movesensitivity,
                                  mousebits & (1<<(playerindex+1)));
            x = ((int) (stickval & 0xffff0000)) >> 14;
            y = ((int)(stickval << 16)) >> 14;
            if (stick & STICK_TURN)
                stickturn = ABSMAX(stickturn, x);
            if (stick & STICK_STRAFE)
                stickstrafe = ABSMAX(stickstrafe, x);
            if (stick & STICK_MOVE)
                stickmove = ABSMAX(stickmove, y);
            if (stick & STICK_VLOOK)
                stickvlook = ABSMAX(stickvlook, y);
        }
    }

    /* button modifiers */
    if (buttons & BB_STRAFE)
    {
        if (buttons & BB_LEFT)
            buttons = (buttons & BB_LEFT) | BB_STRAFELEFT;
        if (buttons & BB_RIGHT)
            buttons = (buttons & BB_RIGHT) | BB_STRAFERIGHT;
    }
    if (buttons & BB_LOOK)
    {
        if (buttons & BB_STRAFELEFT)
            buttons = (buttons & BB_STRAFELEFT) | BB_LEFT;
        if (buttons & BB_STRAFERIGHT)
            buttons = (buttons & BB_STRAFERIGHT) | BB_RIGHT;
        if (buttons & BB_FORWARD)
            buttons = (buttons & BB_FORWARD) | BB_LOOKUP;
        if (buttons & BB_BACK)
            buttons = (buttons & BB_BACK) | BB_LOOKDOWN;
    }

    /* use two stage accelerative vlook */
    if (((buttons & BB_LOOKUP) && (oldbuttons & BB_LOOKUP)))
        player->pitchheld++;
    else if (((buttons & BB_LOOKDOWN) && (oldbuttons & BB_LOOKDOWN)))
        player->pitchheld++;
    else
        player->pitchheld = 0;

    if (player->pitchheld >= SLOWTURNTICS)
        player->pitchheld = SLOWTURNTICS-1;

    /* vertical look */
    if (BB_LOOKUP & buttons)
        move->pitchmove = config->verticallook * (angleturn[player->pitchheld + (speed * SLOWTURNTICS)] << 18);
    if (BB_LOOKDOWN & buttons)
        move->pitchmove = config->verticallook * -(angleturn[player->pitchheld + (speed * SLOWTURNTICS)] << 18);
    if (stickvlook && !(buttons & (BB_LOOKUP | BB_LOOKDOWN)))
        move->pitchmove = ((config->looksensitivity * 8) + 233) * stickvlook * config->verticallook * 4;

    if (BB_FORWARD & buttons)
        move->forwardmove = movesteps[movespeed];
    if (BB_BACK & buttons)
        move->forwardmove = -movesteps[movespeed];
    if (stickmove && !(buttons & (BB_FORWARD | BB_BACK)))
        move->forwardmove = FixedMul(movesteps[movespeed], stickmove);

    /* use two stage accelerative turning on the joypad  */
    if (((buttons & BB_LEFT) && (oldbuttons & BB_LEFT)))
        player->turnheld++;
    else if (((buttons & BB_RIGHT) && (oldbuttons & BB_RIGHT)))
        player->turnheld++;
    else
        player->turnheld = 0;

    if (player->turnheld >= SLOWTURNTICS)
        player->turnheld = SLOWTURNTICS-1;

    /* strafe movement  */
    if (buttons & BB_STRAFELEFT)
        move->sidemove -= movesteps[movespeed];
    if (buttons & BB_STRAFERIGHT)
        move->sidemove += movesteps[movespeed];
    if (stickstrafe && !(buttons & (BB_STRAFELEFT | BB_STRAFERIGHT)))
        move->sidemove += FixedMul(movesteps[movespeed], stickstrafe);

    /* horizontal turning */
    if (buttons & BB_LEFT)
        move->angleturn =  angleturn[player->turnheld + (speed * SLOWTURNTICS)] << 17;
    if (buttons & BB_RIGHT)
        move->angleturn = -angleturn[player->turnheld + (speed * SLOWTURNTICS)] << 17;
    if (stickturn && !(buttons & (BB_LEFT | BB_RIGHT)))
        move->angleturn = ((config->looksensitivity * 8) + 233) * -stickturn * 2;

    /* final clamp of movement vector */
    fixed_t mag = FixedMul(move->forwardmove, move->forwardmove)
        + FixedMul(move->sidemove, move->sidemove);
    if (mag > ((MAXPLAYERSPEED * (MAXPLAYERSPEED >> 8)) >> 8))
    {
        mag = D_rsqrtf(mag * (1.0f / (f32) FRACUNIT)) * (f32) MAXPLAYERSPEED;
        move->forwardmove = FixedMul(mag, move->forwardmove);
        move->sidemove = FixedMul(mag, move->sidemove);
    }

    mo = player->mo;

    /* if slowed down to a stop, change to a standing frame */
    if (!mo->momx && !mo->momy && move->forwardmove == 0 && move->sidemove == 0 )
    {   /* if in a walking frame, stop moving */
        if (mo->state == &states[S_PLAY_RUN1]
                || mo->state == &states[S_PLAY_RUN2]
                || mo->state == &states[S_PLAY_RUN3]
                || mo->state == &states[S_PLAY_RUN4])
            P_SetMobjState (mo, S_PLAY);
    }
}

/*
===============================================================================

                        movement

===============================================================================
*/

/*
==================
=
= P_Thrust
=
= moves the given origin along a given angle
=
==================
*/

SEC_GAME void P_Thrust (player_t *player, angle_t angle, fixed_t move) // 800225BC
{
    angle >>= ANGLETOFINESHIFT;
    player->mo->momx += FixedMul(vblsinframe[0] * move, finecosine(angle));
    player->mo->momy += FixedMul(vblsinframe[0] * move, finesine(angle));
}



/*
==================
=
= P_CalcHeight
=
= Calculate the walking / running height adjustment
=
==================
*/

SEC_GAME void P_CalcHeight (player_t *player) // 80022670
{
    int         angle;
    fixed_t     bob;
    fixed_t     val;
    fixed_t     viewheight;

    /* */
    /* regular movement bobbing (needs to be calculated for gun swing even */
    /* if not on ground) */
    /* OPTIMIZE: tablify angle  */
    /* */
    val = player->mo->momx;
    player->bob = FixedMul(val, val);
    val = player->mo->momy;
    player->bob += FixedMul(val, val);

    player->bob >>= 2;
    if (player->bob > Settings.MotionBob)
        player->bob = Settings.MotionBob;

    viewheight = FixedMul (VIEWHEIGHT, crouchease[player->crouchtimer]);

    if (!player->onground)
    {
        player->viewz = player->mo->z + viewheight;
        if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
            player->viewz = player->mo->ceilingz-4*FRACUNIT;
        return;
    }

    angle = (FINEANGLES/40*ticon)&(FINEANGLES-1);
    bob = FixedMul((player->bob / 2), finesine(angle));

    //ST_DebugPrint("bob %x",FixedMul((player->bob / 2), finesine(angle)));
    //ST_DebugPrint("bob2 %x",FixedMul2((player->bob / 2), finesine(angle)));

    //ST_DebugPrint("bobdiv %x",FixedDiv2(0x49003, 0x2));
    //ST_DebugPrint("bobdiv2 %x",FixedMul3(0x49003, 0x2));

    /* */
    /* move viewheight */
    /* */
    if (player->playerstate == PST_LIVE)
    {
        player->viewheight += player->deltaviewheight;
        if (player->viewheight > viewheight)
        {
            player->viewheight = viewheight;
            player->deltaviewheight = 0;
        }
        if (player->viewheight < viewheight/2)
        {
            player->viewheight = viewheight/2;
            if (player->deltaviewheight <= 0)
                player->deltaviewheight = 1;
        }

        if (player->deltaviewheight)
        {
            player->deltaviewheight += FRACUNIT/2;
            if (!player->deltaviewheight)
                player->deltaviewheight = 1;
        }
    }
    player->viewz = player->mo->z + player->viewheight + bob;
    if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
        player->viewz = player->mo->ceilingz-4*FRACUNIT;
}

#define MAXLOOKANGLE 0x30000000
#define LOOKSPRINGRESET (0x4000 << FRACBITS)
#define FLYTHRUST 0x16000


/*
=================
=
= P_MovePlayer
=
=================
*/

SEC_GAME void P_MovePlayer (player_t *player, const buildmove_t* move) // 8002282C
{
    int aircontrol;

    player->mo->angle += vblsinframe[0] * move->angleturn;

    aircontrol = (player->cheats & CF_FLYMODE);

    // [nova] change pitch along with angleturn
    if (move->pitchmove)
    {
        player->lookspring = 0;
        player->pitch += move->pitchmove;
        if (player->pitch >= ANG180 && player->pitch <= 0xd0000000)
            player->pitch = 0xd0000000 + FINEANGLES;
        else if (player->pitch >= 0x30000000)
            player->pitch = 0x30000000 - FINEANGLES;
    }

    // [nova] handle lookspring
    if (player->config->lookspring)
    {
        if (player->pitch == 0)
            player->lookspring = 0;

        if (player->lookspring >= LOOKSPRINGRESET)
        {
            int angle = angleturn[(player->lookspring - LOOKSPRINGRESET) >> FRACBITS] << 19;
            angle = player->pitch > 0 ? -angle : angle;

            if ((player->pitch > 0) != (player->pitch + angle > 0))
                player->pitch = 0;
            else
                player->pitch += angle;

            if (player->lookspring < LOOKSPRINGRESET + 10 * FRACUNIT)
                player->lookspring += FRACUNIT;
        }
        else
        {
            if (move->forwardmove || move->sidemove)
                player->lookspring += D_abs(move->forwardmove) + D_abs(move->sidemove);
            else if (player->lookspring >= (16 << FRACBITS))
                player->lookspring += FRACUNIT;

            if (player->lookspring >= (64 << FRACBITS) || player->lookspring < 0)
                player->lookspring = LOOKSPRINGRESET;
        }
    }
    else
    {
        player->lookspring = 0;
    }

    if(player->onground || aircontrol)
    {
        if (move->forwardmove)
        {
            if (aircontrol && player->pitch)
            {
                fixed_t forwardmove;
                int finepitch;

                finepitch = player->pitch >> ANGLETOFINESHIFT;
                forwardmove = FixedMul(move->forwardmove, finecosine(finepitch));
                P_Thrust (player, player->mo->angle, forwardmove);
                player->mo->momz += FixedMul(vblsinframe[0] * forwardmove, finesine(finepitch));
            }
            else
            {
                P_Thrust (player, player->mo->angle, move->forwardmove);
            }
        }

        if (move->sidemove)
            P_Thrust (player, player->mo->angle-ANG90, move->sidemove);
    }

    if ((move->forwardmove || move->sidemove) && player->mo->state == &states[S_PLAY]
            && (!aircontrol || player->onground))
        P_SetMobjState (player->mo, S_PLAY_RUN1);

    if (aircontrol ? move->crouchheld : move->crouch)
    {
        if ((!aircontrol || player->onground || player->crouchtimer > 0) && player->crouchtimer < ARRAYLEN(crouchease) - 1)
        {
            player->crouchtimer++;
            player->mo->height = FixedMul (player->mo->info->height, crouchease[player->crouchtimer]);
            player->deltaviewheight -= 1;
        }
        if (aircontrol && player->crouchtimer == 0)
            player->mo->momz -= vblsinframe[0] * FLYTHRUST;
    }
    else if (player->crouchtimer > 0)
    {
        int origviewheight, viewheight, testheight;
        sector_t *sec = player->mo->subsector->sector;

        testheight = FixedMul (player->mo->info->height, crouchease[player->crouchtimer - 1]);
        if (sec->ceilingheight - sec->floorheight >= testheight)
        {
            origviewheight = FixedMul (VIEWHEIGHT, crouchease[player->crouchtimer]);
            player->crouchtimer--;
            viewheight = FixedMul (VIEWHEIGHT, crouchease[player->crouchtimer]);
            player->deltaviewheight += viewheight - origviewheight;
            player->mo->height = testheight;
        }
    }

    if (aircontrol)
    {
        if (move->jumpheld)
            player->mo->momz += vblsinframe[0] * FLYTHRUST;
    }
    else
    {
        if (move->jump
                && (player->onground || player->falltimer < JUMPGRACE)
                && player->mo->momz < JUMPTHRUST)
        {
            player->mo->momz = JUMPTHRUST;
            player->falltimer = 30;
        }
    }
}


/*
=================
=
= P_DeathThink
=
=================
*/

SEC_GAME void P_DeathThink (player_t *player) // 80022914
{
    angle_t     angle, delta;
    fixed_t zdist, xydist;
    int damagefade = 0;
    int mockrandom; // [Immorpher] store mock text randomizer

    P_MovePsprites (player);

    /* fall to the ground */
    if (player->viewheight > 8*FRACUNIT)
        player->viewheight -= FRACUNIT;

    player->onground = (player->mo->z <= player->mo->floorz);
    player->falltimer = 0;
    player->crouchtimer = 0;

    P_CalcHeight (player);

    if (player->attacker && player->attacker != player->mo)
    {
        angle = R_PointToAngle2 (player->mo->x, player->mo->y, player->attacker->x, player->attacker->y);
        delta = angle - player->mo->angle;
        if (delta < ANG5 || delta > (unsigned)-ANG5)
        {
            player->mo->angle = angle;
            damagefade++;
        }
        else if (delta < ANG180)
            player->mo->angle += ANG5;
        else
            player->mo->angle -= ANG5;

        // [nova] change pitch to look at killer too
        zdist = (player->attacker->z + (player->attacker->height >> 1)) - (player->mo->z + player->viewheight);
        xydist = P_AproxDistance(player->attacker->x - player->mo->x, player->attacker->y - player->mo->y);
        angle = R_PointToAngle2 (0, 0, xydist, zdist);
        delta = angle - player->pitch;
        if (delta < ANG5 || delta > (unsigned)-ANG5)
        {
            player->pitch = angle;
            damagefade++;
        }
        else if (delta < ANG180)
            player->pitch += ANG5;
        else
            player->pitch -= ANG5;

        if (player->pitch >= ANG180 && player->pitch <= 0xd0000000)
            player->pitch = 0xd0000000 + FINEANGLES;
        else if (player->pitch >= 0x30000000)
            player->pitch = 0x30000000 - FINEANGLES;

        /* looking at killer, so fade damage flash down */
        if (damagefade == 2 && player->damagecount)
            player->damagecount--;
    }
    else if (player->damagecount)
        player->damagecount--;

    /* mocking text */
    if ((ticon - deathmocktics) > MAXMOCKTIME)
    {
        do { // [Immorpher] Prevent mock string from repeating twice
            mockrandom = (I_Random()+ticon) % 51; // Updated randomizer for more fun
        } while(player->messages[0] == mockstrings1[mockrandom]);

        P_PushMessage(player, mockstrings1[mockrandom], 0xff200000, 2*MSGTICS);
        deathmocktics = ticon;
    }

    if (player->viewheight <= 8*FRACUNIT)
    {
        int extra;
        int playerindex;
        bool respawnpressed;
        controls2_t *ctrl2;

        playerindex = player - players;
        extra = player->automapflags ? 0 : PAD_B|PAD_L_TRIG|PAD_R_TRIG;
        respawnpressed = (ticbuttons[playerindex] & (PAD_A|PAD_Z_TRIG|ALL_CBUTTONS|extra) & ~player->controls->buttons[BT_MAP]) != 0;
        ctrl2 = player->controls2;

        if (ctrl2)
        {
            int mask = PAD_L_TRIG|PAD_R_TRIG|ALL_CBUTTONS;
            if (ctrl2->a != BT_MAP)
                mask |= PAD_A;
            if (ctrl2->b != BT_MAP)
                mask |= PAD_B;
            if (ctrl2->z != BT_MAP)
                mask |= PAD_Z_TRIG;
            if (ticbuttons[playerindex + 1] & mask)
                respawnpressed = true;
        }

        if (respawnpressed)
        {
            player->playerstate = PST_REBORN;
            if (!customskill.pistol_start)
                doLoadSave = true;
        }
    }

    if (player->bonuscount)
        player->bonuscount -= 1;

    // [d64] - recoil pitch from weapons
    if (player->recoilpitch)
        player->recoilpitch = (((player->recoilpitch << 2) - player->recoilpitch) >> 2);

    if(player->bfgcount)
    {
        player->bfgcount -= 6;

        if(player->bfgcount < 0)
            player->bfgcount = 0;
    }
}

/*
===============================================================================
=
= P_PlayerInSpecialSector
=
= Called every tic frame that the player origin is in a special sector
=
===============================================================================
*/

SEC_GAME void P_PlayerInSpecialSector (player_t *player, sector_t *sec) // 80022B1C
{
    fixed_t speed;

    if (player->mo->z != sec->floorheight)
        return;     /* not all the way down yet */

    if(sec->flags & MS_SECRET)
    {
        player->secretcount++;
        P_PushMessage(player, JPMSG(41, "You found a secret area!"), 0x00ffff00, MSGTICS);
        sec->flags &= ~MS_SECRET;
    }

    if(sec->flags & MS_DAMAGEX5)    /* NUKAGE DAMAGE */
    {
        if(!player->powers[pw_ironfeet])
        {
            if ((gamevbls < (int)gametic) && !(gametic & 31))
                  P_DamageMobj(player->mo, NULL, NULL, 5);
        }
    }

    if(sec->flags & MS_DAMAGEX10)    /* HELLSLIME DAMAGE */
    {
        if(!player->powers[pw_ironfeet])
        {
            if ((gamevbls < (int)gametic) && !(gametic & 31))
                  P_DamageMobj(player->mo, NULL, NULL, 10);
        }
    }

    if(sec->flags & MS_DAMAGEX20)    /* SUPER HELLSLIME DAMAGE */
    {
        if(!player->powers[pw_ironfeet] || (P_Random() < 5))
        {
            if ((gamevbls < (int)gametic) && !(gametic & 31))
                  P_DamageMobj(player->mo, NULL, NULL, 20);
        }
    }

    if(sec->flags & MS_SCROLLFLOOR)
    {
        if(sec->flags & MS_SCROLLFAST)
            speed = 0x3000;
        else
            speed = 0x1000;

        if(sec->flags & MS_SCROLLLEFT)
        {
            P_Thrust(player, ANG180, speed);
        }
        else if(sec->flags & MS_SCROLLRIGHT)
        {
            P_Thrust(player, 0, speed);
        }

        if(sec->flags & MS_SCROLLUP)
        {
            P_Thrust(player, ANG90, speed);
        }
        else if(sec->flags & MS_SCROLLDOWN)
        {
            P_Thrust(player, ANG270, speed);
        }
    }
}

/*
=================
=
= P_PlayerThink
=
=================
*/

SEC_GAME void P_PlayerThink (player_t *player) // 80022D60
{
    if (gamepaused)
        return;

    u32         buttons, oldbuttons;
    int         weapon, weaponsearch;
    int         playerindex;
    sector_t   *sec;
    buildmove_t move;

    playerindex = player - players;
    buttons = playerbuttons[playerindex];
    oldbuttons = oldplayerbuttons[playerindex];

    player->attackdown = !!(buttons & BB_ATTACK);

    /* */
    /* check for weapon change */
    /* */
    if (player->playerstate == PST_LIVE)
    {
        weapon = player->pendingweapon;
        if (weapon == wp_nochange)
            weapon = player->readyweapon;

        bool shiftback = (buttons & BB_WEAPONFORWARD) && (buttons & BB_ATTACK) && !(oldbuttons & BB_ATTACK);
        // [nova] use goldeneye/pd style weapon back
        if (shiftback || ((buttons & BB_WEAPONBACKWARD) && !(oldbuttons & BB_WEAPONBACKWARD)))
        {
            // [nova] always cycle weapons
            weaponsearch = weapon;
            if (player->cycledir > 0)
            {
                // override into a back command if the weapon is still lowering
                if (shiftback && player->pendingweapon != wp_nochange && player->pendingweapon != player->readyweapon)
                {
                    do
                    {
                        weaponsearch--;
                        if (weaponsearch < 0)
                            weaponsearch = NUMWEAPONS - 1;
                    }
                    while (!player->weaponowned[weaponsearch] && weaponsearch != weapon);
                    player->weaponwheeltarget -= WHEEL_WEAPON_SIZE;
                }
                player->cycledir = -1;
            }
            do
            {
                weaponsearch--;
                if (weaponsearch < 0)
                    weaponsearch = NUMWEAPONS - 1;
            }
            while (!player->weaponowned[weaponsearch] && weaponsearch != weapon);

            player->pendingweapon = weaponsearch;
            player->weaponwheeltarget -= WHEEL_WEAPON_SIZE;
        }
        else if (buttons & BB_WEAPONFORWARD)
        {
            if (!(oldbuttons & BB_WEAPONFORWARD))
            {
                // [nova] always cycle weapons
                weaponsearch = weapon;
                player->cycledir = 1;
                do
                {
                    weaponsearch++;
                    if (weaponsearch >= NUMWEAPONS)
                        weaponsearch = 0;
                }
                while (!player->weaponowned[weaponsearch] && weaponsearch != weapon);

                player->pendingweapon = weaponsearch;
                player->weaponwheeltarget += WHEEL_WEAPON_SIZE;
            }
        }
        else
        {
            player->cycledir = 0;
        }
    }

    P_PlayerMobjThink(player->mo);
    P_BuildMove(player, &move);

    sec = player->mo->subsector->sector;
    if (sec->flags & (MS_SECRET | MS_DAMAGEX5 | MS_DAMAGEX10 | MS_DAMAGEX20 | MS_SCROLLFLOOR))
        P_PlayerInSpecialSector(player, sec);

    if (player->addfov > 0)
    {
        player->addfov -= ANG1*8;
        if (player->addfov < 0)
            player->addfov = 0;
    }
    if (player->addfov < 0)
    {
        player->addfov += ANG1*8;
        if (player->addfov > 0)
            player->addfov = 0;
    }

    if (player->playerstate == PST_DEAD)
    {
        P_DeathThink(player);
        return;
    }

    /* */
    /* chain saw run forward */
    /* */
    if (player->mo->flags & MF_JUSTATTACKED)
    {
        move.angleturn = 0;
        move.forwardmove = 0xc800;
        move.sidemove = 0;
        move.pitchmove = 0;
        player->mo->flags &= ~MF_JUSTATTACKED;
    }

    /* */
    /* move around */
    /* reactiontime is used to prevent movement for a bit after a teleport */
    /* */

    if (player->mo->reactiontime)
        player->mo->reactiontime--;
    else
        P_MovePlayer(player, &move);

    P_CalcHeight(player);

    /* */
    /* check for use */
    /* */

    if ((buttons & BB_USE) && !(oldbuttons & BB_USE))
        P_UseLines(player);

    /* */
    /* cycle psprites */
    /* */

    P_MovePsprites(player);

    /* */
    /* counters */
    /* */

    if (gamevbls < gametic)
    {
        if (player->powers[pw_strength] > 1)
            player->powers[pw_strength]--;  /* strength counts down to diminish fade */

        if (player->powers[pw_invulnerability])
            player->powers[pw_invulnerability]--;

        if (player->powers[pw_invisibility])
        {
            player->powers[pw_invisibility]--;
            if (!player->powers[pw_invisibility])
            {
                player->mo->flags &= ~MF_SHADOW;
            }
            else if ((player->powers[pw_invisibility] < 61) && !(player->powers[pw_invisibility] & 7))
            {
                player->mo->flags ^= MF_SHADOW;
            }
        }

        if (player->powers[pw_infrared])
            player->powers[pw_infrared]--;

        if (player->powers[pw_ironfeet])
            player->powers[pw_ironfeet]--;

        if (player->damagecount)
            player->damagecount--;

        if (player->bonuscount)
            player->bonuscount--;

        // [d64] - recoil pitch from weapons
        if (player->recoilpitch)
            player->recoilpitch = (((player->recoilpitch << 2) - player->recoilpitch) >> 2);

        if(player->bfgcount)
        {
            player->bfgcount -= 6;

            if(player->bfgcount < 0)
                player->bfgcount = 0;
        }
    }
}
