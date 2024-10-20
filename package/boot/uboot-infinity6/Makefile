include $(TOPDIR)/rules.mk

PKG_NAME:=uboot-sstar-infinity6

PKG_VERSION:=2015.01

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR ?= $(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define uboot-sstar/infinity6
endef

define Package/uboot-sstar-infinity6
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_sstar_infinity6
  TITLE:=U-boot for SigmaStar infinity6
  PROPRIETARY=y
  REPO_URL:=git@atlantis.senao.com:senao_codeaurora/SigmaStar/SAV528/boot.git
endef

### SENAO ###
UBOOT_MAKE_OPTS:=

UBOOT_CONFIG:=$(patsubst CONFIG_PACKAGE_uboot-sstar-infinity6-UBOOT_CONFIG_TARGET=%,%,\
        $(filter CONFIG_PACKAGE_uboot-sstar-infinity6-UBOOT_CONFIG_TARGET%,\
            $(shell cat $(TOPDIR)/.config)))

# uboot configuration
#UBOOT_MAKE_OPTS+=$(patsubst CONFIG_PACKAGE_uboot-sstar-infinity6-MAKEOPTS_%,%,\
        $(filter CONFIG_PACKAGE_uboot-sstar-infinity6-MAKEOPTS_%,\
            #$(shell cat $(TOPDIR)/.config | sed s/\"//g)))
UBOOT_MAKE_OPTS+= $(patsubst CONFIG_PACKAGE_uboot-sstar-infinity6-MAKEOPTS_%,%,\
	$(shell cat $(TOPDIR)/.config | grep CONFIG_PACKAGE_uboot-sstar-infinity6-MAKEOPTS))

# Senao definitions
UBOOT_MAKE_OPTS+=$(patsubst CONFIG_PACKAGE_uboot-sstar-infinity6-%,%,\
        $(filter %=y,\
            $(filter CONFIG_PACKAGE_uboot-sstar-infinity6-SENAO%,\
                $(shell cat $(TOPDIR)/.config | sed s/\"//g))))

ifneq ($(CONFIG_PACKAGE_uboot-sstar-infinity6-SENAO_SPINAND),)
DEFCONFIG=infinity6_spinand_defconfig
else
DEFCONFIG=infinity6_defconfig
endif

MAKE_VARS += $(UBOOT_MAKE_OPTS)

define Build/Configure
	@echo -e "\033[0;33m" UBOOT_MAKE_OPTS [$(UBOOT_MAKE_OPTS)] "\033[0m"
	($(foreach var,$(strip $(UBOOT_MAKE_OPTS)),$(var) ) \
	$(MAKE) -C $(PKG_BUILD_DIR) $(DEFCONFIG))
endef

define Build/Prepare
	$(call DownloadMethod/sngit)
	-$(CP) $(TOPDIR)/SENAO/configs/sysgpio.h $(PKG_BUILD_DIR)/include
endef

define Package/uboot-sstar-infinity6/install
	$(INSTALL_DIR) $(1)/bin
ifneq ($(CONFIG_PACKAGE_uboot-sstar-infinity6-SENAO_SPINAND),)
	@echo "Flash type: SPI NAND"
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/u-boot_spinand.xz.img.bin $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_UBOOT_NAND_SUFFIX)
	-$(INSTALL_BIN) $(PKG_BUILD_DIR)/u-boot_spinand.xz.img.bin /tftpboot/$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_UBOOT_NAND_SUFFIX)
else
	@echo "Flash type: NOR"
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/u-boot.xz.img.bin $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_UBOOT_NOR_SUFFIX)
	-$(INSTALL_BIN) $(PKG_BUILD_DIR)/u-boot.xz.img.bin /tftpboot/$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_UBOOT_NOR_SUFFIX)
endif
endef

### SENAO ###
### enable features here
include Config.in

$(eval $(call BuildPackage,uboot-sstar-infinity6))
