#include <asm.h>
#include <PR/R4300.h>
#include <PR/ultratypes.h>

#define zero	$0
#define at	$1
#define v0	$2
#define v1	$3
#define a0	$4
#define a1	$5
#define a2	$6
#define a3	$7
#define	t0	$8
#define	t1	$9
#define	t2	$10
#define	t3	$11
#define	t4	$12
#define	t5	$13
#define	t6	$14
#define	t7	$15
#define s0	$16
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define t8	$24
#define t9	$25
#define k0	$26
#define k1	$27
#define gp	$28
#define sp	$29
#define fp	$30
#define ra	$31
#define return	s7

    .text
	.set noreorder

/*------------------------------------------*/
/* fixed_t	FixedMul (fixed_t a, fixed_t b) */
/*------------------------------------------*/

.globl FixedMul
.ent FixedMul

FixedMul:   /* 800044D0 */

    dmult   $4, $5
    mflo    $2
    jr      $31
    dsra    $2, $2, 16

.end FixedMul

/*-------------------------------------------*/
/* fixed_t	FixedDiv2 (fixed_t a, fixed_t b) */
/*-------------------------------------------*/

.global FixedDiv5
.ent FixedDiv5
.set noat

FixedDiv5:
                xor     $8, $4, $5
                bgtz    $4, loc_8003EF00
                move    $2, $4
                neg     $2, $4

loc_8003EF00:
                bgtz    $5, loc_8003EF0C
                move    $3, $5
                neg     $3, $5

loc_8003EF0C:
                lui     $6, 1
                sltu    $1, $3, $2
                beqz    $1, loc_8003EF30
                and     $9, $0, $6

loc_8003EF1C:
                sll     $3, 1
                sll     $6, 1
                sltu    $1, $3, $2
                bnez    $1, loc_8003EF1C
                and     $10, $0, $6

loc_8003EF30:

                slt     $1, $2, $3
                bnez    $1, loc_8003EF44
                nop
                sub     $2, $3
                or      $10, $6

loc_8003EF44:
                sll     $2, 1
                srl     $6, 1
                beqz    $6, loc_8003EF5C
                nop
                bnez    $2, loc_8003EF30
                nop

loc_8003EF5C:
                bgez    $8, locret_8003EF68
                move    $2, $10
                neg     $2, $10

locret_8003EF68:
                jr      $31
                nop
.set at
.end FixedDiv5

.globl FixedDiv4
.ent FixedDiv4

FixedDiv4:   /* 800044E4 */

    dsll    $4, $4, 16
    ddiv    $4, $5
    mflo    $2
    jr      $31
    nop
.end FixedDiv4