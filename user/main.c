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
#include <ipc.h>

//#define TEST_TIMER
//#define TEST_MUTEX
#define TEST_SEMAPHORE

struct sk_mutex g_mutex;
struct sk_sem 	g_sem;

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

void user_app_1(void *arg)
{
	while(1) {
#ifdef TEST_MUTEX
		sk_mutex_take(&g_mutex, 10000);
		sk_kprintf("%s take mutex\n", sk_current_thread()->name);	
		sk_thread_delay(100);
		sk_kprintf("%s release mutex\n", sk_current_thread()->name);	
		sk_mutex_release(&g_mutex);
		sk_thread_delay(1000);
#endif
#ifdef TEST_SEMAPHORE
		sk_sem_take(&g_sem, 10000);
		sk_kprintf("%s take sem\n", sk_current_thread()->name);	
		sk_thread_delay(100);
		sk_kprintf("%s release sem\n", sk_current_thread()->name);	
		sk_sem_release(&g_sem);
		sk_thread_delay(1000);
#endif
	}
}

void user_app_2(void *arg)
{
	while(1) {
#ifdef TEST_MUTEX
		sk_mutex_take(&g_mutex, 10000);
		sk_kprintf("%s take mutex\n", sk_current_thread()->name);	
		sk_thread_delay(100);
		sk_kprintf("%s release mutex\n", sk_current_thread()->name);	
		sk_mutex_release(&g_mutex);
		sk_thread_delay(1000);
#endif
#ifdef TEST_SEMAPHORE
		sk_sem_take(&g_sem, 10000);
		sk_kprintf("%s take sem\n", sk_current_thread()->name);	
		sk_thread_delay(100);
		sk_kprintf("%s release sem\n", sk_current_thread()->name);	
		sk_sem_release(&g_sem);
		sk_thread_delay(1000);
#endif
	}
}

void user_app_init(void)
{
	sk_mutex_init(&g_mutex, "test_mutex", SK_IPC_FLAG_PRIO);
	sk_sem_init(&g_sem, "test_sem", 2, SK_IPC_FLAG_PRIO);

	char user_thread_name_1[SK_NAME_MAX] = "user_app_1";
	struct sk_thread *thread_1 = sk_thread_create(user_thread_name_1,
												user_app_1,
												SK_NULL,
												2048,
												20,
												20);
	sk_thread_startup(thread_1);
	char user_thread_name_2[SK_NAME_MAX] = "user_app_2";
	struct sk_thread *thread_2 = sk_thread_create(user_thread_name_2,
												user_app_2,
												SK_NULL,
												2048,
												20,
												20);
	sk_thread_startup(thread_2);
}

void main(void *arg)
{
	sk_ubase_t cnt = 0;
	float cpu_usage = 0.0;
#ifdef TEST_TIMER
    sk_kprintf("prepare run main function cycle... \n");
	sk_thread_delay(5000);
    sk_kprintf("thread delay 5000ms done... \n");

	struct sk_sys_timer test_timer1;
	sk_timer_init(&test_timer1, "debug_timer", test_timer_func1, 
				  SK_NULL, 1000, SK_TIMER_FLAG_PERIODIC);

	sk_timer_start(&test_timer1);

	struct sk_sys_timer test_timer2;
	sk_timer_init(&test_timer2, "debug_timer", test_timer_func2, 
				  SK_NULL, 2000, SK_TIMER_FLAG_ONE_SHOT);

	sk_timer_start(&test_timer2);
#endif

#ifdef TEST_MUTEX
	user_app_init();
#endif
#ifdef TEST_SEMAPHORE
	user_app_init();
#endif
	while(1) {
		cnt++;
		cpu_usage = sk_idle_tick_get()/sk_tick_get();
//		sk_thread_delay(1000);
//    	sk_kprintf("%s thread runing ...\n", sk_current_thread()->name);
#ifdef TEST_TIMER
		if(timer1_cnt == 10) {
			sk_timer_stop(&test_timer1);
			sk_timer_control(&test_timer2, SK_TIMER_CTRL_SET_PERIODIC, SK_NULL);
		}
#endif
	}
}
