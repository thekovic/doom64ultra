/* R_data.c */

#include "doomdef.h"
#include "r_local.h"

SDATA int			firsttex;				// 800A632C
int			lasttex;				// 800A6330
int			numtextures;			// 800A6334
int			firstswx;				// 800A6338
SDATA int    	    *textures;				// 800A633C

int			firstspritelump;			// 800A6320
int			lastspritelump;				// 800A6324
int			numspritelumps;				// 800A6328

int	        skytexture;             // 800A5f14

spritedef_t		sprites[NUMSPRITES];

int bloodlump;
int giblump;
u16 bloodpalettes[5][3][16];

void R_InitTextures(void);
void R_InitSprites(void);
void R_InitBloodPalette(int lump, u16 palettes[static 3][16]);
/*============================================================================ */

#define PI_VAL 3.141592653589793

/*
================
=
= R_InitData
=
= Locates all the lumps that will be used by all views
= Must be called after W_Init
=================
*/

void R_InitData (void) // 80023180
{
/*    int i;
    int val;

    for(i = 0; i < (5*FINEANGLES/4); i++)
    {
        finesine(i) = (fixed_t) (sinf((((f64) val * (f64) PI_VAL) / 8192.0)) * 65536.0);
        val += 2;
    }*/

    R_InitTextures();
    R_InitSprites();

    bloodlump = W_GetNumForName("BLUDA0");
    for (int i = 0; i < 4; i++)
        R_InitBloodPalette(bloodlump + i, bloodpalettes[i]);

    giblump = W_GetNumForName("A027A0");
    R_InitBloodPalette(giblump, bloodpalettes[4]);
}

/*
==================
=
= R_InitTextures
=
= Initializes the texture list with the textures from the world map
=
==================
*/

void R_InitTextures(void) // 8002327C
{
	int swx, i;

	firsttex = W_GetNumForName("T_START") + 1;
	lasttex = W_GetNumForName("T_END") - 1;
	numtextures = (lasttex - firsttex) + 1;

	textures = Z_Malloc(numtextures * sizeof(int), PU_STATIC, NULL);

	for (i = 0; i < numtextures; i++)
	{
	    textures[i] = (i + firsttex) << 4;
	}

    swx = W_CheckNumForName("SWX", 0x7fffff00, 0);
    firstswx = (swx - firsttex);
}

static void
R_InstallSpriteLump
( int			lump,
  unsigned		frame,
  unsigned		rotation,
  boolean		flipped,
  spriteframe_t*	sprtemp,
  int*			maxframe,
  char*			spritename)
{
    int		r;

    if (frame >= 29 || rotation > 8)
	I_Error("R_InstallSpriteLump: "
		"Bad frame characters in lump %i", lump);

    if ((int)frame > *maxframe)
	*maxframe = frame;

    if (rotation == 0)
    {
	// the lump should be used for all rotations
	if (sprtemp[frame].rotate == false)
	    I_Error ("R_InitSprites: Sprite %s frame %c has "
		     "multip rot=0 lump", spritename, 'A'+frame);

	if (sprtemp[frame].rotate == true)
	    I_Error ("R_InitSprites: Sprite %s frame %c has rotations "
		     "and a rot=0 lump", spritename, 'A'+frame);

	sprtemp[frame].rotate = false;
	for (r=0 ; r<8 ; r++)
	{
	    sprtemp[frame].lump[r] = lump - firstspritelump + 1;
	    sprtemp[frame].flip[r] = (byte)flipped;
	}
	return;
    }

    // the lump is only used for one rotation
    if (sprtemp[frame].rotate == false)
	I_Error ("R_InitSprites: Sprite %s frame %c has rotations "
		 "and a rot=0 lump", spritename, 'A'+frame);

    sprtemp[frame].rotate = true;

    // make 0 based
    rotation--;
    if (sprtemp[frame].lump[rotation] != -1)
	I_Error ("R_InitSprites: Sprite %s : %c : %c "
		 "has two lumps mapped to it",
		 spritename, 'A'+frame, '1'+rotation);

    sprtemp[frame].lump[rotation] = lump - firstspritelump + 1;
    sprtemp[frame].flip[rotation] = (byte)flipped;
}

//
// R_InitSpriteDefs
// Pass a null terminated list of sprite names
//  (4 chars exactly) to be used.
// Builds the sprite rotation matrixes to account
//  for horizontally flipped sprites.
// Will report an error if the lumps are inconsistant.
// Only called at startup.
//
// Sprite lump names are 4 characters for the actor,
//  a letter for the frame, and a number for the rotation.
// A sprite that is flippable will have an additional
//  letter/number appended.
// The rotation character can be 0 to signify no rotations.
//
static void
R_InitSpriteDefs (char** namelist)
{
    int		i;
    int		l;
    int		intname;
    int		frame;
    int		rotation;
    int		start;
    int		end;
    spriteframe_t	sprtemp[29];
    int		maxframe;
    char*		spritename;


    start = firstspritelump-1;
    end = lastspritelump+1;

    // scan all the lump names for each of the names,
    //  noting the highest frame letter.
    // Just compare 4 characters as ints
    for (i=0 ; i<NUMSPRITES ; i++)
    {
	spritename = namelist[i];
	D_memset (sprtemp,-1, sizeof(sprtemp));

	maxframe = -1;
	intname = *(int *)namelist[i];

	// scan the lumps,
	//  filling in the frames for whatever is found
	for (l=start+1 ; l<end ; l++)
	{
	    if ((*(int *)lumpinfo[l].name & 0x7fffffff) == intname)
	    {
		frame = lumpinfo[l].name[4] - 'A';
		rotation = lumpinfo[l].name[5] - '0';

		R_InstallSpriteLump (l, frame, rotation, false, sprtemp, &maxframe, spritename);

		if (lumpinfo[l].name[6])
		{
		    frame = lumpinfo[l].name[6] - 'A';
		    rotation = lumpinfo[l].name[7] - '0';
		    R_InstallSpriteLump (l, frame, rotation, true, sprtemp, &maxframe, spritename);
		}
	    }
	}

	// check the frames that were found for completeness
	if (maxframe == -1)
	{
	    sprites[i].numframes = 0;
	    continue;
	}

	maxframe++;

	for (frame = 0 ; frame < maxframe ; frame++)
	{
	    switch ((int)sprtemp[frame].rotate)
	    {
	      case -1:
		// no rotations were found for that frame at all
		I_Error ("R_InitSprites: No patches found "
			 "for %s frame %c", namelist[i], frame+'A');
		break;

	      case 0:
		// only the first rotation is needed
		break;

	      case 1:
		// must have all 8 frames
		for (rotation=0 ; rotation<8 ; rotation++)
		    if (sprtemp[frame].lump[rotation] == -1)
			I_Error ("R_InitSprites: Sprite %s frame %c "
				 "is missing rotations",
				 namelist[i], frame+'A');
		break;
	    }
	}

	// allocate space for the frames present and copy sprtemp to it
	sprites[i].numframes = maxframe;
	sprites[i].spriteframes =
	    Z_Malloc (maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
	D_memcpy (sprites[i].spriteframes, sprtemp, maxframe*sizeof(spriteframe_t));
    }

}
/*
================
=
= R_InitSprites
=
=================
*/

void R_InitSprites(void) // 80023378
{
	firstspritelump = W_GetNumForName("S_START") + 1;
	lastspritelump = W_GetNumForName("S_END") - 1;
	numspritelumps = (lastspritelump - firstspritelump) + 1;
	R_InitSpriteDefs(sprnames);
}

void R_PaletteChangeHSV(u16 *dest, const u16 *src, int h, int s, int v)
{
    u32 c;
    int hsv;

    for (int i = 0; i < 16; i++) {
        c = src[i];
        if (c & 1)
        {
            hsv = LightGetHSV((c&0xf800)>>8, (c&0x7c0)>>3, (c&0x3e)<<2);
            c = (LightGetRGB(((hsv >> 16) + h) & 0xff,
                             CLAMP(((hsv >> 8) & 0xff) + s, 0, 255),
                             CLAMP((hsv & 0xff) + v, 0, 255)) << 8) | 0xff;
            dest[i] = RGBATO5551(c);
        }
        else
        {
            dest[i] = 0;
        }
    }
}

void R_InitBloodPalette(int lump, u16 palettes[static 3][16])
{
    spriteN64_t *sprite;
    u16 *pal;

    sprite = W_GetLump(lump, dec_jag, -1);
    pal = (u16*)(((byte*)sprite) + sizeof(spriteN64_t) + sprite->cmpsize);

    R_PaletteChangeHSV(palettes[0], pal, 11, 0, 0);
    R_PaletteChangeHSV(palettes[1], pal, 85, 0, 0);
    R_PaletteChangeHSV(palettes[2], pal, 192, -128, -16);

    Z_Free(sprite);
}
