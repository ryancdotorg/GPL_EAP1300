###################################################
# uboot platform flags, for TI BSP
# MPU clock = 800MHz, AM3352BZCZ80
###################################################
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_AM335X_PLATFORM
#export PLATFORM_RELFLAGS+=-DCONFIG_SYS_CUSTOMIZE_MPUCLK=800 # Move to Makefile
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_LITTLE_ENDIAN
###################################################
# uboot product flags, for hardware platform
###################################################
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_CUSTOMIZATION
# icplus 101 phy address default value is 1
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_CPSW_PHY1_ADDR=1 -DCONFIG_UBOOT_CPSW_PHY2_ADDR=0

###################################################
# Hardware configuration
###################################################
# DDR2 parameter configuration
#export PLATFORM_RELFLAGS+=-DDDR2_NT5TU64M16HG_AC    # Move to Makefile
# DDR3 NT5CC256M16DP-DI, MT41K256M16HZ-125E on beaglebone black
# 512MB, 800MHz
#export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_HAS_DDR3   # Move to Makefile
#export PLATFORM_RELFLAGS+=-DDDR3_NT5CC256M16DP_DI   # Move to Makefile

# export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_SUPPORT_EEPROM
# export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_SUPPORT_PMIC

#export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_HAS_LTE_PLS8X # MOve to Makefile

###################################################
# U-ENVIRONMENT
###################################################
#export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_SYS_BOOT_LEN=32 # 32MiB length, Move to Makefile
#export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_IPADDR=192.168.1.1
#export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_SERVERIP=192.168.1.183
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_NETMASK=255.255.255.0
# export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_BOARD_INFO_FROM_EEPROM

###################################################
# Feature configuration
###################################################
# Access SPI flash by using FIFO
export PLATFORM_RELFLAGS+=-DCONFIG_MCSPI0_FIFO
# Modified SPI flash for 4K sector format
#export PLATFORM_RELFLAGS+=-DMACRONIX_4K_SECTOR 
export PLATFORM_RELFLAGS+=-DCONFIG_SPI_FLASH_BLOCK_LOCK
# Disable SPI flash protect
export PLATFORM_RELFLAGS+=-DCONFIG_SPI_FLASH_PROTECT_DISABLE
#export PLATFORM_RELFLAGS+=-DCONFIG_SPI_FLASH_SIZE_32MIB # Move to Makefile, undef is for 16MIB

# Recovery mode
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_RECOVERY_MODE -DCONFIG_UBOOT_RECOVERY_MODE_LED_BLINK
export PLATFORM_RELFLAGS+=-DDEFAULT_IP_ADDR=\"192.168.1.1\"
export PLATFORM_RELFLAGS+=-DMAGIC_KEY=0x12345678
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_RECOVERY_TO_MMC
#export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_RECOVERY_TO_SPI

# Silent console
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_SILENT_CONSOLE

# RTC used internal 32KHZ, depend on "CONFIG_SPL_AM33XX_ENABLE_RTC32K_OSC"
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_RTC_INTERNAL_32KHZ

