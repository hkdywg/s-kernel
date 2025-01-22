/*
 *  schedule.c
 *  brief
 *  	thread schedule 
 *  
 *  (C) 2025.01.20 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <interrupt.h>
#include <skernel.h>
#include <hw.h>
#include <klist.h>
#include <sched.h>

/* schedule management data structure */
sk_list_t sk_thread_prio_table[SK_THREAD_PRIORITY_MAX];
struct sk_thread *current_thread = SK_NULL;
sk_uint32_t sk_thread_ready_prio_group;

/*
 * __schedule_get_hp_thread
 * brief
 * 		find the highest priority ready thread
 * param
 * 		highest_prio: the highest priority
 */
static struct sk_thread *__schedule_get_hp_thread(sk_ubase_t *highest_prio)
{
	struct sk_thread *thread;
	sk_ubase_t ready_prio;

	ready_prio = __sk_ffs(sk_thread_ready_prio_group) - 1;

	/* get highest ready priority thread */
	thread = sk_list_entry(sk_thread_prio_table[ready_prio].next,
						   struct sk_thread, tlist);
	*highest_prio = ready_prio;

	return thread;
}

/*
 * sk_schedule_remove_thread
 * brief
 * 		remove thread from ready list
 * param
 * 		thread: the thread of need to be removed
 */
void sk_schedule_remove_thread(struct sk_thread *thread)
{
	sk_base_t level;

	/* disable interrupt */
	level = hw_interrupt_disable();

	/* remove thread from ready list */
	sk_list_del(&(thread->tlist));
	if(sk_list_empty(&(sk_thread_prio_table[thread->current_pri]))) {
		sk_thread_ready_prio_group &= ~thread->number_mask;
	}
	/* enable interrupt */
	hw_interrupt_enable(level);
}

/*
 * sk_schedule_insert_thread
 * brief
 * 		this function will insert a thread to schedule ready list
 * param:
 * 		thread: need to be inserted thread
 */
void sk_schedule_insert_thread(struct sk_thread *thread)
{
	sk_base_t level;

	/* disable interrupt */
	level = hw_interrupt_disable();
	
	/* if thread is current running thread, break */
	if(thread == current_thread) {
		thread->stat = SK_THREAD_RUNNING;
		return;
	}

	/* set thread stat to be ready */
	thread->stat = SK_THREAD_READY;
	/* insert it to list head tail */
	sk_list_add_tail(&(sk_thread_prio_table[thread->current_pri]), &(thread->tlist));

	/* enable interrupt */
	hw_interrupt_enable(level);
}

/*
 * sk_system_schedule_init
 * brief
 * 		initialize system thread scheduler
 */
void sk_system_scheduler_init(void)
{
	sk_base_t level, index;

	/* disable interrupt */
	level = hw_interrupt_disable();

	for(index = 0; index < SK_THREAD_PRIORITY_MAX; index++) {
		sk_list_init(&sk_thread_prio_table[index]);
	}

	/* enable interrupt */
	hw_interrupt_enable(level);
}

/*
 * sk_schedule
 * brief
 * 		select one thread with the highest priority, and switch to it 
 */
void sk_schedule(void)
{
	sk_base_t level;
	sk_ubase_t ready_hp_prio;
	struct sk_thread *to_thread, *from_thread;

	/* disable interrupt */
	level = hw_interrupt_disable();

	if(sk_thread_ready_prio_group != 0) {
		to_thread = __schedule_get_hp_thread(&ready_hp_prio);		
		if(current_thread->stat == SK_THREAD_RUNNING) {
			/* preemption if ready thread's priority lower then curent thread */
			if(current_thread->current_pri <= ready_hp_prio)
				to_thread = current_thread;
		}
		/* if the destination thread is not same as current thread */
		if(current_thread != to_thread) {
			from_thread  = current_thread;
			current_thread = to_thread;
			/* insert thread to ready list */
			sk_schedule_insert_thread(from_thread);
			/* remove thread from ready list */
			sk_schedule_remove_thread(to_thread);
			/* change thread status */
			to_thread->stat = SK_THREAD_RUNNING;

			/* thread context switch */
			hw_context_switch((sk_ubase_t)&from_thread->sp,
								 (sk_ubase_t)&to_thread->sp);
		}
	}

	/* enable interrupt */
	hw_interrupt_enable(level);
}

/*
 * sk_system_scheduler_start
 * brief
 * 		startup the scheduler
 */
void sk_system_scheduler_start(void)
{
	struct sk_thread *to_thread;
	sk_ubase_t prio;

	to_thread = __schedule_get_hp_thread(&prio);

	/* set current thread */
	current_thread = to_thread;
	/* remove thread from ready list */
	sk_schedule_remove_thread(to_thread);
	/* change thread status to RUNNING */
	to_thread->stat = SK_THREAD_RUNNING;

	/* set thread context */
	hw_context_switch_to((sk_ubase_t)&to_thread->sp);
	/* never come back */
}










