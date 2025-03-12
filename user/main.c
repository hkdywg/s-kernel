/*
 *  main.c
 *  brief
 *  	user application main 
 *  
 *  (C) 2025.01.24 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <skernel.h>
#include <timer.h>
#include <ipc.h>
#include <shell.h>
	
static float cpu_usage = 0.0;

void main(void *arg)
{
	sk_ubase_t cnt = 0;

	while(1) {
		cnt++;
		cpu_usage = (1.0 - (float)sk_idle_tick_get()/(float)sk_tick_get());
		sk_thread_delay(1000);
	}
}

void show_cpu_usage()
{
	sk_kprintf("current cpu usage is %d\%\n", (sk_uint8_t)(cpu_usage * 100));
}

SHELL_CMD_EXPORT(show_cpu_usage, test case of ipc semaphore);
