include $(TOPDIR)/rules.mk

PKG_NAME:=uboot-sstar-infinity5

PKG_VERSION:=2015.01

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR ?= $(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define uboot-sstar/infinity5
endef

define Package/uboot-sstar-infinity5
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_sstar_infinity5
  TITLE:=U-boot for SigmaStar infinity5
  PROPRIETARY=y
  REPO_URL:=git@atlantis.senao.com:senao_codeaurora/SigmaStar/SAV528/boot.git
endef
# SAV528/boot.git is the newer version
#REPO_URL:=git@atlantis.senao.com:senao_codeaurora/SigmaStar/SAV532/boot.git

### SENAO ###
UBOOT_MAKE_OPTS:=

UBOOT_CONFIG:=$(patsubst CONFIG_PACKAGE_uboot-sstar-infinity5-UBOOT_CONFIG_TARGET=%,%,\
        $(filter CONFIG_PACKAGE_uboot-sstar-infinity5-UBOOT_CONFIG_TARGET%,\
            $(shell cat $(TOPDIR)/.config)))

# uboot configuration
UBOOT_MAKE_OPTS+=$(patsubst CONFIG_PACKAGE_uboot-sstar-infinity5-MAKEOPTS_%,%,\
        $(filter CONFIG_PACKAGE_uboot-sstar-infinity5-MAKEOPTS_%,\
            $(shell cat $(TOPDIR)/.config | \
	    grep -v MAKEOPTS_SENAO_BOOTCOMMAND | \
	    grep -v MAKEOPTS_SENAO_MTDPARTS | sed s/\"//g)))

UBOOT_MAKE_OPTS+= $(patsubst CONFIG_PACKAGE_uboot-sstar-infinity5-MAKEOPTS_%,%,\
	$(shell cat $(TOPDIR)/.config | grep CONFIG_PACKAGE_uboot-sstar-infinity5-MAKEOPTS_SENAO_MTDPARTS))

UBOOT_MAKE_OPTS+= $(patsubst CONFIG_PACKAGE_uboot-sstar-infinity5-MAKEOPTS_%,%,\
	$(shell cat $(TOPDIR)/.config | grep CONFIG_PACKAGE_uboot-sstar-infinity5-MAKEOPTS_SENAO_BOOTCOMMAND))

# Senao definitions
UBOOT_MAKE_OPTS+=$(patsubst CONFIG_PACKAGE_uboot-sstar-infinity5-%,%,\
        $(filter %=y,\
            $(filter CONFIG_PACKAGE_uboot-sstar-infinity5-SENAO%,\
                $(shell cat $(TOPDIR)/.config | sed s/\"//g))))

ifneq ($(CONFIG_PACKAGE_uboot-sstar-infinity5-SENAO_SPINAND),)
DEFCONFIG=infinity5_spinand_defconfig
else
DEFCONFIG=infinity5_defconfig
endif

MAKE_VARS += $(UBOOT_MAKE_OPTS)

define Build/Configure
	@echo -e "\033[0;33m" UBOOT_MAKE_OPTS [$(UBOOT_MAKE_OPTS)] "\033[0m"
	($(foreach var,$(strip $(UBOOT_MAKE_OPTS)),$(var) ) \
	$(MAKE) -C $(PKG_BUILD_DIR) $(DEFCONFIG))
endef

define Build/Prepare
$(warning ---$(UBOOT_MAKE_OPTS)---)
	$(call DownloadMethod/sngit)
	-$(CP) $(TOPDIR)/SENAO/configs/sysgpio.h $(PKG_BUILD_DIR)/include
endef

define Package/uboot-sstar-infinity5/install
	$(INSTALL_DIR) $(1)/bin
ifneq ($(CONFIG_PACKAGE_uboot-sstar-infinity5-SENAO_SPINAND),)
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

$(eval $(call BuildPackage,uboot-sstar-infinity5))
