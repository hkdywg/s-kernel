/*
 *  task.c
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
 * __task_stack_init
 * brief
 * 		this function will initialize task stack
 * @param: 
 * 		entry: the entry of the task
 * 		param: parameter of entry
 * 		stack_addr: the begining stack address
 * 		exit: the function will be called when thread exit
 */
static sk_uint8_t __task_stack_init(void *entry,void *param,
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

	/* return task current stack address */
	return (sk_uint8_t *)stk;
}

/*
 *	sk_current_task
 *	brief
 *		this function will return current task
 *		
 * */
struct task_struct* sk_current_task(void)
{
	extern task_struct *sk_current_task;

	return sk_current_task;
}

static void __task_exit(void)
{
	struct task_struct *task;
	register sk_base_t level;

	/* get current thread */
	task = sk_current_task();

	/* disable interrupt */
	level = hw_interrupt_disable();

	/* invoke task cleanup */
	if(task->cleanup != SK_NULL)
		task->cleanup(task);

	/* remove from schedule */
	sk_schedule_remove_task(task);

	/* change task state */
	task->stat = SK_TASK_CLOSE;

	/* remove it from system tick list */
	sk_tick_detack(&task->tick);

	/* switch to next task */
	sk_schedule();

	/* enable interrupt */
	hw_interrupt_enable();

}

/*
 *	__task_init
 *	brief
 *		this function will initialize a task
 *	param
 *		task: task_struct 
 *		name: task name
 *		entry: function of task
 *		param: parameter of task enter function
 *		stack_start: the start address of task stack
 *		stack_size: the size of task stack
 *		priority: priority of task
 *		tick: the time slice if there are same priority task
 *
 * */
static sk_err_t __task_init(struct task_struct 	*task,
						   const char 			*name,
						   void 				(*entry)(void *param),
						   void 				*param,
						   void 				*stack_start,
						   sk_uint32_t 			stack_size,
						   sk_uint8_t 			priority,
						   sk_uint32_t 			tick)
{
	/* init thread list */
	sk_list_init(&(task->list));

	task->entry = (void *)entry;
	task->pram = param;

	task->stack_addr = stack_start;
	task->stack_size = stack_size;

	/* init thread stack */
	task->sp = (void *)sk_hw_stack_init();

	/* init task stack */
	task->sp = (void *)sk_hw_stack_init(task->entry, task->param,
										(sk_uint8_t *)((char *)task->stack_addr + task->stack_size - sizeof(sk_ubase_t)),
										(void *)__task_exit);

	/* priority init */
	task->init_pri = priority;
	task->current_pri = priority; 

	task->stat = SK_TASK_INIT;
	task->cleanup = SK_NULL;
	task->user_data = 0;

	/* init task timer */
	sk_tick_init();

	return SK_EOK;
}


sk_err_t sk_task_init(struct task_struct 	*task,
					   const char 			*name,
					   void 				(*entry)(void *param),
					   void 				*param,
					   void 				*stack_start,
					   sk_uint32_t 			stack_size,
					   sk_uint8_t 			priority,
					   sk_uint32_t 			tick)
{
	return __task_init(task, name ,entry, param, stack_start, stack_size, priority, tick);
}

sk_err_t sk_task_resume(struct task_struct *task)
{
	register sk_base_t level;

	if(task->stat != SK_TASK_SUSPEND)
		return SK_ERROR;

	/* disable interrupt */
	level = hw_interrupt_disable();

	/* remove from suspend list */
	sk_list_del(&(task->list));

	/* insert to scedule ready list */
	sk_schedule_insert_task(task);

	/* enable interrupt */
	hw_interrupt_enable(level);

	return SK_EOK;
}


sk_err_t sk_task_startup(struct task_struct *task)
{
	/* change task state */
	task->stat = SK_TASK_SUSPEND;
	/* then resume it */
	sk_task_resume(task);
	if(sk_thread_current() != SK_NULL)
		sk_schedule();			/* do scheduling */

	return SK_EOK;
}

