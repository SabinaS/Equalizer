/*
 * Device driver for FFT
 *
 * A Platform device implemented using the misc subsystem
 *
 * Stephen A. Edwards
 * Columbia University
 * edited by ma2799 and ss3912
 *
 * References:
 * Linux source: Documentation/driver-model/platform.txt
 *               drivers/misc/arm-charlcd.c
 * http://www.linuxforu.com/tag/linux-device-drivers/
 * http://free-electrons.com/docs/
 *
 * "make" to build
 * insmod fft_driver.ko
 *
 * Check code style with
 * checkpatch.pl --file --no-tree fft_driver.c
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include "fft_driver.h"

#define DRIVER_NAME "fft_driver"
#define SAMPLENUM 8192
#define SAMPLEBYTES SAMPLENUM*2

/*
 * Information about our device
 */
struct fft_driver_dev {
	struct resource res; /* Resource: our registers */
	void __iomem *virtbase; /* Where registers can be accessed in memory */
	u16 x; 
} dev;

/*
 * read from array (user gives us) and write to memory address (0x0 to 0x65536)
 */
static void write_mem( u16* dataArray )
{	
	iowrite16(SAMPLEBYTEs+dev.virtbase, dev.virtbase , SAMPLENUM); //is __iomem compatable with u16
	
}
//read from memory address (fpga gives us 0x0 to 0x65536) and write to array
static void read_mem( u16* dataArray)
{
    ioread16( dev.virtbase, dataArray, SAMPLENUM);
      
}

/*
 * Handle ioctl() calls from userspace:
 * Note extensive error checking of arguments
 */
static long fft_driver_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    u16 *dataArray = kmalloc(SAMPLEBYTES, GFP_KERNEL); //allocating space for data array
    
	switch (cmd) {
	case FFT_DRIVER_WRITE_DIGIT:
		if (copy_from_user(dataArray, (u16 *) arg,
				   sizeof(u16)))
			return -EACCES;
		write_mem(dataArray); //write dataArray
		break;

	case FFT_DRIVER_READ_DIGIT:
		if (copy_from_user(dataArray, (u16*) arg,
				   sizeof(u16)))
			return -EACCES;
		read_mem(dataArray); //read from dataArray
		if (copy_to_user((u16 *) arg, dataArray,
				 sizeof(u16)))
			return -EACCES;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/* The operations our device knows how to do */
static const struct file_operations fft_driver_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl = fft_driver_ioctl,

};

/* Information about our device for the "misc" framework -- like a char dev */
static struct miscdevice fft_driver_misc_device = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DRIVER_NAME,
	.fops		= &fft_driver_fops,
};

/*
 * Initialization code: get resources (registers) and display
 * a welcome message
 */
static int __init fft_driver_probe(struct platform_device *pdev)
{
	int ret;

	/* Register ourselves as a misc device: creates /dev/fft_driver */
	ret = misc_register(&fft_driver_misc_device);

	/* Get the address of our registers from the device tree */
	ret = of_address_to_resource(pdev->dev.of_node, 0, &dev.res);
	if (ret) {
		ret = -ENOENT;
		goto out_deregister;
	}

	/* Make sure we can use these registers */
	if (request_mem_region(dev.res.start, resource_size(&dev.res),
			       DRIVER_NAME) == NULL) {
		ret = -EBUSY;
		goto out_deregister;
	}

	/* Arrange access to our registers */
	dev.virtbase = of_iomap(pdev->dev.of_node, 0);
	if (dev.virtbase == NULL) {
		ret = -ENOMEM;
		goto out_release_mem_region;
	}


	return 0;

out_release_mem_region:
	release_mem_region(dev.res.start, resource_size(&dev.res));
out_deregister:
	misc_deregister(&fft_driver_misc_device);
	return ret;
}

/* Clean-up code: release resources */
static int fft_driver_remove(struct platform_device *pdev)
{
	iounmap(dev.virtbase);
	release_mem_region(dev.res.start, resource_size(&dev.res));
	misc_deregister(&fft_driver_misc_device);
	return 0;
}

/* Which "compatible" string(s) to search for in the Device Tree */
#ifdef CONFIG_OF
static const struct of_device_id fft_driver_of_match[] = {
	{ .compatible = "altr,fft_driver" },
	{},
};
MODULE_DEVICE_TABLE(of, fft_driver_of_match);
#endif

/* Information for registering ourselves as a "platform" driver */
static struct platform_driver fft_driver_driver = {
	.driver	= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(fft_driver_of_match),
	},
	.remove	= __exit_p(fft_driver_remove),
};

/* Calball when the module is loaded: set things up */
static int __init fft_driver_init(void)
{
	pr_info(DRIVER_NAME ": init\n");
	return platform_driver_probe(&fft_driver_driver, fft_driver_probe);
}

/* Calball when the module is unloaded: release resources */
static void __exit fft_driver_exit(void)
{
	platform_driver_unregister(&fft_driver_driver);
	pr_info(DRIVER_NAME ": exit\n");
}

module_init(fft_driver_init);
module_exit(fft_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MA2799, SS3912");
MODULE_DESCRIPTION("FFT_DRIVER");
