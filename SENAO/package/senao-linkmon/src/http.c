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
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "mio.h"
#include "http.h"
#include "util.h"

#define HTTP_RCV_BUF_SIZE 1024

int http_init(void *mdata, void *data);
void http_close(void *data);
void http_send(void *data);
void http_recv(void *data);
int http_status(void *data);
char *http_getmsg(void *data);

static monitor_http_t monitor_http =
{
	.name = "http",
	.sec = 5,
	.usec = 0,
	.lm_initfunc = http_init,
	.lm_sendfunc = http_send,
	.lm_recvfunc = http_recv,
	.lm_statusfunc = http_status,
	.lm_msgfunc = http_getmsg,
	.lm_uninitfunc = http_close,
	.desthostname = "aws.amazon.com",
	.destpath = "/",
};

static struct addrinfo *getHostInfo(char* host, char* port)
{
	int r;
	struct addrinfo hints, *getaddrinfo_res;
	// Setup hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	if ((r = getaddrinfo(host, port, &hints, &getaddrinfo_res)) != 0) {
		fprintf(stderr, "getHostInfo: %s\n", gai_strerror(r));
		return NULL;
	}
	return getaddrinfo_res;
}

void http_clean_connection(void *data)
{
	if (data == NULL){
		lmdbg("socket_send_data, data not initialized\n");
		return;
	}
	
	http_data_t *httpdata;
	httpdata = (http_data_t *)data;
	// connect timeout
	if (httpdata->mio_event_write){
		remove_event(httpdata->mio_event_write);
		httpdata->mio_event_write = NULL;
	}
	close(httpdata->fd);
	httpdata->fd = 0;
	httpdata->status = HTTP_NO_RESPONSE;
}

void http_clean_rcv(void *data)
{
	if (data == NULL){
		lmdbg("http_clean_rcv, data not initialized\n");
		return;
	}
	
	http_data_t *httpdata;
	httpdata = (http_data_t *)data;
	// receive timeout
	if (httpdata->mio_event_read){
		remove_event(httpdata->mio_event_read);
		httpdata->mio_event_read = NULL;
	}
	close(httpdata->fd);
	httpdata->fd = 0;
	httpdata->status = HTTP_NO_RESPONSE;
}

void socket_send_data(void *data)
{
	if (data == NULL){
		lmdbg("socket_send_data, data not initialized\n");
		return;
	}

	mio_data_t *mio_data;
	http_data_t *httpdata;
	mio_event_t *mio_event;
	httpdata = (http_data_t *)data;
	mio_data = (mio_data_t *)httpdata->mio_data;

	char req[1024] = {0};
	sprintf(req, "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: linktest\r\nConnection: close\r\nAccept: */*\r\n\r\n", httpdata->path, httpdata->hostname);
	lmdbg("send packet:[%s]\n", req);
	if (send(httpdata->fd, req, strlen(req), 0) == -1)
	{
		lmdbg("sendto error\n");
		httpdata->status = HTTP_SEND_ERR;
		close(httpdata->fd);
		httpdata->fd = 0;
		return;
	}
	mio_event_t *er;
	mio_timer_t *mt_rcv;
	er = add_event(&(mio_data->read_pool), httpdata->fd, (event_func)monitor_http.lm_recvfunc, data);
	httpdata->mio_event_read=(void *)er;

	mt_rcv = add_timer(&(mio_data->timer_pool), 2, 0, http_clean_rcv, (void *)httpdata, 0);
	httpdata->mio_timer_rcv = mt_rcv;
}

void http_check_connection_and_send(void *data)
{
	if (data == NULL){
		lmdbg("socket_send_data, data not initialized\n");
		return;
	}
	http_data_t *httpdata;
	mio_timer_t *mio_timer_conn;
	unsigned so_len;
	int so_error;
	httpdata = (http_data_t *)data;
	mio_timer_conn = (mio_timer_t *)httpdata->mio_timer_conn;
	// remove clean_conn timer
	if (mio_timer_conn){
		remove_timer(mio_timer_conn);
		httpdata->mio_timer_conn = NULL;
	}
	// remove event itself
	if (httpdata->mio_event_write){
		remove_event(httpdata->mio_event_write);
		httpdata->mio_event_write = NULL;
	}
	so_len = sizeof(so_error);
	so_error = 0;
	getsockopt (httpdata->fd, SOL_SOCKET, SO_ERROR, &so_error, &so_len);
	if (so_error == 0){ // connect ok
		// send data
		socket_send_data((void *)httpdata);
	}
	else{ // connect error
		lmdbg("http connect error\n");
		close(httpdata->fd);
		httpdata->fd = 0;
		httpdata->status = HTTP_CONN_ERR;
	}
}

int http_init(void *mdata, void *data)
{
	if (mdata == NULL || data == NULL){
		lmdbg("init error\n");
		return -1;
	}
	mio_data_t *mio_data;
	http_data_t *httpdata;
	mio_data = (mio_data_t *)mdata;
	httpdata = (http_data_t *)data;
	httpdata->mio_data = (void *) mio_data;
	if (monitor_http.lm_sendfunc){
		mio_timer_t *mt;
		mt = add_timer(&(mio_data->timer_pool), monitor_http.sec, monitor_http.usec, (timer_func)monitor_http.lm_sendfunc, (void *)httpdata, 1);
		httpdata->mio_timer_snd = (void *) mt;
	}
	else {
		httpdata->status = HTTP_CREATE_ERR; 
		return -1;
	}
	return 0;
}

void http_close(void *data)
{
	if (data == NULL){
		lmdbg("http_close, data not initialized\n");
		return;
	}
	http_data_t *httpdata;
	mio_timer_t *mio_timer;
	httpdata = (http_data_t *) data;
	mio_timer = (mio_timer_t *)httpdata->mio_timer_snd;
	if (mio_timer != NULL){
		remove_timer(mio_timer);
		httpdata->mio_timer_snd = NULL;
	}
	if (httpdata->fd > 0)
	{
		close(httpdata->fd);
		httpdata->fd = 0;
	}
}

void http_send(void *data)
{
	if (data == NULL){
		lmdbg("send data broken error\n");
		return;
	}
	int fd;
	http_data_t *httpdata;
	mio_data_t *mio_data;
	httpdata = (http_data_t *)data;
	mio_data = (mio_data_t *)httpdata->mio_data;

	if (httpdata->fd > 0){
		lmdbg("previous fd job not finished, ignored. fd:[%d]\n", httpdata->fd);
		return;
	}

	// check if dns ok: TODO: cross reference, how to make it independent?
//	traverse_linkmon(monitor_http.linkmon);
	linkmon_t *dnsmon;
	if ((dnsmon = find_linkmon_by_name(monitor_http.linkmon, "dns")) == NULL ||
		dnsmon->lm_statusfunc == NULL ||
		dnsmon->data == NULL){
		httpdata->status = HTTP_DNS_ERR;
		return;
	}
	int dns_status = dnsmon->lm_statusfunc(dnsmon->data);
	if (dns_status == LINK_INIT ){
		httpdata->status = LINK_INIT;
		return;
	}
	if (dns_status != STATUS_OK){
		httpdata->status = HTTP_DNS_ERR;
		return;
	}
	lmdbg("dns status: %d\n", dnsmon->lm_statusfunc(dnsmon->data));
	// dns status ok

	struct addrinfo *info;
	if ((info = getHostInfo(httpdata->hostname, "http")) == NULL){
		lmdbg("gethostinfo fail\n");
		httpdata->status = HTTP_DNS_ERR;
		return;
	}
	for (; info != NULL; info = info->ai_next) {
		if ((fd = socket(info->ai_family,
					info->ai_socktype,
					info->ai_protocol)) > 0) {
			int flags = 0;
			if( (flags = fcntl(fd, F_GETFL, 0)) < 0){
				fd = 0;
				continue;
			}
			if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0){
				fd = 0;
				continue;
			}
			if (connect(fd, info->ai_addr, info->ai_addrlen) > 0 ||
				errno == EINPROGRESS){
				break;
			}else{
				perror("connect:");
				close(fd);
				fd = 0;
				continue;
			}
		}
	}
	freeaddrinfo(info);
	lmdbg("got addr info\n");

	if (fd <= 0){
		lmdbg("send fd error\n");
		httpdata->status = HTTP_SEND_ERR;
		return;
	}
	httpdata->fd = fd;

	if (errno == EINPROGRESS){ // set write pool for checking connection and send, set timer for timeout remove
		// connect non-block, add write pool to get connect result
		mio_event_t *me_conn;
		mio_timer_t *mt_conn;
		lmdbg("add connection and send event\n");
		me_conn = add_event(&(mio_data->write_pool), httpdata->fd, (event_func)http_check_connection_and_send, (void *)httpdata);
		httpdata->mio_event_write = me_conn;

		lmdbg("add timeout recycling timer\n");
		mt_conn = add_timer(&(mio_data->timer_pool), 2, 0, http_clean_connection, (void *)httpdata, 0);
		httpdata->mio_timer_conn = mt_conn;
	}else{
		// connect non-block but return at once, send data 
		lmdbg("add connection and send event, for non-block return at once\n");
		socket_send_data((void *)httpdata);
	}
}
void http_recv(void *data)
{
	char buf[HTTP_RCV_BUF_SIZE];
	if (data == NULL){
		lmdbg("http_recv, data not initialized\n");
		return;
	}
	http_data_t *httpdata;
	mio_event_t *mio_event;
	mio_timer_t *mio_timer_rcv;
	httpdata = (http_data_t *)data;
	mio_event = (mio_event_t *)httpdata->mio_event_read;
	mio_timer_rcv = (mio_timer_t *)httpdata->mio_timer_rcv;
	// remove clean_conn timer
	if (mio_timer_rcv){
		remove_timer(mio_timer_rcv);
		httpdata->mio_timer_rcv = NULL;
	}
	if (httpdata->fd <= 0){
		httpdata->status = HTTP_RCV_ERR;
		return;
	}
	int nread;
	ioctl(httpdata->fd, FIONREAD, &nread);
	lmdbg("recv fd[%d] nread[%d]\n", httpdata->fd, nread);
	if (nread == 0){ // socket close
		lmdbg("recv buf:[%s]\n", httpdata->recvbuf);
		if (strstr(httpdata->recvbuf, "301 Moved Permanently") != NULL)
			httpdata->status = STATUS_OK;
		else
			httpdata->status = HTTP_PAGE_ERR;

		close(httpdata->fd);
		httpdata->fd = 0;
		free(httpdata->recvbuf);
		httpdata->recvbuf = NULL;
		httpdata->recvbuf_len = 0;
		remove_event(mio_event);
		httpdata->mio_event_read = NULL;
	}else{
		char tmpbuf[HTTP_RCV_BUF_SIZE];
		int byteread;
		
		byteread = recv(httpdata->fd, tmpbuf, sizeof(tmpbuf), 0);
		if (byteread > 0){
			//lmdbg("tmpbuf read(%d):[%s]\n", byteread, tmpbuf);
			if (httpdata->recvbuf_len > 0)
				httpdata->recvbuf = realloc(httpdata->recvbuf, httpdata->recvbuf_len + byteread + 1);
			else
				httpdata->recvbuf = malloc(byteread + 1);
			if (httpdata->recvbuf == NULL){
				perror("realloc fail:\n");
				httpdata->status = HTTP_RCV_ERR;
				close(httpdata->fd);
				httpdata->fd = -1;
				return;
			}
			memcpy(httpdata->recvbuf + httpdata->recvbuf_len, tmpbuf, byteread);
			httpdata->recvbuf_len += byteread;
			*(httpdata->recvbuf + httpdata->recvbuf_len) = 0;
			//fputs(httpdata->recvbuf, stdout);
		}
	}
}
int http_status(void *data)
{
	http_data_t *httpdata;
	if (data == NULL){
		return HTTP_DATA_ERR;
	}
	httpdata = (http_data_t *)data;
	return httpdata->status;
}
char *http_getmsg(void *data)
{
	return "";
}

void init_http(linkmon_t *linkmon_pool, char *query_host)
{
	http_data_t *httpdata;
	httpdata = (http_data_t *)calloc(1,sizeof(http_data_t));
	if(!httpdata) {
		perror("calloc");
		return;
	}
	if (*query_host != '\0')
		strcpy(httpdata->hostname, query_host);
	else
		strcpy(httpdata->hostname, monitor_http.desthostname);

	strcpy(httpdata->path, monitor_http.destpath);

	httpdata->status = LINK_INIT;
	monitor_http.linkmon = add_linkmon(linkmon_pool, monitor_http.name, monitor_http.sec, monitor_http.usec, monitor_http.lm_initfunc, monitor_http.lm_sendfunc, monitor_http.lm_recvfunc, monitor_http.lm_statusfunc, monitor_http.lm_msgfunc, monitor_http.lm_uninitfunc, (void *)httpdata);
}

void uninit_http(linkmon_t *linkmon_pool)
{
	linkmon_t *linkmon;
	if (!linkmon_pool) 
		return;

//	linkmon = find_linkmon_by_name(linkmon_pool, monitor_icmp.name); 
	linkmon = monitor_http.linkmon;
	if (linkmon == NULL)
		return;

	if (linkmon->data != NULL){
		free(linkmon->data);
		linkmon->data = NULL;
	}
	remove_linkmon(linkmon_pool, linkmon);
	monitor_http.linkmon = NULL;
}


/*
int main(int argc, char **argv)
{
	int sockfd;
	sockfd = create_socket_icmpv4();
	in_addr_t addr;
	addr = inet_addr(argv[1]);
	unsigned char *ipBytes;
	ipBytes = (unsigned char *)&addr;
	send_icmpv4(sockfd, ipBytes);

	return 0;
}
*/
