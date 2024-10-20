#
# Copyright (C) 2010-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/target.mk
-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_MAJOR_VERSION=1
PKG_MINOR_VERSION=2
PKG_RELEASE_VERSION=6

PKG_NAME:=syseye
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

ifeq ($(CONFIG_PACKAGE_syseye-use-fork),y)
	CFLAGS_act += -DUSE_FORK
endif

ifeq ($(CONFIG_PACKAGE_syseye-act_sample),y)
	CFLAGS_act += -I$(PKG_BUILD_DIR)/act_sample -DACT_SAMPLE
	LDFLAGS_act += -Lact_sample -lact_sample
	PLUGINS_act += act_sample
endif

ifeq ($(CONFIG_PACKAGE_syseye-act_selftest),y)
	CFLAGS_act += -I$(PKG_BUILD_DIR)/act_selftest -DACT_SELFTEST
	LDFLAGS_act += -Lact_selftest -lact_selftest
	PLUGINS_act += act_selftest
endif

ifeq ($(CONFIG_PACKAGE_syseye-act_wifi_clinfo),y)
	CONFIG_FUNC_LIST= \
         CONFIG_SUPPORT_WLAN_24G_SETTING \
	 CONFIG_SUPPORT_WLAN_5G_SETTING \
	 CONFIG_SUPPORT_WLAN_5G_2_SETTING
	CFLAGS_act += -I$(PKG_BUILD_DIR)/act_ws_wifi_clinfo -DACT_WS_WIFI_CLINFO
	CFLAGS_act += $(foreach n, $(CONFIG_FUNC_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_SUPPORT_%,SUPPORT_%=1,$(n))))
	LDFLAGS_act += -Lact_ws_wifi_clinfo -lact_ws_wifi_clinfo -liwinfo
	PLUGINS_act += act_ws_wifi_clinfo

endif

ifeq ($(CONFIG_PACKAGE_syseye-act_ws_wifi_clinfo),y)
	CONFIG_FUNC_LIST=\
	 CONFIG_SUPPORT_WLAN_24G_SETTING\
	 CONFIG_SUPPORT_WLAN_5G_SETTING\
	 CONFIG_SUPPORT_WLAN_5G_2_SETTING
	CFLAGS_act += -I$(PKG_BUILD_DIR)/act_ws_wifi_clinfo -DACT_WS_WIFI_CLINFO
	CFLAGS_act += $(foreach n, $(CONFIG_FUNC_LIST), $(if $(filter y m, $($(n))),-D$(patsubst CONFIG_SUPPORT_%,SUPPORT_%=1,$(n))))
	LDFLAGS_act += -Lact_ws_wifi_clinfo -lact_ws_wifi_clinfo -liwinfo
	PLUGINS_act += act_ws_wifi_clinfo
endif

ifeq ($(CONFIG_PACKAGE_syseye-act_sitesurvey),y)
	CFLAGS_act += -I$(PKG_BUILD_DIR)/act_sitesurvey -DACT_SITESURVEY
	LDFLAGS_act += -Lact_sitesurvey -lact_sitesurvey
	PLUGINS_act += act_sitesurvey
endif
ifeq ($(CONFIG_PACKAGE_syseye-act_linkcheck),y)
	CFLAGS_act += -I$(PKG_BUILD_DIR)/act_linkcheck -DACT_LINKCHECK
	LDFLAGS_act += -Lact_linkcheck -lact_linkcheck
	PLUGINS_act += act_linkcheck

ifeq ($(CONFIG_TARGET_BOARD),"ipq40xx")
	CFLAGS_act += -DTARGET_BOARD_ipq40xx
endif

endif # CONFIG_PACKAGE_syseye-act_linkcheck

ifeq ($(CONFIG_PACKAGE_syseye-act_psscan),y)
	CFLAGS_act += -I$(PKG_BUILD_DIR)/act_psscan -DACT_PSSCAN
	LDFLAGS_act += -Lact_psscan -lact_psscan
	PLUGINS_act += act_psscan
endif

# defaultly enabled
CFLAGS_ipc = -DIPC_UNIX

ifeq ($(CONFIG_PACKAGE_kmod-syseye-nlc),y)
	CFLAGS_ipc += -DIPC_NL
endif

ifeq ($(CONFIG_PACKAGE_syseye-act_runcmd),y)
	CFLAGS_act += -I$(PKG_BUILD_DIR)/act_runcmd -DACT_RUNCMD
	LDFLAGS_act += -Lact_runcmd -lact_runcmd
	PLUGINS_act += act_runcmd
endif

ifeq ($(CONFIG_PACKAGE_syseye-act_runlib_snlog),y)
	CFLAGS_act += -I$(PKG_BUILD_DIR)/act_runlib_snlog  -I$(STAGING_DIR)/usr/include -DACT_RUNLIB_SNLOG
	LDFLAGS_act += -Lact_runlib_snlog -lact_runlib_snlog -L$(STAGING_DIR)/usr/lib -lsnlog
	PLUGINS_act += act_runlib_snlog
endif

ifeq ($(CONFIG_PACKAGE_syseye-thread_linkmon),y)
	CFLAGS_thread += -I$(PKG_BUILD_DIR)/thread_linkmon -DTHREAD_LINKMON
	LDFLAGS_thread += -Lthread_linkmon -llinkmon
	PLUGINS_thread += thread_linkmon
endif

MAKE_FLAGS_SYSEYE += \
CFLAGS_syseye="-I$(PKG_BUILD_DIR)" \
CFLAGS_mio="-I$(STAGING_DIR)/usr/include" \
CFLAGS_json="-I$(STAGING_DIR)/usr/include" \
LDFLAGS_mio="-L$(STAGING_DIR)/usr/lib -lmio" \
LDFLAGS_json="-L$(STAGING_DIR)/usr/lib -lezjson" \
CFLAGS_libseipc="-I$(PKG_BUILD_DIR)" \
CFLAGS_ipc="$(CFLAGS_ipc)" \
CFLAGS_act="$(CFLAGS_act)" \
CFLAGS_thread="$(CFLAGS_thread)" \
LDFLAGS_act="$(LDFLAGS_act)" \
LDFLAGS_thread="$(LDFLAGS_thread)" \
LDFLAGS_libseipc="-L$(PKG_BUILD_DIR) -lseipc" \
PLUGINS_act="$(PLUGINS_act)" \
PLUGINS_thread="$(PLUGINS_thread)" \
EXTRA_CFLAGS="$(EXTRA_CFLAGS)"

MAKE_FLAGS_SYSEYE_IPC += \
CFLAGS_json="-I$(STAGING_DIR)/usr/include" \
CFLAGS_libseipc="-I$(PKG_BUILD_DIR)" \
LDFLAGS_json="-L$(STAGING_DIR)/usr/lib -lezjson" \
LDFLAGS_libseipc="-L$(PKG_BUILD_DIR) -lseipc"

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)/senlc" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Package/syseye/Default
  SECTION=net
  CATEGORY:=SENAO
  TITLE:=syseye
  DEPENDS:=+mio +ezjson
  SUBMENU:=syseye
  PROPRIETARY=y
  MAINTAINER:=Ryan Hsu <ryan_hsu@hotmail.com.tw>
endef

define Package/syseye
  $(call Package/syseye/Default)
  DEPENDS+=+libpthread +PACKAGE_syseye-act_ws_wifi_clinfo:libiwinfo +PACKAGE_senao-log-proxy:senao-log-proxy
endef

define Package/syseye/description
 senao system management center.
endef

define Package/syseye/config
  source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(MAKE_FLAGS) $(MAKE_FLAGS_SYSEYE) 
ifeq ($(CONFIG_PACKAGE_syseye-ipc),y)
	$(MAKE) -C $(PKG_BUILD_DIR)/seipc $(MAKE_FLAGS) $(MAKE_FLAGS_SYSEYE_IPC)
endif

ifeq ($(CONFIG_PACKAGE_kmod-syseye-nlc),y)
	$(MAKE) -C "$(LINUX_DIR)" \
	$(MAKE_OPTS)\
	modules
endif
endef

define Package/syseye/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/etc/init.d $(1)/etc/config $(1)/usr/lib $(1)/usr/bin $(1)/etc/syseye
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/syseye $(1)/usr/sbin/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/libseipc.so* $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/cfg_ram.json $(1)/etc/syseye
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/cfg_saved.json $(1)/etc/syseye
ifeq ($(CONFIG_PACKAGE_procd),y)
	$(INSTALL_BIN) ./files/syseye.procd.init $(1)/etc/init.d/syseye
else
	$(INSTALL_BIN) ./files/syseye.init $(1)/etc/init.d/syseye
endif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/libseipc.so* $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libseipc.h $(1)/usr/include/
endef

define Package/syseye-act_sample
  SECTION=net
  CATEGORY:=SENAO
  TITLE:=syseye act_sample
  DEPENDS:=+ezjson +syseye
  SUBMENU:=syseye
  PROPRIETARY=y
endef

define Package/syseye-act_sample/description
 Action example
 module: act_sample
endef

define Package/syseye-act_selftest
  SECTION=net
  CATEGORY:=SENAO
  TITLE:=syseye act_selftest
  DEPENDS:=+ezjson +syseye
  SUBMENU:=syseye
  PROPRIETARY=y
endef

define Package/syseye-act_selftest/description
 selftest action, tragger senlc to send command to syseye to touch a test file
 module: act_selftest
endef

define Package/syseye-act_wifi_clinfo
  SECTION=net
  CATEGORY:=SENAO
  TITLE:=syseye act_clinfo
  DEPENDS:=+ezjson +syseye
  SUBMENU:=syseye
  PROPRIETARY=y
endef

define Package/syseye-act_wifi_clinfo/description
 Get wireless client list. (For ezm cloud)
 module: act_wifi_clinfo
endef

define Package/syseye-act_ws_wifi_clinfo
  SECTION=net
  CATEGORY:=SENAO
  TITLE:=syseye act_ws_wifi_clinfo
  DEPENDS:=+ezjson +syseye +libiwinfo
  SUBMENU:=syseye
  PROPRIETARY=y
endef

define Package/syseye-act_ws_wifi_clinfo/description
 Get wireless client list. (for web socket)
 module: act_ws_wifi_clinfo
endef

define Package/syseye-act_sitesurvey
  SECTION=net
  CATEGORY:=SENAO
  TITLE:=syseye act_sitesurvey
  DEPENDS:=+ezjson +syseye
  SUBMENU:=syseye
  PROPRIETARY=y
endef

define Package/syseye-act_sitesurvey/description
 get site survey information
 module: act_sitesurvey
endef

define Package/syseye-act_linkcheck
  SECTION=net
  CATEGORY:=SENAO
  TITLE:=syseye act_linkcheck
  DEPENDS:=+ezjson +syseye
  SUBMENU:=syseye
  PROPRIETARY=y
endef

define Package/syseye-act_linkcheck/description
 get device connectivity status
 module: act_linkcheck
endef

define Package/syseye-act_psscan
  SECTION=net
  CATEGORY:=SENAO
  TITLE:=syseye act_psscan
  DEPENDS:=+ezjson +syseye
  SUBMENU:=syseye
  PROPRIETARY=y
endef

define Package/syseye-act_psscan/description
 Action Present Scanning
 module: act_psscan
endef

define Package/syseye-act_runcmd
  SECTION=net
  CATEGORY:=SENAO
  TITLE:=syseye act_runcmd
  DEPENDS:=+ezjson +syseye
  SUBMENU:=syseye
  PROPRIETARY=y
endef

define Package/syseye-act_runcmd/description
 Action runcmd, action plugin for client to execute user space command
 module: act_runcmd
endef

define Package/syseye-act_runlib_snlog
  $(call Package/syseye/Default)
  TITLE+= plugin act_runlib_snlog
  DEPENDS=+syseye +PACKAGE_senao-log-proxy:senao-log-proxy
endef

define Package/syseye-act_runlib_snlog/description
  syseye action: run (external) library snlog
  module: act_runlib_snlog
endef

define Package/syseye-act_runlib_snlog/install
endef

define Package/syseye-thread_linkmon
  SECTION=net
  CATEGORY:=SENAO
  TITLE:=syseye thread_linkmon
  DEPENDS:=+mio +ezjson +syseye
  SUBMENU:=syseye
  PROPRIETARY=y
endef

define Package/syseye-thread_linkmon/description
 thread example
 module: thread_linkmon
endef

define Package/syseye-ipc
  $(call Package/syseye/Default)
  TITLE+= ipc
  DEPENDS+= +syseye
endef

define Package/syseye-ipc/description
 senao system management center controller client.
endef

define Package/syseye-ipc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/seipc/seipc $(1)/usr/bin/
endef

define KernelPackage/syseye-nlc
  SECTION=net
  CATEGORY:=SENAO
  TITLE:=syseye netlink client
  KCONFIG:=
  AUTOLOAD:=$(call AutoLoad,10,senlc_module)
  FILES:=$(PKG_BUILD_DIR)/senlc/senlc_module.$(LINUX_KMOD_SUFFIX)
  DEPENDS:=$(PKG_DEFAULT_DEPENDS) +syseye
  SUBMENU:=syseye
  PROPRIETARY=y
endef

$(eval $(call BuildPackage,syseye))
$(eval $(call BuildPackage,syseye-act_sample))
$(eval $(call BuildPackage,syseye-act_selftest))
$(eval $(call BuildPackage,syseye-act_wifi_clinfo))
$(eval $(call BuildPackage,syseye-act_ws_wifi_clinfo))
$(eval $(call BuildPackage,syseye-act_sitesurvey))
$(eval $(call BuildPackage,syseye-act_linkcheck))
$(eval $(call BuildPackage,syseye-act_psscan))
$(eval $(call BuildPackage,syseye-act_runcmd))
$(eval $(call BuildPackage,syseye-act_runlib_snlog))
$(eval $(call BuildPackage,syseye-thread_linkmon))
$(eval $(call BuildPackage,syseye-ipc))
$(eval $(call KernelPackage,syseye-nlc))
