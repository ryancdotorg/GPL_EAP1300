#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/WN-AX1167GR_MT7621
	NAME:=WN-AX1167GR_MT7621
	PACKAGES:=\
		kmod-usb-core kmod-usb3 \
		kmod-ledtrig-usbdev
endef

define Profile/WN-AX1167GR_MT7621/Description
	set compatible with WN-AX1167GR_MT7621.
endef
$(eval $(call Profile,WN-AX1167GR_MT7621))
