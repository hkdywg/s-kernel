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
