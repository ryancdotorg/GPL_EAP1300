# 
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
_R:=\\033[31m
_G:=\\033[32m
_Y:=\\033[33m
_B:=\\033[34m
_N:=\\033[m
_LN:=\\033[1;36m

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/host.mk

override MAKEFLAGS=

PKG_OS:=$(word 2,$(subst -, ,$(shell $(HOSTCC) -dumpmachine)))
PKG_CPU:=$(word 1,$(subst -, ,$(shell $(HOSTCC) -dumpmachine)))

ifeq ($(CONFIG_EXTERNAL_TOOLCHAIN),)
SDK_NAME:=OpenWrt-SDK-$(BOARD)-for-$(PKG_OS)-$(PKG_CPU)-gcc-$(GCCV)_$(LIBC)-$(LIBCV)

toolchain:
else
SDK_NAME:=OpenWrt-SDK-$(BOARD)-for-$(PKG_OS)-$(PKG_CPU)-gcc-$(GNU_TARGET_NAME)

toolchain:
	@echo -e "$(_G)[ install toolchain ]$(_N)"
	@echo TOOLCHAIN_ROOT[$(CONFIG_TOOLCHAIN_ROOT)]
	@echo TOOLCHAIN_ROOT[$(notdir $(call qstrip,$(CONFIG_TOOLCHAIN_ROOT)))]
	mkdir -p $(SDK_BUILD_DIR)/staging_dir/$(notdir $(call qstrip,$(CONFIG_TOOLCHAIN_ROOT)))
	$(TAR) -c -C $(CONFIG_TOOLCHAIN_ROOT) . | \
		$(TAR) -xf - -C $(SDK_BUILD_DIR)/staging_dir/$(notdir $(call qstrip,$(CONFIG_TOOLCHAIN_ROOT)))

STAGING_TARGET_DIR:=staging_dir/target-$(GNU_TARGET_NAME)$(TOOLCHAIN_SUFFIX)$(if $(BUILD_SUFFIX),_$(BUILD_SUFFIX))
staging_dir:
	@echo -e "$(_G)[ install staging_dir/target-x ]$(_N)"
	mkdir -p $(SDK_BUILD_DIR)/$(STAGING_TARGET_DIR)
	$(TAR) -cf - -C $(TOPDIR) \
		staging_dir/host \
		$(STAGING_TARGET_DIR) | \
		tar -xf - -C $(SDK_BUILD_DIR)

endif
ifneq ($(CONFIG_EXTERNAL_KERNEL_TREE),)
KERNEL_DIR:=SENAO/kernel/$(notdir $(CONFIG_EXTERNAL_KERNEL_TREE))
kernel:
	@echo -e "$(_G)[ install kernel ]$(_N)"
	mkdir -p $(SDK_BUILD_DIR)/$(KERNEL_DIR)
	$(TAR) -cf - -C $(TOPDIR) \
        	--exclude=.pc --exclude=.svn --exclude=.git \
	        --exclude='boa-0*' --exclude='*.o' --exclude='*.so' \
		$(KERNEL_DIR) | \
		tar -xf - -C $(SDK_BUILD_DIR)
endif

SDK_BUILD_DIR:=$(BUILD_DIR)/$(SDK_NAME)
EXCLUDE_DIRS:=*/ccache \
	*/stamp \
	*/stampfiles \
	*/man \
	*/info

#SVN_PACKAGE_URL=$(shell svn info ../../package | awk '/^URL:/ { print $$2; }')

all: compile

$(BIN_DIR)/$(SDK_NAME).tar.bz2: clean staging_dir kernel
	@echo staging: $(STAGING_DIR)
	mkdir -p $(SDK_BUILD_DIR)/dl $(SDK_BUILD_DIR)/package
	$(CP) $(INCLUDE_DIR) $(SCRIPT_DIR) $(TOPDIR)/docs $(SDK_BUILD_DIR)/
	$(CP) $(TOPDIR)/config $(SDK_BUILD_DIR)/
	$(TAR) -cf - -C $(TOPDIR) \
		$(foreach exclude,$(EXCLUDE_DIRS),--exclude="$(exclude)") \
		--exclude="staging_dir/$(ARCH)" \
		staging_dir/host staging_dir/target-$(ARCH)$(ARCH_SUFFIX)_$(LIBC)-$(LIBCV)$(if $(CONFIG_arm),_eabi) \
		staging_dir/host staging_dir/target-$(GNU_TARGET_NAME) \
		staging_dir/toolchain-$(ARCH)$(ARCH_SUFFIX)_gcc-$(GCCV)_$(LIBC)-$(LIBCV)$(if $(CONFIG_arm),_eabi) \
		staging_dir/toolchain-$(GNU_TARGET_NAME) | \
		$(TAR) -xf - -C $(SDK_BUILD_DIR)
	mkdir -p $(SDK_BUILD_DIR)/target/linux
	$(CP) $(GENERIC_PLATFORM_DIR) $(PLATFORM_DIR) $(SDK_BUILD_DIR)/target/linux/
	rm -rf \
		$(SDK_BUILD_DIR)/target/linux/*/files \
		$(SDK_BUILD_DIR)/target/linux/*/patches
	$(CP) -L \
		$(TOPDIR)/LICENSE \
		$(TOPDIR)/rules.mk \
		$(TOPDIR)/.config \
		./files/Config.in \
		./files/Makefile \
		./files/README.SDK \
		$(SDK_BUILD_DIR)/
	$(CP) \
		./files/package/rules.mk \
		$(TOPDIR)/package/Makefile \
		$(SDK_BUILD_DIR)/package/
	$(if $(SVN_PACKAGE_URL),echo "src-svn trunk $(SVN_PACKAGE_URL)" \
		> $(SDK_BUILD_DIR)/feeds.conf.default,\
		-rm -f $(SDK_BUILD_DIR)/feeds.conf.default)
	cat $(TOPDIR)/feeds.conf.default \
		>> $(SDK_BUILD_DIR)/feeds.conf.default
	$(SED) 's,^# REVISION:=.*,REVISION:=$(REVISION),g' $(SDK_BUILD_DIR)/include/version.mk
	find $(SDK_BUILD_DIR) -name .git | $(XARGS) rm -rf
	find $(SDK_BUILD_DIR) -name .svn | $(XARGS) rm -rf
	find $(SDK_BUILD_DIR) -name CVS | $(XARGS) rm -rf
	sed -i 's/CONFIG_CCACHE=y//g' $(SDK_BUILD_DIR)/.config
	(cd $(BUILD_DIR); \
		tar cfj $@ $(SDK_NAME); \
	)

download:
prepare:
compile: $(BIN_DIR)/$(SDK_NAME).tar.bz2
install: compile

clean:
	rm -rf $(SDK_BUILD_DIR) $(BIN_DIR)/$(SDK_NAME).tar.bz2
