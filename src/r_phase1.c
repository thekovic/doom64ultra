
//Renderer phase 1 - BSP traversal

#include "doomdef.h"
#include "r_local.h"

static const int checkcoord[12][4] =                // 8005B110
{
    { 3, 0, 2, 1 },/* Above,Left */
    { 3, 0, 2, 0 },/* Above,Center */
    { 3, 1, 2, 0 },/* Above,Right */
    { 0, 0, 0, 0 },
    { 2, 0, 2, 1 },/* Center,Left */
    { 0, 0, 0, 0 },/* Center,Center */
    { 3, 1, 3, 0 },/* Center,Right */
    { 0, 0, 0, 0 },
    { 2, 0, 3, 1 },/* Below,Left */
    { 2, 1, 3, 1 },/* Below,Center */
    { 2, 1, 3, 0 },/* Below,Right */
    { 0, 0, 0, 0 }
};

void    R_RenderBSPNode(int bspnum) HOT;
boolean R_CheckBBox(fixed_t bspcoord[4]) HOT;
void    R_Subsector(int num) HOT;
void    R_AddLine(seg_t *line) HOT;
void    R_AddSprite(subsector_t *sub) HOT;
void    R_RenderFilter(filtertype_t type) HOT; // [Immorpher] Rendering function to set filter
boolean R_CheckClipRange(angle_t startAngle, angle_t endAngle) HOT;
void R_AddClipRange(angle_t startangle, angle_t endangle) HOT;
void R_ClipClear(void);

// Kick off the rendering process by initializing the solidsubsectors array and then
// starting the BSP traversal.
//

void R_BSP(void) // 80023F30
{
    int count;
    subsector_t **sub;

    DEBUG_CYCLES_START(bsp_start);

    validcount++;
    rendersky = false;

    numdrawsubsectors = 0;
    numdrawvissprites = 0;

    visspritehead = vissprites;

    endsubsector = solidsubsectors; /* Init the free memory pointer */
    R_ClipClear();
    if (viewmaxhalffov > 0)
        R_AddClipRange(viewmaxhalffov, -((int)viewmaxhalffov));

    R_RenderBSPNode(numnodes - 1);  /* Begin traversing the BSP tree for all walls in render range */

    sub = solidsubsectors;
    count = numdrawsubsectors;
    while(count)
    {
        R_AddSprite(*sub);  /* Render each sprite */
        sub++;              /* Inc the sprite pointer */
        count--;
    }

    DEBUG_CYCLES_END(bsp_start, LastBspCycles);
}

//
// Recursively descend through the BSP, classifying nodes according to the
// player's point of view, and render subsectors in view.
//
static boolean R_RenderBspSubsector(int bspnum)
{
    if(bspnum & NF_SUBSECTOR)
    {
        if(bspnum == -1)
            R_Subsector(0);
        else
            R_Subsector(bspnum & (~NF_SUBSECTOR));

        return true;
    }
    return false;
}

#define MAX_BSP_DEPTH 128
//
// RenderBSPNode
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.
void R_RenderBSPNode(int bspnum) // 80024020
{
    int sp = 0;
    node_t *bsp;
    int     side;
    fixed_t    dx, dy;
    fixed_t    left, right;
    int bspstack[MAX_BSP_DEPTH];

    //printf("R_RenderBSPNode\n");
    while(true)
    {
        while (!R_RenderBspSubsector(bspnum))
        {
            if(sp == MAX_BSP_DEPTH)
                break;

            bsp = &nodes[bspnum];
            //            side = R_PointOnSide(viewx,viewy,bsp);
            dx = (viewx - bsp->line.x);
            dy = (viewy - bsp->line.y);

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
        while(!R_CheckBBox (bsp->bbox[side^1]))
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
#if 0
    while(!(bspnum & NF_SUBSECTOR))
    {
        bsp = &nodes[bspnum];

        // Decide which side the view point is on.
        //side = R_PointOnSide(viewx, viewy, bsp);
        dx = (viewx - bsp->line.x);
        dy = (viewy - bsp->line.y);

        left = (bsp->line.dy >> FRACBITS) * (dx >> FRACBITS);
        right = (dy >> FRACBITS) * (bsp->line.dx >> FRACBITS);

        if (right < left)
            side = 0;        /* front side */
        else
            side = 1;        /* back side */

        // check the front space
        if(R_CheckBBox(bsp->bbox[side]))
        {
            R_RenderBSPNode(bsp->children[side]);
        }

        // continue down the back space
        if(!R_CheckBBox(bsp->bbox[side^1]))
        {
            return;
        }

        bspnum = bsp->children[side^1];
    }

    // subsector with contents
    // add all the drawable elements in the subsector
    if(bspnum == -1)
        bspnum = 0;

    R_Subsector(bspnum & ~NF_SUBSECTOR);
#endif
}

typedef u64 point_t;
#define POINT_PACK(_x, _y) ((((u64)(u32)(_x))<<32)|(u64)(((u32)(_y))&0xffffffff))
#define POINT_UNPACK(_p, _x, _y) do { \
        point_t _pt = (_p); \
        (_x) = (u32)(_pt >> 32); \
        (_y) = ((u32)_pt) & 0xffffffff; \
    } while(0)

static point_t R_PointToViewSpace(int x, int y) {
    int xx;
    x -= viewx;
    y -= viewy;
    xx = FixedMul(viewsin, x) - FixedMul(viewcos, y);
    y = FixedMul(viewcos, x) + FixedMul(viewsin, y);
    return POINT_PACK(xx, y);
}

//
// Checks BSP node/subtree bounding box. Returns true if some part of the bbox
// might be visible.
//


boolean R_CheckBBox(fixed_t bspcoord[4]) // 80024170
{
    int boxx;
    int boxy;
    int boxpos;

    fixed_t x1, y1, x2, y2;
    angle_t angle1, angle2;

    // find the corners of the box that define the edges from current viewpoint
    if (viewx < bspcoord[BOXLEFT])
        boxx = 0;
    else if (viewx <= bspcoord[BOXRIGHT])
        boxx = 1;
    else
        boxx = 2;

    if (viewy > bspcoord[BOXTOP])
        boxy = 0;
    else if (viewy >= bspcoord[BOXBOTTOM])
        boxy = 1;
    else
        boxy = 2;

    boxpos = (boxy << 2) + boxx;
    if (boxpos == 5)
        return true;

    x1 = bspcoord[checkcoord[boxpos][0]];
    y1 = bspcoord[checkcoord[boxpos][1]];
    x2 = bspcoord[checkcoord[boxpos][2]];
    y2 = bspcoord[checkcoord[boxpos][3]];

    POINT_UNPACK(R_PointToViewSpace(x1, y1), x1, y1);
    POINT_UNPACK(R_PointToViewSpace(x2, y2), x2, y2);

    angle1 = R_PointToAngle2(0, 0, y1, -x1);
    angle2 = R_PointToAngle2(0, 0, y2, -x2);
    return R_CheckClipRange(angle2, angle1);
}

//
// Determine floor/ceiling planes, add sprites of things in sector,
// draw one or more segments.
//

void R_Subsector(int num) // 8002451C
{
    subsector_t *sub;
    seg_t       *line;
    int          count;

    if (num >= numsubsectors)
    {
        I_Error("R_Subsector: ss %i with numss = %i", num, numsubsectors);
    }

    if (numdrawsubsectors < MAXSUBSECTORS)
    {
        numdrawsubsectors++;

        sub = &subsectors[num];
        sub->drawindex = numdrawsubsectors;

        *endsubsector = sub;//copy subsector
        endsubsector++;

        frontsector = sub->sector;

        line = &segs[sub->firstline];
        count = sub->numlines;
        do
        {
            R_AddLine(line);    /* Render each line */
            ++line;                /* Inc the line pointer */
        } while (--count);        /* All done? */
    }
}

//
// Clips the given segment and adds any visible pieces to the line list.
//

void R_AddLine(seg_t *line) // 80024604
{
    sector_t    *back;
    vertex_t    *vrt, *vrt2;
    fixed_t     x1, y1, x2, y2;
    angle_t     angle1, angle2;
    int         flags;

    back = line->backsector;

    line->flags &= ~1;

    vrt = line->v1;
    if (vrt->validcount != validcount)
    {
        POINT_UNPACK(R_PointToViewSpace(vrt->x, vrt->y), x1, y1);

        vrt->vx = x1;
        vrt->vy = y1;

        vrt->validcount = validcount;
    }
    else
    {
        x1 = vrt->vx;
        y1 = vrt->vy;
    }

    vrt2 = line->v2;
    if (vrt2->validcount != validcount)
    {
        POINT_UNPACK(R_PointToViewSpace(vrt2->x, vrt2->y), x2, y2);

        vrt2->vx = x2;
        vrt2->vy = y2;

        vrt2->validcount = validcount;
    }
    else
    {
        x2 = vrt2->vx;
        y2 = vrt2->vy;
    }

    angle1 = R_PointToAngle2(0, 0, y1, -x1);
    angle2 = R_PointToAngle2(0, 0, y2, -x2);

    // Back side, i.e. backface culling - read: endAngle >= startAngle!
    if (angle2 - angle1 < ANG180)
      return;
    if (!R_CheckClipRange(angle2, angle1))
      return;

    line->flags |= 1;
    line->linedef->flags |= ML_MAPPED;

    if (frontsector->ceilingpic == -1 || frontsector->floorpic == -1)
        rendersky = true;

    flags = line->linedef->flags;
    if (!(flags & (ML_DONTOCCLUDE|ML_DRAWMASKED)))
    {
        if(!back ||
            back->ceilingheight <= frontsector->floorheight ||
            back->floorheight   >= frontsector->ceilingheight ||
            back->floorheight   == back->ceilingheight) // New line on Doom 64
        {
            R_AddClipRange(angle2, angle1);
        }
    }
}

void R_AddSprite(subsector_t *sub) // 80024A98
{
    spriteN64_t *sprite;
    mobj_t *thing;
    spritedef_t        *sprdef;
    spriteframe_t    *sprframe;

    subsector_t     *pSub;
    subsector_t     *CurSub;
    vissprite_t     *VisSrpCur, *VisSrpCurTmp;
    vissprite_t     *VisSrpNew;

    angle_t         ang, ang2;
    unsigned int    rot;
    boolean         flip;
    int             lump;
    fixed_t         tx, tx2, ty;
    fixed_t         x, y;

    sub->vissprite = NULL;

    for (thing = sub->sector->thinglist; thing; thing = thing->snext)
    {
        if (thing->subsector != sub)
            continue;

        if (thing == cameratarget)
            continue;

        if (numdrawvissprites >= MAXVISSPRITES)
            break;

        if (thing->flags & MF_RENDERLASER)
        {
            visspritehead->zdistance = MAXINT;
            visspritehead->thing = thing;
            visspritehead->next = sub->vissprite;
            sub->vissprite = visspritehead;

            visspritehead++;
            numdrawvissprites++;
        }
        else
        {
            if (thing->alpha <= 0)
                continue;

            // transform origin relative to viewpoint
            POINT_UNPACK(R_PointToViewSpace(thing->x, thing->y), tx, ty);

            sprdef = &sprites[thing->sprite];
            sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];
            if (sprframe->rotate != 0)
            {
                ang = R_PointToAngle2(viewx, viewy, thing->x, thing->y);
                rot = ((ang - thing->angle) + ((unsigned int)(ANG45 / 2) * 9)) >> 29;
                lump = sprframe->lump[rot];
                flip = (boolean)(sprframe->flip[rot]);
            }
            else
            {
                lump = sprframe->lump[0];
                flip = (boolean)(sprframe->flip[0]);
            }

            // TDOO - cache sprite w/xoff/h so can put this after clipping check
            sprite = W_CacheLumpNum(lump, PU_CACHE, dec_jag, sizeof(spriteN64_t));

            if (flip)
                tx -= (((int)sprite->width) - sprite->xoffs) << FRACBITS;
            else
                tx -= ((int)sprite->xoffs) << FRACBITS;

            tx2 = tx + (((int)sprite->width) << FRACBITS);

            // frustum clipping
            if (viewmaxhalffov)
            {
                ang = R_PointToAngle2(0, 0, ty, -tx);
                ang2 = R_PointToAngle2(0, 0, ty, -tx2);
                if (((int)ang) < -(int)viewmaxhalffov || ((int)ang2) > (int)viewmaxhalffov)
                    continue;
            }

            // rotate by pitch into viewspace
            if (viewpitch)
            {
                fixed_t z = ((int)viewpitch) > 0 ? thing->z + (sprite->height << FRACBITS) : thing->z;
                ty = FixedMul(viewpitchsin, z - viewz) + FixedMul(viewpitchcos, ty);
            }

            // thing is behind view plane?
            if (ty < 0)
                continue;

            visspritehead->zdistance = ty;
            visspritehead->thing = thing;
            visspritehead->lump = lump;
            visspritehead->flip = flip;
            visspritehead->next = NULL;
            visspritehead->sector = sub->sector;

            CurSub = sub;
            if (ty < (MAXZ<<FRACBITS))
            {
                if (thing->flags & (MF_CORPSE|MF_SHOOTABLE))
                {
                    x = (sprite->width >> 1) * viewsin;
                    y = (sprite->width >> 1) * viewcos;

                    pSub = R_PointInSubsector((thing->x - x), (thing->y + y));
                    if ((pSub->drawindex) && (pSub->drawindex < sub->drawindex)) {
                        CurSub = pSub;
                    }

                    pSub = R_PointInSubsector((thing->x + x), (thing->y - y));
                    if ((pSub->drawindex) && (pSub->drawindex < CurSub->drawindex)) {
                        CurSub = pSub;
                    }
                }
            }

            VisSrpCur = CurSub->vissprite;
            VisSrpNew = NULL;

            if (VisSrpCur)
            {
                VisSrpCurTmp = VisSrpCur;
                while ((VisSrpCur = VisSrpCurTmp, ty < VisSrpCur->zdistance))
                {
                    VisSrpCur = VisSrpCurTmp->next;
                    VisSrpNew = VisSrpCurTmp;

                    if (VisSrpCur == NULL)
                        break;

                    VisSrpCurTmp = VisSrpCur;
                }
            }

            if (VisSrpNew)
                VisSrpNew->next = visspritehead;
            else
                CurSub->vissprite = visspritehead;

            visspritehead->next = VisSrpCur;

            numdrawvissprites++;
            visspritehead++;
        }
    }
}

#if 0
static void R_RenderBSPNodeNoClip(int bspnum) // 80024E64
{
    subsector_t *sub;
    seg_t       *line;
    int          count;
    node_t      *bsp;
    int          side;
    fixed_t      dx, dy;
    fixed_t      left, right;

    while(!(bspnum & NF_SUBSECTOR))
    {
        bsp = &nodes[bspnum];

        // Decide which side the view point is on.
        //side = R_PointOnSide(viewx, viewy, bsp);
        dx = (viewx - bsp->line.x);
        dy = (viewy - bsp->line.y);

        left = (bsp->line.dy >> FRACBITS) * (dx >> FRACBITS);
        right = (dy >> FRACBITS) * (bsp->line.dx >> FRACBITS);

        if (right < left)
            side = 1;       /* back side */
        else
            side = 0;       /* front side */

        R_RenderBSPNodeNoClip(bsp->children[side ^ 1]);

        bspnum = bsp->children[side];
    }

    // subsector with contents
    // add all the drawable elements in the subsector

    numdrawsubsectors++;

    sub = &subsectors[bspnum & ~NF_SUBSECTOR];
    sub->drawindex = numdrawsubsectors;

    *endsubsector = sub;//copy subsector
    endsubsector++;

    frontsector = sub->sector;

    line = &segs[sub->firstline];
    count = sub->numlines;
    do
    {
        line->flags |= 1;   /* Render each line */
        ++line;             /* Inc the line pointer */
    } while (--count);      /* All done? */
}
#endif
