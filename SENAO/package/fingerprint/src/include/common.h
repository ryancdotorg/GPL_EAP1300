/*
 * fingerd / common helper functions, etc.
 * Copyright (c) 2002-2005, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#ifndef COMMON_H
#define COMMON_H

#ifdef __linux__
#include <endian.h>
#include <byteswap.h>
#endif /* __linux__ */

#if defined(__FreeBSD__) || defined(__NetBSD__)
#include <sys/types.h>
#include <sys/endian.h>
#define __BYTE_ORDER	_BYTE_ORDER
#define	__LITTLE_ENDIAN	_LITTLE_ENDIAN
#define	__BIG_ENDIAN	_BIG_ENDIAN
#define bswap_16 bswap16
#define bswap_32 bswap32
#define bswap_64 bswap64
#endif /* defined(__FreeBSD__) || defined(__NetBSD__) */

#ifdef CONFIG_NATIVE_WINDOWS
#include <winsock2.h>

static inline int daemon(int nochdir, int noclose)
{
    printf("Windows - daemon() not supported yet\n");
    return -1;
}

static inline void sleep(int seconds)
{
    Sleep(seconds * 1000);
}

static inline void usleep(unsigned long usec)
{
    Sleep(usec / 1000);
}

#ifndef timersub
#define timersub(a, b, res) do { \
    (res)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
    (res)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
    if ((res)->tv_usec < 0) { \
        (res)->tv_sec--; \
        (res)->tv_usec += 1000000; \
    } \
} while (0)
#endif

struct timezone {
    int  tz_minuteswest;
    int  tz_dsttime;
};



static inline long int random(void)
{
    return rand();
}

typedef int gid_t;
typedef int socklen_t;

#ifndef FINGER_DONTWAIT
#define FINGER_DONTWAIT 0 /* not supported */
#endif

#endif /* CONFIG_NATIVE_WINDOWS */

#if defined(__CYGWIN__) || defined(CONFIG_NATIVE_WINDOWS)

static inline unsigned short fingersync_swap_16(unsigned short v)
{
    return ((v & 0xff) << 8) | (v >> 8);
}

static inline unsigned int fingersync_swap_32(unsigned int v)
{
    return ((v & 0xff) << 24) | ((v & 0xff00) << 8) |
        ((v & 0xff0000) >> 8) | (v >> 24);
}

#define le_to_host16(n) (n)
#define host_to_le16(n) (n)
#define be_to_host16(n) fingersync_swap_16(n)
#define host_to_be16(n) fingersync_swap_16(n)
#define le_to_host32(n) (n)
#define be_to_host32(n) fingersync_swap_32(n)
#define host_to_be32(n) fingersync_swap_32(n)

#else /* __CYGWIN__ */

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define le_to_host16(n) (n)
#define host_to_le16(n) (n)
#define be_to_host16(n) bswap_16(n)
#define host_to_be16(n) bswap_16(n)
#define le_to_host32(n) (n)
#define be_to_host32(n) bswap_32(n)
#define host_to_be32(n) bswap_32(n)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define le_to_host16(n) bswap_16(n)
#define host_to_le16(n) bswap_16(n)
#define be_to_host16(n) (n)
#define host_to_be16(n) (n)
#define le_to_host32(n) bswap_32(n)
#define be_to_host32(n) (n)
#define host_to_be32(n) (n)
#ifndef WORDS_BIGENDIAN
#define WORDS_BIGENDIAN
#endif
#else
#error Could not determine CPU byte order
#endif

#endif /* __CYGWIN__ */

#include <stdint.h>
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;



/* Debugging function - conditional printf and hex dump. Driver wrappers can
 *  use these for debugging purposes. */

enum { FINGER_MSGDUMP, FINGER_DEBUG, FINGER_INFO, FINGER_WARNING, FINGER_ERROR, FINGER_SHOW };

#ifdef CONFIG_NO_STDOUT_DEBUG

#define fingersync_debug_print_timestamp() do { } while (0)
#define fingersync_printf(args...) do { } while (0)
#define fingersync_hexdump(args...) do { } while (0)
#define fingersync_hexdump_key(args...) do { } while (0)
#define fingersync_hexdump_ascii(args...) do { } while (0)
#define fingersync_hexdump_ascii_key(args...) do { } while (0)

#else /* CONFIG_NO_STDOUT_DEBUG */

/**
 * fingersync_debug_printf_timestamp - Print timestamp for debug output
 *
 * This function prints a timestamp in <seconds from 1970>.<microsoconds>
 * format if debug output has been configured to include timestamps in debug
 * messages.
 */
void fingersync_debug_print_timestamp(void);

/**
 * fingersync_printf - conditional printf
 * @level: priority level (FINGER_*) of the message
 * @fmt: printf format string, followed by optional arguments
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration.
 *
 * Note: New line '\n' is added to the end of the text when printing to stdout.
 */
void fingersync_printf(int level, char *fmt, ...)
__attribute__ ((format (printf, 2, 3)));


/**
 * fingersync_hexdump - conditional hex dump
 * @level: priority level (FINGER_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump.
 */
void fingersync_hexdump(int level, const char *title, const u8 *buf, size_t len);

/**
 * fingersync_hexdump_key - conditional hex dump, hide keys
 * @level: priority level (FINGER_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump. This works
 * like fingersync_hexdump(), but by default, does not include secret keys (passwords,
 * etc.) in debug output.
 */
void fingersync_hexdump_key(int level, const char *title, const u8 *buf, size_t len);

/**
 * fingersync_hexdump_ascii - conditional hex dump
 * @level: priority level (FINGER_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump with both
 * the hex numbers and ASCII characters (for printable range) are shown. 16
 * bytes per line will be shown.
 */
void fingersync_hexdump_ascii(int level, const char *title, const u8 *buf,
        size_t len);

/**
 * fingersync_hexdump_ascii_key - conditional hex dump, hide keys
 * @level: priority level (FINGER_*) of the message
 * @title: title of for the message
 * @buf: data buffer to be dumped
 * @len: length of the buf
 *
 * This function is used to print conditional debugging and error messages. The
 * output may be directed to stdout, stderr, and/or syslog based on
 * configuration. The contents of buf is printed out has hex dump with both
 * the hex numbers and ASCII characters (for printable range) are shown. 16
 * bytes per line will be shown. This works like fingersync_hexdump_ascii(), but by
 * default, does not include secret keys (passwords, etc.) in debug output.
 */
void fingersync_hexdump_ascii_key(int level, const char *title, const u8 *buf,
        size_t len);

#endif /* CONFIG_NO_STDOUT_DEBUG */

void pack32(uint32_t val,uint8_t *dest);
uint32_t unpack32(uint8_t *src);
int my_system(char *output, int size, char *fmt, ...);
int get_cloud_index(char *dut_dhcpif);
int get_ssid_num(char *list_dhcpif);
unsigned long long int meponch(void);

#endif /* COMMON_H */
