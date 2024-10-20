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
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "global.h"
#include "mio.h"
#include "ipc_tcp.h"
#include "util_socket.h"
#include "cmdparser.h"

int tcp_sock(void *data)
{
	tcp_srv_data_t *srvdata;
	if (data == NULL){
		printf("create sock error\n");
		return -1;
	}
	srvdata = (tcp_srv_data_t *) data;

    if ((srvdata->fd = create_socket(AF_UNIX, srvdata->access_point)) == -1){
		perror("create socket:");
        return -1;
    }
    return srvdata->fd;
}

void tcp_close(void *data)
{
	tcp_srv_data_t *srvdata;
	if (data == NULL){
		printf("tcp_close, data not initialized\n");
	}
	srvdata = (tcp_srv_data_t *) data;

	if (srvdata->fd > 0)
	{
		close(srvdata->fd);
	}

}

static unsigned char*
memfind(unsigned char *haystack, int hsize, unsigned char *needle, int nsize)
{
	unsigned char *i;
	for (i = haystack; i < haystack + hsize - nsize + 1; i++) {
		if (!memcmp(i, needle, nsize))
			return i;
	}
	return NULL;
}

buf_status_t mime_parse_header(tcp_cli_data_t *clidata)
{
	mime_header_t *mh = (mime_header_t *)clidata->mh;
	char *line_start, *line_end, *header_end;

	header_end = (char *)memfind(clidata->recvbuf, clidata->recvbuf_len, "\r\n\r\n", 4);

	///
	/// Did not read full mime header, return to read more
	///
	if (!header_end) 
		return EXHAUST;

	///
	/// Get full mime header, start to parse it
	///
	line_start = (char *)clidata->recvbuf;
	do {
		char *value_start;
		line_end = (char *)memfind(line_start, clidata->recvbuf_len, "\r\n", 2);
		*line_end = 0;
		///
		/// Get header line, start to parse it
		///
		if (!strncasecmp(line_start, "Content-Length:", 15)){
			value_start = line_start + 15;
			value_start += strspn(value_start, " \t");
			mh->content_length = atol(value_start);
		}
		line_start = line_end + 2;
	}while (line_end != header_end);
	///
	/// Delete the mime header from data_buffer
	///
	header_end += 4;
	memmove(clidata->recvbuf, header_end, clidata->recvbuf + clidata->recvbuf_len - header_end);
	clidata->recvbuf_len -= (header_end - clidata->recvbuf);
    //printf("content_length:%d\n", mh->content_length);

	clidata->pf = CHUNKHEADER;

	if (clidata->recvbuf_len == 0) 
		return EXHAUST;
	else
		return NOTEXHAUST;
}

buf_status_t mime_parse_chunkhdr(tcp_cli_data_t *clidata)
{
	mime_header_t *mh = (mime_header_t *)clidata->mh;
	char *header_end;

	header_end = memfind(clidata->recvbuf, clidata->recvbuf_len, "\r\n", 2);
	
	///
	/// Did not read full chunk header, return to read more
	///
	if (!header_end) 
		return EXHAUST;
	///
	/// Get full chunk header, start to parse it
	///
	*header_end = 0;
	if (!sscanf(clidata->recvbuf, "%x", &(mh->chunk_size))){
		printf("chunk size parse error\n");
	}
//	printf (" ============== chunk size: [%d] ======================\n", mh->chunk_size);
	///
	/// Delete the chunk header from data_buffer
	///
	header_end += 2; // \r and \n
	memmove(clidata->recvbuf, header_end, (char *)clidata->recvbuf + clidata->recvbuf_len - header_end);
	clidata->recvbuf_len -= (header_end - clidata->recvbuf);
	
	clidata->pf = CHUNKCONTENT;
	if (clidata->recvbuf_len == 0) 
		return EXHAUST;
	else
		return NOTEXHAUST;
}

buf_status_t mime_parse_chunkcontent(tcp_cli_data_t *clidata)
{
	mime_header_t *mh = (mime_header_t *)clidata->mh;
	char *header_end;

	if (clidata->recvbuf_len < mh->chunk_size + 2){
		return EXHAUST;
	}
	if (clidata->content_len > 0){
		clidata->content = realloc(clidata->content, clidata->content_len + mh->chunk_size);
	}
	else{
		clidata->content = calloc(1, mh->chunk_size);
	}
	if (clidata->content == NULL) {
        perror("alloc:");
		return EXHAUST;
    }
	memcpy(clidata->content + clidata->content_len, clidata->recvbuf, mh->chunk_size);
	clidata->content_len += mh->chunk_size;

	header_end = clidata->recvbuf + mh->chunk_size + 2;
	memmove(clidata->recvbuf, header_end, (char *)clidata->recvbuf + clidata->recvbuf_len - header_end);
	clidata->recvbuf_len -= (header_end - (char *)clidata->recvbuf);

	if (mh->chunk_size == 0){
		///
		/// got full data and put in content
		///
		//printf("content:[%s] content len:[%d]\n", clidata->content, clidata->content_len);
		if (clidata->ipc_cmdfunc){
		    clidata->content = realloc(clidata->content, clidata->content_len + 1);
            *(clidata->content + clidata->content_len) = '\0';
			clidata->ipc_cmdfunc((void *)clidata->gdata, clidata->fd, clidata->content, clidata->content_len);
		}
		///
		/// Release handled data
		///
		clidata->pf = HEADER;
		if (clidata->recvbuf_len > 0 || clidata->recvbuf){
			clidata->recvbuf_len = 0;
			free(clidata->recvbuf);
			clidata->recvbuf = NULL;
		}
		if (clidata->content_len > 0 || clidata->content){
			clidata->content_len = 0;	
			free(clidata->content);
			clidata->content = NULL;
		}
		if (clidata->mh != NULL){
			free(clidata->mh);
			clidata->mh = NULL;
		}
	}
	else
		clidata->pf = CHUNKHEADER;

	if (clidata->recvbuf_len == 0) 
		return EXHAUST;
	else
		return NOTEXHAUST;
}

#define IPC_TCP_RCV_BUF_SIZE 1024
void tcp_recv(void *clidata)
{
	mio_data_t *mdata;
	tcp_cli_data_t *tcp_cli_data;
    gdata_t *gdata;
	char tmpbuf[IPC_TCP_RCV_BUF_SIZE];
	int byteread;
    int nread;
    buf_status_t buf_status = EXHAUST;


	if (clidata == NULL){
		printf("recv sock error\n");
		return;
	}
	tcp_cli_data = (tcp_cli_data_t *) clidata;
    gdata = tcp_cli_data->gdata;
	mdata = (mio_data_t *) gdata->mio_data;

    ioctl(tcp_cli_data->fd, FIONREAD, &nread);
    if (nread == 0){ // socket close
        close(tcp_cli_data->fd);
		remove_event(tcp_cli_data->cli_event);
        if (tcp_cli_data->recvbuf!=NULL){
            free(tcp_cli_data->recvbuf);
	    tcp_cli_data->recvbuf=NULL;
        }
        if (tcp_cli_data->content!=NULL){
            free(tcp_cli_data->content);
            tcp_cli_data->content = NULL;
	}
        if (tcp_cli_data->mh!=NULL){
            free(tcp_cli_data->mh);
            tcp_cli_data->mh = NULL;
	}
        free(tcp_cli_data);
        return;
    }
	byteread = recv(tcp_cli_data->fd, tmpbuf, sizeof(tmpbuf), 0);

    if (byteread == -1){
        perror("read:");
        return;
    }
    //printf("(%d)%s fd:%d\n", byteread, tmpbuf, tcp_cli_data->fd);
    if (tcp_cli_data->recvbuf_len > 0){
        tcp_cli_data->recvbuf = realloc(tcp_cli_data->recvbuf, tcp_cli_data->recvbuf_len + byteread);
    }
    else {
        if (tcp_cli_data->recvbuf != NULL){
            free(tcp_cli_data->recvbuf);
	    tcp_cli_data->recvbuf = NULL;
	}
        tcp_cli_data->recvbuf = malloc(byteread);
    }
    if (tcp_cli_data->recvbuf == NULL){
        perror("malloc/realloc:");
        return;
    }

    memcpy(tcp_cli_data->recvbuf + tcp_cli_data->recvbuf_len, tmpbuf, byteread);
    tcp_cli_data->recvbuf_len += byteread;

    // parse recvbuf
    if (tcp_cli_data->mh == NULL){
		///
		/// Receive first packet
		///
		tcp_cli_data->mh = calloc(1, sizeof(mime_header_t));
        if (tcp_cli_data->mh == NULL){
            perror("calloc");
            return;
        }
		tcp_cli_data->pf = HEADER;
	}
    if (tcp_cli_data->pf == HEADER){
        if (mime_parse_header(tcp_cli_data) == EXHAUST)
            return;
    }
    do {
        if (tcp_cli_data->pf == CHUNKHEADER)
            if (mime_parse_chunkhdr(tcp_cli_data) == EXHAUST)
                    break;
        if (tcp_cli_data->pf == CHUNKCONTENT){
            if ((buf_status = mime_parse_chunkcontent(tcp_cli_data)) == EXHAUST)
                break;
        }
    } while(buf_status != EXHAUST);
}

void tcp_accept(void *srvdata)
{
	mio_data_t *mdata;
	tcp_srv_data_t *tcp_srv_data;

	tcp_cli_data_t *clidata;
    mio_event_t *m;
    struct sockaddr_in client_address;
    socklen_t client_len;
    int client_sockfd;

	if (srvdata == NULL){
		printf("create sock error\n");
		return;
	}
	tcp_srv_data = (tcp_srv_data_t *) srvdata;
    gdata_t *gdata = tcp_srv_data->gdata;
	mdata = (mio_data_t *) gdata->mio_data;
    
    client_len = sizeof(client_address);
    client_sockfd = accept(tcp_srv_data->fd, 
        (struct sockaddr *)&client_address,
        &client_len);
    if(client_sockfd == -1){
        perror("accept:");
        return; 
    }
    //printf("client fd to add:[%d]\n", client_sockfd);
	clidata = (tcp_cli_data_t *)calloc(1,sizeof(tcp_cli_data_t));
	if(!clidata) {
		perror("calloc");
		return;
	}
    clidata->fd = client_sockfd;
    clidata->gdata = tcp_srv_data->gdata;
    clidata->ipc_cmdfunc = tcp_srv_data->ipc_cmdfunc;
    m = add_event(&(mdata->read_pool), client_sockfd, (event_func)tcp_recv, (void *)clidata);
    clidata->cli_event = m;
}

static ipc_t ipc_tcp =
{
	.name = "tcp",
	.access_point = "/tmp/syseye.unix",
	.ipc_sockfunc = tcp_sock,
	.ipc_recvfunc = tcp_accept,
	.ipc_closesockfunc = tcp_close,
};

void init_ipc_tcp(cmdipc_t *cmdipc_pool, ipc_cmdfunc cmdfunc, void *gdata)
{
	tcp_srv_data_t *srvdata;
	srvdata = (tcp_srv_data_t *)calloc(1,sizeof(tcp_srv_data_t));
	if(!srvdata) {
		perror("calloc");
		return;
	}
    srvdata->gdata= gdata;
	strcpy(srvdata->access_point, ipc_tcp.access_point);
    srvdata->ipc_cmdfunc = cmdfunc;
	ipc_tcp.cmdipc = add_cmdipc(cmdipc_pool, ipc_tcp.name, ipc_tcp.access_point, ipc_tcp.ipc_sockfunc, ipc_tcp.ipc_recvfunc, ipc_tcp.ipc_closesockfunc, (void *)srvdata);
}

void uninit_ipc_tcp(cmdipc_t *cmdipc_pool)
{
	cmdipc_t *cmdipc;
	if(!cmdipc_pool)
		return;
	cmdipc = ipc_tcp.cmdipc;
	if (cmdipc == NULL)
		return;

	if (cmdipc->data != NULL){
		free(cmdipc->data); // srv_data, tcp_srv_data_t
		cmdipc->data = NULL;
	}
	remove_cmdipc(cmdipc_pool, cmdipc);
}
