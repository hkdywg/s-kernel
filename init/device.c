/*                                                                                                                                                                     
 *  device.c
 *
 *  brif
 *      s-kernel device frame
 *  
 *  (C) 2025.01.23 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <kobj.h>
#include <device.h>
#include <skernel.h>

/*
 * sk_device_find
 * brief
 * 		find and return the device by specified name
 */
struct sk_device *sk_device_find(const char *name)
{
	return (struct sk_device *)sk_object_find(name, SK_OBJECT_DEVICE);
}

/*
 * sk_device_create
 * brief
 * 		create a device object with user data size
 * param
 * 		type: the type of device object
 * 		attach_size: the size of user data
 */
struct sk_device *sk_device_create(enum sk_device_type type, 
								   sk_base_t attach_size)
{
	sk_base_t size;
	struct sk_device *device;

	size = SK_ALIGN(sizeof(struct sk_device), SK_ALIGN_SIZE);
	attach_size = SK_ALIGN(attach_size, SK_ALIGN_SIZE);
	/* use the total size, include user private data size */
	size += attach_size;

	device = (struct sk_device *)sk_malloc(size);
	if(device) {
		sk_memset(device, 0x00, sizeof(struct sk_device));
		device->type = type;
	}

	return device;
}

/*
 * sk_device_destroy
 * brief
 * 		destroy the device object 
 * param
 * 		dev: the device of need to be destroyed
 */
void sk_device_destroy(struct sk_device *dev)
{
	if(dev) {
		sk_object_delete(&(dev->parent));

		/* release this device object */
		sk_free(dev);
	}
}


/*
 * sk_device_init
 * brief
 * 		initialize the specified device
 * param
 * 		dev: the pointer of device structure 
 */
sk_err_t sk_device_init(struct sk_device *dev)
{
	sk_err_t ret = SK_EOK;
	if(dev->flag & SK_DEVICE_FLAG_ACTIVATED)
		return ret;
	/* if device specified init function is be set,
	 * involed it */
	if(dev->ops->init != SK_NULL) {
		ret = dev->ops->init(dev);
		if(ret == SK_EOK) 
			dev->flag |= SK_DEVICE_FLAG_ACTIVATED;
	}

	return ret;
}

/*
 * sk_device_open
 * brief
 * 		this function will open a device
 * param
 * 		dev: device pointer
 * 		flag: the flag of open device 
 */
sk_err_t sk_device_open(struct sk_device *dev, sk_uint16_t flag) 
{
	sk_err_t ret = SK_EOK;

	if(dev->parent.type != SK_OBJECT_DEVICE)
		return SK_EINVAL;

	/* if device is not initialized, initilize it */
	if(!(dev->flag & SK_DEVICE_FLAG_ACTIVATED)) {
		ret = sk_device_init(dev);
	}

	/* if device specified open function is be set,
	 * involed it */
	if(ret == SK_EOK) {
		if(dev->ops->open != SK_NULL)
			ret = dev->ops->open(dev, flag);
	}

	if(ret == SK_EOK) {
		dev->flag |= SK_DEVICE_FLAG_OPEN;
		dev->ref_cnt++;
	}

	return ret;
}

/*
 * sk_device_close
 * brief
 * 		this function will close the device
 * param
 * 		dev: the pointer of device structure
 */
sk_err_t sk_device_close(struct sk_device *dev)
{
	sk_err_t ret = SK_EOK;

	if(dev->parent.type != SK_OBJECT_DEVICE)
		return SK_EINVAL;

	if(dev->ref_cnt == 0)
		return SK_ERROR;

	dev->ref_cnt--;
	if(dev->ref_cnt != 0)
		return SK_EOK;

	if(dev->ops->close != SK_NULL)
		ret = dev->ops->close(dev);

	if(ret == SK_EOK) 
		dev->flag = SK_DEVICE_FLAG_CLOSE;

	return ret;
}

/*
 * sk_device_read
 * brief
 * 		this function will read data from a device
 * param
 * 		dev: the pointer of device structure
 * 		pos: the position when reading
 * 		buf: data buffer to save the read data
 * 		size: size of buffer
 * return
 * 		the actually read size on successful
 */
sk_size_t sk_device_read(struct sk_device *dev, sk_size_t pos,
						 void *buf, sk_size_t size)
{
	if(dev->parent.type != SK_OBJECT_DEVICE)
		return SK_EINVAL;
	/* check device reference count, if 0, the device has been closed */
	if(dev->ref_cnt == 0)
		return 0;

	if(dev->ops->read != SK_NULL)
		return dev->ops->read(dev, pos, buf, size);

	return 0;
}

/*
 * sk_device_write
 * brief
 * 		this function will write data to a device
 * param
 * 		dev: the pointer of device structure
 * 		pos: the position when writing
 * 		buf: data buffer to be write to device
 * 		size: size of buffer
 * return
 * 		the actually write size on successful
 */
sk_size_t sk_device_write(struct sk_device *dev, sk_size_t pos,
						  const void *buf, sk_size_t size)
{
	if(dev->parent.type != SK_OBJECT_DEVICE)
		return SK_EINVAL;
	/* check device reference count, if 0, the device has been closed */
	if(dev->ref_cnt == 0)
		return 0;

	if(dev->ops->write != SK_NULL)
		return dev->ops->write(dev, pos, buf, size);

	return 0;
}

/*
 * sk_device_register
 * brief
 * 		this function will register a device driver with a specified name
 * param
 * 		dev: pointer of device structure
 * 		name: dvice driver's name
 * 		flag: the capabilities flag of device
 */
sk_err_t sk_device_register(struct sk_device *dev,  const char *name,
							sk_uint16_t flag) 
{
	if(sk_device_find(name) != SK_NULL)
		return SK_ERROR;

	/* kernel object initialize */
	sk_object_init(&(dev->parent), SK_OBJECT_DEVICE, name);
	/* config device member */
	dev->flag = flag;
	dev->ref_cnt = 0;

	return SK_EOK;
}

/*
 * sk_device_unregister
 * brief
 * 		this funtion will remove a previously registered device driver
 * param
 * 		dev: the pointer of device driver structure
 */
sk_err_t sk_device_unregister(struct sk_device *dev)
{
	if(dev->parent.type != SK_OBJECT_DEVICE)
		return  SK_ERROR;

	sk_object_delete(&(dev->parent));

	return SK_EOK;
}





