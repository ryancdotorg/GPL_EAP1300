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
#include <guestsyncd.h>
#include <sys/socket.h>
#include <sockIntf.h>
#include <guestsynchandle.h>
#include <netinet/if_ether.h>
#include "eloop.h"
#include "guestsyncdcmd.h"
#include "common.h"
#include <sys/time.h>
#include <arpa/inet.h>
#include <fcntl.h>

extern struct list_head maclistHead;
extern char guestsync_group[32];
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                           Parameter                                     */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/

int is_time_expired(unsigned int check_time)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    guestsync_printf(GUESTSYNC_DEBUG, "mtime now %ld\n", tv.tv_sec);
    if(tv.tv_sec > check_time)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

#if 0
static int get_option_value(uint8_t *option_data, int size, int option, const uint8_t *data, const unsigned int datalen) {
    unsigned int pos = 0;
    unsigned int len = 0;

    //option:      data[0]
    //length:      data[1]
    //option_data: data[2] ...

    guestsync_printf(GUESTSYNC_DEBUG, "find option[%d]\n", option);

    while (pos+1 < datalen)
    {
        len = data[pos+1];
        // protect
        if (pos + 2 + len > datalen)
        {
            return -1;
        }

        if (data[pos] == option)
        {
            if(len > size )
            {
                // option_data size not enough
                return -1;
            }
            memcpy(option_data, &data[pos+2], len);
            return len;
        }
        else
        {
            /* printf("option: %d\n", data[pos]); */
        }
        pos = pos + 2 + len;
    }

    return -1;
}
#endif

#if SYNC_TRAFFIC
static int kmod_coova_traffic_sync(char *dhcpif, char *ipaddr,
        unsigned int output_packets, unsigned int input_packets,
        unsigned int output_octets, unsigned int input_octets)
{
    int fd, rd;
    char line[256] = {0};
    char kcoova_path[128] = {0};

    guestsync_printf(GUESTSYNC_DEBUG, "update %u,%u,%u,%u^%s to /proc/net/coova/coova-%s\n",
            output_packets, input_packets,
            output_octets, input_octets,
            ipaddr, dhcpif);

    snprintf(kcoova_path, sizeof(kcoova_path), "/proc/net/coova/coova-%s", dhcpif);
    fd = open(kcoova_path, O_RDWR, 0);
    if (fd > 0)
    {
        snprintf(line, sizeof(line), "^%u,%u,%u,%u^%s\n",
                output_packets, input_packets,
                output_octets, input_octets, ipaddr);

        rd = write(fd, line, strlen(line));
        close(fd);
        return rd == strlen(line);
    }
    else
    {
        guestsync_printf(GUESTSYNC_ERROR, "kmod_coova_traffic_sync update error\n");
    }
    return 0;
}
#endif
/*****************************************************************
* NAME:    guestsyncLogoutReplyHandleCB
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
void guestsyncLogoutReplyHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt)
{
    struct guest_cfg_list *pguest_list;
    guest_sync_pkt_t *pdata;
    char client_mac[32] = {0};
    char rets[64] = {0};
    int ssid_num = 1;
    char dut_dhcpif[16] = {0};
    struct in_addr node_ip_addr;

    pdata = (guest_sync_pkt_t *)reqPkt;
    pguest_list = guest_list_find((unsigned char *)pdata->mac, pdata->dhcpif, pdata->random, &maclistHead);

    if(pguest_list)
    {
        ssid_num = get_ssid_num(pdata->dhcpif);
        snprintf(dut_dhcpif, sizeof(dut_dhcpif), "br-ssid%d", ssid_num);

        sprintf(client_mac, MAC_PATTERN, \
                pdata->mac[0], pdata->mac[1], pdata->mac[2], \
                pdata->mac[3], pdata->mac[4], pdata->mac[5]);

        node_ip_addr.s_addr = pdata->nodeinfo.ipaddr;

        guestsync_printf(GUESTSYNC_DEBUG, "do logout %s\n", client_mac);
        guestsync_printf(GUESTSYNC_DEBUG, "ipaddr %s\n", inet_ntoa(node_ip_addr));
        memset(rets, 0, sizeof(rets));
        if(my_system(rets, sizeof(rets), "chilli_query -s /var/run/chilli.%s.sock dhcp-release %s", dut_dhcpif, client_mac) < 0)
        {
            // got throught, nothing return
        }
    }
}

/*****************************************************************
* NAME:    guestsyncLogoutRequestHandleCB
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
void guestsyncLogoutRequestHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt)
{
    struct guest_cfg_list *pguest_list;
    guest_sync_pkt_t *pdata;
    unsigned char buf[MAX_GUESTSYNC_DATA_LEN] = {0};
    int bytes;
    guest_sync_pkt_t *pbuf;
    struct in_addr ip_addr;
    ip_addr.s_addr = src_ip;
    struct timeval tv;

    pdata = (guest_sync_pkt_t *)reqPkt;
    pguest_list = guest_list_find((unsigned char *)pdata->mac, pdata->dhcpif, pdata->random, &maclistHead);

    if(pguest_list)
    {
        memcpy(buf, &pguest_list->data, sizeof(pguest_list->data));
        pbuf = (guest_sync_pkt_t *)buf;
        pbuf->type = SYNC_TYPE_LOGOUT_REPLY;
        gettimeofday(&tv, NULL);
        pbuf->expiretime = htonl(tv.tv_sec + GUESTSYNC_REQUEST_EXPIRETIME);

        guestsync_printf(GUESTSYNC_DEBUG, "Send unicast UPDATE REPLY to %s\n", inet_ntoa(ip_addr));
        bytes = udp_packet_enc((uint8_t*)&buf, sizeof(guest_sync_pkt_t), src_ip, GUESTSYNC_UDP_PORT);
        guestsync_printf(GUESTSYNC_DEBUG, "send bytes:[%d]\n", bytes);

    }
}

/*****************************************************************
* NAME:    guestsyncLogoutHandleCB
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
void guestsyncLogoutHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt)
{
    int bytes;
    struct in_addr ip_addr;
    ip_addr.s_addr = src_ip;
    unsigned char buf[MAX_GUESTSYNC_DATA_LEN] = {0};
    guest_sync_pkt_t *pdata;
    guest_sync_pkt_t *pbuf;
    char client_mac[32] = {0};
    char rets[64] = {0};
    int ssid_num = 1;
    int len;
    struct guest_cfg_list guest_list_node;
    struct guest_cfg_list *pguest_list;

    memset(buf, 0, sizeof(buf));

    pdata = (guest_sync_pkt_t *)reqPkt;
    pbuf = (guest_sync_pkt_t *)buf;

    sprintf(client_mac, MAC_PATTERN, \
            pdata->mac[0], pdata->mac[1], pdata->mac[2], \
            pdata->mac[3], pdata->mac[4], pdata->mac[5]);

    ssid_num = get_ssid_num(pdata->dhcpif);
    // ensure memset here before to got ssid
    memset(rets, 0, sizeof(rets));
    if(my_system(rets, sizeof(rets), "printf `uci get wireless.wifi0_ssid_%d.ssid`", ssid_num) < 0)
    {
        guestsync_printf(GUESTSYNC_ERROR, "get ssid command error\n");
        return;
    }

    if(memcmp(pdata->ssid, rets, 32))
    {
        guestsync_printf(GUESTSYNC_DEBUG, "req SSID[%.32s]\nmy  SSID[%.32s]\n", pdata->ssid, rets);
        guestsync_printf(GUESTSYNC_DEBUG, "Different SSID name! skip request\n");
        return;
    }

    pbuf->type = SYNC_TYPE_LOGOUT_REQUEST;

    memcpy(pbuf->random, pdata->random, 16);
    memcpy(pbuf->mac, pdata->mac, 6);
    strcpy(pbuf->dhcpif, pdata->dhcpif);

    memcpy(pbuf->group, guestsync_group, 32);
    pbuf->expiretime = pdata->expiretime;

    memcpy(&(pbuf->nodeinfo), &(pdata->nodeinfo), sizeof(node_info_t));

    len = sizeof(guest_sync_pkt_t);
    guest_list_init_node(&guest_list_node);
    memcpy(&guest_list_node.data, buf, sizeof(guest_sync_pkt_t));

    pguest_list = guest_list_add_node(&guest_list_node, &maclistHead);
    if(pguest_list)
    {
        eloop_register_timeout(10, 0, sync_data_delete_cb, pguest_list, 0);
        guestsync_printf(GUESTSYNC_DEBUG, "Send unicast LOGOUT REQUEST to %s\n", inet_ntoa(ip_addr));
        bytes = udp_packet_enc((uint8_t*)&buf, len, src_ip, GUESTSYNC_UDP_PORT);
        guestsync_printf(GUESTSYNC_DEBUG, "send bytes:[%d]\n", bytes);
    }
}


/*****************************************************************
* NAME:    guestsyncUpdateReplyHandleCB
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
void guestsyncUpdateReplyHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt)
{
    struct guest_cfg_list *pguest_list;
    guest_sync_pkt_t *pdata;
    char client_mac[32] = {0};
    char rets[64] = {0};
    int ssid_num = 1;
    char dut_dhcpif[16] = {0};
    unsigned int update_sessiontime = 0, update_sessiontimeout = 0;
    unsigned int update_idletime = 0, update_idletimeout = 0;
    unsigned int update_gonetime = 0;
#if SYNC_TRAFFIC
    unsigned int update_input_packets = 0, update_output_packets = 0;
    unsigned int update_input_octets = 0, update_output_octets = 0;
    char update_sessionid[32] = {0};
    char client_ip[32] = {0};
#endif
    int update_interiminterval = 0;
    struct in_addr node_ip_addr;

    pdata = (guest_sync_pkt_t *)reqPkt;

    pguest_list = guest_list_find((unsigned char *)pdata->mac, pdata->dhcpif, pdata->random, &maclistHead);

    if(pguest_list)
    {
        ssid_num = get_ssid_num(pdata->dhcpif);
        snprintf(dut_dhcpif, sizeof(dut_dhcpif), "br-ssid%d", ssid_num);

        sprintf(client_mac, MAC_PATTERN_HYPHEN, \
                pdata->mac[0], pdata->mac[1], pdata->mac[2], \
                pdata->mac[3], pdata->mac[4], pdata->mac[5]);

        node_ip_addr.s_addr = pdata->nodeinfo.ipaddr;
        memset(rets, 0, sizeof(rets));

        if(my_system(rets, sizeof(rets), "chilli_query -s /var/run/chilli.%s.sock dhcp-new %s %s", dut_dhcpif, client_mac, inet_ntoa(node_ip_addr)) < 0)
        {
            // got throught, nothing return
        }

        update_sessiontime = ntohl(pdata->nodeinfo.sessiontime);
        update_sessiontimeout = ntohl(pdata->nodeinfo.sessiontimeout);
        update_idletime = ntohl(pdata->nodeinfo.idletime);
        update_idletimeout = ntohl(pdata->nodeinfo.idletimeout);
        update_gonetime = ntohl(pdata->nodeinfo.gonetime);
        update_interiminterval = ntohl(pdata->nodeinfo.interiminterval);
#if SYNC_TRAFFIC
        update_input_packets = ntohl(pdata->nodeinfo.input_packets);
        update_output_packets = ntohl(pdata->nodeinfo.output_packets);
        update_input_octets = ntohl(pdata->nodeinfo.input_octets);
        update_output_octets = ntohl(pdata->nodeinfo.output_octets);
        memcpy(update_sessionid, pdata->nodeinfo.sessionid, sizeof(update_sessionid));
#endif

        guestsync_printf(GUESTSYNC_DEBUG, "sync update_sessiontime %u username %s\n", update_sessiontime, pdata->nodeinfo.username);

        if(update_sessiontime >= 0)
        {
            // update_sessiontime + 1 to avoid sessiontime == 0 not working.
            if(my_system(rets, sizeof(rets), "chilli_query -s /var/run/chilli.%s.sock update mac %s gonetime %d sessiontime %u sessiontimeout %u idletime %u idletimeout %u interiminterval %d username '%s'",
                        dut_dhcpif, client_mac,
                        update_gonetime,
                        update_sessiontime + 1, update_sessiontimeout,
                        update_idletime, update_idletimeout,
                        update_interiminterval, pdata->nodeinfo.username) < 0)
            {
                //guestsync_printf(GUESTSYNC_ERROR, "chilli_query command error\n");
                //go through
            }

#if SYNC_TRAFFIC
            if(node_ip_addr.s_addr == 0)
            {
                //NAT mode or not got it, try to use chilli_query to get ip
                memset(client_ip, 0, sizeof(client_ip));
                if(my_system(client_ip, sizeof(client_ip), "chilli_query -s /var/run/chilli.%s.sock -json list mac %s|jq -r '.sessions[].ipAddress'",
                            dut_dhcpif, client_mac) < 0)
                {
                    //guestsync_printf(GUESTSYNC_ERROR, "not found real mac\n");
                    //go through
                }
            }
            else
            {
                strcpy(client_ip, inet_ntoa(node_ip_addr));
            }

            if(strlen(client_ip) < 6 || strcmp("0.0.0.0", client_ip) == 0)
            {
                guestsync_printf(GUESTSYNC_ERROR, "unknown ip address %s\n", client_ip);
            }
            else
            {
                kmod_coova_traffic_sync(dut_dhcpif, client_ip,
                        update_output_octets, update_input_octets,
                        update_output_packets, update_input_packets);
            }
#endif
        }
    }
}

/*****************************************************************
* NAME:    guestsyncUpdateRequestHandleCB
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
void guestsyncUpdateRequestHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt)
{
    struct guest_cfg_list *pguest_list;
    guest_sync_pkt_t *pdata;
    unsigned char buf[MAX_GUESTSYNC_DATA_LEN] = {0};
    int bytes;
    guest_sync_pkt_t *pbuf;
    struct in_addr ip_addr;
    ip_addr.s_addr = src_ip;
    struct timeval tv;
#if SYNC_TRAFFIC
    unsigned int update_input_packets = 0, update_output_packets = 0;
    unsigned int update_input_octets = 0, update_output_octets = 0;
    int ssid_num = 1;
    char client_ip[32] = {0};
    char dut_dhcpif[16] = {0};
    int needupdate = 0;
    struct in_addr node_ip_addr;
#endif
    char client_mac[32] = {0};

    pdata = (guest_sync_pkt_t *)reqPkt;
    pguest_list = guest_list_find((unsigned char *)pdata->mac, pdata->dhcpif, pdata->random, &maclistHead);

    if(pguest_list)
    {
        memcpy(buf, &pguest_list->data, sizeof(pguest_list->data));
        pbuf = (guest_sync_pkt_t *)buf;
        pbuf->type = SYNC_TYPE_UPDATE_REPLY;
        gettimeofday(&tv, NULL);
        pbuf->expiretime = htonl(tv.tv_sec + GUESTSYNC_REQUEST_EXPIRETIME);

        sprintf(client_mac, MAC_PATTERN_HYPHEN, \
                pdata->mac[0], pdata->mac[1], pdata->mac[2], \
                pdata->mac[3], pdata->mac[4], pdata->mac[5]);

#if SYNC_TRAFFIC
        node_ip_addr.s_addr = pdata->nodeinfo.ipaddr;

        update_input_packets = ntohl(pbuf->nodeinfo.input_packets);
        update_output_packets = ntohl(pbuf->nodeinfo.output_packets);
        update_input_octets = ntohl(pbuf->nodeinfo.input_octets);
        update_output_octets = ntohl(pbuf->nodeinfo.output_octets);

        if(ntohl(pdata->nodeinfo.input_packets) > update_input_packets)
        {
            pbuf->nodeinfo.input_packets = pdata->nodeinfo.input_packets;
            needupdate = 1;
        }
        if(ntohl(pdata->nodeinfo.output_packets) > update_output_packets)
        {
            pbuf->nodeinfo.output_packets = pdata->nodeinfo.output_packets;
            needupdate = 1;
        }
        if(ntohl(pdata->nodeinfo.input_octets) > update_input_octets)
        {
            pbuf->nodeinfo.input_octets = pdata->nodeinfo.input_octets;
            needupdate = 1;
        }
        if(ntohl(pdata->nodeinfo.output_octets) > update_output_octets)
        {
            pbuf->nodeinfo.output_octets = pdata->nodeinfo.output_octets;
            needupdate = 1;
        }

        if(needupdate)
        {
            ssid_num = get_ssid_num(pdata->dhcpif);
            snprintf(dut_dhcpif, sizeof(dut_dhcpif), "br-ssid%d", ssid_num);

            //NAT mode or not contain ipaddr, try to use chilli_query to get ip
            if(node_ip_addr.s_addr == 0)
            {
                memset(client_ip, 0, sizeof(client_ip));
                if(my_system(client_ip, sizeof(client_ip), "chilli_query -s /var/run/chilli.%s.sock -json list mac %s|jq -r '.sessions[].ipAddress'",
                            dut_dhcpif, client_mac) < 0)
                {
                    //guestsync_printf(GUESTSYNC_ERROR, "not found real mac\n");
                    //go through
                }
            }
            else
            {
                strcpy(client_ip, inet_ntoa(node_ip_addr));
            }

            if(strlen(client_ip) < 6 || strcmp("0.0.0.0", client_ip) == 0)
            {
                guestsync_printf(GUESTSYNC_ERROR, "unknown ip address %s\n", client_ip);
            }
            else
            {
                kmod_coova_traffic_sync(dut_dhcpif, client_ip,
                        update_output_octets, update_input_octets,
                        update_output_packets, update_input_packets);
            }
        }
#endif

        guestsync_printf(GUESTSYNC_DEBUG, "Send unicast UPDATE REPLY to %s\n", inet_ntoa(ip_addr));
        bytes = udp_packet_enc((uint8_t*)&buf, sizeof(guest_sync_pkt_t), src_ip, GUESTSYNC_UDP_PORT);
        guestsync_printf(GUESTSYNC_DEBUG, "send bytes:[%d]\n", bytes);
    }
}

/*****************************************************************
* NAME:    guestsyncUpdateHandleCB
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
void guestsyncUpdateHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt)
{
    int bytes;
    struct in_addr ip_addr;
    ip_addr.s_addr = src_ip;
    unsigned char buf[MAX_GUESTSYNC_DATA_LEN] = {0};
    guest_sync_pkt_t *pdata;
    guest_sync_pkt_t *pbuf;
    char client_mac[32] = {0};
    char rets[64] = {0};
    int ssid_num = 1;
    int len;
    struct guest_cfg_list guest_list_node;
    struct guest_cfg_list *pguest_list;
#if SYNC_TRAFFIC
    char attr_values[128] = {0};
    unsigned int update_input_packets = 0, update_output_packets = 0;
    unsigned int update_input_octets = 0, update_output_octets = 0;
    int ret = 0;
#endif
    char dut_dhcpif[16] = {0};

    pdata = (guest_sync_pkt_t *)reqPkt;
    pbuf = (guest_sync_pkt_t *)buf;
    memset(buf, 0, sizeof(buf));

    ssid_num = get_ssid_num(pdata->dhcpif);
    snprintf(dut_dhcpif, sizeof(dut_dhcpif), "br-ssid%d", ssid_num);

    sprintf(client_mac, MAC_PATTERN_HYPHEN, \
            pdata->mac[0], pdata->mac[1], pdata->mac[2], \
            pdata->mac[3], pdata->mac[4], pdata->mac[5]);

    // ensure memset here before to got ssid
    memset(rets, 0, sizeof(rets));
    if(my_system(rets, sizeof(rets), "printf `uci get wireless.wifi0_ssid_%d.ssid`", ssid_num) < 0)
    {
        guestsync_printf(GUESTSYNC_ERROR, "get ssid command error\n");
        return;
    }

    if(memcmp(pdata->ssid, rets, 32))
    {
        guestsync_printf(GUESTSYNC_DEBUG, "req SSID[%.32s]\nmy  SSID[%.32s]\n", pdata->ssid, rets);
        guestsync_printf(GUESTSYNC_DEBUG, "Different SSID name! skip request\n");
        return;
    }

    pbuf->type = SYNC_TYPE_UPDATE_REQUEST;

    memcpy(pbuf->random, pdata->random, 16);
    memcpy(pbuf->mac, pdata->mac, 6);
    strcpy(pbuf->dhcpif, pdata->dhcpif);

    memcpy(pbuf->group, guestsync_group, 32);
    pbuf->expiretime = pdata->expiretime;

    //copy full node info from pdata to pbuf, replace node info belown
    memcpy(&(pbuf->nodeinfo), &(pdata->nodeinfo), sizeof(node_info_t));

#if SYNC_TRAFFIC
    if(my_system(attr_values, sizeof(attr_values),
                "chilli_query -s /var/run/chilli.%s.sock -json list mac %s|jq -r '.sessions[].accounting | \"\\(.inputPackets) \\(.outputPackets) \\(.inputOctets) \\(.outputOctets)\"'",
                dut_dhcpif, client_mac) < 0)
    {
        // got throught, nothing return
    }

    ret = sscanf(attr_values, "%u %u %u %u", &update_input_packets, &update_output_packets, &update_input_octets, &update_output_octets);
    if(ret == 4)
    {
        if(ntohl(pdata->nodeinfo.input_packets) < update_input_packets)
        {
            pbuf->nodeinfo.input_packets = htonl(update_input_packets);
        }
        if(ntohl(pdata->nodeinfo.output_packets) < update_output_packets)
        {
            pbuf->nodeinfo.output_packets = htonl(update_output_packets);
        }
        if(ntohl(pdata->nodeinfo.input_octets) < update_input_octets)
        {
            pbuf->nodeinfo.input_octets = htonl(update_input_octets);
        }
        if(ntohl(pdata->nodeinfo.output_octets) < update_output_octets)
        {
            pbuf->nodeinfo.output_octets = htonl(update_output_octets);
        }
    }
#endif

    len = sizeof(guest_sync_pkt_t);
    guest_list_init_node(&guest_list_node);
    memcpy(&guest_list_node.data, buf, sizeof(guest_sync_pkt_t));

    pguest_list = guest_list_add_node(&guest_list_node, &maclistHead);
    if(pguest_list)
    {
        eloop_register_timeout(10, 0, sync_data_delete_cb, pguest_list, 0);
        guestsync_printf(GUESTSYNC_DEBUG, "Send unicast UPDATE REQUEST to %s\n", inet_ntoa(ip_addr));
        bytes = udp_packet_enc((uint8_t*)&buf, len, src_ip, GUESTSYNC_UDP_PORT);
        guestsync_printf(GUESTSYNC_DEBUG, "send bytes:[%d]\n", bytes);
    }
}

/*****************************************************************
* NAME:    guestsyncGoneHandleCB
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2020-06-22
* Modify:
******************************************************************/
void guestsyncGoneHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt)
{
    guest_sync_pkt_t *pdata;
    //guest_sync_pkt_t *pbuf;
    char client_mac[32] = {0};
    char rets[64] = {0};
    int ssid_num = 1;
    char dut_dhcpif[16] = {0};
    unsigned int update_gonetime = 0;
    unsigned int update_idletime = 0;

    pdata = (guest_sync_pkt_t *)reqPkt;
    ssid_num = get_ssid_num(pdata->dhcpif);
    snprintf(dut_dhcpif, sizeof(dut_dhcpif), "br-ssid%d", ssid_num);

    sprintf(client_mac, MAC_PATTERN, \
            pdata->mac[0], pdata->mac[1], pdata->mac[2], \
            pdata->mac[3], pdata->mac[4], pdata->mac[5]);

    // ensure memset here before to got ssid
    memset(rets, 0, sizeof(rets));
    if(my_system(rets, sizeof(rets), "printf `uci get wireless.wifi0_ssid_%d.ssid`", ssid_num) < 0)
    {
        guestsync_printf(GUESTSYNC_ERROR, "get ssid command error\n");
        return;
    }

    if(memcmp(pdata->ssid, rets, 32))
    {
        guestsync_printf(GUESTSYNC_DEBUG, "req SSID[%.32s]\nmy  SSID[%.32s]\n", pdata->ssid, rets);
        guestsync_printf(GUESTSYNC_DEBUG, "Different SSID name! skip request\n");
        return;
    }

    update_gonetime = ntohl(pdata->nodeinfo.gonetime);
    update_idletime = ntohl(pdata->nodeinfo.idletime);
    guestsync_printf(GUESTSYNC_DEBUG, "sync update gonetime %d idletime %d\n", update_gonetime, update_idletime);

    if(update_idletime > 10)
    {
        // update_sessiontime + 1 to avoid sessiontime == 0 not working.
        if(my_system(rets, sizeof(rets), "chilli_query -s /var/run/chilli.%s.sock update mac %s gonetime %d idletime %d", dut_dhcpif, client_mac, update_gonetime, update_idletime) < 0)
        {
            //guestsync_printf(GUESTSYNC_ERROR, "chilli_query command error\n");
            //go through
        }
    }
    else
    {
        // update_sessiontime + 1 to avoid sessiontime == 0 not working.
        if(my_system(rets, sizeof(rets), "chilli_query -s /var/run/chilli.%s.sock update mac %s gonetime %d", dut_dhcpif, client_mac, update_gonetime) < 0)
        {
            //guestsync_printf(GUESTSYNC_ERROR, "chilli_query command error\n");
            //go through
        }
    }
}

/*****************************************************************
* NAME:    guestsyncPacketHandleCB
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
void guestsyncPacketHandleCB(int cli_socket, uint32_t src_ip, uint8_t *reqPkt, int len)
{
    int i;
    struct in_addr ip_addr;
    ip_addr.s_addr = src_ip;
    uint32_t my_ip;
    int ifidx;
    struct in_addr node_ip_addr;
    guest_sync_pkt_t *pdata;
    uint8_t *popt;
    int tmp;

    pdata = (guest_sync_pkt_t *)reqPkt;
    popt = &reqPkt[sizeof(guest_sync_pkt_t)];

    read_interface(guestsyncInterface(), &ifidx, &my_ip, 0);
    if(src_ip == my_ip)
    {
        guestsync_printf(GUESTSYNC_DEBUG, "Ignore mine request.\n");
        return;
    }

    guestsync_printf(GUESTSYNC_DEBUG, "Receive...\n");

    if(memcmp(pdata->group, guestsync_group, 32))
    {
        guestsync_printf(GUESTSYNC_DEBUG, "Different group [%.32s]:[%.32s]\n", guestsync_group, pdata->group);
        return;
    }

    guestsync_printf(GUESTSYNC_DEBUG, "IP:[%s]\n", inet_ntoa(ip_addr));
    guestsync_printf(GUESTSYNC_DEBUG, "Type:[%x]\n", pdata->type);
    guestsync_printf(GUESTSYNC_DEBUG, "DHCPIF:[%s]\n", pdata->dhcpif);
    guestsync_printf(GUESTSYNC_DEBUG, "Random:[");
    for(i=0; i<16; ++i)
        guestsync_printf(GUESTSYNC_DEBUG,"%c", pdata->random[i]);
    guestsync_printf(GUESTSYNC_DEBUG, "]\n");
    guestsync_printf(GUESTSYNC_DEBUG, "MAC:[%02x:%02x:%02x:%02x:%02x:%02x]\n",
            pdata->mac[0], pdata->mac[1], pdata->mac[2],
            pdata->mac[3], pdata->mac[4], pdata->mac[5]);
    guestsync_printf(GUESTSYNC_DEBUG, "SSID:[%.32s]\n", pdata->ssid);
    guestsync_printf(GUESTSYNC_DEBUG, "Group:[%.32s]\n", pdata->group);
    guestsync_printf(GUESTSYNC_DEBUG, "expiretime:[%u]\n", ntohl(pdata->expiretime));
    guestsync_printf(GUESTSYNC_DEBUG, "node info\n");
    guestsync_printf(GUESTSYNC_DEBUG, "sessiontime:[%u]\n", ntohl(pdata->nodeinfo.sessiontime));
    guestsync_printf(GUESTSYNC_DEBUG, "sessiontimeout:[%u]\n", ntohl(pdata->nodeinfo.sessiontimeout));
    guestsync_printf(GUESTSYNC_DEBUG, "idletime:[%u]\n", ntohl(pdata->nodeinfo.idletime));
    guestsync_printf(GUESTSYNC_DEBUG, "idletimeout:[%u]\n", ntohl(pdata->nodeinfo.idletimeout));
    guestsync_printf(GUESTSYNC_DEBUG, "interiminterval:[%d]\n", ntohl(pdata->nodeinfo.interiminterval));
    guestsync_printf(GUESTSYNC_DEBUG, "input_packets:[%u]\n", ntohl(pdata->nodeinfo.input_packets));
    guestsync_printf(GUESTSYNC_DEBUG, "output_packets:[%u]\n", ntohl(pdata->nodeinfo.output_packets));
    guestsync_printf(GUESTSYNC_DEBUG, "input_octets:[%u]\n", ntohl(pdata->nodeinfo.input_octets));
    guestsync_printf(GUESTSYNC_DEBUG, "output_octets:[%u]\n", ntohl(pdata->nodeinfo.output_octets));
    guestsync_printf(GUESTSYNC_DEBUG, "session_id:[%s]\n", pdata->nodeinfo.sessionid);
    guestsync_printf(GUESTSYNC_DEBUG, "username:[%s]\n", pdata->nodeinfo.username);
    node_ip_addr.s_addr = pdata->nodeinfo.ipaddr;
    guestsync_printf(GUESTSYNC_DEBUG, "ip_addr:[%s]\n", inet_ntoa(node_ip_addr));
    guestsync_printf(GUESTSYNC_DEBUG, "Length:[%d]\n", ntohl(pdata->length));

    if(ntohl(pdata->length))
    {
        while(popt < &reqPkt[len])
        {
            guestsync_printf(GUESTSYNC_DEBUG, "opt   type:[0x%02x]\n", *popt);
            popt += 1;
            guestsync_printf(GUESTSYNC_DEBUG, "    length:[0x%02x]\n", *popt);
            tmp = *popt;
            popt += 1;
            guestsync_printf(GUESTSYNC_DEBUG, "     value:[");
            for(i=0; i<tmp; ++i)
            {
                guestsync_printf(GUESTSYNC_DEBUG, "0x%02x ", popt[i]);
            }
            guestsync_printf(GUESTSYNC_DEBUG, "]\n");
            popt += tmp;
        }
    }

    switch(pdata->type)
    {
        case SYNC_TYPE_UPDATE:
        case SYNC_TYPE_UPDATE_REPLY:
        case SYNC_TYPE_LOGOUT:
        case SYNC_TYPE_LOGOUT_REPLY:
        case SYNC_TYPE_GONE:
            if(is_time_expired(ntohl(pdata->expiretime)))
            {
                guestsync_printf(GUESTSYNC_DEBUG, "time expired...\n");
                return;
            }
            break;
        default:
            break;
    }

    switch(pdata->type)
    {
        // scenario: user connect to ap1, ap1's chilli run guestsyncli send request br-ssidx 00aabbccdd10 update
        //                                  ap1 ---> ap2 guestsyncUpdateHandleCB()
        // guestsyncUpdateRequestHandleCB() ap1 <--- ap2
        //                                  ap1 ----> ap2 guestsyncUpdateReplyHandleCB()
        case SYNC_TYPE_UPDATE:
            guestsyncUpdateHandleCB(cli_socket, src_ip, reqPkt);
            break;
        case SYNC_TYPE_UPDATE_REQUEST:
            guestsyncUpdateRequestHandleCB(cli_socket, src_ip, reqPkt);
            break;
        case SYNC_TYPE_UPDATE_REPLY:
            guestsyncUpdateReplyHandleCB(cli_socket, src_ip, reqPkt);
            break;
        case SYNC_TYPE_LOGOUT:
            guestsyncLogoutHandleCB(cli_socket, src_ip, reqPkt);
            break;
        case SYNC_TYPE_LOGOUT_REQUEST:
            guestsyncLogoutRequestHandleCB(cli_socket, src_ip, reqPkt);
            break;
        case SYNC_TYPE_LOGOUT_REPLY:
            guestsyncLogoutReplyHandleCB(cli_socket, src_ip, reqPkt);
            break;
        case SYNC_TYPE_GONE:
            guestsyncGoneHandleCB(cli_socket, src_ip, reqPkt);
            break;
        default:
            break;
    }
    guestsync_printf(GUESTSYNC_DEBUG, "Done...\n");
}
