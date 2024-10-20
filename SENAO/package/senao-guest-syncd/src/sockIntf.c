
/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*
 * socket.c -- DHCP server client/server socket creation
 *
 * udhcp client/server
 * Copyright (C) 1999 Matthew Ramsay <matthewr@moreton.com.au>
 *			Chris Trew <ctrew@moreton.com.au>
 *
 * Rewrite by Russ Dill <Russ.Dill@asu.edu> July 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <errno.h>
#include <features.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include "common.h"
#include "sockIntf.h"
#include "guestsync.h"
#include "aesapi.h"

/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/*                           Parameter                                     */
/*-------------------------------------------------------------------------*/
#if 0
struct udp_data {
    struct iphdr ip;
    struct udphdr udp;
    char data[5120];
};
#endif


/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/

int read_interface(char *interface, int *ifindex, uint32_t *addr, uint8_t *arp)
{
    int fd;
    struct ifreq ifr;
    struct sockaddr_in *our_ip;

    memset(&ifr, 0, sizeof(struct ifreq));
    if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0)
    {
        ifr.ifr_addr.sa_family = AF_INET;
        strcpy(ifr.ifr_name, interface);

        if(addr)
        {
            if(ioctl(fd, SIOCGIFADDR, &ifr) == 0)
            {
                our_ip = (struct sockaddr_in *) &ifr.ifr_addr;
                *addr = our_ip->sin_addr.s_addr;
                guestsync_printf(GUESTSYNC_DEBUG, "%s (our ip) = %s\n", ifr.ifr_name, inet_ntoa(our_ip->sin_addr));
            }
            else
            {
                guestsync_printf(GUESTSYNC_ERROR, "SIOCGIFADDR failed, is the interface up and configured?\n");
                close(fd);
                return -1;
            }
        }

        if(ioctl(fd, SIOCGIFINDEX, &ifr) == 0)
        {
            guestsync_printf(GUESTSYNC_DEBUG, "adapter index %d\n", ifr.ifr_ifindex);
            *ifindex = ifr.ifr_ifindex;
        }
        else
        {
            guestsync_printf(GUESTSYNC_ERROR, "SIOCGIFINDEX failed!\n");
            close(fd);
            return -1;
        }
        if(arp)
        {
            if(ioctl(fd, SIOCGIFHWADDR, &ifr) == 0)
            {
                memcpy(arp, ifr.ifr_hwaddr.sa_data, 6);
                guestsync_printf(GUESTSYNC_DEBUG, "adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x\n",
                        arp[0], arp[1], arp[2], arp[3], arp[4], arp[5]);
            }
            else
            {
                guestsync_printf(GUESTSYNC_ERROR, "SIOCGIFHWADDR failed!\n");
                close(fd);
                return -1;
            }
        }
    }
    else
    {
        guestsync_printf(GUESTSYNC_ERROR, "socket failed!\n");
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}


int listen_socket(uint32_t ip, int port, char *inf)
{
    struct ifreq interface;
    int fd;
    struct sockaddr_in addr;
    int n = 1;

    guestsync_printf(GUESTSYNC_DEBUG, "Opening listen socket on 0x%08x:%d %s\n", ip, port, inf);
    if((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        guestsync_printf(GUESTSYNC_ERROR, "socket call failed\n");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ip;

    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n)) == -1)
    {
        close(fd);
        return -1;
    }
    if(setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char *) &n, sizeof(n)) == -1)
    {
        close(fd);
        return -1;
    }

    strncpy(interface.ifr_ifrn.ifrn_name, inf, IFNAMSIZ);
    if(setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE,(char *)&interface, sizeof(interface)) < 0)
    {
        close(fd);
        return -1;
    }

    if(bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1)
    {
        close(fd);
        return -1;
    }

    return fd;
}

uint16_t checksum(void *addr, int count)
{
    /* Compute Internet Checksum for "count" bytes
     *         beginning at location "addr".
     */
    register int32_t sum = 0;
    uint16_t *source = (uint16_t *) addr;

    while (count > 1)  {
        /*  This is the inner loop */
        sum += *source++;
        count -= 2;
    }

    /*  Add left-over byte, if any */
    if (count > 0) {
        /* Make sure that the left-over byte is added correctly both
         * with little and big endian hosts */
        uint16_t tmp = 0;
        *(uint8_t *) (&tmp) = * (uint8_t *) source;
        sum += tmp;
    }
    /*  Fold 32-bit sum to 16 bits */
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}

int udp_packet_enc(uint8_t *payload, int payloadSize, uint32_t dest_ip, int dest_port)
{
    // encryption
    int bytes = -1;
    unsigned char *ciphertext = NULL;
    unsigned char packet[1500] = {0};
    int len;
    unsigned char salt[8] = {0};
    guest_sync_header_pkt_t *header = NULL;
    guest_sync_pkt_t *body = NULL;
    int header_length = 0;


    header = (guest_sync_header_pkt_t *) packet;

    header->magic[0] = GUESTSYNC_MAJOR_MAGIC;
    header->magic[1] = GUESTSYNC_MINOR_MAGIC;
    header->magic[2] = GUESTSYNC_NEW_MAJOR_MAGIC;
    header->magic[3] = GUESTSYNC_NEW_MINOR_MAGIC;

    header->version[0] = GUESTSYNC_MAJOR_VERSION;
    header->version[1] = GUESTSYNC_MINOR_VERSION;

    // get salt from payload
    body = (guest_sync_pkt_t *) payload;
    memcpy(salt, body->random, 8);
    memcpy(header->salt, salt, 8);

    len = payloadSize;
    ciphertext = aes_crypt(payload, &len, salt, 1);
    if(ciphertext != NULL)
    {
        header_length = sizeof(guest_sync_header_pkt_t);
        memcpy(&packet[header_length], ciphertext, len);

        bytes = udp_packet(packet, len + header_length, dest_ip, dest_port);
        free(ciphertext);
    }

    return bytes;
}

int udp_packet(uint8_t *payload, int payloadSize, uint32_t dest_ip, int dest_port)
{
    int fd;
    int result;
    struct sockaddr_in dest;
    int broadcast = 1;

    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        guestsync_printf(GUESTSYNC_ERROR, "socket call failed\n");
        return -1;
    }
    memset(&dest, 0, sizeof(dest));

    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
        guestsync_printf(GUESTSYNC_ERROR, "setsockopt (SO_BROADCAST)");
        close(fd);
        return 0;
    }

    dest.sin_family = AF_INET;
    dest.sin_port = htons(dest_port);
    dest.sin_addr.s_addr = dest_ip;
    bzero(&(dest.sin_zero), 8);

    result = sendto(fd, payload, payloadSize, 0, (struct sockaddr *) &dest, sizeof(dest));
    if(result <= 0)
    {
        guestsync_printf(GUESTSYNC_ERROR, "write on socket failed\n");
    }
    close(fd);

    return result;
}

#if 0
/* Construct a ip/udp header for a packet, and specify the source and dest hardware address */
int raw_packet(uint8_t *payload, int payloadSize, uint32_t source_ip, int source_port,
        uint32_t dest_ip, int dest_port, uint8_t *dest_arp, int ifindex)
{
    int fd;
    int result;
    struct sockaddr_ll dest;
    static struct udp_data packet;

    if((fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0)
    {
        guestsync_printf(GUESTSYNC_ERROR, "socket call failed\n");
        return -1;
    }
    memset(&dest, 0, sizeof(dest));
    memset(&packet, 0, sizeof(packet));

    dest.sll_family = AF_PACKET;
    dest.sll_protocol = htons(ETH_P_IP);
    dest.sll_ifindex = ifindex;
    dest.sll_halen = 6;
    memcpy(dest.sll_addr, dest_arp, 6);
    if(bind(fd, (struct sockaddr *)&dest, sizeof(struct sockaddr_ll)) < 0)
    {
        guestsync_printf(GUESTSYNC_ERROR, "bind call failed\n");
        close(fd);
        return -1;
    }

    packet.ip.protocol = IPPROTO_UDP;
    packet.ip.saddr = source_ip;
    packet.ip.daddr = dest_ip;
    packet.udp.source = htons(source_port);
    packet.udp.dest = htons(dest_port);
    packet.udp.len = htons(sizeof(packet.udp) + payloadSize); /* cheat on the psuedo-header */
    packet.ip.tot_len = packet.udp.len;
    memcpy(&(packet.data), payload, payloadSize);
    packet.udp.check = checksum(&packet, sizeof(struct iphdr)+sizeof(struct udphdr)+payloadSize);

    packet.ip.tot_len = htons(sizeof(struct iphdr)+sizeof(struct udphdr)+payloadSize);
    packet.ip.ihl = sizeof(packet.ip) >> 2;
    packet.ip.version = IPVERSION;
    packet.ip.ttl = IPDEFTTL;
    packet.ip.check = checksum(&(packet.ip), sizeof(packet.ip));

    result = sendto(fd, &packet, sizeof(struct iphdr)+sizeof(struct udphdr)+payloadSize, 0, (struct sockaddr *) &dest, sizeof(dest));
    if(result <= 0)
    {
        guestsync_printf(GUESTSYNC_ERROR, "write on socket failed\n");
    }
    close(fd);

    return result;
}
#endif
