#ifndef _GUESTSYNC_WAN_CONF_H_
#define _GUESTSYNC_WAN_CONF_H_

/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/
#include <common.h>
#include <netinet/if_ether.h>
#include "guestsync_list.h"
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/
#define GUESTSYNC_MAJOR_VERSION           9
#define GUESTSYNC_MINOR_VERSION           0

#define GUESTSYNC_MAJOR_MAGIC          0x35
#define GUESTSYNC_MINOR_MAGIC          0x58
#define GUESTSYNC_NEW_MAJOR_MAGIC      0x52
#define GUESTSYNC_NEW_MINOR_MAGIC      0x57

#define GUESTSYNC_UDP_PORT          19840

#define GUESTSYNC_DEFAULT_INTF      "eth0"

#define MAX_GUESTSYNC_DATA_LEN      1024
#define MAX_CONF_ATTR_LEN           64

#define MAC_PATTERN                 "%02X%02X%02X%02X%02X%02X"
#define MAC_PATTERN_HYPHEN          "%02X-%02X-%02X-%02X-%02X-%02X"

#define GUESTSYNC_REQUEST_EXPIRETIME 30 // secs


enum {
    SYNC_TYPE_REQUEST = 1,  // not use
    SYNC_TYPE_REPLY,        // not use
    SYNC_TYPE_ARP,          // not use
    SYNC_TYPE_UPDATE,
    SYNC_TYPE_UPDATE_REQUEST,
    SYNC_TYPE_UPDATE_REPLY,
    SYNC_TYPE_LOGOUT,
    SYNC_TYPE_LOGOUT_REQUEST,
    SYNC_TYPE_LOGOUT_REPLY,
    SYNC_TYPE_GONE
};


enum {
    OPT_IS_LOGIN = 1,
    OPT_SESSION_TIME_REMAINS,
    OPT_IDLE_TIMEOUT
};


/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/

/* Local modules open the port to receive messages from GUESTSYNC, e.g. Rainier
 * Agent or FXO Agent.
 */

#define RANDOM_TIME(x, y)   ((x)+(rand()%((y)-(x))))

/*-------------------------------------------------------------------------*/
/*                        General definition                               */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                        packet struct                                    */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/
char* guestsyncInterface(void);

#endif
