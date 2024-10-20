#ifndef _PROTO_HANDLER_H
#define _PROTO_HANDLER_H

#define PORT_DNS   53
#define PORT_DHCP  SERVER_PORT
#define PORT_HTTP  HTTP_PORT

extern int protocolHandler(struct ip_udp_hdr *, unsigned int);

#endif  /* #ifndef _PROTO_HANDLER_H */
