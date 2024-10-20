#ifndef SOCKET_INTERFACE_H
#define SOCKET_INTERFACE_H

/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/

#include <netinet/udp.h>
#include <netinet/ip.h>
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/*                           STRUCTURE                                     */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/

int read_interface(char *interface, int *ifindex, uint32_t *addr);
int listen_socket(uint32_t ip, int port, char *inf);
uint16_t checksum(void *addr, int count);
int udp_packet(uint8_t *payload, int payloadSiz, uint32_t dest_ip, int dest_port);
int udp_packet_enc(uint8_t *payload, int payloadSize, uint32_t dest_ip, int dest_port, unsigned char *type);
/* int raw_packet(uint8_t *payload, int payloadSize, uint32_t source_ip, int source_port, */
/*         uint32_t dest_ip, int dest_port, uint8_t *dest_arp, int ifindex); */

#endif

