#
# Copyright (C) 2012-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=u-boot-1.0
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)
PKG_VERSION:=2016.05
PKG_RELEASE:=1

PLATFORM_DEFINE=am335x_evm_spiboot

#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)
#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
#PKG_SOURCE_URL:= \
#	http://mirror2.openwrt.org/sources \
#	ftp://ftp.denx.de/pub/u-boot
#PKG_MD5SUM:=a076a044b64371edc52f7e562b13f6b2

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

include $(INCLUDE_DIR)/package.mk

define uboot/Default
  TITLE:=
  CONFIG:=
  IMAGE:=
endef

define uboot/$(PLATFORM_DEFINE)
  TITLE:=U-Boot for the SENAO am335x platform
endef

UBOOTS:=$(PLATFORM_DEFINE)

define Package/uboot/template
define Package/uboot-omap-$(1)
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_omap
  TITLE:=$(2)
  URL:=http://www.denx.de/wiki/U-Boot
  VARIANT:=$(1)
endef
endef

### SENAO ###
UBOOT_MAKE_OPTS:=

UBOOT_CONFIG:=$(patsubst CONFIG_PACKAGE_uboot-omap-$(PLATFORM_DEFINE)-UBOOT_CONFIG_TARGET=%,%,\
        $(filter CONFIG_PACKAGE_uboot-omap-$(PLATFORM_DEFINE)-UBOOT_CONFIG_TARGET%,\
            $(shell cat $(TOPDIR)/.config)))

# uboot configuration
UBOOT_MAKE_OPTS+=$(patsubst CONFIG_PACKAGE_uboot-omap-$(PLATFORM_DEFINE)-MAKEOPTS_%,%,\
        $(filter CONFIG_PACKAGE_uboot-omap-$(PLATFORM_DEFINE)-MAKEOPTS_%,\
            $(shell cat $(TOPDIR)/.config | sed s/\"//g)))

# Senao definitions
UBOOT_MAKE_OPTS+=$(patsubst CONFIG_PACKAGE_uboot-omap-$(PLATFORM_DEFINE)-%,%,\
        $(filter %=y,\
            $(filter CONFIG_PACKAGE_uboot-omap-$(PLATFORM_DEFINE)-SENAO%,\
                $(shell cat $(TOPDIR)/.config | sed s/\"//g))))

#UBOOT_MAKE_OPTS += CONFIG_ARCH_MISC_INIT=n

include $(PLATFORM_DEFINE)/uboot.mk


# define hardware relative configuration
ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "EIG9100")
include $(PLATFORM_DEFINE)/uboot-eig9100.mk
endif


ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "SP938BS")
include $(PLATFORM_DEFINE)/uboot-sp938bs.mk
endif

ifeq ($(CONFIG_PACKAGE_uboot-omap-am335x_evm_spiboot-MAKEOPTS_SENAO_MODEL_NAME), "SP935")
include $(PLATFORM_DEFINE)/uboot-sp935.mk
endif

IS_EMMC_BOOT:= 
#IS_EMMC_BOOT:= EMMC_AT_DEV1

ifdef IS_EMMC_BOOT
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_EMMC_BOOT
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_MMC_DEV1
else
# default settings
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_SPI_BOOT
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_MMC_DEV0
endif


$(warning ---$(UBOOT_MAKE_OPTS)---)

define Package/uboot-omap
  SECTION:=boot
  CATEGORY:=Boot Loaders
  URL:=http://www.denx.de/wiki/U-Boot
  MAINTAINER:=Texas Instruments
  TITLE:=U-boot for omap based platforms
  DEFAULT:=y
  DEPENDS:=@TARGET_omap
endef

#define Package/uboot-ipq40xx-fwupgrade-tools
#  SECTION:=boot
#  CATEGORY:=Boot Loaders
#  DEPENDS:=@TARGET_omap
#  TITLE:=U-boot images tools (dumpimage, mkimage)
#  URL:=http://www.denx.de/wiki/U-Boot
#endef

define BuildUBootPackage
	$(eval $(uboot/Default))
	$(eval $(uboot/$(1)))
	$(call Package/uboot/template,$(1),$(TITLE))
endef


ifdef BUILD_VARIANT
$(eval $(call uboot/$(BUILD_VARIANT)))
UBOOT_CONFIG:=$(if $(CONFIG),$(CONFIG),$(BUILD_VARIANT))
UBOOT_IMAGE:=$(if $(IMAGE),$(IMAGE),openwrt-$(BOARD)-$(BUILD_VARIANT)-u-boot.img)
endif

ifdef IS_EMMC_BOOT
export UBOOT_MACHINE=eig9100_emmc_config
else
export UBOOT_MACHINE=$(UBOOT_CONFIG)_config
endif

define Build/Prepare
	@echo "#########################################"
	@echo "########## U-BOOT-Prepare ###############"
	@echo "#########################################"
	$(CP) $(TOPDIR)/package/boot/uboot-omap/src/* $(PKG_BUILD_DIR)/
	@echo "#########################################"
	@echo "########## U-BOOT-CustomCopy ############"
	@echo "#########################################"
	$(CP) $(PLATFORM_DEFINE)/uboot/* $(PKG_BUILD_DIR)/
	@echo "#########################################"
	@echo "########## U-BOOT-Prepare DONE ##########"
	@echo "#########################################"
endef

define Build/Compile
	@echo "#########################################"
	@echo "########## $(UBOOT_CONFIG) ##############"
	@echo "#########################################"
	$(MAKE) -C $(PKG_BUILD_DIR) CROSS_COMPILE=$(TARGET_CROSS) \
		$(UBOOT_MACHINE)
	@echo "#########################################"
	@echo "########## U-BOOT-Compile ###############"
	@echo "#########################################"
	($(foreach var,$(strip $(UBOOT_MAKE_OPTS)), export $(var);) \
	$(MAKE) -C $(PKG_BUILD_DIR) CROSS_COMPILE=$(TARGET_CROSS) \
		DTC=$(KERNEL_BUILD_DIR)/$(LINUX_VERSION)/scripts/dtc/dtc)
	@echo "#########################################"
	@echo "########## U-BOOT-Compile DONE ##########"
	@echo "#########################################"

endef

define Package/uboot/install/default
	$(INSTALL_DIR) $(BIN_DIR)/uboot-$(BOARD)-$(1)
	$(CP) $(PKG_BUILD_DIR)/u-boot.img $(BIN_DIR)/uboot-$(BOARD)-$(1)/u-boot.img
	$(CP) $(PKG_BUILD_DIR)/MLO $(BIN_DIR)/uboot-$(BOARD)-$(1)/MLO
	$(CP) $(PKG_BUILD_DIR)/MLO.byteswap $(BIN_DIR)/uboot-$(BOARD)-$(1)/MLO.byteswap
endef

define Package/uboot/install/template
define Package/uboot-omap-$(1)/install
	$(call Package/uboot/install/default,$(2))
endef
endef

#define Package/uboot-ipq40xx-fwupgrade-tools/install
#	$(INSTALL_DIR) $(1)/usr/bin
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tools/mkimage $(1)/usr/bin
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tools/dumpimage $(1)/usr/bin
#endef

### SENAO ###
### enable features here
include Config.in

$(foreach u,$(UBOOTS), \
	$(eval $(call Package/uboot/install/template,$(u),$(u))) \
)

$(foreach u,$(UBOOTS), \
	$(eval $(call BuildUBootPackage,$(u))) \
	$(eval $(call BuildPackage,uboot-omap-$(u))) \
)
