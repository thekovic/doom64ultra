
#pragma once

#include "doomdef.h"

#define FLASHDELAY  8       /* # of tics delay (1/30 sec) */
#define FLASHTIMES  6       /* # of times to flash new frag amount (EVEN!) */

typedef struct
{
    int active;
    int doDraw;
    int delay;
    int times;
} sbflash_t;

extern boolean tryopen[6]; // 800A81E0

extern spriteN64_t *sfontlump;     // 800A81F8
extern spriteN64_t *statuslump;   // 800A81FC
extern int symbolslump;    // 800A8204

#define FIRST_SYMBOL   0x80
#define LAST_SYMBOL    0x90 // 0x91 for Right arrow

void ST_Init(void) SEC_STARTUP; // 80029BA0
void ST_InitEveryLevel(void); // 80029C00
void ST_Ticker (void) HOT; // 80029C88
void ST_Drawer (void) HOT; // 80029DC0
void ST_Message(int x,int y,const char *text,int color) HOT; // 8002A36C
void ST_DrawNumber(int x, int y, int val, int mode, int color) HOT; // 8002A79C
void ST_DrawString(int x, int y, const char *text, int color) HOT; // 8002A930
int ST_GetCenterTextX(byte *text); // 8002AAF4
void ST_UpdateFlash(void); // 8002AC30
void ST_DrawSymbol(int xpos, int ypos, int index, int color) HOT; // 8002ADEC
void ST_DrawMessages(player_t *player) HOT;

#if REGION == REGION_JP
void ST_DrawStringJp(int x, int y, const char *text, int color); // 8002Af94 (JP only)
#endif

// Debug
void ST_DrawDebug(void);
void ST_EnableDebug(void);
void ST_DisableDebug(void);
void ST_DebugSetPrintPos(int x, int y);
void ST_DebugPrint(const char *text, ...) __attribute__ ((format (printf, 1, 2)));
void ST_DebugClear(void);
