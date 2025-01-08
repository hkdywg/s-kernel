/*                                                                                                                                                                     
 *  type_def.h
 *
 *  brif
 *      base data struct definition
 *  
 *  (C) 2025.01.08 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#ifndef __TYPE_DEF_H_
#define __TYPE_DEF_H_

/* s-kernel version information */
#define SK_VERSION			1L		/* major version number */
#define SK_SUBVERSION		0L		/* minor version number */
#define SK_REVISION			0L		/* revise version number */

#define KERNEL_VERISON		((SK_VERSION * 10000) + (SK_SUBVERSION * 100) \
							 + SK_REVISION)

/* s-kernel basic data type definitions */
typedef signed char 				sk_int8_t;		/* 8bit integer type */
typedef signed short 				sk_int16_t; 	/* 16bit integer type */
typedef signed int 					sk_int32_t; 	/* 32bit integer type */
typedef unsigned char 				sk_uint8_t;		/* 8bit unsigned integer type */
typedef unsigned short 				sk_uint16_t;	/* 16bit unsigned integer type */
typedef unsigned int 				sk_uint32_t;	/* 32bit unsigned integer type */

typedef signed long 				sk_int64_t; 	/* 64bit integer type */
typedef unsigned long 				sk_uint64_t; 	/* 64bit unsigned integer type */

typedef int 						sk_bool_t; 		/* boolean type */
typedef long 						sk_base_t; 		/* Nbit CPU related data type */
typedef unsigned long 				sk_ubase_t; 	/* Nbit unsigned CPU related data type */

typedef sk_base_t 					sk_err_t;		/* type for error number */
typedef sk_uint32_t 				sk_time_t; 		/* type for time stamp */
typedef sk_uint32_t 				sk_tick_t;		/* type for tick count */
typedef sk_base_t 					sk_flag_t;		/* type for flags */
typedef sk_ubase_t 					sk_size_t;		/* type for size number */
typedef sk_ubase_t 					sk_dev_t;		/* type for device */
typedef sk_base_t 					sk_off_t;		/* type for offset */

/*  boolean type definition */
#define SK_TRUE 					1 				/* boolean true */
#define SK_FALSE 					0				/* boolean false */

/* maximum value  of base type */
#define SK_UINT8_MAX 				0xff 			/* maxium number of UINT8 */
#define SK_UINT16_MAX 				0xffff 			/* maxium number of UINT16 */
#define SK_UINT32_MAX 				0xffffffff 		/* maxium number of UINT32 */
#define SK_TICK_MAX 				SK_UINT32_MAX 	/* maxium number of tick */

/* error code definitions */
#define SK_EOK 						0				/* no error */
#define SK_ERROR 					-1				/* generic error */
#define SK_ETIMEOUT 				-2				/* time out */
#define SK_EFULL 					-3				/* resource is full */
#define SK_EEMPTY 					-4				/* resource is empty */
#define SK_ENOMEM					-5				/* no memory */
#define SK_EBUSY 					-6				/* busy */
#define SK_EIO	 					-7				/* IO error */
#define SK_EINVAL 	 				-8				/* Invalid argument */

/*
 *	@def SK_NULL
 *	Similar as the c NULL in C libary
 */
#define SK_NULL						(0)


#endif
