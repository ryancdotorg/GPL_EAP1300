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
#ifndef _PHYLINK_H_
#define _PHYLINK_H_
#include "linkmon.h"
typedef struct phylink_data_t {
	int fd;
	int status;
	// private data
	char if_name[16];
	void *mio_data;
	void *mio_event_read;
	void *mio_timer_snd;
	void *mio_timer_rcv;
}phylink_data_t;

typedef struct monitor_phylink_t {
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
}monitor_phylink_t;

void init_phylink(linkmon_t *linkmon_pool, char *ipaddr);
void uninit_phylink(linkmon_t *linkmon_pool);
#endif
