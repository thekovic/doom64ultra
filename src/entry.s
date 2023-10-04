# assembler directives
.set noat      # allow manual use of $at
.set noreorder # don't insert nops after branches

#include <asm.h>
#include <PR/R4300.h>
#include <PR/ultratypes.h>
#include <regdef.h>

.include "macros.inc"

.section .start, "ax"

glabel __start
    la      gp, _gp
    la      t0, _bss_start
    la      t1, _bss_size
bss_clear:
    addi    t1, t1, -8
#undef gp
.set gp=64; \
    sd      zero, 0(t0)
.set gp=32; \
    bnez    t1, bss_clear
    addi    t0, t0, 8
    la      t2, I_Start #Boot function address
    la      sp, bootStack+0x100 #Setup boot stack pointer, change stack size if needed here
    jr      t2
    nop
