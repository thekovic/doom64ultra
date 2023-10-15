/* W_wad.c */

#include "doomdef.h"
#include "config.h"
#include "i_main.h"
#ifdef USB
#include "i_usb.h"
#endif
//#include "r_local.h"

//char str[64];

/*=============== */
/*   TYPES */
/*=============== */


typedef struct
{
    char        identification[4];      /* should be IWAD */
    int         numlumps;
    int         infotableofs;
} wadinfo_t;

/*============= */
/* GLOBALS */
/*============= */

static lumpcache_t  *lumpcache;             //800B2220
static int          numlumps;               //800B2224
lumpinfo_t  *lumpinfo;              //800B2228 /* points directly to rom image */

static int          mapnumlumps;            //800B2230 psxdoom/doom64
static lumpinfo_t   *maplump;               //800B2234 psxdoom/doom64
static byte         *mapfileptr;            //800B2238 psxdoom/doom64
static u32           maplumppos;

static int maxcompressedsize = 0;
static byte *decompressbuf;

/*=========*/
/* EXTERNS */
/*=========*/

extern OSMesgQueue romcopy_msgque;

void AllocDecodeBuffers(void);

/*
============================================================================

                        LUMP BASED ROUTINES

============================================================================
*/

static void W_GetRomData(u32 offset, void *dest, u32 len, u32 usable)
{
    OSIoMesg romio_msgbuf;

    osInvalDCache(dest, usable);

    DEBUG_COUNTER(u32 dma_cycles);
    DEBUG_CYCLES_START(dma_start);

    osPiStartDma(&romio_msgbuf, OS_MESG_PRI_NORMAL, OS_READ,
            (u32)_doom64_wadSegmentRomStart + offset,
             dest, len, &romcopy_msgque);

    osRecvMesg(&romcopy_msgque, NULL, OS_MESG_BLOCK);

    DEBUG_CYCLES_END(dma_start, dma_cycles);
    DEBUG_COUNTER(LastDmaCycles += dma_cycles);
}

/*
====================
=
= W_Init
=
====================
*/

void W_Init (void) // 8002BEC0
{
    wadinfo_t wadfileheader ALIGNED(16);
    int infotableofs, i;
    bool intextures = false;

    W_GetRomData(0, &wadfileheader, sizeof(wadinfo_t), sizeof(wadinfo_t));

    //sprintf(str, "identification %s",wadfileptr->identification);
    //printstr(WHITE, 0, 4, str);

    numlumps = LONGSWAP(wadfileheader.numlumps);
    i = numlumps * sizeof(lumpinfo_t);
    lumpinfo = Z_BumpAlloc(i);
    infotableofs = LONGSWAP(wadfileheader.infotableofs);
    W_GetRomData(infotableofs, lumpinfo, i, i);

    //sprintf(str, "identification %s",wadfileptr->identification);
    //printstr(WHITE, 0, 4, str);
    //sprintf(str, "numlumps %d",numlumps);
    //printstr(WHITE, 0, 5, str);
    //sprintf(str, "infotableofs %d",infotableofs);
    //printstr(WHITE, 0, 6, str);

    for(i = 0; i < numlumps; i++)
    {
        lumpinfo[i].filepos = LONGSWAP(lumpinfo[i].filepos);
        lumpinfo[i].size = LONGSWAP(lumpinfo[i].size);

        //sprintf(str, "filepos %d        ",lumpinfo[i].filepos);
        //printstr(WHITE, 0, 7, str);
        //sprintf(str, "size %d           ",lumpinfo[i].size);
        //printstr(WHITE, 0, 8, str);
    }

    // [nova] - find appropriate size for a buffer for decompressing textures/sprites
    for(i = 0; i < numlumps; i++)
    {
        char *name = lumpinfo[i].name;

        if (D_strncmp(name, "T_START", 8) == 0)
            intextures = true;
        else if (D_strncmp(name, "T_END", 8) == 0)
            intextures = false;

        if (!W_IsLumpCompressed(i))
            continue;

        // skip maps
        if (name[0] == ('M' | -0x80) && name[1] == 'A' && name[2] == 'P'
                && name[3] >= '0' && name[3] <= '9'
                && name[4] >= '0' && name[4] <= '9'
                && name[5] == '\0')
        {
            AllocDecodeBuffers();
            continue;
        }

        if (intextures
                || (name[0] == ('D' | -0x80) && name[1] == 'E' && name[2] == 'M'
                    && name[3] == 'O' && name[4] >= '0' && name[4] <= '9'
                    && name[5] == 'L' && name[6] == 'M' && name[7] == 'P'))
            AllocDecodeBuffers();

        // skip some lumps never loaded during gameplay
        if (D_strncmp(name, "\xc5VIL", 8) == 0
                || D_strncmp(name, "\xc6INAL", 8) == 0
                || D_strncmp(name, "\xd4ITLE", 8) == 0)
            continue;

        int lumpsize = lumpinfo[i+1].filepos - lumpinfo[i].filepos;
        maxcompressedsize = MAX(maxcompressedsize, lumpsize);
    }

    lumpcache = Z_BumpAlloc(numlumps * sizeof(lumpcache_t));
    bzero(lumpcache, numlumps * sizeof(lumpcache_t));

    if (maxcompressedsize)
        decompressbuf = Z_BumpAlloc(maxcompressedsize);
}


/*
====================
=
= W_CheckNumForName
=
= Returns -1 if name not found
=
====================
*/
//int W_CheckNumForName(char *name, int unk1, int hibit1, int hibit2)    // original
int W_CheckNumForName(const char *name, int hibit1, int hibit2) // 8002C0F4 removed unknown parameter
{
    char    name8[12];
    char    c, *tmp;
    int     i;
    lumpinfo_t  *lump_p;

    /* make the name into two integers for easy compares */

    *(int *)&name8[4] = 0;
    *(int *)&name8[0] = 0;

    tmp = name8;
    while ((c = *name) != 0)
    {
        *tmp++ = c;

        if ((tmp >= name8+8))
        {
            break;
        }

        name++;
    }

    /* scan backwards so patch lump files take precedence */

    lump_p = lumpinfo;
    for(i = 0; i < numlumps; i++)
    {
        if (    (*(int *)&name8[0] == (*(int *)&lump_p->name[0] & hibit1)) &&
            (*(int *)&name8[4] == (*(int *)&lump_p->name[4] & hibit2))  )
        {
            return i;
        }

        lump_p++;
    }

    return -1;
}

/*
====================
=
= W_GetNumForName
=
= Calls W_CheckNumForName, but bombs out if not found
=
====================
*/

int W_GetNumForName (const char *name) // 8002C1B8
{
    int i;

    i = W_CheckNumForName (name, 0x7fffffff, 0xFFFFFFFF);
    if (i != -1)
        return i;

    I_Error ("W_GetNumForName: %s not found!",name);
    return -1;
}


/*
====================
=
= W_LumpLength
=
= Returns the buffer size needed to load the given lump
=
====================
*/

int W_LumpLength (int lump) // 8002C204
{
    if ((lump < 0) || (lump >= numlumps))
        I_Error ("W_LumpLength: lump %i out of range",lump);

    return lumpinfo[lump].size;
}

bool W_IsLumpCompressed (int lump)
{
    return !!(lumpinfo[lump].name[0] & 0x80);
}

/*
====================
=
= W_ReadLump
=
= Loads the lump into the given buffer, which must be >= W_LumpLength()
=
====================
*/

void W_ReadLump (int lump, void *dest, decodetype dectype, int usable) // 8002C260
{
    lumpinfo_t *l;
    int lumpsize;
    byte *input;

    if ((lump < 0) || (lump >= numlumps))
        I_Error ("W_ReadLump: lump %i out of range",lump);

    l = &lumpinfo[lump];
    if(dectype != dec_none)
    {
        if ((l->name[0] & 0x80)) /* compressed */
        {
            lumpsize = l[1].filepos - (l->filepos);

            if (lumpsize <= maxcompressedsize)
                input = decompressbuf;
            else
                input = Z_Alloc(lumpsize, PU_STATIC, NULL);

            W_GetRomData(l->filepos, input, lumpsize, lumpsize);

            if (dectype == dec_jag)
                DecodeJaguar(input, (byte *)dest);
            else // dec_d64
                DecodeD64(input, (byte *)dest);

            if (input != decompressbuf)
                Z_Free(input);

            return;
        }
    }

    if (l->name[0] & 0x80)
        lumpsize = l[1].filepos - (l->filepos);
    else
        lumpsize = (l->size);

    W_GetRomData(l->filepos, dest, lumpsize, usable >= 0 ? usable : lumpsize);
}

void *W_GetLump(int lump, decodetype dectype, int usable)
{
  void *ptr = Z_Alloc(W_LumpLength(lump), PU_STATIC, NULL);

  W_ReadLump(lump, ptr, dectype, usable);

  return ptr;
}

/*
====================
=
= W_CacheLumpNum
=
====================
*/

void *W_CacheLumpNum (int lump, int tag, decodetype dectype, int usable) // 8002C430
{
    int lumpsize;
    lumpcache_t *lc;

    if ((lump < 0) || (lump >= numlumps))
        I_Error ("W_CacheLumpNum: lump %i out of range",lump);

    lc = &lumpcache[lump];

    if (!lc->cache)
    {   /* read the lump in */
        //if (dectype == dec_d64)
            //ST_DebugPrint("W_CacheLumpNum: lump %i", lump);

        if (dectype == dec_none)
            lumpsize = lumpinfo[lump + 1].filepos - lumpinfo[lump].filepos;
        else
            lumpsize = lumpinfo[lump].size;

        Z_Malloc(lumpsize, tag, &lc->cache);

        W_ReadLump(lump, lc->cache, dectype, usable);
    }
    else
    {
        if (tag & PU_CACHE) {
            Z_Touch(lc->cache);
        }
    }

    return lc->cache;
}

/*
====================
=
= W_CacheLumpName
=
====================
*/

void *W_CacheLumpName (char *name, int tag, decodetype dectype, int usable) // 8002C57C
{
    return W_CacheLumpNum (W_GetNumForName(name), tag, dectype, usable);
}

void *W_GetInitLump (char *name, decodetype dectype, int usable)
{
    int lumpsize;
    lumpcache_t *lc;
    int lump;

    lump = W_GetNumForName(name);
    if ((lump < 0) || (lump >= numlumps))
        I_Error ("W_CacheLumpNum: lump %i out of range",lump);

    lc = &lumpcache[lump];

    if (!lc->cache)
    {
        if (dectype == dec_none)
            lumpsize = lumpinfo[lump + 1].filepos - lumpinfo[lump].filepos;
        else
            lumpsize = lumpinfo[lump].size;

        lc->cache = Z_BumpAlloc(lumpsize);

        W_ReadLump(lump, lc->cache, dectype, usable);
    }

    return lc->cache;
}


/*
============================================================================

MAP LUMP BASED ROUTINES

============================================================================
*/

/*
====================
=
= W_OpenMapWad
=
= Exclusive Psx Doom / Doom64
====================
*/

void W_OpenMapWad(int mapnum) // 8002C5B0
{
    int lump, size, infotableofs, i;
    char name [8];

    if (mapnum > 0)
    {
        name[0] = 'M';
        name[1] = 'A';
        name[2] = 'P';
        name[3] = '0' + (char)(mapnum / 10);
        name[4] = '0' + (char)(mapnum % 10);
        name[5] = NULL;

        lump = W_GetNumForName(name);
        if (W_IsLumpCompressed(lump))
        {
            size = W_LumpLength(lump);

            //sprintf(str, "name %s           ",name);
            //printstr(WHITE, 0, 7, str);
            //sprintf(str, "lump %d           ",lump);
            //printstr(WHITE, 0, 8, str);
            //sprintf(str, "size %d           ",size);
            //printstr(WHITE, 0, 9, str);

            mapfileptr = Z_Alloc(size, PU_STATIC, NULL);
            maplumppos = 0;

            W_ReadLump(lump, mapfileptr, dec_d64, -1);
        }
        else
        {
            mapfileptr = NULL;
            maplumppos = lumpinfo[lump].filepos;
        }
    }
#ifdef USB
    else if (PendingUSBOperations & USB_OP_LOADMAP)
    {
        PendingUSBOperations &= ~USB_OP_LOADMAP;

        u32 size = I_CmdNextTokenSize();
        if (size == 0)
            I_Error("No map provided");

        mapfileptr = Z_Alloc(size, PU_STATIC, NULL);
        maplumppos = 0;
        I_CmdGetNextToken(mapfileptr);
        I_CmdSkipAllTokens();
        gamemap = 0;
    }
#endif
    else
    {
        I_Error("Invalid map %d", mapnum);
    }

    if (mapfileptr)
    {
        mapnumlumps = LONGSWAP(((wadinfo_t*)mapfileptr)->numlumps);
        infotableofs = LONGSWAP(((wadinfo_t*)mapfileptr)->infotableofs);

        //sprintf(str, "mapnumlumps %d           ",mapnumlumps);
        //printstr(WHITE, 0, 10, str);
        //sprintf(str, "infotableofs %d           ",infotableofs);
        //printstr(WHITE, 0, 11, str);

        maplump = (lumpinfo_t*)(mapfileptr + infotableofs);
    }
    else if (maplumppos)
    {
        wadinfo_t wadfileheader ALIGNED(16);
        int ls;

        W_GetRomData(maplumppos, &wadfileheader, sizeof(wadinfo_t), sizeof(wadinfo_t));

        mapnumlumps = LONGSWAP(wadfileheader.numlumps);
        infotableofs = LONGSWAP(wadfileheader.infotableofs);

        ls = mapnumlumps * sizeof(lumpinfo_t);
        maplump = (lumpinfo_t *) Z_Malloc(ls, PU_STATIC, 0);
        W_GetRomData(maplumppos + infotableofs, maplump, ls, ls);
    }

    for(i = 0; i < mapnumlumps; i++)
    {
        maplump[i].filepos = LONGSWAP(maplump[i].filepos);
        maplump[i].size = LONGSWAP(maplump[i].size);
    }
}
/*
====================
=
= W_FreeMapLump
=
= Exclusive Doom64
====================
*/

void W_FreeMapLumps(void) // 8002C748
{
    if (mapfileptr)
    {
        Z_Free(mapfileptr);
        mapfileptr = NULL;
    }
    else
    {
        Z_Free(maplump);
    }
    maplump = NULL;
    maplumppos = 0;
    mapnumlumps = 0;
}

void W_FreeMapLump(void *ptr)
{
    if (!mapfileptr)
        Z_Free(ptr);
}

/*
====================
=
= W_MapLumpLength
=
= Exclusive Psx Doom / Doom64
====================
*/

int W_MapLumpLength(int lump) // 8002C77C
{
    if (lump >= mapnumlumps)
        I_Error("W_MapLumpLength: %i out of range", lump);

    return maplump[lump].size;
}


/*
====================
=
= W_MapGetNumForName
=
= Exclusive Psx Doom / Doom64
====================
*/

int W_MapGetNumForName(const char *name) // 8002C7D0
{
    char    name8[12];
    char    c, *tmp;
    int     i;
    lumpinfo_t  *lump_p;

    /* make the name into two integers for easy compares */

    *(int *)&name8[4] = 0;
    *(int *)&name8[0] = 0;

    tmp = name8;
    while ((c = *name) != 0)
    {
        *tmp++ = c;

        if (tmp >= (name8+8))
        {
            break;
        }

        name++;
    }

    /* scan backwards so patch lump files take precedence */

    lump_p = maplump;
    for(i = 0; i < mapnumlumps; i++)
    {
        if (    (*(int *)&name8[0] == (*(int *)&lump_p->name[0] & 0x7fffffff)) &&
            (*(int *)&name8[4] == (*(int *)&lump_p->name[4]))   )
        {
            return i;
        }

            lump_p++;
    }

    return -1;
}

/*
====================
=
= W_GetMapLump
=
= Exclusive Doom64
====================
*/

void  *W_GetMapLump(int lump) // 8002C890
{
    if (lump >= mapnumlumps)
        I_Error("W_GetMapLump: lump %d out of range", lump);

    if (mapfileptr)
        return (void *) ((byte *)mapfileptr + maplump[lump].filepos);

    int size = ALIGN(maplump[lump].size, 2);
    void *ptr = Z_Alloc(size, PU_STATIC, NULL);

    W_GetRomData(maplumppos + maplump[lump].filepos, ptr, size, size);

    return ptr;
}

void W_ReadMapLump(int lump, void *ptr)
{
    if (mapfileptr)
    {
        D_memcpy(ptr, mapfileptr + maplump[lump].filepos, maplump[lump].size);
    }
    else
    {
        int size = ALIGN(maplump[lump].size, 2);
        W_GetRomData(maplumppos + maplump[lump].filepos, ptr, size, size);
    }
}
