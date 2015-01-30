/*
 * Copyright (C) 2015 IMMS GmbH
 * Author: Robin Pricken <robin.pricken@imms.de>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/spi/spi.h>
#include "cc3100-spi.h"

struct cc3100_spi {
	struct device *dev;
	struct spi_device *spidev;
	struct miscdevice mdev;
	struct cc3100_spi_platform_data *pdata;
	int irq;
	struct fasync_struct *async_queue;
	/* max size a raw socket from cc3100 can transmit */
	unsigned char buffer[1536];
};

ssize_t cc3100_spi_read(struct file *fildes, char __user * buff, size_t count,
	loff_t * offp)
{
	int err;
	struct cc3100_spi *fp=
		container_of(fildes->private_data, struct cc3100_spi, mdev);

        /* size check of given length */
	if (count > sizeof(fp->buffer)) return -EFAULT;

	err = spi_read(fp->spidev, fp->buffer, count);

	if(copy_to_user(buff, fp->buffer, count)) return -EFAULT;

	return err;
}

ssize_t cc3100_spi_write(struct file *fildes, const char __user * buff,
	size_t count, loff_t * offp){
	struct cc3100_spi *fp=
		container_of(fildes->private_data, struct cc3100_spi, mdev);

	/* size check of given length */
	if (count > sizeof(fp->buffer)) return -EFAULT;

        if(copy_from_user(fp->buffer, buff, count )) return -EFAULT;

	return spi_write(fp->spidev, fp->buffer, count);
}

static long cc3100_spi_ioctl(struct file *file, unsigned int cmd,
	unsigned long arg)
{
	struct cc3100_spi *fp=
		container_of(file->private_data, struct cc3100_spi, mdev);

	switch(cmd)
	{
		case CC3100_nHIB_HIGH:
			gpio_set_value(fp->pdata->gpio_nHIB, 1);
			break;
		case CC3100_nHIB_LOW:
			gpio_set_value(fp->pdata->gpio_nHIB, 0);
			break;
		default:
			dev_err(fp->dev, "unkown ioctl\n");
			return -EINVAL;
	}

	return 0;
}

int cc3100_spi_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int cc3100_spi_release(struct inode *inode, struct file *filp)
{
	struct cc3100_spi *fp=
		container_of(filp->private_data, struct cc3100_spi, mdev);

	/* set nHIB low */
	gpio_set_value(fp->pdata->gpio_nHIB, 0);

	return 0;
}

/* function is called when F_SETFL is executed by a process */
static int cc3100_spi_fasync(int fd, struct file *filp, int mode)
{
	struct cc3100_spi *fp=
		container_of(filp->private_data, struct cc3100_spi, mdev);

	/* register process for recieving SIGIOs  */
	return fasync_helper(fd, filp, mode, &fp->async_queue);
}

static struct file_operations cc3100_spi_fops = {
	.owner 		= THIS_MODULE,
	.read 		= cc3100_spi_read,
	.write 		= cc3100_spi_write,
	.unlocked_ioctl = cc3100_spi_ioctl,
	.open 		= cc3100_spi_open,
	.release 	= cc3100_spi_release,
	.fasync 	= cc3100_spi_fasync,
};

static irqreturn_t cc3100_spi_interrupt(int irq, void *dev_id)
{
	struct cc3100_spi *fp = dev_id;

	if (fp->async_queue)
	    /* send SIGIO to userspace program */
	    kill_fasync(&fp->async_queue, SIGIO, POLL_IN);

	return IRQ_HANDLED;
}

#ifdef CONFIG_OF
static struct cc3100_spi_platform_data *cc3100_spi_of_init(
	struct spi_device *spi)
{
	struct device_node *node = spi->dev.of_node;
	struct cc3100_spi_platform_data *pdata;

	if (!node) {
		dev_err(&spi->dev, "device node not found\n");
		return ERR_PTR(-EINVAL);
	}

	pdata = devm_kzalloc(&spi->dev,
		sizeof(struct cc3100_spi_platform_data), GFP_KERNEL);
	if(!pdata) {
		dev_err(&spi->dev, "could not allocate memory for pdata\n");
		return ERR_PTR(-ENOMEM);
	}

	if(of_property_read_u32(node, "gpio_irq", &pdata->gpio_irq) ||
		of_property_read_u32(node, "gpio_nHIB", &pdata->gpio_nHIB))
	{
		dev_err(&spi->dev, "missing device tree property\n");
		kfree(pdata);
		return ERR_PTR(-ENODEV);
	}

	return pdata;
}
#else
static struct cc3100_spi_platform_data *cc3100_spi_of_init(
	struct spi_device *spi)
{
	return ERR_PTR(-EINVAL);
}
#endif

static int cc3100_spi_probe(struct spi_device *spi)
{
	struct cc3100_spi *fp;
	int err=0;

	/* CC3100 supports wordsize: 8, 16 or 32 */
	spi->bits_per_word = 8;

	/* CC3100 works in MODE_0 (Sample on rising_edge) */
	spi->mode = SPI_MODE_0;

	/* CC3100 works stable up to 24 MHz */
	spi->max_speed_hz = 24000000;

	err = spi_setup(spi);
	if (err) return err;

	/* alloc memory for private data structure */
	fp=devm_kzalloc(&spi->dev, sizeof(struct cc3100_spi), GFP_KERNEL);
	if(!fp){
		err=-ENOMEM;
		goto out_kzalloc_failed;
	}

	/* set variables of the data structure */
	fp->spidev=spi;
	fp->dev=&spi->dev;

	/* get platform data */
	if(fp->dev->platform_data) {
		fp->pdata=fp->dev->platform_data;
	}
	/* if none is set, try to probe from device tree */
	else {
		fp->pdata=cc3100_spi_of_init(spi);
		if(!fp->pdata) {
			dev_err(fp->dev, "platform data not available\n");
			err=-ENODEV;
			goto out_no_pdata;
		}
	}

	/* register a minimal instruction set computer device (misc) */
	fp->mdev.minor=MISC_DYNAMIC_MINOR;
	fp->mdev.name="cc3100-spi";
	fp->mdev.fops=&cc3100_spi_fops;
	err=misc_register(&fp->mdev);
	if(err){
		dev_err(&spi->dev,"failed to register misc device\n");
		goto out_misc_register_failed;
	}

	/* register pin for nHIB */
	err=gpio_request(fp->pdata->gpio_nHIB, "ccHIB");
	if(err) {
		dev_err(&spi->dev, "failed to register GPIO\n");
		goto out_nHIB_request_failed;
	}
	gpio_direction_output(fp->pdata->gpio_nHIB, 0);

	/* register pin for IRQ */
        err=gpio_request(fp->pdata->gpio_irq, "ccIRQ");
	if(err){
		dev_err(&spi->dev, "failed to register GPIO\n");
		goto out_irq_request_failed;
	}
	gpio_direction_input(fp->pdata->gpio_irq);

	/* register interrupt for IRQ pin */
	fp->irq=gpio_to_irq(fp->pdata->gpio_irq);
        err=request_irq(fp->irq, cc3100_spi_interrupt,
                   IRQF_TRIGGER_RISING, spi->dev.driver->name, fp);
	if(err) {
		dev_err(&spi->dev, "failed to register irq\n");
		goto out_irq_interrupt_request_failed;
	}

	/* "save" memory of private data structure
	 * can be accessed from other functions using container_of() */
	dev_set_drvdata(&spi->dev, fp);

	return 0;

out_irq_interrupt_request_failed:
	gpio_free(fp->pdata->gpio_irq);
out_irq_request_failed:
	gpio_free(fp->pdata->gpio_nHIB);
out_nHIB_request_failed:
	misc_deregister(&fp->mdev);
out_misc_register_failed:
	if(!fp->dev->platform_data) {
		if(fp->pdata) devm_kfree(fp->dev, fp->pdata);
	}
out_no_pdata:
	devm_kfree(&spi->dev, fp);
out_kzalloc_failed:
	return err;
}

static int cc3100_spi_remove(struct spi_device *dev)
{
	struct cc3100_spi *fp = dev_get_drvdata(&dev->dev);

	free_irq(fp->irq, fp);
	gpio_free(fp->pdata->gpio_irq);
	gpio_free(fp->pdata->gpio_nHIB);

	misc_deregister(&fp->mdev);

	if(!fp->dev->platform_data) {
		if(fp->pdata) devm_kfree(&dev->dev, fp->pdata);
	}

	devm_kfree(&dev->dev, fp);

	return 0;
}

static const struct of_device_id cc3100_spi_dt_ids[] = {
	{ .compatible = "ti,cc3100" },
	{},
};
MODULE_DEVICE_TABLE(of, cc3100_spi_dt_ids);

static struct spi_driver cc3100_spi_driver = {
	.driver = {
		.name = "cc3100-spi",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(cc3100_spi_dt_ids),
	},
	.probe		= cc3100_spi_probe,
	.remove		= cc3100_spi_remove,
};

module_spi_driver(cc3100_spi_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Robin Pricken <robin.pricken@imms.de");
MODULE_DESCRIPTION("TI CC3100 Boosterpack driver");
