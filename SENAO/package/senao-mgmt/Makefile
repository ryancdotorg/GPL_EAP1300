include $(TOPDIR)/rules.mk
-include $(INCLUDE_DIR)/sn-package-var.mk
PKG_NAME:=senao-mgmt
PKG_VERSION:=1.2
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,1),1)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(SN_PATCH_CONFIG_FILE)

define Package/senao-mgmt
	CATEGORY:=SENAO
	TITLE:=MANAGEMENT_SSID
	PROPRIETARY=y
	DEPENDS:=+kmod-ebtables +kmod-ebtables-ipv4 +ebtables +(!PACKAGE_kmod-syseye-nlc):kmod-senao-nlh +isc-dhcp-server-ipv4 @BUSYBOX_CONFIG_ARPING
endef
#FIXME logic correct, but mm menu, save, kmod-senao-nlh/kmod-syseye-nlc will be stored in strange logic
#	DEPENDS:=+kmod-ebtables +kmod-ebtables-ipv4 +ebtables +(!PACKAGE_kmod-syseye-nlc):kmod-senao-nlh +(!PACKAGE_kmod-senao-nlh):kmod-syseye-nlc +isc-dhcp-server-ipv4 @BUSYBOX_CONFIG_ARPING

define Package/senao-mgmt/config
  choice
    depends on PACKAGE_senao-mgmt
    prompt "MGMT bridge choice"
    default PACKAGE_mgmt-bridge-lan

    config PACKAGE_mgmt-bridge-lan
      bool "Mgmt in br-lan."

    config PACKAGE_mgmt-bridge-independent
      bool "Mgmt in independent bridge."

  endchoice
endef

define Package/senao-mgmt/description
	MANAGEMENT_SSID
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile
endef

define Package/senao-mgmt/install
	$(INSTALL_DIR) $(1)/etc/mgmt.d/ $(1)/etc/uci-defaults $(1)/etc/assoc_notify.d/ $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/mgmt.rule $(1)/etc/mgmt.d/mgmt_rule.sh
	$(INSTALL_BIN) ./files/mgmt.filePath $(1)/etc/mgmt.d/mgmt_path.sh
	$(INSTALL_BIN) ./files/mgmt.dhcpd $(1)/etc/mgmt.d/mgmt_dhcpd.sh
	$(INSTALL_BIN) ./files/mgmt.vap $(1)/etc/mgmt.d/mgmt_vap.sh
	$(INSTALL_BIN) ./files/mgmt.timer $(1)/etc/mgmt.d/mgmt_timer.sh
	$(INSTALL_BIN) ./files/mgmt.init $(1)/etc/init.d/senao-mgmt
	$(INSTALL_BIN) ./files/mgmt.preinit $(1)/etc/init.d/senao-mgmt-preinit
	$(INSTALL_BIN) ./files/mgmt.postinit $(1)/etc/init.d/senao-mgmt-postinit
	$(INSTALL_BIN) ./files/mgmt.notify $(1)/etc/assoc_notify.d/mgmt_notify.sh
ifeq ($(CONFIG_PACKAGE_mgmt-bridge-independent),y)
	$(INSTALL_BIN) ./files/mgmt.ind.defaults $(1)/etc/uci-defaults/z99senao-mgmt
else
	$(INSTALL_BIN) ./files/mgmt.defaults $(1)/etc/uci-defaults/z99senao-mgmt
endif
	$(INSTALL_DIR) $(1)/etc/hotplug.d/ifplugd
	$(INSTALL_BIN) ./files/mgmt.release $(1)/etc/hotplug.d/ifplugd/20-mgmt-release

endef

$(eval $(call BuildPackage,senao-mgmt))
