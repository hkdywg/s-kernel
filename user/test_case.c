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


void mutex_thread_1(void *param)
{
	struct sk_mutex *mutex = (struct sk_mutex *)param;
	sk_mutex_lock(mutex, 10000);
	sk_kprintf("%s take mutex\n", sk_current_thread()->name);	
	sk_thread_delay(2000);
	sk_mutex_unlock(mutex);
	sk_kprintf("%s release mutex\n", sk_current_thread()->name);	
}

void mutex_thread_2(void *param)
{
	struct sk_mutex *mutex = (struct sk_mutex *)param;
	sk_mutex_lock(mutex, -1);
	sk_kprintf("%s take mutex\n", sk_current_thread()->name);	
	sk_thread_delay(100);
	sk_mutex_unlock(mutex);
	sk_kprintf("%s release mutex\n", sk_current_thread()->name);	
}

void test_mutex(void)
{
	static struct sk_mutex mutex;
	sk_mutex_init(&mutex, "test_mutex", SK_IPC_FLAG_PRIO);

	char thread_name_1[SK_NAME_MAX] = "mutex_thread_1";
	struct sk_thread *thread_1 = sk_thread_create(thread_name_1,
												mutex_thread_1,
												&mutex,
												2048,
												20,
												20);
	sk_thread_startup(thread_1);
	char thread_name_2[SK_NAME_MAX] = "mutex_thread_2";
	struct sk_thread *thread_2 = sk_thread_create(thread_name_2,
												mutex_thread_2,
												&mutex,
												2048,
												20,
												20);
	sk_thread_startup(thread_2);
}

SHELL_CMD_EXPORT(test_mutex, test case of ipc mutex);

