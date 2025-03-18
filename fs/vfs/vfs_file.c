/*                                                                                                                                                                     
 *  vfs_file.c
 *
 *  brif
 *      s-kernel file operation of virtual file system 
 *  
 *  (C) 2025.03.18 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <vfs.h>

/*
 * sk_vfs_file_open
 * brief
 * 		this function will open a file by path with specified flag
 * param
 * 		fd: the file descriptor
 * 		path: specified file path
 * 		flag: the flag for open operator
 */
int sk_vfs_file_open(struct sk_vfs_fd *fd, const char *path, int flag)
{
	struct sk_vfs_filesystem *fs;
	int ret;

	if(fd == SK_NULL)
		return SK_EINVAL;

	/* find filesystem */
	fs = sk_vfs_filesystem_lookup(path);
	if(fs == SK_NULL)
		return SK_ERROR;

	/* set file system and file ops */
	fd->fs 	 = fs;
	fd->fops = fs->ops->fops;

	/* initialize the fd item */
	fd->type = FT_REGULAR;
	fd->flag = flag;
	fd->size = 0;
	fd->pos  = 0;
	fd->data = fs;
	/* current just support full path access */
	fd->path = (char *)path;
	/* specific file system open routine */
	if(fd->fops->open == SK_NULL) {
		sk_free(fd->path);
		fd->path = SK_NULL;
		return SK_ERROR;
	}
	if(ret = fd->fops->open(fd) < 0) {
		sk_free(fd->path);
		fd->path = SK_NULL;
		return SK_ERROR;
	}

	fd->flag |= VFS_F_OPEN;
	if(flag & VFS_F_DIRECTORY) {
		fd->type = FT_DIRECTORY;
		fd->flag |= VFS_F_DIRECTORY;
	}

	return 0;
}

/*
 * sk_vfs_file_close
 * brief
 * 		this function will close a file description
 * param
 * 		fd: the file descriptor to be closed
 */
int sk_vfs_file_close(struct sk_vfs_fd *fd)
{
	int ret = 0;

	if(fd == SK_NULL)
		return SK_EINVAL;

	if(fd->fops->close != SK_NULL)
		ret = fd->fops->close(fd);

	if(ret < 0)
		return ret;

	sk_free(fd->path);
	fd->path = SK_NULL;

	return ret;
}

/*
 * sk_vfs_file_read
 * brief
 * 		this function will read specified length data from a 
 * 		file descriptor to a buffer 
 * param
 * 		fd: the file descriptor
 * 		buf: the buffer to save read data
 * 		len: the length of data buffer to be read
 */
int sk_vfs_file_read(struct sk_vfs_fd *fd, void *buf, sk_size_t len)
{
	int ret = 0;

	if(fd == SK_NULL)
		return SK_EINVAL;

	if(fd->fops->read == SK_NULL)
		return SK_ERROR;

	if((ret = fd->fops->read(fd, buf, len)) < 0)
		fd->flag |= VFS_F_EOF;

	return ret;
}

/*
 * sk_vfs_file_write
 * brief
 * 		this function will write specified length data to file system
 * param
 * 		fd: the file descriptor
 * 		buf: the data buffer to be write
 * 		len: the length of data buffer to be write
 */
int sk_vfs_file_write(struct sk_vfs_fd *fd, const void *buf, sk_size_t len)
{
	int ret = 0;

	if(fd == SK_NULL)
		return SK_EINVAL;

	if(fd->fops->write == SK_NULL)
		return SK_ERROR;

	return fd->fops->write(fd, buf, len);
}

/*
 * sk_vfs_file_lseek
 * brief
 * 		this function will seek the offset for file descriptor
 * param
 * 		fd: the file descriptor
 * 		offset: the offset to be seek
 */
int sk_vfs_file_lseek(struct sk_vfs_fd *fd, sk_size_t offset)
{
	int ret = 0;

	if(fd == SK_NULL)
		return SK_EINVAL;

	if(fd->fops->lseek == SK_NULL)
		return SK_ERROR;

	if((ret = fd->fops->lseek(fd, offset)) >= 0)
		fd->pos = ret;

	return ret;
}


/*
 * sk_vfs_file_unlink
 * brief
 * 		this function will unlink (remove) a specified path file from file system
 */
int sk_vfs_file_unlink(const char *path)
{
	struct sk_vfs_filesystem *fs;
	int ret;

	/* get filesystem */
	if((fs = sk_vfs_filesystem_lookup(path)) == SK_NULL)
		return SK_EINVAL;

	/* check file is open */
	if(sk_fd_is_open(path) == 0)
		return SK_EBUSY;

	if(!fs->ops->unlink != SK_NULL)
		ret = fs->ops->unlink(fs, path);
	else
		ret = SK_ERROR;

	return ret;
}








