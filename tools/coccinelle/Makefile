#
# Copyright (C) 2014 2015 The Linux Foundation. All rights reserved.
#
include $(TOPDIR)/rules.mk

OS=$(shell lsb_release -si)
ARCH=$(shell uname -m | sed 's/x86_//;s/i[3-6]86/32/')
VER=$(shell lsb_release -sr)

ifeq ("$(OS)_${VER}","Ubuntu_12.04")

PKG_NAME:=coccinelle
PKG_VERSION:=1.0.0-rc24
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/coccinelle/coccinelle/archive/
PKG_MD5SUM:=8cbe943716a7a1033d46f8157ea986d4
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/coccinelle-coccinelle-$(PKG_VERSION)
HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += --with-pdflatex=no --disable-pcre-syntax

else

ifneq ("$(wildcard /etc/fedora-release)","")
PKG_NAME:=coccinelle
PKG_VERSION:=1.0.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/coccinelle/coccinelle/archive/
PKG_MD5SUM:=3f6edfc54a9e664a9ba20141fd08e647

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/coccinelle-coccinelle-$(PKG_VERSION)

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += --with-pdflatex=no --disable-pcre-syntax

HOST_CFLAGS += -fPIC

$(eval $(call HostBuild))
else
PKG_NAME:=coccinelle
PKG_VERSION:=1.0.6

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/coccinelle/coccinelle/archive/
PKG_MD5SUM:=071ca5cfbfce623f30186995131b7882

#HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/coccinelle-coccinelle-$(PKG_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/coccinelle-$(PKG_VERSION)

HOST_BUILD_PARALLEL:=0

include $(INCLUDE_DIR)/host-build.mk

HOST_CONFIGURE_ARGS += --with-pdflatex=no --disable-pcre-syntax

HOST_CFLAGS += -fPIC

define Host/Configure
	cd $(HOST_BUILD_DIR) && ./autogen
	$(call Host/Configure/Default)
	[ -f $(HOST_BUILD_DIR)/Makefile ]
endef
endif

$(eval $(call HostBuild))
endif
