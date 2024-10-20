define Gen_Bootloader
	$(call Get_IPL)
	$(call Get_CIS)
	$(call Gen_Part)
	$(call Gen_Total)
endef

staging_dir=$(BIN_DIR)/build

define Get_IPL
	rm -rf $(staging_dir)
	mkdir -p $(staging_dir)
	cd files/$(PRODUCT_NAME) && $(CP) IPL*.bin $(staging_dir)
endef

define Get_CIS
	mkdir -p $(staging_dir)
	cd files/$(PRODUCT_NAME) && $(CP) GCIS.bin $(staging_dir)
endef

ifneq ($(CONFIG_PACKAGE_uboot-sstar-infinity5-SENAO_SPINOR),)
# SPI NOR
loader=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-bootloader.bin

define Gen_Part
	@echo -e "$(_G)  [[Gen_Part]] $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-mxp_sf.bin$(_N)"
	$(CP) tools/mxp_gen.c $(BIN_DIR)/
	echo gcc $(BIN_DIR)/mxp_gen.c -o $(BIN_DIR)/mxp_gen -I../../SENAO/configs/
	( cd $(BIN_DIR) && \
		gcc mxp_gen.c -o mxp_gen -I../../SENAO/configs/ )
	$(BIN_DIR)/mxp_gen $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-mxp_sf.bin && chmod +r $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-mxp_sf.bin
endef

define Gen_Total
	@echo -e "$(_G)  [[Gen_Total]] $(loader)$(_N)"
	echo "# Append IPL, default bs = 512 bytes" > /dev/null
	dd if=$(staging_dir)/IPL.bin of=$(loader)
	echo "append real mxp.bin in 0xc000= 49152 bytes, 49152/512=96 count" > /dev/null
	dd if=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-mxp_sf.bin of=$(loader) seek=96
	echo "append IPL_CUST.bin in 0x10000= 65536 bytes, 65536/512=128 count" > /dev/null
	dd if=$(staging_dir)/IPL_CUST.bin of=$(loader) seek=128
	echo "append backup mxp.bin in 0x20000 = 131072 bytes, 131072/512=256 count" > /dev/null
	dd if=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-mxp_sf.bin of=$(loader) seek=256
	echo "append uboot.bin in 0x30000 = 196608 bytes, 196608/512=384 count" > /dev/null
	dd if=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-u-boot.xz.bin of=$(loader) seek=384
endef

kernel_part_offset=$(shell $(BIN_DIR)/mxp_gen -n KERNEL $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-mxp_sf.bin | cut -d ':' -f 1)
kernel_part_size=$(shell $(BIN_DIR)/mxp_gen -n KERNEL $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-mxp_sf.bin | cut -d ':' -f 2)
rootfs_part_offset=$(shell $(BIN_DIR)/mxp_gen -n rootfs $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-mxp_sf.bin | cut -d ':' -f 1)
rootfs_part_size=$(shell $(BIN_DIR)/mxp_gen -n rootfs $(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-mxp_sf.bin | cut -d ':' -f 2)

define SenaoCatFiles
	if [ `stat -c%s "$(1)"` -gt $(2) ]; then \
		echo "Warning: $(1) is too big"; \
	else if [ `stat -c%s $(3)` -gt $(4) ]; then \
		echo "Warning: $(3) is too big"; \
	else \
		( dd if=$(1) bs=$(2) conv=sync; dd if=$(3) ) > $(5); \
	fi; fi ;
	$(call prepare_generic_squashfs,$(5))
endef

define Sysupgrade
	@echo -e "$(_G)  [[Sysupgrade]] $(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_KERNEL_ROOTFS_SUFFIX)$(_N)"
	$(call SenaoCatFiles,$(LINUX_DIR)/arch/arm/boot/uImage.xz,$(kernel_part_size),$(KDIR)/root.squashfs,$(rootfs_part_size),$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-$(FW_KERNEL_ROOTFS_SUFFIX))

endef

else # CONFIG_PACKAGE_uboot-sstar-infinity5-SENAO_SPINOR
# SPI NAND

loader=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-bootloader_spinand.bin
# ref. [[ipl.es
#device nand0 <nand0>, # parts = 10
# #: name                size            offset          mask_flags
# 0: IPL0                0x00020000      0x00060000      0
# 1: IPL1                0x00020000      0x00080000      0
# 2: IPL2                0x00020000      0x000a0000      0
# 3: IPL_CUST0           0x00020000      0x000c0000      0
# 4: IPL_CUST1           0x00020000      0x000e0000      0
# 5: IPL_CUST2           0x00020000      0x00100000      0
# 6: UBOOT0              0x00060000      0x00120000      0
# 7: UBOOT1              0x00060000      0x00180000      0
# 8: ENV                 0x00020000      0x001e0000      0
# 9: rootfs              0x07e00000      0x00200000      0

define Gen_Total
	@echo -e "$(_G)  [[Gen_Total]] $(loader)$(_N)"
	echo "# Append GCIS, default bs = 512 bytes" > /dev/null
	dd if=$(staging_dir)/GCIS.bin of=$(loader)
	echo "append IPL0 in 0x60000 = 393216 bytes, 393216/512=768 count" > /dev/null
	dd if=$(staging_dir)/IPL.bin of=$(loader) seek=768
	echo "append IPL1 in 0x80000 = 524288 bytes, 524288/512=1024 count" > /dev/null
	dd if=$(staging_dir)/IPL.bin of=$(loader) seek=1024
#	echo "append IPL2 in 0xa0000 = 655360 bytes, 655360/512=1280 count" > /dev/null
#	dd if=$(staging_dir)/IPL.bin of=$(loader) seek=1280
	echo "append IPL_CUST0 in 0xc0000 = 786432 bytes, 786432/512=1536 count" > /dev/null
	dd if=$(staging_dir)/IPL_CUST.bin of=$(loader) seek=1536
	echo "append IPL_CUST1 in 0xe0000 = 917504 bytes, 917504/512=1792 count" > /dev/null
	dd if=$(staging_dir)/IPL_CUST.bin of=$(loader) seek=1792
#	echo "append IPL_CUST2 in 0x100000 = 1048576 bytes, 1048576/512=2048 count" > /dev/null
#	dd if=$(staging_dir)/IPL_CUST.bin of=$(loader) seek=2048
	echo "append UBOOT0 in 0x120000 = 1179648 bytes, 1179648/512=2304 count" > /dev/null
	dd if=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-u-boot_spinand.xz.bin of=$(loader) seek=2304
	echo "append UBOOT1 in 0x180000 = 1572864 bytes, 1572864/512=3072 count" > /dev/null
	dd if=$(BIN_DIR)/$(FW_PREFIX)$(PRODUCT_NAME_L)-u-boot_spinand.xz.bin of=$(loader) seek=3072
endef
endif

