/* m_main.c -- menu routines */

#include "config.h"
#include "doomdef.h"
#include "p_local.h"
#include "r_local.h"
#include "st_main.h"
#include "i_usb.h"

extern void P_RefreshBrightness(void);
extern void I_RefreshVideo(void);

static void M_DebugMenuDrawer(void);
static int M_MenuCreditsTicker(void);
static void M_IdCreditsDrawer(void);
static void M_WmsCreditsDrawer(void);

static void M_CustomSkillDrawer(void);
static void M_UpdateSkillPreset(void);

static bool M_SaveMustMove(void);

#define MENU_STRINGS \
    _F(MTXT_MAIN_MENU, "Main Menu") \
    _F(MTXT_LOAD_GAME, "Load Game") \
    _F(MTXT_QUICK_SAVE, "Quick Save") \
    _F(MTXT_QUICK_LOAD, "Quick Load") \
    _F(MTXT_RESTART, "Restart Level") \
    _F(MTXT_RETURN, "\x90 Return") \
    _F(MTXT_MRETURN, "Return") \
    _F(MTXT_YES, "Yes") \
    _F(MTXT_NO, "No") \
    \
    _F(MTXT_LOAD_GAME_PAK, "Game Pak") \
    _F(MTXT_LOAD_CONTROLLER_PAK, "Controller Pak") \
    _F(MTXT_LOAD_QUICK_LOAD, "Quick Load") \
    _F(MTXT_LOAD_PASSWORD, "Password") \
    _F(MTXT_GAME_PAK_DISABLED, "Game Pak") \
    _F(MTXT_CONTROLLER_PAK_DISABLED, "Controller Pak") \
    \
    _F(MTXT_SAVE_GAME_PAK, "Game Pak") \
    _F(MTXT_SAVE_CONTROLLER_PAK, "Controller Pak") \
    _F(MTXT_DONT_SAVE, "Do not save") \
    \
    _F(MTXT_QUICK_SAVE_DISABLED, "Quick Save") \
    _F(MTXT_QUICK_LOAD_DISABLED, "Quick Load") \
    _F(MTXT_GAME_SAVED, "Game Saved") \
    \
    _F(MTXT_MANAGE_PAK, "Manage Pak") \
    _F(MTXT_DONT_USE_PAK, "Do not use Pak") \
    _F(MTXT_TRY_AGAIN, "Try again") \
    _F(MTXT_CREATE_GAME_NOTE, "Create game note") \
    \
    _F(MTXT_NEW_GAME, "New Game") \
    _F(MTXT_SKILL1, "Be Gentle!") \
    _F(MTXT_SKILL2, "Bring It On!") \
    _F(MTXT_SKILL3, "I Own Doom!") \
    _F(MTXT_SKILL4, "Watch Me Die!") \
    _F(MTXT_SKILL5, "Hardcore!") \
    _F(MTXT_CUSTOM_SKILL, "Custom") \
    \
    _F(MTXT_START_GAME, "Start Game") \
    _F(MTXT_PRESET, "Preset") \
    _F(MTXT_PLAYER_DAMAGE, "Player Damage") \
    _F(MTXT_PLAYER_AMMO, "Player Ammo") \
    _F(MTXT_MONSTER_COUNTS, "Monster Counts") \
    _F(MTXT_MONSTER_SPEED, "Monster Speed") \
    _F(MTXT_MONSTER_RESPAWNS, "Monster Respawns") \
    _F(MTXT_MONSTER_INFIGHTING, "Monster Infighting") \
    _F(MTXT_MONSTER_REACTIONS, "Monster Reactions") \
    _F(MTXT_MONSTER_COLLISION, "Monster Collision") \
    _F(MTXT_MONSTER_PAIN, "Monster Pain") \
    _F(MTXT_MONSTER_AIM, "Monster Aim") \
    _F(MTXT_PISTOL_START, "Pistol Start") \
    _F(MTXT_PERMA_DEATH, "Perma-Death") \
    \
    _F(MTXT_FEATURES, "Features") \
    _F(MTXT_WARP, "WARP TO LEVEL") \
    _F(MTXT_INVULNERABLE, "INVULNERABLE") \
    _F(MTXT_FLY, "FLY MODE") \
    _F(MTXT_KEYS, "SECURITY KEYS") \
    _F(MTXT_WEAPONS, "WEAPONS") \
    _F(MTXT_ARTIFACTS, "ARTIFACTS") \
    _F(MTXT_WALL_BLOCKING, "WALL BLOCKING") \
    _F(MTXT_LOCK_MONSTERS, "LOCK MONSTERS") \
    _F(MTXT_MAP_EVERYTHING, "MAP EVERYTHING") \
    _F(MTXT_HEALTH_BOOST, "HEALTH BOOST") \
    _F(MTXT_MUSIC_TEST, "MUSIC TEST") \
    _F(MTXT_RECORD_DEMO, "Record Demo") \
    _F(MTXT_CREDITS, "Show Credits") \
    \
    _F(MTXT_DEBUG, "Debug") \
    _F(MTXT_DEBUG_DISPLAY, "DISPLAY") \
    _F(MTXT_SECTOR_COLORS, "SECTOR COLORS") \
    _F(MTXT_FULL_BRIGHT, "FULL BRIGHT") \
    \
    _F(MTXT_OPTIONS, "Options") \
    \
    _F(MTXT_PLAYER, "Player") \
    _F(MTXT_MOVE_SENSITIVITY, "Move Sensitivity") \
    _F(MTXT_LOOK_SENSITIVITY, "Look Sensitivity") \
    _F(MTXT_VERTICAL_LOOK, "Vertical Look") \
    _F(MTXT_LOOK_SPRING, "Look Spring") \
    _F(MTXT_CROSSHAIR, "Crosshair") \
    _F(MTXT_AUTORUN, "Auto Run") \
    _F(MTXT_AUTOAIM, "Auto Aim") \
    _F(MTXT_PLAYER_COLOR, "Change Player Color") \
    _F(MTXT_CONTROLLER, "Configure Controller") \
    _F(MTXT_CONTROLLER_2, "Configure Controller 2") \
    \
    _F(MTXT_PLAYER_ROTATE, NULL) \
    _F(MTXT_PRESET_COLOR, "Preset") \
    _F(MTXT_HUE, "Hue") \
    _F(MTXT_SATURATION, "Saturation") \
    _F(MTXT_VALUE, "Value") \
    \
    _F(MTXT_SOUND, "Sound") \
    _F(MTXT_MUSIC_VOLUME, "Music Volume") \
    _F(MTXT_EFFECT_VOLUME, "Effect Volume") \
    \
    _F(MTXT_VIDEO, "Video") \
    _F(MTXT_RESOLUTION, "Resolution") \
    _F(MTXT_COLOR_DEPTH, "Color Depth") \
    _F(MTXT_ASPECT_RATIO, "Aspect Ratio") \
    _F(MTXT_GAMMA_CORRECT, "Gamma Correct") \
    _F(MTXT_DITHER_FILTER, "Dither Filter") \
    _F(MTXT_COLOR_DITHER, "Color Dither") \
    _F(MTXT_RESAMPLING, "Screen Filter") \
    _F(MTXT_ANTIALIASING, "Anti-Aliasing") \
    _F(MTXT_INTERLACING, "Interlacing") \
    _F(MTXT_CENTER_DISPLAY, "Center Display") \
    \
    _F(MTXT_DISPLAY, "Display") \
    _F(MTXT_BRIGHTNESS, "Brightness") \
    _F(MTXT_FLASH_BRIGHTNESS, "Flash Brightness") \
    _F(MTXT_TEXTURE_FILTER, "Texture Filter") \
    _F(MTXT_SPRITE_FILTER, "Sprite Filter") \
    _F(MTXT_SKY_FILTER, "Sky Filter") \
    _F(MTXT_BLOOD_COLOR, "Blood Color") \
    \
    _F(MTXT_HUD, "HUD") \
    _F(MTXT_MARGIN, "HUD Margin") \
    _F(MTXT_OPACITY, "HUD Opacity") \
    _F(MTXT_MOTION_BOB, "Motion Bob") \
    _F(MTXT_TEXT_COLORS, "Text Colors") \
    _F(MTXT_MESSAGES, "Messages") \
    _F(MTXT_MAP_STATS, "Map Stats") \
    _F(MTXT_STORY_TEXT, "Story Text") \
    \
    _F(MTXT_DEFAULTS, "Defaults") \
    _F(MTXT_PRESET_MODERN, "Modern") \
    _F(MTXT_PRESET_VANILLA, "Vanilla") \
    _F(MTXT_PRESET_MERCILESS, "Merciless") \
    _F(MTXT_PRESET_RETRO, "Retro") \
    _F(MTXT_PRESET_ACCESSIBLE, "Accessible") \

#define _F(_id, _s) _id,
typedef enum { MENU_STRINGS } menuentry_t;
#undef _F

#define _F(_id, _s) _s,
const char *MenuText[] = { MENU_STRINGS };
#undef _F

const menuitem_t Menu_Title[] =
{
    { MTXT_NEW_GAME,  115, 170 },
    { MTXT_LOAD_GAME, 115, 190 },
    { MTXT_OPTIONS,   115, 210 },
};

const menuitem_t Menu_Skill[] =
{
    { MTXT_SKILL1,       102, 70 },
    { MTXT_SKILL2,       102, 90},
    { MTXT_SKILL3,       102, 110},
    { MTXT_SKILL4,       102, 130},
    { MTXT_SKILL5,       102, 150},
    { MTXT_CUSTOM_SKILL, 102, 170},
    { MTXT_RETURN,       102, 200},
};

const menuitem_t Menu_Custom_Skill[] =
{
    { MTXT_START_GAME,         32, 50},
    { MTXT_PRESET,             32, 60},

    { MTXT_PLAYER_DAMAGE,      32, 80},
    { MTXT_PLAYER_AMMO,        32, 90},
    { MTXT_MONSTER_COUNTS,     32, 100},
    { MTXT_MONSTER_SPEED,      32, 110},
    { MTXT_MONSTER_RESPAWNS,   32, 120},
    { MTXT_MONSTER_INFIGHTING, 32, 130},
    { MTXT_MONSTER_REACTIONS,  32, 140},
    { MTXT_MONSTER_COLLISION,  32, 150},
    { MTXT_MONSTER_PAIN,       32, 160},
    { MTXT_MONSTER_AIM,        32, 170},
    { MTXT_PISTOL_START,       32, 180},
    { MTXT_PERMA_DEATH,        32, 190},

    { MTXT_MRETURN,            32, 210},
};

const menuitem_t Menu_Options[] =
{
    { MTXT_PLAYER,   112, 60 },
    { MTXT_DISPLAY,  112, 80 },
    { MTXT_SOUND,    112, 100},
    { MTXT_VIDEO,    112, 120},
    { MTXT_HUD,      112, 140},
    { MTXT_DEFAULTS, 112, 160},
    { MTXT_RETURN,   112, 190},
};

const menuitem_t Menu_Volume[] =
{
    { MTXT_MUSIC_VOLUME,  82, 60 },
    { MTXT_EFFECT_VOLUME, 82, 100},
    { MTXT_RETURN,        82, 140},
};

const menuitem_t Menu_Player[] =
{
    { MTXT_MOVE_SENSITIVITY,   42, 60},
    { MTXT_LOOK_SENSITIVITY,   42, 70},
    { MTXT_VERTICAL_LOOK,      42, 80},
    { MTXT_LOOK_SPRING,        42, 90},
    { MTXT_CROSSHAIR,          42, 100},
    { MTXT_AUTORUN,            42, 110},
    { MTXT_AUTOAIM,            42, 120},
    { MTXT_CONTROLLER,         42, 140},
    { MTXT_CONTROLLER_2,       42, 150},
    { MTXT_PLAYER_COLOR,       42, 170},
    { MTXT_MRETURN,            42, 190},
};

const menuitem_t Menu_PlayerColor[] =
{
    { MTXT_PLAYER_ROTATE, 0, 0},
    { MTXT_PRESET_COLOR,  52, 160},
    { MTXT_HUE,           52, 170},
    { MTXT_SATURATION,    52, 180},
    { MTXT_VALUE,         52, 190},
    { MTXT_MRETURN,       52, 210},
};

const menuitem_t Menu_Video[] =
{
    { MTXT_RESOLUTION,       42, 60},
    { MTXT_COLOR_DEPTH,      42, 70},
    { MTXT_ASPECT_RATIO,     42, 80},
    { MTXT_GAMMA_CORRECT,    42, 90},
    { MTXT_DITHER_FILTER,    42, 100},
    { MTXT_COLOR_DITHER,     42, 110},
    { MTXT_RESAMPLING,       42, 120},
    { MTXT_ANTIALIASING,     42, 130},
    { MTXT_INTERLACING,      42, 140},
    { MTXT_CENTER_DISPLAY,   42, 150},
    { MTXT_MRETURN,          42, 170},
};

const menuitem_t Menu_Display[] =
{
    { MTXT_BRIGHTNESS,       42, 60},
    { MTXT_FLASH_BRIGHTNESS, 42, 70},
    { MTXT_TEXTURE_FILTER,   42, 80},
    { MTXT_SPRITE_FILTER,    42, 90},
    { MTXT_SKY_FILTER,       42, 100},
    { MTXT_BLOOD_COLOR,      42, 110},
    { MTXT_MRETURN,          42, 130},
};

const menuitem_t Menu_StatusHUD[] =
{
    { MTXT_MARGIN,      62, 60},
    { MTXT_OPACITY,     62, 70},
    { MTXT_MOTION_BOB,  62, 80},
    { MTXT_TEXT_COLORS, 62, 90},
    { MTXT_MESSAGES,    62, 100},
    { MTXT_MAP_STATS,   62, 110},
    { MTXT_STORY_TEXT,  62, 120},
    { MTXT_MRETURN,     62, 140},
};

const menuitem_t Menu_Defaults[] =
{
    { MTXT_PRESET_MODERN,     102, 60},
    { MTXT_PRESET_VANILLA,    102, 80},
    { MTXT_PRESET_MERCILESS,  102, 100},
    { MTXT_PRESET_RETRO,      102, 120},
    { MTXT_PRESET_ACCESSIBLE, 102, 140},
    { MTXT_RETURN,            102, 170},
};

menuitem_t Menu_Game[] =
{
    { MTXT_OPTIONS,    122, 60},
    { MTXT_QUICK_SAVE, 122, 78},
    { MTXT_QUICK_LOAD, 122, 96},
    { MTXT_LOAD_GAME,  122, 114},
    { MTXT_MAIN_MENU,  122, 132},
    { MTXT_RESTART,    122, 150},
    { MTXT_FEATURES,     122, 168},
};

const menuitem_t Menu_Quit[] =
{
    { MTXT_YES, 142, 100},
    { MTXT_NO,  142, 120},
};

const menuitem_t Menu_DeleteNote[] =
{
    { MTXT_YES, 142, 100},
    { MTXT_NO,  142, 120},
};

menuitem_t Menu_Load[] =
{
    { MTXT_LOAD_GAME_PAK,       122, 60 },
    { MTXT_LOAD_CONTROLLER_PAK, 122, 80 },
    { MTXT_LOAD_PASSWORD,       122, 100 },
    { MTXT_LOAD_QUICK_LOAD,     122, 120 },
};

const menuitem_t Menu_Save[] =
{
    { MTXT_SAVE_GAME_PAK,       122, 60 },
    { MTXT_SAVE_CONTROLLER_PAK, 122, 80 },
    { MTXT_DONT_SAVE,           122, 100 },
};

const menuitem_t Menu_ControllerPakBad[] =
{
    { MTXT_TRY_AGAIN,    120, 100},
    { MTXT_DONT_USE_PAK, 120, 120},
};

const menuitem_t Menu_ControllerPakFull[] =
{
    { MTXT_MANAGE_PAK,       110, 90 },
    { MTXT_CREATE_GAME_NOTE, 110, 110},
    { MTXT_DONT_USE_PAK,     110, 130},
};

const menuitem_t Menu_CreateNote[] =
{
    { MTXT_YES,          110, 90 },
    { MTXT_DONT_USE_PAK, 110, 110},
    { MTXT_MANAGE_PAK,   110, 130},
};

const menuitem_t Menu_Features[] =
{
    { MTXT_INVULNERABLE,   40, 50},
    { MTXT_FLY,            40, 60},
    { MTXT_WEAPONS,        40, 70},
    { MTXT_ARTIFACTS,      40, 80},
    { MTXT_KEYS,           40, 90},
    { MTXT_MAP_EVERYTHING, 40, 100},
    { MTXT_WALL_BLOCKING,  40, 110},
    { MTXT_LOCK_MONSTERS,  40, 120},
    { MTXT_HEALTH_BOOST,   40, 130},
    { MTXT_WARP,           40, 140},
    { MTXT_MUSIC_TEST,     40, 150},

    { MTXT_CREDITS,        40, 170},
    { MTXT_DEBUG,          40, 180},

    { MTXT_MRETURN,        40, 200},
};

static const menuitem_t Menu_Debug[] =
{
    { MTXT_DEBUG_DISPLAY, 40, 120},
    { MTXT_SECTOR_COLORS, 40, 130},
    { MTXT_FULL_BRIGHT,   40, 140},

    { MTXT_RECORD_DEMO,   40, 160},

    { MTXT_MRETURN,       40, 180},
};

typedef struct
{
    char *text;
    int x;
    int y;
} credit_t;

static const credit_t Ultra_Credits[] = {
    {"PROGRAMMING          IMMORPHER",   20, 48},
    {                     "JNMARTIN84", 188, 58},
    {                     "NOVA",       188, 68}
};

static const credit_t Merciless_Credits[] =
{
    {"PROGRAMMING          IMMORPHER",     20, 48},

    {"REVERSE ENGINEERING  ERICK194",      20, 65},
    {                     "KAISER",       188, 75},
    {                     "BODB DEARG",   188, 85},
    {                     "QUASAR",       188, 95},

    {"COMPILER ASSETS      CRASHOVERIDE",  20, 112},
    {                     "ALPHATANGO",   188, 122},

    {"PLAY TESTING",              -1, 139},
    {"BUU342, IRL RANDOM HAJILE", -1, 151},
    {"SCD, TAUFAN99",             -1, 161},

    {"SPECIAL THANKS",                       -1, 178},
    {"GEC TEAM, DOOMWORLD, DOOM 64 DISCORD", -1, 190},
    {"NEIGH WINNY, ISANN KEKET, NEVANDER",   -1, 200},
};

static const char *ControlText[] =   //8007517C
{
    "default  %s",
    "  stick ",

    "move forward",
    "move backward",
    "strafe left",
    "strafe right",
    "strafe on",
    "attack",
    "run",
    "jump",
    "crouch",

    "turn left",
    "turn right",
    "look up",
    "look down",
    "look on",
    "use",
    "weapon up",
    "weapon down",
    "map"
};

static const u8 ControlMappings[] = {
    2,  3,  9, 10,  8,  4,  7, 16, 17,
    1,  0, 14, 15,  13, 5, 12, 11, 6
};

static const struct {
    const char *name;
    u8 hsv[3];
} PlayerColorPresets[] = {
    { "Green",  { 84,  159, 52  } },
    { "Yellow", { 24,  252, 52  } },
    { "Red",    { 236, 252, 36  } },
    { "Blue",   { 148, 192, 36  } },
    { "Violet", { 192, 252, 52  } },
    { "Brown",  { 4,   98,  60  } },
    { "Teal",   { 116, 164, 136 } },
    { "Gray",   { 0,   0,   24  } },
};

//-----------------------------------------

#ifndef DEBUG_DISPLAY
#define DEBUG_DISPLAY 0
#endif

#ifdef NDEBUG
#define MAXDEBUGCOUNTERS 2
#else
#define MAXDEBUGCOUNTERS 8
#endif

#define SET_MENU(_m) do { MenuItem = (_m); itemlines = ARRAYLEN(_m); } while(0)

//-----------------------------------------

gamesettings_t Settings;

static levelsave_t *GamePak_Data = NULL;
levelsave_t LevelSaveBuffer;
boolean doLoadSave = false;

//intermission
int DrawerStatus;

menudata_t MenuData[8]; // 800A54F0
int MenuAnimationTic;   // 800a5570
int cursorpos;          // 800A5574
int m_vframe1;          // 800A5578
const menuitem_t *MenuItem;   // 800A5578
int itemlines;          // 800A5580
menufunc_t MenuCall;    // 800A5584

int linepos;            // 800A5588
int text_alpha_change_value;    // 800A558C
int MusicID;            // 800A5590
int m_actualmap;        // 800A5594
int last_ticon;         // 800A5598

customskill_t startskill;     // 800A55A0
int startmap;           // 800A55A4
int EnableExpPak;       // 800A55A8

int MenuIdx = 0;                 // 8005A7A4
int text_alpha = 255;            // 8005A7A8
int ShowDebugCounters = DEBUG_DISPLAY; // [nova] debug counters
boolean runintroduction = false; // [Immorpher] New introduction sequence!

boolean ConfigChanged = false;

s8 SkillPreset = SKILL_DEFAULT;
static s8 skillpresetsetup;
static customskill_t skillsetup;
static u8 buttonbindstate;
static u8 configcontroller;
static u8 playerpreviewshoot;
static u8 playerpreviewrotate;
static s8 playercolorpreset;
static s8 lastsetdefaults;
static s16 logo_alpha;

typedef enum {
    copy_none,
    copy_game_pak,
    copy_controller_pak,
} copysource_t;

static u8 copysource = copy_none;
static u8 copyindex;
static u8 lastpaksave = 0;
static u8 lastgamepaksave = 0;

controls_t CurrentControls[MAXPLAYERS] ALIGNED(16);
controls2_t CurrentControls2[1];

//-----------------------------------------

int M_RunTitle(void) // 80007630
{
    int exit;

    DrawerStatus = 0;
    startskill = SkillPresets[1].skill;
    startmap = 1;
    MenuIdx = 0;
    SET_MENU(Menu_Title);
    MenuCall = M_MenuTitleDrawer;
    text_alpha = 0;
    cursorpos = 0;
    last_ticon = 0;
    logo_alpha = 80;

    S_StartMusic(mus_title);

    exit = MiniLoop(M_FadeInStart, M_MenuClearCall, M_MenuTicker, M_MenuGameDrawer);
    I_WIPE_FadeOutScreen();
    S_StopMusic();

    if (exit == ga_timeout)
        return exit;
    if (exit == ga_loadquicksave)
        gameaction = exit;

    G_InitNew(startskill, startmap, gt_single);
    G_RunGame();

    return ga_nothing;
}

int M_ControllerPak(void) // 80007724
{
    int exit;
    int ret;
    boolean PakBad;

    PakBad = false;

    while(1)
    {
        ret = I_CheckControllerPak();

        if ((ret != PFS_ERR_NOPACK) && (ret != PFS_ERR_ID_FATAL))
            PakBad = true;

        if(ret == 0)
        {
            ret = I_ReadPakFile();

            if(ret == 0)
            {
                exit = ga_nothing;
                break;
            }

            // Create Controller Pak Note
            SET_MENU(Menu_CreateNote);
            MenuCall = M_MenuTitleDrawer;
            cursorpos = 0;

            exit = MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);
            M_FadeOutStart(ga_exit);

            if (exit == ga_exit)
                return exit;

            // Check Memory and Files Used on Controller Pak
            if ((Pak_Memory > 0) && (FilesUsed != 16))
            {
                if (I_CreatePakFile() != 0)
                    goto ControllerPakBad;

                exit = ga_nothing;
                break;
            }

            // Show Controller Pak Full
            SET_MENU(Menu_ControllerPakFull);
            MenuCall = M_MenuTitleDrawer;
            cursorpos = 0;

            exit = MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);
            M_FadeOutStart(ga_exit);

            if (exit == ga_exit)
                return exit;
        }
        else
        {
            if (PakBad == false)
            {
                exit = ga_exit;
                break;
            }

            // Show Controller Pak Bad
        ControllerPakBad:
            SET_MENU(Menu_ControllerPakBad);
            MenuCall = M_MenuTitleDrawer;
            cursorpos = 0;

            exit = MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);
            M_FadeOutStart(ga_exit);

            if (exit == ga_exit)
                return exit;
        }
    }

    return exit;
}

void M_PauseMenu(void)
{
    MenuCall = M_MenuTitleDrawer;
    SET_MENU(Menu_Game);

    MenuIdx = 0;
    text_alpha = 255;
    MenuAnimationTic = 0;

    Menu_Game[1].casepos = I_IsQuickSaveAvailable() ? MTXT_QUICK_SAVE : MTXT_QUICK_SAVE_DISABLED;
    Menu_Game[2].casepos = I_IsQuickLoadAvailable() ? MTXT_QUICK_LOAD : MTXT_QUICK_LOAD_DISABLED;
}

static boolean M_AdjustLoadMenu(void)
{
    boolean pakpresent = I_CheckControllerPak() == 0;
    boolean quickloadpresent = !gamepaused && I_IsQuickLoadAvailable();
    menuentry_t casepos;
    int i;

    for (i = 0; i < ARRAYLEN(Menu_Load); i++)
    {
        casepos = Menu_Load[i].casepos;

        if (casepos == MTXT_LOAD_GAME_PAK || casepos == MTXT_GAME_PAK_DISABLED)
            Menu_Load[i].casepos = SramPresent ? MTXT_LOAD_GAME_PAK : MTXT_GAME_PAK_DISABLED;
        if (casepos == MTXT_LOAD_CONTROLLER_PAK || casepos == MTXT_CONTROLLER_PAK_DISABLED)
            Menu_Load[i].casepos = pakpresent ? MTXT_LOAD_CONTROLLER_PAK : MTXT_CONTROLLER_PAK_DISABLED;
        if (casepos == MTXT_LOAD_QUICK_LOAD || casepos == MTXT_QUICK_LOAD_DISABLED)
            Menu_Load[i].casepos = quickloadpresent ? MTXT_LOAD_QUICK_LOAD : MTXT_QUICK_LOAD_DISABLED;
    }

    return SramPresent || pakpresent || quickloadpresent;
}

static boolean M_ItemIsDisabled(menuentry_t casepos)
{
    if (!HAS_EXPANSION_PAK())
    {
        if (casepos == MTXT_COLOR_DEPTH || casepos == MTXT_RESOLUTION)
            return true;
    }
    if (casepos == MTXT_RESAMPLING)
    {
        if (VideoSettings.Resolution == VIDEO_RES_HI_VERT)
            return true;
        if (VideoSettings.Resolution == VIDEO_RES_HI_HORIZ && VideoSettings.BitDepth == BITDEPTH_32)
            return true;
    }
    return casepos == MTXT_CONTROLLER_PAK_DISABLED
        || casepos == MTXT_GAME_PAK_DISABLED
        || casepos == MTXT_QUICK_LOAD_DISABLED
        || casepos == MTXT_QUICK_SAVE_DISABLED
        || casepos == MTXT_GAME_SAVED;
}

static void M_AdjustCursorPos(int dir)
{
    int bound;

    dir = dir < 0 ? -1 : 1;
    bound = dir < 0 ? 0 : itemlines - 1;
    while (cursorpos != bound && M_ItemIsDisabled(MenuItem[cursorpos].casepos))
        cursorpos += dir;
    if (cursorpos == bound && M_ItemIsDisabled(MenuItem[cursorpos].casepos)) // wrap around
    {
        cursorpos = dir < 0 ? itemlines - 1 : 0;
        while (cursorpos != bound && M_ItemIsDisabled(MenuItem[cursorpos].casepos))
            cursorpos += dir;
    }
}

#define MAXSENSIVITY    40

int M_ButtonResponder(int buttons) // 80007960
{
    int sensitivity;
    int NewButtons;

    /* Copy Default Buttons */
    NewButtons = (buttons);

    /* Analyze Analog Stick (up / down) */
    sensitivity = STICK_Y(buttons);

    if (sensitivity <= -MAXSENSIVITY)
        NewButtons |= PAD_DOWN;
    else if (sensitivity >= MAXSENSIVITY)
        NewButtons |= PAD_UP;

    /* Analyze Analog Stick (left / right) */
    sensitivity = STICK_X(buttons);

    if (sensitivity <= -MAXSENSIVITY)
        NewButtons |= PAD_LEFT;
    else if (sensitivity >= MAXSENSIVITY)
        NewButtons |= PAD_RIGHT;

    return NewButtons & 0xffff0000;
}

static void M_LogoTicker(void)
{
    s16 logo_alpha_target;
    s8 logo_alpha_change_value;

    if (MenuItem == Menu_Title && MenuIdx == 0)
    {
        logo_alpha_change_value = 28;
        logo_alpha_target = 255;
    }
    else
    {
        logo_alpha_change_value = -28;
        logo_alpha_target = 32;
    }

    logo_alpha += (logo_alpha_change_value * vblsinframe[0]) >> 1;
    if ((logo_alpha_change_value < 0 && logo_alpha < logo_alpha_target)
            || (logo_alpha_change_value > 0 && logo_alpha > logo_alpha_target))
        logo_alpha = logo_alpha_target;
}

void M_AlphaInStart(void) // 800079E0
{
    text_alpha = 0;
    text_alpha_change_value = 32;
}

void M_AlphaOutStart(void) // 800079F8
{
    text_alpha = 255;
    text_alpha_change_value = -32;
}

int M_AlphaInOutTicker(void) // 80007A14
{
    if ((gamevbls < gametic) && ((gametic & 3U) == 0)) {
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;
    }

    M_LogoTicker();

    text_alpha += (text_alpha_change_value * vblsinframe[0]) >> 1;
    if (text_alpha_change_value < 0 && text_alpha < 0)
    {
        text_alpha = 0;
        return ga_exit;
    }
    else if (text_alpha_change_value > 0 && text_alpha >= 256)
    {
        text_alpha = 255;
        return ga_exit;
    }

    return ga_nothing;
}

void M_FadeInStart(void) // 80007AB4
{
    MiniLoop(M_AlphaInStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

void M_FadeOutStart(int exitmode) // 80007AEC
{
    if (ConfigChanged)
    {
        I_SaveConfig();
        ConfigChanged = false;
    }

    if (exitmode == ga_exit)
        MiniLoop(M_AlphaOutStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

void M_SaveMenuData(void) // 80007B2C
{
    menudata_t *mdat;

    assert(MenuIdx < ARRAYLEN(MenuData) - 1);

    // Save Actual Menu Page
    mdat = &MenuData[MenuIdx];
    MenuIdx += 1;

    mdat->menu_item  = MenuItem;
    mdat->item_lines = itemlines;
    mdat->menu_call  = MenuCall;
    mdat->cursor_pos = cursorpos;

    // Start Menu Fade Out
    MiniLoop(M_AlphaOutStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

void M_RestoreMenuData(boolean alpha_in) // 80007BB8
{
    menudata_t *mdat;

    assert(MenuIdx > 0);

    // Restore Previous Save Menu Page
    MenuIdx -= 1;
    mdat = &MenuData[MenuIdx];

    MenuItem  = mdat->menu_item;
    itemlines = mdat->item_lines;
    MenuCall  = mdat->menu_call;
    cursorpos = mdat->cursor_pos;

    if (MenuItem == Menu_Load && !M_AdjustLoadMenu())
    {
        M_RestoreMenuData(alpha_in);
        return;
    }

    // Start Menu Fade In
    if (alpha_in)
        MiniLoop(M_AlphaInStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

void ST_DrawDebug (void);

void M_MenuGameDrawer(void) // 80007C48
{
    if (DrawerStatus == 1) {
        P_Drawer();
    }
    else if (DrawerStatus == 2) {
        F_DrawerIntermission();
    }
    else if (DrawerStatus == 3) {
        F_Drawer();
    }
    else
    {
        I_ClearFrame();
        I_ClearFB(0x000000ff);

        if (MenuData[0].menu_item == Menu_Save)
            M_DrawBackground(63, 25, 128, "EVIL");
        else
            M_DrawBackground(56, 57, logo_alpha, "TITLE");

        ST_DrawDebug();
        MenuCall();
        I_DrawFrame();
    }
}

extern int globalcheats; // [GEC]

int M_QuickLoadFailedTicker(void) // 8002BA88
{
    if ((ticon - last_ticon) >= 60 && ((u32)allticbuttons >> 16) != 0)
        return ga_exit;

    return ga_nothing;
}

void M_QuickLoadFailedDrawer(void) // 8002BBE4
{
    I_ClearFrame();
    I_ClearFB(0x000000ff);

    ST_DrawString(-1,  40, "quick load data", 0xffffffff);
    ST_DrawString(-1,  60, "is corrupted.", 0xffffffff);

    if ((ticon - last_ticon) >= 60)
        ST_DrawString(-1, SCREEN_HT/2, "press any button to return.", 0xffffffff);

    I_DrawFrame();
}

SEC_MENU void M_QuickLoadFailed(void)
{
    last_ticon = 0;
    MiniLoop(NULL, NULL, M_QuickLoadFailedTicker, M_QuickLoadFailedDrawer);
    I_WIPE_FadeOutScreen();
}

int M_PasswordMenu()
{
    int exit;

    M_SaveMenuData();
    MenuCall = M_PasswordDrawer;
    exit = MiniLoop(M_PasswordStart,M_PasswordStop,M_PasswordTicker,M_MenuGameDrawer);
    M_RestoreMenuData(true);

    return exit;
}

bool M_IsPressed(int buttons, int oldbuttons, int mask)
{
    return !!(buttons & mask) && !(oldbuttons & mask);
}

static inline bool M_IsHeldNoVert(int buttons, int mask)
{
    return !!(buttons & mask) && !(buttons & (PAD_UP|PAD_DOWN));
}

#define IS_PRESSED(mask) M_IsPressed(buttons, oldbuttons, (mask))
#define IS_HELD(mask) M_IsHeldNoVert(buttons, (mask))

static bool M_ModifySlider(int *value, int step, int max, int buttons, int oldbuttons)
{
    if (IS_HELD(PAD_RIGHT))
    {
        *value += step;
        if (*value <= max)
        {
            if (*value & step)
                S_StartSound(NULL, sfx_secmove);
            ConfigChanged = true;
            return true;
        }
        else
        {
            *value = max;
        }
    }
    else if (IS_HELD(PAD_LEFT))
    {
        *value -= step;
        if (*value < 0)
        {
            *value = 0;
        }
        else
        {
            if (*value & step)
                S_StartSound(NULL, sfx_secmove);
            ConfigChanged = true;
            return true;
        }
    }
    return false;
}

#define CHANGESLIDER(ptr, step, max) M_ModifySlider((ptr), (step), (max), buttons, oldbuttons)
#define CHANGESLIDER8(ptr, step, max) ({ \
        typeof((ptr)) _ptr = (ptr); \
        int _ival = *(ptr); \
        bool _res = M_ModifySlider(&_ival, (step), (max), buttons, oldbuttons); \
        *_ptr = _ival; \
        _res; \
    })

int M_MenuTicker(void) // 80007E0C
{
    unsigned int buttons, oldbuttons;
    int exit;
    bool truebuttons, leftbutton, rightbutton;
    int i;
    menuentry_t casepos;
    boolean padrepeat = false;

    if (ConfigChanged)
    {
        I_SaveConfig();
        ConfigChanged = false;
    }

    M_LogoTicker();

    /* animate skull */
    if ((gamevbls < gametic) && ((gametic & 3U) == 0))
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;

    buttons = M_ButtonResponder(allticbuttons);
    oldbuttons = M_ButtonResponder(alloldticbuttons);

    /* exit menu if button press */
    if (buttons != 0)
        last_ticon = ticon;

    /* exit menu if time out */
    if ((MenuItem == Menu_Title) && ((ticon - last_ticon) >= 900)) // 30 * TICRATE
    {
        exit = ga_timeout;
    }
    else
    {
        /* check for movement */
        if (!(buttons & (PAD_Z_TRIG|ALL_JPAD)))
            m_vframe1 = 0;
        else
        {
            m_vframe1 = m_vframe1 - vblsinframe[0];
            if (m_vframe1 <= 0)
            {
                m_vframe1 = 0xf; // TICRATE / 2
                padrepeat = true;

                if (buttons & PAD_DOWN)
                {
                    cursorpos += 1;

                    if (cursorpos >= itemlines)
                        cursorpos = 0;

                    M_AdjustCursorPos(1);

                    S_StartSound(NULL, sfx_switch1);
                }
                else if (buttons & PAD_UP)
                {
                    cursorpos -= 1;

                    if (cursorpos < 0)
                        cursorpos = itemlines-1;

                    M_AdjustCursorPos(-1);

                    S_StartSound(NULL, sfx_switch1);
                }
            }
        }

        if (playerpreviewshoot)
            playerpreviewshoot--;

        if (IS_PRESSED(PAD_START) || IS_PRESSED(PAD_B))
        {
            if (MenuItem == Menu_Title || (MenuItem == Menu_Save && MenuIdx == 0))
            {
                return ga_nothing;
            }
            else
            {
                if (MenuIdx != 0)
                    S_StartSound(NULL, sfx_pistol);

                return ga_exit;
            }
        }
        else
        {
            truebuttons = IS_PRESSED(PAD_A);
            if (buttons & (PAD_UP|PAD_DOWN))
            {
                leftbutton = 0;
                rightbutton = 0;
            }
            else
            {
                leftbutton = IS_PRESSED(PAD_LEFT);
                rightbutton = IS_PRESSED(PAD_RIGHT);
            }
            casepos = (menuentry_t) MenuItem[cursorpos].casepos;

            switch(casepos)
            {

            case MTXT_SOUND:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    SET_MENU(Menu_Volume);
                    MenuCall = M_VolumeDrawer;
                    cursorpos = 0;

                    MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                    M_RestoreMenuData(true);
                    return ga_nothing;
                }
                break;

            case MTXT_VIDEO:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    SET_MENU(Menu_Video);
                    MenuCall = M_VideoDrawer;
                    cursorpos = 0;

                    MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                    M_RestoreMenuData(true);
                    return ga_nothing;
                }
                break;

            case MTXT_MAIN_MENU:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    SET_MENU(Menu_Quit);
                    MenuCall = M_MenuTitleDrawer;
                    cursorpos = 1;

                    exit = MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                    M_RestoreMenuData((exit == ga_exit));
                    if (exit == ga_exit) {
                        return ga_nothing;
                    }

                    return ga_exitdemo;
                }
                break;

            case MTXT_RESTART:
                if (truebuttons)
                {

                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    skillsetup = customskill;
                    M_UpdateSkillPreset();
                    if (SkillPreset >= 0 && SkillPreset <= 4)
                    {
                        SET_MENU(Menu_Skill);
                        MenuCall = M_MenuTitleDrawer;
                        cursorpos = SkillPreset;  // Set default to current difficulty
                    }
                    else
                    {
                        SET_MENU(Menu_Custom_Skill);
                        MenuCall = M_CustomSkillDrawer;
                        cursorpos = 1;
                    }

                    exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);

                    if (exit == ga_exit) {
                        M_RestoreMenuData(true);
                        return ga_nothing;
                    }

                    SkillPreset = skillpresetsetup;
                    startskill = customskill = skillsetup;
                    startmap = gamemap;
                    G_InitSkill (customskill); // [Immorpher] initialize new skill

                    return ga_warped;
                }
                break;

            case MTXT_RETURN:
            case MTXT_MRETURN:
            case MTXT_DONT_SAVE:
            case MTXT_NO:
            case MTXT_DONT_USE_PAK:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    return ga_exit;
                }
                break;

            case MTXT_MUSIC_VOLUME:
                if (CHANGESLIDER(&Settings.MusVolume, 1, 100))
                    S_SetMusicVolume(Settings.MusVolume);
                break;

            case MTXT_EFFECT_VOLUME:
                if (CHANGESLIDER(&Settings.SfxVolume, 1, 100))
                    S_SetSoundVolume(Settings.SfxVolume);
                break;

            case MTXT_DISPLAY:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    SET_MENU(Menu_Display);
                    MenuCall = M_DisplayDrawer;
                    cursorpos = 0;

                    MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
                    M_RestoreMenuData(true);

                    return ga_nothing;
                }
                break;

            case MTXT_BRIGHTNESS:
                if (CHANGESLIDER(&Settings.Brightness, 8, 800))
                    P_RefreshBrightness();
                break;

            case MTXT_OPTIONS:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    SET_MENU(Menu_Options);
                    MenuCall = M_MenuTitleDrawer;
                    cursorpos = 0;

                    exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
                    M_RestoreMenuData((exit == ga_exit));

                    if (exit == ga_exit)
                        return ga_nothing;

                    return exit;
                }
                break;

            case MTXT_AUTORUN:
                if (truebuttons || leftbutton || rightbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    playerconfigs[0].autorun ^= true;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_DEFAULTS:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    SET_MENU(Menu_Defaults);
                    MenuCall = M_DefaultsDrawer;
                    cursorpos = 0;
                    lastsetdefaults = -1;

                    MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                    M_RestoreMenuData(true);
                    return ga_nothing;
                }
                break;

            case MTXT_NEW_GAME:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);

                    M_SaveMenuData();

                    skillsetup = customskill;
                    M_UpdateSkillPreset();
                    SET_MENU(Menu_Skill);
                    MenuCall = M_MenuTitleDrawer;
                    cursorpos = SKILL_DEFAULT;

                    exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);

                    if (exit == ga_exit) {
                        M_RestoreMenuData(true);
                        return ga_nothing;
                    }


                    nextmap = 1; // [Immorpher] For running introduction text"
                    runintroduction = true; // [Immorpher] turn introduction on

                    SkillPreset = skillpresetsetup;
                    startskill = skillsetup;

                    if (!SramPresent)
                    {
                        // Check ControllerPak
                        EnableExpPak = (M_ControllerPak() == 0);

                        I_FreePakData();
                    }

                    return ga_exit;
                }
                break;

            case MTXT_SKILL1:
            case MTXT_SKILL2:
            case MTXT_SKILL3:
            case MTXT_SKILL4:
            case MTXT_SKILL5:

                if (truebuttons)
                {
                    skillsetup = SkillPresets[casepos - MTXT_SKILL1].skill;
                    S_StartSound(NULL, sfx_pistol);
                    return ga_warped;
                }
                break;

            case MTXT_CUSTOM_SKILL:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);

                    M_SaveMenuData();

                    SET_MENU(Menu_Custom_Skill);
                    MenuCall = M_CustomSkillDrawer;
                    cursorpos = 1;
                    linepos = 0;

                    exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);

                    if (exit == ga_exit) {
                        M_RestoreMenuData(true);
                        return ga_nothing;
                    }
                    if (exit == ga_warped)
                        return exit;
                }
                break;
            case MTXT_PRESET:
                if (truebuttons || rightbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillpresetsetup == ARRAYLEN(SkillPresets) - 1)
                        skillpresetsetup = 0;
                    else
                        skillpresetsetup++;
                    skillsetup = SkillPresets[skillpresetsetup].skill;
                    return ga_nothing;
                }
                if (leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillpresetsetup <= 0)
                        skillpresetsetup = ARRAYLEN(SkillPresets) - 1;
                    else
                        skillpresetsetup--;
                    skillsetup = SkillPresets[skillpresetsetup].skill;
                    return ga_nothing;
                }
                break;
            case MTXT_PLAYER_DAMAGE:
                if (truebuttons || rightbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.player_damage == 3)
                        skillsetup.player_damage = 0;
                    else
                        skillsetup.player_damage++;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                if (leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.player_damage == 0)
                        skillsetup.player_damage = 3;
                    else
                        skillsetup.player_damage--;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_PLAYER_AMMO:
                if (truebuttons || rightbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.player_ammo == 2)
                        skillsetup.player_ammo = 0;
                    else
                        skillsetup.player_ammo++;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                if (leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.player_ammo == 0)
                        skillsetup.player_ammo = 2;
                    else
                        skillsetup.player_ammo--;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_MONSTER_COUNTS:
                if (truebuttons || rightbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.monster_counts == 3)
                        skillsetup.monster_counts = 0;
                    else
                        skillsetup.monster_counts++;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                if (leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.monster_counts == 0)
                        skillsetup.monster_counts = 3;
                    else
                        skillsetup.monster_counts--;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_MONSTER_SPEED:
                if (truebuttons || rightbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.monster_speed == 2)
                        skillsetup.monster_speed = 0;
                    else
                        skillsetup.monster_speed++;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                if (leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.monster_speed == 0)
                        skillsetup.monster_speed = 2;
                    else
                        skillsetup.monster_speed--;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_MONSTER_RESPAWNS:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    skillsetup.monster_respawns ^= 1;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_MONSTER_INFIGHTING:
                if (truebuttons || rightbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.monster_infighting == 2)
                        skillsetup.monster_infighting = 0;
                    else
                        skillsetup.monster_infighting++;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                if (leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.monster_infighting == 0)
                        skillsetup.monster_infighting = 2;
                    else
                        skillsetup.monster_infighting--;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_MONSTER_REACTIONS:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    skillsetup.monster_reactions ^= 1;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_MONSTER_COLLISION:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    skillsetup.monster_shrink ^= 1;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_MONSTER_PAIN:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    skillsetup.monster_reduced_pain ^= 1;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_MONSTER_AIM:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    skillsetup.monster_random_aim ^= 1;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_PISTOL_START:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    skillsetup.pistol_start ^= 1;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_PERMA_DEATH:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    skillsetup.permadeath ^= 1;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_START_GAME:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    return ga_warped;
                }
                break;

            case MTXT_YES:
            case MTXT_TRY_AGAIN:
            case MTXT_CREATE_GAME_NOTE:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    return ga_exitdemo;
                }
                break;
            case MTXT_LOAD_GAME:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);

                    if (!M_AdjustLoadMenu())
                    {
                        exit = M_PasswordMenu();
                    }
                    else
                    {
                        M_SaveMenuData();
                        SET_MENU(Menu_Load);
                        itemlines = 3;
                        MenuCall = M_MenuTitleDrawer;
                        cursorpos = 0;
                        M_AdjustCursorPos(0);
                        exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
                        if (exit == ga_exit)
                            M_RestoreMenuData(true);
                    }

                    if (exit == ga_exit)
                        return ga_nothing;

                    return exit;
                }
                break;

            case MTXT_FEATURES:
                if (truebuttons)
                {
                    int cheats;

                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    cheats = players[0].cheats & (CF_WEAPONS|CF_HEALTH|CF_ALLKEYS);
                    players[0].cheats &= ~(CF_WEAPONS|CF_HEALTH|CF_ALLKEYS);

                    SET_MENU(Menu_Features);
                    MenuCall = M_FeaturesDrawer;
                    cursorpos = 0;
                    m_actualmap = gamemap;

                    exit = MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                    M_RestoreMenuData((exit == ga_exit));
                    if ((players[0].cheats & CF_HEALTH) && players[0].playerstate == PST_DEAD)
                        players[0].playerstate = PST_LIVE;
                    players[0].cheats |= cheats;

                    if (exit == ga_exit)
                        return ga_nothing;

                    return exit;
                }
                break;

            case MTXT_WARP:
                if (padrepeat)
                {
                    if (IS_HELD(PAD_LEFT))
                    {
                        m_actualmap -= 1;
                        if (m_actualmap < 1)
                        {
                            m_actualmap = 1;
                        }
                        else
                        {
                            S_StartSound(NULL, sfx_switch2);
                        }
                        return ga_nothing;
                    }
                    else if (IS_HELD(PAD_RIGHT))
                    {
                        m_actualmap += 1;
                        if (m_actualmap > 32)
                        {
                            m_actualmap = 32;
                        }
                        else
                        {
                            S_StartSound(NULL, sfx_switch2);
                        }
                        return ga_nothing;
                    }
                }
                if (truebuttons)
                {
                    gamemap = m_actualmap;
                    startmap = m_actualmap;
                    return ga_warped;
                }
                break;

            case MTXT_INVULNERABLE:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    players[0].cheats ^= CF_GODMODE;
                    return ga_nothing;
                }
                break;

            case MTXT_FLY:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    players[0].cheats ^= CF_FLYMODE;
                    return ga_nothing;
                }
                break;

            case MTXT_KEYS:
                /* Not available in the release code */
                /*
                Reconstructed code based on Psx Doom
                */
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    players[0].cheats |= CF_ALLKEYS;

                    P_GiveAllKeys(&players[0]);

                    return ga_nothing;
                }
                break;

            case MTXT_WEAPONS:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    players[0].cheats |= CF_WEAPONS;

                    P_GiveAllWeapons(&players[0]);

                    return ga_nothing;
                }
                break;

            case MTXT_DEBUG:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    SET_MENU(Menu_Debug);
                    MenuCall = M_DebugMenuDrawer;
                    cursorpos = 0;

                    exit = MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                    M_RestoreMenuData((exit == ga_exit));

                    if (exit == ga_exit)
                        return ga_nothing;

                    return exit;
                }
                break;

            case MTXT_DEBUG_DISPLAY:
                if (truebuttons || rightbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    ShowDebugCounters += 1;
                    if (ShowDebugCounters > MAXDEBUGCOUNTERS)
                        ShowDebugCounters = 0;
                    return ga_nothing;
                }
                if (leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    ShowDebugCounters -= 1;
                    if (ShowDebugCounters < 0)
                        ShowDebugCounters = MAXDEBUGCOUNTERS;
                    return ga_nothing;
                }
                break;

            case MTXT_WALL_BLOCKING:
                /* Not available in the release code */
                /*
                In my opinion it must have been the NOCLIP cheat code
                */
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    players[0].cheats ^= CF_WALLBLOCKING;
                    players[0].mo->flags ^= MF_NOCLIP;
                    return ga_nothing;
                }
                break;

            case MTXT_CENTER_DISPLAY:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    MenuCall = M_CenterDisplayDrawer;

                    MiniLoop(M_FadeInStart,M_FadeOutStart,M_CenterDisplayTicker,M_MenuGameDrawer);
                    M_RestoreMenuData(true);

                    return ga_nothing;
                }
                break;

            case MTXT_MESSAGES:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    Settings.EnableMessages ^= true;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_OPACITY:
                CHANGESLIDER(&Settings.HudOpacity, 4, 255);
                break;

            case MTXT_LOCK_MONSTERS:
                /* Not available in the release code */
                /*
                Reconstructed code based on Doom 64 Ex
                */
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    players[0].cheats ^= CF_LOCKMONSTERS;
                    return ga_nothing;
                }
                break;

            case MTXT_RECORD_DEMO:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    return ga_recorddemo;
                }
                break;

            case MTXT_MAP_EVERYTHING:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    players[0].cheats ^= CF_ALLMAP;
                    return ga_nothing;
                }
                break;
            case MTXT_HEALTH_BOOST:
                if (truebuttons || rightbutton || leftbutton)
                {
                    players[0].cheats |= CF_HEALTH;
                    players[0].health = MAXHEALTH;
                    players[0].mo->health = MAXHEALTH;
                    if (players[0].playerstate == PST_DEAD)
                    {
                        players[0].viewheight = VIEWHEIGHT;
                        players[0].mo->flags
                            = (players[0].mo->flags|MF_SOLID|MF_SHOOTABLE)
                            & ~(MF_CORPSE|MF_DROPOFF);
                        switch (players[0].psprites[ps_weapon].state->sprite)
                        {
                            case SPR_SAWG: players[0].readyweapon = wp_chainsaw; break;
                            case SPR_PISG: players[0].readyweapon = wp_pistol; break;
                            case SPR_SHT1: players[0].readyweapon = wp_shotgun; break;
                            case SPR_SHT2: players[0].readyweapon = wp_supershotgun; break;
                            case SPR_CHGG: players[0].readyweapon = wp_chaingun; break;
                            case SPR_ROCK: players[0].readyweapon = wp_missile; break;
                            case SPR_PLAS: players[0].readyweapon = wp_plasma; break;
                            case SPR_BFGG: players[0].readyweapon = wp_bfg; break;
                            case SPR_LASR: players[0].readyweapon = wp_laser; break;
                            default: players[0].readyweapon = wp_fist; break;
                        }
                        P_SetupPsprites(0);
                        P_SetMobjState(players[0].mo, S_PLAY_RUN1);
                    }
                    return ga_nothing;
                }
                break;

            case MTXT_MUSIC_TEST:
                /* Not available in the release code */
                /*
                Reconstructed code in my interpretation
                */
                if (padrepeat)
                {
                    if (IS_HELD(PAD_LEFT))
                    {
                        MusicID -= 1;
                        if (MusicID > 0)
                        {
                            S_StartSound(NULL, sfx_switch2);
                            return ga_nothing;
                        }
                        MusicID = 1;
                    }
                    else if (IS_HELD(PAD_RIGHT))
                    {
                        MusicID += 1;
                        if (MusicID < 25)
                        {
                            S_StartSound(NULL, sfx_switch2);
                            return ga_nothing;
                        }
                        MusicID = 24;
                    }
                }
                if (truebuttons)
                {
                    S_StopMusic();
                    S_StartMusic(MusicID+(NUMSFX-1));
                    return ga_nothing;
                }
                break;

            case MTXT_PLAYER:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    SET_MENU(Menu_Player);
                    MenuCall = M_PlayerSetupDrawer;
                    cursorpos = 0;

                    MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
                    M_RestoreMenuData(true);

                    return ga_nothing;
                }
                break;

            case MTXT_LOOK_SENSITIVITY:
                CHANGESLIDER8(&playerconfigs[0].looksensitivity, 1, 100);
                break;

            case MTXT_MOVE_SENSITIVITY:
                CHANGESLIDER8(&playerconfigs[0].movesensitivity, 1, 100);
                break;

            case MTXT_MANAGE_PAK:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    MenuCall = M_ManagePakDrawer;
                    linepos = 0;
                    cursorpos = 0;

                    exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_ManagePakTicker, M_MenuGameDrawer);
                    M_RestoreMenuData((exit == ga_exit));

                    if (exit == ga_exit)
                        return ga_nothing;

                    return exit;
                }
                break;

            case MTXT_SECTOR_COLORS:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    players[0].cheats ^= CF_NOCOLORS;
                    globalcheats ^= CF_NOCOLORS;
                    P_RefreshBrightness();
                    return ga_nothing;
                }
                break;

            case MTXT_FULL_BRIGHT:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    players[0].cheats ^= CF_FULLBRIGHT;
                    globalcheats ^= CF_FULLBRIGHT;
                    P_RefreshBrightness();
                    return ga_nothing;
                }
                break;

            case MTXT_TEXTURE_FILTER:
            case MTXT_SPRITE_FILTER:
            case MTXT_SKY_FILTER:
                if (truebuttons || rightbutton || leftbutton)
                {
                    i = casepos - MTXT_TEXTURE_FILTER;
                    S_StartSound(NULL, sfx_switch2);
                    Settings.VideoFilters[i] ^= 1;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_BLOOD_COLOR:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    Settings.GreenBlood ^= 1;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_MOTION_BOB:
                CHANGESLIDER(&Settings.MotionBob, 0x8000, 0x100000);
                break;
            case MTXT_RESOLUTION:
                if (truebuttons || rightbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    VideoSettings.Resolution += 1;
                    if (VideoSettings.Resolution > 2)
                        VideoSettings.Resolution = 0;
                    I_BlankScreen(2);
                    I_RefreshVideo();
                    ConfigChanged = true;
                    return ga_nothing;
                }
                if (leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    VideoSettings.Resolution -= 1;
                    if (VideoSettings.Resolution < 0)
                        VideoSettings.Resolution = 2;
                    I_BlankScreen(2);
                    I_RefreshVideo();
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_COLOR_DEPTH:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    VideoSettings.BitDepth ^= 1;
                    I_BlankScreen(2);
                    I_RefreshVideo();
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_ASPECT_RATIO:
                if (truebuttons || rightbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    VideoSettings.ScreenAspect += 1;
                    if (VideoSettings.ScreenAspect > 2)
                        VideoSettings.ScreenAspect = 0;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                if (leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    VideoSettings.ScreenAspect -= 1;
                    if (VideoSettings.ScreenAspect < 0)
                        VideoSettings.ScreenAspect = 2;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_DITHER_FILTER:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    VideoSettings.DitherFilter ^= true;
                    I_RefreshVideo();
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_RESAMPLING:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    VideoSettings.TvMode ^= 1;
                    I_RefreshVideo();
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_ANTIALIASING:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    VideoSettings.AntiAliasing ^= 1;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_INTERLACING:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    VideoSettings.TvMode ^= 2;
                    I_RefreshVideo();
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_COLOR_DITHER:
                if (truebuttons || rightbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    Settings.ColorDither += 1;
                    if (Settings.ColorDither > 3)
                        Settings.ColorDither = 0;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                if (leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    Settings.ColorDither -= 1;
                    if (Settings.ColorDither < 0)
                        Settings.ColorDither = 3;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_FLASH_BRIGHTNESS:
                CHANGESLIDER(&Settings.FlashBrightness, 1, 32);
                break;

            case MTXT_PRESET_MODERN:
            case MTXT_PRESET_VANILLA:
            case MTXT_PRESET_MERCILESS:
            case MTXT_PRESET_RETRO:
            case MTXT_PRESET_ACCESSIBLE:
                if (IS_PRESSED(PAD_DOWN_C))
                {
                    const gamesettingspreset_t *preset;

                    S_StartSound(NULL, sfx_switch2);
                    preset = &SettingsPresets[casepos - MTXT_PRESET_MODERN];

                    Settings = preset->settings;
                    VideoSettings = preset->video;
                    for (int i = 0; i < MAXPLAYERS; i++)
                    {
                        CurrentControls[i] = ControllerPresets[Settings.ControlPreset[i]].ctrl;
                        playerconfigs[i] = preset->player;
                    }
                    CurrentControls2[0] = ControllerPresets[Settings.ControlPreset[0]].ctrl2;
                    P_RefreshBrightness();
                    I_RefreshVideo();
                    S_SetMusicVolume(Settings.MusVolume);
                    S_SetSoundVolume(Settings.SfxVolume);

                    lastsetdefaults = casepos;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_STORY_TEXT:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    Settings.StoryText ^= true;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_MAP_STATS:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    Settings.MapStats ^= true;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_HUD:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    SET_MENU(Menu_StatusHUD);
                    MenuCall = M_StatusHUDDrawer;
                    cursorpos = 0;

                    MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                    M_RestoreMenuData(true);
                    return ga_nothing;
                }
                break;

            case MTXT_MARGIN:
                CHANGESLIDER(&Settings.HudMargin, 1, 20);
                break;

            case MTXT_TEXT_COLORS:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    Settings.HudTextColors ^= true;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_GAMMA_CORRECT:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    VideoSettings.GammaCorrect = !VideoSettings.GammaCorrect;
                    I_RefreshVideo();
                    return ga_nothing;
                }
                break;

            case MTXT_CREDITS:
                if (truebuttons)
                {
                    static const menufunc_t cred_drawers[] = {
                        M_ModCredits1Drawer,
                        M_ModCredits2Drawer,
                        M_IdCreditsDrawer,
                        M_WmsCreditsDrawer,
                    };

                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();
                    exit = ga_nothing;
                    for (int i = 0; i < ARRAYLEN(cred_drawers); i++)
                    {
                        MenuCall = cred_drawers[i];
                        exit = MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuCreditsTicker,M_MenuGameDrawer);
                    }
                    M_RestoreMenuData(true);

                    if (exit == ga_exit)
                        return ga_nothing;

                    return exit;
                }
                break;

            case MTXT_ARTIFACTS:
                if (truebuttons || rightbutton || leftbutton)
                {
                    players[0].cheats |= CF_ARTIFACTS;
                    players[0].artifacts |= 1 | 2 | 4;

                    S_StartSound(NULL, sfx_switch2);
                    return ga_nothing;
                }
                break;
            case MTXT_CROSSHAIR:
                if (truebuttons || rightbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    playerconfigs[0].crosshair += 1;
                    if (playerconfigs[0].crosshair > 3)
                        playerconfigs[0].crosshair = 0;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                else if (leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    playerconfigs[0].crosshair -= 1;
                    if (playerconfigs[0].crosshair < 0)
                        playerconfigs[0].crosshair = 3;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;
            case MTXT_VERTICAL_LOOK:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    playerconfigs[0].verticallook
                        = playerconfigs[0].verticallook == 1 ? -1 : 1;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;
            case MTXT_LOOK_SPRING:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    playerconfigs[0].lookspring ^= 1;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;
            case MTXT_AUTOAIM:
                if (truebuttons || rightbutton || leftbutton)
                {
                    S_StartSound(NULL, sfx_switch2);
                    playerconfigs[0].autoaim ^= true;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;
            case MTXT_PLAYER_COLOR:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    SET_MENU(Menu_PlayerColor);
                    MenuCall = M_PlayerColorDrawer;
                    cursorpos = 1;
                    playerpreviewshoot = 0;
                    playerpreviewrotate = 0;
                    playercolorpreset = -1;
                    for (int i = 0; i < ARRAYLEN(PlayerColorPresets); i++)
                    {
                        if (PlayerColorPresets[i].hsv[0] == playerconfigs[0].hsv[0]
                                && PlayerColorPresets[i].hsv[1] == playerconfigs[0].hsv[1]
                                && PlayerColorPresets[i].hsv[2] == playerconfigs[0].hsv[2])
                        {
                            playercolorpreset = i;
                            break;
                        }
                    }

                    MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                    M_RestoreMenuData(true);
                    return ga_nothing;
                }
                break;
            case MTXT_CONTROLLER:
            case MTXT_CONTROLLER_2:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    MenuCall = M_ControlPadDrawer;
                    linepos = 0;
                    buttonbindstate = 0;
                    cursorpos = configcontroller = casepos - MTXT_CONTROLLER;

                    MiniLoop(M_FadeInStart,M_FadeOutStart,M_ControlPadTicker,M_MenuGameDrawer);
                    M_RestoreMenuData(true);
                    return ga_nothing;
                }
                break;
            case MTXT_PLAYER_ROTATE:
                if ((truebuttons || IS_PRESSED(PAD_Z_TRIG)) && !playerpreviewshoot)
                {
                    playerpreviewshoot = 12;
                    S_StartSound(NULL, sfx_shotgun);
                }
                if (leftbutton)
                    playerpreviewrotate = (playerpreviewrotate + 1) & 7;
                if (rightbutton)
                    playerpreviewrotate = (playerpreviewrotate - 1) & 7;
                break;
            case MTXT_PRESET_COLOR:
                if (truebuttons || rightbutton)
                {
                    playercolorpreset += 1;
                    if (playercolorpreset >= ARRAYLEN(PlayerColorPresets))
                        playercolorpreset = 0;
                    playerconfigs[0].hsv[0] = PlayerColorPresets[playercolorpreset].hsv[0];
                    playerconfigs[0].hsv[1] = PlayerColorPresets[playercolorpreset].hsv[1];
                    playerconfigs[0].hsv[2] = PlayerColorPresets[playercolorpreset].hsv[2];
                    S_StartSound(NULL, sfx_switch2);
                    R_UpdatePlayerPalette(0);
                    ConfigChanged = true;
                    return ga_nothing;
                }
                else if (leftbutton)
                {
                    playercolorpreset -= 1;
                    if (playercolorpreset < 0)
                        playercolorpreset = ARRAYLEN(PlayerColorPresets) - 1;
                    playerconfigs[0].hsv[0] = PlayerColorPresets[playercolorpreset].hsv[0];
                    playerconfigs[0].hsv[1] = PlayerColorPresets[playercolorpreset].hsv[1];
                    playerconfigs[0].hsv[2] = PlayerColorPresets[playercolorpreset].hsv[2];
                    S_StartSound(NULL, sfx_switch2);
                    R_UpdatePlayerPalette(0);
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;
            case MTXT_HUE:
            case MTXT_SATURATION:
            case MTXT_VALUE:
                {
                    int cur = playerconfigs[0].hsv[casepos - MTXT_HUE];
                    int next = cur;

                    if (IS_HELD(PAD_RIGHT))
                        next += 4;
                    else if (IS_HELD(PAD_LEFT))
                        next -= 4;

                    if (casepos == MTXT_HUE)
                        next = next & 0xff;
                    else
                        next = CLAMP(next, 0, 0xfc);

                    if (next != cur)
                    {
                        playerconfigs[0].hsv[casepos - MTXT_HUE] = next;
                        playercolorpreset = -1;
                        R_UpdatePlayerPalette(0);
                        S_StartSound(NULL, sfx_secmove);
                        ConfigChanged = true;
                        return ga_nothing;
                    }
                }
                break;
            case MTXT_QUICK_SAVE:
                if (IS_PRESSED(PAD_DOWN_C))
                {
                    if (I_IsQuickSaveAvailable())
                    {
                        S_StartSound(NULL, sfx_pistol);
                        I_QuickSave();
                        if (customskill.permadeath)
                            return ga_exitdemo;
                        Menu_Game[1].casepos = MTXT_GAME_SAVED;
                        Menu_Game[2].casepos = MTXT_QUICK_LOAD_DISABLED;
                    }
                    return ga_nothing;
                }
                break;
            case MTXT_QUICK_LOAD:
                if (IS_PRESSED(PAD_DOWN_C))
                    return ga_loadquicksave;
                break;
            case MTXT_LOAD_QUICK_LOAD:
                if (truebuttons)
                    return ga_loadquicksave;
                break;
            case MTXT_QUICK_LOAD_DISABLED:
            case MTXT_QUICK_SAVE_DISABLED:
            case MTXT_GAME_PAK_DISABLED:
            case MTXT_CONTROLLER_PAK_DISABLED:
            case MTXT_GAME_SAVED:
                break;
            case MTXT_LOAD_GAME_PAK:
            case MTXT_SAVE_GAME_PAK:
                if (truebuttons)
                {
                    exit = ga_nothing;
                    if (SramPresent)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();
                        if (casepos == MTXT_LOAD_GAME_PAK)
                        {
                            MenuCall = M_LoadGamePakDrawer;
                            exit = MiniLoop(M_LoadGamePakStart,M_LoadGamePakStop,M_LoadGamePakTicker,M_MenuGameDrawer);
                        }
                        else
                        {
                            MenuCall = M_SaveGamePakDrawer;
                            exit = MiniLoop(M_SaveGamePakStart,M_SaveGamePakStop,M_SaveGamePakTicker,M_MenuGameDrawer);
                        }
                        MenuCall = NULL;
                        if (exit == ga_exit || MenuData[0].menu_item != Menu_Save)
                            M_RestoreMenuData(exit == ga_exit);
                    }
                    if (exit == ga_exit)
                        exit = ga_nothing;

                    return exit;
                }
                break;
            case MTXT_LOAD_CONTROLLER_PAK:
            case MTXT_SAVE_CONTROLLER_PAK:
                if (truebuttons)
                {
                    exit = ga_nothing;

                    S_StartSound(NULL, sfx_pistol);

                    M_SaveMenuData();
                    EnableExpPak = (M_ControllerPak() == 0);

                    if (EnableExpPak)
                    {
                        EnableExpPak = 1;
                        if (casepos == MTXT_LOAD_CONTROLLER_PAK)
                        {
                            MenuCall = M_LoadPakDrawer;
                            exit = MiniLoop(M_LoadPakStart,M_LoadPakStop,M_LoadPakTicker,M_MenuGameDrawer);
                            MenuCall = NULL;
                        }
                        else
                        {
                            MenuCall = M_SavePakDrawer;
                            exit = MiniLoop(M_SavePakStart,M_SavePakStop,M_SavePakTicker,M_MenuGameDrawer);
                            MenuCall = NULL;
                        }
                    }

                    if (exit == ga_exit || MenuData[0].menu_item != Menu_Save)
                        M_RestoreMenuData(exit == ga_exit);

                    if (exit == ga_exit)
                        return ga_nothing;

                    return exit;
                }
                break;
            case MTXT_LOAD_PASSWORD:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    exit = M_PasswordMenu();
                    if (exit == ga_exit)
                        return ga_nothing;
                    return exit;
                }
                break;
            }
            exit = ga_nothing;
        }
    }

    return exit;
}

void M_MenuClearCall(int exit) // 80008E6C
{
    MenuCall = NULL;
}

void M_MenuTitleDrawer(void) // 80008E7C
{
    const menuitem_t *item;
    int i;
    int y;
    const char *title;

    if (MenuItem == Menu_Title)
    {
        D_DrawUltraTitle(20);
    }
    if (MenuItem == Menu_Game)
    {
        ST_DrawString(-1, 20, "Pause", text_alpha | 0xff000000);
        ST_DrawString(-1, SCREEN_HT - 30, "press \x8b to resume", text_alpha | 0xffffff00);
    }
    else if (MenuItem == Menu_Skill)
    {
        ST_DrawString(-1, 20, "Choose Your Skill...", text_alpha | 0xff000000);
    }
    else if (MenuItem == Menu_Options)
    {
        ST_DrawString(-1, 20, "Options", text_alpha | 0xff000000);
    }
    else if (MenuItem == Menu_Quit)
    {
        ST_DrawString(-1, 20, "Quit Game?", text_alpha | 0xff000000);
    }
    else if (MenuItem == Menu_DeleteNote)
    {
        ST_DrawString(-1, 20, "Delete Game Note?", text_alpha | 0xff000000);
    }
    else if (MenuItem == Menu_ControllerPakBad)
    {
        ST_DrawString(-1, 20, "Controller Pak Bad", text_alpha | 0xff000000);
    }
    else if (MenuItem == Menu_ControllerPakFull)
    {
        ST_DrawString(-1, 20, "Controller Pak Full", text_alpha | 0xff000000);
    }
    else if (MenuItem == Menu_CreateNote)
    {
        ST_DrawString(-1, 20, "Create Game Note?", text_alpha | 0xff000000);
    }
    else if (MenuItem == Menu_Load)
    {
        ST_DrawString(-1, 20, "Load Game", text_alpha | 0xff000000);
    }
    else if (MenuItem == Menu_Save)
    {
        if (copysource == copy_none)
            title = "Save Game";
        else if (!M_SaveMustMove())
            title = "Copy Game File";
        else
            title = "Move Game File";

        ST_DrawString(-1, 20, title, text_alpha | 0xff000000);
    }

    item = MenuItem;
    for(i = 0; i < itemlines; i++)
    {
        u32 alpha = text_alpha;
        u32 color = 0xff000000;
        menuentry_t casepos;

        casepos = item->casepos;
        y = item->y;
        if (MenuItem == Menu_Title && osTvType == OS_TV_PAL)
            y += 34;

        if (casepos == MTXT_GAME_SAVED)
        {
            color = 0x4040f000;
            alpha = (alpha * 160) >> 8;
        }
        else if (M_ItemIsDisabled(casepos))
        {
            alpha = (alpha * 96) >> 8;
        }
        else if (casepos == MTXT_QUICK_SAVE || casepos == MTXT_QUICK_LOAD)
        {
            ST_DrawSymbol(240, y, 87, alpha | 0xffffff00);
        }

        ST_DrawString(item->x, y, MenuText[casepos], alpha | color);

        if (i == cursorpos)
            ST_DrawSymbol(item->x -37, y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00);

        item++;
    }
}

void M_FeaturesDrawer(void) // 800091C0
{
    const char *text;
    char textbuff[32];
    const menuitem_t *item;
    int i;

    ST_DrawString(-1, 20, "Features", text_alpha | 0xff000000);
    item = MenuItem;

    for(i = 0; i < itemlines; i++)
    {
        text = MenuText[item->casepos];

        if (item->casepos == MTXT_WARP)
        {
            if (m_actualmap >= 25 && m_actualmap <= 27)
                text = "WARP TO FUN";
            else if (m_actualmap == 28)
                text = "WARP TO MOTHER";
            else if (m_actualmap > 28 && m_actualmap <= 32)
                text = "WARP TO SECRET";
        }
        else if (item->casepos == MTXT_HEALTH_BOOST && players[0].playerstate == PST_DEAD)
        {
            text = "RESURRECT";
        }

        ST_Message(item->x, item->y, text, text_alpha | 0xffffff00);

        text = textbuff;
        switch(item->casepos)
        {
            case MTXT_WARP:
                sprintf(textbuff, "%s", MapInfo[m_actualmap].name);
                break;
            case MTXT_INVULNERABLE:
                text = (!(players[0].cheats & CF_GODMODE)) ? "OFF": "ON";
                break;
            case MTXT_FLY:
                text = (!(players[0].cheats & CF_FLYMODE)) ? "OFF": "ON";
                break;
            case MTXT_KEYS:
                text = (!(players[0].cheats & CF_ALLKEYS)) ? "-" : "100%";
                break;
            case MTXT_ARTIFACTS:
                text = (!(players[0].cheats & CF_ARTIFACTS)) ? "-": "100%";
                break;
            case MTXT_WEAPONS:
                text = (!(players[0].cheats & CF_WEAPONS)) ? "-" : "100%";
                break;
            case MTXT_WALL_BLOCKING:
                text = (!(players[0].mo->flags & MF_NOCLIP)) ? "ON": "OFF";
                break;
            case MTXT_LOCK_MONSTERS:
                text = (!(players[0].cheats & CF_LOCKMONSTERS)) ? "OFF": "ON";
                break;
            case MTXT_MAP_EVERYTHING:
                text = (!(players[0].cheats & CF_ALLMAP)) ? "OFF": "ON";
                break;
            case MTXT_HEALTH_BOOST:
                text = (!(players[0].cheats & CF_HEALTH)) ? "-" : "100%";
                break;
            case MTXT_MUSIC_TEST:
                sprintf(textbuff, "%d", MusicID);
                break;
            default:
                text = NULL; // [Immorpher] set to null for credits menu
                break;
        }

        if (text)
            ST_Message(item->x + 130, item->y, text, text_alpha | 0xffffff00);
        item++;
    }

    ST_DrawSymbol(MenuItem->x -10, MenuItem[cursorpos].y -1, 78, text_alpha | 0xffffff00);
}

static const char *BuildFlags = ""
#ifndef NDEBUG
"DBG "
#endif
#ifdef USB
"USB "
#endif
#ifdef USB_GDB
"GDB"
#endif
;

static void M_DebugMenuDrawer(void) // 800091C0
{
    char textbuf[40];
    const char *text;
    const menuitem_t *item;
    int i;

    ST_DrawString(-1, 20, "Debug", text_alpha | 0xff000000);

    sprintf(textbuf, "Build Flags   %s", *BuildFlags ? BuildFlags : "-");
    ST_Message(40, 50, textbuf, text_alpha | 0xffffff00);

    sprintf(textbuf, "  Total RAM   %lu bytes", osMemSize);
    ST_Message(40, 70, textbuf, text_alpha | 0xffffff00);
    sprintf(textbuf, "   Zone Mem   %d bytes", mainzone->size);
    ST_Message(40, 80, textbuf, text_alpha | 0xffffff00);
    sprintf(textbuf, "       SRAM   %lu bytes", SramSize);
    ST_Message(40, 90, textbuf, text_alpha | 0xffffff00);

    if (IsEmulator)
        text = "Emulator";
    else
        switch (FlashCart)
        {
            case CART_64DRIVE: text = "64Drive"; break;
            case CART_EVERDRIVE: text = "EverDrive"; break;
            case CART_SC64: text = "SC64"; break;
            default: text = "None"; break;
        }
    sprintf(textbuf, "   Platform   %s Detected", text);
    ST_Message(40, 100, textbuf, text_alpha | 0xffffff00);

    item = MenuItem;
    for(i = 0; i < itemlines; i++)
    {
        text = MenuText[item->casepos];
        ST_Message(item->x, item->y, text, text_alpha | 0xffffff00);

        text = NULL;
        switch(item->casepos)
        {
            case MTXT_DEBUG_DISPLAY:
                switch (ShowDebugCounters) {
                    case 0: text = "NONE"; break;
                    case 1: text = "FPS"; break;
                    case 2: text = "CLOCK"; break;
#ifndef NDEBUG
                    case 3: text = "CPU"; break;
                    case 4: text = "GFX"; break;
                    case 5: text = "BSP"; break;
                    case 6: text = "LEVEL"; break;
                    case 7: text = "CAMERA"; break;
                    case 8: text = "MEMORY"; break;
#endif
                    default: text = NULL; break;
                }
                break;
            case MTXT_SECTOR_COLORS:
                text = (!(players[0].cheats & CF_NOCOLORS)) ? "ON": "OFF";
                break;
            case MTXT_FULL_BRIGHT:
                text = (!(players[0].cheats & CF_FULLBRIGHT)) ? "OFF": "ON";
                break;
            default:
                break;
        }

        if (text)
            ST_Message(item->x + 130, item->y, text, text_alpha | 0xffffff00);
        item++;
    }

    ST_DrawSymbol(MenuItem->x -10, MenuItem[cursorpos].y -1, 78, text_alpha | 0xffffff00);
}

static int M_MenuCreditsTicker(void)
{
    int buttons = allticbuttons;
    int oldbuttons = alloldticbuttons;

    if (IS_PRESSED(PAD_A) || IS_PRESSED(PAD_B) || IS_PRESSED(PAD_START))
    {
        S_StartSound(NULL, sfx_pistol);
        return ga_exit;
    }

    return ga_nothing;
}

static void M_IdCreditsDrawer(void)
{
    M_DrawBackground(68, 21, text_alpha, "IDCRED1");
    M_DrawBackground(32, 41, text_alpha, "IDCRED2");
}

static void M_WmsCreditsDrawer(void)
{
    M_DrawBackground(22, 82, text_alpha, "WMSCRED1");
    M_DrawBackground(29, 28, text_alpha, "WMSCRED2");
}

static void M_ModCreditsDrawer(const credit_t *credits, int items)
{
    char *text;
    int i;
    int x;

    for(i = 0; i < items; i++)
    {
        x = credits->x;
        text = credits->text;

        if (x < 0)
            x = (320 - D_strlen(text) * 8) / 2;

        ST_Message(x, credits->y, text, text_alpha | 0xffffff00);
        credits++;
    }
}

void M_ModCredits1Drawer(void)
{
    ST_DrawString(-1, 20, "Doom 64 Ultra", text_alpha | 0xc080c000);
    M_ModCreditsDrawer(Ultra_Credits, ARRAYLEN(Ultra_Credits));
}

void M_ModCredits2Drawer(void)
{
    ST_DrawString(-1, 20, "Merciless Edition", text_alpha | 0x8080ff00);
    M_ModCreditsDrawer(Merciless_Credits, ARRAYLEN(Merciless_Credits));
}

void M_VolumeDrawer(void) // 800095B4
{
    const menuitem_t *item;
    int i;

    ST_DrawString(-1, 20, "Sound", text_alpha | 0xff000000);
    item = MenuItem;

    for(i = 0; i < itemlines; i++)
    {
        ST_DrawString(item->x, item->y, MenuText[item->casepos], text_alpha | 0xff000000);
        item++;
    }

    ST_DrawSymbol(MenuItem->x - 37, MenuItem[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00);

    ST_DrawSymbol(82, 80, 68, text_alpha | 0xffffff00);
    ST_DrawSymbol(Settings.MusVolume + 83, 80, 69, text_alpha | 0xffffff00);

    ST_DrawSymbol(82, 120, 68, text_alpha | 0xffffff00);
    ST_DrawSymbol(Settings.SfxVolume + 83, 120, 69, text_alpha | 0xffffff00);
}

void M_DisplayDrawer(void) // 80009884
{
    const char *text;
    const menuitem_t *item;
    int i;
    menuentry_t casepos;

    ST_DrawString(-1, 20, "Display", text_alpha | 0xff000000);

    item = MenuItem;

    for(i = 0; i < itemlines; i++)
    {
        casepos = item->casepos;

        if (casepos == MTXT_TEXTURE_FILTER || casepos == MTXT_SPRITE_FILTER || casepos == MTXT_SKY_FILTER)
        {
            int ti = casepos - MTXT_TEXTURE_FILTER;
            if (Settings.VideoFilters[ti] == 0)
                text = "Bilinear";
            else
                text = "Off";
        }
        else if (casepos == MTXT_BLOOD_COLOR)
        {
            if (Settings.GreenBlood)
                text = "Green";
            else
                text = "Red";
        }
        else
        {
            text = NULL;
        }

        if (casepos == MTXT_BRIGHTNESS)
        {
            ST_DrawSymbol(item->x + 140, item->y, 68, text_alpha | 0xffffff00);
            ST_DrawSymbol(item->x + 141 + Settings.Brightness/8, item->y, 69, text_alpha | 0xffffff00);
        }
        else if (casepos == MTXT_FLASH_BRIGHTNESS)
        {
            ST_DrawSymbol(item->x + 140, item->y, 68, text_alpha | 0xffffff00);
            ST_DrawSymbol(item->x + 141 + 100*Settings.FlashBrightness/32, item->y, 69, text_alpha | 0xffffff00);
        }

        if (text)
            ST_Message(item->x + 140, item->y, text, text_alpha | 0xff000000);

        ST_Message(item->x, item->y, MenuText[casepos], text_alpha | 0xff000000);

        item++;
    }

    ST_DrawSymbol(MenuItem[0].x - 10, MenuItem[cursorpos].y - 2, 78, text_alpha | 0x90600000);
}

void M_PlayerSetupDrawer(void) // 80009738
{
    char *text;
    const menuitem_t *item;
    int i;
    menuentry_t casepos;

    ST_DrawString(-1, 20, "Player Setup", text_alpha | 0xff000000);

    item = MenuItem;

    for(i = 0; i < itemlines; i++)
    {
        casepos = item->casepos;

        if (casepos == MTXT_AUTORUN) // [Immorpher] Autorun
        {
            if (playerconfigs[0].autorun)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_VERTICAL_LOOK) // [nova] Vertical Look
        {
            if (playerconfigs[0].verticallook == 1)
                text = "Normal";
            else
                text = "Inverted";
        }
        else if (casepos == MTXT_LOOK_SPRING)
        {
            if (playerconfigs[0].lookspring)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_CROSSHAIR)
        {
            if (playerconfigs[0].crosshair == 0)
                text = "None";
            else if (playerconfigs[0].crosshair == 1)
                text = "Dot";
            else if (playerconfigs[0].crosshair == 2)
                text = "Cross";
            else
                text = "Vertical";
        }
        else if (casepos == MTXT_AUTOAIM) // [nova] Autoaim
        {
            if (playerconfigs[0].autoaim)
                text = "On";
            else
                text = "Off";
        }
        else
        {
            text = NULL;
        }

        if (casepos == MTXT_LOOK_SENSITIVITY)
        {
            ST_DrawSymbol(item->x + 140, item->y, 68, text_alpha | 0xffffff00);
            ST_DrawSymbol(item->x + 141 + playerconfigs[0].looksensitivity, item->y, 69, text_alpha | 0xffffff00);
        }
        else if (casepos == MTXT_MOVE_SENSITIVITY)
        {
            ST_DrawSymbol(item->x + 140, item->y, 68, text_alpha | 0xffffff00);
            ST_DrawSymbol(item->x + 141 + playerconfigs[0].movesensitivity, item->y, 69, text_alpha | 0xffffff00);
        }

        if (text)
            ST_Message(item->x + 140, item->y, text, text_alpha | 0xff000000);

        ST_Message(item->x, item->y, MenuText[casepos], text_alpha | 0xff000000);

        item++;
    }

    ST_DrawSymbol(MenuItem[0].x - 10, MenuItem[cursorpos].y - 2, 78, text_alpha | 0x90600000);

}

void M_VideoDrawer(void)
{
    const char *text;
    const menuitem_t *item;
    int i, alpha;
    menuentry_t casepos;

    ST_DrawString(-1, 20, "Video", text_alpha | 0xff000000);

    item = MenuItem;

    for(i = 0; i < itemlines; i++)
    {
        alpha = text_alpha;
        casepos = item->casepos;

        if (M_ItemIsDisabled(casepos))
            alpha = (alpha * 96) >> 8;

        if (casepos == MTXT_RESOLUTION)
        {
            if (osTvType == OS_TV_PAL)
            {
                if (VideoSettings.Resolution == VIDEO_RES_HI_HORIZ)
                    text = "640 x 288";
                else if (VideoSettings.Resolution == VIDEO_RES_HI_VERT)
                    text = "320 x 576";
                else
                    text = "320 x 288";
            }
            else
            {
                if (VideoSettings.Resolution == VIDEO_RES_HI_HORIZ)
                    text = "640 x 240";
                else if (VideoSettings.Resolution == VIDEO_RES_HI_VERT)
                    text = "320 x 480";
                else
                    text = "320 x 240";
            }
        }
        else if (casepos == MTXT_COLOR_DEPTH)
        {
            if (VideoSettings.BitDepth == BITDEPTH_32)
                text = "32-Bit";
            else
                text = "16-Bit";
        }
        else if (casepos == MTXT_ASPECT_RATIO)
        {
            if (VideoSettings.ScreenAspect == 1)
                text = "16:10";
            else if (VideoSettings.ScreenAspect == 2)
                text = "16:9";
            else
                text = "4:3";
        }
        else if (casepos == MTXT_RESAMPLING)
        {
            if ((VideoSettings.TvMode & 1) && VideoSettings.Resolution != VIDEO_RES_HI_VERT)
                text = "Resample";
            else
                text = "None";
        }
        else if (casepos == MTXT_ANTIALIASING)
        {
            if (VideoSettings.AntiAliasing)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_INTERLACING)
        {
            if (VideoSettings.TvMode & 2)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_GAMMA_CORRECT)
        {
            if (VideoSettings.GammaCorrect)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_DITHER_FILTER)
        {
            if (VideoSettings.DitherFilter)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_COLOR_DITHER)
        {
            if (Settings.ColorDither == 1)
                text = "Square";
            else if (Settings.ColorDither == 2)
                text = "Bayer";
            else if (Settings.ColorDither == 3)
                text = "Noise";
            else
                text = "Off";
        }
        else
        {
            text = NULL;
        }

        if (text)
            ST_Message(item->x + 140, item->y, text, alpha | 0xff000000);

        if (casepos == MTXT_INTERLACING && VideoSettings.Resolution == VIDEO_RES_HI_VERT)
            text = "Deflickering";
        else
            text = MenuText[casepos];

        ST_Message(item->x, item->y, text, alpha | 0xff000000);

        item++;
    }

    ST_DrawSymbol(MenuItem[0].x - 10, MenuItem[cursorpos].y - 2, 78, text_alpha | 0x90600000);
}

void M_StatusHUDDrawer(void) // 80009884
{
    char *text;
    const menuitem_t *item;
    int i;
    menuentry_t casepos;

    ST_DrawString(-1, 20, "Status HUD", text_alpha | 0xff000000);

    item = MenuItem;

    for(i = 0; i < itemlines; i++)
    {
        casepos = item->casepos;

        if (casepos == MTXT_MESSAGES)
        {
            if (Settings.EnableMessages)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_TEXT_COLORS)
        {
            if (Settings.HudTextColors)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_STORY_TEXT)
        {
            if (Settings.StoryText)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_MAP_STATS)
        {
            if (Settings.MapStats)
                text = "On";
            else
                text = "Off";
        }
        else
        {
            text = NULL;
        }

        if (casepos == MTXT_MARGIN)
        {
            ST_DrawSymbol(item->x + 100, item->y, 68, text_alpha | 0xffffff00);
            ST_DrawSymbol(item->x + 101 + 100*Settings.HudMargin/20, item->y, 69, text_alpha | 0xffffff00);
        }
        else if (casepos == MTXT_OPACITY)
        {
            ST_DrawSymbol(item->x + 100, item->y, 68, text_alpha | 0xffffff00);
            ST_DrawSymbol(item->x + 101 + 100*Settings.HudOpacity/255, item->y, 69, text_alpha | 0xffffff00);
        }
        else if (casepos == MTXT_MOTION_BOB)
        {
            ST_DrawSymbol(item->x + 100, item->y, 68, text_alpha | 0xffffff00);
            ST_DrawSymbol(item->x + 101 + Settings.MotionBob/0x28F6, item->y, 69, text_alpha | 0xffffff00);
        }

        if (text)
            ST_Message(item->x + 100, item->y, text, text_alpha | 0xff000000);

        ST_Message(item->x, item->y, MenuText[casepos], text_alpha | 0xff000000);

        item++;
    }

    ST_DrawSymbol(MenuItem[0].x - 10, MenuItem[cursorpos].y - 2, 78, text_alpha | 0x90600000);
}

void M_DefaultsDrawer(void) // [Immorpher] new defaults drawer
{
    char buf[40];
    const menuitem_t *item;
    int i;

    ST_DrawString(-1, 20, "Reset to Defaults", text_alpha | 0xff000000);

    item = MenuItem;

    for(i = 0; i < itemlines; i++)
    {
        ST_DrawString(item->x, item->y, MenuText[item->casepos], text_alpha | 0xff000000);

        if (item->casepos != MTXT_RETURN)
            ST_DrawSymbol(item->x + 108, item->y, 87, text_alpha | 0xffffff00);

        item++;
    }

    ST_DrawSymbol(MenuItem[0].x - 37, MenuItem[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00);

    if (lastsetdefaults >= 0)
        sprintf(buf, "defaults set to %s", MenuText[lastsetdefaults]);
    else if (MenuItem[cursorpos].casepos != MTXT_RETURN)
        sprintf(buf, "press \x85 to reset");
    else
        buf[0] = 0;

    if (buf[0])
        ST_DrawString(-1, SCREEN_HT - 30, buf, text_alpha | 0xffffff00);
}

void M_DrawBackground(int x, int y, int color, char *name) // 80009A68
{
    int width, height;
    int xh, yh, tileh, stileh, sheight, t, th, t2;
    int twidth, tline;
    int offset;
    gfxN64_t *data;
    int dsdx, dtdy;

    data = W_CacheLumpName(name, PU_CACHE, dec_jag, sizeof(gfxN64_t));

    R_RenderModes(rm_background);
    gDPSetTextureFilter(GFX1++, ((osTvType == OS_TV_PAL) ? G_TF_BILERP : G_TF_POINT));

    if (color == 0xff)
    {
        gDPSetRenderMode(GFX1++, G_RM_TEX_EDGE, G_RM_TEX_EDGE2);
    }
    else
    {
        gDPSetRenderMode(GFX1++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    }

    gDPSetPrimColorD64(GFX1++, 0, 0, color);

    width = data->width;
    height = data->height;

    // Load Palette Data
    offset = ALIGN(width * height, 8);
    gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b ,
                        1, data->texels + offset);

    gDPTileSync(GFX1++);
    gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

    gDPLoadSync(GFX1++);
    gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 255);

    gDPPipeSync(GFX1++);

    xh = (width + x) << hudxshift;
    tileh = 2048 / ALIGN(width, 8);
    sheight = height << hudyshift;

    if (osTvType == OS_TV_PAL)
    {
        y = FixedMul(0x13333, y);
        sheight = FixedMul(0x13333, sheight);
        stileh = (0x13333 * tileh) >> (FRACBITS-hudyshift);
    }
    else
    {
        stileh = tileh << hudyshift;
    }

    x <<= hudxshift;
    y <<= hudyshift;

    dsdx = 1 << 12 >> hudxshift;
    if (osTvType == OS_TV_PAL)
        dtdy = 0xd555 >> 4 >> hudyshift;
    else
        dtdy = 1 << 12 >> hudyshift;

    t = 0;
    tileh <<= 2;
    height <<= 2;
    twidth = (width - 1) << 2;
    tline = (width + 7) >> 3;
    while (height > 0)
    {
        th = MIN(tileh, height);
        yh = MIN(stileh, sheight);
        t2 = t + th - 4;

        // Load Image Data
        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b ,
                           width, data->texels);

         // Clip Rectangle From Image
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b,
                   tline, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadTile(GFX1++, G_TX_LOADTILE, 0, t, twidth, t2);

        gDPPipeSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b,
                   tline, 0, G_TX_RENDERTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPSetTileSize(GFX1++, G_TX_RENDERTILE, 0, t, twidth, t2);

        gSPTextureRectangle(GFX1++, x, y, xh, yh + y, G_TX_RENDERTILE,
                            0, (t << 3), dsdx, dtdy);

        sheight -= yh;
        height -= th;
        t += th;
        y += yh;
    }

    globallump = -1;
}

void M_DrawOverlay(int x, int y, int w, int h, int color) // 80009F58
{
    I_CheckGFX();

    R_RenderModes(rm_hudoverlay);

    gDPSetPrimColorD64(GFX1++, 0, 0, color);

    gDPFillRectangle(GFX1++, x, y, x+w, y+h);
    globallump = -1;
}

static void M_MoveScrollCursor(int buttons, int max)
{

    if (!(buttons & ALL_JPAD))
    {
        m_vframe1 = 0;
    }
    else
    {
        m_vframe1 -= vblsinframe[0];

        if (m_vframe1 <= 0)
        {
            m_vframe1 = 0xf; // TICRATE/2

            if (buttons & PAD_DOWN)
            {
                cursorpos += 1;

                if (cursorpos < max)
                    S_StartSound(NULL, sfx_switch1);
                else
                    cursorpos = max - 1;

                if ((linepos + 5) < cursorpos)
                    linepos += 1;
            }
            else if (buttons & PAD_UP)
            {
                cursorpos -= 1;

                if (cursorpos < 0)
                    cursorpos = 0;
                else
                    S_StartSound(NULL, sfx_switch1);

                if(cursorpos < linepos)
                    linepos -= 1;
            }
        }
    }
}

int M_ManagePakTicker(void) // 8000A0F8
{
    int exit;
    unsigned int buttons;
    unsigned int oldbuttons;
    OSPfsState *fState;

    if ((FilesUsed == -1) && (I_CheckControllerPak() == 0))
    {
        cursorpos = 0;
        linepos = 0;
    }

    if ((gamevbls < gametic) && ((gametic & 3U) == 0))
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;

    buttons = M_ButtonResponder(allticbuttons);
    oldbuttons = alloldticbuttons & 0xffff0000;

    M_MoveScrollCursor(buttons, 16);

    if (IS_PRESSED(PAD_START) || IS_PRESSED(PAD_B))
    {
        S_StartSound(NULL, sfx_pistol);
        exit = ga_exit;
    }
    else
    {
        if (IS_PRESSED(PAD_UP_C))
        {
            fState = &FileState[cursorpos];

            if(fState->file_size != 0)
            {
                S_StartSound(NULL, sfx_pistol);
                M_SaveMenuData();

                SET_MENU(Menu_DeleteNote);
                MenuCall = M_MenuTitleDrawer;
                cursorpos = 1;
                MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);

                M_FadeOutStart(ga_exit);
                if (cursorpos == 0)
                {
                    if (I_DeletePakFile(cursorpos) == 0)
                    {
                        fState->file_size = 0;
                    }
                    else
                    {
                        FilesUsed = -1;
                    }
                }
                M_RestoreMenuData(true);
            }
        }
        exit = ga_nothing;
    }
    return exit;
}

#if REGION != REGION_JP
static char M_PakTableChar(char c)
{
    static const char Pak_Table[] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',// 16
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',// 26
      'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',// 36
      'u', 'v', 'w', 'x', 'y', 'z', '!', '"', '#','\'',// 46
      '*', '+', ',', '-', '.', '/', ':', '=', '?', '@',// 56
    };

    if (c == 0)
        return c;
    if (c >= 16 && c < (16 + ARRAYLEN(Pak_Table)))
        return Pak_Table[c - 16];
    return ' ';
}
#endif /* REGION != REGION_JP */

void M_ManagePakDrawer(void) // 8000A3E4
{
    byte idx;
    int i,j;
    OSPfsState *fState;
    char buffer [32];
    char *tmpbuf;

    ST_DrawString(-1, 20, "Manage Controller Pak", text_alpha | 0xff000000);

    if (FilesUsed == -1)
    {
        if ((MenuAnimationTic & 2) != 0)
            ST_DrawString(-1, 114, "Controller Pak removed!", text_alpha | 0xff000000);

        ST_DrawString(-1, SCREEN_HT - 30, "press \x8b to exit", text_alpha | 0xffffff00);
    }
#if REGION == REGION_JP
    else
    {
        fState = &FileState[linepos];

        for(i = linepos; i < (linepos + 6); i++)
        {
            if (fState->file_size == 0)
            {
                buffer[0] = '\x1e';
                buffer[1] = '\x26';
                buffer[2] = '\x29';
                buffer[3] = '\x2d';
                buffer[4] = '\x32';
                tmpbuf = buffer + 5;
            }
            else
            {
                tmpbuf = buffer;

                for(j = 0; j < 16; j++)
                {
                    idx = (byte) fState->game_name[j];
                    if(idx == 0)
                        break;

                    tmpbuf[0] = idx;
                    tmpbuf++;
                }

                idx = (byte) fState->ext_name[0];
                if (idx != 0)
                {
                    tmpbuf[0] = '\x3c';
                    tmpbuf[1] = idx;
                    tmpbuf += 2;
                }
            }

            *tmpbuf = '\0';

            ST_DrawStringJp(60, (i - linepos) * 16 + 60, buffer, text_alpha | 0xc0c0c000);

            fState++;
        }

        if (linepos != 0)
        {
            ST_DrawString(60, 45, "\x8F more...", text_alpha | 0xffffff00);
        }

        if ((linepos + 6) < 16)
        {
            ST_DrawString(60, 156, "\x8E more...", text_alpha | 0xffffff00);
        }

        sprintf(buffer, "pages used: %lu   free: %ld", FileState[cursorpos].file_size >> 8, Pak_Memory);

        ST_DrawString(-1, 180, buffer, text_alpha | 0xff000000);
        ST_DrawSymbol(23, (cursorpos - linepos) * 16 + 52, MenuAnimationTic + 70, text_alpha | 0xffffff00);

        ST_DrawString(-1, SCREEN_HT - 40, "press \x8b to exit", text_alpha | 0xffffff00);
        ST_DrawString(-1, SCREEN_HT - 25, "press \x86 to delete", text_alpha | 0xffffff00);
    }
#else /* REGION == REGION_JP */
    else
    {
        fState = &FileState[linepos];

        for(i = linepos; i < (linepos + 6); i++)
        {
            if (fState->file_size == 0)
            {
                D_memmove(buffer, "empty");
            }
            else
            {
                tmpbuf = buffer;

                for(j = 0; j < 16; j++)
                {
                    idx = (byte) fState->game_name[j];
                    if(idx == 0)
                        break;

                    tmpbuf[0] = M_PakTableChar(idx);
                    tmpbuf++;
                }

                idx = (byte) fState->ext_name[0];
                if (idx != 0)
                {
                    tmpbuf[0] = '.';
                    tmpbuf[1] = M_PakTableChar(idx);
                    tmpbuf += 2;
                }

                *tmpbuf = '\0';
            }

            ST_DrawString(60, (i - linepos) * 15 + 60, buffer, text_alpha | 0xff000000);

            fState++;
        }

        if (linepos != 0)
        {
            ST_DrawString(60, 45, "\x8F more...", text_alpha | 0xffffff00);
        }

        if ((linepos + 6) < 16)
        {
            ST_DrawString(60, 150, "\x8E more...", text_alpha | 0xffffff00);
        }

        sprintf(buffer, "pages used: %lu   free: %ld", FileState[cursorpos].file_size >> 8, Pak_Memory);

        ST_DrawString(-1, 170, buffer, text_alpha | 0xff000000);
        ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 51, MenuAnimationTic + 70, text_alpha | 0xffffff00);

        ST_DrawString(-1, SCREEN_HT - 40, "press \x8b to exit", text_alpha | 0xffffff00);
        ST_DrawString(-1, SCREEN_HT - 25, "press \x86 to delete", text_alpha | 0xffffff00);
    }
#endif /* REGION == REGION_JP */
}

static INLINE_ALWAYS levelsave_t *M_PakDataIndex(int pos)
{
    return (void *) &Pak_Data[pos * sizeof(levelsave_t)];
}

static void M_CopySaveTo(levelsave_t *save)
{
    if (copysource == copy_game_pak)
    {
        *save = GamePak_Data[copyindex];
        if (save->skill.permadeath)
        {
            GamePak_Data[copyindex].present = 0;
            I_DeleteSramSave(copyindex);
        }
    }
    else if (copysource == copy_controller_pak)
    {
        *save = *M_PakDataIndex(copyindex);
        if (save->skill.permadeath)
        {
            if (I_DeletePakFile(copyindex) == 0)
                FileState[copyindex].file_size = 0;
            else
                FilesUsed = -1;
        }
    }
    else
    {
        I_SaveProgress(save);
    }
}

static bool M_SaveMustMove(void)
{
    levelsave_t *save;

    if (copysource == copy_game_pak)
        save = &GamePak_Data[copyindex];
    else if (copysource == copy_controller_pak)
        save = M_PakDataIndex(copyindex);
    else
        return false;

    return I_IsSaveValid(save) && save->skill.permadeath;
}

void M_SavePakStart(void) // 8000A6E8
{
    int i;
    int ret;
    int size;

    cursorpos = lastpaksave;
    linepos = 0;
    last_ticon = 0;

    ret = I_CheckControllerPak();
    if (ret == 0)
    {
        if (I_ReadPakFile() == 0)
        {
            size = Pak_Size / sizeof(levelsave_t);

            i = 0;
            if (size != 0)
            {
                do
                {
                    if (!I_IsSaveValid(M_PakDataIndex(i)))
                        break;

                    i++;
                } while (i != size);
            }

            if (i < size)
            {
                cursorpos = i;

                if (!(size < (i+6)))
                    linepos = i;
                else
                    linepos = (size-6);
            }
        }
    }
    else
    {
        FilesUsed = -1;
    }

    M_FadeInStart();
}

void M_SavePakStop(int exit) // 8000A7B4
{
    M_FadeOutStart(MenuData[0].menu_item == Menu_Save ? exit : ga_exit);

    if (copysource == copy_none)
        I_FreePakData();
}

int M_SavePakTicker(void) // 8000A804
{
    unsigned int buttons;
    unsigned int oldbuttons;

    if ((gamevbls < gametic) && ((gametic & 3U) == 0)) {
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;
    }

    buttons = M_ButtonResponder(allticbuttons);
    oldbuttons = alloldticbuttons & 0xffff0000;

    if (FilesUsed == -1)
    {
        if (I_CheckControllerPak()) {
            return ga_nothing;
        }

        if (I_ReadPakFile()) {
            FilesUsed = -1;
            return ga_nothing;
        }

        cursorpos = 0;
        linepos = 0;
    }

    if (last_ticon == 0)
    {
        if ((MenuIdx < 2 && IS_PRESSED(PAD_START)) || IS_PRESSED(PAD_B))
        {
            S_StartSound(NULL, sfx_pistol);
            return ga_exit;
        }

        M_MoveScrollCursor(buttons, Pak_Size / sizeof(levelsave_t));
        if (IS_PRESSED(PAD_DOWN_C))
        {
            S_StartSound(NULL, sfx_pistol);
            M_CopySaveTo(M_PakDataIndex(cursorpos));
            if (I_SavePakFile(File_Num, PFS_WRITE, Pak_Data, Pak_Size) == 0)
            {
                last_ticon = ticon;
                lastpaksave = cursorpos;
            }
            else
            {
                lastpaksave = 0;
                I_FreePakData();
            }
        }
    }
    else if (IS_PRESSED(PAD_START) || IS_PRESSED(PAD_A) || IS_PRESSED(PAD_B)
            || (ticon - last_ticon) >= 60)
    {
        return ga_completed;
    }

    return ga_nothing;
}

void M_SavePakDrawer(void) // 8000AB44
{
    int i;
    char buffer[36];
    char *title;

    if (copysource == copy_none)
        title = "Save To Controller Pak";
    else if (!M_SaveMustMove())
        title = "Copy To Controller Pak";
    else
        title = "Move To Controller Pak";

    ST_DrawString(-1, 20, title, text_alpha | 0xff000000);

    if (FilesUsed == -1)
    {
        if (MenuAnimationTic & 2)
        {
            ST_DrawString(-1, 100, "Controller Pak removed!", 0xff0000ff);
            ST_DrawString(-1, 120, "Game cannot be saved.", 0xff0000ff);
        }

        ST_DrawString(-1, SCREEN_HT - 30, "press \x8b to exit", text_alpha | 0xffffff00);
    }
    else
    {
        for(i = linepos; i < (linepos + 6); i++)
        {
            levelsave_t *save = M_PakDataIndex(i);

            if (!I_IsSaveValid(save))
                D_memmove(buffer, "empty");
            else
                M_PrintSaveTitle(buffer, save->skill, save->map);

            ST_DrawString(60, (i - linepos) * 15 + 65, buffer, text_alpha | 0xff000000);
        }

        if (linepos != 0) {
            ST_DrawString(60, 50, "\x8f more...", text_alpha | 0xffffff00);
        }

        if ((linepos + 6) <= ((Pak_Size >> 5) - 1)) {
            ST_DrawString(60, 155, "\x8e more...", text_alpha | 0xffffff00);
        }

        ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 56, MenuAnimationTic + 70, text_alpha | 0xffffff00);

        ST_DrawString(-1, SCREEN_HT - 40, "press \x87 to save", text_alpha | 0xffffff00);
        ST_DrawString(-1, SCREEN_HT - 25, "press \x8b to cancel", text_alpha | 0xffffff00);
    }
}

void M_SaveGamePakStart(void)
{
    int i;

    if (!GamePak_Data)
    {
        GamePak_Data = Z_Malloc(sizeof(levelsave_t) * MAXSRAMSAVES, PU_STATIC, NULL);
        I_ReadSramSaves(GamePak_Data);
    }

    cursorpos = lastgamepaksave;
    linepos = 0;
    last_ticon = 0;

    i = 0;
    do
    {
        if (!I_IsSaveValid(&GamePak_Data[i]))
            break;

        i++;
    } while (i != MAXSRAMSAVES);

    if (i < MAXSRAMSAVES)
    {
        cursorpos = i;

        if (!(MAXSRAMSAVES < (i+6)))
            linepos = i;
        else
            linepos = (MAXSRAMSAVES-6);
    }

    M_FadeInStart();
}

void M_SaveGamePakStop(int exit)
{
    M_FadeOutStart(MenuData[0].menu_item == Menu_Save ? exit : ga_exit);

    if (copysource == copy_none)
    {
        Z_Free(GamePak_Data);
        GamePak_Data = NULL;
    }
}

int M_SaveGamePakTicker(void)
{
    unsigned int buttons;
    unsigned int oldbuttons;

    if ((gamevbls < gametic) && ((gametic & 3U) == 0)) {
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;
    }

    buttons = M_ButtonResponder(allticbuttons);
    oldbuttons = alloldticbuttons & 0xffff0000;

    if (last_ticon == 0)
    {
        if ((MenuIdx < 2 && IS_PRESSED(PAD_START)) || IS_PRESSED(PAD_B))
        {
            S_StartSound(NULL, sfx_pistol);
            return ga_exit;
        }

        M_MoveScrollCursor(buttons, MAXSRAMSAVES);

        if (IS_PRESSED(PAD_DOWN_C))
        {
            S_StartSound(NULL, sfx_pistol);
            levelsave_t *save = &GamePak_Data[cursorpos];
            M_CopySaveTo(save);
            I_SaveProgressToSram(cursorpos, save);
            last_ticon = ticon;
            lastgamepaksave = cursorpos;
        }
    }
    else if (IS_PRESSED(PAD_START) || IS_PRESSED(PAD_A) || IS_PRESSED(PAD_B)
            || (ticon - last_ticon) >= 60)
    {
        return ga_completed;
    }

    return ga_nothing;
}

void M_SaveGamePakDrawer(void) // 8000AB44
{
    int i;
    char buffer[36];
    char *title;

    if (copysource == copy_none)
        title = "Save To Game Pak";
    else if (!M_SaveMustMove())
        title = "Copy To Game Pak";
    else
        title = "Move To Game Pak";

    ST_DrawString(-1, 20, title, text_alpha | 0xff000000);
    ST_DrawString(-1, 20, title, text_alpha | 0xff000000);

    for(i = linepos; i < (linepos + 6); i++)
    {
        levelsave_t *save = &GamePak_Data[i];

        if (!I_IsSaveValid(save))
            D_memmove(buffer, "empty");
        else
            M_PrintSaveTitle(buffer, save->skill, save->map);

        ST_DrawString(60, (i - linepos) * 15 + 65, buffer, text_alpha | 0xff000000);
    }

    if (linepos != 0) {
        ST_DrawString(60, 50, "\x8f more...", text_alpha | 0xffffff00);
    }

    if ((linepos + 6) <= (MAXSRAMSAVES - 1)) {
        ST_DrawString(60, 155, "\x8e more...", text_alpha | 0xffffff00);
    }

    ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 56, MenuAnimationTic + 70, text_alpha | 0xffffff00);

    ST_DrawString(-1, SCREEN_HT - 40, "press \x87 to save", text_alpha | 0xffffff00);
    ST_DrawString(-1, SCREEN_HT - 25, "press \x8b to cancel", text_alpha | 0xffffff00);
}

int M_SaveMenu(void)
{
    int exit;

    if (SramPresent && I_CheckControllerPak() == 0)
    {
        MenuItem = Menu_Save;
        itemlines = ARRAYLEN(Menu_Save);
        MenuCall = M_MenuTitleDrawer;
        cursorpos = 0;
        exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
    }
    else if (SramPresent)
    {
        MenuCall = M_SaveGamePakDrawer;
        exit = MiniLoop(M_SaveGamePakStart,M_SaveGamePakStop,M_SaveGamePakTicker,M_MenuGameDrawer);
    }
    else
    {
        if (!EnableExpPak && !customskill.permadeath)
            EnableExpPak = (M_ControllerPak() == 0);

        if (EnableExpPak)
        {
            MenuCall = M_SavePakDrawer;
            exit = MiniLoop(M_SavePakStart,M_SavePakStop,M_SavePakTicker,M_MenuGameDrawer);
        }
        else
        {
            exit = ga_exit;
        }
    }
    MenuCall = NULL;

    return exit;
}

void M_LoadPakStart(void) // 8000AEEC
{
    int i;
    int size;

    cursorpos = 0;
    linepos = 0;

    size = Pak_Size / sizeof(levelsave_t);

    i = 0;
    if (size != 0)
    {
        do
        {
            if (I_IsSaveValid(M_PakDataIndex(i)))
                break;

            i++;
        } while (i != size);
    }

    if (i < size)
    {
        cursorpos = i;

        if (!(size < (i+6)))
            linepos = i;
        else
            linepos = (size-6);
    }

    M_FadeInStart();
}

void M_LoadPakStop(int exit) // 8000AF8C
{
    M_FadeOutStart(exit);
    I_FreePakData();
}

int M_LoadPakTicker(void) // 8000AFE4
{
    unsigned int buttons;
    unsigned int oldbuttons;
    int exit;

    if ((gamevbls < gametic) && ((gametic & 3U) == 0)) {
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;
    }

    buttons = M_ButtonResponder(allticbuttons);
    oldbuttons = alloldticbuttons & 0xffff0000;

    M_MoveScrollCursor(buttons, Pak_Size / sizeof(levelsave_t));

    if (IS_PRESSED(PAD_START) || IS_PRESSED(PAD_B))
    {
        S_StartSound(NULL, sfx_pistol);
        exit = ga_exit;
    }
    else if (IS_PRESSED(PAD_DOWN_C) && I_IsSaveValid(M_PakDataIndex(cursorpos)))
    {
        S_StartSound(NULL, sfx_pistol);

        doLoadSave = true;
        LevelSaveBuffer = *M_PakDataIndex(cursorpos);

        startmap = gamemap = LevelSaveBuffer.map;
        startskill = customskill = LevelSaveBuffer.skill;

        G_InitSkill (customskill);
        if (customskill.permadeath) {
            if (I_DeletePakFile(cursorpos) == 0)
                FileState[cursorpos].file_size = 0;
            else
                FilesUsed = -1;
        }
        exit = ga_warped;
    }
    else if (IS_PRESSED(PAD_RIGHT_C) && I_IsSaveValid(M_PakDataIndex(cursorpos)))
    {
        copysource = copy_controller_pak;
        copyindex = cursorpos;
        S_StartSound(NULL, sfx_pistol);
        M_SaveMenuData();
        int savedlinepos = linepos;
        M_SaveMenu();
        linepos = savedlinepos;
        M_RestoreMenuData(true);
        copysource = copy_none;
        exit = ga_nothing;
    }
    else if (IS_PRESSED(PAD_UP_C) && I_IsSaveValid(M_PakDataIndex(cursorpos)))
    {
        S_StartSound(NULL, sfx_saw2);
        GamePak_Data[cursorpos].present = 0;
        if (I_DeletePakFile(cursorpos) == 0)
            FileState[cursorpos].file_size = 0;
        else
            FilesUsed = -1;
        exit = ga_nothing;
    }
    else
    {
        exit = ga_nothing;
    }

    return exit;
}

void M_LoadPakDrawer(void) // 8000B270
{
    int i;
    char buffer[32];
    char *copytext;

    ST_DrawString(-1, 20, "Controller Pak", text_alpha | 0xff000000);

    for(i = linepos; i < (linepos + 6); i++)
    {
        if (FilesUsed == -1)
        {
            D_memmove(buffer, "-");
        }
        else if (!I_IsSaveValid(M_PakDataIndex(i)))
        {
            D_memmove(buffer, "no save");
        }
        else
        {
            levelsave_t *save = M_PakDataIndex(i);
            M_PrintSaveTitle(buffer, save->skill, save->map);
        }

        ST_DrawString(60, (i - linepos) * 15 + 65, buffer, text_alpha | 0xff000000);
    }

    if (linepos != 0) {
        ST_DrawString(60, 50, "\x8f more...", text_alpha | 0xffffff00);
    }

    if ((linepos + 6) <= ((Pak_Size >> 5) - 1)) {
        ST_DrawString(60, 155, "\x8e more...", text_alpha | 0xffffff00);
    }

    ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 56, MenuAnimationTic + 70, text_alpha | 0xffffff00);

    if (!M_SaveMustMove())
        copytext = "\x85 copy";
    else
        copytext = "\x85 move";

    ST_DrawString((SCREEN_WD>>1)-80, SCREEN_HT - 40, "\x87 load", text_alpha | 0xffffff00);
    ST_DrawString((SCREEN_WD>>1)-80, SCREEN_HT - 25, "\x8b cancel", text_alpha | 0xffffff00);
    ST_DrawString((SCREEN_WD>>1)+40, SCREEN_HT - 40, "\x86 delete", text_alpha | 0xffffff00);
    ST_DrawString((SCREEN_WD>>1)+40, SCREEN_HT - 25, copytext, text_alpha | 0xffffff00);
}

void M_LoadGamePakStart(void)
{
    int i;

    GamePak_Data = Z_Malloc(sizeof(levelsave_t) * MAXSRAMSAVES, PU_STATIC, NULL);
    I_ReadSramSaves(GamePak_Data);

    cursorpos = 0;
    linepos = 0;

    i = 0;
    do
    {
        if (I_IsSaveValid(&GamePak_Data[i]))
            break;

        i++;
    } while (i != MAXSRAMSAVES);

    if (i < MAXSRAMSAVES)
    {
        cursorpos = i;

        if (!(MAXSRAMSAVES < (i+6)))
            linepos = i;
        else
            linepos = (MAXSRAMSAVES-6);
    }

    M_FadeInStart();
}

void M_LoadGamePakStop(int exit)
{
    M_FadeOutStart(exit);

    Z_Free(GamePak_Data);
    GamePak_Data = NULL;
}

int M_LoadGamePakTicker(void)
{
    unsigned int buttons;
    unsigned int oldbuttons;
    int exit;

    if ((gamevbls < gametic) && ((gametic & 3U) == 0)) {
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;
    }

    buttons = M_ButtonResponder(allticbuttons);
    oldbuttons = alloldticbuttons & 0xffff0000;

    M_MoveScrollCursor(buttons, MAXSRAMSAVES);

    if (IS_PRESSED(PAD_START) || IS_PRESSED(PAD_B))
    {
        S_StartSound(NULL, sfx_pistol);
        exit = ga_exit;
    }
    else if (IS_PRESSED(PAD_DOWN_C) && I_IsSaveValid(&GamePak_Data[cursorpos]))
    {
        S_StartSound(NULL, sfx_pistol);

        doLoadSave = true;
        LevelSaveBuffer = GamePak_Data[cursorpos];

        startmap = gamemap = LevelSaveBuffer.map;
        startskill = customskill = LevelSaveBuffer.skill;

        G_InitSkill (customskill);
        if (customskill.permadeath) {
            GamePak_Data[cursorpos].present = 0;
            I_DeleteSramSave(cursorpos);
        }
        exit = ga_warped;
    }
    else if (IS_PRESSED(PAD_RIGHT_C) && I_IsSaveValid(&GamePak_Data[cursorpos]))
    {
        copysource = copy_game_pak;
        copyindex = cursorpos;
        S_StartSound(NULL, sfx_pistol);
        M_SaveMenuData();
        int savedlinepos = linepos;
        M_SaveMenu();
        linepos = savedlinepos;
        M_RestoreMenuData(true);
        copysource = copy_none;
        exit = ga_nothing;
    }
    else if (IS_PRESSED(PAD_UP_C) && I_IsSaveValid(&GamePak_Data[cursorpos]))
    {
        S_StartSound(NULL, sfx_saw2);
        GamePak_Data[cursorpos].present = 0;
        I_DeleteSramSave(cursorpos);
        exit = ga_nothing;
    }
    else
    {
        exit = ga_nothing;
    }

    return exit;
}

void M_LoadGamePakDrawer(void)
{
    int i;
    char buffer[32];
    char *copytext;

    ST_DrawString(-1, 20, "Game Pak", text_alpha | 0xff000000);

    for(i = linepos; i < (linepos + 6); i++)
    {
        levelsave_t *save = &GamePak_Data[i];

        if (!I_IsSaveValid(save))
            D_memmove(buffer, "no save");
        else
            M_PrintSaveTitle(buffer, save->skill, save->map);

        ST_DrawString(60, (i - linepos) * 15 + 65, buffer, text_alpha | 0xff000000);
    }

    if (linepos != 0) {
        ST_DrawString(60, 50, "\x8f more...", text_alpha | 0xffffff00);
    }

    if ((linepos + 6) <= (MAXSRAMSAVES - 1)) {
        ST_DrawString(60, 155, "\x8e more...", text_alpha | 0xffffff00);
    }

    ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 56, MenuAnimationTic + 70, text_alpha | 0xffffff00);

    if (!M_SaveMustMove())
        copytext = "\x85 copy";
    else
        copytext = "\x85 move";

    ST_DrawString((SCREEN_WD>>1)-80, SCREEN_HT - 40, "\x87 load", text_alpha | 0xffffff00);
    ST_DrawString((SCREEN_WD>>1)-80, SCREEN_HT - 25, "\x8b cancel", text_alpha | 0xffffff00);
    ST_DrawString((SCREEN_WD>>1)+40, SCREEN_HT - 40, "\x86 delete", text_alpha | 0xffffff00);
    ST_DrawString((SCREEN_WD>>1)+40, SCREEN_HT - 25, copytext, text_alpha | 0xffffff00);
}

int M_CenterDisplayTicker(void) // 8000B4C4
{
    unsigned int buttons, oldbuttons;
    int exit;

    buttons = M_ButtonResponder(allticbuttons);
    oldbuttons = alloldticbuttons & 0xffff0000;

    if (IS_PRESSED(PAD_START) || IS_PRESSED(PAD_B))
    {
        S_StartSound(NULL, sfx_pistol);
        exit = ga_exit;
    }
    else
    {
        if (buttons & PAD_LEFT)
        {
            VideoSettings.Display_X -= 1;
            if (VideoSettings.Display_X < -16)
                VideoSettings.Display_X = -16;
            else
                ConfigChanged = true;
        }
        else if (buttons & PAD_RIGHT)
        {
            VideoSettings.Display_X += 1;
            if (VideoSettings.Display_X > 24)
                VideoSettings.Display_X = 24;
            else
                ConfigChanged = true;
        }

        if (buttons & PAD_UP)
        {
            VideoSettings.Display_Y -= 1;
            if (VideoSettings.Display_Y < -20)
                VideoSettings.Display_Y = -20;
            else
                ConfigChanged = true;
        }
        else if (buttons & PAD_DOWN)
        {
            VideoSettings.Display_Y += 1;
            if (VideoSettings.Display_Y > 12)
                VideoSettings.Display_Y = 12;
            else
                ConfigChanged = true;
        }

        if (IS_PRESSED(PAD_A) && (VideoSettings.Display_X || VideoSettings.Display_Y))
        {
            VideoSettings.Display_X = 0;
            VideoSettings.Display_Y = 0;
            ConfigChanged = true;
        }

        if (buttons & (ALL_JPAD|PAD_A))
            I_RefreshVideo();

        exit = ga_nothing;
    }

    if (ConfigChanged)
    {
        I_SaveConfig();
        ConfigChanged = false;
    }

    return exit;
}

void M_CenterDisplayDrawer(void) // 8000B604
{
    ST_DrawString(-1, 20, "Center Display", text_alpha | 0xff000000);
    ST_DrawString(-1, 114, "use gamepad to adjust", text_alpha | 0xffffff00);
    ST_DrawString(-1, SCREEN_HT - 50, "press \x8a to reset", text_alpha | 0xffffff00);
    ST_DrawString(-1, SCREEN_HT - 30, "press \x8b to exit", text_alpha | 0xffffff00);
}

void M_PlayerColorDrawer(void)
{
    const menuitem_t *item;
    int i;
    menuentry_t casepos;
    statenum_t state;

    if (playerpreviewshoot >= 6)
        state = S_PLAY_ATK2;
    else if (playerpreviewshoot > 0)
        state = S_PLAY_ATK1;
    else
        state = S_PLAY_RUN1 + (MenuAnimationTic&3);

    M_DrawOverlay(((SCREEN_WD>>1)-44)<<(hudxshift-2), 44<<(hudyshift-2),
                  88<<(hudxshift-2), 104<<(hudyshift-2),
                  (text_alpha*224)>>8);

    F_DrawSprite(MT_PLAYER, &states[state], playerpreviewrotate,
                 text_alpha | 0xffffff00, SCREEN_WD>>1, 134, FRACUNIT, 0);

    ST_DrawString(-1, 20, "Player Color", text_alpha | 0xff000000);

    item = MenuItem;

    for(i = 0; i < itemlines; i++)
    {
        casepos = item->casepos;

        if (casepos == MTXT_HUE || casepos == MTXT_SATURATION || casepos == MTXT_VALUE)
        {
            ST_DrawSymbol(item->x + 110, item->y, 68, text_alpha | 0xffffff00);
            ST_DrawSymbol(item->x + 111 + playerconfigs[0].hsv[casepos - MTXT_HUE]*25/63, item->y, 69, text_alpha | 0xffffff00);
        }
        else if (casepos == MTXT_PRESET_COLOR)
        {
            ST_Message(item->x + 110, item->y,
                       playercolorpreset < 0 ? "Custom" : PlayerColorPresets[playercolorpreset].name,
                       text_alpha | 0xff000000);
        }

        if (MenuText[casepos])
            ST_Message(item->x, item->y, MenuText[casepos], text_alpha | 0xff000000);

        item++;
    }

    if (cursorpos == 0)
        ST_DrawSymbol((SCREEN_WD>>1)-5, 136, 95, text_alpha | 0x90600000);
    else
        ST_DrawSymbol(MenuItem[cursorpos].x - 10, MenuItem[cursorpos].y - 2, 78, text_alpha | 0x90600000);
}

#define CONTROLCOLSIZE ((ARRAYLEN(ControlText) - 2) / 2)

int M_ControlPadTicker(void) // 8000B694
{
    unsigned int buttons;
    unsigned int oldbuttons;
    int code = 0;

    buttons = M_ButtonResponder(allticbuttons);
    oldbuttons = M_ButtonResponder(alloldticbuttons);

    if (buttonbindstate == 1)
    {
        int m = ControlMappings[cursorpos - 2];

        if (!IS_PRESSED(PAD_START))
        {
            for (int i = 16; i < 32; i++)
            {
                register int bit = 1 << i;
                if ((allticbuttons & bit) && !(alloldticbuttons & bit))
                {
                    code = (1 << i);
                    break;
                }
            }
            if (!code)
                return ga_nothing;
        }

        Settings.ControlPreset[0] = -1;
        if (configcontroller == 0)
        {
            CurrentControls[0].buttons[m] = code;
            /* if setting, unset anything previously bound to this button */
            if (CurrentControls[0].buttons[m])
            {
                for (int i = 0; i < ARRAYLEN(CurrentControls[0].buttons); i++)
                    if (i != m && CurrentControls[0].buttons[i] == buttons)
                        CurrentControls[0].buttons[i] = 0;
            }
        }
        else if (!(code & ~(PAD_A|PAD_B|PAD_Z_TRIG)))
        {
            if (CurrentControls2[0].a == m)
                CurrentControls2[0].a = BT_NONE;
            else if (CurrentControls2[0].b == m)
                CurrentControls2[0].b = BT_NONE;
            else if (CurrentControls2[0].z == m)
                CurrentControls2[0].z = BT_NONE;

            if (code == PAD_A)
                CurrentControls2[0].a = m;
            else if (code == PAD_B)
                CurrentControls2[0].b = m;
            else if (code == PAD_Z_TRIG)
                CurrentControls2[0].z = m;
        }
        else
            return ga_nothing;

        S_StartSound(NULL,sfx_switch2);
        ConfigChanged = true;
        buttonbindstate = 2;

        return ga_nothing;
    }
    else if (buttonbindstate == 2)
    {
        /* wait for the button to be released */
        if (buttons & ALL_JPAD)
            return ga_nothing;

        buttonbindstate = 0;
    }

    if (!(buttons & ALL_JPAD))
    {
        m_vframe1 = 0;
    }
    else
    {
        m_vframe1 = m_vframe1 - vblsinframe[0];
        if (m_vframe1 <= 0)
        {
            m_vframe1 = 0xf; // TICRATE / 2

            if (buttons & PAD_DOWN)
            {
                cursorpos += 1;
                if (cursorpos >= ARRAYLEN(ControlText))
                    cursorpos = ARRAYLEN(ControlText) - 1;
                else
                    S_StartSound(NULL, sfx_switch1);
            }
            else if (buttons & PAD_UP)
            {
                if (cursorpos == 2 + CONTROLCOLSIZE)
                    cursorpos = 1;
                else
                    cursorpos -= 1;

                if (cursorpos < configcontroller) // hack
                    cursorpos = configcontroller;
                else
                    S_StartSound(NULL, sfx_switch1);
            }
            else if (buttons & PAD_RIGHT)
            {
                if (cursorpos >= 2 && cursorpos < 2 + CONTROLCOLSIZE)
                {
                    S_StartSound(NULL, sfx_switch1);
                    cursorpos += CONTROLCOLSIZE;
                }
            }
            else if (buttons & PAD_LEFT)
            {
                if (cursorpos >= 2 + CONTROLCOLSIZE)
                {
                    S_StartSound(NULL, sfx_switch1);
                    cursorpos -= CONTROLCOLSIZE;
                }
            }
        }
    }

    if (IS_PRESSED(PAD_START) || IS_PRESSED(PAD_B))
    {
        buttonbindstate = 0;
        S_StartSound(NULL, sfx_pistol);
        return ga_exit;
    }
    if (buttons == oldbuttons)
    {
        return ga_nothing;
    }

    if (cursorpos == 0) // Set Default Configuration
    {
        if (IS_PRESSED(PAD_A) || IS_PRESSED(PAD_RIGHT))
        {
            Settings.ControlPreset[0] += 1;
            if(Settings.ControlPreset[0] >= ARRAYLEN(ControllerPresets))
                Settings.ControlPreset[0] = 0;
            ConfigChanged = true;
        }
        else if (IS_PRESSED(PAD_LEFT))
        {
            Settings.ControlPreset[0] -= 1;
            if (Settings.ControlPreset[0] < 0)
                Settings.ControlPreset[0] = ARRAYLEN(ControllerPresets) - 1;
            ConfigChanged = true;
        }
        else
        {
            return ga_nothing;
        }

        D_memcpy(&CurrentControls[0], &ControllerPresets[Settings.ControlPreset[0]].ctrl, sizeof(controls_t));
        D_memcpy(&CurrentControls2[0], &ControllerPresets[Settings.ControlPreset[0]].ctrl2, sizeof(controls2_t));

        S_StartSound(NULL, sfx_switch2);
    }
    else if (cursorpos == 1)
    {
        const int modes[] = {
            STICK_TURN,
            STICK_MOVE | STICK_TURN,
            STICK_MOVE | STICK_STRAFE,
            STICK_TURN | STICK_VLOOK,
            STICK_STRAFE | STICK_VLOOK
        };

        u8 *modeptr = configcontroller == 0 ? &CurrentControls[0].stick : &CurrentControls2[0].stick;
        int mode = *modeptr;
        int modeindex = 0;

        for (int i = 0; i < ARRAYLEN(modes); i++)
        {
            if (modes[i] == mode)
            {
                modeindex = i;
                break;
            }
        }
        if (IS_PRESSED(PAD_A) || IS_PRESSED(PAD_RIGHT))
        {
            modeindex += 1;
            if(modeindex >= ARRAYLEN(modes))
                modeindex = 0;
            ConfigChanged = true;
        }
        else if (IS_PRESSED(PAD_LEFT))
        {
            modeindex -= 1;
            if (modeindex < 0)
                modeindex = ARRAYLEN(modes) - 1;
            ConfigChanged = true;
        }
        else
        {
            return ga_nothing;
        }

        Settings.ControlPreset[0] = -1;
        *modeptr = modes[modeindex];
        S_StartSound(NULL,sfx_switch2);
    }
    else if (buttons & PAD_A)
    {
        buttonbindstate = 1;
    }

    if (ConfigChanged)
    {
        I_SaveConfig();
        ConfigChanged = false;
    }

    return ga_nothing;
}

static int button_code_to_symbol_index(u32 code)
{
    // see doomdef.h for PAD_* definitions
    switch(code) {
    case PAD_LEFT:
        // see st_main.c for symboldata layout
        // gamepad button symbols start at index 80
        return 80;
    case PAD_RIGHT:
        return 81;
    case PAD_UP:
        return 82;
    case PAD_DOWN:
        return 83;
    case PAD_LEFT_C:
        return 84;
    case PAD_RIGHT_C:
        return 85;
    case PAD_UP_C:
        return 86;
    case PAD_DOWN_C:
        return 87;
    case PAD_L_TRIG:
        return 88;
    case PAD_R_TRIG:
        return 89;
    case PAD_A:
        return 90;
    case PAD_B:
        return 91;
    case PAD_Z_TRIG:
        return 92;
    default:
        // question mark
        return 14;
    }
}

void M_ControlPadDrawer(void) // 8000B988
{
    char buffer [44];
    int c, stick;
    int cursor_alpha = text_alpha;

    ST_DrawString(-1, 20, configcontroller == 0 ? "Controller Setup" : "Controller 2 Setup", text_alpha | 0xff000000);

    if (configcontroller == 0)
    {
        if (Settings.ControlPreset[0] == -1)
            sprintf(buffer, ControlText[0], "Custom");
        else
            sprintf(buffer, ControlText[0], ControllerPresets[Settings.ControlPreset[0]].name);
        ST_Message(28, 40, buffer, text_alpha | 0xff000000);
    }

    c = sprintf(buffer, ControlText[1]);
    stick = configcontroller == 0 ? CurrentControls[0].stick : CurrentControls2[0].stick;
    if (stick & STICK_MOVE)
        c += sprintf(&buffer[c], " move");
    else if (stick & STICK_VLOOK)
        c += sprintf(&buffer[c], " look");
    if (stick & STICK_STRAFE)
        sprintf(&buffer[c], " strafe");
    else if (stick & STICK_TURN)
        sprintf(&buffer[c], " turn");
    ST_Message(28, 50, buffer, text_alpha | 0xff000000);

    for(int i = 2; i < ARRAYLEN(ControlText); i++)
    {
        int code;
        int x = (i < (2 + CONTROLCOLSIZE)) ? 18 : 170;
        int y = ((i - 2) % CONTROLCOLSIZE) * 16 + 68;
        int len = D_strlen(ControlText[i]);

        if (configcontroller == 0)
            code = CurrentControls[0].buttons[ControlMappings[i - 2]];
        else if (CurrentControls2[0].a == i)
            code = PAD_A;
        else if (CurrentControls2[0].b == i)
            code = PAD_B;
        else if (CurrentControls2[0].z == i)
            code = PAD_Z_TRIG;
        else
            code = 0;

        ST_Message(x + 98 - len * 8, y + 3, ControlText[i], text_alpha | 0xff000000);
        if (code)
        {
            int symbol = button_code_to_symbol_index(code);
            ST_DrawSymbol(x + 120, y, symbol, text_alpha | 0xffffff00);
        }
    }

    if (buttonbindstate == 1)
        cursor_alpha = ((finesine((gametic & 31) << 8) * 96) >> FRACBITS) + 128;

    if (cursorpos < 2)
        ST_DrawSymbol(18, cursorpos * 10 + 39, 78, text_alpha | 0x90600000);
    else if (cursorpos < 2 + CONTROLCOLSIZE)
        ST_DrawSymbol(124, ((cursorpos - 2) * 16) + 69, 78, cursor_alpha | 0x90600000);
    else
        ST_DrawSymbol(276, (cursorpos - 2) % CONTROLCOLSIZE * 16 + 70 , 78, cursor_alpha | 0x90600000);

    if (buttonbindstate == 1)
        ST_DrawString(-1, SCREEN_HT - 20, "press \x8d to clear", text_alpha | 0xffffff00);
    else
        ST_DrawString(-1, SCREEN_HT - 20, "press \x8b to exit", text_alpha | 0xffffff00);
}

void M_UpdateSkillPreset(void)
{
    for (int i = 0; i < ARRAYLEN(SkillPresets); i++)
    {
        if (memcmp(&skillsetup, (void*) &SkillPresets[i].skill, sizeof skillsetup) == 0) {
            skillpresetsetup = i;
            return;
        }
    }
    skillpresetsetup = -1;
}

void M_CustomSkillDrawer(void)
{
    const char *text;
    const menuitem_t *item;
    int i;

    ST_DrawString(-1, 20, "Custom Difficulty", text_alpha | 0xff000000);

    item = MenuItem;
    for(i = 0; i < itemlines; i++)
    {
        menuentry_t casepos;
        text = NULL;

        casepos = item->casepos;

        switch (casepos)
        {
        case MTXT_PRESET:
            if (skillpresetsetup < 0)
                text = "Custom";
            else
                text = SkillPresets[skillpresetsetup].name;
            break;
        case MTXT_PLAYER_DAMAGE:
            if (skillsetup.player_damage == 0)
                text = "Half";
            else if (skillsetup.player_damage == 1)
                text = "Normal";
            else if (skillsetup.player_damage == 2)
                text = "Double";
            else if (skillsetup.player_damage == 3)
                text = "Quadruple";
            break;
        case MTXT_PLAYER_AMMO:
            if (skillsetup.player_ammo == 0)
                text = "Normal";
            else if (skillsetup.player_ammo == 1)
                text = "Double";
            else if (skillsetup.player_ammo == 2)
                text = "150%";
            break;
        case MTXT_MONSTER_COUNTS:
            if (skillsetup.monster_counts == sk_easy)
                text = "Less";
            else if (skillsetup.monster_counts == sk_medium)
                text = "Normal";
            else if (skillsetup.monster_counts == sk_hard)
                text = "More";
            else if (skillsetup.monster_counts == 3)
                text = "None";
            break;
        case MTXT_MONSTER_SPEED:
            if (skillsetup.monster_speed == 0)
                text = "Normal";
            else if (skillsetup.monster_speed == 1)
                text = "Fast";
            else if (skillsetup.monster_speed == 2)
                text = "Very Fast";
            break;
        case MTXT_MONSTER_RESPAWNS:
            if (skillsetup.monster_respawns)
                text = "On";
            else
                text = "Off";
            break;
        case MTXT_MONSTER_INFIGHTING:
            if (skillsetup.monster_infighting == infighting_map)
                text = "Map-Defined";
            else if (skillsetup.monster_infighting == infighting_always)
                text = "Always";
            else if (skillsetup.monster_infighting == infighting_never)
                text = "Never";
            break;
        case MTXT_MONSTER_REACTIONS:
            if (skillsetup.monster_reactions)
                text = "Instant";
            else
                text = "Normal";
            break;
        case MTXT_MONSTER_COLLISION:
            if (skillsetup.monster_shrink)
                text = "Smaller";
            else
                text = "Normal";
            break;
        case MTXT_MONSTER_PAIN:
            if (skillsetup.monster_reduced_pain)
                text = "Reduced";
            else
                text = "Normal";
            break;
        case MTXT_MONSTER_AIM:
            if (skillsetup.monster_random_aim)
                text = "Randomized";
            else
                text = "Normal";
            break;
        case MTXT_PISTOL_START:
            if (skillsetup.pistol_start)
                text = "On";
            else
                text = "Off";
            break;
        case MTXT_PERMA_DEATH:
            if (skillsetup.permadeath)
                text = "On";
            else
                text = "Off";
            break;
        default:
            text = NULL;
            break;
        }

        if (text)
            ST_Message(item->x + 152, item->y, text, text_alpha | 0xff000000);

        ST_Message(item->x, item->y, MenuText[casepos], text_alpha | 0xff000000);
        item++;
    }


    ST_DrawSymbol(MenuItem->x -10, MenuItem[cursorpos].y -2, 78, text_alpha | 0x90600000);
}
