/*                                                                                                                                                                     
 *  entry_point.S
 *
 *  brif
 *      kernel entry point
 *  
 *  (C) 2024.12.30 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */

#ifndef __ASSEMBLY__
#define __ASSEMBLY__
#endif

.section ".text.entrypoint","ax"

.globl _start
_start:
    bl      cpu_setup

cpu_idle:
    wfe
    b       cpu_idle

cpu_setup:
    ldr     x1, =_start
    bl      cpu_in_el1
    eret                            

cpu_in_el1:
    mov     sp, x1                  /* Set sp in el1 */

    /* Avoid trap from SIMD or float point instruction */
    mov     x1, #0x00300000         /* Don't trap any SIMD/FP instructions in both EL0 and EL1 */
    msr     cpacr_el1, x1

    mrs     x1, sctlr_el1
    orr     x1, x1, #(1 << 12)      /* Enable Instruction */
    bic     x1, x1, #(3 << 3)       /* Disable SP Alignment check */
    bic     x1, x1, #(1 << 1)       /* Disable Alignment check */
    msr     sctlr_el1, x1

    ldr     x1, =__bss_start
    ldr     w2, =__bss_size

clean_bss_loop:
    cbz     w2, jump_to_entry
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, clean_bss_loop

jump_to_entry:
    b       skernel_startup
    b       cpu_idle                /* For failsafe, halt this core too */

