#include <ultra64.h>
#include <PR/ramrom.h>	/* needed for argument passing into the app */
#include <os_internal.h>
#include <stdarg.h>

#include "i_main.h"
#include "doomdef.h"
#include "os_system.h"
#include "os_thread.h"
#include "config.h"
#include "i_debug.h"
#include "i_usb.h"

/*
 * Symbol genererated by "makerom" to indicate the end of the code segment
 * in virtual (and physical) memory
 */
extern char _codeSegmentEnd[];

/*
 * Symbols generated by "makerom" to tell us where the static segment is
 * in ROM.
 */

/*
 * Stacks for the threads as well as message queues for synchronization.
 */

/* this stack size is in bytes */
#define	BOOT_STACKSIZE	0x100
u64	bootStack[BOOT_STACKSIZE/sizeof(u64)];

u8 *cfb;
SDATA u16 SCREEN_HT = 240;

extern int globallump; // 800A68f8 r_local.h
extern int globalcm;   // 800A68fC r_local.h

//"\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91"
//static char	sysmbols[] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91};

//----------

#define SYS_THREAD_ID_IDLE 1
#define SYS_THREAD_ID_MAIN 2
#define SYS_THREAD_ID_TICKER 3
#define SYS_THREAD_ID_JOY 4

OSThread	idle_thread;                        // 800A4A18

#define SYS_MAIN_STACKSIZE 0xA000
OSThread	main_thread;                        // 800A4BC8
u64	main_stack[SYS_MAIN_STACKSIZE/sizeof(u64)]; // 80099A00

#define	JOY_STACKSIZE	0x100
OSThread	joy_thread;
u64	joy_stack[JOY_STACKSIZE/sizeof(u64)];

#define SYS_TICKER_STACKSIZE 0x800
OSThread	sys_ticker_thread;                          // 800A4D78
u64	sys_ticker_stack[SYS_TICKER_STACKSIZE/sizeof(u64)]; // 800A3A00

#define SYS_MSGBUF_SIZE_PI 128
OSMesgQueue msgque_Pi;                  // 800A4FA0
OSMesg msgbuf_Pi[SYS_MSGBUF_SIZE_PI];   // 800A4FD0

#define	SYS_FIFO_SIZE	512

#if __GNUC__ /* for GNU compiler */
u64 fifo_buff[2][SYS_FIFO_SIZE] ALIGNED(16);          /* buffer for RDP DL */      // 800633E0
u64 sys_rcp_stack[SP_DRAM_STACK_SIZE64] ALIGNED(16);  /* used for matrix stack */  // 800915E0
#else /* for SGI compiler */
u64 fifo_buff[2][SYS_FIFO_SIZE];            /* buffer for RDP DL */      // 800633E0
u64 sys_rcp_stack[SP_DRAM_STACK_SIZE64];    /* used for matrix stack */  // 800915E0
#endif

#define	SYS_YIELD_SIZE  OS_YIELD_DATA_SIZE
u64 gfx_yield_buff[SYS_YIELD_SIZE];     // 800919E0

OSTask vid_rsptask[2] = // 8005A590
{
    { .t = {
        M_GFXTASK,                          /* task type */
        NULL,                               /* task flags */
        (u64*) rspbootTextStart,            /* boot ucode pointer (fill in later) */
        0,                                  /* boot ucode size (fill in later) */
        (u64*) gspF3DEX2_NoN_fifoTextStart,  /* task ucode pointer (fill in later) */
        SP_UCODE_SIZE,                      /* task ucode size */
        (u64*) gspF3DEX2_NoN_fifoDataStart,  /* task ucode data pointer (fill in later) */
        SP_UCODE_DATA_SIZE,                 /* task ucode data size */
        &sys_rcp_stack[0],                  /* task dram stack pointer */
        SP_DRAM_STACK_SIZE8,                /* task dram stack size */
        &fifo_buff[0][0],                   /* task fifo buffer start ptr */
        &fifo_buff[0][0]+SYS_FIFO_SIZE,     /* task fifo buffer end ptr */
        NULL,                               /* task data pointer (fill in later) */
        0,                                  /* task data size (fill in later) */
        &gfx_yield_buff[0],                 /* task yield buffer ptr (not used here) */
        SYS_YIELD_SIZE                      /* task yield buffer size (not used here) */
    } },
    { .t = {
        M_GFXTASK,                          /* task type */
        NULL,                               /* task flags */
        (u64*) rspbootTextStart,            /* boot ucode pointer (fill in later) */
        0,                                  /* boot ucode size (fill in later) */
        (u64*) gspF3DEX2_NoN_fifoTextStart,  /* task ucode pointer (fill in later) */
        SP_UCODE_SIZE,                      /* task ucode size */
        (u64*) gspF3DEX2_NoN_fifoDataStart,  /* task ucode data pointer (fill in later) */
        SP_UCODE_DATA_SIZE,                 /* task ucode data size */
        &sys_rcp_stack[0],                  /* task dram stack pointer */
        SP_DRAM_STACK_SIZE8,                /* task dram stack size */
        &fifo_buff[1][0],                   /* task fifo buffer start ptr */
        &fifo_buff[1][0]+SYS_FIFO_SIZE,     /* task fifo buffer end ptr */
        NULL,                               /* task data pointer (fill in later) */
        0,                                  /* task data size (fill in later) */
        &gfx_yield_buff[0],                 /* task yield buffer ptr (not used here) */
        SYS_YIELD_SIZE                      /* task yield buffer size (not used here) */
    } }
};

Vp vid_viewport = { // 8005A610
    .vp = {
        {0, 0, G_MAXZ,   0},		/* scale */
        {0, 0,      0,   0},		/* translate */
    } };

OSMesgQueue romcopy_msgque; // 800A4F70
OSMesg		romcopy_msgbuf; // 800A51D0

OSMesgQueue sys_msgque_joy; // 800A4F88
OSMesg		sys_msg_joy;    // 800A51D4

OSMesgQueue joy_cmd_msgque;
OSMesg		joy_cmd_msg;

#define SYS_MSGBUF_SIZE_VID 16
OSMesgQueue sys_ticker_queue; // 800A4FB8
OSMesg		sys_ticker_msgbuf[SYS_MSGBUF_SIZE_VID]; // 800A51E0

#define SYS_MSGBUF_SIZE_VID2 2
OSMesgQueue rdp_done_queue; // 800A4F28
OSMesg		rdp_done_msgbuf[SYS_MSGBUF_SIZE_VID2]; // 800A51D8

OSMesgQueue vid_task_queue; // 800A4F40
OSMesg		vid_task_msgbuf[SYS_MSGBUF_SIZE_VID2]; // 800A5220

OSMesgQueue audio_task_queue; // 800A4F58
OSMesg		audio_task_msgbuf[SYS_MSGBUF_SIZE_VID2]; // 800A5228

OSContStatus gamepad_status[MAXCONTROLLERS]; // 800a5230
OSContPad   *gamepad_data;    // 800A5240

OSTask *vid_task;   // 800A5244
u32 vid_side;       // 800A5248

u32 video_hStart;   // 800A524c
u32 video_vStart1;  // 800A5250
u32 video_vStart2;  // 800A5254

u32 GfxIndex;       // 800A5258
u32 VtxIndex;       // 800A525C

u8 gamepad_bit_pattern; // 800A5260 // one bit for each controller
u8 rumblepak_bit_pattern = 0;
u8 motor_bit_pattern = 0;

OSPfs RumblePaks[MAXCONTROLLERS];
u16 MotorAmbientCount[MAXCONTROLLERS];
u16 MotorDamageTimers[MAXCONTROLLERS];

// Controller Pak
OSPfs ControllerPak;        // 800A5270
OSPfsState FileState[16];   // 800A52D8
s32 File_Num;   // 800A54D8
s32 Pak_Size;   // 800A54DC
u8 *Pak_Data;   // 800A54E0
s32 Pak_Memory; // 800A54E4

static const char Game_Name[16] = // 8005A790
{
    0x1D, 0x28, 0x28, 0x26, 0x0F, 0x16, 0x14, 0x0F, // (doom 64 ultra) byte index from Pak_Table
    0x2E, 0x25, 0x2D, 0x2B, 0x1A, 0x00, 0x00, 0x00
};

boolean disabledrawing = false; // 8005A720

SDATA u16 XResolution;
SDATA u16 YResolution;
SDATA u8 hudxshift;
SDATA u8 hudyshift;

static u8 blanktimer;

u32 motor = 0;
s32 vsync = 0;              // 8005A724
s32 drawsync2 = 0;          // 8005A728
s32 drawsync1 = 0;          // 8005A72C
u32 NextFrameIdx = 0;       // 8005A730

static bool PiLockedMain = false; // 8005A738
static bool PiLockedJoy = false; // 8005A73C
s32 FilesUsed = -1;                 // 8005A740
static SDATA u32 SystemTickerStatus = 0;  // 8005a744

Gfx Gfx_base[2][MAX_GFX];    // 800653E0
Mtx Mtx_base[2][MAX_MTX];    // 800793E0
Vtx Vtx_base[2][MAX_VTX];    // 800795E0

SDATA Gfx *GFX1;	// 800A4A00
SDATA Gfx *GFX2;	// 800A4A04

SDATA Vtx *VTX1;	// 800A4A08
SDATA Vtx *VTX2;	// 800A4A0C

SDATA Mtx *MTX1;	// 800A4A10
SDATA Mtx *MTX2;	// 800A4A14

static Gfx *GfxBlocks[8] = {0,0,0,0,0,0,0,0}; // 8005A748
static Vtx *VtxBlocks[8] = {0,0,0,0,0,0,0,0}; // 8005A768

u32 LastFrameCycles = 0;
static u32 LastCpuStart = 0;
u32 LastCpuCycles = 0;
static u32 LastGfxRspStart = 0;
u32 LastGfxRspCycles = 0;
static u32 LastAudioRspStart = 0;
u32 LastAudioRspCycles = 0;
static u32 LastRdpStart = 0;
u32 LastRdpCycles = 0;
DEBUG_COUNTER(u32 LastWorldCycles = 0);
DEBUG_COUNTER(u32 LastAudioCycles = 0);
DEBUG_COUNTER(u32 LastBspCycles = 0);
DEBUG_COUNTER(u32 LastPhase3Cycles = 0);
DEBUG_COUNTER(SDATA u32 LastVisTriangles = 0);
DEBUG_COUNTER(SDATA u32 LastVisSubsectors = 0);
DEBUG_COUNTER(SDATA u32 LastVisLeaves = 0);
DEBUG_COUNTER(SDATA u32 LastVisSegs = 0);
DEBUG_COUNTER(SDATA u32 LastVisThings = 0);

void S_Init(void);
void I_InitSram(void);

void I_Start(void) SEC_STARTUP;  // 80005620
NO_RETURN void I_IdleGameThread(void *arg) SEC_STARTUP; // 8000567C
NO_RETURN void I_SystemTicker(void *arg) HOT; // 80005730
NO_RETURN void I_ControllerThread(void *) HOT;

OSTask * wess_work(void);

void I_Start(void)  // 80005620
{
    /* Re-initialize U64 operating system... */
    osInitialize();

    /* Create and start idle thread... */
    osCreateThread(&idle_thread, SYS_THREAD_ID_IDLE, I_IdleGameThread, (void *)0,
                   bootStack + BOOT_STACKSIZE/sizeof(u64), 100);
    osStartThread(&idle_thread);
}

static HOT NO_RETURN INLINE_NEVER void I_IdleLoop(void)
{
    do {
        osYieldThread();
    } while(TRUE);
}

void I_IdleGameThread(void *arg) // 8000567C
{
    SET_GP();

    /* Create and start the PI and VI managers... */
    osCreatePiManager((OSPri)OS_PRIORITY_PIMGR, &msgque_Pi, msgbuf_Pi, SYS_MSGBUF_SIZE_PI);
    osCreateViManager(OS_PRIORITY_VIMGR);

    if (osTvType == OS_TV_PAL)
        SCREEN_HT = 288;

    /* Init debugger/USB support after PI */
    I_InitFlashCart();
    I_InitDebugging();

    /* Create main thread... */
    osCreateThread(&main_thread, SYS_THREAD_ID_MAIN, D_DoomMain, (void *)0,
                   main_stack + SYS_MAIN_STACKSIZE/sizeof(u64), 10);
    osStartThread(&main_thread);

    osSetThreadPri(&idle_thread, OS_PRIORITY_IDLE);

    I_IdleLoop();
}

#define STF_GFX_PENDING 1
#define STF_GFX_YIELDED 2
#define STF_GFX_RESUME 4
#define STF_RDP_PENDING 8
#define STF_RDP_DONE 16
#define STF_AUDIO_PENDING 32

static int vbi_msg;
static OSTask *rspTask;
static int read_vid_side;

static HOT void I_LoadAudioTask(void)
{
    register int ret = osRecvMesg(&audio_task_queue, (OSMesg *)&vbi_msg, OS_MESG_NOBLOCK);
    rspTask = (OSTask*)vbi_msg;

    if(ret!= -1)
    {
        LastAudioRspStart = osGetCount();

        SystemTickerStatus |= STF_AUDIO_PENDING;

        osWritebackDCacheAll();
        osSpTaskLoad(rspTask);
        osSpTaskStartGo(rspTask);
    }
}

static HOT void I_LoadGfxTask(void)
{
    register int ret = osRecvMesg(&vid_task_queue, (OSMesg *)&vbi_msg, OS_MESG_NOBLOCK);
    rspTask = (OSTask*)vbi_msg;

    //D_printf("osRecvMesg gfx task ret %d\n", ret);

    if(ret != -1)
    {
        //D_printf("rspTask %lu  0x%08lx\\n",rspTask->t.type,(u32)rspTask->t.ucode);

        if(rspTask == vid_rsptask)
            read_vid_side = 0;
        else
            read_vid_side = 1;

        LastGfxRspStart = LastRdpStart = osGetCount();

        SystemTickerStatus |= STF_GFX_PENDING;

        osWritebackDCacheAll();
        osSpTaskLoad(rspTask);
        osSpTaskStartGo(rspTask);
    }
}

void I_SystemTicker(void *arg) // 80005730
{
    SET_GP();

    int side;
    int current_fbuf, next_fbuf;
    OSTask *wess;
    OSTask *rspTaskPrev;

    //char str[64];

    read_vid_side = 0;
    rspTask = NULL;
    rspTaskPrev = NULL;
    side = 1;

    while(true)
    {
        osRecvMesg(&sys_ticker_queue, (OSMesg *)&vbi_msg, OS_MESG_BLOCK);

        //D_printf("  SystemTickerStatus %d vbi_msg %d read_vid_side %d\n", (int)SystemTickerStatus, vbi_msg,read_vid_side);

        switch (vbi_msg)
        {
            case VID_MSG_RSP:				// end of signal processing
                {
                    //D_printf("VID_MSG_RSP || type(%lu)\n", rspTask->t.type);

                    if(rspTask->t.type == M_AUDTASK)
                    {
                        if (LastAudioRspStart)
                            LastAudioRspCycles = osGetCount() - LastAudioRspStart;

                        SystemTickerStatus &= ~STF_AUDIO_PENDING;

                        if (SystemTickerStatus & STF_GFX_RESUME)
                        {
                            SystemTickerStatus &= ~STF_GFX_RESUME;
                            SystemTickerStatus |= STF_GFX_PENDING;

                            rspTask = rspTaskPrev;
                            osWritebackDCacheAll();
                            osSpTaskLoad(rspTask);
                            osSpTaskStartGo(rspTask);
                        }
                        else
                        {
                            if ((SystemTickerStatus & (STF_RDP_PENDING|STF_RDP_DONE)) == 0)
                                I_LoadGfxTask();
                        }
                    }
                    else
                    {
                        SystemTickerStatus &= ~STF_GFX_PENDING;

                        if(SystemTickerStatus & STF_GFX_YIELDED)
                        {
                            SystemTickerStatus &= ~STF_GFX_YIELDED;

                            if (osSpTaskYielded(rspTask))
                            {
                                rspTaskPrev = rspTask;

                                SystemTickerStatus |= STF_GFX_RESUME;

                                I_LoadAudioTask();
                            }
                        }
                        else
                        {
                            if (LastGfxRspStart)
                                LastGfxRspCycles = osGetCount() - LastGfxRspStart;

                            if ((SystemTickerStatus & STF_RDP_DONE) == 0)
                                SystemTickerStatus |= STF_RDP_PENDING;

                            I_LoadAudioTask();
                        }
                    }
                }
                break;

            case VID_MSG_RDP:                 // end of display processing
                {
                    //D_printf("VID_MSG_RDP\n");

                    SystemTickerStatus &= ~STF_RDP_PENDING;
                    SystemTickerStatus |= STF_RDP_DONE;

                    if (LastRdpStart)
                    {
                        LastRdpCycles = osGetCount() - LastRdpStart;
                        if (LastCpuCycles)
                            LastFrameCycles = LastRdpCycles + LastCpuCycles;
                    }

                    osViSwapBuffer(CFB(read_vid_side));
                }
                break;

            case VID_MSG_PRENMI:
                {
                    //D_printf("VID_MSG_PRENMI\n");
                    disabledrawing = true;
                    S_StopAll();
                }
                break;

            case VID_MSG_VBI:
                {
                    //D_printf("VID_MSG_VBI || vsync(%ld) || side(%d)\n", vsync, side);

                    vsync += 1;

                    if (audio_task_queue.validCount)
                    {
                        if (SystemTickerStatus & STF_GFX_PENDING)
                        {
                            SystemTickerStatus |= STF_GFX_YIELDED;
                            osSpTaskYield();
                        }
                        else
                        {
                            if ((SystemTickerStatus & STF_AUDIO_PENDING) == 0)
                                I_LoadAudioTask();
                        }
                    }

                    if (side & 1)
                    {
                        DEBUG_CYCLES_START(audio_start);
                        // next audio function task
                        wess = wess_work();
                        DEBUG_CYCLES_END(audio_start, LastAudioCycles);
                        if (wess)
                            osSendMesg(&audio_task_queue,(OSMesg) wess, OS_MESG_NOBLOCK);
                    }
                    side++;

                    if (SystemTickerStatus & STF_RDP_DONE)
                    {
                        if ((u32)(vsync - drawsync2) < 2) continue;

                        current_fbuf = (int)osViGetCurrentFramebuffer();
                        next_fbuf = (int)osViGetNextFramebuffer();

                        if (next_fbuf != current_fbuf)
                            continue;

                        SystemTickerStatus &= ~STF_RDP_DONE;

                        if (demoplayback || demorecording)
                        {
                            vsync = drawsync2 + 2;
                        }

                        drawsync1 = vsync - drawsync2;
                        drawsync2 = vsync;

                        osSendMesg(&rdp_done_queue, (OSMesg)VID_MSG_KICKSTART, OS_MESG_NOBLOCK);
                    }

                    if(SystemTickerStatus == 0)
                        I_LoadGfxTask();

                    osSendMesg(&joy_cmd_msgque, NULL, OS_MESG_NOBLOCK);
                }
                break;
        }
    }
}

void I_Init(void) // 80005C50
{
    vid_rsptask[0].t.ucode_boot_size = (int)rspbootTextEnd - (int)rspbootTextStart;	// set ucode size (waste but who cares)
    vid_rsptask[1].t.ucode_boot_size = (int)rspbootTextEnd - (int)rspbootTextStart;	// set ucode size (waste but who cares)

    osCreateMesgQueue( &romcopy_msgque, &romcopy_msgbuf, 1 );

    osCreateMesgQueue( &sys_ticker_queue, sys_ticker_msgbuf, SYS_MSGBUF_SIZE_VID );

    osCreateMesgQueue(&rdp_done_queue, rdp_done_msgbuf, SYS_MSGBUF_SIZE_VID2);//&sys_msgque_jam, sys_msgbuf_jam
    osCreateMesgQueue(&vid_task_queue, vid_task_msgbuf, SYS_MSGBUF_SIZE_VID2);//&sys_msgque_ser, sys_msgbuf_ser
    osCreateMesgQueue(&audio_task_queue, audio_task_msgbuf, SYS_MSGBUF_SIZE_VID2);//&sys_msgque_tmr, sys_msgbuf_tmr

    osCreateMesgQueue(&joy_cmd_msgque, &joy_cmd_msg, 1);

    // Init the video mode...
    I_RefreshVideo();
    osViBlack(TRUE);

    if (osMemSize >= 0x800000)
        BitDepth = BITDEPTH_32;
    else
        BitDepth = BITDEPTH_16;

    cfb = CFBS_ADDR;
    D_memset(cfb, 0, CFBS_SIZE);

    osViSwapBuffer(cfb);

    if (osViGetCurrentFramebuffer() != cfb) {
        do {
        } while (osViGetCurrentFramebuffer() != cfb);
    }

    osViBlack(FALSE);

    osSetEventMesg( OS_EVENT_SP, &sys_ticker_queue, (OSMesg)VID_MSG_RSP );
    osSetEventMesg( OS_EVENT_DP, &sys_ticker_queue, (OSMesg)VID_MSG_RDP );
    osSetEventMesg( OS_EVENT_PRENMI, &sys_ticker_queue, (OSMesg)VID_MSG_PRENMI );

    osViSetEvent( &sys_ticker_queue, (OSMesg)VID_MSG_VBI, 1 ); // last parm: 2 indicates 30 FPS (1=60)

    vid_side = 1;

    /* Serial/Joy queue */

    osCreateMesgQueue(&sys_msgque_joy, &sys_msg_joy, 1);
    osSetEventMesg(OS_EVENT_SI, &sys_msgque_joy, &sys_msg_joy);

    osContInit(&sys_msgque_joy, &gamepad_bit_pattern, gamepad_status);

    I_InitSram();
    if (osMemSize < 0x800000)
    {
        VideoResolution = VIDEO_RES_LOW;
        BitDepth = BITDEPTH_16;
    }
    I_RefreshVideo(); // set vid mode again after loading settings

    gamepad_data = (OSContPad *)bootStack;

    if ((gamepad_bit_pattern & 1) != 0)
    {
        osContStartReadData(&sys_msgque_joy);
        osRecvMesg(&sys_msgque_joy, NULL, OS_MESG_BLOCK);
        osContGetReadData(gamepad_data);
    }

    osCreateThread(&joy_thread, SYS_THREAD_ID_JOY, I_ControllerThread, (void *)0,
                   joy_stack + JOY_STACKSIZE/sizeof(u64), 11);
    osStartThread(&joy_thread);

    D_printf ("S_Init: Setting up sound.\n");
    S_Init();

    /* Create and start ticker thread... */
    osCreateThread(&sys_ticker_thread, SYS_THREAD_ID_TICKER, I_SystemTicker, (void *)0,
                   sys_ticker_stack + SYS_TICKER_STACKSIZE/sizeof(u64), 11);
    osStartThread(&sys_ticker_thread);

    osJamMesg(&rdp_done_queue, (OSMesg)VID_MSG_KICKSTART, OS_MESG_NOBLOCK);
}

typedef struct
{
	int pad_data;
} pad_t;

int I_GetControllerData(void) // 800060D0
{
    return ((pad_t*)gamepad_data)->pad_data;
}

void I_CheckGFX(void) // 800060E8
{
	memblock_t *block;

	Gfx **Gfx_Blocks;
	Vtx **Vtx_Blocks;

	int i, index;
	int block_idx;

	index = (int)((int)GFX1 - (int)GFX2) / sizeof(Gfx);

	if (index > MAX_GFX)
		I_Error("I_CheckGFX: GFX Overflow by %d\n",index);

	if ((index < (MAX_GFX-1024)) == 0)
	{
		Gfx_Blocks = GfxBlocks;
		block_idx = -1;

		for(i = 0; i < 8; i++)
		{
			block = (memblock_t *)((byte *)*Gfx_Blocks - sizeof(memblock_t));

			if (*Gfx_Blocks)
			{
				if(((u32)block->lockframe < NextFrameIdx - 1) == 0)
                {
                    Gfx_Blocks++;
                    continue;
                }

                block->lockframe = NextFrameIdx;
                GFX2 = (Gfx *)*Gfx_Blocks;
                goto move_gfx;
			}

            block_idx = i;
		}

		if (block_idx < 0)
			I_Error("I_CheckGFX: GFX Cache overflow");

		GFX2 = (Gfx *)Z_Malloc(MAX_GFX * sizeof(Gfx), PU_CACHE, &GfxBlocks[block_idx]);

	move_gfx:
		gSPBranchList(GFX1,GFX2);
		GFX1 = GFX2;
		GfxIndex += index;
	}

	index = (int)((int)VTX1 - (int)VTX2) / sizeof(Vtx);

	if (index > MAX_VTX)
		I_Error("I_CheckVTX: VTX Overflow by %d\n",index);

	if ((index < (MAX_VTX-615)) == 0)
	{
		Vtx_Blocks = VtxBlocks;
		block_idx = -1;

		for(i = 0; i < 8; i++)
		{
		    block = (memblock_t *)((byte *)*Vtx_Blocks - sizeof(memblock_t));

			if (*Vtx_Blocks)
			{
				if(((u32)block->lockframe < NextFrameIdx - 1) == 0)
                {
                    Vtx_Blocks++;
                    continue;
                }

                block->lockframe = NextFrameIdx;
                VTX2 = (Vtx *)*Vtx_Blocks;
                goto move_vtx;
			}

            block_idx = i;
		}

		if (block_idx < 0)
			I_Error("I_CheckGFX: VTX Cache overflow");

        VTX2 = (Vtx *)Z_Malloc(MAX_VTX * sizeof(Vtx), PU_CACHE, &VtxBlocks[block_idx]);

	move_vtx:
		VTX1 = VTX2;
		VtxIndex += index;
	}
}

void I_ClearFrame(void) // 8000637C
{
    NextFrameIdx += 1;

    GFX1 = Gfx_base[vid_side];
    GFX2 = GFX1;
    GfxIndex = 0;

    VTX1 = Vtx_base[vid_side];
    VTX2 = VTX1;
    VtxIndex = 0;

    MTX1 = Mtx_base[vid_side];

    DEBUG_COUNTER(LastBspCycles = 0);
    DEBUG_COUNTER(LastPhase3Cycles = 0);
    DEBUG_COUNTER(LastVisTriangles = 0);
    DEBUG_COUNTER(LastVisSubsectors = 0);
    DEBUG_COUNTER(LastVisLeaves = 0);
    DEBUG_COUNTER(LastVisSegs = 0);
    DEBUG_COUNTER(LastVisThings = 0);

    vid_task = &vid_rsptask[vid_side];

    vid_task->t.ucode = (u64 *) gspF3DEX2_NoN_fifoTextStart;
    vid_task->t.ucode_data = (u64 *) gspF3DEX2_NoN_fifoDataStart;

    gDPSetColorImage(GFX1++, G_IM_FMT_RGBA, BitDepth + G_IM_SIZ_16b, XResolution, CFB_SPADDR);
    gDPSetScissor(GFX1++, G_SC_NON_INTERLACE, 0, 0, XResolution, YResolution);

    gDPSetTextureFilter(GFX1++, G_TF_POINT);
    gDPSetColorDither(GFX1++, G_CD_DISABLE);

    vid_viewport.vp.vscale[0] = vid_viewport.vp.vtrans[0] = XResolution * 2;
    vid_viewport.vp.vscale[1] = vid_viewport.vp.vtrans[1] = YResolution * 2;
    gSPViewport(GFX1++, &vid_viewport);

    gSPClearGeometryMode(GFX1++, -1);
    gSPSetGeometryMode(GFX1++, G_SHADE|G_SHADING_SMOOTH|G_FOG );

    globallump = -1;
    globalcm = 0;
}

void I_DrawFrame(void)  // 80006570
{
    int index;

    gDPFullSync(GFX1++);
    gSPEndDisplayList(GFX1++);

    index = (int)((int)GFX1 - (int)GFX2) / sizeof(Gfx);
	if (index > MAX_GFX)
		I_Error("I_DrawFrame: GFX Overflow by %d\n\n",index);

    index = (int)((int)VTX1 - (int)VTX2) / sizeof(Vtx);
	if (index > MAX_VTX)
		I_Error("I_DrawFrame: VTX Overflow by %d\n",index);

    vid_task->t.data_ptr = (u64 *) Gfx_base[vid_side];
    vid_task->t.data_size = (u32)((((int)((int)GFX1 - (int)GFX2) / sizeof(Gfx)) + GfxIndex) * sizeof(Gfx));

    osSendMesg(&vid_task_queue,(OSMesg) vid_task, OS_MESG_NOBLOCK);

    if (LastCpuStart)
        LastCpuCycles = osGetCount() - LastCpuStart;

    osRecvMesg(&rdp_done_queue, NULL, OS_MESG_BLOCK);//retraceMessageQ
    vid_side ^= 1;

    if (blanktimer)
    {
        blanktimer--;
        if (!blanktimer)
            osViBlack(FALSE);
    }

    LastCpuStart = osGetCount();
}

void I_GetScreenGrab(void) // 800066C0
{
    if ((SystemTickerStatus & ~STF_AUDIO_PENDING) || (vid_task_queue.validCount != 0)) {
        osRecvMesg(&rdp_done_queue, (OSMesg *)0, OS_MESG_BLOCK);
        osJamMesg(&rdp_done_queue, (OSMesg)VID_MSG_KICKSTART, OS_MESG_NOBLOCK);
    }
}

void I_RefreshVideo(void) // [Immorpher] video refresh
{
    int modeidx = OS_VI_NTSC_LPN1;
    int special;
    OSViMode *ViMode;

    if (osTvType == OS_TV_PAL)
        modeidx += 14*3;
    else if (osTvType == OS_TV_MPAL)
        modeidx += 14*2;

    if(TvMode & 2) // interlacing
        modeidx += 1;

    if (BitDepth == BITDEPTH_32)
        modeidx += 4;

    if (VideoResolution == VIDEO_RES_HI_VERT)
        modeidx += 8;
    else if((TvMode & 1) && (VideoResolution == VIDEO_RES_LOW || BitDepth == BITDEPTH_16))
        modeidx += 2; // antialiasing

    ViMode = &osViModeTable[modeidx];

    if (VideoResolution == VIDEO_RES_HI_HORIZ)
    {
        ViMode->comRegs.width = 640;
        ViMode->comRegs.xScale = 1024;
        ViMode->fldRegs[0].origin = ViMode->fldRegs[1].origin = 640*(BitDepth?4:2);
    }
    else
    {
        ViMode->comRegs.xScale = 512;
        ViMode->fldRegs[0].origin = ViMode->fldRegs[1].origin = 320*(BitDepth?4:2);
        if (VideoResolution == VIDEO_RES_HI_VERT)
        {
            ViMode->comRegs.width = 640;
            ViMode->fldRegs[1].origin <<= 1;
            if(TvMode & 2) // deflickering
                ViMode->comRegs.width = 320;
        }
        else
        {
            ViMode->comRegs.width = 320;
        }
    }

    osViSetMode(ViMode);

    if (blanktimer)
        osViBlack(TRUE);

    special = (TvMode & 1) ? OS_VI_DIVOT_ON : OS_VI_DIVOT_OFF;
    special |= DitherFilter ? OS_VI_DITHER_FILTER_ON : OS_VI_DITHER_FILTER_OFF;
    special |= NoGammaCorrect
        ? OS_VI_GAMMA_OFF | OS_VI_GAMMA_DITHER_OFF
        : OS_VI_GAMMA_ON | OS_VI_GAMMA_DITHER_ON;

    osViSetSpecialFeatures(special);

    switch (VideoResolution)
    {
    case VIDEO_RES_LOW:
        XResolution = 320;
        if (osTvType == OS_TV_PAL)
            YResolution = 288;
        else
            YResolution = 240;
        hudxshift = 2;
        hudyshift = 2;
        break;
    case VIDEO_RES_HI_HORIZ:
        XResolution = 640;
        if (osTvType == OS_TV_PAL)
            YResolution = 288;
        else
            YResolution = 240;
        hudxshift = 3;
        hudyshift = 2;
        break;
    case VIDEO_RES_HI_VERT:
        XResolution = 320;
        if (osTvType == OS_TV_PAL)
            YResolution = 576;
        else
            YResolution = 480;
        hudxshift = 2;
        hudyshift = 3;
        break;
    }

    video_hStart = ViMode->comRegs.hStart;
    video_vStart1 = ViMode->fldRegs[0].vStart;
    video_vStart2 = ViMode->fldRegs[1].vStart;
}

void I_BlankScreen(u8 vbls)
{
    blanktimer = vbls;
    osViBlack(TRUE);
}

void I_ClearFB(register u32 color)
{
    if (BitDepth == BITDEPTH_16)
    {
        color = RGBATO551(color);
        color |= (color << 16);
    }

    gDPPipeSync(GFX1++);
    gDPSetCycleType(GFX1++, G_CYC_FILL);
    gDPSetRenderMode(GFX1++,G_RM_NOOP,G_RM_NOOP2);
    gDPSetFillColor(GFX1++, color);
    gDPFillRectangle(GFX1++, 0, 0, XResolution-1, YResolution-1);
}

long LongSwap(long dat) // 80006724
{
    return (u32)dat >> 0x18 | (dat >> 8 & 0xff00U) | (dat & 0xff00U) << 8 | dat << 0x18;
}

short LittleShort(short dat) // 80006750
{
    return ((((dat << 8) | (dat >> 8 & 0xff)) << 16) >> 16);
}

short BigShort(short dat) // 80006770
{
    return ((dat << 8) | (dat >> 8 & 0xff)) & 0xffff;
}

void I_MoveDisplay(int x,int y) // 80006790
{
  int ViMode;

  ViMode = osViGetCurrentMode();

  osViModeTable[ViMode].comRegs.hStart =
       (int)(((int)video_hStart >> 0x10 & 65535) + x) % 65535 << 0x10 |
       (int)((video_hStart & 65535) + x) % 65535;

  osViModeTable[ViMode].fldRegs[0].vStart =
       (int)(((int)video_vStart1 >> 0x10 & 65535) + y) % 65535 << 0x10 |
       (int)((video_vStart1 & 65535) + y) % 65535;

  osViModeTable[ViMode].fldRegs[1].vStart =
       (int)(((int)video_vStart2 >> 0x10 & 65535) + y) % 65535 << 0x10 |
       (int)((video_vStart2 & 65535) + y) % 65535;
}

static bool skipfade = false;
s8 fadetick = 8;

void I_WIPE_MeltScreen(void) // 80006964
{
    u16 *fb;
    int y1;
    int yscroll;
    int height;
    int size;
    int tileheight;
    int fbsize;
    int buttons;
    int shift = 0;

    skipfade = false;

    {
        int pixelsize = BitDepth == BITDEPTH_32 ? sizeof(u32) : sizeof(u16);

        size = BitDepth + G_IM_SIZ_16b;
        tileheight = 4096/(XResolution*pixelsize);
        fbsize = XResolution*YResolution*pixelsize;
        fb = Z_Malloc(fbsize, PU_STATIC, NULL);
        I_GetScreenGrab();
        D_memcpy(CFB(vid_side), CFB(vid_side ^ 1), fbsize);
    }

    yscroll = 1;
    while( true )
    {
        y1 = 0;
        D_memcpy(fb, CFB(vid_side ^ 1), fbsize);

        buttons = I_GetControllerData();
        if (buttons & (PAD_A|PAD_B|PAD_START))
        {
            shift = 2;
            skipfade = true;
        }

        I_ClearFrame();

        gDPSetCycleType(GFX1++, G_CYC_1CYCLE);
        gDPSetTextureLUT(GFX1++, G_TT_NONE);
        gDPSetTexturePersp(GFX1++, G_TP_NONE);
        gDPSetAlphaCompare(GFX1++, G_AC_THRESHOLD);
        gDPSetBlendColor(GFX1++, 0, 0, 0, 0);
        gDPSetCombineMode(GFX1++, G_CC_D64COMB19, G_CC_D64COMB19);
        gDPSetRenderMode(GFX1++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
        gDPSetPrimColor(GFX1++, 0, 0, 15, 0, 0, 22); // 0x0f000016

        height = SCREEN_HT - (yscroll >> 2);
        if (height > 0)
        {
            do
            {
                gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, size , XResolution, fb);
                gDPSetTile(GFX1++, G_IM_FMT_RGBA, size,
                           (XResolution >> 2), 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

                gDPLoadSync(GFX1++);
                gDPLoadTile(GFX1++, G_TX_LOADTILE,
                            (0 << 2), (y1 << 2),
                            ((XResolution-1) << 2), ((y1+tileheight-1) << 2));

                gDPPipeSync(GFX1++);
                gDPSetTile(GFX1++, G_IM_FMT_RGBA, size,
                           (XResolution >> 2), 0, G_TX_RENDERTILE, 0, 0, 0, 0, 0, 0, 0);

                gDPSetTileSize(GFX1++, G_TX_RENDERTILE,
                               (0 << 2), (y1 << 2),
                               ((XResolution-1) << 2), ((y1+tileheight-1) << 2));

                gSPTextureRectangle(GFX1++,
                                    (0 << 2), (y1 << 2) + (yscroll<<(hudyshift-2)),
                                    (XResolution << 2), ((y1 + tileheight) << 2) + (yscroll<<(hudyshift-2)),
                                    G_TX_RENDERTILE,
                                    (0 << 5), (y1 << 5),
                                    (1 << 10), (1 << 10));

                y1 += tileheight;
            } while (y1 < height);
        }

        yscroll += (drawsync1 << shift);
        if (yscroll >= 160) break;
        I_DrawFrame();
    }

    Z_Free(fb);
    I_WIPE_FadeOutScreen();
}

void I_WIPE_FadeOutScreen(void) // 80006D34
{
    u32 *fb;
    int y1, outcnt;
    int size;
    int tileheight;
    int fbsize;
    int buttons;
    int shift = skipfade ? 2 : 0;

    {
        int pixelsize = BitDepth == BITDEPTH_32 ? sizeof(u32) : sizeof(u16);

        size = BitDepth + G_IM_SIZ_16b;
        tileheight = 4096/(XResolution*pixelsize);
        fbsize = XResolution*YResolution*pixelsize;
        fb = Z_Malloc(fbsize, PU_STATIC, NULL);
        I_GetScreenGrab();
        D_memcpy(fb, CFB(vid_side ^ 1), fbsize);
    }

    outcnt = 256;
    do
    {
        outcnt -= (((int) fadetick) * drawsync1) << shift >> 1;
        outcnt = MAX(outcnt, 0);

        I_ClearFrame();

        buttons = I_GetControllerData();
        if (buttons & (PAD_A|PAD_B|PAD_START))
            shift = 2;

        gDPSetCycleType(GFX1++, G_CYC_1CYCLE);
        gDPSetTextureLUT(GFX1++, G_TT_NONE);
        gDPSetTexturePersp(GFX1++, G_TP_NONE);
        gDPSetAlphaCompare(GFX1++, G_AC_NONE);
        gDPSetCombineMode(GFX1++, G_CC_D64COMB06, G_CC_D64COMB06);
        gDPSetRenderMode(GFX1++,G_RM_OPA_SURF,G_RM_OPA_SURF2);
        gDPSetPrimColor(GFX1++, 0, 0, outcnt, outcnt, outcnt, 0);

        y1 = 0;
        do
        {
            gDPSetTextureImage(GFX1++, G_IM_FMT_RGBA, size , XResolution, fb);
            gDPSetTile(GFX1++, G_IM_FMT_RGBA, size,
                       (XResolution >> 2), 0, G_TX_LOADTILE, 0, 0, 0, 0, 0, 0, 0);

            gDPLoadSync(GFX1++);
            gDPLoadTile(GFX1++, G_TX_LOADTILE,
                        (0 << 2), (y1 << 2),
                        ((XResolution-1) << 2), (((y1+tileheight)-1) << 2));

            gDPPipeSync(GFX1++);
            gDPSetTile(GFX1++, G_IM_FMT_RGBA, size,
                       (XResolution >> 2), 0, G_TX_RENDERTILE, 0, 0, 0, 0, 0, 0, 0);

            gDPSetTileSize(GFX1++, G_TX_RENDERTILE,
                           (0 << 2), (y1 << 2),
                           ((XResolution-1) << 2), (((y1+tileheight)-1) << 2));

            gSPTextureRectangle(GFX1++,
                                (0 << 2), (y1 << 2),
                                (XResolution << 2), ((y1+tileheight) << 2),
                                G_TX_RENDERTILE,
                                (0 << 5), (y1 << 5),
                                (1 << 10), (1 << 10));

            y1 += tileheight;
        } while (y1 < YResolution);

        I_DrawFrame();
    } while (outcnt > 0);

    I_GetScreenGrab();
    Z_Free(fb);
    skipfade = false;
}

static void I_LockPi(void)
{
    PiLockedMain = true;

    while (PiLockedJoy)
        osYieldThread();
}

static void I_UnlockPi(void)
{
    PiLockedMain = false;
}

int I_CheckControllerPak(void) // 800070B0
{
    int ret, file;
    OSPfsState *fState;
    s32 MaxFiles [2];
    u8 validpaks = 0;

    I_LockPi();

    FilesUsed = -1;
    ret = PFS_ERR_NOPACK;

    osPfsIsPlug(&sys_msgque_joy, &validpaks);

    /* does the current controller have a memory pak? */
    if (validpaks & 1)
    {
        ret = osPfsInit(&sys_msgque_joy, &ControllerPak, NULL);

        if (ret == 0 || ret == PFS_ERR_NEW_PACK)
        {
            ret = osPfsNumFiles(&ControllerPak, MaxFiles, &FilesUsed);

            if (ret == PFS_ERR_INCONSISTENT)
                ret = osPfsChecker(&ControllerPak);

            if (ret == 0)
            {
                Pak_Memory = 123;
                fState = FileState;
                file = 0;
                do
                {
                    ret = osPfsFileState(&ControllerPak, file, fState);
                    file += 1;

                    if (ret != 0)
                      fState->file_size = 0;

                    Pak_Memory -= (fState->file_size >> 8);
                    fState += 1;
                } while (file != 16);
                ret = 0;
            }
        }
    }

    I_UnlockPi();

    return ret;
}

int I_DeletePakFile(int filenumb) // 80007224
{
    int ret;
    OSPfsState *fState;

    I_LockPi();

    fState = &FileState[filenumb];

    if (fState->file_size == 0) {
        ret = 0;
    }
    else
    {
        ret = osPfsDeleteFile(&ControllerPak,
            FileState[filenumb].company_code,
            FileState[filenumb].game_code,
            (u8*)FileState[filenumb].game_name,
            (u8*)FileState[filenumb].ext_name);

        if (ret == PFS_ERR_INCONSISTENT)
            ret = osPfsChecker(&ControllerPak);

        if (ret == 0)
        {
            Pak_Memory += (fState->file_size >> 8);
            fState->file_size = 0;
        }
    }

    I_UnlockPi();

    return ret;
}

int I_SavePakFile(int filenumb, int flag, byte *data, int size) // 80007308
{
    int ret;

    I_LockPi();

    ret = osPfsReadWriteFile(&ControllerPak, filenumb, (u8)flag, 0, size, (u8*)data);

    if (ret == PFS_ERR_INCONSISTENT)
        ret = osPfsChecker(&ControllerPak);

    I_UnlockPi();

    return ret;
}

#define COMPANY_CODE 0x3544     // 5D
#define GAME_CODE 0x4e454441    // NEDA

int I_ReadPakFile(void) // 800073B8
{
    int ret;
    u8 *ext_name;

    I_LockPi();

    Pak_Data = NULL;
    Pak_Size = 0;
    ext_name = NULL;

    ret = osPfsFindFile(&ControllerPak, COMPANY_CODE, GAME_CODE, (u8*)Game_Name, ext_name, &File_Num);

    if (ret == 0)
    {
        Pak_Size = FileState[File_Num].file_size;
        Pak_Data = (byte *)Z_Malloc(Pak_Size, PU_STATIC, NULL);
        ret = osPfsReadWriteFile(&ControllerPak, File_Num, PFS_READ, 0, Pak_Size, Pak_Data);
    }

    I_UnlockPi();

    return ret;
}

int I_CreatePakFile(void) // 800074D4
{
    int ret;
    u8 ExtName [8];

    I_LockPi();

    if (Pak_Memory < 2)
        Pak_Size = 256;
    else
        Pak_Size = 512;

    Pak_Data = (byte *)Z_Malloc(Pak_Size, PU_STATIC, NULL);
    D_memset(Pak_Data, 0, Pak_Size);

    *(int*)ExtName = 0;

    ret = osPfsAllocateFile(&ControllerPak, COMPANY_CODE, GAME_CODE, (u8*)Game_Name, ExtName, Pak_Size, &File_Num);

    if (ret == PFS_ERR_INCONSISTENT)
        ret = osPfsChecker(&ControllerPak);

    if (ret == 0)
        ret = osPfsReadWriteFile(&ControllerPak, File_Num, PFS_WRITE, 0, Pak_Size, Pak_Data);

    I_UnlockPi();

    return ret;
}

void I_CheckControllerStatus()
{
    I_LockPi();

    osContStartQuery(&sys_msgque_joy);
    osRecvMesg(&sys_msgque_joy, NULL, OS_MESG_BLOCK);
    osContGetQuery(gamepad_status);

    for (register int i = 0; i < MAXCONTROLLERS; i++)
    {
        register int bit = (1 << i);
        register boolean rumble = false;

        if ((gamepad_status[i].type & CONT_TYPE_MASK) == CONT_TYPE_NORMAL)
        {
            gamepad_bit_pattern |= bit;

            if (osMotorInit(&sys_msgque_joy, &RumblePaks[i], i) == 0)
                rumble = true;
        }
        else
        {
            gamepad_bit_pattern &= ~bit;
        }

        if (rumble)
            rumblepak_bit_pattern |= bit;
        else
            rumblepak_bit_pattern &= ~bit;
    }

    I_UnlockPi();
}

void I_RumbleAmbient(int pad, int count)
{
    if (rumblepak_bit_pattern & (1 << pad))
        MotorAmbientCount[pad] += count;
}

void I_RumbleShot(int pad, int tics)
{
    if (rumblepak_bit_pattern & (1 << pad))
        MotorDamageTimers[pad] = MAX(tics, MotorDamageTimers[pad]);
}

void I_RumbleDamage(int pad, int damage)
{
    if (rumblepak_bit_pattern & (1 << pad))
    {
        damage = sqrtf(MAX(damage, 10) << 3);
        MotorDamageTimers[pad] = MIN(damage + MotorDamageTimers[pad], 0xffff);
    }
}

void I_StopRumble(void)
{
    D_memset(MotorDamageTimers, 0, sizeof MotorDamageTimers);
    D_memset(MotorAmbientCount, 0, sizeof MotorAmbientCount);
}

void I_ControllerThread(void *arg)
{
    SET_GP();

    while (1)
    {
        osRecvMesg(&joy_cmd_msgque, NULL, OS_MESG_BLOCK);

        motor += 1;
        if (!gamepaused)
        {
            for (int i = 0; i < MAXCONTROLLERS; i++)
            {
                if (MotorDamageTimers[i] > 0)
                    MotorDamageTimers[i]--;
            }
        }

        PiLockedJoy = true;

        if (PiLockedMain)
        {
            osSetThreadPri(&joy_thread, 10);
            while (PiLockedMain)
                osYieldThread();
            osSetThreadPri(&joy_thread, 11);
        }

        osContStartReadData(&sys_msgque_joy);
        osRecvMesg(&sys_msgque_joy, NULL, OS_MESG_BLOCK);
        osContGetReadData(gamepad_data);

        for (register int i = 0; i < MAXCONTROLLERS; i++)
        {
            register int bit = 1 << i;
            if (rumblepak_bit_pattern & bit)
            {
                register int set = !gamepaused
                    && (MotorDamageTimers[i] || (MotorAmbientCount[i] && (motor & 1)));
                set = (!!set) << i;
                if ((motor_bit_pattern & bit) != set)
                {
                    motor_bit_pattern = (motor_bit_pattern & ~bit) | set;
                    if (__osMotorAccess(&RumblePaks[i], set ? MOTOR_START : MOTOR_STOP) != 0)
                        rumblepak_bit_pattern &= ~bit;
                }
            }
        }

        PiLockedJoy = false;
    }
}
