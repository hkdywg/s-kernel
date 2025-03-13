/*
 *  test_ipc.c
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
	
#define EVENT_FLAG0 (1 << 0)
#define EVENT_FLAG1 (1 << 1)

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
	sk_kprintf("%s take mutex\n", sk_current_thread()->name);	
	sk_mutex_lock(mutex, 10000);
	sk_thread_delay(2000);
	sk_mutex_unlock(mutex);
	sk_kprintf("%s release mutex\n", sk_current_thread()->name);	
}

void mutex_thread_2(void *param)
{
	struct sk_mutex *mutex = (struct sk_mutex *)param;
	sk_kprintf("%s take mutex\n", sk_current_thread()->name);	
	sk_mutex_lock(mutex, -1);
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

void sem_thread_1(void *param)
{
	struct sk_sem *sem = (struct sk_sem *)param;
	sk_kprintf("%s wait semaphore\n", sk_current_thread()->name);	
	sk_sem_wait(sem, 10000);
	sk_thread_delay(2000);
	sk_sem_post(sem);
	sk_kprintf("%s post semaphore \n", sk_current_thread()->name);	
}

void sem_thread_2(void *param)
{
	struct sk_sem *sem = (struct sk_sem *)param;
	sk_sem_wait(sem, -1);
	sk_kprintf("%s get semaphore\n", sk_current_thread()->name);	
	sk_thread_delay(100);
	sk_sem_post(sem);
	sk_kprintf("%s post semaphore \n", sk_current_thread()->name);	
}

void test_sem(void)
{
	static struct sk_sem sem;
	sk_sem_init(&sem, "test_sem", 2, SK_IPC_FLAG_PRIO);

	char thread_name_1[SK_NAME_MAX] = "sem_thread_1";
	struct sk_thread *thread_1 = sk_thread_create(thread_name_1,
												sem_thread_1,
												&sem,
												2048,
												20,
												20);
	sk_thread_startup(thread_1);
	char thread_name_2[SK_NAME_MAX] = "mutex_thread_2";
	struct sk_thread *thread_2 = sk_thread_create(thread_name_2,
												sem_thread_2,
												&sem,
												2048,
												20,
												20);
	sk_thread_startup(thread_2);
}

SHELL_CMD_EXPORT(test_sem, test case of ipc semaphore);


void event_thread_1(void *param)
{
	struct sk_event *event = (struct sk_event *)param;
	sk_thread_delay(100);
	sk_event_send(event, EVENT_FLAG1);
	sk_thread_delay(100);
	sk_event_send(event, EVENT_FLAG0);
}

void event_thread_2(void *param)
{
	struct sk_event *event = (struct sk_event *)param;
	sk_uint32_t e;

	if(sk_event_recv(event, (EVENT_FLAG0 | EVENT_FLAG1),
					 SK_EVENT_FLAG_OR,
					 -1, &e) == SK_EOK) {
		sk_kprintf("%s recv event: %d\n", sk_current_thread()->name, e);
	}
	if(sk_event_recv(event, (EVENT_FLAG0 | EVENT_FLAG1),
					 SK_EVENT_FLAG_AND | SK_EVENT_FLAG_CLEAR,
					 -1, &e) == SK_EOK) {
		sk_kprintf("%s recv event: %d\n", sk_current_thread()->name, e);
	}
}

void test_event(void)
{
	static struct sk_event event; 
	sk_event_init(&event, "test_event", SK_IPC_FLAG_PRIO);

	char thread_name_1[SK_NAME_MAX] = "sem_thread_1";
	struct sk_thread *thread_1 = sk_thread_create(thread_name_1,
												event_thread_1,
												&event,
												2048,
												20,
												20);
	sk_thread_startup(thread_1);
	char thread_name_2[SK_NAME_MAX] = "mutex_thread_2";
	struct sk_thread *thread_2 = sk_thread_create(thread_name_2,
												event_thread_2,
												&event,
												2048,
												20,
												20);
	sk_thread_startup(thread_2);
}

SHELL_CMD_EXPORT(test_event, test case of ipc semaphore);

void mailbox_thread_1(void *param)
{
	struct sk_mailbox *mb = (struct sk_mailbox *)param;
	for(sk_uint8_t i = 0; i < 128; i++) {
		sk_mailbox_send_wait(mb, i, 1000);
		sk_thread_delay(100);
	}
}

void mailbox_thread_2(void *param)
{
	struct sk_mailbox *mb = (struct sk_mailbox *)param;
	sk_ubase_t value;
	sk_uint8_t cnt = 0;

	while(1) {
		sk_mailbox_recv(mb, &value, 10000);
		sk_kprintf("%s recv mail: %d\n", sk_current_thread()->name, value);
		if(cnt >= 127)
			break;
	}
}

void test_mailbox(void)
{
	static struct sk_mailbox mb; 
	static sk_ubase_t  mb_pool[64];
	sk_mailbox_init(&mb, "test_mailbox", mb_pool, 64, SK_IPC_FLAG_PRIO);

	char thread_name_1[SK_NAME_MAX] = "mailbox_thread_1";
	struct sk_thread *thread_1 = sk_thread_create(thread_name_1,
												mailbox_thread_1,
												&mb,
												2048,
												20,
												20);
	sk_thread_startup(thread_1);
	char thread_name_2[SK_NAME_MAX] = "mailbox_thread_2";
	struct sk_thread *thread_2 = sk_thread_create(thread_name_2,
												mailbox_thread_2,
												&mb,
												2048,
												20,
												20);
	sk_thread_startup(thread_2);
}

SHELL_CMD_EXPORT(test_mailbox, test case of ipc mailbox);

