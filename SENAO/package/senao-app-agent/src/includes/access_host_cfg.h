#if __cplusplus
extern "C" {
#endif

#ifndef  _ACCESS_HOST_CFG_H_
#define  _ACCESS_HOST_CFG_H_

#include <uaptype.h>

typedef struct
{
    UINT32  net;
    UINT32  mask;
}
ACCESS_HOST_CFG_T;

typedef struct
{
    INT32   num;
    ACCESS_HOST_CFG_T   *pDefault;
}
ACCESS_HOST_CFG_DATA_T;

/***********************
 Function declaration 
************************/

void    AHCfg_Init(ACCESS_HOST_CFG_DATA_T *pMap);
void    AHCfg_Cleanup();
INT32   AHCfg_Host(UINT32 *pNet, UINT32 *pMask, INT32 index);
INT32   AHCfg_SetHost(UINT32 net, UINT32 mask, INT32 index);
INT32   AHCfg_HostCheck(UINT32 ipaddr);

#endif

#if __cplusplus
}
#endif
