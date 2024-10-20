define KernelPackage/crypto-mstar-aes
  TITLE:=MStar iNfinity AES engine
  DEPENDS:=@TARGET_sstar
  KCONFIG:=CONFIG_MS_CRYPTO
  FILES:= \
	  $(LINUX_DIR)/drivers/sstar/crypto/mdrv_crypto.ko \
	  $(LINUX_DIR)/drivers/sstar/crypto/mdrv_cryptodev.ko
  AUTOLOAD:=$(call AutoProbe,mdrv_crypto mdrv_cryptodev)
  $(call AddDepends/crypto)
endef

define KernelPackage/crypto-mstar-aes/description
 use the iNfinity AES engine for the CryptoAPI AES algorithm.
endef

$(eval $(call KernelPackage,crypto-mstar-aes))


define KernelPackage/mstar_mmc
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Mstar MMC/SD Card Interfaces Support
  DEPENDS:=@TARGET_sstar +kmod-mmc
  KCONFIG:= CONFIG_MS_SDMMC
  FILES:= $(LINUX_DIR)/drivers/sstar/sdmmc/kdrv_sdmmc.ko
  AUTOLOAD:=$(call AutoProbe,kdrv_sdmmc,1)
endef

define KernelPackage/mstar_mmc/description
 Enable SD/MMC Driver Support for MStar Product
endef

$(eval $(call KernelPackage,mstar_mmc))

OTHER_MENU:=Other modules

define KernelPackage/ms_notify
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Mstar NOTIFY driver
  KCONFIG:= CONFIG_MS_NOTIFY
  DEPENDS:=@TARGET_sstar
  FILES:= $(LINUX_DIR)/drivers/sstar/notify/ms_notify.ko
  AUTOLOAD:=$(call AutoLoad,45,ms_notify,1)
endef

define KernelPackage/ms_notify/description
    tristate "Mstar NOTIFY driver"
endef

$(eval $(call KernelPackage,ms_notify))
