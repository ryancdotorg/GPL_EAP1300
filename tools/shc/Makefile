#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=shc
PKG_VERSION:=3.9.6

# https://github.com/neurobin/shc.git
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

HOST_BUILD_PARALLEL:=1
HOST_CONFIGURE_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_VARS :=

HOST_CONFIGURE_ARGS := \
	--prefix=$(STAGING_DIR_HOST)

### SENAO ###
define SNCopySrc
	@echo "Preparing Source Directory :"
	@[ -e $(HOST_BUILD_DIR) ] && rm -rf $(HOST_BUILD_DIR)
	@mkdir -p $(HOST_BUILD_DIR)
	-@[ -n "$(1)" ] || ($(call SNCopy,"./src"); echo "./src -> $(HOST_BUILD_DIR)")
	-@[ -n "$(1)" ] && ($(call SNCopy,"./src/$(1)"); echo "./src/$(1) -> $(HOST_BUILD_DIR)")
endef

define SNCopy
    $(TAR) c -C $(1) . \
        --exclude=.pc --exclude=.svn --exclude=.git \
        --exclude='boa-0*' --exclude='*.o' --exclude='*.so' | \
            tar x -C $(HOST_BUILD_DIR)/
endef

define Host/Prepare
  $(call SNCopySrc)
  $(call Host/Prepare/Default)
endef

define Host/Configure
  $(call Host/Configure/Default)
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/src/shc $(STAGING_DIR_HOST)/bin/
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/shc
endef

$(eval $(call HostBuild))
