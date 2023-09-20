/* am_main.c -- automap */

#include "doomdef.h"
#include "p_local.h"
#include "r_local.h"
#include "st_main.h"

#define COLOR_RED     0xA40000FF
#define COLOR_GREEN   0x00C000FF
#define COLOR_BROWN   0x8A5C30ff
#define COLOR_YELLOW  0xCCCC00FF
#define COLOR_GREY    0x808080FF
#define COLOR_AQUA    0x3373B3FF

#define MAXSCALE	1500
#define MINSCALE	200

fixed_t am_box[4]; // 80063110
int am_plycolor;    // 80063120
int am_plyblink;    // 80063124

void AM_DrawSubsectors(player_t *player, fixed_t cx, fixed_t cy, fixed_t bbox[static 4]);
void AM_DrawThings(fixed_t x, fixed_t y, angle_t angle, int color);
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

#define MAXSENSIVITY    10

void AM_Control (player_t *player) // 800004F4
{
	int buttons, oldbuttons;

	controls_t   *cbuttons;
	fixed_t     block[8];
	angle_t     angle;
	fixed_t     fs, fc;
	fixed_t     x, y, x1, y1, x2, y2;
	int         scale, sensitivity;
	int         i;

	if (gamepaused)
        return;

    cbuttons = player->controls;
    buttons = ticbuttons[0];
    oldbuttons = oldticbuttons[0];

    if (player->playerstate != PST_LIVE)
    {
        am_plycolor = 79;
        return;
    }

    if ((buttons & cbuttons->BT_MAP) && !(oldbuttons & cbuttons->BT_MAP))
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

    if (!(buttons & cbuttons->BT_USE))
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
	sensitivity = (int)(((buttons & 0xff00) >> 8) << 24) >> 24;

    if(sensitivity >= MAXSENSIVITY || sensitivity <= -MAXSENSIVITY)
    {
        player->automapx += (sensitivity * scale) / 80;
    }

    /* Analyze analog stick movement (up / down) */
    sensitivity = (int)((buttons) << 24) >> 24;

    if(sensitivity >= MAXSENSIVITY || sensitivity <= -MAXSENSIVITY)
    {
        player->automapy += (sensitivity * scale) / 80;
    }

    /* X movement */
    if (player->automapx > am_box[BOXRIGHT])
    {
        player->automapx = am_box[BOXRIGHT];
    }
    else if (player->automapx < am_box[BOXLEFT])
    {
        player->automapx = am_box[BOXLEFT];
    }

    /* Y movement */
    if (player->automapy > am_box[BOXTOP])
    {
        player->automapy = am_box[BOXTOP];
    }
    else if (player->automapy < am_box[BOXBOTTOM])
    {
        player->automapy = am_box[BOXBOTTOM];
    }

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

    ticbuttons[0] &= ~(cbuttons->BT_LEFT | cbuttons->BT_RIGHT |
                       cbuttons->BT_FORWARD | cbuttons->BT_BACK |
                       PAD_L_TRIG | PAD_R_TRIG | 0xffff);
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
	player_t	*p;
	mobj_t		*mo;
	mobj_t		*next;
	fixed_t		xpos, ypos;
	fixed_t		ox, oy;
	fixed_t		c, s, ts, tc;
	angle_t     angle;
	int			color;
	int			scale;
	int         artflag;
	char        buf[48];
    fixed_t     hcot, vcot;
    fixed_t     screen_box[4];
    fixed_t     boxscale;
    bool        linemode;

    hcot = aspectscale[ScreenAspect];
    vcot = aspectratios[0];
    R_ProjectionMatrix.m[0][0] = hcot & 0xffff0000;
    R_ProjectionMatrix.m[2][0] = (hcot << 16) & 0xffff0000;
    R_ProjectionMatrix.m[0][2] = (vcot >> 16) & 0xffff;
    R_ProjectionMatrix.m[2][2] = vcot & 0xffff;

    I_ClearFB(0x000000ff);

    R_RenderFilter(filt_textures);
    gSPTexture(GFX1++, (1024 << 6)-1, (1024 << 6)-1, 0, G_TX_RENDERTILE, G_ON);

    p = &players[0];

    scale = (p->automapscale << 16);
    xpos = p->mo->x;
    ypos = p->mo->y;

    if (p->onground)
    {
        xpos += (quakeviewx >> 7);
        ypos += quakeviewy;
    }

    if (p->automapflags & AF_FOLLOW)
    {
        angle = (p->mo->angle + ANG270) >> ANGLETOFINESHIFT;
        ox = (p->automapx - xpos) >> 16;
        oy = (p->automapy - ypos) >> 16;
        xpos += ((ox * finecosine(angle)) - (oy * finesine(angle)));
        ypos += ((ox * finesine(angle)));
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

        if (ScreenAspect)
            cx = FixedMul(cx, invaspectscale[ScreenAspect]);

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
    if (p->cheats & CF_ALLMAP)
    {
        for (mo = mobjhead.next; mo != (void*) &mobjhead; mo = next)
        {
            fixed_t bbox[4];

            I_CheckGFX();
            next = mo->next;

            if (mo == p->mo)
                continue;  /* Ignore player */

            if (mo->flags & (MF_NOSECTOR|MF_RENDERLASER))
                continue;

            if ((players->artifacts & 1) != 0 && mo->type == MT_ITEM_ARTIFACT1) continue;
            if ((players->artifacts & 2) != 0 && mo->type == MT_ITEM_ARTIFACT2) continue;
            if ((players->artifacts & 4) != 0 && mo->type == MT_ITEM_ARTIFACT3) continue;

            if (mo->flags & (MF_SHOOTABLE|MF_MISSILE))
                color = COLOR_RED;
            else
                color = COLOR_AQUA;

            bbox[BOXTOP   ] = mo->y + 0x2d413c; // sqrt(2) * 32;
            bbox[BOXBOTTOM] = mo->y - 0x2d413c;
            bbox[BOXRIGHT ] = mo->x + 0x2d413c;
            bbox[BOXLEFT  ] = mo->x - 0x2d413c;

            if (!M_BoxIntersect(bbox, screen_box))
                continue;

            AM_DrawThings(mo->x, mo->y, mo->angle, color);

            if (linemode)
            {

                gSPLine3D(GFX1++, 0, 1, 0 /*flag*/);
                gSPLine3D(GFX1++, 1, 2, 0 /*flag*/);
                gSPLine3D(GFX1++, 2, 0, 0 /*flag*/);
            }
            else
            {
                gSP1Triangle(GFX1++, 0, 1, 2, 0 /*flag*/);
                DEBUG_COUNTER(LastVisTriangles += 1);
            }
        }
    }

    /* SHOW PLAYERS */
    AM_DrawThings(p->mo->x, p->mo->y, p->mo->angle, am_plycolor << 16 | 0xff);

    if (linemode)
    {
        gSPLine3D(GFX1++, 0, 1, 0 /*flag*/);
        gSPLine3D(GFX1++, 1, 2, 0 /*flag*/);
        gSPLine3D(GFX1++, 2, 0, 0 /*flag*/);

        gDPPipeSync(GFX1++);
        gDPSetScissor(GFX1++, G_SC_NON_INTERLACE, 0, 0, XResolution, YResolution);
    }
    else
    {
        gSP1Triangle(GFX1++, 0, 1, 2, 0 /*flag*/);
        DEBUG_COUNTER(LastVisTriangles += 1);
    }


    if (enable_messages)
    {
        if (p->messagetic <= 0)
        {
            sprintf(buf, "LEVEL %d: %s", gamemap, MapInfo[gamemap].name);
            ST_Message(2+HUDmargin,HUDmargin, buf, 196 | 0xffffff00);
        }
        else
        {
            ST_Message(2+HUDmargin,HUDmargin, p->message, 196 | p->messagecolor);
        }
    }
	

	// [Immorpher] kill count
	if(MapStats) {
		sprintf(buf, "KILLS: %d/%d", players[0].killcount, totalkills);
		ST_Message(2+HUDmargin, 212-HUDmargin, buf, 196 | 0xffffff00);
		sprintf(buf, "ITEMS: %d/%d", players[0].itemcount, totalitems);
		ST_Message(2+HUDmargin, 222-HUDmargin, buf, 196| 0xffffff00);
		sprintf(buf, "SECRETS: %d/%d", players[0].secretcount, totalsecret);
		ST_Message(2+HUDmargin, 232-HUDmargin, buf, 196 | 0xffffff00);
	}

    xpos = 297-HUDmargin;
    artflag = 4;
    do
    {
        if ((players->artifacts & artflag) != 0)
        {
            if (artflag == 4)
                BufferedDrawSprite(MT_ITEM_ARTIFACT3, &states[S_ART3], 0, 0xffffff80, xpos, 266-HUDmargin, FRACUNIT);
            else if (artflag == 2)
                BufferedDrawSprite(MT_ITEM_ARTIFACT2, &states[S_ART2], 0, 0xffffff80, xpos, 266-HUDmargin, FRACUNIT);
            else if (artflag == 1)
                BufferedDrawSprite(MT_ITEM_ARTIFACT1, &states[S_ART1], 0, 0xffffff80, xpos, 266-HUDmargin, FRACUNIT);

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

    I_CheckGFX();

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

            left = (bsp->line.dy >> 16) * (dx >> 16);
            right = (dy >> 16) * (bsp->line.dx >> 16);

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

        if(((l->flags & ML_MAPPED) || player->powers[pw_allmap]) || (player->cheats & CF_ALLMAP))
        {
            I_CheckGFX();

            /* */
            /* Figure out color */
            /* */
            color = COLOR_BROWN;

            if((player->powers[pw_allmap] || (player->cheats & CF_ALLMAP)) && !(l->flags & ML_MAPPED))
                color = COLOR_GREY;
            else if (l->flags & ML_SECRET)
                color = COLOR_RED;
            else if(l->special && !(l->flags & ML_HIDEAUTOMAPTRIGGER))
                color = COLOR_YELLOW;
            else if (!(l->flags & ML_TWOSIDED)) /* ONE-SIDED LINE */
                color = COLOR_RED;

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
= AM_DrawThings
=
==================
*/

void AM_DrawThings(fixed_t x, fixed_t y, angle_t angle, int color) // 80001834
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
