#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

override TARGET_BUILD=
include $(INCLUDE_DIR)/prereq.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/host.mk

.NOTPARALLEL:
override MAKEFLAGS=
override MAKE:=$(SUBMAKE)
KDIR=$(KERNEL_BUILD_DIR)
DTS_DIR:=$(LINUX_DIR)/arch/$(LINUX_KARCH)/boot/dts/$(if $(CONFIG_TARGET_ipq807x)$(CONFIG_TARGET_ipq_ipq807x_64)$(CONFIG_TARGET_ipq_ipq60xx_64),qcom)
MKFS_DEVTABLE_OPT := -D $(INCLUDE_DIR)/device_table.txt

IMG_PREFIX:=openwrt-$(BOARD)$(if $(SUBTARGET),-$(SUBTARGET))

ifneq ($(CONFIG_BIG_ENDIAN),)
  JFFS2OPTS     :=  --big-endian --squash-uids -v
else
  JFFS2OPTS     :=  --little-endian --squash-uids -v
endif

ifeq ($(CONFIG_JFFS2_RTIME),y)
  JFFS2OPTS += -X rtime
endif
ifeq ($(CONFIG_JFFS2_ZLIB),y) 
  JFFS2OPTS += -X zlib
endif
ifeq ($(CONFIG_JFFS2_LZMA),y)
  JFFS2OPTS += -X lzma --compression-mode=size
endif
ifneq ($(CONFIG_JFFS2_RTIME),y)
  JFFS2OPTS += -x rtime
endif
ifneq ($(CONFIG_JFFS2_ZLIB),y)
  JFFS2OPTS += -x zlib
endif
ifneq ($(CONFIG_JFFS2_LZMA),y)
  JFFS2OPTS += -x lzma
endif

JFFS2OPTS += $(MKFS_DEVTABLE_OPT)

SQUASHFS_BLOCKSIZE := 256k
SQUASHFSOPT := -b $(SQUASHFS_BLOCKSIZE)
SQUASHFSOPT += $(if $(call kernel_patchver_gt,3.4),-p '/dev d 755 0 0' -p '/dev/console c 600 0 0 5 1')
SQUASHFSCOMP := gzip
LZMA_XZ_OPTIONS := -Xpreset 9 -Xe -Xlc 0 -Xlp 2 -Xpb 2
ifeq ($(CONFIG_SQUASHFS_LZMA),y)
  SQUASHFSCOMP := lzma $(LZMA_XZ_OPTIONS)
endif
ifeq ($(CONFIG_SQUASHFS_XZ),y)
  ifneq ($(filter arm x86 powerpc sparc,$(LINUX_KARCH)),)
    BCJ_FILTER:=-Xbcj $(LINUX_KARCH)
  endif
  SQUASHFSCOMP := xz $(LZMA_XZ_OPTIONS) $(if $(call kernel_patchver_gt,3.4),$(BCJ_FILTER))
endif

JFFS2_BLOCKSIZE ?= 64k 128k

define add_jffs2_mark
	echo -ne '\xde\xad\xc0\xde' >> $(1)
endef

define toupper
$(shell echo $(1) | tr '[:lower:]' '[:upper:]')
endef

# pad to 4k, 8k, 16k, 64k, 128k, 256k and add jffs2 end-of-filesystem mark
define prepare_generic_squashfs
	$(STAGING_DIR_HOST)/bin/padjffs2 $(1) 4 8 16 64 128 256
endef


ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)

  define Image/BuildKernel/Initramfs
	cp $(KDIR)/vmlinux-initramfs.elf $(BIN_DIR)/$(IMG_PREFIX)-vmlinux-initramfs.elf
	$(call Image/Build/Initramfs)
  endef

else
  define Image/BuildKernel/Initramfs
  endef
endif

define Image/BuildKernel/MkuImage
	mkimage -A $(ARCH) -O linux -T kernel -C $(1) -a $(2) -e $(3) \
		-n '$(call toupper,$(ARCH)) OpenWrt Linux-$(LINUX_VERSION)' -d $(4) $(5)
endef

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_8_0_CS qca_spf_8_0_CSU1 qca_spf_9_0_ES qca_spf_9_0_rel3 qca_spf_9_0_rel4 qca_spf_9_0_CS))
define Image/BuildKernel/MkFIT
	$(TOPDIR)/scripts/mkits_qca_spf_8_0_CS.sh \
		-D $(1) -o $(KDIR)/fit-$(1).its -k $(2) $(if $(3),-d $(3)) -C $(4) -a $(5) -e $(6) \
		-A $(ARCH) -v $(LINUX_VERSION)
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $(KDIR)/fit-$(1).its $(KDIR)/fit-$(1)$(7).itb
endef
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_10_0_ES2 qca_spf_10_0_FC qca_spf_10_0_CS_PREVIEW qca_spf_10_0_CS qca_spf_10_0_CS_PATCH1 \
	qca_spf_10_0_CS_PATCH2 qca_spf_10_0_CS_PATCH5 qca_ipq6018_ilq12.0_es1 qca_ipq6018_ilq11.0_fc qca_spf_11_0_csu1 qca_spf_11_0_csu3 qca_spf_11_1_csu2 qca_spf_11_2))
define Image/BuildKernel/MkFIT
	$(TOPDIR)/scripts/mkits_qca_spf_8_0_CS.sh \
		-D $(1) -o $(KDIR)/fit-$(1).its -k $(2) $(if $(3),-d $(3)) -C $(4) -a $(5) -e $(6) \
		-A $(LINUX_KARCH) -v $(LINUX_VERSION)
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $(KDIR)/fit-$(1).its $(KDIR)/fit-$(1)$(7).itb
endef
else
define Image/BuildKernel/MkFIT
	$(TOPDIR)/scripts/mkits.sh \
		-D $(1) -o $(KDIR)/fit-$(1).its -k $(2) $(if $(3),-d $(3)) -C $(4) -a $(5) -e $(6) \
		-A $(ARCH) -v $(LINUX_VERSION)
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $(KDIR)/fit-$(1).its $(KDIR)/fit-$(1)$(7).itb
endef
endif

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_8_0_CS qca_spf_8_0_CSU1 qca_spf_9_0_ES qca_spf_9_0_rel3 qca_spf_9_0_rel4 qca_spf_9_0_CS))
define Image/BuildKernel/MkFITMulti
	$(TOPDIR)/scripts/mkits_qca_spf_8_0_CS.sh \
		-D $(1) -o $(KDIR)/fit-$(1).its -k $(2) $(foreach dtb,$(subst ",,$(3)), -d $(LINUX_DIR)/arch/arm/boot/dts/$(dtb).dtb) -C $(4) -a $(5) -e $(6) \
		-A $(ARCH) -v $(LINUX_VERSION)
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $(KDIR)/fit-$(1).its $(KDIR)/fit-$(1).itb
endef
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_10_0_ES2 qca_spf_10_0_FC qca_spf_10_0_CS_PREVIEW qca_spf_10_0_CS qca_spf_10_0_CS_PATCH1 \
	qca_spf_10_0_CS_PATCH2 qca_spf_10_0_CS_PATCH5 qca_ipq6018_ilq12.0_es1 qca_ipq6018_ilq11.0_fc qca_spf_11_0_csu1 qca_spf_11_0_csu3 qca_spf_11_1_csu2 qca_spf_11_2))
define Image/BuildKernel/MkFITMulti
	$(TOPDIR)/scripts/mkits_qca_spf_8_0_CS.sh \
		-D $(1) -o $(KDIR)/fit-$(1).its -k $(2) $(foreach dtb,$(subst \",,$(3)), -d $(DTS_DIR)/$(dtb).dtb$(if $(7),.gz)) -C $(4) -a $(5) -e $(6) $(if $(7),-c $(7) -l $(8))\
		-A $(LINUX_KARCH) -v $(LINUX_VERSION)
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $(KDIR)/fit-$(1).its $(KDIR)/fit-$(1).itb
endef
else
define Image/BuildKernel/MkFITMulti
	$(TOPDIR)/scripts/mkits.sh \
		-D $(1) -o $(KDIR)/fit-$(1).its -k $(2) $(foreach dtb,$(subst ",,$(3)), -d $(LINUX_DIR)/arch/arm/boot/dts/$(dtb).dtb) -C $(4) -a $(5) -e $(6)\
		-A $(ARCH) -v $(LINUX_VERSION)
	PATH=$(LINUX_DIR)/scripts/dtc:$(PATH) mkimage -f $(KDIR)/fit-$(1).its $(KDIR)/fit-$(1).itb
endef
endif

define Image/mkfs/jffs2/sub
	# FIXME: removing this line will cause strange behaviour in the foreach loop below
	$(STAGING_DIR_HOST)/bin/mkfs.jffs2 $(3) --pad -e $(patsubst %k,%KiB,$(1)) -o $(KDIR)/root.jffs2-$(2) -d $(TARGET_DIR) -v 2>&1 1>/dev/null | awk '/^.+$$$$/'
	$(STAGING_DIR_HOST)/bin/mkfs.jffs2 $(3) -e $(patsubst %k,%KiB,$(1)) -o $(KDIR)/root.jffs2-$(2)-raw -d $(TARGET_DIR) -v 2>&1 1>/dev/null | awk '/^.+$$$$/'
	$(call add_jffs2_mark,$(KDIR)/root.jffs2-$(2))
	$(call Image/Build,jffs2-$(2))
endef

ifneq ($(CONFIG_TARGET_ROOTFS_JFFS2),)
    define Image/mkfs/jffs2
		$(foreach SZ,$(JFFS2_BLOCKSIZE),$(call Image/mkfs/jffs2/sub,$(SZ),$(SZ),$(JFFS2OPTS)))
    endef
endif
ifneq ($(CONFIG_TARGET_ROOTFS_JFFS2_NAND),)
    define Image/mkfs/jffs2_nand
               $(foreach SZ,$(NAND_BLOCKSIZE), $(call Image/mkfs/jffs2/sub, \
                       $(word 2,$(subst :, ,$(SZ))),nand-$(subst :,-,$(SZ)), \
                       $(JFFS2OPTS) --no-cleanmarkers --pagesize=$(word 1,$(subst :, ,$(SZ)))) \
               )
    endef
endif

ifneq ($(CONFIG_TARGET_ROOTFS_SQUASHFS),)
    define Image/mkfs/squashfs
		@mkdir -p $(TARGET_DIR)/overlay
		$(TOPDIR)/SENAO/scripts/fw_version.sh $(TOPDIR) $(TARGET_DIR)/etc/version
		$(STAGING_DIR_HOST)/bin/mksquashfs4 $(TARGET_DIR) $(KDIR)/root.squashfs -nopad -noappend -root-owned -comp $(SQUASHFSCOMP) $(SQUASHFSOPT) -processors $(if $(CONFIG_PKG_BUILD_JOBS),$(CONFIG_PKG_BUILD_JOBS),1)
		$(call Image/Build,squashfs)
    endef
endif

ifneq ($(CONFIG_TARGET_ROOTFS_UBIFS),)
  define Image/mkfs/ubifs/generate
	$(CP) ./ubinize$(1).cfg $(KDIR)
	( cd $(KDIR); \
		$(STAGING_DIR_HOST)/bin/ubinize \
		$(if $($(PROFILE)_UBI_OPTS), \
			$(shell echo $($(PROFILE)_UBI_OPTS)), \
			$(shell echo $(UBI_OPTS)) \
		) \
		-o $(KDIR)/root$(1).ubi \
		ubinize$(1).cfg \
	)
  endef

    define Image/mkfs/ubifs

        ifneq ($($(PROFILE)_UBIFS_OPTS)$(UBIFS_OPTS),)
		$(STAGING_DIR_HOST)/bin/mkfs.ubifs \
			$(if $($(PROFILE)_UBIFS_OPTS), \
				$(shell echo $($(PROFILE)_UBIFS_OPTS)), \
				$(shell echo $(UBIFS_OPTS)) \
			) \
			$(if $(CONFIG_TARGET_UBIFS_FREE_SPACE_FIXUP),--space-fixup) \
			$(if $(CONFIG_TARGET_UBIFS_COMPRESSION_NONE),--force-compr=none) \
			$(if $(CONFIG_TARGET_UBIFS_COMPRESSION_LZO),--force-compr=lzo) \
			$(if $(CONFIG_TARGET_UBIFS_COMPRESSION_ZLIB),--force-compr=zlib) \
			$(if $(shell echo $(CONFIG_TARGET_UBIFS_JOURNAL_SIZE)),--jrn-size=$(CONFIG_TARGET_UBIFS_JOURNAL_SIZE)) \
			--squash-uids \
			-o $(KDIR)/root.ubifs \
			-d $(TARGET_DIR)
        endif
		$(call Image/Build,ubifs)
        ifneq ($($(PROFILE)_UBI_OPTS)$(UBI_OPTS),)
		$(if $(wildcard $KDIR/uImage),$(call Image/mkfs/ubifs/generate,))
		$(if $(wildcard ./ubinize-overlay.cfg),$(call Image/mkfs/ubifs/generate,-overlay))
        endif
		$(if $(wildcard $KDIR/root.ubi),$(call Image/Build,ubi))
    endef
endif

ifneq ($(CONFIG_TARGET_ROOTFS_CPIOGZ),)
  define Image/mkfs/cpiogz
		( cd $(TARGET_DIR); find . | cpio -o -H newc | gzip -9 >$(BIN_DIR)/$(IMG_PREFIX)-rootfs.cpio.gz )
  endef
endif

ifneq ($(CONFIG_TARGET_ROOTFS_TARGZ),)
  define Image/mkfs/targz
	# Preserve permissions (-p) when building as non-root user
	$(TAR) -czpf $(BIN_DIR)/$(IMG_PREFIX)$(if $(PROFILE),-$(PROFILE))-rootfs.tar.gz --numeric-owner --owner=0 --group=0 -C $(TARGET_DIR)/ .
  endef
endif

ifneq ($(CONFIG_TARGET_ROOTFS_EXT4FS),)
  E2SIZE=$(shell echo $$(($(CONFIG_TARGET_ROOTFS_PARTSIZE)*1024)))

  define Image/mkfs/ext4
# generate an ext2 fs
	$(STAGING_DIR_HOST)/bin/genext2fs -U -b $(E2SIZE) -N $(CONFIG_TARGET_ROOTFS_MAXINODE) -d $(TARGET_DIR)/ $(KDIR)/root.ext4 -m $(CONFIG_TARGET_ROOTFS_RESERVED_PCT) $(MKFS_DEVTABLE_OPT)
# convert it to ext4
	$(STAGING_DIR_HOST)/bin/tune2fs -O extents,uninit_bg,dir_index $(KDIR)/root.ext4
# fix it up
	$(STAGING_DIR_HOST)/bin/e2fsck -fy $(KDIR)/root.ext4
	$(call Image/Build,ext4)
  endef
endif

ifneq ($(CONFIG_TARGET_ROOTFS_ISO),)
  define Image/mkfs/iso
		$(call Image/Build,iso)
  endef
endif


define Image/mkfs/prepare/default
	# Use symbolic permissions to avoid clobbering SUID/SGID/sticky bits
	- $(FIND) $(TARGET_DIR) -type f -not -perm +0100 -not -name 'ssh_host*' -not -name 'shadow' -print0 | $(XARGS) -0 chmod u+rw,g+r,o+r
	- $(FIND) $(TARGET_DIR) -type f -perm +0100 -print0 | $(XARGS) -0 chmod u+rwx,g+rx,o+rx
	- $(FIND) $(TARGET_DIR) -type d -print0 | $(XARGS) -0 chmod u+rwx,g+rx,o+rx
	$(INSTALL_DIR) $(TARGET_DIR)/tmp
	chmod 1777 $(TARGET_DIR)/tmp
endef

define Image/mkfs/prepare
	$(call Image/mkfs/prepare/default)
endef


define Image/Checksum
	( cd ${BIN_DIR} ; \
		$(FIND) -maxdepth 1 -type f \! -name 'md5sums'  -printf "%P\n" | sort | xargs \
		md5sum --binary > md5sums \
	)
endef

### SENAO ###
  define Image/CopyLoader
	if [ -e ${BIN_DIR}/$(1) ]; then \
		echo -e "Copy \e[33m$(1)\e[0m to /tftpboot/"; \
		${CP} ${BIN_DIR}/$(1) /tftpboot/; \
	fi
  endef
ifeq ($(BOARD),ipq40xx)
  define Image/Loader
        ( cd ${BIN_DIR} ; \
                $(FIND) -maxdepth 1 -type f \! -name 'md5sums'  -printf "%P\n" | sort | xargs \
                md5sum --binary > md5sums \
        )
	$(call checkConfig)
	$(call PrepareBuildDir)
	$(call GenCDT)
	$(call GenMIBIB)
	$(call BuildLoader)
	$(call BuildFitLoader)
	$(call Image/CopyLoader,$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_UBOOT_SUFFIX))
	$(call Image/CopyLoader,$(FW_PREFIX)$(PRODUCT_NAME_L)-u-boot.elf)
	$(call Image/CopyLoader,$(FW_PREFIX)$(PRODUCT_NAME_L)-u-boot.img)
	$(call Image/CopyLoader,$(FW_PREFIX)$(PRODUCT_NAME_L)-u-boot.mbn)
	$(call Image/CopyLoader,$(FW_PREFIX)$(PRODUCT_NAME_L)-u-boot-stripped.elf)
  endef
endif
ifeq ($(BOARD),ipq806x)
  define Image/Loader
        ( cd ${BIN_DIR} ; \
                $(FIND) -maxdepth 1 -type f \! -name 'md5sums'  -printf "%P\n" | sort | xargs \
                md5sum --binary > md5sums \
        )
	$(call checkConfig)
	$(call BuildFitBootloader)
	$(call Image/CopyLoader,$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_UBOOT_SUFFIX))
	$(call Image/CopyLoader,$(FW_PREFIX)$(PRODUCT_NAME_L)-u-boot.mbn)
  endef
endif
ifeq ($(BOARD),ipq)
  define Image/Loader
        ( cd ${BIN_DIR} ; \
                $(FIND) -maxdepth 1 -type f \! -name 'md5sums'  -printf "%P\n" | sort | xargs \
                md5sum --binary > md5sums \
        )
	$(call checkConfig)
	$(call Prepare)
	$(call GenCDT)
	$(call GenBoot)
	$(call GenPartition)
	$(call GenMbn)
	$(call PackFitLoader)
	$(call Image/CopyLoader,$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_UBOOT_SUFFIX))
	$(call Image/CopyLoader,$(FW_PREFIX)$(PRODUCT_NAME_L)-u-boot.mbn)
  endef
endif
ifeq ($(BOARD),sstar)
  define Image/Loader
        ( cd ${BIN_DIR} ; \
                $(FIND) -maxdepth 1 -type f \! -name 'md5sums'  -printf "%P\n" | sort | xargs \
                md5sum --binary > md5sums \
        )
	$(call Gen_Bootloader)
  endef
endif
#############

define BuildImage

### SENAO ###
    lfw:
	$(call Image/Loader)
#############

  download:
  prepare:

  ifeq ($(IB),)
    compile: compile-targets FORCE
		$(call Build/Compile)
  else
    compile:
  endif

  ifeq ($(IB),)
    install: compile install-targets FORCE
		$(call Image/Prepare)
		$(call Image/mkfs/prepare)
		$(call Image/BuildKernel)
		$(call Image/BuildKernel/Initramfs)
		$(call Image/InstallKernel)
		$(call Image/mkfs/cpiogz)
		$(call Image/mkfs/targz)
		$(call Image/mkfs/ext4)
		$(call Image/mkfs/iso)
		$(call Image/mkfs/jffs2)
		$(call Image/mkfs/jffs2_nand)
		$(call Image/mkfs/squashfs)
		$(call Image/mkfs/ubifs)
ifneq ($(BOARD),ipq)
		$(call Image/mkfs/ubifs_fit)
endif
		$(call Image/Checksum)
		$(call Image/mkimage,squashfs)
  else
    install: compile install-targets
		$(call Image/BuildKernel)
		$(call Image/BuildKernel/Initramfs)
		$(call Image/InstallKernel)
		$(call Image/mkfs/cpiogz)
		$(call Image/mkfs/targz)
		$(call Image/mkfs/ext4)
		$(call Image/mkfs/iso)
		$(call Image/mkfs/jffs2)
		$(call Image/mkfs/jffs2_nand)
		$(call Image/mkfs/squashfs)
		$(call Image/mkfs/ubifs)
ifneq ($(BOARD),ipq)
		$(call Image/mkfs/ubifs_fit)
endif
		$(call Image/Checksum)
  endif

  ifeq ($(IB),)
    clean: clean-targets
		$(call Build/Clean)
  else
    clean:
  endif

  compile-targets:
  install-targets:
  clean-targets:

endef
