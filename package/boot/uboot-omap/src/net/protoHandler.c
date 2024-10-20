#include <malloc.h>
#include <common.h>
#include <net.h>
#include "tcp.h"
#include "httpd.h"
#include "protoHandler.h"

/* define foe protoHandler debugging */
//#define DBG_PROTO_HANDLER

/*
 * Output:
 *   0 : Unknown protocol, ip_pkt will be processed by default packetHandler.
 *   1 : Success.
 */
int protocolHandler(struct ip_udp_hdr *ip_pkt, unsigned int datalen)
{
    struct tcphdr tcp_hdr;
    struct in_addr ip;

    if(!ip_pkt)
        return 0;

    switch(ip_pkt->ip_p)
    {
        case IPPROTO_TCP:
            /* common job */
            memcpy(&tcp_hdr, (unsigned char*)ip_pkt + IP_HDR_SIZE, TCP_HDR_SIZE);

            switch(htons(tcp_hdr.dest))
            {
                case PORT_HTTP:
                    tcp_handler((unsigned char *)ip_pkt, 0, ip, 0, datalen);
                    break;
                default:
                    /* No matched handler found */
                    return 0;
                    break;
            }
            break;
        case IPPROTO_UDP:
            switch (ntohs(ip_pkt->udp_dst))
            {
                default:
                    /* No matched handler found */
                    return 0;
                    break;
            }
            break;
        default:
            /* do nothing */
            return 0;
            break;
    }
    return 1;
}

