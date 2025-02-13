
#
# Copyright (C) 2014-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libwebsockets
PKG_VERSION:=3.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://codeload.github.com/warmcat/libwebsockets/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=29414be4f79f6abc0e6aadccd09a4da0f0c431e3b5691f496acd081ae6a8240c


PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_SOURCE_SUBDIR)

PKG_LICENSE:=LGPL-2.1+exception
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS += -DLWS_IPV6=$(if $(CONFIG_IPV6),ON,OFF)
CMAKE_OPTIONS += -DLWS_WITHOUT_TESTAPPS=ON
CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE=DEBUG
CMAKE_OPTIONS += -DLWS_HAVE_TCP_USER_TIMEOUT=OFF
CMAKE_OPTIONS += -DCMAKE_C_FLAGS="-Wno-error"

# other options worth noting
# CMAKE_OPTIONS += -DLWS_WITHOUT_EXTENSIONS=ON
# CMAKE_OPTIONS += -DLWS_WITHOUT_DAEMONIZE=ON
# CMAKE_OPTIONS += -DLWS_WITHOUT_SERVER=ON
# CMAKE_OPTIONS += -DLWS_WITHOUT_DEBUG=ON


define Package/libwebsockets/Default
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=libwebsockets
	DEPENDS:=+zlib +libcap
	URL:=https://libwebsockets.org
	MAINTAINER:=Karl Palsson <karlp@etactica.com>
	PROVIDES:= libwebsockets
endef

define Package/libwebsockets-openssl
	$(call Package/libwebsockets/Default)
	TITLE += (OpenSSL)
	DEPENDS += +libopenssl
	VARIANT:=openssl
endef

define Package/libwebsockets-mbedtls
	$(call Package/$(PKG_NAME)/Default)
	TITLE += (mbedTLS)
	DEPENDS += +libmbedtls
	VARIANT:=mbedtls
endef

define Package/libwebsockets-full
	$(call Package/libwebsockets/Default)
	TITLE += (Full - OpenSSL, libuv, plugins, CGI)
	DEPENDS += +libopenssl +libuv
	VARIANT:=full
endef

ifeq ($(BUILD_VARIANT),openssl)
    CMAKE_OPTIONS += -DLWS_OPENSSL_CLIENT_CERTS=/etc/ssl/certs
    CMAKE_OPTIONS += -DLWS_OPENSSL_SUPPORT=ON
    CMAKE_OPTIONS += -DLWS_WITH_SSL=ON
	CMAKE_OPTIONS += -DLWS_SSL_CLIENT_USE_OS_CA_CERTS=ON
endif

ifeq ($(BUILD_VARIANT),mbedtls)
    CMAKE_OPTIONS += -DLWS_WITH_MBEDTLS=1
endif

ifeq ($(BUILD_VARIANT),full)
    CMAKE_OPTIONS += -DLWS_OPENSSL_CLIENT_CERTS=/etc/ssl/certs
    CMAKE_OPTIONS += -DLWS_OPENSSL_SUPPORT=ON
    CMAKE_OPTIONS += -DLWS_WITH_SSL=ON
    CMAKE_OPTIONS += -DLWS_WITH_LIBUV=ON
    CMAKE_OPTIONS += -DLWS_WITH_PLUGINS=ON
    CMAKE_OPTIONS += -DLWS_WITH_SERVER_STATUS=ON
    CMAKE_OPTIONS += -DLWS_WITH_ACCESS_LOG=ON
    CMAKE_OPTIONS += -DLWS_WITH_CGI=ON
endif

define Package/libwebsockets/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwebsockets.so* $(1)/usr/lib/
endef

Package/libwebsockets-mbedtls/install = $(Package/libwebsockets/install)
Package/libwebsockets-openssl/install = $(Package/libwebsockets/install)
Package/libwebsockets-full/install = $(Package/libwebsockets/install)

$(eval $(call BuildPackage,libwebsockets-openssl))
$(eval $(call BuildPackage,libwebsockets-mbedtls))
$(eval $(call BuildPackage,libwebsockets-full))
