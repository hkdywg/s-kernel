/*                                                                                                                                                                     
 *  trap.c
 *
 *  brif
 *      cpu trap definitions
 *  
 *  (C) 2025.01.08 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <interrupt.h>
#include <armv8.h>
#include <type_def.h>

/*
 * When comes across an instruction which it can't handle,
 * it take the undefined instruction trap
 * @param: 
 * 		regs: system regs
 */
void sk_hw_trap_error(struct sk_hw_exp_stack *regs)
{
	sk_hw_cpu_shutdown();
}

void sk_hw_trap_irq(void)
{
	void *param;
	sk_int32_t irq;
	sk_isr_handler_t isr_func;
	extern struct sk_irq_desc isr_table[];

	/* get irq number */
	irq = sk_hw_interrupt_get_irq();
	if(irq == 1023)
		return;

	/* get interrupt service routine */
	isr_func = isr_table[irq].handler;
	if(isr_func) {
		param = isr_table[irq].param;
		isr_func(irq, param);
	}

	/* end of interrupt */
	sk_hw_interrupt_ack(irq);
}

void sk_hw_trap_fiq(void)
{
	sk_hw_trap_irq();	
}
