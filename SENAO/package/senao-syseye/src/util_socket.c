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
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <stdarg.h>
#include <stdio.h>
int create_socket(int domain, void *access_point){
	int sockfd;
	int server_len;
	
	sockfd = socket(domain, SOCK_STREAM, 0);
	if (domain == AF_INET){
		struct sockaddr_in server_address;
		int port = *((int *)access_point);
		server_address.sin_family = domain;
		server_address.sin_addr.s_addr = htonl(INADDR_ANY);
		server_address.sin_port = htons(port);
		server_len = sizeof(server_address);
		bind(sockfd, (struct sockaddr *)&server_address, server_len);
		listen(sockfd, 5);
	}
	else if (domain == AF_UNIX){
		struct sockaddr_un server_address;
		char *path = (char *)access_point;
		unlink(path);
		server_address.sun_family = domain;
		strcpy(server_address.sun_path, path);
		server_len = sizeof(server_address);
		bind(sockfd, (struct sockaddr *)&server_address, server_len);
		listen(sockfd, 5);
	}
	return sockfd;
}

int write_chunked_terminate(int sockfd)
{
   /* hex zeor + CRLF */
    if (sockfd <= 0)
        return -1;
    return write(sockfd, "0\r\n\r\n", 5);
}

ssize_t 
write_chunked(int sockfd, const void *buf, size_t count)
{
   char len[16];
   int wlen = 0;
   if (sockfd <= 0)
       return -1;

   if(buf == NULL) {
		return write_chunked_terminate(sockfd);
   }
	sprintf(len, "%lx\r\n", count);

	wlen += write(sockfd, len, strlen(len));
	wlen += write(sockfd, buf, count);	
	wlen += write(sockfd, "\r\n", 2);	
   return wlen;
}

int 
chunked_printf(int sockfd, const char *format, ...)
{
   char buf[1024], tmp[128];
   va_list ap;
   int wlen=0;
   if (sockfd <=0)
        return -1;
   if(format == NULL) {
		return write_chunked_terminate(sockfd);
   }
   memset(buf, 0, sizeof(buf));
   va_start(ap, format);
   vsprintf(buf, format, ap);
   va_end(ap);

	sprintf(tmp, "%lx\r\n",strlen(buf));

	wlen += write(sockfd, tmp, strlen(tmp));
	wlen += write(sockfd, buf, strlen(buf));	
	wlen += write(sockfd, "\r\n", 4);	
   return wlen;
}
