/*
 * exceptionstub.S
 *
 * USPi - An USB driver for Raspberry Pi written in C
 * Copyright (C) 2014  R. Stange <rsta2@o2online.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <uspienv/sysconfig.h>

/*
 * IRQ stub
 */
	.globl	IRQStub
IRQStub:
	mov	sp, #MEM_IRQ_STACK
	sub	lr, lr, #4			/* lr: return address */
	stmfd	sp!, {r0-r12, lr}		/* save r0-r12 and return address */
	bl	InterruptHandler
	ldmfd	sp!, {r0-r12, pc}^		/* restore registers and return */

/* End */
