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
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sockIntf.h>
#include <netinet/if_ether.h>
#include "fingerd.h"
#include "fingerhandle.h"
#include "eloop.h"
#include "fingerdcmd.h"
#include "common.h"
#include "fingerprint.h"

extern struct list_head maclistHead;
extern struct list_head iplistHead;
extern int DebugEnable;
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                           Parameter                                     */
/*-------------------------------------------------------------------------*/


/*****************************************************************
* NAME:    fingersyncPacketHandleCB
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
void fingersyncPacketHandleCB(int cli_socket, uint32_t src_ip, uint8_t *reqPkt, int len, unsigned char *type)
{
    struct in_addr ip_addr;
    ip_addr.s_addr = src_ip;
    uint32_t my_ip;
    int ifidx;
    unsigned char buf[MAX_FINGER_DATA_LEN] = {0};
    int sendlen = 0;
    int bytes;
    finger_sync_pkt_t *pbuf;
    finger_sync_pkt_t *pdata;

    pdata = (finger_sync_pkt_t *)reqPkt;
    pbuf = (finger_sync_pkt_t *)buf;

    read_interface(fingersyncInterface(), &ifidx, &my_ip);
    if(src_ip == my_ip)
    {
        fp_debug("Ignore mine request.\n");
        return;
    }

    memcpy(pbuf->mac, pdata->mac, 6);
    sendlen = sizeof(finger_sync_pkt_t);

    fp_debug("Send unicast REPLY to %s\n", inet_ntoa(ip_addr));
    bytes = udp_packet_enc((uint8_t*)&buf, sendlen, src_ip, FINGER_UDP_PORT, type);
    fp_debug("send bytes:[%d]\n", bytes);
    fp_debug("fingersyncPacketHandleCB...\n");

}
void fingerprintsyncUpdateHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt)
{
    int bytes;
    struct in_addr ip_addr, node_ip_addr;
    ip_addr.s_addr = src_ip;
    unsigned char buf[MAX_FINGER_DATA_LEN] = {0};
    finger_sync_pkt_t *pbuf;
    finger_sync_pkt_t *pdata;
    char macs[32] = {0};
    int sendlen = 0;
    struct finger_cfg_list *pfinger_list;
    char cmd[64] = {0};

    memset(buf, 0, sizeof(buf));

    pdata = (finger_sync_pkt_t *)reqPkt;
    pbuf = (finger_sync_pkt_t *)buf;

    if(pdata->send_time + 30000 < meponch())
    {
        fp_debug("time < now_time, ignore it\n");
        return;
    }

    sprintf(macs, "%02x:%02x:%02x:%02x:%02x:%02x", \
            pdata->mac[0], pdata->mac[1], pdata->mac[2], \
            pdata->mac[3], pdata->mac[4], pdata->mac[5]);
    fp_debug("mac:[%s]\n", macs);

    pfinger_list = finger_list_find(pdata->mac, &maclistHead);
    if(pfinger_list)
    {
        node_ip_addr.s_addr = pfinger_list->data.nodeinfo.ipaddr;
        if((strlen(inet_ntoa(node_ip_addr)) != 0) && (strcmp(inet_ntoa(node_ip_addr), "0.0.0.0") != 0))
        {
            sendlen = sizeof(finger_sync_pkt_t);
            pbuf->nodeinfo.ipaddr =  pfinger_list->data.nodeinfo.ipaddr;
            memcpy(pbuf->mac, pfinger_list->data.mac, 6);
            memcpy(pbuf->nodeinfo.system, pfinger_list->data.nodeinfo.system, 32);
            memcpy(pbuf->nodeinfo.device, pfinger_list->data.nodeinfo.device, 64);

            fp_debug("Send unicast UPDATE REQUEST to %s\n", inet_ntoa(ip_addr));
            bytes = udp_packet_enc((uint8_t*)&buf, sendlen, src_ip, FINGER_UDP_PORT, SYNC_TYPE_UPDATE_REPLY);
            fp_debug("send bytes:[%d]\n", bytes);

            /* 20210125 Tim.Wu: fix issue that ping failed when roam to another ap using proxyarp */
            memset(cmd, 0, sizeof(cmd));
            snprintf(cmd, sizeof(cmd), "/sbin/proxyarp_clear.sh %s &", macs);
            system(cmd);
            /** end **/
        }
    }
}

void fingerprintsyncUpdateReplyHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt)
{
    struct finger_cfg_list *pfinger_list;
    struct in_addr node_ip_addr, pdata_ip_addr;
    finger_sync_pkt_t *pdata;

    pdata = (finger_sync_pkt_t *)reqPkt;

    pfinger_list = finger_list_find(pdata->mac, &maclistHead);
    if(pfinger_list)
    {
        node_ip_addr.s_addr = pfinger_list->data.nodeinfo.ipaddr;
        pdata_ip_addr.s_addr = pdata->nodeinfo.ipaddr;
        if((strlen(inet_ntoa(pdata_ip_addr)) != 0) && (strcmp(inet_ntoa(pdata_ip_addr), "0.0.0.0") != 0))
        {
            if(pfinger_list->data.nodeinfo.isOccupy == 1 || strcmp(inet_ntoa(node_ip_addr), inet_ntoa(pdata_ip_addr)))
            {
                pfinger_list->data.nodeinfo.ipaddr = pdata->nodeinfo.ipaddr;
                pfinger_list->data.update_time = meponch();
                pfinger_list->data.nodeinfo.isOccupy = 0;

                if(strlen(pfinger_list->data.nodeinfo.system)==0)
                    memcpy(pfinger_list->data.nodeinfo.system, pdata->nodeinfo.system, 32);
                if(strlen(pfinger_list->data.nodeinfo.device)==0)
                    memcpy(pfinger_list->data.nodeinfo.device, pdata->nodeinfo.device, 64);

                printFingerDataToFile(FINGERPRINT_FILE_PATH, pfinger_list);
                dumpFingerDataToFile(FINGERPRINT_DB_FILE_PATH, 0);
            }
        }
    }
}
void fingerprintsyncRenewHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt)
{
    int bytes;
    struct in_addr ip_addr;
    ip_addr.s_addr = src_ip;
    unsigned char buf[MAX_FINGER_DATA_LEN] = {0};
    ip_sync_pkt_t *pbuf;
    ip_sync_pkt_t *pdata;
    char macs[32] = {0};
    int sendlen = 0;
    struct ip_cfg_list *pip_list;

    memset(buf, 0, sizeof(buf));

    pdata = (ip_sync_pkt_t *)reqPkt;
    pbuf = (ip_sync_pkt_t *)buf;

    if(pdata->send_time + 30000 < meponch())
    {
        fp_debug("time < now_time, ignore it\n");
        return;
    }

    sprintf(macs, "%02x:%02x:%02x:%02x:%02x:%02x", \
            pdata->mac[0], pdata->mac[1], pdata->mac[2], \
            pdata->mac[3], pdata->mac[4], pdata->mac[5]);
    fp_debug("mac:[%s]\n", macs);

    pip_list = ip_list_find(pdata->mac, &iplistHead);
    if(pip_list)
    {
        if(strlen(pip_list->data.ipnodeinfo.dhcp_mac) != 0 && strlen(pip_list->data.ipnodeinfo.gateway_mac) != 0)
        {
            sendlen = sizeof(finger_sync_pkt_t);
            memcpy(pbuf->mac, pip_list->data.mac, 6);
            pbuf->ipnodeinfo.offer_ip =  pip_list->data.ipnodeinfo.offer_ip;
            pbuf->ipnodeinfo.mask_ip =  pip_list->data.ipnodeinfo.mask_ip;
            pbuf->ipnodeinfo.gateway_ip =  pip_list->data.ipnodeinfo.gateway_ip;
            memcpy(pbuf->ipnodeinfo.dhcp_mac, pip_list->data.ipnodeinfo.dhcp_mac, 6);
            memcpy(pbuf->ipnodeinfo.gateway_mac, pip_list->data.ipnodeinfo.gateway_mac, 6);

            fp_debug("Send unicast Renew REQUEST to %s\n", inet_ntoa(ip_addr));
            bytes = udp_packet_enc((uint8_t*)&buf, sendlen, src_ip, FINGER_UDP_PORT, SYNC_TYPE_RENEW_REPLY);
            fp_debug("send bytes:[%d]\n", bytes);
        }
    }
}

void fingerprintsyncRenewReplyHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt)
{
    struct ip_cfg_list *pip_list;
    struct in_addr node_ip_addr, pdata_ip_addr;
    ip_sync_pkt_t *pdata;

    pdata = (ip_sync_pkt_t *)reqPkt;

    pip_list = ip_list_find(pdata->mac, &iplistHead);
    if(pip_list)
    {
        node_ip_addr.s_addr = pip_list->data.ipnodeinfo.offer_ip;
        pdata_ip_addr.s_addr = pdata->ipnodeinfo.offer_ip;
        if(pip_list->data.ipnodeinfo.isOccupy == 1 || strcmp(inet_ntoa(node_ip_addr), inet_ntoa(pdata_ip_addr)))
        {
            pip_list->data.ipnodeinfo.offer_ip = pdata->ipnodeinfo.offer_ip;
            pip_list->data.ipnodeinfo.mask_ip =  pdata->ipnodeinfo.mask_ip;
            pip_list->data.ipnodeinfo.gateway_ip =  pdata->ipnodeinfo.gateway_ip;
            memcpy(pip_list->data.ipnodeinfo.dhcp_mac, pdata->ipnodeinfo.dhcp_mac, 6);
            memcpy(pip_list->data.ipnodeinfo.gateway_mac, pdata->ipnodeinfo.gateway_mac, 6);
            pip_list->data.update_time = meponch();
            pip_list->data.ipnodeinfo.isOccupy = 0;
            dumpIpDataToFile(1);
        }
    }
}
void fingerprintsyncPacketHandleCB(int cli_socket, uint32_t src_ip, uint8_t *reqPkt, int len, unsigned char *type)
{
    struct in_addr ip_addr;
    ip_addr.s_addr = src_ip;
    uint32_t my_ip;
    int ifidx;
    unsigned char headerType;
    finger_sync_pkt_t *pdata;

    pdata = (finger_sync_pkt_t *)reqPkt;
    headerType = type;

    read_interface(fingersyncInterface(), &ifidx, &my_ip);
    if(src_ip == my_ip)
    {
        fp_debug("Ignore mine request.\n");
        return;
    }

    fp_debug("Receive...\n");
    fp_debug("IP:[%s]\n", inet_ntoa(ip_addr));
    fp_debug("Type:[%x]\n", type);
    fp_debug("MAC:[%02x:%02x:%02x:%02x:%02x:%02x]\n",
                pdata->mac[0], pdata->mac[1], pdata->mac[2],
                pdata->mac[3], pdata->mac[4], pdata->mac[5]);

    switch(headerType)
    {
        case SYNC_TYPE_UPDATE:
            fingerprintsyncUpdateHandleCB(cli_socket, src_ip, reqPkt);
            break;
        case SYNC_TYPE_UPDATE_REPLY:
            fingerprintsyncUpdateReplyHandleCB(cli_socket, src_ip, reqPkt);
            break;
        case SYNC_TYPE_RENEW:
            fingerprintsyncRenewHandleCB(cli_socket, src_ip, reqPkt);
            break;
        case SYNC_TYPE_RENEW_REPLY:
            fingerprintsyncRenewReplyHandleCB(cli_socket, src_ip, reqPkt);
            break;
        default:
            break;
    }
    fp_debug("Done...\n");
}
