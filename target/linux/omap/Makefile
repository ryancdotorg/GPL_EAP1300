#
# Copyright (C) 2012-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(TOPDIR)/.config # for CONFIG_SOFT_FLOAT

ARCH:=arm
BOARD:=omap
BOARDNAME:=TI OMAP3/4/AM33xx
ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "EIG9100")
FEATURES:=ubifs squashfs usbgadget ext4 targz audio nand usb
endif
ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "SP938BS")
FEATURES:=ubifs squashfs ext4 targz audio
endif
ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "SP935")
FEATURES:=ubifs squashfs ext4 targz audio
endif
CPU_TYPE:=cortex-a8

ifeq ($(CONFIG_SOFT_FLOAT),y)
CPU_SUBTYPE:=vfpv3
else
CPU_SUBTYPE:=neon 
endif

MAINTAINER:=Imre Kaloz <kaloz@openwrt.org>

ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "EIG9100")
KERNELNAME:=zImage dtbs uImage
LINUX_VERSION:=4.4.32
endif
ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "SP938BS")
KERNELNAME:=zImage uImage
LINUX_VERSION:=3.2.82
endif
ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "SP935")
KERNELNAME:=zImage uImage
LINUX_VERSION:=3.2.82
endif

CODE_BASE_TYPE=SmartOpenWrt_EX
ifeq ($(CONFIG_SOFT_FLOAT),y)
CFLAGS:=-Os -pipe -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp
export KBUILD_CFLAGS_KERNEL+=-DCONFIG_KERNEL_SOFT_FLOAT=y
else
CFLAGS:=-Os -pipe -march=armv7-a -marm -mthumb-interwork -mfpu=neon -mfloat-abi=hard -mtune=cortex-a8
endif

ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "EIG9100")
export KBUILD_CFLAGS_KERNEL+=-DCONFIG_KERNEL_MACRONIX_4K_SECTOR
export KBUILD_CFLAGS_KERNEL+=-DCONFIG_KERNEL_BT_QCA_PATCH
endif
ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "SP938BS")
export KBUILD_CFLAGS_KERNEL+=-DCONFIG_SP938BS_HW=y # Add AR803X driver
endif

ifneq ($(CONFIG_EXTERNAL_KERNEL_TREE),"")
  $(eval _LINUX_DIR:=$(CONFIG_EXTERNAL_KERNEL_TREE))
  $(shell touch $(_LINUX_DIR)/.scmversion)
  _LINUX_MAJOR:=$(shell sed -n 's,VERSION = \([0-9]*\)$$,\1,p' $(_LINUX_DIR)/Makefile)
  _LINUX_MINOR:=$(shell sed -n 's,PATCHLEVEL = \([0-9]*\)$$,\1,p' $(_LINUX_DIR)/Makefile)
  _LINUX_SUBLEVEL:=$(shell sed -n 's,SUBLEVEL = \([0-9]*\)$$,\1,p' $(_LINUX_DIR)/Makefile)
  _LINUX_EXTRAVERSION:=$(shell sed -n 's,EXTRAVERSION = \([-rc0-9]*\)$$,\1,p' $(_LINUX_DIR)/Makefile)
  LINUX_VERSION:=$(_LINUX_MAJOR).$(_LINUX_MINOR)$(if $(_LINUX_SUBLEVEL),.$(_LINUX_SUBLEVEL))$(if $(_LINUX_EXTRAVERSION),$(_LINUX_EXTRAVERSION))
endif

ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "EIG9100")
KERNEL_PATCHVER:=4.4
endif
ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "SP938BS")
KERNEL_PATCHVER:=3.2
endif

ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "SP935")
KERNEL_PATCHVER:=3.2
endif
LINUX_VERSION?=4.4.32

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += uboot-omap-omap4_am335x_evm_spiboot

define Target/Description
	TI OMAP boards
endef
	

$(eval $(call BuildTarget))
