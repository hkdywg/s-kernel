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

void sk_show_version(void) {
    sk_kprintf("      .-'`'-.        \n");
    sk_kprintf("    /        \\      \n");
    sk_kprintf("   | s-kernel |     \n");
    sk_kprintf("   | Operation|     \n");
    sk_kprintf("   | System   |     \n");
    sk_kprintf("    \\________/      \n");
    sk_kprintf("      V %d.%d.%d      \n", SK_VERSION, SK_SUBVERSION, SK_REVISION);
    sk_kprintf("   Build: %s %s\n", __DATE__, __TIME__);
    sk_kprintf("   Copyright 2024-2025\n");
    sk_kprintf("   Powered by yinwg\n");
}

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
	//sk_kprintf("------skernel-------- %d\n", 123);
	sk_show_version();
}

