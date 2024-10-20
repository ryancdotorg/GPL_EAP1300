/*******************************************************************************************
 * Copyright (c) 2006-7 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica *
 *                      Universita' Campus BioMedico - Italy                               *
 *                                                                                         *
 * This program is free software; you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License as published by the Free Software Foundation; either  *
 * version 2 of the License, or (at your option) any later version.                        *
 *                                                                                         *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY         *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 	       *
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


#ifndef __CAPWAP_CWNetwork_HEADER__
#define __CAPWAP_CWNetwork_HEADER__

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netdb.h>

#include "CWStevens.h"

typedef int CWSocket;

typedef struct sockaddr_storage CWNetworkLev4Address;

typedef enum {
	CW_IPv6,
	CW_IPv4
} CWNetworkLev3Service;

extern CWNetworkLev3Service gNetworkPreferredFamily;

#define	CW_COPY_NET_ADDR_PORT(addr1, addr2)  	sock_cpy_addr_port(((struct sockaddr*)(addr1)), ((struct sockaddr*)(addr2)))

#define CW_ADDR_CMP(addr1, addr2) sock_cmp_addr(((struct sockaddr*)(addr1)), ((struct sockaddr*)(addr2)), sizeof(CWNetworkLev4Address))

#define CW_ADDR_SET_IP(_addr, _ip) do{((struct sockaddr *)(_addr))->sa_family = AF_INET;((struct sockaddr_in *)(_addr))->sin_addr.s_addr = _ip;}while(0)

#define CW_ADDR_GET_IP(_addr) (((struct sockaddr_in *)(_addr))->sin_addr.s_addr)

#define CW_ADDR_SET_PORT(_addr, _port) sock_set_port_cw(((struct sockaddr*)(_addr)), htons(_port))

#define CW_ADDR_GET_PORT(_addr) ntohs(sock_get_port_cw(((struct sockaddr*)(_addr))))

#define CW_ADDR_PORT_CMP(addr1, addr2) (sock_cmp_addr(((struct sockaddr*)(addr1)), ((struct sockaddr*)(addr2)), sizeof(CWNetworkLev4Address)) || \
											sock_cmp_port(((struct sockaddr*)(addr1)), ((struct sockaddr*)(addr2)), sizeof(CWNetworkLev4Address)))

#define CW_SAME_SUBNET(addr1, addr2, mask)  ((((struct sockaddr_in *)(addr1))->sin_addr.s_addr & ((struct sockaddr_in *)(mask))->sin_addr.s_addr) == \
												(((struct sockaddr_in *)(addr2))->sin_addr.s_addr & ((struct sockaddr_in *)(mask))->sin_addr.s_addr))

#define CWUseSockNtop(sa, block) 		{ 						\
							char __str[128];			\
							char *str; str = sock_ntop_r(((struct sockaddr*)(sa)), __str);\
							{block}					\
						}

#define CWNetworkRaiseSystemError(error)	{						\
							char buf[256];				\
							if(strerror_r(errno, buf, 256) < 0) {	\
								CWErrorRaise(error, NULL);	\
								return CW_FALSE;		\
							}					\
							CWErrorRaise(error, NULL);		\
							return CW_FALSE;			\
						}

#define		CWNetworkCloseSocket(x)		do{ shutdown(x,SHUT_RDWR); close(x); }while(0)

int CWNetworkGetAddressSize(CWNetworkLev4Address *addrPtr);
CWBool CWNetworkSendUnsafeConnected(CWSocket sock, const char *buf, int len, int cid);
CWBool CWNetworkSendUnsafeUnconnected(CWSocket sock, CWNetworkLev4Address *addrPtr, const char *buf, int len, int cid);
CWBool CWNetworkReceiveUnsafe(CWSocket sock, char *buf, int len, int flags, CWNetworkLev4Address *addrPtr, int *readBytesPtr);
CWBool CWNetworkReceiveUnsafeConnected(CWSocket sock, char *buf, int len, int *readBytesPtr);
CWBool CWNetworkInitSocketClient(CWSocket *sockPtr, CWNetworkLev4Address *addrPtr, const char* bindIfName, unsigned short bindPort);
CWBool CWNetworkTimedPollRead(CWSocket sock, struct timeval *timeout);
CWBool CWNetworkGetAddressForHostTimeout(char *host, CWNetworkLev4Address *addrPtr, int timeout);
CWBool CWNetworkGetAddressForHost(char *host, CWNetworkLev4Address *addrPtr);
CWBool CWNetworkAssociateMulticastWithSocket(const char* ifname, CWSocket sock);
CWBool CWNetworkAddStaticArpEntry(CWNetworkLev4Address *addrPtr, CWMacAddress mac);
CWBool CWNetworkDelArpEntry(CWNetworkLev4Address *addrPtr);
CWBool CWNetworkAddHostStaticRoute(const char *ifname, CWNetworkLev4Address *addrPtr);
CWBool CWNetworkDelHostStaticRoute(const char *ifname, CWNetworkLev4Address *addrPtr);
CWBool CWNetworkGetInterfaceMacAddress(const char *ifname, CWMacAddress mac);
CWBool CWNetworkGetInterfaceAddressMask(const char *ifname, unsigned int *ip, unsigned int *mask);
#endif
