#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=kexec-tools
PKG_VERSION:=2.0.18
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@KERNEL/pub/linux/utils/kernel/kexec
PKG_MD5SUM:=43845327af54b002aaebd5b8076c7bd7

PKG_FIXUP:=autoreconf

PKG_CONFIG_DEPENDS := CONFIG_KEXEC_ZLIB CONFIG_KEXEC_LZMA

include $(INCLUDE_DIR)/package.mk

define Package/kexec-tools/Default
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=http://kernel.org/pub/linux/kernel/people/horms/kexec-tools/
  MAINTAINER:=Florian Fainelli <florian@openwrt.org>
endef

define Package/kexec-tools
  $(call Package/kexec-tools/Default)
  TITLE:=kexec-tools transition meta package
  DEPENDS:=+kexec
endef

define Package/kexec-tools/description
 kexec is a set of system calls that allows you to load
 another kernel from the currently executing Linux kernel.
 The kexec utility allows to load and boot another kernel.
endef

define Package/kexec
  $(call Package/kexec-tools/Default)
  TITLE:=Kernel boots kernel
  DEPENDS:=\
	@armeb||@arm||@i386||@x86_64||@powerpc64||@mipsel||@mips \
	+KEXEC_ZLIB:zlib +KEXEC_LZMA:liblzma
endef

define Package/kexec/description
 The kexec utility allows to load and boot another kernel.
endef

define Package/kexec/config
	source "$(SOURCE)/Config.in"
endef

#KEXEC_TARGET_NAME:=$(ARCH)-linux-$(TARGET_SUFFIX)
KEXEC_TARGET_NAME:=$(CONFIG_TARGET_NAME)

CONFIGURE_ARGS = \
		--target=$(KEXEC_TARGET_NAME) \
		--host=$(REAL_GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--program-prefix="" \
		--program-suffix="" \
		--prefix=/usr \
		--exec-prefix=/usr \
		--bindir=/usr/bin \
		--sbindir=/usr/sbin \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		$(if $(CONFIG_KEXEC_ZLIB),--with,--without)-zlib \
		$(if $(CONFIG_KEXEC_LZMA),--with,--without)-lzma \
		TARGET_LD="$(TARGET_CROSS)ld"

TARGET_CFLAGS += -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections

CONFIGURE_VARS += \
	BUILD_CC="$(HOSTCC)" \
	TARGET_CC="$(TARGET_CC)"

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) DESTDIR="$(PKG_INSTALL_DIR)" all install
endef

define Package/kexec-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/vmcore-dmesg $(1)/usr/sbin
endef

define Package/kexec/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/kexec $(1)/usr/sbin

# make a link for compatability with other distros
	$(INSTALL_DIR) $(1)/sbin
	$(LN) ../usr/sbin/kexec $(1)/sbin/kexec
endef

$(eval $(call BuildPackage,kexec-tools))
$(eval $(call BuildPackage,kexec))
