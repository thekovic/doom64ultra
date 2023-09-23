/* Z_zone.c */

#include "doomdef.h"
#include "i_debug.h"

#include <stdalign.h>

/*
==============================================================================

                        ZONE MEMORY ALLOCATION

There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

It is of no value to free a cachable block, because it will get overwritten
automatically if needed

==============================================================================
*/

extern u32 NextFrameIdx;

SDATA memzone_t    *mainzone;

DEBUG_COUNTER(SDATA u32 OccupiedMem = 0);
DEBUG_COUNTER(SDATA u32 UsedMem = 0);
DEBUG_COUNTER(SDATA u32 LevelMem = 0);

/*
========================
=
= Z_Init
=
========================
*/


void Z_Init (void) // 8002C8F0
{

    /* mars doesn't have a refzone */
    mainzone = Z_InitZone(MAIN_HEAP_ADDR, MAIN_HEAP_SIZE());

    //PRINTF_D2(WHITE, 0, 25, "%d", (u32)size);
    //while(1){}
}

/*
========================
=
= Z_InitZone
=
========================
*/

memzone_t *Z_InitZone(byte *base, int size) // 8002C934
{
    memzone_t *zone;

    zone = (memzone_t *)base;
    zone->size = size;
    zone->rover = &zone->blocklist;
    zone->rover2 = &zone->blocklist;
    zone->rover3 = &zone->blocklist;
    zone->blocklist.size = size - (int)((byte *)&zone->blocklist - (byte *)zone);
    zone->blocklist.user = NULL;
    zone->blocklist.tag = 0;
    zone->blocklist.id = ZONEID;
    zone->blocklist.next = NULL;
    zone->blocklist.prev = NULL;
    //zone->blocklist.lockframe = -1;

    return zone;
}

/*
========================
=
= Z_SetAllocBase
= Exclusive Doom64
=
========================
*/

void Z_SetAllocBase(memzone_t *mainzone) // 8002C970
{
    mainzone->rover2 = mainzone->rover;
}

/*
========================
=
= Z_Malloc2
=
= Allocates block starting from the beginning of the heap
= You can pass a NULL user if the tag is < PU_PURGELEVEL
========================
*/

#define MINFRAGMENT    64

void *Z_Malloc2 (register memzone_t *mainzone, register int size, register int tag, void *user) // 8002C97C
{
    int        extra;
    register memblock_t    *start, *rover, *newblock, *base;

#ifdef DEBUG_MEM
    Z_CheckZone (mainzone);    /* DEBUG */
#endif

    /* */
    /* scan through the block list looking for the first free block */
    /* of sufficient size, throwing out any purgable blocks along the way */
    /* */

    size += sizeof(memblock_t);    /* account for size of block header */
    size = ALIGN(size, 16);        /* phrase align everything */

    start = base = mainzone->rover;

    while (base->user || base->size < size)
    {
        if (base->user)
            rover = base;
        else
            rover = base->next;

        if (!rover)
            goto backtostart;

        if (rover->user)
        {
            if (!(rover->tag & PU_PURGELEVEL))
            {
                if (!(rover->tag & PU_CACHE) || (u32)rover->lockframe >= (NextFrameIdx - 1))
                {
                    /* hit an in use block, so move base past it */
                    base = rover->next;
                    if (!base)
                    {
backtostart:
                        base = mainzone->rover2;
                    }

                    if (base == start)    /* scaned all the way around the list */
                    {
                        Z_DumpHeap(mainzone);
                        I_Error("Z_Malloc2: failed allocation on %i", size);
                        return NULL;
                    }
                    continue;
                }
            }

            /* */
            /* free the rover block (adding the size to base) */
            /* */
            Z_Free2(mainzone, (byte *)rover + sizeof(memblock_t)); /* mark as free */
        }

        if (base != rover)
        {    /* merge with base */
            base->size += rover->size;
            base->next = rover->next;
            if (rover->next)
                rover->next->prev = base;
            else
                mainzone->rover3 = base;
        }
    }

    /* */
    /* found a block big enough */
    /* */
    extra = base->size - size;
    if (extra >  MINFRAGMENT)
    {    /* there will be a free fragment after the allocated block */
        newblock = (memblock_t *) ((byte *)base + size );
        newblock->prev = base;
        newblock->next = base->next;
        if (newblock->next)
            newblock->next->prev = newblock;
        else
            mainzone->rover3 = newblock;

        base->next = newblock;
        base->size = size;

        newblock->size = extra;
        newblock->user = NULL;        /* free block */
        newblock->tag = 0;
        newblock->id = ZONEID;
    }

    DEBUG_COUNTER(OccupiedMem += base->size);
    DEBUG_COUNTER(if (tag < PU_PURGELEVEL) UsedMem += base->size);
    DEBUG_COUNTER(if (tag & (PU_LEVEL|PU_LEVSPEC)) LevelMem += base->size);

    if (user)
    {
        base->user = user;            /* mark as an in use block */
        *(void **)user = (void *)((byte *)base + sizeof(memblock_t));
    }
    else
    {
        if (tag >= PU_PURGELEVEL)
            I_Error ("Z_Malloc: an owner is required for purgable blocks");
        base->user = (void *)1;        /* mark as in use, but unowned     */
    }

    base->tag = tag;
    base->id = ZONEID;
    base->lockframe = NextFrameIdx;

    mainzone->rover = base->next;    /* next allocation will start looking here */
    if (!mainzone->rover)
    {
        mainzone->rover3 = base;
        mainzone->rover = mainzone->rover2;//mainzone->rover = &mainzone->blocklist;
    }

#ifdef DEBUG_MEM
    Z_CheckZone (mainzone);    /* DEBUG */
#endif

    return (void *) ((byte *)base + sizeof(memblock_t));
}

/* Allocate a block at the specified address. The address must be aligned to
 * a 64-byte boundary.
 */
void Z_Reserve2 (memzone_t *mainzone, void *addr, int size)
{
    int     extra;
    memblock_t  *block, *newblock, *base;

    if ((u32)(addr) % 16)
        I_Error ("Z_Reserve: invalid alignment");

    #if DEBUG_
    Z_CheckZone (mainzone); /* DEBUG */
    #endif

    block = (void*)(((char*)addr) - sizeof(memblock_t));

    /* */
    /* scan through the block list looking for the block containing the address */
    /* */
    size += sizeof(memblock_t); /* account for size of block header */

    base = &mainzone->blocklist;
    while (base->next && base->next < block)
        base = base->next;

    if (base->user)
        I_Error ("Z_Reserve: address in use");
    if (base->size < size)
        I_Error ("Z_Reserve: no space left");

    if (block != base)
    {
        if (((char*)base) + MINFRAGMENT > (char*)block)
            I_Error ("Z_Reserve: invalid alignment");

        extra = (char*)block - (char*)base;

        block->prev = base;
        block->next = base->next;
        if (block->next)
            block->next->prev = block;

        base->next = block;
        block->size = base->size - extra;
        base->size = extra;
    }

    extra = block->size - size;
    if (extra >  MINFRAGMENT)
    {   /* there will be a free fragment after the allocated block */
        newblock = (memblock_t *) ((byte *)block + size );
        newblock->prev = block;
        newblock->next = block->next;
        if (newblock->next)
            newblock->next->prev = newblock;
        else
            mainzone->rover3 = newblock;

        block->next = newblock;
        block->size = size;

        newblock->size = extra;
        newblock->user = NULL;      /* free block */
        newblock->tag = 0;
        newblock->id = ZONEID;
    }

    DEBUG_COUNTER(OccupiedMem += block->size);
    DEBUG_COUNTER(UsedMem += block->size);

    block->user = (void *)1;        /* mark as in use, but unowned   */

    block->tag = PU_STATIC;
    block->id = ZONEID;

    if (!block->next)
        mainzone->rover3 = block;

#if DEBUG_
    Z_CheckZone (mainzone); /* DEBUG */
#endif
}

/*
========================
=
= Z_Alloc2
=
= Allocates block starting from the end of the heap
= You can pass a NULL user if the tag is < PU_PURGELEVEL
= Exclusive Psx Doom
========================
*/

void *Z_Alloc2(register memzone_t *mainzone, register int size, register int tag, register void *user) // 8002CBE0
{
    int extra;
    register memblock_t *rover, *base, *block, *newblock;

#ifdef DEBUG_MEM
    Z_CheckZone (mainzone);    /* DEBUG */
#endif

    /* */
    /* scan through the block list looking for the first free block */
    /* of sufficient size, throwing out any purgable blocks along the way */
    /* */

    size += sizeof(memblock_t);    /* account for size of block header */
    size = ALIGN(size, 16);        /* phrase align everything */

    base = mainzone->rover3;

    while (base->user || base->size < size)
    {
        if (base->user)
            rover = base;
        else
        {
            /* hit an in use block, so move base past it */
            rover = base->prev;
            if (!rover)
            {
                Z_DumpHeap(mainzone);
                I_Error("Z_Alloc: failed allocation on %i", size);
                return NULL;
            }
        }

        if (rover->user)
        {
            if (!(rover->tag & PU_PURGELEVEL))
            {
                if (!(rover->tag & PU_CACHE) || (u32)rover->lockframe >= (NextFrameIdx - 1))
                {
                    /* hit an in use block, so move base past it */
                    base = rover->prev;
                    if (!base)
                    {
                        Z_DumpHeap(mainzone);
                        I_Error("Z_Alloc: failed allocation on %i", size);
                        return NULL;
                    }
                    continue;
                }
            }

            /* */
            /* free the rover block (adding the size to base) */
            /* */
            Z_Free2(mainzone, (byte *)rover + sizeof(memblock_t)); /* mark as free */
        }

        if (base != rover)
        {
            /* merge with base */
            rover->size += base->size;
            rover->next = base->next;

            if (base->next)
                base->next->prev = rover;
            else
                mainzone->rover3 = rover;

            base = rover;
        }
    }

    /* */
    /* found a block big enough */
    /* */
    extra = (base->size - size);

    newblock = base;
    block = base;

    if (extra > MINFRAGMENT)
    {
        block = (memblock_t *)((byte *)base + extra);
        block->prev = newblock;
        block->next = (void *) newblock->next;

        if (newblock->next)
            newblock->next->prev = block;

        newblock->next = block;
        block->size = size;
        newblock->size = extra;
        newblock->user = 0;
        newblock->tag = 0;
        newblock->id = ZONEID;
    }

    DEBUG_COUNTER(OccupiedMem += block->size);
    DEBUG_COUNTER(if (tag < PU_PURGELEVEL) UsedMem += block->size);
    DEBUG_COUNTER(if (tag & (PU_LEVEL|PU_LEVSPEC)) LevelMem += block->size);

    if (block->next == 0)
        mainzone->rover3 = block;

    if (user)
    {
        block->user = user;            /* mark as an in use block */
        *(void **)user = (void *)((byte *)block + sizeof(memblock_t));
    }
    else
    {
        if (tag >= PU_PURGELEVEL)
            I_Error("Z_Alloc: an owner is required for purgable blocks");
        block->user = (void *)1;        /* mark as in use, but unowned     */
    }

    block->id = ZONEID;
    block->tag = tag;
    block->lockframe = NextFrameIdx;

#ifdef DEBUG_MEM
    Z_CheckZone (mainzone);    /* DEBUG */
#endif

    return (void *) ((byte *)block + sizeof(memblock_t));
}

/*
========================
=
= Z_Free2
=
========================
*/

void Z_Free2(memzone_t *mainzone, void *ptr) // 8002CE28
{
    memblock_t    *block;

    block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
    if (block->id != ZONEID)
        I_Error("Z_Free: freed a pointer without ZONEID");

    DEBUG_COUNTER(OccupiedMem -= block->size);
    DEBUG_COUNTER(if (block->tag < PU_PURGELEVEL) UsedMem -= block->size);
    DEBUG_COUNTER(if (block->tag & (PU_LEVEL|PU_LEVSPEC)) LevelMem -= block->size);

    if (block->user > (void **)0x100)    /* smaller values are not pointers */
        *block->user = 0;        /* clear the user's mark */
    block->user = NULL;    /* mark as free */
    block->tag = 0;

#ifdef DEBUG_MEM
    Z_CheckZone (mainzone);    /* DEBUG */
#endif
}

/*
========================
=
= Z_FreeTags
=
========================
*/

void Z_FreeTags (memzone_t *mainzone, int tag) // 8002CE8C
{
    memblock_t *block, *next;

    for (block = &mainzone->blocklist; block ; block = next)
    {
        /* get link before freeing */
        next = block->next;

        /* free block */
        if (block->user)
        {
            if(block->tag & tag)
            {
                Z_Free2(mainzone, (byte *)block + sizeof(memblock_t));
            }
        }
    }

    for (block = &mainzone->blocklist; block; block = next)
    {
        /* get link before freeing */
        next = block->next;

        if (!block->user && next && !next->user)
        {
            block->size += next->size;
            block->next = next->next;
            if (next->next)
                next->next->prev = block;
            next = block;
        }
    }

    mainzone->rover = &mainzone->blocklist;
    mainzone->rover2 = &mainzone->blocklist;
    mainzone->rover3 = &mainzone->blocklist;

    block = mainzone->blocklist.next;
    while (block)
    {
        mainzone->rover3 = block;
        block = block->next;
    }
}

/*
========================
=
= Z_Touch
= Exclusive Doom64
=
========================
*/

void Z_Touch(void *ptr) // 8002CF9C
{
    memblock_t    *block;

    block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
#ifdef DEBUG_MEM
    if (block->id != ZONEID)
        I_Error("Z_Touch: touched a pointer without ZONEID");
#endif

    block->lockframe = NextFrameIdx;
}

bool Z_Stale(void *ptr)
{
    memblock_t    *block;

    block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
#ifdef DEBUG_MEM
    if (block->id != ZONEID)
        I_Error("Z_Stale: inspected a pointer without ZONEID");
#endif
    return (u32)block->lockframe < NextFrameIdx - 1;
}

/*
========================
=
= Z_CheckZone
=
========================
*/

void Z_CheckZone (memzone_t *mainzone) // 8002CFEC
{
    memblock_t *checkblock;
    int size;

    for (checkblock = &mainzone->blocklist ; checkblock; checkblock = checkblock->next)
    {
        if(checkblock->id != ZONEID)
            I_Error("Z_CheckZone: block missing ZONEID");

        if (!checkblock->next)
        {
            size = (byte *)checkblock + checkblock->size - (byte *)mainzone;
            if (size != mainzone->size)
                I_Error ("Z_CheckZone: zone size changed from %d to %d\n", mainzone->size, size);
            break;
        }

        if ( (byte *)checkblock + checkblock->size != (byte *)checkblock->next)
            I_Error ("Z_CheckZone: block size does not touch the next block\n");
        if ( checkblock->next->prev != checkblock)
            I_Error ("Z_CheckZone: next block doesn't have proper back link\n");
    }
}


/*
========================
=
= Z_ChangeTag
=
========================
*/

void Z_ChangeTag (void *ptr, int tag) // 8002D0F0
{
    memblock_t    *block;

    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
    if (block->id != ZONEID)
        I_Error ("Z_ChangeTag: freed a pointer without ZONEID");
    if (tag >= PU_PURGELEVEL && (int)block->user < 0x100)
        I_Error ("Z_ChangeTag: an owner is required for purgable blocks");

#ifndef NDEBUG
    if ((tag < PU_PURGELEVEL) != (block->tag < PU_PURGELEVEL))
    {
        if (tag < PU_PURGELEVEL)
            UsedMem += block->size;
        else
            UsedMem -= block->size;
    }
    if ((!!(tag & (PU_LEVEL|PU_LEVSPEC))) != (!!(block->tag & (PU_LEVEL|PU_LEVSPEC))))
    {
        if (tag & (PU_LEVEL|PU_LEVSPEC))
            LevelMem -= block->size;
        else
            LevelMem += block->size;
    }
#endif

    block->tag = tag;
    block->lockframe = NextFrameIdx;
}


/*
========================
=
= Z_FreeMemory
=
========================
*/

int Z_FreeMemory (memzone_t *mainzone) // 8002D188
{
    memblock_t    *block;
    int            free;

    free = 0;
    for (block = &mainzone->blocklist ; block ; block = block->next)
    {
        if (!block->user)
            free += block->size;
    }

    return free;
}

/*
========================
=
= Z_DumpHeap
=
========================
*/

void Z_DumpHeap(memzone_t *mainzone) // 8002D1C8
{
#ifndef NDEBUG
    memblock_t    *block;

    for (block = &mainzone->blocklist; block; block = block->next)
    {
        D_printf("0x%08lx, %7i, 0x%08lx, %3i, %i\n",
                 (u32)block, block->size, (u32)block->user, block->tag,
                 block->lockframe);

        if (!block->next)
            continue;

        if ((byte *)block + block->size != (byte *)block->next)
            D_printf("ERROR: block size does not touch the next block\n");
        if (block->next->prev != block)
            D_printf("ERROR: next block doesn't have proper back link\n");
    }
#endif
}

