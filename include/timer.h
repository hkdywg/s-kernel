/*
 *  timer.h
 *  brief
 *  	s-kernel system timer definitions
 *  
 *  (C) 2025.01.15 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef __TIMER_H_
#define __TIMER_H_

#include <base_def.h>
#include <klist.h>
#include <kobj.h>

#define SK_TIMER_FLAG_DEACTIVE 		(0x0)		/* timer is deactive */
#define SK_TIMER_FLAG_ACTIVE 		(0x1)		/* timer is active */
#define SK_TIMER_FLAG_ONE_SHOT 		(0x0)		/* one shot timer */
#define SK_TIMER_FLAG_PERIODIC 		(0x2)		/* periodic timer */

/*
 * system timer structure
 */
struct sk_sys_timer
{
	struct sk_object parent;				/* inherit from sk_object */
	sk_list_t list;							/* embedded list */

	void (*timeout_func)(void *param);		/* timeout function */
	void *param;							/* timeout function's parameter */

	sk_tick_t 	init_tick;					/* system time timeout tick */
	sk_tick_t	timeout_tick;				/* timeout tick */ 	
};

#endif
