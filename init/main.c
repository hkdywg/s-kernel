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
#include <interrupt.h>
#include <skernel.h>
#include <hw.h>
#include <skernel.h>

extern unsigned char __bss_start;
extern unsigned char __bss_end;

#define SK_HEAP_BEGIN    (void*)&__bss_end
#define SK_HEAP_END      (void*)(SK_HEAP_BEGIN + 1 * 1024 * 1024)

/*
 * Init the hardware related 
 *
 * @param: none
 */
void sk_hw_board_init(void)
{
	/* Initialize hardware interrupt */
	sk_hw_interrupt_init();

	/* kernel tick init */
	sk_hw_timer_init();

	/* memory management init */
	sk_system_mem_init(SK_HEAP_BEGIN, SK_HEAP_END);
}

int skernel_startup(void)
{
	/* hardware related init, must be first called in skernel_startup*/
	sk_hw_board_init();

	sk_uint8_t *ptr_1 = SK_NULL;
	ptr_1 = (sk_uint8_t *)sk_malloc(15);
	*ptr_1 = 12;
	sk_uint8_t test_var = 0xa5;
	while(1);

	return 0;
}
