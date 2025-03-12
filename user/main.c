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
#include <shell.h>

//#define TEST_TIMER
//#define TEST_MUTEX
//#define TEST_SEMAPHORE
//#define TEST_EVENT

struct sk_mutex g_mutex;
struct sk_sem 	g_sem;
struct sk_event g_event;

sk_uint8_t timer1_cnt = 0; 
sk_uint8_t timer2_cnt = 0; 

#define EVENT_FLAG0 (1 << 0)
#define EVENT_FLAG1 (1 << 1)

void print_current_thread_name()
{
	sk_kprintf("current thread is %s\n", sk_current_thread()->name);
}

SHELL_CMD_EXPORT(print_current_thread_name, show current running thread);


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
		sk_mutex_lock(&g_mutex, 10000);
		sk_kprintf("%s take mutex\n", sk_current_thread()->name);	
		sk_thread_delay(100);
		sk_kprintf("%s release mutex\n", sk_current_thread()->name);	
		sk_mutex_unlock(&g_mutex);
		sk_thread_delay(1000);
#endif
#ifdef TEST_SEMAPHORE
		sk_sem_wait(&g_sem, 10000);
		sk_kprintf("%s take sem\n", sk_current_thread()->name);	
		sk_thread_delay(100);
		sk_kprintf("%s release sem\n", sk_current_thread()->name);	
		sk_sem_post(&g_sem);
		sk_thread_delay(1000);
#endif
#ifdef TEST_EVENT
		sk_thread_delay(10);
		sk_event_send(&g_event, EVENT_FLAG0);
		sk_thread_delay(10);
		sk_event_send(&g_event, EVENT_FLAG1);
		sk_thread_delay(1000);
		sk_event_send(&g_event, EVENT_FLAG1);
		sk_thread_delay(10000);
#endif
	}
}

void user_app_2(void *arg)
{
	while(1) {
#ifdef TEST_MUTEX
		sk_mutex_lock(&g_mutex, 10000);
		sk_kprintf("%s take mutex\n", sk_current_thread()->name);	
		sk_thread_delay(100);
		sk_kprintf("%s release mutex\n", sk_current_thread()->name);	
		sk_mutex_unlock(&g_mutex);
		sk_thread_delay(1000);
#endif
#ifdef TEST_SEMAPHORE
		sk_sem_wait(&g_sem, 10000);
		sk_kprintf("%s take sem\n", sk_current_thread()->name);	
		sk_thread_delay(100);
		sk_kprintf("%s release sem\n", sk_current_thread()->name);	
		sk_sem_post(&g_sem);
		sk_thread_delay(1000);
#endif
#ifdef TEST_EVENT
	sk_uint32_t e;
	if(sk_event_recv(&g_event, (EVENT_FLAG0 | EVENT_FLAG1),
					 SK_EVENT_FLAG_OR | SK_EVENT_FLAG_CLEAR,
					 -1, &e) == SK_EOK) {
		sk_kprintf("%s recv event: %d\n", sk_current_thread()->name, e);
	}
	sk_thread_delay(100);
	if(sk_event_recv(&g_event, (EVENT_FLAG0 | EVENT_FLAG1),
					 SK_EVENT_FLAG_AND | SK_EVENT_FLAG_CLEAR,
					 -1, &e) == SK_EOK) {
		sk_kprintf("%s recv event: %d\n", sk_current_thread()->name, e);
	}
#endif
	}
}

void user_app_init(void)
{
	sk_mutex_init(&g_mutex, "test_mutex", SK_IPC_FLAG_PRIO);
	sk_sem_init(&g_sem, "test_sem", 2, SK_IPC_FLAG_PRIO);
	sk_event_init(&g_event, "test_event", SK_IPC_FLAG_PRIO);

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

//	user_app_init();

	while(1) {
		cnt++;
		cpu_usage = sk_idle_tick_get()/sk_tick_get();
		sk_thread_delay(1000);
//    	sk_kprintf("%s thread runing ...\n", sk_current_thread()->name);
	}
}
