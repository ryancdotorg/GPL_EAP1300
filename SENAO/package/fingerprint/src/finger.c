/*****************************************************************************
;
;   (C) Unpublished Work of Senao Networks, Inc.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF SENAO INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) SENAO EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN SENAO WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF SENAO.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;------------------------------------------------------------------------------
;
;    Project :
;    Creator :
;    File    :
;    Abstract:
;
;       Modification History:
;       By              Date     Ver.   Modification Description
;       --------------- -------- -----  --------------------------------------
;       Yolin          2017-08-01
;*****************************************************************************/

/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <net/if_arp.h>
#include "eloop.h"
#include "sockIntf.h"
#include "finger.h"
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/
#define MAC_BCAST_ADDR  (uint8_t *) "\xff\xff\xff\xff\xff\xff"

/*-------------------------------------------------------------------------*/
/*                           Parameter                                     */
/*-------------------------------------------------------------------------*/
extern char* fingersync_interface;
/*****************************************************************
* NAME:    fingersyncInterface
* ---------------------------------------------------------------
* FUNCTION: FINGER Interface. Now only one interface. (2008-05)
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
char* fingersyncInterface(void)
{
    if(fingersync_interface != NULL)
    {
        return fingersync_interface;
    }
#if !TARGET
    return FINGER_DEFAULT_INTF;
#else
    return BRG_DEV;
#endif
}

/*****************************************************************
* NAME:    fingersyncSendTcpPkt
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  Yolin 2017-08-01
* Modify:
******************************************************************/
void fingersyncSendTcpPkt(int socket, u8 *pkt, int pktSize)
{
    fingersync_hexdump_ascii(FINGER_MSGDUMP, "TX ctrl_iface", (u8 *) pkt, pktSize);

    send(socket, pkt, pktSize, 0);
}
