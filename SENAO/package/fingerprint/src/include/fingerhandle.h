#ifndef _FINGER_HANDLE_H_
#define _FINGER_HANDLE_H_

/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/
#include <common.h>
#include "finger.h"
#include "finger_list.h"

/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/

void fingersyncPacketHandleCB(int cli_socket, uint32_t src_ip, uint8_t *reqPkt, int len, unsigned char *type);
void fingerprintsyncUpdateHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt);
void fingerprintsyncUpdateReplyHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt);
void fingerprintsyncRenewHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt);
void fingerprintsyncRenewReplyHandleCB(int socket, uint32_t src_ip, uint8_t *reqPkt);
void fingerprintsyncPacketHandleCB(int cli_socket, uint32_t src_ip, uint8_t *reqPkt, int len, unsigned char *type);

#endif /* _FINGER_HANDLE_H_ */
