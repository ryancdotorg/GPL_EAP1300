#
# Copyright (C) 2008-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_MAJOR_VERSION=1
PKG_MINOR_VERSION=0
PKG_RELEASE_VERSION=3

PKG_NAME:=sn-websockets-server
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
include $(INCLUDE_DIR)/package.mk

JSON_INCS += "$(STAGING_DIR)/usr/include/json-c"

LIBS+=-lwebsockets -lpthread -lapi -lfcgi -lm -ljson-c

ifeq ($(CONFIG_PACKAGE_syseye),y)
LIBS+=-lezjson -lseipc
endif

MAKE_FLAGS+= \
        SN_CONFIGS_DIR="$(SN_CONFIGS_DIR)" \
        LIBS="$(LIBS)" \
        EXTRA_INCLUDE="-I$(STAGING_DIR)/usr/include -I$(JSON_INCS) -I$(STAGING_DIR)/usr/include/api-json"

define Package/sn-websockets-server
	CATEGORY:=SENAO
	SECTION:=net
	TITLE:=Senao WebSocket Server
	PROPRIETARY=y
	DEPENDS:=+syseye +mio +libwebsockets +libapi +libpthread +libjson-c +libsnjson +ezjson
	VARIANT:=sn-websockets-server
endef

define Package/sn-websockets-server/description
	Senao WebSocket Server
endef

ifndef BUILD_DIR_DEV
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)
endef
endif

define Package/sn-websockets-server/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/share/swd $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/server $(1)/usr/bin/swd
	$(INSTALL_DATA) ./files/swd.key.pem $(1)/usr/share/swd/
	$(INSTALL_DATA) ./files/swd.pem $(1)/usr/share/swd/
ifeq ($(CONFIG_PACKAGE_procd),y)
	$(INSTALL_BIN) ./files/swd.procd.init $(1)/etc/init.d/swd
else
	$(INSTALL_BIN) ./files/swd.init $(1)/etc/init.d/swd
endif
endef

$(eval $(call BuildPackage,sn-websockets-server))
