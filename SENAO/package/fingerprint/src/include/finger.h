#ifndef _FINGER_H_
#define _FINGER_H_

/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/
#include <common.h>
#include <netinet/if_ether.h>
#include "finger_list.h"
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/
#define FINGER_MAJOR_VERSION           0
#define FINGER_MINOR_VERSION           1

#define FINGER_MAJOR_MAGIC          0x32
#define FINGER_MINOR_MAGIC          0x89
#define FINGER_NEW_MAJOR_MAGIC      0x28
#define FINGER_NEW_MINOR_MAGIC      0x90

#define FINGER_UDP_PORT          12432

#define FINGER_DEFAULT_INTF      "eth0"

#define MAX_FINGER_DATA_LEN      1024
#define MAX_IP_DATA_LEN          1024
#define MAX_CONF_ATTR_LEN           64

#define MAC_PATTERN                 "%02X%02X%02X%02X%02X%02X"

enum {
    SYNC_TYPE_UPDATE = 1,
    SYNC_TYPE_UPDATE_REPLY,
    SYNC_TYPE_RENEW,
    SYNC_TYPE_RENEW_REPLY
};

/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/

/* Local modules open the port to receive messages from FINGER, e.g. Rainier
 * Agent or FXO Agent.
 */

#define RANDOM_TIME(x, y)   ((x)+(rand()%((y)-(x))))

/*-------------------------------------------------------------------------*/
/*                           STRUCTURE                                     */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/*                        General definition                               */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                        packet struct                                    */
/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------*/
/*                           Function                                      */
/*-------------------------------------------------------------------------*/
char* fingersyncInterface(void);

#endif
