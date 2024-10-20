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
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "ipc_udp.h"
#include "cmdparser.h"

int udp_sock(void *data)
{
	struct sockaddr_un sock_un;
	udp_data_t *udpdata;
	if (data == NULL){
		printf("create sock error\n");
		return -1;
	}
	udpdata = (udp_data_t *) data;
	memset(&sock_un, 0, sizeof(struct sockaddr_un));

	if ((udpdata->fd = socket(AF_UNIX, SOCK_DGRAM, 0))== -1 ){
		perror("create socket:");
		return -1;
	}
	unlink(udpdata->access_point);
	sock_un.sun_family = AF_UNIX;
	strcpy(sock_un.sun_path, udpdata->access_point);

	if (bind(udpdata->fd, (struct sockaddr *)&sock_un, sizeof(struct sockaddr_un)) != 0){
		perror("bind:");
		return -1;
	}
	return udpdata->fd;
}

void udp_close(void *data)
{
	udp_data_t *udpdata;
	if (data == NULL){
		printf("udp_close, data not initialized\n");
	}
	udpdata = (udp_data_t *) data;

	if (udpdata->fd > 0)
	{
		close(udpdata->fd);
	}

}

void udp_recv(void *data)
{	int fd;
	int res = -1;
	udp_data_t *udpdata;
	char recvbuf[1024];
	struct sockaddr_un from;
	socklen_t fromlen;

	if (data == NULL){
		printf("create sock error\n");
		return;
	}
	udpdata = (udp_data_t *) data;
	fd = udpdata->fd;
	if (fd <= 0){
		printf("fd error:[%d]\n", fd);
		return;
	}

        memset(recvbuf, 0, sizeof(recvbuf));
        memset(&from, 0, sizeof(struct sockaddr_un));
	fromlen = sizeof(from);
	if ((res= recvfrom (fd, recvbuf, sizeof(recvbuf), 0,
			(struct sockaddr *) &from, &fromlen)) == -1){
		perror("recvfrom error:");
		return;
	}
	//printf("from: [%s]\n", from.sun_path);
	memcpy(&(udpdata->from), &from, sizeof(struct sockaddr_un));
	memcpy(&(udpdata->fromlen), &fromlen, sizeof(socklen_t));

	//printf("recv:[%s]\n", recvbuf);
	if (udpdata->ipc_cmdfunc){
		udpdata->ipc_cmdfunc(udpdata, recvbuf);
	}
	else{
		printf("ipc_cmdfunc error\n");
	}
}
int udp_send(void *udata, void *data)
{
	if (udata == NULL || data == NULL){
		printf("invalid data\n");
		return -1;
	}
	udp_data_t *udpdata = (udp_data_t *)udata;
	char *sendbuf = (char *)data;
	int result;
	int byteSent;
	int len;

	len = strlen(sendbuf);
	byteSent = sendto(udpdata->fd, sendbuf, len, 0, (struct sockaddr *) &(udpdata->from), udpdata->fromlen);
	if (byteSent != len){
		perror("send err:");
		return -1;
	}
	return 0;
}
extern void cmdparser(void *udata, void *data);
static ipc_t ipc_udp =
{
	.name = "udp",
	.access_point = "/tmp/linkmon.unix",
	.ipc_sockfunc = udp_sock,
	.ipc_recvfunc = udp_recv,
	.ipc_sendfunc = udp_send,
	.ipc_closesockfunc = udp_close,
	.ipc_cmdfunc = cmdparser
};

void init_ipc_udp(cmdipc_t *cmdipc_pool, linkmon_t *linkmon, int *terminate_ignore)
{
	udp_data_t *udpdata;
	udpdata = (udp_data_t *)calloc(1,sizeof(udp_data_t));
	if(!udpdata) {
		perror("calloc");
		return;
	}
	strcpy(udpdata->access_point, ipc_udp.access_point);
	udpdata->ipc_cmdfunc = ipc_udp.ipc_cmdfunc;
	udpdata->ipc_sendfunc = ipc_udp.ipc_sendfunc;
	udpdata->linkmon = linkmon;
	udpdata->terminate_ignore = terminate_ignore;
	// TODO: cmdfunc should be independent from ipc_xxx.c? --> how about recv buffer handling?
	ipc_udp.cmdipc = add_cmdipc(cmdipc_pool, ipc_udp.name, ipc_udp.access_point, ipc_udp.ipc_sockfunc, ipc_udp.ipc_recvfunc, ipc_udp.ipc_sendfunc, ipc_udp.ipc_closesockfunc, ipc_udp.ipc_cmdfunc, (void *)udpdata);
}
void uninit_ipc_udp(cmdipc_t *cmdipc_pool)
{
	cmdipc_t *cmdipc;
	if(!cmdipc_pool)
		return;
	cmdipc = ipc_udp.cmdipc;
	if (cmdipc == NULL)
		return;

	if (cmdipc->data != NULL){
		free(cmdipc->data);
		cmdipc->data = NULL;
	}
	remove_cmdipc(cmdipc_pool, cmdipc);
	ipc_udp.cmdipc = NULL;
}
