#ifndef __BOARD_CUSTOMIZATION_SP938BS_H
#define __BOARD_CUSTOMIZATION_SP938BS_H

/* LED */
#define LED_ON      1
#define LED_OFF     0

#define GPIO_TO_PIN(bank, gpio)	(32 * (bank) + (gpio))

#ifdef CONFIG_SP938BS_LED
#define LINK_1_LED              GPIO_TO_PIN(1,17)
#define LINK_2_LED              GPIO_TO_PIN(1,18)
#define LINK_3_LED              GPIO_TO_PIN(1,19)
#define LINK_4_LED              GPIO_TO_PIN(1,23)
#else   /* #ifdef CONFIG_SP938BS_LED */
#define IN_USE_LED              GPIO_TO_PIN(1,17)
#define INT_LED                 GPIO_TO_PIN(1,18)
#define PA_LED                  GPIO_TO_PIN(1,19)
#endif  /* #ifdef CONFIG_SP938BS_LED */
#define POWER_LED               GPIO_TO_PIN(1,22)

/* button define */
#define PRESSED                 1
#define RELEASE                 0

#define RESET_BUTTON            GPIO_TO_PIN(2,0)
#define REG_BUTTON              GPIO_TO_PIN(2,1)

#define ENTER_RECOVERY_MODE     0           /* Default is high, get low after pressing button */
#define ENTER_BOARDTEST_MODE    0           /* REG button, default is high, get low after pressing button */


#endif  /* #ifndef __BOARD_CUSTOMIZATION_SP938BS_H */
