#pragma once

#include "doomdef.h"

typedef struct ALIGNED(4) {
    u32 looksensitivity: 7;
    u32 movesensitivity: 7;
    u32 crosshair: 1;
    u32 autorun: 1;
    u32 autoaim: 1;
    u32 vlookinverted: 1;
    u32 lookspring: 1;
    u32 hue: 6;
    u32 saturation: 6;
    u32 value: 6;

    u32 customconfig: 1;
    u32 confignum: 4;
    u32 stickmode: 4;
    u32 bt_right: 13;
    u32 bt_left: 13;
    u32 bt_forward: 13;
    u32 bt_back: 13;
    u32 bt_attack: 13;
    u32 bt_use: 13;
    u32 bt_map: 13;
    u32 bt_speed: 13;
    u32 bt_strafe: 13;
    u32 bt_strafeleft: 13;
    u32 bt_straferight: 13;
    u32 bt_weaponbackward: 13;
    u32 bt_weaponforward: 13;
    u32 bt_look: 13;
    u32 bt_lookup: 13;
    u32 bt_lookdown: 13;
    u32 bt_jump: 13;
    u32 bt_crouch: 13;

    u32 stickmode2: 4;
    u32 ctrl2a: 5;
    u32 ctrl2b: 5;
    u32 ctrl2z: 5;
} savedplayerconfig_t;

void P_ArchivePlayerConfig(int pi, savedplayerconfig_t *buf);
void P_UnArchivePlayerConfig(int pi, const savedplayerconfig_t *buf);

typedef struct
{
    u16 tag;
    u16 activator;
} savedmacroactivator_t;

typedef struct ALIGNED(8) {
    u32 cameratarget: 16;
    u32 active: 1;
    u32 macroidx1: 2;
    u32 macroidx2: 2;
    u32 activemacro: 16;
    u32 restartmacro: 16;
    u32 macrointeger: 16;
    u32 macrocounter: 16;
    u32 macroactivator: 16;
    u32 macroline: 16;
    savedmacroactivator_t macroqueue[4];
} savedmacrosheader_t;

void P_ArchiveActiveMacro (savedmacrosheader_t *header) SEC_MENU;
void P_UnArchiveActiveMacro (const savedmacrosheader_t *header) SEC_STARTUP;

u32 P_ArchivePlayers (u8 *savep) SEC_MENU;
u32 P_ArchiveSectors (u8 *savep, u32 savepsize, u32 *start) SEC_MENU;
u32 P_ArchiveLines (u8 *savep, u32 savepsize, u32 *start) SEC_MENU;
u32 P_ArchiveMacros (u8 *savep, u32 savepsize, u32 *start) SEC_MENU;
u32 P_ArchiveMobjs (u8 *savep, u32 savepsize, mobj_t **start, u32 *counter) SEC_MENU;
u32 P_ArchiveThinkers (u8 *savep, u32 savepsize, thinker_t **start, u32 *counter) SEC_MENU;

u32 P_UnArchivePlayers (const u8 *savep) SEC_STARTUP;
u32 P_UnArchiveSectors (const u8 *savep, u32 savepsize, u32 *start) SEC_STARTUP;
u32 P_UnArchiveLines (const u8 *savep, u32 savepsize, u32 *start) SEC_STARTUP;
u32 P_UnArchiveMacros (const u8 *savep, u32 savepsize, u32 *start) SEC_STARTUP;
u32 P_UnArchiveMobjs (const u8 *savep, u32 savepsize, u32 *counter) SEC_STARTUP;
u32 P_UnArchiveThinkers (const u8 *savep, u32 savepsize, u32 *counter) SEC_STARTUP;
void P_LinkUnArchivedMobjs(void) SEC_STARTUP;

u32 P_CurrentQuickSaveSize(u32 max);
