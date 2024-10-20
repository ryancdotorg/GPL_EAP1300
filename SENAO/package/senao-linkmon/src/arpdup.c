/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                     *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  linkmon                                                                       *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#include <stdio.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <arpa/inet.h> // uint_8_t
#include <netpacket/packet.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "mio.h"
#include "arpdup.h" // TODO: cannot after net/if.h, why?
#include "util.h"
#include <net/if.h>

int arpdup_init(void *mdata, void *data);
void arpdup_close(void *data);
void arpdup_send(void *data);
void arpdup_recv(void *data);
int arpdup_status(void *data);
char *arpdup_dupmac(void *data);

static monitor_arpdup_t monitor_arpdup =
{
	.name = "arpdup",
	.sec = 5,
	.usec = 0,
	.lm_initfunc = arpdup_init,
	.lm_sendfunc = arpdup_send,
	.lm_recvfunc = arpdup_recv,
	.lm_statusfunc = arpdup_status,
	.lm_msgfunc = arpdup_dupmac,
	.lm_uninitfunc = arpdup_close,
	.destIF = "br-lan",
};

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
	lmdbg("------------------\n");
	lmdbg("ether_header --- \n" \
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
	lmdbg("arp_header --- \n" \
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
	lmdbg("------------------\n");
}
int arpdup_init(void *mdata, void *data)
{
	if (mdata == NULL || data == NULL){
		lmdbg("%s(%d) init error\n", __func__, __LINE__);
		return -1;
	}
	mio_data_t *mio_data;
	arpdup_data_t *arpdupdata;
	mio_data = (mio_data_t *) mdata;
	arpdupdata = (arpdup_data_t *)data;

	arpdupdata->fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (arpdupdata->fd < 0)
	{
		arpdupdata->status = ARP_CREATE_ERR; 
		perror("socket error:");
		lmdbg("socket error:");
		return -1;
	}
	if (monitor_arpdup.lm_recvfunc){
		mio_event_t *me;
		me = add_event(&(mio_data->read_pool), arpdupdata->fd, (event_func)monitor_arpdup.lm_recvfunc, (void *)arpdupdata);
	}
	else {
		arpdupdata->status = ARP_CREATE_ERR; 
		return -1;
	}
	if (monitor_arpdup.lm_sendfunc){
		mio_timer_t *mt;
		mt = add_timer(&(mio_data->timer_pool), monitor_arpdup.sec, monitor_arpdup.usec, (timer_func)monitor_arpdup.lm_sendfunc, (void *)arpdupdata, 1);
	}
	else {
		arpdupdata->status = ARP_CREATE_ERR; 
		return -1;
	}
	return 0;
}

void arpdup_close(void *data)
{
	if (data == NULL){
		lmdbg("icmp_close, data not initialized\n");
	}
	arpdup_data_t *arpdupdata;
	mio_event_t *mio_event;
	mio_timer_t *mio_timer;
	arpdupdata = (arpdup_data_t *) data;
	mio_event = (mio_event_t *)arpdupdata->mio_event;
	mio_timer = (mio_timer_t *)arpdupdata->mio_timer;
	if (mio_timer != NULL)
		remove_timer(mio_timer);;
	if (mio_event != NULL)
		remove_event(mio_event);

	if (arpdupdata->fd > 0)
	{
		close(arpdupdata->fd);
		arpdupdata->fd = 0;
	}
}

void arpdup_send(void *data)
{
	int fd, if_fd;
	struct ifreq ifr;
	int if_index;
	if (data == NULL){
		lmdbg("send data broken error\n");
		return;
	}
	arpdup_data_t *arpdupdata;
	arpdupdata = (arpdup_data_t *)data;
	lmdbg("socket fd set\n");

	if_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (if_fd < 0){
		perror("interface socket error:");
		lmdbg("interface socket error:");
		return;
	}

	memcpy(ifr.ifr_name, arpdupdata->if_name, IF_NAMESIZE);
	if (ioctl(if_fd, SIOCGIFADDR, &ifr, sizeof(ifr)) < 0){
		perror("SIOCGIFADDR:");
		lmdbg("SIOCGIFADDR:");
		close(if_fd);
		return;	
	}

	struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
	arpdupdata->if_ip = sin->sin_addr.s_addr;
	
	if (ioctl(if_fd, SIOCGIFINDEX, &ifr, sizeof(ifr)) < 0){
		perror("SIOCGIFINDEX:");
		lmdbg("SIOCGIFINDEX:");
		close(if_fd);
		return;	
	}
	if_index = ifr.ifr_ifindex;

	if (ioctl(if_fd, SIOCGIFHWADDR, &ifr, sizeof(ifr)) < 0){
		perror("SIOCGIFHWADDR:");
		lmdbg("SIOCGIFHWADDR:");
		close(if_fd);
		return;	
	}
	memcpy(arpdupdata->if_mac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

	close(if_fd);

	lmdbg("socket create\n");
	fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (fd < 0)
	{
		arpdupdata->status = ARP_CREATE_ERR; 
		perror("socket error:");
		lmdbg("socket error:");
		return;
	}
	
	struct arp_packet_t pkt;
	memset(&pkt, 0, sizeof(struct arp_packet_t));

	memset(pkt.ether.ether_dhost, 0xFF, sizeof(pkt.ether.ether_dhost)); 
	memcpy(pkt.ether.ether_shost, arpdupdata->if_mac, sizeof(pkt.ether.ether_shost));
	pkt.ether.ether_type = htons(ETHERTYPE_ARP);

	pkt.arp.ar_hrd = htons(ARPHRD_ETHER);	
	pkt.arp.ar_pro = htons(ETHERTYPE_IP);
	pkt.arp.ar_hln = ETHER_ADDR_LEN;	// 06
	pkt.arp.ar_pln = sizeof(pkt.spa); // 04
	pkt.arp.ar_op = htons(ARPOP_REQUEST);	// 00 01

	memcpy(pkt.sha, arpdupdata->if_mac, sizeof(pkt.sha));
	//pkt.spa = htonl(if_ip);
	pkt.spa = 0;
	memset(pkt.tha, 0xFF, sizeof(pkt.tha)); 
	pkt.tpa = htonl(arpdupdata->if_ip);
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

	lmdbg("packet send\n");
	if (sendto(fd, &pkt, sizeof(pkt), 0, (struct sockaddr *) ll, sizeof(*ll)) < 0)
	{
		perror("sendto error:");
		lmdbg("sendto error:");
		arpdupdata->status = ARP_SEND_ERR;
		close(fd);
		return;
	}
	close(fd);
	arpdupdata->status = STATUS_OK; // suppose no duplicate before test
	memset(arpdupdata->msg, 0, sizeof(arpdupdata->msg));
	memset(arpdupdata->dup_mac, 0, sizeof(arpdupdata->dup_mac));
	return;
}
void arpdup_recv(void *data)
{
	int fd;
	struct arp_packet_t pkt;
	struct sockaddr_ll sa;                                         
	socklen_t salen;
	arpdup_data_t *arpdupdata;
	if (data == NULL){
		arpdupdata->status = ARP_DATA_ERR;
		return;
	}
	arpdupdata = (arpdup_data_t *)data;
	fd = arpdupdata->fd;
	if (fd <= 0){
		arpdupdata->status = ARP_RCV_ERR;
		return;
	}

	memset(&pkt, 0, sizeof(pkt));
	memset(&sa, 0, sizeof(sa));
	salen = sizeof(struct sockaddr_ll);
	if(recvfrom(fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&sa, &salen) <= 0){
		arpdupdata->status = ARP_RCV_ERR;
		return;
	}
	if (memcmp(pkt.tha,arpdupdata->if_mac, ETH_ALEN)){
		return; // filterred, not my packet
	}

	switch(ntohs(pkt.arp.ar_op)){
		case ARPOP_REQUEST:             
			break;
		case ARPOP_REPLY:
			print_arp_packet(&pkt);
			if (htonl(arpdupdata->if_ip) == pkt.spa &&
				memcmp(pkt.sha, arpdupdata->if_mac, ETH_ALEN))
				arpdupdata->status = ARP_DUPLICATE; 
				memcpy(arpdupdata->dup_mac,pkt.sha, ETH_ALEN);
			break;
		default:
			break;
	
	}
}
int arpdup_status(void *data)
{
	if (data == NULL){
		return ARP_DATA_ERR;
	}
	arpdup_data_t *arpdupdata;
	arpdupdata = (arpdup_data_t *)data;
	return arpdupdata->status;
}
char *arpdup_dupmac(void *data)
{
	if (data == NULL){
		return NULL;
	}
	arpdup_data_t *arpdupdata;
	arpdupdata = (arpdup_data_t *)data;
	sprintf(arpdupdata->msg, "%02X:%02X:%02X:%02X:%02X:%02X", 
			arpdupdata->dup_mac[0],
			arpdupdata->dup_mac[1],
			arpdupdata->dup_mac[2],
			arpdupdata->dup_mac[3],
			arpdupdata->dup_mac[4],
			arpdupdata->dup_mac[5]);
	return arpdupdata->msg;

}
void init_arpdup(linkmon_t *linkmon_pool, char *wan_ifname)
{
	arpdup_data_t *arpdupdata;
	arpdupdata = (arpdup_data_t *)calloc(1,sizeof(arpdup_data_t));
	if(!arpdupdata) {
		perror("calloc");
		lmdbg("calloc");
		return;
	}
	if (*wan_ifname != '\0')
		strcpy(arpdupdata->if_name, wan_ifname);
	else
		strcpy(arpdupdata->if_name, monitor_arpdup.destIF);
	arpdupdata->status = LINK_INIT;

	monitor_arpdup.linkmon = add_linkmon(linkmon_pool, monitor_arpdup.name, monitor_arpdup.sec, monitor_arpdup.usec, monitor_arpdup.lm_initfunc, monitor_arpdup.lm_sendfunc, monitor_arpdup.lm_recvfunc, monitor_arpdup.lm_statusfunc, monitor_arpdup.lm_msgfunc, monitor_arpdup.lm_uninitfunc, (void *)arpdupdata);
	
}

void uninit_arpdup(linkmon_t *linkmon_pool)
{
	
	linkmon_t *linkmon;
	if (!linkmon_pool) 
		return;

//	linkmon = find_linkmon_by_name(linkmon_pool, monitor_icmp.name); 
	linkmon = monitor_arpdup.linkmon;
	if (linkmon == NULL)
		return;

	if (linkmon->data != NULL){ // arpdup_data_t
		free(linkmon->data);
		linkmon->data = NULL;
	}
	remove_linkmon(linkmon_pool, linkmon);
	monitor_arpdup.linkmon = NULL;
	
}


/*
int main(int argc, char **argv)
{
	int sockfd;
	sockfd = create_socket_icmpv4();
	in_addr_t addr;
	addr = inet_addr(argv[1]);
	unsigned char *ipBytes;
	ipBytes = (unsigned char *)&addr;
	send_icmpv4(sockfd, ipBytes);

	return 0;
}
*/
