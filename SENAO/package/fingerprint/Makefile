include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk
-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_MAJOR_VERSION=2
PKG_MINOR_VERSION=0
PKG_RELEASE_VERSION=12

PKG_NAME:=fingerprint
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
#include $(INCLUDE_DIR)/develop.mk

ifeq ($(CONFIG_TOOLCHAIN_LIBC),"musl")
MAKE_FLAGS += EXTRA_CFLAGS="-DTOOLCHAIN_MUSL=1"
endif

ifeq ($(CONFIG_LIBC),"musl")
MAKE_FLAGS += EXTRA_CFLAGS="-DTOOLCHAIN_MUSL=1"
endif

define Package/fingerprint
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=fingerprint
	DEPENDS:=+libopenssl +arp-scan
	PROPRIETARY=y
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(call Build/Patch/Default)
endef

#define Build/Compile
#   $(MAKE) -C $(PKG_BUILD_DIR) $(MAKEOPTS)
#endef

define Package/fingerprint/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/finger_syncli $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d $(1)/etc/uci-defaults/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fingerprint $(1)/usr/bin/
	$(INSTALL_BIN) ./files/fingerprint_dump.sh $(1)/usr/bin/
	$(INSTALL_BIN) ./files/arp_scan.sh $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/assoc_notify.d/
	$(INSTALL_BIN) ./files/fingerprint.notify.sh $(1)/etc/assoc_notify.d/fingerprint_notify.sh
ifeq ($(CONFIG_PACKAGE_procd),y)
	$(INSTALL_BIN) ./files/fingerprint.procd.init $(1)/etc/init.d/fingerprint
else
	$(INSTALL_BIN) ./files/fingerprint.init $(1)/etc/init.d/fingerprint
endif
	$(INSTALL_BIN) ./files/fingerprint.defaults $(1)/etc/uci-defaults/fingerprint
	$(INSTALL_BIN) ./files/cpu_rps.init $(1)/etc/init.d/cpu_rps
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/wifi_list.sh $(1)/usr/sbin/wifi_list
endef

$(eval $(call BuildPackage,fingerprint))
