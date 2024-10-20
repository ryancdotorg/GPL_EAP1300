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
#ifndef _IPC_UDP_H_
#define _IPC_UDP_H_
#include "cmdipc.h"
#include "linkmon.h"
#include <sys/socket.h>
#include <sys/un.h>
typedef struct udp_data_t {
	int fd;
	char access_point[64];
	struct sockaddr_un from;	// client info
	socklen_t fromlen;		// client info
	void (*ipc_cmdfunc)(void *, void *); // udp_data, recv_buf
	int (*ipc_sendfunc)(void *, void *); // udp_data, recv_write
	linkmon_t *linkmon; // TODO: should be global data?
	int *terminate_ignore;
}udp_data_t;
void init_ipc_udp(cmdipc_t *cmdipc_pool, linkmon_t *linkmon, int *terminate_ignore);
void uninit_ipc_udp(cmdipc_t *cmdipc_pool);
#endif
