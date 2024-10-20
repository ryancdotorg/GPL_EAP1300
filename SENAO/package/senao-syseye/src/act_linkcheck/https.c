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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h> // uint_8_t
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "https.h"
#include "statuscode.h"

#define HTTPS_RCV_BUF_SIZE 1024

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

static int socket_send_http_data(int fd, char *host, char *path)
{
	char req[1024] = {0};
	sprintf(req, "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: linkcheck\r\nConnection: close\r\nAccept: */*\r\n\r\n", path, host);
	if (send(fd, req, strlen(req), 0) == -1)
	{
		printf("sendto error\n");
		return -1;
	}
    return 0;
}

int check_https_status(char *query_host, char *query_path)
{
    int status = LINK_INIT;
	int fd;
	struct addrinfo *info;
    int flags;

	char *recvbuf = NULL, *tmp;
	int recvbuf_len;
	char tmpbuf[HTTPS_RCV_BUF_SIZE];
	int byteread;

	unsigned so_len;
	int so_error;

    fd_set readfds;
    fd_set writefds;
    struct timeval tv;
    int nselect = 1;
	int nread;
    int res = 0;

    if (query_host == NULL || query_path == NULL){
        status = HTTPS_DATA_ERR; 
        return status;
    }

	if ((info = getHostInfo(query_host, "https")) == NULL){
		printf("gethostinfo fail\n");
		status = HTTPS_DNS_ERR;
		return status;
	}
	for (; info != NULL; info = info->ai_next) {
		if ((fd = socket(info->ai_family,
					info->ai_socktype,
					info->ai_protocol)) > 0) {
			flags = 0;
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
	if (fd <= 0){
		printf("send fd error\n");
		status = HTTPS_SEND_ERR;
		return status;
	}

	if (errno == EINPROGRESS){ 
        FD_ZERO(&writefds); FD_SET(fd, &writefds);
        tv.tv_sec = 2;  tv.tv_usec = 0;
        while (1){
            nselect = select(fd + 1, NULL, &writefds, NULL, &tv);
            if (nselect > 0){
                so_len = sizeof(so_error);
                so_error = 0;
                getsockopt (fd, SOL_SOCKET, SO_ERROR, &so_error, &so_len);
                if (so_error == 0){ // connect ok
                    if (socket_send_http_data(fd, query_host, query_path) != 0){
                        status = HTTPS_SEND_ERR;
                    }
                    // else send success
                }else {
                    status = HTTPS_CONN_ERR;
                }
                break;
            }
            else if (nselect == 0){ // timeout
                status = HTTPS_CONN_ERR;
                break;  
            }
        }
    }else{
        if (socket_send_http_data(fd, query_host, query_path) != 0){
            status = HTTPS_SEND_ERR;
        }
        // else send success
    }
    if (status != LINK_INIT){
        close(fd);
        return status;
    }
    FD_ZERO(&readfds); FD_SET(fd, &readfds);
    tv.tv_sec = 3;  tv.tv_usec = 0;
	recvbuf_len = 0;

    while (1){
        nselect = select(fd + 1, &readfds, NULL, NULL, &tv);
        if (nselect > 0){
            ioctl(fd, FIONREAD, &nread);
            if (nread == 0){ // socket close
                if (recvbuf && strstr(recvbuf, "HTTP/1.1 ") != NULL)
                    status = STATUS_OK;
                else
                    status = HTTP_PAGE_ERR;
		if (recvbuf)
		    free(recvbuf);
                recvbuf = NULL;
                recvbuf_len = 0;
                break;
	    } else {
                memset(tmpbuf, 0, sizeof(tmpbuf));
                byteread = recv(fd, tmpbuf, sizeof(tmpbuf), 0);
                if (byteread > 0){
                    if (recvbuf_len > 0){
                        if ((tmp = realloc(recvbuf, recvbuf_len + byteread + 1)) == NULL){
                            perror("realloc fail:\n");
                            status = HTTP_RCV_ERR;
                            break;
                        }else{
                            recvbuf = tmp;
                        }
                    }
                    else {
                        if (recvbuf != NULL){
                            free(recvbuf);
			    recvbuf = NULL;
                        }
                        if ((tmp = calloc(1, byteread + 1)) == NULL){
                            perror("realloc fail:\n");
                            status = HTTP_RCV_ERR;
                            break;
                        }else{
                            recvbuf = tmp;
                        }
                    }
                    memcpy(recvbuf + recvbuf_len, tmpbuf, byteread);
                    recvbuf_len += byteread;
                    *(recvbuf + recvbuf_len) = 0;
                    //fputs(recvbuf, stdout);
                }
            }
        }
        else if (nselect == 0){ // timeout
            if (recvbuf != NULL){
                free(recvbuf);
                recvbuf = NULL;
            }
            status = HTTPS_NO_RESPONSE;
            break; 
        }
    }
    close(fd);
    return status;
}
