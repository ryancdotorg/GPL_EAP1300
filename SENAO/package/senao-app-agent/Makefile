#
# Copyright (C) 2008-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk
-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_MAJOR_VERSION=1
PKG_MINOR_VERSION=0
PKG_RELEASE_VERSION=2

PKG_NAME:=senao-app-agent
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

JSON_BUILD_DIR:=$(BUILD_DIR)/json-c-0.11
JSON_LIBS:=$(JSON_BUILD_DIR)/ipkg-install/usr/lib
JSON_INCS:=$(JSON_BUILD_DIR)/ipkg-install/usr/include/json

ifeq ($(CONFIG_USE_LIB_MATRIXSSL),y)
SSL_BUILD_DIR:=$(BUILD_DIR)/matrixssl
SSL_LIBS:=$(SSL_BUILD_DIR)/
else
SSL_BUILD_DIR:=$(BUILD_DIR)/openssl
SSL_LIBS:=$(SSL_BUILD_DIR)/
endif

STAGING_LIBS_DIR:=$(STAGING_DIR)/usr/lib/

include $(INCLUDE_DIR)/package.mk

EXTRA_CFLAGS += -D_GNU_SOURCE

ifeq ($(DUMP),)
  PKG_DEPEND_CONFIGS=$(shell grep '^CONFIG_APP_AGENT_SUPPORT_' $(TOPDIR)/.config)
  PKG_DEPEND_CONFIGS+=$(shell grep '^CONFIG_APP_AGENT_' $(SN_CONFIGS_DIR)/gconfig.mk)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_sn_$(shell $(SH_FUNC) echo $(PKG_DEPEND_CONFIGS) | md5s)
endif

define Package/senao-app-agent/config
  source "$(SOURCE)/Config.in"
endef

CONFIG_PACKAGE_LIST=$(shell awk -F'=' '/^CONFIG_APP_AGENT_SUPPORT_/{print $$1}' $(TOPDIR)/.config)
EXTRA_CFLAGS += $(foreach n, $(CONFIG_PACKAGE_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_APP_AGENT_SUPPORT_%,HAS_%=1,$(n))))

ifeq ($(CONFIG_PACKAGE_alfred),y)
EXTRA_CFLAGS += -DHAS_ALFRED=1
endif

ifeq ($(CONFIG_PACKAGE_FOR_SC),y)
EXTRA_CFLAGS += -DFOR_SC=1
endif

ifeq ($(CONFIG_PACKAGE_speedtest),y)
EXTRA_CFLAGS += -DHAS_SPEEDTEST_THROUGHPUT_TEST=1
endif

ifeq ($(CONFIG_PACKAGE_lighttpd),y)
EXTRA_CFLAGS += -DHAS_LIGHTTPD_SERVER=1
endif

ifeq ($(CONFIG_PACKAGE_speedtest-master),y)
EXTRA_CFLAGS += -DHAS_SPEEDTEST_THROUGHPUT_TEST=1
endif

ifeq ($(CONFIG_PACKAGE_senao-guest-syncd),y)
EXTRA_CFLAGS += -DHAS_SUPPORT_GUEST_NETWORK_BY_SSID=1
endif

MAKEOPTS:= \
	$(TARGET_CONFIGURE_OPTS) \
	COPTS="-I$(PKG_BUILD_DIR) $(TARGET_CFLAGS) -I$(SN_CONFIGS_DIR)" \
	CPPFLAGS="-I$(PKG_BUILD_DIR) $(TARGET_CPPFLAGS)" \
	FPIC="$(FPIC)" \
	STAGING_DIR="$(STAGING_DIR)" \
	BUILD_DIR="$(BUILD_DIR)" \
	JSON_LIBS="$(JSON_LIBS)" \
	SSL_LIBS="$(SSL_LIBS)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	EXTRA_INCLUDE="-I$(JSON_INCS)" \
	EXTRA_LIBS="-ljson-c -L$(JSON_LIBS) -L$(STAGING_LIBS_DIR) -lmatrixssl -lssl -lcrypto -L$(SSL_LIBS)"

define Package/FOR_SC
  SECTION=libs
  CATEGORY=SENAO
  TITLE:=Senao APP agent
  PROPRIETARY=y
  DEPENDS:=+libjson-c +libmatrixssl +libopenssl +libapi +senao-sysutil
  VARIANT:=senao-app-agent
  SUBMENU:=appagent
  MENU:=1
endef

define Package/senao-app-agent
  SECTION=libs
  CATEGORY=SENAO
  TITLE:=Senao APP agent
  PROPRIETARY=y
  DEPENDS:=+libjson-c +libmatrixssl +libopenssl +libapi +senao-sysutil
  VARIANT:=senao-app-agent
  SUBMENU:=appagent
  MENU:=1
endef

define Package/senao-app-agent/description
  Senao APP agent
endef

ifndef BUILD_DIR_DEV
  define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)
  endef
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(MAKEOPTS)
endef

define Package/senao-app-agent/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/usr/sbin $(1)/etc/config $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app_agentd $(1)/usr/sbin/
	$(INSTALL_DATA) ./sn_files/agent.pem $(1)/etc/ || $(INSTALL_BIN) ./files/agent.pem $(1)/etc/
	$(INSTALL_DATA) ./sn_files/app_agent.config $(1)/etc/config/app_agent || $(INSTALL_BIN) ./files/app_agent.config $(1)/etc/config/app_agent
	$(INSTALL_BIN) ./sn_files/app_agent.init $(1)/etc/init.d/app_agent || $(INSTALL_BIN) ./files/app_agent.init $(1)/etc/init.d/app_agent
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/usr/sbin $(1)/etc/config $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app_agentd $(1)/usr/sbin/
	$(INSTALL_DATA) ./sn_files/app_agent.config $(1)/etc/config/app_agent || $(INSTALL_BIN) ./files/app_agent.config $(1)/etc/config/app_agent
	$(INSTALL_BIN) ./sn_files/app_agent.init $(1)/etc/init.d/app_agent || $(INSTALL_BIN) ./files/app_agent.init $(1)/etc/init.d/app_agent
endef

$(eval $(call BuildPackage,senao-app-agent))
$(eval $(call BuildPackage,FOR_SC))
