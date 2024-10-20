include $(TOPDIR)/rules.mk
-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_MAJOR_VERSION=1
PKG_MINOR_VERSION=3
PKG_RELEASE_VERSION=2

PKG_NAME:=senao-syskey
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/senao-syskey
	SECTION=net
	CATEGORY:=SENAO
	TITLE:=SENAO system control key.
	PROPRIETARY=y
endef

define Package/senao-syskey/description
	SENAO system control key.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile                                                                                                                                                        
endef

define Package/senao-syskey/install
	$(INSTALL_DIR) $(1)/lib $(1)/etc/init.d $(1)/etc/config $(1)/etc/uci-defaults $(1)/usr/sbin
	$(INSTALL_BIN) ./files/etc/config/syskey.conf $(1)/etc/config/syskey
	$(INSTALL_BIN) ./files/etc/init.d/syskey.init $(1)/etc/init.d/syskey
	$(INSTALL_BIN) ./files/lib/syskey.sh $(1)/lib/syskey.sh
	$(INSTALL_BIN) ./files/etc/uci-defaults/syskey.default $(1)/etc/uci-defaults/z99syskey
	$(INSTALL_BIN) ./files/usr/sbin/iface_mark.sh $(1)/usr/sbin/iface_mark.sh
	$(INSTALL_BIN) ./files/usr/sbin/iface_mark_boot.sh $(1)/etc/init.d/iface_mark
endef

$(eval $(call BuildPackage,senao-syskey))
