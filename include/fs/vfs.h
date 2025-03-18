/*                                                                                                                                                                     
 *  vfs.h
 *
 *  brief
 *      virtual file system relative definition
 *  
 *  (C) 2025.03.18 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */
#ifndef __VFS_H_
#define __VFS_H_

#include <base_def.h>
#include <skernel.h>
#include <ipc.h>

/* vfs max number */
#define VFS_MAX_FS_TYPE 		(3)
#define VFS_MAX_DIRENT_NAME		(256)
#define VFS_MAX_FD 				(16)
#define VFS_FD_MAGIC 			(0x142431)

/* file type */
#define FT_REGULAR 				1
#define FT_SOCKET 				2
#define FT_DIRECTORY 			4
#define FT_DEVICE 				8

/* file flags */
#define VFS_F_OPEN				0x100
#define VFS_F_DIRECTORY			0x200
#define VFS_F_EOF				0x400

struct sk_vfs_file_ops;
struct sk_vfs_filesystem_ops;

/*
 * File descriptor
 */
struct sk_vfs_fd
{
	sk_uint32_t magic;			/* file descriptor magic number */
	sk_uint16_t type; 			/* Type (regular or socket or device) */

	char *path;					/* name (below mounnt point) */
	sk_uint32_t ref_count;		/* descriptor reference count */

	struct sk_vfs_filesystem *fs;
	const struct sk_vfs_file_ops *fops;

	sk_uint32_t flag; 			/* descriptor flag */
	sk_size_t 	size;			/* size in bytes */
	sk_size_t 	pos;			/* current file position */

	void *data;					/* specific file system data */
};


/*
 * File operation structure
 */
struct sk_vfs_file_ops
{
	int (*open)		(struct sk_vfs_fd *fd);
	int (*close)	(struct sk_vfs_fd *fd);
	int (*read)		(struct sk_vfs_fd *fd, void *buf, sk_size_t count);
	int (*write)	(struct sk_vfs_fd *fd, const void *buf, sk_size_t count);
	int (*lseek)	(struct sk_vfs_fd *fd, sk_size_t offset);
};


/*
 * File system operation structure
 */
struct sk_vfs_filesystem_ops
{
	char *name;
	sk_uint32_t flag;					/* flags for file system operations */

	/* operations for file */
	const struct sk_vfs_file_ops *fops;	

	/* mount and unmount for file system */
	int (*mount)   (struct sk_vfs_filesystem *fs, sk_uint32_t flag, const void *data);
	int (*unmount) (struct sk_vfs_filesystem *fs);

	/* make a file system */
	int (*mkfs)   (sk_base_t devid);
//	int (*statfs) (struct sk_vfs_filesystem *fs, struct statfs *buf);

	int (*unlink) (struct sk_vfs_filesystem *fs, const char *path_name);
//	int (*stat)   (struct sk_vfs_filesystem *fs, const char *file_name, struct stat *buf);
	int (*rename) (struct sk_vfs_filesystem *fs, const char *old_path, const char *new_path);
};

/*
 * Mounted file system
 */
struct sk_vfs_filesystem
{
	sk_base_t dev_id;							/* attached device */
	char *path;									/* file system mount point */
	const struct sk_vfs_filesystem_ops *ops;	/* operations for file system type */
	void *data; 								/* specific file system data */
};

/*
 * vfs fd table 
 */
struct sk_vfs_fdtable
{
	sk_uint32_t max_fd;
	struct sk_vfs_fd **fds;
};



/* vfs interfaces */
void sk_vfs_lock();
void sk_vfs_unlock();
int sk_fd_new();
struct sk_vfs_fd *sk_get_fd(int fd);
void sk_put_fd(struct sk_vfs_fd *fd);
int sk_fd_is_open(const char *path);
struct sk_vfs_filesystem *sk_vfs_filesystem_lookup(const char *path);

#endif
