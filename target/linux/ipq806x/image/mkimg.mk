config=$(SN_CONFIGS_DIR)/ipq-image-config/boardconfig
#target=$(shell grep -ws "^genericmodel" $(SN_CONFIGS_DIR)/sysProductInfo.mk | cut -d '=' -f 2)

partitions=$(grep -sw "Size" $(TOPDIR)/tmp/partitions_size.txt | awk -F ":" '{print $2}')
K_SIZE=$(echo $partitions | awk -F " " '{print $1}')

### SENAO #### redefine Image/Checksum(ref. image.mk) ###
# To generate fit bootloader image for ipq806x,
# 1. wget some binary files(we have no src)
# 2. combine binary files

NAND_SBL=nand_sbl1.mbn nand_sbl2.mbn nand_sbl3.mbn
NOR_SBL=nor_sbl1.mbn nor_sbl2.mbn nor_sbl3.mbn
DDRCONFIG=cb-cdt.mbn
RPM=rpm.mbn
TRUSTZONE=tz.mbn
IPQ_TOOLS=nor_tool
IPQ_FILES=$(NAND_SBL) $(NOR_SBL) $(DDRCONFIG) $(RPM) $(TRUSTZONE) $(IPQ_TOOLS)

DUAL_IMAGE = n

SN_SCRIPTS_DIR?=$(TOPDIR)/SENAO/scripts

define Image/Checksum
        ( cd ${BIN_DIR} ; \
                $(FIND) -maxdepth 1 -type f \! -name 'md5sums'  -printf "%P\n" | sort | xargs \
                md5sum --binary > md5sums \
        )
	$(call checkConfig)
	$(call BuildFitBootloader)
	$(call BuildKRimg)
	$(call BuildFitImg)
endef

# don't re-retrieve files unless newer than local.
define GetFtpFile
	wget -nv -N ftp://ftpuser:senao123@rdserver.senao.com/SmartOpenWrt/bin/ipq806x/$(target)/$(1) -P $(BIN_DIR)/$(PRODUCT_NAME_U)
endef

ifeq (, $(shell which git-lfs))
$(error "No git-lfs found, sudo apt-get install git-lfs")
else
define GetFile
	git lfs pull
	$(CP) files/$(target)/* -P $(BIN_DIR)/$(PRODUCT_NAME_U)
endef
endif


prefix_output=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-
#loader=$(prefix_output)$(subst nor,nand,$(subst -s,,$(FW_UBOOT_SUFFIX)))
#nand_s_output=$(prefix_output)$(FW_UBOOT_SUFFIX)

staging_dir=$(BIN_DIR)/$(PRODUCT_NAME_U)/

#IMG_TYPE=$(shell grep "_available=true" $(config) |cut -d '_' -f 1) # move to sysProductInfo.mk
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

# assign the target name
loader=$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_loader),$(if $(findstring nor,$(IMG_TYPE)),$(nor_loader),$(nand_loader)))
fit_loader=$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_loader),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_loader),$(nand_fit_loader)))
fit_kr=$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_kr),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_kr),$(nand_fit_kr)))
fit_all=$(if $(findstring norplusnand,$(IMG_TYPE)),$(nornand_fit_all),$(if $(findstring nor,$(IMG_TYPE)),$(nor_fit_all),$(nand_fit_all)))

#NOR_MIBIB=$(shell cat $(config) | awk -F "=" '/^nor_partition_mbn/{print$$2}')
#NAND_MIBIB=$(shell cat $(config) | awk -F "=" '/^nand_partition_mbn/{print$$2}')
#NORPLUSNAND_MIBIB=$(shell cat $(config) | awk -F "=" '/^norplusnand_partition_mbn/{print$$2}')
NOR_MIBIB=nor-system-partition.bin
NAND_MIBIB=nand-system-partition.bin
NORPLUSNAND_MIBIB=norplusnand-system-partition.bin

ifneq ($(CONFIG_PACKAGE_uboot-ipq806x-SENAO_NOR_HAS_FW2),)
DUAL_IMAGE = y
$(warning =======> Nor DualImage=$(DUAL_IMAGE))
endif

ifneq ($(CONFIG_PACKAGE_uboot-ipq806x-SENAO_NORnNAND_HAS_FW2),)
DUAL_IMAGE = y
$(warning =======> Nor2Nand DualImage=$(DUAL_IMAGE))
endif

define checkConfig
	@echo -e "\e[32m!!! check config !!!\e[0m"
	@if [ "$(num_of_en)" != "1" ]; then \
		echo "Failed!!"; \
		echo "Please select the firmware type, one at a time."; \
		exit 1; \
	fi
	@echo "$(IMG_TYPE) selected"
endef

define MakeFitLoader
	@echo -e "\e[32m!!! MakeFitLoader !!!\e[0m"
	@if [ $(IMG_TYPE) = "nor" ] || [ $(IMG_TYPE) = "norplusnand" ]; then \
	    echo "# Append SBL1, default bs = 512 bytes" > /dev/null; \
	    dd if=$(staging_dir)/nor_sbl1.mbn of=$(loader); \
	    if [ $(IMG_TYPE) = "nor" ]; then \
		dd if=$(staging_dir)/$(NOR_MIBIB) of=$(loader) seek=256; \
	    fi; \
	    if [ $(IMG_TYPE) = "norplusnand" ]; then \
		dd if=$(staging_dir)/$(NORPLUSNAND_MIBIB) of=$(loader) seek=256; \
	    fi; \
	    echo "append SBL2 in 0x00040000 = 262,144 bytes, 262,144/512=512 count" > /dev/null; \
	    dd if=$(staging_dir)/nor_sbl2.mbn of=$(loader) seek=512; \
	    echo "# append SBL3 in 0x00080000 = 524,288 bytes, 524,288/512=1024 count" > /dev/null; \
	    dd if=$(staging_dir)/nor_sbl3.mbn of=$(loader) seek=1024; \
	    echo "# append DDRCONFIG in 0x100000 = 1,048,576 bytes, 1,048,576/512=2048 count" > /dev/null; \
	    dd if=$(staging_dir)/$(PRODUCT_NAME_U)-cdt.mbn of=$(loader) seek=2048; \
	    echo "# append SSD in 0x00110000 = 11,114,112 bytes, 1,114,112/512=2176 count" > /dev/null; \
	    dd if=$(staging_dir)/ssd.mbn of=$(loader) seek=2176; \
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
	    dd if=$(staging_dir)/nand-system-partition.bin of=$(loader) seek=512; \
	    echo "# append SBL2 in 0x00180000 = 1,572,864 bytes, 1,572,864/512=3072 count" > /dev/null; \
	    dd if=$(staging_dir)/nand_sbl2.mbn of=$(loader) seek=3072; \
	    echo "# append SBL3 in 0x002c0000 = 2,883,584 bytes, 2,883,584/512=5632 count" > /dev/null; \
	    dd if=$(staging_dir)/nand_sbl3.mbn of=$(loader) seek=5632; \
	    echo "# append DDRCONFIG in 0x00540000 = 5,505,024 bytes, 5,505,024/512=10752 count" > /dev/null; \
	    dd if=$(staging_dir)/$(PRODUCT_NAME_U)-cdt.mbn of=$(loader) seek=10752; \
	    echo "# append SSD in 0x00660000 = 6,684,672 bytes, 6,684,672/512=13056 count" > /dev/null; \
	    dd if=$(staging_dir)/ssd.mbn of=$(loader) seek=13056; \
	    echo "# append tz in 0x00780000 = 7,864,320 bytes, 7,864,320/512=15360 count" > /dev/null; \
	    dd if=$(staging_dir)/tz.mbn of=$(loader) seek=15360; \
	    echo "# append rpm in 0x00a00000 = 10,485,760 bytes, 10,485,760/512=20480 count" > /dev/null; \
	    dd if=$(staging_dir)/rpm.mbn of=$(loader) seek=20480; \
	    echo "# append uboot in 0x00c80000 = 13,107,200 bytes, 13,107,200/512=25600 count" > /dev/null; \
	    dd if=$(prefix_output)u-boot.mbn of=$(loader) seek=25600; \
	fi
	@find $(BIN_DIR) -type l -delete
	@ls $(staging_dir) | xargs -i ln -sf $(staging_dir)/{} $(BIN_DIR)/{}
	@python ../../../../SENAO/scripts/pack.py -t $(IMG_TYPE) -B -F boardconfig -o $(fit_loader) \
		--flash_config_dir $(SN_CONFIGS_DIR)/ipq-image-config \
		--image_dir $(BIN_DIR)/ \
		--staging_host_dir $(STAGING_DIR_HOST)/ \
		--section bootloader \
		--dualimage $(DUAL_IMAGE) \
		--linux_dtc_dir $(LINUX_DIR)/scripts/dtc
	@find $(BIN_DIR) -type l -delete
	@cp $(BIN_DIR)/flash.scr $(BIN_DIR)/flash.scr.loader 2>/dev/null || true
	@cp $(BIN_DIR)/flash.its $(BIN_DIR)/flash.its.loader 2>/dev/null || true
endef

define BuildFitBootloader
	@echo ""
	@echo -e "\e[32m=== Build Fit Bootloader ===\e[0m"
	@echo ""
	mkdir -p $(BIN_DIR)/$(PRODUCT_NAME_U)
	$(call GetFile)
	@echo "!!! genimg.py - start !!!"
	@python $(SN_SCRIPTS_DIR)/genimg.py \
	    --nor_tool=$(BIN_DIR)/$(PRODUCT_NAME_U)/nor_tool \
	    --mbn_gen=$(SN_SCRIPTS_DIR)/nand_mbn_generator.py \
	    --cdt_mod=$(SN_SCRIPTS_DIR)/cdt_mod.py \
	    --smem_gen=$(SN_SCRIPTS_DIR)/smem-tool.py \
	    --configdir=$(SN_CONFIGS_DIR)/ipq-image-config \
	    --sblrootdir=$(BIN_DIR)/$(PRODUCT_NAME_U) \
	    --skip_export \
	    --cdt_bin=cb-cdt.mbn \
	    --cdt_modxml=cb-cdtmod.xml \
	    --cdt_outbin=$(PRODUCT_NAME_U)-cdt.mbn \
	    --outdir=$(BIN_DIR)/ \
	    --prefix=$(BIN_DIR)/$(PRODUCT_NAME_U)
	@echo "!!! genimg.py - end !!!"
	$(call MakeFitLoader)
endef

define BuildKRimg
	@echo ""
	@echo -e "\e[32m=== Build KR image ===\e[0m"
	@echo ""
	@mkdir -p $(BIN_DIR)/$(PRODUCT_NAME_U)
	@python $(SN_SCRIPTS_DIR)/pack.py \
		-t $(IMG_TYPE) \
		-B -F boardconfig \
		-o $(fit_kr) \
		--flash_config_dir $(SN_CONFIGS_DIR)/ipq-image-config \
		--image_dir $(BIN_DIR)/ \
		--staging_host_dir $(STAGING_DIR_HOST)/ \
		--section apps \
		--dualimage $(DUAL_IMAGE) \
		--linux_dtc_dir $(LINUX_DIR)/scripts/dtc
	@cp $(BIN_DIR)/flash.scr $(BIN_DIR)/flash.scr.apps 2>/dev/null || true
	@cp $(BIN_DIR)/flash.its $(BIN_DIR)/flash.its.apps 2>/dev/null || true
endef


define BuildFitImg
	@if [ $(IMG_TYPE) = "norplusnand" ]; then \
		echo ""; \
		echo -e "\e[32m=== Build fit image ===\e[0m"; \
		echo ""; \
		find $(BIN_DIR) -type l -delete; \
		ls $(staging_dir) | xargs -i ln -sf $(staging_dir)/{} $(BIN_DIR)/{}; \
		python $(SN_SCRIPTS_DIR)/pack.py \
			-t $(IMG_TYPE) \
			-B -F boardconfig \
			-o $(fit_all) \
			--flash_config_dir $(SN_CONFIGS_DIR)/ipq-image-config \
			--image_dir $(BIN_DIR)/ \
			--staging_host_dir $(STAGING_DIR_HOST)/ \
			--section fitimage \
			--dualimage $(DUAL_IMAGE) \
			--linux_dtc_dir $(LINUX_DIR)/scripts/dtc; \
		find $(BIN_DIR) -type l -delete; \
		@cp $(BIN_DIR)/flash.scr $(BIN_DIR)/flash.scr.all 2>/dev/null || true; \
		@cp $(BIN_DIR)/flash.its $(BIN_DIR)/flash.its.all 2>/dev/null || true; \
	fi
endef
### SEANO ###

