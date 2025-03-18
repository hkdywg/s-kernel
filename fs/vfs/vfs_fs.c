/*                                                                                                                                                                     
 *  vfs_fs.c
 *
 *  brif
 *      s-kernel file system operation of virtual file system 
 *  
 *  (C) 2025.03.18 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <vfs.h>

extern struct sk_vfs_filesystem_ops file_system_opt[VFS_MAX_FS_TYPE];
extern struct sk_vfs_filesystem file_system[VFS_MAX_FS_TYPE];

/*
 * sk_vfs_filesystem_lookup
 * brief
 * 		this function will return the filesystem mounted on specified path
 * param
 * 		path: specified path string
 */
struct sk_vfs_filesystem *sk_vfs_filesystem_lookup(const char *path)
{
	struct sk_vfs_filesystem *iter;
	struct sk_vfs_filesystem *fs = SK_NULL;

	/* lock filesystem */
	sk_vfs_lock();

	/* lookup it in filesystem table */
	for(iter = &file_system[0]; iter < &file_system[VFS_MAX_FS_TYPE]; iter++) {
		if(iter->path == SK_NULL || iter->ops == SK_NULL)
			continue;
		if(0 == sk_strcmp(iter->path, path, sk_strlen(iter->path)))
			break;
	}

	if(iter < &file_system[VFS_MAX_FS_TYPE])
		fs = iter;

	/* unlock filesystem */
	sk_vfs_unlock();

	return fs;
}
