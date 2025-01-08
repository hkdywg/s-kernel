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

#include "type_def.h"

/* dist and cpu definitions */
#define GIC_DIST_BASE 				0x08000000
#define GIC_CPU_BASE				0x08010000
#define GIC_IRQ_START 				0
#define GIC_MAX_HANDLERS 			96

#endif
