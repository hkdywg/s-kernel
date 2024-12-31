/*                                                                                                                                                                     
 *  armv8.h
 *
 *  brif
 *      arm v8 register define
 *  
 *  (C) 2024.12.31 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#ifndef __ARMV8_H_
#define __ARMV8_H_

/* the exception stack without VFP register */
struct hw_exp_stack {
	unsigned long long pc;
	unsigned long long spsr;
	unsigned long long x30;
	unsigned long long xz;
	unsigned long long x28;
	unsigned long long x29;
	unsigned long long x26;
	unsigned long long x27;
	unsigned long long x24;
	unsigned long long x25;
	unsigned long long x22;
	unsigned long long x23;
	unsigned long long x20;
	unsigned long long x21;
	unsigned long long x18;
	unsigned long long x19;
	unsigned long long x16;
	unsigned long long x17;
	unsigned long long x14;
	unsigned long long x15;
	unsigned long long x12;
	unsigned long long x13;
	unsigned long long x10;
	unsigned long long x11;
	unsigned long long x8;
	unsigned long long x9;
	unsigned long long x6;
	unsigned long long x7;
	unsigned long long x4;
	unsigned long long x5;
	unsigned long long x2;
	unsigned long long x3;
	unsigned long long x0;
	unsigned long long x1;
};

#define SP_ELx				((unsigned long long)0x01)
#define SP_EL0				((unsigned long long)0x00)
#define PSTATE_EL1 			((unsigned long long)0x04)
#define PSTATE_EL2 			((unsigned long long)0x08)
#define PSTATE_EL3 			((unsigned long long)0x0c)

#endif

