/*
 * SPI flash internal definitions
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SF_INTERNAL_H_
#define _SF_INTERNAL_H_

#include <linux/types.h>
#include <linux/compiler.h>

/* Dual SPI flash memories - see SPI_COMM_DUAL_... */
enum spi_dual_flash {
	SF_SINGLE_FLASH	= 0,
	SF_DUAL_STACKED_FLASH	= BIT(0),
	SF_DUAL_PARALLEL_FLASH	= BIT(1),
};

/* Enum list - Full read commands */
enum spi_read_cmds {
	ARRAY_SLOW		= BIT(0),
	ARRAY_FAST		= BIT(1),
	DUAL_OUTPUT_FAST	= BIT(2),
	QUAD_OUTPUT_FAST	= BIT(3),
	DUAL_IO_FAST		= BIT(4),
	QUAD_IO_FAST		= BIT(5),
};

/* Normal - Extended - Full command set */
#define RD_NORM		(ARRAY_SLOW | ARRAY_FAST)
#define RD_EXTN		(RD_NORM | DUAL_OUTPUT_FAST | DUAL_IO_FAST)
#define RD_FULL		(RD_EXTN | QUAD_OUTPUT_FAST | QUAD_IO_FAST)

/* sf param flags */
enum {
#ifndef CONFIG_SPI_FLASH_USE_4K_SECTORS
	SECT_4K		= 0,
#else
	SECT_4K		= BIT(0),
#endif
	SECT_32K	= BIT(1),
	E_FSR		= BIT(2),
	SST_WR		= BIT(3),
	WR_QPP		= BIT(4),
};

enum spi_nor_option_flags {
	SNOR_F_SST_WR		= BIT(0),
	SNOR_F_USE_FSR		= BIT(1),
};

#define SPI_FLASH_3B_ADDR_LEN		3
#define SPI_FLASH_CMD_LEN		(1 + SPI_FLASH_3B_ADDR_LEN)
#define SPI_FLASH_16MB_BOUN		0x1000000

/* CFI Manufacture ID's */
#define SPI_FLASH_CFI_MFR_SPANSION	0x01
#define SPI_FLASH_CFI_MFR_STMICRO	0x20
#define SPI_FLASH_CFI_MFR_MACRONIX	0xc2
#define SPI_FLASH_CFI_MFR_SST		0xbf
#define SPI_FLASH_CFI_MFR_WINBOND	0xef
#define SPI_FLASH_CFI_MFR_ATMEL		0x1f

/* Erase commands */
#define CMD_ERASE_4K			0x20
#define CMD_ERASE_32K			0x52
#define CMD_ERASE_CHIP			0xc7
#define CMD_ERASE_64K			0xd8

/* Write commands */
#define CMD_WRITE_STATUS		0x01
#define CMD_PAGE_PROGRAM		0x02
#define CMD_WRITE_DISABLE		0x04
#define CMD_WRITE_ENABLE		0x06
#define CMD_QUAD_PAGE_PROGRAM		0x32
#define CMD_WRITE_EVCR			0x61

/* Read commands */
#define CMD_READ_ARRAY_SLOW		0x03
#define CMD_READ_ARRAY_FAST		0x0b
#define CMD_READ_DUAL_OUTPUT_FAST	0x3b
#define CMD_READ_DUAL_IO_FAST		0xbb
#define CMD_READ_QUAD_OUTPUT_FAST	0x6b
#define CMD_READ_QUAD_IO_FAST		0xeb
#define CMD_READ_ID			0x9f
#define CMD_READ_STATUS			0x05
#define CMD_READ_STATUS1		0x35
#define CMD_READ_CONFIG			0x35
#define CMD_FLAG_STATUS			0x70
#define CMD_READ_EVCR			0x65

/* Bank addr access commands */
#ifdef CONFIG_SPI_FLASH_BAR
# define CMD_BANKADDR_BRWR		0x17
# define CMD_BANKADDR_BRRD		0x16
# define CMD_EXTNADDR_WREAR		0xC5
# define CMD_EXTNADDR_RDEAR		0xC8
#endif

/* Common status */
#define STATUS_WIP			BIT(0)
#define STATUS_QEB_WINSPAN		BIT(1)
#define STATUS_QEB_MXIC			BIT(6)
#define STATUS_PEC			BIT(7)
#define STATUS_QEB_MICRON		BIT(7)
#define SR_BP0				BIT(2)  /* Block protect 0 */
#define SR_BP1				BIT(3)  /* Block protect 1 */
#define SR_BP2				BIT(4)  /* Block protect 2 */

/* Flash timeout values */
#define SPI_FLASH_PROG_TIMEOUT		(2 * CONFIG_SYS_HZ)
#define SPI_FLASH_PAGE_ERASE_TIMEOUT	(5 * CONFIG_SYS_HZ)
#define SPI_FLASH_SECTOR_ERASE_TIMEOUT	(10 * CONFIG_SYS_HZ)

/* SST specific */
#ifdef CONFIG_SPI_FLASH_SST
# define CMD_SST_BP		0x02    /* Byte Program */
# define CMD_SST_AAI_WP		0xAD	/* Auto Address Incr Word Program */

int sst_write_wp(struct spi_flash *flash, u32 offset, size_t len,
		const void *buf);
int sst_write_bp(struct spi_flash *flash, u32 offset, size_t len,
		const void *buf);
#endif

#ifdef CONFIG_SPI_FLASH_MACRONIX
#ifdef CONFIG_SPI_FLASH_BLOCK_LOCK

#define BLOCK_NONE                  0x00/* protect none */
#define BLOCK_AREA                  0x04/* protect block 0-7, configuration reg's TB=1 */
#define BLOCK_SHIFH                 2
#define CONFIGURATION_REG_DEFAULT   0x07/* output driver 30 Ohms */
#define CON_REG_TB_SHIFT            3
#define CON_REG_TB_BOTTOM           1
#define CON_REG_TB_TOP              0
#define CONFIGURATION_REG           (CONFIGURATION_REG_DEFAULT | (CON_REG_TB_BOTTOM << CON_REG_TB_SHIFT))

#define MACRONIX_STATUS_WIP         BIT(0)
#define MACRONIX_STATUS_WEL         BIT(1) /* write enable */

#define BLOCK_LOCK                  1
#define BLOCK_UNLOCK                0

#endif  /* #ifdef CONFIG_SPI_FLASH_BLOCK_LOCK */
#endif  /* #ifdef CONFIG_SPI_FLASH_MACRONIX */

/**
 * struct spi_flash_params - SPI/QSPI flash device params structure
 *
 * @name:		Device name ([MANUFLETTER][DEVTYPE][DENSITY][EXTRAINFO])
 * @jedec:		Device jedec ID (0x[1byte_manuf_id][2byte_dev_id])
 * @ext_jedec:		Device ext_jedec ID
 * @sector_size:	Isn't necessarily a sector size from vendor,
 *			the size listed here is what works with CMD_ERASE_64K
 * @nr_sectors:		No.of sectors on this device
 * @e_rd_cmd:		Enum list for read commands
 * @flags:		Important param, for flash specific behaviour
 */
struct spi_flash_params {
	const char *name;
	u32 jedec;
	u16 ext_jedec;
	u32 sector_size;
	u32 nr_sectors;
	u8 e_rd_cmd;
	u16 flags;
};

extern const struct spi_flash_params spi_flash_params_table[];

/* Send a single-byte command to the device and read the response */
int spi_flash_cmd(struct spi_slave *spi, u8 cmd, void *response, size_t len);

/*
 * Send a multi-byte command to the device and read the response. Used
 * for flash array reads, etc.
 */
int spi_flash_cmd_read(struct spi_slave *spi, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len);

/*
 * Send a multi-byte command to the device followed by (optional)
 * data. Used for programming the flash array, etc.
 */
int spi_flash_cmd_write(struct spi_slave *spi, const u8 *cmd, size_t cmd_len,
		const void *data, size_t data_len);


/* Flash erase(sectors) operation, support all possible erase commands */
int spi_flash_cmd_erase_ops(struct spi_flash *flash, u32 offset, size_t len);

/* Lock stmicro spi flash region */
int stm_lock(struct spi_flash *flash, u32 ofs, size_t len);

/* Unlock stmicro spi flash region */
int stm_unlock(struct spi_flash *flash, u32 ofs, size_t len);

/* Check if a stmicro spi flash region is completely locked */
int stm_is_locked(struct spi_flash *flash, u32 ofs, size_t len);

/* Enable writing on the SPI flash */
static inline int spi_flash_cmd_write_enable(struct spi_flash *flash)
{
	return spi_flash_cmd(flash->spi, CMD_WRITE_ENABLE, NULL, 0);
}

/* Disable writing on the SPI flash */
static inline int spi_flash_cmd_write_disable(struct spi_flash *flash)
{
	return spi_flash_cmd(flash->spi, CMD_WRITE_DISABLE, NULL, 0);
}

/*
 * Used for spi_flash write operation
 * - SPI claim
 * - spi_flash_cmd_write_enable
 * - spi_flash_cmd_write
 * - spi_flash_cmd_wait_ready
 * - SPI release
 */
int spi_flash_write_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, const void *buf, size_t buf_len);

/*
 * Flash write operation, support all possible write commands.
 * Write the requested data out breaking it up into multiple write
 * commands as needed per the write size.
 */
int spi_flash_cmd_write_ops(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf);

/*
 * Same as spi_flash_cmd_read() except it also claims/releases the SPI
 * bus. Used as common part of the ->read() operation.
 */
int spi_flash_read_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len);

/* Flash read operation, support all possible read commands */
int spi_flash_cmd_read_ops(struct spi_flash *flash, u32 offset,
		size_t len, void *data);

#ifdef CONFIG_SPI_FLASH_MTD
int spi_flash_mtd_register(struct spi_flash *flash);
void spi_flash_mtd_unregister(void);
#endif

/**
 * spi_flash_scan - scan the SPI FLASH
 * @flash:	the spi flash structure
 *
 * The drivers can use this fuction to scan the SPI FLASH.
 * In the scanning, it will try to get all the necessary information to
 * fill the spi_flash{}.
 *
 * Return: 0 for success, others for failure.
 */
int spi_flash_scan(struct spi_flash *flash);

#endif /* _SF_INTERNAL_H_ */
