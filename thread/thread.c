/*
 *  thread.c
 *  brief
 *  	thread 
 *  
 *  (C) 2025.01.14 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <interrupt.h>
#include <skernel.h>
#include <hw.h>
#include <klist.h>

#define INITIAL_SPSR_EL1	(0x04)


/*
 * __thread_stack_init
 * brief
 * 		this function will initialize thread stack
 * @param: 
 * 		entry: the entry of the thread
 * 		param: parameter of entry
 * 		stack_addr: the begining stack address
 * 		exit: the function will be called when thread exit
 */
static sk_uint8_t __thread_stack_init(void *entry,void *param,
									sk_uint8_t *stack_addr, void *exit) 
{
	sk_ubase_t 	*stk; 

	stk = (sk_ubase_t *)stack_addr;

	*(--stk) = (sk_ubase_t) 11;			/* X1 */
	*(--stk) = (sk_ubase_t) param;		/* X0 */
	*(--stk) = (sk_ubase_t) 33;			/* X3 */
	*(--stk) = (sk_ubase_t) 22;			/* X2 */
	*(--stk) = (sk_ubase_t) 55;			/* X5 */
	*(--stk) = (sk_ubase_t) 44;			/* X4 */
	*(--stk) = (sk_ubase_t) 77;			/* X7 */
	*(--stk) = (sk_ubase_t) 66;			/* X6 */
	*(--stk) = (sk_ubase_t) 99;			/* X9 */
	*(--stk) = (sk_ubase_t) 88;			/* X8 */
	*(--stk) = (sk_ubase_t) 11;			/* X11 */
	*(--stk) = (sk_ubase_t) 10;			/* X10 */
	*(--stk) = (sk_ubase_t) 13;			/* X13 */
	*(--stk) = (sk_ubase_t) 12;			/* X12 */
	*(--stk) = (sk_ubase_t) 15;			/* X15 */
	*(--stk) = (sk_ubase_t) 14;			/* X14 */
	*(--stk) = (sk_ubase_t) 17;			/* X17 */
	*(--stk) = (sk_ubase_t) 16;			/* X16 */
	*(--stk) = (sk_ubase_t) 19;			/* X19 */
	*(--stk) = (sk_ubase_t) 18;			/* X18 */
	*(--stk) = (sk_ubase_t) 21;			/* X21 */
	*(--stk) = (sk_ubase_t) 20;			/* X20 */
	*(--stk) = (sk_ubase_t) 23;			/* X23 */
	*(--stk) = (sk_ubase_t) 22;			/* X22 */
	*(--stk) = (sk_ubase_t) 25;			/* X25 */
	*(--stk) = (sk_ubase_t) 24;			/* X24 */
	*(--stk) = (sk_ubase_t) 27;			/* X27 */
	*(--stk) = (sk_ubase_t) 26;			/* X26 */
	*(--stk) = (sk_ubase_t) 29;			/* X29 */
	*(--stk) = (sk_ubase_t) 28;			/* X28 */
	*(--stk) = (sk_ubase_t) 0;			/* XZR */
	*(--stk) = (sk_ubase_t) exit;		/* XZR */

	*(--stk) = INITIAL_SPSR_EL1;
	*(--stk) = (sk_ubase_t) entry;		/* exception return address */

	/* return thread current stack address */
	return (sk_uint8_t *)stk;
}

/*
 *	sk_current_thread
 *	brief
 *		this function will return current thread
 *		
 * */
struct thread_struct* sk_current_thread(void)
{
	extern thread_struct *sk_current_thread;

	return sk_current_thread;
}

static void __thread_exit(void)
{
	struct thread_struct *thread;
	register sk_base_t level;

	/* get current thread */
	thread = sk_current_thread();

	/* disable interrupt */
	level = hw_interrupt_disable();

	/* invoke thread cleanup */
	if(thread->cleanup != SK_NULL)
		thread->cleanup(thread);

	/* remove from schedule */
	sk_schedule_remove_thread(thread);

	/* change thread state */
	thread->stat = SK_thread_CLOSE;

	/* remove it from system tick list */
	sk_tick_detack(&thread->tick);

	/* switch to next thread */
	sk_schedule();

	/* enable interrupt */
	hw_interrupt_enable();

}

/*
 *	__thread_init
 *	brief
 *		this function will initialize a thread
 *	param
 *		thread: thread_struct 
 *		name: thread name
 *		entry: function of thread
 *		param: parameter of thread enter function
 *		stack_start: the start address of thread stack
 *		stack_size: the size of thread stack
 *		priority: priority of thread
 *		tick: the time slice if there are same priority thread
 *
 * */
static sk_err_t __thread_init(struct thread_struct 	*thread,
						   const char 			*name,
						   void 				(*entry)(void *param),
						   void 				*param,
						   void 				*stack_start,
						   sk_uint32_t 			stack_size,
						   sk_uint8_t 			priority,
						   sk_uint32_t 			tick)
{
	/* init thread list */
	sk_list_init(&(thread->list));

	thread->entry = (void *)entry;
	thread->pram = param;

	thread->stack_addr = stack_start;
	thread->stack_size = stack_size;

	/* init thread stack */
	thread->sp = (void *)sk_hw_stack_init();

	/* init thread stack */
	thread->sp = (void *)sk_hw_stack_init(thread->entry, thread->param,
										(sk_uint8_t *)((char *)thread->stack_addr + thread->stack_size - sizeof(sk_ubase_t)),
										(void *)__thread_exit);

	/* priority init */
	thread->init_pri = priority;
	thread->current_pri = priority; 

	thread->stat = SK_thread_INIT;
	thread->cleanup = SK_NULL;
	thread->user_data = 0;

	/* init thread timer */
	sk_tick_init();

	return SK_EOK;
}


sk_err_t sk_thread_init(struct thread_struct 	*thread,
					   const char 			*name,
					   void 				(*entry)(void *param),
					   void 				*param,
					   void 				*stack_start,
					   sk_uint32_t 			stack_size,
					   sk_uint8_t 			priority,
					   sk_uint32_t 			tick)
{
	return __thread_init(thread, name ,entry, param, stack_start, stack_size, priority, tick);
}

sk_err_t sk_thread_resume(struct thread_struct *thread)
{
	register sk_base_t level;

	if(thread->stat != SK_thread_SUSPEND)
		return SK_ERROR;

	/* disable interrupt */
	level = hw_interrupt_disable();

	/* remove from suspend list */
	sk_list_del(&(thread->list));

	/* insert to scedule ready list */
	sk_schedule_insert_thread(thread);

	/* enable interrupt */
	hw_interrupt_enable(level);

	return SK_EOK;
}


sk_err_t sk_thread_startup(struct thread_struct *thread)
{
	/* change thread state */
	thread->stat = SK_thread_SUSPEND;
	/* then resume it */
	sk_thread_resume(thread);
	if(sk_thread_current() != SK_NULL)
		sk_schedule();			/* do scheduling */

	return SK_EOK;
}

