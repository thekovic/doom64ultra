#pragma once

#include "doomdef.h"

typedef struct __attribute__((aligned(4))) {
    u32 sensitivity: 7;
    u32 crosshair: 1;
    u32 autorun: 1;
    u32 autoaim: 1;
    u32 vlookinverted: 1;

    u32 customconfig: 1;
    u32 confignum: 4;
    u32 stickmode: 2;
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
} savedplayerconfig_t;

void P_ArchivePlayerConfig(int pi, savedplayerconfig_t *buf);
void P_UnArchivePlayerConfig(int pi, const savedplayerconfig_t *buf);
