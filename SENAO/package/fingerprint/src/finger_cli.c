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
#include <signal.h>
#include <unistd.h>
#include <dirent.h>
#include <assert.h>
#include "finger.h"
#include "fingerd.h"
#include "finger_ctrl.h"

/*-------------------------------------------------------------------------*/
/*                           Parameter                                     */
/*-------------------------------------------------------------------------*/

static fingersync_ctrl *ctrl_conn;
static const char *ctrl_iface_dir = FINGERD_CTRL_INTERFACE;
static char *ctrl_ifname = NULL;

/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/
static fingersync_ctrl * fingersync_cli_open_connection(const char *ifname)
{
    char *cfile;
    int flen;

    /* printf("---> fingersync_cli_open_connection  ifname [%s]\n",ifname); */
    if(ifname == NULL)
        return NULL;

    flen = strlen(ctrl_iface_dir) + strlen(ifname) + 2;
    cfile = malloc(flen);
    if(cfile == NULL)
        return NULL;
    snprintf(cfile, flen, "%s/%s", ctrl_iface_dir, ifname);

    ctrl_conn = fingersync_ctrl_open(cfile);
    free(cfile);
    return ctrl_conn;
}

static void fingersync_cli_close_connection(void)
{
    if (ctrl_conn == NULL)
        return;

    fingersync_ctrl_close(ctrl_conn);
    ctrl_conn = NULL;
}

static void fingersync_cli_msg_cb(char *msg, size_t len)
{
    printf("%s\n", msg);
}

static int _fingersync_ctrl_command(fingersync_ctrl *ctrl, char *cmd, int print)
{
    char buf[4096];
    size_t len;
    int ret;

    if(ctrl_conn == NULL)
    {
        printf("Not connected to %s - command dropped.\n", FINGERSYNC);
        return -1;
    }
    len = sizeof(buf) - 1;
    ret = fingersync_ctrl_request(ctrl, cmd, strlen(cmd), buf, &len,
            fingersync_cli_msg_cb);
    if(ret == -2)
    {
        printf("'%s' command timed out.\n", cmd);
        return -2;
    }
    else if(ret < 0)
    {
        printf("'%s' command failed.\n", cmd);
        return -1;
    }
    if(print)
    {
        buf[len] = '\0';
        printf("==>%s", buf);
    }
    return 0;
}

static inline int fingersync_ctrl_command(fingersync_ctrl *ctrl, char *cmd)
{
    return _fingersync_ctrl_command(ctrl, cmd, 1);
}

static void fingersync_request(fingersync_ctrl *ctrl, int argc, char *argv[])
{
    int i, val;
    char buf[1024];

    val = sprintf(buf, "%s", argv[0]);

    for(i=1; i<argc; ++i)
    {
        val += sprintf(buf+val, " %s", argv[i]);
    }

    fingersync_ctrl_command(ctrl, buf);

    return;
}

int main(int argc, char *argv[])
{
    int interactive;
    int c;

    for(;;)
    {
        c = getopt(argc, argv, "i:p:");
        if(c < 0)
            break;
        switch(c)
        {
            case 'i':
                ctrl_ifname = strdup(optarg);
                break;
            case 'p':
                ctrl_iface_dir = optarg;
                break;
            default:
                return -1;
        }
    }

    interactive = argc == optind;

    if(ctrl_ifname == NULL)
    {
        struct dirent *dent;
        DIR *dir = opendir(ctrl_iface_dir);
        if(dir)
        {
            while((dent = readdir(dir)))
            {
                if(strcmp(dent->d_name, ".") == 0 ||
                        strcmp(dent->d_name, "..") == 0)
                    continue;
                printf("Selected interface '%s'\n",  dent->d_name);
                ctrl_ifname = strdup(dent->d_name);
                break;
            }
            closedir(dir);
        }
    }
    else
    {
        printf("Selected interface '%s'\n",  ctrl_ifname);
    }
    ctrl_conn = fingersync_cli_open_connection(FINGERD_CTRL_INTERFACE_IF);

    if(!ctrl_conn)
    {
        perror("Failed to connect to " FINGERSYNC "- fingersync_ctrl_open");
        free(ctrl_ifname);
        return -1;
    }

    if(!interactive)
    {
        fingersync_request(ctrl_conn, argc - optind, &argv[optind]);
    }

    free(ctrl_ifname);
    fingersync_cli_close_connection();
    return 0;
}
