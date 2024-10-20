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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "ctrl_iface.h"
#include "eloop.h"
#include "sockIntf.h"
#include "guestsync.h"
#include "guestsynchandle.h"
#include "aesapi.h"
#include <sys/time.h>
#include "common.h"
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/

#define STR_REPLY_OK              "ok\n"
#define STR_REPLY_ERROR           "error\n"
/*-------------------------------------------------------------------------*/
/*                           Parameter                                     */
/*-------------------------------------------------------------------------*/


#include "guestsync_list.h"
extern struct list_head maclistHead;

extern int guestsync_debug_level;
extern char guestsync_group[32];

static int guestsyncdCmd_send(char *cmd, char *reply);
static int guestsyncdCmd_list(char *cmd, char *reply);
static int guestsyncdCmd_system(char *cmd, char *reply);
static int guestsyncdCmd_help(char *cmd, char *reply);

static int guestsync_list_data_send_update_cb(char *cmd);
static int guestsync_list_data_send_logout_cb(char *cmd);
static int guestsync_list_data_send_gone_cb(char *cmd);
static int guestsync_list_all_cb(char *cmd);

static int guestsync_system_debug_level_cb(char *cmd);


/*-------------------------------------------------------------------------*/
/*                           structure                                     */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/*                           CMD Table                                     */
/*-------------------------------------------------------------------------*/
struct guestsyncdCmdCB guestsync_send_cb[] = {
    {"update",          guestsync_list_data_send_update_cb,       "Update request"},
    {"logout",          guestsync_list_data_send_logout_cb,        "Logout request"},
    {"gone",            guestsync_list_data_send_gone_cb,          "Gone request"},
    {NULL,              NULL},
};


struct guestsyncdCmdCB guestsync_list_cb[] = {
    {"all",             guestsync_list_all_cb,                   "Show All Requst List"},
    {NULL,              NULL},
};

struct guestsyncdCmdCB guestsync_system_cb[] = {
    {"debugLevel",      guestsync_system_debug_level_cb,                "Debug Level"},
    {NULL,              NULL},
};

struct guestsyncdCmd guestsyncd_commands[] = {
    {"send",       guestsyncdCmd_send,       guestsync_send_cb,         "Send Commands Table"},
    {"forcesend",  guestsyncdCmd_send,       guestsync_send_cb,         "Send Commands Table"},
    {"list",       guestsyncdCmd_list,       guestsync_list_cb,         "List Guest Request"},
    {"system",     guestsyncdCmd_system,     guestsync_system_cb,       "System Commands"},
    {"help",       guestsyncdCmd_help,       NULL,                     "Help"},
};

int GUESTSYNC_CMDSIZE = T_NUM_OF_ELEMENTS(guestsyncd_commands);

/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/
/*****************************************************************
* NAME:    gen_random
* ---------------------------------------------------------------
* FUNCTION: gen_random
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int gen_random(char *output, int size)
{
    FILE *fp;
    int randno, i;
    const char valid_char[62]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    // shell: head /dev/urandom | tr -dc A-Za-z0-9 | head -c 13 ; echo ''

    fp = fopen("/dev/urandom", "r");
    if (fp < 0)
    {
        guestsync_printf(GUESTSYNC_DEBUG, "/dev/urandom not exist");
        return -1;
    }
    else
    {
        for(i=0; i<size; ++i)
        {
            randno = fgetc(fp);
            output[i] = valid_char[randno % sizeof(valid_char)];
        }
    }

    fclose(fp);

    return 0;
}
/*****************************************************************
* NAME:    guestsyncdCmdHandle
* ---------------------------------------------------------------
* FUNCTION: Handle guestsync_cli commands
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
int guestsyncdCmdHandle(char* cmd, char* reply)
{
    char bufCmd[80];
    char bufArg[80];
    int i, j;

    memset(bufCmd, 0, sizeof(bufCmd));
    memset(bufArg, 0, sizeof(bufArg));
    //printf("%s receives event [%s]\n",__FUNCTION__, cmd);
    sscanf(cmd, "%s %s",bufCmd, bufArg);
    //printf( "cmd [%s] [%s]\n",bufCmd, bufArg);

    for(i=0; i<GUESTSYNC_CMDSIZE; ++i)
    {
        if(strcmp(bufCmd, guestsyncd_commands[i].cmdName) == 0)
        {
            if(guestsyncd_commands[i].subCmd && bufArg[0] != '\0')
            {
                for(j=0; guestsyncd_commands[i].subCmd[j].cmdName != NULL; ++j)
                {
                    if(strcmp(bufArg, guestsyncd_commands[i].subCmd[j].cmdName) == 0)
                    {
                        guestsyncd_commands[i].subCmd[j].handler(cmd);
                        memcpy(reply, "OK\n", 3);
                        return 3;
                    }
                }
            }
            return guestsyncd_commands[i].handler(cmd, reply);
        }
    }

    memcpy(reply, "UNKNOWN COMMAND\n", 16);
    return 16;
}
/*****************************************************************
* NAME:    guestsyncdCmdShowHelp
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int guestsyncdCmdShowHelp(int isShowAll, char *cmdName)
{
    int i,j;

    guestsync_printf(GUESTSYNC_SHOW, "%20s\t\t\t%s\n", "Command", "Description");
    guestsync_printf(GUESTSYNC_SHOW, "=======================================================\n");

    for(i=0;i<GUESTSYNC_CMDSIZE ;i++)
    {
        if(isShowAll == 0 && strcmp(guestsyncd_commands[i].cmdName, cmdName) != 0)
        {
            continue;
        }
        guestsync_printf(GUESTSYNC_SHOW, "\t* %-24s %s\n", guestsyncd_commands[i].cmdName, guestsyncd_commands[i].description);

        if(guestsyncd_commands[i].subCmd)
        {
            for(j=0; guestsyncd_commands[i].subCmd[j].cmdName != NULL; ++j)
            {
                guestsync_printf(GUESTSYNC_SHOW, "\t  - %-24s %s\n",
                        guestsyncd_commands[i].subCmd[j].cmdName,
                        guestsyncd_commands[i].subCmd[j].description);
            }
        }
        guestsync_printf(GUESTSYNC_SHOW, "\n");
    }

    return 1;
}
/*****************************************************************
* NAME:    guestsyncdCmd_help
* ---------------------------------------------------------------
* FUNCTION: Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int guestsyncdCmd_help(char *cmd, char *reply)
{
    int i;
    char bufCmd[80];
    char bufArg[80];
    int ret;

    memset(bufCmd, 0, sizeof(bufCmd));
    memset(bufArg, 0, sizeof(bufArg));
    //printf("%s receives event [%s]\n",__FUNCTION__, cmd);
    ret = sscanf(cmd, "%s %s",bufCmd, bufArg);
    //printf( "cmd [%s] [%s]\n",bufCmd, bufArg);

    if(ret == 1)
    {
        guestsyncdCmdShowHelp(1, "");
        strcpy(reply, "ok\n");
        return 3;
    }
    else if(ret == 2)
    {
        for(i=0;i<GUESTSYNC_CMDSIZE ;i++)
        {
            if(strcmp(guestsyncd_commands[i].cmdName, bufArg) == 0)
            {
                guestsyncdCmdShowHelp(0, bufArg);
                strcpy(reply, "ok\n");
                return 3;
            }
        }
    }

    memcpy(reply, "UNKNOWN COMMAND\n", 16);
    return 16;
}
/*****************************************************************
* NAME:    guestsyncdCmd_send
* ---------------------------------------------------------------
* FUNCTION: Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int guestsyncdCmd_send(char *cmd, char *reply)
{
    char bufCmd[80];
    char bufArg[80];

    memset(bufCmd, 0, sizeof(bufCmd));
    memset(bufArg, 0, sizeof(bufArg));

    //printf("%s receives event [%s]\n",__FUNCTION__, cmd);
    sscanf(cmd, "%s %s",bufCmd, bufArg);
    //printf( "cmd [%s] [%s]\n",bufCmd, bufArg);

    if(bufArg[0] == '\0')
    {
        guestsyncdCmdShowHelp(0, bufCmd);
        strcpy(reply, "ok\n");
        return 3;
    }

    memcpy(reply, "UNKNOWN COMMAND\n", 16);
    return 16;
}
/*****************************************************************
* NAME:    guestsyncdCmd_list
* ---------------------------------------------------------------
* FUNCTION: Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int guestsyncdCmd_list(char *cmd, char *reply)
{
    char bufCmd[80];
    char bufArg[80];

    memset(bufCmd, 0, sizeof(bufCmd));
    memset(bufArg, 0, sizeof(bufArg));

    //printf("%s receives event [%s]\n",__FUNCTION__, cmd);
    sscanf(cmd, "%s %s",bufCmd, bufArg);
    //printf( "cmd [%s] [%s]\n",bufCmd, bufArg);

    if(bufArg[0] == '\0')
    {
        guestsyncdCmdShowHelp(0, bufCmd);
        strcpy(reply, "ok\n");
        return 3;
    }

    memcpy(reply, "UNKNOWN COMMAND\n", 16);
    return 16;
}
/*****************************************************************
* NAME:    guestsyncdCmd_system
* ---------------------------------------------------------------
* FUNCTION: Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int guestsyncdCmd_system(char *cmd, char *reply)
{
    char bufCmd[80];
    char bufArg[80];

    memset(bufCmd, 0, sizeof(bufCmd));
    memset(bufArg, 0, sizeof(bufArg));

    //printf("%s receives event [%s]\n",__FUNCTION__, cmd);
    sscanf(cmd, "%s %s",bufCmd, bufArg);
    //printf( "cmd [%s] [%s]\n",bufCmd, bufArg);

    if(bufArg[0] == '\0')
    {
        guestsyncdCmdShowHelp(0, bufCmd);
        strcpy(reply, "ok\n");
        return 3;
    }

    memcpy(reply, "UNKNOWN COMMAND\n", 16);
    return 16;
}

/*****************************************************************
                        Sub Command
******************************************************************/

/*****************************************************************
* NAME:    guestsync_debug_level_cb
* ---------------------------------------------------------------
* FUNCTION: Sub Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int guestsync_system_debug_level_cb(char *cmd)
{
    int lvl, ret;

    ret = sscanf(cmd, "%*s %*s %d", &lvl);

    if(ret != 1)
    {
        guestsync_printf(GUESTSYNC_SHOW, "Debug Level: %d\n", guestsync_debug_level);
    }
    else
    {
        if(lvl >= GUESTSYNC_MSGDUMP && lvl <= GUESTSYNC_SHOW)
        {
#ifdef CONFIG_NO_STDOUT_DEBUG
            /* no debug log */
#else
            guestsync_debug_level = lvl;
#endif
        }
    }

    return 0;
}
/*****************************************************************
* NAME:    sync_data_delete_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
void sync_data_delete_cb(void *eloop_ctx, void *timeout_ctx)
{
	int ret, i;
	struct guest_cfg_list *sync_data = (struct guest_cfg_list *)eloop_ctx;

    printf("delete =>");
    printf("type:%d ", sync_data->data.type);
    printf("random:");
    for(i=0; i<16; ++i)
        printf("%c", sync_data->data.random[i]);
    printf(" ");
    printf("dhcpif:%s ", sync_data->data.dhcpif);
    printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\n", \
            sync_data->data.mac[0], sync_data->data.mac[1], sync_data->data.mac[2], \
            sync_data->data.mac[3], sync_data->data.mac[4], sync_data->data.mac[5]);

	ret = guest_list_delete(sync_data->data.mac, sync_data->data.dhcpif, sync_data->data.random, &maclistHead);
    if(!ret)
        printf("delete failure");
}
/*****************************************************************
* NAME:    guestsync_list_data_send_update_cb
* ---------------------------------------------------------------
* FUNCTION: Sub Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int guestsync_list_data_send_update_cb(char *cmd)
{
    int ret, i;
    int bytes;
    int maci[6];
    u8 mac[6];
    char dhcpif[16] = {0};
    char list_dhcpif[16] = {0};
    char rets[64] = {0};
    int ssid_num = 1, is_forcesend = 0, cloud_index = 1;
    char random[16];
    struct guest_cfg_list *pguest_list = NULL;
    struct guest_cfg_list guest_list_node;
    char forcesend[16] = {0};
    unsigned int sessiontimeout = 0, sessiontime = 0;
    unsigned int idletimeout = 0, idletime = 0;
    unsigned int gonetime = 0;
    char username[256] = {0};
    char ip_str[32] = {0};
    struct in_addr node_ip_addr;
    struct timeval tv;
    int interiminterval = 0;
    unsigned int input_packets = 0, output_packets = 0;
    unsigned int input_octets = 0, output_octets = 0;
    char sessionid[32] = {0};

    ret = sscanf(cmd, "%s %*s "MAC_PATTERN" %s %s %u %u %u %u %u %u %u %u %u %u %s %255[\001-\377]",
            forcesend, &maci[0], &maci[1], &maci[2], &maci[3], &maci[4], &maci[5],
            dhcpif, ip_str, &sessiontime, &sessiontimeout,
            &idletime, &idletimeout, &gonetime,
            &input_packets, &output_packets,
            &input_octets, &output_octets,
            &interiminterval, sessionid, username);

    if(ret < 21)
    {
        guestsync_printf(GUESTSYNC_SHOW, "send update 00AABBCCDD11 br-ssidx\n");
        guestsync_printf(GUESTSYNC_SHOW, "ip sessiontime sessiontimeout idletime idletimeout gonetime input_pkts output_pkts input_octets output_octets interiminterval sessionid username\n");
        return 0;
    }

    mac[0] = maci[0]&0xff;
    mac[1] = maci[1]&0xff;
    mac[2] = maci[2]&0xff;
    mac[3] = maci[3]&0xff;
    mac[4] = maci[4]&0xff;
    mac[5] = maci[5]&0xff;

    if(strlen(ip_str))
    {
        inet_pton(AF_INET, ip_str, &node_ip_addr);
    }

    if(strcmp(forcesend, "forcesend") == 0)
        is_forcesend = 1;

    sscanf(dhcpif, "br-ssid%d", &ssid_num);
    cloud_index = get_cloud_index(dhcpif);
    snprintf(list_dhcpif, sizeof(list_dhcpif), "br-ssid%d", cloud_index);

    pguest_list = guest_list_find(mac, list_dhcpif, NULL, &maclistHead);

    if(!pguest_list || is_forcesend)
    {
        if(gen_random(random, sizeof(random)) < 0)
        {
            guestsync_printf(GUESTSYNC_DEBUG, "cannot gen random");
            srand((unsigned int) time(NULL));
            for (i=0; i<16; i++)
            {
                random[i] = rand();
            }
        }

        guest_list_init_node(&guest_list_node);

        guest_list_node.data.type = SYNC_TYPE_UPDATE;

        memcpy(guest_list_node.data.random, random, 16);
        memcpy(guest_list_node.data.mac, mac, 6);

        memset(guest_list_node.data.group, 0, 32);
        memcpy(guest_list_node.data.group, guestsync_group, 32);

        strncpy(guest_list_node.data.dhcpif, list_dhcpif, sizeof(guest_list_node.data.dhcpif));

        memset(rets, 0, sizeof(rets));
        if(my_system(rets, sizeof(rets), "printf `uci get wireless.wifi0_ssid_%d.ssid`", ssid_num) < 0)
        {
            guestsync_printf(GUESTSYNC_ERROR, "get ssid command error\n");
            return 0;
        }
        memcpy(guest_list_node.data.ssid, rets, 32);
        guestsync_printf(GUESTSYNC_SHOW, "my SSID[%.32s]\n", guest_list_node.data.ssid);
        guestsync_printf(GUESTSYNC_SHOW, "my Group[%.32s]\n", guest_list_node.data.group);

        gettimeofday(&tv, NULL);
        guest_list_node.data.expiretime = htonl(tv.tv_sec + GUESTSYNC_REQUEST_EXPIRETIME);
        guest_list_node.data.nodeinfo.sessiontime = htonl(sessiontime);
        guest_list_node.data.nodeinfo.sessiontimeout = htonl(sessiontimeout);
        guest_list_node.data.nodeinfo.idletime = htonl(idletime);
        guest_list_node.data.nodeinfo.idletimeout = htonl(idletimeout);
        guest_list_node.data.nodeinfo.gonetime = htonl(gonetime);
        guest_list_node.data.nodeinfo.interiminterval = htonl(interiminterval);
        guest_list_node.data.nodeinfo.input_packets = htonl(input_packets);
        guest_list_node.data.nodeinfo.output_packets = htonl(output_packets);
        guest_list_node.data.nodeinfo.input_octets = htonl(input_octets);
        guest_list_node.data.nodeinfo.output_octets = htonl(output_octets);
        memcpy(guest_list_node.data.nodeinfo.sessionid, sessionid, sizeof(guest_list_node.data.nodeinfo.sessionid));
        memcpy(guest_list_node.data.nodeinfo.username, username, sizeof(guest_list_node.data.nodeinfo.username));
        guest_list_node.data.nodeinfo.ipaddr = node_ip_addr.s_addr;
        guest_list_node.data.length = 0;

        pguest_list = guest_list_add_node(&guest_list_node, &maclistHead);
        eloop_register_timeout(3, 0, sync_data_delete_cb, pguest_list, 0);
        bytes = udp_packet_enc((uint8_t *)&guest_list_node.data, sizeof(guest_list_node.data), inet_addr("255.255.255.255"), GUESTSYNC_UDP_PORT);
        printf("sent %d bytes to 255.255.255.255\n", bytes);
    }
    else
    {
        printf("[%s] don't send %02x:%02x:%02x:%02x:%02x:%02x too fast, wait 3 secs...\n",
                list_dhcpif, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    guest_list_display(&maclistHead);

    return 0;
}
/*****************************************************************
* NAME:    guestsync_list_data_send_logout_cb
* ---------------------------------------------------------------
* FUNCTION: Sub Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int guestsync_list_data_send_logout_cb(char *cmd)
{
    int ret, i;
    int bytes;
    int maci[6];
    u8 mac[6];
    char dhcpif[16] = {0};
    char list_dhcpif[16] = {0};
    char rets[64] = {0};
    int ssid_num = 1, is_forcesend = 0, cloud_index = 1;
    char random[16];
    struct guest_cfg_list *pguest_list = NULL;
    struct guest_cfg_list guest_list_node;
    char forcesend[16] = {0};
    struct timeval tv;

    ret = sscanf(cmd, "%s %*s "MAC_PATTERN" %s",
            forcesend, &maci[0], &maci[1], &maci[2], &maci[3], &maci[4], &maci[5],
            dhcpif);

    if(ret < 8)
    {
        guestsync_printf(GUESTSYNC_SHOW, "send logout 00AABBCCDD11 br-ssidx \n");
        return 0;
    }

    mac[0] = maci[0]&0xff;
    mac[1] = maci[1]&0xff;
    mac[2] = maci[2]&0xff;
    mac[3] = maci[3]&0xff;
    mac[4] = maci[4]&0xff;
    mac[5] = maci[5]&0xff;

    if(strcmp(forcesend, "forcesend") == 0)
        is_forcesend = 1;

    sscanf(dhcpif, "br-ssid%d", &ssid_num);
    cloud_index = get_cloud_index(dhcpif);
    snprintf(list_dhcpif, sizeof(list_dhcpif), "br-ssid%d", cloud_index);

    pguest_list = guest_list_find(mac, list_dhcpif, NULL, &maclistHead);

    if(!pguest_list || is_forcesend)
    {
        if(gen_random(random, sizeof(random)) < 0)
        {
            guestsync_printf(GUESTSYNC_DEBUG, "cannot gen random");
            srand((unsigned int) time(NULL));
            for (i=0; i<16; i++)
            {
                random[i] = rand();
            }
        }

        guest_list_init_node(&guest_list_node);

        guest_list_node.data.type = SYNC_TYPE_LOGOUT;

        memcpy(guest_list_node.data.random, random, 16);
        memcpy(guest_list_node.data.mac, mac, 6);

        memset(guest_list_node.data.group, 0, 32);
        memcpy(guest_list_node.data.group, guestsync_group, 32);

        strncpy(guest_list_node.data.dhcpif, list_dhcpif, sizeof(guest_list_node.data.dhcpif));

        memset(rets, 0, sizeof(rets));
        if(my_system(rets, sizeof(rets), "printf `uci get wireless.wifi0_ssid_%d.ssid`", ssid_num) < 0)
        {
            guestsync_printf(GUESTSYNC_ERROR, "get ssid command error\n");
            return 0;
        }
        memcpy(guest_list_node.data.ssid, rets, 32);
        guestsync_printf(GUESTSYNC_SHOW, "my SSID[%.32s]\n", guest_list_node.data.ssid);
        guestsync_printf(GUESTSYNC_SHOW, "my Group[%.32s]\n", guest_list_node.data.group);

        gettimeofday(&tv, NULL);
        guest_list_node.data.expiretime = htonl(tv.tv_sec + GUESTSYNC_REQUEST_EXPIRETIME);
        guest_list_node.data.length = 0;

        pguest_list = guest_list_add_node(&guest_list_node, &maclistHead);
        eloop_register_timeout(3, 0, sync_data_delete_cb, pguest_list, 0);
        bytes = udp_packet_enc((uint8_t *)&guest_list_node.data, sizeof(guest_list_node.data), inet_addr("255.255.255.255"), GUESTSYNC_UDP_PORT);
        printf("sent %d bytes to 255.255.255.255\n", bytes);
    }
    else
    {
        printf("[%s] don't send %02x:%02x:%02x:%02x:%02x:%02x too fast, wait 3 secs...\n",
                list_dhcpif, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    guest_list_display(&maclistHead);

    return 0;
}
/*****************************************************************
* NAME:    guestsync_list_data_send_gone_cb
* ---------------------------------------------------------------
* FUNCTION: Sub Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int guestsync_list_data_send_gone_cb(char *cmd)
{
    int ret, i;
    int bytes;
    int maci[6];
    u8 mac[6];
    char dhcpif[16] = {0};
    char list_dhcpif[16] = {0};
    char rets[64] = {0};
    int ssid_num = 1, is_forcesend = 0, cloud_index = 1;
    char random[16];
    struct guest_cfg_list *pguest_list = NULL;
    struct guest_cfg_list guest_list_node;
    char forcesend[16] = {0};
    unsigned int gonetime = 0;
    unsigned int idletime = 0;
    struct timeval tv;

    ret = sscanf(cmd, "%s %*s "MAC_PATTERN" %s %u %u",
            forcesend, &maci[0], &maci[1], &maci[2], &maci[3], &maci[4], &maci[5],
            dhcpif, &gonetime, &idletime);

    if(ret < 9)
    {
        guestsync_printf(GUESTSYNC_SHOW, "send gone 00AABBCCDD11 br-ssidx gonetime idletime\n");
        return 0;
    }

    mac[0] = maci[0]&0xff;
    mac[1] = maci[1]&0xff;
    mac[2] = maci[2]&0xff;
    mac[3] = maci[3]&0xff;
    mac[4] = maci[4]&0xff;
    mac[5] = maci[5]&0xff;

    if(strcmp(forcesend, "forcesend") == 0)
        is_forcesend = 1;

    sscanf(dhcpif, "br-ssid%d", &ssid_num);
    cloud_index = get_cloud_index(dhcpif);
    snprintf(list_dhcpif, sizeof(list_dhcpif), "br-ssid%d", cloud_index);

    pguest_list = guest_list_find(mac, list_dhcpif, NULL, &maclistHead);

    if(!pguest_list || is_forcesend)
    {
        if(gen_random(random, sizeof(random)) < 0)
        {
            guestsync_printf(GUESTSYNC_DEBUG, "cannot gen random");
            srand((unsigned int) time(NULL));
            for (i=0; i<16; i++)
            {
                random[i] = rand();
            }
        }

        guest_list_init_node(&guest_list_node);

        guest_list_node.data.type = SYNC_TYPE_GONE;

        memcpy(guest_list_node.data.random, random, 16);
        memcpy(guest_list_node.data.mac, mac, 6);

        memset(guest_list_node.data.group, 0, 32);
        memcpy(guest_list_node.data.group, guestsync_group, 32);

        strncpy(guest_list_node.data.dhcpif, list_dhcpif, sizeof(guest_list_node.data.dhcpif));

        memset(rets, 0, sizeof(rets));
        if(my_system(rets, sizeof(rets), "printf `uci get wireless.wifi0_ssid_%d.ssid`", ssid_num) < 0)
        {
            guestsync_printf(GUESTSYNC_ERROR, "get ssid command error\n");
            return 0;
        }
        memcpy(guest_list_node.data.ssid, rets, 32);
        guestsync_printf(GUESTSYNC_SHOW, "my SSID[%.32s]\n", guest_list_node.data.ssid);
        guestsync_printf(GUESTSYNC_SHOW, "my Group[%.32s]\n", guest_list_node.data.group);

        gettimeofday(&tv, NULL);
        guest_list_node.data.expiretime = htonl(tv.tv_sec + GUESTSYNC_REQUEST_EXPIRETIME);
        guest_list_node.data.nodeinfo.gonetime = htonl(gonetime);
        guest_list_node.data.nodeinfo.idletime = htonl(idletime);
        guest_list_node.data.length = 0;

        pguest_list = guest_list_add_node(&guest_list_node, &maclistHead);
        eloop_register_timeout(3, 0, sync_data_delete_cb, pguest_list, 0);
        bytes = udp_packet_enc((uint8_t *)&guest_list_node.data, sizeof(guest_list_node.data), inet_addr("255.255.255.255"), GUESTSYNC_UDP_PORT);
        printf("sent %d bytes to 255.255.255.255\n", bytes);
    }
    else
    {
        printf("[%s] don't send %02x:%02x:%02x:%02x:%02x:%02x too fast, wait 3 secs...\n",
                list_dhcpif, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

    guest_list_display(&maclistHead);

    return 0;
}
/*****************************************************************
* NAME:    guestsync_list_all_cb
* ---------------------------------------------------------------
* FUNCTION: Sub Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int guestsync_list_all_cb(char *cmd)
{
    guestsync_printf(GUESTSYNC_SHOW,
       "===================================================================================\n");
    guest_list_display(&maclistHead);

    return 0;
}

