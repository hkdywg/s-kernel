/*                                                                                                                                                                     
 *  interrupt.h
 *
 *  brif
 *      kernel entry point
 *  
 *  (C) 2025.01.08 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#ifndef __INTERRUPT_H_
#define __INTERRUPT_H_

#include "base_def.h"

#define MAX_HANDLERS 				96

/*
 * Interrupt handler definition
 */
typedef void (*sk_isr_handler_t)(int vector, void *param);

void sk_hw_interrupt_mask(int vector);
void sk_hw_interrupt_umask(int vector);
void sk_hw_interrupt_ack(int vector);
sk_int32_t sk_hw_interrupt_get_irq(void);
sk_isr_handler_t sk_hw_interrupt_install(int vector, sk_isr_handler_t handler, void *param);
void sk_hw_interrupt_init(void);

#endif
