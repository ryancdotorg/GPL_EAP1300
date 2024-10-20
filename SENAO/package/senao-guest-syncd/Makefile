#
# Copyright (C) 2010-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk
-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_MAJOR_VERSION=4
PKG_MINOR_VERSION=1
PKG_RELEASE_VERSION=4

PKG_NAME:=senao-guest-syncd
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

# ifeq ($(DUMP),)
#   STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_sn_$(shell $(SH_FUNC) grep '^CONFIG_SENAO_RAWFRAME_' $(TOPDIR)/.config | md5s)
# endif

TARGET_CFLAGS += -DPKG_VERSION=\"$(PKG_VERSION).$(PKG_RELEASE)\"

define Package/senao-guest-syncd
  SECTION:=net
  CATEGORY:=SENAO
  SUBMENU:=Captive Portals
  TITLE:=senao-guest-syncd
  DEPENDS:=+libopenssl
  PROPRIETARY=y
endef

define Package/senao-guest-syncd/conffiles
/etc/config/guestsyncd
endef

define Package/senao-guest-syncd/description
 for senao-coova-chilli sync data.
endef

#Useless. It causes error in openwrt 18.06
#define Package/senao-guest-syncd/config
#  source "$(SOURCE)/Config.in"
#endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)
endef

define Package/senao-guest-syncd/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d $(1)/etc/config $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/guest_syncd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/guest_syncli $(1)/usr/sbin/
ifeq ($(CONFIG_PACKAGE_procd),y)
	$(INSTALL_BIN) ./files/guestsyncd.procd.init $(1)/etc/init.d/guestsyncd
else
	$(INSTALL_BIN) ./files/guestsyncd.init $(1)/etc/init.d/guestsyncd
endif
	$(INSTALL_DATA) ./files/guestsyncd.config $(1)/etc/config/guestsyncd
	$(INSTALL_DATA) ./files/guestsyncd.defaults $(1)/etc/uci-defaults/guestsyncd
endef

# CONFIG_PACKAGE_STRING_LIST=$(shell awk -F'=' '/^CONFIG_WSDISCOVERY_/{print $$1}' $(TOPDIR)/.config)

# EXTRA_CFLAGS += $(foreach n, $(CONFIG_PACKAGE_STRING_LIST), -D$(patsubst CONFIG_WSDISCOVERY_%,%,$(n))=\\\"$($(n))\\\")

$(eval $(call BuildPackage,senao-guest-syncd))
