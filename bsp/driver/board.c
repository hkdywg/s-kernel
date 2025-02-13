/*                                                                                                                                                                     
 *  board.c
 *
 *  brief
 *      board relative function
 *  
 *  (C) 2025.02.13 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <serial.h>
#include <config.h>
#include <board.h>

/*
 * Init the hardware related 
 *
 * @param: none
 */
void sk_hw_board_drv_init(void)
{
	/* uart device init */
	sk_hw_uart_init();

	/* set console device */
	sk_console_set_device(SK_CONSOLE_DEVICE_NAME);

	/**/
	sk_kprintf("------skernel--------\n");
}

