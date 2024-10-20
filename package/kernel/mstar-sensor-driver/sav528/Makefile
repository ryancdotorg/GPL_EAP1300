#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mstar-senor-driver
PKG_RELEASE:=3

include $(INCLUDE_DIR)/package.mk

define KernelPackage/mstar-senor-driver
  SUBMENU:=Other modules
  TITLE:=mstar-senor-driver
  PROPRIETARY=y
  DEPENDS:=@TARGET_sstar_infinity6
  FILES:= \
	  $(PKG_BUILD_DIR)/sensor_SC4238.ko \
	  $(PKG_BUILD_DIR)/sensor_ar0237_rgbir.ko \
	  $(PKG_BUILD_DIR)/sensor_c2390.ko \
	  $(PKG_BUILD_DIR)/sensor_imx230.ko \
	  $(PKG_BUILD_DIR)/sensor_imx274_hdr.ko \
	  $(PKG_BUILD_DIR)/sensor_imx291.ko \
	  $(PKG_BUILD_DIR)/sensor_imx307_hdr.ko \
	  $(PKG_BUILD_DIR)/sensor_imx317_hdr.ko \
	  $(PKG_BUILD_DIR)/sensor_imx323.ko \
	  $(PKG_BUILD_DIR)/sensor_imx327.ko \
	  $(PKG_BUILD_DIR)/sensor_nvp6124b.ko \
	  $(PKG_BUILD_DIR)/sensor_os05a10.ko \
	  $(PKG_BUILD_DIR)/sensor_os08a10.ko \
	  $(PKG_BUILD_DIR)/sensor_ps5250.ko \
	  $(PKG_BUILD_DIR)/sensor_ps5260.ko
  AUTOLOAD:=$(call AutoLoad,80,mstar-senor-driver,1)
  KCONFIG:=
endef

define KernelPackage/mstar-senor-driver/description
  Mstar mstar-senor-driver
endef

EXTRA_KCONFIG:= \
	CONFIG_MS_SENSORDRIVER=m \
	CONFIG_MS_SENSORDRIVER_INFINITY=m

EXTRA_CFLAGS= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \

EXTRA_CFLAGS+= \
	-I$(PKG_BUILD_DIR)/release/ipc/include \
	-I$(PKG_BUILD_DIR)/release/ipc/include/drivers/sensorif

EXTRA_CFLAGS+= -I$(LINUX_DIR)/drivers/sstar/include/ -I$(LINUX_DIR)/drivers/sstar/cpu/include/ -I$(LINUX_DIR)/drivers/sstar/miu/

EXTRA_CFLAGS+= \
	-I$(PKG_BUILD_DIR)/pub \
	-I$(PKG_BUILD_DIR)/inc

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./release/ $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		lib
endef

$(eval $(call KernelPackage,mstar-senor-driver))
