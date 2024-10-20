#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# Main makefile for the host tools
#
curdir:=tools

# subdirectories to descend into
tools-y :=

ifeq ($(CONFIG_EXTERNAL_TOOLCHAIN),)
  BUILD_TOOLCHAIN := y
endif
ifdef CONFIG_GCC_USE_GRAPHITE
  ifeq ($(CONFIG_GCC_USE_SYSTEM_PPL_CLOOG),)
    BUILD_PPL_CLOOG = y
  endif
endif
ifneq ($(CONFIG_PACKAGE_kmod-b43)$(CONFIG_PACKAGE_kmod-b43legacy)$(CONFIG_BRCMSMAC_USE_FW_FROM_WL),)
  BUILD_B43_TOOLS = y
endif

tools-$(BUILD_TOOLCHAIN) += gmp mpfr mpc libelf
tools-y += m4 libtool autoconf automake flex bison pkg-config sed mklibs
tools-y += sstrip make-ext4fs genext2fs e2fsprogs mtd-utils mkimage
tools-y += firmware-utils patch-image patch quilt yaffs2 flock padjffs2
tools-y += mm-macros missing-macros xz cmake scons bc findutils gengetopt patchelf
### SENAO ###
tools-y += patchutils
tools-y += shc
tools-y += cfdk
tools-y += ipkg-utils
#############
tools-$(CONFIG_TARGET_orion_generic) += wrt350nv2-builder upslug2
tools-$(CONFIG_powerpc) += upx
tools-$(CONFIG_TARGET_x86) += qemu
tools-$(CONFIG_TARGET_mxs) += elftosb
tools-$(CONFIG_TARGET_brcm2708)$(CONFIG_TARGET_sunxi)$(CONFIG_TARGET_mxs) += mtools dosfstools
tools-$(CONFIG_TARGET_ar71xx) += lzma-old squashfs
ifeq ($(filter y, $(CONFIG_TARGET_ipq40xx_5_2) $(CONFIG_TARGET_ipq806x_5_2)),y)
tools-$(CONFIG_TARGET_ipq40xx_5_2)$(CONFIG_TARGET_ipq) += ramdump-parser
endif
tools-y += lzma squashfs4
tools-$(BUILD_B43_TOOLS) += b43-tools
tools-$(BUILD_PPL_CLOOG) += ppl cloog
tools-$(CONFIG_USE_SPARSE) += sparse
ifeq ($(CONFIG_TARGET_sstar),)
tools-y += coccinelle
endif

# builddir dependencies
$(curdir)/bison/compile := $(curdir)/flex/compile
$(curdir)/flex/compile := $(curdir)/libtool/compile
$(curdir)/pkg-config/compile := $(curdir)/sed/compile
$(curdir)/libtool/compile := $(curdir)/sed/compile $(curdir)/m4/compile $(curdir)/autoconf/compile $(curdir)/automake/compile $(curdir)/missing-macros/compile
$(curdir)/squashfs/compile := $(curdir)/lzma-old/compile
$(curdir)/squashfs4/compile := $(curdir)/xz/compile
$(curdir)/quilt/compile := $(curdir)/sed/compile $(curdir)/autoconf/compile $(curdir)/findutils/compile
$(curdir)/autoconf/compile := $(curdir)/m4/compile
$(curdir)/automake/compile := $(curdir)/m4/compile $(curdir)/autoconf/compile $(curdir)/pkg-config/compile $(curdir)/xz/compile
$(curdir)/gmp/compile := $(curdir)/libtool/compile
$(curdir)/mpc/compile := $(curdir)/mpfr/compile $(curdir)/gmp/compile
$(curdir)/mpfr/compile := $(curdir)/gmp/compile
$(curdir)/ppl/compile := $(curdir)/gmp/compile
$(curdir)/cloog/compile := $(curdir)/ppl/compile
$(curdir)/mtd-utils/compile := $(curdir)/e2fsprogs/compile $(curdir)/xz/compile
$(curdir)/mkimage/compile := $(curdir)/sed/compile
$(curdir)/mklibs/compile := $(curdir)/libtool/compile
$(curdir)/qemu/compile := $(curdir)/e2fsprogs/compile
$(curdir)/upslug2/compile := $(curdir)/libtool/compile
$(curdir)/mm-macros/compile := $(curdir)/libtool/compile
$(curdir)/missing-macros/compile := $(curdir)/autoconf/compile
$(curdir)/e2fsprogs/compile := $(curdir)/libtool/compile
$(curdir)/libelf/compile := $(curdir)/libtool/compile
$(curdir)/sdcc/compile := $(curdir)/bison/compile
$(curdir)/b43-tools/compile := $(curdir)/bison/compile
$(curdir)/padjffs2/compile := $(curdir)/findutils/compile
$(curdir)/cloog/compile := $(curdir)/ppl/compile
$(curdir)/bc/compile := $(curdir)/bison/compile
$(curdir)/findutils/compile := $(curdir)/bison/compile
$(curdir)/gengetopt/compile := $(curdir)/libtool/compile
$(curdir)/patchelf/compile := $(curdir)/libtool/compile
$(curdir)/genext2fs/compile := $(curdir)/genext2fs/compile
$(curdir)/patchutils/compile := $(curdir)/m4/compile

ifneq ($(CONFIG_CCACHE)$(CONFIG_SDK),)
$(foreach tool, $(filter-out patch,$(tools-y)), $(eval $(curdir)/$(tool)/compile += $(curdir)/ccache/compile))
tools-y += ccache
endif

# in case there is no patch tool on the host we need to make patch tool a
# dependency for tools which have patches directory
$(foreach tool, $(tools-y), $(if $(wildcard $(curdir)/$(tool)/patches),$(eval $(curdir)/$(tool)/compile += $(curdir)/patch/compile)))

$(curdir)/builddirs := $(tools-y) $(tools-dep) $(tools-)
$(curdir)/builddirs-default := $(tools-y)

ifndef DUMP_TARGET_DB
define PrepareStaging
	@for dir in $(1); do ( \
		$(if $(QUIET),,set -x;) \
		mkdir -p "$$dir"; \
		cd "$$dir"; \
		mkdir -p bin lib include stamp; \
	); done
endef

# preparatory work
$(STAGING_DIR)/.prepared: $(TMP_DIR)/.build
	$(call PrepareStaging,$(STAGING_DIR))
	mkdir -p $(BUILD_DIR)/stamp
	touch $@

$(STAGING_DIR_HOST)/.prepared: $(TMP_DIR)/.build
	$(call PrepareStaging,$(STAGING_DIR_HOST))
	mkdir -p $(BUILD_DIR_HOST)/stamp $(STAGING_DIR_HOST)/include/sys
	$(INSTALL_DATA) $(TOPDIR)/tools/include/*.h $(STAGING_DIR_HOST)/include/
	$(INSTALL_DATA) $(TOPDIR)/tools/include/sys/*.h $(STAGING_DIR_HOST)/include/sys/
	ln -sf lib $(STAGING_DIR_HOST)/lib64
	touch $@

endif

$(curdir)//prepare = $(STAGING_DIR)/.prepared $(STAGING_DIR_HOST)/.prepared
$(curdir)//compile = $(STAGING_DIR)/.prepared $(STAGING_DIR_HOST)/.prepared

# prerequisites for the individual targets
$(curdir)/ := .config prereq

$(curdir)/install: $(curdir)/compile

tools_enabled = $(foreach tool,$(sort $(tools-y) $(tools-)),$(if $(filter $(tool),$(tools-y)),y,n))
$(eval $(call stampfile,$(curdir),tools,compile,,_$(subst $(space),,$(tools_enabled))))
$(eval $(call subdir,$(curdir)))
