//Led
#define WLAN1_5G_LED		0 //led8 high
#define LAN_LED2			1 //led2(Reserve) high
#define LAN_LED1			2 //led3 high
#define WLAN_2G_LED			3 //led4 high
//#define WLAN_Strong_LED	4
//#define WLAN_Good_LED		5
#define JEC_MDC	 			52 //
#define JEC_MDIO			53 //
#define SPI_CS 				54 //BISP_SPI0_SS0_N
#define SPI_MOSI			55 //BISP_SPI0_MOSI
#define SPI_CLK				56 //BISP_SPI0_SCK
#define SPI_MISO			57 //BISP_SPI0_MISO
//Led
#define IPQ40XX_PLATFORM_MACHID MACH_TYPE_IPQ40XX_AP_DK01_1_C1
#define POWER_LED1			58 //led6 low
#define POWER_LED1_ACTIVE_LOW	 1
#define RST_PHY				59
#define UART_RX				60 //BISP_UART0_RXD
#define UART_TX				61 //BISP_UART0_TXD 
#define SYS_RST_L			62 //GPIO62 CHIP_RST_OUT
//Button
#define CHIP_IRQ_IN			63 //SW_Reset SW1


//Button
#define RESET_BTN	CHIP_IRQ_IN


#define POWER_LED1_NAME			"power_led1"
#define LAN_LED1_NAME			"lan_led1"
#define MESH_LED_NAME			"mesh_led"
#define WLAN_5G_LED_NAME		"wlan_5g_led"
#define WLAN_2G_LED_NAME		"wlan_2g_led"

/*
 * define for u-boot gpio button and gpio led test feature
*/
#define SENAO_BUTTON1	RESET_BTN
//#define SENAO_BUTTON2
//#define SENAO_BUTTON3
//#define SENAO_BUTTON4

#define SENAO_LED1	POWER_LED1
#define SENAO_LED1_ACTIVE_LOW	1
#define SENAO_LED2	LAN_LED1
//#define SENAO_LED2_ACTIVE_LOW	1
#define SENAO_LED3 LAN_LED2
//#define SENAO_LED3_ACTIVE_LOW	1
#define SENAO_LED4 WLAN_2G_LED
//#define SENAO_LED4_ACTIVE_LOW	1
#define SENAO_LED5 WLAN1_5G_LED  
//#define SENAO_LED5_ACTIVE_LOW	1
//#define SENAO_LED6  
//#define SENAO_LED6_ACTIVE_LOW	1
//#define SENAO_LED7 
//#define SENAO_LED7_ACTIVE_LOW	1
//#define SENAO_LED8 
//#define SENAO_LED8_ACTIVE_LOW	1
