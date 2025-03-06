/*
 *  main.c
 *  brief
 *  	user application main 
 *  
 *  (C) 2025.01.24 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <skernel.h>
#include <timer.h>

sk_uint8_t timer1_cnt = 0; 
sk_uint8_t timer2_cnt = 0; 

void test_timer_func1(void *param)
{
    sk_kprintf("timer_1 timeout ... \n");
	timer1_cnt++;
}

void test_timer_func2(void *param)
{
    sk_kprintf("timer_2 timeout ... \n");
	timer2_cnt++;
}

void main(void *arg)
{
	sk_ubase_t cnt = 0;
	float cpu_usage = 0.0;

	struct sk_sys_timer test_timer1;
	sk_timer_init(&test_timer1, "debug_timer", test_timer_func1, 
				  SK_NULL, 1000, SK_TIMER_FLAG_PERIODIC);

	sk_timer_start(&test_timer1);

	struct sk_sys_timer test_timer2;
	sk_timer_init(&test_timer2, "debug_timer", test_timer_func2, 
				  SK_NULL, 2000, SK_TIMER_FLAG_ONE_SHOT);

	sk_timer_start(&test_timer2);

	while(1) {
		cnt++;
		cpu_usage = sk_idle_tick_get()/sk_tick_get();
		if(timer1_cnt == 10) {
			sk_timer_stop(&test_timer1);
			sk_timer_control(&test_timer2, SK_TIMER_CTRL_SET_PERIODIC, SK_NULL);
		}
	}
}
