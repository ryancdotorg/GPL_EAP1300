#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/sound-soc-omap
  TITLE:=OMAP SoC sound support
  KCONFIG:= \
	CONFIG_SND_OMAP_SOC
  FILES:=$(LINUX_DIR)/sound/soc/omap/snd-soc-omap.ko
  AUTOLOAD:=$(call AutoLoad,60,snd-soc-omap)
  DEPENDS:=@TARGET_omap24xx +kmod-sound-soc-core
  $(call AddDepends/sound)
endef

$(eval $(call KernelPackage,sound-soc-omap))


define KernelPackage/sound-soc-omap-mcbsp
  TITLE:=OMAP SoC MCBSP support
  KCONFIG:= \
	CONFIG_SND_OMAP_SOC_MCBSP
  FILES:=$(LINUX_DIR)/sound/soc/omap/snd-soc-omap-mcbsp.ko
  AUTOLOAD:=$(call AutoLoad,61,snd-soc-omap-mcbsp)
  DEPENDS:=@TARGET_omap24xx +kmod-sound-soc-omap
  $(call AddDepends/sound)
endef

$(eval $(call KernelPackage,sound-soc-omap-mcbsp))


define KernelPackage/sound-soc-n810
  TITLE:=Nokia n810 SoC sound support
  KCONFIG:= \
	CONFIG_SND_OMAP_SOC_N810
  FILES:= \
	$(LINUX_DIR)/sound/soc/codecs/snd-soc-tlv320aic3x.ko \
	$(LINUX_DIR)/sound/soc/omap/snd-soc-n810.ko
  AUTOLOAD:=$(call AutoLoad,65,snd-soc-tlv320aic3x snd-soc-n810)
  DEPENDS:=@TARGET_omap24xx +kmod-sound-soc-omap +kmod-sound-soc-omap-mcbsp
  $(call AddDepends/sound)
endef

$(eval $(call KernelPackage,sound-soc-n810))


define KernelPackage/n810bm
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Nokia N810 battery management driver
  DEPENDS:=@TARGET_omap24xx
  KCONFIG:=CONFIG_N810BM
  FILES:=$(LINUX_DIR)/drivers/cbus/n810bm.ko
  AUTOLOAD:=$(call AutoLoad,01,n810bm)
endef

define KernelPackage/n810bm/description
  Nokia N810 battery management driver.
  Controls battery power management and battery charging.
endef

$(eval $(call KernelPackage,n810bm))


define KernelPackage/musb-hdrc
  TITLE:=Support for Mentor Graphics silicon dual role USB
  KCONFIG:= \
	CONFIG_USB_MUSB_HDRC \
	CONFIG_MUSB_PIO_ONLY=n \
	CONFIG_USB_MUSB_OTG=y \
	CONFIG_USB_MUSB_DEBUG=y
  DEPENDS:=@TARGET_omap24xx
  FILES:=$(LINUX_DIR)/drivers/usb/musb/musb_hdrc.ko
  AUTOLOAD:=$(call AutoLoad,46,musb_hdrc)
  $(call AddDepends/usb)
endef

define KernelPackage/musb-hdrc/description
  Kernel support for Mentor Graphics silicon dual role USB device.
endef

$(eval $(call KernelPackage,musb-hdrc))


define KernelPackage/nop-usb-xceiv
  TITLE:=Support for USB OTG NOP transceiver
  KCONFIG:= \
	CONFIG_NOP_USB_XCEIV
  DEPENDS:=@TARGET_omap24xx
  FILES:=$(LINUX_DIR)/drivers/usb/otg/nop-usb-xceiv.ko
  AUTOLOAD:=$(call AutoLoad,45,nop-usb-xceiv)
  $(call AddDepends/usb)
endef

define KernelPackage/nop-usb-xceiv/description
  Support for USB OTG NOP transceiver
endef

$(eval $(call KernelPackage,nop-usb-xceiv))


define KernelPackage/usb-tahvo
  TITLE:=Support for Tahvo (Nokia n810) USB
  KCONFIG:= \
	CONFIG_CBUS_TAHVO_USB \
	CONFIG_CBUS_TAHVO_USB_HOST_BY_DEFAULT=n \
	CONFIG_USB_OHCI_HCD_OMAP1=y \
	CONFIG_USB_GADGET_DEBUG_FS=n
  DEPENDS:=@TARGET_omap24xx +kmod-usb-gadget
  FILES:=$(LINUX_DIR)/drivers/cbus/tahvo-usb.ko
  AUTOLOAD:=$(call AutoLoad,45,tahvo-usb)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-tahvo/description
  Kernel support for Nokia n810 USB OHCI controller.
endef

$(eval $(call KernelPackage,usb-tahvo))
