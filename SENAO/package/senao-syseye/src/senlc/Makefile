#/*******************************************************************************************
# * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
# *                                                                                         *
# * This code has to be used in SenaoNetworks internally                                    *
# * Unauthorized copying of this file, via any medium is strictly prohibite                 *
# * Proprietary and confidential                                                            *
# * --------------------------------------------------------------------------------------- *
# * Project:  syseye                                                                        *
# *                                                                                         *
# * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
# *******************************************************************************************/

PWD         := $(shell pwd)
KVERSION    := $(shell uname -r)
KERNEL_DIR   = /usr/src/linux-headers-$(KVERSION)/
MODULE_NAME  = senlc_module
obj-m       := $(MODULE_NAME).o
$(MODULE_NAME)-objs += senlc_kernel.o

all: module

module:
	make -C $(KERNEL_DIR) M=$(PWD) modules

module_clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

clean: module_clean


