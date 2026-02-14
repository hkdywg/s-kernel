/*                                                                                                                                                                     
 *  ramfs.h
 *
 *  brief
 *      ram file system relative definition
 *  
 *  (C) 2026.02.14 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#ifndef __RAM_FS_H_
#define __RAM_FS_H_

#include <vfs.h>

#define RAMFS_NAME_MAX  32
#define RAMFS_MAGIC     0x296328

struct sk_ramfs;

struct sk_ramfs_dirent {
    sk_list_t list;
    struct sk_ramfs *fs;        /* file system ref */

    char name[RAMFS_NAME_MAX];  /* dirent name */
    sk_uint8_t *data;

    sk_size_t size;             /* file size */
};

#endif
