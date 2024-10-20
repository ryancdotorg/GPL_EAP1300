/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                    *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  syseye                                                                        *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#ifndef _CMDIPC_H_
#define _CMDIPC_H_
#include <string.h>
#include "list.h"
typedef struct cmdipc_t {
	struct list_head list;
	char name[15];
	int fd;
	char access_point[64];
	int (*ipc_sockfunc)(void *srvdata); // tcp_srv_data_t
	void (*ipc_recvfunc)(void *tcp_xxx_data);
	void (*ipc_closesockfunc)(void *);
	void *data;
}cmdipc_t;

typedef struct ipc_t {
	char name[15];
	char *access_point;
	int (*ipc_sockfunc)(void *);
	void (*ipc_recvfunc)(void *xxx_ooo_data); // miodata, ipt xxxx_data_t
	void (*ipc_closesockfunc)(void *);
	cmdipc_t *cmdipc;
}ipc_t;

typedef int (*ipc_sockfunc)(void *);
typedef void (*ipc_recvfunc)(void *xxx_ooo_data);	
typedef	int (*ipc_sendfunc)(void *, void *);
typedef void (*ipc_closesock_func)(void *);

cmdipc_t *cmdipc_init(void);
void cmdipc_uninit(cmdipc_t *cmdipc_pool);
cmdipc_t *add_cmdipc(cmdipc_t *cmdipc_pool, char *name, char *access_point, ipc_sockfunc sock_func, ipc_recvfunc recv_func, ipc_closesock_func closesock_func, void *data);
void remove_cmdipc(cmdipc_t *cmdipc_pool, cmdipc_t *cmdipc);
cmdipc_t *find_cmdipc_by_name(cmdipc_t *cmdipc_pool, char *name);
void traverse_cmdipc(cmdipc_t *cmdipc);
#endif
