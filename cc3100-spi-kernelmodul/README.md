#cc3100-spi kernel module

To compile the kernel module type: 
- `make CROSS_COMPILE=<pathToYourCrossCompiler> KERNEL_SRC=<pathToYourKernelSource>`

The driver was tested on Beaglebone and Raspbery Pi. To insert the compiled kernelmodule follow the instructions for your platform listed bellow.

CC3100 Boost pin assignment: http://www.ti.com/lit/ug/swru371b/swru371b.pdf

Note: Some plattforms for example the beaglebone support a spi wordsize of 32, to reach a better performance. To modify the wordsize, edit spi->bits_per_word in the function cc3100_spi_probe (cc3100-spi.c).

###Beaglebone

- add am335x-bone-cc3100-spi.dtsi to /linux/arch/arm/boot/dts
- include the file in am335x-bone.dts (under #include "am335x-can1.dtsi")
    - `#include "am335x-bone-cc3100-spi.dtsi"`
- compile your kernel
- place am335x-bone.dtb in /boot/dtbs/
- after a reboot you can insert the kernel module
    - `insmod cc3100-spi.ko`

- pin assignment (used SPI0): http://beagleboard.org/static/images/cape-headers-spi.png
        SCLK:   P9-22
        MISO:   P9-18
        MOSI:   P9-21
        CS:     P9-17 (CS0)
        nHIB:   P8-4
        IRQ:    P8-22


###RaspberryPi
- go to your kernel source and edit /linux/arch/arm/mach-bcm2708/bcm2708.c
    - include the header file:
`#include <<pathToSource>/cc3100-spi.h>`

	- replace struct spi_board_info with the following:
   ```C
	static struct cc3100_spi_platform_data cc3100_spi_data = {
		.gpio_irq=24,
		.gpio_nHIB=22,
	};

	#ifdef CONFIG_BCM2708_SPIDEV
	static struct spi_board_info bcm2708_spi_devices[] = {
	#ifdef CONFIG_SPI_SPIDEV
		{
			.modalias = "cc3100-spi",
			.max_speed_hz = 500000,
			.bus_num = 0,
			.chip_select = 0,
			.platform_data=&cc3100_spi_data,
			.mode = SPI_MODE_0,
			}, {
			.modalias = "spidev",
			.max_speed_hz = 500000,
			.bus_num = 0,
			.chip_select = 1,
			.mode = SPI_MODE_0,
		}
	#endif
	};
	#endif
```


- compile your kernel (http://elinux.org/Raspberry_Pi_Kernel_Compilation)
- after a reboot of your Raspberry Pi you can insert the kernel module
    - `insmod cc3100-spi.ko`

- pin assignment: http://elinux.org/RPi_Low-level_peripherals
        SCLK:   11
        MISO:   9
        MOSI:   10
        CS:     8
        nHIB:   22
        IRQ:    24
        
