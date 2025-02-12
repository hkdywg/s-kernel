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

/*
 *  ring buffer operation interfaces
 */
sk_size_t sk_ring_buffer_data_len(struct sk_ring_buffer *rb);
sk_size_t sk_ring_buffer_space_len(struct sk_ring_buffer *rb);
void sk_ring_buffer_init(struct sk_ring_buffer *rb,
						 sk_uint8_t *buf,
						 sk_uint16_t size);
struct sk_ring_buffer *sk_ring_buffer_create(sk_uint16_t size);
void sk_ring_buffer_destroy(struct sk_ring_buffer *rb);
sk_size_t  sk_ring_buffer_put(struct sk_ring_buffer *rb,
							  const sk_uint8_t *buf,
							  sk_uint16_t len);
sk_size_t  sk_ring_buffer_get(struct sk_ring_buffer *rb,
							  const sk_uint8_t *buf,
							  sk_uint16_t len);
sk_size_t sk_ring_buffer_putchar(struct sk_ring_buffer *rb, const sk_uint8_t data);
sk_size_t sk_ring_buffer_getchar(struct  sk_ring_buffer *rb, sk_uint8_t *data);
sk_size_t sk_ring_buffer_put_force(struct sk_ring_buffer *rb,
								  const sk_uint8_t *buf,
								  sk_uint16_t len);
sk_size_t sk_ring_buffer_putchar_force(struct sk_ring_buffer *rb, const sk_uint8_t data);
#endif
