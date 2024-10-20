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
#ifndef _IPC_TCP_H_
#define _IPC_TCP_H_
#include "cmdipc.h"
#include <sys/socket.h>
#include <sys/un.h>

typedef void (*ipc_cmdfunc)(void *gdata, int sockfd, void *content, int content_len);
typedef struct tcp_srv_data_t {
void *gdata; //gdata_t
    int fd;
    char access_point[64];
    int (*tcp_initfunc)(void *); // tcp_data_t
    void (*tcp_recvfunc)(void *tcp_data);
    void (*ipc_cmdfunc)(void *gdata, int sockfd, void *content, int content_len);
}tcp_srv_data_t;

typedef enum {
   HEADER = 0,
   CHUNKHEADER,
   CHUNKCONTENT,
   CONTENT
}parseflag_t;

typedef enum {
	EXHAUST,
	NOTEXHAUST
}buf_status_t;

typedef struct {
	unsigned int content_length;
	unsigned int chunk_size;
}mime_header_t;

typedef struct tcp_cli_data_t {
	int fd;
    mio_event_t *cli_event;
    void *gdata;
    void (*ipc_cmdfunc)(void *gdata, int sockfd, void *content, int content_len);
    parseflag_t pf;
    mime_header_t *mh;
    char *recvbuf;
    int recvbuf_len;
    char *content;
    int content_len;
}tcp_cli_data_t;

void init_ipc_tcp(cmdipc_t *cmdipc_pool, ipc_cmdfunc cmdparser, void *gdata);
void uninit_ipc_tcp(cmdipc_t *cmdipc_pool);
#endif
