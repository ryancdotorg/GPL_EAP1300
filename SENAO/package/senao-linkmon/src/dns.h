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
#ifndef _DNS_H_
#define _DNS_H_
#include "linkmon.h"
typedef struct dns_data_t {
	int fd;
	int status;
	// private data
	struct sockaddr_in dest;
	uint32_t dns_serv;
	char query_host[32];
	char msg[64];
	void *mio_data;
	void *mio_event_read;
	void *mio_timer_snd;
	void *mio_timer_rcv;
}dns_data_t;

typedef struct monitor_dns_t {
	char name[15];
	unsigned int sec;
	unsigned int usec;
	int (*lm_initfunc)(void *, void *); // mio_data_t, http_data_t
	void (*lm_sendfunc)(void *);
	void (*lm_recvfunc)(void *);
	int (*lm_statusfunc)(void *);
	char *(*lm_msgfunc)(void *);
	void (*lm_uninitfunc)(void *);
	char dns_server[16];
	char query_host[32];
	linkmon_t *linkmon;
}monitor_dns_t;

void init_dns(linkmon_t *linkmon_pool, char *dns_server, char *query_host);
void uninit_dns(linkmon_t *linkmon_pool);
#endif
