/*
 *  mutex.c
 *  brief
 *  	mutex module of ipc
 *  
 *  (C) 2025.03.07 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <ipc.h>

extern sk_err_t __ipc_list_resume_all(sk_list_t *list);
extern sk_err_t __ipc_object_init(struct sk_ipc_object *ipc);
extern sk_err_t __ipc_list_suspend(sk_list_t *list, 
									  struct sk_thread *thread,
									  sk_uint8_t flag);

/*
 * sk_mutex_init
 * brief
 * 		this function will init a mutex object
 * param
 * 		name: the name of mutex
 * 		flag: 
 */
sk_err_t sk_mutex_init(struct sk_mutex *mutex, const char *name, sk_uint8_t flag)
{
	/* eliminate compile warning */
	(void)flag;

	/* initialize object */
	sk_object_init(&(mutex->parent.parent), SK_OBJECT_MUTEX, name);

	/* initialize ipc object */
	__ipc_object_init(&(mutex->parent));

	mutex->value 		= 1;
	mutex->owner 		= SK_NULL;
	mutex->original_pri = 0xFF;
	mutex->hold 		= 0;

	/* flag can only be SK_IPC_FLAG_PRIO */
	mutex->parent.parent.flag = SK_IPC_FLAG_PRIO;

	return SK_EOK;
}
/*
 * sk_mutex_create
 * brief
 * 		this function will create a mutex object
 * param
 * 		name: the name of mutex
 * 		flag: 
 */
struct sk_mutex *sk_mutex_create(const char *name, sk_uint8_t flag)
{
	struct sk_mutex *mutex;
	/* eliminate compile warning */
	(void)flag;

	/* allocate object */
	mutex = (struct sk_mutex *)sk_object_alloc(SK_OBJECT_MUTEX, name);
	if(mutex == SK_NULL)
		return mutex;

	/* initialize ipc object */
	__ipc_object_init(&(mutex->parent));

	mutex->value 		= 1;
	mutex->owner 		= SK_NULL;
	mutex->original_pri = 0xFF;
	mutex->hold 		= 0;

	/* flag can only be SK_IPC_FLAG_PRIO */
	mutex->parent.parent.flag = SK_IPC_FLAG_PRIO;

	return mutex;
}

/*
 * sk_mutex_delete
 * brief
 * 		delete a mutex object which is created by the sk_mutex_create function
 * param
 * 		mutex: pointer to mutex object to be deleted
 */
sk_err_t sk_mutex_delete(struct sk_mutex *mutex)
{
	if(mutex == SK_NULL)
		return SK_EOK;

	/* wakeup all syspended threads */
	__ipc_list_resume_all(&(mutex->parent.suspend_thread));

	/* delete mutex object */
	sk_object_delete(&(mutex->parent.parent));

	return SK_EOK;
}


/*
 * sk_mutex_lock
 * brief
 * 		this function will take a mutex, if the mutex is unavailable, the thread shall
 * 		wait for the mutex up to specified time
 * param
 * 		mutex: pointer to mutex
 * 		time: time out period
 */
sk_err_t sk_mutex_lock(struct sk_mutex *mutex, sk_int32_t time)
{
	struct sk_thread *thread;
	sk_ubase_t temp;

	/* get current thread */
	thread = sk_current_thread();

	/* disable interrupt */	
	temp = hw_interrupt_disable();

	if(mutex->owner == thread) {
		if(mutex->hold < SK_MUTEX_HOLD_MAX){
			mutex->hold++;
		} else {
			/* enable interrupt */
			hw_interrupt_enable(temp);
			return SK_EFULL;
		}
	} else {
		/* the value of mutex is 1 in initial status. therefore, if the value
		 * is great thann 0, it indicates the mutex is avaible */
		if(mutex->value > 0) {
			/* mutex is available */
			mutex->value --;

			/* set mutex owner and original priority */
			mutex->owner = thread;
			mutex->original_pri = thread->current_pri;
			if(mutex->hold < SK_MUTEX_HOLD_MAX){
				mutex->hold++;
			} else {
				/* enable interrupt */
				hw_interrupt_enable(temp);
				return SK_EFULL;
			}
		} else {
			/* no waiting, return with timeout */
			if(time == 0) {
				/* enable interrupt */
				hw_interrupt_enable(temp);
				return SK_ETIMEOUT;
			} else {
				/* suspend current thread */
				__ipc_list_suspend(&(mutex->parent.suspend_thread), thread, SK_IPC_FLAG_PRIO);

				if(time > 0) {
					/* reset the timeout thread timer and start it */
					sk_timer_control(&(thread->thread_timer), SK_TIMER_CTRL_SET_TIME, &time);
					sk_timer_start(&(thread->thread_timer));
				}

				/* enable interrupt */
				hw_interrupt_enable(temp);

				/* do schedule */
				sk_schedule();

				return SK_EOK;
			}
		}
	}
	
	/* enable interrupt */
	hw_interrupt_enable(temp);

	return SK_EOK;
}


/*
 * sk_mutex_trytake
 * brief
 * 		this function will try to take a mutex. if the mutex is unavailable, the thread
 * 		return immediately.
 * pram
 * 		mutex: pointer to mutex object
 */
sk_err_t sk_mutex_trylock(struct sk_mutex *mutex)
{
	return sk_mutex_lock(mutex, 0);
}


/*
 * sk_mutex_release
 * brief
 * 		this function will release a mutex. if there is a thread suspend on the mutex,
 * 		the thread will be resumed.
 * param
 * 		mutex: poniter to a mutex object 
 */
sk_err_t sk_mutex_unlock(struct sk_mutex *mutex)
{
	struct sk_thread *thread;
	sk_ubase_t temp;

	/* get current thread */
	thread = sk_current_thread();

	/* disable interrupt */	
	temp = hw_interrupt_disable();

	/* mutex only can be released by owner */
	if(thread != mutex->owner) {
		/* enable interrupt */
		hw_interrupt_enable(temp);

		return SK_ERROR;
	}

	mutex->hold--;
	if(mutex->hold == 0) {
		/* wakeup suspended thread */
		if(!sk_list_empty(&mutex->parent.suspend_thread)) {

			/* get suspended thread */
			thread = sk_list_entry(mutex->parent.suspend_thread.next, 
								   struct sk_thread,
								   tlist); 

			/* set new owner and priority */
			mutex->owner = thread;
			mutex->original_pri = thread->current_pri;
			mutex->hold++;

			/* resume thread */
			sk_thread_resume(thread);

			/* enable interrupt */
			hw_interrupt_enable(temp);

			/* do schedule */
			sk_schedule();

			return SK_EOK;
		} else {
			mutex->value++;
			mutex->owner = SK_NULL;
			mutex->original_pri = 0xFF;
		}
	}

	/* enable interrupt */
	hw_interrupt_enable(temp);

	return SK_EOK;
}

