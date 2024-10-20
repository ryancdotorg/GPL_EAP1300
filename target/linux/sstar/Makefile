# Copyright (c) 2013 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=sstar
CPU_TYPE:=cortex-a7
BOARDNAME:=SigmaStar DoorBell & AC Camera
SUBTARGETS:=infinity5 infinity6

LINUX_VERSION=4.9.84
include $(INCLUDE_DIR)/target.mk

SSTAR_KERNEL_MAKEOPTS := -C $(LINUX_DIR) -f Makefile CROSS_COMPILE="$(KERNEL_CROSS)" \
	ARCH="$(LINUX_KARCH)"

define Kernel/CompileModules
	$(call sn_copy_conffiles)
	$(call Kernel/CompileModules/Default_Sstar)
endef

define Kernel/CompileImage
	$(call sn_copy_conffiles)
	$(call Kernel/CompileImage/Default_Sstar)
	$(call Kernel/CompileImage/Initramfs_Sstar)
endef

$(eval $(call BuildTarget))
