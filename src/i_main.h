#pragma once

#define SCREEN_WD 320
extern unsigned short SCREEN_HT;
extern unsigned long CFB_SIZE;

#define BASE_RAM_ADDR ((byte*)K0BASE)
#define BASE_RAM_END (BASE_RAM_ADDR + osMemSize)

extern char _doom64_wadSegmentRomStart[], _doom64_wadSegmentRomEnd[];
extern char _doom64_wmdSegmentRomStart[], _doom64_wmdSegmentRomEnd[];
extern char _doom64_wsdSegmentRomStart[], _doom64_wsdSegmentRomEnd[];
extern char _doom64_wddSegmentRomStart[], _doom64_wddSegmentRomEnd[];

extern char _bss_end[];

#ifdef NDEBUG
#define AUDIO_HEAP_BASE_SIZE 0x25000
#else
// alHeapDBAlloc stores extra debug info
#define AUDIO_HEAP_BASE_SIZE 0x26000
#endif

/*
 * setup addresses for the memory layout:
 *
 * 0mb            1mb             2mb             3mb             4mb
 * +---------------+---------------+---------------+---------------+
 * | code |                | aheap | cfb0 |                 | cfb1 |
 * +---------------+---------------+---------------+---------------+
 * |      | <------------------- mheap -------------------> |      |
 * +---------------+---------------+---------------+---------------+
 */

#define BASEPROG_SIZE (ALIGN(_bss_end, 32) - (u32)BASE_RAM_ADDR)
#define AUDIO_HEAP_SIZE ALIGN( \
        AUDIO_HEAP_BASE_SIZE \
        + ALIGN(_doom64_wmdSegmentRomEnd - _doom64_wmdSegmentRomStart, 16) \
        + ALIGN(_doom64_wsdSegmentRomEnd - _doom64_wsdSegmentRomStart, 16) \
    , 64)
#define MAIN_HEAP_SIZE (osMemSize - BASEPROG_SIZE - CFB_SIZE)

#define MAIN_HEAP_ADDR ((byte*)ALIGN(_bss_end, 32))
#define CFB0_ADDR ((byte*)(BASE_RAM_ADDR + (osMemSize>>1)))
#define CFB1_ADDR ((byte*)(BASE_RAM_END - CFB_SIZE))
#define AUDIO_HEAP_ADDR (CFB0_ADDR - AUDIO_HEAP_SIZE)

#define CFB(i) ((i) ? CFB1_ADDR : CFB0_ADDR)
#define CFB_SPADDR (OS_K0_TO_PHYSICAL(CFB(vid_side)))

#define STACK_GUARD 0xed8c82721e8025b2ULL // random value

