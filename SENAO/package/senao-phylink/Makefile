#
#
# Copyright (C) 2010-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk
-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_MAJOR_VERSION=1
PKG_MINOR_VERSION=2
PKG_RELEASE_VERSION=1

PKG_NAME:=senao-phylink
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))

define KernelPackage/senao-phylink
  SUBMENU:=Senao Networks Modules
  TITLE:=Physical Link Reporter from ethernet driver
  PROPRIETARY=y
  KCONFIG:=
  AUTOLOAD:=$(call AutoLoad,10,phylink_module)
  FILES:=$(PKG_BUILD_DIR)/phylink_module.$(LINUX_KMOD_SUFFIX)
  DEPENDS:=$(PKG_DEFAULT_DEPENDS)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -R ./src/* $(PKG_BUILD_DIR)/
endef

#for debug use
EXTRA_CFLAGS +=-DDEBUG=1 \
		-I$(PKG_BUILD_DIR) 

MAKE_FLAGS += KERNEL_DIR="$(LINUX_DIR)"

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
	$(MAKE_OPTS) \
	modules
endef

define KernelPackage/senao-phylink/install
endef

$(eval $(call KernelPackage,senao-phylink))
