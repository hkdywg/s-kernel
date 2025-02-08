/*
 *  kobj.h
 *  brief
 *  	kernel object definitions of s-kernel 
 *  
 *  (C) 2025.01.17 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef __KOBJ_H_
#define __KOBJ_H_

#include <base_def.h>
#include <klist.h>

/* 
 * basic structure of kernel object 
 */
struct sk_object
{
	char 		name[SK_NAME_MAX];			/* name of kernel object */
	sk_uint8_t  type;						/* type of kernel object */
	sk_uint8_t  flag;						/* flag of kernel object */

	sk_list_t  	list;						/* list node of kernel object */ 				
};


/*
 * define object_type 
 */
enum sk_object_type {
	SK_OBJECT_THREAD = 0,					/* thread object */
	SK_OBJECT_SEMAPHORE, 					/* semaphore object */
	SK_OBJECT_MUTEX,	 					/* mutex object */
	SK_OBJECT_EVENT,	 					/* event object */
	SK_OBJECT_MAILBOX,	 					/* mailbox object */
	SK_OBJECT_MSQUE,	 					/* message queue object */
	SK_OBJECT_DEVICE,	 					/* device object */
	SK_OBJECT_TIMER,	 					/* tick object */
	SK_OBJECT_UNKNOWN,
};

/*
 * the information of the kernel object
 */
struct sk_object_info
{
	enum sk_object_type type;				/* object class type */
	sk_list_t 			obj_list; 			/* object list */
	sk_size_t 			obj_size;			/* object size */
};

struct sk_object *sk_object_alloc(enum sk_object_type type, const char *name);
void sk_object_delete(struct sk_object *obj);
void sk_object_init(struct sk_object *obj,
					enum sk_object_type type,
					const char *name);
struct sk_object *sk_object_find(const char *name, enum sk_object_type type);

#endif

