#
# Copyright (C) 2010-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_NAME:=senao-linkmon
PKG_VERSION:=1.0
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,2),2)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

ifeq ($(CONFIG_TARGET_BOARD),"ipq40xx")
EXTRA_CFLAGS += -DTARGET_BOARD_ipq40xx
endif

ifeq ($(CONFIG_PACKAGE_libopenssl),y)
#EXTRA_CFLAGS += -DOPENSSL_DEV
#EXTRA_LDFLAGS += -lssl -lcrypto
endif

ifeq ($(CONFIG_TOOLCHAIN_LIBC), "musl")
EXTRA_CFLAGS += -DTOOLCHAIN_MUSL=1
endif

ifeq ($(CONFIG_LIBC), "musl")
EXTRA_CFLAGS += -DTOOLCHAIN_MUSL=1
endif

MAKE_FLAGS += \
JSON_C_DIR="$(STAGING_DIR)/usr/" \
LIBLMIPC_DIR="./" \
EXTRA_CFLAGS="$(EXTRA_CFLAGS)"

define Package/senao-linkmon
  SECTION=libs
  CATEGORY:=SENAO
  TITLE:=senao-linkmon
  DEPENDS:=+libjson-c #+libopenssl
  PROPRIETARY=y
endef

define Package/senao-linkmon/description
 link status monitor.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)
endef

define Package/senao-linkmon/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d $(1)/etc/config $(1)/usr/lib $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/linkmon $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lmipc $(1)/usr/bin/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/liblmipc.so* $(1)/usr/lib
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/liblmipc.so* $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/liblmipc.h $(1)/usr/include/
endef

$(eval $(call BuildPackage,senao-linkmon))
