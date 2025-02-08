/*
 *  skernel.h
 *  brief
 *  	s-kernel system compont interface
 *  
 *  (C) 2025.01.09 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#ifndef __SKERNEL_H_
#define __SKERNEL_H_

#include <base_def.h>

/*
 * kernel tick interface 
 */
sk_tick_t sk_tick_get(void);
int sk_hw_timer_init(void);

/*
 * memory management interface
 */
extern sk_err_t sk_system_mem_init(void *begin_addr, void *end_addr);
extern void *sk_malloc(sk_size_t size);
extern void sk_free(void *ptr);

/* system tick */
sk_tick_t sk_tick_get(void);
sk_tick_t sk_idle_tick_get();

#endif
