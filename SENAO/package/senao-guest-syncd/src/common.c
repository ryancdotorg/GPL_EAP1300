/*****************************************************************************
;
;   (C) Unpublished Work of Senao Networks, Inc.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF SENAO INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) SENAO EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN SENAO WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF SENAO.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;------------------------------------------------------------------------------
;
;    Project :
;    Creator :
;    File    :
;    Abstract:
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;       Yolin          2017-08-01
;*****************************************************************************/
/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <common.h>
#include <sys/wait.h>
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/
#ifndef CONFIG_NO_STDOUT_DEBUG
/*-------------------------------------------------------------------------*/
/*                           Parameter                                     */
/*-------------------------------------------------------------------------*/
int guestsync_debug_level = GUESTSYNC_ERROR;
int guestsync_debug_show_keys = 0;
int guestsync_debug_timestamp = 0;
/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/
void guestsync_debug_print_timestamp(void)
{
    struct timeval tv;
    char buf[16];

    if (!guestsync_debug_timestamp)
        return;

    gettimeofday(&tv, NULL);

    if (strftime(buf, sizeof(buf), "%b %d %H:%M:%S",
                localtime((const time_t *) &tv.tv_sec)) <= 0) {
        snprintf(buf, sizeof(buf), "%u", (int) tv.tv_sec);
    }
    printf("%s.%06u: ", buf, (unsigned int) tv.tv_usec);
}


/**
 * guestsync_printf - conditional printf
 * @level: priority level (GUESTSYNC_*) of the message
 * @fmt: printf format string, followed by optional arguments
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration.
 *
 * Note: New line '\n' is added to the end of the text when printing to stdout.
 */
void guestsync_printf(int level, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (level >= guestsync_debug_level) {
        guestsync_debug_print_timestamp();
        vprintf(fmt, ap);
    }
    va_end(ap);
}

static void _guestsync_hexdump(int level, const char *title, const u8 *buf,
        size_t len, int show)
{
    size_t i;
    if (level < guestsync_debug_level)
        return;
    guestsync_debug_print_timestamp();
    printf("%s - hexdump(len=%lu):", title, (unsigned long) len);
    if (buf == NULL) {
        printf(" [NULL]");
    } else if (show) {
        for (i = 0; i < len; i++)
            printf(" %02x", buf[i]);
    } else {
        printf(" [REMOVED]");
    }
    printf("\n");
}

void guestsync_hexdump(int level, const char *title, const u8 *buf, size_t len)
{
    _guestsync_hexdump(level, title, buf, len, 1);
}


void guestsync_hexdump_key(int level, const char *title, const u8 *buf, size_t len)
{
    _guestsync_hexdump(level, title, buf, len, guestsync_debug_show_keys);
}


static void _guestsync_hexdump_ascii(int level, const char *title, const u8 *buf,
        size_t len, int show)
{
    int i, llen;
    const u8 *pos = buf;
    const int line_len = 16;

    if (level < guestsync_debug_level)
        return;
    guestsync_debug_print_timestamp();
    if (!show) {
        printf("%s - hexdump_ascii(len=%lu): [REMOVED]\n",
                title, (unsigned long) len);
        return;
    }
    if (buf == NULL) {
        printf("%s - hexdump_ascii(len=%lu): [NULL]\n",
                title, (unsigned long) len);
        return;
    }
    printf("%s - hexdump_ascii(len=%lu):\n", title, (unsigned long) len);
    while (len) {
        llen = len > line_len ? line_len : len;
        printf("    ");
        for (i = 0; i < llen; i++)
            printf(" %02x", pos[i]);
        for (i = llen; i < line_len; i++)
            printf("   ");
        printf("   ");
        for (i = 0; i < llen; i++) {
            if (isprint(pos[i]))
                printf("%c", pos[i]);
            else
                printf("_");
        }
        for (i = llen; i < line_len; i++)
            printf(" ");
        printf("\n");
        pos += llen;
        len -= llen;
    }
}


void guestsync_hexdump_ascii(int level, const char *title, const u8 *buf, size_t len)
{
    _guestsync_hexdump_ascii(level, title, buf, len, 1);
}


void guestsync_hexdump_ascii_key(int level, const char *title, const u8 *buf,
        size_t len)
{
    _guestsync_hexdump_ascii(level, title, buf, len, guestsync_debug_show_keys);
}

#if 0
void pack32(uint32_t val,uint8_t *dest)
{
    dest[0] = (val & 0xff000000) >> 24;
    dest[1] = (val & 0x00ff0000) >> 16;
    dest[2] = (val & 0x0000ff00) >>  8;
    dest[3] = (val & 0x000000ff);
}
uint32_t unpack32(uint8_t *src)
{
    uint32_t val;

    val = src[0] << 24;
    val |= src[1] << 16;
    val |= src[2] << 8;
    val |= src[3];

    return val;
}
#endif

int my_system(char *output, int length, char *fmt, ...)
{
    char command[1024];
    FILE *pipe;
    int c;
    int i;
    va_list ap;

    memset(command, 0, sizeof(command));

    va_start(ap, fmt);
    vsnprintf(command, sizeof(command), fmt, ap);
    va_end(ap);

    if ((pipe = popen(command, "r")) == NULL)
    {
        goto err;
    }

    for (i = 0; ((c = fgetc(pipe)) != EOF) && (i < length - 1); i++)
    {
        output[i] = (char) c;
    }
    output[i] = '\0';

    pclose(pipe);

    if (strlen(output) == 0)
    {
        goto err;
    }

    return strlen(output);

err:
    output[0] = '\0';
    return -1;
}

int get_cloud_index(char *dut_dhcpif)
{
    int ssid_num = 1;
    int cloud_index = 1;
    char rets[64] = {0};

    if(access("/etc/config/ezmcloud", F_OK) == 0)
    {
        sscanf(dut_dhcpif, "br-ssid%d", &ssid_num);
        memset(rets, 0, sizeof(rets));
        my_system(rets, sizeof(rets), "uci get cloud_mapping.`foreach cloud_mapping ap dut_config_index %d`.cloud_config_index", ssid_num);
        sscanf(rets, "%d", &cloud_index);
    }
    else
    {
        sscanf(dut_dhcpif, "br-ssid%d", &cloud_index);
    }
    return cloud_index;
}

int get_ssid_num(char *list_dhcpif)
{
    int ssid_num = 1;
    int cloud_index = 1;
    char rets[64] = {0};

    if(access("/etc/config/ezmcloud", F_OK) == 0)
    {
        sscanf(list_dhcpif, "br-ssid%d", &cloud_index);
        memset(rets, 0, sizeof(rets));
        my_system(rets, sizeof(rets), "uci get cloud_mapping.`foreach cloud_mapping ap cloud_config_index %d`.dut_config_index", cloud_index);
        sscanf(rets, "%d", &ssid_num);
    }
    else
    {
        sscanf(list_dhcpif, "br-ssid%d", &ssid_num);
    }
    return ssid_num;
}
#endif /* CONFIG_NO_STDOUT_DEBUG */

