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
PKG_MINOR_VERSION=9
PKG_RELEASE_VERSION=51

PKG_NAME:=senao_capwap_c1.9.x
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))


#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=@SF/capwap
#PKG_MD5SUM:=9ec53447b6d6fe0516c811fb48160b85
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(PKG_NAME)-${SUBTARGET}/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
#include $(INCLUDE_DIR)/develop.mk

PRODUCT_NAME:=$(shell cat $(TOPDIR)/SENAO/configs/gconfig.mk | grep "CONFIG_MODEL_NAME=" | cut -d '=' -f 2 | sed -n 's/"//gp')
VERSION_TARGET:=$(shell ( grep "CW_VERSION_$(PRODUCT_NAME)" ./src/CWVersion.h || grep CW_VERSION_UNDEF_MODEL ./src/CWVersion.h ) | awk -F\  '{print $$3}')
MODEL_CFLAGS:=-DCW_BOARD_${PRODUCT_NAME}
# Determine the vendor type 
ifeq ($(findstring ATMWS,$(PRODUCT_NAME)),ATMWS)
MODEL_CFLAGS += -DCW_VENDOR_ID=207 -DCW_CERT_PASSWORD=\"CwDefPwd\" -DCW_DEF_EZCOM_URL=\"\"
MODEL_VENDOR:=ati
else ifeq ($(findstring WAP,$(PRODUCT_NAME)),WAP)
MODEL_CFLAGS += -DCW_VENDOR_ID=44292 -DCW_CERT_PASSWORD=\"CwDefPwd\" -DCW_DEF_EZCOM_URL=\"ezreg.emplustech.com\"
MODEL_VENDOR:=emplus
else
MODEL_CFLAGS += -DCW_VENDOR_ID=14125 -DCW_CERT_PASSWORD=\"EnGenius520\" -DCW_DEF_EZCOM_URL=\"ezreg.engeniusnetworks.com\"
MODEL_VENDOR:=senao
endif

CONFIG_PACKAGE_LIST=\
 CONFIG_SUPPORT_WLAN_24G_SETTING\
 CONFIG_SUPPORT_WLAN_5G_SETTING\
 CONFIG_SUPPORT_WLAN_5G_2_SETTING\
 CONFIG_SUPPORT_FAST_HANDOVER_INDEPENDENT_SETTING\
 CONFIG_SUPPORT_CHANNEL_CONFIG_SETTING\
 CONFIG_SUPPORT_IPV6_SETTING\
 CONFIG_SUPPORT_MULTICAST_TO_UNICAST_SETTING\
 CONFIG_SUPPORT_WLAN_EXTENSION_CHANNEL\
 CONFIG_SUPPORT_WLAN_BANDSTEER_ENHANCE\
 CONFIG_SUPPORT_LED_MODULE_NAME\
 CONFIG_SUPPORT_MULTI_ACCOUNT\
 CONFIG_SUPPORT_MESH_SETTING\
 CONFIG_SUPPORT_JAPAN_BANDWIDTH_HT80\
 CONFIG_SUPPORT_FAST_ROAMING_PER_SSID\
 CONFIG_SUPPORT_MAC_FILTER_NO_RELOAD\
 CONFIG_SUPPORT_FORCE_REBOOT\
 CONFIG_SUPPORT_WLAN_OUTDOOR_DISTANCE\
 CONFIG_WEB_SUPPORT_SENAOWRT_IMAGE\
 CONFIG_WEB_SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING\
 CONFIG_WEB_HWMODE_AC\
 CONFIG_WEB_HWMODE_AX\
 CONFIG_SUPPORT_WIFI_CERTIFIED\
 CONFIG_SUPPORT_CONFIG_HAS_SECTIONNAME\
 CONFIG_AP_MODE_SUPPORT_MESH_EZSETUP\
 CONFIG_WEB_SUPPORT_COMBINED_SSID_SETTING\
 CONFIG_WEB_DEF_PORTAL_LIMIT_NUM\
 CONFIG_SUPPORT_VLAN_PORT_ONLY_LAN2\
 CONFIG_SUPPORT_HOTSPOT_SETTING\
 CONFIG_WEB_SUPPORT_WPA3\
 CONFIG_SUPPORT_CAPWAP_NO_BACKGROUND_SCAN

TARGET_CFLAGS += $(foreach n, $(CONFIG_PACKAGE_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_SUPPORT_%,SUPPORT_%,$(n))))
TARGET_CFLAGS += $(foreach n, $(CONFIG_PACKAGE_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_WEB_SUPPORT_%,SUPPORT_%,$(n))))
TARGET_CFLAGS += $(foreach n, $(CONFIG_PACKAGE_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_WEB_%,%,$(n))))

ifeq ($(DUMP),)
STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_sn_$(shell $(SH_FUNC) grep '$(subst $(space),\|,$(patsubst %,^%=,$(strip $(CONFIG_PACKAGE_LIST))))' $(TOPDIR)/.config | md5s)
endif

ifeq ($(CONFIG_PACKAGE_senao-coova-chilli),y)
TARGET_CFLAGS += -DSUPPORT_CAPTIVE_PORTAL_SETTING=1
endif

MAKE_OPTS:= \
	$(TARGET_CONFIGURE_OPTS) \
	EXTRA_CFLAGS="$(TARGET_CFLAGS)" \
	STAGING_DIR="$(STAGING_DIR)" \
	BUILD_DIR="$(BUILD_DIR)" \
	SN_CONFIGS_DIR="$(SN_CONFIGS_DIR)" \
	SN_INCLUDE_DIR="$(SN_INCLUDE_DIR)" \
	TARGET_CFLAGS='$(MODEL_CFLAGS)'

define Package/senao_capwap_c1.9.x
  SECTION:=net
  CATEGORY:=SENAO
  SUBMENU:=capwap
  TITLE:=CAPWAP
  PROPRIETARY=y
  URL:=http://capwap.sourceforge.net/
  DEPENDS:=+libopenssl +openlldp +libpthread +librt +libapi +fcgi +libjson-c +libsnjson +senao-sysutil +ezCom +discovery +@BUSYBOX_CONFIG_USLEEP +fingerprint +senao-coova-chilli +senao-acs-shorterm
endef

define Package/senao_capwap_c1.9.x/description
  This package is an implementation of CAPWAP agents for WTPs and ACs.
endef

define Package/senao_capwap_c1.9.x/config
       source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	printf "$$$$(echo $$$$(for var in MAJOR MINOR; do grep CW_VERSION_$$$${var} $(PKG_BUILD_DIR)/CWVersion.h | awk -F\  '{print $$$$NF}'; done;echo -n $(VERSION_TARGET)) | tr ' ' '.')" >"$(PKG_BUILD_DIR)/capwap.ver"
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(MAKE_OPTS) \
		wtp.bin
endef

define Package/senao_capwap_c1.9.x/install
	$(INSTALL_DIR) $(1)/usr/bin
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/AC $(1)/usr/bin/ac
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/WTP $(1)/usr/bin/wtp
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/WUA $(1)/usr/bin/wua
#ifeq ($(findstring ATMWS,$(MODEL_NAME)),)
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ezCom/ezCom_ap $(1)/usr/bin/ezCom
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/discovery/findAc $(1)/usr/bin/findAc
#endif
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wtp.bin $(1)/usr/bin/wtp
	$(INSTALL_DIR) $(1)/etc/capwap $(1)/etc/uci-defaults
#	$(INSTALL_DATA) $(PKG_BUILD_DIR)/root.pem $(1)/etc/capwap/
#	$(INSTALL_DATA) $(PKG_BUILD_DIR)/client.pem $(1)/etc/capwap/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/config.wtp $(1)/etc/capwap/
#	$(INSTALL_DATA) $(PKG_BUILD_DIR)/settings.wtp.txt $(1)/etc/capwap/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./sn_files/apcontroller.conf $(1)/etc/config/apcontroller || $(INSTALL_DATA) ./files/apcontroller.conf $(1)/etc/config/apcontroller
	$(INSTALL_DIR) $(1)/etc/init.d
ifeq ($(CONFIG_PACKAGE_procd),y)
	$(INSTALL_BIN) ./files/apcontroller.procd.init $(1)/etc/init.d/apcontroller
else
	$(INSTALL_BIN) ./sn_files/apcontroller.init $(1)/etc/init.d/apcontroller || $(INSTALL_BIN) ./files/apcontroller.init $(1)/etc/init.d/apcontroller
endif
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cert-$(MODEL_VENDOR)/defaultWTP.pem $(1)/etc/capwap/defaultWTP.pem
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cert-$(MODEL_VENDOR)/defaultWTPRoot.pem $(1)/etc/capwap/defaultWTPRoot.pem
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cert-$(MODEL_VENDOR)/defaultWTPKey.pem $(1)/etc/capwap/defaultWTPKey.pem
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cert-$(MODEL_VENDOR)/currentWTP.pem $(1)/etc/capwap/currentWTP.pem
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cert-$(MODEL_VENDOR)/currentWTPRoot.pem $(1)/etc/capwap/currentWTPRoot.pem
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cert-$(MODEL_VENDOR)/currentWTPKey.pem $(1)/etc/capwap/currentWTPKey.pem
	-$(CP) ./files/base-files/* $(1)/ 2>&-
	-printf "$$$$(echo $$$$(for var in MAJOR MINOR; do grep CW_VERSION_$$$${var} $(PKG_BUILD_DIR)/CWVersion.h | awk -F\  '{print $$$$NF}'; done;echo $(VERSION_TARGET)) | tr ' ' '.')" >"$(1)/etc/version_capwap"
	cp -f $(PKG_BUILD_DIR)/capwap.ver $(SN_CONFIGS_DIR)/capwap.ver
	$(INSTALL_DATA) ./sn_files/capwap.defaults $(1)/etc/uci-defaults/z99capwap || $(INSTALL_DATA) ./files/capwap.defaults $(1)/etc/uci-defaults/z99capwap
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DATA) ./sn_files/portal.sh $(1)/usr/sbin/portal.sh || $(INSTALL_DATA) ./files/portal.sh $(1)/usr/sbin/portal.sh
	$(INSTALL_DATA) ./sn_files/capwap_sn_event.sh $(1)/usr/sbin/capwap_sn_event.sh || $(INSTALL_DATA) ./files/capwap_sn_event.sh $(1)/usr/sbin/capwap_sn_event.sh
endef

$(eval $(call BuildPackage,senao_capwap_c1.9.x))
