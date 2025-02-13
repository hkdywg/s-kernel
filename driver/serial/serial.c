/*                                                                                                                                                                     
 *  serial.c
 *
 *  brif
 *      serial device  driver 
 *  
 *  (C) 2025.02.08 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <kobj.h>
#include <device.h>
#include <serial.h>
#include <skernel.h>

sk_size_t __serial_poll_tx(struct sk_device *dev,
						   sk_size_t 		pos,
						   const void 		*buf,
						   sk_size_t 		size)
{
	struct sk_serial_device *serial = (struct sk_serial_device *)dev;
	sk_uint8_t *putc_buf = (sk_uint8_t *)buf;
	
	while(size) {
		serial->ops->putc(serial, *putc_buf);
		++putc_buf;
		--size;
	}
	return size;
}

static sk_size_t sk_serial_get_linear_buf(struct sk_ring_buffer *rb, sk_uint8_t **ptr)
{
	sk_size_t  size;

	/* whether has enough data */
	size = sk_ring_buffer_data_len(rb);

	if(size == 0)
		return 0;

	*ptr = &rb->buffer[rb->read_index];

	if(rb->buffer_size - rb->read_index > size)
		return size;

	return rb->buffer_size - rb->read_index;
}

/*
 * sk_serial_init
 * brief
 * 		Initialize the serial device
 * param
 * 		dev: the pointer of device driver structure
 */
static sk_err_t sk_serial_init(struct sk_device *dev)
{
	sk_err_t ret = SK_EOK;
	struct sk_serial_device *serial = (struct sk_serial_device *)dev;

	/* initialize rx/tx */
	serial->serial_rx = SK_NULL;
	serial->serial_tx = SK_NULL;

	/* apply configureation */
	if(serial->ops->configure)
		ret = serial->ops->configure(serial, &serial->config);

	return ret;
}

/* */
static sk_err_t sk_serial_rx_enable(struct sk_device *dev)
{
	sk_err_t ret = SK_EOK;
	struct sk_serial_rx_fifo *rx_fifo = SK_NULL; 
	struct sk_serial_device *serial = (struct sk_serial_device *)dev;

	/* if the device flag use SK_SERIAL_RX_NON_BLOCK, the rx_buf_size 
	 * can't be set 0 */
	if((serial->config.rx_buf_size == 0) &&
	   (dev->flag & SK_SERIAL_RX_NON_BLOCK))
		return SK_EINVAL;

	/* allocate fifo memory */
	rx_fifo = (struct sk_serial_rx_fifo *)sk_malloc(sizeof(struct sk_serial_rx_fifo) + serial->config.rx_buf_size);
	//SK_ASSERT(rx_fifo != SK_NULL);
	sk_ring_buffer_init(&(rx_fifo->rb), rx_fifo->buffer, serial->config.rx_buf_size);

	serial->serial_rx = rx_fifo;
}

static sk_err_t sk_serial_tx_enable(struct sk_device *dev)
{
	struct sk_serial_device *serial = (struct sk_serial_device *)dev;
	struct sk_serial_tx_fifo *tx_fifo = SK_NULL;

	if(serial->config.tx_buf_size == 0) {
		return SK_EOK;
	}

	if(serial->parent.flag & SK_SERIAL_TX_NON_BLOCK) {
		/* allocate fifo memory */
		tx_fifo = (struct sk_serial_tx_fifo *)sk_malloc(sizeof(struct sk_serial_tx_fifo) + serial->config.tx_buf_size);
		//SK_ASSERT(tx_fifo != SK_NULL);
		sk_ring_buffer_init(&(tx_fifo->rb), tx_fifo->buffer, serial->config.tx_buf_size);
		tx_fifo->activated = SK_FALSE;
		serial->serial_tx = tx_fifo;
	}
}

/*
 * sk_serial_open
 * brief
 * 		open the serial device
 * param
 * 		dev: the pointer of device driver structure
 * 		flag: the flag of serial port open
 */
static sk_err_t sk_serial_open(struct sk_device *dev, sk_uint16_t flag)
{
	sk_err_t ret = SK_EOK;
	struct sk_serial_device *serial = (struct sk_serial_device *)dev;

	/* check the device has been open */
	if(dev->flag & SK_DEVICE_FLAG_OPEN)
		return SK_EOK;

	/* set the device flag */
	dev->flag |= flag;

	/* initialize the rx structure */
	if(serial->serial_rx == SK_NULL)
		sk_serial_rx_enable(dev);

	/* initialize the tx structure */
	if(serial->serial_tx == SK_NULL)
		sk_serial_tx_enable(dev);

	return ret;
}

sk_err_t sk_serial_close(struct sk_device *dev)
{
	return SK_EOK;
}

sk_size_t sk_serial_read(struct sk_device *dev, sk_size_t pos, void *buf, sk_size_t size)
{
	return SK_EOK;
}

sk_size_t sk_serial_write(struct sk_device *dev, sk_size_t pos, const void  *buf, sk_size_t size)
{
	struct sk_serial_device *serial = (struct sk_serial_device *)dev;
	struct sk_serial_tx_fifo *tx_fifo;
	sk_size_t length;

	if(size == 0)
		return 0;

	tx_fifo = serial->serial_tx;
	if(serial->config.tx_buf_size == 0) {
		return __serial_poll_tx(dev, pos, buf, size);
	}

	if(tx_fifo->activated == SK_FALSE) {
		/* set activate to true, and start copy data to ring buffer */
		tx_fifo->activated = SK_TRUE;
		/* copy data to ring buffer */
		length = sk_ring_buffer_put(&(tx_fifo->rb), buf, size);

		sk_uint8_t *put_ptr = SK_NULL;
		/* get the linear length  buffer from ring buffer */
		tx_fifo->put_size = sk_serial_get_linear_buf(&(tx_fifo->rb), &put_ptr);
		/* call the transmit interface for transmission */
		serial->ops->transmit(serial,
							  put_ptr,
							  tx_fifo->put_size);

		return length;
	}

	/* if the tx_fifo->activated mode is SK_TRUE, it means that serial device is transmitting,
	 * where  only the data in the ring buffer and there is no need to call the transmit() API */
	length = sk_ring_buffer_put(&(tx_fifo->rb), buf, size);

	return length;
}

sk_err_t sk_serial_control(struct sk_device *dev,  int cmd, void *args)
{
	return SK_EOK;
}

const static struct sk_device_ops serial_ops = {
	sk_serial_init,
	sk_serial_open,
	sk_serial_close,
	sk_serial_read,
	sk_serial_write,
	sk_serial_control,
};

/*
 * sk_hw_serial_register
 * breif
 * 		register the serial device
 * param
 * 		serial: the pointer of serial device 
 * 		name: device name
 * 		flag: the capabilities flag of device
 * 		data: device driver's data
 */
sk_err_t sk_hw_serial_register(struct sk_serial_device  *serial,
							   const char *name, sk_uint32_t flag, void *data)
{
	sk_err_t ret;
	struct sk_device *device = &(serial->parent);

	device->type 		= SK_DEVICE_CHAR;
	device->rx_indicate = SK_NULL;
	device->tx_complete = SK_NULL;
	device->ops 		= &serial_ops;
	device->user_data   = data;

	/* register a character device */
	ret = sk_device_register(device, name, flag);

	return ret;
}

void sk_hw_serial_isr(struct sk_serial_device *serial, int event)
{
	switch(event)
	{
		case SK_SERIAL_EVENT_RX_IND:
		case SK_SERIAL_EVENT_RX_DMADONE:
		{
			sk_size_t rx_length = 0;
			struct  sk_serial_rx_fifo *rx_fifo;
			rx_fifo = (struct sk_serial_rx_fifo *)serial->serial_rx;
			
			/* get the length of the data from ring buffer */
			rx_length = sk_ring_buffer_data_len(&(rx_fifo->rb));
			if(rx_length == 0)
				break;
			/* trigger the receiving completion callback */
			if(serial->parent.rx_indicate != SK_NULL)
				serial->parent.rx_indicate(&(serial->parent), rx_length);

			break;
		}
		case SK_SERIAL_EVENT_TX_DONE:
		{
			struct  sk_serial_tx_fifo *tx_fifo;
			tx_fifo = (struct sk_serial_tx_fifo *)serial->serial_tx;
			break;
		}
	}
}
