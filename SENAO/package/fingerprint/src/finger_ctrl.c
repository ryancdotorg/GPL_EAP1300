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
;       Yolin          2011-10-25
;*****************************************************************************/
/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#ifndef CONFIG_NATIVE_WINDOWS
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#endif /* CONFIG_NATIVE_WINDOWS */

#include "finger_ctrl.h"
#ifdef CONFIG_NATIVE_WINDOWS
#include <common.h>
#endif /* CONFIG_NATIVE_WINDOWS */


/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/

fingersync_ctrl * fingersync_ctrl_open(const char *ctrl_path)
{
    fingersync_ctrl *ctrl;
#ifndef CONFIG_CTRL_IFACE_UDP
    static int counter = 0;
#endif /* CONFIG_CTRL_IFACE_UDP */

    ctrl = malloc(sizeof(*ctrl));
    if (ctrl == NULL)
        return NULL;
    memset(ctrl, 0, sizeof(*ctrl));

#ifdef CONFIG_CTRL_IFACE_UDP
    ctrl->s = socket(PF_INET, SOCK_DGRAM, 0);
    if (ctrl->s < 0) {
        perror("socket");
        free(ctrl);
        return NULL;
    }

    ctrl->local.sin_family = AF_INET;
    ctrl->local.sin_addr.s_addr = htonl((127 << 24) | 1);
    if (bind(ctrl->s, (struct sockaddr *) &ctrl->local,
                sizeof(ctrl->local)) < 0) {
        close(ctrl->s);
        free(ctrl);
        return NULL;
    }

    ctrl->dest.sin_family = AF_INET;
    ctrl->dest.sin_addr.s_addr = htonl((127 << 24) | 1);
    ctrl->dest.sin_port = htons(SYSCONF_CTRL_IFACE_PORT);
    if (connect(ctrl->s, (struct sockaddr *) &ctrl->dest,
                sizeof(ctrl->dest)) < 0) {
        perror("connect");
        close(ctrl->s);
        free(ctrl);
        return NULL;
    }
#else /* CONFIG_CTRL_IFACE_UDP */
    ctrl->s = socket(PF_UNIX, SOCK_DGRAM, 0);
    if (ctrl->s < 0) {
        free(ctrl);
        return NULL;
    }

    ctrl->local.sun_family = AF_UNIX;
    snprintf(ctrl->local.sun_path, sizeof(ctrl->local.sun_path),
            "/var/fingersync_ctrl_%d-%d", getpid(), counter++);
    if (bind(ctrl->s, (struct sockaddr *) &ctrl->local,
                sizeof(ctrl->local)) < 0) {
        close(ctrl->s);
        free(ctrl);
        return NULL;
    }

    ctrl->dest.sun_family = AF_UNIX;
    snprintf(ctrl->dest.sun_path, sizeof(ctrl->dest.sun_path), "%s",
            ctrl_path);
    if (connect(ctrl->s, (struct sockaddr *) &ctrl->dest,
                sizeof(ctrl->dest)) < 0) {
        close(ctrl->s);
        unlink(ctrl->local.sun_path);
        free(ctrl);
        return NULL;
    }
#endif /* CONFIG_CTRL_IFACE_UDP */

    return ctrl;
}


void fingersync_ctrl_close(fingersync_ctrl *ctrl)
{
#ifndef CONFIG_CTRL_IFACE_UDP
    unlink(ctrl->local.sun_path);
#endif /* CONFIG_CTRL_IFACE_UDP */
    close(ctrl->s);
    free(ctrl);
}


int fingersync_ctrl_request(fingersync_ctrl *ctrl, const char *cmd, size_t cmd_len,
        char *reply, size_t *reply_len,
        void (*msg_cb)(char *msg, size_t len))
{
    struct timeval tv;
    int res;
    fd_set rfds;

    if (send(ctrl->s, cmd, cmd_len, 0) < 0)
        return -1;

    for (;;) {
        tv.tv_sec = 40; //cfho 2007-0402
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(ctrl->s, &rfds);
        res = select(ctrl->s + 1, &rfds, NULL, NULL, &tv);
        if (FD_ISSET(ctrl->s, &rfds)) {
            res = recv(ctrl->s, reply, *reply_len, 0);
            if (res < 0)
                return res;
            if (res > 0 && reply[0] == '<') {
                /* This is an unsolicited message from
                 * fingersyncd, not the reply to the
                 * request. Use msg_cb to report this to the
                 * caller. */
                if (msg_cb) {
                    /* Make sure the message is nul
                     * terminated. */
                    if ((size_t) res == *reply_len)
                        res = (*reply_len) - 1;
                    reply[res] = '\0';
                    msg_cb(reply, res);
                }
                continue;
            }
            *reply_len = res;
            break;
        } else {
            return -2;
        }
    }
    return 0;
}

int fingersync_ctrl_recv(fingersync_ctrl *ctrl, char *reply, size_t *reply_len)
{
    int res;

    res = recv(ctrl->s, reply, *reply_len, 0);
    if (res < 0)
        return res;
    *reply_len = res;
    return 0;
}


int fingersync_ctrl_pending(fingersync_ctrl *ctrl)
{
    struct timeval tv;
    int res;
    fd_set rfds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(ctrl->s, &rfds);
    res = select(ctrl->s + 1, &rfds, NULL, NULL, &tv);
    if (res < 0)
        return res;
    return FD_ISSET(ctrl->s, &rfds);
}


int fingersync_ctrl_get_fd(fingersync_ctrl *ctrl)
{
    return ctrl->s;
}
