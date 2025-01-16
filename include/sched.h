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

#include <type_def.h>
#include <tick.h>

#define SK_NAME_MAX 			(32U)

/*
 * thread state definitions
 */
#define SK_thread_INIT 			(0x00)			/* initialized status */
#define SK_thread_READY 		(0x01)			/* ready status */
#define SK_thread_SUSPEND 		(0x02)			/* suspend status */
#define SK_thread_RUNNING 		(0x04)			/* running status */
#define SK_thread_CLOSE 		(0x08)			/* close status */

/*
 * thread structure
 */
struct thread_struct
{
	/* base information */
	char 		name[SK_NAME_MAX];				/* the name of thread */
	sk_uint8_t 	stat;							/* thread state */
	sk_uint8_t 	current_pri;					/* current priority */
	sk_uint8_t 	init_pri;						/* initialized priority */

	/* stack point and entry */
	void 		*sp;							/* stack point */
	void 		*entry;							/* entry */
	void 		*param;							/* parameter */
	void 		*stack_addr;					/* stack address */
	sk_uint32_t stack_size;						/* stack size */

	sk_list_t 	list;							/* the thread list */

	/* tick */
	sk_ubase_t 	init_tick;						/* thread's initialized tick */
	sk_ubase_t 	remain_tick;					/* remaining tick */
	struct sk_sys_tick	tick;					/* system tick */

	void (*cleanup)(struct thread_struct *thread);	/* cleanup function when thread exit */
	sk_ubase_t 	user_data; 						/* private user data bind this thread */
};

#endif
