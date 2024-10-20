#if __cplusplus
extern "C" {
#endif

#ifndef  IP_CFG_H
#define  IP_CFG_H

#include <uaptype.h>

#define IP_CFG_MAX_ALIAS_NUM    10

/**********************************/
/* data sturcture definition      */
/**********************************/
enum IP_STATUS_E
{
    IP_CFG_ENABLED = 1,
    IP_CFG_DISABLED,
};

enum IP_CFG_ERR_CODE_E
{
    IP_CFG_NO_ERROR = 0,
    IP_CFG_IP_ATTACH_FAILED = IP_CFG_ERROR_BASE
};

typedef struct
{
    char	if_name[UAP_DEV_NAME_LEN+1];
    UINT32	status;   
    UINT32  ip;
    UINT32  mask;
    UINT32  alias_ip[IP_CFG_MAX_ALIAS_NUM];
    UINT32  alias_mask[IP_CFG_MAX_ALIAS_NUM];
}
IP_CFG_T;   


typedef struct
{
    INT32       if_num;
    INT32       alias_num;
    IP_CFG_T    *pDefault;
}
IP_CFG_DATA_T;


/***********************************
 Function declaration
************************************/
void IpCfg_Init(IP_CFG_DATA_T *pIpCfgData_Map);
void IpCfg_Terminate();

UINT32 IpCfg_IpStatus		(UINT32 *pStatus,	char *pIfName);
UINT32 IpCfg_SetIpStatus	(UINT32 status,		char *pIfName);
UINT32 IpCfg_IpAddr			(UINT32 *pIpAddr,	char *pIfName);
UINT32 IpCfg_SetIpAddr		(UINT32 ipaddr,		char *pIfName);
UINT32 IpCfg_SubnetMask		(UINT32 *pMask,	    char *pIfName);
UINT32 IpCfg_SetSubnetMask	(UINT32 mask,	    char *pIfName);
UINT32 IpCfg_AliasGetNext   (UINT32 *pAliasIp, UINT32 *pAliasMask, char *pIfName);
UINT32 IpCfg_AddAlias(UINT32 ip, UINT32 mask, char *pIfName);
UINT32 IpCfg_DeleteAlias(UINT32 ip, UINT32 mask, char *pIfName);

//cfho 1209
UINT32  IpCfg_StoreIpAddr(UINT32 ipaddr, char *pIfName);
UINT32	IpCfg_StoreSubnetMask(UINT32 mask, char *pIfName);
#endif

#if __cplusplus
}
#endif
