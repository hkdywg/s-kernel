/*
 *  kobj.c
 *  brief
 *  	object definitions, used by ipc and thread and device
 *  
 *  (C) 2025.01.17 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <skernel.h>
#include <kobj.h>
#include <timer.h>
#include <interrupt.h>

/* init the sk_object double list */
#define _OBJ_CONTAINER_LIST_INIT(c)	\
	{&(_object_container[c].obj_list), &(_object_container[c].obj_list)}

static struct sk_object_info _object_container[] = {
	{SK_OBJECT_TIMER, _OBJ_CONTAINER_LIST_INIT(SK_OBJECT_TIMER), sizeof(struct sk_sys_timer)},
};


/*
 * sk_object_get_info
 * brief
 * 		this function will return the specified type  of object information
 * param
 * 		type: the object type
 */
struct sk_object_info *sk_object_get_info(enum sk_object_type type)
{
	int index;
	for(index = 0; index < SK_OBJECT_UNKNOWN; index++) {
		if(_object_container[index].type == type)
			return &_object_container[index];
	}

	return SK_NULL;
}

/*
 *	sk_object_alloc
 *	brief
 *		allocate an object from object system
 *	param
 *		type: the type of object
 *		name: the name of object name
 */
struct sk_object *sk_object_alloc(enum sk_object_type type, const char *name) 
{
	struct sk_object *obj;
	struct sk_object_info  *info;
	sk_base_t level;

	/* get object information */
	info = sk_object_get_info(type);

	obj = (struct sk_object *)sk_malloc(info->obj_size);
	if(obj == SK_NULL)
		return SK_NULL;

	/* clean memory data of object */
	sk_memset(obj, 0, info->obj_size);

	/* initialize object's paramters */
	obj->type = type;
	obj->flag = 0;

	/* copy name */
	sk_memcpy(obj->name, name, SK_NAME_MAX);

	/* disable interrupt */
	level = hw_interrupt_disable();

	/* insert object into information object list */
	sk_list_add(&(info->obj_list), &(obj->list));

	/* enable interrupt */
	hw_interrupt_enable(level);

	return obj;
}

/*
 * sk_object_delete
 * brief
 * 		delete a object from object system
 * param
 * 		obj: the object of need to be delete
 */
void sk_object_delete(struct sk_object *obj)
{
	sk_base_t level;

	/* reset object type */
	obj->type = -1;

	/* disable interrupt */
	level = hw_interrupt_disable();

	/* insert object into information object list */
	sk_list_del(&(obj->list));

	/* enable interrupt */
	hw_interrupt_enable(level);

	/* free the memory of object */
	sk_free(obj);
}

/*
 * sk_object_init
 * brief
 * 		initialize a object and add it to object system management
 * param
 * 
 */
void sk_object_init(struct sk_object *obj,
					enum sk_object_type type,
					const char *name)
{
	struct sk_object_info *info;
	struct sk_list_node *node;

	/* get object information */
	info = sk_object_get_info(type);

	/* try to find object */
	for(node = info->object_list.next; node != &(info->object_list); 
		node = node->next) {
		struct sk_object *obj_tmp;

		obj_tmp = sk_list_entry(node, struct sk_object, list);
		if(obj_tmp == obj)
			return;
	}
	/* initialize object's parameters */
	obj->type = type;
	sk_memcpy(obj->name, name, SK_NAME_MAX);
	/* insert object into information object list */
	sk_list_add(&(info->obj_list), &(obj->list));

	return;
}







