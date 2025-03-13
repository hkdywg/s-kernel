/*
 *  mailbox.c
 *  brief
 *  	mailbox ipc related funtion definition
 *  
 *  (C) 2025.03.13 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <ipc.h>
#include <skernel.h>

extern sk_err_t __ipc_list_resume_all(sk_list_t *list);
extern sk_err_t __ipc_object_init(struct sk_ipc_object *ipc);
extern sk_err_t __ipc_list_suspend(sk_list_t *list, 
									  struct sk_thread *thread,
									  sk_uint8_t flag);

/*
 * sk_mailbox_create
 * brief
 * 		this function will create a mailbox object(dynamic)
 * param
 * 		name: the name of mailbox
 * 		size: number of mails in the mailbox
 * 		flag: the flag of mailbox, can be set SK_IPC_FLAG_PRIO or SK_IPC_FLAG_FIFO 
 */
struct sk_mailbox *sk_mailbox_create(const char *name, sk_size_t size, sk_uint8_t flag)
{
	struct sk_mailbox *mb;

	/* allocate object */
	mb = (struct sk_mailbox *)sk_object_alloc(SK_OBJECT_MAILBOX, name);
	if(mb == SK_NULL)
		return mb;

	/* set mailbox flag */
	mb->parent.parent.flag = flag;

	/* initialize ipc object */
	__ipc_object_init(&(mb->parent));

	mb->size = size;
	mb->msg_pool = (sk_ubase_t *)sk_malloc(mb->size * sizeof(sk_ubase_t)); 
	if(mb->msg_pool) {
		/* delete mailbox object */
		sk_object_delete(&(mb->parent.parent));

		return SK_NULL;
	}
	mb->entry 		= 0;
	mb->in_offset 	= 0;
	mb->out_offset 	= 0;

	/* initialize an additional list of sender suspend thread */
	sk_list_init(&(mb->suspend_sender_thread));

	return mb;
}

/*
 * sk_mailbox_init
 * brief
 * 		this function will create a mailbox object(static)
 * param
 * 		mb: pointer to mailbox object that to be initialized
 * 		name: the name of mailbox
 * 		msgpool: mailbox buffer address
 * 		size: number of mails in the mailbox
 * 		flag: the flag of mailbox, can be set SK_IPC_FLAG_PRIO or SK_IPC_FLAG_FIFO 
 */
sk_err_t sk_mailbox_init(struct sk_mailbox *mb, const char *name, 
								   void *msgpool, sk_size_t size, sk_uint8_t flag)
{
	/* initialize object */
	sk_object_init(&(mb->parent.parent), SK_OBJECT_MAILBOX, name);

	/* set mailbox flag */
	mb->parent.parent.flag = flag;

	/* initialize ipc object */
	__ipc_object_init(&(mb->parent));

	mb->size 		= size;
	mb->msg_pool 	= (sk_ubase_t *)msgpool;
	mb->entry 		= 0;
	mb->in_offset 	= 0;
	mb->out_offset 	= 0;

	/* initialize an additional list of sender suspend thread */
	sk_list_init(&(mb->suspend_sender_thread));

	return SK_EOK;
}

/*
 * sk_mailbox_delete
 * brief
 * 		delete a mailbox object which is created by the sk_mailbox_create function
 * param
 * 		mb: pointer to mailbox object to be deleted
 */
sk_err_t sk_mailbox_delete(struct sk_mailbox *mb)
{
	if(mb == SK_NULL)
		return SK_EOK;

	/* wakeup all syspended threads */
	__ipc_list_resume_all(&(mb->parent.suspend_thread));

	/* wakeup all sender syspended threads */
	__ipc_list_resume_all(&(mb->suspend_sender_thread));

	/* free mailbox pool */
	if(mb->msg_pool)
		sk_free(mb->msg_pool);

	/* delete mutex object */
	sk_object_delete(&(mb->parent.parent));

	return SK_EOK;
}

/*
 * sk_mailbox_send_wait
 * brief
 * 		this function will send a mail to the mailbox object. if there is a thread suspended on
 * 		the mailbox, the thread will be resumed
 * param
 * 		mb: pointer to the mailbox object to be sent 
 * 		value: the content of the mail you want to send
 * 		timeout: timeout period
 */
sk_err_t sk_mailbox_send_wait(struct sk_mailbox *mb,
						 sk_ubase_t value,
						 sk_int32_t timeout)
{
	struct sk_thread *thread;
	sk_ubase_t temp;

	/* get current thread */
	thread = sk_current_thread();

	/* disable interrupt */
	temp = hw_interrupt_disable();

	/* for non-blocking call */
	if(mb->entry == mb->size && timeout == 0) {
		hw_interrupt_enable(temp);
		return SK_EFULL;
	}

	/* mailbox is full */
	if(mb->entry == mb->size) {
		/* suspend current thread */
		__ipc_list_suspend(&(mb->suspend_sender_thread),
						   thread,
						   mb->parent.parent.flag);
		/* has waiting time, start thread timer */
		if(timeout > 0) {
			/* reset the timeout of thread timer and start it */
			sk_timer_control(&(thread->thread_timer),
							 SK_TIMER_CTRL_SET_TIME,
							 &timeout);
			sk_timer_start(&(thread->thread_timer));
		}

		/* enable interrupt */
		hw_interrupt_enable(temp);
	
		/* do schedule */
		sk_schedule();
	}

	/* thread timer timeout and thread rerun, check mailbox is full */
	if(mb->entry == mb->size) 
		return SK_EFULL;

	/* set ptr */
	mb->msg_pool[mb->in_offset] = value;
	++mb->in_offset;
	if(mb->in_offset >= mb->size)
		mb->in_offset = 0;
	if(mb->entry < mb->size){
		mb->entry++;
	} else{
		hw_interrupt_enable(temp);
		return SK_EFULL;
	}
	/* resume suspended thread */
	if(!sk_list_empty(&mb->parent.suspend_thread)){
		/* get suspended thread */
		thread = sk_list_entry(mb->parent.suspend_thread.next, 
							   struct sk_thread,
							   tlist); 
		/* resume thread */
		sk_thread_resume(thread);

		/* enable interrupt */
		hw_interrupt_enable(temp);
	
		/* do schedule */
		sk_schedule();

		return SK_EOK;
	}
	/* enable interrupt */
	hw_interrupt_enable(temp);

	return SK_EOK;
}

/*
 * sk_mailbox_send
 * brief
 * 		this function will send a mail to the mailbox object. if there is a thread suspended on
 * 		the mailbox, the thread will be resumed
 * param
 * 		mb: pointer to the mailbox object to be sent 
 * 		value: the content of the mail you want to send
 */
sk_err_t sk_mailbox_send(struct sk_mailbox *mb, sk_ubase_t value)
{
	return sk_mailbox_send_wait(mb, value, 0);
}

/*
 * sk_mailbox_recv
 * breif
 * 		this function will received a mail from the mailbox object. if there is no mails inn mailbox
 * 		object, the thread will wait for a specified time
 * param
 * 		mb: pointer to mailbox objcet that you want to received
 * 		value: pointer for receive mailbox
 * 		timeout: timeout period
 */
sk_err_t sk_mailbox_recv(struct sk_mailbox *mb, sk_ubase_t *value, sk_int32_t timeout)
{
	struct sk_thread *thread;
	sk_ubase_t temp;

	/* get current thread */
	thread = sk_current_thread();

	/* disable interrupt */
	temp = hw_interrupt_disable();

	/* for non-blocking call */
	if(mb->entry == 0 && timeout == 0) {
		hw_interrupt_enable(temp);
		return SK_ETIMEOUT;
	}

	/* mailbox is empty */
	if(mb->entry == 0){
		/* suspend current thread */
		__ipc_list_suspend(&mb->parent.suspend_thread,
						   thread,
						   mb->parent.parent.flag);

		/* has waiting time, start thread timer */
		if(timeout > 0) {
			/* reset the timeout of thread timer and start it */
			sk_timer_control(&(thread->thread_timer),
							 SK_TIMER_CTRL_SET_TIME,
							 &timeout);
			sk_timer_start(&(thread->thread_timer));
		}

		/* enable interrupt */
		hw_interrupt_enable(temp);
	
		/* do schedule */
		sk_schedule();
	}

	/* thread timer timeout and thread rerun, check mailbox is empty */
	if(mb->entry == 0) 
		return SK_ETIMEOUT;

	/* get mails */
	*value = mb->msg_pool[mb->out_offset];
	/* increase output offset */
	++mb->out_offset;
	if(mb->out_offset >= mb->size)
		mb->out_offset = 0;

	if(mb->entry > 0)
		mb->entry--;

	/* resume suspended thread */
	if(!sk_list_empty(&mb->parent.suspend_thread)){
		/* get suspended thread */
		thread = sk_list_entry(mb->parent.suspend_thread.next, 
							   struct sk_thread,
							   tlist); 
		/* resume thread */
		sk_thread_resume(thread);

		/* enable interrupt */
		hw_interrupt_enable(temp);
	
		/* do schedule */
		sk_schedule();

		return SK_EOK;
	}
	/* enable interrupt */
	hw_interrupt_enable(temp);

	return SK_EOK;
}





