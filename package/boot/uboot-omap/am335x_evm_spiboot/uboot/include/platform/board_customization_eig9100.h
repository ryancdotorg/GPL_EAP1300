#ifndef __BOARD_CUSTOMIZATION_EIG9100_H
#define __BOARD_CUSTOMIZATION_EIG9100_H

/* LED */
#define LED_ON      1
#define LED_OFF     0

#define GPIO_TO_PIN(bank, gpio)	(32 * (bank) + (gpio))

#define LTE_LED_G               GPIO_TO_PIN(0,22)/* HW 0.30 add */
#define SYSTEM_LED_R            GPIO_TO_PIN(0,23)
#define SYSTEM_LED_G            GPIO_TO_PIN(0,26)
#define BT_LED                  GPIO_TO_PIN(0,27)
#define WLAN_LED                GPIO_TO_PIN(1,12)
#define ANT_LED_G               GPIO_TO_PIN(1,13)
#define ANT_LED_Y               GPIO_TO_PIN(1,14)
#define LORA_LED_G              GPIO_TO_PIN(1,15)
#define LORA_LED_Y              GPIO_TO_PIN(1,16)

#define BT_EN                   GPIO_TO_PIN(1,25)
#define WLAN_EN                 GPIO_TO_PIN(1,26)

#define LTE_RST_N_PCIE          GPIO_TO_PIN(2,22)
#define W_DISABLE_MPCI          GPIO_TO_PIN(2,23)
#define LTE_PWR_ON_PCIE         GPIO_TO_PIN(2,24)

#define MODULE_DISABLE          0
#define MODULE_ENABLE           1

/* button define */
#define PRESSED                 1
#define RELEASE                 0

#define RESET_BUTTON            GPIO_TO_PIN(1,29)
#define WPS_BUTTON              GPIO_TO_PIN(2,4)

#define ENTER_RECOVERY_MODE     0           /* Default is high, get low after pressing button */
#define ENTER_BOARDTEST_MODE    0           /* REG button, default is high, get low after pressing button */

#endif  /* #ifndef __BOARD_CUSTOMIZATION_EIG9100_H */
