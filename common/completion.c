/*                                                                                                                                                                     
 *  completion.c
 *
 *  brif
 *      completion
 *  
 *  (C) 2025.02.10 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <completion.h>

#define SK_COMPLETION 		1
#define SK_UNCOMPLETION 	0


/*
 * sk_completion_init
 * brief
 * 		this function will initialize a completion object
 * param
 * 		cpt: the pointer of a completion object
 */
void sk_completion_init(struct  sk_completion *cpt)
{
	sk_base_t level;

	level = sk_hw_interrupt_disable();
	cpt->flag = SK_UNCOMPLETION;
	sk_list_init(&cpt->suspend_list);
	sk_hw_interrupt_enable(level);
}

/*
 *
 */
sk_err_t sk_completion_wait(struct sk_completion *cpt, sk_int32_t timeout)
{
	sk_err_t ret = SK_EOK;
	sk_base_t level;
	struct sk_thread *thread;

	/* get current thread */
	thread = sk_current_thread();

	level = sk_hw_interrupt_disable();
	if(cpt->flag != SK_COMPLETION) {
		if(timeout == 0) {
			ret = SK_ETIMEOUT;
			sk_hw_interrupt_enable(level);
			return ret;
		} else {
			/* suspend thread */
			sk_thead_suspend(thread);
			/* add to suspend list */
			sk_list_add_tail(&(cpt->suspend_list), &(thread->tlist));
		}
	}
	sk_hw_interrupt_enable(level);
}	
