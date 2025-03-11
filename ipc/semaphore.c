/*
 *  semaphore.c
 *  brief
 *  	semaphore module of ipc
 *  
 *  (C) 2025.03.11 <hkdywg@163.com>
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
 * sk_sem_init
 * brief
 * 		this function will init a semaphore object
 * param
 * 		sem: opinter to the semaphore to initialize
 * 		name: the name of semaphore 
 * 		value: initial value for semaphore
 * 		flag: semaphore flag, can be set SK_IPC_FLAG_FIFO or SK_IPC_FLAG_PRIO
 */
sk_err_t sk_sem_init(struct sk_sem *sem, const char *name, 
					 sk_uint16_t value, sk_uint8_t flag)
{
	/* initialize object */
	sk_object_init(&(sem->parent.parent), SK_OBJECT_SEMAPHORE, name);

	/* initialize ipc object */
	__ipc_object_init(&(sem->parent));

	sem->value = value;

	/* set parent flag */
	sem->parent.parent.flag = flag;

	return SK_EOK;
}
/*
 * sk_sem_create
 * brief
 * 		this function will create a semaphore object
 * param
 * 		name: the name of semaphore
 * 		flag: the flag of semaphore, can be set SK_IPC_FLAG_PRIO or SK_IPC_FLAG_FIFO 
 */
struct sk_sem *sk_sem_create(const char *name, sk_uint16_t value, sk_uint8_t flag)
{
	struct sk_sem *sem;
	/* eliminate compile warning */
	(void)flag;

	/* allocate object */
	sem = (struct sk_sem *)sk_object_alloc(SK_OBJECT_SEMAPHORE, name);
	if(sem == SK_NULL)
		return sem;

	/* initialize ipc object */
	__ipc_object_init(&(sem->parent));

	sem->value = value;

	/* set semaphore flag */
	sem->parent.parent.flag = flag;

	return sem;
}

/*
 * sk_sem_destroy
 * brief
 * 		delete a semaphore object which is created by the sk_sem_create function
 * param
 * 		sem: pointer to semaphore object to be deleted
 */
sk_err_t sk_sem_destroy(struct sk_sem *sem)
{
	if(sem == SK_NULL)
		return SK_EOK;

	/* wakeup all syspended threads */
	__ipc_list_resume_all(&(sem->parent.suspend_thread));

	/* delete semaphore object */
	sk_object_delete(&(sem->parent.parent));

	return SK_EOK;
}


/*
 * sk_sem_wait
 * brief
 * 		this function will take a semaphore, if the semaphore is unavailable, the thread shall
 * 		wait for the semaphore up to specified time
 * param
 * 		sem: pointer to semaphore
 * 		time: time out period
 */
sk_err_t sk_sem_wait(struct sk_sem *sem, sk_int32_t time)
{
	struct sk_thread *thread;
	sk_ubase_t temp;

	/* get current thread */
	thread = sk_current_thread();

	/* disable interrupt */	
	temp = hw_interrupt_disable();

	/* the value of mutex is 1 in initial status. therefore, if the value
	 * is great thann 0, it indicates the mutex is avaible */
	if(sem->value > 0) {
		/* sem is available */
		sem->value --;

		/* enable interrupt */
		hw_interrupt_enable(temp);
	} else {
		/* no waiting, return with timeout */
		if(time == 0) {
			/* enable interrupt */
			hw_interrupt_enable(temp);
			return SK_ETIMEOUT;
		} else {
			/* suspend current thread */
			__ipc_list_suspend(&(sem->parent.suspend_thread), 
							   thread, 
							   sem->parent.parent.flag);

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
	
	return SK_EOK;
}


/*
 * sk_sem_trywait
 * brief
 * 		this function will try to take a semaphore. if the semaphore is unavailable, the thread
 * 		return immediately.
 * pram
 * 		mutex: pointer to mutex object
 */
sk_err_t sk_sem_trywait(struct sk_sem *sem)
{
	return sk_sem_wait(sem, 0);
}


/*
 * sk_sem_post
 * brief
 * 		this function will release a semaphore. if there is a thread suspend on the semaphore,
 * 		the thread will be resumed.
 * param
 * 		sem: poniter to a semaphore object 
 */
sk_err_t sk_sem_post(struct sk_sem *sem)
{
	struct sk_thread *thread;
	sk_ubase_t temp;

	/* get current thread */
	thread = sk_current_thread();

	/* disable interrupt */	
	temp = hw_interrupt_disable();

	/* wakeup suspended thread */
	if(!sk_list_empty(&sem->parent.suspend_thread)) {

		/* get suspended thread */
		thread = sk_list_entry(sem->parent.suspend_thread.next, 
							   struct sk_thread,
							   tlist); 

		/* resume thread */
		sk_thread_resume(thread);

		/* enable interrupt */
		hw_interrupt_enable(temp);

		/* do schedule */
		sk_schedule();

		return SK_EOK;
	} else {
		if(sem->value < SK_SEM_VALUE_MAX) {
			sem->value ++;
		} else {
			/* enable interrupt */
			hw_interrupt_enable(temp);
			return SK_EFULL;
		}
	}

	/* enable interrupt */
	hw_interrupt_enable(temp);

	return SK_EOK;
}

