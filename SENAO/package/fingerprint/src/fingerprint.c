#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()
#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t, uint32_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_ICMP, INET_ADDRSTRLEN
#include <netinet/ip.h>       // struct ip and IP_MAXPACKET (which is 65535)
#include "dhcp.h"  // struct dhcp
#include <netinet/udp.h>
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#if TOOLCHAIN_MUSL
#include <asm/ioctls.h>
#else
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#endif
#include <net/if.h>           // struct ifreq
#if TOOLCHAIN_MUSL
#define _NETINET_IF_ETHER_H
#include <linux/if_ether.h>   // ETH_P_IP = 0x0800, ETH_P_IPV6 = 0x86DD
#endif
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <net/ethernet.h>
#include <sys/time.h>         // gettimeofday()
#include <errno.h>            // errno, perror()
#include <sys/wait.h>
#include "finger_list.h"
#include "finger.h"
#include "fingerprint.h"
#include "fingerdcmd.h"

extern struct list_head maclistHead;
extern struct list_head iplistHead;
static uint8_t bmac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static uint8_t zmac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
int DebugEnable = 0;
int ifcount;
int node_count = 0;
int ip_node_count = 0;
unsigned int mon_ifindex[32];

finger fingerprint_db[]={
    {"Apple iOS", "0103060f77fc"}
    ,{"Apple iOS", "017903060f7277fc"}
    ,{"Apple iOS", "017903060f77fc"}
    ,{"Apple iOS", "017903060f6c7277fc"}
    ,{"Android", "012103060f1c333a3b"}
    ,{"Android", "012103060f1c333a3b2b"}
    ,{"Android", "0103060c0f1c28292a"}
    ,{"Android", "012103060f1a1c333a3b"}
    ,{"Android", "012103060c0f1c2a333a3b77"}
    ,{"Android", "012103060c0f1c333a3b77"}
    ,{"Android", "01792103060f1c333a3b77"}
    ,{"Android", "0103060f1a1c333a3b2b"}
    ,{"Android", "0103060f1a1c333a3b"}
    ,{"Android", "012103061c333a3b"}
    ,{"Android", "01031afc2a0f060c"}  // GalaxyWatchActive2-8B3A
    ,{"Apple Mac OS X", "0103060f775ffc2c2e2f"}
    ,{"Apple Mac OS X", "0103060f775ffc2c2e"}
    ,{"Apple Mac OS X", "017903060f77fc5f2c2e"}
    ,{"Apple Mac OS X", "017903060f77fc5f2c2e9f"}
    ,{"Apple Mac OS X", "017903060f7277fc5f2c2e"}
    ,{"Apple Mac OS X", "017903060f7277fc5f2c2e9f"}
    ,{"Windows 8/8.1/10", "010f03062c2e2f1f2179f9fc2b"}
    ,{"Windows 8/8.1/10", "0103060f1f212b2c2e2f79f9fc"}
    ,{"Windows 8/8.1/10", "0103060f1f212b2c2e2f7779f9fc"}
    ,{"Windows 7/Vista/Server 2008", "010f03062c2e2f1f2179f92b"}
    ,{"Windows 7/Vista/Server 2008", "010f03062c2e2f1f2179f92bfc"}
    ,{"Windows XP", "010f03062c2e2f1f21f92b"}
    ,{"Windows Mobile", "0103060f2c2e2f"}
    ,{"PlayStation 3/PSP", "01031c060f"}
    ,{"PlayStation 4", "01030f06"}
    ,{"Nintendo Switch", "0103061c"}
    ,{"Nintendo DS", "010306"}
    ,{"Maemo OS", "0103060c0f111c28292a"}
    ,{"BlackBerry", "0103060f"}
    ,{"Samsung Bada", "010305060c0d0f11171c2a32333536384243"}
    ,{"Roku", "0103060f0c"}
    ,{"Symbian OS", "0c060f01031c78"}
    ,{"Linux", "011c02030f06770c2c2f1a792a79f921fc2a"}
    ,{"Linux", "011c02030f06770c2c2f1a792af921fc"}
    ,{"HarmonyOS", "011c030f06"}
};

vcidb_t vci_db[]={
    {"Windows", "MSFT 5.0"}
    ,{"Android", "android-dhcp"}  //android-dhcp-7.0, android-dhcp-8.1.0 ...
    ,{"PlayStation 4", "PS4"}
};

ouidb_t oui_db[]={
    {"Apple", "Apple iOS", {0x20, 0xa2, 0xe4}}
    ,{"Cadmus Computer Systems", "LINUX", {0x08, 0x00, 0x27}}
    ,{"Canon", "LINUX", {0x00, 0xbb, 0xc1}}
    ,{"Canon", "LINUX", {0x00, 0x1e, 0x8f}}
    ,{"Canon", "LINUX", {0x18, 0x0c, 0xac}}
    ,{"Canon", "LINUX", {0x2c, 0x9e, 0xfc}}
    ,{"Canon", "LINUX", {0x88, 0x87, 0x17}}
    ,{"Canon", "LINUX", {0xf4, 0x81, 0x39}}
    ,{"EnGenius", "LINUX", {0x88, 0xdc, 0x96}}
    ,{"EnGenius", "LINUX", {0x8e, 0xdc, 0x96}}
    ,{"EnGenius", "LINUX", {0x92, 0xdc, 0x96}}
    ,{"EnGenius", "LINUX", {0x96, 0xdc, 0x96}}
    ,{"EnGenius", "LINUX", {0x9a, 0xdc, 0x96}}
    ,{"EnGenius", "LINUX", {0x9e, 0xdc, 0x96}}
    ,{"EnGenius", "LINUX", {0xa2, 0xdc, 0x96}}
    ,{"EnGenius", "LINUX", {0xa6, 0xdc, 0x96}}
    ,{"EnGenius", "LINUX", {0x00, 0x02, 0x6f}}
    ,{"Honeywell International", "Honeywell IoT", {0x00, 0x80, 0xa7}}
    ,{"Honeywell", "Honeywell IoT", {0x00, 0x22, 0x6a}}
};

void dumpIpDataToFile(int gateway_done)
{
    char rets[64] = {0}, rets2[64] = {0};
    char dhcpmac[32] = {0}, gatewaymac[32] = {0};
    char cmd[128] = {0};
    struct list_head *iter;
    struct ip_cfg_list *objPtr = NULL;
    struct in_addr offeraddr, gatewayaddr, maskaddr;
    char offerip[16] = {0}, gatewayip[16] = {0}, maskip[16] = {0};
    char old_md5sum[512] = {0}, new_md5sum[512] = {0};
    FILE *fPtr, *fPtr_gw;

    fp_debug("Dump finger data to %s", FINGERPRINT_IP_FILE_PATH);
    fPtr = fopen(FINGERPRINT_IP_FILE_PATH, "w");
    if (!fPtr)
    {
        fp_debug("create file failed");
        return;
    }
    fp_debug("Dump finger data to %s", FINGERPRINT_GW_TMP_FILE_PATH);
    fPtr_gw = fopen(FINGERPRINT_GW_TMP_FILE_PATH, "w");
    if (!fPtr_gw)
    {
        fp_debug("create file failed");
        return;
    }

    __list_for_each(iter, &iplistHead) {
        objPtr = list_entry(iter, struct ip_cfg_list, list_member);
        offeraddr.s_addr = objPtr->data.ipnodeinfo.offer_ip;
        gatewayaddr.s_addr = objPtr->data.ipnodeinfo.gateway_ip;
        maskaddr.s_addr = objPtr->data.ipnodeinfo.mask_ip;
        inet_ntop(AF_INET, &(offeraddr.s_addr), offerip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(gatewayaddr.s_addr), gatewayip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(maskaddr.s_addr), maskip, INET_ADDRSTRLEN);

        sprintf(dhcpmac, "%02x:%02x:%02x:%02x:%02x:%02x",
                objPtr->data.ipnodeinfo.dhcp_mac[0], objPtr->data.ipnodeinfo.dhcp_mac[1], objPtr->data.ipnodeinfo.dhcp_mac[2],
                objPtr->data.ipnodeinfo.dhcp_mac[3], objPtr->data.ipnodeinfo.dhcp_mac[4], objPtr->data.ipnodeinfo.dhcp_mac[5]);
        sprintf(gatewaymac, "%02x:%02x:%02x:%02x:%02x:%02x",
                objPtr->data.ipnodeinfo.gateway_mac[0], objPtr->data.ipnodeinfo.gateway_mac[1], objPtr->data.ipnodeinfo.gateway_mac[2],
                objPtr->data.ipnodeinfo.gateway_mac[3], objPtr->data.ipnodeinfo.gateway_mac[4], objPtr->data.ipnodeinfo.gateway_mac[5]);

        if(strcmp(offerip, "0.0.0.0") != 0 && memcmp(objPtr->data.ipnodeinfo.gateway_mac, zmac, 6))
        {
            memset(rets, 0, sizeof(rets));
            my_system(rets, sizeof(rets), "ipcalc.sh %s %s |grep PREFIX |awk -F= '{printf $2}'", gatewayip, maskip);
            fprintf(fPtr, MAC_STR_FORMAT"|%s|%s/%s|%s|%s\n",
                            objPtr->data.mac[0], objPtr->data.mac[1], objPtr->data.mac[2],
                            objPtr->data.mac[3], objPtr->data.mac[4], objPtr->data.mac[5],
                            offerip,
                            gatewayip,
                            rets,
                            dhcpmac,
                            gatewaymac);
            fprintf(fPtr_gw, "%s\n%s\n", dhcpmac, gatewaymac);
        }
    }
    fclose(fPtr);
    fclose(fPtr_gw);

    my_system(old_md5sum, sizeof(old_md5sum), "md5sum "FINGERPRINT_GW_FILE_PATH" | awk '{printf $1}'");

    memset(rets, 0, sizeof(rets));
    my_system(rets, sizeof(rets), "uci show wireless | grep l2_isolatior=\\'1\\' | wc -l");
    memset(rets2, 0, sizeof(rets2));
    my_system(rets2, sizeof(rets2), "uci show wireless | grep multi_group_key=\\'1\\' | wc -l");
    if(atoi(rets) > 0 && atoi(rets2))
    {
        sprintf(cmd, "sort "FINGERPRINT_GW_TMP_FILE_PATH" | uniq > "FINGERPRINT_GW_FILE_PATH"");
        system(cmd);
        my_system(new_md5sum, sizeof(new_md5sum), "md5sum "FINGERPRINT_GW_FILE_PATH" | awk '{printf $1}'");
        if(gateway_done == 1 && strcmp(old_md5sum, new_md5sum) != 0)
        {
            system("luci-reload auto l2_isolation");
        }
    }
}

void dumpFingerDataToFile(char *filename, int ip_done)
{
    struct list_head *iter;
    struct finger_cfg_list *objPtr = NULL;
    struct in_addr node_ip_addr;
    FILE *fPtr;

    fp_debug("Dump finger data to %s", filename);
    fPtr = fopen(filename, "w");
    if (!fPtr)
    {
        fp_debug("create file failed");
        return;
    }

    if(ip_done)
    {
        __list_for_each(iter, &maclistHead)
        {
            objPtr = list_entry(iter, struct finger_cfg_list, list_member);
            if(objPtr->data.nodeinfo.isOccupy == 0 && memcmp(objPtr->data.mac, zmac, 6))
            {
                node_ip_addr.s_addr = objPtr->data.nodeinfo.ipaddr;
                fprintf(fPtr, MAC_STR_FORMAT"|%s|%s|%s\n",
                                objPtr->data.mac[0], objPtr->data.mac[1], objPtr->data.mac[2],
                                objPtr->data.mac[3], objPtr->data.mac[4], objPtr->data.mac[5],
                                inet_ntoa(node_ip_addr),
                                objPtr->data.nodeinfo.system,
                                objPtr->data.nodeinfo.device);
            }
        }
    }
    else
    {
        __list_for_each(iter, &maclistHead)
        {
            objPtr = list_entry(iter, struct finger_cfg_list, list_member);
            if(memcmp(objPtr->data.mac, zmac, 6))
            {
                node_ip_addr.s_addr = objPtr->data.nodeinfo.ipaddr;
                fprintf(fPtr, MAC_STR_FORMAT"|%s|%s|%s\n",
                                objPtr->data.mac[0], objPtr->data.mac[1], objPtr->data.mac[2],
                                objPtr->data.mac[3], objPtr->data.mac[4], objPtr->data.mac[5],
                                strlen(inet_ntoa(node_ip_addr))==0?"0.0.0.0":inet_ntoa(node_ip_addr),
                                objPtr->data.nodeinfo.system,
                                objPtr->data.nodeinfo.device);
            }

        }
    }
    fclose(fPtr);
}

void dumpFingerData()
{
    struct list_head *iter;
    struct finger_cfg_list *objPtr = NULL;

    __list_for_each(iter, &maclistHead)
    {
        objPtr = list_entry(iter, struct finger_cfg_list, list_member);
            fp_debug("%s "MAC_STR_FORMAT" %s %s",
                objPtr->data.nodeinfo.isOccupy?"*":" ",
                objPtr->data.mac[0], objPtr->data.mac[1], objPtr->data.mac[2],
                objPtr->data.mac[3], objPtr->data.mac[4], objPtr->data.mac[5],
                objPtr->data.nodeinfo.system,
                objPtr->data.nodeinfo.device);
    }
}

void resetFingerData(void)
{
    struct list_head *iter;
    struct finger_cfg_list *objPtr = NULL;
    struct finger_cfg_list *del_objPtr = NULL;
    uint64_t *tmp_time = NULL;

    __list_for_each(iter, &maclistHead)
    {
        objPtr = list_entry(iter, struct finger_cfg_list, list_member);
        if(tmp_time == NULL)
        {
            if(objPtr != NULL)
            {
                tmp_time = objPtr->data.update_time;
            }
        }
        else
        {
            if(objPtr->data.update_time < tmp_time)
            {
                tmp_time = objPtr->data.update_time;
                del_objPtr = objPtr;
            }
        }
    }
    finger_list_delete(del_objPtr->data.mac, &maclistHead);
}

void resetFingerIpData(void)
{
    struct list_head *iter;
    struct ip_cfg_list *objPtr = NULL;
    struct ip_cfg_list *del_objPtr = NULL;
    uint64_t *tmp_time = NULL;

    __list_for_each(iter, &iplistHead)
    {
        objPtr = list_entry(iter, struct ip_cfg_list, list_member);
        if(tmp_time == NULL)
        {
            if(objPtr != NULL)
            {
                tmp_time = objPtr->data.update_time;
            }
        }
        else
        {
            if(objPtr->data.update_time < tmp_time)
            {
                tmp_time = objPtr->data.update_time;
                del_objPtr = objPtr;
            }
        }
    }
    ip_list_delete(del_objPtr->data.mac, &iplistHead);
}

//Start of finger print related function definition
int lookup_DHCP_DB(char *os_name, int len, char *param_request_list, client_oui_t *client_oui)
{
    int i = 0;

    for(i=0; i<sizeof(fingerprint_db)/sizeof(fingerprint_db[0]); i++)
    {
        if(strcasecmp(fingerprint_db[i].fingerprint, param_request_list) == 0)
        {
            if(strncmp(fingerprint_db[i].os, "Nintendo ", 9) == 0)
            {
                // Nintendo and Canon device have the same fingerprint, check again
                if(client_oui->found == 1)
                {
                    fp_debug("new os_name=%s", client_oui->info.os);
                    snprintf(os_name, len, client_oui->info.os);
                    return 1;
                }
            }

            snprintf(os_name, len, fingerprint_db[i].os);
            fp_debug("os_name=%s", os_name);

            return 1;
        }
    }

    fp_debug("os_name not found");

    snprintf(os_name, len, "LINUX");
    return 0;
}

int lookup_VCI_DB(char *os_name, int len, char *vendor_class_ident)
{
    int i = 0;
    for(i=0; i<sizeof(vci_db)/sizeof(vci_db[0]); i++)
    {
        if(strncmp(vendor_class_ident, vci_db[i].vci, strlen(vci_db[i].vci)) == 0)
        {
            snprintf(os_name, len, vci_db[i].os);
            fp_debug("os_name=%s", os_name);
            return 1;
        }
    }

    fp_debug("os_name not found");

    snprintf(os_name, len, "LINUX");
    return 0;
}

int lookup_OUI_DB(client_oui_t *client_oui, unsigned char *mac)
{
    int i = 0;
    for(i=0; i<sizeof(oui_db)/sizeof(oui_db[0]); i++)
    {
        if(memcmp(oui_db[i].oui, mac, 3) == 0)
        {
            client_oui->found = 1;
            snprintf(client_oui->info.vendor, sizeof(client_oui->info.vendor), oui_db[i].oui_info.vendor);
            snprintf(client_oui->info.os, sizeof(client_oui->info.os), oui_db[i].oui_info.os);

            fp_debug("vendor_name=%s", client_oui->info.vendor);
            fp_debug("os_name=%s", client_oui->info.os);

            return 1;
        }
    }

    fp_debug("os_name not found");

    client_oui->found = 0;
    snprintf(client_oui->info.vendor, sizeof(client_oui->info.vendor), "LINUX");
    snprintf(client_oui->info.os, sizeof(client_oui->info.os), "LINUX");
    return 0;
}

void printFingerDataToFile(char *filename, struct finger_cfg_list *pfinger_list)
{
    struct in_addr node_ip_addr;
    FILE *fPtr;

    if(access(filename, F_OK) || (get_filesize(filename) > 4000000))
    {
        fprintf (stderr, "fingerprint.txt not found or file size > 4000000, auto gen from db\n");
        dumpFingerDataToFile(filename, 1);
    }
    else
    {
        fPtr = fopen(filename, "a");
        if (!fPtr)
        {
            fp_debug("create file failed");
        }
        else
        {
            node_ip_addr.s_addr = pfinger_list->data.nodeinfo.ipaddr;
            fprintf(fPtr, MAC_STR_FORMAT"|%s|%s|%s\n",
                            pfinger_list->data.mac[0], pfinger_list->data.mac[1], pfinger_list->data.mac[2],
                            pfinger_list->data.mac[3], pfinger_list->data.mac[4], pfinger_list->data.mac[5],
                            inet_ntoa(node_ip_addr),
                            pfinger_list->data.nodeinfo.system,
                            pfinger_list->data.nodeinfo.device);
            fclose(fPtr);
        }
    }
}
// Allocate memory for an array of chars.
char * allocate_strmem (int len)
{
    void *tmp;

    if (len <= 0) {
        fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_strmem().\n", len);
        exit (EXIT_FAILURE);
    }

    tmp = (char *) malloc (len * sizeof (char));
    if (tmp != NULL) {
        memset (tmp, 0, len * sizeof (char));
        return (tmp);
    } else {
        fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_strmem().\n");
        exit (EXIT_FAILURE);
    }
}

// Allocate memory for an array of unsigned chars.
uint8_t * allocate_ustrmem (int len)
{
    void *tmp;

    if (len <= 0) {
        fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_ustrmem().\n", len);
        exit (EXIT_FAILURE);
    }

    tmp = (uint8_t *) malloc (len * sizeof (uint8_t));
    if (tmp != NULL) {
        memset (tmp, 0, len * sizeof (uint8_t));
        return (tmp);
    } else {
        fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
        exit (EXIT_FAILURE);
    }
}

// Allocate memory for an array of ints.
int * allocate_intmem (int len)
{
    void *tmp;

    if (len <= 0) {
        fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_intmem().\n", len);
        exit (EXIT_FAILURE);
    }

    tmp = (int *) malloc (len * sizeof (int));
    if (tmp != NULL) {
        memset (tmp, 0, len * sizeof (int));
        return (tmp);
    } else {
        fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_intmem().\n");
        exit (EXIT_FAILURE);
    }
}

int get_filesize(const char *filename)
{
    FILE *fp;
    int filesize = 0;

    if(access(filename, F_OK))
    {
        printf("file not found");
        return 0;
    }

    fp = fopen(filename, "r");
    if(!fp)
    {
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);

    fclose(fp);

    return filesize;
}
void fingerprint_data_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
    int status, bytes;
    int filesize = 0;
    int sizelimit = 10240; //10k ~= 100 lines
    int duplicate = 0;
    int dbupdate = 0;
    uint8_t *recv_ether_frame;
    uint8_t isDhcpRelease = 0, dhcpMsgType = 0;
    uint16_t dhcpOpPointer = 4, j; //skip dhcp magic code
    uint64_t now_time = meponch();
    char *dst_ip;
    char linebuf[1024] = {0};
    char dbg_string[1024] = {0};
    char tmpVal[16] = {0};
    char cmd[128] = {0};
    unsigned char send_mac[6], recv_mac[6], client_mac[6];
    unsigned char buf[MAX_FINGER_DATA_LEN] = {0};
    unsigned char buf2[MAX_IP_DATA_LEN] = {0};
    unsigned char option, optionLen;
    unsigned char vendor_class_ident[DHCP_VCI_BUFFER_SIZE] = {0}, param_request_list[DHCP_PARAM_REQ_LIST_BUFFER_SIZE] = {0};
    unsigned char Hostname[DHCP_HOSTNAME_BUFFER_SIZE] = {0}, OS_name[OS_NAME_BUFFER_SIZE] = {0}, OS_name_prefix[32] = {0};
    unsigned char subnetmask[16] = {0};
    unsigned char gatewayip[16] = {0};
    struct ether_header *recv_ethh;
    struct ip *recv_iphdr;
    struct udphdr *recv_udphdr;
    struct dhcp_packet *recv_dhcp;
    struct in_addr offeraddr, gatewayaddr, maskaddr;
    struct finger_cfg_list finger_list_node;
    struct finger_cfg_list *pfinger_list;
    struct ip_cfg_list ip_list_node;
    struct ip_cfg_list *pip_list;
    struct in_addr node_ip_addr, list_ip_addr;
    struct sockaddr_ll from;
    finger_sync_pkt_t *pbuf;
    pbuf = (finger_sync_pkt_t *)buf;
    ip_sync_pkt_t *ipbuf;
    ipbuf = (ip_sync_pkt_t *)buf2;
    FILE *fptr_dbg;
    socklen_t fromlen;

    recv_ether_frame = allocate_ustrmem (IP_MAXPACKET);
    dst_ip = allocate_strmem (INET_ADDRSTRLEN);
    memset (recv_ether_frame, 0, IP_MAXPACKET * sizeof (uint8_t));
    memset (&from, 0, sizeof (from));
    fromlen = sizeof (from);

    if ((bytes = recvfrom (sock, recv_ether_frame, IP_MAXPACKET, 0, (struct sockaddr *) &from, &fromlen)) < 0)
    {
        // Deal with error conditions first.
        fp_debug("recvfrom error\n");
        // error, break and let procd restart.
        goto end;
    }

    /* fp_debug("sll_pkttype:%d", from.sll_pkttype); */
    /* fp_debug("sll_ifindex:%d", from.sll_ifindex); */

    /*
    if(ifcount > 0)
    {
        for(i=0; i<ifcount; ++i)
        {
            if(from.sll_ifindex == mon_ifindex[i])
            {
                // fp_debug("monitor this interface");
                break;
            }
        }

        if(i==ifcount)
        {
            // fp_debug("skip non monitor interface packet");
            goto end;
        }
    }
    */

#if 0
#define PACKET_HOST     0       /* To us        */
#define PACKET_BROADCAST    1       /* To all       */
#define PACKET_MULTICAST    2       /* To group     */
#define PACKET_OTHERHOST    3       /* To someone else  */
#define PACKET_OUTGOING     4       /* Outgoing of any type */
                    /* These ones are invisible by user level */
#define PACKET_LOOPBACK     5       /* MC/BRD frame looped back */
#define PACKET_FASTROUTE    6       /* Fastrouted frame */
                recv_ethh = (struct ether_header *) (recv_ether_frame);

                if(from.sll_pkttype == PACKET_BROADCAST && !memcmp(recv_ethh->ether_dhost, bmac, 6))
                {
                    fp_debug("skip pkttype PACKET_BROADCAST and dst_mac == bmac packet\n");
                    continue;
                }
#endif

    // Cast recv_iphdr as pointer to IPv4 header within received ethernet frame.
    recv_iphdr = (struct ip *) (recv_ether_frame + ETH_HDRLEN);
    // Case recv_icmphdr as pointer to ICMP header within received ethernet frame.
    recv_udphdr = (struct udphdr *) (recv_ether_frame + ETH_HDRLEN + IP4_HDRLEN);
    recv_dhcp = (struct dhcp_packet *) (recv_ether_frame + ETH_HDRLEN + IP4_HDRLEN + UDP_HDRLEN );

    if((recv_iphdr->ip_p==IPPROTO_UDP) && (memcmp(recv_dhcp -> options, DHCP_OPTIONS_COOKIE, 4)==0)) // check it is udp packet and dhcp packet
    {
        memcpy(client_mac, recv_dhcp->chaddr, 6);
        memcpy(recv_mac, recv_ether_frame, 6);
        memcpy(send_mac, &recv_ether_frame[6], 6);
        if(DebugEnable)
        {
            fp_debug("Received dhcp packet, source="MAC_STR_FORMAT" target="MAC_STR_FORMAT" opCode=%d cmac="MAC_STR_FORMAT,
                    send_mac[0], send_mac[1], send_mac[2], send_mac[3], send_mac[4], send_mac[5],
                    recv_mac[0], recv_mac[1], recv_mac[2], recv_mac[3], recv_mac[4], recv_mac[5],
                    recv_dhcp->op,
                    client_mac[0], client_mac[1], client_mac[2], client_mac[3], client_mac[4], client_mac[5]
                    );
        }
        if(recv_dhcp->op == BOOTREQUEST) //opcode == 1, it's dhcp request, we record the fingerprint(mac / OS) here
        {
            fp_debug("receive bytes=%d", bytes);
            for(dhcpOpPointer = 4; (dhcpOpPointer+DHCP_FIXED_NON_UDP) <= bytes;)
            {
                option = recv_dhcp -> options[dhcpOpPointer];
                if(option == DHO_END)
                {
                    fp_debug("received DHO_END");
                    break;
                }
                dhcpOpPointer++;
                optionLen = recv_dhcp -> options[dhcpOpPointer];
                if(optionLen == 0)
                {
                    fp_debug("optionLen==0");
                    break;
                }
                dhcpOpPointer++;

                fp_debug("option=%d", option);
                switch(option)
                {
                    case DHO_VENDOR_CLASS_IDENTIFIER:
                        for(j=0; j<optionLen; j++)
                        {
                            sprintf(tmpVal, "%c", recv_dhcp -> options[dhcpOpPointer+j]);
                            if(strlen(vendor_class_ident) < DHCP_VCI_BUFFER_SIZE - 1)
                            {
                                strcat(vendor_class_ident, tmpVal);
                            }
                        }
                        fp_debug("VendorClassIdent=%s", vendor_class_ident);
                        break;
                        ;;
                    case DHO_DHCP_PARAMETER_REQUEST_LIST:
                        for(j=0; j<optionLen; j++)
                        {
                            sprintf(tmpVal, "%02x", recv_dhcp -> options[dhcpOpPointer+j]);
                            if(strlen(param_request_list) < DHCP_PARAM_REQ_LIST_BUFFER_SIZE - 2)
                            {
                                strcat(param_request_list, tmpVal);
                            }
                        }
                        fp_debug("Fingerprint=%s", param_request_list);
                        break;
                        ;;
                    case DHO_HOST_NAME:
                        for(j=0; j<optionLen; j++)
                        {
                            sprintf(tmpVal, "%c", recv_dhcp -> options[dhcpOpPointer+j]);
                            if(strlen(Hostname) < DHCP_HOSTNAME_BUFFER_SIZE - 1)
                            {
                                strcat(Hostname, tmpVal);
                            }
                        }
                        fp_debug("Hostname=%s", Hostname);
                        break;
                        ;;
                    case DHO_DHCP_MESSAGE_TYPE:
                        dhcpMsgType = recv_dhcp -> options[dhcpOpPointer];
                        fp_debug("DHCP_MESSAGE_TYPE=%d\n", dhcpMsgType);
                        if(dhcpMsgType == DHCPRELEASE)
                        {
                            isDhcpRelease = 1;
                            break;
                        }
                        break;
                    default:
                        break;
                }
                if(isDhcpRelease == 1)
                    break;

                dhcpOpPointer += optionLen;
                fp_debug("dhcpOpPointer=%d", dhcpOpPointer);
            }

            fp_debug("End of parse dhcp option loop");

            if(isDhcpRelease == 1)
            {
                goto end;
            }

            client_oui_t client_oui;
            memset(&client_oui, 0, sizeof(client_oui));
            lookup_OUI_DB(&client_oui, client_mac);

            if(!lookup_DHCP_DB(OS_name, OS_NAME_BUFFER_SIZE, param_request_list, &client_oui))
            {
                if(!lookup_VCI_DB(OS_name, OS_NAME_BUFFER_SIZE, vendor_class_ident))
                {
                    if(!client_oui.found)
                    {
                        // android device use android- as Hostname prefix
                        if(strncmp(Hostname, "android-", 8) == 0)
                        {
                            snprintf(OS_name, OS_NAME_BUFFER_SIZE, "Android");
                            fp_debug("os_name=%s", OS_name);
                        }

                        /* if(DebugEnable) */
                        {
                            if(DebugEnable)
                                sizelimit = 102400; //100k ~= 1000 lines

                            fp_debug("Found unknown OS!!");

                            filesize = get_filesize(FINGERPRINT_DEBUG_FILE_PATH);
                            if(filesize > sizelimit)
                            {
                                if(access(FINGERPRINT_DEBUG_FILE_PATH".old", F_OK) != -1)
                                {
                                    unlink(FINGERPRINT_DEBUG_FILE_PATH".old");
                                }
                                rename(FINGERPRINT_DEBUG_FILE_PATH, FINGERPRINT_DEBUG_FILE_PATH".old");
                            }

                            // a+ : For glibc, the initial file position for reading is at the beginning of the
                            // file, but for Android/BSD/MacOS, the initial file position for
                            // reading is at the end of the file.
                            fptr_dbg = fopen(FINGERPRINT_DEBUG_FILE_PATH, "a+");
                            if(fptr_dbg)
                            {
                                snprintf(dbg_string, sizeof(dbg_string), MAC_STR_FORMAT"|"MAC_STR_FORMAT"|%s|%s|%s\n",
                                        send_mac[0], send_mac[1], send_mac[2],
                                        send_mac[3], send_mac[4], send_mac[5],
                                        client_mac[0], client_mac[1], client_mac[2],
                                        client_mac[3], client_mac[4], client_mac[5],
                                        param_request_list, vendor_class_ident, Hostname);

                                while(fgets(linebuf, 1024, fptr_dbg))
                                {
                                    /* fp_debug("linebuf:%s", linebuf); */
                                    /* fp_debug("dbg str:%s", dbg_string); */

                                    if(strcmp(linebuf, dbg_string) == 0)
                                    {
                                        duplicate = 1;
                                        break;
                                    }
                                }

                                if(!duplicate)
                                {
                                    fp_debug("Output data to %s", FINGERPRINT_DEBUG_FILE_PATH);
                                    fprintf(fptr_dbg, "%s", dbg_string);
                                }
                                fclose(fptr_dbg);
                            }
                        }
                    }
                }
            }
            if(strlen(Hostname) == 0)
            {
                if(strlen(vendor_class_ident) > 1)
                {
                    snprintf(Hostname, sizeof(Hostname), "%s", vendor_class_ident);
                }
                else
                {
                    if(strncmp(OS_name, "LINUX", 5) == 0 && client_oui.found == 1)
                    {
                        snprintf(OS_name_prefix, sizeof(OS_name_prefix), client_oui.info.vendor);
                    }
                    else
                    {
                        sscanf(OS_name, "%s", OS_name_prefix);
                    }

                    snprintf(Hostname, sizeof(Hostname), "%s%s", OS_name_prefix, "_device");
                }
                fp_debug("Hostname=%s", Hostname);
            }
            pfinger_list = finger_list_find(client_mac, &maclistHead);
            if(!pfinger_list)
            {
                if(node_count >= MAX_FINGERPRINT_RING_SIZE)
                {
                    node_count--;
                    fp_debug("The buffer of fingerData is full. Delete oldest one");
                    resetFingerData();
                }
                fp_debug(""MAC_STR_FORMAT" %s %s\n",
                        client_mac[0], client_mac[1], client_mac[2],
                        client_mac[3], client_mac[4], client_mac[5],
                        OS_name,
                        Hostname);
                memcpy(pbuf->mac, client_mac, 6);
                pbuf->update_time =  now_time;
                memcpy(pbuf->nodeinfo.system, OS_name, OS_NAME_BUFFER_SIZE);
                memcpy(pbuf->nodeinfo.device, Hostname, DHCP_HOSTNAME_BUFFER_SIZE);
                pbuf->nodeinfo.isOccupy = 1 ; //content changed, update information

                finger_list_init_node(&finger_list_node);
                memcpy(&finger_list_node.data, buf, sizeof(finger_sync_pkt_t));
                pfinger_list = finger_list_add_node(&finger_list_node, &maclistHead);
                node_count++;
                dbupdate = 1;
            }
            else
            {
                if(memcmp(pfinger_list->data.mac, client_mac, 6) ||
                        strcmp(pfinger_list->data.nodeinfo.system, OS_name) ||
                        strcmp(pfinger_list->data.nodeinfo.device, Hostname))
                {
                    fp_debug(""MAC_STR_FORMAT" %s %s\n",
                            client_mac[0], client_mac[1], client_mac[2],
                            client_mac[3], client_mac[4], client_mac[5],
                            OS_name,
                            Hostname);
                    memcpy(pfinger_list->data.mac, client_mac, 6);
                    pfinger_list->data.update_time =  now_time;
                    memcpy(pfinger_list->data.nodeinfo.system, OS_name, OS_NAME_BUFFER_SIZE);
                    memcpy(pfinger_list->data.nodeinfo.device, Hostname, DHCP_HOSTNAME_BUFFER_SIZE);
                    pfinger_list->data.nodeinfo.isOccupy = 1 ; //content changed, update information
                    dbupdate = 1;
                }
            }

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
                ipbuf->update_time = now_time;
                ipbuf->ipnodeinfo.vlan_id = 0;
                ipbuf->ipnodeinfo.isOccupy = 1;
                ip_list_init_node(&ip_list_node);
                memcpy(&ip_list_node.data, buf2, sizeof(ip_sync_pkt_t));
                pip_list = ip_list_add_node(&ip_list_node, &iplistHead);
            }
        }
        else if(recv_dhcp->op == BOOTREPLY) //opcode==2, it's dhcp reply, we append the ip address here
        {
            if (inet_ntop (AF_INET, &recv_dhcp->yiaddr.s_addr, dst_ip, INET_ADDRSTRLEN) == NULL) {
                status = errno;
                fprintf (stderr, "inet_ntop() failed.\nError message: %s", strerror(status));
                exit (EXIT_FAILURE);
            }

            if(strcmp(dst_ip, "0.0.0.0") == 0)  //this reply didn't contain offer IP, no use
            {
                goto end;
            }

            fp_debug("client_mac="MAC_STR_FORMAT", dst_ip=%s\n",
                    client_mac[0], client_mac[1], client_mac[2],
                    client_mac[3], client_mac[4], client_mac[5],
                    dst_ip);

            pfinger_list = finger_list_find(client_mac, &maclistHead);
            if(pfinger_list)
            {
                node_ip_addr.s_addr = pfinger_list->data.nodeinfo.ipaddr;
                if(pfinger_list->data.nodeinfo.isOccupy == 1 || strcmp(inet_ntoa(node_ip_addr), dst_ip))
                {
                    pfinger_list->data.update_time = now_time;
                    inet_pton(AF_INET, dst_ip, &(node_ip_addr.s_addr));
                    pfinger_list->data.nodeinfo.ipaddr = node_ip_addr.s_addr;
                    pfinger_list->data.nodeinfo.isOccupy = 0;
                    printFingerDataToFile(FINGERPRINT_FILE_PATH, pfinger_list);
                    dbupdate = 1;
                }
                else
                {
                    if(access(FINGERPRINT_FILE_PATH, F_OK))
                    {
                        dumpFingerDataToFile(FINGERPRINT_FILE_PATH, 1);
                    }
                    if(access(FINGERPRINT_DB_FILE_PATH, F_OK))
                    {
                        dumpFingerDataToFile(FINGERPRINT_DB_FILE_PATH, 0);
                    }
                }
            }

            for(dhcpOpPointer = 4; (dhcpOpPointer+DHCP_FIXED_NON_UDP) <= bytes;)
            {
                option = recv_dhcp -> options[dhcpOpPointer];
                if(option == DHO_END)
                {
                    fp_debug("received DHO_END");
                    break;
                }
                dhcpOpPointer++;
                optionLen = recv_dhcp -> options[dhcpOpPointer];
                if(optionLen == 0)
                {
                    fp_debug("optionLen==0");
                    break;
                }
                dhcpOpPointer++;

                fp_debug("option=%d", option);
                switch(option)
                {
                    case DHO_SUBNET_MASK:
                        sprintf(subnetmask, "%d.%d.%d.%d", recv_dhcp -> options[dhcpOpPointer], recv_dhcp -> options[dhcpOpPointer+1], recv_dhcp -> options[dhcpOpPointer+2], recv_dhcp -> options[dhcpOpPointer+3]);
                        inet_pton(AF_INET, subnetmask, &(maskaddr.s_addr));
                        fp_debug("subnetmask=%s", inet_ntoa(maskaddr));
                        break;
                    case DHO_ROUTERS:
                        sprintf(gatewayip, "%d.%d.%d.%d", recv_dhcp -> options[dhcpOpPointer], recv_dhcp -> options[dhcpOpPointer+1], recv_dhcp -> options[dhcpOpPointer+2], recv_dhcp -> options[dhcpOpPointer+3]);
                        inet_pton(AF_INET, gatewayip, &(gatewayaddr.s_addr));
                        fp_debug("gatewayip=%s", inet_ntoa(gatewayaddr));
                        break;
                    default:
                        break;
                }
                dhcpOpPointer += optionLen;
                fp_debug("dhcpOpPointer=%d", dhcpOpPointer);
            }
            pip_list = ip_list_find(client_mac, &iplistHead);
            if(pip_list)
            {
                list_ip_addr.s_addr = pip_list->data.ipnodeinfo.offer_ip;
                if(pip_list->data.ipnodeinfo.isOccupy == 1 || strcmp(inet_ntoa(list_ip_addr), dst_ip))
                {
                    pip_list->data.update_time = now_time;
                    inet_pton(AF_INET, dst_ip, &(offeraddr.s_addr));
                    pip_list->data.ipnodeinfo.offer_ip = offeraddr.s_addr;
                    pip_list->data.ipnodeinfo.mask_ip = maskaddr.s_addr;
                    pip_list->data.ipnodeinfo.gateway_ip = gatewayaddr.s_addr;
                    memcpy(pip_list->data.ipnodeinfo.dhcp_mac, send_mac, 6);
                    pip_list->data.ipnodeinfo.isOccupy = 0;

                    fp_debug("vlan_id=%d", pip_list->data.ipnodeinfo.vlan_id);
                    if(pip_list->data.ipnodeinfo.vlan_id == 0)
                    {
                        sprintf(cmd, "arp_scan.sh "MAC_STR_FORMAT" %s &",
                        client_mac[0], client_mac[1], client_mac[2], client_mac[3], client_mac[4], client_mac[5],
                        gatewayip);
                    }
                    else
                    {
                        sprintf(cmd, "arp_scan.sh "MAC_STR_FORMAT" %s %d &",
                        client_mac[0], client_mac[1], client_mac[2], client_mac[3], client_mac[4], client_mac[5],
                        gatewayip,
                        pip_list->data.ipnodeinfo.vlan_id);
                    }
                    system(cmd);
                }
                else
                {
                    if(access(FINGERPRINT_IP_FILE_PATH, F_OK) && access(FINGERPRINT_GW_TMP_FILE_PATH, F_OK))
                    {
                        dumpIpDataToFile(0);
                    }
                }
            }
        }
        if(dbupdate)
        {
            dumpFingerDataToFile(FINGERPRINT_DB_FILE_PATH, 0);
        }
    }

    goto end;

end:
    // Free allocated memory.
    free (recv_ether_frame);
    free (dst_ip);
    return;
}
