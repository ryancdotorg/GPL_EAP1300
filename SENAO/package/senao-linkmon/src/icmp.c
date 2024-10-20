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
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "mio.h"
#include "icmp.h"
#include "util.h"

int icmp_init(void *mdata, void *idata);
void icmp_close(void *data);
void icmp_send(void *data);
void icmp_recv(void *data);
int icmp_status(void *data);
char *icmp_getmsg(void *data);

static monitor_icmp_t monitor_icmp =
{
	.name = "icmp",
	.sec = 5,
	.usec = 0,
	.lm_initfunc = icmp_init,
	.lm_sendfunc = icmp_send,
	.lm_recvfunc = icmp_recv,
	.lm_statusfunc = icmp_status,
	.lm_msgfunc = icmp_getmsg,
	.lm_uninitfunc = icmp_close,
	.destIPstr = "8.8.8.8"
};

const int DEFDATALEN = 56;
const int MAXIPLEN = 60;
const int MAXICMPLEN = 76;

static int short icmp_checksum (unsigned short *buf, int bytes)
{
    unsigned int i;
    unsigned short  *p = buf;
    unsigned short sum = 0;

    /* Sum */
    for (i=bytes; i > 1; i -= 2)
        sum += *p++;

    /* If uneven length */
    if (i > 0)
        sum += *((unsigned char *) (p));

    /* Carry */
    while ((sum & 0xFFFF0000) != 0)
        sum = (sum >> 16) + (sum & 0xFFFF);

    return ~(sum);
}

void icmp_clean_rcv(void *data)
{
	if (data == NULL){
		lmdbg("icmp_clean_rcv, data not initialized\n");
		return;
	}
	icmp_data_t *icmpdata;
	icmpdata = (icmp_data_t *)data;
	// receive timeout
	if (icmpdata->mio_event_read){
		remove_event(icmpdata->mio_event_read);
		icmpdata->mio_event_read = NULL;
	}
	close(icmpdata->fd);
	icmpdata->fd = 0;
	icmpdata->status = ICMP_NO_RESPONSE;
}

int icmp_init(void *mdata, void *data)
{
	if (mdata == NULL || data == NULL){
		lmdbg("init error\n");
		return -1;
	}
	mio_data_t *mio_data;
	icmp_data_t *icmpdata;
	mio_data = (mio_data_t *) mdata;
	icmpdata = (icmp_data_t *)data;
	icmpdata->mio_data = (void *) mio_data;

	if (monitor_icmp.lm_sendfunc){
		mio_timer_t *mt;
		mt = add_timer(&(mio_data->timer_pool), monitor_icmp.sec, monitor_icmp.usec, (timer_func)monitor_icmp.lm_sendfunc, (void *)icmpdata, 1);
		icmpdata->mio_timer_snd = (void *) mt;
		return 0;
	}
	else {
		icmpdata->status = ICMP_CREATE_ERR;
		return -1;
	}
}

void icmp_close(void *data)
{
	if (data == NULL){
		lmdbg("icmp_close, data not initialized\n");
		return;
	}
	icmp_data_t *icmpdata;
	mio_event_t *mio_event;
	mio_timer_t *mio_timer;

	icmpdata = (icmp_data_t *) data;
	mio_timer = (mio_timer_t *)icmpdata->mio_timer_snd;
	if (mio_timer != NULL){
		remove_timer(mio_timer);;
		icmpdata->mio_timer_snd = NULL;
	}

	if (icmpdata->fd > 0)
	{
		close(icmpdata->fd);
		icmpdata->fd = 0;
	}
}

void icmp_send(void *data)
{
	if (data == NULL){
		lmdbg("send data broken error\n");
		return;
	}
	int fd;
	icmp_data_t *icmpdata;
	mio_data_t *mio_data;
	icmpdata = (icmp_data_t *)data;
	mio_data = (mio_data_t *)icmpdata->mio_data;

	if (icmpdata->fd > 0){
		lmdbg("previous fd job not finished, ignored. fd:[%d]\n", icmpdata->fd);
		return;
	}
	// initial send request
	if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) <= 0){
		icmpdata->status = ICMP_SEND_ERR;
		perror("socket error:");
		return;
	}
	unsigned char *ipBytes;
	ipBytes = icmpdata->ipBytes;

	struct sockaddr_in destaddr;
	struct icmp *pkt;
	char sendpacket[DEFDATALEN + MAXIPLEN + MAXICMPLEN];

	memset(&destaddr, 0,sizeof(struct sockaddr_in));
	memset(sendpacket, 0, sizeof(sendpacket));

	// Set IP msg 
	destaddr.sin_family = AF_INET;
	memcpy(&destaddr.sin_addr, ipBytes, 4);

	// Set ICMP msg 
	pkt = (struct icmp*)sendpacket;
	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_cksum = icmp_checksum((unsigned short *)pkt, sizeof(struct icmp));
	if (sendto(fd, sendpacket, sizeof(sendpacket), 0, (struct sockaddr *)&destaddr, sizeof(destaddr)) == -1)
	{
		lmdbg("sendto error\n");
		icmpdata->status = ICMP_SEND_ERR;
		close(fd);
		return;
	}
	close(fd);
	// register receive and receive timeout
	mio_event_t *er;
	mio_timer_t *mt_rcv;
	// create rcv fd
	fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (fd < 0)
	{
		icmpdata->status = ICMP_CREATE_ERR;
		perror("socket error:");
		return;
	}
	icmpdata->fd = fd;

	er = add_event(&(mio_data->read_pool), icmpdata->fd, (event_func)monitor_icmp.lm_recvfunc, (void *)icmpdata);
	icmpdata->mio_event_read=(void *)er;

	mt_rcv = add_timer(&(mio_data->timer_pool), 3, 0, icmp_clean_rcv, (void *)icmpdata, 0);
	icmpdata->mio_timer_rcv = mt_rcv;
	return;
}
void icmp_recv(void *data)
{
	if (data == NULL){
		lmdbg("icmp_recv, data not initialized\n");
		return;
	}
	icmp_data_t *icmpdata;
	mio_event_t *mio_event;
	mio_timer_t *mio_timer_rcv;
	icmpdata = (icmp_data_t *)data;
	mio_event = (mio_event_t *)icmpdata->mio_event_read;
	mio_timer_rcv = (mio_timer_t *)icmpdata->mio_timer_rcv;
	if (icmpdata->fd <= 0){
		icmpdata->status = ICMP_RCV_ERR;
		return;
	}
        int res = -1;
	char PDU[DEFDATALEN + MAXIPLEN + MAXICMPLEN];
	struct sockaddr_in from;
	socklen_t fromlen;
	unsigned char ipBytes[4];
	struct iphdr *iphdr;
	struct icmp *pkt;

        memset(&from, 0, sizeof(struct sockaddr_in));
	fromlen = sizeof(from);
	res= recvfrom (icmpdata->fd, PDU, sizeof(PDU), 0,
			(struct sockaddr *) &from, &fromlen);
	//DumpHex(PDU, res);
	if(res == -1 || res<76) {
		icmpdata->status = ICMP_RCV_ERR;
		return;
	}

	if (res >= 76) {			/* ip + icmp */
            memcpy(&(ipBytes[0]), &from.sin_addr, 4);
	    iphdr = (struct iphdr *) PDU;
	    pkt = (struct icmp *) (PDU + (iphdr->ihl << 2));
	    if (pkt->icmp_type == ICMP_ECHOREPLY) {
		icmpdata->status = STATUS_OK;
		close(icmpdata->fd);
		icmpdata->fd = 0;
		remove_event(mio_event);
		icmpdata->mio_event_read = NULL;

		// remove clean_conn timer
		if (mio_timer_rcv){
			remove_timer(mio_timer_rcv);
			icmpdata->mio_timer_rcv = NULL;
		}
		return;
	    }
	}
}
int icmp_status(void *data)
{
	icmp_data_t *icmpdata;
	if (data == NULL){
		return ICMP_DATA_ERR;
	}
	icmpdata = (icmp_data_t *)data;
	return icmpdata->status;
}

char *icmp_getmsg(void *data)
{
	return "";
}

void init_icmp(linkmon_t *linkmon_pool, char *ipaddr)
{
	icmp_data_t *icmpdata;
	icmpdata = (icmp_data_t *)calloc(1,sizeof(icmp_data_t));
	if(!icmpdata) {
		perror("calloc");
		return;
	}
	in_addr_t addr;
	if (*ipaddr != '\0')
		addr = inet_addr(ipaddr);
	else
		addr = inet_addr(monitor_icmp.destIPstr);
	memcpy(icmpdata->ipBytes, (unsigned char *)&addr, 4);
	icmpdata->status = LINK_INIT;

	monitor_icmp.linkmon = add_linkmon(linkmon_pool, monitor_icmp.name, monitor_icmp.sec, monitor_icmp.usec, monitor_icmp.lm_initfunc, monitor_icmp.lm_sendfunc, monitor_icmp.lm_recvfunc, monitor_icmp.lm_statusfunc, monitor_icmp.lm_msgfunc, monitor_icmp.lm_uninitfunc, (void *)icmpdata);
}

void uninit_icmp(linkmon_t *linkmon_pool)
{
	linkmon_t *linkmon;
	if (!linkmon_pool) 
		return;

//	linkmon = find_linkmon_by_name(linkmon_pool, monitor_icmp.name); 
	linkmon = monitor_icmp.linkmon;
	if (linkmon == NULL)
		return;

	if (linkmon->data != NULL){ // icmp_data_t
		free(linkmon->data);
		linkmon->data = NULL;
	}
	remove_linkmon(linkmon_pool, linkmon);
	monitor_icmp.linkmon = NULL;
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
