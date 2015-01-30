#CROSS_COMPILE=../../../beaglebone/compiler/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin/arm-linux-gnueabihf-
#KERNEL_SRC=../../../beaglebone/kernel/linux/

ARCH ?= arm

EXTRA_CFLAGS := -Wall

obj-m := cc3100-spi.o

all: default

default:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C ${KERNEL_SRC} SUBDIRS=`pwd` modules

clean:
	@-rm -fr *.o *.ko *~ Module.symvers
	@-rm -fr .tmp_versions .*.cmd *.mod.c modules.order
	@echo done cleaning
