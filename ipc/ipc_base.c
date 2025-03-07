/*
 *  ipc_base.c
 *  brief
 *  	basic common ipc related funtion definition
 *  
 *  (C) 2025.03.07 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <ipc.h>

/*
 * __ipc_object_init
 * brief 
 * 		this function will initialize a ipc object, such as mutex, semaphore, messagequeue and mailbox
 * param
 * 		ipc: the pointer to ipc object
 */
sk_inline sk_err_t __ipc_object_init(struct sk_ipc_object *ipc)
{
	/* initialize ipc object */
	sk_list_init(&(ipc->suspend_thread));

	return SK_EOK;
}

/*
 *
 */
sk_inline sk_err_t __ipc_list_resume_all(sk_list_t *list)
{
	struct sk_thread *thread;
	sk_ubase_t temp;

	/* wakeup all suspended threads */
	while(!sk_list_empty(list)) {
		/* disable interrupt */	
		temp = sk_hw_interrupt_disable();

		/* get next suspended thread */
		thread = sk_list_entry(list->next, struct sk_thread, tlist); 

		/* resume thread */
		sk_thread_resume(thread);

		/* enable interrupt */
		sk_hw_interrupt_enable(temp);
	}

	return SK_EOK;
}

/*
 * __ipc_list_suspend
 * brief 
 * 		this function will suspend a thread to a IPC object list
 * param
 * 		list: pointer to a suspended thread list of IPC object
 * 		thread: thread object to be suspended
 * 		flag: flag for thread object to be suspended
 */
sk_inline sk_err_t __ipc_list_suspend(sk_list_t *list, 
									  struct sk_thread *thread,
									  sk_uint8_t flag)
{
	/* suspend thread */
	sk_thread_suspend(thread);

	switch(flag) {
		case SK_IPC_FLAG_FIFO:
			sk_list_add_tail(list, &(thread->tlist));
			break;
		case SK_IPC_FLAG_PRIO:
			struct sk_list_node *n;
			struct sk_thread *t;
			/* find a suitable position */
			for(n = list->next; n != list; n = n->next) {
				t = sk_list_entry(n, struct sk_thread, tlist);
				/* find out */
				if(thread->current_pri < t->current_pri) {
					sk_list_add_tail(&(t->tlist), &(thread->tlist));
					break;
				}
			}

			/* not find a suitable position */
			if(n == list)
				sk_list_add_tail(list, &(thread->tlist));
			break;
		default:
			break;
	}

	return SK_EOK;
}

