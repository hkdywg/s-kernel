/*                                                                                                                                                                     
 *  interrupt.c
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
#include <type_def.h>
#include <config.h>
#include <interrupt.h>
#include <armv8.h>
#include <hw.h>

/* Macro to accress the generic interrupt controller interface */
#define GIC_CPU_CTRL(hw_base)				__REG32((hw_base) + 0x00U)
#define GIC_CPU_PRIMASK(hw_base)			__REG32((hw_base) + 0x04U)
#define GIC_CPU_BINPOINT(hw_base)			__REG32((hw_base) + 0x08U)
#define GIC_CPU_INTACK(hw_base)				__REG32((hw_base) + 0x0CU)
#define GIC_CPU_EOI(hw_base)				__REG32((hw_base) + 0x10U)
#define GIC_CPU_RUNNINGPRI(hw_base)			__REG32((hw_base) + 0x14U)
#define GIC_CPU_HIGHPRI(hw_base)			__REG32((hw_base) + 0x18U)
#define GIC_CPU_IIDR(hw_base)				__REG32((hw_base) + 0xFCU)

/* Macro to accress the generic interruot controller distributor */
#define GIC_DIST_CTRL(hw_base)				__REG32((hw_base) + 0x00U)
#define GIC_DIST_TYPE(hw_base)				__REG32((hw_base) + 0x04U)
#define GIC_DIST_IGROUP(hw_base, n)			__REG32((hw_base) + 0x080U + ((n)/32U) * 4U)
#define GIC_DIST_ENABLE_SET(hw_base, n)		__REG32((hw_base) + 0x100U + ((n)/32U) * 4U)
#define GIC_DIST_ENABLE_CLEAR(hw_base, n)	__REG32((hw_base) + 0x180U + ((n)/32U) * 4U)
#define GIC_DIST_PENDING_SET(hw_base, n)	__REG32((hw_base) + 0x200U + ((n)/32U) * 4U)
#define GIC_DIST_PENDING_CLEAR(hw_base, n)	__REG32((hw_base) + 0x280U + ((n)/32U) * 4U)
#define GIC_DIST_ACTIVE_SET(hw_base, n)		__REG32((hw_base) + 0x300U + ((n)/32U) * 4U)
#define GIC_DIST_ACTIVE_CLEAR(hw_base, n)	__REG32((hw_base) + 0x380U + ((n)/32U) * 4U)
#define GIC_DIST_PRI(hw_base, n)			__REG32((hw_base) + 0x400U +  ((n)/4U) * 4U)
#define GIC_DIST_TARGET(hw_base, n)			__REG32((hw_base) + 0x800U +  ((n)/4U) * 4U)
#define GIC_DIST_CONFIG(hw_base, n)			__REG32((hw_base) + 0xC00U + ((n)/16U) * 4U)
#define GIC_DIST_SOFTINT(hw_base)			__REG32((hw_base) + 0xF00U)
#define GIC_DIST_CPENDSGI(hw_base, n)		__REG32((hw_base) + 0xF10U +  ((n)/4U) * 4U)
#define GIC_DIST_SPENDSGI(hw_base, n)		__REG32((hw_base) + 0xF20U +  ((n)/4U) * 4U)
#define GIC_DIST_ICPIDR2(hw_base)			__REG32((hw_base) + 0xFE8U)

struct gic_info
{
	sk_uint64_t offset;			/* the first interrupt index in the vector table */
	sk_uint64_t dist_base;		/* the base address of the gic distributor */
	sk_uint64_t cpu_base;		/* the base address of the gic cpu interface */
};
static struct gic_info gic_ctl;

/* exception and interrupt handler table */
struct sk_irq_desc isr_table[GIC_MAX_HANDLERS];

const unsigned int vector_base = 0x00;

static void default_isr_handler(int vector, void *param)
{

}

void sk_hw_vector_init(void)
{
	set_current_vbar();
}

/*
 * This function will mask a interrupt 
 * @param: 
 * 		vector: the interrupt number
 */
void sk_hw_interrupt_mask(int vector)
{
	sk_uint64_t mask = 1U << (vector % 32U);
	sk_int32_t  irq = vector - gic_ctl.offset;

	GIC_DIST_ENABLE_CLEAR(gic_ctl.dist_base, irq) = mask;
}

/*
 * This function will umask a interrupt 
 * @param: 
 * 		vector: the interrupt number
 */
void sk_hw_interrupt_umask(int vector)
{
	sk_uint64_t mask = 1U << (vector % 32U);
	sk_int32_t  irq = vector - gic_ctl.offset;

	GIC_DIST_ENABLE_SET(gic_ctl.dist_base, irq) = mask;
}

/*
 * This function will return eth active interrupt number
 * @param: none
 */
sk_int32_t sk_hw_interrupt_get_irq(void)
{
	sk_int32_t irq;

	irq = GIC_CPU_INTACK(gic_ctl.cpu_base);
	irq += gic_ctl.offset;
	return irq;
}

/*
 * This function acknowledges the interrupt
 * @param: 
 * 		vector: the interrupt number
 */
void sk_hw_interrupt_ack(int vector)
{
	sk_uint64_t mask = 1U << (vector % 32U);
	sk_int32_t  irq = vector - gic_ctl.offset;

	GIC_DIST_PENDING_CLEAR(gic_ctl.dist_base, irq) = mask;
	GIC_CPU_EOI(gic_ctl.cpu_base) = irq;
}


/*
 * This function will install a interrupt service routine to a interrupt
 * @param: 
 * 		vector: the interrupt number
 */
sk_isr_handler_t sk_hw_interrupt_install(int vector, sk_isr_handler_t handler,
								void *param)
{
	sk_isr_handler_t old_handler = SK_NULL;

	if(vector < GIC_MAX_HANDLERS) {
		old_handler = isr_table[vector].handler;

		if(handler != SK_NULL) {
			isr_table[vector].handler = handler;
			isr_table[vector].param = param;
		}
	}

	return old_handler;
}

sk_int32_t gic_dist_init(sk_uint64_t index, sk_uint64_t dist_base, sk_uint32_t irq_start)
{
	sk_uint32_t gic_type, gic_max_irq, i;
	sk_uint64_t cpu_mask = 1U << 0U;

	gic_ctl.dist_base = dist_base;
	gic_ctl.offset = irq_start;

	/* Find out how many interrupts are supported */
	gic_type = GIC_DIST_TYPE(dist_base);
	gic_max_irq = ((gic_type & 0x1F) + 1) * 32;

	/*
	 * The GIC only supports up to 1020 interrupt sources
	 */
	if(gic_max_irq > 1020)
		gic_max_irq = 1020;

	cpu_mask |= cpu_mask << 8U;
	cpu_mask |= cpu_mask << 16U;
	cpu_mask |= cpu_mask << 24U;

	GIC_DIST_CTRL(dist_base) = 0x0;

	/* set all global interrupts to be level triggered, active low */
	for(i  = 32; i < gic_max_irq; i += 16) {
		GIC_DIST_CONFIG(dist_base, i) = 0x0;
	}

	/* set all global interrupts to this cpu only */
	for(i  = 32; i < gic_max_irq; i += 4) {
		GIC_DIST_TARGET(dist_base, i) = cpu_mask;
	}

	/* set priority on all interrupts */
	for(i  = 0; i < gic_max_irq; i += 4) {
		GIC_DIST_PRI(dist_base, i) = 0xa0a0a0a0;
	}
	
	/* disable all interrupts */
	for(i  = 0; i < gic_max_irq; i += 32) {
		GIC_DIST_ENABLE_CLEAR(dist_base, i) = 0xffffffff;
	}

	/* All interrupts defaults to IGROUP1(IRQ) */
	for(i  = 0; i < gic_max_irq; i += 32) {
		GIC_DIST_IGROUP(dist_base, i) = 0x0;
	}

	/* Enable group0 and group1 interrupt forwarding */
	GIC_DIST_CTRL(dist_base) = 0x01;

	return 0;
}

sk_int32_t gic_cpu_init(sk_uint64_t index, sk_uint64_t cpu_base)
{
	gic_ctl.cpu_base = cpu_base;

	GIC_CPU_PRIMASK(cpu_base) = 0xF0;
	GIC_CPU_BINPOINT(cpu_base) = 0x7;

	/* Enable CPU innterrupt */
	GIC_CPU_CTRL(cpu_base) = 0x01;

	return 0;
}



/*
 * This function will initialize hardware interrupt
 */
void sk_hw_interrupt_init(void)
{	
	/* initialize vector table */
	sk_hw_vector_init();

	/* initialize exceptions table */
	//memset(isr_table, 0x00, sizeof(isr_table));
	
	/* initialize gic */
	gic_dist_init(0, GIC_DIST_BASE, GIC_IRQ_START);
	gic_cpu_init(0, GIC_CPU_BASE);
}

