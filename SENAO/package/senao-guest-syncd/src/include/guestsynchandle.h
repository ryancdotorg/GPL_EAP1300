#ifndef _GUESTSYNC_HANDLE_H_
#define _GUESTSYNC_HANDLE_H_

/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/
#include <common.h>
#include "guestsync.h"
#include "guestsync_list.h"

/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/

void guestsyncPacketHandleCB(int cli_socket, uint32_t src_ip, uint8_t *reqPkt, int len);

#endif /* _GUESTSYNC_HANDLE_H_ */