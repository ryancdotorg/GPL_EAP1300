### SENAO ### 
### redefine Image/Checksum(ref. image.mk) ###
# To generate SENAO custom image besides single all image
# 1. wget some mandatory binary files
# 2. combine binary files to three combination(loader-s, fw-s, and all)

### 2018.4.18 SENAO Goerge ###
# Reference Document
# (80-YB481-10 Rev. B QCA_Networking_2017.SPF.8.0 ED7: Section 2.3.5 Generate a complte firmware image)
# (80-YA728-4 Rev. M IPQ807x SoC Software User Guide: Section 3 Flash memory layout)

ifneq ("$(wildcard /etc/fedora-release)","")
PYTHON=/usr/bin/python
else
PYTHON=python
endif

build_dir=$(BIN_DIR)/build

# ex: ipq807x
SOC_ARCH=$(shell grep 'SOC' $(SN_CONFIGS_DIR)/ipq-image-config/config.xml | awk -F ">" '{print $$2}' | awk -F "<" '{print $$1}')
ifeq ("$(SOC_ARCH)","")
SOC_ARCH:=$(SUBTARGET)
endif

NAND_SBL=sbl1_nand.mbn
NOR_SBL=sbl1_nor.mbn
EMMC_SBL=sbl1_emmc.mbn
TRUSTZONE=tz.mbn
RESOURCE_POWER_MANAGER=rpm.mbn
DEVCFG=devcfg_nosmmu.mbn
PARTITION_TOOLS=partition_tool
BOOTCONFIG_TOOLS=bootconfig_tool
WIFI_FW_IMG=wifi_fw_ubi.img wifi_fw_squashfs.img
IPQ_FILES=$(NAND_SBL) $(NOR_SBL) $(EMMC_SBL) $(TRUSTZONE) $(RESOURCE_POWER_MANAGER) $(DEVCFG) $(PARTITION_TOOLS) $(BOOTCONFIG_TOOLS) $(WIFI_FW_IMG)

prefix_output=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-
#loader=$(prefix_output)$(subst nor,nand,$(subst -s,,$(FW_UBOOT_SUFFIX)))
#nand_s_output=$(prefix_output)$(FW_UBOOT_SUFFIX)

# file name : nor
nor_loader=$(prefix_output)$(subst -s,,$(FW_FIT_NOR_UBOOT_SUFFIX))
nor_fit_loader=$(prefix_output)$(FW_FIT_NOR_UBOOT_SUFFIX)
nor_fit_kr=$(prefix_output)$(FW_FIT_NOR_KR_SUFFIX)
nor_fit_all=$(prefix_output)$(FW_FIT_NOR_ALL_SUFFIX)

# file name : nand
nand_loader=$(prefix_output)$(subst -s,,$(FW_FIT_NAND_UBOOT_SUFFIX))
nand_fit_loader=$(prefix_output)$(FW_FIT_NAND_UBOOT_SUFFIX)
nand_fit_kr=$(prefix_output)$(FW_FIT_NAND_KR_SUFFIX)
nand_fit_all=$(prefix_output)$(FW_FIT_NAND_ALL_SUFFIX)

# file name : nand-4k
nand_4k_loader=$(prefix_output)$(subst -s,,$(FW_FIT_NAND_4K_UBOOT_SUFFIX))
nand_4k_fit_loader=$(prefix_output)$(FW_FIT_NAND_4K_UBOOT_SUFFIX)
nand_4k_fit_kr=$(prefix_output)$(FW_FIT_NAND_4K_KR_SUFFIX)
nand_4k_fit_all=$(prefix_output)$(FW_FIT_NAND_4K_ALL_SUFFIX)

# file name : nor + nand
nornand_loader=$(prefix_output)$(subst -s,,$(FW_FIT_NORNAND_UBOOT_SUFFIX))
nornand_fit_loader=$(prefix_output)$(FW_FIT_NORNAND_UBOOT_SUFFIX)
nornand_fit_kr=$(prefix_output)$(FW_FIT_NORNAND_KR_SUFFIX)
nornand_fit_all=$(prefix_output)$(FW_FIT_NORNAND_ALL_SUFFIX)

# file name : nor + nand-4k
nornand_4k_loader=$(prefix_output)$(subst -s,,$(FW_FIT_NORNAND_4K_UBOOT_SUFFIX))
nornand_4k_fit_loader=$(prefix_output)$(FW_FIT_NORNAND_4K_UBOOT_SUFFIX)
nornand_4k_fit_kr=$(prefix_output)$(FW_FIT_NORNAND_4K_KR_SUFFIX)
nornand_4k_fit_all=$(prefix_output)$(FW_FIT_NORNAND_4K_ALL_SUFFIX)

ifeq ($(IMG_TYPE),$(filter $(IMG_TYPE), emmc))
# file name : emmc
emmc_loader=$(prefix_output)$(subst -s,,$(FW_FIT_EMMC_UBOOT_SUFFIX))
emmc_fit_loader=$(prefix_output)$(FW_FIT_EMMC_UBOOT_SUFFIX)
emmc_fit_kr=$(prefix_output)$(FW_FIT_EMMC_KR_SUFFIX)
emmc_fit_all=$(prefix_output)$(FW_FIT_EMMC_ALL_SUFFIX)

# assign the target name
loader=$(if $(findstring emmc,$(IMG_TYPE)),$(emmc_loader),$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_loader),$(if $(findstring nor,$(IMG_TYPE)),$(nor_loader),$(nand_loader))))
fit_loader=$(if $(findstring emmc,$(IMG_TYPE)),$(emmc_fit_loader),$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_loader),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_loader),$(nand_fit_loader))))
fit_kr=$(if $(findstring emmc,$(IMG_TYPE)),$(emmc_fit_kr),$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_kr),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_kr),$(nand_fit_kr))))
fit_all=$(if $(findstring emmc,$(IMG_TYPE)),$(emmc_fit_all),$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_all),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_all),$(nand_fit_all))))
else ifeq ($(IMG_TYPE),norplusemmc)
# file name : nor + emmc
noremmc_loader=$(prefix_output)$(subst -s,,$(FW_FIT_NOREMMC_UBOOT_SUFFIX))
noremmc_fit_loader=$(prefix_output)$(FW_FIT_NOREMMC_UBOOT_SUFFIX)
noremmc_fit_kr=$(prefix_output)$(FW_FIT_NOREMMC_KR_SUFFIX)
noremmc_fit_all=$(prefix_output)$(FW_FIT_NOREMMC_ALL_SUFFIX)

# assign the target name
loader=$(if $(findstring norplusemmc,$(IMG_TYPE)),$(noremmc_loader),$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_loader),$(if $(findstring nor,$(IMG_TYPE)),$(nor_loader),$(nand_loader))))
fit_loader=$(if $(findstring norplusemmc,$(IMG_TYPE)),$(noremmc_fit_loader),$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_loader),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_loader),$(nand_fit_loader))))
fit_kr=$(if $(findstring norplusemmc,$(IMG_TYPE)),$(noremmc_fit_kr),$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_kr),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_kr),$(nand_fit_kr))))
fit_all=$(if $(findstring norplusemmc,$(IMG_TYPE)),$(noremmc_fit_all),$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_all),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_all),$(nand_fit_all))))

else ifeq ($(IMG_TYPE),nand-4k)
loader=$(nand_4k_loader)
fit_loader=$(nand_4k_fit_loader)
fit_kr=$(nand_4k_fit_kr)
fit_all=$(nand_4k_fit_all)
else ifeq ($(IMG_TYPE),norplusnand-4k)
loader=$(nornand_4k_loader)
fit_loader=$(nornand_4k_fit_loader)
fit_kr=$(nornand_4k_fit_kr)
fit_all=$(nornand_4k_fit_all)
else
# assign the target name
loader=$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_loader),$(if $(findstring nor,$(IMG_TYPE)),$(nor_loader),$(nand_loader)))
fit_loader=$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_loader),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_loader),$(nand_fit_loader)))
fit_kr=$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_kr),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_kr),$(nand_fit_kr)))
fit_all=$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_all),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_all),$(nand_fit_all)))
endif

# IMG_TYPE=nor nand norplusnand emmc norplusemmc #move to sysProductInfo.mk of vendor

num_of_en=$(shell echo $(IMG_TYPE) | wc -w)

define Image/Checksum
	( cd ${BIN_DIR} ; \
		$(FIND) -maxdepth 1 -type f \! -name 'md5sums'  -printf "%P\n" | sort | xargs \
		md5sum --binary > md5sums \
	)
	$(call Prepare)
	$(call GenCDT)
	$(call GenBoot)
	$(call GenPartition)
	$(call GenMbn)
	$(call PackFitKRimg)
	$(call PackFitLoader)
	$(call PackAllImg)
	$(call CopyRenameImg)
endef

define CheckConfig
	@echo -e "\e[32m!!! check config !!!\e[0m"
	@if [ "$(num_of_en)" != "1" ]; then \
		echo "Failed!!"; \
		echo "Please select the firmware type, one at a time."; \
		exit 1; \
	fi
	@echo "OK!!"
	@echo "$(IMG_TYPE) selected"
endef


define GetFtpFile
	wget -nv -N ftp://ftpuser:senao123@rdserver.senao.com/SmartOpenWrt/bin/$(SOC_ARCH)/$(QCA_SP_REPO)/$(1) -P $(build_dir)/bin
endef

ifeq (, $(shell which git-lfs))
$(error "No git-lfs found, sudo apt-get install git-lfs")
else
define GetFile
	git lfs pull
	mkdir -p $(build_dir)/bin
	$(CP) files/$(SOC_ARCH)/$(QCA_SP_REPO)/* $(build_dir)/bin
endef
endif

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_10_0_ES2 qca_spf_10_0_FC qca_spf_10_0_CS_PREVIEW qca_spf_10_0_CS qca_spf_10_0_CS_PATCH1 \
	qca_spf_10_0_CS_PATCH2 qca_spf_10_0_CS_PATCH5))
define PrepareBuildDir
	rm -rf $(build_dir)
	mkdir -p $(build_dir)
	$(CP) scripts_qca_spf_10_0_ES2 $(build_dir)/scripts
	mkdir -p $(build_dir)/$(SOC_ARCH)
	cp -rf $(SN_CONFIGS_DIR)/ipq-image-config/* $(build_dir)/$(SOC_ARCH)
endef
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_ipq6018_ilq12.0_es1 qca_ipq6018_ilq11.0_fc))
define PrepareBuildDir
	rm -rf $(build_dir)
	mkdir -p $(build_dir)
	$(CP) scripts_ipq6018_ilq12_es1 $(build_dir)/scripts
	mkdir -p $(build_dir)/$(SOC_ARCH)
	cp -rf $(SN_CONFIGS_DIR)/ipq-image-config/* $(build_dir)/$(SOC_ARCH)
endef
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_11_0_csu1 qca_spf_11_0_csu3))
define PrepareBuildDir
	rm -rf $(build_dir)
	mkdir -p $(build_dir)
	$(CP) scripts_qca_spf_11_0_csu1 $(build_dir)/scripts
	mkdir -p $(build_dir)/$(SOC_ARCH)
	cp -rf $(SN_CONFIGS_DIR)/ipq-image-config/* $(build_dir)/$(SOC_ARCH)
endef
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_11_1_csu2))
define PrepareBuildDir
	rm -rf $(build_dir)
	mkdir -p $(build_dir)
	$(CP) scripts_qca_spf_11_1_csu2 $(build_dir)/scripts
	mkdir -p $(build_dir)/$(SOC_ARCH)
	cp -rf $(SN_CONFIGS_DIR)/ipq-image-config/* $(build_dir)/$(SOC_ARCH)
endef
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_11_2))
define PrepareBuildDir
	rm -rf $(build_dir)
	mkdir -p $(build_dir)
	$(CP) scripts_qca_spf_11_2 $(build_dir)/scripts
	mkdir -p $(build_dir)/$(SOC_ARCH)
	cp -rf $(SN_CONFIGS_DIR)/ipq-image-config/* $(build_dir)/$(SOC_ARCH)
endef
else
define PrepareBuildDir
	rm -rf $(build_dir)
	mkdir -p $(build_dir)
	$(CP) scripts $(build_dir)
	mkdir -p $(build_dir)/$(SOC_ARCH)
	cp -rf $(SN_CONFIGS_DIR)/ipq-image-config/* $(build_dir)/$(SOC_ARCH)
endef
endif

ifeq ("$(wildcard /etc/fedora-release)","")
define Prepare
	@echo ""
	@echo -e "\e[32m=== Prepare ===\e[0m"
	@echo ""
	$(call CheckConfig)
	$(call PrepareBuildDir)
	$(call GetFile)
endef
else
ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO),qca_spf_10_0_ES2 qca_spf_10_0_FC qca_spf_10_0_CS_PREVIEW qca_spf_10_0_CS qca_spf_10_0_CS_PATCH1 qca_spf_10_0_CS_PATCH2 qca_spf_10_0_CS_PATCH5))
define Prepare
	@echo ""
	@echo -e "\e[32m=== Prepare Fedora $(QCA_SP_REPO) ===\e[0m"
	@echo ""
	$(CP) $(TOPDIR)/build_dir/host/u-boot-2014.10/tools/mkimage $(TOPDIR)/target/linux/ipq/image/files/$(SOC_ARCH)/$(QCA_SP_REPO)/mkimage
	$(call CheckConfig)
	$(call PrepareBuildDir)
	$(call GetFile)
endef
else ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO), qca_ipq6018_ilq12.0_es1 qca_ipq6018_ilq11.0_fc qca_spf_11_0_csu1 qca_spf_11_0_csu3 qca_spf_11_1_csu2 qca_spf_11_2))
define Prepare
	@echo ""
	@echo -e "\e[32m=== Prepare Fedora $(QCA_SP_REPO) ===\e[0m"
	@echo ""
	$(CP) $(TOPDIR)/build_dir/host/u-boot-2014.10/tools/mkimage $(TOPDIR)/target/linux/ipq/image/files/$(SOC_ARCH)/$(QCA_SP_REPO)/mkimage
	$(call CheckConfig)
	$(call PrepareBuildDir)
	$(call GetFile)
endef
else
define Prepare
	@echo ""
	@echo -e "\e[32m=== Prepare Fedora ===\e[0m"
	@echo ""
	$(call CheckConfig)
	$(call PrepareBuildDir)
	$(call GetFile)
endef
endif
endif

define GenCDT
	@echo ""
	@echo -e "\e[32m=== GenCDT ===\e[0m"
	@echo ""
	$(PYTHON) $(build_dir)/scripts/gen_cdt_bin.py \
		-c $(build_dir)/$(SOC_ARCH)/config.xml \
		-r $(build_dir)/ \
		-o $(BIN_DIR)
endef

define GenBoot
	@echo ""
	@echo -e "\e[32m=== GenBoot ===\e[0m"
	@echo ""
	$(PYTHON) $(build_dir)/scripts/gen_bootconfig_bin.py \
		-c $(build_dir)/$(SOC_ARCH)/config.xml \
		-r $(build_dir)/ \
		-o $(BIN_DIR)
endef

define GenPartition
	@echo ""
	@echo -e "\e[32m=== GenPartition ===\e[0m"
	@echo ""
	$(PYTHON) $(build_dir)/scripts/gen_flash_partition_bin.py \
	-c $(build_dir)/$(SOC_ARCH)/config.xml \
	-f $(IMG_TYPE) \
	-r $(build_dir)/ \
	-o $(BIN_DIR)
endef

ifeq ($(SOC_ARCH),$(filter $(SOC_ARCH),ipq6018))
define GenMbn
	@echo ""
	@echo -e "\e[32m=== GenMbn ===\e[0m"
	@echo ""
	$(PYTHON) $(build_dir)/scripts/elftombn.py \
		-f $(BIN_DIR)/openwrt-$(SOC_ARCH)-u-boot.elf \
		-o $(BIN_DIR)/openwrt-$(SOC_ARCH)-u-boot.mbn \
		-v 6
endef
else ifeq ($(SOC_ARCH),$(filter $(SOC_ARCH),ipq807x))
define GenMbn
	@echo ""
	@echo -e "\e[32m=== GenMbn ===\e[0m"
	@echo ""
	$(PYTHON) $(build_dir)/scripts/elftombn.py \
		-f $(BIN_DIR)/openwrt-ipq807x-u-boot.elf \
		-o $(BIN_DIR)/openwrt-ipq807x-u-boot.mbn
endef
endif

ifneq (,$(findstring $(ARCH), aarch64 aarch64_be))
define PackFitLoader
	@echo ""
	@echo -e "\e[32m=== Build FIT loader 64bit image===\e[0m"
	@echo ""
	@$(CP) $(build_dir)/bin/* $(BIN_DIR)
	$(PYTHON) $(build_dir)/scripts/pack_hk.py \
		--arch $(SOC_ARCH)_64 \
		--fltype $(IMG_TYPE) \
		--srcPath $(build_dir) \
		--inImage $(BIN_DIR) \
		--outImage $(BIN_DIR) \
		--image_type sn_loader_s
	$(CP) $(BIN_DIR)/$(IMG_TYPE)-$(SOC_ARCH)_64-single.img $(fit_loader)
endef

define PackFitKRimg
	@echo ""
	@echo -e "\e[32m=== Build KR image 64bit===\e[0m"
	@echo ""
	@$(CP) $(build_dir)/bin/* $(BIN_DIR)
	$(PYTHON) $(build_dir)/scripts/pack_hk.py \
		--arch $(SOC_ARCH)_64 \
		--fltype $(IMG_TYPE) \
		--srcPath $(build_dir) \
		--inImage $(BIN_DIR) \
		--outImage $(BIN_DIR) \
		--image_type hlos
	$(CP) $(BIN_DIR)/$(IMG_TYPE)-$(SOC_ARCH)_64-single.img $(fit_kr)
endef

define PackAllImg
	@echo ""
	@echo -e "\e[32m=== PackAllImg 64 bit===\e[0m"
	@echo ""
	@$(CP) $(build_dir)/bin/* $(BIN_DIR)
	$(PYTHON) $(build_dir)/scripts/pack_hk.py \
		--arch $(SOC_ARCH)_64 \
		--fltype $(IMG_TYPE) \
		--srcPath $(build_dir) \
		--inImage $(BIN_DIR) \
		--outImage $(BIN_DIR)
	$(CP) $(BIN_DIR)/$(IMG_TYPE)-$(SOC_ARCH)_64-single.img $(fit_all)
endef
else
define PackFitLoader
	@echo ""
	@echo -e "\e[32m=== Build FIT loader image===\e[0m"
	@echo ""
	@$(CP) $(build_dir)/bin/* $(BIN_DIR)
	$(PYTHON) $(build_dir)/scripts/pack_hk.py \
		--arch $(SOC_ARCH) \
		--fltype $(IMG_TYPE) \
		--srcPath $(build_dir) \
		--inImage $(BIN_DIR) \
		--outImage $(BIN_DIR) \
		--image_type sn_loader_s
	$(CP) $(BIN_DIR)/$(IMG_TYPE)-$(SOC_ARCH)-single.img $(fit_loader)
endef

define PackFitKRimg
	@echo ""
	@echo -e "\e[32m=== Build KR image===\e[0m"
	@echo ""
	@$(CP) $(build_dir)/bin/* $(BIN_DIR)
	$(PYTHON) $(build_dir)/scripts/pack_hk.py \
		--arch $(SOC_ARCH) \
		--fltype $(IMG_TYPE) \
		--srcPath $(build_dir) \
		--inImage $(BIN_DIR) \
		--outImage $(BIN_DIR) \
		--image_type hlos
	$(CP) $(BIN_DIR)/$(IMG_TYPE)-$(SOC_ARCH)-single.img $(fit_kr)
endef

define PackAllImg
	@echo ""
	@echo -e "\e[32m=== PackAllImg ===\e[0m"
	@echo ""
	@$(CP) $(build_dir)/bin/* $(BIN_DIR)
	$(PYTHON) $(build_dir)/scripts/pack_hk.py \
		--arch $(SOC_ARCH) \
		--fltype $(IMG_TYPE) \
		--srcPath $(build_dir) \
		--inImage $(BIN_DIR) \
		--outImage $(BIN_DIR)
	$(CP) $(BIN_DIR)/$(IMG_TYPE)-$(SOC_ARCH)-single.img $(fit_all)
endef
endif

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO), qca_ipq6018_ilq12.0_es1 qca_ipq6018_ilq11.0_fc qca_spf_11_0_csu1 qca_spf_11_0_csu3 qca_spf_11_1_csu2 qca_spf_11_2))
QCA_IMG_PREFIX=qcom-$(SOC_ARCH)
else
QCA_IMG_PREFIX=qcom-$(SUBTARGET)
endif
QCA_IMG_SUFFIX=fit-uImage
QCA_KNL_IMG_NAME=$(IMG_PREFIX)-$(QCA_IMG_PREFIX)$(if $(QCA_BOARD),-$(QCA_BOARD))-$(QCA_IMG_SUFFIX).itb
define CopyRenameImg

ifeq ($(IMG_TYPE),$(filter $(IMG_TYPE), emmc norplusemmc))
	@echo ""
	@echo -e "\e[32m=== emmc/norplusemmc ALL-image copy ===\e[0m"
	@echo ""
	$(CP) $(fit_all) /tftpboot
	@echo ""
	@echo -e "\e[32m=== emmc/norplusemmc ALL-image copy DONE !! ===\e[0m"
	@echo ""
endif
	-cp $(BIN_DIR)/$(QCA_KNL_IMG_NAME) $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_KERNEL_SUFFIX)
	-cp $(BIN_DIR)/$(IMG_PREFIX)-$(FW_ROOTFS_SUFFIX) $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_ROOTFS_SUFFIX)
endef

