include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=ifplugd
PKG_VERSION:=0.28
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

# This line will trigger the download process.
PKG_SOURCE_URL:=ftp://rdserver/SmartOpenWrt/firmware/sources

include $(INCLUDE_DIR)/package.mk

define Package/ifplugd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Network Interface Plug Detection Daemon
  URL:=http://0pointer.de/lennart/projects/ifplugd/
  DEPENDS:=+libdaemon
endef

CONFIGURE_ARGS+= \
	--disable-lynx \
	--disable-xmltoman \
	--disable-subversion \

define Package/ifplugd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ifplugd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/ifplugd
ifeq ("$(wildcard ./sn_files/ifplugd.action)","")
	$(INSTALL_BIN) ./files/ifplugd.action $(1)/etc/ifplugd/ifplugd.action
else
	$(call SENAO_SCRIPT_INSTALL,./sn_files/ifplugd.action,$(1)/etc/ifplugd,ifplugd.action)
endif
	$(INSTALL_DIR) $(1)/etc/init.d
ifeq ("$(wildcard ./sn_files/ifplugd.shc)","")
	$(INSTALL_BIN) ./sn_files/ifplugd.init $(1)/etc/init.d/ifplugd || $(INSTALL_BIN) ./files/ifplugd.init $(1)/etc/init.d/ifplugd
else
	$(INSTALL_BIN) ./sn_files/ifplugd.init $(1)/etc/init.d/ifplugd
	$(INSTALL_DIR) $(1)/usr/shc
	$(call SENAO_SCRIPT_INSTALL,./sn_files/ifplugd.shc,$(1)/usr/shc,ifplugd)
endif

endef

$(eval $(call BuildPackage,ifplugd))
