# SP-935 hardware platform
# CPU: TI AM3352 600MHz, 128MB DDR2
# SPI Flash
# Modify to SP-938 BS soon.
export PLATFORM_RELFLAGS+=-DCONFIG_SP938BS_LED
export PLATFORM_RELFLAGS+=-DCONFIG_HARDWARE_SP938BS
export PLATFORM_RELFLAGS+=-DCONFIG_SYS_CUSTOMIZE_MPUCLK=600
export PLATFORM_RELFLAGS+=-DDDR2_NT5TU64M16HG_AC
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_SYS_BOOT_LEN=16 # 16MiB length, change to 32MiB after changing flash
#export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_UIMAGE_WITH_HEADER # in file driver/spi/omap3_spi.c
export PLATFORM_RELFLAGS+=-DCONFIG_SPI_FLASH_SIZE_32MIB # undef is 16MIB SPI FLASH
export PLATFORM_RELFLAGS+=-DCONFIG_PHY_INTERFACE_RGMII
export PLATFORM_RELFLAGS+=-DCONFIG_UBOOT_AR803x_PHY1_ADDR=2 