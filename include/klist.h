/*
 *  klist.h
 *  brief
 *  	list releted definitions of s-kernel 
 *  
 *  (C) 2025.01.14 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#ifndef __KLIST_H_
#define __KLIST_H_

#include <base_def.h>

/*
 * sk_container_of
 * brief
 * 		return the the member address of ptr
 */
#define sk_container_of(ptr, type, member) \
	((type *)((char *)ptr - (unsigned long)(&((type *)0)->member)))

/*
 *		----------------------------------------------------- 
 *      |                                                   | 
 *      |    node        node        node        node       | 
 *      |   +++++++     +++++++     +++++++     +++++++     | 
 *      --->|     |     |     |     |     |     |     |     | 
 *          |  n  |---->|  n  |---->|  n  |---->|  n  |------ 
 *      ----|  p  |<----|  p  |<----|  p  |<----|  p  |       
 *      |   |     |     |     |     |     |     |     |<----- 
 *      |   +++++++     +++++++     +++++++     +++++++     | 
 *      |                                                   | 
 *       ---------------------------------------------------- 
 */


/*
 * Double list structure
 */
struct sk_list_node
{
	struct sk_list_node *next;			/* point to next node */
	struct sk_list_node *prev;			/* point to prev node */
};
typedef struct sk_list_node sk_list_t;

/*
 * Single list structure
 */
struct sk_slist_node
{
	struct sk_slist_node *next;			/* point to next node */
};
typedef struct sk_slist_node sk_slist_t;

/*
 * sk_list_entry
 * brief 
 * 		get the struct for this entry
 * 	param
 * 		ptr: the struct sk_list_node pointer
 * 		type: the type of the struct this is embedded in
 * 		member: the name of the list_struct within the struct
 */
#define sk_list_entry(ptr, type, member) \
	sk_container_of(ptr, type, member)


/*
 * sk_list_for_each
 * brief 
 * 		iterate over a list
 * 	param
 * 		pos: the sk_list_t * to use as a loop cursor
 * 		head: the head for your list
 */
#define sk_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)


/*
 * sk_list_init
 * brief 
 * 		initialize of a list
 * 	param
 * 		list: list to be initialized
 */
static inline void sk_list_init(sk_list_t *list)
{
	list->next = list->prev = list;
}

/*
 * __list_add
 * brief 
 * 		Insert a new list between two known consecutive entries
 * 	param
 * 		new: new entry to be added
 */
static inline void __list_add(sk_list_t *new, sk_list_t *prev,
					  sk_list_t *next)
{
	next->prev = new;
	new->next  = next;
	new->prev = prev;
	prev->next = new;
}

/*
 * __list_del
 * brief 
 * 		delte a list entry by making the prev/next enties
 * 	param
 * 		new: new entry to be added
 */
static void inline __list_del(sk_list_t *prev, sk_list_t *next)
{
	next->prev = prev;
	prev->next = next;
}

/*
 * sk_list_add
 * brief 
 * 		Insert a new entry
 * 	param
 * 		new: new entry to be added
 * 		head: list head to be add it after
 */
static inline void sk_list_add(sk_list_t *new, sk_list_t *head)
{
	__list_add(new, head, head->next);
}

/*
 * sk_list_add_tail
 * brief 
 * 		Insert a new entry
 * param
 * 		new: new entry to be added
 * 		head: list head to be add it befor
 */
static inline void sk_list_add_tail(sk_list_t *new, sk_list_t *head)
{
	__list_add(new, head->prev, head);
}

/*
 * sk_list_del
 * brief 
 * 		delete a entry
 * param
 * 		entry: list node of need to be delete 
 */
static inline void sk_list_del(sk_list_t *entry)
{
	__list_del(entry->prev, entry->next);
}

/*
 * sk_list_empty
 * brief 
 * 		test whether a list is empty
 * param
 * 		head: the list to test
 */
static inline sk_bool_t sk_list_empty(const sk_list_t *head)
{
	return head->next == head;
}

#endif
