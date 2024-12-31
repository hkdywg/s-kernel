/*                                                                                                                                                                     
 *  mmu.h
 *
 *  brif
 *      mmu operational interface
 *  
 *  (C) 2024.12.31 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#ifndef __MMU_H_
#define __MMU_H_

/*
 * CR1 bits
 */
#define CR_M 		(1 << 0)	/* MMU enable */
#define CR_A 		(1 << 1)	/* Alignment abort enable */
#define CR_C 		(1 << 2)	/* Dcache enable */
#define CR_W 		(1 << 3)	/* Write buffer enable */
#define CR_P 		(1 << 4)	/* 32-bit exception handler */
#define CR_D 		(1 << 5)	/* 32-bit data address range */
#define CR_L		(1 << 6)	/* Implementation defined */
#define CR_B		(1 << 7)	/* Big endian */
#define CR_S		(1 << 8)	/* System MMU protection */
#define CR_R		(1 << 9)	/* ROM MMU protection */
#define CR_F		(1 << 10)	/* Implementation defined */
#define CR_Z		(1 << 11)	/* Implementation defined */
#define CR_I		(1 << 12)	/* Icache enable */
#define CR_V		(1 << 13)	/* Vectors relocated to 0xffff0000 */
#define CR_RR		(1 << 14)	/* Round Robin cache replacement */
#define CR_L4		(1 << 15)	/* LDR pc can set T bit */
#define CR_DT		(1 << 16)	
#define CR_IT		(1 << 18)	
#define CR_ST		(1 << 19)	
#define CR_FI		(1 << 21)	/* Fast interrupt (lower latency mode) */
#define CR_U		(1 << 22)	/* Unaligned access operation */
#define CR_XP		(1 << 23)	/* Extended page tables */
#define CR_VE		(1 << 24)	/* Vectored interrupts */
#define CR_EE		(1 << 25)	/* Exception (big) Endian */
#define CR_TRE		(1 << 28)	/* TEX remap enable */
#define CR_AFE		(1 << 29)	/* Access flag enable */
#define CR_TE 		(1 << 30)	/* Thumb exception enable */

#define MMU_LEVEL_MASK 		0x1ffUL
#define MMU_MAP_ERROR_VANOTALIGN 		-1
#define MMU_MAP_ERROR_PANOTALIGN		-2
#define MMU_MAP_ERROR_NOPAGE 			-3
#define MMU_MAP_ERROR_CONFLICT 			-4

#define MEM_ATTR_MEMORY 		((0x1UL << 10) | (0x2UL << 8) | (0x0UL << 6) | (0x1UL << 2) | )
#define MEM_ATTR_IO 			((0x1UL << 10) | (0x2UL << 8) | (0x0UL << 6) | (0x2UL << 2) | )

#define BUS_ADDRESS(phys)		(((phys) & ~0xC0000000) | 0xC0000000)

void mmu_init(void);
void mmu_enable(void);
void armv8_map_large(unsigned long va, unsigned long pa, int count, unsigned long attr);
void armv8_map(unsigned long va, unsigned long pa, unsigned long size, unsigned long attr);

//dcache
void hw_dcache_enable(void);
void hw_dcache_flush_all(void);
void hw_dcache_flush_range(unsigned long start_addr, unsigned long size);
void hw_dcache_invalidate_range(unsigned long start_addr, unsigned long size);
void hw_dcache_invalidate_all(void);
void hw_dcache_disable(void);

//icache
void hw_icache_enable(void);
void hw_icache_invalidate_all(void);
void hw_icache_disable(void);
#endif

