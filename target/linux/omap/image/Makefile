#
# Copyright (C) 2012-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk
include $(TOPDIR)/.config # for product config

UBIFS_OPTS = -F -m 2048 -e 124KiB -c 4096 -U
UBI_OPTS = -m 2048 -p 128KiB -s 512 -O 2048

DTS_DIR=$(BUILD_DIR)/linux-$(BOARD)/linux-$(LINUX_VERSION)/arch/arm/boot/dts
TFTP_DIR=/tftpboot
NFS_DIR=/tftpboot/target-fs
prefix_output=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-

define Image/BuildKernel
	$(CP) $(KDIR)/zImage $(BIN_DIR)/$(IMG_PREFIX)-zImage
	$(CP) $(KDIR)/uImage $(BIN_DIR)/$(IMG_PREFIX)-uImage
 ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
	$(CP) $(KDIR)/zImage-initramfs $(BIN_DIR)/$(IMG_PREFIX)-zImage-initramfs
	$(CP) $(KDIR)/uImage-initramfs $(BIN_DIR)/$(IMG_PREFIX)-uImage-initramfs
 endif

 ifneq ($(CONFIG_TARGET_ROOTFS_INCLUDE_KERNEL),)
	$(INSTALL_DIR) $(TARGET_DIR)/boot
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-zImage $(TARGET_DIR)/boot/zImage
 endif
 ifneq ($(CONFIG_TARGET_ROOTFS_INCLUDE_DTB),)
	$(INSTALL_DIR) $(TARGET_DIR)/boot
	$(CP) $(DTS_DIR)/am335x*.dtb $(TARGET_DIR)/boot/
	$(CP) $(DTS_DIR)/omap3*.dtb $(TARGET_DIR)/boot/
	$(CP) $(DTS_DIR)/omap4*.dtb $(TARGET_DIR)/boot/
 endif
 ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "EIG9100")
	-mkdir -p $(BIN_DIR)/dtbs
	-$(CP) $(DTS_DIR)/am335x*.dtb $(BIN_DIR)/dtbs/
	#$(CP) $(BIN_DIR)/$(IMG_PREFIX)-uImage-initramfs $(prefix_output)$(FW_FIT_NORNAND_KR_SUFFIX)
	-cat $(BIN_DIR)/$(IMG_PREFIX)-zImage-initramfs $(BIN_DIR)/dtbs/am335x-evm.dtb > $(BIN_DIR)/$(IMG_PREFIX)-zImage-initramfs-dtb
	-cat $(BIN_DIR)/$(IMG_PREFIX)-zImage $(BIN_DIR)/dtbs/am335x-evm.dtb > $(BIN_DIR)/zImage-dtb
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-zImage-initramfs-dtb $(prefix_output)$(FW_FIT_NORNAND_KR_SUFFIX)
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-zImage-initramfs-dtb $(TFTP_DIR)
	$(CP) $(BIN_DIR)/zImage-dtb $(TFTP_DIR)/zImage
 endif
 ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "SP938BS")
	@echo '----------------------------------------------------------------'
	@echo '----- [SP938BS] build kernel image !!! '
	@echo '----------------------------------------------------------------'
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-zImage-initramfs $(prefix_output)$(FW_FIT_NORNAND_KR_SUFFIX)
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-zImage-initramfs $(TFTP_DIR)
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-zImage $(TFTP_DIR)
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-uImage $(TFTP_DIR)
 endif
 ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "SP935")
	@echo '----------------------------------------------------------------'
	@echo '----- [SP935] build kernel image !!! '
	@echo '----------------------------------------------------------------'
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-zImage-initramfs $(prefix_output)$(FW_FIT_NORNAND_KR_SUFFIX)
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-zImage-initramfs $(TFTP_DIR)
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-zImage $(TFTP_DIR)
	$(CP) $(BIN_DIR)/$(IMG_PREFIX)-uImage $(TFTP_DIR)
 endif
	$(RM) -rf $(NFS_DIR)/*
	-mkdir -p $(NFS_DIR)
	$(CP) $(TARGET_DIR)/* $(NFS_DIR)
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
endef

define Image/Build/jffs2-64k
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-$(1).img bs=65536 conv=sync
endef

define Image/Build/jffs2-128k
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-$(1).img bs=131072 conv=sync
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-$(1).img bs=131072 conv=sync
endef

$(eval $(call BuildImage))
