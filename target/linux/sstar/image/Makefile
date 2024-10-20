# Copyright (c) 2014 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

_R:=\\033[31m
_G:=\\033[32m
_Y:=\\033[33m
_B:=\\033[34m
_N:=\\033[m
_LN:=\\033[1;36m

DTS_DIR:=$(LINUX_DIR)/arch/arm/boot/dts/

ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
define Image/BuildKernel/FITInitramfs
	$(CP) $(KDIR)/Image-initramfs $(BIN_DIR)/dtbs/
	$(CP) $(KDIR)/Image-initramfs $(BIN_DIR)/$(IMG_PREFIX)-vmlinux-initramfs.bin
	$(call Image/BuildKernel/MkFIT,$(1)-initramfs, $(KDIR)/Image-initramfs, $(DTS_DIR)/$(1).dtb, none,$(2),$(2))
	$(CP) $(KDIR)/fit-$(1)-initramfs.itb $(BIN_DIR)/$(IMG_PREFIX)-$(1)-fit-uImage-initramfs.itb
endef
endif

define Image/Prepare
	$(CP) $(LINUX_DIR)/vmlinux $(KDIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-vmlinux.elf
	$(CP) $(LINUX_DIR)/arch/arm/boot/uImage.xz $(KDIR)/
	$(CP) $(LINUX_DIR)/arch/arm/boot/uImage.xz $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_KERNEL_SUFFIX)
endef

# some tree's don't have all the device tree's so we filter out
# the ones that are not present
# $(1) is the filter to apply
FindDeviceTrees ?= $(notdir $(basename $(shell find $(DTS_DIR)/ -name $(1)*.dtb $(2) 2>/dev/null)))

define Image/BuildKernel
	$(call Image/BuildKernel/template,FIT)
endef

ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
define Image/BuildKernel/Initramfs
	$(call Image/BuildKernel/template,FITInitramfs)
endef
endif

UBIFS_OPTS = -m 2048 -e 124KiB -c 4096 -U -F
UBI_OPTS = -m 2048 -p 128KiB

#ifneq ($(CONFIG_TARGET_ROOTFS_UBIFS),)
define Image/Build/ubifs
	@echo -e "$(_G)  [[ubifs]] root.ubifs to $(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_UBI_ROOTFS_SUFFIX)$(_N)"
	$(CP) $(KDIR)/root.ubifs $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_UBI_ROOTFS_SUFFIX)
	$(call Image/ubifs_128MB,-$(CONFIG_TARGET_SUBTARGET))
	dd if=$(KDIR)/root.ubifs of=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_UBI_ROOTFS_SUFFIX) bs=2k conv=sync
endef
define Image/ubifs_128MB
	@echo -e "$(_G)  [[ubifs_128MB]] $(KDIR)/root$(1).ubi$(_N)"
	$(eval UBI_OPTS:="-m 2048 -p 128KiB")
	$(call Image/mkfs/ubifs/generate,$(1))
	$(CP) $(KDIR)/root-$(CONFIG_TARGET_SUBTARGET).ubi $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L).ubi
endef
#endif

ifneq ($(CONFIG_TARGET_ROOTFS_SQUASHFS),)
### FIXME - package/libs/toolchain
    define Image/mkfs/squashfs
		@echo -e "$(_G)  [[Image/mkfs/squashfs]] root.squashfs$(_N)"
		@mkdir -p $(TARGET_DIR)/overlay
ifeq ($(CONFIG_GCC_VERSION_8),)
		-(cd $(TARGET_DIR) && \
			$(CP) arm-linux-gnueabihf/libc/lib/arm-linux-gnueabihf/* lib/ && \
			$(CP) arm-linux-gnueabihf/libc/usr/lib/arm-linux-gnueabihf/libc.so usr/lib/ && \
			$(CP) arm-linux-gnueabihf/lib/lib* lib/ && \
			rm -rf arm-linux-gnueabihf/)
		-(cd $(TARGET_DIR)/lib && rm -f ld-linux-armhf.so.3 && ln -sf ld-2.19-2014.04.so ld-linux-armhf.so.3)
		-(cd $(TARGET_DIR)/lib && rm -f arm-linux-gnueabihf && ln -sf /lib/ arm-linux-gnueabihf)
		-(cd $(TARGET_DIR)/usr/lib && rm -f arm-linux-gnueabihf && ln -sf /usr/lib/ arm-linux-gnueabihf)
else
		-(cd $(TARGET_DIR) && \
			$(CP) arm-linux-gnueabihf/libc/lib/* lib/ && \
			$(CP) arm-linux-gnueabihf/libc/usr/lib/libc.so usr/lib/ && \
			$(CP) arm-linux-gnueabihf/lib/lib* lib/ && \
			rm -rf arm-linux-gnueabihf/)
endif
		$(TOPDIR)/SENAO/scripts/fw_version.sh $(TOPDIR) $(TARGET_DIR)/etc/version
		$(STAGING_DIR_HOST)/bin/mksquashfs4 $(TARGET_DIR) $(KDIR)/root.squashfs -comp xz -all-root -noappend
		$(call Image/Build,squashfs)
    endef
endif

### append 'deadc0de'
define Image/Build/squashfs
	@echo -e "$(_G)  [[Image/Build/squashfs]] root.squashfs to $(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_ROOTFS_SUFFIX)$(_N)"
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	dd if=$(KDIR)/root.squashfs of=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_ROOTFS_SUFFIX) bs=2k conv=sync
endef

define Image/Build
    $(call Image/Build/$(1),$(1))
endef

define Image/Checksum
	( cd ${BIN_DIR} ; \
		$(FIND) -maxdepth 1 -type f \! -name 'md5sums'  -printf "%P\n" | sort | xargs \
		md5sum --binary > md5sums \
	)
	$(call Gen_Bootloader)
	$(call Sysupgrade)
	-$(CP) ${BIN_DIR}/$(FW_PREFIX)$(PRODUCT_NAME_L)* /tftpboot
endef

ifneq ($(CONFIG_PACKAGE_uboot-sstar-infinity6-SENAO_SPINOR),)
FLASH_TYPE:=nor
else
FLASH_TYPE:=spinand
endif
include mkimg.mk
$(eval $(call BuildImage))
