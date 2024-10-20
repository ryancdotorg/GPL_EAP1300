/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                    *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  syseye                                                                        *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <arpa/inet.h> // uint_8_t
#include <netpacket/packet.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "arpdup.h"
#include <net/if.h>
#include "statuscode.h"

#define SYSEYE_ARPDUP_FILENAME "/tmp/syseye_arpdup.txt"

struct arp_packet_t{
    struct ether_header ether;
    struct arphdr arp;
    uint8_t sha[ETH_ALEN]; // sender hardware address
    uint32_t spa; // sender protocol address
    uint8_t tha[ETH_ALEN]; // target hardware address
    uint32_t tpa;	// target protocol address
    uint8_t padding[18];
} __attribute__ ((__packed__));

void print_arp_packet(struct arp_packet_t *pkt)
{
    printf("------------------\n");
    printf("ether_header --- \n" \
"ether_dhost %02X:%02X:%02X:%02X:%02X:%02X\n" \
"ether_shost %02X:%02X:%02X:%02X:%02X:%02X\n" \
"ether_type %d\n",
        (pkt->ether).ether_dhost[0],
        (pkt->ether).ether_dhost[1],
        (pkt->ether).ether_dhost[2],
	(pkt->ether).ether_dhost[3],
	(pkt->ether).ether_dhost[4],
	(pkt->ether).ether_dhost[5],
	(pkt->ether).ether_shost[0],
	(pkt->ether).ether_shost[1],
	(pkt->ether).ether_shost[2],
	(pkt->ether).ether_shost[3],
	(pkt->ether).ether_shost[4],
	(pkt->ether).ether_shost[5],
	(pkt->ether).ether_type);
        printf("arp_header --- \n" \
"hw type %d proto type %d hlen %d plen %d opcode %d\n"\
"sha %02X:%02X:%02X:%02X:%02X:%02X\n" \
"spa %u.%u.%u.%u\n" 
"tha %02X:%02X:%02X:%02X:%02X:%02X\n" \
"tpa %u.%u.%u.%u\n",
            ntohs((pkt->arp).ar_hrd),
	    ntohs((pkt->arp).ar_pro),
	    ntohs((pkt->arp).ar_hln),
	    ntohs((pkt->arp).ar_pln),
	    ntohs((pkt->arp).ar_op),
	    pkt->sha[0],
	    pkt->sha[1],
	    pkt->sha[2],
	    pkt->sha[3],
	    pkt->sha[4],
	    pkt->sha[5],
	    pkt->spa & 0xFF,
	    (pkt->spa>>8)&0xFF,
	    (pkt->spa >>16)&0xFF,
	    (pkt->spa >>24)&0xFF,
	    pkt->tha[0],
	    pkt->tha[1],
	    pkt->tha[2],
	    pkt->tha[3],
	    pkt->tha[4],
	    pkt->tha[5],
	    pkt->tpa & 0xFF,
	    (pkt->tpa>>8)&0xFF,
	    (pkt->tpa >>16)&0xFF,
	    (pkt->tpa >>24)&0xFF
        );
        printf("------------------\n");
}

int check_arpdup_status(char *ifname)
{
    int status = LINK_INIT;
    int fd, if_fd;
    struct ifreq ifr;
    int if_index;
    uint8_t if_mac[ETH_ALEN];
    uint32_t if_ip;
    struct arp_packet_t pkt;
    struct sockaddr_ll sa;
    socklen_t salen;
    fd_set readfds;
    struct timeval tv;
    int nselect = 1;
    
    if (ifname == NULL){
        status = ARP_DATA_ERR; 
        return status;
    }

    if ((if_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("interface socket error:");
        status = ARP_CREATE_ERR;
        return status;
    }
    memcpy(ifr.ifr_name, ifname, IF_NAMESIZE);
    if (ioctl(if_fd, SIOCGIFADDR, &ifr, sizeof(ifr)) < 0){
        perror("SIOCGIFADDR:");
        close(if_fd);
        status = ARP_CREATE_ERR;
        return status;
    }
    struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
    if_ip = sin->sin_addr.s_addr; // s_addr is network order

    if (ioctl(if_fd, SIOCGIFINDEX, &ifr, sizeof(ifr)) < 0){
        perror("SIOCGIFINDEX:");
	close(if_fd);
	status = ARP_CREATE_ERR;
	return status;
    }
    if_index = ifr.ifr_ifindex;

    if (ioctl(if_fd, SIOCGIFHWADDR, &ifr, sizeof(ifr)) < 0){
        perror("SIOCGIFHWADDR:");
        close(if_fd);
        status = ARP_CREATE_ERR;
        return status;
    }
    memcpy(if_mac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    close(if_fd);

    if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) < 0) {
        perror("socket error:");
        status = ARP_CREATE_ERR;
        return status;
    }

    memset(&pkt, 0, sizeof(struct arp_packet_t));

    memset(pkt.ether.ether_dhost, 0xFF, sizeof(pkt.ether.ether_dhost));
    memcpy(pkt.ether.ether_shost, if_mac, sizeof(pkt.ether.ether_shost));
    pkt.ether.ether_type = htons(ETHERTYPE_ARP);
    pkt.arp.ar_hrd = htons(ARPHRD_ETHER);
    pkt.arp.ar_pro = htons(ETHERTYPE_IP);
    pkt.arp.ar_hln = ETHER_ADDR_LEN;	// 06
    pkt.arp.ar_pln = sizeof(pkt.spa); // 04
    pkt.arp.ar_op = htons(ARPOP_REQUEST);	// 00 01
    status = STATUS_OK;

    memcpy(pkt.sha, if_mac, sizeof(pkt.sha));
    pkt.spa = 0;
    memset(pkt.tha, 0xFF, sizeof(pkt.tha));
    pkt.tpa = if_ip;
    memset(pkt.padding, 0 , sizeof(pkt.padding));

    struct sockaddr_storage storage;
    struct sockaddr_ll *ll;
    memset(&storage, 0, sizeof(storage));
    ll = (struct sockaddr_ll *) &storage;
    ll->sll_family		= AF_PACKET;
    ll->sll_protocol	= htons(ETH_P_ARP);
    ll->sll_ifindex		= if_index;
    ll->sll_hatype		= ARPHRD_ETHER;
    ll->sll_pkttype		= PACKET_BROADCAST;
    ll->sll_halen		= 0;
    //print_arp_packet(&pkt);
   if (sendto(fd, &pkt, sizeof(pkt), 0, (struct sockaddr *) ll, sizeof(*ll)) < 0)
    {
        perror("sendto error:");
	close(fd);
	status = ARP_SEND_ERR;
	return status;
    }

    FD_ZERO(&readfds); FD_SET(fd, &readfds);
    tv.tv_sec = 3;  tv.tv_usec = 0;
    while (1){
        nselect = select(fd + 1, &readfds, 
            NULL, NULL, &tv);
        if (nselect > 0){
            memset(&pkt, 0, sizeof(pkt));
	    memset(&sa, 0, sizeof(sa));
	    salen = sizeof(struct sockaddr_ll);
	    if(recvfrom(fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&sa, &salen) <= 0)
                continue;
            //print_arp_packet(&pkt);
	    if (memcmp(pkt.tha, if_mac, ETH_ALEN))
                continue; // filterred, not my packet
            if (ntohs(pkt.arp.ar_op) == ARPOP_REPLY &&
                if_ip == pkt.spa &&
                memcmp(pkt.sha, if_mac, ETH_ALEN)){
                //printf("address duplicate\n");
		status = ARP_DUPLICATE;
		{
			FILE *fp_arpdup = fopen(SYSEYE_ARPDUP_FILENAME, "w");
			struct in_addr tmpaddr;
			char *tmp_ip_str = NULL;

			tmpaddr.s_addr = if_ip;
			tmp_ip_str = inet_ntoa(tmpaddr);
			if (fp_arpdup)
			{
				fprintf(fp_arpdup, "%s conflict with %02X:%02X:%02X:%02X:%02X:%02X",
						tmp_ip_str ? tmp_ip_str : "NO IP",
						pkt.sha[0], pkt.sha[1], pkt.sha[2], pkt.sha[3], pkt.sha[4], pkt.sha[5]);
				fclose(fp_arpdup);
			}
		}

		break;
            }
        }
        else if (nselect == 0){ // timeout without duplicated mac arp reply
            status = STATUS_OK;
            break;  
       }
    }
    close(fd);
    return status;
}
