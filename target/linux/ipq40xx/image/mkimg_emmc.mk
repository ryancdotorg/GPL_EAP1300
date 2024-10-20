vendor_config_dir=$(SN_CONFIGS_DIR)/ipq-image-config/
config=$(vendor_config_dir)/boardconfig
#target=$(shell grep -ws "^genericmodel" $(SN_CONFIGS_DIR)/sysProductInfo.mk | cut -d '=' -f 2)
dirname=$(shell grep -ws "^dirname" $(config) | cut -d '=' -f 2)

partitions=$(grep -sw "Size" $(TOPDIR)/tmp/partitions_size.txt | awk -F ":" '{print $2}')
K_SIZE=$(echo $partitions | awk -F " " '{print $1}')

### SENAO #### redefine Image/Checksum(ref. image.mk) ###
# To generate fit bootloader image for ipq40xx,
# 1. wget some binary files(we have no src)
# 2. combine binary files

DUAL_IMAGE = n


BYPASS_BUILD = n
ifeq (y,$(BYPASS_BUILD))
define Image/BuildKernel
endef

define Image/BuildKernel/Initramfs
endef

define Image/mkfs/squashfs
endef

define Image/Build/ubifs
endef
endif

define Image/Checksum
        ( cd ${BIN_DIR} ; \
                $(FIND) -maxdepth 1 -type f \! -name 'md5sums'  -printf "%P\n" | sort | xargs \
                md5sum --binary > md5sums \
        )
	$(call checkConfig)
	$(call PrepareBuildDir)
	$(call GenCDT)
	$(call GenMIBIB)
	$(call BuildLoader)
	$(call BuildFitLoader)
	$(call BuildFitKRimg)
	$(call BuildFitAll)
	$(call CopyRenameImg)
endef

define GetFtpFile
	wget -nv -N ftp://ftpuser:senao123@rdserver.senao.com/SmartOpenWrt/bin/ipq806x/$(target)/$(1) -P $(staging_dir)
endef

ifeq (, $(shell which git-lfs))
$(error "No git-lfs found, sudo apt-get install git-lfs")
else
define GetFile
	git lfs pull
	mkdir -p $(staging_dir)
	$(CP) files/$(QCA_SP_REPO)/* $(staging_dir)
endef
endif

prefix_output=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-

staging_dir=$(BIN_DIR)/build_emmc

num_of_en=$(shell grep "_available=true" $(config) | wc -l)

# file name : nor + emmc
noremmc_loader=$(prefix_output)$(subst -s,,$(FW_FIT_NOREMMC_UBOOT_SUFFIX))
noremmc_fit_loader=$(prefix_output)$(FW_FIT_NOREMMC_UBOOT_SUFFIX)
noremmc_fit_kr=$(prefix_output)$(FW_FIT_NOREMMC_KR_SUFFIX)
noremmc_fit_all=$(prefix_output)$(FW_FIT_NOREMMC_ALL_SUFFIX)

norplusemmc_parts="$(shell echo `grep -oP '(?<=size_kb length="4">)[^<]+' "$(SN_CONFIGS_DIR)/ipq-image-config/norplusemmc-partition.xml"`)"
norplusemmc_parts_pad="$(shell echo `grep -oP '(?<=pad_kb length="2">)[^<]+' "$(SN_CONFIGS_DIR)/ipq-image-config/norplusemmc-partition.xml"`)"


ifeq (norplusemmc,$(IMG_TYPE))
  Image_Name="NOR_PLUS_EMMC_IMAGES"
  loader=$(noremmc_loader)
  fit_loader=$(noremmc_fit_loader)
  fit_kr=$(noremmc_fit_kr)
  fit_all=$(noremmc_fit_all)
  size_kb=$(norplusemmc_parts)
  pad_kb=$(norplusemmc_parts_pad)
  num_parts=$(words $(norplusemmc_parts))
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

define PrepareBuildDir
	rm -rf $(staging_dir)
	$(CP) build_emmc $(staging_dir)
	mkdir -p $(staging_dir)/config
	find $(vendor_config_dir)/* -exec cp {} $(staging_dir)/config \;
	@sed 's/FIRMWARE_TYPE/loader/g' $(config) > $(staging_dir)/config/loader_config
	@sed 's/FIRMWARE_TYPE/apps/g' $(config) > $(staging_dir)/config/apps_config
	@sed 's/FIRMWARE_TYPE/all/g' $(config) > $(staging_dir)/config/all_config
endef

define get_size_kb
	( \
		list_size=$(size_kb); \
		list_pad=$(pad_kb); \
		x=0; \
		for i in $$$$(seq 1 $2); do \
			size=`echo $$$$list_size|cut -d " " -f $$$$i`; \
			pad=`echo $$$$list_pad|cut -d " " -f $$$$i`; \
			x=$$$$((x+$$$$size)); \
			x=$$$$((x+$$$$pad)); \
			echo $$$$size kb; \
		done; \
		x=$$$$((x*1024)); \
		x=$$$$((x/512)); \
		echo $$$$x; \
		echo $$$$x > $(TMP_DIR)/.ipq_part_size_$$$$i; \
	)
endef

define cal_part_size
	for num in $$$$(seq 1 $(num_parts)); do \
		$(call get_size_kb,$(IMG_TYPE),$$$$num); \
	done
endef

define BuildLoader
	$(call cal_part_size)
	@echo -e "\e[32m!!! Make Loader !!!\e[0m"
	echo "# dd: default bs = 512 bytes" > /dev/null; \
	echo "# Append SBL1 @0x0" > /dev/null; \
	dd if=$(staging_dir)/sbl1_nor.mbn of=$(loader); \
	echo "# Append MIBIB @0x040000 = 262144 bytes, 262141/512=512 count" > /dev/null; \
	dd if=$(BIN_DIR)/norplusemmc-system-partition.bin of=$(loader) seek=`cat $(TMP_DIR)/.ipq_part_size_1`; \
	echo "# Append QSEE @0x060000 = 393216 bytes, 393216/512=768" > /dev/null; \
	dd if=$(staging_dir)/tz.mbn of=$(loader) seek=`cat $(TMP_DIR)/.ipq_part_size_2`; \
	echo "# Append CDT @0x0c0000 = 786432 bytes, 786432/512=1536 count" > /dev/null; \
	dd if=$(staging_dir)/cdt-AP.DK04.1-C1.bin of=$(loader) seek=`cat $(TMP_DIR)/.ipq_part_size_3`; \
	echo "# Append DDRPARAMS in 0xd0000= 851968 bytes, 851968/512=1664 count" > /dev/null; \
	echo "# Append APPSBLENV in 0xe0000= 917504 bytes, 917504/512=1792 count" > /dev/null; \
	echo "# Append APPSBL(uboot) @0xf0000 = 983040 bytes, 983040/512=1920 count" > /dev/null; \
	dd if=$(prefix_output)u-boot-stripped.elf of=$(loader) seek=`cat $(TMP_DIR)/.ipq_part_size_6`;
endef

define BuildFitLoader
	@echo ""
	@echo -e "\e[32m=== BuildFitLoader ===\e[0m"
	@echo ""
	@if [ -z `which dtc` ]; then \
		rm -f /usr/bin/dtc; \
		ln -sf $(LINUX_DIR)/scripts/dtc/dtc /usr/bin/dtc; \
	fi;
	(cd $(BIN_DIR) && dd if=$(prefix_output)u-boot.mbn of=$(prefix_output)u-boot.mbn.padded bs=64k)
	@find $(BIN_DIR) -type l -delete
	@ls $(staging_dir) | xargs -i ln -sf $(staging_dir)/{} $(BIN_DIR)/{}
	python $(staging_dir)/pack.py \
		-t $(IMG_TYPE) \
		-B \
		-F $(staging_dir)/config/loader_config \
		-o $(fit_loader) \
		$(BIN_DIR)
	@find $(BIN_DIR) -type l -delete
	@cp $(BIN_DIR)/flash.scr $(BIN_DIR)/flash.scr.loader 2>/dev/null || true
	@cp $(BIN_DIR)/flash.its $(BIN_DIR)/flash.its.loader 2>/dev/null || true
endef

define GenCDT
	@echo ""
	@echo -e "\e[32m=== Generate CDT ===\e[0m"
	@echo ""
	@echo "!!! genimg.py CDT - start !!!"
	$(call GetFile)
	(cd $(staging_dir) && \
	python genimg.py --partition_tool=partition_tool \
		--cdt_mod=cdt_mod.py --configdir=config/ --skip_export \
		--cdt_bin=cdt-$(shell echo $(QCA_BOARD)|tr '[:lower:]' '[:upper:]').bin \
		--cdt_modxml=config/pcddr_40xx.xml --cdt_outbin=cb-cdtnew.mbn \
		--outdir=$(BIN_DIR) --image_name=CDT_IMAGES \
	)
	@echo "!!! genimg.py CDT - end !!!"
endef

define GenMIBIB
	@echo ""
	@echo -e "\e[32m=== Generate GenMIBIB ===\e[0m"
	@echo ""
	@echo "!!! genimg.py CDT - start !!!"
	(cd $(staging_dir) && \
	python genimg.py --partition_tool=partition_tool --mbn_gen=nand_mbn_generator.py \
		--smem_gen=smem-tool.py --configdir=config/ --skip_export \
		--outdir=$(BIN_DIR) --image_name=NOR_PLUS_EMMC_IMAGES \
	)
	@echo "!!! genimg.py - end !!!"
	$(call GenGPT)
endef

define GenGPT
	@echo ""
	@echo -e "\e[32m=== Generate GenGPT ===\e[0m"
	@echo ""
	@echo "!!! prepareSingleImage.py - start !!!"
	(cd $(staging_dir) && \
	python $(staging_dir)/prepareSingleImage.py --arch ipq40xx \
		--fltype norplusemmc \
		--genpart \
		--in ./ \
	)
	@echo "!!! prepareSingleImage.py - end !!!"
endef

define BuildFitKRimg
	@echo ""
	@echo -e "\e[32m=== Build KR image ===\e[0m"
	@echo ""
	@find $(BIN_DIR) -type l -delete
	@ls $(staging_dir) | xargs -i ln -sf $(staging_dir)/{} $(BIN_DIR)/{}
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

QCA_IMG_PREFIX=qcom-ipq4019
QCA_IMG_SUFFIX=fit-uImage
QCA_KNL_IMG_NAME=$(IMG_PREFIX)-$(QCA_IMG_PREFIX)-$(QCA_BOARD)-$(QCA_IMG_SUFFIX).itb
QCA_KNL_INITRAM_IMG_NAME=$(IMG_PREFIX)-$(QCA_IMG_PREFIX)-$(QCA_BOARD)-$(QCA_IMG_SUFFIX)-initramfs.itb
define CopyRenameImg
	-cp $(BIN_DIR)/$(QCA_KNL_IMG_NAME) $(BIN_DIR)/$(IMG_PREFIX)-$(PRODUCT_NAME_L)-$(FW_KERNEL_SUFFIX)
	-cp $(BIN_DIR)/$(QCA_KNL_INITRAM_IMG_NAME) $(BIN_DIR)/$(IMG_PREFIX)-$(PRODUCT_NAME_L)-initramfs-$(FW_KERNEL_SUFFIX)
	-cp $(BIN_DIR)/$(IMG_PREFIX)-$(FW_ROOTFS_SUFFIX) $(BIN_DIR)/$(IMG_PREFIX)-$(PRODUCT_NAME_L)-$(FW_ROOTFS_SUFFIX)
endef
