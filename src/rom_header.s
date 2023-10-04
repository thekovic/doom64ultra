# assembler directives
/*
 * ROM header
 * Only the first 0x18 bytes matter to the console.
 */

 .section .rodata

.byte  0x80, 0x37, 0x12, 0x40   /* PI BSD Domain 1 register */
.word  0x0000000F               /* Clockrate setting*/
.word  __start                  /* Entrypoint */

/* SDK Revision */
.word  0x0000144C

.word  0x00000000               /* Checksum 1 (OVERWRITTEN BY MAKEMASK)*/
.word  0x00000000               /* Checksum 2 (OVERWRITTEN BY MAKEMASK)*/
.word  0x00000000               /* Unknown */
.word  0x00000000               /* Unknown */
.ascii "DOOM 64 ULTRA       "   /* Internal ROM name (Max 20 characters) */
/* Homebrew header */
.byte  0x01, 0x01, 0x01, 0x01   /* Controller info */
.byte  0x00, 0x00, 0x00         /* Empty */
.ascii "NEDA"                   /* Category, Game ID, Country Code */
.byte  0x61                     /* Savetype */
