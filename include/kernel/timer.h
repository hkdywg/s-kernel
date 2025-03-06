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

#define SK_TIMER_CTRL_GET_TIME 		(0x00)
#define SK_TIMER_CTRL_SET_TIME 		(0x01)
#define SK_TIMER_CTRL_SET_ONSHOT 	(0x02)
#define SK_TIMER_CTRL_SET_PERIODIC 	(0x04)
#define SK_TIMER_CTRL_GET_STATE 	(0x08)

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

void sk_system_timer_init(void);
sk_err_t sk_timer_delete(struct sk_sys_timer *timer);
sk_err_t sk_timer_start(struct sk_sys_timer *timer);
sk_err_t sk_timer_stop(struct sk_sys_timer *timer);
sk_err_t sk_timer_control(struct sk_sys_timer *timer, int cmd, void *arg);
sk_tick_t sk_tick_from_ms(sk_uint32_t ms);
struct sk_sys_timer* sk_timer_create(const char *name,
									  void (timeout)(void *param),
									  void *param,
									  sk_tick_t tick,
									  sk_uint8_t flag);

void sk_timer_init(struct sk_sys_timer *timer, const char *name,
				   void (timeout)(void *param), void *param,
				   sk_tick_t tick, sk_uint8_t flag);
#endif
