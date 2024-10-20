/*******************************************************************************************
 * Copyright (c) 2006-7 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica *
 *                      Universita' Campus BioMedico - Italy                               *
 *                                                                                         *
 * This program is free software; you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License as published by the Free Software Foundation; either  *
 * version 2 of the License, or (at your option) any later version.                        *
 *                                                                                         *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY         *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 	   *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.                *
 *                                                                                         *
 * You should have received a copy of the GNU General Public License along with this       *
 * program; if not, write to the:                                                          *
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,                    *
 * MA  02111-1307, USA.                                                                    *
 *                                                                                         *
 * --------------------------------------------------------------------------------------- *
 * Project:  Capwap                                                                        *
 *                                                                                         *
 * Author :  Ludovico Rossi (ludo@bluepixysw.com)                                          *
 *           Del Moro Andrea (andrea_delmoro@libero.it)                                    *
 *           Giovannini Federica (giovannini.federica@gmail.com)                           *
 *           Massimo Vellucci (m.vellucci@unicampus.it)                                    *
 *           Mauro Bisson (mauro.bis@gmail.com)                                            *
 *******************************************************************************************/


#include "CWCommon.h"
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <netdb.h>
#include <net/if.h>
#include <net/if_arp.h>
#ifdef __GLIBC__
# include <net/ethernet.h>
# include <netinet/if_ether.h>
#endif
#include <setjmp.h>
#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

CWNetworkLev3Service gNetworkPreferredFamily = CW_IPv4;

/*
 * Assume address is valid
 */
int CWNetworkGetAddressSize(CWNetworkLev4Address *addrPtr)
{
    switch(((struct sockaddr *)(addrPtr))->sa_family)
    {

#ifdef	IPV6
            /* IPv6 is defined in Stevens' library */
        case AF_INET6:
            return sizeof(struct sockaddr_in6);
            break;
#endif
        case AF_INET:
        default:
            return sizeof(struct sockaddr_in);
    }
}

/*
 * Send buf on an unconnected UDP socket. Unsafe means that we don't use DTLS.
 */
CWBool CWNetworkSendUnsafeUnconnected(CWSocket sock,
                                      CWNetworkLev4Address *addrPtr,
                                      const char *buf,
                                      int len,
                                      int cid)
{
    char *buffer = NULL;
    short *cidPtr;

    if(buf == NULL || addrPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    /* add cid tag header */
    if(cid != 0)
    {
        CW_CREATE_OBJECT_SIZE_ERR(buffer, len + CW_CID_HEADER_SIZE,
                                  return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        buffer[0] = CW_PACKET_CID_TAG; /* type */
        cidPtr = (short *) &buffer[1];
        *cidPtr = htons(cid);
        buffer[3] = 0;
        CW_COPY_MEMORY(&buffer[CW_CID_HEADER_SIZE], buf, len);
        len += CW_CID_HEADER_SIZE;
        buf = buffer;
    }

    CWUseSockNtop(addrPtr, CWDebugLog("Send packet to %s", str););

    while(sendto(sock, buf, len, 0, (struct sockaddr *)addrPtr, CWNetworkGetAddressSize(addrPtr)) < 0)
    {
        if(errno == EINTR)
        {
            continue;
        }
        CW_FREE_OBJECT(buffer);
        CWNetworkRaiseSystemError(CW_ERROR_SENDING);
    }

    CW_FREE_OBJECT(buffer);

    return CW_TRUE;
}

/*
 * Send buf on a "connected" UDP socket. Unsafe means that we don't use DTLS.
 */
CWBool CWNetworkSendUnsafeConnected(CWSocket sock, const char *buf, int len, int cid)
{
    char *buffer = NULL;
    short *cidPtr;

    if(buf == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    /* add cid tag header */
    if(cid != 0)
    {
        CW_CREATE_OBJECT_SIZE_ERR(buffer, len + CW_CID_HEADER_SIZE,
                                  return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

        buffer[0] = CW_PACKET_CID_TAG; /* type */
        cidPtr = (short *) &buffer[1];
        *cidPtr = htons(cid);
        buffer[3] = 0;
        CW_COPY_MEMORY(&buffer[CW_CID_HEADER_SIZE], buf, len);
        len += CW_CID_HEADER_SIZE;
        buf = buffer;
    }

    while(send(sock, buf, len, 0) < 0)
    {
        if(errno == EINTR)
        {
            continue;
        }
        CW_FREE_OBJECT(buffer);
        CWNetworkRaiseSystemError(CW_ERROR_SENDING);
    }

    CW_FREE_OBJECT(buffer);

    return CW_TRUE;
}

/*
 * Receive a datagram on an connected UDP socket (blocking).
 * Unsafe means that we don't use DTLS.
 */
CWBool CWNetworkReceiveUnsafeConnected(CWSocket sock, char *buf, int len, int *readBytesPtr)
{

    if(buf == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    while((*readBytesPtr = recv(sock, buf, len, 0)) < 0)
    {
        if(errno == EINTR)
        {
            continue;
        }
        CWNetworkRaiseSystemError(CW_ERROR_RECEIVING);
    }
    return CW_TRUE;
}

/*
 * Receive a datagram on an unconnected UDP socket (blocking).
 * Unsafe means that we don't use DTLS.
 */
CWBool CWNetworkReceiveUnsafe(CWSocket sock,
                              char *buf,
                              int len,
                              int flags,
                              CWNetworkLev4Address *addrPtr,
                              int *readBytesPtr)
{
    socklen_t addrLen = sizeof(CWNetworkLev4Address);

    if(buf == NULL || addrPtr == NULL || readBytesPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    while((*readBytesPtr = recvfrom(sock, buf, len, flags, (struct sockaddr *)addrPtr, &addrLen)) < 0)
    {
        if(errno == EINTR)
        {
            continue;
        }

        if(errno == EAGAIN && (flags & MSG_DONTWAIT))
        {
            *readBytesPtr = 0;
            break;
        }

        CWNetworkRaiseSystemError(CW_ERROR_RECEIVING);
    }
    return CW_TRUE;
}

/*
 * Init network for client.
 */
CWBool CWNetworkInitSocketClient(CWSocket *sockPtr, CWNetworkLev4Address *addrPtr, const char *bindIfName, unsigned short bindPort)
{
    int yes = 1;
    struct ifreq ifr;

    /* NULL addrPtr means that we don't want to connect to a
     * specific address
     */
    if(sockPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

#ifdef IPv6
    if(((*sockPtr) = socket((gNetworkPreferredFamily == CW_IPv4) ? AF_INET : AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) < 0)
#else
    if(((*sockPtr) = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
#endif
    {
        CWNetworkRaiseSystemError(CW_ERROR_CREATING);
    }

    if(bindIfName || bindPort != 0)
    {
#ifdef IPv6
        if(gNetworkPreferredFamily != CW_IPv4)
        {
            struct sockaddr_in6 bindaddr6;

            memset(&bindaddr6, 0, sizeof(bindaddr6));
            bindaddr6.sin6_family = AF_INET6;
            if(!bindIfName)
            {
                bindaddr6.sin6_addr = in6addr_any;
            }
            else
            {
                ifr.ifr_addr.sa_family = AF_INET6;

                strncpy(ifr.ifr_name, bindIfName, IFNAMSIZ - 1);
                if(ioctl((*sockPtr), SIOCGIFADDR, &ifr) < 0)
                {
                    return CWErrorRaise(CW_ERROR_GENERAL, NULL);
                }
                bindaddr6.sin6_addr = ((struct sockaddr_in6 *)&ifr.ifr_addr)->sin6_addr;
            }
            bindaddr6.sin6_port = htons(bindPort);

            if(bind((*sockPtr), (struct sockaddr *)&bindaddr6, sizeof(bindaddr6)) < 0)
            {
                CWNetworkRaiseSystemError(CW_ERROR_CREATING);
            }
        }
        else
#endif
        {
            struct sockaddr_in bindaddr;

            memset((char *)&bindaddr, 0, sizeof(bindaddr));
            bindaddr.sin_family = AF_INET;
            if(!bindIfName)
            {
                bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
            }
            else
            {
                ifr.ifr_addr.sa_family = AF_INET;

                strncpy(ifr.ifr_name, bindIfName, IFNAMSIZ - 1);
                if(ioctl((*sockPtr), SIOCGIFADDR, &ifr) < 0)
                {
                    return CWErrorRaise(CW_ERROR_GENERAL, NULL);
                }
                bindaddr.sin_addr.s_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
            }

            bindaddr.sin_port = htons(bindPort);

            if(bind((*sockPtr), (struct sockaddr *)&bindaddr, sizeof(bindaddr)) < 0)
            {
                CWNetworkRaiseSystemError(CW_ERROR_CREATING);
            }
        }
    }

    if(addrPtr != NULL)
    {
        CWUseSockNtop(((struct sockaddr *)addrPtr), CWDebugLog(str););

        if(connect((*sockPtr), ((struct sockaddr *)addrPtr), CWNetworkGetAddressSize(addrPtr)) < 0)
        {
            CWNetworkRaiseSystemError(CW_ERROR_CREATING);
        }
    }
    /* allow sending broadcast packets */
    setsockopt(*sockPtr, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));

    return CW_TRUE;
}

/*
 * Wrapper for select
 */
CWBool CWNetworkTimedPollRead(CWSocket sock, struct timeval *timeout)
{
    int r;

    fd_set fset;

    if(timeout == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    FD_ZERO(&fset);
    FD_SET(sock, &fset);

    if((r = select(sock + 1, &fset, NULL, NULL, timeout)) == 0)
    {
        CWDebugLog("Select Time Expired");
        return CWErrorRaise(CW_ERROR_TIME_EXPIRED, NULL);
    }
    else if(r < 0)
    {
        CWDebugLog("Select Error");

        if(errno == EINTR)
        {

            CWDebugLog("Select Interrupted by signal");
            return CWErrorRaise(CW_ERROR_INTERRUPTED, NULL);
        }

        CWNetworkRaiseSystemError(CW_ERROR_GENERAL);
    }

    return CW_TRUE;
}

/*
 * Given an host int the form of C string (e.g. "192.168.1.2" or "localhost"),
 * returns the address.
 */

CWBool CWNetworkGetAddressForHostTimeout(char *host, CWNetworkLev4Address *addrPtr, int waitSec)
{
    char resFile[32] = {0};
    char pidFile[32] = {0};
    char ip_str[32] = {0};
    char *val = NULL, *c;
    int microSec;
    struct in_addr addr;
    const int step = 200000; // poll interval is 200 ms
    CWBool find = CW_FALSE;
    char *tmpHost, *portStr, *controllerIdStr;

    /* Check the hostname is ipv4 form */
    c = host;
    while(*c != '\0')
    {
        if(!isdigit(*c) && *c != '.' && *c != ':' && *c != '#')
        {
            break;
        }
        c++;
    }

    /* host is ipv4 form */
    if(*c == '\0')
    {
        return CWNetworkGetAddressForHost(host, addrPtr);
    }

    CW_CREATE_STRING_FROM_STRING_ERR(tmpHost, host,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    /* search the port number following a ':' */
    if((portStr = strchr(tmpHost, ':')))
    {
        *portStr = '\0';
        portStr++;
    }
    else if((controllerIdStr = strchr(tmpHost, '#')))
    {
        *controllerIdStr = '\0';
    }

    sprintf(resFile, "/tmp/nr.%d.%x", getpid(), (unsigned int) CWThreadSelf());
    sprintf(pidFile, "/tmp/np.%d.%x", getpid(), (unsigned int) CWThreadSelf());
    microSec = waitSec * 1000000;

    /* call nslookup in background, and then poll evey 200 ms */
    CWSystem("nslookup %s > %s & echo $! > %s", tmpHost, resFile, pidFile);


    while(microSec > 0 && !find)
    {
        CWWaitMicroSec(step);

        if(CWCreateStringByFile(resFile, &val) &&
           (c = strstr(val, "Name:")))
        {

            while((c = strstr(c, "Address")))
            {
                c = strstr(c, ": ");
                if(!c)
                {
                    break;
                }

                c += 2;

                // nslookup result
                // old
                // Name:	www.google.com
                // Address: 172.217.24.4
                // new
                // Name:      www.google.com
                // Address 1: 172.217.24.4 tsa01s07-in-f4.1e100.net
                sscanf(c, "%s", ip_str);

                if(inet_aton(ip_str, &addr) != 0)
                {
                    CW_ADDR_SET_IP(addrPtr, addr.s_addr);
                    CW_ADDR_SET_PORT(addrPtr, portStr ? atoi(portStr) : CW_CONTROL_PORT);
                    find = CW_TRUE;
                    break;
                }
            }
        }
        CW_FREE_OBJECT(val);
        microSec -= step;
    }

    /* kill the background process  */
    if(!find && CWCreateStringByFile(pidFile, &val))
    {
        CWSystem("kill -9 %s", val);
        CW_FREE_OBJECT(val);
    }

    unlink(resFile);
    unlink(pidFile);
    CW_FREE_OBJECT(tmpHost);

    return find ? CW_TRUE : CWErrorRaise(CW_ERROR_TIME_EXPIRED, NULL);
}

CWBool CWNetworkGetAddressForHost(char *host, CWNetworkLev4Address *addrPtr)
{
    struct addrinfo hints, *res, *ressave;
    char serviceName[5];
    CWSocket sock;
    char *tmpHost, *portStr, *controllerIdStr;

    if(host == NULL || addrPtr == NULL)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, NULL);
    }

    CW_CREATE_STRING_FROM_STRING_ERR(tmpHost, host,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    /* search the port number following a ':' */
    if((portStr = strchr(tmpHost, ':')))
    {
        *portStr = '\0';
        portStr++;
    }
	/* search the tag Id following a '#' */
    else if((controllerIdStr = strchr(tmpHost, '#')))
    {
        *controllerIdStr = '\0';
    }

    CW_ZERO_MEMORY(&hints, sizeof(struct addrinfo));

#ifdef IPv6
    if(gNetworkPreferredFamily == CW_IPv6)
    {
        hints.ai_family = AF_INET6;
        hints.ai_flags = AI_V4MAPPED;
    }
    else
    {
        hints.ai_family = AF_INET;
    }
#else
    hints.ai_family = AF_INET;
#endif
    hints.ai_socktype = SOCK_DGRAM;

    /* endianness will be handled by getaddrinfo */
    snprintf(serviceName, 5, "%d", portStr ? atoi(portStr) : CW_CONTROL_PORT);

    if(getaddrinfo(tmpHost, serviceName, &hints, &res) != 0)
    {
        CW_FREE_OBJECT(tmpHost);
        return CWErrorRaise(CW_ERROR_GENERAL, "Can't resolve hostname");
    }

    ressave = res;
    do
    {
        if((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        {
            /* try next address */
            continue;
        }
        /* success */
        break;
    }
    while((res = res->ai_next) != NULL);

    close(sock);

    if(res == NULL)
    {
        /* error on last iteration */
        freeaddrinfo(ressave);
        CWNetworkRaiseSystemError(CW_ERROR_CREATING);
    }

    CW_COPY_NET_ADDR_PORT(addrPtr, (res->ai_addr));
    freeaddrinfo(ressave);
    CW_FREE_OBJECT(tmpHost);

    return CW_TRUE;
}

CWBool CWNetworkAssociateMulticastWithSocket(const char *ifname, CWSocket sock)
{
    CWSocket fd;
    struct ifreq ifr;
    struct in_addr sa;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    ifr.ifr_addr.sa_family = AF_INET;

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    if(ioctl(fd, SIOCGIFADDR, &ifr) < 0)
    {
        close(fd);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    close(fd);

    sa.s_addr = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;
    if(setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF, (char *)&sa, sizeof(sa)) < 0)
    {
        CWNetworkRaiseSystemError(CW_ERROR_GENERAL);
    }

    return CW_TRUE;
}

CWBool CWNetworkAddStaticArpEntry(CWNetworkLev4Address *addrPtr, CWMacAddress mac)
{
    struct arpreq ar;
    struct sockaddr_in *sin;
    int s;

    bzero((caddr_t)&ar, sizeof(ar));

    ar.arp_pa.sa_family = AF_INET;
    sin = (struct sockaddr_in *)&ar.arp_pa;
    bcopy(addrPtr, sin, sizeof(struct sockaddr_in));

 	ar.arp_ha.sa_family = ARPHRD_ETHER;
    ar.arp_ha.sa_data[0] = mac[0];
    ar.arp_ha.sa_data[1] = mac[1];
    ar.arp_ha.sa_data[2] = mac[2];
    ar.arp_ha.sa_data[3] = mac[3];
    ar.arp_ha.sa_data[4] = mac[4];
    ar.arp_ha.sa_data[5] = mac[5];
    ar.arp_flags = ATF_PERM;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if(s < 0)
    {
        CWNetworkRaiseSystemError(CW_ERROR_GENERAL);
    }

    if(ioctl(s, SIOCSARP, (caddr_t)&ar) < 0)
    {
        close(s);
        CWNetworkRaiseSystemError(CW_ERROR_GENERAL);
    }
    close(s);

    return CW_TRUE;
}

CWBool CWNetworkDelArpEntry(CWNetworkLev4Address *addrPtr)
{
    struct arpreq ar;
    struct sockaddr_in *sin;
    int s;

    bzero((caddr_t)&ar, sizeof(ar));

    ar.arp_pa.sa_family = AF_INET;
    sin = (struct sockaddr_in *)&ar.arp_pa;
    bcopy(addrPtr, sin, sizeof(struct sockaddr_in));

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if(s < 0)
    {
        CWNetworkRaiseSystemError(CW_ERROR_GENERAL);
    }

    if(ioctl(s, SIOCDARP, (caddr_t)&ar) < 0)
    {
        if(errno != ENXIO)
        {
        	close(s);
            CWNetworkRaiseSystemError(CW_ERROR_GENERAL);
        }
    }
    close(s);

    return CW_TRUE;
}

CWBool CWNetworkAddHostStaticRoute(const char *ifname, CWNetworkLev4Address *addrPtr)
{
    struct rtentry rt;
    int s;

    if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    rt.rt_flags = (RTF_UP | RTF_HOST);
    memcpy(&rt.rt_dst, addrPtr, sizeof(struct sockaddr));
    rt.rt_dev = (char*) ifname;

    if(ioctl(s, SIOCADDRT, &rt) < 0)
    {
    	close(s);
		CWNetworkRaiseSystemError(CW_ERROR_GENERAL);
    }
    close(s);

    return CW_TRUE;
}

CWBool CWNetworkDelHostStaticRoute(const char *ifname, CWNetworkLev4Address *addrPtr)
{
    struct rtentry rt;
    int s;

    if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    rt.rt_flags = (RTF_UP | RTF_HOST);
    memcpy(&rt.rt_dst, addrPtr, sizeof(struct sockaddr));
    rt.rt_dev = (char*) ifname;

    if(ioctl(s, SIOCDELRT, &rt) < 0)
    {
    	close(s);
		CWNetworkRaiseSystemError(CW_ERROR_GENERAL);
    }
    close(s);

    return CW_TRUE;
}

CWBool CWNetworkGetInterfaceMacAddress(const char *ifname, CWMacAddress mac)
{
    int s;
    struct ifreq ifr;

    if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    strcpy(ifr.ifr_name, ifname);
    if(ioctl(s, SIOCGIFHWADDR, &ifr) < 0)
    {
        close(s);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    bcopy(ifr.ifr_hwaddr.sa_data, mac, 6);

    close(s);

    return CW_TRUE;
}

CWBool CWNetworkGetInterfaceAddressMask(const char *ifname, unsigned int *ip, unsigned int *mask)
{
    int s;
    struct ifreq ifr;

    if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    if(ioctl(s, SIOCGIFADDR, &ifr) < 0)
    {
        close(s);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    *ip = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;

    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    if(ioctl(s, SIOCGIFNETMASK, &ifr) < 0)
    {
        close(s);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    *mask = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr;

    close(s);

    return CW_TRUE;
}

