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
#include <sched.h>

#define INITIAL_SPSR_EL1			(0x04)
#define SK_IDLE_THREAD_STACK_SIZE 	(128)
#define SK_IDLE_THREAD_TICK 		(32)

static sk_tick_t idle_tick = 10;


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
static sk_uint8_t *__thread_stack_init(void *entry,void *param,
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
struct sk_thread* sk_current_thread(void)
{
	extern struct sk_thread *current_thread;

	return current_thread;
}

/*
 * sk_thread_timeout
 * brief
 * 		this function will be called when thread timer timeout
 * param
 * 		param: argument of timer function
 */
void sk_thread_timeout(void *param)
{
	sk_base_t level;
	struct sk_thread *thread = (struct sk_thread *)param;

	/* disable interrupt */
	level = hw_interrupt_disable();

	/* remove from suspend list */
	sk_list_del(&(thread->tlist));

	/* insert to schedule ready list */
	sk_schedule_insert_thread(thread);

	/* enable interrupt */
	hw_interrupt_enable(level);

	/* do schedule */
	sk_schedule();
}

/*
 * __thread_exit
 * brief
 * 		this function will be called when thread delete
 */
static void __thread_exit(void)
{
	struct sk_thread *thread;
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
	thread->stat = SK_THREAD_CLOSE;

	/* remove it from system tick list */
	sk_timer_delete(&thread->thread_timer);

	/* switch to next thread */
	sk_schedule();

	/* enable interrupt */
	hw_interrupt_enable(level);

}

/*
 *	__thread_init
 *	brief
 *		this function will initialize a thread
 *	param
 *		thread: sk_thread 
 *		name: thread name
 *		entry: function of thread
 *		param: parameter of thread enter function
 *		stack_start: the start address of thread stack
 *		stack_size: the size of thread stack
 *		priority: priority of thread
 *		tick: the time slice if there are same priority thread
 *
 * */
static sk_err_t __thread_init(struct sk_thread 	*thread,
						   const char 			*name,
						   void 				(*entry)(void *param),
						   void 				*param,
						   void 				*stack_start,
						   sk_uint32_t 			stack_size,
						   sk_uint8_t 			priority,
						   sk_uint32_t 			tick)
{
	/* init thread list */
	sk_list_init(&(thread->tlist));

	/* init thread name */
	sk_memcpy(thread->name, name, SK_NAME_MAX);

	/* init thread entry and param */
	thread->entry = (void *)entry;
	thread->param = param;

	/* thread stack address and size config */
	thread->stack_addr = stack_start;
	thread->stack_size = stack_size;

	/* init thread stack */
	thread->sp = (void *)__thread_stack_init(thread->entry, thread->param,
										(sk_uint8_t *)((char *)thread->stack_addr + thread->stack_size - sizeof(sk_ubase_t)),
										(void *)__thread_exit);

	/* priority init */
	thread->init_pri = priority;
	thread->current_pri = priority; 

	/* set priority attribute */
	thread->number_mask = 1 << thread->current_pri;

	/* init thread state and tick */
	thread->init_tick = tick;
	thread->remain_tick = tick;

	/* initialize thread timer */
	sk_timer_init(&(thread->thread_timer), thread->name,
				  sk_thread_timeout, thread, 0, SK_TIMER_FLAG_ONE_SHOT);

	thread->stat = SK_THREAD_INIT;
	/* set cleanup function and userdata */
	thread->cleanup = SK_NULL;
	thread->user_data = 0;
		
	/* insert to schedule ready list */
	sk_schedule_insert_thread(thread);

	return SK_EOK;
}


/*
 * sk_thread_create
 * brief
 * 		create a thread object and allocate thread stack memory	
 * param
 * 		name: the name of thread
 * 		entry: the entry function of thread
 * 		param: parameter of entry function
 * 		stack_size: the stack size of thread
 * 		priority: the priority of thread
 * 		tick: the time slice if there are same priority thread
 */
struct sk_thread *sk_thread_create(const char 			*name,
							   void 				(*entry)(void *param),
							   void 				*param,
							   sk_uint32_t 			stack_size,
							   sk_uint8_t 			priority,
							   sk_uint32_t 			tick)
{
	struct sk_thread *thread; 
	void *stack_start;

	thread = (struct sk_thread *)sk_object_alloc(SK_OBJECT_THREAD, name);
	if(thread == SK_NULL)
		return SK_NULL;

	stack_start = (void *)sk_malloc(stack_size);
	if(stack_start == SK_NULL) {
		/* delete allocated object */
		sk_object_delete((struct sk_object *)thread);
		return SK_NULL;
	}

	__thread_init(thread, name ,entry, param, stack_start, stack_size, priority, tick);

	return thread;
}

/*
 * sk_thread_suspend
 * brief
 * 		suspend the specified thread and change it to suspend state
 * param
 * 		thread: the thread to be suspended
 */
sk_err_t sk_thead_suspend(struct sk_thread *thread)
{
	sk_base_t stat;
	sk_base_t level;

	/* ony thread is ready or running, can be suspend */
	if((thread->stat !=  SK_THREAD_READY) && (thread->stat != SK_THREAD_RUNNING)) {
		return SK_ERROR;
	}

	/* disable interrupt */
	level = hw_interrupt_disable();
	if(thread->stat == SK_THREAD_RUNNING) {
		/* not support suspend */
		if(thread == sk_current_thread())
			return SK_ERROR;
	}
	/* change thread state */
	sk_schedule_remove_thread(thread);
	thread->stat = SK_THREAD_SUSPEND;

	/* enable interrupt */
	hw_interrupt_enable(level);

	return SK_EOK;
}

/*
 * sk_thread_resume
 * brief
 * 		resume a thread and put it to system ready queue
 * param
 * 		thread: the pointer of thread to be resumed
 */
sk_err_t sk_thread_resume(struct sk_thread *thread)
{
	register sk_base_t level;

	if(thread->stat != SK_THREAD_SUSPEND)
		return SK_ERROR;

	/* disable interrupt */
	level = hw_interrupt_disable();

	/* remove from suspend list */
	sk_list_del(&(thread->tlist));

	/* insert to scedule ready list */
	sk_schedule_insert_thread(thread);

	/* enable interrupt */
	hw_interrupt_enable(level);

	return SK_EOK;
}


sk_err_t sk_thread_startup(struct sk_thread *thread)
{
	/* change thread state */
	thread->stat = SK_THREAD_SUSPEND;
	/* then resume it */
	//sk_thread_resume(thread);
	if(sk_current_thread() != SK_NULL)
		sk_schedule();			/* do scheduling */

	return SK_EOK;
}

/*
 * sk_thread_suspend
 * brief
 * 		this function will suspend the specified thread and change it to suspend state
 * param
 * 		thread: the thread to be suspended
 */
sk_err_t sk_thread_suspend(struct sk_thread *thread)
{
	sk_base_t level;
	sk_base_t stat = thread->stat & SK_THREAD_MASK;

	if(stat != SK_THREAD_READY && stat != SK_THREAD_RUNNING)
		return SK_ERROR;

	/* disable interrupt */
	level = hw_interrupt_disable();

	/* change thread state */
	sk_schedule_remove_thread(thread);
	thread->stat = SK_THREAD_SUSPEND;

	/* stop thread timer */
	sk_timer_stop(&(thread->thread_timer));

	/* enable interrupt */
	hw_interrupt_enable(level);

	return SK_EOK;
}


/*
 * sk_thread_sleep
 * brief 
 * 		let current thread sleep for some ticks. this function will change current
 * 		thead state to suspend, and set thread timer, after the timeout schedule will awaken this thread
 * param
 * 		tick: sleep ticks
 */
sk_err_t sk_thread_sleep(sk_tick_t tick)
{
	struct sk_thread *thread;
	sk_base_t level;

	/* get current thread */
	thread = sk_current_thread();

	/* disable interrupt */
	level = hw_interrupt_disable();

	/* suspend thread */
	sk_thread_suspend(thread);

	/* reset the timeout thread timer and start it */
	sk_timer_control(&(thread->thread_timer), SK_TIMER_CTRL_SET_TIME, &tick);
	sk_timer_start(&(thread->thread_timer));

	/* enable interrupt */
	hw_interrupt_enable(level);

	sk_schedule();

	return SK_EOK;
}

/*
 * sk_thread_delay
 * brief
 * 		this function will let current thread delay for some ms
 * param
 * 		ms: the specifial millisecond will be delay
 */
sk_err_t sk_thread_delay(sk_uint32_t ms)
{
	sk_tick_t tick;

	tick = sk_tick_from_ms(ms);

	return sk_thread_sleep(tick);
}

static void sk_idle_entry(void *param) 
{
	while(1) {
		idle_tick++;
		/* put cpu into sleep mode and wait for wake-up,
		 * can be woken up by interrupt */
		asm volatile ("wfi");
	}
}

sk_tick_t sk_idle_tick_get()
{
	return idle_tick;
}

void sk_thread_idle_init(void) 
{
	char idle_thread_name[SK_NAME_MAX] = "kidle";

	struct sk_thread *thread = sk_thread_create(idle_thread_name,
												sk_idle_entry,
												SK_NULL,
												SK_IDLE_THREAD_STACK_SIZE,
												SK_THREAD_PRIORITY_MAX - 1,
												SK_IDLE_THREAD_TICK);
	sk_thread_startup(thread);
}

