/* am_main.c -- automap */

#include "doomdef.h"
#include "p_local.h"
#include "r_local.h"
#include "st_main.h"

#define COLOR_WHITE   0xF0F0F0FF
#define COLOR_RED     0xA40000FF
#define COLOR_GREEN   0x00A400FF
#define COLOR_BLUE    0x0000A4FF
#define COLOR_BROWN   0x603818FF
#define COLOR_YELLOW  0xCCCC00FF
#define COLOR_GREY    0x181818FF
#define COLOR_AQUA    0x3373B3FF

#define MAXSCALE    1500
#define MINSCALE    200

fixed_t am_box[4]; // 80063110
int am_plycolor;    // 80063120
int am_plyblink;    // 80063124

void AM_DrawSubsectors(player_t *player, fixed_t cx, fixed_t cy, fixed_t bbox[static 4]);
void AM_DrawThing(mobj_t *thing, angle_t angle, fixed_t sin, fixed_t cos, fixed_t cx, fixed_t cy, fixed_t scale);
void AM_DrawThingTriangle(fixed_t x, fixed_t y, angle_t angle, int color);
void AM_DrawLines(player_t *player, fixed_t bbox[static 4]);

/*================================================================= */
/* */
/* Start up Automap */
/* */
/*================================================================= */

void AM_Start(void) // 800004D8
{
    am_plycolor = 95;
    am_plyblink = 16;
}

/*
==================
=
= AM_Control
=
= Called by P_PlayerThink before any other player processing
=
= Button bits can be eaten by clearing them in ticbuttons[playernum]
==================
*/

#define MAXSENSIVITY    7

void AM_Control (player_t *player) // 800004F4
{
    int buttons, pbuttons, oldpbuttons;

    fixed_t     block[8];
    angle_t     angle;
    fixed_t     fs, fc;
    fixed_t     x, y, x1, y1, x2, y2;
    int         scale, sensitivity;
    int         i;
    int         playernum;

    if (gamepaused)
        return;

    playernum = players - player;
    buttons = ticbuttons[playernum];
    pbuttons = playerbuttons[playernum];
    oldpbuttons = oldplayerbuttons[playernum];

    if (player->playerstate != PST_LIVE)
        am_plycolor = 79;

    /* allow zoom to take precedence if it is bound to the map button */
    if ((!(player->controls->buttons[BT_MAP] & (PAD_L_TRIG|PAD_R_TRIG)) || !(pbuttons & BB_USE))
            && (pbuttons & BB_MAP) && !(oldpbuttons & BB_MAP))
    {
        if(player->automapflags & AF_SUBSEC)
        {
            player->automapflags &= ~AF_SUBSEC;
            player->automapflags |= AF_LINES;
        }
        else if(player->automapflags & AF_LINES)
        {
            player->automapflags &= ~AF_LINES;
        }
        else
        {
            player->automapflags |= AF_SUBSEC;
        }

        player->automapx = player->mo->x;
        player->automapy = player->mo->y;
    }

    if(!(player->automapflags & (AF_LINES|AF_SUBSEC)))
        return;

    /* update player flash */
    am_plycolor = (unsigned int)(am_plycolor + am_plyblink);
    if(am_plycolor < 80 || (am_plycolor >= 255))
    {
        am_plyblink = -am_plyblink;
    }

    if (!(pbuttons & BB_USE))
    {
        player->automapflags &= ~AF_FOLLOW;
        return;
    }

    if (!(player->automapflags & AF_FOLLOW))
    {
        player->automapflags |= AF_FOLLOW;
        player->automapx = player->mo->x;
        player->automapy = player->mo->y;

        M_ClearBox(am_box);

        block[2] = block[4] = (bmapwidth << 23 ) + bmaporgx;
        block[1] = block[3] = (bmapheight << 23) + bmaporgy;
        block[0] = block[6] = bmaporgx;
        block[5] = block[7] = bmaporgy;

        angle = (ANG90 - player->mo->angle) >> ANGLETOFINESHIFT;

        fs = finesine(angle);
        fc = finecosine(angle);

        for(i = 0; i < 8; i+=2)
        {
            x = (block[i]   - player->automapx) >> FRACBITS;
            y = (block[i+1] - player->automapy) >> FRACBITS;

            x1 = (x * fc);
            y1 = (y * fs);
            x2 = (x * fs);
            y2 = (y * fc);

            x = (x1 - y1) + player->automapx;
            y = (x2 + y2) + player->automapy;

            M_AddToBox(am_box, x, y);
        }
    }

    if (!(player->automapflags & AF_FOLLOW))
        return;

    scale = player->automapscale << 15;
    scale = (scale / 1500) << 8;

    /* Analyze analog stick movement (left / right) */
    sensitivity = STICK_X(buttons);

    if(sensitivity >= MAXSENSIVITY || sensitivity <= -MAXSENSIVITY)
        player->automapx += (sensitivity * scale) / 80;

    /* Analyze analog stick movement (up / down) */
    sensitivity = STICK_Y(buttons);

    if(sensitivity >= MAXSENSIVITY || sensitivity <= -MAXSENSIVITY)
        player->automapy += (sensitivity * scale) / 80;

    /* X movement */
    if (player->automapx > am_box[BOXRIGHT])
        player->automapx = am_box[BOXRIGHT];
    else if (player->automapx < am_box[BOXLEFT])
        player->automapx = am_box[BOXLEFT];

    /* Y movement */
    if (player->automapy > am_box[BOXTOP])
        player->automapy = am_box[BOXTOP];
    else if (player->automapy < am_box[BOXBOTTOM])
        player->automapy = am_box[BOXBOTTOM];

    /* Zoom scale in */
    if (buttons & PAD_L_TRIG)
    {
        player->automapscale -= 32;

        if (player->automapscale < MINSCALE)
            player->automapscale = MINSCALE;
    }

    /* Zoom scale out */
    if (buttons & PAD_R_TRIG)
    {
        player->automapscale += 32;

        if (player->automapscale > MAXSCALE)
            player->automapscale = MAXSCALE;
    }

    /* capture stick movement and trigger button presses */
    ticbuttons[playernum] &= ~0xffff;
    for (i = 0; i < NUMBUTTONS; i++)
        if (player->controls->buttons[i] & (PAD_L_TRIG|PAD_R_TRIG))
            playerbuttons[0] &= ~(1<<i);
}

extern Mtx R_ProjectionMatrix;
/*
==================
=
= AM_Drawer
=
= Draws the current frame to workingscreen
==================
*/

void AM_Drawer (void) // 800009AC
{
    player_t    *p;
    mobj_t      *mo;
    mobj_t      *next;
    fixed_t     xpos, ypos;
    fixed_t     c, s, ts, tc;
    angle_t     angle;
    int         color;
    int         scale;
    int         artflag;
    char        buf[48];
    fixed_t     hcot, vcot;
    fixed_t     screen_box[4];
    fixed_t     boxscale;
    bool        linemode;
    fixed_t     thingscale = 0;

    hcot = aspectscale[VideoSettings.ScreenAspect];
    vcot = aspectratios[0];
    R_ProjectionMatrix.m[0][0] = hcot & 0xffff0000;
    R_ProjectionMatrix.m[2][0] = (hcot << 16) & 0xffff0000;
    R_ProjectionMatrix.m[0][2] = (vcot >> 16) & 0xffff;
    R_ProjectionMatrix.m[2][2] = vcot & 0xffff;

    I_ClearFB(0x000000ff);

    R_RenderFilter(filt_textures);
    gSPTexture(GFX1++, (1024 << 6)-1, (1024 << 6)-1, 0, G_TX_RENDERTILE, G_ON);

    p = &players[0];

    scale = (p->automapscale << FRACBITS);
    xpos = p->mo->x;
    ypos = p->mo->y;

    if (p->onground)
    {
        xpos += (quakeviewx >> 7);
        ypos += quakeviewy;
    }

    if (p->automapflags & AF_FOLLOW)
    {
        s64 ox, oy;

        angle = (p->mo->angle + ANG270) >> ANGLETOFINESHIFT;
        ox = p->automapx - xpos;
        oy = p->automapy - ypos;
        s = finesine(angle);
        c = finecosine(angle);
        xpos += (ox * (s64)c - oy * (s64)s) >> FRACBITS;
        ypos += (ox * (s64)s + oy * (s64)c) >> FRACBITS;
    }

    angle = p->mo->angle >> ANGLETOFINESHIFT;

    s = finesine(angle);
    c = finecosine(angle);

    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(MTX1), G_MTX_MODELVIEW| G_MTX_LOAD | G_MTX_NOPUSH);
    MTX1->m[0][0] = 0x10000;
    MTX1->m[0][1] = 0;
    MTX1->m[0][2] = 0;
    MTX1->m[0][3] = 0x10000;
    MTX1->m[1][0] = 0xffff;
    MTX1->m[1][1] = 0;
    MTX1->m[1][2] = 0;
    MTX1->m[1][3] = 1;
    MTX1->m[2][0] = 0;
    MTX1->m[2][1] = 0;
    MTX1->m[2][2] = 0;
    MTX1->m[2][3] = 0;
    MTX1->m[3][0] = 0;
    MTX1->m[3][1] = 0;
    MTX1->m[3][2] = 0;
    MTX1->m[3][3] = 0;
    MTX1+=1;

    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(MTX1), G_MTX_MODELVIEW| G_MTX_MUL | G_MTX_NOPUSH);
    MTX1->m[0][0] = (s & 0xffff0000);
    MTX1->m[0][1] = ((-c) & 0xffff0000);
    MTX1->m[0][2] = 1;
    MTX1->m[0][3] = 0;
    MTX1->m[1][0] = (c & 0xffff0000);
    MTX1->m[1][1] = (s & 0xffff0000);
    MTX1->m[1][2] = 0;
    MTX1->m[1][3] = 1;
    MTX1->m[2][0] = ((s << 16) & 0xffff0000);
    MTX1->m[2][1] = (((-c)<<16) & 0xffff0000);
    MTX1->m[2][2] = 0;
    MTX1->m[2][3] = 0;
    MTX1->m[3][0] = ((c << 16) & 0xffff0000);
    MTX1->m[3][1] = ((s << 16) & 0xffff0000);
    MTX1->m[3][2] = 0;
    MTX1->m[3][3] = 0;
    MTX1+=1;

    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(MTX1), G_MTX_MODELVIEW| G_MTX_MUL | G_MTX_NOPUSH);
    MTX1->m[0][0] = 0x10000;
    MTX1->m[0][1] = 0;
    MTX1->m[0][2] = 1;
    MTX1->m[0][3] = 0;
    MTX1->m[1][0] = 0;
    MTX1->m[1][1] = 0x10000;
    MTX1->m[1][2] = ((-xpos) & 0xffff0000) | (((-scale) >> 16) &0xffff);
    MTX1->m[1][3] = (ypos & 0xffff0000) | 1;
    MTX1->m[2][0] = 0;
    MTX1->m[2][1] = 0;
    MTX1->m[2][2] = 0;
    MTX1->m[2][3] = 0;
    MTX1->m[3][0] = 0;
    MTX1->m[3][1] = 0;
    MTX1->m[3][2] = (((-xpos) << 16) & 0xffff0000) | ((-scale) &0xffff);
    MTX1->m[3][3] = ((ypos << 16) & 0xffff0000);
    MTX1+=1;

    boxscale = scale / 160;

    {
        fixed_t cx, cy, tx, x, y;
        angle_t thingangle;

        thingangle = (ANG90 - p->mo->angle) >> ANGLETOFINESHIFT;
        ts = finesine(thingangle);
        tc = finecosine(thingangle);

        cx = FixedMul(SCREEN_WD<<(FRACBITS-1), boxscale);
        cy = FixedMul(SCREEN_HT<<(FRACBITS-1), boxscale);

        if (VideoSettings.ScreenAspect)
            cx = FixedMul(cx, invaspectscale[VideoSettings.ScreenAspect]);

        M_ClearBox(screen_box);

        for (int i = 0; i < 2; i++)
        {
            tx = i ? -cx : cx;
            x = ((s64) tx * (s64) tc + (s64) cy * (s64) ts) >> FRACBITS;
            y = ((s64) -tx * (s64) ts + (s64) cy * (s64) tc) >> FRACBITS;
            M_AddToBox(screen_box, x, y);
            M_AddToBox(screen_box, -x, -y);
        }

        screen_box[BOXTOP] += ypos;
        screen_box[BOXBOTTOM] += ypos;
        screen_box[BOXLEFT] += xpos;
        screen_box[BOXRIGHT] += xpos;
    }

    linemode = !!(p->automapflags & AF_LINES);
    if (linemode)
    {
        AM_DrawLines(p, screen_box);
    }
    else
    {
        AM_DrawSubsectors(p, xpos, ypos, screen_box);
        gDPPipeSync(GFX1++);
        gDPSetCombineMode(GFX1++, G_CC_SHADE, G_CC_SHADE);
    }

    /* SHOW ALL MAP THINGS (CHEAT) */
    if (!linemode)
        thingscale = FixedDiv2(FRACUNIT, boxscale);

    R_RenderFilter(filt_sprites);

    for (mo = mobjhead.next; mo != (void*) &mobjhead; mo = next)
    {
        next = mo->next;

        /* [nova] - always draw keys on automap */
        if (!(p->cheats & CF_ALLMAP)
                && mo->type != MT_ITEM_BLUECARDKEY
                && mo->type != MT_ITEM_REDCARDKEY
                && mo->type != MT_ITEM_YELLOWCARDKEY
                && mo->type != MT_ITEM_YELLOWSKULLKEY
                && mo->type != MT_ITEM_REDSKULLKEY
                && mo->type != MT_ITEM_BLUESKULLKEY)
            continue;

        if (mo == p->mo)
            continue;  /* Ignore player */

        if (mo->flags & (MF_NOSECTOR|MF_RENDERLASER))
            continue;

        if ((players->artifacts & 1) != 0 && mo->type == MT_ITEM_ARTIFACT1) continue;
        if ((players->artifacts & 2) != 0 && mo->type == MT_ITEM_ARTIFACT2) continue;
        if ((players->artifacts & 4) != 0 && mo->type == MT_ITEM_ARTIFACT3) continue;

        if (mo->type == MT_ITEM_BLUECARDKEY || mo->type == MT_ITEM_BLUESKULLKEY)
            color = COLOR_BLUE;
        else if (mo->type == MT_ITEM_YELLOWCARDKEY || mo->type == MT_ITEM_YELLOWSKULLKEY)
            color = COLOR_YELLOW;
        else if (mo->type == MT_ITEM_REDCARDKEY || mo->type == MT_ITEM_REDSKULLKEY)
            color = COLOR_RED;
        else if (mo->flags & (MF_SHOOTABLE|MF_MISSILE))
            color = COLOR_WHITE;
        else
            color = COLOR_AQUA;

        if (linemode)
        {
            fixed_t bbox[4];

            bbox[BOXTOP   ] = mo->y + 0x2d413c; // sqrt(2) * 32
            bbox[BOXBOTTOM] = mo->y - 0x2d413c;
            bbox[BOXRIGHT ] = mo->x + 0x2d413c;
            bbox[BOXLEFT  ] = mo->x - 0x2d413c;

            if (!M_BoxIntersect(bbox, screen_box))
                continue;

            if (I_GFXFull())
                break;

            AM_DrawThingTriangle(mo->x, mo->y, mo->angle, color);

            gSPLine3D(GFX1++, 0, 1, 0 /*flag*/);
            gSPLine3D(GFX1++, 1, 2, 0 /*flag*/);
            gSPLine3D(GFX1++, 2, 0, 0 /*flag*/);
        }
        else
        {
            if (I_GFXFull())
                break;

            AM_DrawThing(mo, p->mo->angle, ts, tc, xpos, ypos, thingscale);
        }
    }

    /* SHOW PLAYERS */
    if (linemode)
    {
        AM_DrawThingTriangle(p->mo->x, p->mo->y, p->mo->angle, am_plycolor << 16 | 0xff);

        gSPLine3D(GFX1++, 0, 1, 0 /*flag*/);
        gSPLine3D(GFX1++, 1, 2, 0 /*flag*/);
        gSPLine3D(GFX1++, 2, 0, 0 /*flag*/);

        gDPPipeSync(GFX1++);
        gDPSetScissor(GFX1++, G_SC_NON_INTERLACE, 0, 0, XResolution, YResolution);
    }
    else
    {
        AM_DrawThing(p->mo, p->mo->angle + ANG180, ts, tc, xpos, ypos, thingscale);
    }


    if (Settings.EnableMessages)
    {
        if (p->messagetics[0] <= 0)
        {
            sprintf(buf, "LEVEL %d: %s", gamemap, MapInfo[gamemap].name);
            ST_Message(2+Settings.HudMargin, Settings.HudMargin, buf, 196 | 0xffffff00);
        }
        else
        {
            ST_DrawMessages(p);
        }
    }

    // [Immorpher] kill count
    if(Settings.MapStats) {
        sprintf(buf, "KILLS: %d/%d", players[0].killcount, totalkills);
        ST_Message(2+Settings.HudMargin, 212-Settings.HudMargin, buf, 196 | 0xffffff00);
        sprintf(buf, "ITEMS: %d/%d", players[0].itemcount, totalitems);
        ST_Message(2+Settings.HudMargin, 222-Settings.HudMargin, buf, 196| 0xffffff00);
        sprintf(buf, "SECRETS: %d/%d", players[0].secretcount, totalsecret);
        ST_Message(2+Settings.HudMargin, 232-Settings.HudMargin, buf, 196 | 0xffffff00);
    }

    xpos = 297-Settings.HudMargin;
    artflag = 4;
    do
    {
        if ((players->artifacts & artflag) != 0)
        {
            if (artflag == 4)
                F_DrawSprite(MT_ITEM_ARTIFACT3, &states[S_ART3], 0, 0xffffff80, xpos, 266-Settings.HudMargin, FRACUNIT, -1);
            else if (artflag == 2)
                F_DrawSprite(MT_ITEM_ARTIFACT2, &states[S_ART2], 0, 0xffffff80, xpos, 266-Settings.HudMargin, FRACUNIT, -1);
            else if (artflag == 1)
                F_DrawSprite(MT_ITEM_ARTIFACT1, &states[S_ART1], 0, 0xffffff80, xpos, 266-Settings.HudMargin, FRACUNIT, -1);

            xpos -= 40;
        }
        artflag >>= 1;
    } while (artflag != 0);

    ST_DrawDebug();
}


static bool AM_DrawSubsector(player_t *player, int bspnum)
{
    subsector_t *sub;
    sector_t *sec;

    if(!(bspnum & NF_SUBSECTOR))
        return false;

    sub = &subsectors[bspnum & (~NF_SUBSECTOR)];

    if(!sub->drawindex && !player->powers[pw_allmap] && !(player->cheats & CF_ALLMAP))
        return true;

    sec = sub->sector;

    if((sec->flags & MS_HIDESSECTOR) || (sec->floorpic == -1))
        return true;

    if (I_GFXFull())
        return true;

    DEBUG_COUNTER(LastVisSubsectors += 1);

    R_RenderPlane(&leafs[sub->leaf], sub->numverts, 0,
                  textures[sec->floorpic], 0, 0,
                  lights[sec->colors[1]].rgba);

    return true;
}

/*
==================
=
= AM_DrawSubsectors
=
==================
*/

#define MAX_BSP_DEPTH 128

void AM_DrawSubsectors(player_t *player, fixed_t cx, fixed_t cy, fixed_t bbox[static 4]) // 800012A0
{
    int sp = 0;
    node_t *bsp;
    int     side;
    fixed_t    dx, dy;
    fixed_t    left, right;
    int bspnum = numnodes - 1;
    int bspstack[MAX_BSP_DEPTH];

    gDPPipeSync(GFX1++);
    gDPSetCycleType(GFX1++, G_CYC_1CYCLE);
    gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
    gDPSetTexturePersp(GFX1++, G_TP_PERSP);
    gDPSetRenderMode(GFX1++, G_RM_OPA_SURF,G_RM_OPA_SURF2);
    gDPSetCombineMode(GFX1++, G_CC_D64COMB01, G_CC_D64COMB01);

    globallump = -1;

    while(true)
    {
        while (!AM_DrawSubsector(player, bspnum))
        {
            if(sp == MAX_BSP_DEPTH)
                break;

            bsp = &nodes[bspnum];
            dx = (cx - bsp->line.x);
            dy = (cy - bsp->line.y);

            left = (bsp->line.dy >> FRACBITS) * (dx >> FRACBITS);
            right = (dy >> FRACBITS) * (bsp->line.dx >> FRACBITS);

            if (right < left)
                side = 0;        /* front side */
            else
                side = 1;        /* back side */

            bspstack[sp++] = bspnum;
            bspstack[sp++] = side;

            bspnum = bsp->children[side];

        }
        if(sp == 0)
        {
            //back at root node and not visible. All done!
            return;
        }

        //Back sides.
        side = bspstack[--sp];
        bspnum = bspstack[--sp];
        bsp = &nodes[bspnum];

        // Possibly divide back space.
        //Walk back up the tree until we find
        //a node that has a visible backspace.
        while(!M_BoxIntersect (bbox, bsp->bbox[side^1]))
        {
            if(sp == 0)
            {
                //back at root node and not visible. All done!
                return;
            }

            //Back side next.
            side = bspstack[--sp];
            bspnum = bspstack[--sp];

            bsp = &nodes[bspnum];
        }

        bspnum = bsp->children[side^1];
    }
}

/*
==================
=
= AM_DrawLine
=
==================
*/

void AM_DrawLines(player_t *player, fixed_t bbox[static 4]) // 800014C8
{
    line_t *l;
    int i, color;

    vid_task->t.ucode = (u64 *) gspL3DEX2_fifoTextStart;
    vid_task->t.ucode_data = (u64 *) gspL3DEX2_fifoDataStart;

    gDPPipeSync(GFX1++);
    gDPSetCycleType(GFX1++, G_CYC_1CYCLE);

    gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
    gDPSetTexturePersp(GFX1++, G_TP_PERSP);

    gDPSetRenderMode(GFX1++,G_RM_AA_XLU_LINE,G_RM_AA_XLU_LINE2);
    gDPSetCombineMode(GFX1++, G_CC_D64COMB02, G_CC_D64COMB02);

    l = lines;
    for (i = 0; i < numlines; i++, l++)
    {
        if(l->flags & ML_DONTDRAW)
            continue;

        if (!M_BoxIntersect(bbox, l->bbox))
            continue;

        if((l->flags & ML_MAPPED) || player->powers[pw_allmap] || (player->cheats & CF_ALLMAP))
        {
            if (I_GFXFull())
                break;

            /* */
            /* Figure out color */
            /* */
            color = COLOR_BROWN;

            if(player->powers[pw_allmap] && !(player->cheats & CF_ALLMAP) && !(l->flags & ML_MAPPED))
                color = COLOR_GREY;
            else if (l->flags & ML_SECRET)
                color = COLOR_WHITE;
            else if(l->special && !(l->flags & ML_HIDEAUTOMAPTRIGGER))
            {
                if(l->special & MLU_BLUE)
                    color = COLOR_BLUE;
                else if(l->special & MLU_YELLOW)
                    color = COLOR_YELLOW;
                else if(l->special & MLU_RED)
                    color = COLOR_RED;
                else
                    color = COLOR_GREEN;
            }
            else if (!(l->flags & ML_TWOSIDED)) /* ONE-SIDED LINE */
                color = COLOR_WHITE;

            gSPVertex(GFX1++, (VTX1), 2, 0);
            gSPLine3D(GFX1++, 0, 1, 0);

            /* x, z */
            VTX1[0].v.ob[0] =  l->v1->x >> FRACBITS;
            VTX1[0].v.ob[2] = -l->v1->y >> FRACBITS;

            /* x, z */
            VTX1[1].v.ob[0] =  l->v2->x >> FRACBITS;
            VTX1[1].v.ob[2] = -l->v2->y >> FRACBITS;

            /* y */
            VTX1[0].v.ob[1] = VTX1[1].v.ob[1] = 0;

            /* rgba */
            *(int *)VTX1[1].v.cn = color;
            *(int *)VTX1[0].v.cn = color;

            VTX1 += 2;
        }
    }
}

/*
==================
=
= AM_DrawThingTriangle
=
==================
*/

void AM_DrawThingTriangle(fixed_t x, fixed_t y, angle_t angle, int color) // 80001834
{
    angle_t ang;

    gSPVertex(GFX1++, (VTX1), 3, 0);

    ang = (angle) >> ANGLETOFINESHIFT;
    VTX1[0].v.ob[0] = ((finecosine(ang) << 5) + x) >> FRACBITS;
    VTX1[0].v.ob[2] =-((finesine(ang) << 5) + y) >> FRACBITS;

    ang = (angle + 0xA0000000) >> ANGLETOFINESHIFT;
    VTX1[1].v.ob[0] = ((finecosine(ang) << 5) + x) >> FRACBITS;
    VTX1[1].v.ob[2] =-((finesine(ang) << 5) + y) >> FRACBITS;

    ang = (angle + 0x60000000) >> ANGLETOFINESHIFT;
    VTX1[2].v.ob[0] = ((finecosine(ang) << 5) + x) >> FRACBITS;
    VTX1[2].v.ob[2] =-((finesine(ang) << 5) + y) >> FRACBITS;

    VTX1[0].v.ob[1] = VTX1[1].v.ob[1] = VTX1[2].v.ob[1] = 0;

    *(int *)VTX1[0].v.cn = *(int *)VTX1[1].v.cn = *(int *)VTX1[2].v.cn = color;

    VTX1 += 3;
}

void AM_DrawThing(mobj_t *thing, angle_t angle, fixed_t sin, fixed_t cos, fixed_t cx, fixed_t cy, fixed_t scale)
{
    int rot;
    int color;
    fixed_t x, y, tx;
    sector_t *sector;
    int translation;

    rot = ((angle - thing->angle) + ((unsigned int)(ANG45 / 2) * 9)) >> 29;

    // translate and rotate to camera space
    x = thing->x - cx;
    y = thing->y - cy;

    tx = x;
    x = ((s64) tx * (s64) cos - (s64) y * (s64) sin) >> FRACBITS;
    y = ((s64) tx * (s64) sin + (s64) y * (s64) cos) >> FRACBITS;

    // scale to screen space
    if (VideoSettings.ScreenAspect)
        x = FixedMul(x, aspectscale[VideoSettings.ScreenAspect]);

    x = FixedMul(scale, x);
    y = -FixedMul(scale, y);

    // translate by screen bounds
    x += (SCREEN_WD<<(FRACBITS-1));
    y += (SCREEN_HT<<(FRACBITS-1));

    sector = thing->subsector->sector;
    if (thing->flags & MF_NIGHTMARE)
    {
        color = PACKRGBA(64, 255, 0, 0);
    }
    else
    {
        if (thing->frame & FF_FULLBRIGHT)
            color = PACKRGBA(255, 255, 255, 0);
        else
            color = lights[sector->colors[2]].rgba & 0xffffff00;
    }

    if (thing->player)
        translation = thing->player - players;
    else if ((thing->type == MT_BLOOD || thing->state == &states[S_GIBS]) && thing->extradata)
        translation = ((int) thing->extradata) - 1;
    else
        translation = -1;

    F_DrawSprite(thing->type, thing->state, rot, color | thing->alpha,
                 x >> FRACBITS, y >> FRACBITS, scale, translation);
}
