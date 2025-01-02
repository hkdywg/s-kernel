/*                                                                                                                                                                     
 *  vector.S
 *
 *  brif
 *      system vectors, used by interrupt
 *  
 *  (C) 2025.01.02 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */

.text

.global set_current_vbar
.global system_vectors
.global vector_error
.global vector_irq
.global vector_frq

system_vectors:
.align 11
	.set 	vbar, system_vectors
	.org 	vbar
	/* Exception form currentEL (EL1) with SP_EL0 */
	.org (vbar + 0x00 + 0)
	b vector_error				/* Synchronous */
	.org (vbar + 0x80 + 0)
	b vector_irq				/* IRQ/vIRQ */
	.org (vbar + 0x100 + 0)
	b vector_frq				/* FIQ/vFIQ */
	.org (vbar + 0x180 + 0)
	b vector_error				/* Error/vError */


set_current_vbar:
	ldr x0, =system_vectors
	msr vbar_el1, x0
	ret



