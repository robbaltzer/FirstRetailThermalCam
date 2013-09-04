/*
 * Lepton Device Driver
 *
 * Copyright (c) 2013, Rob Baltzer All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * 3. The name of Rob Baltzer may not be used to endorse or promote products
 *    derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Rob Baltzer ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include <linux/moduleparam.h>
#include <linux/slab.h>         /* kmalloc() */
#include <linux/types.h>        /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>        /* O_ACCMODE */
#include <asm/system.h>         /* cli(), *_flags */
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "lepton.h"

#define USER_BUFF_SIZE 128

#define SPI_BUS 1
#define SPI_BUS_CS1 1
#define SPI_BUS_SPEED 1000000

const char this_driver_name[]="lepton";

struct lepton_dev {
	struct semaphore spi_sem;
	struct semaphore fop_sem;
	dev_t devt;
	struct cdev cdev;
	struct class *class;
	struct spi_device *spi_device;
	char *user_buff;
};

static struct lepton_dev lepton_dev;

static int lepton_transfer(struct spi_device *spi)
{
	struct spi_message	msg;
	struct spi_transfer	xfer;
	u8			*buf;
	int			ret;

	/*
	 * Buffers must be properly aligned for DMA. kmalloc() ensures
	 * that.
	 */
	buf = kmalloc(328, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	/* Put some "interesting" values into the TX buffer. */
	buf[0] = 0x55;
	buf[1] = 0x56;
	buf[2] = 0x57;
	buf[3] = 0x58;

	/* Initialize the SPI message and transfer data structures */
	spi_message_init(&msg);
	memset(&xfer, 0, sizeof(xfer));
	xfer.tx_buf = buf;
	xfer.rx_buf = buf+164;
	xfer.len = 164;


	/* Add our only transfer to the message */
	spi_message_add_tail(&xfer, &msg);

	// dev_info(&spi->dev, "sending %02x %02x %02x %02x...\n",
	// 	buf[0], buf[1], buf[2], buf[3]);

	/* Send the message and wait for completion */
	ret = spi_sync(spi, &msg);
	if (ret == 0) {
		// if (buf[167] == 0xfe)
			printk(KERN_ALERT "received %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			buf[164], buf[165], buf[166], buf[167], buf[168], buf[169], buf[170], buf[171], buf[172], buf[173], buf[174], buf[175]);
	}
	else
		printk(KERN_ALERT "spi_sync() failed %d\n", ret);

	/* Clean up and pass the spi_sync() return value on to the caller */
	kfree(buf);

	return ret;
}

static ssize_t lepton_read(struct file *filp, char __user *buff, size_t count,
	loff_t *offp)
{
	size_t len;
	ssize_t status = 0;

	if (!buff)
		return -EFAULT;

	if (*offp > 0)
		return 0;

	if (down_interruptible(&lepton_dev.fop_sem))
		return -ERESTARTSYS;

	if (!lepton_dev.spi_device)
		strcpy(lepton_dev.user_buff, "spi_device is NULL\n");
	else if (!lepton_dev.spi_device->master)
		strcpy(lepton_dev.user_buff, "spi_device->master is NULL\n");
	else
		sprintf(lepton_dev.user_buff, "%s ready on SPI%d.%d\n",
			this_driver_name,
			lepton_dev.spi_device->master->bus_num,
			lepton_dev.spi_device->chip_select);


	len = strlen(lepton_dev.user_buff);

	if (len < count)
		count = len;

	if (copy_to_user(buff, lepton_dev.user_buff, count)) {
		printk(KERN_ALERT "lepton_read(): copy_to_user() failed\n");
		status = -EFAULT;
	} else {
		*offp += count;
		status = count;
	}

	up(&lepton_dev.fop_sem);

	return status;	
}

static int lepton_open(struct inode *inode, struct file *filp)
{	
	int status = 0;

	if (down_interruptible(&lepton_dev.fop_sem))
		return -ERESTARTSYS;

	if (!lepton_dev.user_buff) {
		lepton_dev.user_buff = kmalloc(USER_BUFF_SIZE, GFP_KERNEL);
		if (!lepton_dev.user_buff)
			status = -ENOMEM;
	}	

	up(&lepton_dev.fop_sem);

	return status;
}

static int __devinit lepton_probe(struct spi_device *spi)
{
//	struct lepton_data *pdata = spi->dev.platform_data;
	int ret = 0;
	 int j = 100;

	/* Add per-device initialization code here */
	printk(KERN_ALERT "lepton_probe\n");

	dev_notice(&spi->dev, "probe() called, value: %d\n", 4);

	if (down_interruptible(&lepton_dev.spi_sem))
		return -EBUSY;

	lepton_dev.spi_device = spi;

	up(&lepton_dev.spi_sem);
	/* Try communicating with the device. */
	while(j--) {
		ret = lepton_transfer(spi);
		// j = 0xffff; while(j--);
	}
	return ret;
}

#ifdef CONFIG_PM
static int lepton_suspend(struct spi_device *spi, pm_message_t state)
{
	/* Add code to suspend the device here */

	dev_notice(&spi->dev, "suspend() called\n");

	return 0;
}

static int lepton_resume(struct spi_device *spi)
{
	/* Add code to resume the device here */

	dev_notice(&spi->dev, "resume() called\n");

	return 0;
}
#else
/* No need to do suspend/resume if power management is disabled */
#define lepton_suspend	NULL
#define lepton_resume	NULL
#endif

static int lepton_remove(struct spi_device *spi_device)
{
	if (down_interruptible(&lepton_dev.spi_sem))
		return -EBUSY;

	lepton_dev.spi_device = NULL;

	up(&lepton_dev.spi_sem);

	return 0;
}

static struct spi_driver lepton_driver = {
	.driver	= {
		.name		= this_driver_name,
		.owner		= THIS_MODULE,
	},
	.probe		= lepton_probe,
	.remove		= __devexit_p(lepton_remove),
	.suspend	= lepton_suspend,
	.resume		= lepton_resume,
};

static int __init lepton_init_spi(void)
{
	int error;

	error = spi_register_driver(&lepton_driver);
	if (error < 0) {
		printk(KERN_ALERT "spi_register_driver() failed %d\n", error);
		return error;
	}
	return 0;
}

long lepton_unlocked_ioctl(struct file *filp, unsigned int cmd, 
                            unsigned long arg)
{
	printk(KERN_ALERT "lepton_unlocked_ioctl\n");
	switch(cmd) {
		case LEPTON_IOCTL_TRANSFER:
			lepton_transfer(lepton_dev.spi_device);
			break;
		default:
		printk(KERN_ALERT "Bad ioctl called %d\n", cmd);
	}
	return 0;
}

static const struct file_operations lepton_fops = {
	.owner =	THIS_MODULE,
	.read = lepton_read,
	.open =	lepton_open,	
	.unlocked_ioctl = lepton_unlocked_ioctl,
};

static int __init lepton_init_cdev(void)
{
	int error;

	lepton_dev.devt = MKDEV(0, 0);
	printk(KERN_ALERT "lepton_init_cdev()\n");
	error = alloc_chrdev_region(&lepton_dev.devt, 0, 1, this_driver_name);
	if (error < 0) {
		printk(KERN_ALERT "alloc_chrdev_region() failed: %d \n",
			error);
		return -1;
	}

	cdev_init(&lepton_dev.cdev, &lepton_fops);
	lepton_dev.cdev.owner = THIS_MODULE;

	error = cdev_add(&lepton_dev.cdev, lepton_dev.devt, 1);
	if (error) {
		printk(KERN_ALERT "cdev_add() failed: %d\n", error);
		unregister_chrdev_region(lepton_dev.devt, 1);
		return -1;
	}	
	else {
		printk(KERN_ALERT "cdev_add() succeeded: %d\n", error);
	}

	return 0;
}

static int __init lepton_init_class(void)
{
	lepton_dev.class = class_create(THIS_MODULE, this_driver_name);

	if (!lepton_dev.class) {
		printk(KERN_ALERT "class_create() failed\n");
		return -1;
	}

	if (!device_create(lepton_dev.class, NULL, lepton_dev.devt, NULL,
		this_driver_name)) {
		printk(KERN_ALERT "device_create(..., %s) failed\n",
			this_driver_name);
	class_destroy(lepton_dev.class);
	return -1;
}

return 0;
}

static int __init lepton_init(void)
{

	printk(KERN_ALERT "lepton_init\n");

	memset(&lepton_dev, 0, sizeof(lepton_dev));

	sema_init(&lepton_dev.spi_sem, 1);
	sema_init(&lepton_dev.fop_sem, 1);

	if (lepton_init_cdev() < 0)
		goto fail_1;

	if (lepton_init_class() < 0)
		goto fail_2;

	if (lepton_init_spi() < 0)
		goto fail_3;

	return 0;

fail_3:
	device_destroy(lepton_dev.class, lepton_dev.devt);
	class_destroy(lepton_dev.class);

fail_2:
	cdev_del(&lepton_dev.cdev);
	unregister_chrdev_region(lepton_dev.devt, 1);

fail_1:
	return -1;
}
module_init(lepton_init);

static void __exit lepton_exit(void)
{
	spi_unregister_device(lepton_dev.spi_device);
	spi_unregister_driver(&lepton_driver);

	device_destroy(lepton_dev.class, lepton_dev.devt);
	class_destroy(lepton_dev.class);

	cdev_del(&lepton_dev.cdev);
	unregister_chrdev_region(lepton_dev.devt, 1);

	if (lepton_dev.user_buff)
		kfree(lepton_dev.user_buff);


	printk(KERN_ALERT "lepton_exit\n");
	

}
module_exit(lepton_exit);

/* Information about this module */
MODULE_DESCRIPTION("Lepton Camera VoSPI Driver");
MODULE_AUTHOR("Rob Baltzer");
MODULE_LICENSE("Dual BSD/GPL");
