/*                                                                                                                                                                     
 *  base_def.h
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
#ifndef __BASE_DEF_H_
#define __BASE_DEF_H_

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


#define SK_NAME_MAX 				(16U)
#define SK_ALIGN_SIZE 				(4U)

/*
 *	@def SK_NULL
 *	Similar as the c NULL in C libary
 */
#define SK_NULL						(0)

/*
 * return the most contifuous size aligned.
 * sample
 * 		SK_ALIGN(22, 4) will return 24
 */
#define SK_ALIGN(size, align)	(((size) + (align) - 1) & ~((align) -1 ))

/*
 * return the most contifuous size aligned.
 * sample
 * 		SK_ALIGN(22, 4) will return 24
 */
#define SK_ALIGN_DOWN(size, align)	((size) & ~((align) -1 ))


/*
 * sk_memset
 * brief
 * 		set the content of memory to specified value
 * param
 * 		s: the address of source memory
 *		c: the value to be set
 *		count: the number of bytes to be set
 */
static inline void *sk_memset(void *s, int c, sk_ubase_t count)
{
	char *dst = (char *)s;
	while(count--)
		*dst++ = c;

	return s;
}

/*
 * sk_memcpy
 * brief
 * 		this function will copy memory content from source address to dst address
 * param
 * 		dst: the address of destribution memory
 * 		src: the address of source memory
 * 		count: the number of bytes need to be copy
 * 
 */
static inline void *sk_memcpy(void *dst, const void *src, sk_ubase_t count)
{
	char *d = (char *)dst;
	char *s = (char *)src;
	sk_ubase_t len;

	if(d <= s || d > (s + count)) {
		while(count--)
			*d++ = *s++;
	} else {
		for(len = count; len > 0; len--)
			d[len - 1] = s[len - 1];
	}
	return dst;
}

/*
 * sk_strcmy
 * brief
 * 		compare two strings
 * param
 * 		st_1: string to be compared
 * 		st_2: string to be compared
 */
static sk_int8_t sk_strcmp(const char *st_1, const char *st_2, sk_ubase_t count)
{
	sk_int8_t ret = 0;

	while(count) {
		if((ret = *st_1++ - *st_2) != 0 || !*st_2++)
			break;
		count--;
	}

	return ret;
}

                                                                                                                                                                      
/**                                                                                                                                                                   
 * This function finds the first bit set (beginning with the least significant bit)                                                                                   
 * in value and return the index of that bit.                                                                                                                         
 *                                                                                                                                                                    
 * Bits are numbered starting at 1 (the least significant bit).  A return value of                                                                                    
 * zero from any of these functions means that the argument was zero.                                                                                                 
 *                                                                                                                                                                    
 * @return Return the index of the first bit set. If value is 0, then this function                                                                                   
 *         shall return 0.                                                                                                                                            
 */                                                                                                                                                                   
static inline int __sk_ffs(int value)                                                                                                                                               
{                                                                                                                                                                     
	static const sk_uint8_t __lowest_bit_bitmap[] =                                                                                                                              
	{                                                                                                                                                                     
		/* 00 */ 0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* 10 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* 20 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* 30 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* 40 */ 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* 50 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* 60 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* 70 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* 80 */ 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* 90 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* A0 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* B0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* C0 */ 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* D0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* E0 */ 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,                                                                                                          
		/* F0 */ 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0                                                                                                           
	};                                                                                                                                                                    

    if (value == 0) return 0;                                                                                                                                         
                                                                                                                                                                      
    if (value & 0xff)                                                                                                                                                 
        return __lowest_bit_bitmap[value & 0xff] + 1;                                                                                                                 
                                                                                                                                                                      
    if (value & 0xff00)                                                                                                                                               
        return __lowest_bit_bitmap[(value & 0xff00) >> 8] + 9;                                                                                                        
                                                                                                                                                                      
    if (value & 0xff0000)                                                                                                                                             
        return __lowest_bit_bitmap[(value & 0xff0000) >> 16] + 17;                                                                                                    
                                                                                                                                                                      
    return __lowest_bit_bitmap[(value & 0xff000000) >> 24] + 25;                                                                                                      
} 

#endif

