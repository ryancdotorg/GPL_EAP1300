include $(TOPDIR)/rules.mk

PKG_NAME:=sstar_mi_libs
PKG_VERSION:=1.0.0
ifeq ($(CONFIG_TARGET_sstar_infinity6),y)
REPO_URL:=git@atlantis.senao.com:senao_codeaurora/SigmaStar/SAV528/release.git
else
REPO_URL:=git@atlantis.senao.com:senao_codeaurora/SigmaStar/SAV532/release.git
endif
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/sstar_mi_libs
  SECTION:=libs
  SUBMENU:=Libraries
  CATEGORY:=SigmaStar
  TITLE:=SAV528/SAV532 MI Libraries
  PROPRIETARY=y
  DEPENDS:=@TARGET_sstar +libstdcpp
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	(cd $(PKG_BUILD_DIR) && git archive --remote=$(REPO_URL) master | tar x)
endef

define Build/Compile
endef

ifeq ($(CONFIG_TARGET_sstar_infinity6),y)

  define Package/sstar_mi_libs/install
	$(INSTALL_DIR) $(1)/usr/lib
  ifeq ($(CONFIG_GCC_VERSION_8),y)
	$(CP) $(PKG_BUILD_DIR)/ipc/i6/009B/glibc/8.2.1/lib/dynamic/* $(1)/usr/lib
  else
	$(CP) $(PKG_BUILD_DIR)/ipc/i6/009B/glibc/4.8.3/lib/dynamic/* $(1)/usr/lib
  endif
  endef

else

  define Package/sstar_mi_libs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/ipc/i5/007B/glibc/4.8.3/lib/dynamic/* $(1)/usr/lib
  endef

endif

$(eval $(call BuildPackage,sstar_mi_libs))
