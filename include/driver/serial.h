/*                                                                                                                                                                     
 *  serial.h
 *
 *  brif
 *      serial device driver header file
 *  
 *  (C) 2025.02.08 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef __SERIAL_H_
#define __SERIAL_H_

#include <base_def.h>
#include <completion.h>
#include <ring_buffer.h>

#define BAUD_RATE_2400 					2400
#define BAUD_RATE_9600 					9600
#define BAUD_RATE_19200 				19200
#define BAUD_RATE_38400 				38400
#define BAUD_RATE_115200 				115200
#define BAUD_RATE_921600 				921600

#define DATA_BITS_8						8
#define STOP_BITS_1 					0
#define PARITY_NONE 					0
#define PARITY_ODD 						1
#define PARITY_EVEN 					2

#define BIT_ORDER_LSB	 				0
#define BIT_ORDER_MSB 					1

#define FLOWCONTROL_NONE 				0
#define FLOWCONTROL_CTSRTS 				1

#define SK_SERIAL_RX_BUF_SIZE 			64
#define SK_SERIAL_TX_BUF_SIZE 			64

#define SK_SERIAL_RX_BLOCK 				0x100
#define SK_SERIAL_RX_NON_BLOCK 			0x200
#define SK_SERIAL_TX_BLOCK 				0x400
#define SK_SERIAL_TX_NON_BLOCK 			0x800

/* default confnig for serial_configure structure */
#define SK_DEFAULT_SERIAL_CONFIG \
{ \
 	BAUD_RATE_115200, 		/* 115200 bits/s */		\
	DATA_BITS_8, 			/* 8 databits */ 		\
	STOP_BITS_1, 			/* 1 stopbit */			\
	PARITY_NONE, 			/* No parity */			\
	FLOWCONTROL_NONE,		/* off flowcontrol  */	\
	BIT_ORDER_LSB, 			/* LSB first sent */	\
	SK_SERIAL_RX_BUF_SIZE,  /* rx buffer size */	\
	SK_SERIAL_TX_BUF_SIZE,  /* tx buffer size */	\
	0												\
}


/*
 * serial configure structure
 */
struct sk_serial_configure
{
	sk_uint32_t baud_rate;

	sk_uint32_t data_bits 		: 4;
	sk_uint32_t stop_bits 		: 2;
	sk_uint32_t parity 			: 2;
	sk_uint32_t flowcontrol 	: 1;
	sk_uint32_t bit_order 		: 1;
	sk_uint32_t rx_buf_size 	: 16;
	sk_uint32_t tx_buf_size 	: 16;
	sk_uint32_t reserved 		: 6;
};

/*
 * serial receive fifo structure
 */
struct sk_serial_rx_fifo
{
	struct sk_ring_buffer rb;
	struct sk_completion sk_cpt;
	sk_uint16_t sk_cpt_index;
	sk_uint8_t buffer[];
};

/*
 * serial send fifo structure
 */
struct sk_serial_tx_fifo
{
	struct sk_ring_buffer rb;
	sk_size_t put_size;
	sk_bool_t activated;
	struct sk_completion tx_cpt;
	sk_uint8_t buffer[];
};

struct sk_serial_ops;
/*
 * serial device structure
 */
struct sk_serial_device
{
	struct sk_device 	parent;

	const struct sk_serial_ops *ops;
	struct sk_serial_configure config;

	void *serial_rx;
	void *serial_tx;
};

/*
 * serial operations
 */
struct sk_serial_ops
{
	sk_err_t (*configure)(struct sk_serial_device *serial, struct sk_serial_configure *config);
	sk_err_t (*control)(struct sk_serial_device *serial, int cmd, void *arg);

	int(*putc)(struct sk_serial_device *serial, char c);
	int(*getc)(struct sk_serial_device *serial);

	sk_size_t (*transmit)(struct sk_serial_device *serial, sk_uint8_t *buf, sk_size_t size);
};

#endif

