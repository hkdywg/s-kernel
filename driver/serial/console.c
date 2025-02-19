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

sk_size_t skip_atoi(const char **s)
{
	sk_size_t i = 0;
	while(SK_ISDIGIT(**s))
		i = i * 10 + *((*s)++) - '0';

	return i;
}

static void sk_itoa(int num, char *str, int base)
{
	sk_uint32_t i = 0;
	sk_uint32_t rem;
	sk_bool_t is_negative = SK_FALSE;

	/* handle 0 explicityly */
	if(num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return;
	}
	/* handle negative numbers only if base is 10 */
	if(num < 0 && base == 10) {
		is_negative = SK_TRUE;
		num = -num;
	}
	/* process individual digits */
	while(num != 0) {
		rem = num % base;
		str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		num = num / base;
	}
	/* append negative sign for negative numbers */
	if(is_negative)
		str[i++] = '-';
	str[i] = '\0';

	/* reverse the string */
	sk_uint32_t start = 0;
	sk_uint32_t end = i - 1;
	while(start < end) {
		char temp = str[start];
		str[start] = str[end];
		str[end] = temp;
		start++;
		end--;
	}
}

/*
 * sk_vsprintf
 * brief
 * 		this function will fill a formatted string to buffer
 * param
 * 		buf: the buffer to save formatted string
 * 		fmt: the format parameters
 * 		args: a list of variable parameters
 */
static sk_size_t sk_vsprintf(char *buf,
							 const char *fmt, va_list args)
{
	char *str = buf;
	const char *p = fmt;
	char temp[32];		/* temp buffer */

	while(*p) {
		if(*p == '%' && *(p + 1)) {
			p++;	/* skip '%' */
			switch(*p) {
			case 'd':
			{
				int val = va_arg(args, int);
				sk_itoa(val, temp, 10);
				sk_strcpy(str, temp);
				str += sk_strlen(temp);
			}
			break;
			case 'x':
			{
				int val = va_arg(args, int);
				sk_itoa(val, temp, 16);
				sk_strcpy(str, temp);
				str += sk_strlen(temp);
			}
			break;
			case 's':
			{
				char *val = va_arg(args, char*);
				sk_strcpy(str, val);
				str += sk_strlen(val);
			}
			break;
			case 'c':
			{
				char val = (char)va_arg(args, int);
				*str = val;
				str++;
			}
			break;
			default:
				*str = '%';
				str++;
				*str = *p;
				str++;
				break;
			}
		} else {
			*str = *p;
			str++;
		}
		p++;
	}
	*str = '\0';
	return str - buf;
}

/*
 * sk_kprintf
 * brief
 * 		this function will print a formatted string on system console.
 */
void sk_kprintf(const char *fmt, ...)
{
	va_list args;
	sk_size_t length;
	static char sk_log_buf[SK_CONSOLE_BUF_SIZE];

	va_start(args, fmt);
	length = sk_vsprintf(sk_log_buf, fmt, args);
	if(length > SK_CONSOLE_BUF_SIZE)
		length = SK_CONSOLE_BUF_SIZE;
	if(_console_device)
		sk_device_write(_console_device, 0, sk_log_buf, length);

	va_end(args);
}



