/*                                                                                                                                                                     
 *  config.h
 *
 *  brif
 *      basic configure 
 *  
 *  (C) 2025.01.08 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#ifndef __CONFIG_H_
#define __CONFIG_H_

#include "base_def.h"

/* s-kernel version information */
#define SK_VERSION			1L		/* major version number */
#define SK_SUBVERSION		0L		/* minor version number */
#define SK_REVISION			0L		/* revise version number */

#define KERNEL_VERISON		((SK_VERSION * 10000) + (SK_SUBVERSION * 100) \
							 + SK_REVISION)


#define VIRTIO_SPI_IRQ_BASE 		32

/* dist and cpu definitions */
#define GIC_DIST_BASE 				0x08000000
#define GIC_CPU_BASE				0x08010000
#define GIC_IRQ_START 				0
#define GIC_MAX_HANDLERS 			96

#define TICK_PER_SECOND 			100

/* uart */
#define PL011_UART_DR 				0x000
#define PL011_UART_FR  				0x018
#define PL011_UART0_BASE 			0x09000000
#define PL011_UART0_SIZE 			0x00001000
#define PL011_UART0_IRQ_NUM 		(VIRTIO_SPI_IRQ_BASE + 1)
/* system console */
#define SK_CONSOLE_BUF_SIZE 		128
#define SK_CONSOLE_DEVICE_NAME 		"uart0"

#endif
