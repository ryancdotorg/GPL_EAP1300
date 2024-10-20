include $(TOPDIR)/rules.mk

PKG_NAME:=exif
PKG_VERSION:=0.6
PKG_RELEASE:=20

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CONFIGURE_OPTS+= \
 POPT_CFLAGS="$(TARGET_CFLAGS)" \
 POPT_LIBS="$(TARGET_LDFLAGS) -lpopt" \
 LIBEXIF_CFLAGS="$(TARGET_CFLAGS)" \
 LIBEXIF_LIBS="$(TARGET_LDFLAGS) -lexif"

define Package/exif
  TITLE:=Embedded exif
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libexif +libpopt
endef

define Package/exif/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/exif/exif $(1)/sbin/
endef

$(eval $(call BuildPackage,exif))
