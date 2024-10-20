#if __cplusplus
extern "C" {
#endif


#ifndef _APPAGENT_CFG_H_
#define _APPAGENT_CFG_H_

#include "uaptype.h"


/**********************************/
/* data sturcture definition      */
/**********************************/
enum HTTP_CFG_STATES_VALUE_E
{
    HTTP_CFG_ENABLED  = 1,
    HTTP_CFG_DISABLED 
};

struct HTTP_CFG_S
{
    UINT32  status;
    UINT16  port[1]; 
} __attribute__((__packed__));

typedef struct HTTP_CFG_S  HTTP_CFG_T;


typedef struct
{
    int num;
    int status;
    UINT16 *ports;
}
HTTP_CFG_DATA_T;


/**********************************/
/*  default value                 */
/**********************************/
#define HTTP_CFG_PORT_DEFAULT   80
#define HTTP_CFG_PORT_TELNET    23

/*---------------------------------------------------------------------*/
/* Function Prototypes                                                 */
/*---------------------------------------------------------------------*/
INT32   HttpCfg_Init        (HTTP_CFG_DATA_T *map);

#endif /* Resrouce API */
#if __cplusplus
}
#endif
