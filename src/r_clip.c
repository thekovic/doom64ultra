/* Emacs style mode select     -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *    PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *    based on BOOM, a modified and improved DOOM engine
 *    Copyright (C) 1999 by
 *    id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *    Copyright (C) 1999-2000 by
 *    Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *    Copyright 2005, 2006 by
 *    Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *    02111-1307, USA.
 *
 * DESCRIPTION:
 *
 *---------------------------------------------------------------------
 */

/*
*
** gl_clipper.cpp
**
** Handles visibility checks.
** Loosely based on the JDoom clipper.
**
**---------------------------------------------------------------------------
** Copyright 2003 Tim Stump
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**        notice, this list of conditions and the following disclaimer in the
**        documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**        derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#include "doomdef.h"

typedef struct clipnode_s
{
    struct clipnode_s *prev, *next;
    angle_t start, end;
} clipnode_t;

#define MAXCLIPNODES 512

static clipnode_t clipnodes[MAXCLIPNODES];
static int numclipnodes;
static clipnode_t *freelist;
SDATA static clipnode_t *cliphead = NULL;

static clipnode_t *R_ClipNodeGetNew(void)
{
    if (freelist)
    {
        clipnode_t * p = freelist;
        freelist = p->next;
        return p;
    }
    if (numclipnodes >= MAXCLIPNODES)
        return NULL;
    return &clipnodes[numclipnodes++];
}

static clipnode_t *R_ClipNodeNewRange(angle_t start, angle_t end)
{
    clipnode_t * c = R_ClipNodeGetNew();
    if (!c)
            return NULL;
    c->start = start;
    c->end = end;
    c->next = c->prev=NULL;
    return c;
}

static boolean R_ClipIsRangeVisible(angle_t startAngle, angle_t endAngle)
{
    clipnode_t *ci;
    ci = cliphead;

    if (endAngle == 0 && ci && ci->start == 0)
        return false;

    while (ci != NULL && ci->start < endAngle)
    {
        if (startAngle >= ci->start && endAngle <= ci->end)
        {
            return false;
        }
        ci = ci->next;
    }

    return true;
}

boolean R_CheckClipRange(angle_t startAngle, angle_t endAngle)
{
    if(startAngle > endAngle)
    {
        return (R_ClipIsRangeVisible(startAngle, ANGMAX) || R_ClipIsRangeVisible(0, endAngle));
    }

    return R_ClipIsRangeVisible(startAngle, endAngle);
}


static void R_RemoveClipRange(clipnode_t *range)
{
    if (range == cliphead)
    {
        cliphead = cliphead->next;
    }
    else
    {
        if (range->prev)
        {
            range->prev->next = range->next;
        }
        if (range->next)
        {
            range->next->prev = range->prev;
        }
    }

    range->next = freelist;
    freelist = range;
}

static void R_AddClipRange2(angle_t start, angle_t end)
{
    clipnode_t *node, *temp, *prevNode, *node2, *delnode;

    if (cliphead)
    {
        //check to see if range contains any old ranges
        node = cliphead;
        while (node != NULL && node->start < end)
        {
            if (node->start >= start && node->end <= end)
            {
                temp = node;
                node = node->next;
                R_RemoveClipRange(temp);
            }
            else
            {
                if (node->start <= start && node->end >= end)
                {
                    return;
                }
                else
                {
                    node = node->next;
                }
            }
        }

        //check to see if range overlaps a range (or possibly 2)
        node = cliphead;
        while (node != NULL && node->start <= end)
        {
            if (node->end >= start)
            {
                // we found the first overlapping node
                if (node->start > start)
                {
                    // the new range overlaps with this node's start point
                    node->start = start;
                }
                if (node->end < end)
                {
                    node->end = end;
                }

                node2 = node->next;
                while (node2 && node2->start <= node->end)
                {
                    if (node2->end > node->end)
                    {
                        node->end = node2->end;
                    }

                    delnode = node2;
                    node2 = node2->next;
                    R_RemoveClipRange(delnode);
                }
                return;
            }
            node = node->next;
        }

        //just add range
        node = cliphead;
        prevNode = NULL;
        temp = R_ClipNodeNewRange(start, end);
        if (!temp)
                return;
        while (node != NULL && node->start < end)
        {
            prevNode = node;
            node = node->next;
        }
        temp->next = node;
        if (node == NULL)
        {
            temp->prev = prevNode;
            if (prevNode)
            {
                prevNode->next = temp;
            }
            if (!cliphead)
            {
                cliphead = temp;
            }
        }
        else
        {
            if (node == cliphead)
            {
                cliphead->prev = temp;
                cliphead = temp;
            }
            else
            {
                temp->prev = prevNode;
                prevNode->next = temp;
                node->prev = temp;
            }
        }
    }
    else
    {
        temp = R_ClipNodeNewRange(start, end);
        if (!temp)
                return;
        cliphead = temp;
        return;
    }
}

void R_AddClipRange(angle_t startangle, angle_t endangle)
{
    if(startangle > endangle)
    {
        // The range has to added in two parts.
        R_AddClipRange2(startangle, ANGMAX);
        R_AddClipRange2(0, endangle);
    }
    else
    {
        // Add the range as usual.
        R_AddClipRange2(startangle, endangle);
    }
}

void R_ClipClear(void)
{
    numclipnodes = 0;
    freelist = NULL;
    cliphead = NULL;
}
