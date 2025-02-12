/*                                                                                                                                                                     
 *  device.h
 *
 *  brif
 *      device frame
 *  
 *  (C) 2025.01.23 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef __DEVICE_H_
#define __DEVICE_H_

#include <base_def.h>
#include <kobj.h>
#include <klist.h>

/*
 * device flag definitions
 */
#define SK_DEVICE_FLAG_NOACTIVATED 	0x000	/* device is not be initialized */

#define SK_DEVICE_FLAG_RDONLY 		0x001 	/* read only */
#define SK_DEVICE_FLAG_WRONLY 		0x002 	/* write only */
#define SK_DEVICE_FLAG_RDWR 		0x003 	/* read and write */

#define SK_DEVICE_FLAG_CLOSE 		0x000 	/* device is be closed or created(not init)*/
#define SK_DEVICE_FLAG_ACTIVATED 	0x010	/* device initialzed */
#define SK_DEVICE_FLAG_OPEN 		0x020 	/* device has be opened */

/*
 * device class type
 */
enum sk_device_type
{
	SK_DEVICE_CHAR = 0,				/* character device */
	SK_DEVICE_BLOCK, 				/* block device */
	SK_DEVICE_NET,					/* net device */
};

struct sk_device;
/*
 * device operations set for device object 
 */
struct sk_device_ops
{
	sk_err_t (*init)(struct sk_device *dev);													/* init device interface */
	sk_err_t (*open)(struct sk_device *dev, sk_uint16_t flag);									/* open device interface */ 
	sk_err_t (*close)(struct sk_device *dev);													/* close device interface */
	sk_size_t (*read)(struct sk_device *dev, sk_size_t pos, void *buf, sk_size_t size);			/* read device interface */
	sk_size_t (*write)(struct sk_device *dev, sk_size_t pos, const void *buf, sk_size_t size);	/* write device interface */
	sk_err_t (*ioctl)(struct sk_device *dev, int cmd, void *args);								/* ioctl interface */
};

/*
 * waitqueue structure
 */
struct sk_wqueue
{
	sk_uint32_t flag;
	sk_list_t 	wait_list;
};


/*
 * device structure
 */
struct sk_device
{
	struct sk_object 	parent;			/* inherit from sk_object */

	enum sk_device_type type; 			/* device type */
	sk_uint16_t 		flag;			/* device flag */

	sk_uint8_t 			ref_cnt; 		/* reference count */
	sk_uint8_t 			device_id; 		/* device id number */

	/* device call back */
	sk_err_t (*rx_indicate)(struct sk_device *dev, sk_size_t size);
	sk_err_t (*tx_complete)(struct sk_device *dev, void *buf);

	const struct sk_device_ops *ops;	/* device operation function */
	void 				*user_data;		/* user private data */
};

/*
 * device operation interfaces
 */
struct sk_device *sk_device_create(enum sk_device_type type, 
								   sk_base_t attach_size);
void sk_device_destroy(struct sk_device *dev);
sk_err_t sk_device_init(struct sk_device *dev);
sk_err_t sk_device_open(struct sk_device *dev, sk_uint16_t flag);
sk_err_t sk_device_close(struct sk_device *dev);
sk_size_t sk_device_read(struct sk_device *dev, sk_size_t pos,
						 void *buf, sk_size_t size);
sk_size_t sk_device_write(struct sk_device *dev, sk_size_t pos,
						  const void *buf, sk_size_t size);
sk_err_t sk_device_register(struct sk_device *dev,  const char *name,
							sk_uint16_t flag);
sk_err_t sk_device_unregister(struct sk_device *dev);
#endif


