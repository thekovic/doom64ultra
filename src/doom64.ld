OUTPUT_ARCH (mips)

/* Starts a segment */
#define BEGIN_SEG(name, addr) \
    _##name##SegmentStart = ADDR(.name); \
    _##name##SegmentRomStart = __romPos; \
    .name addr : AT(__romPos)

/* Ends a segment */
#define END_SEG(name) \
    _##name##SegmentEnd = ALIGN(ADDR(.name) + SIZEOF(.name),0x10); \
    _##name##SegmentRomEnd = _##name##SegmentEnd - _##name##SegmentStart + _##name##SegmentRomStart; \
    __romPos = _##name##SegmentRomEnd; \
    . = _##name##SegmentEnd;

/* Starts a noload segment which is intended to contain variables which do not have data yet. */
#define BEGIN_NOLOAD(name) \
    _##name##SegmentBssStart = ADDR(.name.noload); \
    .name.noload (NOLOAD) :

/* Ends a noload segment */
#define END_NOLOAD(name) \
    _##name##SegmentBssEnd = ALIGN(ADDR(.name.noload) + SIZEOF(.name.noload),0x10); \
    _##name##SegmentBssSize = _##name##SegmentBssEnd - _##name##SegmentBssStart; \
    . = _##name##SegmentBssEnd;

SECTIONS
{
   __romPos = 0;

   _RomStart = 0x1000;
   _RomSize = _RomStart;

   BEGIN_SEG(boot, 0x04000000)
   {
      KEEP(BUILD_DIR/src/asm/rom_header.o(.text)); /* ROM Header */
      KEEP(BUILD_DIR/boot.6102.o(.data)); /* CIC 6102 bootcode */
   }
   END_SEG(boot)

   _RomSize = (0x80000450 - 0x80000400) + _RomStart;

   BEGIN_SEG(code, 0x80000400) SUBALIGN(16)
   {
      KEEP(BUILD_DIR/src/asm/entry.o(.start));
      _gp = ALIGN(16) + 0x8000;
      :*.o(.sdata);
      :*.o(.text.hot);
      */libdoom64.a:*.o(.text.hot);
      *.ltrans.o(.text);
      */libdoom64.a:*.o(.text);
      */LIBULTRA.a:*.o(.text);
      */libgcc.a:*.o(.text);
      :*.o(.text.game);
      :*.o(.text.menu);
      :*.o(.text.startup);
      :*.o(.text.unlikely);

      /* data */
      *.ltrans.o(.data*);
      */libdoom64.a:*.o(.data*);

      */LIBULTRA.a:*.o(.data*);

      /usr/lib/n64/PR/rspboot.o(.data*);
      /usr/lib/n64/PR/gspL3DEX2.fifo.o(.data*);
      /usr/lib/n64/PR/gspF3DEX2.NoN.fifo.o(.data*);
      /usr/lib/n64/PR/aspMain.o(.data*);

      /* rodata */
      *.ltrans.o(.rodata*);
      */libdoom64.a:*.o(.*rodata*);

      */LIBULTRA.a:*.o(.*rodata*);
      */libgcc.a:*.o(.*rodata*);
      */libgcc.a:*.o(.eh_frame*);

      /usr/lib/n64/PR/rspboot.o(.text);
      /usr/lib/n64/PR/gspL3DEX2.fifo.o(.text);
      /usr/lib/n64/PR/gspF3DEX2.NoN.fifo.o(.text);
      /usr/lib/n64/PR/aspMain.o(.text);
   }
   END_SEG(code)

   _RomSize += (_codeSegmentEnd - _codeSegmentStart);

   BEGIN_NOLOAD(code)
   {
      *.ltrans.o(.*bss*);
      */libdoom64.a:*.o(.scommon);
      */libdoom64.a:*.o(.*bss*);

      */LIBULTRA.a:*.o(COMMON);
      */LIBULTRA.a:*.o(.scommon);
      */LIBULTRA.a:*.o(.*bss*);
   }
   END_NOLOAD(code)

   _bssSegmentEnd = .;

   BEGIN_SEG(doom64_wad,_RomSize) {
      KEEP(BUILD_DIR/DOOM64.WAD.o)
   }
   END_SEG(doom64_wad)
   _RomSize += SIZEOF(.doom64_wad);

   BEGIN_SEG(doom64_wmd,_RomSize) {
      KEEP(BUILD_DIR/DOOM64.WMD.o)
   }
   END_SEG(doom64_wmd)
   _RomSize += SIZEOF(.doom64_wmd);

   BEGIN_SEG(doom64_wsd,_RomSize) {
      KEEP(BUILD_DIR/DOOM64.WSD.o)
   }
   END_SEG(doom64_wsd)
   _RomSize += SIZEOF(.doom64_wsd);

   BEGIN_SEG(doom64_wdd,_RomSize) {
      KEEP(BUILD_DIR/DOOM64.WDD.o)
   }
   END_SEG(doom64_wdd)

   /DISCARD/ :
   {
      *crti.o;
      *crtbegin.o;
      *crtend.o;
      *crtn.o;
   }
#ifdef NDEBUG
   /* Discard everything not specifically mentioned above. */
   /DISCARD/ :
   {
      *(*);
   }
#endif
}

