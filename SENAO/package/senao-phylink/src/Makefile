#
# Makefile for kernel test
#
PWD         := $(shell pwd)
KVERSION    := $(shell uname -r)
KERNEL_DIR   = /usr/src/linux-headers-$(KVERSION)/

MODULE_NAME  = phylink_module
obj-m       := $(MODULE_NAME).o
$(MODULE_NAME)-objs += phylink_kernel.o


all: module

module:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

