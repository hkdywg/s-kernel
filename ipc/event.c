/*
 *  event.c
 *  brief
 *  	event module of ipc
 *  
 *  (C) 2025.03.12 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <ipc.h>

extern sk_err_t __ipc_list_resume_all(sk_list_t *list);
extern sk_err_t __ipc_object_init(struct sk_ipc_object *ipc);
extern sk_err_t __ipc_list_suspend(sk_list_t *list, 
									  struct sk_thread *thread,
									  sk_uint8_t flag);

/*
 * sk_event_create
 * brief
 * 		this function will create a event object
 * param
 * 		name: the name of event
 * 		flag: the flag of event, can be set SK_IPC_FLAG_PRIO or SK_IPC_FLAG_FIFO 
 */
struct sk_event *sk_event_create(const char *name, sk_uint8_t flag)
{
	struct sk_event *event;

	/* allocate object */
	event = (struct sk_event *)sk_object_alloc(SK_OBJECT_EVENT, name);
	if(event == SK_NULL)
		return event;

	/* initialize ipc object */
	__ipc_object_init(&(event->parent));

	event->set = 0;

	/* set event flag */
	event->parent.parent.flag = flag;

	return event;
}

/*
 * sk_event_init
 * brief
 * 		this function will init a event object
 * param
 * 		event: opinter to the event to initialize
 * 		name: the name of event
 * 		flag: event flag, can be set SK_IPC_FLAG_FIFO or SK_IPC_FLAG_PRIO
 */
sk_err_t sk_event_init(struct sk_event *event, const char *name, 
					 sk_uint8_t flag)
{
	/* initialize object */
	sk_object_init(&(event->parent.parent), SK_OBJECT_EVENT, name);

	/* initialize ipc object */
	__ipc_object_init(&(event->parent));

	event->set = 0;

	/* set parent flag */
	event->parent.parent.flag = flag;

	return SK_EOK;
}

/*
 * sk_event_destroy
 * brief
 * 		destroy a event object which is created by the sk_event_create function
 * param
 * 		event: pointer to event object to be deleted
 */
sk_err_t sk_event_destroy(struct sk_event *event)
{
	if(event == SK_NULL)
		return SK_EOK;

	/* wakeup all syspended threads */
	__ipc_list_resume_all(&(event->parent.suspend_thread));

	/* delete event object */
	sk_object_delete(&(event->parent.parent));

	return SK_EOK;
}

/*
 * sk_event_send
 * brief
 * 		this function will send an event to the event object. if there is a thread
 * 		suspended on the event, the thread will be resumed
 * param
 * 		event: a pointer to the event object to be sent
 * 		set: a flag that you will set for this event's flag
 */
sk_err_t sk_event_send(struct sk_event *event, sk_uint16_t set)
{
	struct sk_list_node *n;
	struct sk_thread *thread;
	sk_ubase_t level;
	sk_ubase_t status;
	sk_bool_t need_schedule;

	if(set == 0)
		return SK_ERROR;

	/* disable interrupt */	
	level = hw_interrupt_disable();

	/* set event */
	event->set |= set;

	if(!sk_list_empty(&event->parent.suspend_thread)) {
		/* search thread list to resume thread */
		n = event->parent.suspend_thread.next;
		thread = sk_list_entry(n, struct sk_thread, tlist);

		status = SK_ERROR;

		if(thread->event_info & SK_EVENT_FLAG_AND) {
			if((thread->event_set & event->set) == thread->event_set)
				/* received an AND event */
				status = SK_EOK;
		} else if(thread->event_info & SK_EVENT_FLAG_OR) {
			if(thread->event_set & event->set) {
				/* save the received event set */
				thread->event_set = thread->event_set & event->set;

				/* received an OR event */
				status = SK_EOK;
			}
		} else {
			/* enable interrupt */
			hw_interrupt_enable(level);

			return SK_EINVAL;
		}

		/* move node to the next */
		n = n->next;
		if(status == SK_EOK) {
			/* clear event */
			if(thread->event_info & SK_EVENT_FLAG_CLEAR) 
				event->set &= ~thread->event_set;	
			/* resume thread */
			sk_thread_resume(thread);

			/* need do a schedule */
			need_schedule = SK_TRUE;
		}
	}

	/* enable interrupt */
	hw_interrupt_enable(level);

	if(need_schedule == SK_TRUE)
		sk_schedule();

	return SK_EOK;
}

/*
 * sk_event_recv
 * brief
 * 		this function will receive an event from event object. if the event is unavailable,
 * 		the thread shall wait for the event up to a spefified name
 * param
 * 		event: pointer to the event object to be received
 * 		set: that you will set for this event's flag
 * 		option: the option of this receiving event
 * 		timeout: timeout 
 * 		recved: a pointer to the received event
 */
sk_err_t sk_event_recv(struct sk_event *event, sk_uint32_t set, 
					   sk_uint8_t option, sk_int32_t timeout, sk_uint32_t *recved)
{
	struct sk_thread *thread;
	sk_ubase_t level;
	sk_ubase_t status;

	if(set == 0)
		return SK_ERROR;

	/* initialize status */
	status = SK_ERROR;
	
	/* get current thread */
	thread = sk_current_thread();

	/* disable interrupt */	
	level = hw_interrupt_disable();

	/* check event set */
	if(option & SK_EVENT_FLAG_AND) {
		if((event->set & set) == set)
			status = SK_EOK;
	} else if(option & SK_EVENT_FLAG_OR) {
		if(event->set & set)
			status = SK_EOK;
	} else {
		/* enable interrupt */
		hw_interrupt_enable(level);

		return SK_EINVAL;
	}

	if(status == SK_EOK) {
		/* set received event */
		if(recved)
			*recved = (event->set & set);
		/* fill thread event info */
		thread->event_set = (event->set & set);
		thread->event_info = option;

		/* received event */
		if(option & SK_EVENT_FLAG_CLEAR)
			event->set &= ~set;
	} else if(timeout == 0) {
		/* no waiting */

		/* enable interrupt */
		hw_interrupt_enable(level);

		return SK_ETIMEOUT;
	} else {
		/* fill thread event info */
		thread->event_set = set;
		thread->event_info = option;

		/* put thread to event suspend list */
		__ipc_list_suspend(&(event->parent.suspend_thread),
						   thread,
						   event->parent.parent.flag);
		if(timeout > 0) {
			/* reset the timeout thread timer and start it */
			sk_timer_control(&(thread->thread_timer), SK_TIMER_CTRL_SET_TIME, &timeout);
			sk_timer_start(&(thread->thread_timer));
		}

		/* enable interrupt */
		hw_interrupt_enable(level);

		/* do schedule */
		sk_schedule();

		/* disable interrupt */	
		level = hw_interrupt_disable();

		/* set received event */
		if(recved)
			*recved = thread->event_set;
	}

	/* enable interrupt */
	hw_interrupt_enable(level);

	return SK_EOK;
}

