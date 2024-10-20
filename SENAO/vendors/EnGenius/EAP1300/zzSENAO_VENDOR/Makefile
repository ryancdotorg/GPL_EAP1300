include $(TOPDIR)/rules.mk

PKG_NAME:=zzSENAO_VENDOR
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/zzSENAO_VENDOR
	CATEGORY:=SENAO
	TITLE:=SENAO vendor scripts
	PROPRIETARY=y
	DEPENDS:= +libpthread
endef

define Package/zzSENAO_VENDOR/description
	SENAO vendor shell scripts compile as binary
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile                                                                                                                                               
endef

define Package/zzSENAO_VENDOR/install
	$(INSTALL_DIR) $(1)/usr/shc $(1)/etc/init.d
	$(call SENAO_SCRIPT_INSTALL,./files/usr/shc/cron.shc,$(1)/usr/shc,cron)
	$(INSTALL_BIN) ./files/etc/init.d/cron $(1)/etc/init.d/cron
	$(call SENAO_SCRIPT_INSTALL,./files/usr/shc/led.shc,$(1)/usr/shc,led)
	$(INSTALL_BIN) ./files/etc/init.d/led $(1)/etc/init.d/led
	$(call SENAO_SCRIPT_INSTALL,./files/usr/shc/factory_setting.shc,$(1)/usr/shc,factory_setting)
	$(INSTALL_BIN) ./files/etc/init.d/factory_setting $(1)/etc/init.d/factory_setting
	$(call SENAO_SCRIPT_INSTALL,./files/usr/shc/boot.shc,$(1)/usr/shc,boot)
	$(INSTALL_BIN) ./files/etc/init.d/boot $(1)/etc/init.d/boot
#	$(call SENAO_SCRIPT_INSTALL,./files/usr/shc/ubootenvcfg.shc,$(1)/usr/shc,ubootenvcfg)
#	$(INSTALL_BIN) ./files/etc/init.d/ubootenvcfg $(1)/etc/init.d/ubootenvcfg
#	$(call SENAO_SCRIPT_INSTALL,./files/usr/shc/mount-nand-ubifs.shc,$(1)/usr/shc,mount-nand-ubifs)
#	$(INSTALL_BIN) ./files/etc/init.d/mount-nand-ubifs $(1)/etc/init.d/mount-nand-ubifs
	$(call SENAO_SCRIPT_INSTALL,./files/usr/shc/setmac.shc,$(1)/usr/shc,setmac)
	$(INSTALL_BIN) ./files/etc/init.d/setmac $(1)/etc/init.d/setmac
	$(call SENAO_SCRIPT_INSTALL,./files/usr/shc/telnet.shc,$(1)/usr/shc,telnet)
	$(INSTALL_BIN) ./files/etc/init.d/telnet $(1)/etc/init.d/telnet
	$(call SENAO_SCRIPT_INSTALL,./files/usr/shc/umount.shc,$(1)/usr/shc,umount)
	$(INSTALL_BIN) ./files/etc/init.d/umount $(1)/etc/init.d/umount
	$(call SENAO_SCRIPT_INSTALL,./files/usr/shc/syslogd.shc,$(1)/usr/shc,syslogd)
	$(INSTALL_BIN) ./files/etc/init.d/syslogd $(1)/etc/init.d/syslogd
	$(call SENAO_SCRIPT_INSTALL,./files/usr/shc/setfastscan.shc,$(1)/usr/shc,setfastscan)
	$(INSTALL_BIN) ./files/etc/init.d/setfastscan $(1)/etc/init.d/setfastscan
	$(call SENAO_SCRIPT_INSTALL,./files/usr/shc/correctWifiConfig.shc,$(1)/usr/shc,correctWifiConfig)
	$(INSTALL_BIN) ./files/etc/init.d/correctWifiConfig $(1)/etc/init.d/correctWifiConfig
endef

$(eval $(call BuildPackage,zzSENAO_VENDOR))
