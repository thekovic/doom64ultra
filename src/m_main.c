/* m_main.c -- menu routines */

#include "doomdef.h"
#include "r_local.h"
#include "st_main.h"

extern void P_RefreshBrightness(void);
extern void P_RefreshVideo(void);

static int M_MenuCreditsTicker(void);
static void M_IdCreditsDrawer(void);
static void M_WmsCreditsDrawer(void);

//intermission
int DrawerStatus;

#define CT_TXT00	"default: %s"
#define CT_TXT01	"  stick:"

#define CT_TXT02	"move forward"
#define CT_TXT03	"move backward"
#define CT_TXT04	"strafe left"
#define CT_TXT05	"strafe right"
#define CT_TXT06	"strafe on"
#define CT_TXT07	"attack"
#define CT_TXT08	"run"
#define CT_TXT09	"jump"
#define CT_TXT10	"crouch"

#define CT_TXT11	"turn left"
#define CT_TXT12	"turn right"
#define CT_TXT13	"look up"
#define CT_TXT14	"look down"
#define CT_TXT15	"look on"
#define CT_TXT16	"use"
#define CT_TXT17	"weapon up"
#define CT_TXT18	"weapon down"
#define CT_TXT19	"map"

char *ControlText[] =   //8007517C
{
    CT_TXT00, CT_TXT01, CT_TXT02, CT_TXT03, CT_TXT04,
	CT_TXT05, CT_TXT06, CT_TXT07, CT_TXT08, CT_TXT09,
	CT_TXT10, CT_TXT11, CT_TXT12, CT_TXT13, CT_TXT14,
    CT_TXT15, CT_TXT16, CT_TXT17, CT_TXT18, CT_TXT19
};

u8 ControlMappings[] = {
    2,  3,  9, 10,  8,  4,  7, 16, 17,
    1,  0, 14, 15,  13, 5, 12, 11, 6
};

#define M_TXT00	"Gamepad"
#define M_TXT01	"Volume"
#define M_TXT02	"Video"
#define M_TXT03	"Password"
#define M_TXT04	"Main Menu"
#define M_TXT05	"Restart Level"
#define M_TXT06	"\x90 Return"
#define M_TXT07 "Music Volume"
#define M_TXT08 "Sound Volume"
#define M_TXT09 "Brightness"
#define M_TXT10 "Resume"
#define M_TXT11 "Options"
#define M_TXT12 "Autorun:"
#define M_TXT13 "Defaults" // default for video
#define M_TXT14 "New Game"
#define M_TXT15 "Be Gentle!"
#define M_TXT16 "Bring It On!"
#define M_TXT17 "I Own Doom!"
#define M_TXT18 "Watch Me Die!"
#define M_TXT19 "Be Merciless!"
#define M_TXT20 "Yes"
#define M_TXT21 "No"
#define M_TXT22 "Cheats"
#define M_TXT69 "Show Credits"   // [Immorpher] Credits
#define M_TXT23 "WARP TO LEVEL"
#define M_TXT24 "INVULNERABLE"
#define M_TXT25 "HEALTH BOOST"
#define M_TXT26 "SECURITY KEYS"
#define M_TXT27 "WEAPONS"
#define M_TXT85 "ARTIFACTS"
#define M_TXT28 "Exit"
#define M_TXT29 "DEBUG"
#define M_TXT30 "TEXTURE TEST"
#define M_TXT31 "WALL BLOCKING"
#define M_TXT32 "Center Display"
#define M_TXT33 "Messages:"
#define M_TXT34 "Opacity"
#define M_TXT35 "LOCK MONSTERS"
#define M_TXT36 "SCREENSHOT"
#define M_TXT37 "MAP EVERYTHING"
#define M_TXT38 "MACRO PEEK"
#define M_TXT39 "MUSIC TEST"
#define M_TXT40 "WARP TO FUN"
#define M_TXT41 "Movement"
#define M_TXT42 "Original" // Original default for Doom 64
#define M_TXT43 "Sensitivity"
#define M_TXT87 "Vert Look:"
#define M_TXT88 "Autoaim:"
#define M_TXT44 "Manage Pak"
#define M_TXT45 "Do not use Pak"
#define M_TXT46 "Try again"
#define M_TXT47 "Create game note"

// New additions to Doom 64 RE and Merciless Edition
#define M_TXT48 "COLORS"     // [GEC] NEW CHEAT CODE
#define M_TXT49 "FULL BRIGHT"   // [GEC] NEW CHEAT CODE
#define M_TXT50 "Filtering:"   // [GEC] New video filter option
#define M_TXT51 "Display"   // [Immorpher] New menu item
#define M_TXT52 "Motion Bob"   // [Immorpher] New menu item
#define M_TXT53 "Dither Filter:" // [Immorpher] Dither Filter
#define M_TXT54 "Anti-Aliasing:" // [Immorpher] New anti-aliasing option
#define M_TXT55 "Interlacing:" // [Immorpher] New interlacing option
#define M_TXT56 "Color Dither:" // [Immorpher] New color dither options
#define M_TXT57 "Flash Brightness" // [Immorpher] New flash brightness option
#define M_TXT58 "Merciless" // [Immorpher] Merciless default settings
#define M_TXT59 "Immorpher" // [Immorpher] Immorpher default settings
#define M_TXT60 "Accessible" // [Immorpher] Increased accessibility default settings
#define M_TXT61 "Story Text:" // [Immorpher] Skip cut scenes
#define M_TXT62 "Map Stats:" // [Immorpher] Display automap statistics
#define M_TXT63 "Status HUD" // [Immorpher] New menu option for HUD elements!
#define M_TXT64 "Margin" // [Immorpher] Adjust the margin for the HUD
#define M_TXT65 "WARP TO MOTHER" // [Immorpher] New features menu warps
#define M_TXT66 "WARP TO SECRET" // [Immorpher] New features menu warps
#define M_TXT67 "Colored:" // [Immorpher] Colored hud
#define M_TXT86 "Crosshair:" // [Immorpher] Crosshair
#define M_TXT68 "GAMMA CORRECT"   // [Immorpher] NEW CHEAT CODE

// Merciless Edition Credits
#define M_TXT70 "PROGRAMMING          IMMORPHER"   // [Immorpher] Credits
#define M_TXT71 "JNMARTIN84"   // [Immorpher] Credits
#define M_TXT72 "NOVA"   // [Immorpher] Credits
#define M_TXT73 "REVERSE ENGINEERING  ERICK194"   // [Immorpher] Credits
#define M_TXT74 "KAISER"   // [Immorpher] Credits
#define M_TXT75 "BODB DEARG"   // [Immorpher] Credits
#define M_TXT76 "QUASAR"   // [Immorpher] Credits
#define M_TXT77 "COMPILER ASSETS      CRASHOVERIDE"   // [Immorpher] Credits
#define M_TXT78 "ALPHATANGO"   // [Immorpher] Credits
#define M_TXT79 "PLAY TESTING"   // [Immorpher] Credits
#define M_TXT80 "BUU342, IRL RANDOM HAJILE"   // [Immorpher] Credits
#define M_TXT81 "SCD, TAUFAN99"   // [Immorpher] Credits
#define M_TXT82 "SPECIAL THANKS"   // [Immorpher] Credits
#define M_TXT83 "GEC TEAM, DOOMWORLD, DOOM 64 DISCORD"   // [Immorpher] Credits
#define M_TXT84 "NEIGH WINNY, ISANN KEKET, NEVANDER"   // [Immorpher] Credits

char *MenuText[] =   // 8005ABA0
{
    M_TXT00, M_TXT01, M_TXT02, M_TXT03, M_TXT04,
    M_TXT05, M_TXT06, M_TXT07, M_TXT08, M_TXT09,
    M_TXT10, M_TXT11, M_TXT12, M_TXT13, M_TXT14,
    M_TXT15, M_TXT16, M_TXT17, M_TXT18, M_TXT19,
    M_TXT20, M_TXT21, M_TXT22, M_TXT23, M_TXT24,
    M_TXT25, M_TXT26, M_TXT27, M_TXT28, M_TXT29,
    M_TXT30, M_TXT31, M_TXT32, M_TXT33, M_TXT34,
    M_TXT35, M_TXT36, M_TXT37, M_TXT38, M_TXT39,
    M_TXT40, M_TXT41, M_TXT42, M_TXT43, M_TXT44,
    M_TXT45, M_TXT46, M_TXT47, M_TXT48, M_TXT49,
    M_TXT50, M_TXT51, M_TXT52, M_TXT53, M_TXT54,
    M_TXT55, M_TXT56, M_TXT57, M_TXT58, M_TXT59,
    M_TXT60, M_TXT61, M_TXT62, M_TXT63, M_TXT64,
    M_TXT65, M_TXT66, M_TXT67, M_TXT68, M_TXT69,
    M_TXT70, M_TXT71, M_TXT72, M_TXT73, M_TXT74,
    M_TXT75, M_TXT76, M_TXT77, M_TXT78, M_TXT79,
    M_TXT80, M_TXT81, M_TXT82, M_TXT83, M_TXT84,
    M_TXT85, M_TXT86, M_TXT87, M_TXT88
};

menuitem_t Menu_Title[] = // 8005A978
{
    { 14, 115, 170 },   // New Game
    { 3, 115, 190 },   // Password
	{ 11, 115, 210 },   // Options
};

menuitem_t Menu_Skill[] = // 8005A990
{
    { 15, 102, 70 },    // Be Gentle!
    { 16, 102, 90},    // Bring it on!
    { 17, 102, 110},    // I own Doom!
    { 18, 102, 130},    // Watch me die!
    { 19, 102, 150},    // Be merciless!
    { 6, 102, 180},    	// Return
};

menuitem_t Menu_Options[] = // 8005A9C0
{
    {  0, 112, 60 },    // Gamepad
    { 41, 112, 80 },    // Movement
    {  1, 112, 100},    // Volume
    {  2, 112, 120},    // Video
    { 51, 112, 140},    // Display
    { 63, 112, 160},    // Status HUD
    { 13, 112, 180},    // Default settings
    {  6, 112, 200},    // Return
};

menuitem_t Menu_Volume[] = // 8005AA08
{
    {  7, 82, 60 },    // Music Volume
    {  8, 82, 100},    // Sound Volume
    {  6, 82, 140},    // Return
};

menuitem_t Menu_Movement[] = // [Immorpher] Movement
{
    { 52, 82, 60 },    // Motion Bob
    { 43, 82, 100 },   // Sensitivity
    { 12, 82, 140},    // Autorun
    { 87, 82, 160},    // Vertical Look
    { 88, 82, 180},    // Autoaim
    {  6, 82, 200},    // Return
};

menuitem_t Menu_Video[] = // 8005AA5C
{
    {  9, 82, 60 },    // Brightness
    { 50, 82, 100},    // Video Filter
    { 56, 82, 120},    // Color Dither
    { 54, 82, 140},    // Anti-Aliasing
    { 55, 82, 160},    // Interlacing
    { 53, 82, 180},    // Dither Filter
    {  6, 82, 200},    // Return
};

menuitem_t Menu_Display[] = // [Immorpher] Display menu
{
    { 57, 82, 60},    	// Flash Brightness
    { 32, 82, 100},    // Center Display
    { 61, 82, 120},    // Story Text
    { 62, 82, 140},    // Map Stats
    {  6, 82, 160},    // Return
};

menuitem_t Menu_StatusHUD[] = // [Immorpher] Status HUD
{
    { 64, 82, 60},   	// Margin
    { 34, 82, 100},    // Opacity
    { 67, 82, 140},    // Colored HUD
    { 86, 82, 160},    // Crosshair
    { 33, 82, 180},    // Messages
    {  6, 82, 200},    // Return
};

menuitem_t Menu_Defaults[] = // [Immorpher] Defaults menu
{
    { 42, 102, 60},    	// Original doom 64 defaults
    { 58, 102, 80},    	// Merciless edition defaults
    { 59, 102, 100},    	// Immorpher's defaults
    { 60, 102, 120},    	// Accessibility defaults
    {  6, 102, 140},    	// Return
};

menuitem_t Menu_Game[] = // 8005AAA4
{
    { 3, 122, 60 },    // Password
    { 11, 122, 80 },    // Options
    {  4, 122, 100},    // Main Menu
    {  5, 122, 120},    // Restart Level
    { 22, 122, 140},    // Cheats
};

menuitem_t Menu_Quit[] = // 8005AAD4
{
    { 20, 142, 100},    // Yes
    { 21, 142, 120},    // No
};

menuitem_t Menu_DeleteNote[] = // 8005AAEC
{
    { 20, 142, 100},    // Yes
    { 21, 142, 120},    // No
};

menuitem_t Menu_ControllerPakBad[] = // 8005AB04
{
    { 46, 120, 100},    // Try again
    { 45, 120, 120},    // Do not use Pak
};

menuitem_t Menu_ControllerPakFull[] = // 8005AB1C
{
    { 44, 110, 90 },    // Manage Pak
    { 47, 110, 110},    // Create game note
    { 45, 110, 130},    // Do not use Pak
};

menuitem_t Menu_CreateNote[] = // 8005AB40
{
    { 20, 110, 90 },    // Yes
    { 45, 110, 110},    // Do not use Pak
    { 44, 110, 130},    // Manage Pak
};

//#define MAXFEATURES 5 [Original]
//#define MAXFEATURES 9 [Doom 64 RE]
#define MAXFEATURES 14
menuitem_t Menu_Features[MAXFEATURES] = // 8005AB64
{
    { 24, 40, 50},      // INVULNERABLE
    { 25, 40, 60},      // HEALTH BOOST
    { 27, 40, 70},      // WEAPONS
    { 85, 40, 80},      // ARTIFACTS
    { 37, 40, 90},      // MAP EVERYTHING

    { 26, 40, 100},      // SECURITY KEYS
    { 31, 40, 110},      // WALL BLOCKING
    { 35, 40, 120},      // LOCK MONSTERS

    { 23, 40, 130},      // WARP TO LEVEL
    { 39, 40, 140},      // MUSIC TEST

    { 48, 40, 150},      // COLORS [GEC] NEW CHEAT CODE
    { 49, 40, 160},      // FULL BRIGHT [GEC] NEW CHEAT CODE
    { 68, 40, 170},      // Gamma correction [Immorpher] NEW CHEAT CODE
    { 69, 40, 190},      // Credits
};

menuitem_t Ultra_Credits[] = {
    {70, 20, 48},       // Credits
    {71, 188, 58},       // Credits
    {72, 188, 68}       // Credits
};

menuitem_t Merciless_Credits[] =
{
    {70, 20, 48},       // Credits

    {73, 20, 65},       // Credits
    {74, 188, 75},      // Credits
    {75, 188, 85},      // Credits
    {76, 188, 95},     // Credits

    {77, 20, 112},      // Credits
    {78, 188, 122},     // Credits

    {79, -1, 139},      // Credits
    {80, -1, 151},      // Credits
    {81, -1, 161},      // Credits

    {82, -1, 178},      // Credits
    {83, -1, 190},      // Credits
    {84, -1, 200},      // Credits
};

menudata_t MenuData[8]; // 800A54F0
int MenuAnimationTic;   // 800a5570
int cursorpos;          // 800A5574
int m_vframe1;          // 800A5578
menuitem_t *MenuItem;   // 800A5578
int itemlines;          // 800A5580
menufunc_t MenuCall;    // 800A5584

int linepos;            // 800A5588
int text_alpha_change_value;    // 800A558C
int MusicID;            // 800A5590
int m_actualmap;        // 800A5594
int last_ticon;         // 800A5598

skill_t startskill;     // 800A55A0
int startmap;           // 800A55A4
int EnableExpPak;       // 800A55A8

//-----------------------------------------

int MenuIdx = 0;                // 8005A7A4
int text_alpha = 255;           // 8005A7A8
int ConfgNumb[MAXPLAYERS] = {0}; // 8005A7AC
int Display_X = 0;              // 8005A7B0
int Display_Y = 0;              // 8005A7B4
boolean enable_messages = true; // 8005A7B8
int HUDopacity = 255;			// [Immorpher] HUD opacity
int SfxVolume = 100;             // 8005A7C0
int MusVolume = 80;             // 8005A7C4
int brightness = 125;             // 8005A7C8
fixed_t MotionBob = 0x100000; // [Immorpher] Motion Bob works in hexadecimal
int VideoFilter = 0; // [GEC & Immorpher] Set 3 point filtering on or off
boolean antialiasing = false; // [Immorpher] Anti-Aliasing
boolean interlacing = false; // [Immorpher] Interlacing
boolean DitherFilter = false; // [Immorpher] Dither filter
int ColorDither = 0; // [Immorpher] Color dithering options (Off, Square, Bayer, Noise)
int FlashBrightness = 32; // [Immorpher] Strobe brightness adjustment, will need to change to float
boolean runintroduction = false; // [Immorpher] New introduction sequence!
boolean StoryText = false; // [Immorpher] Skip story cut scenes?
boolean MapStats = false; // [Immorpher] Enable map statistics for automap?
int HUDmargin = 20; // [Immorpher] HUD margin options (default 20)
boolean ColoredHUD = true; // [Immorpher] Colored hud

boolean ConfigChanged = false;

controls_t CurrentControls[MAXPLAYERS] __attribute__((aligned(16))) = {
{
    {{
    PAD_RIGHT, PAD_LEFT, PAD_UP, PAD_DOWN,
    PAD_Z_TRIG,
    PAD_RIGHT_C, PAD_UP_C, PAD_LEFT_C, PAD_DOWN_C,
    PAD_L_TRIG, PAD_R_TRIG, PAD_A, PAD_B,
    }},
    STICK_MOVE | STICK_TURN
}
};

char *ControlSetupNames[MAXCONTROLSETUPS] =
{
    "Classic", "Modern", "Arcade", "Dark", "Hunter", "Action", "Nuke", "Fighter",
};

controls_t DefaultControlSetups[MAXCONTROLSETUPS] __attribute__((aligned(16))) = {
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
    startskill = sk_easy;
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
        return ga_timeout;

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

            // Free Pak_Data
            if (Pak_Data)
            {
                Z_Free(Pak_Data);
                Pak_Data = NULL;
            }

            if(ret == 0)
            {
                exit = ga_nothing;
                break;
            }

            // Create Controller Pak Note
            SET_MENU(Menu_CreateNote);
            MenuCall = M_MenuTitleDrawer;
            cursorpos = 0;

            MiniLoop(M_FadeInStart, NULL, M_MenuTicker, M_MenuGameDrawer);
            M_FadeOutStart(ga_exit);

            if (cursorpos != 0)
            {
                exit = ga_exit;
                break;
            }

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

            if (cursorpos != 1)
            {
                exit = ga_exit;
                break;
            }
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

            if (cursorpos != 0)
            {
                exit = ga_exit;
                break;
            }
        }
    }

    return exit;
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
    text_alpha_change_value = 20;
}

void M_AlphaOutStart(void) // 800079F8
{
    text_alpha = 255;
    text_alpha_change_value = -20;
}

int M_AlphaInOutTicker(void) // 80007A14
{
    if ((gamevbls < gametic) && ((gametic & 3U) == 0)) {
        MenuAnimationTic = (MenuAnimationTic + 1) & 7;
    }

    text_alpha += text_alpha_change_value;
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

    // Start Menu Fade In
    if (alpha_in)
        MiniLoop(M_AlphaInStart, NULL, M_AlphaInOutTicker, M_MenuGameDrawer);
}

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

        gDPPipeSync(GFX1++);
        gDPSetCycleType(GFX1++, G_CYC_FILL);
        gDPSetRenderMode(GFX1++,G_RM_NOOP,G_RM_NOOP2);
        gDPSetColorImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, OS_K0_TO_PHYSICAL(cfb[vid_side]));
        gDPSetFillColor(GFX1++, GPACK_RGBA5551(0,0,0,0) << 16 | GPACK_RGBA5551(0,0,0,0));
        gDPFillRectangle(GFX1++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);

        M_DrawBackground(56, 57, 80, "TITLE");

        if (MenuItem != Menu_Title) {
            M_DrawOverlay(0, 0, 320, 240, 96);
        }

        MenuCall();
        I_DrawFrame();
    }
}

extern mobj_t mobjhead;
extern mapthing_t *spawnlist;   // 800A5D74
extern int spawncount;          // 800A5D78
extern int gobalcheats; // [GEC]

int M_MenuTicker(void) // 80007E0C
{
    unsigned int buttons, oldbuttons;
    int exit;
    int truebuttons;
    int ret;
    int i;
    boolean padrepeat = false;
    mobj_t *m;

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

                    S_StartSound(NULL, sfx_switch1);
                }
                else if (buttons & PAD_UP)
                {
                    cursorpos -= 1;

                    if (cursorpos < 0)
                        cursorpos = itemlines-1;

                    S_StartSound(NULL, sfx_switch1);
                }
            }
        }

        if (((buttons & PAD_START) && !(oldticbuttons[0] & PAD_START))
                || ((buttons & PAD_B) && !(oldticbuttons[0] & PAD_B)))
        {
            if ((MenuItem == Menu_Title) ||
                (MenuItem == Menu_ControllerPakBad) ||
                (MenuItem == Menu_CreateNote) ||
                (MenuItem == Menu_ControllerPakFull))
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

                switch(MenuItem[cursorpos].casepos)
                {

                case 0: // Gamepad
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

                case 1: // Volume
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

                case 2: // Video
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

                case 3: // Password
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        ret = I_CheckControllerPak();
                        exit = ga_exit;

                        if (ret == 0)
                        {
                            if (I_ReadPakFile() == 0)
                            {
                                EnableExpPak = 1;
                                MenuCall = M_LoadPakDrawer;
                                exit = MiniLoop(M_LoadPakStart,M_LoadPakStop,M_LoadPakTicker,M_MenuGameDrawer);
                            }
                            else
                                exit = ga_exit;
                        }

                        if (exit == ga_exit)
                        {
                            MenuCall = M_PasswordDrawer;
                            exit = MiniLoop(M_PasswordStart,M_PasswordStop,M_PasswordTicker,M_MenuGameDrawer);
                        }

                        if (exit == ga_exit)
                        {
                            M_RestoreMenuData(true);
                            return ga_nothing;
                        }

                        if (EnableExpPak != 0)
                        {
                            return exit;
                        }

                        EnableExpPak = (M_ControllerPak() == 0);
                        return exit;
                    }
                    break;

                case 4: // Main Menu
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

                case 5: // [Immorpher] Updated Restart Level
                    if (truebuttons)
                    {
						
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        SET_MENU(Menu_Skill);
                        MenuCall = M_MenuTitleDrawer;
                        cursorpos = gameskill;  // Set default to current difficulty

                        exit = MiniLoop(M_FadeInStart, M_FadeOutStart, M_MenuTicker, M_MenuGameDrawer);

                        if (exit == ga_exit) {
							M_RestoreMenuData(true);
                            return ga_nothing;
						}
						
                        gameskill = cursorpos;
						
						startmap = gamemap;
						startskill = gameskill;
						G_InitSkill (gameskill); // [Immorpher] initialize new skill
						
						return ga_warped;
                    }
                    break;

                case 6: // Return
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        return ga_exit;
                    }
                    break;

                case 7: // Music Volume
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

                case 8: // Sound Volume
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

                case 9: // Brightness
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

                case 11: // Options
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

                case 12: // Autorun
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        playerconfigs[0].autorun ^= true;
                        return ga_nothing;
                    }
                    break;

                case 13: // Set defaults
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

                case 14: // New Game
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
						
                        M_SaveMenuData();

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

                        startskill = cursorpos;
						
                        // Check ControllerPak
                        EnableExpPak = (M_ControllerPak() == 0);

                        return ga_exit;
                    }
                    break;

                case 15: // Be Gentle!
                case 16: // Bring it on!
                case 17: // I own Doom!
                case 18: // Watch me die!
				case 19: // Be merciless!
                
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        return ga_warped;
                    }
                    break;

                case 20: // Yes
                case 46: // Try again
                case 47: // Create game note
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        return ga_exitdemo;
                    }
                    break;

                case 21: // No
                case 45: // Do not use Pak
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        return ga_exit;
                    }
                    break;

                case 22: // Cheats
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        players[0].cheats &= 0xffff1fff;

                        SET_MENU(Menu_Features);
                        MenuCall = M_FeaturesDrawer;
                        cursorpos = 0;
                        m_actualmap = gamemap;

                        exit = MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                        M_RestoreMenuData((exit == ga_exit));

                        if (exit == ga_exit)
                            return ga_nothing;

                        return exit;
                    }
                    break;

                case 23: // WARP TO LEVEL
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
                    
                case 24: // INVULNERABLE
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        players[0].cheats ^= CF_GODMODE;
                        return ga_nothing;
                    }
                    break;

                case 25: // HEALTH BOOST
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        players[0].cheats |= CF_HEALTH;
                        players[0].health = 100;
                        players[0].mo->health = 100;
                        return ga_nothing;
                    }
                    break;

                case 26: // SECURITY KEYS
                    /* Not available in the release code */
                    /*
                    Reconstructed code based on Psx Doom
                    */
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        players[0].cheats |= CF_ALLKEYS;

                        for (m = mobjhead.next; m != &mobjhead; m = m->next)
                        {
                            switch (m->type)
                            {
                            case MT_ITEM_BLUECARDKEY:
                                players[0].cards[it_bluecard] = true;
                                break;
                            case MT_ITEM_REDCARDKEY:
                                players[0].cards[it_redcard] = true;
                                break;
                            case MT_ITEM_YELLOWCARDKEY:
                                players[0].cards[it_yellowcard] = true;
                                break;
                            case MT_ITEM_YELLOWSKULLKEY:
                                players[0].cards[it_yellowskull] = true;
                                break;
                            case MT_ITEM_REDSKULLKEY:
                                players[0].cards[it_redskull] = true;
                                break;
                            case MT_ITEM_BLUESKULLKEY:
                                players[0].cards[it_blueskull] = true;
                                break;
                            default:
                                break;
                            }
                        }

                        for (i = 0; i < spawncount; i++)
                        {
                            switch (spawnlist[i].type)
                            {
                            case 5:
                                players[0].cards[it_bluecard] = true;
                                break;
                            case 13:
                                players[0].cards[it_redcard] = true;
                                break;
                            case 6:
                                players[0].cards[it_yellowcard] = true;
                                break;
                            case 39:
                                players[0].cards[it_yellowskull] = true;
                                break;
                            case 38:
                                players[0].cards[it_redskull] = true;
                                break;
                            case 40:
                                players[0].cards[it_blueskull] = true;
                                break;
                            default:
                                break;
                            }
                        }

                        return ga_nothing;
                    }
                    break;

                case 27: // WEAPONS
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        players[0].cheats |= CF_WEAPONS;

                        for(i = 0; i < NUMWEAPONS; i++) {
                            players[0].weaponowned[i] = true;
                        }

                        if (!players[0].backpack)
                        {
                            for (i=0 ; i<NUMAMMO ; i++)
                                players[0].maxammo[i] *= 2;
                            players[0].backpack = true;
                        }

                        for(i = 0; i < NUMAMMO; i++) {
                            players[0].ammo[i] = players[0].maxammo[i];
                        }

                        return ga_nothing;
                    }
                    break;

                case 28: // Exit
                    /* nothing special */
                    break;

                case 29: // DEBUG
                    /* Not available in the release code */
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        players[0].cheats ^= CF_DEBUG;
                        return ga_nothing;
                    }
                    break;

                case 30: // TEXTURE TEST
                    /* Not available in the release code */
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        players[0].cheats ^= CF_TEX_TEST;
                        return ga_nothing;
                    }
                    break;

                case 31: // WALL BLOCKING
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

                case 32: // Center Display
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

                case 33: // Messages
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        enable_messages ^= true;
                        ConfigChanged = true;
                        return ga_nothing;
                    }
                    break;

                case 34: // [Immorpher] HUD opacity
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

                case 35: // LOCK MONSTERS
                    /* Not available in the release code */
                    /*
                    Reconstructed code based on Doom 64 Ex
                    */
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        players[0].cheats ^= CF_LOCKMOSTERS;
                        return ga_nothing;
                    }
                    break;

                case 36: // SCREENSHOT
                    /* Not available in the release code */
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        players[0].cheats ^= CF_SCREENSHOT;
                        return ga_nothing;
                    }
                    break;

                case 37: // MAP EVERYTHING
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        players[0].cheats ^= CF_ALLMAP;
                        return ga_nothing;
                    }
                    break;

                case 38: // MACRO PEEK
                    /* Not available in the release code */
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        players[0].cheats ^= CF_MACROPEEK;
                        return ga_nothing;
                    }
                    break;

                case 39: // MUSIC TEST
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

                case 41: // Control Stick
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

                case 42: // Original Doom 64 defaults
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_switch2);
						
						// Set movement/controller options
						MotionBob = 0x100000;

						// Set video options
                        brightness = 0;
						VideoFilter = 0; // [Immorpher] new video option
						antialiasing = false; // [Immorpher] new video option
						interlacing = false;  // [Immorpher] new video option
						DitherFilter = false;  // [Immorpher] new video option
						ColorDither = 0;  // [Immorpher] new video option
						
						// Set display options
						FlashBrightness = 32;  // [Immorpher] new video option
						StoryText = true; // [Immorpher] Skip story cut scenes?
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
                            ConfgNumb[i] = 0;    // gamepad configuration
                            D_memcpy(&CurrentControls[i], &DefaultControlSetups[ConfgNumb[i]], sizeof CurrentControls);
                            playerconfigs[i].crosshair = 0;
                            playerconfigs[i].sensitivity = 0;
                            playerconfigs[i].verticallook = 1;
                            playerconfigs[i].autorun = false;
                            playerconfigs[i].autoaim = true;
                        }
                        I_MoveDisplay(0,0);
                        P_RefreshBrightness();
						P_RefreshVideo();
                        S_SetMusicVolume(MusVolume);
                        S_SetSoundVolume(SfxVolume);
						
                        ConfigChanged = true;
                        return ga_nothing;
                    }
                    break;

                case 43: // Sensitivity
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

                case 44: // Manage Pak
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

                case 48: // COLORS [GEC] NEW CHEAT CODE
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        players[0].cheats ^= CF_NOCOLORS;
                        gobalcheats ^= CF_NOCOLORS;
                        P_RefreshBrightness();
                        return ga_nothing;
                    }
                    break;

                case 49: // FULL BRIGHT [GEC] NEW CHEAT CODE
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        players[0].cheats ^= CF_FULLBRIGHT;
                        gobalcheats ^= CF_FULLBRIGHT;
                        P_RefreshBrightness();
                        return ga_nothing;
                    }
                    break;
					
                case 50: // [GEC and Immorpher] Video filtering mode
                    if (truebuttons || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        VideoFilter += 1;
						if (VideoFilter > 2)
						{
							VideoFilter = 0;
						}
                        ConfigChanged = true;
                        return ga_nothing;
                    }
                    else if ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        VideoFilter -= 1;
						if (VideoFilter < 0)
						{
							VideoFilter = 2;
						}
                        ConfigChanged = true;
                        return ga_nothing;
                    }
                    break;
			
                case 51: // Display
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_pistol);
                        M_SaveMenuData();

                        SET_MENU(Menu_Display);
                        MenuCall = M_DisplayDrawer;
                        cursorpos = 0;

                        MiniLoop(M_FadeInStart,M_FadeOutStart,M_MenuTicker,M_MenuGameDrawer);
                        M_RestoreMenuData(true);
                        return ga_nothing;
                    }
                    break;
					
                case 52: // Motion Bob
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

                case 53: // [Immorpher] Dither Filter
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        DitherFilter ^= true;
						P_RefreshVideo();
                        ConfigChanged = true;
                        return ga_nothing;
                    }
                    break;
					
                case 54: // [Immorpher] Anti-Aliasing
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        antialiasing ^= true;
						P_RefreshVideo();
                        ConfigChanged = true;
                        return ga_nothing;
                    }
                    break;
					
                case 55: // [Immorpher] Interlacing
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        interlacing ^= true;
                        P_RefreshVideo();
                        ConfigChanged = true;
                        return ga_nothing;
                    }
                    break;

                case 56: // [Immorpher] Color Dither
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

                case 57: // Flash Brightness
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
					
                case 58: // Merciless edition defaults
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_switch2);
						
						// Set movement/controller options
						MotionBob = 0x100000;

						// Set video options
                        brightness = 100;
						VideoFilter = 0; // [Immorpher] new video option
						antialiasing = false; // [Immorpher] new video option
						interlacing = false;  // [Immorpher] new video option
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
                            playerconfigs[i].verticallook = 1;
                            playerconfigs[i].autorun = false;
                            playerconfigs[i].autoaim = true;
                        }
                        I_MoveDisplay(0,0);
                        P_RefreshBrightness();
						P_RefreshVideo();
                        S_SetMusicVolume(MusVolume);
                        S_SetSoundVolume(SfxVolume);
                        ConfigChanged = true;
						
                        return ga_nothing;
                    }
                    break;
					
                case 59: // Immorpher's defaults
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_switch2);
						
						// Set movement/controller options
						MotionBob = 0x100000;

						// Set video options
                        brightness = 200;
						VideoFilter = 1; // [Immorpher] new video option
						antialiasing = false; // [Immorpher] new video option
						interlacing = false;  // [Immorpher] new video option
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
                            ConfgNumb[i] = 6;    // gamepad configuration
                            D_memcpy(&CurrentControls[i], &DefaultControlSetups[ConfgNumb[i]], sizeof CurrentControls);
                            playerconfigs[i].crosshair = 2;
                            playerconfigs[i].sensitivity = 0;
                            playerconfigs[i].verticallook = 1;
                            playerconfigs[i].autorun = true;
                            playerconfigs[i].autoaim = false;
                        }
                        I_MoveDisplay(0,0);
                        P_RefreshBrightness();
						P_RefreshVideo();
                        S_SetMusicVolume(MusVolume);
                        S_SetSoundVolume(SfxVolume);
                        ConfigChanged = true;
						
                        return ga_nothing;
                    }
                    break;

                case 60: // Accessibility defaults
                    if (truebuttons)
                    {
                        S_StartSound(NULL, sfx_switch2);
						
						// Set movement/controller options
						MotionBob = 0x0;

						// Set video options
                        brightness = 200;
						VideoFilter = 0; // [Immorpher] new video option
						antialiasing = false; // [Immorpher] new video option
						interlacing = false;  // [Immorpher] new video option
						DitherFilter = false;  // [Immorpher] new video option
						ColorDither = 0;  // [Immorpher] new video option
						
						// Set display options
						FlashBrightness = 0;  // [Immorpher] new video option
						StoryText = true; // [Immorpher] Skip story cut scenes?
						MapStats = false; // [Immorpher] Display automap stats?
						
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
						P_RefreshVideo();
                        S_SetMusicVolume(MusVolume);
                        S_SetSoundVolume(SfxVolume);
                        ConfigChanged = true;
						
                        return ga_nothing;
                    }
                    break;	
					
                case 61: // [Immorpher] Story Text
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        StoryText ^= true;
                        return ga_nothing;
                    }
                    break;	
					
                case 62: // [Immorpher] Map stats
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        MapStats ^= true;
                        return ga_nothing;
                    }
                    break;	
							
                case 63: // [Immorpher] Status HUD
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
					
                case 64: // HUDmargin
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
					
                case 67: // [Immorpher] Colored HUD
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        ColoredHUD ^= true;
                        ConfigChanged = true;
                        return ga_nothing;
                    }
                    break;	

				case 68: // Gamma Correction [Immorpher] NEW CHEAT CODE
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        players[0].cheats ^= CF_GAMMA;
                        gobalcheats ^= CF_GAMMA;
                        P_RefreshVideo();
                        return ga_nothing;
                    }
                    break;

                case 69: // Credits
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

                case 85: // ARTIFACTS
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        players[0].artifacts |= 4;
                        players[0].artifacts |= 2;
                        players[0].artifacts |= 1;

                        S_StartSound(NULL, sfx_switch2);
                        return ga_nothing;
                    }
                    break;
                case 86: // [Immorpher] Crosshair
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
                case 87: // [nova] Vertical Look
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
                case 88: // [nova] Auto aim
                    if (truebuttons || ((buttons & PAD_LEFT) && !(oldbuttons & PAD_LEFT))
                            || ((buttons & PAD_RIGHT) && !(oldbuttons & PAD_RIGHT)))
                    {
                        S_StartSound(NULL, sfx_switch2);
                        playerconfigs[0].autoaim ^= true;
                        ConfigChanged = true;
                        return ga_nothing;
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
    menuitem_t *item;
    int i;

    if (MenuItem == Menu_Game)
    {
        ST_DrawString(-1, 20, "Pause", text_alpha | 0xc0000000);
        ST_DrawString(-1, 210, "press \x8b to resume", text_alpha | 0xffffff00);
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

    item = MenuItem;
    for(i = 0; i < itemlines; i++)
    {
        ST_DrawString(item->x, item->y, MenuText[item->casepos], text_alpha | 0xc0000000);
        item++;
    }

    ST_DrawSymbol(MenuItem[0].x -37, MenuItem[cursorpos].y -9, MenuAnimationTic + 70, text_alpha | 0xffffff00);
}

void M_FeaturesDrawer(void) // 800091C0
{
    char *text, textbuff[32];
    menuitem_t *item;
    int i;

    ST_DrawString(-1, 20, "Cheats", text_alpha | 0xc0000000);
    item = MenuItem;

    for(i = 0; i < itemlines; i++)
    {
        if ((item->casepos == 23) && ((m_actualmap >= 25) && (m_actualmap <= 27)))
        {
            /* Show "WARP TO FUN" text */
            ST_Message(item->x, item->y, MenuText[40], text_alpha | 0xffffff00);
        }
		else if ((item->casepos == 23) && (m_actualmap == 28)) // [Immorpher] Show "WARP TO MOTHER" text
		{ 
            ST_Message(item->x, item->y, MenuText[65], text_alpha | 0xffffff00);
		}
		else if ((item->casepos == 23) && (m_actualmap > 28)) // [Immorpher] Show "WARP TO SECRET" text
		{
            ST_Message(item->x, item->y, MenuText[66], text_alpha | 0xffffff00);
		}
        else
        {
            /* Show "WARP TO LEVEL" text */
            ST_Message(item->x, item->y, MenuText[item->casepos], text_alpha | 0xffffff00);
        }

        text = textbuff;
        switch(item->casepos)
        {
            case 23: /* WARP TO LEVEL */
                sprintf(textbuff, "%s", MapInfo[m_actualmap].name);
                break;
            case 24: /* INVULNERABLE */
                text = (!(players[0].cheats & CF_GODMODE)) ? "OFF": "ON";
                break;
            case 25: /* HEALTH BOOST */
                text = (!(players[0].cheats & CF_HEALTH)) ? "-" : "100%";
                break;
            case 26: /* SECURITY KEYS */
                text = (!(players[0].cheats & CF_ALLKEYS)) ? "-" : "100%";
                break;
            case 27: /* WEAPONS */
                text = (!(players[0].cheats & CF_WEAPONS)) ? "-" : "100%";
                break;
            case 28: /* Exit */
                break;
            case 29: /* DEBUG */
                text = (!(players[0].cheats & CF_DEBUG)) ? "OFF": "ON";
                break;
            case 30: /* TEXTURE TEST */
                text = (!(players[0].cheats & CF_TEX_TEST)) ? "OFF": "ON";
                break;
            case 31: /* WALL BLOCKING */
                text = (!(players[0].mo->flags & MF_NOCLIP)) ? "ON": "OFF";
                break;
            case 35: /* LOCK MONSTERS */
                text = (!(players[0].cheats & CF_LOCKMOSTERS)) ? "OFF": "ON";
                break;
            case 36: /* SCREENSHOT */
                text = (!(players[0].cheats & CF_SCREENSHOT)) ? "OFF": "ON";
                break;
            case 37: /* MAP EVERYTHING */
                text = (!(players[0].cheats & CF_ALLMAP)) ? "OFF": "ON";
                break;
            case 38: /* MACRO PEEK */
                text = ((players[0].cheats & CF_MACROPEEK)) ? "ON": "OFF";
                break;
            case 39: /* MUSIC TEST */
                sprintf(textbuff, "%d", MusicID);
                break;

            // [GEC] NEW CHEAT CODES
            case 48: /* COLORS */
                text = (!(players[0].cheats & CF_NOCOLORS)) ? "ON": "OFF";
                break;

            case 49: /* FULL BRIGHT */
                text = (!(players[0].cheats & CF_FULLBRIGHT)) ? "OFF": "ON";
                break;

            case 68: /* Gamma Correction */
                text = (!(players[0].cheats & CF_GAMMA)) ? "OFF": "ON";
                break;

            case 85: /* ARTIFACTS */
                text = (!(players[0].artifacts & 1 && players[0].artifacts & 2 && players[0].artifacts & 4)) ? "-" : "100%";
                break;

            default:
                text = ""; // [Immorpher] set to null for credits menu
                break;
        }

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

static void M_ModCreditsDrawer(menuitem_t *menu, int items)
{
    char *text;
    int i;
    int x;

    for(i = 0; i < items; i++)
    {
        x = menu->x;
        text = MenuText[menu->casepos];

        if (x < 0)
            x = (320 - D_strlen(text) * 8) / 2;

        ST_Message(x, menu->y, text, text_alpha | 0xffffff00);
        menu++;
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
    menuitem_t *item;
    int i;

    ST_DrawString(-1, 20, "Volume", text_alpha | 0xc0000000);
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
    menuitem_t *item;
    int i, casepos;

    ST_DrawString(-1, 20, "Movement", text_alpha | 0xc0000000);

    item = Menu_Movement;

    for(i = 0; i < itemlines; i++)
    {
        casepos = item->casepos;

        if (casepos == 12) // [Immorpher] Autorun
        {
            if (playerconfigs[0].autorun)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == 87) // [nova] Vertical Look
        {
            if (playerconfigs[0].verticallook == 1)
                text = "Normal";
            else
                text = "Inverted";
        }
        else if (casepos == 88) // [nova] Autoaim
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
		
        ST_DrawString(item->x, item->y, MenuText[item->casepos], text_alpha | 0xc0000000);
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
    menuitem_t *item;
    int i, casepos;

    ST_DrawString(-1, 20, "Video", text_alpha | 0xc0000000);

    item = Menu_Video;

    for(i = 0; i < 7; i++)
    {
        casepos = item->casepos;
		
        if (casepos == 50) // [GEC and Immorpher] New video filter
        {
            if (VideoFilter == 0)
                text = "On";
            else if (VideoFilter == 1)
                text = "Skies";
            else
                text = "Off";
        }		
        else if (casepos == 54) // [Immorpher] Anti-Aliasing
        {
            if (antialiasing)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == 55) // [Immorpher] Interlacing
        {
            if (interlacing)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == 53) // [Immorpher] Dither Filter
        {
            if (DitherFilter)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == 56) // [Immorpher] Dither Filter
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

        if (text)
			ST_DrawString(item->x + 140, item->y, text, text_alpha | 0xc0000000);

        ST_DrawString(item->x, item->y, MenuText[casepos], text_alpha | 0xc0000000);

        item++;
    }

    ST_DrawSymbol(82, 80, 68, text_alpha | 0xffffff00);
    ST_DrawSymbol(brightness/2 + 83, 80, 69, text_alpha | 0xffffff00);

    ST_DrawSymbol(Menu_Video[0].x - 37, Menu_Video[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00);
}

void M_DisplayDrawer(void) // 80009884
{
    char *text;
    menuitem_t *item;
    int i, casepos;

    ST_DrawString(-1, 20, "Display", text_alpha | 0xc0000000);

    item = Menu_Display;

    for(i = 0; i < itemlines; i++)
    {
        casepos = item->casepos;

        if (casepos == 61) // Story Text:
        {
            if (StoryText)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == 62) // Map stats:
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

        if (text)
            ST_DrawString(item->x + 130, item->y, text, text_alpha | 0xc0000000);

        ST_DrawString(item->x, item->y, MenuText[casepos], text_alpha | 0xc0000000);

        item++;
    }
	
	// Flash brightness
    ST_DrawSymbol(82, 80, 68, text_alpha | 0xffffff00);
	ST_DrawSymbol(100*FlashBrightness/32 + 83, 80, 69, text_alpha | 0xffffff00);

    ST_DrawSymbol(Menu_Display[0].x - 37, Menu_Display[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00);
}

void M_StatusHUDDrawer(void) // 80009884
{
    char *text;
    menuitem_t *item;
    int i, casepos;

    ST_DrawString(-1, 20, "Status HUD", text_alpha | 0xc0000000);

    item = Menu_StatusHUD;

    for(i = 0; i < itemlines; i++)
    {
        casepos = item->casepos;

        if (casepos == 33) // Messages:
        {
            if (enable_messages)
                text = "On";
            else
                text = "Off";
        }
        else if (casepos == 67) // Colored HUD:
        {
            if (ColoredHUD)
                text = "On";
            else
                text = "Off";
        }	
        else if (casepos == 86) // [Immorpher] Crosshair
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
        else
        {
            text = NULL;
        }

        if (text)
            ST_DrawString(item->x + 130, item->y, text, text_alpha | 0xc0000000);

        ST_DrawString(item->x, item->y, MenuText[casepos], text_alpha | 0xc0000000);

        item++;
    }
	
	// HUD Margin
    ST_DrawSymbol(82, 80, 68, text_alpha | 0xffffff00);
	ST_DrawSymbol(100*HUDmargin/20 + 83, 80, 69, text_alpha | 0xffffff00);
	
	// HUD Opacity
    ST_DrawSymbol(82, 120, 68, text_alpha | 0xffffff00);
    ST_DrawSymbol(100*HUDopacity/255 + 83, 120, 69, text_alpha | 0xffffff00);

    ST_DrawSymbol(Menu_StatusHUD[0].x - 37, Menu_StatusHUD[cursorpos].y - 9, MenuAnimationTic + 70, text_alpha | 0xffffff00);
}

void M_DefaultsDrawer(void) // [Immorpher] new defaults drawer
{
    menuitem_t *item;
    int i, casepos;

    ST_DrawString(-1, 20, "Set Defaults", text_alpha | 0xc0000000);

    item = Menu_Defaults;

    for(i = 0; i < itemlines; i++)
    {
        casepos = item->casepos;

        ST_DrawString(item->x, item->y, MenuText[casepos], text_alpha | 0xc0000000);

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
            (x << 2), (y << 2),
            ((width + x) << 2), ((yh + y) << 2),
            G_TX_RENDERTILE,
            (0 << 5), (t << 5),
            (1 << 10), (1 << 10));

        height -= yh;
        t += yh;
        y += yh;
    }

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
        if (buttons ^ oldbuttons)
        {
            if(buttons == (PAD_LEFT_C|PAD_RIGHT_C))
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
        }
        exit = ga_nothing;
    }
    return exit;
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

        ST_DrawString(-1, 210, "press \x8b to exit", text_alpha | 0xffffff00);
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

                    tmpbuf[0] = Pak_Table[idx];
                    tmpbuf++;
                }

                idx = (byte) fState->ext_name[0];
                if (idx != 0)
                {
                    tmpbuf[0] = '.';
                    tmpbuf[1] = Pak_Table[idx];
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

        sprintf(buffer, "pages used: %d   free: %d", FileState[cursorpos].file_size >> 8, Pak_Memory);

        ST_DrawString(-1, 170, buffer, text_alpha | 0xc0000000);
        ST_DrawSymbol(23, (cursorpos - linepos) * 15 + 51, MenuAnimationTic + 70, text_alpha | 0xffffff00);

        ST_DrawString(-1, 200, "press \x8b to exit", text_alpha | 0xffffff00);
        ST_DrawString(-1, 215, "press \x84\x85 to delete", text_alpha | 0xffffff00);
    }
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
                    if (Pak_Data[i * 32] == 0)
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
}

void M_SavePakStop(void) // 8000A7B4
{
    S_StartSound(NULL, sfx_pistol);

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
        if ((buttons != oldbuttons) && (buttons == (PAD_RIGHT_C|PAD_LEFT_C)))
        {
            // save the next level number and password data in text format
            if (gameskill == sk_baby) {
                sprintf((char*) &Pak_Data[cursorpos * 32], "level %2.2d - bg", nextmap);
            } else if (gameskill == sk_easy) {
                sprintf((char*) &Pak_Data[cursorpos * 32], "level %2.2d - bio", nextmap);
            } else if (gameskill == sk_medium) {
                sprintf((char*) &Pak_Data[cursorpos * 32], "level %2.2d - iod", nextmap);
            } else if (gameskill == sk_hard) {
                sprintf((char*) &Pak_Data[cursorpos * 32], "level %2.2d - wmd", nextmap);
            } else if (gameskill == sk_nightmare) {
                sprintf((char*) &Pak_Data[cursorpos * 32], "level %2.2d - bm", nextmap);
            } else {
                sprintf((char*) &Pak_Data[cursorpos * 32], "level %2.2d", nextmap);
            }
            D_memcpy(&Pak_Data[(cursorpos * 32) + 16], &Passwordbuff, 16);

            if (I_SavePakFile(File_Num, PFS_WRITE, Pak_Data, Pak_Size) == 0) {
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
    else if ((ticon - last_ticon) >= 60) // 2 * TICRATE
    {
        return ga_exit;
    }

    return ga_nothing;
}

void M_SavePakDrawer(void) // 8000AB44
{
    int i;
    char buffer[36];

    I_ClearFrame();

    gDPPipeSync(GFX1++);
    gDPSetCycleType(GFX1++, G_CYC_FILL);
    gDPSetRenderMode(GFX1++,G_RM_NOOP,G_RM_NOOP2);
    gDPSetColorImage(GFX1++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD, OS_K0_TO_PHYSICAL(cfb[vid_side]));
    // Fill borders with black
    gDPSetFillColor(GFX1++, GPACK_RGBA5551(0,0,0,0) << 16 | GPACK_RGBA5551(0,0,0,0)) ;
    gDPFillRectangle(GFX1++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);

    M_DrawBackground(63, 25, 128, "EVIL");

    ST_DrawString(-1, 20, "Controller Pak", text_alpha | 0xc0000000);

    if (FilesUsed == -1)
    {
        if (MenuAnimationTic & 2)
        {
            ST_DrawString(-1, 100, "Controller Pak removed!", 0xc00000ff);
            ST_DrawString(-1, 120, "Game cannot be saved.", 0xc00000ff);
        }

        ST_DrawString(-1, 210, "press \x8b to exit", text_alpha | 0xffffff00);
    }
    else
    {
        for(i = linepos; i < (linepos + 6); i++)
        {
            if (Pak_Data[i * 32] == 0) {
                D_memmove(buffer, "empty");
            }
            else {
                D_memmove(buffer, &Pak_Data[i * 32]);
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

        ST_DrawString(-1, 195, "press \x8b to exit", text_alpha | 0xffffff00);
        ST_DrawString(-1, 210, "press \x84\x85 to save", text_alpha | 0xffffff00);
    }

    I_DrawFrame();
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
            if (Pak_Data[i * 32])
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
    int skill;
    int levelnum;
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
    else
    {
        if (!(buttons ^ oldbuttons) || buttons != (PAD_RIGHT_C|PAD_LEFT_C) ||
            (Pak_Data[cursorpos * 32] == 0))
        {
            exit = ga_nothing;
        }
        else
        {
            // load the password data in text format
            D_memcpy(&Passwordbuff, &Pak_Data[((cursorpos * 32) + 16)], 16);

            if (M_DecodePassword(Passwordbuff, &levelnum, &skill, 0) == 0)
            {
                CurPasswordSlot = 0;
                exit = ga_exit;
            }
            else
            {
                doPassword = true;
                CurPasswordSlot = 16;

                startmap = gamemap = levelnum;
                startskill = gameskill = skill;

				G_InitSkill (gameskill); // [Immorpher] Initialize new game skill
                exit = ga_warped;
            }
        }
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
        if (FilesUsed == -1) {
            D_memmove(buffer, "-");
        }
        else if (Pak_Data[i * 32] == 0) {
            D_memmove(buffer, "no save");
        }
        else {
            D_memmove(buffer, &Pak_Data[i * 32]);
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

    ST_DrawString(-1, 195, "press \x8b to exit", text_alpha | 0xffffff00);
    ST_DrawString(-1, 210, "press \x84\x85 to load", text_alpha | 0xffffff00);
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
    ST_DrawString(-1, 190, "press \x8a to reset", text_alpha | 0xffffff00);
    ST_DrawString(-1, 210, "press \x8b to exit", text_alpha | 0xffffff00);
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

    ST_DrawString(-1, 20, "Gamepad", text_alpha | 0xc0000000);

    if (ConfgNumb[0] == -1)
        sprintf(buffer, ControlText[0], "Custom");
    else
        sprintf(buffer, ControlText[0], ControlSetupNames[ConfgNumb[0]]);
    ST_Message(20, 40, buffer, text_alpha | 0xffffff00);

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
    ST_Message(20, 50, buffer, text_alpha | 0xffffff00);

    for(int i = 2; i < ARRAYLEN(ControlText); i++)
    {
        int code = CurrentControls[0].BUTTONS[ControlMappings[i - 2]];
        int x = (i < (2 + CONTROLCOLSIZE)) ? 10 : 170;
        int y = ((i - 2) % CONTROLCOLSIZE) * 16 + 68;
        int len = D_strlen(ControlText[i]);

        ST_Message(x + 98 - len * 8, y + 3, ControlText[i], text_alpha | 0xffffff00);
        if (code)
        {
            int symbol = button_code_to_symbol_index(code);
            ST_DrawSymbol(x + 120, y, symbol, text_alpha | 0xffffff00);
        }
    }

    if (cursorpos < 2)
        ST_DrawSymbol(10, cursorpos * 10 + 39, 78, text_alpha | 0xffffff00);
    else if (cursorpos < 2 + CONTROLCOLSIZE)
        ST_DrawSymbol(116, ((cursorpos - 2) * 16) + 69, 78, text_alpha | 0xffffff00);
    else
        ST_DrawSymbol(276, (cursorpos - 2) % CONTROLCOLSIZE * 16 + 69, 78, text_alpha | 0xffffff00);

    ST_DrawString(-1, 220, "press \x8d to exit", text_alpha | 0xffffff00);
}
