#define SCREEN_WD 320
extern unsigned short SCREEN_HT;

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

#define BASEPROG_SIZE (ALIGN(_bss_end, 16) - (u32)BASE_RAM_ADDR)
#define CFB_SIZE (osMemSize >= 0x800000 ? SCREEN_WD*SCREEN_HT*2*sizeof(u32) : SCREEN_WD*SCREEN_HT*sizeof(u16))
#define CFBS_SIZE (CFB_SIZE*2)
#define AUDIO_HEAP_SIZE	ALIGN( \
        AUDIO_HEAP_BASE_SIZE \
        + ALIGN(_doom64_wmdSegmentRomEnd - _doom64_wmdSegmentRomStart, 16) \
        + ALIGN(_doom64_wsdSegmentRomEnd - _doom64_wsdSegmentRomStart, 16) \
    , 64)
#define MAIN_HEAP_SIZE (osMemSize - BASEPROG_SIZE - CFBS_SIZE - AUDIO_HEAP_SIZE)

#define MAIN_HEAP_ADDR ((byte*)ALIGN(_bss_end, 16))
#define CFBS_ADDR (BASE_RAM_END - CFBS_SIZE)
#define AUDIO_HEAP_ADDR (CFBS_ADDR - AUDIO_HEAP_SIZE)

extern unsigned char *cfb;

#define CFB(i) ((i) ? &cfb[CFB_SIZE] : &cfb[0])
#define CFB_SPADDR (OS_K0_TO_PHYSICAL(CFB(vid_side)))

