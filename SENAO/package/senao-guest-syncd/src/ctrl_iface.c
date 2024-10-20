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
#include <guestsyncd.h>
#include <sockIntf.h>
#include <guestsynchandle.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>
#include "ctrl_iface.h"
#include "eloop.h"
#include "guestsync.h"
#include "guestsyncdcmd.h"
#include "aesapi.h"
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
 * NAME:    guestsyncd_ctrl_cli_path
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
static char * guestsyncd_ctrl_cli_path(struct guestsyncd_data *hapd)
{
    char *buf;
    size_t len;

    len = strlen(GUESTSYNC_CTRL_INTERFACE) + strlen(GUESTSYNC_CTRL_INTERFACE_IF) + 2;
    buf = malloc(len);
    if(buf == NULL)
        return NULL;

    snprintf(buf, len, "%s/%s", GUESTSYNC_CTRL_INTERFACE, GUESTSYNC_CTRL_INTERFACE_IF);

    return buf;
}

/*****************************************************************
 * NAME:    guestsyncd_cli_receive
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
static void guestsyncd_cli_receive(int sock, void *eloop_ctx, void *sock_ctx)
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
    /* guestsync_hexdump_ascii(GUESTSYNC_MSGDUMP, "RX ctrl_iface", (u8 *) buf, res); */

    reply = malloc(REPLY_SIZE);
    if(reply == NULL)
    {
        sendto(sock, "FAIL\n", 5, 0, (struct sockaddr *) &from, fromlen);
        return;
    }

    reply_len = guestsyncdCmdHandle(buf, reply);

    if(reply_len < 0)
    {
        memcpy(reply, "FAIL\n", 5);
        reply_len = 5;
    }
    sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from, fromlen);
    free(reply);
}

/*****************************************************************
 * NAME:    guestsyncd_cli_init
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
int guestsyncd_cli_init(struct guestsyncd_data *syscd)
{
    int s = -1;
    char *fname = NULL;
    struct sockaddr_un addr;

    syscd->ctrl_sock = -1;

    if(mkdir(GUESTSYNC_CTRL_INTERFACE, S_IRWXU | S_IRWXG) < 0)
    {
        if(errno == EEXIST)
        {
            guestsync_printf(GUESTSYNC_ERROR, "Using existing control interface directory.\n");
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
    fname = guestsyncd_ctrl_cli_path(syscd);
    if(fname == NULL)
        goto fail;
    strncpy(addr.sun_path, fname, sizeof(addr.sun_path));
    if(bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        perror("bind(PF_UNIX) "GUESTSYNC_CTRL_INTERFACE);
        goto fail;
    }

    if(chmod(fname, S_IRWXU | S_IRWXG) < 0)
    {
        perror("chmod[ctrl_interface/ifname]");
        goto fail;
    }
    free(fname);

    syscd->ctrl_sock = s;

    eloop_register_read_sock(s, guestsyncd_cli_receive, syscd, NULL);

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
 * NAME:    guestsyncd_cli_deinit
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
void guestsyncd_cli_deinit(struct guestsyncd_data *syscd)
{
    if(syscd->ctrl_sock > -1)
    {
        char *fname;
        eloop_unregister_read_sock(syscd->ctrl_sock);
        close(syscd->ctrl_sock);
        syscd->ctrl_sock = -1;
        fname = guestsyncd_ctrl_cli_path(syscd);
        if(fname)
            unlink(fname);
        free(fname);

        rmdir(GUESTSYNC_CTRL_INTERFACE);
    }
}
/*****************************************************************
 * NAME:    guestsyncd_ctrl_data_receive
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
static void guestsyncd_ctrl_data_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
    char ciphertext[RECV_BUFF_MAX_SIZE];
    int res;
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    //decryption
    char *plaintext = NULL;
    int len;
    unsigned char salt[8] = {0};
    guest_sync_header_pkt_t *header = NULL;
    int header_length = 0;

    res = recvfrom(sock, ciphertext, sizeof(ciphertext) - 1, 0, (struct sockaddr *) &from, &fromlen);
    if(res < 0)
    {
        perror("recvfrom(ctrl_iface)");
        return;
    }

    header_length = sizeof(guest_sync_header_pkt_t);
    if(res < header_length)
    {
        perror("header too short");
        return;
    }

    header = (guest_sync_header_pkt_t *)ciphertext;

    if(header->magic[0] != GUESTSYNC_MAJOR_MAGIC ||
            header->magic[1] != GUESTSYNC_MINOR_MAGIC ||
            header->magic[2] != GUESTSYNC_NEW_MAJOR_MAGIC ||
            header->magic[3] != GUESTSYNC_NEW_MINOR_MAGIC)
    {
        guestsync_printf(GUESTSYNC_DEBUG, "Magic wrong [%02x %02x %02x %02x].\n",
                header->magic[0], header->magic[1], header->magic[2], header->magic[3]);
        return;
    }

    if(header->version[0] != GUESTSYNC_MAJOR_VERSION)
    {
        guestsync_printf(GUESTSYNC_DEBUG, "Different major version, ignore [%02x %02x].\n",
                header->version[0], header->version[1]);
        return;
    }

    memcpy(salt, header->salt, 8);

    ciphertext[res] = '\0';

    len = res - header_length;
    if(len > 31)
    {
        plaintext = (char *)aes_crypt((unsigned char *)&ciphertext[header_length], &len, salt, 0);

        if(plaintext != NULL)
        {
            guestsync_hexdump_ascii(GUESTSYNC_MSGDUMP, "RX data plaintext", (u8 *) plaintext, len);
            guestsyncPacketHandleCB(sock, from.sin_addr.s_addr, (uint8_t *)plaintext, len);
            free(plaintext);
        }
    }

}
/*****************************************************************
 * NAME:    guestsyncd_ctrl_data_init
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
int guestsyncd_ctrl_data_init(struct guestsyncd_data *syscd)
{
    int s = -1;

    s = listen_socket(htonl(INADDR_ANY), GUESTSYNC_UDP_PORT, guestsyncInterface());
    syscd->ctrl_sock = s;
    eloop_register_read_sock(s, guestsyncd_ctrl_data_receive, syscd, NULL);

    return 0;
}

/*****************************************************************
 * NAME:    guestsyncd_ctrl_iface_deinit
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  Yolin 2017-08-01
 * Modify:
 ******************************************************************/
void guestsyncd_ctrl_data_deinit(struct guestsyncd_data *syscd)
{
    if(syscd->ctrl_sock > -1)
    {
        eloop_unregister_read_sock(syscd->ctrl_sock);
        close(syscd->ctrl_sock);
        syscd->ctrl_sock = -1;
    }
}
