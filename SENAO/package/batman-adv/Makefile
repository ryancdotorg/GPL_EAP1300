#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 5624 2006-11-23 00:29:07Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=batman-adv

PKG_VERSION:=2013.4.0
BATCTL_VERSION:=2013.4.0
PKG_RELEASE:=1
#PKG_MD5SUM:=6590caa324709289e3cb142273a5ff57
#BATCTL_MD5SUM:=42e269cc710bbc9a8fd17628201d4258

ifdef CONFIG_$(PKG_NAME)_2013.4.0
PKG_VERSION:=2013.4.0
BATCTL_VERSION:=2013.4.0
endif

ifdef CONFIG_$(PKG_NAME)_2015.2
PKG_VERSION:=2015.2
BATCTL_VERSION:=2015.2
endif

ifdef CONFIG_$(PKG_NAME)_2016.0
PKG_VERSION:=2016.0
BATCTL_VERSION:=2016.0
endif

ifdef CONFIG_$(PKG_NAME)_2016.3
PKG_VERSION:=2016.3
BATCTL_VERSION:=2016.3
endif

ifdef CONFIG_$(PKG_NAME)_2016.5
PKG_VERSION:=2016.5
BATCTL_VERSION:=2016.5
endif

ifdef CONFIG_$(PKG_NAME)_2017.2
PKG_VERSION:=2017.2
BATCTL_VERSION:=2017.2
endif

ifdef CONFIG_$(PKG_NAME)_2018.0
PKG_VERSION:=2018.0
BATCTL_VERSION:=2018.0
endif

ifdef CONFIG_$(PKG_NAME)_2019.3
PKG_VERSION:=2019.3
BATCTL_VERSION:=2019.3
endif

ifdef CONFIG_$(PKG_NAME)_2020.2
PKG_VERSION:=2020.2
BATCTL_VERSION:=2020.2
endif

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=http://downloads.open-mesh.org/batman/releases/batman-adv-$(PKG_VERSION)

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BATCTL_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/batctl-$(BATCTL_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/sn-package-version-override.mk

define KernelPackage/batman-adv
  URL:=http://www.open-mesh.org/
  MAINTAINER:=Marek Lindner <lindner_marek@yahoo.de>
  SUBMENU:=Network Support
  DEPENDS:=+kmod-lib-crc16 +kmod-crypto-core +kmod-crypto-crc32c +kmod-lib-crc32c +libc
  PROPRIETARY=y
  TITLE:=B.A.T.M.A.N. Adv
  FILES:=$(PKG_BUILD_DIR)/batman-adv.$(LINUX_KMOD_SUFFIX)
  AUTOLOAD:=$(call AutoLoad,50,batman-adv)
endef

ifdef CONFIG_$(PKG_NAME)_2015.2
define KernelPackage/batman-adv
 URL:=http://www.open-mesh.org/
 MAINTAINER:=Marek Lindner <lindner_marek@yahoo.de>
 SUBMENU:=Network Support
 DEPENDS:=+kmod-lib-crc16 +kmod-crypto-core +kmod-crypto-crc32c +kmod-lib-crc32c +libc +libnl-tiny
 TITLE:=B.A.T.M.A.N. Adv
 PROPRIETARY=y
 FILES:=$(PKG_BUILD_DIR)/net/batman-adv/batman-adv.$(LINUX_KMOD_SUFFIX)
 AUTOLOAD:=$(call AutoLoad,50,batman-adv)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2016.0
define KernelPackage/batman-adv
 URL:=http://www.open-mesh.org/
 MAINTAINER:=Marek Lindner <lindner_marek@yahoo.de>
 SUBMENU:=Network Support
 DEPENDS:=+kmod-lib-crc16 +kmod-crypto-core +kmod-crypto-crc32c +kmod-lib-crc32c +libc +libnl-tiny
 TITLE:=B.A.T.M.A.N. Adv
 PROPRIETARY=y
 FILES:=$(PKG_BUILD_DIR)/net/batman-adv/batman-adv.$(LINUX_KMOD_SUFFIX)
 AUTOLOAD:=$(call AutoLoad,50,batman-adv)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2016.3
define KernelPackage/batman-adv
 URL:=http://www.open-mesh.org/
 MAINTAINER:=Marek Lindner <lindner_marek@yahoo.de>
 SUBMENU:=Network Support
 DEPENDS:=+kmod-lib-crc16 +kmod-crypto-core +kmod-crypto-crc32c +kmod-lib-crc32c +libc +libnl-tiny
 TITLE:=B.A.T.M.A.N. Adv
 PROPRIETARY=y
 FILES:=$(PKG_BUILD_DIR)/net/batman-adv/batman-adv.$(LINUX_KMOD_SUFFIX)
 AUTOLOAD:=$(call AutoLoad,50,batman-adv)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2016.5
define KernelPackage/batman-adv
 URL:=http://www.open-mesh.org/
 MAINTAINER:=Marek Lindner <lindner_marek@yahoo.de>
 SUBMENU:=Network Support
 DEPENDS:=+kmod-lib-crc16 +kmod-crypto-core +kmod-crypto-crc32c +kmod-lib-crc32c +libc +libnl-tiny +librt
 TITLE:=B.A.T.M.A.N. Adv
 PROPRIETARY=y
 FILES:=$(PKG_BUILD_DIR)/build/net/batman-adv/batman-adv.$(LINUX_KMOD_SUFFIX)
 AUTOLOAD:=$(call AutoLoad,50,batman-adv)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2017.2
define KernelPackage/batman-adv
 URL:=http://www.open-mesh.org/
 MAINTAINER:=Marek Lindner <lindner_marek@yahoo.de>
 SUBMENU:=Network Support
 DEPENDS:=+kmod-lib-crc16 +kmod-crypto-core +kmod-crypto-crc32c +kmod-lib-crc32c +libc +libnl-tiny +librt +kmod-cfg80211-linux
 TITLE:=B.A.T.M.A.N. Adv
 PROPRIETARY=y
 FILES:=$(PKG_BUILD_DIR)/build/net/batman-adv/batman-adv.$(LINUX_KMOD_SUFFIX)
 AUTOLOAD:=$(call AutoLoad,50,batman-adv)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2018.0
define KernelPackage/batman-adv
 URL:=http://www.open-mesh.org/
 MAINTAINER:=Marek Lindner <lindner_marek@yahoo.de>
 SUBMENU:=Network Support
 DEPENDS:=+kmod-lib-crc16 +kmod-crypto-core +kmod-crypto-crc32c +kmod-lib-crc32c +libc +libnl-tiny +librt +kmod-cfg80211-linux
 TITLE:=B.A.T.M.A.N. Adv
 PROPRIETARY=y
 FILES:=$(PKG_BUILD_DIR)/build/net/batman-adv/batman-adv.$(LINUX_KMOD_SUFFIX)
 AUTOLOAD:=$(call AutoLoad,50,batman-adv)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2019.3
define KernelPackage/batman-adv
 URL:=http://www.open-mesh.org/
 MAINTAINER:=Marek Lindner <lindner_marek@yahoo.de>
 SUBMENU:=Network Support
 DEPENDS:=+kmod-lib-crc16 +kmod-crypto-core +kmod-crypto-crc32c +kmod-lib-crc32c +libc +libnl-tiny +librt +kmod-cfg80211-linux
 TITLE:=B.A.T.M.A.N. Adv
 PROPRIETARY=y
 FILES:=$(PKG_BUILD_DIR)/net/batman-adv/batman-adv.$(LINUX_KMOD_SUFFIX)
 AUTOLOAD:=$(call AutoLoad,50,batman-adv)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2020.2
define KernelPackage/batman-adv
 URL:=http://www.open-mesh.org/
 MAINTAINER:=Marek Lindner <lindner_marek@yahoo.de>
 SUBMENU:=Network Support
 DEPENDS:=+kmod-lib-crc16 +kmod-crypto-core +kmod-crypto-crc32c +kmod-lib-crc32c +libc +libnl-tiny +librt +kmod-cfg80211-linux
 TITLE:=B.A.T.M.A.N. Adv
 PROPRIETARY=y
 FILES:=$(PKG_BUILD_DIR)/net/batman-adv/batman-adv.$(LINUX_KMOD_SUFFIX)
 AUTOLOAD:=$(call AutoLoad,50,batman-adv)
endef
endif

define KernelPackage/batman-adv/description
B.A.T.M.A.N. advanced is a kernel module which allows to
build layer 2 mesh networks. This package contains the
version $(PKG_VERSION) of the kernel module plus its user space
configuration & managerment tool batctl.
endef

#define KernelPackage/batman-adv/config
#	source "$(SOURCE)/Config.in"
#endef

ifneq ("$(findstring linux-4.4.60_spf_10_0_cs_preview,$(CONFIG_EXTERNAL_KERNEL_TREE))","")
MAKE_BATMAN_ADV_ARGS += CFLAGS="-DSENAO_FIX_COMPILE"
endif

ifneq ("$(findstring linux-4.4.60_spf_10_0_cs,$(CONFIG_EXTERNAL_KERNEL_TREE))","")
MAKE_BATMAN_ADV_ARGS += CFLAGS="-DSENAO_FIX_COMPILE"
endif

ifneq ("$(findstring linux-4.4.60_ipq6018_ilq11.0_fc,$(CONFIG_EXTERNAL_KERNEL_TREE))","")
MAKE_BATMAN_ADV_ARGS += CFLAGS="-DSENAO_FIX_COMPILE"
endif

ifneq ("$(findstring linux-4.4.60_spf_11_0_csu1,$(CONFIG_EXTERNAL_KERNEL_TREE))","")
MAKE_BATMAN_ADV_ARGS += CFLAGS="-DSENAO_FIX_COMPILE"
endif

ifneq ("$(findstring linux-4.4.60_spf_11_0_csu3,$(CONFIG_EXTERNAL_KERNEL_TREE))","")
MAKE_BATMAN_ADV_ARGS += CFLAGS="-DSENAO_FIX_COMPILE"
endif

ifneq ("$(findstring linux-4.4.60_spf_11_1_csu2,$(CONFIG_EXTERNAL_KERNEL_TREE))","")
MAKE_BATMAN_ADV_ARGS += CFLAGS="-DSENAO_FIX_COMPILE"
endif

ifneq ("$(findstring linux-4.4.60_spf_11_2,$(CONFIG_EXTERNAL_KERNEL_TREE))","")
MAKE_BATMAN_ADV_ARGS += CFLAGS="-DSENAO_FIX_COMPILE"
endif

MAKE_BATMAN_ADV_ARGS += \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	KERNELPATH="$(LINUX_DIR)" \
	ARCH="$(LINUX_KARCH)" \
	PATH="$(TARGET_PATH)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	PWD="$(PKG_BUILD_DIR)" \
	LINUX_VERSION="$(LINUX_VERSION)" \
	CONFIG_BATMAN_ADV_DEBUG=$(if $(CONFIG_KMOD_BATMAN_ADV_DEBUG_LOG),y,n) \
	CONFIG_BATMAN_ADV_BLA=$(if $(CONFIG_KMOD_BATMAN_ADV_BLA),y,n) \
	CONFIG_BATMAN_ADV_DAT=$(if $(CONFIG_KMOD_BATMAN_ADV_DAT),y,n) \
	CONFIG_BATMAN_ADV_NC=$(if $(CONFIG_KMOD_BATMAN_ADV_NC),y,n) \
	CONFIG_BATMAN_ADV_BATMAN_V=$(if $(CONFIG_KMOD_BATMAN_ADV_BATMAN_V),y,n) \
	CONFIG_SENAO_BATCTL_ICMP_COMMUNICATION=$(if $(CONFIG_SENAO_BATCTL_ICMP_COMMUNICATION),y,n) \
	CONFIG_SENAO_SPIN_LOCK_BH_DELAY=$(if $(CONFIG_SENAO_SPIN_LOCK_BH_DELAY),y,n) \
	REVISION="" all

# The linker can identify unused sections of a binary when each symbol is stored
# in a separate section. This mostly removes unused linker sections and reduces 
# the size by ~3% on mipsel.

TARGET_CFLAGS  += -ffunction-sections -fdata-sections
ifdef CONFIG_$(PKG_NAME)_2015.2
TARGET_CFLAGS  += -I$(STAGING_DIR)/usr/include/libnl-tiny
endif
ifdef CONFIG_$(PKG_NAME)_2016.0
TARGET_CFLAGS  += -I$(STAGING_DIR)/usr/include/libnl-tiny
endif
ifdef CONFIG_$(PKG_NAME)_2016.3
TARGET_CFLAGS  += -I$(STAGING_DIR)/usr/include/libnl-tiny
endif
ifdef CONFIG_$(PKG_NAME)_2016.5
TARGET_CFLAGS  += -I$(STAGING_DIR)/usr/include/libnl-tiny
endif
ifdef CONFIG_$(PKG_NAME)_2017.2
TARGET_CFLAGS  += -I$(STAGING_DIR)/usr/include/libnl-tiny
endif
ifdef CONFIG_$(PKG_NAME)_2018.0
TARGET_CFLAGS  += -I$(STAGING_DIR)/usr/include/libnl-tiny
endif
ifdef CONFIG_$(PKG_NAME)_2019.3
TARGET_CFLAGS  += -I$(STAGING_DIR)/usr/include/libnl-tiny
endif
ifdef CONFIG_$(PKG_NAME)_2020.2
TARGET_CFLAGS  += -I$(STAGING_DIR)/usr/include/libnl-tiny
endif
TARGET_LDFLAGS += -Wl,--gc-sections

# Link-time optimization allows to move parts of the optimization from the single
# source file to the global source view. This is done by emitting the GIMPLE
# representation in each object file and analyzing it again during the link step.

TARGET_CFLAGS  += -flto
TARGET_LDFLAGS += -fuse-linker-plugin

MAKE_BATCTL_ENV += \
        CPPFLAGS="$(TARGET_CPPFLAGS)" \
        CFLAGS="$(TARGET_CFLAGS)" \
        LDFLAGS="$(TARGET_LDFLAGS)"

ifdef CONFIG_$(PKG_NAME)_2015.2
MAKE_BATCTL_ENV += \
        LIBNL_NAME="libnl-tiny"
endif

ifdef CONFIG_$(PKG_NAME)_2016.0
MAKE_BATCTL_ENV += \
        LIBNL_NAME="libnl-tiny"
endif

ifdef CONFIG_$(PKG_NAME)_2016.3
MAKE_BATCTL_ENV += \
        LIBNL_NAME="libnl-tiny"
endif

ifdef CONFIG_$(PKG_NAME)_2016.5
MAKE_BATCTL_ENV += \
        LIBNL_NAME="libnl-tiny"
endif

ifdef CONFIG_$(PKG_NAME)_2017.2
MAKE_BATCTL_ENV += \
        LIBNL_NAME="libnl-tiny"
endif

ifdef CONFIG_$(PKG_NAME)_2018.0
MAKE_BATCTL_ENV += \
        LIBNL_NAME="libnl-tiny"
endif

ifdef CONFIG_$(PKG_NAME)_2019.3
MAKE_BATCTL_ENV += \
        LIBNL_NAME="libnl-tiny"
endif

ifdef CONFIG_$(PKG_NAME)_2020.2
MAKE_BATCTL_ENV += \
        LIBNL_NAME="libnl-tiny"
endif

MAKE_BATCTL_ARGS += \
	REVISION="" \
	CC="$(TARGET_CC)" \
	DESTDIR="$(PKG_INSTALL_DIR)" \
	CONFIG_SENAO_BATCTL_ICMP_COMMUNICATION=$(if $(CONFIG_SENAO_BATCTL_ICMP_COMMUNICATION),y,n) \
	batctl install

ifneq ($(DEVELOPER)$(CONFIG_KMOD_BATMAN_ADV_BATCTL),)
define Download/batctl
  FILE:=batctl-$(BATCTL_VERSION).tar.gz
  URL:=$(PKG_SOURCE_URL)
  MD5SUM:=$(BATCTL_MD5SUM)
endef
#$(eval $(call Download,batctl))

BATCTL_EXTRACT = tar xzf "$(DL_DIR)/batctl-$(BATCTL_VERSION).tar.gz" -C "$(BUILD_DIR)/$(PKG_NAME)"
BATCTL_PATCH = $(call Build/DoPatch,"$(PKG_BATCTL_BUILD_DIR)","$(PATCH_DIR)","*batctl*")
BATCTL_BUILD = $(MAKE_BATCTL_ENV) $(MAKE) -C $(PKG_BATCTL_BUILD_DIR) $(MAKE_BATCTL_ARGS)
BATCTL_INSTALL = $(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/local/sbin/batctl $(1)/usr/sbin/
endif

KPATCH ?= $(PATCH)
define Build/DoPatch
	@if [ -d "$(2)" ]; then \
		if [ "$$$$(ls $(2) | grep -Ec $(3))" -gt 0 ]; then \
			$(KPATCH) "$(1)" "$(2)" "$(3)"; \
		fi; \
	fi
endef

define Build/Patch
	$(call Build/DoPatch,"$(PKG_BUILD_DIR)","$(PATCH_DIR)","*batman*")
	$(BATCTL_EXTRACT)
	$(BATCTL_PATCH)
endef

ifdef CONFIG_$(PKG_NAME)_2013.4.0
define Build/Prepare
	$(call Build/SNCopySrc,$(PKG_VERSION)/batman-adv)
	$(SED) '/#define _NET_BATMAN_ADV_MAIN_H_/a\#undef CONFIG_MODULE_STRIPPED' \
		$(PKG_BUILD_DIR)/main.h
	mkdir -p $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
	$(TAR) c -C ./src/$(PKG_VERSION)/batctl . \
		--exclude=.pc --exclude=.svn --exclude=.git \
		--exclude='boa-0*' --exclude='*.o' --exclude='*.so' | \
			tar x -C $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2015.2
define Build/Prepare
	$(call Build/SNCopySrc,$(PKG_VERSION)/batman-adv)
	$(SED) '/#define _NET_BATMAN_ADV_MAIN_H_/a\#undef CONFIG_MODULE_STRIPPED' \
		$(PKG_BUILD_DIR)/net/batman-adv/main.h
	mkdir -p $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
	$(TAR) c -C ./src/$(PKG_VERSION)/batctl . \
		--exclude=.pc --exclude=.svn --exclude=.git \
		--exclude='boa-0*' --exclude='*.o' --exclude='*.so' | \
			tar x -C $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2016.0
define Build/Prepare
	$(call Build/SNCopySrc,$(PKG_VERSION)/batman-adv)
	$(SED) '/#define _NET_BATMAN_ADV_MAIN_H_/a\#undef CONFIG_MODULE_STRIPPED' \
		$(PKG_BUILD_DIR)/net/batman-adv/main.h
	mkdir -p $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
	$(TAR) c -C ./src/$(PKG_VERSION)/batctl . \
		--exclude=.pc --exclude=.svn --exclude=.git \
		--exclude='boa-0*' --exclude='*.o' --exclude='*.so' | \
			tar x -C $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2016.3
define Build/Prepare
	$(call Build/SNCopySrc,$(PKG_VERSION)/batman-adv)
	$(SED) '/#define _NET_BATMAN_ADV_MAIN_H_/a\#undef CONFIG_MODULE_STRIPPED' \
		$(PKG_BUILD_DIR)/net/batman-adv/main.h
	mkdir -p $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
	$(TAR) c -C ./src/$(PKG_VERSION)/batctl . \
		--exclude=.pc --exclude=.svn --exclude=.git \
		--exclude='boa-0*' --exclude='*.o' --exclude='*.so' | \
			tar x -C $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2016.5
define Build/Prepare
	$(call Build/SNCopySrc,$(PKG_VERSION)/batman-adv)
	$(SED) '/#define _NET_BATMAN_ADV_MAIN_H_/a\#undef CONFIG_MODULE_STRIPPED' \
		$(PKG_BUILD_DIR)/net/batman-adv/main.h
	mkdir -p $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
	$(TAR) c -C ./src/$(PKG_VERSION)/batctl . \
		--exclude=.pc --exclude=.svn --exclude=.git \
		--exclude='boa-0*' --exclude='*.o' --exclude='*.so' | \
			tar x -C $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2017.2
define Build/Prepare
	$(call Build/SNCopySrc,$(PKG_VERSION)/batman-adv)
	$(SED) '/#define _NET_BATMAN_ADV_MAIN_H_/a\#undef CONFIG_MODULE_STRIPPED' \
		$(PKG_BUILD_DIR)/net/batman-adv/main.h
	mkdir -p $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
	$(TAR) c -C ./src/$(PKG_VERSION)/batctl . \
		--exclude=.pc --exclude=.svn --exclude=.git \
		--exclude='boa-0*' --exclude='*.o' --exclude='*.so' | \
			tar x -C $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2018.0
define Build/Prepare
	$(call Build/SNCopySrc,$(PKG_VERSION)/batman-adv)
	$(SED) '/#define _NET_BATMAN_ADV_MAIN_H_/a\#undef CONFIG_MODULE_STRIPPED' \
		$(PKG_BUILD_DIR)/net/batman-adv/main.h
	mkdir -p $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
	$(TAR) c -C ./src/$(PKG_VERSION)/batctl . \
		--exclude=.pc --exclude=.svn --exclude=.git \
		--exclude='boa-0*' --exclude='*.o' --exclude='*.so' | \
			tar x -C $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2019.3
define Build/Prepare
	$(call Build/SNCopySrc,$(PKG_VERSION)/batman-adv)
	$(SED) '/#define _NET_BATMAN_ADV_MAIN_H_/a\#undef CONFIG_MODULE_STRIPPED' \
		$(PKG_BUILD_DIR)/net/batman-adv/main.h
	mkdir -p $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
	$(TAR) c -C ./src/$(PKG_VERSION)/batctl . \
		--exclude=.pc --exclude=.svn --exclude=.git \
		--exclude='boa-0*' --exclude='*.o' --exclude='*.so' | \
			tar x -C $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
endef
endif

ifdef CONFIG_$(PKG_NAME)_2020.2
define Build/Prepare
	$(call Build/SNCopySrc,$(PKG_VERSION)/batman-adv)
	$(SED) '/#define _NET_BATMAN_ADV_MAIN_H_/a\#undef CONFIG_MODULE_STRIPPED' \
		$(PKG_BUILD_DIR)/net/batman-adv/main.h
	mkdir -p $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
	$(TAR) c -C ./src/$(PKG_VERSION)/batctl . \
		--exclude=.pc --exclude=.svn --exclude=.git \
		--exclude='boa-0*' --exclude='*.o' --exclude='*.so' | \
			tar x -C $(PKG_BUILD_DIR)/../batctl-$(PKG_VERSION)
endef
endif

define Build/Compile
	$(MAKE) -C "$(PKG_BUILD_DIR)" $(MAKE_BATMAN_ADV_ARGS)
	$(BATCTL_BUILD)
endef

define Build/Clean
        rm -rf $(BUILD_DIR)/$(PKG_NAME)/
endef

# senao add, copy batman-adv.ko and batctl to /tftpboot for debugging
SENAO_COPY_BATCTL = $(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/local/sbin/batctl /tftpboot
SENAO_COPY_BATMAN_ADV:=$(INSTALL_BIN) $(PKG_INSTALL_DIR)/../batman-adv.ko /tftpboot
ifdef CONFIG_$(PKG_NAME)_2015.2
SENAO_COPY_BATMAN_ADV:=$(INSTALL_BIN) $(PKG_INSTALL_DIR)/../net/batman-adv/batman-adv.ko /tftpboot
endif
ifdef CONFIG_$(PKG_NAME)_2016.0
SENAO_COPY_BATMAN_ADV:=$(INSTALL_BIN) $(PKG_INSTALL_DIR)/../net/batman-adv/batman-adv.ko /tftpboot
endif
ifdef CONFIG_$(PKG_NAME)_2016.3
SENAO_COPY_BATMAN_ADV:=$(INSTALL_BIN) $(PKG_INSTALL_DIR)/../net/batman-adv/batman-adv.ko /tftpboot
endif
ifdef CONFIG_$(PKG_NAME)_2016.5
SENAO_COPY_BATMAN_ADV:=$(INSTALL_BIN) $(PKG_INSTALL_DIR)/../build/net/batman-adv/batman-adv.ko /tftpboot
endif
ifdef CONFIG_$(PKG_NAME)_2017.2
SENAO_COPY_BATMAN_ADV:=$(INSTALL_BIN) $(PKG_INSTALL_DIR)/../build/net/batman-adv/batman-adv.ko /tftpboot
endif
ifdef CONFIG_$(PKG_NAME)_2018.0
SENAO_COPY_BATMAN_ADV:=$(INSTALL_BIN) $(PKG_INSTALL_DIR)/../build/net/batman-adv/batman-adv.ko /tftpboot
endif
ifdef CONFIG_$(PKG_NAME)_2019.3
SENAO_COPY_BATMAN_ADV:=$(INSTALL_BIN) $(PKG_INSTALL_DIR)/../net/batman-adv/batman-adv.ko /tftpboot
endif
ifdef CONFIG_$(PKG_NAME)_2020.2
SENAO_COPY_BATMAN_ADV:=$(INSTALL_BIN) $(PKG_INSTALL_DIR)/../net/batman-adv/batman-adv.ko /tftpboot
endif

define KernelPackage/batman-adv/install
	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/hotplug.d/net $(1)/etc/hotplug.d/iface $(1)/lib/batman-adv $(1)/usr/sbin $(1)/lib/netifd/proto
ifeq ("$(wildcard ./sn_files/batman-adv.config)","")
	$(INSTALL_DATA) ./files/etc/config/batman-adv $(1)/etc/config/batman-adv
else
	$(INSTALL_CONF) ./sn_files/batman-adv.config $(1)/etc/config/batman-adv
endif
	$(INSTALL_DATA) ./files/lib/batman-adv/config.sh $(1)/lib/batman-adv
	$(INSTALL_BIN) ./files/etc/hotplug.d/net/99-batman-adv $(1)/etc/hotplug.d/net
	$(INSTALL_BIN) ./files/lib/netifd/proto/batadv.sh $(1)/lib/netifd/proto
	$(BATCTL_INSTALL)
	$(SENAO_COPY_BATCTL)
	$(SENAO_COPY_BATMAN_ADV)
endef

define Package/kmod-$(PKG_NAME)/config
	$(call Package/kmod-$(PKG_NAME)/override_version)
endef

$(eval $(call KernelPackage,batman-adv))
