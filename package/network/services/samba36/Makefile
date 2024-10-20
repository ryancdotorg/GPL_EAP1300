#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=samba
PKG_VERSION:=3.6.25
PKG_RELEASE:=5

PKG_SOURCE_URL:=http://ftp.samba.org/pub/samba \
	http://ftp.samba.org/pub/samba/stable
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MD5SUM:=76da2fa64edd94a0188531e7ecb27c4e

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=COPYING

PKG_MAINTAINER:=Felix Fietkau <nbd@openwrt.org>

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

MAKE_PATH:=source3
CONFIGURE_PATH:=source3

PKG_BUILD_BIN:=$(PKG_BUILD_DIR)/$(MAKE_PATH)/bin

define Package/samba36-server
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Samba 3.6 SMB/CIFS server
  URL:=http://www.samba.org/
  DEPENDS:=+USE_GLIBC:librt $(ICONV_DEPENDS) +libuuid +SAMBA36_SUPPORT_ADS:krb5-libs +SAMBA36_SUPPORT_ADS:libopenldap
endef

define Package/samba36-client
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Samba 3.6 SMB/CIFS client
  URL:=http://www.samba.org/
  DEPENDS:=+libreadline +libncurses +SAMBA36_SUPPORT_ADS:krb5-libs +SAMBA36_SUPPORT_ADS:libopenldap +SAMBA36_SUPPORT_ADS:libuuid
endef

define Package/samba36-server/config
	config SAMBA36_MAX_DEBUG_LEVEL
		int "Maximum level of compiled-in debug messages"
		depends on PACKAGE_samba36-server || PACKAGE_samba36-client
		default -1

	config SAMBA36_SUPPORT_ADS
		select PACKAGE_samba36-server
		select PACKAGE_samba36-client
		bool "Samba36 support ADS domain functions"
		default n

menu "Samba36 ads function configuration"
	depends on SAMBA36_SUPPORT_ADS
choice
prompt "Samba36 support ADS functions"
	config SAMBA36_SUPPORT_ADS_ONLY
		bool "Support ADS domain functions only"
		default y

	config SAMBA36_SUPPORT_FULL_AND_ADS
		bool "Support full and ADS domain functions"
		default n
endchoice
endmenu

endef

define Package/samba36-server/description
 The Samba software suite is a collection of programs that implements the
 SMB protocol for UNIX systems, allowing you to serve files and printers to
 Windows, NT, OS/2 and DOS clients. This protocol is sometimes also referred
 to as the LanManager or Netbios protocol.
endef

TARGET_CFLAGS += -DMAX_DEBUG_LEVEL=$(CONFIG_SAMBA36_MAX_DEBUG_LEVEL) -D__location__=\\\"\\\" -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections

ifeq ($(CONFIG_SAMBA36_SUPPORT_ADS),y)
TARGET_CFLAGS += -DACTIVE_DIRECTORY -DNETLOGON_SUPPORT -DSAMR_SUPPORT -DLSA_SUPPORT
CONFIGURE_VARS += \
	ac_cv_lib_attr_getxattr=no \
	ac_cv_search_getxattr=no \
	ac_cv_file__proc_sys_kernel_core_pattern=yes \
	libreplace_cv_HAVE_C99_VSNPRINTF=yes \
	libreplace_cv_HAVE_GETADDRINFO=yes \
	libreplace_cv_HAVE_IFACE_IFCONF=yes \
	LINUX_LFS_SUPPORT=yes \
	samba_cv_CC_NEGATIVE_ENUM_VALUES=yes \
	samba_cv_HAVE_GETTIMEOFDAY_TZ=yes \
	samba_cv_HAVE_IFACE_IFCONF=yes \
	samba_cv_HAVE_KERNEL_OPLOCKS_LINUX=yes \
	samba_cv_HAVE_SECURE_MKSTEMP=yes \
	samba_cv_HAVE_WRFILE_KEYTAB=no \
	samba_cv_USE_SETREUID=yes \
	samba_cv_USE_SETRESUID=yes \
	samba_cv_have_setreuid=yes \
	samba_cv_have_setresuid=yes \
	ac_cv_header_libunwind_h=no \
	ac_cv_header_zlib_h=no \
	samba_cv_zlib_1_2_3=no \
	ac_cv_path_PYTHON="" \
	ac_cv_path_PYTHON_CONFIG="" \
	smb_krb5_cv_enctype_to_string_takes_krb5_context_arg=no \
	smb_krb5_cv_enctype_to_string_takes_size_t_arg=yes \
	enable_developer=yes \
	LINK_LIBWBCLIENT=STATIC

CONFIGURE_ARGS += \
	--exec-prefix=/usr \
	--prefix=/ \
	--disable-avahi \
	--disable-cups \
	--disable-pie \
	--disable-relro \
	--disable-swat \
	--with-libiconv="$(ICONV_PREFIX)" \
	--with-codepagedir=/etc/samba \
	--with-configdir=/etc/samba \
	--with-included-iniparser \
	--with-included-popt \
	--with-lockdir=/var/lock \
	--with-logfilebase=/var/log \
	--with-nmbdsocketdir=/var/nmbd \
	--with-piddir=/var/run \
	--with-privatedir=/etc/samba \
	--with-sendfile-support \
	--without-acl-support \
	--without-cluster-support \
	--without-pam \
	--without-libtdb \
	--without-libtalloc \
	--without-libnetapi \
	--without-libsmbclient \
	--without-libsmbsharemodes \
	--without-libtevent \
	--without-libaddns \
	--with-ads \
	--with-winbind \
	--disable-external-libtalloc

ifneq ($(CONFIG_SAMBA36_MAX_DEBUG_LEVEL),-1)
CONFIGURE_ARGS += \
	--enable-debug=yes
endif

else
CONFIGURE_VARS += \
	ac_cv_lib_attr_getxattr=no \
	ac_cv_search_getxattr=no \
	ac_cv_file__proc_sys_kernel_core_pattern=yes \
	libreplace_cv_HAVE_C99_VSNPRINTF=yes \
	libreplace_cv_HAVE_GETADDRINFO=yes \
	libreplace_cv_HAVE_IFACE_IFCONF=yes \
	LINUX_LFS_SUPPORT=yes \
	samba_cv_CC_NEGATIVE_ENUM_VALUES=yes \
	samba_cv_HAVE_GETTIMEOFDAY_TZ=yes \
	samba_cv_HAVE_IFACE_IFCONF=yes \
	samba_cv_HAVE_KERNEL_OPLOCKS_LINUX=yes \
	samba_cv_HAVE_SECURE_MKSTEMP=yes \
	samba_cv_HAVE_WRFILE_KEYTAB=no \
	samba_cv_USE_SETREUID=yes \
	samba_cv_USE_SETRESUID=yes \
	samba_cv_have_setreuid=yes \
	samba_cv_have_setresuid=yes \
	ac_cv_header_zlib_h=no \
	samba_cv_zlib_1_2_3=no \
	ac_cv_path_PYTHON="" \
	ac_cv_path_PYTHON_CONFIG=""

CONFIGURE_ARGS += \
	--exec-prefix=/usr \
	--prefix=/ \
	--disable-avahi \
	--disable-cups \
	--disable-pie \
	--disable-relro \
	--disable-static \
	--disable-swat \
	--disable-shared-libs \
	--with-libiconv="$(ICONV_PREFIX)" \
	--with-codepagedir=/etc/samba \
	--with-configdir=/etc/samba \
	--with-included-iniparser \
	--with-included-popt \
	--with-lockdir=/var/lock \
	--with-logfilebase=/var/log \
	--with-nmbdsocketdir=/var/nmbd \
	--with-piddir=/var/run \
	--with-privatedir=/etc/samba \
	--with-sendfile-support \
	--without-acl-support \
	--without-cluster-support \
	--without-ads \
	--without-krb5 \
	--without-ldap \
	--without-pam \
	--without-winbind \
	--without-libtdb \
	--without-libtalloc \
	--without-libnetapi \
	--without-libsmbclient \
	--without-libsmbsharemodes \
	--without-libtevent \
	--without-libaddns \
	--with-shared-modules=pdb_tdbsam,pdb_wbc_sam,idmap_nss,nss_info_template,auth_winbind,auth_wbc,auth_domain
endif




MAKE_FLAGS += DYNEXP= PICFLAG= MODULES=

ifneq ($(CONFIG_SAMBA36_SUPPORT_ADS_ONLY),y)
define Package/samba36-server/conffiles
/etc/config/samba
/etc/samba/smb.conf.template
endef
endif

define Package/samba36-server/install
	$(INSTALL_DIR) $(1)/usr/sbin
ifneq ($(CONFIG_SAMBA36_SUPPORT_ADS_ONLY),y)
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/samba.config $(1)/etc/config/samba
	$(INSTALL_DIR) $(1)/etc/samba
	$(INSTALL_DATA) ./files/smb.conf.template $(1)/etc/samba
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/codepages/lowcase.dat $(1)/etc/samba
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/codepages/upcase.dat $(1)/etc/samba
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/codepages/valid.dat $(1)/etc/samba
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/samba.init $(1)/etc/init.d/samba
	$(INSTALL_BIN) $(PKG_BUILD_BIN)/samba_multicall $(1)/usr/sbin
	$(LN) samba_multicall $(1)/usr/sbin/smbd
	$(LN) samba_multicall $(1)/usr/sbin/nmbd
	$(LN) samba_multicall $(1)/usr/sbin/smbpasswd
endif
ifeq ($(CONFIG_SAMBA36_SUPPORT_ADS),y)
	$(INSTALL_BIN) $(PKG_BUILD_BIN)/winbindd $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_BIN)/ntlm_auth $(1)/usr/sbin
endif
endef

define Package/samba36-client/install
	$(INSTALL_DIR) $(1)/usr/sbin
ifneq ($(CONFIG_SAMBA36_SUPPORT_ADS_ONLY),y)
	$(INSTALL_BIN) $(PKG_BUILD_BIN)/smbclient $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_BIN)/nmblookup $(1)/usr/sbin
endif
ifeq ($(CONFIG_SAMBA36_SUPPORT_ADS),y)
	$(INSTALL_BIN) $(PKG_BUILD_BIN)/net $(1)/usr/sbin
endif
endef

$(eval $(call BuildPackage,samba36-client))
$(eval $(call BuildPackage,samba36-server))

