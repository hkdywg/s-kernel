/*
 *  msg_queue.c
 *  brief
 *  	message queue module of ipc
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

struct sk_mq_message
{
	struct sk_mq_message *next;
};

/*
 * sk_msg_queue_create
 * brief
 * 		this function will create a message queue object(dynamic)
 * param
 * 		name: the name of message queue
 * 		msg_size: number of each message list
 * 		max_msgs: maximum number of messages inn the message queue
 * 		flag: the flag of mailbox, can be set SK_IPC_FLAG_PRIO or SK_IPC_FLAG_FIFO 
 */
struct sk_msg_queue *sk_msg_queue_create(const char *name, sk_size_t msg_size,
									   sk_size_t max_msgs, sk_uint8_t flag)
{
	struct sk_msg_queue *mq;
	struct sk_mq_message *head;

	/* allocate object */
	mq = (struct sk_msg_queue *)sk_object_alloc(SK_OBJECT_MSQUE, name);
	if(mq == SK_NULL)
		return mq;

	/* set mailbox flag */
	mq->parent.parent.flag = flag;

	/* initialize ipc object */
	__ipc_object_init(&(mq->parent));

	mq->msg_size = msg_size;
	mq->max_msgs = max_msgs;
	mq->msg_pool = sk_malloc((mq->msg_size + sizeof(struct sk_mq_message)) * mq->max_msgs);
	if(mq->msg_pool) {
		/* delete message queue object */
		sk_object_delete(&(mq->parent.parent));

		return SK_NULL;
	}
	/* initialize message list */
	mq->msg_queue_head = SK_NULL;
	mq->msg_queue_tail = SK_NULL;
	mq->msg_queue_free = SK_NULL;

	for(sk_ubase_t i = 0; i < mq->max_msgs; i++){
		head = (struct sk_mq_message *)((sk_uint8_t *)mq->msg_pool + 
										i * (mq->msg_size + sizeof(struct sk_mq_message)));
		head->next = (struct sk_mq_message *)mq->msg_queue_free;
		mq->msg_queue_free = head;
	}

	/* the initial entry is zero */
	mq->entry = 0;

	/* initialize an additional list of sender suspend thread */
	sk_list_init(&(mq->suspend_sender_thread));

	return mq;
}


/*
 * sk_msg_queue_init
 * brief
 * 		this function will create a message queue object(static)
 * param
 * 		mq: pointer to message queue 
 * 		name: the name of message queue
 * 		msg_size: number of each message list
 * 		max_msgs: maximum number of messages inn the message queue
 * 		flag: the flag of mailbox, can be set SK_IPC_FLAG_PRIO or SK_IPC_FLAG_FIFO 
 */
struct sk_msg_queue *sk_msg_queue_init(struct sk_msg_queue *mq, const char *name, void *msgpool, 
									   sk_size_t msg_size, sk_size_t pool_size, sk_uint8_t flag)
{
	struct sk_mq_message *head;

	/* set mailbox flag */
	mq->parent.parent.flag = flag;

	/* initialize ipc object */
	__ipc_object_init(&(mq->parent));

	/* set message pool */
	mq->msg_pool = msgpool;

	mq->msg_size = msg_size;
	mq->max_msgs = pool_size / (mq->msg_size + sizeof(struct sk_mq_message));
	/* initialize message list */
	mq->msg_queue_head = SK_NULL;
	mq->msg_queue_tail = SK_NULL;
	mq->msg_queue_free = SK_NULL;

	for(sk_ubase_t i = 0; i < mq->max_msgs; i++){
		head = (struct sk_mq_message *)((sk_uint8_t *)mq->msg_pool + 
										i * (mq->msg_size + sizeof(struct sk_mq_message)));
		head->next = (struct sk_mq_message *)mq->msg_queue_free;
		mq->msg_queue_free = head;
	}

	/* the initial entry is zero */
	mq->entry = 0;

	/* initialize an additional list of sender suspend thread */
	sk_list_init(&(mq->suspend_sender_thread));

	return SK_EOK;
}

/*
 * sk_msg_queue_delete
 * brief
 * 		delete a mailbox object which is created by the sk_msg_queue_create function
 * param
 * 		mq: pointer to message queue object to be deleted
 */
sk_err_t sk_msg_queue_delete(struct sk_msg_queue *mq)
{
	if(mq == SK_NULL)
		return SK_EOK;

	/* wakeup all syspended threads */
	__ipc_list_resume_all(&(mq->parent.suspend_thread));

	/* wakeup all sender syspended threads */
	__ipc_list_resume_all(&(mq->suspend_sender_thread));

	/* free mailbox pool */
	if(mq->msg_pool)
		sk_free(mq->msg_pool);

	/* delete mutex object */
	sk_object_delete(&(mq->parent.parent));

	return SK_EOK;
}


/*
 * sk_msg_queue_send_wait
 * brief
 * 		this function will send a message to the message queue object. if there is a thread suspended on
 * 		the message queue, the thread will be resumed
 * param
 * 		mq: pointer to the message queue object to be sent 
 * 		buffer: the content of the message
 * 		size: the length of message
 * 		timeout: timeout period
 */
sk_err_t sk_msg_queue_send_wait(struct sk_msg_queue *mq,
						 const void *buffer,
						 sk_size_t size,
						 sk_int32_t timeout)
{
	struct sk_thread *thread;
	struct sk_mq_message *msg;
	sk_ubase_t temp;

	/* greater than one message size */
	if(size > mq->msg_size)
		return SK_ERROR;

	/* get current thread */
	thread = sk_current_thread();

	/* disable interrupt */
	temp = hw_interrupt_disable();

	/* get a free list, there must be a empty item */
	msg = (struct sk_mq_message *)mq->msg_queue_free;

	/* for non-blocking call */
	if(msg == SK_NULL && timeout == 0) {
		hw_interrupt_enable(temp);
		return SK_EFULL;
	}

	/* message queue is full */
	if(mq == SK_NULL) {
		/* suspend current thread */
		__ipc_list_suspend(&(mq->suspend_sender_thread),
						   thread,
						   mq->parent.parent.flag);
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

	/* move free list pointer */
	mq->msg_queue_free = msg->next;

	/* enable interrupt */
	hw_interrupt_enable(temp);

	/* the msg is the new tailer of list, the next shall be NULL */
	msg->next = SK_NULL;
	/* copy buffer */
	sk_memcpy(msg + 1, buffer, size);

	/* disable interrupt */
	temp = hw_interrupt_disable();

	/* link msg to message queue */
	if(mq->msg_queue_tail != SK_NULL)
		((struct  sk_mq_message *)mq->msg_queue_tail)->next = msg;
	else
		mq->msg_queue_tail = msg;
	/* if the head is empty, set head */
	if(mq->msg_queue_head == SK_NULL)
		mq->msg_queue_head = msg;
	/* increase message entry */
	mq->entry++;

	/* resume suspended thread */
	if(!sk_list_empty(&mq->parent.suspend_thread)){
		/* get suspended thread */
		thread = sk_list_entry(mq->parent.suspend_thread.next, 
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
 * sk_msg_queue_send
 * brief
 * 		this function will send a message no wait
 */
sk_err_t sk_msg_queue_send(struct sk_msg_queue *mq, const void *buffer, sk_size_t size)
{
	return sk_msg_queue_send_wait(mq, buffer, size, 0);
}

/*
 * sk_msg_queue_recv
 * breif
 * 		this function will received a mail from the mailbox object. if there is no mails inn mailbox
 * 		object, the thread will wait for a specified time
 * param
 * 		mb: pointer to mailbox objcet that you want to received
 * 		value: pointer for receive mailbox
 * 		timeout: timeout period
 */
sk_err_t sk_msg_queue_recv(struct sk_msg_queue *mq, void *buffer, 
						   sk_size_t size, sk_int32_t timeout)
{
	struct sk_thread *thread;
	struct sk_mq_message *msg;
	sk_ubase_t temp;

	/* get current thread */
	thread = sk_current_thread();

	/* disable interrupt */
	temp = hw_interrupt_disable();

	/* for non-blocking call */
	if(mq->entry == 0 && timeout == 0) {
		hw_interrupt_enable(temp);
		return SK_ETIMEOUT;
	}

	/* message queue is empty */
	if(mq->entry == 0){
		/* suspend current thread */
		__ipc_list_suspend(&mq->parent.suspend_thread,
						   thread,
						   mq->parent.parent.flag);

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
	if(mq->entry == 0) 
		return SK_ETIMEOUT;

	/* get message from queue */
	msg = (struct sk_mq_message *)mq->msg_queue_head;

	/* move message queue head */
	mq->msg_queue_head = msg->next;
	/*  reach queue tail, set to NULL */
	if(mq->msg_queue_tail == msg)
		mq->msg_queue_tail = SK_NULL;

	if(mq->entry > 0)
		mq->entry--;

	/* enable interrupt */
	hw_interrupt_enable(temp);

	/* copy message */
	sk_memcpy(buffer, msg + 1, size > mq->msg_size ? mq->msg_size : size);

	/* disable interrupt */
	temp = hw_interrupt_disable();
	/* put message to free list */
	msg->next = (struct sk_mq_message *)mq->msg_queue_free;
	mq->msg_queue_free = msg;

	/* resume suspended thread */
	if(!sk_list_empty(&mq->parent.suspend_thread)){
		/* get suspended thread */
		thread = sk_list_entry(mq->parent.suspend_thread.next, 
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


