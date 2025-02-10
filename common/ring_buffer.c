/*                                                                                                                                                                     
 *  ring_buffer.c
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
#include <ring_buffer.h>
#include <skernel.h>

enum sk_ring_buffer_state sk_get_ring_buffer_status(struct sk_ring_buffer *rb)
{
	if(rb->read_index == rb->write_index)
	{
		if(rb->read_mirror == rb->write_mirror)
			return SK_RING_BUFFER_EMPTY;
		else
			return SK_RING_BUFFER_FULL;
	}
	return SK_RING_BUFFER_HALFFULL;
}

/*
 * sk_ring_buffer_data_len
 * brief
 * 		get the size of data in ring buffer
 * param
 * 		rb: the pointer of ring buffer object
 */
sk_size_t sk_ring_buffer_data_len(struct sk_ring_buffer *rb)
{
	switch(sk_get_ring_buffer_status(rb))
	{
		case SK_RING_BUFFER_EMPTY:
			return 0;
		case SK_RING_BUFFER_FULL:
			return rb->buffer_size;
		case SK_RING_BUFFER_HALFFULL:
			if(rb->write_index > rb->read_index)
				return rb->write_index - rb->read_index;
			else
				return rb->buffer_size - (rb->read_index - rb->write_index);
	}
}

/*
 * sk_ring_buffer_space_len
 * brief
 * 		get the size of empty space in ring buffer 
 * param
 * 		rb: the pointer of ring buffer object
 */
sk_size_t sk_ring_buffer_space_len(struct sk_ring_buffer *rb)
{
	return (rb->buffer_size - sk_ring_buffer_data_len(rb));
}

/*
 * sk_ring_buffer_init
 * brief
 * 		initialize the ring buffer
 * param
 * 		rb: the pointer of ring buffer object
 * 		buf: data buffer
 * 		size: the size of data buffer
 */
void sk_ring_buffer_init(struct sk_ring_buffer *rb,
						 sk_uint8_t *buf,
						 sk_uint16_t size)
{
	/* initialize read write index */
	rb->read_mirror = rb->read_index = 0;
	rb->write_mirror = rb->write_index = 0;

	/* set ring buffer and size */
	rb->buffer  	= buf;
	rb->buffer_size = size;

}

/*
 * sk_ring_buffer_create
 * brief
 * 		create a ring buffer with given size
 * param
 * 		size: the size of ring buffer
 */
struct sk_ring_buffer *sk_ring_buffer_create(sk_uint16_t size)
{
	struct sk_ring_buffer *rb;
	sk_uint8_t *buf;

	rb = (struct sk_ring_buffer *)sk_malloc(sizeof(struct sk_ring_buffer));
	if(rb == SK_NULL)
		return SK_NULL;

	buf = (sk_uint8_t *)sk_malloc(size);
	if(buf == SK_NULL) {
		sk_free(rb);
		return SK_NULL;
	}
	/* initialize the ring buffer */
	sk_ring_buffer_init(rb, buf, size);

	return rb;
}

/*
 * sk_ring_buffer_destroy
 * brief
 * 		destroy the ring buffer object
 * param
 * 		rb: the pointer of ring buffer, need to be destroyed
 * 
 */
void sk_ring_buffer_destroy(struct sk_ring_buffer *rb)
{
	if(rb != SK_NULL) {
		sk_free(rb->buffer);
		sk_free(rb);
	}
}

/*
 * sk_ring_buffer_put
 * brief
 * 		put a block data to the ring buffer
 * param
 * 		rb: the pointer of ring buffer object
 * 		buf: the pointer of data buffer, need to be put
 * 		len: the size of data 
 */
sk_size_t  sk_ring_buffer_put(struct sk_ring_buffer *rb,
							  const sk_uint8_t *buf,
							  sk_uint16_t len)
{
	sk_uint16_t size, side_len, put_len = len;

	/* whether has enough space */
	size = sk_ring_buffer_space_len(rb);

	/* if no space */
	if(size == 0)
		return 0;

	/* drop some data */
	if(size < len)
		put_len = size;

	side_len = rb->buffer_size - rb->write_index;

	if(side_len > put_len) {
		sk_memcpy((void *)&rb->buffer[rb->write_index], (const void *)buf, put_len);	
		rb->write_index += put_len;
		return put_len;
	}

	sk_memcpy(&rb->buffer[rb->write_index], &buf[0], side_len);
	sk_memcpy(&rb->buffer[0], &buf[side_len], put_len - side_len);
	/* need to use other side of the mirror */
	rb->write_mirror = ~rb->write_mirror;
	rb->write_index = put_len - side_len;

	return put_len;
}

/*
 * sk_ring_buffer_get
 * brief
 * 		get a block data from ring buffer
 * param
 * 		rb: the pointer of ring buffer object
 * 		buf: the pointer of data buffer, save the get data
 * 		len: the size of data needed to be read from ring buffer 
 */
sk_size_t  sk_ring_buffer_get(struct sk_ring_buffer *rb,
							  const sk_uint8_t *buf,
							  sk_uint16_t len)
{
	sk_uint16_t size, side_len, get_len  = len;

	/*  whether has enough data */
	size = sk_ring_buffer_data_len(rb);

	/* no data */
	if(size == 0)
		return 0;

	/* less data */
	if(size < len)
		get_len = size;

	side_len = rb->buffer_size - rb->read_index;
	if(side_len > get_len) {
		sk_memcpy((void *)buf, (const void *)&rb->buffer[rb->read_index], get_len);
		rb->read_index += get_len;
		return get_len;
	}

	sk_memcpy((void *)&buf[0], (const void  *)&rb->buffer[rb->read_index], side_len);
	sk_memcpy((void *)&buf[side_len], (const void  *)&rb->buffer[0], get_len - side_len);

	/* need to use other side of the mirror */
	rb->read_mirror = ~rb->read_mirror;
	rb->read_index = get_len - side_len;

	return get_len;
}

/*
 * sk_ring_buffer_putchar
 * brief
 * 		put one data to the ring buffer
 */
sk_size_t sk_ring_buffer_putchar(struct sk_ring_buffer *rb, const sk_uint8_t data)
{
	/* whether has enougn space */
	if(!sk_ring_buffer_space_len(rb))
		return 0;

	rb->buffer[rb->write_index] = data;

	if(rb->write_index + 1 == rb->buffer_size) {
		rb->write_index = 0;
		rb->write_mirror = ~rb->write_mirror;
	} else {
		rb->write_index++;
	}
		
	return 1;
}

/*
 * sk_ring_buffer_getchar
 * brief
 * 		get one data from ring buffer
 */
sk_size_t sk_ring_buffer_getchar(struct  sk_ring_buffer *rb, sk_uint8_t *data)
{
	/* whether ring buffer is empty */
	if(!sk_ring_buffer_data_len(rb))
		return 0;

	*data = rb->buffer[rb->read_index];

	if(rb->read_index + 1 == rb->buffer_size) {
		rb->read_index = 0;
		rb->read_mirror =  ~rb->read_mirror;
	} else {
		rb->read_index ++;
	}
	
	return 1;
}

/*
 * sk_ring_buffer_put_force
 */
sk_size_t sk_ring_buffer_put_force(struct sk_ring_buffer *rb,
								  const sk_uint8_t *buf,
								  sk_uint16_t len)
{
	sk_uint16_t space_size, side_len, put_len = len;

	/* whether has enough space */
	space_size = sk_ring_buffer_space_len(rb);

	side_len = rb->buffer_size - rb->write_index;

	if(side_len > put_len) {
		sk_memcpy(&rb->buffer[rb->write_index], buf, put_len);	
		rb->write_index += put_len;
		return put_len;
	}

	sk_memcpy(&rb->buffer[rb->write_index], &buf[0], side_len);
	sk_memcpy(&rb->buffer[0], &buf[side_len], put_len - side_len);
	/* need to use other side of the mirror */
	rb->write_mirror = ~rb->write_mirror;
	rb->write_index = put_len - side_len;

	if(put_len > space_size) {
		if(rb->write_index <= rb->read_index)
			rb->read_mirror = ~rb->read_mirror;
		rb->read_index = rb->write_index;
	}

	return put_len;
}

/*
 * sk_ring_buffer_putchar_force
 * brief
 * 		put one data to the ring buffer
 */
sk_size_t sk_ring_buffer_putchar_force(struct sk_ring_buffer *rb, const sk_uint8_t data)
{
	enum sk_ring_buffer_state state = sk_get_ring_buffer_status(rb);
	rb->buffer[rb->write_index] = data;

	if(rb->write_index + 1 == rb->buffer_size) {
		rb->write_index = 0;
		rb->write_mirror = ~rb->write_mirror;
		if(state == SK_RING_BUFFER_FULL) {
			rb->write_mirror = ~rb->write_mirror;
			rb->read_index = rb->write_index;
		}
	} else {
		rb->write_index++;
		if(state == SK_RING_BUFFER_FULL) {
			rb->read_index = rb->write_index;
		}
	}
		
	return 1;
}

