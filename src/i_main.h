#pragma once

#define SCREEN_WD 320
extern unsigned short SCREEN_HT;
extern unsigned long CFB_SIZE;

#ifdef FORCE_NO_EXPANSION_PAK
#define MEM_SIZE() 0x400000
#else
#define MEM_SIZE() osMemSize
#endif

#define HAS_EXPANSION_PAK() (MEM_SIZE() >= 0x800000)

#define BASE_RAM_ADDR ((byte*)K0BASE)
#define BASE_RAM_END() (BASE_RAM_ADDR + MEM_SIZE())

extern char _doom64_wadSegmentRomStart[], _doom64_wadSegmentRomEnd[];
extern char _doom64_wmdSegmentRomStart[], _doom64_wmdSegmentRomEnd[];
extern char _doom64_wsdSegmentRomStart[], _doom64_wsdSegmentRomEnd[];
extern char _doom64_wddSegmentRomStart[], _doom64_wddSegmentRomEnd[];

extern char _bss_end[];

/*
 * setup addresses for the memory layout:
 *
 * 0mb            1mb             2mb             3mb             4mb
 * +---------------+---------------+---------------+---------------+
 * | code |                 | cfb0 | aheap |                | cfb1 |
 * +---------------+---------------+---------------+---------------+
 * |      | <------------------- mheap -------------------> |      |
 * +---------------+---------------+---------------+---------------+
 */

#define CFB0_ADDR() ((byte*)(BASE_RAM_ADDR + (MEM_SIZE()>>1)))
#define CFB1_ADDR() ((byte*)(BASE_RAM_END() - CFB_SIZE))
#define AUDIO_HEAP_END() (CFB0_ADDR())
#define AUDIO_HEAP_MAX_SIZE() (0x100000)
#define MAIN_HEAP_START ((byte*)ALIGN(_bss_end, 32))
#define MAIN_HEAP_END() CFB1_ADDR()

#define CFB(i) ((i) ? CFB1_ADDR() : CFB0_ADDR())
#define CFB_SPADDR() (OS_K0_TO_PHYSICAL(CFB(vid_side)))

#define STACK_GUARD 0xed8c82721e8025b2ULL // random value

