vendor_config_dir=$(SN_CONFIGS_DIR)/ipq-image-config
config=$(vendor_config_dir)/boardconfig
#target=$(shell grep -ws "^genericmodel" $(SN_CONFIGS_DIR)/sysProductInfo.mk | cut -d '=' -f 2)
dirname=$(shell grep -ws "^dirname" $(config) | cut -d '=' -f 2)

partitions=$(grep -sw "Size" $(TOPDIR)/tmp/partitions_size.txt | awk -F ":" '{print $2}')
K_SIZE=$(echo $partitions | awk -F " " '{print $1}')

### SENAO #### redefine Image/Checksum(ref. image.mk) ###
# To generate fit bootloader image for ipq806x,
# 1. wget some binary files(we have no src)
# 2. combine binary files

NAND_SBL=nand_sbl1.mbn nand_sbl2.mbn nand_sbl3.mbn
NOR_SBL=nor_sbl1.mbn nor_sbl2.mbn nor_sbl3.mbn
DDRCONFIG=cdt.mbn
RPM=rpm.mbn
TRUSTZONE=tz.mbn
BOOT_IMAGE_TOOLS=nor_tool bootconfig_tool
BOOT_FILES=$(NAND_SBL) $(NOR_SBL) $(DDRCONFIG) $(RPM) $(TRUSTZONE) $(BOOT_IMAGE_TOOLS)

DUAL_IMAGE = n

define Image/Checksum
        ( cd ${BIN_DIR} ; \
                $(FIND) -maxdepth 1 -type f \! -name 'md5sums'  -printf "%P\n" | sort | xargs \
                md5sum --binary > md5sums \
        )
	$(call checkConfig)
	$(call BuildFitBootloader)
	$(call BuildSingleImage)
	$(call CopyRenameImg)
endef

# split the process to support "mm loader"
define BuildFitBootloader
	$(call PrepareBuildDir)
	$(call GenMIBIBs)
	$(call BuildFitLoader)
	$(call BuildLoader)
endef
define BuildSingleImage
	$(call BuildFitKRimg)
	$(call BuildFitAll)
endef

ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO), qca_spf_11_0_csu1))
target=ipq806x/qca_spf_11_0_csu1
endif
ifeq ($(QCA_SP_REPO),$(filter $(QCA_SP_REPO), qca_spf_11_0_csu3))
target=ipq806x/qca_spf_11_0_csu3
endif
#define GetFtpFile
#	wget -nv -N ftp://ftpuser:senao123@rdserver.senao.com/SmartOpenWrt/bin/ipq806x/$(target)/$(1) -P $(staging_dir)
#endef

#ifeq (, $(shell which git-lfs))
#$(error "No git-lfs found, sudo apt-get install git-lfs")
#else
define GetFile
	git lfs pull
	mkdir -p $(staging_dir)
	$(CP) files/$(target)/* $(staging_dir)
endef
#endif

prefix_output=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-

staging_dir=$(BIN_DIR)/build

num_of_en=$(shell grep "_available=true" $(config) | wc -l)

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

# file name : nor + nand
nornand_loader=$(prefix_output)$(subst -s,,$(FW_FIT_NORNAND_UBOOT_SUFFIX))
nornand_fit_loader=$(prefix_output)$(FW_FIT_NORNAND_UBOOT_SUFFIX)
nornand_fit_kr=$(prefix_output)$(FW_FIT_NORNAND_KR_SUFFIX)
nornand_fit_all=$(prefix_output)$(FW_FIT_NORNAND_ALL_SUFFIX)

# file name : nor + emmc
noremmc_loader=$(prefix_output)$(subst -s,,$(FW_FIT_NOREMMC_UBOOT_SUFFIX))
noremmc_fit_loader=$(prefix_output)$(FW_FIT_NOREMMC_UBOOT_SUFFIX)
noremmc_fit_kr=$(prefix_output)$(FW_FIT_NOREMMC_KR_SUFFIX)
noremmc_fit_all=$(prefix_output)$(FW_FIT_NOREMMC_ALL_SUFFIX)

# file name : nor + sata
norsata_loader=$(prefix_output)$(subst -s,,$(FW_FIT_NORSATA_UBOOT_SUFFIX))
norsata_fit_loader=$(prefix_output)$(FW_FIT_NORSATA_UBOOT_SUFFIX)
norsata_fit_kr=$(prefix_output)$(FW_FIT_NORSATA_KR_SUFFIX)
norsata_fit_all=$(prefix_output)$(FW_FIT_NORSATA_ALL_SUFFIX)

# assign the target name
loader=$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_loader),$(if $(findstring nor,$(IMG_TYPE)),$(nor_loader),$(nand_loader)))
fit_loader=$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_loader),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_loader),$(nand_fit_loader)))
fit_kr=$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_kr),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_kr),$(nand_fit_kr)))
fit_all=$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_all),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_all),$(nand_fit_all)))


NOR_MIBIB=nor-system-partition.bin
NAND_MIBIB=nand-system-partition.bin
NORPLUSNAND_MIBIB=norplusnand-system-partition.bin

ifeq (nor,$(IMG_TYPE))
  Image_Name="NOR_IMAGES"
endif
ifeq (norplusnand,$(IMG_TYPE))
  Image_Name="NOR_PLUS_NAND_IMAGES"
endif
ifeq (nand,$(IMG_TYPE))
  Image_Name="NAND_IMAGES"
endif
ifeq (norplusemmc,$(IMG_TYPE))
  Image_Name="NOR_PLUS_EMMC_IMAGES"
  loader=$(noremmc_loader)
  fit_loader=$(noremmc_fit_loader)
  fit_kr=$(noremmc_fit_kr)
  fit_all=$(noremmc_fit_all)
endif
ifeq (true,$(SATA_REPLACE_EMMC))
  Image_Name="NOR_PLUS_SATA_IMAGES"
  loader=$(norsata_loader)
  fit_loader=$(norsata_fit_loader)
  fit_kr=$(norsata_fit_kr)
  fit_all=$(norsata_fit_all)
endif


ifneq ($(CONFIG_PACKAGE_uboot-ipq806x-SENAO_NOR_HAS_FW2 ),)
DUAL_IMAGE = y
$(warning =======> Nor DualImage=$(DUAL_IMAGE))
endif

ifneq ($(CONFIG_PACKAGE_uboot-ipq806x-SENAO_NORnNAND_HAS_FW2),)
DUAL_IMAGE = y
$(warning =======> Nor2Nand DualImage=$(DUAL_IMAGE))
endif

define checkConfig
@echo -e "\e[32m!!! check config (806x) num_of_en:[$(num_of_en)]!!!\e[0m"
	@if [ "$(num_of_en)" != "1" ]; then \
		echo "Failed!!"; \
		echo "Please select the firmware type, one at a time."; \
		exit 1; \
	fi
	@echo "$(IMG_TYPE) selected"
endef

define PrepareBuildDir
	rm -rf $(staging_dir)
	mkdir -p $(staging_dir)/config
	$(CP) scripts_qca_spf_11_0_csu1_ipq806x/* $(staging_dir)
	find $(vendor_config_dir)/config/* -exec cp {} $(staging_dir)/config \;
	find $(vendor_config_dir)/boardconfig -exec cp {} $(staging_dir)/config \;
	find $(vendor_config_dir)/files* -exec cp {} $(staging_dir)/ \;
	@sed 's/FIRMWARE_TYPE/loader/g' $(config) > $(staging_dir)/config/loader_config
	@sed 's/FIRMWARE_TYPE/apps/g' $(config) > $(staging_dir)/config/apps_config
	@sed 's/FIRMWARE_TYPE/all/g' $(config) > $(staging_dir)/config/all_config
endef

define BuildLoader
	@echo -e "\e[32m!!! Make Loader !!!\e[0m"
	@if [ $(IMG_TYPE) = "nor" ] || [ $(IMG_TYPE) = "norplusnand" ]; then \
	    echo "# Append SBL1, default bs = 512 bytes" > /dev/null; \
	    dd if=$(staging_dir)/nor_sbl1.mbn of=$(loader); \
	    if [ $(IMG_TYPE) = "nor" ]; then \
		dd if=$(BIN_DIR)/$(dirname)/$(NOR_MIBIB) of=$(loader) seek=256; \
	    fi; \
	    if [ $(IMG_TYPE) = "norplusnand" ]; then \
		dd if=$(BIN_DIR)/$(dirname)/$(NORPLUSNAND_MIBIB) of=$(loader) seek=256; \
	    fi; \
	    echo "append SBL2 in 0x00040000 = 262,144 bytes, 262,144/512=512 count" > /dev/null; \
	    dd if=$(staging_dir)/nor_sbl2.mbn of=$(loader) seek=512; \
	    echo "# append SBL3 in 0x00080000 = 524,288 bytes, 524,288/512=1024 count" > /dev/null; \
	    dd if=$(staging_dir)/nor_sbl3.mbn of=$(loader) seek=1024; \
	    echo "# append DDRCONFIG in 0x100000 = 1,048,576 bytes, 1,048,576/512=2048 count" > /dev/null; \
	    dd if=$(BIN_DIR)/$(dirname)/$(PRODUCT_NAME_U)-cdt.mbn of=$(loader) seek=2048; \
	    echo "# append SSD in 0x00110000 = 11,114,112 bytes, 1,114,112/512=2176 count" > /dev/null; \
	    dd if=$(BIN_DIR)/$(dirname)/ssd.mbn of=$(loader) seek=2176; \
	    echo "# append tz in 0x00120000 = 1,179,648 bytes, 1,179,648/512=2304 count" > /dev/null; \
	    dd if=$(staging_dir)/tz.mbn of=$(loader) seek=2304; \
	    echo "# append rpm in 0x001a0000 = 1,703,936 bytes, 1,703,936/512=3328 count" > /dev/null; \
	    dd if=$(staging_dir)/rpm.mbn of=$(loader) seek=3328; \
	    echo "# append uboot in 0x00220000 = 2,228,224 bytes, 2,228,224/512=4352 count" > /dev/null; \
	    dd if=$(prefix_output)u-boot.mbn of=$(loader) seek=4352; \
	else \
	    echo "# Append SBL1, default bs = 512 bytes" > /dev/null; \
	    dd if=$(staging_dir)/nand_sbl1.mbn of=$(loader); \
	    echo "# append MIBIB in 0x00040000 = 262,144 bytes, 262,144/512=512 count" > /dev/null; \
	    dd if=$(BIN_DIR)/$(dirname)/nand-system-partition.bin of=$(loader) seek=512; \
	    echo "# append SBL2 in 0x00180000 = 1,572,864 bytes, 1,572,864/512=3072 count" > /dev/null; \
	    dd if=$(staging_dir)/nand_sbl2.mbn of=$(loader) seek=3072; \
	    echo "# append SBL3 in 0x002c0000 = 2,883,584 bytes, 2,883,584/512=5632 count" > /dev/null; \
	    dd if=$(staging_dir)/nand_sbl3.mbn of=$(loader) seek=5632; \
	    echo "# append DDRCONFIG in 0x00540000 = 5,505,024 bytes, 5,505,024/512=10752 count" > /dev/null; \
	    dd if=$(BIN_DIR)/$(dirname)/$(PRODUCT_NAME_U)-cdt.mbn of=$(loader) seek=10752; \
	    echo "# append SSD in 0x00660000 = 6,684,672 bytes, 6,684,672/512=13056 count" > /dev/null; \
	    dd if=$(BIN_DIR)/$(dirname)/ssd.mbn of=$(loader) seek=13056; \
	    echo "# append tz in 0x00780000 = 7,864,320 bytes, 7,864,320/512=15360 count" > /dev/null; \
	    dd if=$(staging_dir)/tz.mbn of=$(loader) seek=15360; \
	    echo "# append rpm in 0x00a00000 = 10,485,760 bytes, 10,485,760/512=20480 count" > /dev/null; \
	    dd if=$(staging_dir)/rpm.mbn of=$(loader) seek=20480; \
	    echo "# append uboot in 0x00c80000 = 13,107,200 bytes, 13,107,200/512=25600 count" > /dev/null; \
	    dd if=$(prefix_output)u-boot.mbn of=$(loader) seek=25600; \
	fi
endef

define BuildFitLoader
	@echo ""
	@echo -e "\e[32m=== prefix_output:[$(prefix_output)] ===\e[0m"
	@echo ""
	@echo "!!! BuildFitLoader - start !!!"
	@if [ -z `which dtc` ]; then \
		rm -f /usr/bin/dtc; \
		ln -sf $(LINUX_DIR)/scripts/dtc/dtc /usr/bin/dtc; \
	fi;
	(cd $(BIN_DIR) && cp build/openwrt-ipq806x_standard-u-boot-2016.mbn $(prefix_output)u-boot.mbn)
	(cd $(BIN_DIR) && dd if=$(prefix_output)u-boot.mbn of=$(prefix_output)u-boot.mbn.padded bs=64k)
	@find $(BIN_DIR) -type l -delete
	@ls $(staging_dir) | xargs -i ln -sf $(staging_dir)/{} $(BIN_DIR)/{}
	@ls $(BIN_DIR)/$(dirname) | xargs -i ln -sf $(BIN_DIR)/$(dirname)/{} $(BIN_DIR)/{}
	python $(staging_dir)/pack.py \
		-t $(IMG_TYPE) \
		-B \
		-F $(staging_dir)/config/loader_config \
		-o $(fit_loader) \
		$(BIN_DIR)
	@find $(BIN_DIR) -type l -delete
	@cp $(BIN_DIR)/flash.scr $(BIN_DIR)/flash.scr.loader 2>/dev/null || true
	@cp $(BIN_DIR)/flash.its $(BIN_DIR)/flash.its.loader 2>/dev/null || true
	@echo "!!! BuildFitLoader - end !!!"
endef

ifeq (norplusemmc,$(IMG_TYPE))
define GenMIBIBs
	@echo ""
	@echo -e "\e[32m=== Generate MIBIB,CDT,SMEM ===\e[0m"
	@echo ""
	@echo "!!! genimg.py - start !!!"
	$(call GetFile)
	@if [ "$(target)" = "SPF5.3/CS" ]; then \
	(cd $(staging_dir) && \
	python $(staging_dir)/genimg.py --nor_tool=nor_tool --mbn_gen=nand_mbn_generator.py \
		--cdt_mod=cdt_mod.py --smem_gen=smem-tool.py --configdir=config/ --skip_export \
		--cdt_bin=cdt.mbn --cdt_modxml=config/cb-cdtmod.xml --cdt_outbin=cb-cdtnew.mbn \
		--bootconfig_gen=bootconfig_tool \
		--outdir=$(BIN_DIR) \
	) \
	else \
	(cd $(staging_dir) && \
	python $(staging_dir)/genimg.py --nor_tool=nor_tool --mbn_gen=nand_mbn_generator.py \
		--cdt_mod=cdt_mod.py --smem_gen=smem-tool.py --configdir=config/ --skip_export \
		--cdt_bin=cb-cdt.mbn --cdt_modxml=config/cb-cdtmod.xml --cdt_outbin=cb-cdtnew.mbn \
		--bootconfig_gen=bootconfig_tool \
		--outdir=$(BIN_DIR) \
	) \
	fi;
	mv $(BIN_DIR)/$(dirname)/cb-cdtnew.mbn $(BIN_DIR)/$(dirname)/$(dirname)-cdt.mbn
	@echo "!!! genimg.py - end !!!"
	@echo -e "\e[32m=== Generate MIBIB ===\e[0m"
	@echo "!!! prepareSingleImage.py - start !!!"
	(cd $(staging_dir) && \
	python $(staging_dir)/prepareSingleImage.py --arch ipq806x \
		--fltype norplusemmc \
		--genpart \
		--in ./ \
	)
	@echo "!!! prepareSingleImage.py - end !!!"
endef
else
define GenMIBIBs
	@echo ""
	@echo -e "\e[32m=== Generate MIBIB,CDT,SMEM | CP:[$(CP)] target:[$(target)] BIN_DIR:[$(BIN_DIR)] staging_dir:[$(staging_dir)] ===\e[0m"
	@echo ""
	@echo "!!! genimg.py - start !!!"
	$(call GetFile)
	@if [ "$(target)" = "SPF5.3/CS" ]; then \
	(cd $(staging_dir) && \
	python $(staging_dir)/genimg.py --nor_tool=nor_tool --mbn_gen=nand_mbn_generator.py \
		--cdt_mod=cdt_mod.py --smem_gen=smem-tool.py --configdir=config/ --skip_export \
		--cdt_bin=cdt.mbn --cdt_modxml=config/cb-cdtmod.xml --cdt_outbin=cb-cdtnew.mbn \
		--bootconfig_gen=bootconfig_tool \
		--outdir=$(BIN_DIR) \
	) \
	else \
	(cd $(staging_dir) && \
	python $(staging_dir)/genimg.py --nor_tool=nor_tool --mbn_gen=nand_mbn_generator.py \
		--cdt_mod=cdt_mod.py --smem_gen=smem-tool.py --configdir=config/ --skip_export \
		--cdt_bin=cb-cdt.mbn --cdt_modxml=config/cb-cdtmod.xml --cdt_outbin=cb-cdtnew.mbn \
		--bootconfig_gen=bootconfig_tool \
		--outdir=$(BIN_DIR) \
	) \
	fi;
	mv $(BIN_DIR)/$(dirname)/cb-cdtnew.mbn $(BIN_DIR)/$(dirname)/$(dirname)-cdt.mbn
	@echo "!!! genimg.py - end !!!"
endef
endif
define BuildFitKRimg
	@echo ""
	@echo -e "\e[32m=== Build KR image ===\e[0m"
	@echo ""
	@find $(BIN_DIR) -type l -delete
	@ls $(staging_dir) | xargs -i ln -sf $(staging_dir)/{} $(BIN_DIR)/{}
	@ls $(BIN_DIR)/$(dirname) | xargs -i ln -sf $(BIN_DIR)/$(dirname)/{} $(BIN_DIR)/{}
	python $(staging_dir)/pack.py \
		-t $(IMG_TYPE) \
		-B \
		-F $(staging_dir)/config/apps_config \
		-o $(fit_kr) \
		$(BIN_DIR)
	@find $(BIN_DIR) -type l -delete
	@cp $(BIN_DIR)/flash.scr $(BIN_DIR)/flash.scr.apps 2>/dev/null || true
	@cp $(BIN_DIR)/flash.its $(BIN_DIR)/flash.its.apps 2>/dev/null || true
endef


define BuildFitAll
	@echo "";
	@echo -e "\e[32m=== Build fit image ===\e[0m";
	@echo "";
	@find $(BIN_DIR) -type l -delete;
	@ls $(staging_dir) | xargs -i ln -sf $(staging_dir)/{} $(BIN_DIR)/{};
	@ls $(BIN_DIR)/$(dirname) | xargs -i ln -sf $(BIN_DIR)/$(dirname)/{} $(BIN_DIR)/{}
	python $(staging_dir)/pack.py \
		-t $(IMG_TYPE) \
		-B \
		-F $(staging_dir)/config/all_config \
		-o $(fit_all) \
		$(BIN_DIR) ;
	find $(BIN_DIR) -type l -delete;
	@cp $(BIN_DIR)/flash.scr $(BIN_DIR)/flash.scr.all 2>/dev/null || true
	@cp $(BIN_DIR)/flash.its $(BIN_DIR)/flash.its.all 2>/dev/null || true
endef

QCA_IMG_PREFIX=qcom-ipq806x
QCA_IMG_PREFIX_2=qcom-ipq806x
QCA_IMG_SUFFIX=fit-uImage
ifeq ($(target),SPF5.3/CS)
QCA_KNL_IMG_NAME=$(IMG_PREFIX)-$(QCA_IMG_PREFIX_2)$(if $(QCA_BOARD),-$(QCA_BOARD))-$(QCA_IMG_SUFFIX).itb
else
QCA_KNL_IMG_NAME=$(IMG_PREFIX)-$(QCA_IMG_PREFIX)$(if $(QCA_BOARD),-$(QCA_BOARD))-$(QCA_IMG_SUFFIX).itb
endif
define CopyRenameImg
	-cp $(BIN_DIR)/$(QCA_KNL_IMG_NAME) $(BIN_DIR)/$(subst -$(SUBTARGET),,$(IMG_PREFIX))-$(PRODUCT_NAME_L)-$(FW_KERNEL_SUFFIX)
	-cp $(BIN_DIR)/$(IMG_PREFIX)-$(FW_ROOTFS_SUFFIX) $(BIN_DIR)/$(subst -$(SUBTARGET),,$(IMG_PREFIX))-$(PRODUCT_NAME_L)-$(FW_ROOTFS_SUFFIX)
endef
