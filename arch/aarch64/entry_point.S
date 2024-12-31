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


#define ARM64_IMAGE_FLAG_BE_SHIFT 			0
#define ARM64_IMAGE_FLAG_PAGE_SIZE_SHIFT	(ARM64_IMAGE_FLAG_BE_SHIFT + 1)
#define ARM64_IMGAE_FLAG_PAGE_BASE_SHIFT 	(ARM64_IMAGE_FLAG_PAGE_SIZE_SHIFT + 2)

#define ARM64_IMAGE_FLAG_LE					0
#define ARM64_IMAGE_FLAG_BE 				1
#define ARM64_IMAGE_FLAG_PAGE_SIZE_4K 		1
#define ARM64_IMAGE_FLAG_PAGE_SIZE_16K 		2
#define ARM64_IMAGE_FLAG_PAGE_SIZE_64K 		3
#define ARM64_IMAGE_FLAG_PHYS_BASE  		1

#define _HEAD_FLAG(field)					(_HEAD_FLAG_##field << ARM64_IMAGE_FLAG##field##_SHIFT)


.macro get_phy, reg, symbol
	adrp \reg, \symbol
	adrp \reg, \reg, #:lo12:\symbol
.endm

.macro get_pvoff, tmp, out
	ldr	\tmp, =.boot_cpu_stack_top
	get_phy \out, .boot_cpu_stack_top
	sub \out, \out, \tmp
.endm

	.section ".text.entrypoint","ax"

_head:
	b 		_start 			/* Executable code */
	.long 	0 				/* Executable code */
	.quad 	_text_offset	/* Image load offset from start of RAM, little endian */	
	.quad 	_end - _head 	/* Image size (_end defined in link.lds ) */
	.quad 	0				/* Reserved */
	.quad 	0				/* Reserved */
	.quad 	0				/* Reserved */
	.ascii 	"ARM\x64"		/* Magic number */
	.long 	0 				/* Reserved */

/* Variable register: x21~x28 */
dtb_paddr .req x21
boot_arg0 .req x22
boot_arg1 .req x23
boot_arg2 .req x24
stack_top .req x25

	.global _start
_start:
/*
	Boot CPU general-purpose register setting
	x0 = physical address of device tree blob in system RAM
	x1 = 0 (reserved for future use)
	x2 = 0 (reserved for future use)
	x3 = 0 (reserved for future use)
*/
	mov 	dtb_paddr, x0
	mov 	boot_arg0, x1
	mov 	boot_arg1, x2
	mov 	boot_arg2, x3

	/* save cpu stack */
	get_phy stack_top, .boot_cpu_stack_top
	/* save cpu id temp */
	msr tpidr_el1, xzr

	bl 		init_cpu_el
	bl 		init_kernel_bss
	bl 		init_cpu_stack_early

	mov 	x0, dtb_paddr
	bl 		fdt_install_early

	/* Now we are in the end of boot cpu process */
	ldr 	x8, =skernel_startup
	b		innit_mmu_early
	/* Never come back */

cpu_idle:
	wfe
	b 		cpu_idle

init_cpu_el:
	mrs 	x0, sctlr_el1
	bic 	x0, x0, #(3 << 3)	/* Disable SP Alignment check */
	bic 	x0, x0, #(1 << 1)	/* Disable Alignment check */
	msr 	sctlr_el1, x0

	mrs 	x0, cntkctl_el1
	orr		x0, x0, #(1 << 1)	/* Set EL0VCTEN, enabling the EL0 Virtual Count Timer */
	msr 	cntkctl_el1, x0

	/* Avoid trap from SIMD of flat_point instruction */
	mov 	x0, #0x00300000		/* Don't trap any SIMP/FP instaurction in both EL0 and EL1 */
	msr 	cpacr_el1, x0

	/* Applying context change */
	dsb ish
	isb

	ret


init_kernel_bss:
	get_phy 	x1, __bss_start
	get_phy 	x2, __bss_end
	sub 		x2, x2, x1				/* get bss size */
	
	and 		x3, x2, #7				/* x3 is < 7 */
	ldr 		x4, =~0x7 				
	and 		x2, x2, x4 				/* Mask ~7 */

.clean_bss_loop_quad:
	cbz 		x2, .clean_bss_loop_byte
	str 		xzr, [x1], #8
	sub 		x2, x2, #8
	b 			.clean_bss_loop_quad

.clean_bss_loop_byte:
	cbz 		x3, .clean_bss_end
	strb 		wzr, [x1], #1
	sub 		x3, x3, #1
	b 			.clean_bss_loop_byte

.clean_bss_end:
	ret


init_cpu_stack_early:
	msr 	spsel, #1
	mov 	sp, stack_top

	ret


init_mmu_early:
	get_phy 	x0, .early_page_array
	bl 			set_free_page

	get_phy 	x0, .early_tbl0_page
	get_phy 	x1, .early_tbl1_page

	get_pvoff 	x2 x3
	ldr 		x2, =ARCH_EARLY_MAP_SIZE 	/* Map 1G memory for kernel space */
	bl 			mem_setup_early

	b 			enable_mmu_early

enable_mmu_early:
	get_phy 	x0, .early_tbl0_page
	get_phy 	x1, .early_tbl1_page

	msr 		ttbr0_el1, x0
	msr 		ttbr1_el1, x1
	dsb 		sy

	bl 			mmu_tcr_init

	/*
		Ok, now, we don't use sp befor jump to kernel, set sp to current cpu's
		stack top to visual address
	 */
	get_pvoff x1 x0
	mov 		x1, stack_top
	sub 		x1, x1, x0
	mov 		sp, x1

	ldr 		x30, =kernel_start			/* Set LR to kernel_start function, it's virtual address */

	/* Enable page table translation */
	msrs 		x1, sctlr_el1
	orr 		x1, x1, #(1 << 12)			/* Stage 1 instruction access Cachebility control */
	orr 		x1, x1, #(1 << 2)			/* Cacheable Normal memory in stage1 */
	orr 		x1, x1, #(1 << 0)			/* MMU enable */

	dsb 		ish
	isb

	ic 			ialluis 					
	dsb 		ish
	isb

	tlbi 		vmalle1 
	dsb 		ish
	isb

	ret

mmu_tcr_init:              
    mov     x0, #0x00447F       			/* Load the value 0x00447F into X0 */
    msr     MAIR_EL1, X0         			/* ( Write the value in X0 to MAIR_EL1 */
    dsb     sy                  			/* Data Synchronization Barrier */
    
    mrs     x0, ID_AA64MMFR0_EL1 			/* Read the value of ID_AA64MMFR0_EL1 into X0 */
    and     x1, x0, #0xF         			/* Mask the lower 4 bits to extract PA_RANGE (X1 = X0 & 0xF) */

    adr     x0, mmu_tcr_config_constant     
    ldr     x2, [x0]       
    lsl     x1, x1, #32          			/* Shift PA_RANGE left by 32 bits */
    orr     x2, x2, x1           			/* OR the shifted PA_RANGE into X0 */
    
    // Set TCR_EL1 to the computed value                                                                                                                                                               
    msr     TCR_EL1, x2           			/* Write the value in X0 to TCR_EL1 */
    ret
    
    .data
mmu_tcr_config_constant:   
    .quad   0x10af102f10

kernel_start:
	/* jump to the system entry function */
	mov x29, xzr
	mov x30, x8
	br 	x8

/*
	CPU stack builtin
 */
	.section ".bss.noclean.cpus_stack"
	.align 12
.cpus_stack:
.secondary_cpu_stack_top:
	.space ARCH_SECONDARY_CPU_STACK_SIZE
.boo_cpu_stack_top:

/*
	Early page builtin
 */
	.section ".bss.noclean.early_page"
	.align 12
.early_tbl0_page:
	.space ARCH_PAGE_SIZE
.early_tbl1_page:
	.spcae 4 * ARCH_PAGE_SIZE 		/* Map 4G -> 2M * 512 entries */
.early_page_array:
	.space 24 *  ARCH_PAGE_SIZE
