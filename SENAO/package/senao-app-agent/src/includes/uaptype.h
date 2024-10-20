#ifndef UAP_TYPE_H
#define UAP_TYPE_H

#include "ostypes.h"
//#include <netmall.h>
//desker+- 020221
//#include <uap_const.h>
#define UAP_DEV_NAME_LEN   16	// =IFNAMSIZ in "if.h"
//desker..

/* For debug purpose */
#define UAP_DEBUG  1
#if UAP_DEBUG

#define UAP_PRINTF     printf

#else

#define UAP_PRINTF

#endif

/************************************/
/* Global type definition           */
/************************************/
#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef YES
#define YES     1
#endif

#ifndef NO
#define NO      0
#endif


/************************************/
/*  Global structure types define   */
/************************************/
#if 0

#ifdef WIN32
#define STRUCT  struct
#define CLASS   class
#else
#define STRUCT	__packed__ struct
#define CLASS   __packed__ class
#endif

#endif


#define ELEMENT_OFF(type, off)  ((int)&(((type)0)->off))


#define STRUCT  struct

typedef struct UAP_OID_S
{
    UINT32	num_components;
    UINT32	*component_list;
}
UAP_OID_T;

typedef struct UAP_OCTET_S
{
    UINT32  len;
    INT8    *pString;
}
UAP_OCTET_T;

typedef struct UAP_ULONG_LIST_S
{
    UINT32  len;
    UINT32	*pUlong;
}
UAP_ULONG_LIST_T;

/************************************************
 * Error code is returned when errors occured.  *
 ************************************************/
enum   UAP_ERROR_CODE_RANGE_E
{
    UAP_ERROR_BASE             = 0,
    USER_CFG_ERROR_BASE         = 0x0100,
    DOWNLOAD_CFG_ERROR_BASE     = 0x0200,
    DHCPC_CFG_ERROR_BASE        = 0x0300,
    IP_CFG_ERROR_BASE           = 0x0400,
    ROUTE_CFG_ERROR_BASE        = 0x0500,
    DNS_RELAY_CFG_ERROR_BASE    = 0x0600,
    IF_CFG_ERROR_BASE           = 0x0700,
    PRD_INFO_ERROR_BASE         = 0x2500
};


enum   UAP_ERROR_CODE_E
{
    UAP_NO_ERROR = 0,
    UAP_UNINITIALIZED,
    UAP_NO_SUCH_NAME,
    UAP_BAD_VALUE,
    UAP_NULL_BUFFER,
    UAP_BUFFER_LEN_ERROR,
    UAP_BAD_IP_ADDR,
    UAP_BAD_SUBNET_MASK,
    UAP_BAD_IP_NET,
    UAP_BAD_IP_HOST_ID,
    UAP_BAD_GATEWAY_IP,
    UAP_BAD_IPX_NET,
    UAP_BAD_IPX_NODE,
    UAP_BAD_MAC_ADDR,
    UAP_BAD_INDEX,
    UAP_BAD_PORT_NO, 
    UAP_NO_SUCH_MAC_ADDR,
    UAP_CONFLICT_ENTRY,
    UAP_END_OF_TABLE,
    UAP_TIMEOUT,
    UAP_RESOURCE_LOCK,
    UAP_NO_RESOURCE,
    UAP_GEN_ERROR,
};

#endif /* UAP_TYPE_H */

