/*                                                                                                                                                                     
 *  console.c
 *
 *  brif
 *      console
 *  
 *  (C) 2025.02.13 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <device.h>
#include <serial.h>
#include <config.h>
#include <stdarg.h>

static struct sk_device *_console_device = SK_NULL;

/*
 * sk_console_set_device
 * brief
 * 		set a device as console device
 * param
 * 		name: the name of new console device
 */
struct sk_device *sk_console_set_device(const char *name)
{
	struct sk_device *new_device, *old_device;

	/* save old device */
	old_device = _console_device;

	/* find new console device */
	new_device = sk_device_find(name);

	/* check whether it's a same device */
	if(new_device == old_device)
		return SK_NULL;

	if(new_device != SK_NULL) {
		if(_console_device != SK_NULL)
			/* close old console device */
			sk_device_close(_console_device);

		/* set new console device */
		sk_device_open(new_device, SK_DEVICE_FLAG_RDWR);
		_console_device = new_device;
	}

	return old_device;
}

/*
 * sk_console_get_device
 * brief
 * 		return the device using in console
 */
struct sk_device *sk_console_get_device(void)
{
	return  _console_device;
}

/*
 * sk_kprintf
 * brief
 * 		this function will print a formatted string on system console.
 */
#if 0
void sk_kprintf(const char *fmt, ...)
{
	va_list args;
	sk_size_t length;
	static char sk_log_buf[SK_CONSOLE_BUF_SIZE];

	va_start(args, fmt);
//	length = vsprintf(sk_log_buf, fmt, args);
	if(length > SK_CONSOLE_BUF_SIZE)
		length = SK_CONSOLE_BUF_SIZE;
	if(!_console_device)
		sk_device_write(_console_device, 0, sk_log_buf, length);

	va_end(args);
}
#else
void sk_kprintf(char *buf)
{
	if(_console_device)
		sk_device_write(_console_device, 0, buf, 21);
}
#endif



