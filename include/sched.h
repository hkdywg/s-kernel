/*
 *  sched.h
 *  brief
 *  	schedule and task releted definitions of s-kernel 
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

#define SK_NAME_MAX 			(32U)

/*
 * task state definitions
 */
#define SK_TASK_INIT 			(0x00)			/* initialized status */
#define SK_TASK_READY 			(0x01)			/* ready status */
#define SK_TASK_SUSPEND 		(0x02)			/* suspend status */
#define SK_TASK_RUNNING 		(0x04)			/* running status */
#define SK_TASK_CLOSE 			(0x08)			/* close status */

/*
 * task structure
 */
struct task_struct
{
	/* base information */
	char 		name[SK_NAME_MAX];				/* the name of task */
	sk_uint8_t 	stat;							/* task state */
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
	sk_ubase_t 	init_tick;						/* task's initialized tick */
	sk_ubase_t 	remain_tick;					/* remaining tick */
	struct sk_tick	tick;						/* system tick */

	void (*cleanup)(struct task_struct *task);	/* cleanup function when task exit */
	sk_ubase_t 	user_data; 						/* private user data bind this task */
};

#endif
