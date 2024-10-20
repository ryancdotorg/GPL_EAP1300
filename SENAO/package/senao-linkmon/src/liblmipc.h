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
#ifndef _LIBLMIPC_H_
#define _LIBLMIPC_H_
#include <sys/socket.h>
#include <sys/un.h>
#include <json.h>

// Private data
typedef struct ipc_handle_t{
	int sockfd;
	struct sockaddr_un addr_cli;
	struct sockaddr_un addr_srv;
	socklen_t addr_srv_len;
}ipc_handle_t;
ipc_handle_t *ipc_open();
int ipc_send(ipc_handle_t *ih, char *bufsend, int slen, char *bufrcv, int rlen);
void ipc_close(ipc_handle_t *ih);
#define MAX_RCV_BUFFER_SIZE 1024
// public data
typedef struct lm_handle_t{
	struct ipc_handle_t *ih;
	char *rdata;
	int rdata_len;
	json_object *robj;
	json_object *subobj;
}lm_handle_t;

struct lm_handle_t *lm_open();

struct lm_handle_t *lm_getobj(struct lm_handle_t *lh, char *path);
json_object *lm_getjobj(struct lm_handle_t *lh, char *path);
int  lm_getint(struct lm_handle_t *lh, char *path);
void lm_show_obj(lm_handle_t *lh, char *path);
void lm_close(struct lm_handle_t *lh);
#endif
