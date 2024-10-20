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
#ifndef _UTIL_SOCKET_H_
#define _UTIL_SOCKET_H_
#include <unistd.h>
#include <string.h>
int create_socket(int domain, void *access_point);
int write_chunked_terminate(int sockfd);
ssize_t write_chunked(int sockfd, const void *buf, size_t count);
int chunked_printf(int sockfd, const char *format, ...);
#endif
