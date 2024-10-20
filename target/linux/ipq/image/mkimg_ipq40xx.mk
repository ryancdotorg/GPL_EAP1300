ifneq ("$(wildcard /etc/fedora-release)","")
PYTHON=/usr/bin/python
else
PYTHON=python
endif

define CatFiles
	(dd if=$(1) bs=$(2) conv=sync; dd if=$(3) ) > $(4) 
endef

partitions=$(grep -sw "Size" $(TOPDIR)/tmp/partitions_size.txt | awk -F ":" '{print $2}')
K_SIZE=$(echo $partitions | awk -F " " '{print $1}')

# 2097152 = 2M
#define Sysupgrade/squashfs
#	$(call CatFiles,$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-kernel.bin,$(if $(K_SIZE),$(K_SIZE),2097152),$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-squashfs-root.img,$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-sysupgrade-squashfs.bin)
#endef

### SENAO #### redefine Image/Checksum(ref. image.mk) ###
# To generate fit bootloader image for ipq806x,
# 1. wget some binary files(we have no src)
# 2. combine binary files

NAND_SBL=sbl1_nand.mbn
NOR_SBL=sbl1_nor.mbn
TRUSTZONE=tz.mbn
CDT=cb-cdt.mbn
DDRCONFIG=\
cdt-AP.DK01.1-C1.bin \
cdt-AP.DK01.1-C2.bin \
cdt-AP.DK01.1-S1.bin \
cdt-AP.DK04.1-C1.bin \
cdt-AP.DK04.1-C2.bin \
cdt-AP.DK04.1-C3.bin \
cdt-AP.DK04.1-C4.bin \
cdt-AP.DK04.1-C5.bin \
cdt-AP.DK04.1-C6.bin \
cdt-AP.DK04.1-S1.bin \
cdt-AP.DK05.1-C1.bin \
cdt-AP.DK06.1-C1.bin \
cdt-AP.DK07.1-C1.bin \
cdt-AP.DK07.1-C2.bin \
cdt-AP.DK07.1-C3.bin \
cdt-AP.DK07.1-C4.bin \
cdt-DB.DK01.1-C1.bin \
cdt-DB.DK02.1-C1.bin

PARTITION_TOOLS=partition_tool
IPQ_FILES=$(NAND_SBL) $(NOR_SBL) $(DDRCONFIG) $(TRUSTZONE) $(CDT) $(PARTITION_TOOLS)
DUAL_IMAGE = n

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

config=$(SN_CONFIGS_DIR)/ipq-image-config/boardconfig

target=$(shell grep "genericmodel" $(config) | cut -d '=' -f 2)

prefix_output=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-
loader_prefix=$(BIN_DIR)/$(FW_PREFIX)
#prefix_output=$(BIN_DIR)/$(FW_PREFIX)
#loader=$(prefix_output)$(subst nor,nand,$(subst -s,,$(FW_UBOOT_SUFFIX)))
#nand_s_output=$(prefix_output)$(FW_UBOOT_SUFFIX)

build_dir=$(BIN_DIR)/build

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
NOR_MIBIB=nor-user-partition-ipq40xx.bin
NAND_MIBIB=nand-user-partition-ipq40xx.bin
NORPLUSNAND_MIBIB=norplusnand-user-partition-ipq40xx.bin

ifeq (nor,$(IMG_TYPE))
  Image_Name="NOR_IMAGES"
endif
ifeq (norplusnand,$(IMG_TYPE))
  Image_Name="NOR_PLUS_NAND_IMAGES"
endif
ifeq (nand,$(IMG_TYPE))
  Image_Name="NAND_IMAGES"
endif

ifneq ($(CONFIG_PACKAGE_uboot-ipq40xx-SENAO_NOR_HAS_FW2 ),)
DUAL_IMAGE = y
$(warning =======> Nor DualImage=$(DUAL_IMAGE))
endif

ifneq ($(CONFIG_PACKAGE_uboot-ipq40xx-SENAO_NORnNAND_HAS_FW2),)
DUAL_IMAGE = y
$(warning =======> Nor2Nand DualImage=$(DUAL_IMAGE))
endif

# don't re-retrieve files unless newer than local.
define GetFtpFile
	wget -nv -N ftp://ftpuser:senao123@rdserver.senao.com/SmartOpenWrt/bin/ipq40xx/$(QCA_SP_REPO)/$(1) -P $(build_dir)
endef

ifeq (, $(shell which git-lfs))
$(error "No git-lfs found, sudo apt-get install git-lfs")
else
define GetFile
	git lfs pull
	$(CP) files/$(SUBTARGET)/$(QCA_SP_REPO)/* -P $(build_dir)
endef
endif

define checkConfig
	@echo -e "\e[32m!!! check config !!!\e[0m"
	@if [ "$(num_of_en)" != "1" ]; then \
		echo "Failed!!"; \
		echo "Please select the firmware type, one at a time."; \
		exit 1; \
	fi
	@echo "OK!!"
	@echo "$(IMG_TYPE) selected"
endef

define PrepareBuildDir
	@rm -rf $(build_dir)
	@$(CP) build $(BIN_DIR)
	@mkdir -p $(build_dir)/config
	@find $(SN_CONFIGS_DIR)/ipq-image-config/* -exec cp {} $(build_dir)/config \;
	@sed 's/FIRMWARE_TYPE/loader/g' $(SN_CONFIGS_DIR)/ipq-image-config/boardconfig > $(build_dir)/config/loader_config
	@sed 's/FIRMWARE_TYPE/apps/g' $(SN_CONFIGS_DIR)/ipq-image-config/boardconfig > $(build_dir)/config/apps_config
	@sed 's/FIRMWARE_TYPE/all/g' $(SN_CONFIGS_DIR)/ipq-image-config/boardconfig > $(build_dir)/config/all_config
endef

nor_parts="$(shell echo `grep -oP '(?<=size_kb length="4">)[^<]+' "$(SN_CONFIGS_DIR)/ipq-image-config/nor/nor-partition.xml"`)"
nor_parts_pad="$(shell echo `grep -oP '(?<=pad_kb length="2">)[^<]+' "$(SN_CONFIGS_DIR)/ipq-image-config/nor/nor-partition.xml"`)"
norplusnand_parts="$(shell echo `grep -oP '(?<=size_kb length="4">)[^<]+' "$(SN_CONFIGS_DIR)/ipq-image-config/norplusnand/nor-plus-nand-parition.xml"`)"
norplusnand_parts_pad="$(shell echo `grep -oP '(?<=pad_kb length="2">)[^<]+' "$(SN_CONFIGS_DIR)/ipq-image-config/norplusnand/nor-plus-nand-parition.xml"`)"
nand_parts="$(shell echo `grep -oP '(?<=size_kb length="4">)[^<]+' "$(SN_CONFIGS_DIR)/ipq-image-config/nand/nand-partition.xml"`)"
nand_parts_pad="$(shell echo `grep -oP '(?<=pad_kb length="2">)[^<]+' "$(SN_CONFIGS_DIR)/ipq-image-config/nand/nand-partition.xml"`)"

ifeq ($(IMG_TYPE),nor)
size_kb=$(nor_parts)
pad_kb=$(nor_parts_pad)
num_parts=$(words $(nor_parts))
endif
ifeq ($(IMG_TYPE),norplusnand)
size_kb=$(norplusnand_parts)
pad_kb=$(norplusnand_parts_pad)
num_parts=$(words $(norplusnand_parts))
endif
ifeq ($(IMG_TYPE),nand)
size_kb=$(nand_parts)
pad_kb=$(nand_parts_pad)
num_parts=$(words $(nand_parts))
endif

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
	@if [ $(IMG_TYPE) = "nor" ] || [ $(IMG_TYPE) = "norplusnand" ]; then \
		echo "# dd: default bs = 512 bytes" > /dev/null; \
	    echo "# Append SBL1 @0x0" > /dev/null; \
	    dd if=$(build_dir)/sbl1_nor.mbn of=$(loader); \
	    echo "# Append MIBIB @0x040000 = 262144 bytes, 262141/512=512 count" > /dev/null; \
	    if [ $(IMG_TYPE) = "nor" ]; then \
		dd if=$(BIN_DIR)/$(NOR_MIBIB) of=$(loader) seek=`cat $(TMP_DIR)/.ipq_part_size_1`; \
	    fi; \
	    if [ $(IMG_TYPE) = "norplusnand" ]; then \
		dd if=$(BIN_DIR)/$(NORPLUSNAND_MIBIB) of=$(loader) seek=`cat $(TMP_DIR)/.ipq_part_size_1`; \
	    fi; \
	    echo "# Append QSEE @0x060000 = 393216 bytes, 393216/512=768" > /dev/null; \
	    dd if=$(build_dir)/tz.mbn of=$(loader) seek=`cat $(TMP_DIR)/.ipq_part_size_2`; \
	    echo "# Append CDT @0x0c0000 = 786432 bytes, 786432/512=1536 count" > /dev/null; \
	    dd if=$(BIN_DIR)/cdt-pcddr_40xx.bin of=$(loader) seek=`cat $(TMP_DIR)/.ipq_part_size_3`; \
	    echo "# Append DDRPARAMS in 0xd0000= 851968 bytes, 851968/512=1664 count" > /dev/null; \
	    echo "# Append APPSBLENV in 0xe0000= 917504 bytes, 917504/512=1792 count" > /dev/null; \
	    echo "# Append APPSBL(uboot) @0xf0000 = 983040 bytes, 983040/512=1920 count" > /dev/null; \
	    dd if=$(loader_prefix)u-boot-2016-stripped.elf of=$(loader) seek=`cat $(TMP_DIR)/.ipq_part_size_6`; \
	else \
		echo "# dd: default bs = 512 bytes" > /dev/null; \
	    echo "# Append SBL1 @0x0" > /dev/null; \
	    dd if=$(build_dir)/sbl1_nand.mbn of=$(loader).tmp; \
	    echo "# Append MIBIB @0x0100000 = 1048576 bytes, 1048576/512=2048 count" > /dev/null; \
	    dd if=$(BIN_DIR)/$(NAND_MIBIB) of=$(loader).tmp seek=`cat $(TMP_DIR)/.ipq_part_size_1`; \
	    echo "# Append BOOTCONFIG @0x0200000 = 2097152 bytes, 2097152/512=4096 count" > /dev/null; \
	    echo "# Append QSEE @0x0300000 = 3145728 bytes, 3145728/512=6144 count" > /dev/null; \
		dd if=$(build_dir)/tz.mbn of=$(loader).tmp seek=`cat $(TMP_DIR)/.ipq_part_size_3`; \
	    echo "# Append QSEE_ALT @0x0400000 = 4194304 bytes, 4194304/512=8192 count" > /dev/null; \
	    echo "# Append CDT @0x0500000 = 5242880 bytes, 5242880/512=10240 count" > /dev/null; \
	    dd if=$(BIN_DIR)/cdt-pcddr_40xx.bin of=$(loader).tmp seek=`cat $(TMP_DIR)/.ipq_part_size_5`; \
	    echo "# Append CDT_ALT @0x0580000 = 5767168 bytes, 5767168/512=11264 count" > /dev/null; \
	    echo "# Append DDRPARAMS @0x0600000 = 6291456 bytes, 6291456/512=12288 count" > /dev/null; \
	    echo "# Append APPSBLENV @0x0680000 = 6815744 bytes, 6815744/512=13312 count" > /dev/null; \
	    echo "# Append APPSBL(uboot) @0x0700000 = 7340032 bytes, 7340032/512=14336 count" > /dev/null; \
	    dd if=$(prefix_output)u-boot-stripped.elf of=$(loader).tmp seek=`cat $(TMP_DIR)/.ipq_part_size_9`; \
	    echo "# Append APPSBL_ALT @0x0900000 = 9437184 bytes, 9437184/512=18432 count" > /dev/null; \
	    dd if=$(loader).tmp of=$(loader) bs=2048 conv=sync; \
	fi
endef

define BuildFitLoader
	@if [ -z `which dtc` ]; then \
		rm -f /usr/bin/dtc; \
		ln -sf $(LINUX_DIR)/scripts/dtc/dtc /usr/bin/dtc; \
	fi;
	@find $(BIN_DIR) -type l -delete
	@ls $(build_dir) | xargs -i ln -sf $(build_dir)/{} $(BIN_DIR)/{}
	$(PYTHON) $(build_dir)/pack.py \
		-t $(IMG_TYPE) \
		-B \
		-F $(build_dir)/config/loader_config \
		-o $(fit_loader) \
		$(BIN_DIR)
	@find $(BIN_DIR) -type l -delete
	@cp $(BIN_DIR)/flash.scr $(BIN_DIR)/flash.scr.loader 2>/dev/null || true
	@cp $(BIN_DIR)/flash.its $(BIN_DIR)/flash.its.loader 2>/dev/null || true
endef

### TODO
### image_name : NOR NAND EMMC NORPULSNAND SMEM CDT
###

define GenCDT
	@echo ""
	@echo -e "\e[32m=== Generate CDT ===\e[0m"
	@echo ""
	$(call GetFile)
	@echo "!!! genimg.py - CDT - start !!!"
	$(PYTHON) $(build_dir)/genimg.py \
		--cdt_gen=$(build_dir)/cdt_generator.py \
		--configdir=$(build_dir)/config/ \
		--cdt_modxml=$(build_dir)/config/pcddr_40xx.xml \
		--outdir=$(BIN_DIR) \
		--image_name=CDT_IMAGES
	@echo "!!! genimg.py - end !!!"
endef

define GenMIBIB
	@echo ""
	@echo -e "\e[32m=== Generate MIBIB ===\e[0m"
	@echo ""
	@echo "!!! genimg.py - MIBIB - start !!!"
	$(PYTHON) $(build_dir)/genimg.py \
		--partition_tool=$(build_dir)/partition_tool \
		--mbn_gen=$(build_dir)/nand_mbn_generator.py \
		--smem_gen=$(build_dir)/smem-tool.py \
		--configdir=$(build_dir)/config/ \
		--outdir=$(BIN_DIR) \
		--image_name=$(Image_Name)
	@echo "!!! genimg.py - end !!!"
endef

define BuildFitKRimg
	@echo ""
	@echo -e "\e[32m=== Build KR image ===\e[0m"
	@echo ""
	@find $(BIN_DIR) -type l -delete
	@ls $(build_dir) | xargs -i ln -sf $(build_dir)/{} $(BIN_DIR)/{}
	$(PYTHON) $(build_dir)/pack.py \
		-t $(IMG_TYPE) \
		-B \
		-F $(build_dir)/config/apps_config \
		-d $(DUAL_IMAGE) \
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
	@ls $(build_dir) | xargs -i ln -sf $(build_dir)/{} $(BIN_DIR)/{};
	$(PYTHON) $(build_dir)/pack.py \
		-t $(IMG_TYPE) \
		-B \
		-F $(build_dir)/config/all_config \
		-d $(DUAL_IMAGE) \
		-o $(fit_all) \
		$(BIN_DIR) ;
	@find $(BIN_DIR) -type l -delete;	
	@cp $(BIN_DIR)/flash.scr $(BIN_DIR)/flash.scr.all 2>/dev/null || true
	@cp $(BIN_DIR)/flash.its $(BIN_DIR)/flash.its.all 2>/dev/null || true
endef

QCA_IMG_PREFIX=qcom-ipq40xx
QCA_IMG_SUFFIX=fit-uImage
QCA_KNL_IMG_NAME=$(IMG_PREFIX)-$(QCA_IMG_PREFIX)-$(QCA_BOARD)-$(QCA_IMG_SUFFIX).itb
define CopyRenameImg	
	-cp $(BIN_DIR)/$(QCA_KNL_IMG_NAME) $(BIN_DIR)/$(IMG_PREFIX)-$(PRODUCT_NAME_L)-$(FW_KERNEL_SUFFIX)
	-cp $(BIN_DIR)/$(IMG_PREFIX)-$(FW_ROOTFS_SUFFIX) $(BIN_DIR)/$(IMG_PREFIX)-$(PRODUCT_NAME_L)-$(FW_ROOTFS_SUFFIX)	
endef

