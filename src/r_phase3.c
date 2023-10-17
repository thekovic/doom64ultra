
//Renderer phase 3 - World Rendering Routines

#include "doomdef.h"
#include "r_local.h"

//-----------------------------------//
void R_RenderWorld(subsector_t *sub) HOT;

void R_WallPrep(seg_t *seg) HOT;
void R_RenderWall(seg_t *seg, int flags, int texture, int topHeight, int bottomHeight, int topOffset, int bottomOffset, int topColor, int bottomColor) HOT;
void R_RenderSwitch(seg_t *seg, int texture, int topOffset, int color) HOT;

void R_RenderPlane(leaf_t *leaf, int numverts, int zpos, int texture, int xpos, int ypos, int color) HOT;

void R_RenderThings(subsector_t *sub) HOT;
void R_RenderLaser(mobj_t *thing) HOT;
void R_RenderPSprites(void) HOT;
//-----------------------------------//

void R_RenderAll(void) // 80026590
{
    subsector_t *sub;
    DEBUG_CYCLES_START(phase3_start);

    while (endsubsector--, (endsubsector >= solidsubsectors))
    {
        sub = *endsubsector;
        frontsector = sub->sector;
        R_RenderWorld(sub);

        sub->drawindex = 0x7fff;
    }

    DEBUG_CYCLES_END(phase3_start, LastPhase3Cycles);
}

void R_RenderWorld(subsector_t *sub) // 80026638
{
    leaf_t *lf;
    seg_t *seg;

    fixed_t xoffset;
    fixed_t yoffset;
    int numverts;
    int i;

    DEBUG_COUNTER(LastVisSubsectors += 1);
    I_CheckGFX();

    gDPSetPrimColor(GFX1++, 0, frontsector->lightlevel, 0, 0, 0, 255);

    R_RenderModes(rm_texture);

    numverts = sub->numverts;

    /* */
    /* Render Walls */
    /* */
    lf = &leafs[sub->leaf];
    for (i = 0; i < numverts; i++)
    {
        seg = lf->seg;

        if (seg && (seg->flags & 1))
        {
            R_WallPrep(seg);
        }

        lf++;
    }

    /* */
    /* Render Ceilings */
    /* */
    if ((frontsector->ceilingpic != -1) && (viewz < frontsector->ceilingheight))
    {
        if (frontsector->flags & MS_SCROLLCEILING)
        {
            xoffset = frontsector->xoffset;
            yoffset = frontsector->yoffset;
        }
        else
        {
            xoffset = 0;
            yoffset = 0;
        }

        lf = &leafs[sub->leaf];
        R_RenderPlane(lf, numverts, frontsector->ceilingheight >> FRACBITS,
                        textures[frontsector->ceilingpic],
                        xoffset, yoffset,
                        lights[frontsector->colors[0]].rgba);
    }

    /* */
    /* Render Floors */
    /* */
    if ((frontsector->floorpic != -1) && (frontsector->floorheight < viewz))
    {
        if (!(frontsector->flags & MS_LIQUIDFLOOR))
        {
            if (frontsector->flags & MS_SCROLLFLOOR)
            {
                xoffset = frontsector->xoffset;
                yoffset = frontsector->yoffset;
            }
            else
            {
                xoffset = 0;
                yoffset = 0;
            }

            lf = &leafs[sub->leaf];
            R_RenderPlane(lf, numverts, frontsector->floorheight >> FRACBITS,
                            textures[frontsector->floorpic],
                            xoffset, yoffset,
                            lights[frontsector->colors[1]].rgba);
        }
        else
        {
            if (frontsector->flags & MS_SCROLLFLOOR)
            {
                xoffset = frontsector->xoffset;
                yoffset = frontsector->yoffset;
            }
            else
            {
                xoffset = scrollfrac;
                yoffset = 0;
            }

            R_RenderModes(rm_liquid);

            //--------------------------------------------------------------
            lf = &leafs[sub->leaf];
            R_RenderPlane(lf, numverts, frontsector->floorheight >> FRACBITS,
                            textures[frontsector->floorpic + 1],
                            xoffset, yoffset,
                            lights[frontsector->colors[1]].rgba);
            //--------------------------------------------------------------
            gDPSetPrimColor(GFX1++, 0, frontsector->lightlevel, 0, 0, 0, 160);

            lf = &leafs[sub->leaf];
            R_RenderPlane(lf, numverts, frontsector->floorheight >> FRACBITS,
                            textures[frontsector->floorpic],
                            -yoffset, xoffset,
                            lights[frontsector->colors[1]].rgba);
        }
    }

    /* */
    /* Render Things */
    /* */
    R_RenderThings(sub);
}

void R_WallPrep(seg_t *seg) // 80026A44
{
    sector_t *backsector;
    line_t *li;
    side_t *side;
    fixed_t f_ceilingheight;
    fixed_t f_floorheight;
    fixed_t b_ceilingheight;
    fixed_t b_floorheight;
    fixed_t m_top;
    fixed_t m_bottom;
    fixed_t height;
    fixed_t rowoffs;
    int pic;

    unsigned int height2;
    unsigned int r1 = 0, g1 = 0, b1 = 0;
    unsigned int r2 = 0, g2 = 0, b2 = 0;
    unsigned int thingcolor;
    unsigned int upcolor;
    unsigned int lowcolor;
    unsigned int topcolor = 0;
    unsigned int bottomcolor = 0;
    unsigned int tmp_upcolor = 0;
    unsigned int tmp_lowcolor = 0;
    int toffset;

    li = seg->linedef;
    side = seg->sidedef;

    // [GEC] Prevents errors in textures in T coordinates, but is not applied to switches
    toffset = side->rowoffset >> FRACBITS;

    f_ceilingheight = frontsector->ceilingheight >> FRACBITS;
    f_floorheight = frontsector->floorheight >> FRACBITS;

    thingcolor = lights[frontsector->colors[2]].rgba;
    upcolor = lights[frontsector->colors[3]].rgba;
    lowcolor = lights[frontsector->colors[4]].rgba;

    if (li->flags & ML_BLENDING)
    {
        r1 = upcolor  >> 24;
        g1 = upcolor  >> 16 & 0xff;
        b1 = upcolor  >> 8 & 0xff;
        r2 = lowcolor >> 24;
        g2 = lowcolor >> 16 & 0xff;
        b2 = lowcolor >> 8 & 0xff;

        tmp_upcolor = upcolor;
        tmp_lowcolor = lowcolor;
    }
    else
    {
        topcolor = thingcolor;
        bottomcolor = thingcolor;
    }

    m_bottom = f_floorheight; // set middle bottom
    m_top = f_ceilingheight;  // set middle top

    backsector = seg->backsector;
    if (backsector)
    {
        b_floorheight = backsector->floorheight >> FRACBITS;
        b_ceilingheight = backsector->ceilingheight >> FRACBITS;

        if ((backsector->ceilingheight < frontsector->ceilingheight) && (backsector->ceilingpic != -1))
        {
            if (li->flags & ML_DONTPEGTOP)
            {
                height = (f_ceilingheight - b_ceilingheight);
                rowoffs = toffset + height;
            }
            else
            {
                height = (f_ceilingheight - b_ceilingheight);
                rowoffs = ((height + 127) & -128) + toffset;
            }

            if (li->flags & ML_BLENDING)
            {
                if (!(li->flags & ML_BLENDFULLTOP))
                {
                    if (f_floorheight < f_ceilingheight)
                    {
                        height2 = ((height << FRACBITS) / (f_ceilingheight - f_floorheight));
                    }
                    else
                    {
                        height2 = 0;
                    }

                    tmp_lowcolor = (((((r2 - r1) * height2) >> FRACBITS) + r1) << 24) |
                                   (((((g2 - g1) * height2) >> FRACBITS) + g1) << 16) |
                                   (((((b2 - b1) * height2) >> FRACBITS) + b1) << 8)  | 0xff;
                }

                if (li->flags & ML_INVERSEBLEND)
                {
                    bottomcolor = tmp_upcolor;
                    topcolor = tmp_lowcolor;
                }
                else
                {
                    topcolor = tmp_upcolor;
                    bottomcolor = tmp_lowcolor;
                }

                // clip middle color upper
                upcolor = tmp_lowcolor;
            }
            R_RenderModes(rm_texture);

            DEBUG_COUNTER(LastVisSegs += 1);
            R_RenderWall(seg, li->flags, textures[side->toptexture],
                         f_ceilingheight, b_ceilingheight,
                         rowoffs - height, height,
                         topcolor, bottomcolor);

            m_top = b_ceilingheight; // clip middle top height
            if ((li->flags & (ML_CHECKFLOORHEIGHT|ML_SWITCHX08)) == ML_SWITCHX08)
            {
                if (SWITCHMASK(li->flags) == ML_SWITCHX04)
                    pic = side->bottomtexture;
                else
                    pic = side->midtexture;

                rowoffs = side->rowoffset >> FRACBITS;
                R_RenderSwitch(seg, pic, b_ceilingheight + rowoffs + 48, thingcolor);
            }
        }

        if (frontsector->floorheight < backsector->floorheight)
        {
            height = (f_ceilingheight - b_floorheight);

            if ((li->flags & ML_DONTPEGBOTTOM) == 0)
                rowoffs = toffset;
            else
                rowoffs = height + toffset;

            if (li->flags & ML_BLENDING)
            {
                if (!(li->flags & ML_BLENDFULLBOTTOM))
                {
                    if (f_floorheight < f_ceilingheight)
                    {
                        height2 = ((height << FRACBITS) / (f_ceilingheight - f_floorheight));
                    }
                    else
                    {
                        height2 = 0;
                    }

                    tmp_upcolor = (((((r2 - r1) * height2) >> FRACBITS) + r1) << 24) |
                                  (((((g2 - g1) * height2) >> FRACBITS) + g1) << 16) |
                                  (((((b2 - b1) * height2) >> FRACBITS) + b1) << 8)  | 0xff;
                }

                topcolor = tmp_upcolor;
                bottomcolor = lowcolor;

                // clip middle color lower
                lowcolor = tmp_upcolor;
            }
            R_RenderModes(rm_texture);

            R_RenderWall(seg, li->flags, textures[side->bottomtexture],
                         b_floorheight, f_floorheight,
                         rowoffs, b_floorheight - f_floorheight,
                         topcolor, bottomcolor);

            m_bottom = b_floorheight; // clip middle bottom height
            if ((li->flags & (ML_CHECKFLOORHEIGHT|ML_SWITCHX08)) == ML_CHECKFLOORHEIGHT)
            {
                if (SWITCHMASK(li->flags) == ML_SWITCHX02)
                    pic = side->toptexture;
                else
                    pic = side->midtexture;

                rowoffs = side->rowoffset >> FRACBITS;
                R_RenderSwitch(seg, pic, b_floorheight + rowoffs - 16, thingcolor);
            }
        }

        if (!(li->flags & ML_DRAWMASKED))
        {
            return;
        }
    }

    R_RenderModes(rm_texture);

    if (li->flags & ML_DONTPEGBOTTOM)
    {
        height = m_top - m_bottom;
        rowoffs = ((height + 127) & -128) + toffset;
    }
    else if (li->flags & ML_DONTPEGTOP)
    {
        rowoffs = toffset - m_bottom;
        height = m_top - m_bottom;
    }
    else
    {
        height = m_top - m_bottom;
        rowoffs = toffset + height;
    }

    if (li->flags & ML_BLENDING)
    {
        topcolor = upcolor;
        bottomcolor = lowcolor;
    }

    DEBUG_COUNTER(LastVisSegs += 1);
    R_RenderWall(seg, li->flags, textures[side->midtexture],
                 m_top, m_bottom,
                 rowoffs - height, height,
                 topcolor, bottomcolor);

    if ((li->flags & (ML_CHECKFLOORHEIGHT|ML_SWITCHX08)) == (ML_CHECKFLOORHEIGHT|ML_SWITCHX08))
    {
        if (SWITCHMASK(li->flags) == ML_SWITCHX02)
            pic = side->toptexture;
        else
            pic = side->bottomtexture;

        rowoffs = side->rowoffset >> FRACBITS;
        R_RenderSwitch(seg, pic, m_bottom + rowoffs + 48, thingcolor);
    }
}

void R_RenderWall(seg_t *seg, int flags, int texture, int topHeight, int bottomHeight,
                  int topOffset, int offsetHeight, int topColor, int bottomColor) // 80027138
{
    byte *data;
    byte *src;
    vertex_t *v1;
    vertex_t *v2;
    int cms, cmt;
    int wshift, hshift;

    if ((texture >> 4) == firsttex + 1)
        return;

    topOffset = (topOffset & 1023) - 1024;

    v1 = seg->v1;
    v2 = seg->v2;

    // x coordinates
    VTX1[0].v.ob[0] = VTX1[3].v.ob[0] = (signed short)(v1->x >> FRACBITS);
    VTX1[1].v.ob[0] = VTX1[2].v.ob[0] = (signed short)(v2->x >> FRACBITS);

    // y coordinates
    VTX1[0].v.ob[1] = VTX1[1].v.ob[1] = topHeight;
    VTX1[3].v.ob[1] = VTX1[2].v.ob[1] = bottomHeight;

    // z coordinates
    VTX1[0].v.ob[2] = VTX1[3].v.ob[2] = (signed short)-(v1->y >> FRACBITS);
    VTX1[1].v.ob[2] = VTX1[2].v.ob[2] = (signed short)-(v2->y >> FRACBITS);

    // vertex color
    *(int*)VTX1[0].v.cn = *(int*)VTX1[1].v.cn = topColor;
    *(int*)VTX1[2].v.cn = *(int*)VTX1[3].v.cn = bottomColor;

    // texture s coordinates
    // [GEC] Prevents errors in textures in S coordinates
    VTX1[0].v.tc[0] = VTX1[3].v.tc[0] = (((seg->sidedef->textureoffset + seg->offset) >> 11) & ((1023 << 5) | 0x1f)) - (1024 << 5);
    VTX1[1].v.tc[0] = VTX1[2].v.tc[0] = VTX1[0].v.tc[0] + (seg->length << 1);

    // texture t coordinates
    VTX1[0].v.tc[1] = VTX1[1].v.tc[1] = topOffset << 5;
    VTX1[2].v.tc[1] = VTX1[3].v.tc[1] = (topOffset + offsetHeight) << 5;

    if (flags & ML_HMIRROR) {
        cms = G_TX_MIRROR;
    }
    else {
        cms = G_TX_NOMIRROR;
    }

    if (flags & ML_VMIRROR) {
        cmt = G_TX_MIRROR;
    }
    else {
        cmt = G_TX_NOMIRROR;
    }

    if ((texture != globallump) || (globalcm != (cms | cmt)))
    {
        /*
        In Doom 64 all textures are compressed with the second method (dec_d64),
        in the original line it was declared that if a texture was not stored,
        it would be stored from the DOOM64.WAD and decompressed with the Jaguar Doom
        method (dec_jag) which is wrong since all the textures are previously
        loaded from the P_Init function with the second decompression method (dec_d64)
        */
        //data = W_CacheLumpNum(texture >> 4, PU_CACHE, dec_jag); // error decomp mode
        data = W_CacheLumpNum(texture >> 4, PU_CACHE, dec_d64, sizeof(textureN64_t)); /* [GEC] FIXED */

        wshift = ((textureN64_t*)data)->wshift;
        hshift = ((textureN64_t*)data)->hshift;

        src = data + sizeof(textureN64_t);

        // Load Image Data
        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0,
                     (((1 << wshift) * (1 << hshift)) >> 2) - 1, 0);

        gDPPipeSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_4b,
                   (((1 << wshift) >> 1) + 7) >> 3, 0, G_TX_RENDERTILE, 0,
                   cmt, hshift, 0,
                   cms, wshift, 0);

        gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0,
                       ((1 << wshift) - 1) << 2,
                       ((1 << hshift) - 1) << 2);

        // Load Palette Data
        gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1,
                           src + (1 << ((wshift + hshift + 31) & 31)) + ((texture & 15) << 5));

        gDPTileSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 15);

        gDPPipeSync(GFX1++);

        globallump = texture;
        globalcm = (cms | cmt);
    }

    gSPVertex(GFX1++, VTX1, 4, 0);
    gSP1Quadrangle(GFX1++, 0, 1, 2, 3, 1);
    DEBUG_COUNTER(LastVisTriangles += 2);

    VTX1 += 4;
}

void R_RenderSwitch(seg_t *seg, int texture, int topOffset, int color) // 80027654
{
    byte *data;
    byte *src;
    vertex_t *v1;
    vertex_t *v2;
    fixed_t x, y;
    fixed_t sin, cos;
    int wshift = 5, hshift = 5;

    if (texture <= 1) return;

    v1 = seg->linedef->v1;
    v2 = seg->linedef->v2;

    x = (v1->x + v2->x);
    if (x < 0) {x = x + 1;}

    y = (v1->y + v2->y);
    if (y < 0) {y = y + 1;}

    x >>= 1;
    y >>= 1;

    cos = finecosine(seg->angle >> ANGLETOFINESHIFT) << 1;
    sin = finesine(seg->angle >> ANGLETOFINESHIFT) << 1;

    // x coordinates
    VTX1[0].v.ob[0] = VTX1[3].v.ob[0] = ((x) - (cos << 3) + sin) >> FRACBITS;
    VTX1[1].v.ob[0] = VTX1[2].v.ob[0] = ((x) + (cos << 3) + sin) >> FRACBITS;

    // y coordinates
    VTX1[0].v.ob[1] = VTX1[1].v.ob[1] = topOffset;
    VTX1[3].v.ob[1] = VTX1[2].v.ob[1] = topOffset - 32;

    // z coordinates
    VTX1[0].v.ob[2] = VTX1[3].v.ob[2] = ((-y) + (sin << 3) + cos) >> FRACBITS;
    VTX1[1].v.ob[2] = VTX1[2].v.ob[2] = ((-y) - (sin << 3) + cos) >> FRACBITS;

    // texture s coordinates
    VTX1[0].v.tc[0] = VTX1[3].v.tc[0] = (0 << 6);
    VTX1[1].v.tc[0] = VTX1[2].v.tc[0] = ((1 << wshift) << 6);

    // texture t coordinates
    VTX1[0].v.tc[1] = VTX1[1].v.tc[1] = (0 << 6);
    VTX1[2].v.tc[1] = VTX1[3].v.tc[1] = ((1 << hshift) << 6);

    // vertex color
    *(int*)VTX1[0].v.cn = *(int*)VTX1[1].v.cn = *(int*)VTX1[2].v.cn = *(int*)VTX1[3].v.cn = color;

    if (texture != globallump)
    {
        /*
        In Doom 64 all textures are compressed with the second method (dec_d64),
        in the original line it was declared that if a texture was not stored,
        it would be stored from the DOOM64.WAD and decompressed with the Jaguar Doom
        method (dec_jag) which is wrong since all the textures are previously
        loaded from the P_Init function with the second decompression method (dec_d64)
        */
        //data = W_CacheLumpNum(firsttex + texture, PU_CACHE, dec_jag); // error decomp mode
        data = W_CacheLumpNum(firsttex + texture, PU_CACHE, dec_d64, sizeof(textureN64_t)); /* [GEC] FIXED */

        wshift = ((textureN64_t*)data)->wshift;
        hshift = ((textureN64_t*)data)->hshift;

        src = data + sizeof(textureN64_t);

        // Load Image Data
        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0,
                         (((1 << wshift) * (1 << hshift)) >> 2) - 1, 0);

        gDPPipeSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_4b,
                       (((1 << wshift) >> 1) + 7) >> 3, 0, G_TX_RENDERTILE, 0,
                       0, 0, 0,
                       0, 0, 0);

        gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0,
                           ((1 << wshift) - 1) << 2,
                           ((1 << hshift) - 1) << 2);

        // Load Palette Data
        gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1,
                            src + (1 << ((wshift + hshift + 31) & 31)));

        gDPTileSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 15);

        gDPPipeSync(GFX1++);

        globallump = texture;
    }

    R_RenderModes(rm_switch);

    gSPVertex(GFX1++, VTX1, 4, 0);
    gSP1Quadrangle(GFX1++, 0, 1, 2, 3, 1);
    DEBUG_COUNTER(LastVisTriangles += 2);

    VTX1 += 4;
}

void R_RenderPlane(leaf_t *leaf, int numverts, int zpos, int texture, int xpos, int ypos, int color) // 80027B68
{
    byte *data;
    byte *src;
    vertex_t *vrt;
    fixed_t x;
    fixed_t y;
    int idx, i;
    int v00, v01, v02;

    if (texture != globallump)
    {
        /*
        In Doom 64 all textures are compressed with the second method (dec_d64),
        in the original line it was declared that if a texture was not stored,
        it would be stored from the DOOM64.WAD and decompressed with the Jaguar Doom
        method (dec_jag) which is wrong since all the textures are previously
        loaded from the P_Init function with the second decompression method (dec_d64)
        */
        //data = W_CacheLumpNum(texture >> 4, PU_CACHE, dec_jag); // error decomp mode
        data = W_CacheLumpNum(texture >> 4, PU_CACHE, dec_d64, sizeof(textureN64_t)); /* [GEC] FIXED */

        src = data + sizeof(textureN64_t);

        // Load Image Data
        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0,
                         (((1 << 6) * (1 << 6)) >> 2) - 1, 0);

        gDPPipeSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_4b,
                       (((1 << 6) >> 1) + 7) >> 3, 0, G_TX_RENDERTILE, 0,
                       0, 6, 0,
                       0, 6, 0);

        gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0,
                           ((1 << 6) - 1) << 2,
                           ((1 << 6) - 1) << 2);

        // Load Palette Data
        gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1,
                               src + (1 << (6 + 6 - 1)) + ((texture & 15) << 5));

        gDPTileSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 15);

        gDPPipeSync(GFX1++);

        globallump = texture;
    }

    DEBUG_COUNTER(LastVisLeaves += 1);

    vrt = leaf->vertex;
    VTX1[0].v.ob[0] = (vrt->x >> FRACBITS);
    VTX1[0].v.ob[1] = zpos;
    VTX1[0].v.ob[2] =-(vrt->y >> FRACBITS);
    VTX1[0].v.tc[0] = ((((vrt->x + xpos) & 0x3f0000U) >> FRACBITS) << 5);
    VTX1[0].v.tc[1] =-((((vrt->y + ypos) & 0x3f0000U) >> FRACBITS) << 5);
    *(int *)VTX1[0].v.cn = color;

    x = ((vrt->x + xpos) >> FRACBITS) & -64;
    y = ((vrt->y + ypos) >> FRACBITS) & -64;

    if (numverts >= 32)
        numverts = 32;

    gSPVertex(GFX1++, VTX1, numverts, 0);
    VTX1++;

    if (numverts & 1)
    {
        idx = 2;
        gSP1Triangle(GFX1++, 0, 1, 2, 0);
        DEBUG_COUNTER(LastVisTriangles += 1);
    }
    else
    {
        idx = 1;
    }

    leaf++;
    numverts--;

    if (idx < numverts)
    {
        v00 = idx + 0;
        v01 = idx + 1;
        v02 = idx + 2;
        do
        {
            gSP2Triangles(GFX1++,
                          v00, v01, v02, 0, // 0, 1, 2
                          v00, v02, 0, 0);  // 0, 2, 0
            DEBUG_COUNTER(LastVisTriangles += 2);

            v00 += 2;
            v01 += 2;
            v02 += 2;
        } while (v02 < (numverts + 2));
    }

    /*i = 0;
    if (numverts > 0)
    {
        if ((numverts & 3))
        {
            while(i != (numverts & 3))
            {
                vrt = leaf->vertex;
                VTX1[0].v.ob[0] = (vrt->x >> FRACBITS);
                VTX1[0].v.ob[1] = zpos;
                VTX1[0].v.ob[2] =-(vrt->y >> FRACBITS);
                VTX1[0].v.tc[0] = ((((vrt->x + xpos) >> FRACBITS) - x) << 5);
                VTX1[0].v.tc[1] =-((((vrt->y + ypos) >> FRACBITS) - y) << 5);
                *(int *)VTX1[0].v.cn = color;
                VTX1++;
                leaf++;
                i++;
            }
        }

        while(i != numverts)
        {
            vrt = leaf->vertex;
            VTX1[0].v.ob[0] = (vrt->x >> FRACBITS);
            VTX1[0].v.ob[1] = zpos;
            VTX1[0].v.ob[2] =-(vrt->y >> FRACBITS);
            VTX1[0].v.tc[0] = ((((vrt->x + xpos) >> FRACBITS) - x) << 5);
            VTX1[0].v.tc[1] =-((((vrt->y + ypos) >> FRACBITS) - y) << 5);
            *(int *)VTX1[0].v.cn = color;
            VTX1++;
            leaf++;

            vrt = leaf->vertex;
            VTX1[0].v.ob[0] = (vrt->x >> FRACBITS);
            VTX1[0].v.ob[1] = zpos;
            VTX1[0].v.ob[2] =-(vrt->y >> FRACBITS);
            VTX1[0].v.tc[0] = ((((vrt->x + xpos) >> FRACBITS) - x) << 5);
            VTX1[0].v.tc[1] =-((((vrt->y + ypos) >> FRACBITS) - y) << 5);
            *(int *)VTX1[0].v.cn = color;
            VTX1++;
            leaf++;

            vrt = leaf->vertex;
            VTX1[0].v.ob[0] = (vrt->x >> FRACBITS);
            VTX1[0].v.ob[1] = zpos;
            VTX1[0].v.ob[2] =-(vrt->y >> FRACBITS);
            VTX1[0].v.tc[0] = ((((vrt->x + xpos) >> FRACBITS) - x) << 5);
            VTX1[0].v.tc[1] =-((((vrt->y + ypos) >> FRACBITS) - y) << 5);
            *(int *)VTX1[0].v.cn = color;
            VTX1++;
            leaf++;

            vrt = leaf->vertex;
            VTX1[0].v.ob[0] = (vrt->x >> FRACBITS);
            VTX1[0].v.ob[1] = zpos;
            VTX1[0].v.ob[2] =-(vrt->y >> FRACBITS);
            VTX1[0].v.tc[0] = ((((vrt->x + xpos) >> FRACBITS) - x) << 5);
            VTX1[0].v.tc[1] =-((((vrt->y + ypos) >> FRACBITS) - y) << 5);
            *(int *)VTX1[0].v.cn = color;
            VTX1++;
            leaf++;

            i += 4;
        }
    }*/

    for(i = 0; i < numverts; i++)
    {
        vrt = leaf->vertex;
        VTX1[0].v.ob[0] = (vrt->x >> FRACBITS);
        VTX1[0].v.ob[1] = zpos;
        VTX1[0].v.ob[2] =-(vrt->y >> FRACBITS);
        VTX1[0].v.tc[0] = ((((vrt->x + xpos) >> FRACBITS) - x) << 5);
        VTX1[0].v.tc[1] =-((((vrt->y + ypos) >> FRACBITS) - y) << 5);
        *(int *)VTX1[0].v.cn = color;
        VTX1++;
        leaf++;
    }
}

void R_RenderThings(subsector_t *sub) // 80028248
{
    byte *data;
    byte *src;
    u16 *paldata;
    vissprite_t *vissprite_p;

    mobj_t *thing;
    boolean flip;
    int lump;

    int compressed;
    int tileh;
    int tilew;
    int height;
    int width;
    int tiles;
    int color;
    int nightmare;

    fixed_t xx, yy;
    int xpos1, xpos2;
    int ypos;
    int zpos1, zpos2;
    int spos, tpos;
    int v00, v01, v02, v03;

    vissprite_p = sub->vissprite;
    if (vissprite_p)
    {
        if (vissprite_p->thing->flags & MF_RENDERLASER)
        {
            R_RenderModes(rm_laser);

            do
            {
                I_CheckGFX();
                R_RenderLaser(vissprite_p->thing);

                vissprite_p = vissprite_p->next;
                if(vissprite_p == NULL) {
                    break;
                }

            } while(vissprite_p->thing->flags & MF_RENDERLASER);

        }

        while (vissprite_p)
        {
            I_CheckGFX();

            thing = vissprite_p->thing;
            lump = vissprite_p->lump;
            flip = vissprite_p->flip;

            nightmare = !!(thing->flags & MF_NIGHTMARE);

            if (nightmare)
            {
                R_RenderModes(rm_nightmaresprite);
                color = PACKRGBA(64, 255, 0, 255);
            }
            else
            {
                R_RenderModes(thing->alpha == 255 ? rm_sprite : rm_transparentsprite);
                if (thing->frame & FF_FULLBRIGHT)
                    color = PACKRGBA(255, 255, 255, 0);//0xffffffff;
                else
                    color = lights[vissprite_p->sector->colors[2]].rgba;
            }

            gDPSetPrimColorD64(GFX1++, 0, vissprite_p->sector->lightlevel, thing->alpha);

            data = W_CacheLumpNum(lump, PU_CACHE, dec_jag, sizeof(spriteN64_t));

            compressed = ((spriteN64_t*)data)->compressed;
            tileh = ((spriteN64_t*)data)->tileheight;
            width = ((spriteN64_t*)data)->width;
            height = ((spriteN64_t*)data)->height;
            tiles = ((spriteN64_t*)data)->tiles << 1;

            spos = width;
            tpos = 0;

            src = data + sizeof(spriteN64_t);

            if (flip)
            {
                xx = thing->x + (((spriteN64_t*)data)->xoffs * viewsin);
                xpos1 = (xx - (width * viewsin)) >> FRACBITS;
                xpos2 = (xx) >> FRACBITS;

                yy = thing->y - (((spriteN64_t*)data)->xoffs * viewcos);
                zpos1 = -(yy + (width * viewcos)) >> FRACBITS;
                zpos2 = -(yy) >> FRACBITS;
            }
            else
            {
                xx = thing->x - (((spriteN64_t*)data)->xoffs * viewsin);
                xpos2 = (xx + (width * viewsin)) >> FRACBITS;
                xpos1 = (xx) >> FRACBITS;

                yy = thing->y + (((spriteN64_t*)data)->xoffs * viewcos);
                zpos2 = -(yy - (width * viewcos)) >> FRACBITS;
                zpos1 = -(yy) >> FRACBITS;
            }

            DEBUG_COUNTER(LastVisThings += 1);
            gSPVertex(GFX1++, VTX1, MIN(tiles+2, 32), 0);

            if (compressed < 0)
            {
                width = ALIGN(((spriteN64_t*)data)->width, 8);
                tilew = tileh * width;

                if (lump >= firstplayerlump && lump <= lastplayerlump && thing->player)
                    paldata = playerpalettes[thing->player - players];
                else if (((spriteN64_t*)data)->cmpsize & 1)
                    paldata = W_CacheLumpNum((lump - (((spriteN64_t*)data)->cmpsize >> 1)) +
                                             thing->info->palette, PU_CACHE, dec_jag, 0) + 8;
                else
                    paldata = (u16 *) (src + ((spriteN64_t*)data)->cmpsize);

                // Load Palette Data (256 colors)
                gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1, paldata);

                gDPTileSync(GFX1++);
                gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

                gDPLoadSync(GFX1++);
                gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 255);
            }
            else
            {
                width = ALIGN(((spriteN64_t*)data)->width, 16);
                tilew = tileh * width;

                if (tilew < 0) {
                    tilew = tilew + 1;
                }

                tilew >>= 1;

                register const bool isblood = lump >= bloodlump && lump < bloodlump + 4;
                if (isblood && Settings.GreenBlood)
                    paldata = bloodpalettes[lump - bloodlump][1];
                else if (isblood && thing->extradata)
                    paldata = bloodpalettes[lump - bloodlump][((int) thing->extradata) - 1];
                else if (lump == giblump && thing->extradata)
                    paldata = bloodpalettes[4][((int) thing->extradata) - 1];
                else
                    paldata = (u16 *) (src + ((spriteN64_t*)data)->cmpsize);

                // Load Palette Data (16 colors)
                gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1, paldata);

                gDPTileSync(GFX1++);
                gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

                gDPLoadSync(GFX1++);
                gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 15);
            }

            gDPPipeSync(GFX1++);

            ypos = (thing->z >> FRACBITS) + ((spriteN64_t*)data)->yoffs;

            VTX1[0].v.ob[0] = xpos1;
            VTX1[0].v.ob[1] = ypos;
            VTX1[0].v.ob[2] = zpos1;

            VTX1[1].v.ob[0] = xpos2;
            VTX1[1].v.ob[1] = ypos;
            VTX1[1].v.ob[2] = zpos2;

            VTX1[flip].v.tc[0] = 0;
            VTX1[flip^1].v.tc[0] = (spos << 6);

            VTX1[0].v.tc[1] = VTX1[1].v.tc[1] = (tpos << 6);

            *(int *)VTX1[0].v.cn = *(int *)VTX1[1].v.cn = color;
            VTX1 += 2;

            v03 = 0;
            v00 = 1;
            v01 = 3;
            v02 = 2;

            if (tiles > 0)
            {
                int tilesleft = tiles+2;
                do
                {
                    gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
                    gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);
                    gDPLoadSync(GFX1++);

                    if (compressed < 0)
                    {
                        // Load Image Data (8bit)
                        gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, (tilew >> 1) - 1, 0);
                        gDPPipeSync(GFX1++);
                        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b, (width >> 3), 0,
                                   G_TX_RENDERTILE , 0, 0, 0, 0, 0, 0, 0);
                    }
                    else
                    {
                        // Load Image Data (4bit)
                        gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, (tilew >> 1) - 1, 0);
                        gDPPipeSync(GFX1++);
                        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_4b, (width >> 4), 0,
                                   G_TX_RENDERTILE , 0, 0, 0, 0, 0, 0, 0);
                    }

                    gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, tpos << 2, ((width - 1) << 2), (tpos + tileh - 1) << 2);

                    if (v02 >= 32)
                    {
                        v03 = 0;
                        v00 = 1;
                        v01 = 3;
                        v02 = 2;
                        tilesleft -= 30;
                        gSPVertex(GFX1++, VTX1 - 2, MIN(tilesleft, 32), 0);
                    }

                    tpos += tileh;
                    ypos -= tileh;

                    gSP2Triangles(GFX1++, v00, v01, v02, 0, // 1, 3, 2
                                          v00, v02, v03, 0); // 1, 2, 0
                    DEBUG_COUNTER(LastVisTriangles += 2);

                    VTX1[0].v.ob[0] = xpos1;
                    VTX1[0].v.ob[1] = ypos;
                    VTX1[0].v.ob[2] = zpos1;

                    VTX1[1].v.ob[0] = xpos2;
                    VTX1[1].v.ob[1] = ypos;
                    VTX1[1].v.ob[2] = zpos2;

                    VTX1[flip].v.tc[0] = 0;
                    VTX1[flip^1].v.tc[0] = (spos << 6);

                    VTX1[0].v.tc[1] = VTX1[1].v.tc[1] = (tpos << 6);

                    *(int *)VTX1[0].v.cn = *(int *)VTX1[1].v.cn = color;
                    VTX1 += 2;

                    src += tilew;

                    height -= tileh;
                    if (height < tileh) {
                        tileh = height;
                    }

                    v00 += 2;
                    v01 += 2;
                    v02 += 2;
                    v03 += 2;
                } while (v02 < tilesleft);
            }

            vissprite_p = vissprite_p->next;
        }

        globallump = -1;
    }
}

void R_RenderLaser(mobj_t *thing) // 80028CCC
{
    laserdata_t *laserdata;

    laserdata = (laserdata_t *)thing->extradata;

    gSPVertex(GFX1++, (VTX1), 6, 0);

    gSP2Triangles(GFX1++, 0, 2, 3, 1/*flag1*/,
                          0, 1, 2, 2/*flag2*/);

    gSP2Triangles(GFX1++, 0, 3, 5, 2/*flag1*/,
                          3, 4, 5, 2/*flag2*/);
    DEBUG_COUNTER(LastVisTriangles += 4);

    VTX1[0].v.ob[0] = (laserdata->x1 >> FRACBITS);
    VTX1[0].v.ob[1] = (laserdata->z1 >> FRACBITS);
    VTX1[0].v.ob[2] = -(laserdata->y1 >> FRACBITS);

    VTX1[1].v.ob[0] = ((laserdata->x1 - laserdata->slopey) >> FRACBITS);
    VTX1[1].v.ob[1] = (laserdata->z1 >> FRACBITS);
    VTX1[1].v.ob[2] = (-(laserdata->y1 + laserdata->slopex) >> FRACBITS);

    VTX1[2].v.ob[0] = ((laserdata->x2 - laserdata->slopey) >> FRACBITS);
    VTX1[2].v.ob[1] = (laserdata->z2 >> FRACBITS);
    VTX1[2].v.ob[2] = (-(laserdata->y2 + laserdata->slopex) >> FRACBITS);

    VTX1[3].v.ob[0] = (laserdata->x2 >> FRACBITS);
    VTX1[3].v.ob[1] = (laserdata->z2 >> FRACBITS);
    VTX1[3].v.ob[2] = -(laserdata->y2 >> FRACBITS);

    VTX1[4].v.ob[0] = ((laserdata->x2 + laserdata->slopey) >> FRACBITS);
    VTX1[4].v.ob[1] = (laserdata->z2 >> FRACBITS);
    VTX1[4].v.ob[2] = (-(laserdata->y2 - laserdata->slopex) >> FRACBITS);

    VTX1[5].v.ob[0] = ((laserdata->x1 + laserdata->slopey) >> FRACBITS);
    VTX1[5].v.ob[1] = (laserdata->z1 >> FRACBITS);
    VTX1[5].v.ob[2] = (-(laserdata->y1 - laserdata->slopex) >> FRACBITS);

    *(int *)VTX1[0].v.cn = PACKRGBA(255,0,0,255); // 0xff0000ff;
    *(int *)VTX1[1].v.cn = PACKRGBA(0,0,0,255);   // 0xff;
    *(int *)VTX1[2].v.cn = PACKRGBA(0,0,0,255);   // 0xff;
    *(int *)VTX1[3].v.cn = PACKRGBA(255,0,0,255); // 0xff0000ff;
    *(int *)VTX1[4].v.cn = PACKRGBA(0,0,0,255);   // 0xff;
    *(int *)VTX1[5].v.cn = PACKRGBA(0,0,0,255);   // 0xff;

    VTX1 += 6;
}

void R_RenderPSprites(void) // 80028f20
{
    int             i;
    pspdef_t        *psp, *psptmp;
    state_t         *state;
    spritedef_t     *sprdef;
    spriteframe_t   *sprframe;
    int             lump;
    int             flagtranslucent;

    boolean         palloaded;
    spriteN64_t     *sprite;
    byte            *paldata;
    byte            *src;

    int             tilecnt;
    int             tiles;
    int             tileh;
    int             tilew;
    int             width;
    int             height;
    int             width2;
    int             yh;
    int             x, y;
    int             dsdx, dtdy;
    int             stileh;

    I_CheckGFX();

    gDPPipeSync(GFX1++);
    gDPSetCycleType(GFX1++, G_CYC_2CYCLE);
    gDPSetTexturePersp(GFX1++, G_TP_NONE);
    gDPSetCombineMode(GFX1++, G_CC_D64COMB17, G_CC_D64COMB18);
    if (VideoSettings.ScreenAspect)
        R_RenderFilter(filt_sprites);
    else
        gDPSetTextureFilter(GFX1++, G_TF_POINT);

    psp = &viewplayer->psprites[0];

    flagtranslucent = (viewplayer->mo->flags & MF_SHADOW) != 0;

    psptmp = psp;
    for (i = 0; i < NUMPSPRITES; i++, psptmp++)
    {
        if(flagtranslucent || ((psptmp->state != 0) && (psptmp->alpha < 255)))
        {
            gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_XLU_SURF2_CLAMP);
            goto draw;
        }
    }

    gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_TEX_EDGE2);
draw:
    palloaded = false;

    if (VideoSettings.ScreenAspect)
        dsdx = invaspectscale[VideoSettings.ScreenAspect] >> 4 >> hudxshift;
    else
        dsdx = 1 << 12 >> hudxshift;

    if (osTvType == OS_TV_PAL)
        dtdy = 0xd555 >> 4 >> hudyshift;
    else
        dtdy = 1 << 12 >> hudyshift;

    for (i = 0; i < NUMPSPRITES; i++, psp++)
    {
        if ((state = psp->state) != 0) /* a null state means not active */
        {
            sprdef = &sprites[state->sprite];
            sprframe = &sprdef->spriteframes[state->frame & FF_FRAMEMASK];
            lump = sprframe->lump[0];

            sprite = W_CacheLumpNum(lump, PU_CACHE, dec_jag, sizeof(spriteN64_t));

            tilecnt = 0;
            tiles = sprite->tiles;
            width = sprite->width;
            tileh = sprite->tileheight;
            width2 = ALIGN(width, 8);
            tilew = tileh * width2;
            height = sprite->height;
            src = ((byte*)sprite) + sizeof(spriteN64_t);

            x = (((psp->sx >> FRACBITS) - sprite->xoffs) + SCREEN_WD/2) << hudxshift;
            y = (((psp->sy >> FRACBITS) - sprite->yoffs) + SCREEN_HT - 1) << hudyshift;
            if (viewplayer->onground)
            {
                x += (quakeviewx << hudxshift) >> 22;
                y += (quakeviewy << hudyshift) >> 16;
            }
            if (VideoSettings.ScreenAspect)
            {
                x += (sprite->xoffs - FixedMul(aspectscale[VideoSettings.ScreenAspect], sprite->xoffs)) << hudxshift;
                width = FixedMul(aspectscale[VideoSettings.ScreenAspect], width);
            }
            if (osTvType == OS_TV_PAL)
            {
                y += (sprite->yoffs - FixedMul(0x13333, sprite->yoffs)) << hudyshift;
                height = FixedMul(0x13333, height);
                stileh = (0x13333 * tileh) >> (FRACBITS-hudyshift);
            }
            else
            {
                stileh = tileh << hudyshift;
            }

            if (psp->state->frame & FF_FULLBRIGHT)
            {
                gDPSetPrimColorD64(GFX1, 0, 0, PACKRGBA(255,255,255,0));//0xffffff00
            }
            else
            {
                gDPSetPrimColorD64(GFX1, 0, frontsector->lightlevel,
                          lights[frontsector->colors[2]].rgba & ~255); // remove alpha value
            }

            // apply alpha value
            if (flagtranslucent)
            {
                GFX1->words.w1 |= 144;
            }
            else
            {
                GFX1->words.w1 |= psp->alpha;
            }
            GFX1++; // continue to next GFX1

            if (!palloaded)
            {
                palloaded = true;

                if (sprite->cmpsize & 1)
                {
                /* Loads the palette from the first frame of the animation,  */
                /* which uses an odd number to get to the lump */
                    paldata = W_CacheLumpNum((lump - (sprite->cmpsize >> 1)),
                                             PU_CACHE, dec_jag, sizeof(spriteN64_t));

                    paldata += (((spriteN64_t*)paldata)->cmpsize + sizeof(spriteN64_t));
                }
                else
                {
                /* Loads the palette if it is included in the image data */
                    paldata = (src + sprite->cmpsize);
                }

                /* Load Palette Data (256 colors) */
                gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b , 1, paldata);

                gDPTileSync(GFX1++);
                gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

                gDPLoadSync(GFX1++);
                gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 255);

                gDPPipeSync(GFX1++);
            }

            if (tiles > 0)
            {
                int xh = x + (width << hudxshift);

                do
                {
                    /* Load Image Data (8bit) */
                    gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b , 1, src);
                    gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_16b, 0, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

                    gDPLoadSync(GFX1++);
                    gDPLoadBlock(GFX1++, G_TX_LOADTILE, 0, 0, ((width2 * tileh + 1) >> 1) - 1, 0);

                    gDPPipeSync(GFX1++);
                    gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b, ((width2 + 7) >> 3), 0,
                                   G_TX_RENDERTILE , 0, 0, 0, 0, 0, 0, 0);

                    gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, 0, ((width2 - 1) << 2), (tileh - 1) << 2);

                    yh = stileh + y;

                    gSPTextureRectangle(GFX1++, x, y, xh, yh, G_TX_RENDERTILE,
                                        0, 0, dsdx, dtdy);

                    height -= tileh;
                    if (height < tileh) {
                        tileh = height;
                    }

                    y = yh;

                    src += tilew;
                    tilecnt += 1;
                } while (tilecnt != (int)tiles);
            }
        }
    }
}
