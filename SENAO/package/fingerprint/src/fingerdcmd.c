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
#include "finger.h"
#include "fingerhandle.h"
#include "fingerprint.h"
#include "aesapi.h"
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/

#define STR_REPLY_OK              "ok\n"
#define STR_REPLY_ERROR           "error\n"
/*-------------------------------------------------------------------------*/
/*                           Parameter                                     */
/*-------------------------------------------------------------------------*/

#include "finger_list.h"
extern struct list_head maclistHead;
extern struct list_head iplistHead;
extern int fingersync_debug_level;
extern int DebugEnable;
extern int node_count;
extern int ip_node_count;

static int fingersyncdCmd_send(char *cmd, char *reply);
static int fingersyncdCmd_set(char *cmd, char *reply);
static int fingersyncdCmd_system(char *cmd, char *reply);
static int fingersyncdCmd_help(char *cmd, char *reply);

static int fingersync_list_data_send_request_cb(char *cmd);
static int fingersync_list_data_set_data_cb(char *cmd);
static int fingersync_list_data_set_vlan_info_cb(char *cmd);
static int fingersync_system_debug_level_cb(char *cmd);

/*-------------------------------------------------------------------------*/
/*                           CMD Table                                     */
/*-------------------------------------------------------------------------*/
struct fingersyncdCmdCB fingersync_send_cb[] = {
    {"request",         fingersync_list_data_send_request_cb,                  "Broadcast request"},
    {NULL,              NULL},
};

struct fingersyncdCmdCB fingersync_set_cb[] = {
    {"data",            fingersync_list_data_set_data_cb,                  "Change data"},
    {"vlanInfo",        fingersync_list_data_set_vlan_info_cb,             "Save vlan info"},
    {NULL,              NULL},
};

struct fingersyncdCmdCB fingersync_system_cb[] = {
    {"debugLevel",      fingersync_system_debug_level_cb,                "Debug Level"},
    {NULL,              NULL},
};

struct fingersyncdCmd fingersyncd_commands[] = {
    {"send",       fingersyncdCmd_send,       fingersync_send_cb,         "Send Commands Table"},
    {"set",        fingersyncdCmd_set,        fingersync_set_cb,          "Data Commands"},
    {"system",     fingersyncdCmd_system,     fingersync_system_cb,       "System Commands"},
    {"help",       fingersyncdCmd_help,       NULL,                     "Help"},
};

#define T_NUM_OF_ELEMENTS(x) (sizeof(x)/sizeof(x[0]))
int FINGER_CMDSIZE = T_NUM_OF_ELEMENTS(fingersyncd_commands);

/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/
/*****************************************************************
* NAME:    fingersyncdCmdHandle
* ---------------------------------------------------------------
* FUNCTION: Handle fingersync_cli commands
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
int fingersyncdCmdHandle(char* cmd, char* reply)
{
    char bufCmd[80];
    char bufArg[80];
    int i, j;

    memset(bufCmd, 0, sizeof(bufCmd));
    memset(bufArg, 0, sizeof(bufArg));
    //printf("%s receives event [%s]\n",__FUNCTION__, cmd);
    sscanf(cmd, "%s %s",bufCmd, bufArg);
    //printf( "cmd [%s] [%s]\n",bufCmd, bufArg);

    for(i=0; i<FINGER_CMDSIZE; ++i)
    {
        if(strcmp(bufCmd, fingersyncd_commands[i].cmdName) == 0)
        {
            if(fingersyncd_commands[i].subCmd && bufArg[0] != '\0')
            {
                for(j=0; fingersyncd_commands[i].subCmd[j].cmdName != NULL; ++j)
                {
                    if(strcmp(bufArg, fingersyncd_commands[i].subCmd[j].cmdName) == 0)
                    {
                        fingersyncd_commands[i].subCmd[j].handler(cmd);
                        memcpy(reply, "OK\n", 3);
                        return 3;
                    }
                }
            }
            return fingersyncd_commands[i].handler(cmd, reply);
        }
    }

    memcpy(reply, "UNKNOWN COMMAND\n", 16);
    return 16;
}
/*****************************************************************
* NAME:    fingersyncdCmdShowHelp
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int fingersyncdCmdShowHelp(int isShowAll, char *cmdName)
{
    int i,j;

    fingersync_printf(FINGER_SHOW, "%20s\t\t\t%s\n", "Command", "Description");
    fingersync_printf(FINGER_SHOW, "=======================================================\n");

    for(i=0;i<FINGER_CMDSIZE ;i++)
    {
        if(isShowAll == 0 && strcmp(fingersyncd_commands[i].cmdName, cmdName) != 0)
        {
            continue;
        }
        fingersync_printf(FINGER_SHOW, "\t* %-24s %s\n", fingersyncd_commands[i].cmdName, fingersyncd_commands[i].description);

        if(fingersyncd_commands[i].subCmd)
        {
            for(j=0; fingersyncd_commands[i].subCmd[j].cmdName != NULL; ++j)
            {
                fingersync_printf(FINGER_SHOW, "\t  - %-24s %s\n",
                        fingersyncd_commands[i].subCmd[j].cmdName,
                        fingersyncd_commands[i].subCmd[j].description);
            }
        }
        fingersync_printf(FINGER_SHOW, "\n");
    }

    return 1;
}
/*****************************************************************
* NAME:    fingersyncdCmd_help
* ---------------------------------------------------------------
* FUNCTION: Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int fingersyncdCmd_help(char *cmd, char *reply)
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
        fingersyncdCmdShowHelp(1, "");
        strcpy(reply, "ok\n");
        return 3;
    }
    else if(ret == 2)
    {
        for(i=0;i<FINGER_CMDSIZE ;i++)
        {
            if(strcmp(fingersyncd_commands[i].cmdName, bufArg) == 0)
            {
                fingersyncdCmdShowHelp(0, bufArg);
                strcpy(reply, "ok\n");
                return 3;
            }
        }
    }

    memcpy(reply, "UNKNOWN COMMAND\n", 16);
    return 16;
}
/*****************************************************************
* NAME:    fingersyncdCmd_send
* ---------------------------------------------------------------
* FUNCTION: Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int fingersyncdCmd_send(char *cmd, char *reply)
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
        fingersyncdCmdShowHelp(0, bufCmd);
        strcpy(reply, "ok\n");
        return 3;
    }

    memcpy(reply, "UNKNOWN COMMAND\n", 16);
    return 16;
}

static int fingersyncdCmd_set(char *cmd, char *reply)
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
        fingersyncdCmdShowHelp(0, bufCmd);
        strcpy(reply, "ok\n");
        return 3;
    }

    memcpy(reply, "UNKNOWN COMMAND\n", 16);
    return 16;
}
/*****************************************************************
* NAME:    fingersyncdCmd_system
* ---------------------------------------------------------------
* FUNCTION: Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int fingersyncdCmd_system(char *cmd, char *reply)
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
        fingersyncdCmdShowHelp(0, bufCmd);
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
* NAME:    fingersync_debug_level_cb
* ---------------------------------------------------------------
* FUNCTION: Sub Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int fingersync_system_debug_level_cb(char *cmd)
{
    int lvl, ret;

    ret = sscanf(cmd, "%*s %*s %d", &lvl);

    if(ret != 1)
    {
        fingersync_printf(FINGER_SHOW, "Debug Level: %d\n", fingersync_debug_level);
    }
    else
    {
        if(lvl >= FINGER_MSGDUMP && lvl <= FINGER_SHOW)
        {
#ifdef CONFIG_NO_STDOUT_DEBUG
            /* no debug log */
#else
            fingersync_debug_level = lvl;
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
    int ret;
    struct finger_cfg_list *sync_data = (struct finger_cfg_list *)eloop_ctx;

    printf("delete =>");
    printf("mac:%02x:%02x:%02x:%02x:%02x:%02x\n", \
            sync_data->data.mac[0], sync_data->data.mac[1], sync_data->data.mac[2], \
            sync_data->data.mac[3], sync_data->data.mac[4], sync_data->data.mac[5]);

    ret = finger_list_delete(sync_data->data.mac, &maclistHead);
    if(!ret)
        fp_debug("delete failure");
}

/*****************************************************************
* NAME:    fingersync_list_data_send_request_cb
* ---------------------------------------------------------------
* FUNCTION: Sub Command
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
static int fingersync_list_data_send_request_cb(char *cmd)
{
    int ret;
    int bytes;
    int maci[6];
    u8 mac[6];
    struct finger_cfg_list finger_list_node;
    struct finger_cfg_list *pfinger_list = NULL;
    struct ip_cfg_list ip_list_node;
    struct ip_cfg_list *pip_list = NULL;
    char action[16] = {0};
    uint64_t now_time = meponch();

    fp_debug("cmd[%s]\n", cmd);
    ret = sscanf(cmd, "%*s %*s "MAC_PATTERN" %255[\001-\377]",
            &maci[0], &maci[1], &maci[2], &maci[3], &maci[4], &maci[5], action);

    if(ret < 7)
    {
        fp_debug("send error ret [%d]\n", ret);
        return 0;
    }

    mac[0] = maci[0]&0xff;
    mac[1] = maci[1]&0xff;
    mac[2] = maci[2]&0xff;
    mac[3] = maci[3]&0xff;
    mac[4] = maci[4]&0xff;
    mac[5] = maci[5]&0xff;

    if(strcmp("update", action) == 0)
    {
        finger_list_init_node(&finger_list_node);

        memcpy(finger_list_node.data.mac, mac, 6);
        finger_list_node.data.send_time = now_time;
        finger_list_node.data.length = 0;

        pfinger_list = finger_list_find(mac, &maclistHead);
        if(pfinger_list)
        {
            pfinger_list->data.send_time = now_time;
        }
        else
        {
            if(node_count >= MAX_FINGERPRINT_RING_SIZE)
            {
                node_count--;
                fp_debug("The buffer of fingerData is full. Delete oldest one");
                resetFingerData();
            }
            finger_list_add_node(&finger_list_node, &maclistHead);
            node_count++;
        }
        bytes = udp_packet_enc((uint8_t *)&finger_list_node.data, sizeof(finger_list_node.data), inet_addr("255.255.255.255"), FINGER_UDP_PORT, SYNC_TYPE_UPDATE);
        fp_debug("sent %d bytes to 255.255.255.255\n", bytes);
    }
    else if(strcmp("renew", action) == 0)
    {
        pip_list = ip_list_find(mac, &iplistHead);
        if(pip_list)
        {
            if(pip_list->data.send_time + 3000 > now_time)
            {
                fp_debug("Don't send %02x:%02x:%02x:%02x:%02x:%02x too fast, wait 3 secs...\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            }
            else
            {
                memcpy(ip_list_node.data.mac, mac, 6);
                pip_list->data.send_time = now_time;
                ip_list_node.data.send_time = now_time;

                bytes = udp_packet_enc((uint8_t *)&ip_list_node.data, sizeof(ip_list_node.data), inet_addr("255.255.255.255"), FINGER_UDP_PORT, SYNC_TYPE_RENEW);
                fp_debug("sent %d bytes to 255.255.255.255\n", bytes);
            }
        }
    }

    return 0;
}

static int fingersync_list_data_set_data_cb(char *cmd)
{
    int ret;
    int mac[6], mac2[6];
    uint64_t now_time = meponch();
    char action[16] = {0};
    u8 client_mac[6], gateway_mac[6];
    struct ip_cfg_list ip_list_node;
    struct ip_cfg_list *pip_list;

    ret = sscanf(cmd, "%*s %*s "MAC_PATTERN" "MAC_PATTERN" %255[\001-\377]",
            &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5],
            &mac2[0], &mac2[1], &mac2[2], &mac2[3], &mac2[4], &mac2[5],
            action);

    if(ret < 13)
    {
        fp_debug("send error ret [%d]\n", ret);
        return 0;
    }

    client_mac[0] = mac[0]&0xff;
    client_mac[1] = mac[1]&0xff;
    client_mac[2] = mac[2]&0xff;
    client_mac[3] = mac[3]&0xff;
    client_mac[4] = mac[4]&0xff;
    client_mac[5] = mac[5]&0xff;
    gateway_mac[0] = mac2[0]&0xff;
    gateway_mac[1] = mac2[1]&0xff;
    gateway_mac[2] = mac2[2]&0xff;
    gateway_mac[3] = mac2[3]&0xff;
    gateway_mac[4] = mac2[4]&0xff;
    gateway_mac[5] = mac2[5]&0xff;

    pip_list = ip_list_find(client_mac, &iplistHead);
    if(pip_list)
    {
        if(strcmp("add", action) == 0)
        {
            pip_list->data.update_time = now_time;
            memcpy(pip_list->data.ipnodeinfo.gateway_mac, gateway_mac, 6);
            dumpIpDataToFile(1);
        }
    }

    return 0;
}

static int fingersync_list_data_set_vlan_info_cb(char *cmd)
{
    struct ip_cfg_list *pip_list;
    struct ip_cfg_list ip_list_node;
    int mac[6], ret;
    int vlanId = 0;
    char action[16] = {}, ifname[16] = {};
    u8 client_mac[6];
    unsigned char buf2[MAX_IP_DATA_LEN] = {0};
    uint64_t now_time = meponch();
    ip_sync_pkt_t *ipbuf;
    ipbuf = (ip_sync_pkt_t *)buf2;

    ret = sscanf(cmd, "%*s %*s "MAC_PATTERN" %d %s %255[\001-\377]",
            &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5],
            &vlanId,
            ifname,
            action);

    if(ret < 9)
    {
        fp_debug("send error ret [%d]\n", ret);
        return 0;
    }

    client_mac[0] = mac[0]&0xff;
    client_mac[1] = mac[1]&0xff;
    client_mac[2] = mac[2]&0xff;
    client_mac[3] = mac[3]&0xff;
    client_mac[4] = mac[4]&0xff;
    client_mac[5] = mac[5]&0xff;

    fp_debug("Client Mac: %02x:%02x:%02x:%02x:%02x:%02x \n", client_mac[0], client_mac[1], client_mac[2], client_mac[3], client_mac[4], client_mac[5]);
    fp_debug("vlanId %d \n", vlanId);
    fp_debug("ifname %s \n", ifname);
    fp_debug("action %s \n", action);

    if(strcmp("add", action) == 0)
    {
        pip_list = ip_list_find(client_mac, &iplistHead);
        if(!pip_list)
        {
            if(ip_node_count >= MAX_FINGERPRINT_RING_SIZE)
            {
                ip_node_count--;
                fp_debug("The buffer of fingerIpData is full. Delete oldest one");
                resetFingerIpData();
            }
            ip_node_count++;
            memcpy(ipbuf->mac, client_mac, 6);
            memcpy(ipbuf->ipnodeinfo.ifname, ifname, 16);
            ipbuf->ipnodeinfo.vlan_id = vlanId;
            ipbuf->update_time = now_time;
            ipbuf->ipnodeinfo.isOccupy = 1;
            ip_list_init_node(&ip_list_node);
            memcpy(&ip_list_node.data, buf2, sizeof(ip_sync_pkt_t));

            pip_list = ip_list_add_node(&ip_list_node, &iplistHead);
        }
        else
        {
            pip_list->data.update_time = now_time;
            pip_list->data.ipnodeinfo.vlan_id = vlanId;
            memcpy(pip_list->data.ipnodeinfo.ifname, ifname, 16);
        }
    }

    return 0;
}
