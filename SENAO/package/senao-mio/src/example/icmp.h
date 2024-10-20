#ifndef _ICMP_H_
#define _ICMP_H_
int create_socket_icmpv4();
void send_icmpv4(int sockfd, unsigned char *ipBytes);
#endif
