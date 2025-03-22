/*                                                                                                                                                                     
 *  vfs.c
 *
 *  brif
 *      s-kernel vfs
 *  
 *  (C) 2025.03.18 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <vfs.h>

/*
 * global filesytem relative data struct
 */
struct sk_vfs_filesystem_ops file_system_opt[VFS_MAX_FS_TYPE];
struct sk_vfs_filesystem file_system[VFS_MAX_FS_TYPE];
struct sk_vfs_fdtable fd_tab;
struct sk_mutex fs_lock;
char working_dir[VFS_MAX_DIRENT_NAME] = {"/"};

/*
 * sk_fdt_init
 * brief
 * 		initialize the file descriptor structure
 * param
 * 		fdt: pointer to the file descriptor
 */
static sk_err_t sk_fdt_init(struct sk_vfs_fdtable *fdt)
{
	fdt->max_fd = VFS_MAX_FD;
	for(sk_uint32_t i = 0; i < fdt->max_fd; i++) {
		fdt->fds[i] = (struct sk_vfs_fd *)sk_malloc(sizeof(struct sk_vfs_fd));
		sk_memset(fdt->fds[i], 0, sizeof(struct sk_vfs_fd));
	}

	return SK_EOK;
}


/*
 * sk_vfs_init
 * brief
 * 		kernel virtual filesystem initialize
 */
int sk_vfs_init(void)
{
	/* clear filesystem operation table */
	sk_memset((void *)file_system_opt, 0, sizeof(file_system_opt));
	/* clear filesytem table */
	sk_memset((void *)file_system, 0, sizeof(file_system));
	/* clear fd table */
	sk_memset(&fd_tab, 0, sizeof(fd_tab));
	/* create device filesystem lock */
	sk_mutex_init(&fs_lock, "fslock", SK_IPC_FLAG_FIFO);
	/* initialize file description */
	sk_fdt_init(&fd_tab);
	/* set current working directory */
	sk_memset(working_dir, 0, sizeof(working_dir));
	working_dir[0] = '/';
}
//INIT_PREV_EXPORT(sk_vfs_init);

/*
 * sk_vfs_lock
 * brief
 * 		this function will lock virtual file system
 */
void sk_vfs_lock()
{
	/* wait mutex forever */
	sk_mutex_lock(&fs_lock, 0);
}

/*
 * sk_vfs_unlock
 * brief
 * 		this function will unlock virtual file system
 */
void sk_vfs_unlock()
{
	/* wait mutex forever */
	sk_mutex_unlock(&fs_lock);
}


/*
 * sk_fd_alloc
 * brief
 * 		return the file descriptor index
 * param
 * 		fdt: pointer to file descriptor 
 * 		start_fd: start fd
 */
static int sk_fd_alloc(struct sk_vfs_fdtable *fdt, int start_fd)
{
	sk_uint32_t idx;

	/* find an empty fd entry */
	for(idx = start_fd; idx < fdt->max_fd; idx++) {
		if(fdt->fds[idx]->ref_count == 0)
			break;
	}
	/* file descriptor is full */
	if(idx == fdt->max_fd) 
		return -1;	

	return idx;
}

/*
 * sk_fd_new 
 * brief
 * 		alloc and initialize an empty file descriptor index  
 */
int sk_fd_new()
{
	int index;
	struct sk_vfs_fd *fd;

	/* lock filesystem */
	sk_vfs_lock();

	/* find an empty fd entry */
	index = sk_fd_alloc(&fd_tab, 0);

	/* can't find an empty fd entry */
	if(index == -1) {
		sk_kprintf("vfs fd new is failed! can't find an empty fd entry\n");
		/* unlock filesystem */
		sk_vfs_unlock();
		return -1;
	}
	/* configure file descriptor */
	fd = fd_tab.fds[index];
	fd->ref_count = 1;
	fd->magic = VFS_FD_MAGIC;

	/* unlock filesystem */
	sk_vfs_unlock();

	return index;
}

/*
 * sk_get_fd
 * brief
 * 		this function will return a file descriptor structure
 * param
 * 		fd: index of file descriptor structure 
 */
struct sk_vfs_fd *sk_get_fd(int fd)
{
	struct sk_vfs_fd *d;

	if(fd < 0 || fd >= fd_tab.max_fd)
		return SK_NULL;

	sk_vfs_lock();
	d = fd_tab.fds[fd];
	if(d->magic != VFS_FD_MAGIC) {
		sk_vfs_unlock();
		return SK_NULL;
	}
	/* increase the reference count */
	d->ref_count++;
	sk_vfs_unlock();

	return d;
}

/*
 * sk_put_fd 
 * brief
 * 		this function will put file descriptor
 * param
 * 		fd: pointer to file descriptor structure
 */
void sk_put_fd(struct sk_vfs_fd *fd)
{
	if(fd->ref_count > 0)
		fd->ref_count--;
}

/*
 * sk_fd_is_open
 * brief
 * 		this function will return wheter this file has been opened
 * param
 * 		path: the file path name
 */
int sk_fd_is_open(const char *path)
{
	struct sk_vfs_filesystem *fs;
	struct sk_vfs_fd *fd;
	sk_uint32_t index = 0;

	if((fs = sk_vfs_filesystem_lookup(path)) == SK_NULL)
		return -1;

	sk_vfs_lock();

	for(index = 0; index < fd_tab.max_fd; index++) {
		fd = fd_tab.fds[index];
		if(fd->fops == SK_NULL || fd->path == SK_NULL)
			continue;
		if(fd->fs == fs && sk_strcmp(fd->path, path, sk_strlen(fd->path))) {
			sk_vfs_unlock();
			return 0;
		}
	}
	sk_vfs_unlock();

	return -1;
}




