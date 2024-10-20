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
#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <arpa/inet.h> // uint_8_t
#include <unistd.h>
#include <sys/time.h>
#include "icmp.h"
#include "statuscode.h"

static const int DEFDATALEN = 56;
static const int MAXIPLEN = 60;
static const int MAXICMPLEN = 76;

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

int check_icmp_status(char *ipaddr)
{
    int status = LINK_INIT;
	int fd;
	unsigned char ipBytes[4];
	in_addr_t addr;
	struct sockaddr_in destaddr;
	struct icmp *pkt;
	char packet[DEFDATALEN + MAXIPLEN + MAXICMPLEN];

	struct sockaddr_in from;
	socklen_t fromlen;
	struct iphdr *iphdr;

    fd_set readfds;
    struct timeval tv;
    int nselect = 1;
    int res = 0;

    if (ipaddr == NULL) {
        status = ICMP_DATA_ERR;
        return status;
    }

	if ((fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) <= 0){
		perror("socket error:");
		status = ICMP_SEND_ERR;
		return status;
	}
	addr = inet_addr(ipaddr);
    memcpy(ipBytes, (unsigned char *)&addr, 4);

	memset(&destaddr, 0,sizeof(struct sockaddr_in));
	memset(packet, 0, sizeof(packet));

	// Set IP msg 
	destaddr.sin_family = AF_INET;
	memcpy(&destaddr.sin_addr, ipBytes, 4);

	// Set ICMP msg 
	pkt = (struct icmp*)packet;
	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_cksum = icmp_checksum((unsigned short *)pkt, sizeof(struct icmp));
	if (sendto(fd, packet, sizeof(packet), 0, (struct sockaddr *)&destaddr, sizeof(destaddr)) == -1)
	{
		perror("sendto error\n");
		status = ICMP_SEND_ERR;
		close(fd);
		return status;
	}

    FD_ZERO(&readfds); FD_SET(fd, &readfds);
    tv.tv_sec = 2;  tv.tv_usec = 0;
    while (1){
        nselect = select(fd + 1, &readfds, 
           NULL, NULL, &tv);
        if (nselect > 0){
            memset(&from, 0, sizeof(struct sockaddr_in));
	        fromlen = sizeof(from);
	        memset(packet, 0, sizeof(packet));

	        res= recvfrom (fd, packet, sizeof(packet), 0,
			(struct sockaddr *) &from, &fromlen);

            if(res == -1 || res<76) {
        		continue;
        	}
            if (res >= 76) {			/* ip + icmp */
                memcpy(&(ipBytes[0]), &from.sin_addr, 4);
                iphdr = (struct iphdr *) packet;
                pkt = (struct icmp *) (packet + (iphdr->ihl << 2));
                if (pkt->icmp_type == ICMP_ECHOREPLY) {
                    status = STATUS_OK;
                    break;
                }
            }
        }
        else if (nselect == 0){ // timeout without duplicated mac arp reply
            status = ICMP_NO_RESPONSE;
            break;  
        }
    }
	close(fd);
    return status;
}
