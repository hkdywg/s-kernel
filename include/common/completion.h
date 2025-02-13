/*                                                                                                                                                                     
 *  completion.h
 *
 *  brif
 *      completion
 *  
 *  (C) 2025.02.10 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef __COMPLETION_H_
#define __COMPLETION_H_

#include <base_def.h>
#include <klist.h>

/*
 * completion
 */
struct sk_completion
{
	sk_uint32_t  flag;
	/* suspended list */
	sk_list_t suspend_list;
};

void sk_completion_init(struct sk_completion *cpt);
sk_err_t sk_completion_wait(struct sk_completion *cpt, sk_int32_t timeout);
void sk_completion_done(struct sk_completion *cpt);

#endif
