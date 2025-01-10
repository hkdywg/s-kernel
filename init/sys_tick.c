/*
 *  sys_tick.c
 *  brief
 *  	system tick timing
 *  
 *  (C) 2025.01.09 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <interrupt.h>
#include <config.h>
#include <hw.h>

#define HW_TIMER_VECTOR_NUM		27

static sk_uint64_t timer_val;
static sk_uint64_t timer_step;
static volatile sk_tick_t sk_tick = 0;

/*
 * This function will be called by timer isr
 *
 * @param: none
 */
void sk_tick_increase(void)
{
	++sk_tick;
}

/*
 * This function will be called by timer isr
 *
 * @param: none
 */
sk_tick_t sk_tick_get(void)
{
	return sk_tick;
}

/*
 * timer isr process
 *
 * @param: none
 */
void sk_hw_timer_isr(int vector, void *param)
{
	timer_val += timer_step;
	__asm__ volatile ("msr CNTV_CVAL_EL0, %0"::"r"(timer_val));
	__asm__ volatile ("isb":::"memory");
	
	sk_tick_increase();
}

/*
 * timer init, include timer_isr install and register configure
 *
 * @param: none
 */
int sk_hw_timer_init(void)
{
	sk_hw_interrupt_install(HW_TIMER_VECTOR_NUM, sk_hw_timer_isr, SK_NULL);
	sk_hw_interrupt_umask(HW_TIMER_VECTOR_NUM);

	__asm__ volatile ("msr CNTV_CTL_EL0, %0"::"r"(0));
	__asm__ volatile ("isb 0xf":::"memory");
	__asm__ volatile ("mrs %0, CNTFRQ_EL0" : "=r" (timer_step));
	timer_step /= TICK_PER_SECOND;
	timer_val = timer_step;
	__asm__ volatile ("dsb 0xf":::"memory");

	__asm__ volatile ("msr CNTV_CVAL_EL0, %0"::"r"(timer_val));
	__asm__ volatile ("msr CNTV_CTL_EL0, %0"::"r"(1));

    sk_base_t level;                                                                                                                                                   
    
    //level = hw_interrupt_disable();
    //hw_interrupt_enable(level);
	return 0;
}

