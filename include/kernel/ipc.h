/*
 *  ipc.h
 *  brief
 *  	ipc communication releted definitions of s-kernel 
 *  
 *  (C) 2025.01.14 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#ifndef __IPC_H_
#define __IPC_H_

#include <base_def.h>
#include <klist.h>
#include <sched.h>
#include <interrupt.h>
#include <hw.h>

/*
 * IPC flags annd control command definitions
 */
#define SK_IPC_FLAG_FIFO 	0x00 		/* FIFO IPC */
#define SK_IPC_FLAG_PRIO 	0x01 		/* PRIO IPC */

#define SK_MUTEX_HOLD_MAX	0xFF 		/* maxium number of mutex */
#define SK_SEM_VALUE_MAX 	0xFFFF 		/* maxium number of semaphore */

#define SK_EVENT_FLAG_AND 	0x01 		/* logic and */
#define SK_EVENT_FLAG_OR 	0x02 		/* logic or */
#define SK_EVENT_FLAG_CLEAR 0x04 		/* clear flag */

/*
 * base structure of IPC object
 */
struct sk_ipc_object
{
	struct sk_object parent;			/* inherit from sk_object */
	sk_list_t 		 suspend_thread;	/* threads pended on this resource */
};


/*
 * mutual exclusion (mutex) structure
 */
struct sk_mutex
{
	struct sk_ipc_object parent;		/* inherit from ipc_object */

	sk_uint16_t 		 value;			/* value of mutex */

	sk_uint8_t 			 original_pri;	/* priority of last thread hold the mutex */
	sk_uint8_t 			 hold; 			/* numbers of thread hold the mutex */

	struct sk_thread 	 *owner;		/* current owner of mutex */
};

/*
 * semaphore structure
 */
struct sk_sem
{
	struct sk_ipc_object parent;		/* inherit from ipc_object */

	sk_uint16_t 		 value;			/* value of mutex */
};

/*
 * event structure
 */
struct sk_event
{
	struct sk_ipc_object parent;		/* inherit from ipc_object */

	sk_uint32_t 		 set;			/* event set */
};

/*
 * mailbox structure 
 */
struct sk_mailbox
{
	struct sk_ipc_object parent;		/* inherit from ipc_object */

	sk_ubase_t 		*msg_pool;			/* start address of message buffer */
	sk_uint16_t 	size;				/* size of message pool */

	sk_uint16_t		entry;				/* index of messages in msg_pool */
	sk_uint16_t 	in_offset;			/* input offset of  the message buffer */
	sk_uint16_t 	out_offset;			/* output offset of message buffer */

	sk_list_t 		suspend_sender_thread; 	/*  sender thread  suspended on this mailbox */
};

/*
 * message queue structure
 */
struct sk_msg_queue
{
	struct sk_ipc_object parent;		/* inherit from ipc_object */

	sk_ubase_t 		*msg_pool;			/* start address of message queue */
	sk_uint16_t 	msg_size;			/* message size of message */
	sk_uint16_t 	max_msgs; 			/* max number of messages */

	sk_uint16_t 	entry;				/*  index of messages in the queue */

	void 			*msg_queue_head;	/* list head */
	void 			*msg_queue_tail; 	/* list tail */
	void 			*msg_queue_free;	/* pointer indicated the free node of queue */

	sk_list_t 		suspend_sender_thread;	/* sender thread suspended on this message queue */
};

/* mutex relative interface */
sk_err_t sk_mutex_init(struct sk_mutex *mutex, const char *name, sk_uint8_t flag);
struct sk_mutex *sk_mutex_create(const char *name, sk_uint8_t flag);
sk_err_t sk_mutex_delete(struct sk_mutex *mutex);
sk_err_t sk_mutex_lock(struct sk_mutex *mutex, sk_int32_t time);
sk_err_t sk_mutex_trylock(struct sk_mutex *mutex);
sk_err_t sk_mutex_unlock(struct sk_mutex *mutex);


/* semaphore relative interface */
sk_err_t sk_sem_init(struct sk_sem *sem, const char *name, 
					 sk_uint16_t value, sk_uint8_t flag);
struct sk_sem *sk_sem_create(const char *name, sk_uint16_t value, sk_uint8_t flag);
sk_err_t sk_sem_destroy(struct sk_sem *sem);
sk_err_t sk_sem_wait(struct sk_sem *sem, sk_int32_t time);
sk_err_t sk_sem_trywait(struct sk_sem *sem);
sk_err_t sk_sem_post(struct sk_sem *sem);

/* event relative interface */
struct sk_event *sk_event_create(const char *name, sk_uint8_t flag);
sk_err_t sk_event_init(struct sk_event *event, const char *name, sk_uint8_t flag);
sk_err_t sk_event_destroy(struct sk_event *event);
sk_err_t sk_event_send(struct sk_event *event, sk_uint16_t set);
sk_err_t sk_event_recv(struct sk_event *event, sk_uint32_t set, 
					   sk_uint8_t option, sk_int32_t timeout, sk_uint32_t *recved);

/* mailbox relative interface */
struct sk_mailbox *sk_mailbox_create(const char *name, sk_size_t size, sk_uint8_t flag);
sk_err_t sk_mailbox_init(struct sk_mailbox *mb, const char *name, void *msgpool, sk_size_t size, sk_uint8_t flag);
sk_err_t sk_mailbox_delete(struct sk_mailbox *mb);
sk_err_t sk_mailbox_send_wait(struct sk_mailbox *mb, sk_ubase_t value, sk_int32_t timeout);
sk_err_t sk_mailbox_send(struct sk_mailbox *mb, sk_ubase_t value);
sk_err_t sk_mailbox_recv(struct sk_mailbox *mb, sk_ubase_t *value, sk_int32_t timeout);

/* message queue interface */
struct sk_msg_queue *sk_msg_queue_create(const char *name, sk_size_t msg_size, sk_size_t max_msgs, sk_uint8_t flag);
struct sk_msg_queue *sk_msg_queue_init(struct sk_msg_queue *mq, const char *name, void *msgpool, sk_size_t msg_size, sk_size_t pool_size, sk_uint8_t flag);
sk_err_t sk_msg_queue_delete(struct sk_msg_queue *mq);
sk_err_t sk_msg_queue_send_wait(struct sk_msg_queue *mq, const void *buffer, sk_size_t size, sk_int32_t timeout);
sk_err_t sk_msg_queue_send(struct sk_msg_queue *mq, const void *buffer, sk_size_t size);
sk_err_t sk_msg_queue_recv(struct sk_msg_queue *mq, void *buffer, sk_size_t size, sk_int32_t timeout);

#endif
