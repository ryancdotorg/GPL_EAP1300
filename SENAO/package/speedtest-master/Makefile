#
# Source from https://github.com/YuLinLee/speedtest
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk
-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_MAJOR_VERSION=1
PKG_MINOR_VERSION=0
PKG_RELEASE_VERSION=1

PKG_NAME:=speedtest-master
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))



PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk


define Package/speedtest-master
  CATEGORY:=SENAO
  SUBMENU:=bandwidth applications
  TITLE:=speedtest-master
  PROPRIETARY=y
  DEPENDS:=+libpthread +libcurl +@LIBCURL_CRYPTO_AUTH +@LIBCURL_TLS_SRP +@LIBCURL_ZLIB +@LIBCURL_NTLM
endef

define Package/speedtest-master/description
	Speed test in C code.
endef

#Useless. It causes error in openwrt 18.06
#define Package/speedtest-master/config
#  source "$(SOURCE)/Config.in"
#endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)
endef

define Package/speedtest-master/install
	$(INSTALL_DIR) $(1)/sbin $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/sbin/
	$(INSTALL_BIN) ./files/speedtest.init $(1)/etc/init.d/speedtest
endef

$(eval $(call BuildPackage,speedtest-master))
