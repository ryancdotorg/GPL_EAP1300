#
# Copyright (C) 2008-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_NAME:=mio
PKG_VERSION:=1.0
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,1),1)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

MAKE_FLAGS +=

define Package/mio
  SECTION=libs
  CATEGORY=Libraries
  TITLE:=mio
  PROPRIETARY=y
endef

define Package/mio/description
  mio Library
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR);
	$(CP) src/* $(PKG_BUILD_DIR);
endef

define Package/mio/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libmio.so* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/list.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/mio.h $(1)/usr/include/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libmio.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,mio))
