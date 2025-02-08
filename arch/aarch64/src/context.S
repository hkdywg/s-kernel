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

#include <context_macro.h>

.globl hw_get_current_el
hw_get_current_el:                                                                                                                                                                                  
    mrs     x0, CurrentEL
    cmp     x0, 0xc
    b.eq    3f
    cmp     x0, 0x8
    b.eq    2f
    cmp     x0, 0x4
    b.eq    1f

    ldr     x0, =0
    b       0f
3:
    ldr     x0, =3
    b       0f
2:
    ldr     x0, =2
    b       0f
1:
    ldr     x0, =1
    b       0f
0:
    ret

.text

/*
 * interrupt_disable()
 */
.global hw_interrupt_disable
hw_interrupt_disable:
	mrs 	x0, daif
	msr 	daifset, #3
	dsb 	sy
	ret


/*
 * interrupt_enable(level)
 * daif register bit6/7 control irq/fiq mask
 */
.global hw_interrupt_enable
hw_interrupt_enable:
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
.global hw_context_switch_to
hw_context_switch_to:
	ldr 	x0, [x0]
	restore_context


/*
 * context_switch(frome, to)
 */
.global hw_context_switch
hw_context_switch:
	mov 	x8, x0
	mov 	x9, x1
	save_context_t 
	str 	x0, [x8]		/* store sp in preempted tasks TCB */
	ldr 	x0, [x9]		/* get new task stack pointer */
	restore_context

/*
 * context_switch_interrupt(frome, to)
 */
.global hw_context_switch_interrupt
hw_context_switch_interrupt:
	adr 	x2, thread_switch_interrupt_flag
	ldr 	x3, [x2]
	cmp 	x3, #1
	b.eq 	_reswitch
	adr 	x4, interrupt_from_thread
	mov 	x3, #1
	str 	x0, [x4]				/* save the from thread sp to interrupt_from_thread */
	str 	x3, [x2]				/* set thread_switch_interrupt_flag to 1 */
_reswitch:
	adr 	x2, interrupt_to_thread
	str 	x1, [x2]				/* save the to thread sp to interrupt_to_thread */
	ret

.global thread_switch_interrupt_flag
.global interrupt_from_thread
.global interrupt_to_thread
.section .data
	.align 3
	thread_switch_interrupt_flag:
		.quad 0
	interrupt_from_thread:
		.quad 0
	interrupt_to_thread:
		.quad 0
