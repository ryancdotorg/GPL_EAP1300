/*
 * Copyright (C) 2016 Jagan Teki <jteki@openedev.com>
 *		      Christophe Ricard <christophe.ricard@gmail.com>
 *
 * Copyright (C) 2010 Dirk Behme <dirk.behme@googlemail.com>
 *
 * Driver for McSPI controller on OMAP3. Based on davinci_spi.c
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 *
 * Copyright (C) 2007 Atmel Corporation
 *
 * Parts taken from linux/drivers/spi/omap2_mcspi.c
 * Copyright (C) 2005, 2006 Nokia Corporation
 *
 * Modified by Ruslan Araslanov <ruslan.araslanov@vitecmm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <spi.h>
#include <malloc.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_AM33XX) || defined(CONFIG_AM43XX)
#define OMAP3_MCSPI1_BASE	0x48030100
#define OMAP3_MCSPI2_BASE	0x481A0100
#else
#define OMAP3_MCSPI1_BASE	0x48098000
#define OMAP3_MCSPI2_BASE	0x4809A000
#define OMAP3_MCSPI3_BASE	0x480B8000
#define OMAP3_MCSPI4_BASE	0x480BA000
#endif

/* per-register bitmasks */
#define OMAP3_MCSPI_SYSCONFIG_SMARTIDLE (2 << 3)
#define OMAP3_MCSPI_SYSCONFIG_ENAWAKEUP BIT(2)
#define OMAP3_MCSPI_SYSCONFIG_AUTOIDLE	BIT(0)
#define OMAP3_MCSPI_SYSCONFIG_SOFTRESET BIT(1)

#define OMAP3_MCSPI_SYSSTATUS_RESETDONE BIT(0)

#define OMAP3_MCSPI_MODULCTRL_SINGLE	BIT(0)
#define OMAP3_MCSPI_MODULCTRL_MS	BIT(2)
#define OMAP3_MCSPI_MODULCTRL_STEST	BIT(3)

#define OMAP3_MCSPI_CHCONF_PHA		BIT(0)
#define OMAP3_MCSPI_CHCONF_POL		BIT(1)
#define OMAP3_MCSPI_CHCONF_CLKD_MASK	GENMASK(5, 2)
#define OMAP3_MCSPI_CHCONF_EPOL		BIT(6)
#define OMAP3_MCSPI_CHCONF_WL_MASK	GENMASK(11, 7)
#define OMAP3_MCSPI_CHCONF_TRM_RX_ONLY	BIT(12)
#define OMAP3_MCSPI_CHCONF_TRM_TX_ONLY	BIT(13)
#define OMAP3_MCSPI_CHCONF_TRM_MASK	GENMASK(13, 12)
#define OMAP3_MCSPI_CHCONF_DMAW		BIT(14)
#define OMAP3_MCSPI_CHCONF_DMAR		BIT(15)
#define OMAP3_MCSPI_CHCONF_DPE0		BIT(16)
#define OMAP3_MCSPI_CHCONF_DPE1		BIT(17)
#define OMAP3_MCSPI_CHCONF_IS		BIT(18)
#define OMAP3_MCSPI_CHCONF_TURBO	BIT(19)
#define OMAP3_MCSPI_CHCONF_FORCE	BIT(20)
#define OMAP3_MCSPI_CHCONF_FFEW		BIT(27)
#define OMAP3_MCSPI_CHCONF_FFER		BIT(28)

#define OMAP3_MCSPI_CHSTAT_RXS		BIT(0)
#define OMAP3_MCSPI_CHSTAT_TXS		BIT(1)
#define OMAP3_MCSPI_CHSTAT_EOT		BIT(2)

#define OMAP3_MCSPI_CHCTRL_EN		BIT(0)
#define OMAP3_MCSPI_CHCTRL_DIS		(0 << 0)

#define OMAP3_MCSPI_WAKEUPENABLE_WKEN	BIT(0)
#define MCSPI_PINDIR_D0_IN_D1_OUT	0
#define MCSPI_PINDIR_D0_OUT_D1_IN	1

#define OMAP3_MCSPI_MAX_FREQ		48000000
#define SPI_WAIT_TIMEOUT		10

/* idleman */
#define HWREG(x)			(*((volatile unsigned int *)(x)))

#define MCSPI0_CHSTAT			(OMAP3_MCSPI1_BASE + 0x30)
#define MCSPI0_RX_DATA			(OMAP3_MCSPI1_BASE + 0x3C)

#ifdef CONFIG_MCSPI0_FIFO
#define MCSPI0_FIFO /* SPI bus to read SPI FLASH */
#endif  /* #ifdef CONFIG_MCSPI0_FIFO */

#ifdef MCSPI0_FIFO
#define OMAP3_MCSPI_CHSTAT_RXFFF        (1 << 6)/* 0/1 = FIFO receive buffer is not full/full */
#define OMAP3_MCSPI_CHSTAT_RXFFE        (1 << 5)/* 0/1 = FIFO receive buffer is not empty/empty */
#define OMAP3_MCSPI_CHSTAT_TXFFF        (1 << 4)/* 0/1 = FIFO transmit buffer is not full/full */
#define OMAP3_MCSPI_CHSTAT_TXFFE        (1 << 3)/* 0/1 = FIFO transmit buffer is not empty/empty */

#define MCSPI_XFERLEVEL                 (OMAP3_MCSPI1_BASE + 0x7C)
#define MCSPI_XFERLEVEL_AFL             (0x3C << 8)/* Buffer almost full */
#define MCSPI_XFERLEVEL_AEL             (0x3C << 0)/* Buffer almost empty */
#define MCSPI_XFERLEVEL_AFL_SHIFT       8
#define MCSPI_XFERLEVEL_AEL_SHIFT       0
#define MCSPI_AFL_BYTE                  48
#define MCSPI_AEL_BYTE                  16

#define MCSPI0_RX_FULL                  (1 << 2)
#define MCSPI0_TX_EMPTY                 (1 << 0)

#define MCSPI_FIFO_SIZE_THD             (128*1024*5)/* means ignore u-boot and MLO, they used old method */
#define IMAGE_HEADER_SIZE               (64)/* bytes */

#define MCSPI0_FIFO_WORD_LEN            32/* 8 for bytes, 16 for words */

#define MCSPI0_DEF_WORD_LEN             8

static int getheaderinfo = 0;		/* record the first read command for kernel */
static unsigned int imagesize = 0;	/* record the kernel size */

#endif  /* #ifdef MCSPI0_FIFO */

/* OMAP3 McSPI registers */
struct mcspi_channel {
	unsigned int chconf;		/* 0x2C, 0x40, 0x54, 0x68 */
	unsigned int chstat;		/* 0x30, 0x44, 0x58, 0x6C */
	unsigned int chctrl;		/* 0x34, 0x48, 0x5C, 0x70 */
	unsigned int tx;		/* 0x38, 0x4C, 0x60, 0x74 */
	unsigned int rx;		/* 0x3C, 0x50, 0x64, 0x78 */
};

struct mcspi {
	unsigned char res1[0x10];
	unsigned int sysconfig;		/* 0x10 */
	unsigned int sysstatus;		/* 0x14 */
	unsigned int irqstatus;		/* 0x18 */
	unsigned int irqenable;		/* 0x1C */
	unsigned int wakeupenable;	/* 0x20 */
	unsigned int syst;		/* 0x24 */
	unsigned int modulctrl;		/* 0x28 */
	struct mcspi_channel channel[4];
	/* channel0: 0x2C - 0x3C, bus 0 & 1 & 2 & 3 */
	/* channel1: 0x40 - 0x50, bus 0 & 1 */
	/* channel2: 0x54 - 0x64, bus 0 & 1 */
	/* channel3: 0x68 - 0x78, bus 0 */
};

struct omap3_spi_priv {
#ifndef CONFIG_DM_SPI
	struct spi_slave slave;
#endif
	struct mcspi *regs;
	unsigned int cs;
	unsigned int freq;
	unsigned int mode;
	unsigned int wordlen;
	unsigned int pin_dir:1;
};

static void omap3_spi_write_chconf(struct omap3_spi_priv *priv, int val)
{
	writel(val, &priv->regs->channel[priv->cs].chconf);
	/* Flash post writes to make immediate effect */
	readl(&priv->regs->channel[priv->cs].chconf);
}

static void omap3_spi_set_enable(struct omap3_spi_priv *priv, int enable)
{
	writel(enable, &priv->regs->channel[priv->cs].chctrl);
	/* Flash post writes to make immediate effect */
	readl(&priv->regs->channel[priv->cs].chctrl);
}

static int omap3_spi_write(struct omap3_spi_priv *priv, unsigned int len,
			   const void *txp, unsigned long flags)
{
	ulong start;
	int i, chconf;

	chconf = readl(&priv->regs->channel[priv->cs].chconf);

	/* Enable the channel */
	omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_EN);

	chconf &= ~(OMAP3_MCSPI_CHCONF_TRM_MASK | OMAP3_MCSPI_CHCONF_WL_MASK);
	chconf |= (priv->wordlen - 1) << 7;
	chconf |= OMAP3_MCSPI_CHCONF_TRM_TX_ONLY;
	chconf |= OMAP3_MCSPI_CHCONF_FORCE;
	omap3_spi_write_chconf(priv, chconf);

	for (i = 0; i < len; i++) {
		/* wait till TX register is empty (TXS == 1) */
		start = get_timer(0);
		while (!(readl(&priv->regs->channel[priv->cs].chstat) &
			 OMAP3_MCSPI_CHSTAT_TXS)) {
			if (get_timer(start) > SPI_WAIT_TIMEOUT) {
				printf("SPI TXS timed out, status=0x%08x\n",
					readl(&priv->regs->channel[priv->cs].chstat));
				return -1;
			}
		}
		/* Write the data */
		unsigned int *tx = &priv->regs->channel[priv->cs].tx;
		if (priv->wordlen > 16)
			writel(((u32 *)txp)[i], tx);
		else if (priv->wordlen > 8)
			writel(((u16 *)txp)[i], tx);
		else
			writel(((u8 *)txp)[i], tx);
	}

	/* wait to finish of transfer */
	while ((readl(&priv->regs->channel[priv->cs].chstat) &
			(OMAP3_MCSPI_CHSTAT_EOT | OMAP3_MCSPI_CHSTAT_TXS)) !=
			(OMAP3_MCSPI_CHSTAT_EOT | OMAP3_MCSPI_CHSTAT_TXS))
		;

	/* Disable the channel otherwise the next immediate RX will get affected */
	omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_DIS);

	if (flags & SPI_XFER_END) {

		chconf &= ~OMAP3_MCSPI_CHCONF_FORCE;
		omap3_spi_write_chconf(priv, chconf);
	}
	return 0;
}

static int omap3_spi_read(struct omap3_spi_priv *priv, unsigned int len,
			  void *rxp, unsigned long flags)
{
	int i = 0, chconf;
#ifdef MCSPI0_FIFO /* idleman */

	int xferlevel = HWREG(MCSPI_XFERLEVEL);
	int chstatus = 0;
	unsigned int readbytecnt = 0;

#if MCSPI0_FIFO_WORD_LEN == 8
	u8 *ByteDestAddr;
	ByteDestAddr = rxp;
#elif MCSPI0_FIFO_WORD_LEN == 16
	u16 *WordDestAddr;
#else   /* #if MCSPI0_FIFO_WORD_LEN == 8 */
	u32 *LongDestAddr;
#endif  /* #if MCSPI0_FIFO_WORD_LEN == 8 */

#else   /* #ifdef MCSPI0_FIFO */
	//ulong start;
#endif  /* #ifdef MCSPI0_FIFO */

	chconf = readl(&priv->regs->channel[priv->cs].chconf);

	/* Enable the channel */
	omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_EN);

	chconf &= ~(OMAP3_MCSPI_CHCONF_TRM_MASK | OMAP3_MCSPI_CHCONF_WL_MASK);
	chconf |= (priv->wordlen - 1) << 7;
	chconf |= OMAP3_MCSPI_CHCONF_TRM_RX_ONLY;
	chconf |= OMAP3_MCSPI_CHCONF_FORCE;

#ifdef MCSPI0_FIFO /* idleman */
	if(len > MCSPI_FIFO_SIZE_THD)
	{
		//printf("(%d)rxp = 0x%8X, WordDestAddr = 0x%8X\n", len, rxp, WordDestAddr);
		xferlevel &= ~MCSPI_XFERLEVEL_AFL;
		xferlevel |= (MCSPI_AFL_BYTE << MCSPI_XFERLEVEL_AFL_SHIFT);
		xferlevel |= (MCSPI_AEL_BYTE << MCSPI_XFERLEVEL_AEL_SHIFT);
		HWREG(MCSPI_XFERLEVEL) = xferlevel;
		/* enable FIFO buffer is used for receive data */
		chconf |= OMAP3_MCSPI_CHCONF_FFER;
		/* wordlength */
		chconf &= ~OMAP3_MCSPI_CHCONF_WL_MASK;
		chconf |= (MCSPI0_FIFO_WORD_LEN - 1) << 7;
	}
#endif  /* #ifdef MCSPI0_FIFO */

	omap3_spi_write_chconf(priv, chconf);

	writel(0, &priv->regs->channel[priv->cs].tx);

#ifdef MCSPI0_FIFO /* idleman */
	if(len > MCSPI_FIFO_SIZE_THD)
	{
		/* read kernel only */
		if(getheaderinfo == 0)
		{
			imagesize = len;
		}
#if MCSPI0_FIFO_WORD_LEN == 16
		WordDestAddr = (rxp);
#elif MCSPI0_FIFO_WORD_LEN == 32
		LongDestAddr = (rxp);
#endif  /* #if MCSPI0_FIFO_WORD_LEN == 16 */

		while(i < imagesize)
		{
			chstatus = HWREG(MCSPI0_CHSTAT);
			if(chstatus & OMAP3_MCSPI_CHSTAT_RXFFF)
			{
				readbytecnt = MCSPI_AFL_BYTE;
				i += readbytecnt;
				if(i > imagesize)
				{
					readbytecnt -= (i - imagesize);
				}
				while(readbytecnt > 0)
				{
#if MCSPI0_FIFO_WORD_LEN == 8
					*ByteDestAddr++ = (unsigned char)HWREG(MCSPI0_RX_DATA);
					readbytecnt--;
#elif MCSPI0_FIFO_WORD_LEN == 16
					*WordDestAddr++ = be16_to_cpu((u16)HWREG(MCSPI0_RX_DATA));
					readbytecnt -= 2;
#else   /* #if MCSPI0_FIFO_WORD_LEN == 8 */
					*LongDestAddr++ = be32_to_cpu((u32)HWREG(MCSPI0_RX_DATA));
					readbytecnt -= 4;
#endif  /* #if MCSPI0_FIFO_WORD_LEN == 8 */
				}
#ifdef CONFIG_UBOOT_UIMAGE_WITH_HEADER
				if(getheaderinfo == 0)
				{
#if MCSPI0_FIFO_WORD_LEN == 8
					imagesize = ((u32)(((u8 *)rxp)[12]) << 24) |
						((u32)(((u8 *)rxp)[13]) << 16) |
						((u32)(((u8 *)rxp)[14]) << 8) |
						((u32)(((u8 *)rxp)[15]) << 0);
#elif MCSPI0_FIFO_WORD_LEN == 16
					imagesize = ((u32)(cpu_to_be16(((u16 *)rxp)[6])) << 16) |
						((u32)(cpu_to_be16(((u16 *)rxp)[7])) << 0);
#else   /* #if MCSPI0_FIFO_WORD_LEN == 8 */
					imagesize = ((u32)(cpu_to_be32(((u32 *)rxp)[3])) << 0);
#endif  /* #if MCSPI0_FIFO_WORD_LEN == 8 */
					printf("Get KERNEL real size (%d bytes) from image header!\n", imagesize);
					imagesize += IMAGE_HEADER_SIZE;
					getheaderinfo = 1;
				}
#endif  /* #ifdef CONFIG_UBOOT_UIMAGE_WITH_HEADER */
			}
		}

		omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_DIS);
		chconf &= ~OMAP3_MCSPI_CHCONF_FFER;
#if (MCSPI0_FIFO_WORD_LEN == 16) || (MCSPI0_FIFO_WORD_LEN == 32)
		/* change it back to default 8 bits format */
		chconf &= ~OMAP3_MCSPI_CHCONF_WL_MASK;
		chconf |= (MCSPI0_FIFO_WORD_LEN - 1) << 7;
#endif  /* #if MCSPI0_FIFO_WORD_LEN == 16 */
		omap3_spi_write_chconf(priv,chconf);
	}
	else
#endif  /* #ifdef MCSPI0_FIFO */	
	{
	for (i = 0; i < len; i++) 
	{
#if 1 /* idleman, speed up the original code when reading */
		while (!(HWREG(MCSPI0_CHSTAT) & OMAP3_MCSPI_CHSTAT_RXS))
		{
			/* empty */
		}

		/* Disable the channel to prevent furher receiving */
		if(i == (len - 1))
			omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_DIS);

		/* Read the data */
		if (priv->wordlen > 16)
			((u32 *)rxp)[i] = HWREG(MCSPI0_RX_DATA);
		else if (priv->wordlen > 8)
			((u16 *)rxp)[i] = (u16)HWREG(MCSPI0_RX_DATA);
		else
			((u8 *)rxp)[i] = (u8)HWREG(MCSPI0_RX_DATA);

#else   /* original code */ 
		start = get_timer(0);
		/* Wait till RX register contains data (RXS == 1) */
		while (!(readl(&priv->regs->channel[priv->cs].chstat) &
			 OMAP3_MCSPI_CHSTAT_RXS)) {
			if (get_timer(start) > SPI_WAIT_TIMEOUT) {
				printf("SPI RXS timed out, status=0x%08x\n",
					readl(&priv->regs->channel[priv->cs].chstat));
				return -1;
			}
		}

		/* Disable the channel to prevent furher receiving */
		if (i == (len - 1))
			omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_DIS);

		/* Read the data */
		unsigned int *rx = &priv->regs->channel[priv->cs].rx;
		if (priv->wordlen > 16)
			((u32 *)rxp)[i] = readl(rx);
		else if (priv->wordlen > 8)
			((u16 *)rxp)[i] = (u16)readl(rx);
		else
			((u8 *)rxp)[i] = (u8)readl(rx);
#endif
	} /* for (i = 0; i < len; i++) */
	} /* else */

	if (flags & SPI_XFER_END) {
		chconf &= ~OMAP3_MCSPI_CHCONF_FORCE;
		omap3_spi_write_chconf(priv, chconf);
	}

	return 0;
}

/*McSPI Transmit Receive Mode*/
static int omap3_spi_txrx(struct omap3_spi_priv *priv, unsigned int len,
			  const void *txp, void *rxp, unsigned long flags)
{
	ulong start;
	int chconf, i = 0;
	printf("SPI TXRX length = %d\n", len);
	chconf = readl(&priv->regs->channel[priv->cs].chconf);

	/*Enable SPI channel*/
	omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_EN);

	/*set TRANSMIT-RECEIVE Mode*/
	chconf &= ~(OMAP3_MCSPI_CHCONF_TRM_MASK | OMAP3_MCSPI_CHCONF_WL_MASK);
	chconf |= (priv->wordlen - 1) << 7;
	chconf |= OMAP3_MCSPI_CHCONF_FORCE;
	omap3_spi_write_chconf(priv, chconf);

	/*Shift in and out 1 byte at time*/
	for (i=0; i < len; i++){
		/* Write: wait for TX empty (TXS == 1)*/
		start = get_timer(0);
		while (!(readl(&priv->regs->channel[priv->cs].chstat) &
			 OMAP3_MCSPI_CHSTAT_TXS)) {
			if (get_timer(start) > SPI_WAIT_TIMEOUT) {
				printf("SPI TXS timed out, status=0x%08x\n",
					readl(&priv->regs->channel[priv->cs].chstat));
				return -1;
			}
		}
		/* Write the data */
		unsigned int *tx = &priv->regs->channel[priv->cs].tx;
		if (priv->wordlen > 16)
			writel(((u32 *)txp)[i], tx);
		else if (priv->wordlen > 8)
			writel(((u16 *)txp)[i], tx);
		else
			writel(((u8 *)txp)[i], tx);

		/*Read: wait for RX containing data (RXS == 1)*/
		start = get_timer(0);
		while (!(readl(&priv->regs->channel[priv->cs].chstat) &
			 OMAP3_MCSPI_CHSTAT_RXS)) {
			if (get_timer(start) > SPI_WAIT_TIMEOUT) {
				printf("SPI RXS timed out, status=0x%08x\n",
					readl(&priv->regs->channel[priv->cs].chstat));
				return -1;
			}
		}
		/* Read the data */
		unsigned int *rx = &priv->regs->channel[priv->cs].rx;
		if (priv->wordlen > 16)
			((u32 *)rxp)[i] = readl(rx);
		else if (priv->wordlen > 8)
			((u16 *)rxp)[i] = (u16)readl(rx);
		else
			((u8 *)rxp)[i] = (u8)readl(rx);
	}
	/* Disable the channel */
	omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_DIS);

	/*if transfer must be terminated disable the channel*/
	if (flags & SPI_XFER_END) {
		chconf &= ~OMAP3_MCSPI_CHCONF_FORCE;
		omap3_spi_write_chconf(priv, chconf);
	}

	return 0;
}

static int _spi_xfer(struct omap3_spi_priv *priv, unsigned int bitlen,
		     const void *dout, void *din, unsigned long flags)
{
	unsigned int	len;
	int ret = -1;

	if (priv->wordlen < 4 || priv->wordlen > 32) {
		printf("omap3_spi: invalid wordlen %d\n", priv->wordlen);
		return -1;
	}

	if (bitlen % priv->wordlen)
		return -1;

	len = bitlen / priv->wordlen;

	if (bitlen == 0) {	 /* only change CS */
		int chconf = readl(&priv->regs->channel[priv->cs].chconf);

		if (flags & SPI_XFER_BEGIN) {
			omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_EN);
			chconf |= OMAP3_MCSPI_CHCONF_FORCE;
			omap3_spi_write_chconf(priv, chconf);
		}
		if (flags & SPI_XFER_END) {
			chconf &= ~OMAP3_MCSPI_CHCONF_FORCE;
			omap3_spi_write_chconf(priv, chconf);
			omap3_spi_set_enable(priv, OMAP3_MCSPI_CHCTRL_DIS);
		}
		ret = 0;
	} else {
		if (dout != NULL && din != NULL)
			ret = omap3_spi_txrx(priv, len, dout, din, flags);
		else if (dout != NULL)
			ret = omap3_spi_write(priv, len, dout, flags);
		else if (din != NULL)
			ret = omap3_spi_read(priv, len, din, flags);
	}
	return ret;
}

static void _omap3_spi_set_speed(struct omap3_spi_priv *priv)
{
	uint32_t confr, div = 0;

	confr = readl(&priv->regs->channel[priv->cs].chconf);

	/* Calculate clock divisor. Valid range: 0x0 - 0xC ( /1 - /4096 ) */
	if (priv->freq) {
		while (div <= 0xC && (OMAP3_MCSPI_MAX_FREQ / (1 << div))
					> priv->freq)
			div++;
	} else {
		 div = 0xC;
	}

	/* set clock divisor */
	confr &= ~OMAP3_MCSPI_CHCONF_CLKD_MASK;
	confr |= div << 2;

	omap3_spi_write_chconf(priv, confr);
}

static void _omap3_spi_set_mode(struct omap3_spi_priv *priv)
{
	uint32_t confr;

	confr = readl(&priv->regs->channel[priv->cs].chconf);

	/* standard 4-wire master mode:  SCK, MOSI/out, MISO/in, nCS
	 * REVISIT: this controller could support SPI_3WIRE mode.
	 */
	if (priv->pin_dir == MCSPI_PINDIR_D0_IN_D1_OUT) {
		confr &= ~(OMAP3_MCSPI_CHCONF_IS|OMAP3_MCSPI_CHCONF_DPE1);
		confr |= OMAP3_MCSPI_CHCONF_DPE0;
	} else {
		confr &= ~OMAP3_MCSPI_CHCONF_DPE0;
		confr |= OMAP3_MCSPI_CHCONF_IS|OMAP3_MCSPI_CHCONF_DPE1;
	}

	/* set SPI mode 0..3 */
	confr &= ~(OMAP3_MCSPI_CHCONF_POL | OMAP3_MCSPI_CHCONF_PHA);
	if (priv->mode & SPI_CPHA)
		confr |= OMAP3_MCSPI_CHCONF_PHA;
	if (priv->mode & SPI_CPOL)
		confr |= OMAP3_MCSPI_CHCONF_POL;

	/* set chipselect polarity; manage with FORCE */
	if (!(priv->mode & SPI_CS_HIGH))
		confr |= OMAP3_MCSPI_CHCONF_EPOL; /* active-low; normal */
	else
		confr &= ~OMAP3_MCSPI_CHCONF_EPOL;

	/* Transmit & receive mode */
	confr &= ~OMAP3_MCSPI_CHCONF_TRM_MASK;

	omap3_spi_write_chconf(priv, confr);
}

static void _omap3_spi_set_wordlen(struct omap3_spi_priv *priv)
{
	unsigned int confr;

	/* McSPI individual channel configuration */
	confr = readl(&priv->regs->channel[priv->wordlen].chconf);

	/* wordlength */
	confr &= ~OMAP3_MCSPI_CHCONF_WL_MASK;
	confr |= (priv->wordlen - 1) << 7;

	omap3_spi_write_chconf(priv, confr);
}

static void spi_reset(struct mcspi *regs)
{
	unsigned int tmp;

	writel(OMAP3_MCSPI_SYSCONFIG_SOFTRESET, &regs->sysconfig);
	do {
		tmp = readl(&regs->sysstatus);
	} while (!(tmp & OMAP3_MCSPI_SYSSTATUS_RESETDONE));

	writel(OMAP3_MCSPI_SYSCONFIG_AUTOIDLE |
	       OMAP3_MCSPI_SYSCONFIG_ENAWAKEUP |
	       OMAP3_MCSPI_SYSCONFIG_SMARTIDLE, &regs->sysconfig);

	writel(OMAP3_MCSPI_WAKEUPENABLE_WKEN, &regs->wakeupenable);
}

static void _omap3_spi_claim_bus(struct omap3_spi_priv *priv)
{
	unsigned int conf;

	spi_reset(priv->regs);

	/*
	 * setup when switching from (reset default) slave mode
	 * to single-channel master mode
	 */
	conf = readl(&priv->regs->modulctrl);
	conf &= ~(OMAP3_MCSPI_MODULCTRL_STEST | OMAP3_MCSPI_MODULCTRL_MS);
	conf |= OMAP3_MCSPI_MODULCTRL_SINGLE;

	writel(conf, &priv->regs->modulctrl);

	_omap3_spi_set_mode(priv);
	_omap3_spi_set_speed(priv);
}

#ifndef CONFIG_DM_SPI

static inline struct omap3_spi_priv *to_omap3_spi(struct spi_slave *slave)
{
	return container_of(slave, struct omap3_spi_priv, slave);
}

void spi_init(void)
{
	/* do nothing */
}

void spi_free_slave(struct spi_slave *slave)
{
	struct omap3_spi_priv *priv = to_omap3_spi(slave);

	free(priv);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct omap3_spi_priv *priv = to_omap3_spi(slave);

	_omap3_spi_claim_bus(priv);
	_omap3_spi_set_wordlen(priv);
	_omap3_spi_set_mode(priv);
	_omap3_spi_set_speed(priv);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct omap3_spi_priv *priv = to_omap3_spi(slave);

	/* Reset the SPI hardware */
	spi_reset(priv->regs);
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				     unsigned int max_hz, unsigned int mode)
{
	struct omap3_spi_priv *priv;
	struct mcspi *regs;

	/*
	 * OMAP3 McSPI (MultiChannel SPI) has 4 busses (modules)
	 * with different number of chip selects (CS, channels):
	 * McSPI1 has 4 CS (bus 0, cs 0 - 3)
	 * McSPI2 has 2 CS (bus 1, cs 0 - 1)
	 * McSPI3 has 2 CS (bus 2, cs 0 - 1)
	 * McSPI4 has 1 CS (bus 3, cs 0)
	 */

	switch (bus) {
	case 0:
		 regs = (struct mcspi *)OMAP3_MCSPI1_BASE;
		 break;
#ifdef OMAP3_MCSPI2_BASE
	case 1:
		 regs = (struct mcspi *)OMAP3_MCSPI2_BASE;
		 break;
#endif
#ifdef OMAP3_MCSPI3_BASE
	case 2:
		 regs = (struct mcspi *)OMAP3_MCSPI3_BASE;
		 break;
#endif
#ifdef OMAP3_MCSPI4_BASE
	case 3:
		 regs = (struct mcspi *)OMAP3_MCSPI4_BASE;
		 break;
#endif
	default:
		 printf("SPI error: unsupported bus %i.  Supported busses 0 - 3\n", bus);
		 return NULL;
	}

	if (((bus == 0) && (cs > 3)) ||
	    ((bus == 1) && (cs > 1)) ||
	    ((bus == 2) && (cs > 1)) ||
	    ((bus == 3) && (cs > 0))) {
		printf("SPI error: unsupported chip select %i on bus %i\n", cs, bus);
		return NULL;
	}

	if (max_hz > OMAP3_MCSPI_MAX_FREQ) {
		printf("SPI error: unsupported frequency %i Hz. Max frequency is 48 Mhz\n", max_hz);
		return NULL;
	}

	if (mode > SPI_MODE_3) {
		printf("SPI error: unsupported SPI mode %i\n", mode);
		return NULL;
	}

	priv = spi_alloc_slave(struct omap3_spi_priv, bus, cs);
	if (!priv) {
		printf("SPI error: malloc of SPI structure failed\n");
		return NULL;
	}

	priv->regs = regs;
	priv->cs = cs;
	priv->freq = max_hz;
	priv->mode = mode;
	priv->wordlen = priv->slave.wordlen;
#ifdef CONFIG_OMAP3_SPI_D0_D1_SWAPPED
	priv->pin_dir = MCSPI_PINDIR_D0_OUT_D1_IN;
#endif

	return &priv->slave;
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
	     const void *dout, void *din, unsigned long flags)
{
	struct omap3_spi_priv *priv = to_omap3_spi(slave);

	return _spi_xfer(priv, bitlen, dout, din, flags);
}

#else

static int omap3_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct omap3_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);

	priv->cs = slave_plat->cs;
	priv->mode = slave_plat->mode;
	priv->freq = slave_plat->max_hz;
	_omap3_spi_claim_bus(priv);

	return 0;
}

static int omap3_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct omap3_spi_priv *priv = dev_get_priv(bus);

	/* Reset the SPI hardware */
	spi_reset(priv->regs);

	return 0;
}

static int omap3_spi_set_wordlen(struct udevice *dev, unsigned int wordlen)
{
	struct udevice *bus = dev->parent;
	struct omap3_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);

	priv->cs = slave_plat->cs;
	priv->wordlen = wordlen;
	_omap3_spi_set_wordlen(priv);

	return 0;
}

static int omap3_spi_probe(struct udevice *dev)
{
	struct omap3_spi_priv *priv = dev_get_priv(dev);
	const void *blob = gd->fdt_blob;
	int node = dev->of_offset;

	priv->regs = (struct mcspi *)dev_get_addr(dev);
	priv->pin_dir = fdtdec_get_uint(blob, node, "ti,pindir-d0-out-d1-in",
					    MCSPI_PINDIR_D0_IN_D1_OUT);
	priv->wordlen = SPI_DEFAULT_WORDLEN;
	return 0;
}

static int omap3_spi_xfer(struct udevice *dev, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct omap3_spi_priv *priv = dev_get_priv(bus);

	return _spi_xfer(priv, bitlen, dout, din, flags);
}

static int omap3_spi_set_speed(struct udevice *bus, unsigned int speed)
{
	return 0;
}

static int omap3_spi_set_mode(struct udevice *bus, uint mode)
{
	return 0;
}

static const struct dm_spi_ops omap3_spi_ops = {
	.claim_bus      = omap3_spi_claim_bus,
	.release_bus    = omap3_spi_release_bus,
	.set_wordlen    = omap3_spi_set_wordlen,
	.xfer	    = omap3_spi_xfer,
	.set_speed      = omap3_spi_set_speed,
	.set_mode	= omap3_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id omap3_spi_ids[] = {
	{ .compatible = "ti,omap2-mcspi" },
	{ .compatible = "ti,omap4-mcspi" },
	{ }
};

U_BOOT_DRIVER(omap3_spi) = {
	.name   = "omap3_spi",
	.id     = UCLASS_SPI,
	.of_match = omap3_spi_ids,
	.probe = omap3_spi_probe,
	.ops    = &omap3_spi_ops,
	.priv_auto_alloc_size = sizeof(struct omap3_spi_priv),
	.probe = omap3_spi_probe,
};
#endif

