/*                                                                                                                                                                     
 *  ring_buffer.h
 *
 *  brif
 *      ring buffer 
 *  
 *  (C) 2025.02.08 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef __RING_BUFFER_H_
#define __RING_BUFFER_H_

#include <base_def.h>

/*
 * ring buffer structure
 */
struct sk_ring_buffer
{
	sk_uint8_t *buffer;

	sk_uint16_t read_mirror : 1;
	sk_uint16_t read_index : 15;
	sk_uint16_t write_mirror : 1;
	sk_uint16_t write_index : 15;

	sk_int16_t buffer_size;
};

enum sk_ring_buffer_state
{
	SK_RING_BUFFER_EMPTY,
	SK_RING_BUFFER_FULL,
	SK_RING_BUFFER_HALFFULL,
};

#endif
