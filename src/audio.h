#ifndef _AUDIO_H
#define _AUDIO_H

#include <ultra64.h>
#include <os.h>
#include <n_libaudio_sc.h>
#include <ucode.h>

#include "wessapi.h"
#include "wessarc.h"
#include "wessshell.h"

#include "doomdef.h"

/*--------------------------------------------------*/
/*  type define's for structures unique to audiomgr */
/*--------------------------------------------------*/

typedef struct AudioInfo_s
{
    short       *data;          /* Output data pointer */
    short       frameSamples;   /* # of samples synthesized in this frame */
    OSTask      task;           /* scheduler structure */
} AudioInfo;

#define    NUM_ACMD_LISTS 2
#define    NUM_OUTPUT_BUFFERS 3

typedef struct
{
    Acmd          *ACMDList[NUM_ACMD_LISTS];
    AudioInfo     *audioInfo[NUM_OUTPUT_BUFFERS];
    N_ALGlobals   g;
} AMAudioMgr;

typedef struct
{
    ALLink        node;         //0
    u32           startAddr;    //8
    u32           lastFrame;    //12
    char          *ptr;         //16
} AMDMABuffer;

typedef struct
{
    u8            initialized;
    AMDMABuffer   *firstUsed;
    AMDMABuffer   *firstFree;
} AMDMAState;

extern AMAudioMgr   __am;           //800B4060
extern N_ALVoice    *voice;         //800B40E0
extern char         *reverb_status; //800B40E4

extern s32 milli_to_param(register s32 paramvalue, register s32 rate);      // 8002E3D0
extern int wess_memfill(void *dst, unsigned char fill, int count) SEC_STARTUP;          // 8002E300
extern int wess_rom_copy(char *src, char *dest, int len) SEC_STARTUP;                   // 8002E334
extern void wess_init(WessConfig *wessconfig) SEC_STARTUP;                         // 8002E41C
extern void amCreateAudioMgr(ALSynConfig *config, WessConfig *wessconfig) SEC_STARTUP;  // 8002E610
extern OSTask * wess_work(void) HOT;                                        // 8002EB2C
extern OSTask *__amHandleFrameMsg(AudioInfo *info) HOT;                     // 8002EBD8
extern s32 __amDMA(s32 addr, s32 len, void *state) HOT;                     // 8002ED74
extern ALDMAproc __amDmaNew(AMDMAState **state) HOT;                        // 8002EF48
extern void __clearAudioDMA(void) HOT;                                      // 8002EF7C
extern void wess_exit(void) SEC_STARTUP;                                           // 8002F0CC

#endif // _AUDIO_H
