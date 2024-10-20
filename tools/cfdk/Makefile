include $(TOPDIR)/rules.mk

PKG_NAME:=cfdk

include $(INCLUDE_DIR)/host-build.mk

WEB_DEF_CONFIG_FILE=$(TOPDIR)/SENAO/configs/webDefConfig.h
WEB_CONFIG_FILE=$(TOPDIR)/SENAO/configs/webConfig.h
ifneq ("$(wildcard $(WEB_DEF_CONFIG_FILE))","")
HOST_MAKE_FLAGS+=WEB_DEF_CONFIG_FILE=$(WEB_DEF_CONFIG_FILE)
CFDK_MD5=$(shell $(SH_FUNC) find $(realpath $(WEB_DEF_CONFIG_FILE)) -print0|xargs -0 stat -c %Y%n)
ifneq ("$(wildcard $(WEB_CONFIG_FILE))","")
HOST_MAKE_FLAGS+=WEB_CONFIG_FILE=$(WEB_CONFIG_FILE)
CFDK_MD5+=$(shell $(SH_FUNC) find $(realpath $(WEB_CONFIG_FILE)) -print0|xargs -0 stat -c %Y%n)
endif
endif
ifeq ($(DUMP),)
  HOST_STAMP_CONFIGURED:=$(strip $(HOST_STAMP_CONFIGURED))_sn_$(shell $(SH_FUNC) echo $(CFDK_MD5) | md5s)
endif

### SENAO ###
define SNCopySrc
	@echo "Preparing Source Directory :"
	@[ -e $(HOST_BUILD_DIR) ] && rm -rf $(HOST_BUILD_DIR)
	@mkdir -p $(HOST_BUILD_DIR)
	cp -a src/* $(HOST_BUILD_DIR)
endef

define Host/Prepare
  $(call SNCopySrc)
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/cfdk $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/cfdk
endef

$(eval $(call HostBuild))
