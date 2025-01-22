/*
 *  sched.h
 *  brief
 *  	schedule and thread releted definitions of s-kernel 
 *  
 *  (C) 2025.01.14 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#ifndef __SCHED_H_
#define __SCHED_H_

#include <base_def.h>
#include <timer.h>

/*
 * thread state definitions
 */
#define SK_THREAD_INIT 			(0x00)			/* initialized status */
#define SK_THREAD_READY 		(0x01)			/* ready status */
#define SK_THREAD_SUSPEND 		(0x02)			/* suspend status */
#define SK_THREAD_RUNNING 		(0x04)			/* running status */
#define SK_THREAD_CLOSE 		(0x08)			/* close status */

/* thread priority */
#define SK_THREAD_PRIORITY_MAX 	(32)			/* support max priority */

/*
 * thread structure
 */
struct sk_thread
{
	/* object information ,data structure is same as struct object */
	char 		name[SK_NAME_MAX];				/* the name of thread */
	sk_uint8_t  type;							/* type of object */
	sk_uint8_t  flags; 							/* thread's flags */
	sk_list_t 	list;							/* the object list */

	sk_list_t 	tlist;							/* the thread list */

	sk_uint8_t 	stat;							/* thread state */
	sk_uint8_t 	current_pri;					/* current priority */
	sk_uint8_t 	init_pri;						/* initialized priority */
	sk_uint32_t number_mask;					

	/* stack point and entry */
	void 		*sp;							/* stack point */
	void 		*entry;							/* entry */
	void 		*param;							/* parameter */
	void 		*stack_addr;					/* stack address */
	sk_uint32_t stack_size;						/* stack size */

	/* tick */
	sk_ubase_t 	init_tick;						/* thread's initialized tick */
	sk_ubase_t 	remain_tick;					/* remaining tick */
	struct sk_sys_timer	tick;					/* system tick */

	void (*cleanup)(struct sk_thread *thread);	/* cleanup function when thread exit */
	sk_ubase_t 	user_data; 						/* private user data bind this thread */
};

/*
 * thread interfaces
 */
struct sk_thread* sk_current_thread(void);
void sk_system_scheduler_init(void);
sk_err_t sk_thread_startup(struct sk_thread *thread);
struct sk_thread *sk_thread_create(const char 			*name,
							   void 				(*entry)(void *param),
							   void 				*param,
							   sk_uint32_t 			stack_size,
							   sk_uint8_t 			priority,
							   sk_uint32_t 			tick);

/*
 * scheduler interfaces
 */
void sk_system_scheduler_init(void);
void sk_system_scheduler_start(void);
void sk_schedule_insert_thread(struct sk_thread *thread);
void sk_schedule_remove_thread(struct sk_thread *thread);
void sk_schedule(void);


#endif
