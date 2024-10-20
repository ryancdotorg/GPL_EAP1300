#include "linux/types.h"
#include "asm-generic/gpio.h" /* led and reset pin */
#include "common.h"
#include "platform/board_customization.h"
#include "config.h"

#ifdef CONFIG_UBOOT_HAS_LTE_PLS8X
#define CONFIG_UBOOT_LTE_PLS8X
#endif  /* CONFIG_UBOOT_HAS_LTE_PLS8X */

void recovery_mode_start_led_on(void)
{
    gpio_set_value(LTE_LED_G, LED_OFF);/* HW 0.30 add */
    gpio_set_value(SYSTEM_LED_R, LED_ON);
    gpio_set_value(SYSTEM_LED_G, LED_OFF);
    gpio_set_value(BT_LED, LED_OFF);
    gpio_set_value(WLAN_LED, LED_OFF);
    gpio_set_value(ANT_LED_G, LED_OFF);
    gpio_set_value(ANT_LED_Y, LED_ON);
    gpio_set_value(LORA_LED_G, LED_OFF);
    gpio_set_value(LORA_LED_Y, LED_ON);
}

void recovery_mode_start_led_off(void)
{
    gpio_set_value(LTE_LED_G, LED_ON);/* HW 0.30 add */
    gpio_set_value(SYSTEM_LED_R, LED_OFF);
    gpio_set_value(SYSTEM_LED_G, LED_ON);
    gpio_set_value(BT_LED, LED_ON);
    gpio_set_value(WLAN_LED, LED_ON);
    gpio_set_value(ANT_LED_G, LED_ON);
    gpio_set_value(ANT_LED_Y, LED_OFF);
    gpio_set_value(LORA_LED_G, LED_ON);
    gpio_set_value(LORA_LED_Y, LED_OFF);
}

void board_gpio_init(void)
{
    printf(">> UBOOT: %s GPIO Initialization!! <<\n", SENAO_MODEL_NAME);
    /* led gpio initialization */
    gpio_request(LTE_LED_G, "lte_led_g");/* HW 0.30 add */
    gpio_request(SYSTEM_LED_R, "system_led_r");
    gpio_request(SYSTEM_LED_G, "system_led_g");
    gpio_request(BT_LED, "bt_led");
    gpio_request(WLAN_LED, "wlan_led");
    gpio_request(ANT_LED_G, "ant_led_g");
    gpio_request(ANT_LED_Y, "ant_led_y");
    gpio_request(LORA_LED_G, "lora_led_g");
    gpio_request(LORA_LED_Y, "lora_led_y");
    /* turn on all leds for checking leds work or not */
    gpio_direction_output(LTE_LED_G, LED_ON);/* HW 0.30 add */
    gpio_direction_output(SYSTEM_LED_R, LED_ON);
    gpio_direction_output(SYSTEM_LED_G, LED_ON);
    gpio_direction_output(BT_LED, LED_ON);
    gpio_direction_output(WLAN_LED, LED_ON);
    gpio_direction_output(ANT_LED_G, LED_ON);
    gpio_direction_output(ANT_LED_Y, LED_ON);
    gpio_direction_output(LORA_LED_G, LED_ON);
    gpio_direction_output(LORA_LED_Y, LED_ON);
    /* button gpio initialization */
    gpio_request(RESET_BUTTON, "rst_button");
    gpio_request(WPS_BUTTON, "wps_button");
    /* set button to input for detecting */
    gpio_direction_input(RESET_BUTTON);
    gpio_direction_input(WPS_BUTTON);
    /* BT, WLAN and LTE enable pin control */
    gpio_request(BT_EN, "bt_en");
    gpio_request(WLAN_EN, "wlan_en");
    gpio_request(W_DISABLE_MPCI, "lte_w");
    gpio_request(LTE_PWR_ON_PCIE, "lte_pwr");
    gpio_request(LTE_RST_N_PCIE, "lte_rst");
    
    if(1)
    {
        printf(">> DISABLE BT, WLAN and LTE!!! <<\n");
        gpio_direction_output(BT_EN, MODULE_DISABLE);
        gpio_direction_output(WLAN_EN, MODULE_DISABLE);
#ifdef CONFIG_UBOOT_LTE_PLS8X
        gpio_direction_output(LTE_PWR_ON_PCIE, MODULE_DISABLE); /* invert, set EMERG_OFF to high */
        gpio_direction_output(LTE_RST_N_PCIE, MODULE_ENABLE); /* invert, set IGT to low */
#else /* ublox toby-l200 */
        gpio_direction_output(W_DISABLE_MPCI, MODULE_DISABLE);
        gpio_direction_output(LTE_PWR_ON_PCIE, MODULE_DISABLE);
        gpio_direction_output(LTE_RST_N_PCIE, MODULE_DISABLE);
#endif
    }
    else
    {
        printf(">> ENABLE BT, WLAN and LTE!!! <<\n");
        gpio_direction_output(BT_EN, MODULE_ENABLE);
        gpio_direction_output(WLAN_EN, MODULE_ENABLE);
#ifdef CONFIG_UBOOT_LTE_PLS8X
        gpio_direction_output(LTE_PWR_ON_PCIE, MODULE_DISABLE); /* invert */
        gpio_direction_output(LTE_RST_N_PCIE, MODULE_DISABLE); /* invert */
#else /* ublox toby-l200 */
        gpio_direction_output(W_DISABLE_MPCI, MODULE_ENABLE);
        gpio_direction_output(LTE_PWR_ON_PCIE, MODULE_ENABLE);
        gpio_direction_output(LTE_RST_N_PCIE, MODULE_ENABLE);
#endif
    }
}

void boardtest_mode_led_indication(void)
{
    gpio_set_value(LTE_LED_G, LED_ON);/* HW 0.30 add */
    gpio_set_value(SYSTEM_LED_R, LED_OFF);
    gpio_set_value(SYSTEM_LED_G, LED_ON);
    gpio_set_value(BT_LED, LED_OFF);
    gpio_set_value(WLAN_LED, LED_ON);
    gpio_set_value(ANT_LED_G, LED_ON);
    gpio_set_value(ANT_LED_Y, LED_OFF);
    gpio_set_value(LORA_LED_G, LED_ON);
    gpio_set_value(LORA_LED_Y, LED_OFF);
}

void recovery_mode_led_indication(int status)
{
    gpio_set_value(ANT_LED_Y, status);
    gpio_set_value(LORA_LED_Y, status);
}

int get_reset_button_status(void)
{
    //unsigned long trigger_button = gpio_get_value(RESET_BUTTON);/* reset button only */
    /* Modified forpower button */
    unsigned long trigger_button = (gpio_get_value(RESET_BUTTON) | gpio_get_value(WPS_BUTTON));/* wps and reset button */
    if(trigger_button == ENTER_RECOVERY_MODE)
    {
        printf("Enter recovery mode by pressing RESET/WPS button!!!\n");
        recovery_mode_start_led_on();
        return PRESSED;
    }
    return get_reg_button_status();
}

int get_reg_button_status(void)
{
    unsigned long trigger_button = gpio_get_value(WPS_BUTTON);
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
