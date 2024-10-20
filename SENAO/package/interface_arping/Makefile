include $(TOPDIR)/rules.mk

PKG_NAME:=interface_arping
PKG_RELEASE:=1
PKG_VERSION:=1.1.1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/interface_arping
	SECTION:=utils
	CATEGORY:=SENAO
	TITLE:=interface_arping
	PROPRIETARY=y
endef

ifeq ($(CONFIG_TOOLCHAIN_LIBC), "musl")
MAKE_FLAGS += EXTRA_CFLAGS="-DTOOLCHAIN_MUSL=1"
endif

ifeq ($(CONFIG_LIBC), "musl")
MAKE_FLAGS += EXTRA_CFLAGS="-DTOOLCHAIN_MUSL=1"
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(call Build/Patch/Default)
endef


define Package/interface_arping/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/interface_arping $(1)/bin/
	@echo
endef

$(eval $(call BuildPackage,interface_arping))
