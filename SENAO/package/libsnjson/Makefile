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
PKG_MINOR_VERSION=5
PKG_RELEASE_VERSION=1

PKG_NAME:=libsnjson
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

CONFIG_PACKAGE_LIST=\
	CONFIG_APP_AGENT_SUPPORT_MESH_JSON \
	CONFIG_APP_AGENT_SUPPORT_ENGENIUS \
	CONFIG_APP_AGENT_SUPPORT_IPCAM

EXTRA_CFLAGS += $(foreach n, $(CONFIG_PACKAGE_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_APP_AGENT_SUPPORT_%,HAS_%=1,$(n))))

CONFIG_FUNC_LIST=\
	CONFIG_SUPPORT_WLAN_EXTENSION_CHANNEL \
	CONFIG_SUPPORT_FINGER_PRINT\
	CONFIG_SUPPORT_CONFIG_HAS_SECTIONNAME\
	CONFIG_SUPPORT_JAPAN_BANDWIDTH_HT80\
	CONFIG_SUPPORT_RSSI_MINIMUM_NEGATIVE90\
	CONFIG_SUPPORT_TXPOWER_PERCENTAGE_SETTING\
	CONFIG_SUPPORT_IPV6_SETTING\
	CONFIG_SUPPORT_WIFI_CERTIFIED\
	CONFIG_SUPPORT_ATKK_FW_VERSION\
	CONFIG_SUPPORT_FULL_FW_VERSION\
	CONFIG_SUPPORT_PEOPLE_FUNCTION\
	CONFIG_SUPPORT_FAST_ROAMING_PER_SSID\
	CONFIG_SUPPORT_REPEATER_SETTING\
	CONFIG_SUPPORT_FIREWALL_URL_CATEGORY\
	CONFIG_SUPPORT_LONG_WLANCONFIG_CHANNEL\
	CONFIG_SUPPORT_WLAN_24G_SETTING\
	CONFIG_SUPPORT_WLAN_5G_SETTING\
	CONFIG_SUPPORT_WLAN_5G_2_SETTING\
	CONFIG_SUPPORT_WAN_SETTING\
	CONFIG_SUPPORT_DHCP6S_SETTING\
	CONFIG_SUPPORT_DHCP6C_SETTING\
	CONFIG_SUPPORT_CHANNEL_CONFIG_SETTING\
	CONFIG_SUPPORT_MULTI_OPMODE\
	CONFIG_SUPPORT_MULTI_LOGIN\
	CONFIG_SUPPORT_NETGEAR_FUNCTION\
	CONFIG_SUPPORT_SWOS_FUNCTION\
	CONFIG_SUPPORT_SYSTEM_LOG\
	CONFIG_SUPPORT_LXC_EZMASTER_ACCOUNT_SETTING\
	CONFIG_SUPPORT_SCANNING_RADIO_MODE\
	CONFIG_SUPPORT_WATCHGUARD_FUNCTION\
	CONFIG_SUPPORT_VPN_FUNCTION

EXTRA_CFLAGS += $(foreach n, $(CONFIG_FUNC_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_SUPPORT_%,SUPPORT_%=1,$(n))))

CONFIG_WEB_FUNC_LIST=\
 	CONFIG_WEB_SUPPORT_COMBINED_SSID_SETTING\
 	CONFIG_WEB_SUPPORT_LED_ONLY_STATUS\
 	CONFIG_WEB_SUPPORT_ENJET\
	CONFIG_WEB_SUPPORT_AP_RP_SETUP_WIZARD\
	CONFIG_WEB_HWMODE_AX\
	CONFIG_WEB_SUPPORT_WPA3\
	CONFIG_WEB_SUPPORT_RP_SSID_SETTING


EXTRA_CFLAGS += $(foreach n, $(CONFIG_WEB_FUNC_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_WEB_SUPPORT_%,SUPPORT_%,$(n))))
EXTRA_CFLAGS += $(foreach n, $(CONFIG_WEB_FUNC_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_WEB_%,%,$(n))))

ifeq ($(DUMP),)
PKG_DEPEND_LIST=$(CONFIG_PACKAGE_LIST) $(CONFIG_FUNC_LIST)
PKG_DEPEND_CONFIGS=$(shell grep "$(subst $(space),=\|,$(patsubst %,^%,$(call qstrip,$(PKG_DEPEND_LIST))))" $(TOPDIR)/.config)
STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_sn_$(shell $(SH_FUNC) echo $(PKG_DEPEND_CONFIGS) | md5s)
endif

#ifeq ($(CONFIG_PACKAGE_senao-systime),y)
#EXTRA_CFLAGS+=-DSUPPORT_SYSTIME=1
#endif
#
#ifeq ($(CONFIG_ROUTER_SUPPORT_IPCAM),y)
#EXTRA_CFLAGS+=-DROUTER_SUPPORT_IPCAM=1
#endif
#
#ifeq ($(CONFIG_PACKAGE_lighttpd),y)
#EXTRA_CFLAGS += -DHAS_LIGHTTPD_SERVER=1
#endif
#
#ifeq ($(CONFIG_PACKAGE_easyupload),y)
#EXTRA_CFLAGS += -DHAS_EASYUPLOAD=1
#endif
#
#ifeq ($(CONFIG_PACKAGE_senao-nat-traversal),y)
#EXTRA_CFLAGS += -DHAS_NAT_TRAVERSAL=1
#endif
#
#ifeq ($(CONFIG_PACKAGE_senao-lionic-dpi),y)
#EXTRA_CFLAGS += -DHAS_LIONIC_DPI=1
#endif
#
ifeq ($(CONFIG_PACKAGE_libcloudap),y)
EXTRA_CFLAGS += -DSUPPORT_LIBCLOUDAP_SETTING=1
endif
ifeq ($(CONFIG_PACKAGE_senao-coova-chilli),y)
EXTRA_CFLAGS += -DSUPPORT_CAPTIVE_PORTAL_SETTING=1
endif
ifeq ($(CONFIG_PACKAGE_senao-linkmon),y)
EXTRA_CFLAGS += -DHAS_LINKMON=1
EXTRA_LIBS += -llmipc -lubox -lubus -lblobmsg_json
endif
ifeq ($(CONFIG_PACKAGE_syseye-act_linkcheck),y)
EXTRA_CFLAGS += -DHAS_SYSEYE=1
EXTRA_LIBS += -lseipc -lezjson -lubox -lubus -lblobmsg_json
endif
ifeq ($(CONFIG_TOOLCHAIN_LIBC), "musl")
EXTRA_CFLAGS += -DTOOLCHAIN_MUSL=1
endif
ifeq ($(CONFIG_LIBC), "musl")
EXTRA_CFLAGS += -DTOOLCHAIN_MUSL=1
endif

ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "SP938BS")
EXTRA_CFLAGS += -DSUPPORT_SP938BS_OPENAPI_SERVER=1
EXTRA_CFLAGS += -DCREATE_BSC_BIND_AUTH=1
endif

ifeq ($(CONFIG_PACKAGE_sn-uboot-2016-ipq6018-MAKEOPTS_SENAO_MODEL_NAME), "SP938BSC")
EXTRA_CFLAGS += -DSUPPORT_SP938BSC_OPENAPI_SERVER=1
endif

ifeq ($(CONFIG_PACKAGE_uboot-ipq40xx-MAKEOPTS_SENAO_MODEL_NAME), "EnBSC")
EXTRA_CFLAGS += -DSUPPORT_ENBSC_OPENAPI_SERVER=1
endif

QCA_WIFI_PKG:=$(lastword $(strip $(foreach n, $(.VARIABLES), $(if $(filter CONFIG_PACKAGE_kmod-qca-wifi-10.2-%,$(n)),$(subst CONFIG_PACKAGE_,,$(n))))))

ifeq ($(CONFIG_PACKAGE_$(QCA_WIFI_PKG)),y)
	EXTRA_CFLAGS+=-DSUPPORT_WIFI_10_2_VERSION=1
endif

EXTRA_LIBS+=-ljson-c -lfcgi -lm

MAKE_FLAGS+= \
	EXTRA_LIBS="$(EXTRA_LIBS)" \
	EXTRA_INCLUDE="-I$(SN_CONFIGS_DIR) -I$(SN_DIR)/h -I$(SN_INCLUDE_DIR) -I$(STAGING_DIR)/usr/include/json-c -I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/utility -I$(STAGING_DIR)/usr/include/variable" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	STAGING_DIR="$(STAGING_DIR)" \
	BUILD_DIR="$(BUILD_DIR)"

define Package/libsnjson
  SECTION=libs
  CATEGORY:=SENAO
  TITLE:=API Library (include lua plugin)
  PROPRIETARY=y
  DEPENDS:=+liblua +libuci +libpcre +libjson-c +fcgi +libsqlite3 +PACKAGE_senao-linkmon:senao-linkmon +PACKAGE_senao-linkmon:libubus +PACKAGE_senao-linkmon:ubox +libapi +senao-sysutil +PACKAGE_syseye-act_linkcheck:syseye +PACKAGE_syseye-act_linkcheck:ezjson +PACKAGE_syseye-act_linkcheck:libubus +PACKAGE_syseye-act_linkcheck:ubox +PACKAGE_senao-frame-server:senao-frame-server
  VARIANT:=libsnjson
endef

define Package/libsnjson/description
  API Library (include lua plugin).
endef

ifndef BUILD_DIR_DEV
  define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR);
	$(CP) src/* $(PKG_BUILD_DIR);
  endef
endif

define Package/libsnjson/install
	$(INSTALL_DIR) $(1)/lib/ $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libsnjson.so $(1)/lib/
	$(INSTALL_BIN) ./files/getLatestFWInfo.sh $(1)/sbin/getLatestFWInfo.sh
	$(INSTALL_BIN) ./files/doDownloadFW.sh $(1)/sbin/doDownloadFW.sh
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib	 $(1)/usr/include/api-json
	$(CP) $(PKG_BUILD_DIR)/libsnjson.so $(1)/usr/lib 
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/api-json
endef

$(eval $(call BuildPackage,libsnjson))
