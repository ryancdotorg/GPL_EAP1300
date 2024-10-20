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
PKG_RELEASE_VERSION=1

PKG_NAME:=senao-app-client
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

ifeq ($(DUMP),)
  PKG_DEPEND_CONFIGS=$(shell grep '^CONFIG_APP_AGENT_' $(SN_CONFIGS_DIR)/gconfig.mk)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_sn_$(shell $(SH_FUNC) echo $(PKG_DEPEND_CONFIGS) | md5s)
endif

EXTRA_CFLAGS+=-I$(SN_CONFIGS_DIR)

define Package/senao-app-client
	SECTION=utils
	CATEGORY=SENAO
	SUBMENU:=appagent
	TITLE:=Senao APP client
	PROPRIETARY=y
	DEPENDS:=+libopenssl
endef

define Package/senao-app-client/description
	Senao APP client
endef

ifndef BUILD_DIR_DEV
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)
endef
endif

define Package/senao-app-client/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app_client $(1)/usr/sbin/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app_client $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,senao-app-client))
