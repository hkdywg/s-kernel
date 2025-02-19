/*                                                                                                                                                                     
 *  shell.c
 *
 *  brif
 *      shell
 *  
 *  (C) 2025.02.18 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <base_def.h>
#include <device.h>
#include <shell.h>
#include <sched.h>
#include <serial.h>
#include <skernel.h>

static struct shell_syscall *_syscall_table_begin = SK_NULL;
static struct shell_syscall *_syscall_table_end = SK_NULL;
static struct shell_cmd *shell = SK_NULL;

/*
 * shell_set_device
 * brief
 * 		set the input device for shell
 * param
 * 		device_name: the name of input device
 */
void shell_set_device(const char *device_name)
{
	struct sk_device *dev = sk_device_find(device_name);
	if(dev == SK_NULL) {
		sk_kprintf("shell: can't find device: %s\n", device_name);
		return;
	}
	/* check whether it's a same device */
	if(dev == shell->device)
		return;
	/* open this device and set the new device in shell */
	if(sk_device_open(shell->device, SK_DEVICE_FLAG_RDWR) == SK_EOK) {
		if(shell->device != SK_NULL) {
			sk_device_close(shell->device);
		}
	}
	/* clear line buffer befor switch to new device */
	sk_memset(shell->line, 0, sizeof(shell->line));
	shell->line_curpos = shell->line_position = 0;
	shell->device = dev;
}

/*
 * shell_getchar
 * brief
 * 		get the input character from terminal device
 */
int shell_getchar()
{
	char ch = 0;
	if(shell->device == SK_NULL)
		return -1;
	if(0 == sk_device_read(shell->device, 0, &ch, 1))
		return -1;

	return ch;
}

/*
 * shell_handle_history
 * brief
 * 		process the command to display of history
 * param
 * 		shell: the pointer of shell command struct 
 */
static sk_bool_t shell_handle_history(struct shell_cmd *shell)
{
	/*
	 * clear current line and set custor to home
	 * \033: ASCII 27, ESC, it is start of ANSI escape sequence
	 * [2K: clear curent line
	 * \r: return the cursor to the begining of curent line
	 */
	sk_kprintf("\033[2K\r");
	sk_kprintf("%s%s", SHELL_PROMPT, shell->line);
}

/*
 * shell_auto_complete
 * brief
 * 		check the internal command and compare with prefix of the input command
 * param
 * 		prefix: the input command prefix
 */
static void shell_auto_complete(char *prefix)
{
	struct shell_syscall *index;
	const char *cmd_name;
	sk_kprintf("\n");
	for(index = _syscall_table_begin; index < _syscall_table_end; index++) {
		cmd_name = (const char *)index->name;
		if(sk_strncmp(prefix, cmd_name, sk_strlen(prefix)) == 0) {
			sk_kprintf("%s\n", cmd_name);
		}
	}
	
	sk_kprintf("%s%s", SHELL_PROMPT, prefix);
}

/*
 *	shell_push_history
 *	brief
 *		put the input command to history table
 */
static void shell_push_history(struct shell_cmd *shell)
{
	/* check the command is empty */
	if(shell->line_position != 0) {
		/* push histoty */
		if(shell->history_count >= SHELL_CMD_HISTORY_LINE) {
			/* if current cmd is same as last cmd, don't push */
			if(sk_memcmp(&shell->cmd_history[SHELL_CMD_HISTORY_LINE - 1], shell->line, SHELL_CMD_SIZE) != 0) {
				sk_uint8_t i = 0;
				for(i = 0; i < SHELL_CMD_HISTORY_LINE - 1; i++) {
					sk_memcpy(&shell->cmd_history[i][0],
							  &shell->cmd_history[i + 1][0], SHELL_CMD_SIZE);
				}
				sk_memset(&shell->cmd_history[i][0], 0, SHELL_CMD_SIZE);
				sk_memcpy(&shell->cmd_history[i][0], shell->line, shell->line_position);

				shell->history_count = SHELL_CMD_HISTORY_LINE;
			}
		} else {
			if(shell->history_count == 0 || sk_memcmp(&shell->cmd_history[shell->history_count - 1], shell->line, SHELL_CMD_SIZE)) {
				shell->current_history = shell->history_count;
				sk_memset(&shell->cmd_history[shell->history_count][0], 0, SHELL_CMD_SIZE);
				sk_memcpy(&shell->cmd_history[shell->history_count][0], shell->line, shell->line_position);
				shell->history_count++;
			}
		}
	}
	shell->current_history = shell->history_count;
}

/*
 * shell_exec
 * brief
 * 		find the command function and execute it
 * param
 * 		cmd: the string of command
 * 		len: length of command
 */
static sk_uint8_t shell_exec(char *cmd, sk_size_t len)
{
	struct shell_syscall *index;
	syscall_func cmd_func = SK_NULL;
	/* delete space character beginnig of comand*/
	while((len > 0) && (*cmd == ' ' || *cmd == '\t')) {
		cmd++;
		len--;
	}
	/* empty command */
	if(len == 0)
		return 0;

	for(index = _syscall_table_begin; index < _syscall_table_end; index++) {
		if(sk_strncmp(index->name, cmd, len) == 0) {
			index->func();
			return 0;
		}
	}
	sk_kprintf("%s: command not found.\n", cmd);
	
	return -1;
}

/*
 * shell_thread_entry
 * brief
 * 		shell thread process, all command of shell will process by this function
 */
void shell_thread_entry(void *arg)
{
	int ch;

	shell->echo_mode = 1;
	if(shell->device == SK_NULL) {
		struct sk_device *console = sk_console_get_device();
		if(console)
			shell_set_device(console->parent.name);
	}

	sk_kprintf(SHELL_PROMPT);
	while(1) {
		ch  = shell_getchar();
		if(ch == -1)
			continue;

		/*
		 * handle control key
		 * up 	 key: 0x1b 0x5b 0x41
		 * down  key: 0x1b 0x5b 0x42
		 * right key: 0x1b 0x5b 0x43
		 * left  key: 0x1b 0x5b 0x44
		 */
		if(ch == 0x1b) {
			shell->stat = WAIT_SPEC_KEY;
			continue;
		} else if(shell->stat == WAIT_SPEC_KEY) {
			if(ch == 0x5b) {
				shell->stat = WAIT_FUNC_KEY;
				continue;
			}
			shell->stat = WAIT_NORMAL;
		} else if(shell->stat == WAIT_FUNC_KEY) {
			shell->stat = WAIT_NORMAL;
			/* process up key */
			if(ch == 0x41) {
				/* prev history */
				if(shell->current_history > 0)
					shell->current_history--;
				else {
					shell->current_history = 0;
					continue;
				}
				sk_memcpy(shell->line, &shell->cmd_history[shell->current_history][0], SHELL_CMD_SIZE);
				shell->line_curpos = shell->line_position = sk_strlen(shell->line);
				shell_handle_history(shell);
				continue;
			}
			/* process down key */
			else if(ch == 0x42) {
				/* prev history */
				if(shell->current_history < shell->history_count - 1)
					shell->current_history++;
				else {
					/* set to the end of history */
					if(shell->history_count != 0)
						shell->current_history = shell->history_count - 1;
					else 
						continue;
				}
				sk_memcpy(shell->line, &shell->cmd_history[shell->current_history][0], SHELL_CMD_SIZE);
				shell->line_curpos = shell->line_position = sk_strlen(shell->line);
				shell_handle_history(shell);
				continue;
			} 
			/* process left key */
			else if(ch == 0x44) {
				if(shell->line_curpos) {
					sk_kprintf("\b");
					shell->line_curpos--;
				}
				continue;
			}
			/* process right key */
			else if(ch == 0x43) {
				if(shell->line_curpos < shell->line_position) {
					sk_kprintf("%c", shell->line[shell->line_curpos]);
					shell->line_curpos++;
				}
				continue;
			}
		}

		/* received null or error */
		if(ch == '\0' || ch == 0xff)
			continue;
		/* handle tab key */
		else if(ch == '\t') {
			int i = 0;
			for(i = 0; i < shell->line_curpos; i++)
				sk_kprintf("\b");
			/* auto complete */
			shell_auto_complete(&shell->line[0]);
			/* re-calculate position */
			shell->line_curpos = shell->line_position = sk_strlen(shell->line);
			continue;
		}
		/* handle backspace key */
		else if(ch == 0x7f || ch == 0x08) {
			if(shell->line_curpos == 0)
				continue;
			shell->line_position--;
			shell->line_curpos--;
			sk_kprintf("\b \b");
			shell->line[shell->line_position] = 0;
			continue;
		}

		/* handle end of line, break */
		if(ch == '\r' || ch == '\n' || ch == 0x03) {
			shell_push_history(shell);
			sk_kprintf("\n");
			if(ch != 0x03)
				shell_exec(shell->line, shell->line_position);
			sk_kprintf(SHELL_PROMPT);
			sk_memset(shell->line, 0, sizeof(shell->line));
			shell->line_curpos = shell->line_position = 0;
			continue;
		}

		/* normal character */
		shell->line[shell->line_position] = ch;
		sk_kprintf("%c", ch);
		shell->line_curpos++;
		shell->line_position++;
	}
}

/*
 * shell_system_init
 * brief
 * 		initialize shell system
 */
int shell_system_init(void)
{
	sk_err_t ret = SK_EOK;

	extern const int __tsymtab_start, __tsymtab_end;
	_syscall_table_begin = (struct shell_syscall *)&__tsymtab_start;
	_syscall_table_end   = (struct shell_syscall *)&__tsymtab_end;

	/* create and set shell structure */
	shell = (struct shell_cmd *)sk_malloc(sizeof(struct shell_cmd));
	if(shell == SK_NULL) {
		sk_kprintf("no memory for shell!\n");
		return -1;
	}
	sk_memset(shell, 0, sizeof(struct shell_cmd));

	char shell_thread_name[SK_NAME_MAX] = "shell";

	struct sk_thread *thread = sk_thread_create(shell_thread_name,
												shell_thread_entry,
												SK_NULL,
												SHELL_THREAD_STACK_SIZE,
												SHELL_THREAD_PRIORITY,
												10);
	sk_thread_startup(thread);
}
