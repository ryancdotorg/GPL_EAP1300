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
#ifndef _IPC_NL_H_
#define _IPC_NL_H_
#include "cmdipc.h"
#include <sys/socket.h>

typedef void (*nl_cmdfunc)(void *gdata, int sockfd, void *content, int content_len);
typedef struct nl_srv_data_t {
    void *gdata; //gdata_t
    int fd;
    char access_point[64];
    int iov_len;
    char *nlmsg_buf;
    int nlmsg_buflen;
    int seq;
    char *content;
    int content_len;
    int (*nl_initfunc)(void *); // nl_srv_data_t
    void (*nl_recvfunc)(void *nl_srv_data);
    void (*nl_cmdfunc)(void *gdata, int sockfd, void *content, int content_len);
}nl_srv_data_t;

void init_ipc_nl(cmdipc_t *cmdipc_pool, nl_cmdfunc nl_cmdparser, void *gdata);
void uninit_ipc_nl(cmdipc_t *cmdipc_pool);
#endif
