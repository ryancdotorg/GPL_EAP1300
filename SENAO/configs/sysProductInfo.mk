TOP_DIR?=$(TOPDIR)
SN_VENDOR_DIR?=$(dir $(realpath $(TOP_DIR)/SENAO/configs/product_config.make))

export PRODUCT_NAME=EAP1300
export PRODUCT_NAME_U=EAP1300
export PRODUCT_NAME_L=eap1300
export SN_BOARD=$(shell grep CONFIG_TARGET_BOARD $(SN_VENDOR_DIR)/product_config.make | awk -F\" '{print $$2}')
ifneq ($(shell grep CONFIG_SENAOWRT_IMAGE=y $(SN_VENDOR_DIR)/product_config.make),)
export SN_IMAGE=y
else
export SN_IMAGE=n
endif
# TODO: use gmenu?
export QCA_SP_REPO=qca_spf_5_3_CS
export QCA_BOARD=ap.dk01.1-c1

export FW_PREFIX=$(shell grep -ws FW_PREFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')
#export FW_UBOOT_SUFFIX=$(shell grep -ws FW_UBOOT_SUFFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')
export FW_KERNEL_SUFFIX=$(shell grep -ws FW_KERNEL_SUFFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')
export FW_ROOTFS_SUFFIX=$(shell grep -ws FW_ROOTFS_SUFFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')
#export FW_KERNEL_ROOTFS_SUFFIX=$(shell grep -ws FW_KERNEL_ROOTFS_SUFFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')

export FW_FIT_NOR_UBOOT_SUFFIX=$(shell grep -ws FW_FIT_NOR_UBOOT_SUFFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')
export FW_FIT_NOR_KR_SUFFIX=$(shell grep -ws FW_FIT_NOR_KR_SUFFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')
export FW_FIT_NOR_ALL_SUFFIX=$(shell grep -ws FW_FIT_NOR_ALL_SUFFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')
export FW_FIT_NAND_UBOOT_SUFFIX=$(shell grep -ws FW_FIT_NAND_UBOOT_SUFFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')
export FW_FIT_NAND_KR_SUFFIX=$(shell grep -ws FW_FIT_NAND_KR_SUFFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')
export FW_FIT_NAND_ALL_SUFFIX=$(shell grep -ws FW_FIT_NAND_ALL_SUFFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')
export FW_FIT_NORNAND_UBOOT_SUFFIX=$(shell grep -ws FW_FIT_NORNAND_UBOOT_SUFFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')
export FW_FIT_NORNAND_KR_SUFFIX=$(shell grep -ws FW_FIT_NORNAND_KR_SUFFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')
export FW_FIT_NORNAND_ALL_SUFFIX=$(shell grep -ws FW_FIT_NORNAND_ALL_SUFFIX $(SN_VENDOR_DIR)/gconfig.h | awk -F "\"" '{print $$2}')

export IMG_TYPE=$(shell grep "_available=true" $(SN_VENDOR_DIR)/ipq-image-config/boardconfig |cut -d '_' -f 1)

ifeq ($(IMG_TYPE),nor)
export FW_UBOOT_SUFFIX=$(FW_FIT_NOR_UBOOT_SUFFIX)
export FW_KERNEL_ROOTFS_SUFFIX=$(FW_FIT_NOR_KR_SUFFIX)
else ifeq ($(IMG_TYPE),nand)
export FW_UBOOT_SUFFIX=$(FW_FIT_NAND_UBOOT_SUFFIX)
export FW_KERNEL_ROOTFS_SUFFIX=$(FW_FIT_NAND_KR_SUFFIX)
else
export FW_UBOOT_SUFFIX=$(FW_FIT_NORNAND_UBOOT_SUFFIX)
export FW_KERNEL_ROOTFS_SUFFIX=$(FW_FIT_NORNAND_KR_SUFFIX)
endif

# set boardData
export BOARDDATA_2G=boardData_1_0_IPQ4019_Y9803_wifi0.bin
export BOARDDATA_5G=boardData_1_0_IPQ4019_Y9803_wifi1.bin
export BOARDDATA_3RF=fakeBoardData_AR6004.bin
export BOARDDATA_2G_PATH=IPQ4019/hw.1
export BOARDDATA_5G_PATH=IPQ4019/hw.1
export BOARDDATA_3RF_PATH=AR9887

