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
#ifndef _HTTP_H_
#define _HTTP_H_
#include "linkmon.h"
typedef struct http_data_t {
	int fd;
	int status;
	// private data
	char hostname[32];
	char path[32];
	char *recvbuf;
	int recvbuf_len;
	void *mio_data;
	void *mio_event_read;
	void *mio_event_write;
	void *mio_timer_snd;
	void *mio_timer_conn;
	void *mio_timer_rcv;
}http_data_t;

typedef struct monitor_http_t {
	char name[15];
	unsigned int sec;
	unsigned int usec;
	int (*lm_initfunc)(void *, void *); // mio_data_t, http_data_t
	void (*lm_sendfunc)(void *);
	void (*lm_recvfunc)(void *);
	int (*lm_statusfunc)(void *);
	char *(*lm_msgfunc)(void *);
	void (*lm_uninitfunc)(void *);
	char desthostname[32];
	char destpath[32];
	linkmon_t *linkmon;
}monitor_http_t;

void init_http(linkmon_t *linkmon_pool, char *query_host);
void uninit_http(linkmon_t *linkmon_pool);
#endif
