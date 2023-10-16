OUTPUT_ARCH (mips)

MEMORY {
    rom (R) : ORIGIN = 0, LENGTH = 64M
    ram (RWX) : ORIGIN = 0x80000400, LENGTH = 4M - 0x400
}

#define ADD_DATA(name, obj) \
    .name : ALIGN(16) { \
        _##name##SegmentRomStart = .; \
        KEEP(obj); \
        _##name##SegmentRomEnd = ALIGN(ADDR(.name) + SIZEOF(.name), 16); \
    } AT>rom

SECTIONS
{
    .boot :
    {
        KEEP(BUILD_DIR/src/rom_header.o(.rodata)); /* ROM Header */
        KEEP(BUILD_DIR/boot.6102.o(.text)); /* CIC 6102 bootcode */
        . = 0x1000;
    } AT>rom

    .code : SUBALIGN(16)
    {
        KEEP(BUILD_DIR/src/entry.o(.start));
        _gp = ALIGN(16) + 0x8000;
        *.o(.sdata);
        :*.o(.text.hot);
        */libultra*.a:exceptasm.o(.text);
        */libdoom64.a:*.o(.text.hot);
        *.ltrans.o(.text);
        */libdoom64.a:*.o(.text);
        */LIBULTRA.a:*.o(.text);
        */LIBNAUDIO.a:*.o(.text);
        */libgcc.a:*.o(.text);
        *.o(.text.game);
        *.o(.text.menu);
        *.o(.text.startup);
        :*.o(.text.unlikely);

        /* data */
        *.ltrans.o(.data*);
        */libdoom64.a:*.o(.data*);

        */LIBULTRA.a:*.o(.data*);
        */LIBNAUDIO.a:*.o(.data*);

        /usr/lib/n64/PR/rspboot.o(.data*);
        /usr/lib/n64/PR/gspL3DEX2.fifo.o(.data*);
        /usr/lib/n64/PR/gspF3DEX2.NoN.fifo.o(.data*);
        /usr/lib/n64/PR/n_aspMain.o(.data*);

        /* rodata */
        *.ltrans.o(.rodata*);
        */libdoom64.a:*.o(.*rodata*);

        */LIBULTRA.a:*.o(.*rodata*);
        */LIBNAUDIO.a:*.o(.*rodata*);
        */libgcc.a:*.o(.*rodata*);
        */libgcc.a:*.o(.eh_frame*);

        /usr/lib/n64/PR/rspboot.o(.text);
        /usr/lib/n64/PR/gspL3DEX2.fifo.o(.text);
        /usr/lib/n64/PR/gspF3DEX2.NoN.fifo.o(.text);
        /usr/lib/n64/PR/n_aspMain.o(.text);
    } >ram AT>rom

    .code.noload (NOLOAD) : ALIGN(16)
    {
        *.ltrans.o(.*bss*);
        */libdoom64.a:*.o(.scommon);
        */libdoom64.a:*.o(.*bss*);

        */LIBULTRA.a:*.o(COMMON);
        */LIBULTRA.a:*.o(.scommon);
        */LIBULTRA.a:*.o(.*bss*);
        */LIBNAUDIO.a:*.o(COMMON);
        */LIBNAUDIO.a:*.o(.scommon);
        */LIBNAUDIO.a:*.o(.*bss*);
    } >ram

    _bss_start = ADDR(.code.noload);
    _bss_size = ALIGN(SIZEOF(.code.noload), 16);
    _bss_end = _bss_start + _bss_size;

    ADD_DATA(doom64_wad, BUILD_DIR/DOOM64.WAD.o)
    ADD_DATA(doom64_wmd, BUILD_DIR/DOOM64.WMD.o)
    ADD_DATA(doom64_wsd, BUILD_DIR/DOOM64.WSD.o)
    ADD_DATA(doom64_wdd, BUILD_DIR/DOOM64.WDD.o)

    /DISCARD/ :
    {
        BUILD_DIR/src/entry.o(.text);
        *(.MIPS.abiflags);
        *(.reginfo);
    }
#ifdef NDEBUG
    /* Discard everything not specifically mentioned above. */
    /DISCARD/ :
    {
        *(*);
    }
#endif
}

