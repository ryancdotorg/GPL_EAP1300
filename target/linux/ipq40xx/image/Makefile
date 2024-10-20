# Copyright (c) 2014 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

UBIFS_OPTS = -m 2048 -e 124KiB -c 4096 -U -F
UBI_OPTS = -m 2048 -p 128KiB

E2SIZE=$(shell echo $$(($(CONFIG_TARGET_ROOTFS_PARTSIZE)*1024)))

ifeq ($(strip $(call CompareKernelPatchVer,$(KERNEL_PATCHVER),eq,3.18)),1)
IPQ806X_KERNEL_LOADADDR = 0x42008000
IPQ40XX_KERNEL_LOADADDR = 0x80008000
else
IPQ806X_KERNEL_LOADADDR = 0x42208000
IPQ40XX_KERNEL_LOADADDR = 0x80208000
endif
IPQ807X_KERNEL_LOADADDR = 0x41208000

ifneq ($(CONFIG_TARGET_ROOTFS_UBIFS),)
define Image/Build/ubifs
	$(call Image/ubifs_128MB,-ipq40xx)
	$(call Image/ubifs_512MB,-ipq40xx)
	$(call Image/ubifs_1GB,-ipq40xx)
endef
endif

define Image/BuildKernel/FIT
	$(CP) $(KDIR)/Image $(BIN_DIR)/$(IMG_PREFIX)-vmlinux.bin
	gzip -9 -c $(KDIR)/Image > $(KDIR)/Image.gz
	$(call CompressLzma,$(KDIR)/Image,$(KDIR)/Image.gz)
	$(call Image/BuildKernel/MkFIT,$(1), $(KDIR)/Image.gz, $(LINUX_DIR)/arch/arm/boot/dts/$(1).dtb,gzip,$(2),$(2))
	$(CP) $(KDIR)/fit-$(1).itb $(BIN_DIR)/$(IMG_PREFIX)-$(1)-fit-uImage.itb
	mkdir -p $(BIN_DIR)/dtbs/
	$(CP) $(LINUX_DIR)/arch/arm/boot/dts/qcom-ipq*.dtb $(BIN_DIR)/dtbs/
	$(CP) $(KDIR)/Image $(BIN_DIR)/dtbs/
endef

ifneq ($(CONFIG_TARGET_IMAGE_ZIMAGE_INITRAMFS),)
define Image/BuildKernel/FITInitramfs
	$(CP) $(KDIR)/Image-initramfs $(BIN_DIR)/dtbs/
	$(CP) $(KDIR)/Image-initramfs $(BIN_DIR)/$(IMG_PREFIX)-vmlinux-initramfs.bin
	$(call Image/BuildKernel/MkFIT,$(1), $(KDIR)/Image-initramfs, $(LINUX_DIR)/arch/arm/boot/dts/$(1).dtb, none,$(2),$(2))
	$(CP) $(KDIR)/fit-$(1).itb $(BIN_DIR)/$(IMG_PREFIX)-$(1)-fit-uImage-initramfs.itb

	$(CP) $(KDIR)/zImage-initramfs $(BIN_DIR)/dtbs/
	$(CP) $(KDIR)/zImage-initramfs $(BIN_DIR)/$(IMG_PREFIX)-vmlinuz-initramfs.bin
	$(call Image/BuildKernel/MkFIT,$(1), $(KDIR)/zImage-initramfs, $(LINUX_DIR)/arch/arm/boot/dts/$(1).dtb, none,$(2),$(2),-z)
	$(CP) $(KDIR)/fit-$(1)-z.itb $(BIN_DIR)/$(IMG_PREFIX)-$(1)-fit-uImage-initramfs-z.itb
endef
else
define Image/BuildKernel/FITInitramfs
	$(CP) $(KDIR)/Image-initramfs $(BIN_DIR)/dtbs/
	$(CP) $(KDIR)/Image-initramfs $(BIN_DIR)/$(IMG_PREFIX)-vmlinux-initramfs.bin
	$(call Image/BuildKernel/MkFIT,$(1), $(KDIR)/Image-initramfs, $(LINUX_DIR)/arch/arm/boot/dts/$(1).dtb, none,$(2),$(2))
	$(CP) $(KDIR)/fit-$(1).itb $(BIN_DIR)/$(IMG_PREFIX)-$(1)-fit-uImage-initramfs.itb
endef
endif

define Image/Prepare
	$(CP) $(LINUX_DIR)/vmlinux $(KDIR)/$(IMG_PREFIX)-vmlinux.elf
endef

define Image/BuildKernel/GenericFIT
	$(CP) $(KDIR)/Image $(BIN_DIR)/$(IMG_PREFIX)-vmlinux.bin
	gzip -9 -c $(KDIR)/Image > $(KDIR)/Image.gz
	$(call CompressLzma,$(KDIR)/Image,$(KDIR)/Image.gz)
	$(call Image/BuildKernel/MkFIT,$(1), $(KDIR)/Image.gz,,gzip,$(2),$(2))
	$(CP) $(KDIR)/fit-$(1).itb $(BIN_DIR)/$(IMG_PREFIX)-$(1)-fit-uImage.itb
endef

define Image/BuildKernel/MultiDTBFIT
	$(CP) $(KDIR)/Image $(BIN_DIR)/$(IMG_PREFIX)-vmlinux.bin
	gzip -9 -c $(KDIR)/Image > $(KDIR)/Image.gz
	$(call CompressLzma,$(KDIR)/Image,$(KDIR)/Image.gz)
ifeq ($(CONFIG_PACKAGE_senao-kdump),y)
	$(call Image/BuildKernel/MkFITMulti,$(1),$(KDIR)/zImage,$(2),none,$(3),$(3))
else
	$(call Image/BuildKernel/MkFITMulti,$(1),$(KDIR)/Image.gz,$(2),gzip,$(3),$(3))
endif
	$(CP) $(KDIR)/fit-$(1).itb $(BIN_DIR)/$(IMG_PREFIX)-$(1)-fit-uImage.itb
endef

# some tree's don't have all the device tree's so we filter out
# the ones that are not present
# $(1) is the filter to apply
FindDeviceTrees ?= $(notdir $(basename $(shell find $(LINUX_DIR)/arch/arm/boot/dts/ -name $(1)*.dtb $(2) 2>/dev/null)))

define Image/BuildKernel/template
	$(CP) $(KDIR)/vmlinux$(subst openwrt-$(BOARD),,$(IMG_PREFIX)).debug $(BIN_DIR)/$(IMG_PREFIX)-vmlinux.elf

	$(foreach device, $(call FindDeviceTrees, qcom-ipq40??-), \
		$(call Image/BuildKernel/$(1),$(device),$(IPQ40XX_KERNEL_LOADADDR));)
endef

define Image/BuildKernel
	$(call Image/BuildKernel/template,FIT)

	$(call Image/BuildKernel/GenericFIT,qcom-ipq40xx,$(IPQ40XX_KERNEL_LOADADDR))
ifeq ($(QCA_SP_REPO),qca_spf_6_0_CSU1)
	$(call Image/BuildKernel/MultiDTBFIT,qcom-ipq40xx-ap.dkxx, "qcom-ipq40xx-ap.dk04.1-c1 \
															qcom-ipq40xx-ap.dk04.1-c2 \
															qcom-ipq40xx-ap.dk04.1-c3 \
															qcom-ipq40xx-ap.dk01.1-c1 \
															qcom-ipq40xx-ap.dk01.1-c2"\
															,$(IPQ40XX_KERNEL_LOADADDR))
else
	$(call Image/BuildKernel/MultiDTBFIT,qcom-ipq40xx-ap.dkxx, "qcom-ipq4019-ap.dk04.1-c1 \
																qcom-ipq4019-ap.dk04.1-c2 \
																qcom-ipq4019-ap.dk04.1-c3 \
																qcom-ipq4019-ap.dk01.1-c1 \
																qcom-ipq4019-ap.dk01.1-c2"\
																,$(IPQ40XX_KERNEL_LOADADDR))
endif
endef

define Image/BuildKernel/Initramfs
	$(CP) $(KDIR)/vmlinux-initramfs$(subst openwrt-$(BOARD),,$(IMG_PREFIX)).debug $(BIN_DIR)/$(IMG_PREFIX)-vmlinux-initramfs.elf
	$(call Image/BuildKernel/template,FITInitramfs)
endef

UBI_IMAGES=""

define Image/UbiPrepare
	$(call Image/Build/UbinizeImage,$(1),$(BIN_DIR)/$(IMG_PREFIX)-$(1)-fit-uImage.itb,$(2),$(3),$(4))
endef

define Image/UbiImage
	$(eval UBI_OPTS:="-m 2048 -p 128KiB")
	$(foreach image,$(UBI_IMAGES), \
	$(call Image/UbiPrepare,$(image),squashfs,"ubi_rootfs","rootfs_data"))
endef

define Image/ubifs_1GB
	$(eval UBI_OPTS:="-m 4096 -p 512KiB")
	$(call Image/mkfs/ubifs/generate,$(1))
	$(call Image/Build,ubi,$(1),-1GB)
endef

define Image/ubifs_512MB
	$(eval UBI_OPTS:="-m 4096 -p 256KiB")
	$(call Image/mkfs/ubifs/generate,$(1))
	$(call Image/Build,ubi,$(1),-512MB)
endef

define Image/ubifs_128MB
	$(eval UBI_OPTS:="-m 2048 -p 128KiB")
	$(call Image/mkfs/ubifs/generate,$(1))
	$(call Image/Build,ubi,$(1))
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
	dd if=$(KDIR)/root$(2).$(1) of=$(BIN_DIR)/$(IMG_PREFIX)$(2)-$(1)-root$(3).img bs=2k conv=sync
endef

### SENAO ###
ifeq (norplusemmc,$(IMG_TYPE))
include mkimg_emmc.mk
else
include mkimg.mk
endif
$(eval $(call BuildImage))
