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


/* mutex relative interface */
sk_err_t sk_mutex_init(struct sk_mutex *mutex, const char *name, sk_uint8_t flag);
struct sk_mutex *sk_mutex_create(const char *name, sk_uint8_t flag);
sk_err_t sk_mutex_delete(struct sk_mutex *mutex);
sk_err_t sk_mutex_take(struct sk_mutex *mutex, sk_int32_t time);
sk_err_t sk_mutex_trytake(struct sk_mutex *mutex);
sk_err_t sk_mutex_release(struct sk_mutex *mutex);



#endif
