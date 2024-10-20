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
#ifndef _LINKMON_H_
#define _LINKMON_H_
#include <string.h>
#include "list.h"
typedef struct linkmon_t {
	struct list_head list;
	char name[15];
	int fd;
	unsigned int sec;
	unsigned int usec;	
	int (*lm_initfunc)(void *, void *); // mio_data_t, xxx_data_t
	void (*lm_sendfunc)(void *);
	void (*lm_recvfunc)(void *);	
	int (*lm_statusfunc)(void *);
	char *(*lm_msgfunc)(void *);
	void (*lm_uninitfunc)(void *);
	void *data;
}linkmon_t;

typedef int (*lm_initfunc)(void *, void *); // mio_data_t, xxx_data_t 
typedef void (*lm_sendfunc)(void *); 
typedef void (*lm_recvfunc)(void *); 
typedef int (*lm_statusfunc)(void *); 
typedef char *(*lm_msgfunc)(void *); 
typedef void (*lm_uninitfunc)(void *); 
typedef enum linkstatus_t {
	LINK_INIT = 0,
	STATUS_OK,
	ICMP_CREATE_ERR = 100,
	ICMP_SEND_ERR,
	ICMP_RCV_ERR,
	ICMP_DATA_ERR,
	ICMP_NO_RESPONSE,
	ARP_CREATE_ERR = 200,
	ARP_SEND_ERR,
	ARP_DATA_ERR,
	ARP_RCV_ERR,
	ARP_DUPLICATE,
	DNS_CREATE_ERR = 300,
	DNS_SEND_ERR,
	DNS_RCV_ERR,
	DNS_DATA_ERR,
	DNS_NO_RESPONSE,
	HTTP_DATA_ERR = 400,
	HTTP_DNS_ERR,
	HTTP_CREATE_ERR,
	HTTP_SEND_ERR,
	HTTP_RCV_ERR,
	HTTP_CONN_ERR,
	HTTP_PAGE_ERR,
	HTTP_NO_RESPONSE,
	HTTPS_DATA_ERR = 500,
	HTTPS_DNS_ERR,
	HTTPS_CREATE_ERR,
	HTTPS_SEND_ERR,
	HTTPS_HANDSHAKE_ERR,
	HTTPS_RCV_ERR,
	HTTPS_CONN_ERR,
	HTTPS_PAGE_ERR,
	HTTPS_NO_RESPONSE,
	PHYLINK_DATA_ERR = 600,
	PHYLINK_CREATE_ERR,
	PHYLINK_DOWN,
	PHYLINK_UNKNOWN,
}linkstatus;

linkmon_t *linkmon_init(void);
void linkmon_uninit(linkmon_t *linkmon_pool);
linkmon_t *add_linkmon(linkmon_t *linkmon_pool, char *name, unsigned int sec, unsigned int usec, lm_initfunc init_func, lm_sendfunc send_func, lm_recvfunc recv_func, lm_statusfunc status_func, lm_msgfunc msg_func, lm_uninitfunc uninit_func, void *data);
void remove_linkmon(linkmon_t *linkmon_pool, linkmon_t *linkmon);
linkmon_t *find_linkmon_by_name(linkmon_t *linkmon_pool, char *name);
void traverse_linkmon(linkmon_t *linkmon);
#endif
