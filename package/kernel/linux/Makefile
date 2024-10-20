#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=kernel
PKG_FLAGS:=hold

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/packages

ifeq ($(DUMP),)
  -include $(LINUX_DIR)/.config
  _PLATFORM_SUBDIR:=$(PLATFORM_SUBDIR)
$(warning [$(_PLATFORM_SUBDIR)])
else
  -include $(TOPDIR)/tmp/.sn_ksrc
$(warning [$(_PLATFORM_SUBDIR)])
  target_conf=$(call qstrip,$(subst .,_,$(subst -,_,$(subst /,_,$(1)))))
  _PLATFORM_DIR:=$(TOPDIR)/target/linux/$(call qstrip,$(CONFIG_TARGET_BOARD))
  _SUBTARGET:=$(strip $(foreach subdir,$(patsubst $(_PLATFORM_DIR)/%/target.mk,%,$(wildcard $(_PLATFORM_DIR)/*/target.mk)),$(if $(CONFIG_TARGET_$(call target_conf,$(CONFIG_TARGET_BOARD)_$(subdir))),$(subdir))))
  _PLATFORM_SUBDIR:=$(_PLATFORM_DIR)$(if $(_SUBTARGET),/$(_SUBTARGET))
endif

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_8_0_CS qca_spf_8_0_CSU1))
SCAN_DEPS=modules_qca_spf_8_0_CS/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter-4.4.mk
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_10_0_CS_PREVIEW qca_spf_10_0_CS qca_spf_10_0_CS_PATCH1 qca_spf_10_0_CS_PATCH2 \
	qca_spf_10_0_CS_PATCH5 qca_ipq6018_ilq12.0_es1))
SCAN_DEPS=modules_qca_spf_10_0_CS_PREVIEW/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter-4.4.mk
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_10_0_ES2 qca_spf_10_0_FC))
SCAN_DEPS=modules_qca_spf_10_0_ES2/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter-4.4.mk
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_9_0_rel4 qca_spf_9_0_CS))
SCAN_DEPS=modules_qca_spf_9_0_REL4/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter-4.4.mk
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_9_0_ES qca_spf_9_0_rel3))
SCAN_DEPS=modules_qca_spf_9_0_ES/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter-4.4.mk
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_6_0_CSU1 qca_spf_6_0_CSU1_NHSS_6_1))
SCAN_DEPS=modules_qca_spf_6_0_CSU1/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter-4.4.mk
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_6_0_CSU1_NHSS_MIPS_6_0))
SCAN_DEPS=modules_qca_spf_6_0_CSU1_NHSS_MIPS_6_0/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter-4.4.mk
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_6_1_0_CS))
SCAN_DEPS=modules_qca_spf_6_1_0_CS/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter-4.4.mk
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_ipq6018_ilq11.0_fc qca_spf_11_0_csu1 qca_spf_11_0_csu3))
SCAN_DEPS=modules_qca_ipq6018_ilq11.0_fc/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter-4.4.mk
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_11_1_csu2))
SCAN_DEPS=modules_qca_spf_11_1_csu2/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter-4.4.mk
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_11_2))
SCAN_DEPS=modules_qca_spf_11_2/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter-4.4.mk
else ifeq ($(CONFIG_LINUX_3_2),y)
SCAN_DEPS=modules/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter-3.2.mk
else ifneq ($(wildcard $(call qstrip,$(_PLATFORM_SUBDIR)/openwrt_modules)),)
SCAN_DEPS=$(call qstrip,$(_PLATFORM_SUBDIR)/openwrt_modules)/*.mk $(TOPDIR)/target/linux/*/modules.mk $(call qstrip,$(_PLATFORM_SUBDIR)/openwrt_include)/netfilter.mk
else
SCAN_DEPS=modules/*.mk $(TOPDIR)/target/linux/*/modules.mk $(TOPDIR)/include/netfilter-4.4.mk
endif

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

export SHELL:=/bin/sh
.ONESHELL:

include $(INCLUDE_DIR)/package.mk

STAMP_BUILT:=$(STAMP_BUILT)_$(firstword $(shell $(SCRIPT_DIR)/kconfig.pl $(LINUX_DIR)/.config | md5sum))

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define KernelPackage/depends
endef

CONFIG_PACKAGE_kernel=y
define Package/kernel
  SECTION:=sys
  CATEGORY:=Kernel
  DEFAULT:=y
  TITLE:=Virtual kernel package
  VERSION:=$(LINUX_VERSION)-$(LINUX_RELEASE)-$(LINUX_VERMAGIC)
  URL:=http://www.kernel.org/
endef

define Package/kernel/install
  # nothing to do
endef

define Package/kernel/extra_provides
	sed -e 's,.*/,,' $(LINUX_DIR)/modules.builtin;
endef

$(eval $(if $(DUMP),,$(call BuildPackage,kernel)))
ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_8_0_CS qca_spf_8_0_CSU1))
include $(sort $(wildcard ./modules_qca_spf_8_0_CS/*.mk))
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_10_0_CS_PREVIEW qca_spf_10_0_CS qca_spf_10_0_CS_PATCH1 qca_spf_10_0_CS_PATCH2 \
	qca_spf_10_0_CS_PATCH5 qca_ipq6018_ilq12.0_es1))
include $(sort $(wildcard ./modules_qca_spf_10_0_CS_PREVIEW/*.mk))
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_10_0_ES2 qca_spf_10_0_FC))
include $(sort $(wildcard ./modules_qca_spf_10_0_ES2/*.mk))
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_9_0_rel4 qca_spf_9_0_CS))
include $(sort $(wildcard ./modules_qca_spf_9_0_REL4/*.mk))
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_9_0_ES qca_spf_9_0_rel3))
include $(sort $(wildcard ./modules_qca_spf_9_0_ES/*.mk))
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_6_0_CSU1_NHSS_MIPS_6_0))
include $(sort $(wildcard ./modules_qca_spf_6_0_CSU1_NHSS_MIPS_6_0/*.mk))
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_6_1_0_CS))
include $(sort $(wildcard ./modules_qca_spf_6_1_0_CS/*.mk))
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_ipq6018_ilq11.0_fc qca_spf_11_0_csu1 qca_spf_11_0_csu3))
include $(sort $(wildcard ./modules_qca_ipq6018_ilq11.0_fc/*.mk))
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_11_1_csu2))
include $(sort $(wildcard ./modules_qca_spf_11_1_csu2/*.mk))
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_11_2))
include $(sort $(wildcard ./modules_qca_spf_11_2/*.mk))
else ifneq ($(wildcard $(call qstrip,$(_PLATFORM_SUBDIR)/openwrt_modules)),)
include $(sort $(wildcard $(_PLATFORM_SUBDIR)/openwrt_modules/*.mk))
else
include $(sort $(wildcard ./modules/*.mk))
endif
-include $(TOPDIR)/target/linux/*/modules.mk
