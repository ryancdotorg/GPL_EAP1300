#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/BEAGLEBOARD
	NAME:=EBV BeagleBoard
	FEATURES:= usb ext4 targz
	DEFAULT_PACKAGES += kmod-usb2 kmod-usb2-omap kmod-fs-ext4
endef

define Profile/BEAGLEBOARD/Description
	Package set for the BEAGLEBOARD and similar devices.
	Without various USB-NET drivers for boards without Ethernet.
endef

$(eval $(call Profile,BEAGLEBOARD))
