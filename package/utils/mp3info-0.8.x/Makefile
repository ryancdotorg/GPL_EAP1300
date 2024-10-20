include $(TOPDIR)/rules.mk

PKG_NAME:=mp3info
PKG_VERSION:=0.8
PKG_RELEASE:=5

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/mp3info
  TITLE:=Embedded mp3info
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libncurses
endef


define Build/Compile
 $(call Build/Compile/Default,mp3info)
endef

define Package/mp3info/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mp3info $(1)/sbin/
endef

$(eval $(call BuildPackage,mp3info))
