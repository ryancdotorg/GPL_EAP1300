# remember the provided package version
PKG_VERSION_ORGINAL:=$(PKG_VERSION)

# in case that another version is provided, overwrite the original
ifeq ($(CONFIG_$(PKG_NAME)_USE_CUSTOM_VERSION),y)
PKG_VERSION:=$(call qstrip,$(CONFIG_$(PKG_NAME)_CUSTOM_VERSION))
PKG_SOURCE:=$(subst $(PKG_VERSION_ORGINAL),$(PKG_VERSION),$(PKG_SOURCE))
PKG_MD5SUM:=
endif

define Package/$(PKG_NAME)/config
   $(call Package/$(PKG_NAME)/override_version)
endef

include Config.in

newver:
	sh $(SN_SCRIPTS_DIR)/new_version.sh $(CURDIR)

