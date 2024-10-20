/*
 * Host AP (software wireless LAN access point) user space daemon for
 * Host AP kernel driver / UNIX domain socket -based control interface
 * Copyright (c) 2004, Jouni Malinen <jkmaline@cc.hut.fi>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <sockIntf.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>
#include "ctrl_iface.h"
#include "eloop.h"
#include "finger.h"
#include "fingerhandle.h"
#include "fingerdcmd.h"
#include "aesapi.h"
#include "fingerprint.h"
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/
#define REPLY_SIZE          128
#define RECV_BUFF_MAX_SIZE  4096
/*-------------------------------------------------------------------------*/
/*                           Parameter                                     */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/
/*****************************************************************
 * NAME:    fingersyncd_ctrl_cli_path
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
static char * fingersyncd_ctrl_cli_path(struct fingersyncd_data *hapd)
{
    char *buf;
    size_t len;

    len = strlen(FINGERD_CTRL_INTERFACE) + strlen(FINGERD_CTRL_INTERFACE_IF) + 2;
    buf = malloc(len);
    if(buf == NULL)
        return NULL;

    snprintf(buf, len, "%s/%s", FINGERD_CTRL_INTERFACE, FINGERD_CTRL_INTERFACE_IF);

    return buf;
}

/*****************************************************************
 * NAME:    fingersyncd_cli_receive
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
static void fingersyncd_cli_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
    char buf[256];
    int res;
    struct sockaddr_un from;
    socklen_t fromlen = sizeof(from);
    char *reply;
    int reply_len;


    res = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &from, &fromlen);
    if(res < 0)
    {
        perror("recvfrom(ctrl_iface)");
        return;
    }
    buf[res] = '\0';
    /* fingersync_hexdump_ascii(FINGER_MSGDUMP, "RX ctrl_iface", (u8 *) buf, res); */

    reply = malloc(REPLY_SIZE);
    if(reply == NULL)
    {
        sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *) &from, fromlen);
        return;
    }

    reply_len = fingersyncdCmdHandle(buf, reply);

    if(reply_len < 0)
    {
        memcpy(reply, "FAIL\n", 5);
        reply_len = 5;
    }
    sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from, fromlen);
    free(reply);
}

/*****************************************************************
 * NAME:    fingersyncd_cli_init
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
int fingersyncd_cli_init(struct fingersyncd_data *syscd)
{
    int s = -1;
    char *fname = NULL;
    struct sockaddr_un addr;

    syscd->ctrl_sock = -1;

    if(mkdir(FINGERD_CTRL_INTERFACE, S_IRWXU | S_IRWXG) < 0)
    {
        if(errno == EEXIST)
        {
            fingersync_printf(FINGER_ERROR, "Using existing control interface directory.\n");
        }
        else
        {
            perror("mkdir[ctrl_interface]");
            goto fail;
        }
    }

    s = socket(PF_UNIX, SOCK_DGRAM, 0);
    if(s < 0)
    {
        perror("socket(PF_UNIX)");
        goto fail;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    fname = fingersyncd_ctrl_cli_path(syscd);
    if(fname == NULL)
        goto fail;
    strncpy(addr.sun_path, fname, sizeof(addr.sun_path));
    if(bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        perror("bind(PF_UNIX) "FINGERD_CTRL_INTERFACE);
        goto fail;
    }

    if(chmod(fname, S_IRWXU | S_IRWXG) < 0)
    {
        perror("chmod[ctrl_interface/ifname]");
        goto fail;
    }
    free(fname);

    syscd->ctrl_sock = s;

    eloop_register_read_sock(s, fingersyncd_cli_receive, syscd, NULL);

    return 0;

fail:
    if(s >= 0)
        close(s);

    if(fname)
    {
        unlink(fname);
        free(fname);
    }

    return -1;
}

/*****************************************************************
 * NAME:    fingersyncd_cli_deinit
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
void fingersyncd_cli_deinit(struct fingersyncd_data *syscd)
{
    if(syscd->ctrl_sock > -1)
    {
        char *fname;
        eloop_unregister_read_sock(syscd->ctrl_sock);
        close(syscd->ctrl_sock);
        syscd->ctrl_sock = -1;
        fname = fingersyncd_ctrl_cli_path(syscd);
        if(fname)
            unlink(fname);
        free(fname);

        rmdir(FINGERD_CTRL_INTERFACE);
    }
}
/*****************************************************************
 * NAME:    fingersyncd_ctrl_data_receive
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
static void fingersyncd_ctrl_data_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
    char ciphertext[RECV_BUFF_MAX_SIZE];
    int res;
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    int len;
    //decryption
    char *plaintext = NULL;
    unsigned char salt[8] = {0};
    finger_sync_header_pkt_t *header = NULL;
    int header_length = 0;

    res = recvfrom(sock, ciphertext, sizeof(ciphertext) - 1, 0, (struct sockaddr *) &from, &fromlen);
    if(res < 0)
    {
        perror("recvfrom(ctrl_iface)");
        return;
    }

    header_length = sizeof(finger_sync_header_pkt_t);
    if(res < header_length)
    {
        perror("header too short");
        return;
    }

    header = (finger_sync_header_pkt_t *)ciphertext;

    if(header->magic[0] != FINGER_MAJOR_MAGIC ||
            header->magic[1] != FINGER_MINOR_MAGIC ||
            header->magic[2] != FINGER_NEW_MAJOR_MAGIC ||
            header->magic[3] != FINGER_NEW_MINOR_MAGIC)
    {
        fingersync_printf(FINGER_DEBUG, "Magic wrong [%02x %02x %02x %02x].\n",
                header->magic[0], header->magic[1], header->magic[2], header->magic[3]);
        return;
    }

    if(header->version[0] != FINGER_MAJOR_VERSION)
    {
        fingersync_printf(FINGER_DEBUG, "Different major version, ignore [%02x %02x].\n",
                header->version[0], header->version[1]);
        return;
    }

    memcpy(salt, header->salt, 8);

    ciphertext[res] = '\0';

    len = res - header_length;
    if(len < 32)
    {
        // just simple protect...
        return;
    }
#if PACKET_ENC
    plaintext = (char *)aes_crypt((unsigned char *)&ciphertext[header_length], &len, salt, 0);
#else
    plaintext = &ciphertext[header_length];
#endif

    if(plaintext != NULL)
    {
        fingersync_hexdump_ascii(FINGER_MSGDUMP, "RX data plaintext", (u8 *) plaintext, len);
        fingersyncPacketHandleCB(sock, from.sin_addr.s_addr, (uint8_t *)plaintext, len, header->type);
#if PACKET_ENC
        free(plaintext);
#endif
    }
}
/*****************************************************************
 * NAME:    fingersyncd_ctrl_data_receive
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
static void fingersyncd_fingerprint_data_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
    char ciphertext[RECV_BUFF_MAX_SIZE];
    int res;
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    int len;
    //decryption
    char *plaintext = NULL;
    unsigned char salt[8] = {0};
    finger_sync_header_pkt_t *header = NULL;
    int header_length = 0;

    res = recvfrom(sock, ciphertext, sizeof(ciphertext) - 1, 0, (struct sockaddr *) &from, &fromlen);
    if(res < 0)
    {
        perror("recvfrom(ctrl_iface)");
        return;
    }

    header_length = sizeof(finger_sync_header_pkt_t);
    if(res < header_length)
    {
        perror("header too short");
        return;
    }

    header = (finger_sync_header_pkt_t *)ciphertext;
    if(header->magic[0] != FINGER_MAJOR_MAGIC ||
            header->magic[1] != FINGER_MINOR_MAGIC ||
            header->magic[2] != FINGER_NEW_MAJOR_MAGIC ||
            header->magic[3] != FINGER_NEW_MINOR_MAGIC)
    {
        fingersync_printf(FINGER_DEBUG, "Magic wrong [%02x %02x %02x %02x].\n",
                header->magic[0], header->magic[1], header->magic[2], header->magic[3]);
        return;
    }

    if(header->version[0] != FINGER_MAJOR_VERSION)
    {
        fingersync_printf(FINGER_DEBUG, "Different major version, ignore [%02x %02x].\n",
                header->version[0], header->version[1]);
        return;
    }

    memcpy(salt, header->salt, 8);

    ciphertext[res] = '\0';

    len = res - header_length;
    if(len < 32)
    {
        // just simple protect...
        return;
    }
#if PACKET_ENC
    plaintext = (char *)aes_crypt((unsigned char *)&ciphertext[header_length], &len, salt, 0);
#else
    plaintext = &ciphertext[header_length];
#endif

    if(plaintext != NULL)
    {
        fingersync_hexdump_ascii(FINGER_MSGDUMP, "RX data plaintext", (u8 *) plaintext, len);
        fingerprintsyncPacketHandleCB(sock, from.sin_addr.s_addr, (uint8_t *)plaintext, len, header->type);
#if PACKET_ENC
        free(plaintext);
#endif
    }
}
/*****************************************************************
 * NAME:    fingersyncd_ctrl_data_init
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
int fingersyncd_ctrl_data_init(struct fingersyncd_data *syscd)
{
    int s = -1;

    s = listen_socket(htonl(INADDR_ANY), FINGER_UDP_PORT, fingersyncInterface());
    syscd->ctrl_sock = s;
    eloop_register_read_sock(s, fingersyncd_ctrl_data_receive, syscd, NULL);

    return 0;
}

/*****************************************************************
 * NAME:    fingersyncd_ctrl_iface_deinit
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
void fingersyncd_ctrl_data_deinit(struct fingersyncd_data *syscd)
{
    if(syscd->ctrl_sock > -1)
    {
        eloop_unregister_read_sock(syscd->ctrl_sock);
        close(syscd->ctrl_sock);
        syscd->ctrl_sock = -1;
    }
}

/*****************************************************************
 * NAME:    fingersyncd_ctrl_data_init
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
int fingersyncd_fingerprint_init(struct fingersyncd_data *syscd)
{
    int s = -1;

    s = listen_socket(htonl(INADDR_ANY), FINGER_UDP_PORT, fingersyncInterface());
    syscd->ctrl_sock = s;
    eloop_register_read_sock(s, fingersyncd_fingerprint_data_receive, syscd, NULL);

    return 0;
}

/*****************************************************************
 * NAME:    fingersyncd_ctrl_iface_deinit
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
void fingersyncd_fingerprint_deinit(struct fingersyncd_data *syscd)
{
    if(syscd->ctrl_sock > -1)
    {
        eloop_unregister_read_sock(syscd->ctrl_sock);
        close(syscd->ctrl_sock);
        syscd->ctrl_sock = -1;
    }
}

int fingerprint_init(struct fingersyncd_data *syscd)
{
    int s = -1, on = 1;

    // Submit request for a raw socket descriptor to receive packets.
    if ((s = socket (PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) //need to use ETH_P_ALL to listen all packet, and apply USE_TCPDUMP_PACKET_FILTER for only dhcp
    {
        perror ("socket() failed to obtain a receive socket descriptor ");
        goto fail;
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("Can't set reuse option");
    }

#if USE_TCPDUMP_PACKET_FILTER
    struct sock_fprog filter;
    struct sock_filter bpf_code[] = { //tcpdump -dd -s 2048 'udp port 67 or udp port 68'
        { 0x28, 0, 0, 0x0000000c },
        { 0x15, 0, 7, 0x000086dd },
        { 0x30, 0, 0, 0x00000014 },
        { 0x15, 0, 18, 0x00000011 },
        { 0x28, 0, 0, 0x00000036 },
        { 0x15, 15, 0, 0x00000043 },
        { 0x15, 14, 0, 0x00000044 },
        { 0x28, 0, 0, 0x00000038 },
        { 0x15, 12, 11, 0x00000043 },
        { 0x15, 0, 12, 0x00000800 },
        { 0x30, 0, 0, 0x00000017 },
        { 0x15, 0, 10, 0x00000011 },
        { 0x28, 0, 0, 0x00000014 },
        { 0x45, 8, 0, 0x00001fff },
        { 0xb1, 0, 0, 0x0000000e },
        { 0x48, 0, 0, 0x0000000e },
        { 0x15, 4, 0, 0x00000043 },
        { 0x15, 3, 0, 0x00000044 },
        { 0x48, 0, 0, 0x00000010 },
        { 0x15, 1, 0, 0x00000043 },
        { 0x15, 0, 1, 0x00000044 },
        { 0x6, 0, 0, 0x00000800 },
        { 0x6, 0, 0, 0x00000000 },
    };
    filter.len = sizeof(bpf_code)/sizeof(bpf_code[0]);
    filter.filter = bpf_code;

    setsockopt(s, SOL_SOCKET, SO_ATTACH_FILTER, &filter, sizeof(filter));
#endif

    syscd->ctrl_sock = s;
    eloop_register_read_sock(s, fingerprint_data_receive, syscd, NULL);

    return 0;

fail:
    if(s >= 0)
    {
        close(s);
    }
    return -1;
}

void fingerprint_deinit(struct fingersyncd_data *syscd)
{
    if(syscd->ctrl_sock > -1)
    {
        eloop_unregister_read_sock(syscd->ctrl_sock);
        close(syscd->ctrl_sock);
        syscd->ctrl_sock = -1;
    }
}
