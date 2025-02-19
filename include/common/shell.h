/*                                                                                                                                                                     
 *  shell.h
 *
 *  brif
 *      shell
 *  
 *  (C) 2025.02.19 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef __SHELL_H_
#define __SHELL_H_

#include <base_def.h>

typedef long (*syscall_func)(void);

#define SHELL_CMD_HISTORY_LINE 	(5)
#define SHELL_CMD_SIZE 			(80)
#define SHELL_THREAD_STACK_SIZE (4096)
#define SHELL_THREAD_PRIORITY 	(20)

#define SHELL_PROMPT 			"shell:> "

/*
 * system call table 
 */
struct shell_syscall {
	const char 		*name;		/* the name of system call */
	const char 		*desc;		/* description of system call */ 		
	syscall_func 	func;		/* the function address of system call */
};

/*
 * system call item
 */
struct shell_syscall_item {
	struct shell_syscall_item *next;	/* next item */
	struct shell_syscall syscall;		/* syscall */
};

enum input_stat {
	WAIT_NORMAL,
	WAIT_SPEC_KEY,
	WAIT_FUNC_KEY,
};

struct shell_cmd {
	enum input_stat stat;
	sk_uint8_t echo_mode: 1;
	sk_uint8_t prompt_mode: 1;

	sk_uint16_t current_history;
	sk_uint16_t history_count;
	char cmd_history[SHELL_CMD_HISTORY_LINE][SHELL_CMD_SIZE];

	char line[SHELL_CMD_SIZE + 1];
	sk_uint16_t line_position;
	sk_uint16_t line_curpos;
	
	struct sk_device *device;
};


#define SHELL_FUNCTION_EXPORT_CMD(name, cmd, desc)	\
	const char __tsym_##cmd##_name[] SK_SECTION(".rodata.name") = #cmd;	\
	const char __tsym_##cmd##_desc[] SK_SECTION(".rodata.name") = #desc;	\
	const struct shell_syscall __sym_##cmd SK_SECTION("Tsymtab")= \
	{							\
		__tsym_##cmd##_name,	\
		__tsym_##cmd##_desc,	\
		(syscall_func)&name		\
	};


int shell_system_init(void);

#endif

