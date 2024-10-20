#ifndef __BOARD_CUSTOMIZATION_H
#define __BOARD_CUSTOMIZATION_H

#ifdef CONFIG_HARDWARE_SP938BS
#include "board_customization_sp938bs.h"
#elif CONFIG_HARDWARE_EIG9100
#include "board_customization_eig9100.h"
#elif CONFIG_HARDWARE_SP935
#include "board_customization_sp935.h"
#endif

/* gpio initialization for led and button */
void board_gpio_init(void);

/* led indication when entering recovery mode */
void recovery_mode_start_led_on(void);
void recovery_mode_start_led_off(void);
void recovery_mode_led_indication(int status);

/* led indication when entering boardtest */
void boardtest_mode_led_indication(void);

/* button status get */
int get_reset_button_status(void);
int get_reg_button_status(void);

#endif  /* #ifndef __BOARD_CUSTOMIZATION_H */
