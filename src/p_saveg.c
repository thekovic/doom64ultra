#include "doomdef.h"
#include "p_local.h"
#include "r_local.h"
#include "p_saveg.h"

static u32 write_button(u32 button)
{
    if (button >= PAD_Z_TRIG)
        return button >> 19;
    if (button >= PAD_RIGHT)
        return button >> 18;
    return button >> 16;
}

static u32 read_button(u32 button)
{
    if (button & 0x1000)
        return PAD_A;
    if (button & 0x800)
        return PAD_B;
    if (button & 0x400)
        return PAD_Z_TRIG;
    if (button & 0x200)
        return PAD_UP;
    if (button & 0x100)
        return PAD_DOWN;
    if (button & 0x80)
        return PAD_LEFT;
    if (button & 0x40)
        return PAD_RIGHT;
    if (button & 0x20)
        return PAD_L_TRIG;
    if (button & 0x10)
        return PAD_R_TRIG;
    if (button & 0x8)
        return PAD_UP_C;
    if (button & 0x4)
        return PAD_DOWN_C;
    if (button & 0x2)
        return PAD_LEFT_C;
    if (button & 0x1)
        return PAD_RIGHT_C;
    return 0;
}

void P_ArchivePlayerConfig(int pi, savedplayerconfig_t *buf)
{
    const controls_t *controls = &CurrentControls[pi];

    buf->sensitivity = playerconfigs[pi].sensitivity;
    buf->crosshair = playerconfigs[pi].crosshair;
    buf->autorun = playerconfigs[pi].autorun;
    buf->autoaim = playerconfigs[pi].autoaim;
    buf->vlookinverted = playerconfigs[pi].verticallook == -1 ? 1 : 0;
    if (ConfgNumb[pi] == -1)
    {
        buf->customconfig = 1;
        buf->confignum = 0;
    }
    else
    {
        buf->customconfig = 0;
        buf->confignum = ConfgNumb[pi];
    }

    buf->bt_right = write_button(controls->BT_RIGHT);
    buf->bt_left = write_button(controls->BT_LEFT);
    buf->bt_forward = write_button(controls->BT_FORWARD);
    buf->bt_back = write_button(controls->BT_BACK);
    buf->bt_attack = write_button(controls->BT_ATTACK);
    buf->bt_use = write_button(controls->BT_USE);
    buf->bt_map = write_button(controls->BT_MAP);
    buf->bt_speed = write_button(controls->BT_SPEED);
    buf->bt_strafe = write_button(controls->BT_STRAFE);
    buf->bt_strafeleft = write_button(controls->BT_STRAFELEFT);
    buf->bt_straferight = write_button(controls->BT_STRAFERIGHT);
    buf->bt_weaponbackward = write_button(controls->BT_WEAPONBACKWARD);
    buf->bt_weaponforward = write_button(controls->BT_WEAPONFORWARD);
    buf->bt_look = write_button(controls->BT_LOOK);
    buf->bt_lookup = write_button(controls->BT_LOOKUP);
    buf->bt_lookdown = write_button(controls->BT_LOOKDOWN);
    buf->bt_jump = write_button(controls->BT_JUMP);
    buf->bt_crouch = write_button(controls->BT_CROUCH);
    buf->stickmode = controls->STICK_MODE;
}

void P_UnArchivePlayerConfig(int pi, const savedplayerconfig_t *buf)
{
    playerconfigs[pi].sensitivity = buf->sensitivity;
    playerconfigs[pi].crosshair = buf->crosshair;
    playerconfigs[pi].autorun = buf->autorun;
    playerconfigs[pi].autoaim = buf->autoaim;
    playerconfigs[pi].verticallook = buf->vlookinverted ? -1 : 1;

    if (buf->customconfig)
        ConfgNumb[pi] = -1;
    else
        ConfgNumb[pi] = buf->confignum % MAXCONTROLSETUPS;
    CurrentControls[pi].BT_RIGHT = read_button(buf->bt_right);
    CurrentControls[pi].BT_LEFT = read_button(buf->bt_left);
    CurrentControls[pi].BT_FORWARD = read_button(buf->bt_forward);
    CurrentControls[pi].BT_BACK = read_button(buf->bt_back);
    CurrentControls[pi].BT_ATTACK = read_button(buf->bt_attack);
    CurrentControls[pi].BT_USE = read_button(buf->bt_use);
    CurrentControls[pi].BT_MAP = read_button(buf->bt_map);
    CurrentControls[pi].BT_SPEED = read_button(buf->bt_speed);
    CurrentControls[pi].BT_STRAFE = read_button(buf->bt_strafe);
    CurrentControls[pi].BT_STRAFELEFT = read_button(buf->bt_strafeleft);
    CurrentControls[pi].BT_STRAFERIGHT = read_button(buf->bt_straferight);
    CurrentControls[pi].BT_WEAPONBACKWARD = read_button(buf->bt_weaponbackward);
    CurrentControls[pi].BT_WEAPONFORWARD = read_button(buf->bt_weaponforward);
    CurrentControls[pi].BT_LOOK = read_button(buf->bt_look);
    CurrentControls[pi].BT_LOOKUP = read_button(buf->bt_lookup);
    CurrentControls[pi].BT_LOOKDOWN = read_button(buf->bt_lookdown);
    CurrentControls[pi].BT_JUMP = read_button(buf->bt_jump);
    CurrentControls[pi].BT_CROUCH = read_button(buf->bt_crouch);
    CurrentControls[pi].STICK_MODE = buf->stickmode ? buf->stickmode : (STICK_TURN | STICK_MOVE);
}
