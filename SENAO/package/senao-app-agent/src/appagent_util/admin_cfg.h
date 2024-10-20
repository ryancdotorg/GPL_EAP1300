#ifndef __ADMIN_CFG_H__
#include <uaptype.h>

#if 1 //debug
#define DBprintf(x...) cprintf(x);
#else
#define DBprintf(x...) 
#endif

/**********************************/
/*  define                 */
/**********************************/
#define HTTPD_ENABLED	1
#define HTTPD_DISABLED  0
#define NUM_APPAGENT_CONCURENT_LOGIN 10
#define NUM_APPAGENT_CONCURENT_LOGIN_APP_ENMESH 1
#define APPAGENT_CONNECTION_TIMEOUT_MINUTE 10
/**********************************/
/* data sturcture definition      */
/**********************************/
typedef struct
{
	UINT8	admin_ip[64];
	LONG	admin_time;
	INT32	app_type;
}MULTI_LOGIN_CFG_T;

typedef struct
{
    //UINT32  admin_ip;
    //UINT32  admin_time;
    //UINT32  dmz_ip;
	MULTI_LOGIN_CFG_T multi_login[NUM_APPAGENT_CONCURENT_LOGIN];
} Current_ADMIN_CFG_T;

/**********************************/
/*  default value                 */
/**********************************/

/*---------------------------------------------------------------------*/
/* Function Prototypes                                                 */
/*---------------------------------------------------------------------*/
void	AdminCfg_Init();
#if 0
INT32	AdminCfg_AdminTime			(UINT32 *atime);
INT32	AdminCfg_SetAdminTime		(UINT32 atime);
INT32   AdminCfg_AdminIP            (UINT32 *ipaddr);
INT32   AdminCfg_SetAdminIP         (UINT32 ipaddr);
#endif
INT32 	AdminCfg_GetMultiLoginTime	(LONG *time, int arrayintry);
INT32 	AdminCfg_SetMultiLoginTime	(LONG time, int arrayintry);
#if 1//SUPPORT_IPV6_SETTING
INT32 	AdminCfg_GetHasLoginIp		(UINT8 *ip, int arrayintry);
INT32 	AdminCfg_SetHasLoginIp		(UINT8 *ip);
INT32	AdminCfg_SetMultiAdminTimeout(UINT8 *currentIp);
INT32   AdminCfg_HasValidIndex();
#else
INT32 	AdminCfg_GetHasLoginIp		(UINT32 *ip, int arrayintry);
INT32 	AdminCfg_SetHasLoginIp		(UINT32 ip);
INT32	AdminCfg_SetMultiAdminTimeout(UINT32 currentIp);
INT32   AdminCfg_HasValidIndex(UINT32 ip);
#endif
INT32 AdminCfg_ResetMultiAdmin(UINT8 *ip, int index);
INT32 AdminCfg_ClearMultiAdmin();
INT32 AdminCfg_GetHasLoginAppCount(int appType, int *count);
INT32 AdminCfg_GetMultiLoginAppType(int *appType, int index);
INT32 AdminCfg_SetMultiLoginAppType(int appType, int index);
INT32 AdminCfg_AddMultiLoginAppType(int appType, int index);
#endif /* Resrouce API */

