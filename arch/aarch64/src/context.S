/*                                                                                                                                                                     
 *  context.S
 *
 *  brif
 *      context operation, include save and restore and switch
 *  
 *  (C) 2025.01.02 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */


.macro save_context
	/* Switch to use the EL0 stack pointer */
	msr 	spsel, #0

	/* Save the entire context */
	stp 	x0, x1, [sp, #-0x10]!
	stp 	x2, x3, [sp, #-0x10]!
	stp 	x4, x5, [sp, #-0x10]!
	stp 	x6, x7, [sp, #-0x10]!
	stp 	x8, x9, [sp, #-0x10]!
	stp 	x10, x11, [sp, #-0x10]!
	stp 	x12, x13, [sp, #-0x10]!
	stp 	x14, x15, [sp, #-0x10]!
	stp 	x16, x17, [sp, #-0x10]!
	stp 	x18, x19, [sp, #-0x10]!
	stp 	x20, x21, [sp, #-0x10]!
	stp 	x22, x23, [sp, #-0x10]!
	stp 	x24, x25, [sp, #-0x10]!
	stp 	x26, x27, [sp, #-0x10]!
	stp 	x28, x29, [sp, #-0x10]!
	stp 	x30, xzr, [sp, #-0x10]!

	mrs 	x3, spsr_el1
	mrs 	x2, elr_el1
	stp 	x2, x3, [sp. #-0x10]!
	mov 	x0, sp 					/* move sp into x0 for saving */

	/* Switch to use the ELx stack pointer */
	msr 	spsel, #1
	.endm


.macro save_context_t
	/* Switch to use the EL0 stack pointer */
	msr 	spsel, #0

	/* Save the entire context */
	stp 	x0, x1, [sp, #-0x10]!
	stp 	x2, x3, [sp, #-0x10]!
	stp 	x4, x5, [sp, #-0x10]!
	stp 	x6, x7, [sp, #-0x10]!
	stp 	x8, x9, [sp, #-0x10]!
	stp 	x10, x11, [sp, #-0x10]!
	stp 	x12, x13, [sp, #-0x10]!
	stp 	x14, x15, [sp, #-0x10]!
	stp 	x16, x17, [sp, #-0x10]!
	stp 	x18, x19, [sp, #-0x10]!
	stp 	x20, x21, [sp, #-0x10]!
	stp 	x22, x23, [sp, #-0x10]!
	stp 	x24, x25, [sp, #-0x10]!
	stp 	x26, x27, [sp, #-0x10]!
	stp 	x28, x29, [sp, #-0x10]!
	stp 	x30, xzr, [sp, #-0x10]!

	mrs 	x3, spsr_el1
	mov 	x2, x30
	stp 	x2, x3, [sp. #-0x10]!
	mov 	x0, sp 					/* move sp into x0 for saving */

	/* Switch to use the ELx stack pointer */
	msr 	spsel, #1
	.endm

.macro restore_context
	/* Swtich to use the EL0 stack pointer */
	msr 	spsel, #0

	/* Set the SP to pointer to the stack of the task being restored */
	mov 	sp, x0
	ldp 	x2, x3, [sp], #0x10 	/* SPSR and ELR */

	msr 	spsr_el1, x3
	msr 	elr_el1, x2

	ldp 	x30, xzr, [sp], #0x10
	ldp 	x28, x29, [sp], #0x10
	ldp 	x26, x27, [sp], #0x10
	ldp 	x24, x25, [sp], #0x10
	ldp 	x22, x23, [sp], #0x10
	ldp 	x20, x21, [sp], #0x10
	ldp 	x18, x19, [sp], #0x10
	ldp 	x16, x17, [sp], #0x10
	ldp 	x14, x15, [sp], #0x10
	ldp 	x12, x13, [sp], #0x10
	ldp 	x10, x11, [sp], #0x10
	ldp 	x8, x9, [sp], #0x10
	ldp 	x6, x7, [sp], #0x10
	ldp 	x4, x5, [sp], #0x10
	ldp 	x2, x3, [sp], #0x10
	ldp 	x0, x1, [sp], #0x10

	/* Switch to use the ELx stack pointer. */
	msr 	spsel, #1
	eret
	.endm


.text

/*
 * interrupt_disable()
 */
.global interrupt_disable
interrupt_disable:
	mrs 	x0, daif
	msr 	daifset, #3
	dsb 	sy
	ret


/*
 * interrupt_enable(level)
 */
.global interrupt_enable
interrupt_enable:
	dsb 	sy
	mov 	x1, #0xC0
	ands 	x0, x0, x1
	b.ne 	interrupt_enable_exit
	msr 	daifclr, #3
interrupt_enable_exit:
	ret


/*
 * context_switch_to(to)
 */
.global context_switch_to
context_switch_to:
	ldr 	x0, [x0]
	restore_context


/*
 * context_switch(frome, to)
 */
.global context_switch
context_switch:
	mov 	x8, x0
	mov 	x9, x1
	save_context_t 
	str 	x0, [x8]		/* store sp in preempted tasks TCB */
	ldr 	x0, [x9]		/* get new task stack pointer */
	restore_context

/*
 * context_switch_interrupt(frome, to)
 */
.global thread_switch_interrupt_flag
.global interrupt_from_thread
.global interrupt_to_thread
.global context_switch_interrupt
context_switch_interrupt:
	adr 	x2, thread_switch_interrupt_flag
	ldr 	x3, [x2]
	cmp 	x3, #1
	b.eq 	_reswitch
	adr 	x4, interrupt_from_thread
	mov 	x3, #1
	str 	x0, [x4]
	str 	x3, [x2]
_reswitch:
	adr 	x2, interrupt_to_thread
	str 	x1, [x2]
	ret

