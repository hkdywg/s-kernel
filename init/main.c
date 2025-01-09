/*
 *  main.c
 *  brief
 *  	kernel entrance
 *  
 *  (C) 2024.12.25 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#include <type_def.h>

int func_1() 
{
	sk_uint32_t cnt = SK_UINT32_MAX;
	while(cnt--);

	return 0;
}

int func_2() 
{
	sk_uint32_t cnt = SK_UINT32_MAX;
	while(cnt--);

	return 0;
}

int skernel_startup(void)
{
	while(1) {
		func_1();
		func_2();
	}
	return 0;
}
