/*                                                                                                                                                                     
 *  drv_uart.c
 *
 *  brief
 *      uart driver of board support package
 *  
 *  (C) 2025.02.12 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <serial.h>
#include <config.h>
#include <interrupt.h>

struct sk_uart_device
{
	sk_base_t hw_base;
	sk_uint32_t irq_num;
};

unsigned int readl(volatile void *addr)
{
	return *(volatile unsigned int *)addr;
}

void writel(volatile void *addr, unsigned int data)
{
	*(volatile unsigned int *)addr = data;
}

static int uart_getc(struct sk_serial_device *serial)
{
	int ch = -1;
	struct sk_uart_device *uart = (struct sk_uart_device *)serial->parent.user_data;

	if(!(readl((volatile void *)(uart->hw_base + 0x18)) & (1 << 4))) {
		ch = readl((volatile void *)(uart->hw_base)); 
	}

	return ch;
}

static int uart_putc(struct sk_serial_device *serial, char c)
{
	struct sk_uart_device *uart = (struct sk_uart_device *)serial->parent.user_data;
	
	writel((volatile void *)(uart->hw_base + PL011_UART_DR), c);

	return 1;
}

static  sk_size_t uart_transmit(struct sk_serial_device *serial, sk_uint8_t *buf, sk_size_t size)
{
	sk_size_t index = 0;
	while(size--) {
		uart_putc(serial, buf[index++]);
	}

	return 0;
}

static sk_err_t uart_configure(struct sk_serial_device *serial, struct sk_serial_configure *cfg)
{
	sk_uint32_t val;
	struct sk_uart_device *uart = (struct sk_uart_device *)serial->parent.user_data;  
	/* enable rx irq */
    val = readl((volatile void *)(uart->hw_base + 0x38));
    val |= 0x10;
    writel((volatile void *)(uart->hw_base + 0x38), val);

	return SK_EOK;
}

static sk_err_t uart_control(struct sk_serial_device *serial, int cmd, void *arg)
{
	return SK_EOK;
}

static void sk_hw_uart_isr(int irqno, void *param)
{
	struct sk_serial_device *serial = (struct sk_serial_device *)param;

	sk_hw_serial_isr(serial, SK_SERIAL_EVENT_RX_IND);
}

static const struct sk_serial_ops __uart_ops = {
	uart_configure,
	uart_control,
	uart_putc,
	uart_getc,
	uart_transmit,
};

static struct sk_uart_device __uart_device = {
	PL011_UART0_BASE,
	PL011_UART0_IRQ_NUM,
};

static struct sk_serial_device __serial0;

int sk_hw_uart_init()
{
	struct sk_uart_device *uart;
	struct sk_serial_configure config = SK_DEFAULT_SERIAL_CONFIG;

	uart  			 = &__uart_device;
	__serial0.ops 	 = &__uart_ops;
	__serial0.config = config;

	/* register uart0 device */
	sk_hw_serial_register(&__serial0, "uart0", 0, uart);

	sk_hw_interrupt_install(uart->irq_num, sk_hw_uart_isr, &__serial0);
	sk_hw_interrupt_umask(uart->irq_num);

	return 0;
}



