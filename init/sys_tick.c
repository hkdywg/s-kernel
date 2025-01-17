/*
 *  sys_tick.c
 *  brief
 *  	system tick timing
 *  
 *  (C) 2025.01.09 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <interrupt.h>
#include <config.h>
#include <hw.h>
#include <timer.h>

#define HW_TIMER_VECTOR_NUM		27

static sk_uint64_t timer_val;
static sk_uint64_t timer_step;
static volatile sk_tick_t sk_tick = 0;

static sk_list_t __timer_list;

/*
 * This function will be called by timer isr
 *
 * @param: none
 */
void sk_tick_increase(void)
{
	++sk_tick;
}

/*
 * This function will be called by timer isr
 *
 * @param: none
 */
sk_tick_t sk_tick_get(void)
{
	return sk_tick;
}

/*
 * timer isr process
 *
 * @param: none
 */
void sk_hw_timer_isr(int vector, void *param)
{
	timer_val += timer_step;
	__asm__ volatile ("msr CNTV_CVAL_EL0, %0"::"r"(timer_val));
	__asm__ volatile ("isb":::"memory");
	
	sk_tick_increase();
}

/*
 * timer init, include timer_isr install and register configure
 *
 * @param: none
 */
int sk_hw_timer_init(void)
{
	sk_hw_interrupt_install(HW_TIMER_VECTOR_NUM, sk_hw_timer_isr, SK_NULL);
	sk_hw_interrupt_umask(HW_TIMER_VECTOR_NUM);

	__asm__ volatile ("msr CNTV_CTL_EL0, %0"::"r"(0));
	__asm__ volatile ("isb 0xf":::"memory");
	__asm__ volatile ("mrs %0, CNTFRQ_EL0" : "=r" (timer_step));
	timer_step /= TICK_PER_SECOND;
	timer_val = timer_step;
	__asm__ volatile ("dsb 0xf":::"memory");

	__asm__ volatile ("msr CNTV_CVAL_EL0, %0"::"r"(timer_val));
	__asm__ volatile ("msr CNTV_CTL_EL0, %0"::"r"(1));

    sk_base_t level;                                                                                                                                                   
    
    //level = hw_interrupt_disable();
    //hw_interrupt_enable(level);
	return 0;
}

/*
 *	sk_timer_create
 *	brief
 *		this function will create a timer
 *	param
 *		name: the name of timer
 *		timeout: the timeout callback function
 *		param: callabck function parameters
 *		time: the tick of timer
 *		flag: the create timer object 
 */
static void  __timer_init(struct sk_sys_timer *timer,
						   void (timeout)(void *param),
						   void *param,
						   sk_tick_t tick,
						   sk_uint8_t flag) 
{
	/* set flag */
	timer->parent.flag = flag;

	timer->timeout_func = timeout;
	timer->param = param;

	timer->timeout_tick = 0;
	timer->init_tick = tick;

	/* initialize timer list */
	sk_list_init(&(timer->row));
}

/*
 * brief: this function will delete timer
 */
void __timer_remove(struct sk_sys_timer *timer)
{
	sk_list_del(&timer->row);
}

/*
 *	sk_timer_create
 *	brief
 *		this function will create a timer
 *	param
 *		name: the name of timer
 *		timeout: the timeout callback function
 *		param: callabck function parameters
 *		tick: the tick of timer
 *		flag: the create timer object 
 */
struct sk_sys_timer* sk_timer_create(const char *name,
									  void (timeout)(void *param),
									  void *param,
									  sk_tick_t tick,
									  sk_uint8_t flag) 
{
	struct sk_sys_timer *timer;

	/* allocate a object */
	timer = (struct sk_object *)sk_object_alloc(SK_OBJECT_TIMER, name);
	if(timer == SK_NULL)
		return SK_NULL;

	__timer_init(timer, timeout, param, tick, flag);

	return timer;
}

/*
 * bref 
 * 		this function will delete a timer
 */
sk_err_t sk_timer_delete(struct sk_sys_timer *timer)
{
	sk_ubase_t level;

	/* disabled interrupt */
	level = hw_interrupt_disable();

	__timer_remove(timer);
	
	/* stop timer */
	timer->parent.flag &= ~SK_TIMER_FLAG_ACTIVE;

	/* enable interrupt */
	hw_interrupt_enable(level);

	sk_object_detach(&(timer->parent));

	return SK_EOK;
}

void sk_system_timer_init(void)
{
	sk_list_init(&__timer_list);
}

/*
 * sk_timer_start
 * brief
 * 		this function will start the timer
 * 	param
 * 		timer: the timer to be started
 */
sk_err_t sk_timer_start(struct sk_sys_timer *timer)
{
	sk_base_t level;
	sk_bool_t need_sched = SK_FALSE;

	/* disable interrupt */
	level = hw_interrupt_disable();

	/* remove timer from list */
	__timer_remove(timer);
	/* get timeout tick */
	timer->timeout_tick = sk_tick_get() + timer->init_tick;
	/* change status of timer */
	timer->parent.flag |= SK_TIMER_FLAG_ACTIVE; 
	/* enable interrupt */
	hw_interrupt_enable(level);

	return SK_EOK;
}


