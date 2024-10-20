#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

-include $(TMP_DIR)/.packagefeeds

FEEDS_INSTALLED:=$(notdir $(wildcard $(TOPDIR)/package/feeds/*))
FEEDS_AVAILABLE:=$(sort $(FEEDS_INSTALLED) $(shell $(SCRIPT_DIR)/feeds list -n 2>/dev/null))
FEEDS_ENABLED:=$(foreach feed,$(FEEDS_INSTALLED),$(if $(CONFIG_FEED_$(feed)),$(feed)))
FEEDS_DISABLED:=$(filter-out $(FEEDS_ENABLED),$(FEEDS_AVAILABLE))

PKG_CONFIG_DEPENDS += \
	CONFIG_PER_FEED_REPO \
	CONFIG_PER_FEED_REPO_ADD_DISABLED \
	CONFIG_PER_FEED_REPO_ADD_COMMENTED \
	$(foreach feed,$(FEEDS_INSTALLED),CONFIG_FEED_$(feed))

PACKAGE_SUBDIRS=$(PACKAGE_DIR)
ifneq ($(CONFIG_PER_FEED_REPO),)
  PACKAGE_SUBDIRS += $(OUTPUT_DIR)/packages/$(ARCH_PACKAGES)/base
  PACKAGE_SUBDIRS += $(foreach FEED,$(FEEDS_AVAILABLE),$(OUTPUT_DIR)/packages/$(ARCH_PACKAGES)/$(FEED))
endif

opkg_package_files = $(wildcard \
	$(foreach dir,$(PACKAGE_SUBDIRS), \
	  $(foreach pkg,$(1), $(dir)/$(pkg)_*.ipk)))

# 1: package name
define FeedPackageDir
$(strip $(if $(CONFIG_PER_FEED_REPO), \
  $(abspath $(PACKAGE_DIR)/$(if $(Package/$(1)/feed),$(Package/$(1)/feed),)), \
  $(PACKAGE_DIR)))
endef

# 1: destination file
define FeedSourcesAppend
( \
  $(strip $(if $(CONFIG_PER_FEED_REPO), \
	$(foreach feed,base $(FEEDS_ENABLED),echo "src/gz %n_$(feed) %U/$(feed)";) \
	$(if $(CONFIG_PER_FEED_REPO_ADD_DISABLED), \
		$(foreach feed,$(FEEDS_DISABLED),echo "$(if $(CONFIG_PER_FEED_REPO_ADD_COMMENTED),# )src/gz %n_$(feed) %U/$(feed)";)) \
  , \
	echo "src/gz %n %U"; \
  )) \
) >> $(1)
endef

# 1: package name
define GetABISuffix
$(if $(filter-out kmod-%,$(1)),$(if $(Package/$(1)/abiversion),$(if $(filter %0 %1 %2 %3 %4 %5 %6 %7 %8 %9,$(1)),-)$(Package/$(1)/abiversion)))
endef
