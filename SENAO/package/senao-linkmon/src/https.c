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
#if defined(OPENSSL_DEV)
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif
#include <errno.h>
#include "mio.h"
#include "https.h"
#include "util.h"

#define HTTPS_RCV_BUF_SIZE 1024

int https_init(void *mdata, void *data);
void https_close(void *data);
void https_send(void *data);
void https_recv(void *data);
int https_status(void *data);
char *https_getmsg(void *data);

static monitor_https_t monitor_https =
{
	.name = "https",
	.sec = 5,
	.usec = 0,
	.lm_initfunc = https_init,
	.lm_sendfunc = https_send,
	.lm_recvfunc = https_recv,
	.lm_statusfunc = https_status,
	.lm_msgfunc = https_getmsg,
	.lm_uninitfunc = https_close,
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

void https_clean_connection(void *data)
{
	if (data == NULL){
		lmdbg("socket_send_data, data not initialized\n");
		return;
	}

	https_data_t *httpsdata;
	httpsdata = (https_data_t *)data;
	// connect timeout
	if (httpsdata->mio_event_write){
		remove_event(httpsdata->mio_event_write);
		httpsdata->mio_event_write = NULL;
	}
	close(httpsdata->fd);
	httpsdata->fd = 0;
	httpsdata->status = HTTPS_NO_RESPONSE;
}

void https_clean_rcv(void *data)
{
	if (data == NULL){
		lmdbg("https_clean_rcv, data not initialized\n");
		return;
	}

	https_data_t *httpsdata;
	httpsdata = (https_data_t *)data;
	// receive timeout
	if (httpsdata->mio_event_read){
		remove_event(httpsdata->mio_event_read);
		httpsdata->mio_event_read = NULL;
	}
	close(httpsdata->fd);
	httpsdata->fd = 0;
	httpsdata->status = HTTPS_NO_RESPONSE;
}

static void socket_send_data(void *data)
{
	if (data == NULL){
		lmdbg("socket_send_data, data not initialized\n");
		return;
	}

	mio_data_t *mio_data;
	https_data_t *httpsdata;
	mio_event_t *mio_event;
	httpsdata = (https_data_t *)data;
	mio_event = (mio_event_t *)httpsdata->mio_event_write;
	mio_data = (mio_data_t *)httpsdata->mio_data;
	// https client hello use ssl or just ignore (test connection only)
#if defined(OPENSSL_DEV)
	SSL_CTX *ctx;
	SSL *ssl;
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(SSLv23_client_method());
	//ctx = SSL_CTX_new(SSLv3_client_method());
	if (ctx == NULL) {
		lmdbg("ctx create error\n");
		httpsdata->status = HTTPS_CREATE_ERR;
		close(httpsdata->fd);
		httpsdata->fd = 0;
		return;
	}

	ssl = SSL_new(ctx);
	if (!ssl) {
		lmdbg("could not SSL_new\n");
		httpsdata->status = HTTPS_CREATE_ERR;
		close(httpsdata->fd);
		httpsdata->fd = 0;
		SSL_CTX_free(ctx);
		return;
	}
	if (!SSL_set_fd(ssl, httpsdata->fd)) {
		lmdbg("could not SSL_set_fd\n");
		httpsdata->status = HTTPS_HANDSHAKE_ERR;
		close(httpsdata->fd);
		httpsdata->fd = 0;
		SSL_free(ssl);
		SSL_CTX_free(ctx);
		return;
	}
	SSL_set_connect_state(ssl);
	int ssl_conn_ret, ssl_err_ret;
	ssl_conn_ret = SSL_connect(ssl);
	if (ssl_conn_ret <= 0){
		switch (SSL_get_error(ssl, ssl_err_ret))
		{
			case SSL_ERROR_NONE:
				//lmdbg("Error NONE ssl_conn_ret: %d fd:[%d]\n", ssl_conn_ret, httpsdata->fd);
				{

					close(httpsdata->fd);
					httpsdata->fd = 0;
					httpsdata->status = STATUS_OK;
					SSL_shutdown(ssl);
					SSL_free(ssl);
					SSL_CTX_free(ctx);
					return;
				}
				break;
			case SSL_ERROR_ZERO_RETURN:
				lmdbg("Error SSL_ERROR_ZERO_RETURN\n");
				break;
			case SSL_ERROR_WANT_CONNECT:
				lmdbg("Connect Error SSL_ERROR_WANT_CONNECT\n");
				break;
			case SSL_ERROR_WANT_READ:
				lmdbg("Read Error SSL_ERROR_WANT_READ\n");
				break; 
			case SSL_ERROR_WANT_WRITE:
				lmdbg("Write Error SSL_ERROR_WANT_WRITE ssl_conn_ret:%d\n", ssl_conn_ret);
				break;
			case SSL_ERROR_WANT_X509_LOOKUP:
				lmdbg("Write Error SSL_ERROR_WANT_X509_LOOKUP\n");
				break;
			case SSL_ERROR_SYSCALL:
				lmdbg("Write Error SSL_ERROR_SYSCALL ssl_conn_ret: %d fd:%d\n", ssl_conn_ret, httpsdata->fd);
				long error = ERR_get_error();
				const char* error_str = ERR_error_string(error, NULL);
				lmdbg("could not SSL_connect: %s\n", error_str);
				break;
			case SSL_ERROR_SSL:
				lmdbg("Write Error SSL_ERROR_SSL\n");
				break;
			default:
				lmdbg("ssl_conn_ret:%d errno: %d\n", ssl_conn_ret,ssl_err_ret);
		}
		httpsdata->status = HTTPS_HANDSHAKE_ERR;
		SSL_free(ssl);
		close(httpsdata->fd);
		SSL_CTX_free(ctx);
		httpsdata->fd = 0;
	} else if (ssl_conn_ret == 1){
		httpsdata->status = STATUS_OK;
		SSL_shutdown(ssl);
		SSL_free(ssl);
		close(httpsdata->fd);
		SSL_CTX_free(ctx);
		httpsdata->fd = 0;
	} else {
		lmdbg("will never happen, ssl_conn_ret:%d\n", ssl_conn_ret);
		httpsdata->status = HTTPS_HANDSHAKE_ERR;
	}
#else
	httpsdata->status = STATUS_OK;
	close(httpsdata->fd);
	httpsdata->fd = 0;
#endif
}

void https_check_connection_and_send(void *data)
{
	if (data == NULL){
		lmdbg("socket_send_data, data not initialized\n");
		return;
	}
	https_data_t *httpsdata;
	mio_timer_t *mio_timer_conn;
	unsigned so_len;
	int so_error;
	httpsdata = (https_data_t *)data;
	mio_timer_conn = (mio_timer_t *)httpsdata->mio_timer_conn;
	// remove clean_conn timer
	if (mio_timer_conn){
		remove_timer(mio_timer_conn);
		httpsdata->mio_timer_conn = NULL;
	}
	// remove event itself
	if (httpsdata->mio_event_write){
		remove_event(httpsdata->mio_event_write);
		httpsdata->mio_event_write = NULL;
	}

	so_len = sizeof(so_error);
	so_error = 0;
	getsockopt (httpsdata->fd, SOL_SOCKET, SO_ERROR, &so_error, &so_len);
	if (so_error == 0){ // connect ok
		// send data
		socket_send_data((void *)httpsdata);
	}
	else{ // connect error
		close(httpsdata->fd);
		httpsdata->fd = 0;
		httpsdata->status = HTTPS_CONN_ERR;
	}
}

int https_init(void *mdata, void *data)
{
	if (mdata == NULL || data == NULL){
		lmdbg("init error\n");
		return -1;
	}
	mio_data_t *mio_data;
	https_data_t *httpsdata;
	mio_data = (mio_data_t *)mdata;
	httpsdata = (https_data_t *)data;
	httpsdata->mio_data = (void *) mio_data;
	if (monitor_https.lm_sendfunc){
		mio_timer_t *mt;
		mt = add_timer(&(mio_data->timer_pool), monitor_https.sec, monitor_https.usec, (timer_func)monitor_https.lm_sendfunc, (void *)httpsdata, 1);
		httpsdata->mio_timer_snd = (void *) mt;
	}
	else {
		httpsdata->status = HTTPS_CREATE_ERR;
		return -1;
	}
	return 0;
}

void https_close(void *data)
{
	if (data == NULL){
		lmdbg("https_close, data not initialized\n");
		return;
	}
	https_data_t *httpsdata;
	mio_timer_t *mio_timer;
	httpsdata = (https_data_t *) data;
	mio_timer = (mio_timer_t *)httpsdata->mio_timer_snd;
	if (mio_timer != NULL){
		remove_timer(mio_timer);
		httpsdata->mio_timer_snd = NULL;
	}
	if (httpsdata->fd > 0)
	{
		close(httpsdata->fd);
		httpsdata->fd = 0;
	}
}

void https_send(void *data)
{
	if (data == NULL){
		lmdbg("send data broken error\n");
		return;
	}
	int fd;
	https_data_t *httpsdata;
	mio_data_t *mio_data;
	httpsdata = (https_data_t *)data;
	mio_data = (mio_data_t *)httpsdata->mio_data;

	if (httpsdata->fd > 0){
		lmdbg("previous fd job not finished, ignored. fd:[%d]\n", httpsdata->fd);
		return;
	}

	// check if dns ok: TODO: cross reference, how to make it independent?
//	traverse_linkmon(monitor_https.linkmon);
	linkmon_t *dnsmon;
	if ((dnsmon = find_linkmon_by_name(monitor_https.linkmon, "dns")) == NULL ||
		dnsmon->lm_statusfunc == NULL ||
		dnsmon->data == NULL){
		httpsdata->status = HTTPS_DNS_ERR;
		return;
	}
	int dns_status = dnsmon->lm_statusfunc(dnsmon->data);
	if (dns_status == LINK_INIT ){
		httpsdata->status = LINK_INIT;
		return;
	}
	if (dns_status != STATUS_OK){
		httpsdata->status = HTTPS_DNS_ERR;
		return;
	}
	//lmdbg("dns status: %d\n", dnsmon->lm_statusfunc(dnsmon->data));
	// dns status ok

	struct addrinfo *info;
	if ((info = getHostInfo(httpsdata->hostname, "https")) == NULL){
		lmdbg("gethostinfo fail\n");
		httpsdata->status = HTTPS_DNS_ERR;
		return;
	}
	for (; info != NULL; info = info->ai_next) {
		if ((fd = socket(info->ai_family,
					info->ai_socktype,
					info->ai_protocol)) > 0) {
//			int one=1;
			int flags = 0;
			if( (flags = fcntl(fd, F_GETFL, 0)) < 0){
				close(fd);
				fd = 0;
				continue;
			}
			if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0){
				close(fd);
				fd = 0;
				continue;
			}
/*			if (setsockopt(fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one))) {
				lmdbg("could not setsockopt\n");
				close(fd);
				fd = 0;
				continue;
			}*/
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

	if (fd <= 0){
		lmdbg("send fd error\n");
		httpsdata->status = HTTPS_SEND_ERR;
		return;
	}
	httpsdata->fd = fd;

	if (errno == EINPROGRESS){ // set write pool for checking connection and send, set timer for timeout remove
		// connect non-block, add write pool to get connect result
		mio_event_t *me_conn_write, *me_conn_read;
		mio_timer_t *mt_conn;
		me_conn_write = add_event(&(mio_data->write_pool), httpsdata->fd, (event_func)https_check_connection_and_send, (void *)httpsdata);
		httpsdata->mio_event_write = me_conn_write;

		mt_conn = add_timer(&(mio_data->timer_pool), 2, 0, https_clean_connection, (void *)httpsdata, 0);
		httpsdata->mio_timer_conn = mt_conn;
	}else{
		// connect non-block but return at once, send data
		socket_send_data((void *)httpsdata);
	}
}
void https_recv(void *data)
{
	char buf[HTTPS_RCV_BUF_SIZE];
	if (data == NULL){
		lmdbg("https_recv, data not initialized\n");
		return;
	}
	https_data_t *httpsdata;
	mio_event_t *mio_event;
	mio_timer_t *mio_timer_rcv;
	httpsdata = (https_data_t *)data;
	mio_event = (mio_event_t *)httpsdata->mio_event_read;
	mio_timer_rcv = (mio_timer_t *)httpsdata->mio_timer_rcv;
	// remove clean_conn timer
	if (mio_timer_rcv){
		remove_timer(mio_timer_rcv);
		httpsdata->mio_timer_rcv = NULL;
	}
	if (httpsdata->fd <= 0){
		httpsdata->status = HTTPS_RCV_ERR;
		return;
	}
	int nread;
	ioctl(httpsdata->fd, FIONREAD, &nread);
	//lmdbg("recv fd[%d] nread[%d]\n", httpsdata->fd, nread);
	if (nread == 0){ // socket close
		// TODO: read server hello if no openssl support
		//lmdbg("recv: [%s]\n",httpsdata->recvbuf);
		close(httpsdata->fd);
		httpsdata->fd = 0;
		free(httpsdata->recvbuf);
		httpsdata->recvbuf = NULL;
		httpsdata->recvbuf_len = 0;
		remove_event(mio_event);
		httpsdata->mio_event_read = NULL;
	}else{
		char tmpbuf[HTTPS_RCV_BUF_SIZE];
		int byteread;
		byteread = recv(httpsdata->fd, tmpbuf, sizeof(tmpbuf), 0);
		if (byteread > 0){
			if (httpsdata->recvbuf_len > 0)
				httpsdata->recvbuf = realloc(httpsdata->recvbuf, httpsdata->recvbuf_len + byteread + 1);
			else
				httpsdata->recvbuf = malloc(byteread + 1);
			if (httpsdata->recvbuf == NULL){
				perror("realloc fail:\n");
				httpsdata->status = HTTPS_RCV_ERR;
				close(httpsdata->fd);
				httpsdata->fd = -1;
				return;
			}
			memcpy(httpsdata->recvbuf + httpsdata->recvbuf_len, tmpbuf, byteread);
			httpsdata->recvbuf_len += byteread;
			*(httpsdata->recvbuf + httpsdata->recvbuf_len) = 0;
			//fputs(httpsdata->recvbuf, stdout);
		}
	}
}
int https_status(void *data)
{
	https_data_t *httpsdata;
	if (data == NULL){
		return HTTPS_DATA_ERR;
	}
	httpsdata = (https_data_t *)data;
	return httpsdata->status;
}
char *https_getmsg(void *data)
{
	return "";
}

void init_https(linkmon_t *linkmon_pool, char *query_host)
{
	https_data_t *httpsdata;
	httpsdata = (https_data_t *)calloc(1,sizeof(https_data_t));
	if(!httpsdata) {
		perror("calloc");
		return;
	}
	if (*query_host != '\0')
		strcpy(httpsdata->hostname, query_host);
	else
		strcpy(httpsdata->hostname, monitor_https.desthostname);

	strcpy(httpsdata->path, monitor_https.destpath);
	httpsdata->status = LINK_INIT;
	monitor_https.linkmon = add_linkmon(linkmon_pool, monitor_https.name, monitor_https.sec, monitor_https.usec, monitor_https.lm_initfunc, monitor_https.lm_sendfunc, monitor_https.lm_recvfunc, monitor_https.lm_statusfunc, monitor_https.lm_msgfunc, monitor_https.lm_uninitfunc, (void *)httpsdata);
}

void uninit_https(linkmon_t *linkmon_pool)
{
	linkmon_t *linkmon;
	if (!linkmon_pool)
		return;

//	linkmon = find_linkmon_by_name(linkmon_pool, monitor_https.name);
	linkmon = monitor_https.linkmon;
	if (linkmon == NULL)
		return;

	if (linkmon->data != NULL){
		free(linkmon->data);
		linkmon->data = NULL;
	}
	remove_linkmon(linkmon_pool, linkmon);
	monitor_https.linkmon = NULL;
}
