/*
 *  test_case.c
 *  brief
 *  	test case of kernel module
 *  
 *  (C) 2025.03.12 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <skernel.h>
#include <timer.h>
#include <ipc.h>
#include <shell.h>
	

void test_timer_func(void *param)
{
	struct sk_sys_timer *timer = (struct sk_sys_timer *)param;
	static sk_uint8_t cnt = 0;
	sk_err_t err;
	cnt++;
    sk_kprintf("timer timeout(%d) ... \n", cnt);
	if(cnt == 5) {
    	sk_kprintf("stop timer \n");
		sk_timer_stop(timer);
	}
}

void test_sys_timer()
{
	static struct sk_sys_timer test_timer;
	sk_timer_init(&test_timer, "debug_timer", test_timer_func, 
				  &test_timer, 1000, SK_TIMER_FLAG_PERIODIC);

	sk_timer_start(&test_timer);
}

SHELL_CMD_EXPORT(test_sys_timer, test case of system timer);
