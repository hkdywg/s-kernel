/*                                                                                                                                                                     
 *  irq.c
 *
 *  brif
 *      irq operation functions definitions
 *  
 *  (C) 2025.01.09 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <type_def.h>
#include <hw.h>

volatile sk_uint8_t interrupt_nest = 0;

/*
 * This function will be called by assemly code, when enter interrupt service routine
 *
 * note: don't invoke this function in application
 *
 * @param: none
 */
void sk_interrupt_enter(void)
{
	sk_base_t level;

	level = hw_interrupt_disable();
	interrupt_nest ++;
	hw_interrupt_enable(level);
}

/*
 * This function will be called by assemly code, when enter interrupt service routine
 *
 * note: don't invoke this function in application
 *
 * @param: none
 */
void sk_interrupt_leave(void)
{
	sk_base_t level;

	level = hw_interrupt_disable();
	interrupt_nest --;
	hw_interrupt_enable(level);
}

/*
 * This function will be called by assemly code, when enter interrupt service routine
 *
 * note: don't invoke this function in application
 *
 * @param: none
 */
sk_uint8_t sk_interrupt_get_nest(void)
{
	sk_uint8_t ret;
	sk_base_t level;

	level = hw_interrupt_disable();
	ret = interrupt_nest;
	hw_interrupt_enable(level);

	return ret;
}

