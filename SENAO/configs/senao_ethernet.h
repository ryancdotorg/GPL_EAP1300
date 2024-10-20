#ifndef _SENAO_ETHERNET_
#define _SENAO_ETHERNET_

/*--- Single PHY---*/
//#define SN_PHY_PORT_NUMBER 1
//#define SN_PHY_ADDR 1

/*--- 2 Port PHY ---*/
#define SN_PHY_PORT_NUMBER 2
#define USE_SENAO_BOARD_PARAM 1

#if defined(_IPQ40XX_BOARD_PARAM_H_) //be defined at ipq40xx_board_param.h
#if defined(USE_SENAO_BOARD_PARAM) && (USE_SENAO_BOARD_PARAM == 1)
gpio_func_data_t SN_sw_gpio_bga[] = {
	{
		.gpio = 52,
		.func = 2,//MDIO
		.pull = GPIO_PULL_UP,
		.drvstr = GPIO_2MA,
		.oe = GPIO_OE_DISABLE,
		.gpio_vm = GPIO_VM_ENABLE,
		.gpio_od_en = GPIO_OD_DISABLE,
		.gpio_pu_res = GPIO_PULL_RES2
	},
	{
		.gpio = 53,
		.func = 2,//MDC
		.pull = GPIO_PULL_UP,
		.drvstr = GPIO_2MA,
		.oe = GPIO_OE_DISABLE,
		.gpio_vm = GPIO_VM_ENABLE,
		.gpio_od_en = GPIO_OD_DISABLE,
		.gpio_pu_res = GPIO_PULL_RES2
	},
	{
		.gpio = 59,
		.func = 0,//GPIO
		.pull = GPIO_NO_PULL,// no effective
		.out = 1, //1:pull high 0:pull low(Cricket=1 ,Stinkbug =1)
		.drvstr = GPIO_2MA,
		.oe = GPIO_OE_ENABLE,
		.gpio_vm = GPIO_VM_ENABLE,
		.gpio_od_en = GPIO_OD_DISABLE,
		.gpio_pu_res = GPIO_PULL_RES2
	},
};

#if defined(SN_PHY_PORT_NUMBER) && (SN_PHY_PORT_NUMBER == 1)
board_ipq40xx_params_t board_params[] = {
	{
		.machid = MACH_TYPE_IPQ40XX_AP_DK01_1_C1,
		.ddr_size = (256 << 20),
		.mtdids = "nand2=spi0.0",
		.spi_nor_gpio = spi_nor_bga,
		.spi_nor_gpio_count = ARRAY_SIZE(spi_nor_bga),
		.nand_gpio = nand_gpio_bga,
		.nand_gpio_count = ARRAY_SIZE(nand_gpio_bga),
		.sw_gpio = SN_sw_gpio_bga,
		.sw_gpio_count = ARRAY_SIZE(SN_sw_gpio_bga),
//IS_SINGLE_PHY------
        .rgmii_gpio = rgmii_gpio_cfg,
        .rgmii_gpio_count = ARRAY_SIZE(rgmii_gpio_cfg),
//IS_SINGLE_PHY------
		.edma_cfg = {
			ipq40xx_edma_cfg(0, 5, RGMII,
					0, 1, 2, 3, 4)
		},

		//.uart_cfg = &uart2, ble extern control
		.console_uart_cfg = &uart1_console_uart_dk01,
#ifdef CONFIG_IPQ40XX_I2C
		.i2c_cfg = &i2c0,
#endif
		//.mmc_gpio = mmc_ap_dk04,
		//.mmc_gpio_count = ARRAY_SIZE(mmc_ap_dk04),
		.spi_nand_available = 0,
		.nor_nand_available = 0,
		.nor_emmc_available = 0,
		.dtb_config_name = "config@4",//spf_5.3_CS format, no #
#ifdef CONFIG_IPQ40XX_PCI
		.pcie_cfg = {
			pcie_board_cfg(0),
		},
#endif
	},
};
#endif //#if defined(SN_PHY_PORT_NUMBER) && (SN_PHY_PORT_NUMBER == 1)


/* 2 ports PHY */
#if defined(SN_PHY_PORT_NUMBER) && (SN_PHY_PORT_NUMBER > 1)
board_ipq40xx_params_t board_params[] = {
	{
		.machid = MACH_TYPE_IPQ40XX_AP_DK01_1_C1,
		.ddr_size = (256 << 20),
		.mtdids = "nand2=spi0.0",
		.spi_nor_gpio = spi_nor_bga,
		.spi_nor_gpio_count = ARRAY_SIZE(spi_nor_bga),
		.nand_gpio = nand_gpio_bga,
		.nand_gpio_count = ARRAY_SIZE(nand_gpio_bga),
		.sw_gpio = SN_sw_gpio_bga,
		.sw_gpio_count = ARRAY_SIZE(SN_sw_gpio_bga),
		.edma_cfg = {
			ipq40xx_edma_cfg(0, 5, PSGMII,
					0, 1, 2, 3, 4)
		},

		//.uart_cfg = &uart2, ble extern control
		.console_uart_cfg = &uart1_console_uart_dk01,
#ifdef CONFIG_IPQ40XX_I2C
		.i2c_cfg = &i2c0,
#endif
		//.mmc_gpio = mmc_ap_dk04,
		//.mmc_gpio_count = ARRAY_SIZE(mmc_ap_dk04),
		.spi_nand_available = 0,
		.nor_nand_available = 0,
		.nor_emmc_available = 0,
		.dtb_config_name = "config@4",//spf_5.3_CS format, no #
#ifdef CONFIG_IPQ40XX_PCI
		.pcie_cfg = {
			pcie_board_cfg(0),
		},
#endif
	},
};
#endif //#if defined(SN_PHY_PORT_NUMBER) && (SN_PHY_PORT_NUMBER >1)
#endif //#if defined(USE_SENAO_BOARD_PARAM) && (USE_SENAO_BOARD_PARAM == 1)
#endif //#if defined(_IPQ40XX_BOARD_PARAM_H_)
#endif //#ifndef _SENAO_ETHERNET_
