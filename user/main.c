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

int main(void *arg)
{
	sk_ubase_t cnt = 0;
	float cpu_usage = 0.0;
	while(1) {
		cnt++;
		cpu_usage = sk_idle_tick_get()/sk_tick_get();
	}
}
