/*
 *  startup.c
 *  brief
 *  	kernel entrance
 *  
 *  (C) 2024.12.25 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <interrupt.h>
#include <skernel.h>
#include <hw.h>
#include <timer.h>
#include <sched.h>

extern unsigned char __bss_start;
extern unsigned char __bss_end;

#define SK_HEAP_BEGIN    (void*)&__bss_end
#define SK_HEAP_END      (void*)(SK_HEAP_BEGIN + 1 * 1024 * 1024)

#define SK_MAIN_THREAD_STATCK_SIZE 		(2048)
#define SK_MAIN_THREAD_PRIORITY 		(SK_THREAD_PRIORITY_MAX/3)

/*
 * Init the hardware related 
 *
 * @param: none
 */
void sk_hw_board_init(void)
{
    hw_interrupt_disable();

	/* Initialize hardware interrupt */
	sk_hw_interrupt_init();

	/* kernel tick init */
	sk_hw_timer_init();

	/* memory management init */
	sk_system_mem_init(SK_HEAP_BEGIN, SK_HEAP_END);
}

/*
 * sk_application_init
 * brief
 * 		create and start the user main thread, but this thread will not run
 * 		until the scheduler start
 */
void sk_application_init(void)
{
	extern void main(void *); 
	char user_thread_name[SK_NAME_MAX] = "main";

	struct sk_thread *thread = sk_thread_create(user_thread_name,
												main,
												SK_NULL,
												SK_MAIN_THREAD_STATCK_SIZE,
												SK_MAIN_THREAD_PRIORITY,
												20);
	sk_thread_startup(thread);
}

int skernel_startup(void)
{
	/* hardware related init, must be first called in skernel_startup*/
	sk_hw_board_init();

	/* system timer init */
	sk_system_timer_init();
	/* scheduler system init */
	sk_system_scheduler_init();
	/* user main thread init */
	sk_application_init();
	/* idle thread init */
	sk_thread_idle_init();

	/* system scheduler start */
	sk_system_scheduler_start();

	return 0;
}
