/*                                                                                                                                                                     
 *  hw.h
 *
 *  brif
 *      hardware bottom-level operation interface
 *  
 *  (C) 2025.01.09 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#ifndef __HW_H_
#define __HW_H_

#include <type_def.h>

#ifndef __REG32
#define __REG32(x)							(*((volatile sk_uint32_t *)(x)))
#endif
#ifndef __REG16
#define __REG16(x)							(*((volatile sk_uint16_t *)(x)))
#endif
#ifndef __REG8
#define __REG8(x)							(*((volatile sk_uint8_t *)(x)))
#endif

enum HW_CACHE_OPS
{
	HW_CACHE_FLUSH 		= 0x01,
	HW_CACHE_INVALIDATE = 0x02,
};

/*
 * Interrupt handler definition
 */
typedef void (*sk_isr_handler_t)(int vector, void *param);

struct sk_irq_desc
{
	sk_isr_handler_t handler;
	void 			*param;
};

void sk_hw_interrupt_mask(int vector);
void sk_hw_interrupt_umask(int vector);
void sk_hw_interrupt_ack(int vector);
sk_int32_t sk_hw_interrupt_get_irq(void);
sk_isr_handler_t sk_hw_interrupt_install(int vector, sk_isr_handler_t handler, void *param);
void sk_hw_interrupt_init(void);

sk_base_t hw_interrupt_disable();
void hw_interrupt_enable(sk_base_t level);

#endif
