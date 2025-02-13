include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk
-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_MAJOR_VERSION=1
PKG_MINOR_VERSION=2
PKG_RELEASE_VERSION=12

PKG_NAME:=senao-ap-api-module
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
include $(INCLUDE_DIR)/package.mk

JSON_INCS += "$(STAGING_DIR)/usr/include/json-c"
API_JSON_INCS += "$(STAGING_DIR)/usr/include/api-json"

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
 CONFIG_SUPPORT_CAPTIVE_PORTAL_SETTING\
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
 CONFIG_SUPPORT_WIFI_CERTIFIED\
 CONFIG_SUPPORT_CONFIG_HAS_SECTIONNAME\
 CONFIG_AP_MODE_SUPPORT_MESH_EZSETUP\
 CONFIG_SUPPORT_LOCAL_ADDR_WITHOUT_CERTIFICATE\
 CONFIG_SUPPORT_SWOS_FUNCTION\
 CONFIG_SUPPORT_PASSWORD_LENGTH32\
 CONFIG_SUPPORT_COMPLEX_PASSWORD\
 CONFIG_SUPPORT_NETGEAR_FUNCTION\
 CONFIG_SUPPORT_LXC_EZMASTER_ACCOUNT_SETTING

TARGET_CFLAGS += $(foreach n, $(CONFIG_PACKAGE_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_SUPPORT_%,SUPPORT_%,$(n))))
TARGET_CFLAGS += $(foreach n, $(CONFIG_PACKAGE_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_WEB_SUPPORT_%,SUPPORT_%,$(n))))
TARGET_CFLAGS += $(foreach n, $(CONFIG_PACKAGE_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_WEB_%,%,$(n))))
MAKE_FLAGS += \
	$(TARGET_CONFIGURE_OPTS) \
	EXTRA_CFLAGS="$(TARGET_CFLAGS)" \
	STAGING_DIR="$(STAGING_DIR)" \
	BUILD_DIR="$(BUILD_DIR)" \
	SN_CONFIGS_DIR="$(SN_CONFIGS_DIR)" \
	SN_INCLUDE_DIR="$(SN_INCLUDE_DIR)" \
	EXTRA_INCLUDE="-I$(JSON_INCS) -I$(API_JSON_INCS)"

define Package/senao-ap-api-module
	SECTION:=utils
	CATEGORY:=SENAO
	SUBMENU:=openapi server
	DEPENDS:=+libapi +fcgi +libjson-c +libopenssl +libsnjson
	TITLE:=senao-ap-api-module
	PROPRIETARY=y
endef

ifndef BUILD_DIR_DEV
  define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)
  endef
endif

define Package/senao-ap-api-module/install
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.fcgi $(1)/www/cgi-bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.fcgi /tftpboot 
	$(INSTALL_BIN) ./src/cgi-bin/* $(1)/www/cgi-bin/
endef

$(eval $(call BuildPackage,senao-ap-api-module))
