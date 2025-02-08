/*                                                                                                                                                                     
 *  armv8.h
 *
 *  brif
 *      arm v8 register define
 *  
 *  (C) 2024.12.31 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */

#ifndef __CONTEXT_MACRO_H_
#define __CONTEXT_MACRO_H_


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

	mrs 	x3, spsr_el1			/* save PSTATE (progream  status  register) */
	mrs 	x2, elr_el1				/* save PC */
	stp 	x2, x3, [sp, #-0x10]!	/* save PSTATE and PC to sp stack */
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
	mov 	x2, x30					/* save LR/x30(link register, this register store function return address ) to x2 */
	stp 	x2, x3, [sp, #-0x10]!
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

	msr 	spsr_el1, x3			/* set PSTATE (program status register) register */
	msr 	elr_el1, x2				/* set pc register */

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

#endif

