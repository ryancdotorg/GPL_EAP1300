include $(TOPDIR)/rules.mk

PKG_NAME:=wifiParser
PKG_RELEASE:=1
PKG_VERSION:=1.1.1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/wifiParser
	SECTION:=utils
	CATEGORY:=Utilities
	DEFAULT:=y
	TITLE:=wifiParser
	PROPRIETARY=y
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(call Build/Patch/Default)
endef


define Package/wifiParser/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wifiParser $(1)/bin/
	@echo
endef

$(eval $(call BuildPackage,wifiParser))
