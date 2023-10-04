.set noreorder
.option pic0

#include <asm.h>
#include <PR/R4300.h>
#include <PR/ultratypes.h>
#include <regdef.h>

.section .text.hot, "ax"

/* Original Code From Mortal Kombat Trilogy N64 */

/*------------------------------------------*/
/* unsigned long wesssys_disable_ints(void) */
/*------------------------------------------*/

.globl wesssys_disable_ints
.ent wesssys_disable_ints

wesssys_disable_ints:   /* 8003A3E0 */

    mfc0    $8, $12     /* t0 = special register with IE bit */
    and     $9, $8, ~1  /* t1 = same register with IE bit cleared */
    mtc0    $9, $12     /* disable R4300 CPU interrupts */
    andi    $2, $8, 1   /* return the prior state of just the IE bit */
    nop
    jr      $31
    nop

.end wesssys_disable_ints

/*------------------------------------------------*/
/* void wesssys_restore_ints(unsigned long state) */
/*------------------------------------------------*/

.globl wesssys_restore_ints
.ent wesssys_restore_ints

wesssys_restore_ints:   /* 8003A400 */

    mfc0	$8,$12      /* t0 = special register with IE bit */
	nop
	or      $8, $4      /* restore IE bit from passed-in parameter */
	mtc0	$8, $12     /* restore R4300 CPU interrupts */
	nop
	nop
	j		$31         /* return nothing */
	nop

.end wesssys_restore_ints
