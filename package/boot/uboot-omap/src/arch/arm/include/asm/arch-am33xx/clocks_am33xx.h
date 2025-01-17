/*
 * clocks_am33xx.h
 *
 * AM33xx clock define
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _CLOCKS_AM33XX_H_
#define _CLOCKS_AM33XX_H_

/* MAIN PLL Fdll supported frequencies */
#define MPUPLL_M_1000	1000
#define MPUPLL_M_800	800
#define MPUPLL_M_720	720
#define MPUPLL_M_600	600
#define MPUPLL_M_550	550
#define MPUPLL_M_300	300

#ifdef CONFIG_SYS_CUSTOMIZE_MPUCLK
#define CONFIG_SYS_MPUCLK      CONFIG_SYS_CUSTOMIZE_MPUCLK
#endif  /* #ifdef CONFIG_SYS_CUSTOMIZE_MPUCLK */

/* MAIN PLL Fdll = 550 MHz, by default */
#ifndef CONFIG_SYS_MPUCLK
#define CONFIG_SYS_MPUCLK	MPUPLL_M_550
#endif

#define UART_RESET		(0x1 << 1)
#define UART_CLK_RUNNING_MASK	0x1
#define UART_SMART_IDLE_EN	(0x1 << 0x3)

#define CM_DLL_CTRL_NO_OVERRIDE	0x0
#define CM_DLL_READYST		0x4

extern void enable_dmm_clocks(void);
extern const struct dpll_params dpll_core_opp100;
extern struct dpll_params dpll_mpu_opp100;

#endif	/* endif _CLOCKS_AM33XX_H_ */
