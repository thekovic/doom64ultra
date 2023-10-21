/* r_main.c */

#include "doomdef.h"
#include "r_local.h"

/*===================================== */

/* */
/* subsectors */
/* */
//subsector_t       *vissubsectors[MAXVISSSEC], **lastvissubsector;

/* */
/* walls */
/* */
//viswall_t viswalls[MAXWALLCMDS], *lastwallcmd;

/* */
/* planes */
/* */
//visplane_t    visplanes[MAXVISPLANES], *lastvisplane;

/* */
/* sprites */
/* */
//vissprite_t   vissprites[MAXVISSPRITES], *lastsprite_p, *vissprite_p;

/* */
/* openings / misc refresh memory */
/* */
//unsigned short    openings[MAXOPENINGS], *lastopening;


/*===================================== */

SDATA fixed_t       viewx, viewy, viewz;    // 800A6890, 800A6894, 800A6898
angle_t     viewangle;              // 800A689C
SDATA fixed_t       viewcos, viewsin;       // 800A68A0,
SDATA angle_t       viewpitch;
SDATA fixed_t       viewpitchsin, viewpitchcos;
SDATA angle_t       viewmaxhalffov;
fixed_t     viewhcot, viewvcot; // cotangents of the horizontal and vertical fov half angles
fixed_t     viewinvhcot, viewinvvcot;
player_t    *viewplayer;            // 800A688C, 800a68a4

SDATA int           validcount;     /* increment every time a check is made */ // 800A6900
//int           framecount;         /* incremented every frame */

/* */
/* sky mapping */
/* */
boolean     rendersky = false; // 800A68A8

subsector_t *solidsubsectors[MAXSUBSECTORS];    // 800A6488  /* List of valid ranges to scan through */
SDATA subsector_t **endsubsector;                       // 800A6888    /* Pointer to the first free entry */
SDATA int numdrawsubsectors;                          // 800A68AC

vissprite_t vissprites[MAXVISSPRITES];          // 800A6908
SDATA vissprite_t   *visspritehead;                     // 800A8108
SDATA int numdrawvissprites;                          // 800A68B0

SDATA int globallump;                                 // 800A68f8
SDATA int globalcm;                                   // 800A68FC

Mtx R_ProjectionMatrix = { 0, };                     // 800A68B8
/*Mtx R_ProjectionMatrix =                          // 800A68B8
{
    0x00010000, 0x00000000,
    0x00000001, 0x00000000,
    0x00000000, 0xfffeffff,
    0x00000000, 0xffef0000,
    0x00000000, 0x00000000,
    0x00005555, 0x00000000,
    0x00000000, 0xfeed0000,
    0x00000000, 0xf7610000
};*/

Mtx R_ModelMatrix =                             // 8005b0C8
{ .m = {
    {0x00010000,    0x00000000,
     0x00000001,    0x00000000},
    {0x00000000,    0x00010000,
     0x00000000,    0x00000001},
    {0x00000000,    0x00000000,
     0x00000000,    0x00000000},
    {0x00000000,    0x00000000,
     0x00000000,    0x00000000}
} };

/* */
/* precalculated math */
/* */
//fixed_t*    finecosine = &finesine(FINEANGLES / 4); // 8005B890

int         infraredFactor; // 800A810C
int         FlashEnvColor;  // 800A8110
fixed_t     quakeviewx;     // 800A8118
fixed_t     quakeviewy;     // 800A8114
mobj_t      *cameratarget;  // 800A5D70
angle_t     camviewpitch;   // 800A811C

SDATA fixed_t     scrollfrac;     // 800A812C
SDATA sector_t    *frontsector; // 800A6340

/*============================================================================= */

/*
==============
=
= R_Init
=
==============
*/

void R_Init(void) // 800233E0
{
    R_InitData();
    guFrustum(&R_ProjectionMatrix, -8.0f, 8.0f, -6.0f, 6.0f, 8.0f, 3808.0f, 1.0f);

    /*D_printf("R_ProjectionMatrix[0][0] %x\n", R_ProjectionMatrix.m[0][0]);
    D_printf("R_ProjectionMatrix[0][1] %x\n", R_ProjectionMatrix.m[0][1]);
    D_printf("R_ProjectionMatrix[0][2] %x\n", R_ProjectionMatrix.m[0][2]);
    D_printf("R_ProjectionMatrix[0][3] %x\n", R_ProjectionMatrix.m[0][3]);
    D_printf("R_ProjectionMatrix[1][0] %x\n", R_ProjectionMatrix.m[1][0]);
    D_printf("R_ProjectionMatrix[1][1] %x\n", R_ProjectionMatrix.m[1][1]);
    D_printf("R_ProjectionMatrix[1][2] %x\n", R_ProjectionMatrix.m[1][2]);
    D_printf("R_ProjectionMatrix[1][3] %x\n", R_ProjectionMatrix.m[1][3]);
    D_printf("R_ProjectionMatrix[2][0] %x\n", R_ProjectionMatrix.m[2][0]);
    D_printf("R_ProjectionMatrix[2][1] %x\n", R_ProjectionMatrix.m[2][1]);
    D_printf("R_ProjectionMatrix[2][2] %x\n", R_ProjectionMatrix.m[2][2]);
    D_printf("R_ProjectionMatrix[2][3] %x\n", R_ProjectionMatrix.m[2][3]);
    D_printf("R_ProjectionMatrix[3][0] %x\n", R_ProjectionMatrix.m[3][0]);
    D_printf("R_ProjectionMatrix[3][1] %x\n", R_ProjectionMatrix.m[3][1]);
    D_printf("R_ProjectionMatrix[3][2] %x\n", R_ProjectionMatrix.m[3][2]);
    D_printf("R_ProjectionMatrix[3][3] %x\n", R_ProjectionMatrix.m[3][3]);
    while(1){}*/
}

void R_RotateCameraMatrix(void)
{
    fixed_t sin, cos;

    sin = viewpitchsin;
    cos = viewpitchcos;

    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(MTX1), G_MTX_MODELVIEW| G_MTX_LOAD | G_MTX_NOPUSH);
    MTX1->m[0][0] = 0x10000;
    MTX1->m[0][1] = 0;
    MTX1->m[0][2] = ((cos & 0xffff0000) >> 16);
    MTX1->m[0][3] = ((-sin) & 0xffff0000);
    MTX1->m[1][0] = ((sin & 0xffff0000) >> 16);
    MTX1->m[1][1] = (cos & 0xffff0000);
    MTX1->m[1][2] = 0;
    MTX1->m[1][3] = 1;
    MTX1->m[2][0] = 0;
    MTX1->m[2][1] = 0;
    MTX1->m[2][2] = (cos & 0xffff);
    MTX1->m[2][3] = (((-sin) << 16) & 0xffff0000);
    MTX1->m[3][0] = (sin & 0xffff);
    MTX1->m[3][1] = ((cos << 16) & 0xffff0000);
    MTX1->m[3][2] = 0;
    MTX1->m[3][3] = 0;
    MTX1++;

    sin = viewsin;
    cos = viewcos;

    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(MTX1), G_MTX_MODELVIEW| G_MTX_MUL | G_MTX_NOPUSH);
    MTX1->m[0][0] = (sin & 0xffff0000);
    MTX1->m[0][1] = ((-cos) & 0xffff0000);
    MTX1->m[0][2] = 1;
    MTX1->m[0][3] = 0;
    MTX1->m[1][0] = (cos & 0xffff0000);
    MTX1->m[1][1] = (sin & 0xffff0000);
    MTX1->m[1][2] = 0;
    MTX1->m[1][3] = 1;
    MTX1->m[2][0] = ((sin << 16) & 0xffff0000);
    MTX1->m[2][1] = (((-cos) << 16) & 0xffff0000);
    MTX1->m[2][2] = 0;
    MTX1->m[2][3] = 0;
    MTX1->m[3][0] = ((cos << 16) & 0xffff0000);
    MTX1->m[3][1] = ((sin << 16) & 0xffff0000);
    MTX1->m[3][2] = 0;
    MTX1->m[3][3] = 0;
    MTX1++;

}

#define VFOV (0x346feb89) // atan(3/4)*2
const fixed_t aspectscale[3] = { FRACUNIT, 0xd555, 0xc000 }; // 1, 5/6, 3/4
const fixed_t invaspectscale[3] = { FRACUNIT, 0x13333, 0x15555 }; // 1, 6/5, 4/3
const fixed_t aspectratios[3] = { 0x15555, 0x19999, 0x1c71c }; // 4/3, 16/10, 16/9
const angle_t aspectfovs[3] = { ANG90, 0x47633bdd, 0x4b901476 }; // 90deg, 100.38deg, 106.26deg

/*
==============
=
= R_RenderView
=
==============
*/

void R_RenderPlayerView(void) // 80023448
{
    fixed_t Fnear, FnearA, FnearB;
    int finepitch, fineangle;
    angle_t fov;
    int fog;

    viewplayer = &players[0];

    fov = aspectfovs[VideoSettings.ScreenAspect];
    viewhcot = aspectscale[VideoSettings.ScreenAspect];
    viewvcot = aspectratios[0];
    viewinvhcot = invaspectscale[VideoSettings.ScreenAspect];
    viewinvvcot = 0xc000; // 3/4

    if (cameratarget == players[0].mo)
    {
        viewz = players[0].viewz;
        viewpitch = players[0].recoilpitch + players[0].pitch;
        finepitch = viewpitch >> ANGLETOFINESHIFT;

        if (players[0].addfov)
        {
            angle_t finehalffov;

            fov = aspectfovs[VideoSettings.ScreenAspect] + players[0].addfov;
            finehalffov = fov >> (1+ANGLETOFINESHIFT);
            viewhcot = FixedDiv(finecosine(finehalffov), finesine(finehalffov));
            viewvcot = FixedMul(viewhcot, aspectratios[VideoSettings.ScreenAspect]);
            viewinvhcot = FixedDiv(FRACUNIT, viewhcot);
            viewinvvcot = FixedDiv(FRACUNIT, viewvcot);
        }
    }
    else
    {
        viewz = cameratarget->z;
        viewpitch = camviewpitch;
        finepitch = camviewpitch >> ANGLETOFINESHIFT;
    }

    viewx = cameratarget->x;
    viewy = cameratarget->y;
    viewz += quakeviewy;

    viewangle = cameratarget->angle + quakeviewx;
    fineangle = viewangle >> ANGLETOFINESHIFT;
    viewsin = finesine(fineangle);
    viewcos = finecosine(fineangle);
    viewpitchsin = finesine(finepitch);
    viewpitchcos = finecosine(finepitch);

    R_ProjectionMatrix.m[0][0] = viewhcot & 0xffff0000;
    R_ProjectionMatrix.m[2][0] = (viewhcot << 16) & 0xffff0000;
    R_ProjectionMatrix.m[0][2] = (viewvcot >> 16) & 0xffff;
    R_ProjectionMatrix.m[2][2] = viewvcot & 0xffff;

    if (viewpitch == 0)
    {
        viewmaxhalffov = fov >> 1;
    }
    else
    {
        fixed_t corner;

        /* rotate the closest corner of the frustum into into world space and get its angle */
        corner = FixedMul(-D_abs(viewpitchsin), viewinvvcot) + viewpitchcos;
        viewmaxhalffov = R_PointToAngle2(0, 0, corner, viewinvhcot);

        if (viewmaxhalffov >= ANG90)
            viewmaxhalffov = 0;
    }

    // Phase 1
    R_BSP();

    gDPSetEnvColorD64(GFX1++, FlashEnvColor);

    R_RenderModes(rm_reset);
    gDPSetCycleType(GFX1++, G_CYC_2CYCLE);

    // Phase 2
    if (rendersky)
    {
        R_RenderFilter(filt_skies);
        R_RenderSKY();
    }

    gDPSetTextureLOD(GFX1++, G_TL_TILE);
    gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
    gDPSetTexturePersp(GFX1++, G_TP_PERSP);
    gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);
    gDPSetBlendColor(GFX1++, 0, 0, 0, 0);

    if (!(players[0].cheats & CF_FULLBRIGHT))
    {
        FnearA = (1000 - FogNear);
        FnearB = ((0-FogNear) << 8) + 128000;
        Fnear  = (((128000 / FnearA) << 16) | ((FnearB / FnearA) & 0xffff));
        gMoveWd(GFX1++, G_MW_FOG, G_MWO_FOG, Fnear);
    }
    else
    {
        gSPFogPosition(GFX1++, 996, 1000);
    }

    // Apply Fog Color
    fog = FogColor;
    if (!(viewplayer->powers[pw_invulnerability] >= 61 || viewplayer->powers[pw_invulnerability] & 8))
        fog = C_AddColors(fog, FlashEnvColor);
    gDPSetFogColorD64(GFX1++, fog);

    R_RotateCameraMatrix();

    gSPMatrix(GFX1++, OS_K0_TO_PHYSICAL(MTX1), G_MTX_MODELVIEW| G_MTX_MUL | G_MTX_NOPUSH);
    MTX1->m[0][0] = 0x10000;
    MTX1->m[0][1] = 0;
    MTX1->m[0][2] = 1;
    MTX1->m[0][3] = 0;
    MTX1->m[1][0] = 0;
    MTX1->m[1][1] = 0x10000;
    MTX1->m[1][2] = ((-viewx) & 0xffff0000) | (((-viewz) >> 16) & 0xffff);
    MTX1->m[1][3] = (viewy & 0xffff0000) | 1;
    MTX1->m[2][0] = 0;
    MTX1->m[2][1] = 0;
    MTX1->m[2][2] = 0;
    MTX1->m[2][3] = 0;
    MTX1->m[3][0] = 0;
    MTX1->m[3][1] = 0;
    MTX1->m[3][2] = (((-viewx) << 16) & 0xffff0000) | ((-viewz) & 0xffff);
    MTX1->m[3][3] = ((viewy << 16) & 0xffff0000);
    MTX1++;

    // Phase 3
    R_RenderAll();

    if (cameratarget == viewplayer->mo)
        R_RenderPSprites();
}

/*============================================================================= */

/*
===============================================================================
=
= R_PointOnSide
=
= Returns side 0 (front) or 1 (back)
===============================================================================
*/
int R_PointOnSide(int x, int y, node_t *node) // 80023B6C
{
    fixed_t dx, dy;
    fixed_t left, right;

    if (!node->line.dx)
    {
        if (x <= node->line.x)
            return (node->line.dy > 0);
        return (node->line.dy < 0);
    }
    if (!node->line.dy)
    {
        if (y <= node->line.y)
            return (node->line.dx < 0);
        return (node->line.dx > 0);
    }

    dx = (x - node->line.x);
    dy = (y - node->line.y);

    left = (node->line.dy >> 16) * (dx >> 16);
    right = (dy >> 16) * (node->line.dx >> 16);

    if (right < left)
        return 0;       /* front side */
    return 1;           /* back side */
}

/*
==============
=
= R_PointInSubsector
=
==============
*/

struct subsector_s *R_PointInSubsector(fixed_t x, fixed_t y) // 80023C44
{
    node_t  *node;
    int     side, nodenum;

    if (!numnodes)              /* single subsector is a special case */
        return subsectors;

    nodenum = numnodes - 1;

    while (!(nodenum & NF_SUBSECTOR))
    {
        node = &nodes[nodenum];
        side = R_PointOnSide(x, y, node);
        nodenum = node->children[side];
    }

    return &subsectors[nodenum & ~NF_SUBSECTOR];
}

/*
===============================================================================
=
= R_PointToAngle
=
===============================================================================
*/

//extern    angle_t tantoangle(SLOPERANGE + 1);

static int SlopeDiv(unsigned num, unsigned den) // 80023D10
{
    unsigned ans;
    if (den < 512)
        return SLOPERANGE;
    ans = (num << 3) / (den >> 8);
    return ans <= SLOPERANGE ? ans : SLOPERANGE;
}

angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2) // 80023D60
{
    int     x;
    int     y;

    x = x2 - x1;
    y = y2 - y1;

    if ((!x) && (!y))
        return 0;

    if (x >= 0)
    {   /* x >=0 */
        if (y >= 0)
        {   /* y>= 0 */
            if (x>y)
                return tantoangle(SlopeDiv(y, x));     /* octant 0 */
            else
                return ANG90 - 1 - tantoangle(SlopeDiv(x, y));  /* octant 1 */
        }
        else
        {   /* y<0 */
            y = -y;
            if (x>y)
                return -tantoangle(SlopeDiv(y, x));  /* octant 8 */
            else
                return ANG270 + tantoangle(SlopeDiv(x, y));  /* octant 7 */
        }
    }
    else
    {   /* x<0 */
        x = -x;
        if (y >= 0)
        {   /* y>= 0 */
            if (x>y)
                return ANG180 - 1 - tantoangle(SlopeDiv(y, x)); /* octant 3 */
            else
                return ANG90 + tantoangle(SlopeDiv(x, y));  /* octant 2 */
        }
        else
        {   /* y<0 */
            y = -y;
            if (x>y)
                return ANG180 + tantoangle(SlopeDiv(y, x));  /* octant 4 */
            else
                return ANG270 - 1 - tantoangle(SlopeDiv(x, y));  /* octant 5 */
        }
    }
}

SDATA static int cur_filter = -1;
// [GEC and Immorpher] Set texture render options
void R_RenderFilter(filtertype_t type)
{
    int filter;

    if (Settings.VideoFilters[type] == 0) {
        filter = G_TF_BILERP;
    } else {
        filter = G_TF_POINT;
    }

    if (filter == cur_filter)
        return;

    cur_filter = filter;
    gDPSetTextureFilter(GFX1++, filter);
}

SDATA static rendermode_t lastrender = rm_nothing;

void R_RenderModes(rendermode_t mode)
{
    if (lastrender == mode)
        return;

    gDPPipeSync(GFX1++);

    if (mode == rm_reset)
    {
        cur_filter = -1;
        if (Settings.ColorDither == 1) {
            gDPSetColorDither(GFX1++, G_CD_MAGICSQ);
        } else if (Settings.ColorDither == 2) {
            gDPSetColorDither(GFX1++, G_CD_BAYER);
        } else if (Settings.ColorDither == 3) {
            gDPSetColorDither(GFX1++, G_CD_NOISE);
        } else {
            gDPSetColorDither(GFX1++, G_CD_DISABLE);
        }
    }
    else if (mode == rm_texture)
    {
        if (lastrender != rm_liquid) {
            gSPTexture(GFX1++, (1024 << 6)-1, (1024 << 6)-1, 0, G_TX_RENDERTILE, G_ON);
            if (lastrender != rm_switch) {
                gDPSetCombineMode(GFX1++, G_CC_D64COMB07, G_CC_D64COMB08);
            }
        }
        gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A,
                (VideoSettings.AntiAliasing ? (G_RM_AA_XLU_SURF2) : (G_RM_TEX_EDGE2)));
        R_RenderFilter(filt_textures);
    }
    else if (mode == rm_switch)
    {
        gSPTexture(GFX1++, (512 << 6), (512 << 6), 0, G_TX_RENDERTILE, G_ON);
        if (lastrender != rm_texture && lastrender != rm_liquid) {
            gDPSetCombineMode(GFX1++, G_CC_D64COMB07, G_CC_D64COMB08);
        }
        gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A,
                (VideoSettings.AntiAliasing ? (G_RM_AA_OPA_SURF2) : (G_RM_OPA_SURF2)));
        R_RenderFilter(filt_textures);
    }
    else if (mode == rm_liquid)
    {
        if (lastrender != rm_texture) {
            gSPTexture(GFX1++, (1024 << 6)-1, (1024 << 6)-1, 0, G_TX_RENDERTILE, G_ON);
            if (lastrender != rm_switch) {
                gDPSetCombineMode(GFX1++, G_CC_D64COMB07, G_CC_D64COMB08);
            }
        }
        gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_XLU_SURF2);
        R_RenderFilter(filt_textures);
    }
    else if (mode == rm_laser)
    {
        gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_RA_OPA_SURF2);
        gDPSetCombineMode(GFX1++, G_CC_D64COMB15, G_CC_D64COMB16);
        R_RenderFilter(filt_sprites);
    }
    else if (mode == rm_sprite)
    {
#define G_CC_D64COMB07_NOPRIM 1, 0, PRIM_LOD_FRAC, TEXEL0, 1, 0, TEXEL0, 0
        gDPSetCombineMode(GFX1++, G_CC_D64COMB07_NOPRIM, G_CC_D64COMB08);
        if (lastrender != rm_switch) {
            gSPTexture(GFX1++, (512 << 6), (512 << 6), 0, G_TX_RENDERTILE, G_ON);
        }
        if (lastrender != rm_transparentsprite)
        {
            gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_XLU_SURF2_CLAMP);
        }
        R_RenderFilter(filt_sprites);
    }
    else if (mode == rm_transparentsprite)
    {
        if (lastrender != rm_nightmaresprite) {
            gDPSetCombineMode(GFX1++, G_CC_D64COMB07, G_CC_D64COMB08);
            if (lastrender != rm_switch) {
                gSPTexture(GFX1++, (512 << 6), (512 << 6), 0, G_TX_RENDERTILE, G_ON);
            }
        }
        if (lastrender != rm_sprite)
        {
            gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_XLU_SURF2_CLAMP);
        }
        R_RenderFilter(filt_sprites);
    }
    else if (mode == rm_nightmaresprite)
    {
        if (lastrender != rm_transparentsprite) {
            gDPSetCombineMode(GFX1++, G_CC_D64COMB07, G_CC_D64COMB08);
            if (lastrender != rm_switch) {
                gSPTexture(GFX1++, (512 << 6), (512 << 6), 0, G_TX_RENDERTILE, G_ON);
            }
        }
        gDPSetRenderMode(GFX1++, G_RM_FOG_SHADE_A, G_RM_XLU_SURF2_ADD);
        R_RenderFilter(filt_sprites);
    }
    else if (mode == rm_hudsprite)
    {
        gDPSetCycleType(GFX1++, G_CYC_1CYCLE);
        gDPSetTexturePersp(GFX1++, G_TP_NONE);
        gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
        gDPSetAlphaCompare(GFX1++, G_AC_NONE);
        gDPSetBlendColor(GFX1++, 0, 0, 0, 0);
        gDPSetCombineMode(GFX1++, G_CC_D64COMB04, G_CC_D64COMB04);
        R_RenderFilter(filt_sprites);
    }
    else if (mode == rm_hudtext)
    {
        gDPSetCycleType(GFX1++, G_CYC_1CYCLE);
        gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
        gDPSetTexturePersp(GFX1++, G_TP_NONE);
        gDPSetAlphaCompare(GFX1++, G_AC_NONE);
        gDPSetBlendColor(GFX1++, 0, 0, 0, 0);
        gDPSetCombineMode(GFX1++, G_CC_D64COMB04, G_CC_D64COMB04);
        gDPSetRenderMode(GFX1++, G_RM_XLU_SURF_CLAMP, G_RM_XLU_SURF2_CLAMP);
        gDPSetTextureFilter(GFX1++, G_TF_POINT);
    }
    else if (mode == rm_hudoverlay)
    {
        gDPSetCycleType(GFX1++, G_CYC_1CYCLE);
        gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
        gDPSetTexturePersp(GFX1++, G_TP_NONE);
        gDPSetAlphaCompare(GFX1++, G_AC_NONE);
        gDPSetBlendColor(GFX1++, 0, 0, 0, 0);
        gDPSetCombineMode(GFX1++, G_CC_D64COMB05, G_CC_D64COMB05);
        gDPSetRenderMode(GFX1++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    }
    else if (mode == rm_background)
    {
        gDPSetCycleType(GFX1++, G_CYC_1CYCLE);
        gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
        gDPSetTexturePersp(GFX1++, G_TP_NONE);
        gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);
        gDPSetBlendColor(GFX1++, 0, 0, 0, 0);
        gDPSetCombineMode(GFX1++, G_CC_D64COMB03, G_CC_D64COMB03);
    }

    lastrender = mode;
}
