#ifdef CONFIG_UBOOT_RECOVERY_MODE
#include <common.h>
#endif  /* #ifdef CONFIG_UBOOT_RECOVERY_MODE */
#ifdef CONFIG_UBOOT_RECOVERY_MODE_LED_BLINK
//#include asm-generic/gpio.h
#include <command.h>
#include <console.h>
#include <environment.h>
#include "net.h"
#ifdef CONFIG_UBOOT_CUSTOMIZATION
#include "platform/board_customization.h"
#endif  /* CONFIG_UBOOT_CUSTOMIZATION */
#endif  /* #ifdef CONFIG_UBOOT_RECOVERY_MODE_LED_BLINK */
#if defined(CONFIG_CMD_EXT4_WRITE)
#include <ext_common.h>
#endif  /* #if defined(CONFIG_CMD_EXT4_WRITE) */
int http_config(char * argv[])
{
    argv[1] = "0x82000000";
    argv[2] = "uImage";
    printf("\tInput Linux Kernel filename ");
    return 0;
}

void start_recovery_mode(void)
{
    char *argv[6];
    int argc = 3;
    char s[20];
    cmd_tbl_t *cmdtp = NULL;

#if defined(CONFIG_CMD_NET)
    eth_initialize();
#endif  /* #if defined(CONFIG_CMD_NET) */

    /* main_loop() can return to retry autoboot, if so just run it again. */
    http_config(argv);
    argc = 3;

    setenv("autostart", "no");

    /* Set default IP */
    setenv("ipaddr", DEFAULT_IP_ADDR);

    printf("\n..............Start httpd @ [%s]..............\r\n\r\n", DEFAULT_IP_ADDR);

#if defined(CONFIG_HAS_RECOVERY_MODE_LED_BLINK) && defined(CONFIG_UBOOT_CUSTOMIZATION)
    recovery_mode_start_led_on();
#endif  /* #if defined(CONFIG_HAS_RECOVERY_MODE_LED_BLINK) && defined(CONFIG_UBOOT_CUSTOMIZATION) */

    do_http(cmdtp, 0, argc, argv);//cmd_net.c

#if defined(CONFIG_HAS_RECOVERY_MODE_LED_BLINK) && defined(CONFIG_UBOOT_CUSTOMIZATION)
    /* turn on LED. */
    recovery_mode_end_led_off();
#endif  /* #if defined(CONFIG_HAS_RECOVERY_MODE_LED_BLINK) && defined(CONFIG_UBOOT_CUSTOMIZATION) */

#ifdef CONFIG_UBOOT_RECOVERY_TO_MMC
    /*
        "tftp ${loadaddr} openwrt-omap-uImage-initramfs; " \
        "ext4write mmc 1:2 ${loadaddr} /boot/zImage ${filesize}; " \
        "tftp ${loadaddr} ${fdtfile}; " \
        "ext4write mmc 1:2 ${loadaddr} /boot/${fdtfile} ${filesize}; " \
     */
#if defined(CONFIG_CMD_EXT4_WRITE)
    printf("Firmware size is %lx bytes\n", (long unsigned int)net_boot_file_size);
    argc = 6;
    argv[1] = "mmc";
    argv[2] = "1:2";
    argv[3] = getenv("loadaddr");
    printf("Update [Kernel with rootFS]!\n");
    argv[4] = "/boot/zImage";
    sprintf(s, "%lx", (long unsigned int)net_boot_file_size);
    argv[5] = s;/* file size */
    do_ext4_write(cmdtp, 0, argc, argv);/* cmd_sf.c */
#endif  /* #if defined(CONFIG_CMD_EXT4_WRITE) */
#endif  /* #ifdef CONFIG_UBOOT_RECOVERY_TO_MMC */

#ifdef CONFIG_UBOOT_RECOVERY_TO_SPI
    /* Received dlf file and tried to upgrade spi flash */
    argc = 1;
    argv[0] = "probe";
    do_spi_fw_probe(cmdtp, 0, argc, argv);/* cmd_sf.c */

    argc = 3;
    argv[0] = "erase";
    argv[1] = getenv("spisrcaddr");
    argv[2] = getenv("spiimgsize");
    do_spi_fw_erase(cmdtp, 0, argc, argv);/* cmd_sf.c */

    argc = 4;
    argv[0] = "write";
    argv[1] = getenv("loadaddr");
    argv[2] = getenv("spisrcaddr");
    sprintf(s, "%lx", (long unsigned int)net_boot_file_size);
    argv[3] = s;
    do_spi_fw_upgrade(cmdtp, 0, argc, argv);/* cmd_sf.c */
#endif  /* #ifdef CONFIG_UBOOT_RECOVERY_TO_SPI */
    printf("\n..............End httpd @ [%s]..............\r\n\r\n", DEFAULT_IP_ADDR);

#ifdef CONFIG_HAS_RECOVERY_MODE_LED_BLINK
    udelay (10000);
#endif  /* #if defined(CONFIG_HAS_RECOVERY_MODEE_LED_BLINK) */

    // reset after all procedures done.
    do_reset(cmdtp, 0, argc, argv);

    return;
}
