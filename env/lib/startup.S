/*
 * startup.S
 */
#include <uspienv/sysconfig.h>

	.text

	.globl	_start
_start:
	cps	#0x1F				/* set system mode */
	mov	sp, #MEM_KERNEL_STACK
	b	sysinit

#if RASPPI != 1
	
	.globl	_start_secondary
_start_secondary:
	dsb
1:	wfi
	b	1b

#endif

/* End */
