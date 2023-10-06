/***************************************************************
                            debug.c

A basic debug library that makes use of the USB library for N64
flashcarts.
https://github.com/buu342/N64-UNFLoader
***************************************************************/

#include "i_debug.h"
#include "i_usb.h"
#include "doomdef.h"
#ifdef NDEBUG
#include "st_main.h"
#else
#include <os_internal.h>
#include <internal/osint.h>
#include <internal/viint.h>
#include "blit32.h"
#endif

/*********************************
         Settings macros
*********************************/

// Debug thread definitions
#define DEBUG_THREAD_ID    13
#define DEBUG_THREAD_STACK 0x200
#define MAX_STACK_TRACE    100

#ifndef NDEBUG
#ifdef USB_GDB

#include "p_spec.h"

#define DEBUG_MSGS 8

static OSThread *DebugThread;
static int InfoThread = -1;
static boolean NonStop = 0;

static void I_ShowDebugScreen(const char *text);

#else /* USB_GDB */
#define DEBUG_MSGS 1
static const char *ErrorText = NULL;
#endif /* USB_GDB */
#endif /* !defined(NDEBUG) */

/*********************************
           Definitions
*********************************/

#define MSG_FAULT 0
#define MSG_CPU_BREAK 1
#define MSG_SP_BREAK 2
#define MSG_GDB_PACKET 3

#define BUFFER_SIZE    256

#ifdef LOGGING
static void I_PrintNoop(const char *msg, u32 len) {}
void (*D_print)(const char *, u32) = I_PrintNoop;
#endif /* LOGGING */

#ifdef USB
void I_USBPrint(const char* message, unsigned long len);
void I_USBPrintHelp(void) SEC_STARTUP;
#endif /* USB */

#ifndef NDEBUG

static void I_DebuggerThread(void *arg) COLD;

static boolean I_InitIsViewer(void) SEC_STARTUP;
static void I_PrintIsViewer(const char *message, u32 len);

static void* DoomPrintf(void* arg, const u8* str, u32 count);
#if !defined(NDEBUG) && !defined(DEBUGOPT)
static void DoomErrorHandler(s16 code, s16 numArgs, ...);
#endif

// Fault thread globals
static OSMesgQueue debugMessageQ;
static OSMesg      debugMessageBuf[DEBUG_MSGS];
static OSThread    debugThread;
static vu64        debugThreadStack[DEBUG_THREAD_STACK/sizeof(u64)];

#ifdef USB_GDB
static OSMesgQueue gdbDoneQ;
static OSMesg      gdbDoneQBuf;
#endif

#endif /* !defined(NDEBUG) */

/*********************************
         Debug functions
*********************************/

/* Terminates the program with an error message. */

NO_RETURN COLD void I_Fatal(const char *error) // 80005F30
{
#ifdef NDEBUG

    I_ClearFrame();
    I_ClearFB(0x000000ff);
    ST_Message(16, 16, error, 0xffffffff);
    I_DrawFrame();

#else /* NDEBUG */

    // invoke the debugger by triggering a CPU break
#ifndef USB_GDB

    // show the message and a stack trace on screen
    ErrorText = error;

#endif /* !defined(USB_GDB) */

    BREAKPOINT();

#endif /* NDEBUG */

    while (1)
        osYieldThread();
}

#ifdef NDEBUG
void I_Error(const char *error, ...) // 80005F30
{
    char buffer[256];
    va_list args;
    va_start (args, error);
    u32 len = D_vsprintf (buffer, error, args);
    va_end (args);

    buffer[len < 0 ? 0 : len] = '\0';

    I_Fatal(buffer);
}
#else /* NDEBUG */
void I_ErrorFull(const char *file, int line, const char *func, const char *expr, const char *error, ...)
{
    char buffer[512];
    char *cur = buffer;

    if (expr)
        cur += sprintf(cur, "Assertion Failed: %s\n", expr);
    else
        cur += sprintf(cur, "Fatal Error\n");

    if (func && file)
        cur += sprintf(cur, "%s: %s:%d\n", func, file, line);

    if (error)
    {
        va_list args;

        va_start (args, error);
        int len = D_vsprintf (cur, error, args);
        if (len > 0)
            cur += len;
        va_end (args);
    }

    *cur = '\0';

    I_Fatal(buffer);
}

#endif /* NDEBUG */

#ifdef DEBUG_MEM

void I_CheckStack(vu64 *stack, const char *name)
{
    if (stack[0] != STACK_GUARD)
    {
        int len = D_strlen(name);
        char buf[32];
        char *b = buf;

        D_strncpy(b, name, sizeof buf);
        D_strncpy(b + len, " Stack Overflow", sizeof buf - len);

        I_Fatal(buf);
    }
}

#endif /* DEBUG_MEM */

#ifdef LOGGING

#define LoggingEnabled() (D_print != I_PrintNoop)

SEC_STARTUP void I_InitDebugging()
{
#ifdef USB
    if (FlashCart)
        D_print = I_USBPrint;
#endif /* USB */

#ifndef NDEBUG
    if (!LoggingEnabled() && I_InitIsViewer())
        D_print = I_PrintIsViewer;

    extern void* __printfunc;
    __printfunc = DoomPrintf;

#ifndef DEBUGOPT
    extern OSErrorHandler __osCommonHandler;
    __osCommonHandler = DoomErrorHandler;
#endif
#endif /* !defined(NDEBUG) */

#ifdef DEBUG_MEM
    debugThreadStack[0] = STACK_GUARD;
#endif

#ifndef NDEBUG
    // Initialize the debug thread
    osCreateMesgQueue(&debugMessageQ, debugMessageBuf, ARRAYLEN(debugMessageBuf));
#ifdef USB_GDB
    osCreateMesgQueue(&gdbDoneQ, &gdbDoneQBuf, 1);
#endif
    osSetEventMesg(OS_EVENT_FAULT, &debugMessageQ, (OSMesg) MSG_FAULT);
    osSetEventMesg(OS_EVENT_CPU_BREAK, &debugMessageQ, (OSMesg) MSG_CPU_BREAK);
    osSetEventMesg(OS_EVENT_SP_BREAK, &debugMessageQ, (OSMesg) MSG_SP_BREAK);
    osCreateThread(&debugThread, DEBUG_THREAD_ID, I_DebuggerThread, NULL,
                    (void*)(debugThreadStack+DEBUG_THREAD_STACK/sizeof(u64)),
                    OS_PRIORITY_RMON);
    debugThread.context.sr |= SR_CU1;
    debugThread.fp = 0;
    osStartThread(&debugThread);
#endif /* !defined(NDEBUG) */
}

SEC_STARTUP void L_Init()
{
#ifdef USB
    if (FlashCart)
        I_USBPrintHelp();
#endif /* USB */

#ifdef USB_GDB
    I_ShowDebugScreen("Waiting for gdb connection...");
    I_RefreshVideo();
    BREAKPOINT();
#endif
}

#if !defined(NDEBUG) && !defined(DEBUGOPT)
static COLD NO_RETURN void DoomErrorHandler(s16 code, s16 numArgs, ...) {
    va_list args;
    char buf[256];
    char *b = buf;

    extern const char* __os_error_message[];

    va_start(args, numArgs);

    b += D_sprintf(b, "0x%08lX (%04d):", osGetCount(), code);
    b += D_vsprintf(b,  __os_error_message[code], args);
    b += D_sprintf(b, "\n");

    va_end(args);

    I_Error("%s", buf);
}
#endif  /* !defined(NDEBUG) && !defined(DEBUGOPT) */

#ifndef NDEBUG
static void* DoomPrintf(void* arg, const u8* str, u32 count)
{
    D_print((const char *) str, count);
    return (void*) 1;
}
#endif

void D_printf(const char* message, ...)
{
    int len = 0;
    char buf[256];
    va_list args;

    va_start(args, message);
    len = D_vsprintf(buf, message, args);
    va_end(args);

    if (len < 0)
        return;

    buf[len] = '\0';

    D_print(buf, len);
}

#endif /* LOGGING */

#ifndef NDEBUG

#define ISVIEWER_MAGIC           0xB3FF0000
#define ISVIEWER_READ_LEN        0xB3FF0004
#define ISVIEWER_WRITE_LEN       0xB3FF0014
#define ISVIEWER_BUFFER          0xB3FF0020
#define ISVIEWER_BUFFER_LEN      0x00000200

static boolean I_InitIsViewer(void)
{
    OSPiHandle *handle = osCartRomInit();
    u32 data;

    osEPiWriteIo(handle, ISVIEWER_BUFFER, 0x12345678);
    osEPiReadIo(handle, ISVIEWER_BUFFER, &data);
    boolean present = data == 0x12345678;
    if (present)
    {
        osEPiWriteIo(handle, ISVIEWER_READ_LEN, 0);
        osEPiWriteIo(handle, ISVIEWER_WRITE_LEN, 0);
        osEPiWriteIo(handle, ISVIEWER_MAGIC, 0x49533634);
    }

    return present;
}

static void I_PrintIsViewer(const char *message, u32 len)
{
    OSPiHandle *handle = osCartRomInit();
    u32 readstart, start;

    osEPiReadIo(handle, ISVIEWER_MAGIC, &start);
    if (start != 0x49533634)
        return;

    osEPiReadIo(handle, ISVIEWER_READ_LEN, &readstart);
    osEPiReadIo(handle, ISVIEWER_WRITE_LEN, &start);
    u32 end = start + len;

    if (end >= 0xffe0) {
        end -= 0xffe0;
        if (readstart < end || start < readstart)
            return;
    } else {
        if (start < readstart && readstart < end)
            return;
    }

    while (len) {
        if (*message != '\0') {
            u32 c;
            s32 shift = start & 3;
            u32 addr = ISVIEWER_BUFFER + (start & 0xFFFC);

            shift = (3 - shift) * 8;

            osEPiReadIo(handle, addr, &c);
            osEPiWriteIo(handle, addr, (c & ~(0xff << shift)) | (*message << shift));

            start++;
            if (start >= 0xffe0) {
                start -= 0xffe0;
            }
        }
        len--;
        message++;
    }
    osEPiWriteIo(handle, ISVIEWER_WRITE_LEN, start);
}

#define RSP_TID        ((OSId) 1000)
#define RSP_THREAD ((OSThread*)1)

static COLD OSThread *I_FaultedThread(int msg)
{
    switch (msg)
    {
    case MSG_FAULT:
        return __osGetCurrFaultedThread();
        break;
    case MSG_CPU_BREAK:
        {
            register OSThread *thread;

            thread = __osGetActiveQueue();
            while (thread->priority != -1 && !(thread->flags & OS_FLAG_CPU_BREAK))
                thread = thread->tlnext;

            if (thread->priority == -1)
                return NULL;

            return thread;
        }
        break;
    case MSG_SP_BREAK:
        return RSP_THREAD;
    }
    return NULL;
}

static COLD void I_ForEachThread(void (*func)(OSThread *))
{
    OSThread *thread = __osGetActiveQueue();
    while (thread->priority != -1)
    {
        if (thread->priority > OS_PRIORITY_IDLE && thread->priority <= OS_PRIORITY_APPMAX)
            func(thread);

        thread = thread->tlnext;
    }
}

static COLD void I_StopAppThreads(void)
{
    // TODO - stop RSP
    I_ForEachThread(osStopThread);
}

#ifndef USB_GDB

typedef struct
{
    u32 mask;
    u32 value;
    char *string;
} regDesc;

// List of error causes
static const regDesc causeDesc[] = {
    {CAUSE_BD,      CAUSE_BD,    "BD"},
    {CAUSE_IP8,     CAUSE_IP8,   "IP8"},
    {CAUSE_IP7,     CAUSE_IP7,   "IP7"},
    {CAUSE_IP6,     CAUSE_IP6,   "IP6"},
    {CAUSE_IP5,     CAUSE_IP5,   "IP5"},
    {CAUSE_IP4,     CAUSE_IP4,   "IP4"},
    {CAUSE_IP3,     CAUSE_IP3,   "IP3"},
    {CAUSE_SW2,     CAUSE_SW2,   "IP2"},
    {CAUSE_SW1,     CAUSE_SW1,   "IP1"},
    {CAUSE_EXCMASK, EXC_INT,     "Interrupt"},
    {CAUSE_EXCMASK, EXC_MOD,     "TLB modification exception"},
    {CAUSE_EXCMASK, EXC_RMISS,   "TLB exception on load or instruction fetch"},
    {CAUSE_EXCMASK, EXC_WMISS,   "TLB exception on store"},
    {CAUSE_EXCMASK, EXC_RADE,    "Address error on load or instruction fetch"},
    {CAUSE_EXCMASK, EXC_WADE,    "Address error on store"},
    {CAUSE_EXCMASK, EXC_IBE,     "Bus error exception on instruction fetch"},
    {CAUSE_EXCMASK, EXC_DBE,     "Bus error exception on data reference"},
    {CAUSE_EXCMASK, EXC_SYSCALL, "System call exception"},
    {CAUSE_EXCMASK, EXC_BREAK,   "Breakpoint exception"},
    {CAUSE_EXCMASK, EXC_II,      "Reserved instruction exception"},
    {CAUSE_EXCMASK, EXC_CPU,     "Coprocessor unusable exception"},
    {CAUSE_EXCMASK, EXC_OV,      "Arithmetic overflow exception"},
    {CAUSE_EXCMASK, EXC_TRAP,    "Trap exception"},
    {CAUSE_EXCMASK, EXC_VCEI,    "Virtual coherency exception on intruction fetch"},
    {CAUSE_EXCMASK, EXC_FPE,     "Floating point exception (see fpcsr)"},
    {CAUSE_EXCMASK, EXC_WATCH,   "Watchpoint exception"},
    {CAUSE_EXCMASK, EXC_VCED,    "Virtual coherency exception on data reference"},
    {0,             0,           ""}
};

// List of register descriptions
static const regDesc srDesc[] = {
    {SR_CU3,      SR_CU3,     "CU3"},
    {SR_CU2,      SR_CU2,     "CU2"},
    {SR_CU1,      SR_CU1,     "CU1"},
    {SR_CU0,      SR_CU0,     "CU0"},
    {SR_RP,       SR_RP,      "RP"},
    {SR_FR,       SR_FR,      "FR"},
    {SR_RE,       SR_RE,      "RE"},
    {SR_BEV,      SR_BEV,     "BEV"},
    {SR_TS,       SR_TS,      "TS"},
    {SR_SR,       SR_SR,      "SR"},
    {SR_CH,       SR_CH,      "CH"},
    {SR_CE,       SR_CE,      "CE"},
    {SR_DE,       SR_DE,      "DE"},
    {SR_IBIT8,    SR_IBIT8,   "IM8"},
    {SR_IBIT7,    SR_IBIT7,   "IM7"},
    {SR_IBIT6,    SR_IBIT6,   "IM6"},
    {SR_IBIT5,    SR_IBIT5,   "IM5"},
    {SR_IBIT4,    SR_IBIT4,   "IM4"},
    {SR_IBIT3,    SR_IBIT3,   "IM3"},
    {SR_IBIT2,    SR_IBIT2,   "IM2"},
    {SR_IBIT1,    SR_IBIT1,   "IM1"},
    {SR_KX,       SR_KX,      "KX"},
    {SR_SX,       SR_SX,      "SX"},
    {SR_UX,       SR_UX,      "UX"},
    {SR_KSU_MASK, SR_KSU_USR, "USR"},
    {SR_KSU_MASK, SR_KSU_SUP, "SUP"},
    {SR_KSU_MASK, SR_KSU_KER, "KER"},
    {SR_ERL,      SR_ERL,     "ERL"},
    {SR_EXL,      SR_EXL,     "EXL"},
    {SR_IE,       SR_IE,      "IE"},
    {0,           0,          ""}
};

// List of floating point registers descriptions
static const regDesc fpcsrDesc[] = {
    {FPCSR_FS,      FPCSR_FS,    "FS"},
    {FPCSR_C,       FPCSR_C,     "C"},
    {FPCSR_CE,      FPCSR_CE,    "Unimplemented operation"},
    {FPCSR_CV,      FPCSR_CV,    "Invalid operation"},
    {FPCSR_CZ,      FPCSR_CZ,    "Division by zero"},
    {FPCSR_CO,      FPCSR_CO,    "Overflow"},
    {FPCSR_CU,      FPCSR_CU,    "Underflow"},
    {FPCSR_CI,      FPCSR_CI,    "Inexact operation"},
    {FPCSR_EV,      FPCSR_EV,    "EV"},
    {FPCSR_EZ,      FPCSR_EZ,    "EZ"},
    {FPCSR_EO,      FPCSR_EO,    "EO"},
    {FPCSR_EU,      FPCSR_EU,    "EU"},
    {FPCSR_EI,      FPCSR_EI,    "EI"},
    {FPCSR_FV,      FPCSR_FV,    "FV"},
    {FPCSR_FZ,      FPCSR_FZ,    "FZ"},
    {FPCSR_FO,      FPCSR_FO,    "FO"},
    {FPCSR_FU,      FPCSR_FU,    "FU"},
    {FPCSR_FI,      FPCSR_FI,    "FI"},
    {FPCSR_RM_MASK, FPCSR_RM_RN, "RN"},
    {FPCSR_RM_MASK, FPCSR_RM_RZ, "RZ"},
    {FPCSR_RM_MASK, FPCSR_RM_RP, "RP"},
    {FPCSR_RM_MASK, FPCSR_RM_RM, "RM"},
    {0,             0,           ""}
};

/*==============================
    I_PrintRegsiter
    Prints info about a register
    @param The value of the register
    @param The name of the register
    @param The registry description to use
==============================*/

static COLD int I_PrintRegister(char *out, u32 value, const char *name, const regDesc *desc)
{
    char first = 1;
    char *start = out;
    out += sprintf(out, "%5s 0x%08lX <", name, value);
    while (desc->mask != 0)
    {
        if ((value & desc->mask) == desc->value)
        {
            (first) ? (first = 0) : ((void) (out += sprintf(out, ",")));
            out += sprintf(out, "%s", desc->string);
        }
        desc++;
    }
    out += sprintf(out, ">\n");
    return out - start;
}

static COLD int I_FindPrevSP(void** prev_sp, void** prev_ra, const void* sp, const void* ra)
{
    unsigned* wra = (unsigned*)ra;
    unsigned* k0base = (unsigned*)K0BASE;
    int spofft;

    if (wra < k0base) {
        return 0;
    }
    /* scan towards the beginning of the function -
       addui sp,sp,spofft should be the first command */
    while ((*wra >> 16) != 0x27bd) {
        /* test for "scanned too much" */
        if (wra < k0base) {
            return 0;
        }
        wra--;
    }
    spofft = ((int)*wra << 16) >> 16; /* sign-extend */
    *prev_sp = (char*)sp - spofft;
    /* now scan forward for sw r31,raofft(sp) */
    while (wra < (unsigned*)ra) {
        if ((*wra >> 16) == 0xafbf) {
            int raofft = ((int)*wra << 16) >> 16; /* sign */
            *prev_ra = *(void**)((char*)sp + raofft);
            return 1;
        }
        wra++;
    }
    return 0; /* failed to find where ra is saved */
}

static int I_GetCallStack(void **addresses, u64 sp_val, u64 ra_val) {
    void* sp = (void*)(u32)sp_val; /* stack pointer from thread state */
    void* ra = (void*)(u32)ra_val; /* return address from thread state */
    int i = 0;

    while (i < MAX_STACK_TRACE &&
            I_FindPrevSP(&sp, &ra, sp, ra) && ra != 0) {
        addresses[i++] = ra;
    }
    return i; /* stack size */
}

#define FAULT_MSG_BUFFER ((char *)CFB(1))
#define FAULT_BT_BUFFER ((void**)(FAULT_MSG_BUFFER + 0x4000))

static inline COLD __attribute__((nonnull(1, 2)))
char *I_PrintFault(OSThread *curr, char *out)
{
    __OSThreadContext* context = &curr->context;
    // normal framebuffer is not used when showing fault message, so use it as a scratch area
    void **addresses = FAULT_BT_BUFFER;
    int backtracesize;

    // Print any message passed from I_Error
    if (ErrorText)
        out += sprintf(out, "%s\n", ErrorText);

    // Print the basic info
    out += sprintf(out, "Stopped thread %ld\n\n", curr->id);
    out += I_PrintRegister(out, context->cause, "cause", causeDesc);
    out += I_PrintRegister(out, context->sr, "sr", srDesc);
    out += I_PrintRegister(out, context->fpcsr, "fpcsr", fpcsrDesc);
    out += sprintf(out, "badva 0x%08lX\n", context->badvaddr);

    out += sprintf(out, "Backtrace:\n");
    backtracesize = I_GetCallStack(addresses, context->sp, context->ra);

    out += sprintf(out, "  0x%08lX\n", context->pc);
    for (int i = 0; i < backtracesize; ++i) {
        out += sprintf(out, "  0x%08lX\n", (u32)addresses[i]);
    }

    out += sprintf(out, "\n");

    // Print the registers
    out += sprintf(out, "at 0x%016llX v0 0x%016llX v1 0x%016llX\n", context->at, context->v0, context->v1);
    out += sprintf(out, "a0 0x%016llX a1 0x%016llX a2 0x%016llX\n", context->a0, context->a1, context->a2);
    out += sprintf(out, "a3 0x%016llX t0 0x%016llX t1 0x%016llX\n", context->a3, context->t0, context->t1);
    out += sprintf(out, "t2 0x%016llX t3 0x%016llX t4 0x%016llX\n", context->t2, context->t3, context->t4);
    out += sprintf(out, "t5 0x%016llX t6 0x%016llX t7 0x%016llX\n", context->t5, context->t6, context->t7);
    out += sprintf(out, "s0 0x%016llX s1 0x%016llX s2 0x%016llX\n", context->s0, context->s1, context->s2);
    out += sprintf(out, "s3 0x%016llX s4 0x%016llX s5 0x%016llX\n", context->s3, context->s4, context->s5);
    out += sprintf(out, "s6 0x%016llX s7 0x%016llX t8 0x%016llX\n", context->s6, context->s7, context->t8);
    out += sprintf(out, "t9 0x%016llX gp 0x%016llX sp 0x%016llX\n", context->t9, context->gp, context->sp);
    out += sprintf(out, "s8 0x%016llX ra 0x%016llX\n\n",            context->s8, context->ra);

    // Print the floating point registers
    out += sprintf(out, "\n");
    out += sprintf(out, "d0  %.15e\td2  %.15e\n", context->fp0.d,  context->fp2.d);
    out += sprintf(out, "d4  %.15e\td6  %.15e\n", context->fp4.d,  context->fp6.d);
    out += sprintf(out, "d8  %.15e\td10 %.15e\n", context->fp8.d,  context->fp10.d);
    out += sprintf(out, "d12 %.15e\td14 %.15e\n", context->fp12.d, context->fp14.d);
    out += sprintf(out, "d16 %.15e\td18 %.15e\n", context->fp16.d, context->fp18.d);
    out += sprintf(out, "d20 %.15e\td22 %.15e\n", context->fp20.d, context->fp22.d);
    out += sprintf(out, "d24 %.15e\td26 %.15e\n", context->fp24.d, context->fp26.d);
    out += sprintf(out, "d28 %.15e\td30 %.15e\n", context->fp28.d, context->fp30.d);
    out += sprintf(out, "\n\n");

    return out;
}

static OSViMode debug_vi_mode;

static COLD bool I_DebugSetMode(u16 *fb)
{
    bool hires = false;

    for (int i = 0; i < CFB_SIZE/sizeof(u16); i++)
        fb[i] = 0x0001;

    if (CFB_SIZE >= 640*480*sizeof(u16))
    {
        switch(osTvType)
        {
            case OS_TV_PAL: debug_vi_mode = osViModePalHan1; break;
            case OS_TV_MPAL: debug_vi_mode = osViModeMpalHan1; break;
            default: debug_vi_mode = osViModeNtscHan1; break;
        }
        hires = true;
    }
    else
    {
        switch(osTvType)
        {
            case OS_TV_PAL: debug_vi_mode = osViModePalLan1; break;
            case OS_TV_MPAL: debug_vi_mode = osViModeMpalLan1; break;
            default: debug_vi_mode = osViModeNtscLan1; break;
        }
    }

    debug_vi_mode.comRegs.ctrl &= ~(VI_CTRL_GAMMA_DITHER_ON|VI_CTRL_ANTIALIAS_MODE_3);

    osViSetMode(&debug_vi_mode);
    osViBlack(FALSE);
    osViSwapBuffer(fb);
    __osViSwapContext();

    return hires;
}

/* Fault/break thread */

static COLD void NO_RETURN I_DebuggerThread(void *arg)
{
    SET_GP();

    {
        OSMesg msg;
        u16 *fb;
        int mask;
        int shift;

        osRecvMesg(&debugMessageQ, &msg, OS_MESG_BLOCK);

        I_StopAppThreads();

        mask = __osDisableInt();

        {
            int start = osGetCount();

            /* wait up to 1 second for the rcp to stop */
            while (__osSpDeviceBusy() || __osDpDeviceBusy())
                if (osGetCount() - start >= OS_USEC_TO_CYCLES(1000000))
                    break;

            /* wait for vsync */
            while (IO_READ(VI_CURRENT_REG) > 10) {}

            osViSetYScale(1.0);
            osViBlack(TRUE);
            __osViSwapContext();
        }

        fb = (u16*) CFB(0);

        if (I_DebugSetMode(fb))
            shift = 1;
        else
            shift = 0;

        {
            char *buf = FAULT_MSG_BUFFER;
            char *end = buf;
            OSThread *thread = I_FaultedThread((int) msg);

            if (thread == RSP_THREAD)
            {
                end += sprintf(end, "RSP Break");
                // TODO - print RSP registers
            }
            else
            {
                while (thread != NULL)
                {
                    end = I_PrintFault(thread, end);
                    if ((int) msg == MSG_FAULT)
                        thread = __osGetNextFaultedThread(thread);
                    else
                        thread = NULL;
                }
            }

            blit32_TextExplicit(fb, 0xffff, 1, 320<<shift, 240<<shift, blit_Clip, 32, 24, FAULT_MSG_BUFFER);

            osInvalDCache(fb, CFB_SIZE);

            __osRestoreInt(mask);

            D_print(buf, end - buf);
        }
    }

    while (1)
        osYieldThread();
}

#else /* !defined(USB_GDB) */

/*
 * Sections below taken from https://github.com/murachue/gdbstub-ed64
 *
 * Copyright (c) 2018 Murachue <murachue+github@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

// used to derive offsets into 'g' packet
// ref: https://sourceware.org/git/?p=binutils-gdb.git;a=blob;f=gdb/mips-tdep.c;h=4044daa24131517b6cafb7ab536a7e4bdf6a7947;hb=HEAD#l8309
#define REGS_GPR 0
#define REGS_SR 32
#define REGS_LO 33
#define REGS_HI 34
#define REGS_BAD 35
#define REGS_CAUSE 36
#define REGS_PC 37
#define REGS_FGR 38
#define REGS_FPCS 70
#define REGS_FPIR 71
#define REGS_SIZE 72

typedef enum {
    STUB_SUCCESS = 0,
    STUBERR_UNKNOWN = 1,
    STUBERR_UNDERFLOW, /* Not enough data */
    STUBERR_OVERFLOW, /* Too much data */
    STUBERR_PARSE, /* Could not parse hex value */
    STUBERR_THREADNOTFOUND, /* Thread ID Not Found */
    STUBERR_SEGV, /* Bad memory access */
    STUBERR_BADADDR, /* Invalid address */
    STUBERR_TOOMANY, /* Too many breakpoints/watchpoints */
} stubstatus_t;

static struct {
    s32 invaddr; /* 0(~-1) = unset. inverted to be able to initialized as part of bss. */
    u32 value;
} origbpcodes[2] = {{0,0},{0,0}};

static COLD s32 hex2int(u8 c) {
    if('0' <= c && c <= '9') { return c - '0'; }
    if('A' <= c && c <= 'F') { return c - 'A' + 10; }
    if('a' <= c && c <= 'f') { return c - 'a' + 10; }
    return -1;
}

static COLD s32 int2hex(register u8 *buf, register u32 bufsize, register s32 index,
                   register u32 value, register u32 nibbles) {
    u32 lpos;
    static const char i2h[16] = "0123456789ABCDEF";
    for(lpos = nibbles; 0 < lpos; lpos--) {
        if(bufsize <= index) return -1; /* overflow! */
        buf[index] = i2h[(value >> (4 * (lpos - 1))) & 15];
        index++;
    }
    return index;
}

static COLD s32 tohex(register u8 *buf, register u32 bufsize, register s32 index,
                      register const volatile void *src_, register u32 srclen)
{
    u32 i = 0;

    /* note: it sometimes access hardware register that requires specific bit width... don't simplify. */

    if((((u32)src_ & 3) == 0) && ((srclen & 3) == 0)) {
        /* word access */
        const volatile u32 *src = src_;
        for(; i < srclen / 4; i++) {
            u32 s = src[i];

            if((index = int2hex(buf, bufsize, index, s, 8)) < 0) {
                return -1; /* overflow! */
            }
        }
    } else if((((u32)src_ & 1) == 0) && ((srclen & 1) == 0)) {
        /* halfword access */
        const volatile u16 *src = src_;
        for(; i < srclen / 2; i++) {
            u16 s = src[i];

            if((index = int2hex(buf, bufsize, index, s, 4)) < 0) {
                return -1; /* overflow! */
            }
        }
    } else {
        /* byte access */
        const volatile u8 *src = src_;
        for(; i < srclen; i++) {
            u8 s = src[i];
            if((index = int2hex(buf, bufsize, index, s, 2)) < 0) {
                return -1; /* overflow! */
            }
        }
    }
    return index;
}

static u8 ALIGNED(8) DebugTx[2048];
static u8 ALIGNED(8) DebugRx[2048];
static u32 DebugTxPos = 2;
static u32 DebugRxPos = 0;
static u32 DebugRxEnd = 0;
static void (*WritePacket)(const u8 *, u32);

static COLD void pkt_send_raw(u32 bufsize, u32 len) {
    /* zerofill tail */
    for(; len < bufsize; len++) {
        DebugTx[len] = 0;
    }

    /* go!! */
    WritePacket(DebugTx, bufsize);
}

static COLD void pkt_putc(u8 c)
{
    DebugTx[DebugTxPos++] = c;
}

static COLD void pkt_put(const char *data, u32 len)
{
    D_memcpy(&DebugTx[DebugTxPos], data, len);
    DebugTxPos += len;
}

static COLD stubstatus_t pkt_put_hex(const void *data, u32 len)
{
    int ret = tohex(DebugTx, sizeof DebugTx, DebugTxPos, data, len);
    if (ret < 0)
        return -1;
    DebugTxPos = ret;
    return 0;
}

#define pkt_put_str(_str) pkt_put((_str), sizeof(_str) - 1)

static COLD void pkt_send(void) {
    s32 si;

    if(DebugTxPos < 4)
        I_Error("pkt_send: buffer too small");

    DebugTx[0] = '+';
    DebugTx[1] = '$';

    for(si = 0; si < sizeof DebugTx - 3; si++) {
        if(DebugTx[si] == '$') {
            break;
        }
    }
    if(si == sizeof DebugTx - 3)
        I_Error("pkt_send: no start marker");

    si++;

    DebugTx[DebugTxPos] = '#';

    /* calculate and fill checksum */
    {
        u32 sum = 0;
        {
            u32 i;
            for(i = si; i < DebugTxPos; i++) {
                sum += DebugTx[i];
            }
        }
        {
            u8 sumb[1] = {sum};
            if(tohex(DebugTx, sizeof DebugTx, DebugTxPos + 1, sumb, 1) < 0)
                I_Error("tohex: overflow");
        }
    }
    DebugTxPos += 1 + 2;

    /* go!! */
    {
        u32 ssize = (DebugTxPos + (512 - 1)) & -512;
        pkt_send_raw(ssize, DebugTxPos);
        DebugTxPos = 2;
    }
}

static INLINE_ALWAYS boolean pkt_has_bytes(u32 left)
{
    return DebugRxEnd >= DebugRxPos + left;
}

static INLINE_ALWAYS boolean pkt_is_eof(void)
{
    return DebugRxEnd == DebugRxPos;
}

static COLD int pkt_getc(void)
{
    if (!pkt_has_bytes(1))
        return -1;
    return DebugRx[DebugRxPos++];
}

static COLD boolean pkt_acceptc(char c)
{
    if (!pkt_has_bytes(1))
        return false;
    if (DebugRx[DebugRxPos] == c)
    {
        DebugRxPos++;
        return true;
    }
    return false;
}

static COLD boolean pkt_accept(const char *s, u32 len)
{
    if (!pkt_has_bytes(len))
        return false;
    if (bcmp(&DebugRx[DebugRxPos], (void*)s, len) == 0)
    {
        DebugRxPos += len;
        return true;
    }
    return false;
}

#define pkt_accept_str(_str) pkt_accept((_str), sizeof(_str) - 1)

static COLD stubstatus_t pkt_get_hex(void *buf, u32 len)
{
    if (!pkt_has_bytes(len))
        return STUBERR_UNDERFLOW;

    for(u32 i = 0; i < len; i++)
    {
        u32 b = 0;
        s32 nib;
        nib = hex2int(DebugRx[DebugRxPos++]);
        if(nib < 0)
            return STUBERR_PARSE;
        b = nib << 4;

        nib = hex2int(DebugRx[DebugRxPos++]);
        if(nib < 0)
            return STUBERR_PARSE;
        b |= nib;

        *((u8*) buf++) = b;
    }
    return STUB_SUCCESS;
}

static COLD s64 pkt_get_hex_delim(register char delim)
{
    register int count = 0;
    u32 value = 0;
    for (; delim ? DebugRx[DebugRxPos] != delim : DebugRxPos != DebugRxEnd; DebugRxPos++)
    {
        if (DebugRxPos == DebugRxEnd)
            return -1;
        if (count >= 8)
            return -1;
        s32 nib = hex2int(DebugRx[DebugRxPos]);
        if(nib < 0)
            return -1;
        value = (value << 4) | nib;
        count++;
    }
    if (delim)
        DebugRxPos++;

    return value;
}

static COLD OSId pkt_get_threadid(void)
{
    s64 id = 0;

    if (pkt_accept_str("-1"))
        return -1;

    id = pkt_get_hex_delim(0);
    if (id < 0)
        return -2;

    return id;
}

static COLD OSThread *I_GetThreadById(OSId id)
{
    if (id == 0)
    {
        return DebugThread;
    }
    else if (id == RSP_TID)
    {
        return RSP_THREAD;
    }
    else if (id > 0)
    {
        OSThread *thread = __osGetActiveQueue();
        while (thread->priority != -1)
        {
            if (thread->id == id)
                return thread;
            thread = thread->tlnext;
        }
    }
    return NULL;
}

static u64 TmpRegister = 0;

/* convert a gdb register packet position to a pointer inside the OSThread structure */
static COLD void *os_thread_reg_ptr(register __OSThreadContext *c, register int reg)
{
    if (reg >= 1 && reg < REGS_GPR + 26)
        return ((u64*)c) + reg - 1;
    else if (reg >= REGS_GPR + 28 && reg < REGS_SR)
        return ((u64*)&c->gp) + reg - 1;
    else if (reg >= REGS_FGR && reg < REGS_FPCS)
        return ((u64*)&c->fp0) + reg - REGS_FGR;
    else if (reg == REGS_SR)
        return &c->sr;
    else if (reg == REGS_LO)
        return &c->lo;
    else if (reg == REGS_HI)
        return &c->lo;
    else if (reg == REGS_BAD)
        return &c->badvaddr;
    else if (reg == REGS_CAUSE)
        return &c->cause;
    else if (reg == REGS_PC)
        return &c->pc;
    else if (reg == REGS_FPCS)
        return &c->fpcsr;
    else
        return (void*)&TmpRegister;
}

/* g -> <regbytes> */
static COLD stubstatus_t cmd_getregs(void)
{
    __OSThreadContext *ctx = &DebugThread->context;
    const u64 *regptr;

    if (DebugThread == RSP_THREAD)
    {
        // TODO
    }
    else if (DebugThread)
    {
        TmpRegister = 0;
        for (u32 reg = 1; reg < REGS_SIZE; reg++)
        {
            regptr = os_thread_reg_ptr(ctx, reg);
            if (reg == REGS_FPIR) /* this is included even though always the same */
                asm("cfc1 %0, $0" : "=r" (TmpRegister)); /* regptr will be &TmpRegister */

            if (pkt_put_hex(regptr, sizeof *regptr) < 0)
                return STUBERR_UNDERFLOW;
        }
    }
    pkt_send();

    return STUB_SUCCESS;
}

/* G<regbytes> -> OK|Exx */
static COLD stubstatus_t cmd_setregs(void) {
    register __OSThreadContext *ctx = &DebugThread->context;
    register u64 *p;
    register u32 reg;
    register stubstatus_t status;

    if (DebugThread == RSP_THREAD)
    {
        // TODO
    }
    else if (DebugThread)
    {
        for(reg = 1; reg < REGS_SIZE; reg++)
        {
            p = os_thread_reg_ptr(ctx, reg);
            if (p == (void*)&TmpRegister)
                continue;
            status = pkt_get_hex(p, sizeof *p);
            if (status != STUB_SUCCESS)
                return status;
        }
    }
    pkt_put_str("OK");
    pkt_send();

    return STUB_SUCCESS;
}

/* m<ADDR>,<LEN> -> <membytes>|Exx */
static COLD stubstatus_t cmd_getmemory(void) {
    register stubstatus_t status;
    u32 addr, len;
    status = pkt_get_hex(&addr, sizeof addr);
    if (status != STUB_SUCCESS)
        return status;
    if (!pkt_acceptc(','))
        return STUBERR_PARSE;
    status = pkt_get_hex(&len, sizeof len);
    if (status != STUB_SUCCESS)
        return status;

    if(sizeof(DebugTx) - 5 < len / 2) {
        len = (sizeof(DebugTx) - 5) / 2;
    }


    osInvalDCache((void*)addr, len);
    osInvalICache((void*)addr, len);

    if (pkt_put_hex((void*)addr, len) < 0)
        return STUBERR_UNDERFLOW;
    pkt_send();

    return STUB_SUCCESS;
}

/* M<ADDR>,<LEN>:<BYTES> -> OK|Exx */
static COLD stubstatus_t cmd_setmemory(void) {
    register s64 saddr, len;

    saddr = pkt_get_hex_delim(',');
    if (saddr < 0)
        return STUBERR_PARSE;
    len = pkt_get_hex_delim(0);
    if (len < 0)
        return STUBERR_PARSE;

    {
        register u32 addr = saddr;
        /* remember addr,len at entry for flushing cache */
        register const u32 start_addr = addr, tobe_len = len;

        register u32 width;
        register u32 by, phase, i = DebugRxPos;

        /* note: it sometimes access hardware register that requires specific bit width... don't simplify. */

        if(((addr & 3) == 0) && ((len & 3) == 0)) {
            width = 4;
        } else if(((addr & 1) == 0) && ((len & 1) == 0)) {
            width = 2;
        } else {
            width = 1;
        }

        for(by = 0, phase = 0; len; DebugRxPos++) {
            register s32 nib;
            if(DebugRx[i] == '#') {
                break;
            }

            nib = hex2int(DebugRx[DebugRxPos]);
            if(nib < 0)
                return STUBERR_PARSE;

            by = (by << 4) | nib;
            switch(width) {
            case 1:
                phase = (phase + 1) % 2;
                if(phase == 0) {
                    *(u8*)addr = by;
                }
                break;
            case 2:
                phase = (phase + 1) % 4;
                if(phase == 0) {
                    *(u16*)addr = by;
                }
                break;
            case 4:
                phase = (phase + 1) % 8;
                if(phase == 0) {
                    *(u32*)addr = by;
                }
                break;
            }
            if(phase == 0) {
                addr += width;
                len -= width;
            }
        }

        /* to recognize new insn, ex. "break" */
        osInvalDCache((void*)start_addr, tobe_len);
        osInvalICache((void*)start_addr, tobe_len);

        pkt_put_str("OK");
        pkt_send();

        return STUB_SUCCESS;
    }
}

static COLD void bpset(u32 index, u32 addr, u32 value) {
    origbpcodes[index].invaddr = ~addr;
    origbpcodes[index].value = *(u32*)addr;
    *(u32*)addr = value;

    osInvalDCache((void*)addr, 4);
    osInvalICache((void*)addr, 4);
}

static COLD void bprestore() {
    u32 i;

    for(i = 0; i < sizeof(origbpcodes)/sizeof(*origbpcodes); i++) {
        if(~origbpcodes[i].invaddr != -1) {
            u32 *addr = (u32*)~origbpcodes[i].invaddr;

            *addr = origbpcodes[i].value;

            osInvalDCache(addr, 4);
            osInvalICache(addr, 4);

            origbpcodes[i].invaddr = ~-1;
        }
    }
}

/* s -> (stop_reply) */
/* ref: Linux 2.6.25:arch/mips/kernel/gdb-stub.c */
static COLD void cmd_step(OSThread *thread) {
    s32 pc;
    u32 insn;
    s32 tnext, fnext;

    if (!thread)
        return;

    if (thread == RSP_THREAD)
    {
        // TODO
        return;
    }

    pc = thread->context.pc;
    insn = *(u32*)pc;

    tnext = pc + 4;
    fnext = -1;

    if((insn & 0xF8000000) == 0x08000000) { /* J/JAL */
        tnext = (tnext & 0xFFFFffffF0000000) | ((insn & 0x03FFffff) << 2);
    } else if(0
        || ((insn & 0xFC0C0000) == 0x04000000) /* B{LT/GE}Z[AL][L] */
        || ((insn & 0xF0000000) == 0x10000000) /* B{EQ/NE/LE/GT} */
        || ((insn & 0xF3E00000) == 0x41000000) /* BCz{T/F}[L] */
        || ((insn & 0xF0000000) == 0x50000000) /* B{EQ/NE/LE/GT}L */
        ) {
        fnext = tnext + 4; /* next of branch-delay */
        tnext = tnext + ((s32)(s16)insn << 2);
    } else if((insn & 0xFC00003E) == 0x00000008) { /* JR/JALR */
        u32 reg = (insn >> 21) & 0x1F;
        /* ignore kernel registers */
        if (reg == 0 || reg == 26 || reg == 27) {
            tnext = 0;
        } else {
            if (reg >= 28)
                reg -= 2;
            tnext = (&thread->context.at)[reg - 1];
        }
    }

    if(fnext != -1) {
        bpset(1, fnext, 0x0000000D);
    }
    bpset(0, tnext, 0x0000000D);
}

#define _STR(x) #x
#define STR(x) _STR(x)

/* [zZ][0-4]<ADDR>,<KIND> -> OK|Exx */
static COLD stubstatus_t cmd_watch(void) {
    s32 set = 0, type = 0, kind = 0;
    u32 addr = 0;

    if(pkt_acceptc('Z'))
        set = 1;
    else if(pkt_acceptc('z'))
        set = 0;
    else
        return STUBERR_PARSE; /* programming error */

    type = pkt_get_hex_delim(',');
    if (type < 0)
        return STUBERR_PARSE;
    addr = pkt_get_hex_delim(',');
    if (addr < 0)
        return STUBERR_PARSE;
    kind = pkt_get_hex_delim(0);
    if (kind < 0)
        return STUBERR_PARSE;

    if(type < 2 || type > 4 || kind != 4) {
        /* software breakpoint is GDB's task... reply empty = "not supported" */
        pkt_send();
        return STUB_SUCCESS;
    }

    /* only kseg0/kseg1 are supported yet... */
    if(addr < 0x80000000 || 0xC0000000 <= addr)
        return STUBERR_BADADDR;

    /* VA->PA with 8bytes align (WatchLo spec) */
    addr = addr & 0x1FFFfff8;

    {
        u32 expectwlo = set == 0 ? 0 : (addr | (type - 1)); /* hack: ((2..4=w,r,a) - 1) is corresponds to WatchLo[0:1]=R|W */
        u32 wlo;
        asm("mfc0 %0, $" STR(C0_WATCHLO) : "=r"(wlo));

        /* TODO: when set==0, check WatchLo is exactly to-be-unset and return error if not? */
        if(set && (wlo & 3) && (wlo != expectwlo))
        {
            /* already set on other address... return error. (empty="not supported" cause "Protocol error: Z2 (write-watchpoint) conflicting enabled responses" on GDB...) */
            return STUBERR_TOOMANY;
        }

        asm("mtc0 %0, $" STR(C0_WATCHLO) : : "r"(expectwlo));
        asm("mtc0 $0, $" STR(C0_WATCHHI)); /* PA[35:32] always zero at this implementation */
    }

    pkt_put_str("OK");
    pkt_send();

    return STUB_SUCCESS;
}

static COLD stubstatus_t cmd_stopreason(void)
{
    u64 *regptr;
    __OSThreadContext *c = &DebugThread->context;

    if (!DebugThread)
    {
        pkt_put_str("OK");
        pkt_send();
        return STUB_SUCCESS;
    }

    if (DebugThread == RSP_THREAD)
    {
        pkt_put_str("T05thread:000003e8;swbreak:;");
        // TODO - RSP registers
        pkt_send();
        return STUB_SUCCESS;
    }

    pkt_put_str("T05thread:");
    pkt_put_hex(&DebugThread->id, sizeof(OSId));
    pkt_putc(';');
    switch (c->cause & CAUSE_EXCMASK)
    {
        case EXC_BREAK:
            pkt_put_str("swbreak:;");
            break;
        case EXC_WATCH:
            {
                void *addr;
                asm("mfc0 %0, $" STR(C0_WATCHLO) : "=r"(addr));
                pkt_put_str("watch:");
                pkt_put_hex(&addr, sizeof addr);
                pkt_putc(';');
            }
            break;
    }
    TmpRegister = 0;
    for (u8 reg = 1; reg < REGS_SIZE; reg++)
    {
        regptr = os_thread_reg_ptr(c, reg);
        if (reg == REGS_FPIR)
            asm("cfc1 %0, $0" : "=r" (TmpRegister));

        if (pkt_put_hex(&reg, sizeof reg) < 0)
            return STUBERR_OVERFLOW;
        pkt_putc(':');
        if (pkt_put_hex(regptr, sizeof *regptr) < 0)
            return STUBERR_OVERFLOW;
        pkt_putc(';');
    }
    pkt_send();
    return STUB_SUCCESS;
}

static COLD void cmd_threadinfo(void)
{
    OSThread *thread = NULL;

    if (InfoThread > 0)
        thread = I_GetThreadById(InfoThread);

    if (!thread)
    {
        pkt_putc('l');
        pkt_send();
        return;
    }
    pkt_putc('m');
    if (InfoThread < 0)
    {
        OSId tid = RSP_TID;
        pkt_put_hex(&tid, sizeof tid);
        pkt_putc(',');
    }
    pkt_put_hex(&thread->id, sizeof(OSId));
    thread = thread->tlnext;

    while (thread && DebugTxPos < sizeof DebugTx - 12)
    {
        pkt_putc(',');
        pkt_put_hex(&thread->id, sizeof(OSId));
        thread = thread->tlnext;
    }

    InfoThread = thread ? thread->id : -1;
}

static COLD stubstatus_t cmd_threadextrainfo(void)
{
    OSId id;
    OSThread *thread;
    char desc[72];
    u32 descsize = 0;

    if (!pkt_acceptc(','))
        return STUBERR_PARSE;
    id = pkt_get_threadid();
    if (id < -1)
        return STUBERR_PARSE;

    if (id == RSP_TID) {
        pkt_put_str("RSP");
        pkt_send();

        return STUB_SUCCESS;
    }

    thread = I_GetThreadById(id);

    if (!thread)
        return STUBERR_THREADNOTFOUND;

    descsize += sprintf(desc, "Priority %d", thread->priority);
    if (thread->fp)
        descsize += sprintf(desc, ", FP");
    if (thread->state & OS_STATE_STOPPED)
        descsize += sprintf(desc, ", Stopped");
    if (thread->state & OS_STATE_RUNNABLE)
        descsize += sprintf(desc, ", Runnable");
    if (thread->state & OS_STATE_RUNNING)
        descsize += sprintf(desc, ", Running");
    if (thread->state & OS_STATE_WAITING)
        descsize += sprintf(desc, ", Waiting");
    if (thread->flags & OS_FLAG_CPU_BREAK)
        descsize += sprintf(desc, ", Break");
    if (thread->flags & OS_FLAG_FAULT)
        descsize += sprintf(desc, ", Faulted");
    pkt_put_hex(desc, descsize);
    pkt_send();

    return STUB_SUCCESS;
}

static COLD stubstatus_t cmd_threadalive(void)
{
    OSId id = 0;
    OSThread *thread;

    id = pkt_get_threadid();
    if (id < -1)
        return STUBERR_PARSE;

    if (id == RSP_TID)
    {
        // TODO
    }
    else
    {
        thread = I_GetThreadById(id);

        if (!thread)
            return STUBERR_THREADNOTFOUND;
    }

    pkt_put_str("OK");
    pkt_send();

    return STUB_SUCCESS;
}

static COLD stubstatus_t cmd_setthread()
{
    OSId id = 0;
    OSThread *thread;

    id = pkt_get_threadid();
    if (id < -1)
        return STUBERR_PARSE;

    if (id == RSP_TID)
    {
        DebugThread = RSP_THREAD;
    }
    else
    {
        thread = I_GetThreadById(id);

        if (!thread)
            return STUBERR_THREADNOTFOUND;

        DebugThread = thread;
    }

    pkt_put_str("OK");
    pkt_send();

    return STUB_SUCCESS;
}

static COLD void I_ResumeThread(OSThread *thread)
{
    if (thread == RSP_THREAD)
    {
        // TODO
    }
    else if (thread)
    {
        osStartThread(thread);
    }
}

static COLD void I_StopThread(OSThread *thread)
{
    if (thread == RSP_THREAD)
    {
        // TODO
    }
    else if (thread)
    {
        osStopThread(thread);
    }
}

static COLD stubstatus_t cmd_vcont(void)
{
    if (pkt_is_eof())
        return STUBERR_PARSE;

    while(!pkt_is_eof())
    {
        int action;
        OSId tid = -1;
        OSThread *thread;

        if (!pkt_acceptc(';'))
            return STUBERR_PARSE;
        action = pkt_getc();
        if (action < 0)
            return STUBERR_PARSE;
        if (pkt_acceptc(':'))
        {
            tid = pkt_get_threadid();
            if (tid < -1)
                return STUBERR_PARSE;
            thread = I_GetThreadById(tid);
        }
        switch (action)
        {
        case 'c':
            if (tid == -1)
            {
                I_ResumeThread(RSP_THREAD);
                I_ForEachThread(osStartThread);
            }
            else
            {
                I_ResumeThread(thread);
            }
            break;
        case 's':
            if (tid != -1)
                cmd_step(thread);
            break;
        case 't':
            if (tid == -1)
                I_StopAppThreads();
            else
                I_StopThread(thread);

            break;
        }
        pkt_send();
    }

    return STUB_SUCCESS;
}

static COLD void cmd_ctrlc(void)
{

    DebugThread = __osGetActiveQueue();
    while (DebugThread->priority <= OS_PRIORITY_IDLE || DebugThread->priority > OS_PRIORITY_APPMAX)
        DebugThread = DebugThread->tlnext;

    if (NonStop)
        I_StopThread(DebugThread);
    else
        I_StopAppThreads();

    cmd_stopreason();
}

static COLD void I_ParseGDBPacket(void)
{
    u32 i;
    u32 len = I_CmdNextTokenSize();

    DebugTxPos = 2;
    if (len > sizeof DebugRx)
    {
        /* packet too large */
        pkt_put_str("E01");
        pkt_send();
        return;
    }
    else
    {
        I_CmdGetNextToken(DebugRx);
    }

    for(i = 0; i < len; i++) {
        u32 realsum;
        stubstatus_t error;

        DebugTxPos = 2;

        if(DebugRx[i] == '\03')
        {
            cmd_ctrlc();
            continue;
        }

        if(DebugRx[i] != '$') {
            /* not start of GDB remote packet */
            /* TODO ignoreing '+' is ok, but '-' is not!! */
            continue;
        }

        /* found packet head */
        i++; if(len <= i) goto pkt_error;

        DebugRxPos = i;

        realsum = 0;
        for(;;) {
            if(DebugRx[i] == '#') {
                break;
            }
            realsum += DebugRx[i];
            i++; if(len <= i) goto pkt_error;
        }

        DebugRxEnd = i;

        /* skip '#' */
        i++; if(len <= i) goto pkt_error;
        {
            s32 expectsum;
            /* read expect cksum */
            expectsum = hex2int(DebugRx[i]) << 4;
            i++; if(len <= i) goto pkt_error;
            expectsum |= hex2int(DebugRx[i]);

            /* error if checksum mismatch (inclding malformed expect cksum) */
            if((realsum & 0xFF) != expectsum) {
                goto pkt_error;
            }
        }

        /* here, we can assume whole command fits in rxbuf. */

        /* process command */
        error = 0;
        switch(DebugRx[DebugRxPos++]) {
        case 'q': /* query */
            if (pkt_accept_str("C") && pkt_is_eof())
            {
                OSId tid = DebugThread == RSP_THREAD ? RSP_TID : DebugThread->id;

                pkt_put_str("QC");
                pkt_put_hex(&tid, sizeof tid);
                pkt_send();
            }
            else if (pkt_accept_str("Supported"))
            {
                u32 size = sizeof DebugTx;
                pkt_put_str("swbreak+;vContSupported+;QNonStop+;PacketSize=");
                pkt_put_hex(&size, sizeof size);
                pkt_send();
            }
            else if (pkt_accept_str("fThreadInfo") && pkt_is_eof())
            {
                InfoThread = -1;
                cmd_threadinfo();
            }
            else if (pkt_accept_str("qsThreadInfo") && pkt_is_eof())
            {
                cmd_threadinfo();
            }
            else if (pkt_accept_str("qThreadExtraInfo"))
            {
                error = cmd_threadextrainfo();
            }
            else
                goto pkt_unsupported;
            break;
        case 'v':
            if (pkt_accept_str("CtrlC"))
            {
                cmd_ctrlc();
            }
            else if (pkt_accept_str("Cont"))
            {
                if (pkt_acceptc('?'))
                {
                    if (!pkt_is_eof())
                        goto pkt_unsupported;
                    pkt_put_str("vCont;c;s;t");
                    pkt_send();
                }
                else
                    cmd_vcont();
            }
            else
                goto pkt_unsupported;
            break;
        case 'Q':
            if (pkt_accept_str("NonStop:1"))
            {
                NonStop = true;
                pkt_put_str("OK");
                pkt_send();
            }
            else if (pkt_accept_str("NonStop:0"))
            {
                NonStop = false;
                pkt_put_str("OK");
                pkt_send();
            }
            else
                goto pkt_unsupported;
            break;
        case 'k': /* soft reset equivalent */
            __osDisableInt();
            asm("j __start");
            break;
        case 'H':
            if (pkt_acceptc('g'))
                error = cmd_setthread();
            else
                goto pkt_unsupported;
            break;
        case 'T':
            error = cmd_threadalive();
            break;
        case 'g': /* register target2host */
            error = cmd_getregs();
            break;
        case 'G': /* register host2target */
            error = cmd_setregs();
            break;
        case 'm': /* memory target2host */
            error = cmd_getmemory();
            break;
        case 'M': /* memory host2target */
            error = cmd_setmemory();
            break;
        case '?': /* last signal */
            error = cmd_stopreason();
            break;
        case 's': /* step */
            cmd_step(DebugThread);
            DebugTx[0] = '+';
            pkt_send_raw(512, 1);
            I_ResumeThread(DebugThread);
            return;
        case 'c': /* continue */
            /* c[ADDR] */
            if(!pkt_is_eof())
            {
                s64 addr = pkt_get_hex_delim(0);
                if (addr < 0)
                    error = STUBERR_PARSE;
                else if (DebugThread == RSP_THREAD)
                    /* TODO */ (void)0;
                else if (DebugThread)
                    DebugThread->context.pc = addr;
            }

            /* nothing to do */

            DebugTx[0] = '+';
            pkt_send_raw(512, 1);
            I_ResumeThread(DebugThread);
            DebugThread = NULL;
            return;
        case 'Z': /* insert watch */ /*FALLTHROUGH*/
        case 'z': /* remove watch */
            DebugRxPos--;
            error = cmd_watch();
            break;
        pkt_unsupported:
        default:
            /* unknown command... */
            pkt_send();
        }

        if(error != 0) {
            pkt_putc('E');
            pkt_put_hex(&error, sizeof error);
            pkt_send();
        }

        continue;
pkt_error:
        /* return NAK */
        DebugTx[0] = '-'; /* NAK */
        pkt_send_raw(512, 1);
        continue; /* if not invalid sum, should be break. if it is, should be continue. */
    }
}

static COLD void NO_RETURN I_DebuggerThread(void *arg)
{
    SET_GP();

    OSMesg msg;

    while (1)
    {
        osRecvMesg(&debugMessageQ, &msg, OS_MESG_BLOCK);

        switch ((int) msg)
        {
        case MSG_FAULT:
        case MSG_CPU_BREAK:
        case MSG_SP_BREAK:
            bprestore();

            // usb thread still running
            WritePacket = I_USBSendRDB;
            DebugTxPos = 2;
            DebugThread = I_FaultedThread((int) msg);

            if (NonStop)
                I_StopThread(DebugThread);
            else
                I_StopAppThreads();

            switch ((int) msg)
            {
            case MSG_FAULT:
                if (DebugThread)
                    D_printf("Fault in thread %ld at %08lx", DebugThread->id, DebugThread->context.pc);
                break;
            case MSG_CPU_BREAK:
                if (DebugThread)
                    D_printf("Break in thread %ld at %08lx", DebugThread->id, DebugThread->context.pc);
                break;
            case MSG_SP_BREAK:
                D_printf("RSP Break");
                break;
            }

            cmd_stopreason();

            break;
        case MSG_GDB_PACKET:
            // debug thread takes control of usb buffer
            WritePacket = I_USBWriteRDB;
            while (1)
            {
                int size = I_CmdNextTokenSize();
                if (size == 0)
                {
                    I_CmdSkipAllTokens();
                    break;
                }
                I_ParseGDBPacket();
            }
            osSendMesg(&gdbDoneQ, NULL, OS_MESG_NOBLOCK);
            break;
        }

        I_CheckStack(debugThreadStack, "Debug");
    }
}

void COLD I_TakeRDBPacket(void)
{
    osSendMesg(&debugMessageQ, (OSMesg) MSG_GDB_PACKET, OS_MESG_NOBLOCK);
    osRecvMesg(&gdbDoneQ, NULL, OS_MESG_BLOCK);
}

static COLD void I_ShowDebugScreen(const char *text)
{
    void *cfb = CFB0_ADDR();

    bzero(cfb,  CFB_SIZE);
    blit32_TextExplicit(cfb, 0xffff, 1, XResolution, YResolution, blit_Clip, 32, 24, text);
    osViSwapBuffer(cfb);
    __osViSwapContext();
}

#endif /* USB_GDB */

#endif /* !defined(NDEBUG) */
