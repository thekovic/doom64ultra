/* m_main.c -- menu routines */

#include "doomdef.h"
#include "p_local.h"
#include "r_local.h"
#include "st_main.h"
#include "i_usb.h"
#include "config.h"

extern void P_RefreshBrightness(void);
extern void I_RefreshVideo(void);

static void M_DebugMenuDrawer(void);
static int M_MenuCreditsTicker(void);
static void M_IdCreditsDrawer(void);
static void M_WmsCreditsDrawer(void);

static void M_CustomSkillDrawer(void);
static void M_UpdateSkillPreset(void);

static levelsave_t *GamePak_Data = NULL;
levelsave_t LevelSaveBuffer;
boolean doLoadSave = false;

//intermission
int DrawerStatus;

char *ControlText[] =   //8007517C
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

u8 ControlMappings[] = {
    2,  3,  9, 10,  8,  4,  7, 16, 17,
    1,  0, 14, 15,  13, 5, 12, 11, 6
};

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
    _F(MTXT_START_GAME, "Start Game") \
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
    _F(MTXT_MUSIC_TEST, "MUSIC TEST") \
    _F(MTXT_RECORD_DEMO, "Record Demo") \
    _F(MTXT_CREDITS, "Show Credits") \
    \
    _F(MTXT_DEBUG, "Debug") \
    _F(MTXT_DEBUG_DISPLAY, "DISPLAY") \
    _F(MTXT_SECTOR_COLORS, "SECTOR COLORS") \
    _F(MTXT_FULL_BRIGHT, "FULL BRIGHT") \
    _F(MTXT_EXIT, "Exit") \
    \
    _F(MTXT_OPTIONS, "Options") \
    \
    _F(MTXT_CONTROLS, "Controls") \
    \
    _F(MTXT_MOVEMENT, "Movement") \
    _F(MTXT_MOTION_BOB, "Motion Bob") \
    _F(MTXT_SENSITIVITY, "Sensitivity") \
    _F(MTXT_AUTORUN, "Autorun:") \
    _F(MTXT_VERTICAL_LOOK, "Vert Look:") \
    _F(MTXT_AUTOAIM, "Autoaim:") \
    \
    _F(MTXT_SOUND, "Sound") \
    _F(MTXT_MUSIC_VOLUME, "Music Volume") \
    _F(MTXT_EFFECT_VOLUME, "Effect Volume") \
    \
    _F(MTXT_VIDEO, "Video") \
    _F(MTXT_BRIGHTNESS, "Brightness") \
    _F(MTXT_FLASH_BRIGHTNESS, "Flash Brightness") \
    _F(MTXT_RESOLUTION, "Resolution") \
    _F(MTXT_COLOR_DEPTH, "Color Depth") \
    _F(MTXT_ASPECT_RATIO, "Aspect Ratio") \
    _F(MTXT_TEXTURE_FILTER, "Texture Filter") \
    _F(MTXT_SPRITE_FILTER, "Sprite Filter") \
    _F(MTXT_SKY_FILTER, "Sky Filter") \
    _F(MTXT_GAMMA_CORRECT, "Gamma Correct") \
    _F(MTXT_DITHER_FILTER, "Dither Filter") \
    _F(MTXT_COLOR_DITHER, "Color Dither") \
    _F(MTXT_ANTIALIASING, "Anti-Aliasing") \
    _F(MTXT_INTERLACING, "Interlacing") \
    _F(MTXT_CENTER_DISPLAY, "Center Display") \
    \
    _F(MTXT_HUD, "HUD") \
    _F(MTXT_MARGIN, "Margin") \
    _F(MTXT_OPACITY, "Opacity") \
    _F(MTXT_COLORED, "Colored") \
    _F(MTXT_CROSSHAIR, "Crosshair") \
    _F(MTXT_MESSAGES, "Messages") \
    _F(MTXT_STORY_TEXT, "Story Text") \
    _F(MTXT_MAP_STATS, "Map Stats") \
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
char *MenuText[] = { MENU_STRINGS };
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
    { MTXT_PRESET,             32, 50},

    { MTXT_PLAYER_DAMAGE,      32, 70},
    { MTXT_PLAYER_AMMO,        32, 80},
    { MTXT_MONSTER_COUNTS,     32, 90},
    { MTXT_MONSTER_SPEED,      32, 100},
    { MTXT_MONSTER_RESPAWNS,   32, 110},
    { MTXT_MONSTER_INFIGHTING, 32, 120},
    { MTXT_MONSTER_REACTIONS,  32, 130},
    { MTXT_MONSTER_COLLISION,  32, 140},
    { MTXT_MONSTER_PAIN,       32, 150},
    { MTXT_MONSTER_AIM,        32, 160},
    { MTXT_PISTOL_START,       32, 170},
    { MTXT_PERMA_DEATH,        32, 180},

    { MTXT_START_GAME,         32, 200},
    { MTXT_MRETURN,            32, 210},
};

const menuitem_t Menu_Options[] =
{
    { MTXT_CONTROLS, 112, 60 },
    { MTXT_MOVEMENT, 112, 80 },
    { MTXT_SOUND,    112, 100},
    { MTXT_VIDEO,    112, 120},
    { MTXT_HUD,      112, 140},
    { MTXT_DEFAULTS, 112, 160},
    { MTXT_RETURN,   112, 180},
};

const menuitem_t Menu_Volume[] =
{
    { MTXT_MUSIC_VOLUME,  82, 60 },
    { MTXT_EFFECT_VOLUME, 82, 100},
    { MTXT_RETURN,        82, 140},
};

const menuitem_t Menu_Movement[] =
{
    { MTXT_MOTION_BOB,    82, 60},
    { MTXT_SENSITIVITY,   82, 100},
    { MTXT_AUTORUN,       82, 140},
    { MTXT_VERTICAL_LOOK, 82, 160},
    { MTXT_AUTOAIM,       82, 180},
    { MTXT_RETURN,        82, 200},
};

const menuitem_t Menu_Video[] =
{
    { MTXT_BRIGHTNESS,       42, 60},
    { MTXT_FLASH_BRIGHTNESS, 42, 70},
    { MTXT_RESOLUTION,       42, 80},
    { MTXT_COLOR_DEPTH,      42, 90},
    { MTXT_ASPECT_RATIO,     42, 100},
    { MTXT_TEXTURE_FILTER,   42, 110},
    { MTXT_SPRITE_FILTER,    42, 120},
    { MTXT_SKY_FILTER,       42, 130},
    { MTXT_GAMMA_CORRECT,    42, 140},
    { MTXT_DITHER_FILTER,    42, 150},
    { MTXT_COLOR_DITHER,     42, 160},
    { MTXT_ANTIALIASING,     42, 170},
    { MTXT_INTERLACING,      42, 180},
    { MTXT_CENTER_DISPLAY,   42, 190},
    { MTXT_MRETURN,          42, 210},
};

const menuitem_t Menu_StatusHUD[] =
{
    { MTXT_MARGIN,     62, 60},
    { MTXT_OPACITY,    62, 70},
    { MTXT_COLORED,    62, 80},
    { MTXT_CROSSHAIR,  62, 90},
    { MTXT_MESSAGES,   62, 100},
    { MTXT_STORY_TEXT, 62, 110},
    { MTXT_MAP_STATS,  62, 120},
    { MTXT_MRETURN,    62, 140},
};

const menuitem_t Menu_Defaults[] =
{
    { MTXT_PRESET_MODERN,     102, 60},
    { MTXT_PRESET_VANILLA,    102, 80},
    { MTXT_PRESET_MERCILESS,  102, 100},
    { MTXT_PRESET_RETRO,      102, 120},
    { MTXT_PRESET_ACCESSIBLE, 102, 140},
    { MTXT_RETURN,            102, 160},
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
    { MTXT_WARP,           40, 130},
    { MTXT_MUSIC_TEST,     40, 140},

    { MTXT_CREDITS,        40, 160},
    { MTXT_DEBUG,          40, 170},

    { MTXT_EXIT,           40, 190},
};

const menuitem_t Menu_Debug[] =
{
    { MTXT_DEBUG_DISPLAY, 40, 120},
    { MTXT_SECTOR_COLORS, 40, 130},
    { MTXT_FULL_BRIGHT,   40, 140},

    { MTXT_RECORD_DEMO,   40, 160},

    { MTXT_EXIT,          40, 180},
};

typedef struct
{
    char *text;
    int x;
    int y;
} credit_t;

const credit_t Ultra_Credits[] = {
    {"PROGRAMMING          IMMORPHER",   20, 48},
    {                     "JNMARTIN84", 188, 58},
    {                     "NOVA",       188, 68}
};

const credit_t Merciless_Credits[] =
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

//-----------------------------------------

#ifndef DEBUG_DISPLAY
#define DEBUG_DISPLAY 0
#endif

int MenuIdx = 0;                 // 8005A7A4
int text_alpha = 255;            // 8005A7A8
int ConfgNumb[MAXPLAYERS] = {0}; // 8005A7AC
int Display_X = 0;               // 8005A7B0
int Display_Y = 0;               // 8005A7B4
boolean enable_messages = true;  // 8005A7B8
int HUDopacity = 128;            // [Immorpher] HUD opacity
int SfxVolume = 100;             // 8005A7C0
int MusVolume = 80;              // 8005A7C4
int brightness = 125;            // 8005A7C8
int ShowDebugCounters = DEBUG_DISPLAY; // [nova] debug counters
fixed_t MotionBob = 0x100000;    // [Immorpher] Motion Bob works in hexadecimal
int VideoFilters[3] = {0, 1, 0}; // [nova] Independent filter select
int TvMode = 0;                  // [nova] AA, Interlacing
int ScreenAspect = 0;            // [nova] select 4:3, 16:10, 16:9
boolean NoGammaCorrect = false;  // [nova] real gamma option
boolean DitherFilter = false;    // [Immorpher] Dither filter
s8 ColorDither = 0;              // [Immorpher] Color dithering options (Off, Square, Bayer, Noise)
int FlashBrightness = 32;        // [Immorpher] Strobe brightness adjustment, will need to change to float
boolean runintroduction = false; // [Immorpher] New introduction sequence!
boolean StoryText = true;        // [Immorpher] Skip story cut scenes?
boolean MapStats = true;         // [Immorpher] Enable map statistics for automap?
int HUDmargin = 20;              // [Immorpher] HUD margin options (default 20)
boolean ColoredHUD = true;       // [Immorpher] Colored hud
s8 VideoResolution = VIDEO_RES_LOW;
u8 BitDepth = BITDEPTH_16;

#ifdef NDEBUG
#define MAXDEBUGCOUNTERS 2
#else
#define MAXDEBUGCOUNTERS 7
#endif

boolean ConfigChanged = false;

s8 SkillPreset = 1;
static s8 skillpresetsetup;
static customskill_t skillsetup;

const skillpreset_t SkillPresets[] = {
    { "Be Gentle!", { .monster_counts = sk_easy, .player_damage = 0, .player_ammo = 1, } },
    { "Bring It On!", { .monster_counts = sk_easy, .player_damage = 1 } },
    { "I Own Doom!", { .monster_counts = sk_medium, .player_damage = 1 } },
    { "Watch Me Die!", { .monster_counts = sk_hard, .player_damage = 1 } },
    { "Hardcore!", { .monster_counts = sk_hard, .player_damage = 1,
                       .player_ammo = 1, .monster_speed = 1 } },
    { "Be Merciless!", { .monster_counts = sk_hard, .player_damage = 1,
                         .player_ammo = 2, .monster_speed = 2,
                         .monster_shrink = 1, .monster_reduced_pain = 1,
                         .monster_random_aim = 1, } },
    { "Nightmare!", { .monster_counts = sk_hard, .player_damage = 1,
                      .player_ammo = 1, .monster_speed = 1, .monster_respawns = 1,
                      .monster_reactions = 1, } },
    { "Ultra-Nightmare!", { .monster_counts = sk_hard, .player_damage = 1,
                            .player_ammo = 1, .monster_speed = 1, .monster_reactions = 1,
                            .monster_respawns = 1, .permadeath = 1, } },
    { "Impossible!", { .monster_counts = sk_hard, .player_damage = 3,
                       .player_ammo = 1, .monster_speed = 2, .monster_shrink = 1,
                       .monster_reduced_pain = 1, .monster_respawns = 1,
                       .monster_random_aim = 1, .monster_reactions = 1,
                       .permadeath = 1, .pistol_start = 1 } },
};

controls_t CurrentControls[MAXPLAYERS] ALIGNED(16) = {
{
    {{
    PAD_RIGHT_C, PAD_LEFT_C, 0, 0,
    PAD_Z_TRIG,
    PAD_B, PAD_L_TRIG, 0, 0,
    0, 0, PAD_LEFT, PAD_RIGHT,
    0, PAD_UP_C, PAD_DOWN_C, PAD_A, PAD_R_TRIG,
    }},
    STICK_MOVE | STICK_STRAFE
}
};

const char *ControlSetupNames[MAXCONTROLSETUPS] =
{
    "Modern", "Classic", "Arcade", "Dark", "Hunter", "Action", "Nuke", "Fighter",
};

const controls_t DefaultControlSetups[MAXCONTROLSETUPS] ALIGNED(16) = {
    // Modern
    {
        {{
        PAD_RIGHT_C, PAD_LEFT_C, 0, 0,
        PAD_Z_TRIG,
        PAD_B, PAD_L_TRIG, 0, 0,
        0, 0, PAD_LEFT, PAD_RIGHT,
        0, PAD_UP_C, PAD_DOWN_C, PAD_A, PAD_R_TRIG,
        }},
        STICK_MOVE | STICK_STRAFE
    },

    // Classic
    {
        {{
        PAD_RIGHT, PAD_LEFT, PAD_UP, PAD_DOWN,
        PAD_Z_TRIG,
        PAD_RIGHT_C, PAD_UP_C, PAD_LEFT_C, PAD_DOWN_C,
        PAD_L_TRIG, PAD_R_TRIG, PAD_A, PAD_B,
        0, 0, 0, 0, 0,
        }},
        STICK_MOVE | STICK_TURN
    },

    // Arcade
    {
        {{
        0, 0, PAD_UP_C, PAD_DOWN_C,
        PAD_Z_TRIG,
        PAD_R_TRIG, PAD_L_TRIG, 0, 0,
        PAD_LEFT_C, PAD_RIGHT_C, PAD_A, PAD_B,
        0, 0, 0, PAD_UP, PAD_DOWN,
        }},
        STICK_VLOOK | STICK_TURN
    },

    // Dark
    {
        {{
        0, 0, 0, 0,
        PAD_Z_TRIG,
        PAD_B, PAD_L_TRIG, 0, 0,
        PAD_LEFT_C, PAD_RIGHT_C, 0, PAD_A,
        PAD_R_TRIG, PAD_UP_C, PAD_DOWN_C, PAD_UP, PAD_DOWN,
        }},
        STICK_MOVE | STICK_TURN
    },

    // Hunter
    {
        {{
        0, 0, PAD_UP_C, PAD_DOWN_C,
        PAD_Z_TRIG,
        PAD_RIGHT, PAD_L_TRIG, 0, 0,
        PAD_LEFT_C, PAD_RIGHT_C, PAD_B, PAD_A,
        PAD_R_TRIG, 0, 0, PAD_R_TRIG, PAD_DOWN,
        }},
        STICK_VLOOK | STICK_TURN
    },

    // Action
    {
        {{
        0, 0, 0, 0,
        PAD_Z_TRIG,
        PAD_R_TRIG, PAD_L_TRIG, 0, 0,
        PAD_LEFT_C, PAD_RIGHT_C, PAD_B, PAD_A,
        0, PAD_UP, PAD_DOWN, PAD_UP_C, PAD_DOWN_C,
        }},
        STICK_MOVE | STICK_TURN
    },

    // Nuke
    {
        {{
        0, 0, PAD_UP_C, PAD_DOWN_C,
        PAD_Z_TRIG,
        PAD_A, PAD_L_TRIG, 0, 0,
        PAD_LEFT_C, PAD_RIGHT_C, PAD_LEFT, PAD_RIGHT,
        0, 0, 0, PAD_R_TRIG, PAD_B,
        }},
        STICK_VLOOK | STICK_TURN
    },

    // Fighter
    {
        {{
        PAD_RIGHT, PAD_LEFT, PAD_UP_C, PAD_DOWN_C,
        PAD_Z_TRIG,
        PAD_B, PAD_A, 0, 0,
        PAD_LEFT_C, PAD_RIGHT_C, PAD_L_TRIG, PAD_R_TRIG,
        0, 0, 0, PAD_UP, PAD_DOWN,
        }},
        STICK_TURN | STICK_VLOOK
    }
};

#define SET_MENU(_m) do { MenuItem = (_m); itemlines = ARRAYLEN(_m); } while(0)

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

    S_StartMusic(116);

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

            MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);
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

            MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);
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
    cursorpos = 0;

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
    if (osMemSize < 0x800000)
    {
        if (casepos == MTXT_COLOR_DEPTH || casepos == MTXT_RESOLUTION)
            return true;
    }
    if (casepos == MTXT_ANTIALIASING)
    {
        if (VideoResolution == VIDEO_RES_HI_VERT)
            return true;
        if (VideoResolution == VIDEO_RES_HI_HORIZ && BitDepth == BITDEPTH_32)
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

#define MAXSENSIVITY    20

int M_ButtonResponder(int buttons) // 80007960
{
    int sensitivity;
    int NewButtons;

    /* Copy Default Buttons */
    NewButtons = (buttons);

    /* Analyze Analog Stick (up / down) */
    sensitivity = (int)((buttons) << 24) >> 24;

    if (sensitivity <= -MAXSENSIVITY)
        NewButtons |= PAD_DOWN;
    else if (sensitivity >= MAXSENSIVITY)
        NewButtons |= PAD_UP;

    /* Analyze Analog Stick (left / right) */
    sensitivity = (int)(((buttons & 0xff00) >> 8) << 24) >> 24;

    if (sensitivity <= -MAXSENSIVITY)
        NewButtons |= PAD_LEFT;
    else if (sensitivity >= MAXSENSIVITY)
        NewButtons |= PAD_RIGHT;

    return NewButtons & 0xffff0000;
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

    text_alpha += (text_alpha_change_value * vblsinframe[0]) >> 1;
    if (text_alpha_change_value < 0)
    {
        if (text_alpha < 0)
        {
            text_alpha = 0;
            return ga_exit;
        }
    }
    else
    {
        if ((text_alpha_change_value > 0) && (text_alpha >= 256))
        {
            text_alpha = 255;
            return ga_exit;
        }
    }

    return ga_nothing;
}

void M_FadeInStart(void) // 80007AB4
{
    MiniLoop(M_AlphaInStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

void M_FadeOutStart(int exitmode) // 80007AEC
{
    if (exitmode == ga_exit)
    {
        if (ConfigChanged)
        {
            I_SaveConfig();
            ConfigChanged = false;
        }

        MiniLoop(M_AlphaOutStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
    }
}

void M_SaveMenuData(void) // 80007B2C
{
    menudata_t *mdat;

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

    // Restore Previous Save Menu Page
    MenuIdx -= 1;
    mdat = &MenuData[MenuIdx];

    MenuItem  = mdat->menu_item;
    itemlines = mdat->item_lines;
    MenuCall  = mdat->menu_call;
    cursorpos = mdat->cursor_pos;

    if (MenuItem == Menu_Load)
    {
        if (!M_AdjustLoadMenu())
        {
            M_RestoreMenuData(alpha_in);
            return;
        }
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

        if (MenuCall == M_SavePakDrawer || MenuCall == M_SaveGamePakDrawer || MenuItem == Menu_Save
                || (MenuIdx > 0 && MenuData[0].menu_item == Menu_Save))
            M_DrawBackground(63, 25, 128, "EVIL");
        else
            M_DrawBackground(56, 57, 80, "TITLE");

        if (MenuItem != Menu_Title) {
            M_DrawOverlay(0, 0, XResolution, YResolution, 96);
        }

        ST_DrawDebug();
        MenuCall();
        I_DrawFrame();
    }
}

extern int globalcheats; // [GEC]

int M_QuickLoadFailedTicker(void) // 8002BA88
{
    if ((ticon - last_ticon) >= 60 && ((u32)ticbuttons[0] >> 16) != 0)
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

int M_MenuTicker(void) // 80007E0C
{
    unsigned int buttons, oldbuttons;
    int exit;
    int truebuttons;
    int i;
    menuentry_t casepos;
    boolean padrepeat = false;

    /* animate skull */
    if ((gamevbls < gametic) && ((gametic & 3U) == 0))
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;

    buttons = M_ButtonResponder(ticbuttons[0]);
    oldbuttons = M_ButtonResponder(oldticbuttons[0]);

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

        if (((buttons & PAD_START) && !(oldticbuttons[0] & PAD_START))
                || ((buttons & PAD_B) && !(oldticbuttons[0] & PAD_B)))
        {
            if (MenuItem == Menu_Title)
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
            truebuttons = (buttons & PAD_A) && !(oldbuttons & PAD_A);
            casepos = (menuentry_t) MenuItem[cursorpos].casepos;

            switch(casepos)
            {

            case MTXT_CONTROLS:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    MenuCall = M_ControlPadDrawer;
                    cursorpos = 0;
                    linepos = 0;

                    MiniLoop(M_FadeInStart,M_FadeOutStart,M_ControlPadTicker,M_MenuGameDrawer);
                    M_RestoreMenuData(true);
                    return ga_nothing;
                }
                break;

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
                        cursorpos = 0;
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

            case MTXT_EXIT:
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
                if (buttons & PAD_RIGHT)
                {
                    MusVolume += 1;
                    if (MusVolume <= 100)
                    {
                        S_SetMusicVolume(MusVolume);
                        if (MusVolume & 1)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                    else
                    {
                        MusVolume = 100;
                    }
                }
                else if (buttons & PAD_LEFT)
                {
                    MusVolume -= 1;
                    if (MusVolume < 0)
                    {
                        MusVolume = 0;
                    }
                    else
                    {
                        S_SetMusicVolume(MusVolume);
                        if (MusVolume & 1)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                }
                break;

            case MTXT_EFFECT_VOLUME:
                if (buttons & PAD_RIGHT)
                {
                    SfxVolume += 1;
                    if (SfxVolume <= 100)
                    {
                        S_SetSoundVolume(SfxVolume);
                        if (SfxVolume & 1)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                    else
                    {
                        SfxVolume = 100;
                    }
                }
                else if (buttons & PAD_LEFT)
                {
                    SfxVolume -= 1;
                    if (SfxVolume < 0)
                    {
                        SfxVolume = 0;
                    }
                    else
                    {
                        S_SetSoundVolume(SfxVolume);
                        if (SfxVolume & 1)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                }
                break;

            case MTXT_BRIGHTNESS:
                if (buttons & PAD_RIGHT)
                {
                    brightness += 2; // [Immorpher] increments doubled for scroll speed
                    if (brightness <= 200) // [Immorpher] limit extended to 200 from 100 for an optional brightness boost
                    {
                        P_RefreshBrightness();
                        if (brightness & 2)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                    else
                    {
                        brightness = 200; // [Immorpher] new limit is 200 instead of 100
                    }
                }
                else if (buttons & PAD_LEFT)
                {
                    brightness -= 2; // [Immorpher] decrement speed doubled
                    if (brightness < 0)
                    {
                        brightness = 0;
                    }
                    else
                    {
                        P_RefreshBrightness();
                        if (brightness & 2)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                }
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
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
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
                    cursorpos = 1;  // Set Default Bring it on!

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

                        // Free Pak_Data
                        if (Pak_Data)
                        {
                            Z_Free(Pak_Data);
                            Pak_Data = NULL;
                        }
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
                    cursorpos = 0;
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
                if (truebuttons || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillpresetsetup == ARRAYLEN(SkillPresets) - 1)
                        skillpresetsetup = 0;
                    else
                        skillpresetsetup++;
                    skillsetup = SkillPresets[skillpresetsetup].skill;
                    return ga_nothing;
                }
                if ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
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
                if (truebuttons || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.player_damage == 3)
                        skillsetup.player_damage = 0;
                    else
                        skillsetup.player_damage++;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                if ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
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
                if (truebuttons || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.player_ammo == 2)
                        skillsetup.player_ammo = 0;
                    else
                        skillsetup.player_ammo++;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                if ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
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
                if (truebuttons || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.monster_counts == 2)
                        skillsetup.monster_counts = 0;
                    else
                        skillsetup.monster_counts++;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                if ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.monster_counts == 0)
                        skillsetup.monster_counts = 2;
                    else
                        skillsetup.monster_counts--;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_MONSTER_SPEED:
                if (truebuttons || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.monster_speed == 2)
                        skillsetup.monster_speed = 0;
                    else
                        skillsetup.monster_speed++;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                if ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
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
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    skillsetup.monster_respawns ^= 1;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_MONSTER_INFIGHTING:
                if (truebuttons || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    if (skillsetup.monster_infighting == 2)
                        skillsetup.monster_infighting = 0;
                    else
                        skillsetup.monster_infighting++;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                if ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
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
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    skillsetup.monster_reactions ^= 1;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_MONSTER_COLLISION:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    skillsetup.monster_shrink ^= 1;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_MONSTER_PAIN:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    skillsetup.monster_reduced_pain ^= 1;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_MONSTER_AIM:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    skillsetup.monster_random_aim ^= 1;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_PISTOL_START:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    skillsetup.pistol_start ^= 1;
                    M_UpdateSkillPreset();
                    return ga_nothing;
                }
                break;
            case MTXT_PERMA_DEATH:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
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
                    players[0].cheats |= cheats;

                    if (exit == ga_exit)
                        return ga_nothing;

                    return exit;
                }
                break;

            case MTXT_WARP:
                if (padrepeat)
                {
                    if (buttons & PAD_LEFT)
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
                    else if (buttons & PAD_RIGHT)
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
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    players[0].cheats ^= CF_GODMODE;
                    return ga_nothing;
                }
                break;

            case MTXT_FLY:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
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
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    players[0].cheats |= CF_ALLKEYS;

                    P_GiveAllKeys(&players[0]);

                    return ga_nothing;
                }
                break;

            case MTXT_WEAPONS:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
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
                if (truebuttons || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    ShowDebugCounters += 1;
                    if (ShowDebugCounters > MAXDEBUGCOUNTERS)
                        ShowDebugCounters = 0;
                    return ga_nothing;
                }
                if ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
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
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
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
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    enable_messages ^= true;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_OPACITY:
                if (buttons & PAD_RIGHT)
                {
                    HUDopacity += 4;
                    if (HUDopacity <= 255)
                    {
                        if (HUDopacity & 4)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                    else
                    {
                        HUDopacity = 255;
                    }
                }
                else if (buttons & PAD_LEFT)
                {
                    HUDopacity -= 4;
                    if (HUDopacity < 0)
                    {
                        HUDopacity = 0;
                    }
                    else
                    {
                        if (HUDopacity & 4)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                }
                break;

            case MTXT_LOCK_MONSTERS:
                /* Not available in the release code */
                /*
                Reconstructed code based on Doom 64 Ex
                */
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
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
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    players[0].cheats ^= CF_ALLMAP;
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
                    if (buttons & PAD_LEFT)
                    {
                        MusicID -= 1;
                        if (MusicID > 0)
                        {
                            S_StartSound(NULL, sfx_switch2);
                            return ga_nothing;
                        }
                        MusicID = 1;
                    }
                    else if (buttons & PAD_RIGHT)
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
                    S_StartMusic(MusicID+92);
                    return ga_nothing;
                }
                break;

            case MTXT_MOVEMENT:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    SET_MENU(Menu_Movement);
                    MenuCall = M_MovementDrawer;
                    cursorpos = 0;

                    MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);
                    M_RestoreMenuData(true);

                    return ga_nothing;
                }
                break;

            case MTXT_SENSITIVITY:
                if (buttons & PAD_RIGHT)
                {
                    playerconfigs[0].sensitivity += 1;
                    if (playerconfigs[0].sensitivity <= 100)
                    {
                        if (playerconfigs[0].sensitivity & 1)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                    else
                    {
                        playerconfigs[0].sensitivity = 100;
                    }
                }
                else if (buttons & PAD_LEFT)
                {
                    playerconfigs[0].sensitivity -= 1;
                    if (playerconfigs[0].sensitivity < 0)
                    {
                        playerconfigs[0].sensitivity = 0;
                    }
                    else
                    {
                        if (playerconfigs[0].sensitivity & 1)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                }
                break;

            case MTXT_MANAGE_PAK:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_pistol);
                    M_SaveMenuData();

                    MenuCall = M_ControllerPakDrawer;
                    linepos = 0;
                    cursorpos = 0;

                    exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_ScreenTicker, M_MenuGameDrawer);
                    M_RestoreMenuData((exit == ga_exit));

                    if (exit == ga_exit)
                        return ga_nothing;

                    return exit;
                }
                break;

            case MTXT_SECTOR_COLORS:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    players[0].cheats ^= CF_NOCOLORS;
                    globalcheats ^= CF_NOCOLORS;
                    P_RefreshBrightness();
                    return ga_nothing;
                }
                break;

            case MTXT_FULL_BRIGHT:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
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
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    i = casepos - MTXT_TEXTURE_FILTER;
                    S_StartSound(NULL, sfx_switch2);
                    VideoFilters[i] ^= 1;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_MOTION_BOB:
                if (buttons & PAD_RIGHT)
                {
                    MotionBob += 0x8000; // increments
                    if (MotionBob <= 0x100000) // Maximum is 32 in hex
                    {
                        if (MotionBob & 0x8000)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                    else
                    {
                        MotionBob = 0x100000; // The Limit
                    }
                }
                else if (buttons & PAD_LEFT)
                {
                    MotionBob -= 0x8000; // decrements
                    if (MotionBob < 0x0)
                    {
                        MotionBob = 0x0;
                    }
                    else
                    {
                        if (MotionBob & 0x8000)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                }
                break;
            case MTXT_RESOLUTION:
                if (truebuttons || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    VideoResolution += 1;
                    if (VideoResolution > 2)
                        VideoResolution = 0;
                    I_BlankScreen(2);
                    I_RefreshVideo();
                    ConfigChanged = true;
                    return ga_nothing;
                }
                if ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                {
                    S_StartSound(NULL, sfx_switch2);
                    VideoResolution -= 1;
                    if (VideoResolution < 0)
                        VideoResolution = 2;
                    I_BlankScreen(2);
                    I_RefreshVideo();
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_COLOR_DEPTH:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    BitDepth ^= 1;
                    I_BlankScreen(2);
                    I_RefreshVideo();
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_ASPECT_RATIO:
                if (truebuttons || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    ScreenAspect += 1;
                    if (ScreenAspect > 2)
                        ScreenAspect = 0;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                if ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                {
                    S_StartSound(NULL, sfx_switch2);
                    ScreenAspect -= 1;
                    if (ScreenAspect < 0)
                        ScreenAspect = 2;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_DITHER_FILTER:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    DitherFilter ^= true;
                    I_RefreshVideo();
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_ANTIALIASING:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    TvMode ^= 1;
                    I_RefreshVideo();
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_INTERLACING:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    TvMode ^= 2;
                    I_RefreshVideo();
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_COLOR_DITHER:
                if (truebuttons || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    ColorDither += 1;
                    if (ColorDither > 3)
                        ColorDither = 0;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                if ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                {
                    S_StartSound(NULL, sfx_switch2);
                    ColorDither -= 1;
                    if (ColorDither < 0)
                        ColorDither = 3;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_FLASH_BRIGHTNESS:
                if (buttons & PAD_RIGHT)
                {
                    FlashBrightness += 1; // increments
                    if (FlashBrightness  <= 32) // Maximum is 32
                    {
                        if (FlashBrightness & 1)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                    else
                    {
                        FlashBrightness = 32; // The Limit
                    }
                }
                else if (buttons & PAD_LEFT)
                {
                    FlashBrightness -= 1; // decrements
                    if (FlashBrightness < 0)
                    {
                        FlashBrightness = 0;
                    }
                    else
                    {
                        if (FlashBrightness & 1)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                }
                break;

            case MTXT_PRESET_MODERN:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_switch2);

                    // Set movement/controller options
                    MotionBob = 0x100000;

                    // Set video options
                    brightness = 125;
                    VideoFilters[0] = VideoFilters[1] = VideoFilters[2] = 0;
                    TvMode = 0;
                    NoGammaCorrect = false;
                    DitherFilter = false;  // [Immorpher] new video option
                    ColorDither = 0;  // [Immorpher] new video option

                    // Set display options
                    FlashBrightness = 32;  // [Immorpher] new video option
                    StoryText = true; // [Immorpher] Skip story cut scenes?
                    MapStats = true; // [Immorpher] Display automap stats?

                    // Set HUD options
                    enable_messages = true;
                    HUDopacity = 128;
                    HUDmargin = 19; // [Immorpher] HUD margin options
                    ColoredHUD = true; // [Immorpher] Colored hud

                    // Set sound options
                    SfxVolume = 0x50;
                    MusVolume = 0x50;

                    // Reset functions
                    for (int i = 0; i < MAXPLAYERS; i++)
                    {
                        ConfgNumb[i] = 0;    // gamepad configuration
                        D_memcpy(&CurrentControls[i], &DefaultControlSetups[ConfgNumb[i]], sizeof CurrentControls);
                        playerconfigs[i].crosshair = 2;
                        playerconfigs[i].sensitivity = 0;
                        playerconfigs[i].verticallook = -1;
                        playerconfigs[i].autorun = true;
                        playerconfigs[i].autoaim = false;
                    }
                    I_MoveDisplay(0,0);
                    P_RefreshBrightness();
                    I_RefreshVideo();
                    S_SetMusicVolume(MusVolume);
                    S_SetSoundVolume(SfxVolume);

                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_PRESET_VANILLA:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_switch2);

                    // Set movement/controller options
                    MotionBob = 0x100000;

                    // Set video options
                    brightness = 0;
                    VideoFilters[0] = VideoFilters[1] = VideoFilters[2] = 0;
                    TvMode = 0;
                    NoGammaCorrect = true;
                    DitherFilter = false;  // [Immorpher] new video option
                    ColorDither = 0;  // [Immorpher] new video option

                    // Set display options
                    FlashBrightness = 32;  // [Immorpher] new video option
                    StoryText = false; // [Immorpher] Skip story cut scenes?
                    MapStats = false; // [Immorpher] Display automap stats?

                    // Set HUD options
                    enable_messages = true;
                    HUDopacity = 128;
                    HUDmargin = 19; // [Immorpher] HUD margin options
                    ColoredHUD = false; // [Immorpher] Colored hud

                    // Set sound options
                    SfxVolume = 0x50;
                    MusVolume = 0x50;

                    // Reset functions
                    for (int i = 0; i < MAXPLAYERS; i++)
                    {
                        ConfgNumb[i] = 1;    // gamepad configuration
                        D_memcpy(&CurrentControls[i], &DefaultControlSetups[ConfgNumb[i]], sizeof CurrentControls);
                        playerconfigs[i].crosshair = 0;
                        playerconfigs[i].sensitivity = 0;
                        playerconfigs[i].verticallook = -1;
                        playerconfigs[i].autorun = false;
                        playerconfigs[i].autoaim = true;
                    }
                    I_MoveDisplay(0,0);
                    P_RefreshBrightness();
                    I_RefreshVideo();
                    S_SetMusicVolume(MusVolume);
                    S_SetSoundVolume(SfxVolume);

                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_PRESET_MERCILESS:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_switch2);

                    // Set movement/controller options
                    MotionBob = 0x100000;

                    // Set video options
                    brightness = 100;
                    VideoFilters[0] = VideoFilters[1] = VideoFilters[2] = 0;
                    TvMode = 0;
                    NoGammaCorrect = false;
                    DitherFilter = false;  // [Immorpher] new video option
                    ColorDither = 0;  // [Immorpher] new video option

                    // Set display options
                    FlashBrightness = 32;  // [Immorpher] new video option
                    StoryText = true; // [Immorpher] Skip story cut scenes?
                    MapStats = false; // [Immorpher] Display automap stats?

                    // Set HUD options
                    enable_messages = true;
                    HUDopacity = 128;
                    HUDmargin = 15; // [Immorpher] HUD margin options
                    ColoredHUD = true; // [Immorpher] Colored hud

                    // Set sound options
                    SfxVolume = 100;
                    MusVolume = 0x50;

                    // Reset functions
                    for (int i = 0; i < MAXPLAYERS; i++)
                    {
                        ConfgNumb[i] = 0;    // gamepad configuration
                        D_memcpy(&CurrentControls[i], &DefaultControlSetups[ConfgNumb[i]], sizeof CurrentControls);
                        playerconfigs[i].crosshair = 0;
                        playerconfigs[i].sensitivity = 0;
                        playerconfigs[i].verticallook = -1;
                        playerconfigs[i].autorun = false;
                        playerconfigs[i].autoaim = true;
                    }
                    I_MoveDisplay(0,0);
                    P_RefreshBrightness();
                    I_RefreshVideo();
                    S_SetMusicVolume(MusVolume);
                    S_SetSoundVolume(SfxVolume);
                    ConfigChanged = true;

                    return ga_nothing;
                }
                break;

            case MTXT_PRESET_RETRO:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_switch2);

                    // Set movement/controller options
                    MotionBob = 0x100000;

                    // Set video options
                    brightness = 200;
                    VideoFilters[filt_textures] = VideoFilters[filt_sprites] = 0;
                    VideoFilters[filt_skies] = 1;
                    TvMode = 0;
                    NoGammaCorrect = false;
                    DitherFilter = false;  // [Immorpher] new video option
                    ColorDither = 2;  // [Immorpher] new video option

                    // Set display options
                    FlashBrightness = 32;  // [Immorpher] new video option
                    StoryText = true; // [Immorpher] Keep story cut scenes?
                    MapStats = true; // [Immorpher] Display automap stats?

                    // Set HUD options
                    enable_messages = true;
                    HUDopacity = 196;
                    HUDmargin = 5; // [Immorpher] HUD margin options
                    ColoredHUD = true; // [Immorpher] Colored hud

                    // Set sound options
                    SfxVolume = 100;
                    MusVolume = 0x50;

                    // Reset functions
                    for (int i = 0; i < MAXPLAYERS; i++)
                    {
                        ConfgNumb[i] = 5;    // gamepad configuration
                        D_memcpy(&CurrentControls[i], &DefaultControlSetups[ConfgNumb[i]], sizeof CurrentControls);
                        playerconfigs[i].crosshair = 2;
                        playerconfigs[i].sensitivity = 0;
                        playerconfigs[i].verticallook = 1;
                        playerconfigs[i].autorun = true;
                        playerconfigs[i].autoaim = false;
                    }
                    I_MoveDisplay(0,0);
                    P_RefreshBrightness();
                    I_RefreshVideo();
                    S_SetMusicVolume(MusVolume);
                    S_SetSoundVolume(SfxVolume);
                    ConfigChanged = true;

                    return ga_nothing;
                }
                break;

            case MTXT_PRESET_ACCESSIBLE:
                if (truebuttons)
                {
                    S_StartSound(NULL, sfx_switch2);

                    // Set movement/controller options
                    MotionBob = 0x0;

                    // Set video options
                    brightness = 200;
                    VideoFilters[0] = VideoFilters[1] = VideoFilters[2] = 0;
                    TvMode = 0;
                    NoGammaCorrect = false;
                    DitherFilter = false;  // [Immorpher] new video option
                    ColorDither = 0;  // [Immorpher] new video option

                    // Set display options
                    FlashBrightness = 0;  // [Immorpher] new video option
                    StoryText = true; // [Immorpher] Skip story cut scenes?
                    MapStats = true; // [Immorpher] Display automap stats?

                    // Set HUD options
                    enable_messages = true;
                    HUDopacity = 255;
                    HUDmargin = 15; // [Immorpher] HUD margin options (default 20)
                    ColoredHUD = true; // [Immorpher] Colored hud

                    // Set sound options
                    SfxVolume = 100;
                    MusVolume = 0x50;

                    // Reset functions
                    for (int i = 0; i < MAXPLAYERS; i++)
                    {
                        ConfgNumb[i] = 0;    // gamepad configuration
                        D_memcpy(&CurrentControls[i], &DefaultControlSetups[ConfgNumb[i]], sizeof CurrentControls);
                        playerconfigs[i].crosshair = 2;
                        playerconfigs[i].sensitivity = 0;
                        playerconfigs[i].verticallook = 1;
                        playerconfigs[i].autorun = true;
                        playerconfigs[i].autoaim = true;
                    }
                    I_MoveDisplay(0,0);
                    P_RefreshBrightness();
                    I_RefreshVideo();
                    S_SetMusicVolume(MusVolume);
                    S_SetSoundVolume(SfxVolume);
                    ConfigChanged = true;

                    return ga_nothing;
                }
                break;

            case MTXT_STORY_TEXT:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    StoryText ^= true;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_MAP_STATS:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    MapStats ^= true;
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
                if (buttons & PAD_RIGHT)
                {
                    HUDmargin += 1; // increments
                    if (HUDmargin <= 20) // Maximum is 20
                    {
                        if (HUDmargin & 1)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                    else
                    {
                        HUDmargin = 20; // The Limit
                    }
                }
                else if (buttons & PAD_LEFT)
                {
                    HUDmargin -= 1; // decrements
                    if (HUDmargin < 0)
                    {
                        HUDmargin = 0;
                    }
                    else
                    {
                        if (HUDmargin & 1)
                        {
                            S_StartSound(NULL, sfx_secmove);
                            return ga_nothing;
                        }
                        ConfigChanged = true;
                    }
                }
                break;

            case MTXT_COLORED:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    ColoredHUD ^= true;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;

            case MTXT_GAMMA_CORRECT:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    NoGammaCorrect = !NoGammaCorrect;
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
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    players[0].cheats |= CF_ARTIFACTS;
                    players[0].artifacts |= 1 | 2 | 4;

                    S_StartSound(NULL, sfx_switch2);
                    return ga_nothing;
                }
                break;
            case MTXT_CROSSHAIR:
                if (truebuttons || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    playerconfigs[0].crosshair += 1;
                    if (playerconfigs[0].crosshair > 3)
                        playerconfigs[0].crosshair = 0;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                else if ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
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
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    playerconfigs[0].verticallook
                        = playerconfigs[0].verticallook == 1 ? -1 : 1;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;
            case MTXT_AUTOAIM:
                if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                        || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                {
                    S_StartSound(NULL, sfx_switch2);
                    playerconfigs[0].autoaim ^= true;
                    ConfigChanged = true;
                    return ga_nothing;
                }
                break;
            case MTXT_QUICK_SAVE:
                if ((buttons & PAD_RIGHT_C) && !(oldbuttons & PAD_RIGHT_C))
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
                if ((buttons & PAD_RIGHT_C) && !(oldbuttons & PAD_RIGHT_C))
                    return ga_loadquicksave;
                break;
            case MTXT_LOAD_QUICK_LOAD:
                if ((buttons & PAD_A) && !(oldbuttons & PAD_A))
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
                            MenuCall = NULL;
                        }
                        if (exit != ga_completed && exit != ga_warped)
                            M_RestoreMenuData(true);
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

                    if (exit != ga_completed && exit != ga_warped)
                        M_RestoreMenuData(true);

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

void M_MenuClearCall(void) // 80008E6C
{
    MenuCall = NULL;
}

void M_MenuTitleDrawer(void) // 80008E7C
{
    const menuitem_t *item;
    int i;

    if (MenuItem == Menu_Game)
    {
        ST_DrawString(-1, 20, "Pause", text_alpha | 0xc0000000);
        ST_DrawString(-1, SCREEN_HT - 30, "press \x8b to resume", text_alpha | 0xffffff00);
    }
    else if (MenuItem == Menu_Skill)
    {
        ST_DrawString(-1, 20, "Choose Your Skill...", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_Options)
    {
        ST_DrawString(-1, 20, "Options", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_Quit)
    {
        ST_DrawString(-1, 20, "Quit Game?", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_DeleteNote)
    {
        ST_DrawString(-1, 20, "Delete Game Note?", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_ControllerPakBad)
    {
        ST_DrawString(-1, 20, "Controller Pak Bad", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_ControllerPakFull)
    {
        ST_DrawString(-1, 20, "Controller Pak Full", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_CreateNote)
    {
        ST_DrawString(-1, 20, "Create Game Note?", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_Load)
    {
        ST_DrawString(-1, 20, "Load Game", text_alpha | 0xc0000000);
    }
    else if (MenuItem == Menu_Save)
    {
        ST_DrawString(-1, 20, "Save Game", text_alpha | 0xc0000000);
    }

    item = MenuItem;
    for(i = 0; i < itemlines; i++)
    {
        u32 alpha = text_alpha;
        u32 color = 0xc0000000;
        menuentry_t casepos;

        casepos = item->casepos;

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
            ST_DrawSymbol(240, item->y, 85, alpha | 0xffffff00);
        }

        ST_DrawString(item->x, item->y, MenuText[casepos], alpha | color);
        item++;
    }

    ST_DrawSymbol(MenuItem[0].x -37, MenuItem[cursorpos].y -9, MenuAnimationTic + 70, text_alpha | 0xffffff00);
}

void M_FeaturesDrawer(void) // 800091C0
{
    const char *text;
    char textbuff[32];
    const menuitem_t *item;
    int i;

    ST_DrawString(-1, 20, "Features", text_alpha | 0xc0000000);
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

    ST_DrawString(-1, 20, "Debug", text_alpha | 0xc0000000);

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
                    case 7: text = "MEMORY"; break;
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
    int buttons = ticbuttons[0];
    int oldbuttons = oldticbuttons[0];

    if (((buttons & PAD_A) && !(oldbuttons & PAD_A))
            || ((buttons & PAD_B) && !(oldbuttons & PAD_B))
            || ((buttons & PAD_START) && !(oldbuttons & PAD_START)))
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

    ST_DrawString(-1, 20, "Sound", text_alpha | 0xc0000000);
    item = Menu_Volume;

    for(i = 0; i < itemlines; i++)
    {
        ST_DrawString(item->x, item->y, MenuText[item->casepos], text_alpha | 0xc0000000);
        item++;
    }

    ST_DrawSymbol(MenuItem->x - 37, MenuItem[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00);

    ST_DrawSymbol(82, 80, 68, text_alpha | 0xffffff00);
    ST_DrawSymbol(MusVolume + 83, 80, 69, text_alpha | 0xffffff00);

    ST_DrawSymbol(82, 120, 68, text_alpha | 0xffffff00);
    ST_DrawSymbol(SfxVolume + 83, 120, 69, text_alpha | 0xffffff00);
}

void M_MovementDrawer(void) // 80009738
{
    char *text;
    const menuitem_t *item;
    int i;
    menuentry_t casepos;

    ST_DrawString(-1, 20, "Movement", text_alpha | 0xc0000000);

    item = Menu_Movement;

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

        if (text)
            ST_DrawString(item->x + 120, item->y, text, text_alpha | 0xc0000000);

        ST_DrawString(item->x, item->y, MenuText[casepos], text_alpha | 0xc0000000);
        item++;
    }

    ST_DrawSymbol(MenuItem->x - 37, MenuItem[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00);

    // Sensitivity
    ST_DrawSymbol(82,120,68,text_alpha | 0xffffff00);
    ST_DrawSymbol(playerconfigs[0].sensitivity + 83, 120, 69, text_alpha | 0xffffff00);

    // Motion bob
    ST_DrawSymbol(82, 80, 68, text_alpha | 0xffffff00);
    ST_DrawSymbol(MotionBob/0x28F6 + 83, 80, 69, text_alpha | 0xffffff00);

}

void M_VideoDrawer(void) // 80009884
{
    char *text;
    const menuitem_t *item;
    int i, alpha;
    menuentry_t casepos;

    ST_DrawString(-1, 20, "Video", text_alpha | 0xc0000000);

    item = Menu_Video;

    for(i = 0; i < itemlines; i++)
    {
        alpha = text_alpha;
        casepos = item->casepos;

        if (M_ItemIsDisabled(casepos))
            alpha = (alpha * 96) >> 8;

        if (casepos == MTXT_TEXTURE_FILTER || casepos == MTXT_SPRITE_FILTER || casepos == MTXT_SKY_FILTER)
        {
            int ti = casepos - MTXT_TEXTURE_FILTER;
            if (VideoFilters[ti] == 0)
                text = "Bilinear";
            else
                text = "Off";
        }
        else if (casepos == MTXT_RESOLUTION)
        {
            if (osTvType == OS_TV_PAL)
            {
                if (VideoResolution == VIDEO_RES_HI_HORIZ)
                    text = "640 x 288";
                else if (VideoResolution == VIDEO_RES_HI_VERT)
                    text = "320 x 576";
                else
                    text = "320 x 288";
            }
            else
            {
                if (VideoResolution == VIDEO_RES_HI_HORIZ)
                    text = "640 x 240";
                else if (VideoResolution == VIDEO_RES_HI_VERT)
                    text = "320 x 480";
                else
                    text = "320 x 240";
            }
        }
        else if (casepos == MTXT_COLOR_DEPTH)
        {
            if (BitDepth == BITDEPTH_32)
                text = "32-Bit";
            else
                text = "16-Bit";
        }
        else if (casepos == MTXT_ASPECT_RATIO)
        {
            if (ScreenAspect == 1)
                text = "16:10";
            else if (ScreenAspect == 2)
                text = "16:9";
            else
                text = "4:3";
        }
        else if (casepos == MTXT_ANTIALIASING)
        {
            if ((TvMode & 1) && VideoResolution != VIDEO_RES_HI_VERT)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_INTERLACING)
        {
            if (TvMode & 2)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_GAMMA_CORRECT)
        {
            if (NoGammaCorrect)
                text = "Off";
            else
                text = "On";
        }
        else if (casepos == MTXT_DITHER_FILTER)
        {
            if (DitherFilter)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_COLOR_DITHER)
        {
            if (ColorDither == 1)
                text = "Square";
            else if (ColorDither == 2)
                text = "Bayer";
            else if (ColorDither == 3)
                text = "Noise";
            else
                text = "Off";
        }
        else
        {
            text = NULL;
        }

        if (casepos == MTXT_BRIGHTNESS)
        {
            ST_DrawSymbol(item->x + 140, item->y, 68, alpha | 0xffffff00);
            ST_DrawSymbol(item->x + 141 + brightness/2, item->y, 69, alpha | 0xffffff00);
        }
        else if (casepos == MTXT_FLASH_BRIGHTNESS)
        {
            ST_DrawSymbol(item->x + 140, item->y, 68, alpha | 0xffffff00);
            ST_DrawSymbol(item->x + 141 + 100*FlashBrightness/32, item->y, 69, alpha | 0xffffff00);
        }

        if (text)
            ST_Message(item->x + 140, item->y, text, alpha | 0xc0000000);

        if (casepos == MTXT_INTERLACING && VideoResolution == VIDEO_RES_HI_VERT)
            text = "Deflickering";
        else
            text = MenuText[casepos];

        ST_Message(item->x, item->y, text, alpha | 0xc0000000);

        item++;
    }

    ST_DrawSymbol(Menu_Video[0].x - 10, Menu_Video[cursorpos].y - 2, 78, text_alpha | 0x90600000);
}

void M_StatusHUDDrawer(void) // 80009884
{
    char *text;
    const menuitem_t *item;
    int i;
    menuentry_t casepos;

    ST_DrawString(-1, 20, "Status HUD", text_alpha | 0xc0000000);

    item = Menu_StatusHUD;

    for(i = 0; i < itemlines; i++)
    {
        casepos = item->casepos;

        if (casepos == MTXT_MESSAGES)
        {
            if (enable_messages)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_COLORED)
        {
            if (ColoredHUD)
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
        else if (casepos == MTXT_STORY_TEXT)
        {
            if (StoryText)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == MTXT_MAP_STATS)
        {
            if (MapStats)
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
            ST_DrawSymbol(item->x + 101 + 100*HUDmargin/20, item->y, 69, text_alpha | 0xffffff00);
        }
        else if (casepos == MTXT_OPACITY)
        {
            ST_DrawSymbol(item->x + 100, item->y, 68, text_alpha | 0xffffff00);
            ST_DrawSymbol(item->x + 101 + 100*HUDopacity/255, item->y, 69, text_alpha | 0xffffff00);
        }

        if (text)
            ST_Message(item->x + 100, item->y, text, text_alpha | 0xc0000000);

        ST_Message(item->x, item->y, MenuText[casepos], text_alpha | 0xc0000000);

        item++;
    }

    ST_DrawSymbol(Menu_StatusHUD[0].x - 10, Menu_StatusHUD[cursorpos].y - 2, 78, text_alpha | 0x90600000);
}

void M_DefaultsDrawer(void) // [Immorpher] new defaults drawer
{
    const menuitem_t *item;
    int i;

    ST_DrawString(-1, 20, "Set Defaults", text_alpha | 0xc0000000);

    item = Menu_Defaults;

    for(i = 0; i < itemlines; i++)
    {
        ST_DrawString(item->x, item->y, MenuText[item->casepos], text_alpha | 0xc0000000);

        item++;
    }

    ST_DrawSymbol(Menu_Defaults[0].x - 37, Menu_Defaults[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00);
}

void M_DrawBackground(int x, int y, int color, char *name) // 80009A68
{
    int width, height;
    int yh, xh, t;
    int offset;
    byte *data;

    data = (byte *)W_CacheLumpName(name, PU_CACHE, dec_jag);

    gDPPipeSync(GFX1++);
    gDPSetCycleType(GFX1++, G_CYC_1CYCLE);

    gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
    gDPSetTexturePersp(GFX1++, G_TP_NONE);

    gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);

    gDPSetBlendColor(GFX1++, 0, 0, 0, 0);
    gDPSetCombineMode(GFX1++, G_CC_D64COMB03, G_CC_D64COMB03);

    if (color == 0xff)
    {
        gDPSetRenderMode(GFX1++, G_RM_TEX_EDGE, G_RM_TEX_EDGE2);
    }
    else
    {
        gDPSetRenderMode(GFX1++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    }

    gDPSetPrimColorD64(GFX1++, 0, 0, color);

    width = ((gfxN64_t*)data)->width;
    height = ((gfxN64_t*)data)->height;

    // Load Palette Data
    offset = (width * height);
    offset = (offset + 7) & ~7;
    gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b ,
                        1, data + offset + sizeof(gfxN64_t));

    gDPTileSync(GFX1++);
    gDPSetTile(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_4b, 0, 256, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

    gDPLoadSync(GFX1++);
    gDPLoadTLUTCmd(GFX1++, G_TX_LOADTILE, 255);

    gDPPipeSync(GFX1++);

    xh = (width + 7) & ~7;

    t = 0;
    while (height != 0)
    {
        if ((2048 / xh) < height)
            yh = (2048 / xh);
        else
            yh = height;

        // Load Image Data
        gDPSetTextureImage(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b ,
                        width, data + sizeof(gfxN64_t));

         // Clip Rectangle From Image
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b,
                        (width + 7) / 8, 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPLoadSync(GFX1++);
        gDPLoadTile(GFX1++, G_TX_LOADTILE,
                    (0 << 2), (t << 2),
                    ((width - 1) << 2), (((t + yh) - 1) << 2));

        gDPPipeSync(GFX1++);
        gDPSetTile(GFX1++, G_IM_FMT_CI, G_IM_SIZ_8b,
                    (width + 7) / 8, 0, G_TX_RENDERTILE, 0, 0, 0, 0, 0, 0, 0);

        gDPSetTileSize(GFX1++, G_TX_RENDERTILE,
                       (0 << 2), (t << 2),
                       ((width - 1) << 2), (((t + yh) - 1) << 2));

        gSPTextureRectangle(GFX1++,
            (x << hudxshift), (y << hudyshift),
            ((width + x) << hudxshift), ((yh + y) << hudyshift),
            G_TX_RENDERTILE,
            (0 << 5), (t << 5),
            (1 << 12 >> hudxshift), (1 << 12 >> hudyshift));

        height -= yh;
        t += yh;
        y += yh;
    }

    gDPPipeSync(GFX1++);
    gDPSetCycleType(GFX1++, G_CYC_2CYCLE);

    globallump = -1;
}

void M_DrawOverlay(int x, int y, int w, int h, int color) // 80009F58
{
    I_CheckGFX();

    gDPPipeSync(GFX1++);

    gDPSetCycleType(GFX1++, G_CYC_1CYCLE);

    gDPSetTextureLUT(GFX1++, G_TT_RGBA16);
    gDPSetTexturePersp(GFX1++, G_TP_NONE);

    gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);

    gDPSetBlendColor(GFX1++, 0, 0, 0, 0);
    gDPSetCombineMode(GFX1++, G_CC_D64COMB05, G_CC_D64COMB05);
    gDPSetRenderMode(GFX1++, G_RM_XLU_SURF, G_RM_XLU_SURF2);

    gDPSetPrimColorD64(GFX1++, 0, 0, color);

    gDPFillRectangle(GFX1++, x, y, w, h);
    globallump = -1;
}

int M_ScreenTicker(void) // 8000A0F8
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

    buttons = M_ButtonResponder(ticbuttons[0]);
    oldbuttons = oldticbuttons[0] & 0xffff0000;

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

                if (cursorpos < 16)
                    S_StartSound(NULL, sfx_switch1);
                else
                    cursorpos = 15;

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

    if (((buttons & PAD_START) && !(oldbuttons & PAD_START))
            || ((buttons & PAD_B) && !(oldbuttons & PAD_B)))
    {
        S_StartSound(NULL, sfx_pistol);
        exit = ga_exit;
    }
    else
    {
        if ((buttons & PAD_RIGHT_C) && !(oldbuttons & PAD_RIGHT_C))
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

void M_ControllerPakDrawer(void) // 8000A3E4
{
    byte idx;
    int i,j;
    OSPfsState *fState;
    char buffer [32];
    char *tmpbuf;

    ST_DrawString(-1, 20, "Controller Pak", text_alpha | 0xc0000000);

    if (FilesUsed == -1)
    {
        if ((MenuAnimationTic & 2) != 0)
            ST_DrawString(-1, 114, "Controller Pak removed!", text_alpha | 0xc0000000);

        ST_DrawString(-1, SCREEN_HT - 30, "press \x8b to exit", text_alpha | 0xffffff00);
    }
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

            ST_DrawString(60, (i - linepos) * 15 + 60, buffer, text_alpha | 0xc0000000);

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

        ST_DrawString(-1, 170, buffer, text_alpha | 0xc0000000);
        ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 51, MenuAnimationTic + 70, text_alpha | 0xffffff00);

        ST_DrawString(-1, SCREEN_HT - 40, "press \x8b to exit", text_alpha | 0xffffff00);
        ST_DrawString(-1, SCREEN_HT - 25, "press \x84\x85 to delete", text_alpha | 0xffffff00);
    }
}

static INLINE_ALWAYS levelsave_t *M_PakDataIndex(int pos)
{
    return (void *) &Pak_Data[pos * 32];
}

void M_SavePakStart(void) // 8000A6E8
{
    int i;
    int ret;
    int size;

    cursorpos = 0;
    linepos = 0;
    last_ticon = 0;

    ret = I_CheckControllerPak();
    if (ret == 0)
    {
        if (I_ReadPakFile() == 0)
        {
            size = Pak_Size / 32;

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
    S_StartSound(NULL, sfx_pistol);

    M_FadeOutStart(exit);

    if (Pak_Data)
    {
        Z_Free(Pak_Data);
        Pak_Data = NULL;
    }
}

int M_SavePakTicker(void) // 8000A804
{
    unsigned int buttons;
    unsigned int oldbuttons;
    int size;

    if ((gamevbls < gametic) && ((gametic & 3U) == 0)) {
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;
    }

    buttons = M_ButtonResponder(ticbuttons[0]);
    oldbuttons = oldticbuttons[0] & 0xffff0000;

    if (((buttons & PAD_START) && !(oldbuttons & PAD_START))
            || ((buttons & PAD_B) && !(oldbuttons & PAD_B)))
    {
        return ga_exit;
    }

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

    if (!(buttons & ALL_JPAD)) {
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

                size = (Pak_Size / 32) - 1;

                if (size < cursorpos)
                    cursorpos = size;
                else
                    S_StartSound(NULL, sfx_switch1);


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

    if (last_ticon == 0)
    {
        if ((buttons & PAD_A) && !(oldbuttons & PAD_A))
        {
            levelsave_t save;
            I_SaveProgress(&save);
            D_memcpy(M_PakDataIndex(cursorpos), &save, sizeof save);

            if (I_SavePakFile(File_Num, PFS_WRITE, Pak_Data, Pak_Size) == 0)
            {
                last_ticon = ticon;
            }
            else
            {
                FilesUsed = -1;
                if (Pak_Data)
                {
                    Z_Free(Pak_Data);
                    Pak_Data = NULL;
                }
            }
        }
    }
    else if ((ticon - last_ticon) >= 60)
    {
        return ga_completed;
    }

    return ga_nothing;
}

void M_SavePakDrawer(void) // 8000AB44
{
    int i;
    char buffer[36];

    ST_DrawString(-1, 20, "Controller Pak", text_alpha | 0xc0000000);

    if (FilesUsed == -1)
    {
        if (MenuAnimationTic & 2)
        {
            ST_DrawString(-1, 100, "Controller Pak removed!", 0xc00000ff);
            ST_DrawString(-1, 120, "Game cannot be saved.", 0xc00000ff);
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

            ST_DrawString(60, (i - linepos) * 15 + 65, buffer, text_alpha | 0xc0000000);
        }

        if (linepos != 0) {
            ST_DrawString(60, 50, "\x8f more...", text_alpha | 0xffffff00);
        }

        if ((linepos + 6) <= ((Pak_Size >> 5) - 1)) {
            ST_DrawString(60, 155, "\x8e more...", text_alpha | 0xffffff00);
        }

        ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 56, MenuAnimationTic + 70, text_alpha | 0xffffff00);
    }
}

void M_SaveGamePakStart(void)
{
    int i;

    GamePak_Data = Z_Malloc(sizeof(levelsave_t) * MAXSRAMSAVES, PU_STATIC, NULL);
    I_ReadSramSaves(GamePak_Data);

    cursorpos = 0;
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
    S_StartSound(NULL, sfx_pistol);

    M_FadeOutStart(exit);

    Z_Free(GamePak_Data);
    GamePak_Data = NULL;
}

int M_SaveGamePakTicker(void)
{
    unsigned int buttons;
    unsigned int oldbuttons;

    if ((gamevbls < gametic) && ((gametic & 3U) == 0)) {
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;
    }

    buttons = M_ButtonResponder(ticbuttons[0]);
    oldbuttons = oldticbuttons[0] & 0xffff0000;

    if (((buttons & PAD_START) && !(oldbuttons & PAD_START))
            || ((buttons & PAD_B) && !(oldbuttons & PAD_B)))
    {
        return ga_exit;
    }

    if (!(buttons & ALL_JPAD)) {
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

                if (MAXSRAMSAVES - 1 < cursorpos)
                    cursorpos = MAXSRAMSAVES - 1;
                else
                    S_StartSound(NULL, sfx_switch1);


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

    if (last_ticon == 0)
    {
        if ((buttons & PAD_A) && !(oldbuttons & PAD_A))
        {
            I_SaveProgress(&GamePak_Data[cursorpos]);;
            I_SaveProgressToSram(cursorpos, &GamePak_Data[cursorpos]);
            last_ticon = ticon;
        }
    }
    else if ((ticon - last_ticon) >= 60)
    {
        return ga_completed;
    }

    return ga_nothing;
}

void M_SaveGamePakDrawer(void) // 8000AB44
{
    int i;
    char buffer[36];

    ST_DrawString(-1, 20, "Game Pak", text_alpha | 0xc0000000);

    for(i = linepos; i < (linepos + 6); i++)
    {
        levelsave_t *save = &GamePak_Data[i];

        if (!I_IsSaveValid(save))
            D_memmove(buffer, "empty");
        else
            M_PrintSaveTitle(buffer, save->skill, save->map);

        ST_DrawString(60, (i - linepos) * 15 + 65, buffer, text_alpha | 0xc0000000);
    }

    if (linepos != 0) {
        ST_DrawString(60, 50, "\x8f more...", text_alpha | 0xffffff00);
    }

    if ((linepos + 6) <= (MAXSRAMSAVES - 1)) {
        ST_DrawString(60, 155, "\x8e more...", text_alpha | 0xffffff00);
    }

    ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 56, MenuAnimationTic + 70, text_alpha | 0xffffff00);
}

void M_LoadPakStart(void) // 8000AEEC
{
    int i;
    int size;

    cursorpos = 0;
    linepos = 0;

    size = Pak_Size / 32;

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

void M_LoadPakStop(void) // 8000AF8C
{
    S_StartSound(NULL, sfx_pistol);
    M_FadeOutStart(ga_exit);

    if (Pak_Data)
    {
        Z_Free(Pak_Data);
        Pak_Data = NULL;
    }
}

int M_LoadPakTicker(void) // 8000AFE4
{
    unsigned int buttons;
    unsigned int oldbuttons;
    int size;
    int exit;

    if ((gamevbls < gametic) && ((gametic & 3U) == 0)) {
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;
    }

    buttons = M_ButtonResponder(ticbuttons[0]);
    oldbuttons = oldticbuttons[0] & 0xffff0000;

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

                size = (Pak_Size / 32) - 1;

                if (size < cursorpos)
                    cursorpos = size;
                else
                    S_StartSound(NULL, sfx_switch1);

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

    if (((buttons & PAD_START) && !(oldbuttons & PAD_START))
            || ((buttons & PAD_B) && !(oldbuttons & PAD_B)))
    {
        exit = ga_exit;
    }
    else if (((buttons & PAD_A) && !(oldbuttons & PAD_A)) && I_IsSaveValid(M_PakDataIndex(cursorpos)))
    {
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

    ST_DrawString(-1, 20, "Controller Pak", text_alpha | 0xc0000000);

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

        ST_DrawString(60, (i - linepos) * 15 + 65, buffer, text_alpha | 0xc0000000);
    }

    if (linepos != 0) {
        ST_DrawString(60, 50, "\x8f more...", text_alpha | 0xffffff00);
    }

    if ((linepos + 6) <= ((Pak_Size >> 5) - 1)) {
        ST_DrawString(60, 155, "\x8e more...", text_alpha | 0xffffff00);
    }

    ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 56, MenuAnimationTic + 70, text_alpha | 0xffffff00);
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

void M_LoadGamePakStop(void)
{
    S_StartSound(NULL, sfx_pistol);
    M_FadeOutStart(ga_exit);

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

    buttons = M_ButtonResponder(ticbuttons[0]);
    oldbuttons = oldticbuttons[0] & 0xffff0000;

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

                if (MAXSRAMSAVES - 1 < cursorpos)
                    cursorpos = MAXSRAMSAVES - 1;
                else
                    S_StartSound(NULL, sfx_switch1);

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

    if (((buttons & PAD_START) && !(oldbuttons & PAD_START))
            || ((buttons & PAD_B) && !(oldbuttons & PAD_B)))
    {
        exit = ga_exit;
    }
    else if (((buttons & PAD_A) && !(oldbuttons & PAD_A)) && I_IsSaveValid(&GamePak_Data[cursorpos]))
    {
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

    ST_DrawString(-1, 20, "Game Pak", text_alpha | 0xc0000000);

    for(i = linepos; i < (linepos + 6); i++)
    {
        levelsave_t *save = &GamePak_Data[i];

        if (!I_IsSaveValid(save))
            D_memmove(buffer, "no save");
        else
            M_PrintSaveTitle(buffer, save->skill, save->map);

        ST_DrawString(60, (i - linepos) * 15 + 65, buffer, text_alpha | 0xc0000000);
    }

    if (linepos != 0) {
        ST_DrawString(60, 50, "\x8f more...", text_alpha | 0xffffff00);
    }

    if ((linepos + 6) <= (MAXSRAMSAVES - 1)) {
        ST_DrawString(60, 155, "\x8e more...", text_alpha | 0xffffff00);
    }

    ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 56, MenuAnimationTic + 70, text_alpha | 0xffffff00);
}

int M_CenterDisplayTicker(void) // 8000B4C4
{
    unsigned int buttons, oldbuttons;
    int exit;

    buttons = M_ButtonResponder(ticbuttons[0]);
    oldbuttons = oldticbuttons[0] & 0xffff0000;

    if (((buttons & PAD_START) && !(oldbuttons & PAD_START))
            || ((buttons & PAD_B) && !(oldbuttons & PAD_B)))
    {
        S_StartSound(NULL, sfx_pistol);
        exit = ga_exit;
    }
    else
    {
        if (buttons & PAD_LEFT)
        {
            Display_X -= 1;
            if (Display_X < -16)
                Display_X = -16;
            else
                ConfigChanged = true;
        }
        else if (buttons & PAD_RIGHT)
        {
            Display_X += 1;
            if (Display_X > 24)
                Display_X = 24;
            else
                ConfigChanged = true;
        }

        if (buttons & PAD_UP)
        {
            Display_Y -= 1;
            if (Display_Y < -20)
                Display_Y = -20;
            else
                ConfigChanged = true;
        }
        else if (buttons & PAD_DOWN)
        {
            Display_Y += 1;
            if (Display_Y > 12)
                Display_Y = 12;
            else
                ConfigChanged = true;
        }

        if ((buttons & PAD_A) && !(oldbuttons & PAD_A) && (Display_X || Display_Y))
        {
            Display_X = 0;
            Display_Y = 0;
            ConfigChanged = true;
        }

        if (buttons & (ALL_JPAD|PAD_A))
            I_MoveDisplay(Display_X, Display_Y);

        exit = ga_nothing;
    }

    return exit;
}

void M_CenterDisplayDrawer(void) // 8000B604
{
    ST_DrawString(-1, 20, "Center Display", text_alpha | 0xc0000000);
    ST_DrawString(-1, 114, "use gamepad to adjust", text_alpha | 0xffffff00);
    ST_DrawString(-1, SCREEN_HT - 50, "press \x8a to reset", text_alpha | 0xffffff00);
    ST_DrawString(-1, SCREEN_HT - 30, "press \x8b to exit", text_alpha | 0xffffff00);
}

#define CONTROLCOLSIZE ((ARRAYLEN(ControlText) - 2) / 2)

int M_ControlPadTicker(void) // 8000B694
{
    unsigned int buttons;
    unsigned int stickbuttons;
    unsigned int oldbuttons;
    int exit;
    int code = 0;

    if ((gamevbls < gametic) && ((gametic & 3U) == 0)) {
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;
    }

    stickbuttons = M_ButtonResponder(ticbuttons[0] & 0xffff);
    buttons = ticbuttons[0] & 0xffff0000;
    oldbuttons = oldticbuttons[0] & 0xffff0000;

    if (!(stickbuttons & ALL_JPAD))
    {
        m_vframe1 = 0;
    }
    else
    {
        m_vframe1 = m_vframe1 - vblsinframe[0];
        if (m_vframe1 <= 0)
        {
            m_vframe1 = 0xf; // TICRATE / 2

            if (stickbuttons & PAD_DOWN)
            {
                cursorpos += 1;
                if (cursorpos >= ARRAYLEN(ControlText))
                    cursorpos = ARRAYLEN(ControlText) - 1;
                else
                    S_StartSound(NULL, sfx_switch1);
            }
            else if (stickbuttons & PAD_UP)
            {
                cursorpos -= 1;
                if (cursorpos < 0)
                    cursorpos = 0;
                else
                    S_StartSound(NULL, sfx_switch1);
            }
            else if (stickbuttons & PAD_RIGHT)
            {
                if (cursorpos < 2)
                {
                    buttons |= PAD_RIGHT;
                }
                else if (cursorpos < 2 + CONTROLCOLSIZE)
                {
                    S_StartSound(NULL, sfx_switch1);
                    cursorpos += CONTROLCOLSIZE;
                }
            }
            else if (stickbuttons & PAD_LEFT)
            {
                if (cursorpos < 2)
                {
                    buttons |= PAD_LEFT;
                }
                else if (cursorpos >= 2 + CONTROLCOLSIZE)
                {
                    S_StartSound(NULL, sfx_switch1);
                    cursorpos -= CONTROLCOLSIZE;
                }
            }
        }
    }

    if ((buttons & PAD_START) && !(oldbuttons & PAD_START))
    {
        S_StartSound(NULL, sfx_pistol);
        exit = ga_exit;
    }
    else
    {
        if (buttons == oldbuttons)
            exit = ga_nothing;
        else
        {
            if (cursorpos == 0) // Set Default Configuration
            {
                if (buttons & (PAD_RIGHT|PAD_A))
                {
                    ConfgNumb[0] += 1;
                    if(ConfgNumb[0] >= ARRAYLEN(DefaultControlSetups))
                        ConfgNumb[0] = 0;
                    ConfigChanged = true;
                }
                else if (buttons & PAD_LEFT)
                {
                    ConfgNumb[0] -= 1;
                    if (ConfgNumb[0] < 0)
                        ConfgNumb[0] = ARRAYLEN(DefaultControlSetups) - 1;
                    ConfigChanged = true;
                }
                else if ((buttons & PAD_B) && !(oldbuttons & PAD_B))
                {
                    S_StartSound(NULL, sfx_pistol);
                    return ga_exit;
                }
                else
                {
                    return ga_nothing;
                }

                D_memcpy(&CurrentControls[0], &DefaultControlSetups[ConfgNumb[0]], sizeof CurrentControls);
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

                int mode = CurrentControls[0].STICK_MODE;
                int modeindex = 0;

                for (int i = 0; i < ARRAYLEN(modes); i++)
                {
                    if (modes[i] == mode)
                    {
                        modeindex = i;
                        break;
                    }
                }
                if (buttons & (PAD_RIGHT|PAD_A))
                {
                    modeindex += 1;
                    if(modeindex >= ARRAYLEN(modes))
                        modeindex = 0;
                    ConfigChanged = true;
                }
                else if (buttons & PAD_LEFT)
                {
                    modeindex -= 1;
                    if (modeindex < 0)
                        modeindex = ARRAYLEN(modes) - 1;
                    ConfigChanged = true;
                }
                else if ((buttons & PAD_B) && !(oldbuttons & PAD_B))
                {
                    S_StartSound(NULL, sfx_pistol);
                    return ga_exit;
                }
                else
                {
                    return ga_nothing;
                }

                ConfgNumb[0] = -1;
                CurrentControls[0].STICK_MODE = modes[modeindex];
                S_StartSound(NULL,sfx_switch2);
            }
            else // Set Custom Configuration
            {
                for (int i = 16; i < 32; i++)
                {
                    if (buttons & (1 << i))
                    {
                        code = (1 << i);
                        break;
                    }
                }
                if (!code)
                    return ga_nothing;

                int m = ControlMappings[cursorpos - 2];

                ConfgNumb[0] = -1;
                CurrentControls[0].BUTTONS[m]
                    = CurrentControls[0].BUTTONS[m] == code ? 0 : code;
                /* if setting, unset anything previously bound to this button */
                if (CurrentControls[0].BUTTONS[m])
                {
                    for (int i = 0; i < ARRAYLEN(CurrentControls[0].BUTTONS); i++)
                        if (i != m && CurrentControls[0].BUTTONS[i] == buttons)
                            CurrentControls[0].BUTTONS[i] = 0;
                }
                S_StartSound(NULL,sfx_switch2);
                ConfigChanged = true;
            }
            exit = ga_nothing;
        }
    }
    return exit;
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

    ST_DrawString(-1, 20, "Controls", text_alpha | 0xc0000000);

    if (ConfgNumb[0] == -1)
        sprintf(buffer, ControlText[0], "Custom");
    else
        sprintf(buffer, ControlText[0], ControlSetupNames[ConfgNumb[0]]);
    ST_Message(20, 40, buffer, text_alpha | 0xc0000000);

    c = sprintf(buffer, ControlText[1]);
    stick = CurrentControls[0].STICK_MODE;
    if (stick & STICK_MOVE)
        c += sprintf(&buffer[c], " move");
    else if (stick & STICK_VLOOK)
        c += sprintf(&buffer[c], " look");
    if (stick & STICK_STRAFE)
        sprintf(&buffer[c], " strafe");
    else if (stick & STICK_TURN)
        sprintf(&buffer[c], " turn");
    ST_Message(20, 50, buffer, text_alpha | 0xc0000000);

    for(int i = 2; i < ARRAYLEN(ControlText); i++)
    {
        int code = CurrentControls[0].BUTTONS[ControlMappings[i - 2]];
        int x = (i < (2 + CONTROLCOLSIZE)) ? 10 : 170;
        int y = ((i - 2) % CONTROLCOLSIZE) * 16 + 68;
        int len = D_strlen(ControlText[i]);

        ST_Message(x + 98 - len * 8, y + 3, ControlText[i], text_alpha | 0xc0000000);
        if (code)
        {
            int symbol = button_code_to_symbol_index(code);
            ST_DrawSymbol(x + 120, y, symbol, text_alpha | 0xffffff00);
        }
    }

    if (cursorpos < 2)
        ST_DrawSymbol(10, cursorpos * 10 + 39, 78, text_alpha | 0x90600000);
    else if (cursorpos < 2 + CONTROLCOLSIZE)
        ST_DrawSymbol(116, ((cursorpos - 2) * 16) + 69, 78, text_alpha | 0x90600000);
    else
        ST_DrawSymbol(276, (cursorpos - 2) % CONTROLCOLSIZE * 16 + 70 , 78, text_alpha | 0x90600000);

    ST_DrawString(-1, SCREEN_HT - 20, "press \x8d to exit", text_alpha | 0xffffff00);
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

    ST_DrawString(-1, 20, "Custom Difficulty", text_alpha | 0xc0000000);

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
            ST_Message(item->x + 152, item->y, text, text_alpha | 0xc0000000);

        ST_Message(item->x, item->y, MenuText[casepos], text_alpha | 0xc0000000);
        item++;
    }


    ST_DrawSymbol(MenuItem->x -10, MenuItem[cursorpos].y -2, 78, text_alpha | 0x90600000);
}
