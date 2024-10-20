/*
 * ti_armv7_common.h
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * The various ARMv7 SoCs from TI all share a number of IP blocks when
 * implementing a given feature.  Rather than define these in every
 * board or even SoC common file, we define a common file to be re-used
 * in all cases.  While technically true that some of these details are
 * configurable at the board design, they are common throughout SoC
 * reference platforms as well as custom designs and become de facto
 * standards.
 */

#ifndef __CONFIG_TI_ARMV7_COMMON_H__
#define __CONFIG_TI_ARMV7_COMMON_H__

/*
 * We typically do not contain NOR flash.  In the cases where we do, we
 * undefine this later.
 */
#define CONFIG_SYS_NO_FLASH

/* Support both device trees and ATAGs. */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG

/*
 * Our DDR memory always starts at 0x80000000 and U-Boot shall have
 * relocated itself to higher in memory by the time this value is used.
 * However, set this to a 32MB offset to allow for easier Linux kernel
 * booting as the default is often used as the kernel load address.
 */
#define CONFIG_SYS_LOAD_ADDR		0x82000000

/*
 * We setup defaults based on constraints from the Linux kernel, which should
 * also be safe elsewhere.  We have the default load at 32MB into DDR (for
 * the kernel), FDT above 128MB (the maximum location for the end of the
 * kernel), and the ramdisk 512KB above that (allowing for hopefully never
 * seen large trees).  We say all of this must be within the first 256MB
 * as that will normally be within the kernel lowmem and thus visible via
 * bootm_size and we only run on platforms with 256MB or more of memory.
 */
#define DEFAULT_LINUX_BOOT_ENV \
	"loadaddr=0x82000000\0" \
	"kernel_addr_r=0x82000000\0" \
	"fdtaddr=0x87000000\0" \
	"fdt_addr_r=0x87000000\0" \
	"rdaddr=0x88080000\0" \
	"ramdisk_addr_r=0x88080000\0" \
	"scriptaddr=0x80000000\0" \
	"pxefile_addr_r=0x80100000\0" \
	"bootm_size=0x08000000\0" \
	"boot_fdt=no\0"

#ifdef CONFIG_UBOOT_MMC_DEV0
#define BOOT_MMC_DEV "mmcdev=0\0"
#elif CONFIG_UBOOT_MMC_DEV1
#define BOOT_MMC_DEV "mmcdev=1\0"
#endif

#define OPENWRT_RDINIT_MMC "rdinitmmc=/etc/preinit\0"
#define OPENWRT_RDINIT_RAM "rdinitram=/etc/preinit\0"
#define OPENWRT_RDINIT_NET "rdinitnet=/etc/preinit\0"
#define OPENWRT_RDINIT_SPI "rdinitspi=/etc/preinit\0"

#define OPENWRT_OVERLAY_MMC "overlaymmc=/dev/mmcblk0p5\0"


#ifdef CONFIG_HARDWARE_EIG9100  /* eMMC */
#define OPENWRT_OVERLAY_RAM "overlayram=/dev/mmcblk0p5\0"
#define OPENWRT_OVERLAY_SPI "overlayspi=/dev/mmcblk0p5\0"
#define OPENWRT_OVERLAY_RAM_ROOTFS "ramrootfstype=ext2\0"
#elif CONFIG_HARDWARE_SP938BS   /* SPI flash */
#define OPENWRT_OVERLAY_RAM "overlayram=/dev/mtdblock5\0"
#define OPENWRT_OVERLAY_SPI "overlayspi=/dev/mtdblock5\0"
#define OPENWRT_OVERLAY_RAM_ROOTFS "ramrootfstype=jffs2\0"
#elif CONFIG_HARDWARE_SP935   /* SPI flash */
#define OPENWRT_OVERLAY_RAM "overlayram=/dev/mtdblock5\0"
#define OPENWRT_OVERLAY_SPI "overlayspi=/dev/mtdblock5\0"
#define OPENWRT_OVERLAY_RAM_ROOTFS "ramrootfstype=jffs2\0"
#endif
#define OPENWRT_OVERLAY_NET "overlaynet=/dev/mmcblk0p5\0"

/*#define OPENWRT_OVERLAY "overlayargs=/dev/mtdblock5\0"*/
#define BOOT_MMC_ROOT "mmcrootfs=PARTUUID=${uuid} ro\0"

#define DEFAULT_MMC_TI_ARGS \
	BOOT_MMC_DEV \
	BOOT_MMC_ROOT \
	OPENWRT_RDINIT_MMC \
	OPENWRT_OVERLAY_MMC \
	"mmcrootfstype=ext4 rootwait\0" \
	"finduuid=part uuid mmc ${bootpart} uuid\0" \
	"args_mmc=run finduuid;setenv bootargs console=${console} " \
		"${optargs} " \
		"root=PARTUUID=${uuid} rw " \
		"rootfstype=${mmcrootfstype} " \
		"rdinit=${rdinitmmc} " \
		"overlay=${overlaymmc}\0" \
	"loadbootscript=load mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc${mmcdev} ...; " \
		"source ${loadaddr}\0" \
	"bootenvfile=uEnv.txt\0" \
	"importbootenv=echo Importing environment from mmc${mmcdev} ...; " \
		"env import -t ${loadaddr} ${filesize}\0" \
	"loadbootenv=fatload mmc ${mmcdev} ${loadaddr} ${bootenvfile}\0" \
	"loadimage=load ${devtype} ${bootpart} ${loadaddr} ${bootdir}/${bootfile}\0" \
	"loadfdt=load ${devtype} ${bootpart} ${fdtaddr} ${bootdir}/${fdtfile}\0" \
	"envboot=mmc dev ${mmcdev}; " \
		"if mmc rescan; then " \
			"echo SD/MMC found on device ${mmcdev};" \
			"if run loadbootscript; then " \
				"run bootscript;" \
			"else " \
				"if run loadbootenv; then " \
					"echo Loaded env from ${bootenvfile};" \
					"run importbootenv;" \
				"fi;" \
				"if test -n $uenvcmd; then " \
					"echo Running uenvcmd ...;" \
					"run uenvcmd;" \
				"fi;" \
			"fi;" \
		"fi;\0" \
	"mmcloados=run args_mmc; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootm ${loadaddr} - ${fdtaddr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootm; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0" \
	"mmcboot=mmc dev ${mmcdev}; " \
		"setenv devnum ${mmcdev}; " \
		"setenv devtype mmc; " \
		"if mmc rescan; then " \
			"echo SD/MMC found on device ${mmcdev};" \
			"if run loadimage; then " \
				"if test ${boot_fit} -eq 1; then " \
					"run loadfit; " \
				"else " \
					"run mmcloados;" \
				"fi;" \
			"fi;" \
		"fi;\0" \

#define DEFAULT_FIT_TI_ARGS \
	"boot_fit=0\0" \
	"fit_loadaddr=0x88000000\0" \
	"fit_bootfile=fitImage.itb\0" \
	"update_to_fit=setenv loadaddr ${fit_loadaddr}; setenv bootfile ${fit_bootfile}\0" \
	"args_fit=setenv bootargs console=${console} \0" \
	"loadfit=run args_fit; bootm ${loadaddr}#${fdtfile};\0" \

#define DEFAULT_SPI_ENVIRONMENT \
	"spiroot=/dev/mtdblock4 rw\0" \
	"spirootfstype=jffs2\0" \
	"spisrcaddr=0xe0000\0" \
	"spiimgsize=0x362000\0" \
	"spibusno=0\0" \
	"spiargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"rdinit=${rdinitspi} " \
		"overlay=${overlayspi}" \
		"root=${spiroot} " \
		"rootfstype=${spirootfstype}\0"

#define DEFAULT_SPI_BOOT \
	"spiboot=echo Booting from spi ...; " \
	"run spiargs; " \
	"sf probe ${spibusno}:0; " \
	"sf read ${loadaddr} ${spisrcaddr} ${spiimgsize}; " \
	"bootz ${loadaddr}\0"

/*
 * DDR information.  If the CONFIG_NR_DRAM_BANKS is not defined,
 * we say (for simplicity) that we have 1 bank, always, even when
 * we have more.  We always start at 0x80000000, and we place the
 * initial stack pointer in our SRAM. Otherwise, we can define
 * CONFIG_NR_DRAM_BANKS before including this file.
 */
#ifndef CONFIG_NR_DRAM_BANKS
#define CONFIG_NR_DRAM_BANKS		1
#endif
#define CONFIG_SYS_SDRAM_BASE		0x80000000

#ifndef CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_SYS_INIT_SP_ADDR         (NON_SECURE_SRAM_END - \
						GENERATED_GBL_DATA_SIZE)
#endif

/* Timer information. */
#define CONFIG_SYS_PTV			2	/* Divisor: 2^(PTV+1) => 8 */

/*
 * Disable DM_* for SPL build and can be re-enabled after adding
 * DM support in SPL
 */
#ifdef CONFIG_SPL_BUILD
#undef CONFIG_DM_I2C
#endif

/* I2C IP block */
#define CONFIG_I2C
#ifndef CONFIG_DM_I2C
#define CONFIG_SYS_I2C
#else
/*
 * Enable CONFIG_DM_I2C_COMPAT temporarily until all the i2c client
 * devices are adopted to DM
 */
#define CONFIG_DM_I2C_COMPAT
#endif

/* MMC/SD IP block */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC

/* McSPI IP block */
#define CONFIG_SPI

/* GPIO block */

/*
 * The following are general good-enough settings for U-Boot.  We set a
 * large malloc pool as we generally have a lot of DDR, and we opt for
 * function over binary size in the main portion of U-Boot as this is
 * generally easily constrained later if needed.  We enable the config
 * options that give us information in the environment about what board
 * we are on so we do not need to rely on the command prompt.  We set a
 * console baudrate of 115200 and use the default baud rate table.
 */
#ifdef CONFIG_DFU_MMC
#define CONFIG_SYS_MALLOC_LEN	((16 << 20) + CONFIG_SYS_DFU_DATA_BUF_SIZE)
#else
#define CONFIG_SYS_MALLOC_LEN	(16 << 20)
#endif
#define CONFIG_SYS_CONSOLE_INFO_QUIET
#define CONFIG_BAUDRATE			115200
#define CONFIG_ENV_VARS_UBOOT_CONFIG	/* Strongly encouraged */
#define CONFIG_ENV_OVERWRITE		/* Overwrite ethaddr / serial# */

/* As stated above, the following choices are optional. */
#define CONFIG_SYS_LONGHELP
#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_VERSION_VARIABLE

/* We set the max number of command args high to avoid HUSH bugs. */
#define CONFIG_SYS_MAXARGS		64

/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE		1024
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE \
					+ sizeof(CONFIG_SYS_PROMPT) + 16)
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/*
 * When we have SPI, NOR or NAND flash we expect to be making use of
 * mtdparts, both for ease of use in U-Boot and for passing information
 * on to the Linux kernel.
 */
#if defined(CONFIG_SPI_BOOT) || defined(CONFIG_NOR) || defined(CONFIG_NAND) || defined(CONFIG_NAND_DAVINCI)
#define CONFIG_MTD_DEVICE		/* Required for mtdparts */
#define CONFIG_CMD_MTDPARTS
#endif

#define CONFIG_SUPPORT_RAW_INITRD

/*
 * Common filesystems support.  When we have removable storage we
 * enabled a number of useful commands and support.
 */
#if defined(CONFIG_MMC) || defined(CONFIG_USB_STORAGE)
#define CONFIG_DOS_PARTITION
#define CONFIG_FAT_WRITE
#define CONFIG_PARTITION_UUIDS
#define CONFIG_CMD_PART
#endif

/*
 * Our platforms make use of SPL to initalize the hardware (primarily
 * memory) enough for full U-Boot to be loaded. We make use of the general
 * SPL framework found under common/spl/.  Given our generally common memory
 * map, we set a number of related defaults and sizes here.
 */
#if !defined(CONFIG_NOR_BOOT) && \
	!(defined(CONFIG_QSPI_BOOT) && defined(CONFIG_AM43XX))
#define CONFIG_SPL_FRAMEWORK

/*
 * We also support Falcon Mode so that the Linux kernel can be booted
 * directly from SPL. This is not currently available on HS devices.
 */
#if !defined(CONFIG_TI_SECURE_DEVICE)
#define CONFIG_SPL_OS_BOOT
#endif

/*
 * Place the image at the start of the ROM defined image space (per
 * CONFIG_SPL_TEXT_BASE and we limit our size to the ROM-defined
 * downloaded image area.  We initalize DRAM as soon as we can so that
 * we can place stack, malloc and BSS there.  We load U-Boot itself into
 * memory at 0x80800000 for legacy reasons (to not conflict with older
 * SPLs).  We have our BSS be placed 2MiB after this, to allow for the
 * default Linux kernel address of 0x80008000 to work with most sized
 * kernels, in the Falcon Mode case.  We have the SPL malloc pool at the
 * end of the BSS area.  We suggest that the stack be placed at 32MiB after
 * the start of DRAM to allow room for all of the above (handled in Kconfig).
 */
#ifndef CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_TEXT_BASE		0x80800000
#endif
#ifndef CONFIG_SPL_BSS_START_ADDR
#define CONFIG_SPL_BSS_START_ADDR	0x80a00000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000		/* 512 KB */
#endif
#ifndef CONFIG_SYS_SPL_MALLOC_START
#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SPL_BSS_START_ADDR + \
					 CONFIG_SPL_BSS_MAX_SIZE)
#define CONFIG_SYS_SPL_MALLOC_SIZE	CONFIG_SYS_MALLOC_LEN
#endif

/* RAW SD card / eMMC locations. */
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR	0x300 /* address 0x60000 */
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x200 /* 256 KB */

/* FAT sd card locations. */
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot.img"

#ifdef CONFIG_SPL_OS_BOOT
/* FAT */
#define CONFIG_SPL_FS_LOAD_KERNEL_NAME		"uImage"
#define CONFIG_SPL_FS_LOAD_ARGS_NAME		"args"

/* RAW SD card / eMMC */
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0x900	/* address 0x120000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0x80	/* address 0x10000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	0x80	/* 64KiB */

/* spl export command */
#define CONFIG_CMD_SPL
#endif

#ifdef CONFIG_MMC
#define CONFIG_SPL_LIBDISK_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_EXT_SUPPORT
#endif

#define CONFIG_SYS_THUMB_BUILD

/* General parts of the framework, required. */
#define CONFIG_SPL_I2C_SUPPORT
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT
#define CONFIG_SPL_POWER_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_BOARD_INIT


#define ERASE_UENV      ""
#define DL_MLO          ""
#define DL_UBOOT        ""
#define DL_KERNEL       ""
#define DL_FDT          ""
#define CUSTOMIZE_UENV  ""

#if defined(CONFIG_UBOOT_AM335X_PLATFORM)
#if defined(CONFIG_UBOOT_SPI_BOOT)

#ifdef CONFIG_HARDWARE_EIG9100
#define IMAGE_FILE_NAME "img_name=openwrt-omap-zImage-initramfs-dtb\0"
#elif CONFIG_HARDWARE_SP938BS
#define IMAGE_FILE_NAME "img_name=openwrt-am335x-sp938bs-norplusnand-fw-s.img\0"
#elif CONFIG_HARDWARE_SP935   /* SPI flash */
#define IMAGE_FILE_NAME "img_name=openwrt-am335x-sp935-norplusnand-fw-s.img\0"
#else
#define IMAGE_FILE_NAME ""
#endif

/* SP935 download command */
#define SP935_DL_MLO \
	"dlmlo=echo Download MLO.byteswap from tftpboot ... ; " \
		"tftp ${loadaddr} MLO.byteswap; " \
		"sf probe ${spibusno}:0; " \
		"sf erase ${spimloaddr} ${spimlosize}; " \
		"sf write ${loadaddr} ${spimloaddr} ${spimlosize}\0"

#define SP935_DL_UBOOT \
	"dluboot=echo Download u-boot.img from tftpboot ... ; " \
		"tftp ${loadaddr} u-boot.img; " \
		"sf probe ${spibusno}:0; " \
		"sf erase ${spiubootaddr} ${spiubootsize}; " \
		"sf write ${loadaddr} ${spiubootaddr} ${spiubootsize}\0"

#define SP935_DL_KERNEL \
	"dlkl=echo Download uImage from tftpboot ... ; " \
		"tftp ${loadaddr} ${img_name}; " \
		"sf probe ${spibusno}:0; " \
		"sf erase ${spisrcaddr} ${filesize}; " \
		"sf write ${loadaddr} ${spisrcaddr} ${filesize}\0"

#define SP935_DL_FDT \
	"dlfdt=echo Download fdt file from tftpboot ... ; " \
		"tftp ${loadaddr} ${fdtfile}; " \
		"sf probe ${spibusno}:0; " \
		"sf erase ${spifdtsrcaddr} ${spifdtimgsize}; " \
		"sf write ${loadaddr} ${spifdtsrcaddr} ${filesize}\0"

#ifdef CONFIG_HARDWARE_EIG9100
#define MMC_DL_UBOOT \
	"mmcdluboot=echo Download mlo, MLO and u-boot.img from tftpboot ... ; " \
		"tftp ${loadaddr} MLO; " \
		"fatwrite mmc 1:1 ${loadaddr} MLO ${filesize}; " \
		"tftp ${loadaddr} u-boot.img; " \
		"fatwrite mmc 1:1 ${loadaddr} u-boot.img ${filesize}; " \
		"ls mmc 1:1\0"

#define MMC_DL_KERNEL \
	"mmcdlkl=echo Download uImage and am335x-emv.dtb from tftpboot ... ; " \
		"tftp ${loadaddr} openwrt-omap-zImage-initramfs-dtb; " \
		"ext4write mmc 1:2 ${loadaddr} /boot/zImage ${filesize}; " \
		"ls mmc 1:2\0"
#else   /* #ifdef CONFIG_HARDWARE_EIG9100 */
/* empty */
#define MMC_DL_UBOOT ""
#define MMC_DL_KERNEL ""
#endif  /* #ifdef CONFIG_HARDWARE_EIG9100 */

#define SP935_EXTRA_UENV \
	"rssi_70db=0cc\0" \
	"rssi_85db=09a\0" \
	"rxif_cal=0\0" \
	"rf1_pll_hw=2430\0" \
	"rf2_pll_hw=2430\0" \
	"dac_cal=0128\0"	

#undef DL_MLO      
#undef DL_UBOOT    
#undef DL_KERNEL   
#undef DL_FDT      
#undef CUSTOMIZE_UENV
#undef DEFAULT_SPI_BOOT
#undef DEFAULT_SPI_ENVIRONMENT
#undef ERASE_UENV

#define DL_MLO          SP935_DL_MLO
#define DL_UBOOT        SP935_DL_UBOOT
#define DL_KERNEL       SP935_DL_KERNEL
#define DL_FDT          SP935_DL_FDT
#define CUSTOMIZE_UENV  SP935_EXTRA_UENV

#define ERASE_UENV \
	"eraseuenv=sf probe ${spibusno}:0; " \
	"sf erase ${spiuenvaddr} ${spiuenvsize}\0"

#define DEFAULT_SPI_BOOT \
	OPENWRT_RDINIT_SPI \
	OPENWRT_OVERLAY_SPI \
	"spiboot=echo Booting from spi ...; " \
	"run spiramfsargs; " \
	"sf probe ${spibusno}:0; " \
	"sf read ${spikloadaddr} ${spisrcaddr} ${spiimgsize}; " \
	"bootz ${spikloadaddr}\0"

#if defined(CONFIG_SPI_FLASH_SIZE_32MIB)
#define SPI_IMG_SIZE "spiimgsize=0x1E40000\0"
#else
/* 16MIB size */
#define SPI_IMG_SIZE "spiimgsize=0xE40000\0"
#endif


#if (defined(CONFIG_HARDWARE_SP938BS) || defined(CONFIG_HARDWARE_SP935))
#define DEFAULT_SPI_ENVIRONMENT \
	"spimloaddr=0x00000\0" \
	"spimlosize=0x20000\0" \
	"spiubootaddr=0x20000\0" \
	"spiubootsize=0x80000\0" \
	"spiuenvaddr=0xA0000\0" \
	"spiuenvsize=0x10000\0" \
	"spisrcaddr=0x0E0000\0" \
	SPI_IMG_SIZE \
	IMAGE_FILE_NAME \
	"spifdtsrcaddr=0x0B0000\0" \
	"spifdtimgsize=0x010000\0" \
	"spikloadaddr=0x82000000\0" \
	"spifdtaddr=0x87000000\0" \
	"kzloadaddr=0x80008000\0" \
	"spiroot=/dev/mtdblock4 rw\0" \
	"spirootfstype=jffs2\0" \
	"spibusno=0\0" \
	"spiramfsargs=setenv bootargs console=${console} " \
	"${optargs} " \
	"root=${spiroot} " \
	"rootfstype=${spirootfstype} " \
	"rdinit=/etc/preinit " \
	"overlay=/dev/mtdblock5\0"

		/*
		"ramargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=${ramroot} " \
		"rootfstype=${ramrootfstype} " \
		"rdinit=${rdinitram} " \
		"overlay=${overlayram}\0" \
		*/
#else
#define DEFAULT_SPI_ENVIRONMENT \
	"spimloaddr=0x00000\0" \
	"spimlosize=0x20000\0" \
	"spiubootaddr=0x20000\0" \
	"spiubootsize=0x80000\0" \
	"spiuenvaddr=0xA0000\0" \
	"spiuenvsize=0x10000\0" \
	"spisrcaddr=0x0E0000\0" \
	SPI_IMG_SIZE \
	IMAGE_FILE_NAME \
	"spifdtsrcaddr=0x0B0000\0" \
	"spifdtimgsize=0x010000\0" \
	"spikloadaddr=0x82000000\0" \
	"spifdtaddr=0x87000000\0" \
	"kzloadaddr=0x80008000\0" \
	"spiroot=/dev/mtdblock4 rw\0" \
	"spirootfstype=jffs2\0" \
	"spibusno=0\0" \
	"spiramfsargs=setenv bootargs console=${console} " \
	"${optargs} " \
	"root=${spiroot} " \
	"rootfstype=${spirootfstype}\0"
#endif  /* #ifdef CONFIG_HARDWARE_SP938BS || CONFIG_HARDWARE_SP935 */


#endif  /* #if defined(CONFIG_UBOOT_SP935_SPI_BOOT) */

#define RAM_BOOT \
	"ramboot=echo Booting from ramdisk ...; " \
		"tftp ${loadaddr} ${img_name}; " \
		"run ramargs; " \
		"bootz ${loadaddr}\0"

#endif  /* #if defined(CONFIG_UBOOT_AM335X_PLATFORM) */

#ifdef CONFIG_NAND
#define CONFIG_SPL_NAND_SUPPORT
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SPL_MTD_SUPPORT
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#endif
#endif /* !CONFIG_NOR_BOOT */

/* Display CPU and Board info */
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* Generic Environment Variables */

#ifdef CONFIG_CMD_NET
#define NETARGS \
	OPENWRT_RDINIT_NET \
	OPENWRT_OVERLAY_NET \
	"static_ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}" \
		"::off\0" \
	"nfsopts=nolock\0" \
	"rootpath=/tftpboot/target-fs\0" \
	"netloadimage=tftp ${loadaddr} ${bootfile}\0" \
	"netloadfdt=tftp ${fdtaddr} ${fdtfile}\0" \
	"netargs=setenv bootargs console=${console} " \
		"${optargs} " \
		"root=/dev/nfs " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} rw " \
		"rdinit=${rdinitnet} " \
		"overlay=${overlaynet} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}::eth0:off\0" \
	"netboot=echo Booting from network(tftpboot/target-fs) ...; " \
		"setenv autoload no; " \
		"static_ip; " \
		"run netloadimage; " \
		"run netargs; " \
		"bootz ${loadaddr}\0"
#else
#define NETARGS ""
#endif

#define SPI_BOOT        DEFAULT_SPI_BOOT
#define SPI_ENVIRONMENT DEFAULT_SPI_ENVIRONMENT

/*#define CONFIG_IPADDR       CONFIG_UBOOT_IPADDR
#define CONFIG_SERVERIP     CONFIG_UBOOT_SERVERIP*/
#define CONFIG_GATEWAYIP    CONFIG_SERVERIP
#define CONFIG_NETMASK      CONFIG_UBOOT_NETMASK

#include <config_distro_defaults.h>

#endif	/* __CONFIG_TI_ARMV7_COMMON_H__ */

