#
# Copyright (C) 2008-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk
-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_MAJOR_VERSION=1
PKG_MINOR_VERSION=0
PKG_RELEASE_VERSION=3

PKG_NAME:=ezjson
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

MAKE_FLAGS +=

define Package/ezjson
  SECTION=libs
  CATEGORY=SENAO
  TITLE:=ezjson
  PROPRIETARY=y
endef

define Package/ezjson/description
  EZJson Library
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR);
	$(CP) src/* $(PKG_BUILD_DIR);
endef

define Package/ezjson/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libezjson.so* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/ezjson.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/json.h $(1)/usr/include/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libezjson.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,ezjson))
