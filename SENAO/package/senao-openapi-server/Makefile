#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
-include $(INCLUDE_DIR)/sn-package-var.mk

PKG_MAJOR_VERSION=1
PKG_MINOR_VERSION=0
PKG_RELEASE_VERSION=9

PKG_NAME:=senao-openapi-server
PKG_VERSION:=$(PKG_MAJOR_VERSION).$(PKG_MINOR_VERSION)
PKG_RELEASE:=$(if $(SnReleaseVer),$(call SnReleaseVer,$(PKG_RELEASE_VERSION)),$(PKG_RELEASE_VERSION))

DL_PKG_NAME:=lighttpd
DL_PKG_VERSION:=1.4.55
DL_PKG_RELEASE:=1

PKG_SOURCE:=$(DL_PKG_NAME)-$(DL_PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://download.lighttpd.net/lighttpd/releases-1.4.x
PKG_MD5SUM:=be4bda2c28bcbdac6eb941528f6edf03

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/$(DL_PKG_NAME)-$(DL_PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS+=-std=c99

JSON_INCS += "$(STAGING_DIR)/usr/include/json-c"
API_JSON_INCS += "$(STAGING_DIR)/usr/include/api-json"

MAKE_FLAGS += \
        SN_INCLUDE_DIR="$(SN_INCLUDE_DIR)" \
	SN_CONFIGS_DIR="$(SN_CONFIGS_DIR)" \
        EXTRA_INCLUDE="-I$(JSON_INCS) -I$(API_JSON_INCS)"

ifeq ($(DUMP),)
  PKG_DEPEND_CONFIGS=$(shell grep '^CONFIG_SENAO_OPENAPI_SERVER_SUPPORT_' $(TOPDIR)/.config)
  PKG_DEPEND_CONFIGS+=$(shell grep '^CONFIG_SENAO_OPENAPI_SERVER_' $(SN_CONFIGS_DIR)/gconfig.mk)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_sn_$(shell $(SH_FUNC) echo $(PKG_DEPEND_CONFIGS) | md5s)
endif

define Package/senao-openapi-server/config
  source "$(SOURCE)/Config.in"
endef

define Package/senao-openapi-server/Default
  SUBMENU:=Web Servers/Proxies
  SECTION:=net
  CATEGORY:=SENAO
  SUBMENU:=EnGenius API Server
  URL:=http://www.lighttpd.net/
  PROPRIETARY=y
endef

define Package/senao-openapi-server
  $(call Package/senao-openapi-server/Default)
  MENU:=1
  DEPENDS:=+libopenssl +libpcre +libpthread +senao-openapi-server-mod-cgi +senao-openapi-server-mod-fastcgi +senao-openapi-server-mod-auth +senao-openapi-server-mod-redirect +libapi +libjson-c +libsnjson
  TITLE:=A flexible and lightweight web server
endef

define Package/senao-openapi-server-mod-auth
  $(call Package/senao-openapi-server/Default)
  DEPENDS:=+libopenssl
  TITLE:=Authentication module
endef

define Package/senao-openapi-server-mod-fastcgi
  $(call Package/senao-openapi-server/Default)
  TITLE:=FastCGI module
endef

define Package/senao-openapi-server-mod-cgi
  $(call Package/senao-openapi-server/Default)
  TITLE:=CGI module
endef

define Package/senao-openapi-server-mod-redirect
  $(call Package/senao-openapi-server/Default)
  DEPENDS:=libpcre
  TITLE:=Redirect module
endef

CONFIGURE_ARGS+= \
	--libdir=/usr/lib/senao-openapi-server \
	--sysconfdir=/etc/senao-openapi-server \
	--enable-shared \
	--enable-static \
	--disable-rpath \
	--without-attr \
	--without-bzip2 \
	--without-fam \
	--without-gdbm \
	--without-ldap \
	--without-lua \
	--without-memcache \
	--without-mysql \
	--with-pcre \
	--without-valgrind \
	 $(call autoconf_bool,CONFIG_IPV6,ipv6)

CONFIGURE_VARS+= \
	PCRE_LIB="-lpcre" \

CONFIGURE_ARGS+= \
	--with-openssl="$(STAGING_DIR)/usr" \
	--without-webdav-locks \
	--without-webdav-props

define Build/Configure
$(call Build/Configure/Default)
	$(CP) src/mod_auth.c $(PKG_BUILD_DIR)/src/
	$(CP) src/http-header-glue.c $(PKG_BUILD_DIR)/src/
endef

define Package/senao-openapi-server/conffiles
/etc/senao-openapi-server/senao-openapi-server.conf
endef

define Package/senao-openapi-server/install
	$(INSTALL_DIR) $(1)/usr/lib/senao-openapi-server
	$(INSTALL_DIR) $(1)/usr/lib/senao-openapi-server $(1)/www/cgi-bin/
	for m in dirlisting indexfile staticfile authn_file openssl; do \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/senao-openapi-server/mod_$$$${m}.so $(1)/usr/lib/senao-openapi-server/ ; \
	done
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/lighttpd $(1)/usr/sbin/senao-openapi-server
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/senao-openapi-server.conf $(1)/etc/config/senao-openapi-server
	$(INSTALL_DIR) $(1)/etc/uci-defaults
ifeq ($(CONFIG_SENAO_OPENAPI_SERVER_SUPPORT_APP_INTERFACE),y)
	$(INSTALL_BIN) ./files/etc/uci-defaults/senao-openapi-server.app.defaults $(1)/etc/uci-defaults/senao-openapi-server
	$(INSTALL_DIR) $(1)/etc/hotplug.d/net
	$(INSTALL_CONF) ./files/etc/hotplug.d/net/99-app-reload-avahi $(1)/etc/hotplug.d/net/99-app-reload-avahi
else
	$(INSTALL_BIN) ./files/etc/uci-defaults/senao-openapi-server.defaults $(1)/etc/uci-defaults/senao-openapi-server
endif
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/senao-openapi-server $(1)/etc/init.d/senao-openapi-server
	$(INSTALL_DIR) $(1)/etc/senao-openapi-server
	$(INSTALL_CONF) ./sn_files/etc/senao-openapi-server/senao-openapi-server.dav $(1)/etc/senao-openapi-server/senao-openapi-server.dav || $(INSTALL_CONF) ./files/etc/senao-openapi-server/senao-openapi-server.dav $(1)/etc/senao-openapi-server/senao-openapi-server.dav
	$(INSTALL_CONF) ./files/etc/senao-openapi-server/senao-openapi-server.dav $(1)/etc/senao-openapi-server/senao-openapi-server.dav
	$(INSTALL_CONF) ./files/etc/senao-openapi-server/senao-openapi-server.pem $(1)/etc/senao-openapi-server/senao-openapi-server.pem
endef

define BuildPlugin
  define Package/$(1)/install
	[ -z "$(2)" ] || $(INSTALL_DIR) $$(1)/usr/lib/senao-openapi-server
	for m in $(2); do \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/senao-openapi-server/mod_$$$$$$$${m}.so $$(1)/usr/lib/senao-openapi-server/ ; \
	done
  endef

  $$(eval $$(call BuildPackage,$(1)))
endef

$(eval $(call BuildPackage,senao-openapi-server))
$(eval $(call BuildPlugin,senao-openapi-server-mod-auth,auth))
$(eval $(call BuildPlugin,senao-openapi-server-mod-cgi,cgi))
$(eval $(call BuildPlugin,senao-openapi-server-mod-fastcgi,fastcgi))
$(eval $(call BuildPlugin,senao-openapi-server-mod-redirect,redirect))
