#include "linux/types.h"
#include "asm-generic/gpio.h" /* led and reset pin */
#include "common.h"
#include "platform/board_customization.h"
#include "config.h"

void recovery_mode_start_led_on(void)
{
    gpio_set_value(POWER_LED, LED_ON);
#ifdef CONFIG_SP938BS_LED
    gpio_set_value(LINK_1_LED, LED_ON);
    gpio_set_value(LINK_2_LED, LED_ON);
    gpio_set_value(LINK_3_LED, LED_ON);
    gpio_set_value(LINK_4_LED, LED_ON);
#else   /* #ifdef CONFIG_SP938BS_LED */
    gpio_set_value(IN_USE_LED, LED_ON);
    gpio_set_value(INT_LED, LED_ON);
    gpio_set_value(PA_LED, LED_ON);
#endif  /* #ifdef CONFIG_SP938BS_LED */
}

void recovery_mode_start_led_off(void)
{
    gpio_set_value(POWER_LED, LED_ON);
#ifdef CONFIG_SP938BS_LED
    gpio_set_value(LINK_1_LED, LED_OFF);
    gpio_set_value(LINK_2_LED, LED_OFF);
    gpio_set_value(LINK_3_LED, LED_OFF);
    gpio_set_value(LINK_4_LED, LED_OFF);
#else   /* #ifdef CONFIG_SP938BS_LED */
    gpio_set_value(IN_USE_LED, LED_OFF);
    gpio_set_value(INT_LED, LED_OFF);
    gpio_set_value(PA_LED, LED_OFF);
#endif  /* #ifdef CONFIG_SP938BS_LED */
}

void board_gpio_init(void)
{
    printf(">> UBOOT: %s GPIO Initialization!! <<\n", SENAO_MODEL_NAME);
    /* led gpio initialization */
    gpio_request(POWER_LED, "lpower_led");
#ifdef CONFIG_SP938BS_LED
    gpio_request(LINK_1_LED, "link_1_led");
    gpio_request(LINK_2_LED, "link_2_led");
    gpio_request(LINK_3_LED, "link_3_led");
    gpio_request(LINK_4_LED, "link_4_led");
#else   /* #ifdef CONFIG_SP938BS_LED */
    gpio_request(IN_USE_LED, "in_use_led");
    gpio_request(INT_LED, "int_led");
    gpio_request(PA_LED, "pa_led");
#endif  /* #ifdef CONFIG_SP938BS_LED */
    /* turn on all leds for checking leds work or not */
    gpio_direction_output(POWER_LED, LED_ON);
#ifdef CONFIG_SP938BS_LED
    gpio_direction_output(LINK_1_LED, LED_ON);
    gpio_direction_output(LINK_2_LED, LED_ON);
    gpio_direction_output(LINK_3_LED, LED_ON);
    gpio_direction_output(LINK_4_LED, LED_ON);
#else   /* #ifdef CONFIG_SP938BS_LED */
    gpio_direction_output(IN_USE_LED, LED_ON);
    gpio_direction_output(INT_LED, LED_ON);
    gpio_direction_output(PA_LED, LED_ON);
#endif  /* #ifdef CONFIG_SP938BS_LED */
    /* button gpio initialization */
    gpio_request(RESET_BUTTON, "rst_button");
    gpio_request(REG_BUTTON, "reg_button");
    /* set button to input for detecting */
    gpio_direction_input(RESET_BUTTON);
    gpio_direction_input(REG_BUTTON);
}

void boardtest_mode_led_indication(void)
{
#ifdef CONFIG_SP938BS_LED
    gpio_set_value(LINK_1_LED, LED_OFF);
    gpio_set_value(LINK_2_LED, LED_ON);
    gpio_set_value(LINK_3_LED, LED_OFF);
    gpio_set_value(LINK_4_LED, LED_ON);
#else   /* #ifdef CONFIG_SP938BS_LED */
    gpio_set_value(IN_USE_LED, LED_OFF);
    gpio_set_value(INT_LED, LED_ON);
    gpio_set_value(PA_LED, LED_OFF);
#endif  /* #ifdef CONFIG_SP938BS_LED */
}

void recovery_mode_led_indication(int status)
{
#ifdef CONFIG_SP938BS_LED
    gpio_set_value(LINK_1_LED, status);
    gpio_set_value(LINK_2_LED, status);
    gpio_set_value(LINK_3_LED, status);
    gpio_set_value(LINK_4_LED, status);
#else   /* #ifdef CONFIG_SP938BS_LED */
    gpio_set_value(IN_USE_LED, status);
    gpio_set_value(INT_LED, status);
    gpio_set_value(PA_LED, status);
#endif  /* #ifdef CONFIG_SP938BS_LED */
}

int get_reset_button_status(void)
{
    //unsigned long trigger_button = gpio_get_value(RESET_BUTTON);/* reset button only */
    /* Modified forpower button */
    unsigned long trigger_button = (gpio_get_value(RESET_BUTTON) | gpio_get_value(REG_BUTTON));/* reg and reset button */
    if(trigger_button == ENTER_RECOVERY_MODE)
    {
        printf("Enter recovery mode by pressing RESET/REG button!!!\n");
        recovery_mode_start_led_on();
        return PRESSED;
    }
    return get_reg_button_status();
}

int get_reg_button_status(void)
{
    unsigned long trigger_button = gpio_get_value(REG_BUTTON);
    if(trigger_button == ENTER_BOARDTEST_MODE)
    {
        /* set yes to indicate that process boardtest after entering filesystem */
        boardtest_mode_led_indication();
        setenv("optargs", "boardtest=yes");
        return RELEASE;
    }
    else
    {
        setenv("optargs", "boardtest=no");
        return RELEASE;
    }
}
