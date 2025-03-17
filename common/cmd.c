/*                                                                                                                                                                     
 *  cmd.c
 *
 *  brief
 *      system command implementation
 *  
 *  (C) 2025.02.19 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <shell.h>
#include <base_def.h>
#include <kobj.h>
#include <sched.h>

static long clear()
{
	sk_kprintf("\x1b[2J\x1b[H");
	return 0;
}
SHELL_CMD_EXPORT(clear, clear the terminal screen);

extern void sk_show_version();
static long version()
{
	sk_show_version();
	return 0;
}
SHELL_CMD_EXPORT(version, show s-kernel version);

static long top()
{
	struct sk_object_info *info;
	sk_base_t level;
	sk_list_t *entry;
	struct sk_thread *thread;
	sk_kprintf("thread   prio   status      sp     stack_size cpu_usage   remain_tick\n");
	sk_kprintf("------   ---- ----------  ---------- ---------- ---------   -----------\n");
	/* get object information */
	info = sk_object_get_info(SK_OBJECT_THREAD);
	sk_list_for_each(entry, &info->obj_list) {
		thread = (struct sk_thread *)sk_list_entry(entry, struct sk_object, list);
		sk_kprintf("%s 	 %d	", thread->name, thread->current_pri);
		switch(thread->stat) {
		case SK_THREAD_READY:
			sk_kprintf(" ready ");
			break;
		case SK_THREAD_RUNNING:
			sk_kprintf("running");
			break;
		case SK_THREAD_SUSPEND:
			sk_kprintf("suspend");
			break;
		case SK_THREAD_INIT:
			sk_kprintf("init");
			break;
		case SK_THREAD_CLOSE:
			sk_kprintf("close");
			break;
		}
		sk_kprintf("    0x%x 	%d	   %d   	%d\n",thread->sp, thread->stack_size, 10, thread->remain_tick);
	}

	return 0;
}
SHELL_CMD_EXPORT(top, show s-kernel version);
