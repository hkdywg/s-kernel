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

#include <context_macro.h>

.text

.global set_current_vbar
.global system_vectors
.global vector_error
.global vector_irq
.global vector_fiq


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
	b vector_fiq				/* FIQ/vFIQ */
	.org (vbar + 0x180 + 0)
	b vector_error				/* Error/vError */


set_current_vbar:
	ldr x0, =system_vectors
	msr vbar_el1, x0
	ret

	.align 8
vector_fiq:
	save_context
	stp 	x0, x1, [sp, #-0x10]!
	bl 		sk_hw_trap_fiq
	ldp 	x0, x1, [sp], #0x10
	restore_context


	.align 8
vector_irq:
	save_context
	stp 	x0, x1, [sp, #-0x10]!

	bl 		sk_interrupt_enter
	bl 		sk_hw_trap_irq
	bl 		sk_interrupt_leave

	ldp 	x0, x1, [sp], #0x10
	adr 	x1, thread_switch_interrupt_flag
	ldr 	x2, [x1]
	cmp 	x2, #1
	b.ne 	vector_irq_exit

	mov 	x2, #0 		/* clear flag */
	str 	x2, [x1]

	adr 	x3, interrupt_from_thread
	ldr 	x4, [x3]
	str 	x0, [x4]	/* store sp in preempted task's TCB */
						/* assume x0 stores the stack pinter SP of the current thread */

	adr 	x3, interrupt_to_thread
	ldr 	x4, [x3]
	ldr 	x0, [x4]	/* get new task's pointer */
vector_irq_exit:
	restore_context


	.align 8
vector_error:
	save_context
	bl 		sk_hw_trap_error
	b 		.

