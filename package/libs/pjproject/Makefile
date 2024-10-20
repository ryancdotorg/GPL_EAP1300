# Makefile of the package.
include $(TOPDIR)/rules.mk

PKG_NAME:=pjproject
PKG_VERSION:=2.7.2
PKG_RELEASE:=0

PKG_SOURCE:=pjproject-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://www.pjsip.org/release/$(PKG_VERSION)/
PKG_MD5:=fa3f0bc098c4bff48ddd92db1c016a7a

PKG_INSTALL:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/pjproject-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=lib
	CATEGORY:=Libraries
	URL:=http://www.pjsip.org/
	MAINTAINER:=John Crispin <blogic@openwrt.org>
	TITLE:=$(PKG_NAME)-$(PKG_VERSION)
	DEPENDS:=+libuuid +libpthread +librt +libpcre +lstdc++ 
endef 

CONFIGURE_ARGS += \
	--enable-shared \
	--disable-sound \
	--disable-video \
	--disable-oss \
	--enable-ext-sound \
	--disable-small-filter \
	--disable-large-filter \
	--disable-speex-aec \
	--disable-g711-codec \
	--disable-l16-codec \
	--disable-gsm-codec \
	--disable-g722-codec \
	--disable-g7221-codec \
	--disable-speex-codec \
	--disable-ilbc-codec \
	--disable-sdl \
	--disable-ffmpeg \
	--disable-v4l2 \
	--disable-ssl \
	--disable-opencore-amr \
	--disable-silk

CONFIGURE_ARGS += \
	--disable-libwebrtc \
	--prefix=/pj_lib 

MAKEOPTS:= \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(TARGET_CPPFLAGS) $(EXTRA_CPPFLAGS)" \
	CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(TARGET_CPPFLAGS) $(EXTRA_CPPFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS) -lc $(LIBGCC_S) -lm -lpcre -lstdc++ -L$(dir $(LIBGCC_A))" \
	CROSS_COMPILE="$(TARGET_CROSS)"

TARGET_HOST:=$(if $(TARGET_CROSS),$(TARGET_CROSS),$(OPTIMIZE_FOR_CPU)-openwrt-linux$(if $(TARGET_SUFFIX),-$(TARGET_SUFFIX)))

define Build/Configure
	@echo "============================="
	@echo "$(PKG_NAME) V$(PKG_VERSION) Configuration!!!!"
	@echo "============================="
	(cd $(PKG_BUILD_DIR); \
		chmod 777 configure aconfigure; \
		autoconf aconfigure.ac > aconfigure; \
		touch pjlib/include/pj/config_site.h; \
		make dep ; \
		./configure --host=$(TARGET_HOST) $(CONFIGURE_ARGS) \
	)
endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)
endef

define Build/Compile
	CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(TARGET_CPPFLAGS) $(EXTRA_CPPFLAGS)" \
	CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(TARGET_CPPFLAGS) $(EXTRA_CPPFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS) -lc $(LIBGCC_S) -lm -lstdc++ -L$(dir $(LIBGCC_A))" \
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/
	$(CP) $(PKG_BUILD_DIR)/pjnath/lib/ $(1)/
	$(CP) $(PKG_BUILD_DIR)/pjlib/lib/ $(1)/
	$(CP) $(PKG_BUILD_DIR)/pjlib-util/lib/ $(1)/
	$(CP) $(PKG_BUILD_DIR)/pjmedia/lib/ $(1)/
	$(CP) $(PKG_BUILD_DIR)/pjsip/lib/ $(1)/
	$(CP) $(PKG_BUILD_DIR)/third_party/lib/ $(1)/
	cp $(TOOLCHAIN_DIR)/lib/libstdc++.so.6 $(1)/lib
	mkdir $(1)/bin/
	cp $(PKG_BUILD_DIR)/pjsip-apps/bin/pjsua-arm-openwrt-linux-gnu $(1)/bin/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/
	$(CP) $(PKG_BUILD_DIR)/pjnath/lib/ $(1)/usr/
	$(CP) $(PKG_BUILD_DIR)/pjlib/lib/ $(1)/usr/
	$(CP) $(PKG_BUILD_DIR)/pjlib-util/lib/ $(1)/usr/
	$(CP) $(PKG_BUILD_DIR)/pjmedia/lib/ $(1)/usr/
	$(CP) $(PKG_BUILD_DIR)/pjsip/lib/ $(1)/usr/
	$(CP) $(PKG_BUILD_DIR)/third_party/lib/ $(1)/usr/
	cp $(TOOLCHAIN_DIR)/lib/libstdc++.so.6 $(1)/usr/lib
	mkdir $(1)/usr/bin/
	cp $(PKG_BUILD_DIR)/pjsip-apps/bin/pjsua-arm-openwrt-linux-gnu $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
