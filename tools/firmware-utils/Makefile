#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME := firmware-utils

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/kernel.mk

define cc
	$(HOSTCC) $(HOST_CFLAGS) -include endian.h $(HOST_LDFLAGS) -o $(HOST_BUILD_DIR)/bin/$(firstword $(1)) $(foreach src,$(1),src/$(src).c) $(2)
endef

define Host/Compile
	mkdir -p $(HOST_BUILD_DIR)/bin
	$(call cc,addpattern)
	$(call cc,asustrx)
	$(call cc,trx)
	$(call cc,motorola-bin)
	$(call cc,dgfirmware)
	$(call cc,mksenaofw md5)
	$(call cc,trx2usr)
	$(call cc,ptgen)
	$(call cc,airlink)
	$(call cc,srec2bin)
	$(call cc,mkmylofw)
	$(call cc,mkcsysimg)
	$(call cc,mkzynfw)
	$(call cc,lzma2eva,-lz)
	$(call cc,mkcasfw)
	$(call cc,mkfwimage,-lz -Wall)
	$(call cc,mkfwimage2,-lz)
	$(call cc,imagetag imagetag_cmdline cyg_crc32)
	$(call cc,add_header)
	$(call cc,makeamitbin)
	$(call cc,encode_crc)
	$(call cc,nand_ecc)
	$(call cc,mkplanexfw sha1)
	$(call cc,mktplinkfw md5)
	$(call cc,mktplinkfw2 md5)
	$(call cc,tplink-safeloader md5, -Wall)
	$(call cc,pc1crypt)
	$(call cc,osbridge-crc)
	$(call cc,wrt400n cyg_crc32)
	$(call cc,mkdniimg)
	$(call cc,mktitanimg)
	$(call cc,mkchkimg)
	$(call cc,mkzcfw cyg_crc32)
	$(call cc,spw303v)
	$(call cc,zyxbcm)
	$(call cc,trx2edips)
	$(call cc,xorimage)
	$(call cc,buffalo-enc buffalo-lib, -Wall)
	$(call cc,buffalo-tag buffalo-lib, -Wall)
	$(call cc,buffalo-tftp buffalo-lib, -Wall)
	$(call cc,mkwrgimg md5, -Wall)
	$(call cc,mkedimaximg)
	$(call cc,mkbrncmdline)
	$(call cc,mkbrnimg)
	$(call cc,mkdapimg)
	$(call cc, mkcameofw, -Wall)
	$(call cc,seama md5)
	$(call cc,oseama md5, -Wall)
	$(call cc,fix-u-media-header cyg_crc32,-Wall)
	$(call cc,hcsmakeimage bcmalgo)
	$(call cc,mkporayfw, -Wall)
	$(call cc,mkhilinkfw, -lcrypto)
	$(call cc,mkdcs932, -Wall)
	$(call cc,mkheader_gemtek,-lz)
	$(call cc,mkrtn56uimg, -lz)
	$(call cc,dgn3500sum)
	$(call cc,edimax_fw_header, -Wall)
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bin/* $(STAGING_DIR_HOST)/bin/
	$(CP) src/mkmbnimg.py $(HOST_BUILD_DIR)/bin/
endef

$(eval $(call HostBuild))
