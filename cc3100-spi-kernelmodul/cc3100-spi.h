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
#ifndef __CC3100_SPI__H_

#define CC3100_SPI_IOC_MAGIC	0xf2
#define CC3100_SPI_IOC_START	0x40

#define CC3100_nHIB_HIGH	_IOW(CC3100_SPI_IOC_MAGIC, \
                                CC3100_SPI_IOC_START + 0x00, int)
#define CC3100_nHIB_LOW		_IOW(CC3100_SPI_IOC_MAGIC, \
                                CC3100_SPI_IOC_START + 0x01, int)

struct cc3100_spi_platform_data
{
	int gpio_irq;
	int gpio_nHIB;
};

#define DEVICE_FILE_NAME "/dev/cc3100-spi"

#endif
