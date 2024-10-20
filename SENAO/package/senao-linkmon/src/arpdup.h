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
#ifndef _ARPDUP_H_
#define _ARPDUP_H_
#include "linkmon.h"
#include <arpa/inet.h>
typedef struct arpdup_data_t {
	int fd;
	int status;
	// private data
	char if_name[16];
	uint8_t if_mac[6];
	uint32_t if_ip;
	uint8_t dup_mac[6];
	char msg[64];
	void *mio_event;
	void *mio_timer;
}arpdup_data_t;

typedef struct monitor_arpdup_t {
	char name[15];
	unsigned int sec;
	unsigned int usec;
	int (*lm_initfunc)(void *, void *); // mio_data_t, xxxx_data_t
	void (*lm_sendfunc)(void *);
	void (*lm_recvfunc)(void *);
	int (*lm_statusfunc)(void *);
	char *(*lm_msgfunc)(void *);
	void (*lm_uninitfunc)(void *);
	char destIF[16];
	linkmon_t *linkmon;
}monitor_arpdup_t;

void init_arpdup(linkmon_t *linkmon_pool, char *wan_ifname);
void uninit_arpdup(linkmon_t *linkmon_pool);
#endif
