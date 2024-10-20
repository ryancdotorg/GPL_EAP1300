#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_MAJOR_VERSION=4
PKG_MINOR_VERSION=0
PKG_RELEASE_VERSION=2

PKG_NAME:=senao-coova-chilli
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))

#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=http://ap.coova.org/chilli
#PKG_MD5SUM:=dc0037e3cdebcb60508081b4e42e984a

PKG_INSTALL:=1
PKG_REMOVE_FILES:=

PKG_CONFIG_DEPENDS := \
  CONFIG_SENAO_COOVACHILLI_MINIPORTAL \
  CONFIG_SENAO_COOVACHILLI_REDIR \
  CONFIG_SENAO_COOVACHILLI_USERAGENT \
  CONFIG_SENAO_COOVACHILLI_DNSLOG \
  CONFIG_SENAO_COOVACHILLI_UAMDOMAINFILE \
  CONFIG_SENAO_COOVACHILLI_EAPOL \
  CONFIG_SENAO_COOVACHILLI_LARGELIMITS \
  CONFIG_SENAO_COOVACHILLI_NOSSL \
  CONFIG_SENAO_COOVACHILLI_MATRIXSSL \
  CONFIG_SENAO_COOVACHILLI_CYASSL \
  CONFIG_SENAO_COOVACHILLI_OPENSSL \
  CONFIG_SENAO_COOVACHILLI_NFQUEUE \
  CONFIG_SENAO_COOVACHILLI_SESSGARDEN \
  CONFIG_SENAO_COOVACHILLI_INSPECT \
  CONFIG_PACKAGE_kmod-ipt-nfcoova

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/senao-coova-chilli
  SUBMENU:=Captive Portals
  SECTION:=net
  CATEGORY:=SENAO
  DEPENDS:=+kmod-tun +librt +haserl +SENAO_COOVACHILLI_MINIPORTAL:libcurl +SENAO_COOVACHILLI_MATRIXSSL:libmatrixssl +SENAO_COOVACHILLI_CYASSL:libcyassl +SENAO_COOVACHILLI_OPENSSL:libopenssl +SENAO_COOVACHILLI_NFQUEUE:libnetfilter-queue +SENAO_COOVACHILLI_NFQUEUE:libnfnetlink +kmod-ipt-nfcoova +zlib +senao-guest-syncd +iputils-ping +(!PACKAGE_kmod-syseye-nlc):kmod-senao-nlh +jq +PACKAGE_senao-log-proxy:senao-log-proxy +conntrack-tools +kmod-ipt-tee +iptables-mod-tee
  TITLE:=SENAO Wireless LAN HotSpot controller (Coova Chilli Version)
  URL:=http://www.coova.org/CoovaChilli
  MENU:=1
endef
#FIXME logic correct, but mm menu, save, kmod-senao-nlh/kmod-syseye-nlc will be stored in strange logic
#  DEPENDS:=+kmod-tun +librt +haserl +SENAO_COOVACHILLI_MINIPORTAL:libcurl +SENAO_COOVACHILLI_MATRIXSSL:libmatrixssl +SENAO_COOVACHILLI_CYASSL:libcyassl +SENAO_COOVACHILLI_OPENSSL:libopenssl +SENAO_COOVACHILLI_NFQUEUE:libnetfilter-queue +SENAO_COOVACHILLI_NFQUEUE:libnfnetlink +kmod-ipt-nfcoova +zlib +senao-guest-syncd +iputils-ping +(!PACKAGE_kmod-syseye-nlc):kmod-senao-nlh +(!PACKAGE_kmod-senao-nlh):kmod-syseye-nlc +jq +PACKAGE_senao-log-proxy:senao-log-proxy

define Package/senao-coova-chilli/description
	CoovaChilli is an open source access controller for wireless LAN
	access points and is based on ChilliSpot. It is used for authenticating
	users of a wireless (or wired) LAN. It supports web based login (UAM)
	which is today's standard for public HotSpots and it supports Wireless
	Protected Access (WPA) which is the standard of the future.
	Authentication,	authorization and accounting (AAA) is handled by your
	favorite radius server.
endef

define Package/senao-coova-chilli/config
  source "$(SOURCE)/Config.in"
endef

define KernelPackage/ipt-nfcoova
  URL:=http://www.coova.org/CoovaChilli
  SUBMENU:=Captive Portals
  CATEGORY:=SENAO
  SUBMENU:=Senao Networks Modules
  DEPENDS:=+kmod-ipt-core +libxtables +SEANO_COOVACHILLI_NFQUEUE:kmod-nfnetlink-queue +SENAO_COOVACHILLI_NFQUEUE:kmod-ipt-nfqueue
  TITLE:=Coova netfilter module
  FILES:=$(PKG_BUILD_DIR)/src/linux/xt_*.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoProbe,xt_coova)
endef

define KernelPackage/ipt-nfcoova/description
	Netfilter kernel module for CoovaChilli
	Includes:
	- coova
endef

DISABLE_NLS=

TARGET_CFLAGS += $(FPIC)

ifeq ($(CONFIG_PACKAGE_senao-log-proxy),y)
TARGET_CFLAGS += -DSUPPORT_SENAO_LOG=1 -lsnlog
endif

CONFIGURE_VARS += \
       ARCH="$(LINUX_KARCH)" \
       KERNEL_DIR="$(LINUX_DIR)"

MAKE_FLAGS += \
       ARCH="$(LINUX_KARCH)" \
       KERNEL_DIR="$(LINUX_DIR)"

MAKE_INSTALL_FLAGS += \
       ARCH="$(LINUX_KARCH)" \
       KERNEL_DIR="$(LINUX_DIR)" \
       INSTALL_MOD_PATH="$(PKG_INSTALL_DIR)"

# NOTE:
# re-generate cmdline.c & comline.h
# cd src/src; cat cmdline.ggo | gengetopt -C
# sh cmdline.mini.sh > cmdline.mini.gen
# sync the following files
# src/cmdline.c
# src/cmdline.h
# src/cmdline.mini.gen

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -R ./src/* $(PKG_BUILD_DIR)/
	( cd $(PKG_BUILD_DIR) ; \
		[ -f ./configure ] || { \
			./bootstrap $(STAGING_DIR_HOST)/bin/; \
		} \
	)
endef

define Build/Configure
	$(call Build/Configure/Default, \
	$(if $(CONFIG_SENAO_COOVACHILLI_REDIR),--enable,--disable)-chilliredir \
	$(if $(CONFIG_SENAO_COOVACHILLI_DNSLOG),--enable,--disable)-dnslog \
	$(if $(CONFIG_SENAO_COOVACHILLI_MINIPORTAL),--enable,--disable)-miniportal \
	$(if $(CONFIG_SENAO_COOVACHILLI_MINIPORTAL),--enable,--disable)-chilliproxy \
	$(if $(CONFIG_SENAO_COOVACHILLI_MINIPORTAL),--enable,--disable)-chilliquery \
	$(if $(CONFIG_SENAO_COOVACHILLI_USERAGENT),--enable,--disable)-useragent \
	$(if $(CONFIG_SENAO_COOVACHILLI_LARGELIMITS),--enable,--disable)-largelimits \
	$(if $(CONFIG_SENAO_COOVACHILLI_UAMDOMAINFILE),--enable,--disable)-uamdomainfile \
	$(if $(CONFIG_SENAO_COOVACHILLI_EAPOL),--enable,--disable)-eapol \
	$(if $(CONFIG_SENAO_COOVACHILLI_MATRIXSSL),--with,--without)-matrixssl \
	$(if $(CONFIG_SENAO_COOVACHILLI_MINIPORTAL),--with,--without)-curl \
	$(if $(CONFIG_SENAO_COOVACHILLI_CYASSL),--with,--without)-cyassl \
	$(if $(CONFIG_SENAO_COOVACHILLI_OPENSSL),--with,--without)-openssl \
	$(if $(CONFIG_SENAO_COOVACHILLI_SESSGARDEN),--enable,--disable)-sessgarden \
	$(if $(CONFIG_PACKAGE_kmod-ipt-nfcoova),--with-nfcoova) \
	$(if $(CONFIG_SENAO_COOVACHILLI_NFQUEUE),--with-nfqueue) \
	$(if $(CONFIG_SENAO_COOVACHILLI_INSPECT),--enable,--disable)-inspect \
	--enable-json \
	)
endef

define Package/senao-coova-chilli/conffiles
/etc/config/portal
endef

define Package/senao-coova-chilli/install
	$(INSTALL_DIR) $(1)/etc/chilli $(1)/etc/chilli/www $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/usr/lib/ $(1)/etc/uci-defaults
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/chilli.conf $(1)/etc/
	$(INSTALL_BIN) ./files/portal.defaults $(1)/etc/uci-defaults/portal
	$(CP) $(PKG_INSTALL_DIR)/etc/chilli/* $(1)/etc/chilli/
	$(CP) ./files/url_decode.sed $(1)/etc/chilli/
	$(CP) ./files/config-local.sh $(1)/etc/chilli/www
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/etc/init.d/chilli $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/chilli* $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so.* $(1)/usr/lib/
	$(if $(CONFIG_PACKAGE_kmod-ipt-nfcoova), \
		$(INSTALL_DIR) $(1)/usr/lib/iptables; \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/iptables/lib*.so $(1)/usr/lib/iptables/; \
		$(CP) ./files/nfcoova/* $(1); \
	)
endef

$(eval $(call BuildPackage,senao-coova-chilli))
$(eval $(call KernelPackage,ipt-nfcoova))
