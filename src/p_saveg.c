#include "doomdef.h"
#include "p_local.h"
#include "r_local.h"
#include "p_saveg.h"

static u32 write_button(u32 button)
{
    if (button >= PAD_Z_TRIG)
        return button >> 19;
    if (button >= PAD_RIGHT)
        return button >> 18;
    return button >> 16;
}

static u32 read_button(u32 button)
{
    if (button & 0x1000)
        return PAD_A;
    if (button & 0x800)
        return PAD_B;
    if (button & 0x400)
        return PAD_Z_TRIG;
    if (button & 0x200)
        return PAD_UP;
    if (button & 0x100)
        return PAD_DOWN;
    if (button & 0x80)
        return PAD_LEFT;
    if (button & 0x40)
        return PAD_RIGHT;
    if (button & 0x20)
        return PAD_L_TRIG;
    if (button & 0x10)
        return PAD_R_TRIG;
    if (button & 0x8)
        return PAD_UP_C;
    if (button & 0x4)
        return PAD_DOWN_C;
    if (button & 0x2)
        return PAD_LEFT_C;
    if (button & 0x1)
        return PAD_RIGHT_C;
    return 0;
}

void P_ArchivePlayerConfig(int pi, savedplayerconfig_t *buf)
{
    const controls_t *controls = &CurrentControls[pi];
    const controls2_t *controls2 = &CurrentControls2[pi>>1];

    buf->looksensitivity = playerconfigs[pi].looksensitivity;
    buf->movesensitivity = playerconfigs[pi].movesensitivity;
    buf->crosshair = playerconfigs[pi].crosshair;
    buf->autorun = playerconfigs[pi].autorun;
    buf->autoaim = playerconfigs[pi].autoaim;
    buf->vlookinverted = playerconfigs[pi].verticallook == -1 ? 1 : 0;
    buf->lookspring = playerconfigs[pi].lookspring;
    buf->hue = playerconfigs[pi].hue >> 2;
    buf->saturation = playerconfigs[pi].saturation >> 2;
    buf->value = playerconfigs[pi].value >> 2;
    if (Settings.ControlPreset[pi] == -1)
    {
        buf->customconfig = 1;
        buf->confignum = 0;
    }
    else
    {
        buf->customconfig = 0;
        buf->confignum = Settings.ControlPreset[pi];
    }

    buf->bt_right = write_button(controls->buttons[BT_RIGHT]);
    buf->bt_left = write_button(controls->buttons[BT_LEFT]);
    buf->bt_forward = write_button(controls->buttons[BT_FORWARD]);
    buf->bt_back = write_button(controls->buttons[BT_BACK]);
    buf->bt_attack = write_button(controls->buttons[BT_ATTACK]);
    buf->bt_use = write_button(controls->buttons[BT_USE]);
    buf->bt_map = write_button(controls->buttons[BT_MAP]);
    buf->bt_speed = write_button(controls->buttons[BT_SPEED]);
    buf->bt_strafe = write_button(controls->buttons[BT_STRAFE]);
    buf->bt_strafeleft = write_button(controls->buttons[BT_STRAFELEFT]);
    buf->bt_straferight = write_button(controls->buttons[BT_STRAFERIGHT]);
    buf->bt_weaponbackward = write_button(controls->buttons[BT_WEAPONBACKWARD]);
    buf->bt_weaponforward = write_button(controls->buttons[BT_WEAPONFORWARD]);
    buf->bt_look = write_button(controls->buttons[BT_LOOK]);
    buf->bt_lookup = write_button(controls->buttons[BT_LOOKUP]);
    buf->bt_lookdown = write_button(controls->buttons[BT_LOOKDOWN]);
    buf->bt_jump = write_button(controls->buttons[BT_JUMP]);
    buf->bt_crouch = write_button(controls->buttons[BT_CROUCH]);
    buf->stickmode = controls->stick;

    buf->stickmode2 = controls2->stick;
    buf->ctrl2a = controls2->a;
    buf->ctrl2b = controls2->b;
    buf->ctrl2z = controls2->z;
}

void P_UnArchivePlayerConfig(int pi, const savedplayerconfig_t *buf)
{
    playerconfigs[pi].looksensitivity = buf->looksensitivity;
    playerconfigs[pi].movesensitivity = buf->movesensitivity;
    playerconfigs[pi].crosshair = buf->crosshair;
    playerconfigs[pi].autorun = buf->autorun;
    playerconfigs[pi].autoaim = buf->autoaim;
    playerconfigs[pi].verticallook = buf->vlookinverted ? -1 : 1;
    playerconfigs[pi].lookspring = buf->lookspring;
    playerconfigs[pi].hue = buf->hue << 2;
    playerconfigs[pi].saturation = buf->saturation << 2;
    playerconfigs[pi].value = buf->value << 2;

    if (buf->customconfig)
        Settings.ControlPreset[pi] = -1;
    else
        Settings.ControlPreset[pi] = buf->confignum % MAXCONTROLSETUPS;
    CurrentControls[pi].buttons[BT_RIGHT] = read_button(buf->bt_right);
    CurrentControls[pi].buttons[BT_LEFT] = read_button(buf->bt_left);
    CurrentControls[pi].buttons[BT_FORWARD] = read_button(buf->bt_forward);
    CurrentControls[pi].buttons[BT_BACK] = read_button(buf->bt_back);
    CurrentControls[pi].buttons[BT_ATTACK] = read_button(buf->bt_attack);
    CurrentControls[pi].buttons[BT_USE] = read_button(buf->bt_use);
    CurrentControls[pi].buttons[BT_MAP] = read_button(buf->bt_map);
    CurrentControls[pi].buttons[BT_SPEED] = read_button(buf->bt_speed);
    CurrentControls[pi].buttons[BT_STRAFE] = read_button(buf->bt_strafe);
    CurrentControls[pi].buttons[BT_STRAFELEFT] = read_button(buf->bt_strafeleft);
    CurrentControls[pi].buttons[BT_STRAFERIGHT] = read_button(buf->bt_straferight);
    CurrentControls[pi].buttons[BT_WEAPONBACKWARD] = read_button(buf->bt_weaponbackward);
    CurrentControls[pi].buttons[BT_WEAPONFORWARD] = read_button(buf->bt_weaponforward);
    CurrentControls[pi].buttons[BT_LOOK] = read_button(buf->bt_look);
    CurrentControls[pi].buttons[BT_LOOKUP] = read_button(buf->bt_lookup);
    CurrentControls[pi].buttons[BT_LOOKDOWN] = read_button(buf->bt_lookdown);
    CurrentControls[pi].buttons[BT_JUMP] = read_button(buf->bt_jump);
    CurrentControls[pi].buttons[BT_CROUCH] = read_button(buf->bt_crouch);
    CurrentControls[pi].stick = buf->stickmode ? buf->stickmode : (STICK_MOVE | STICK_STRAFE);

    if (!(pi & 1))
    {
        int pi2 = pi>>1;

        CurrentControls2[pi2].stick = buf->stickmode2 ? buf->stickmode2 : STICK_TURN | STICK_VLOOK;
        CurrentControls2[pi2].a = buf->ctrl2a;
        CurrentControls2[pi2].b = buf->ctrl2b;
        CurrentControls2[pi2].z = buf->ctrl2z;
        if (CurrentControls2[pi2].a >= NUMBUTTONS || CurrentControls2[pi2].a <= BT_NONE)
            CurrentControls2[pi2].a = BT_NONE;
        if (CurrentControls2[pi2].b >= NUMBUTTONS || CurrentControls2[pi2].b <= BT_NONE)
            CurrentControls2[pi2].b = BT_NONE;
        if (CurrentControls2[pi2].z >= NUMBUTTONS || CurrentControls2[pi2].z <= BT_NONE)
            CurrentControls2[pi2].z = BT_NONE;
    }
}

#define INVALID_MOBJ ((u16) 0xffff)

static u16 MobjIndex(mobj_t *mo)
{
    u16 index = 0;

    if (!mo)
        return INVALID_MOBJ;

    for (mobj_t *i = mobjhead.next; i != (void*) &mobjhead; i = i->next, index++)
        if (i == mo)
            return index;

    return INVALID_MOBJ;
}

static mobj_t *MobjByIndex(u16 index)
{
    u16 j = 0;

    if (index == INVALID_MOBJ)
        return NULL;

    for (mobj_t *i = mobjhead.next; i != (void*) &mobjhead; i = i->next, j++)
        if (j == index)
            return i;

    return NULL;
}

u32 P_ArchivePlayers (u8 *savep)
{
    int       i;
    int       j;
    u32       written = 0;
    player_t* dest;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        //if (!playeringame[i])
            //continue;

        dest = (player_t *)savep;
        D_memcpy (dest,&players[i],sizeof(player_t));
        dest->mo = NULL;
        dest->messages[0] = dest->messages[1] = dest->messages[2] = NULL;
        dest->attacker = (void*)(u32)MobjIndex(dest->attacker);
        dest->controls = NULL;
        if (dest->lastsoundsector)
            dest->lastsoundsector = (void*)(((sector_t*)dest->lastsoundsector) - sectors);
        for (j=0 ; j<NUMPSPRITES ; j++)
            if (dest->psprites[j].state)
                dest->psprites[j].state = (state_t *)(dest->psprites[j].state-states);
        written += ALIGN(sizeof(player_t), 8);
        savep += ALIGN(sizeof(player_t), 8);
    }
    return written;
}

u32 P_UnArchivePlayers (const u8 *savep)
{
    int i;
    int j;
    u32 read = 0;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        //if (!playeringame[i])
            //continue;

        D_memcpy (&players[i],(void*)savep, sizeof(player_t));

        players[i].mo = NULL;
        players[i].controls = &CurrentControls[i];
        players[i].config = &playerconfigs[i];
        players[i].messagetics[0] = players[i].messagetics[1] = players[i].messagetics[2] = 0;
        players[i].messages[0] = players[i].messages[1] = players[i].messages[2] = NULL;
        if (players[i].lastsoundsector)
            players[i].lastsoundsector =  &sectors[(int)players[i].lastsoundsector];

        for (j=0 ; j<NUMPSPRITES ; j++)
            if (players[i]. psprites[j].state)
                players[i]. psprites[j].state = &states[(int)players[i].psprites[j].state];

        read += ALIGN(sizeof(player_t), 8);
        savep += ALIGN(sizeof(player_t), 8);
    }
    return read;
}

typedef struct ALIGNED(8) {
    s16 floorheight;
    s16 ceilingheight;
    s16 floorpic;
    s16 ceilingpic;
    u16 special;
    u16 tag;
    u16 flags;
    u16 lightlevel;
    s16 xoffset;
    s16 yoffset;
    u16 soundtarget;
    u16 colors[5];
} savedsector_t;

u32 P_ArchiveSectors (u8 *savep, u32 savepsize, u32 *start)
{
    int       i;
    sector_t* sec;
    u32       written = 0;

    for (i=*start, sec = sectors ; i<numsectors ; i++,sec++)
    {
        savedsector_t* put;

        if ((written + sizeof(savedsector_t)) > savepsize)
            break;

        put = (void *)savep;
        put->floorheight = sec->floorheight >> FRACBITS;
        put->ceilingheight = sec->ceilingheight >> FRACBITS;
        put->floorpic = sec->floorpic;
        put->ceilingpic = sec->ceilingpic;
        put->special = sec->special;
        put->tag = sec->tag;
        put->flags = sec->flags;
        put->lightlevel = sec->lightlevel;
        put->xoffset = sec->xoffset >> FRACBITS;
        put->yoffset = sec->yoffset >> FRACBITS;
        put->soundtarget = MobjIndex(sec->soundtarget);
        for (int j = 0; j < ARRAYLEN(put->colors); j++)
            put->colors[j] = sec->colors[j];
        written += sizeof(savedsector_t);
        savep += sizeof(savedsector_t);
    }
    *start = i;
    return written;
}

u32 P_UnArchiveSectors (const u8 *savep, u32 savepsize, u32 *start)
{
    int       i;
    sector_t* sec;
    u32 read = 0;

    // do sectors
    for (i=*start, sec = sectors ; i<numsectors ; i++,sec++)
    {
        savedsector_t* get;

        if ((read + sizeof(savedsector_t)) > savepsize)
            break;

        get = (void *)savep;
        sec->floorheight = get->floorheight << FRACBITS;
        sec->ceilingheight = get->ceilingheight << FRACBITS;
        sec->floorpic = get->floorpic;
        sec->ceilingpic = get->ceilingpic;
        sec->special = get->special;
        sec->tag = get->tag;
        sec->flags = get->flags;
        sec->lightlevel = get->lightlevel;
        sec->xoffset = get->xoffset << FRACBITS;
        sec->yoffset = get->yoffset << FRACBITS;
        for (int j = 0; j < ARRAYLEN(get->colors); j++)
            sec->colors[j] = get->colors[j];
        sec->specialdata = 0;
        sec->soundtarget = (void*) (u32) get->soundtarget;

        read += sizeof(savedsector_t);
        savep += sizeof(savedsector_t);
    }
    *start = i;
    return read;
}

typedef struct ALIGNED(8) {
    u32 flags;
    u16 special;
    u16 tag;
} savedline_t;

typedef struct ALIGNED(8) {
    s16 textureoffset;
    s16 rowoffset;
    s16 toptexture;
    s16 bottomtexture;
    s16 midtexture;
} savedside_t;

u32 P_ArchiveLines (u8 *savep, u32 savepsize, u32 *start)
{
    int     i;
    int     j;
    line_t* li;
    u32     written = 0;

    for (i=*start, li = lines ; i<numlines ; i++,li++)
    {
        savedline_t* put;

        u32 size = sizeof(savedline_t)
            + (li->sidenum[0] == -1 ? 0 : sizeof(savedside_t))
            + (li->sidenum[1] == -1 ? 0 : sizeof(savedside_t));

        if ((written + size) > savepsize)
            break;

        put = (void *)savep;

        put->flags = li->flags;
        put->special = li->special;
        put->tag = li->tag;
        written += sizeof(savedline_t);
        savep += sizeof(savedline_t);

        for (j=0 ; j<2 ; j++)
        {
            savedside_t* sput;
            side_t* si;

            if (li->sidenum[j] == -1)
                continue;

            sput = (void *)savep;
            si = &sides[li->sidenum[j]];

            sput->textureoffset = si->textureoffset >> FRACBITS;
            sput->rowoffset = si->rowoffset >> FRACBITS;
            sput->toptexture = si->toptexture;
            sput->bottomtexture = si->bottomtexture;
            sput->midtexture = si->midtexture;
            written += sizeof(savedside_t);
            savep += sizeof(savedside_t);
        }
    }

    *start = i;
    return written;
}

u32 P_UnArchiveLines (const u8 *savep, u32 savepsize, u32 *start)
{
    int     i;
    int     j;
    line_t* li;
    u32     read = 0;

    for (i=*start, li = lines ; i<numlines ; i++,li++)
    {
        savedline_t* get;

        u32 size = sizeof(savedline_t)
            + (li->sidenum[0] == -1 ? 0 : sizeof(savedside_t))
            + (li->sidenum[1] == -1 ? 0 : sizeof(savedside_t));

        if ((read + size) > savepsize)
            break;

        get = (void *)savep;
        li->flags = get->flags;
        li->special = get->special;
        li->tag = get->tag;

        read += sizeof(savedline_t);
        savep += sizeof(savedline_t);

        for (j=0 ; j<2 ; j++)
        {
            savedside_t* sget;
            side_t* si;

            if (li->sidenum[j] == -1)
                continue;

            sget = (void *)savep;
            si = &sides[li->sidenum[j]];

            si->textureoffset = sget->textureoffset << FRACBITS;
            si->rowoffset = sget->rowoffset << FRACBITS;
            si->toptexture = sget->toptexture;
            si->bottomtexture = sget->bottomtexture;
            si->midtexture = sget->midtexture;

            read += sizeof(savedside_t);
            savep += sizeof(savedside_t);
        }
    }

    *start = i;
    return read;
}

void P_ArchiveActiveMacro (savedmacrosheader_t *header)
{
    header->cameratarget = MobjIndex(cameratarget);

    if (!activemacro)
        return;

    header->active = 1;
    header->activemacro = activemacro - macros[0];
    header->macrointeger = macrointeger;
    header->macrocounter = macrocounter;
    if (macrocounter > 0)
        header->restartmacro = restartmacro - macros[0];
    header->macroactivator = MobjIndex(macroactivator);
    header->macroline = macroline - lines;
    header->macroidx1 = macroidx1;
    header->macroidx2 = macroidx2;
    for (int i = 0; i < ARRAYLEN(macroqueue); i++)
    {
        header->macroqueue[i].tag = macroqueue[i].tag;
        header->macroqueue[i].activator = MobjIndex(macroqueue[i].activator);
    }
}

void P_UnArchiveActiveMacro (const savedmacrosheader_t *header)
{
    cameratarget = (void*)(u32)header->cameratarget;

    if (!header->active)
        return;

    activemacro = macros[0] + header->activemacro;
#ifndef NDEBUG
    activemacroidx = -1;
    for (int i = 0; i < nummacros; i++)
    {
        if (activemacro >= macros[i] && (i == nummacros - 1 || activemacro < macros[i+1]))
        {
            activemacroidx = i - 1;
            break;
        }
    }
#endif
    macrointeger = header->macrointeger;
    macrocounter = header->macrocounter;
    if (macrocounter > 0)
        restartmacro = macros[0] + header->restartmacro;
    macroactivator = (void*)(u32)header->macroactivator;
    macroline = &lines[header->macroline];
    D_memcpy(&macrotempline, macroline, sizeof(line_t));
    macroidx1 = header->macroidx1;
    macroidx2 = header->macroidx2;
    for (int i = 0; i < ARRAYLEN(macroqueue); i++)
    {
        macroqueue[i].tag = header->macroqueue[i].tag;
        macroqueue[i].activator = (void*)(u32)header->macroqueue[i].activator;
    }
}

typedef struct ALIGNED(8) {
    u16 id;
    u16 special;
    u16 tag;
} savedmacro_t;

u32 P_ArchiveMacros (u8 *savep, u32 savepsize, u32 *start)
{
    int      i;
    macro_t *macro;
    u32      written = 0;

    for (i=*start, macro = macros[i]; i<nummacros ; macro = macros[++i])
    {
        savedmacro_t* put;

        if ((written + sizeof(savedmacro_t)) > savepsize)
            break;

        put = (void *)savep;
        put->id = macro->id;
        put->special = macro->special;
        put->tag = macro->tag;

        written += sizeof(savedmacro_t);
        savep += sizeof(savedmacro_t);
    }
    *start = i;
    return written;
}

u32 P_UnArchiveMacros (const u8 *savep, u32 savepsize, u32 *start)
{
    int       i;
    macro_t* macro;
    u32 read = 0;

    for (i=*start, macro = macros[i]; i<nummacros ; macro = macros[++i])
    {
        savedmacro_t* get;

        if ((read + sizeof(savedmacro_t)) > savepsize)
            break;

        get = (void *)savep;
        macro->id = get->id;
        macro->special = get->special;
        macro->tag = get->tag;

        read += sizeof(savedmacro_t);
        savep += sizeof(savedmacro_t);
    }
    *start = i;
    return read;
}

typedef struct ALIGNED(8)
{
    u32 x;
    u32 y;
    u32 z;
    u32 flags;
    u16 subsector;
    u16 type;
    u16 target;
    u16 tracer;
    u32 angle;
    u32 sprite;
    u32 frame;
    u32 floorz;
    u32 ceilingz;
    u32 radius;
    u32 height;
    u32 momx;
    u32 momy;
    u32 momz;
    u32 tics;
    u32 health;
    u32 movecount;
    u32 reactiontime;
    u32 threshold;
    u32 alpha;
    u32 extradata;
    u32 tid;
    u16 state;
    u16 movedir: 3;
    u16 player: 3;
} savedmobj_t;

typedef struct ALIGNED(8)
{
    u32 x1, y1, z1;
    u32 x2, y2, z2;
    u32 slopex, slopey, slopez;
    u32 distmax, dist;
    u16 next;
} savedlaserdata_t;

u32 P_ArchiveMobjs (u8 *savep, u32 savepsize, mobj_t **start, u32 *counter)
{
    mobj_t* mo;
    savedmobj_t *put;
    u32     written = 0;
    u32 c = *counter;

    for (mo = *start ; mo != (void*) &mobjhead; mo=mo->next)
    {
        u32 size = sizeof *put;

        if (mo->flags & MF_RENDERLASER)
            size += sizeof(savedlaserdata_t);

        if ((written + size) > savepsize)
            break;

        put = (void *)savep;
        put->x = mo->x;
        put->y = mo->y;
        put->z = mo->z;
        put->flags = mo->flags;
        put->subsector = mo->subsector - subsectors;
        put->type = mo->type;
        put->target = MobjIndex(mo->target);
        put->tracer = MobjIndex(mo->tracer);
        put->angle = mo->angle;
        put->sprite = mo->sprite;
        put->frame = mo->frame;
        put->floorz = mo->floorz;
        put->ceilingz = mo->ceilingz;
        put->radius = mo->radius;
        put->height = mo->height;
        put->momx = mo->momx;
        put->momy = mo->momy;
        put->momz = mo->momz;
        put->tics = mo->tics;
        put->health = mo->health;
        put->movecount = mo->movecount;
        put->reactiontime = mo->reactiontime;
        put->threshold = mo->threshold;
        put->alpha = mo->alpha;
        put->tid = mo->tid;
        put->state = mo->state - states;
        put->movedir = mo->movedir;
        put->player = mo->player ? mo->player - players + 1 : 0;

        if (mo->flags & MF_RENDERLASER)
        {
            savedlaserdata_t *lput = (void*)(savep + sizeof *put);
            laserdata_t *laser = mo->extradata;

            lput->x1 = laser->x1;
            lput->y1 = laser->y1;
            lput->z1 = laser->z1;
            lput->x2 = laser->x2;
            lput->y2 = laser->y2;
            lput->z2 = laser->z2;
            lput->slopex = laser->slopex;
            lput->slopey = laser->slopey;
            lput->slopez = laser->slopez;
            lput->distmax = laser->distmax;
            lput->dist = laser->dist;
            lput->next = MobjIndex(laser->next ? laser->next->marker : NULL);
        }
        else
        {
            put->extradata = (u32) mo->extradata;
        }
        savep += size;
        written += size;
        c += 1;
    }

    *start = mo;
    *counter = c;

    return written;
}

u32 P_UnArchiveMobjs (const u8 *savep, u32 savepsize, u32 *counter)
{
    u32 read = 0;
    u32 c = *counter;

    while (read + sizeof(savedmobj_t) <= savepsize && c > 0)
    {
        const savedmobj_t *get = (void*) savep;

        if ((get->flags & MF_RENDERLASER)
                && read + sizeof(savedmobj_t) + sizeof(savedlaserdata_t) > savepsize)
            break;

        mobj_t *mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
        savep += sizeof *get;
        read += sizeof *get;
        mobj->x = get->x;
        mobj->y = get->y;
        mobj->z = get->z;
        mobj->flags = get->flags;
        mobj->subsector = &subsectors[get->subsector];
        mobj->type = get->type;
        mobj->info = &mobjinfo[mobj->type];
        mobj->target = (void*)(u32) get->target;
        mobj->tracer = (void*)(u32) get->tracer;
        mobj->angle = get->angle;
        mobj->sprite = get->sprite;
        mobj->frame = get->frame;
        mobj->floorz = get->floorz;
        mobj->ceilingz = get->ceilingz;
        mobj->radius = get->radius;
        mobj->height = get->height;
        mobj->momx = get->momx;
        mobj->momy = get->momy;
        mobj->momz = get->momz;
        mobj->tics = get->tics;
        mobj->health = get->health;
        mobj->movecount = get->movecount;
        mobj->reactiontime = get->reactiontime;
        mobj->threshold = get->threshold;
        mobj->alpha = get->alpha;
        mobj->tid = get->tid;
        mobj->state = &states[get->state];
        mobj->movedir = get->movedir;
        mobj->latecall = NULL;
        if (get->player)
        {
            mobj->player = &players[get->player - 1];
            mobj->player->mo = mobj;
        }
        else
        {
            mobj->player = NULL;
        }
        if (mobj->flags & MF_RENDERLASER)
        {
            const savedlaserdata_t *lget = (void*)(savep + sizeof *get);
            laserdata_t *laser = Z_Malloc(sizeof *laser, PU_LEVSPEC, 0);

            laser->x1 = lget->x1;
            laser->y1 = lget->y1;
            laser->z1 = lget->z1;
            laser->x2 = lget->x2;
            laser->y2 = lget->y2;
            laser->z2 = lget->z2;
            laser->slopex = lget->slopex;
            laser->slopey = lget->slopey;
            laser->slopez = lget->slopez;
            laser->distmax = lget->distmax;
            laser->dist = lget->dist;
            laser->next = (void*)(u32) lget->next;
            laser->marker = mobj;
            mobj->extradata = laser;
        }
        else
        {
            mobj->extradata = (void*) get->extradata;
        }
        P_SetThingPosition (mobj);

        mobjhead.prev->next = mobj;
        mobj->next = (void*) &mobjhead;
        mobj->prev = mobjhead.prev;
        mobjhead.prev = mobj;

        c -= 1;
    }

    *counter = c;

    return read;
}

void P_LinkUnArchivedMobjs(void)
{
    cameratarget = MobjByIndex((u32) cameratarget);
    macroactivator = MobjByIndex((u32) macroactivator);
    for (int i = 0; i < ARRAYLEN(macroqueue); i++)
        macroqueue[i].activator = MobjByIndex((u32)macroqueue[i].activator);

    for (int i = 0; i < MAXPLAYERS; i++)
    {
        //if (!playeringame[i])
            //continue;
        players[i].attacker = MobjByIndex((u32) players[i].attacker);
    }
    for (int i = 0; i < numsectors; i++)
    {
        sectors[i].soundtarget = MobjByIndex((u32) sectors[i].soundtarget);
    }
    for (mobj_t *mo = mobjhead.next; mo != (void*) &mobjhead; mo=mo->next)
    {
        mo->target = MobjByIndex((u32) mo->target);
        mo->tracer = MobjByIndex((u32) mo->tracer);
        if (mo->flags & MF_RENDERLASER)
        {
            laserdata_t *laser = (void*) mo->extradata;
            mobj_t *next = MobjByIndex((u32) laser->next);

            laser->next = next->extradata;
        }
    }
}

typedef enum
{
    tc_ceiling,
    tc_door,
    tc_floor,
    tc_plat,
    tc_flash,
    tc_strobe,
    tc_glow,
    tc_flicker,
    tc_delay,
    tc_aimcam,
    tc_movecam,
    tc_fade,
    tc_fadebright,
    tc_sequence,
    tc_quake,
    tc_combine,
    tc_laser,
    tc_split,
    tc_morph,
    tc_exp,
    tc_numclasses,
} thinkerclass_e;

typedef struct ALIGNED(8) {
    thinkerclass_e tclass;
    u32            activemacro: 1;
} thinkheader_t;

#define ARCTHINKERSIZE(_t) ALIGN(sizeof(_t) - sizeof(thinker_t), 8)

#define ARCHIVE_THINKER(_t, _class) \
    if ((written + sizeof(thinkheader_t) + ARCTHINKERSIZE(_t)) > savepsize) \
        break; \
 \
    header.tclass = (_class); \
    D_memcpy (savep, &header, sizeof(thinkheader_t)); \
    savep += sizeof header; \
    c += 1; \
 \
    D_memcpy (savep, ((u8*)&(_t)) + sizeof(thinker_t), sizeof(_t) - sizeof(thinker_t)); \
    savep += ARCTHINKERSIZE(_t); \
    written += sizeof(thinkheader_t) + ARCTHINKERSIZE(_t)

u32 P_ArchiveThinkers (u8 *savep, u32 savepsize, thinker_t **start, u32 *counter)
{
    thinker_t*    th;
    thinkheader_t header;
    u32           written = 0;
    int           i;
    u32           c = *counter;

    bzero(&header, sizeof header);

    for (th = *start ; th != &thinkercap; th=th->next)
    {
        if (th == macrothinker)
            header.activemacro = 1;
        else
            header.activemacro = 0;

        if (th->function == NULL)
        {
            for (i = 0; i < MAXCEILINGS;i++)
            {
                if (activeceilings[i] == (ceiling_t *)th)
                {
                    ceiling_t ceiling = *(ceiling_t *)th;
                    ARCHIVE_THINKER(ceiling, tc_ceiling);
                    ceiling.sector = (sector_t *)(ceiling.sector - sectors);
                    continue;
                }
                else if (activeplats[i] == (plat_t *)th)
                {
                    plat_t plat = *(plat_t *)th;
                    ARCHIVE_THINKER(plat, tc_plat);
                    plat.sector = (sector_t *)(plat.sector - sectors);
                    continue;
                }
            }
        }
        else if (th->function == (think_t)-1)
        {
            continue;
        }
        else if (th->function == (think_t)T_MoveCeiling)
        {
            ceiling_t ceiling = *(ceiling_t *)th;
            ceiling.sector = (sector_t *)(ceiling.sector - sectors);
            ARCHIVE_THINKER(ceiling, tc_ceiling);
            continue;
        }
        else if (th->function == (think_t)T_VerticalDoor)
        {
            vldoor_t door = *(vldoor_t *)th;
            door.sector = (sector_t *)(door.sector - sectors);
            ARCHIVE_THINKER(door, tc_door);
            continue;
        }
        else if (th->function == (think_t)T_MoveFloor)
        {
            floormove_t floor = *(floormove_t *)th;
            floor.sector = (sector_t *)(floor.sector - sectors);
            ARCHIVE_THINKER(floor, tc_floor);
            continue;
        }
        else if (th->function == (think_t)T_PlatRaise)
        {
            plat_t plat = *(plat_t *)th;
            plat.sector = (sector_t *)(plat.sector - sectors);
            ARCHIVE_THINKER(plat, tc_plat);
            continue;
        }
        if (th->function == (think_t)T_LightFlash)
        {
            lightflash_t flash = *(lightflash_t *)th;
            flash.sector = (sector_t *)(flash.sector - sectors);
            ARCHIVE_THINKER(flash, tc_flash);
            continue;
        }
        else if (th->function == (think_t)T_StrobeFlash)
        {
            strobe_t strobe = *(strobe_t *)th;
            strobe.sector = (sector_t *)(strobe.sector - sectors);
            ARCHIVE_THINKER(strobe, tc_strobe);
            continue;
        }
        else if (th->function == (think_t)T_Glow)
        {
            glow_t glow = *(glow_t *)th;
            glow.sector = (sector_t *)(glow.sector - sectors);
            ARCHIVE_THINKER(glow, tc_glow);
            continue;
        }
        else if (th->function == (think_t)T_FireFlicker)
        {
            fireflicker_t flicker = *(fireflicker_t *)th;
            flicker.sector = (sector_t *)(flicker.sector - sectors);
            ARCHIVE_THINKER(flicker, tc_flicker);
            continue;
        }
        else if (th->function == (think_t)T_CountdownTimer)
        {
            delay_t delay = *(delay_t *)th;
            delay.finishfunc = (void*)(delay.finishfunc ? 1 : 0);
            ARCHIVE_THINKER(delay, tc_delay);
            continue;
        }
        else if (th->function == (think_t)T_AimCamera)
        {
            aimcamera_t aimcam = *(aimcamera_t *)th;
            aimcam.viewmobj = (void*)(u32)MobjIndex(aimcam.viewmobj);
            ARCHIVE_THINKER(aimcam, tc_aimcam);
            continue;
        }
        else if (th->function == (think_t)T_MoveCamera)
        {
            movecamera_t movecam = *(movecamera_t *)th;
            ARCHIVE_THINKER(movecam, tc_movecam);
            continue;
        }
        else if (th->function == (think_t)T_FadeThinker)
        {
            fade_t fade = *(fade_t *)th;
            ARCHIVE_THINKER(fade, tc_fade);
            fade.mobj = (void*)(u32)MobjIndex(fade.mobj);
            continue;
        }
        else if (th->function == (think_t)T_FadeInBrightness)
        {
            fadebright_t fadebright = *(fadebright_t *)th;
            ARCHIVE_THINKER(fadebright, tc_fadebright);
            continue;
        }
        else if (th->function == (think_t)T_SequenceGlow)
        {
            sequenceglow_t sequence = *(sequenceglow_t *)th;
            sequence.sector = (sector_t *)(sequence.sector - sectors);
            sequence.headsector = (sector_t *)(sequence.headsector - sectors);
            ARCHIVE_THINKER(sequence, tc_sequence);
            continue;
        }
        else if (th->function == (think_t)T_Quake)
        {
            quake_t quake = *(quake_t *)th;
            ARCHIVE_THINKER(quake, tc_quake);
            continue;
        }
        else if (th->function == (think_t)T_Combine)
        {
            combine_t combine = *(combine_t *)th;
            combine.sector = (sector_t *)(combine.sector - sectors);
            combine.combiner = (sector_t *)(combine.combiner - sectors);
            ARCHIVE_THINKER(combine, tc_combine);
            continue;
        }
        else if (th->function == (think_t)T_LaserThinker)
        {
            laser_t laser = *(laser_t *)th;
            laser.marker = (void*)(u32)MobjIndex(laser.marker);
            laser.laserdata = (void*)(u32)MobjIndex(laser.laserdata->marker);
            ARCHIVE_THINKER(laser, tc_laser);
            continue;
        }
        else if (th->function == (think_t)T_MoveSplitPlane)
        {
            splitmove_t split = *(splitmove_t *)th;
            ARCHIVE_THINKER(split, tc_split);
            split.sector = (sector_t *)(split.sector - sectors);
            continue;
        }
        else if (th->function == (think_t)T_LightMorph)
        {
            lightmorph_t morph = *(lightmorph_t *)th;
            ARCHIVE_THINKER(morph, tc_morph);
            morph.sector = (sector_t *)(morph.sector - sectors);
            continue;
        }
        else if (th->function == (think_t)T_MobjExplode)
        {
            mobjexp_t exp = *(mobjexp_t *)th;
            ARCHIVE_THINKER(exp, tc_exp);
            exp.mobj = (void*)(u32)MobjIndex(exp.mobj);
            continue;
        }
        I_Error ("P_ArchiveThinkers: Unknown thinker function");
    }

    *start = th;
    *counter = c;

    return written;
}

#define UNARCHIVE_THINKER(_t, _tag, _func) \
    if (read + sizeof header + ARCTHINKERSIZE(*(_t)) > savepsize) \
        return read; \
    (_t) = Z_Malloc (sizeof(*(_t)), (_tag), NULL); \
    D_memcpy (((u8*)(_t)) + sizeof(thinker_t), (void*)savep, sizeof(*(_t)) - sizeof(thinker_t)); \
    savep += ARCTHINKERSIZE(*(_t)); \
    read += sizeof header + ARCTHINKERSIZE(*(_t)); \
    (_t)->thinker.function = (think_t) (_func); \
    if (header.activemacro) \
        macrothinker = &(_t)->thinker; \
    P_AddThinker (&(_t)->thinker)

u32 P_UnArchiveThinkers (const u8 *savep, u32 savepsize, u32 *counter)
{
    thinkheader_t header;
    u32           read = 0;
    u32           c = *counter;

    // read in saved thinkers
    while (read + sizeof header <= savepsize && c > 0)
    {
        D_memcpy(&header, (void*)savep, sizeof header);
        savep += sizeof header;
        switch (header.tclass)
        {
        case tc_ceiling:
            {
                ceiling_t *ceiling;

                UNARCHIVE_THINKER(ceiling, PU_LEVEL, NULL);
                ceiling->sector = &sectors[(u32) ceiling->sector];
                ceiling->sector->specialdata = ceiling;

                if (ceiling->thinker.function)
                    ceiling->thinker.function = (think_t)T_MoveCeiling;

                P_AddActiveCeiling(ceiling);
            }
            break;
        case tc_door:
            {
                vldoor_t *door;

                UNARCHIVE_THINKER(door, PU_LEVSPEC, T_VerticalDoor);
                door->sector = &sectors[(u32) door->sector];
                door->sector->specialdata = door;
            }
            break;

        case tc_floor:
            {
                floormove_t *floor;

                UNARCHIVE_THINKER(floor, PU_LEVSPEC, T_MoveFloor);
                floor->sector = &sectors[(u32) floor->sector];
                floor->sector->specialdata = floor;
            }
            break;

        case tc_plat:
            {
                plat_t *plat;

                UNARCHIVE_THINKER(plat, PU_LEVSPEC, NULL);
                plat->sector = &sectors[(u32) plat->sector];
                plat->sector->specialdata = plat;

                if (plat->thinker.function)
                    plat->thinker.function = (think_t)T_PlatRaise;

                P_AddActivePlat(plat);
            }
            break;

        case tc_flash:
            {
                lightflash_t *flash;

                UNARCHIVE_THINKER(flash, PU_LEVSPEC, T_LightFlash);
                flash->sector = &sectors[(u32) flash->sector];
            }
            break;

        case tc_strobe:
            {
                strobe_t *strobe;

                UNARCHIVE_THINKER(strobe, PU_LEVSPEC, T_StrobeFlash);
                strobe->sector = &sectors[(u32) strobe->sector];
            }
            break;

        case tc_glow:
            {
                glow_t *glow;

                UNARCHIVE_THINKER(glow, PU_LEVSPEC, T_Glow);
                glow->sector = &sectors[(u32) glow->sector];
            }
            break;
        case tc_flicker:
            {
                fireflicker_t *flicker;

                UNARCHIVE_THINKER(flicker, PU_LEVSPEC, T_FireFlicker);
                flicker->sector = &sectors[(u32) flicker->sector];
            }
            break;

        case tc_delay:
            {
                delay_t *delay;

                UNARCHIVE_THINKER(delay, PU_LEVSPEC, T_CountdownTimer);
                delay->finishfunc = delay->finishfunc ? G_CompleteLevel : NULL;
            }
            break;

        case tc_aimcam:
            {
                aimcamera_t *aimcam;

                UNARCHIVE_THINKER(aimcam, PU_LEVSPEC, T_AimCamera);
                aimcam->viewmobj = MobjByIndex((u32) aimcam->viewmobj);
            }
            break;

        case tc_movecam:
            {
                movecamera_t *movecam;

                UNARCHIVE_THINKER(movecam, PU_LEVSPEC, T_MoveCamera);
            }
            break;

        case tc_fade:
            {
                fade_t *fade;

                UNARCHIVE_THINKER(fade, PU_LEVSPEC, T_FadeThinker);
                fade->mobj = MobjByIndex((u32) fade->mobj);
            }
            break;

        case tc_fadebright:
            {
                fadebright_t *fadebright;

                UNARCHIVE_THINKER(fadebright, PU_LEVSPEC, T_FadeInBrightness);
            }
            break;

        case tc_sequence:
            {
                sequenceglow_t *sequence;

                UNARCHIVE_THINKER(sequence, PU_LEVSPEC, T_SequenceGlow);
                sequence->sector = &sectors[(u32) sequence->sector];
                sequence->headsector = &sectors[(u32) sequence->headsector];
            }
            break;

        case tc_quake:
            {
                quake_t *quake;

                UNARCHIVE_THINKER(quake, PU_LEVSPEC, T_Quake);
            }
            break;

        case tc_combine:
            {
                combine_t *combine;

                UNARCHIVE_THINKER(combine, PU_LEVSPEC, T_Combine);
                combine->sector = &sectors[(u32) combine->sector];
                combine->combiner = &sectors[(u32) combine->combiner];
            }
            break;

        case tc_laser:
            {
                laser_t *laser;

                UNARCHIVE_THINKER(laser, PU_LEVSPEC, T_LaserThinker);
                laser->marker = MobjByIndex((u32) laser->marker);
                laser->laserdata = MobjByIndex((u32) laser->laserdata)->extradata;
            }
            break;

        case tc_split:
            {
                splitmove_t *split;

                UNARCHIVE_THINKER(split, PU_LEVSPEC, T_MoveSplitPlane);
                split->sector = &sectors[(u32) split->sector];
            }
            break;

        case tc_morph:
            {
                lightmorph_t *morph;

                UNARCHIVE_THINKER(morph, PU_LEVSPEC, T_LightMorph);
                morph->sector = &sectors[(u32) morph->sector];
            }
            break;

        case tc_exp:
            {
                mobjexp_t *exp;

                UNARCHIVE_THINKER(exp, PU_LEVSPEC, T_MobjExplode);
                exp->mobj = MobjByIndex((u32) exp->mobj);
            }
            break;

        default:
            I_Error ("Unknown tclass %i at %lu (%lu)", header.tclass, c, *counter);
        }

        c -= 1;
    }

    *counter = c;

    return read;
}

u32 P_CurrentQuickSaveSize(u32 max)
{
    u32 size = sizeof(savedmacrosheader_t);

    for (int i = 0; i < MAXPLAYERS; i++)
        //if (playeringame[i])
        size += ALIGN(sizeof(player_t), 8);

    size += numsectors * sizeof(savedsector_t);

    for (int i = 0; i < numlines; i++)
    {
        size += sizeof(savedline_t) * 3;
        if (lines[i].sidenum[0] != -1)
            size += sizeof(savedside_t);
        if (lines[i].sidenum[1] != -1)
            size += sizeof(savedside_t);

        if (size > max)
            return size;
    }

    for (mobj_t *mo = mobjhead.next ; mo != (void*) &mobjhead; mo=mo->next)
    {
        size += sizeof(savedmobj_t);
        if (mo->flags & MF_RENDERLASER)
            size += sizeof(savedlaserdata_t);
    }

    for (int i = 0; i < nummacros; i++)
        size += sizeof(savedmacro_t);

    for (thinker_t *th = thinkercap.next ; th != &thinkercap; th = th->next)
    {
        if (th->function == NULL)
        {
            for (int i = 0; i < MAXCEILINGS;i++)
            {
                if (activeceilings[i] == (ceiling_t *)th)
                    size += 8 + ARCTHINKERSIZE(ceiling_t);
                else if (activeplats[i] == (plat_t *)th)
                    size += 8 + ARCTHINKERSIZE(plat_t);
            }
        }
        else if (th->function == (think_t)T_MoveCeiling)
            size += 8 + ARCTHINKERSIZE(ceiling_t);
        else if (th->function == (think_t)T_VerticalDoor)
            size += 8 + ARCTHINKERSIZE(vldoor_t);
        else if (th->function == (think_t)T_MoveFloor)
            size += 8 + ARCTHINKERSIZE(floormove_t);
        else if (th->function == (think_t)T_PlatRaise)
            size += 8 + ARCTHINKERSIZE(plat_t);
        else if (th->function == (think_t)T_LightFlash)
            size += 8 + ARCTHINKERSIZE(lightflash_t);
        else if (th->function == (think_t)T_StrobeFlash)
            size += 8 + ARCTHINKERSIZE(strobe_t);
        else if (th->function == (think_t)T_Glow)
            size += 8 + ARCTHINKERSIZE(glow_t);
        else if (th->function == (think_t)T_FireFlicker)
            size += 8 + ARCTHINKERSIZE(fireflicker_t);
        else if (th->function == (think_t)T_CountdownTimer)
            size += 8 + ARCTHINKERSIZE(delay_t);
        else if (th->function == (think_t)T_AimCamera)
            size += 8 + ARCTHINKERSIZE(aimcamera_t);
        else if (th->function == (think_t)T_MoveCamera)
            size += 8 + ARCTHINKERSIZE(movecamera_t);
        else if (th->function == (think_t)T_FadeThinker)
            size += 8 + ARCTHINKERSIZE(fade_t);
        else if (th->function == (think_t)T_FadeInBrightness)
            size += 8 + ARCTHINKERSIZE(fadebright_t);
        else if (th->function == (think_t)T_SequenceGlow)
            size += 8 + ARCTHINKERSIZE(sequenceglow_t);
        else if (th->function == (think_t)T_Quake)
            size += 8 + ARCTHINKERSIZE(quake_t);
        else if (th->function == (think_t)T_Combine)
            size += 8 + ARCTHINKERSIZE(combine_t);
        else if (th->function == (think_t)T_LaserThinker)
            size += 8 + ARCTHINKERSIZE(laser_t);
        else if (th->function == (think_t)T_MoveSplitPlane)
            size += 8 + ARCTHINKERSIZE(splitmove_t);
        else if (th->function == (think_t)T_LightMorph)
            size += 8 + ARCTHINKERSIZE(lightmorph_t);
        else if (th->function == (think_t)T_MobjExplode)
            size += 8 + ARCTHINKERSIZE(mobjexp_t);

        if (size > max)
            return size;
    }

    return size;
}

