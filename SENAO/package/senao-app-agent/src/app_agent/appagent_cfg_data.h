#if __cplusplus
extern "C" {
#endif

#ifndef _APPAGENT_CFG_DATA_H_
#define _APPAGENT_CFG_DATA_H_

#include "app_agent.h"
#include "envcfg.h"

/**********************************/
/* data sturcture definition      */
/**********************************/
enum AGENT_AUTH_ERROR_CODE_E
{
	AGENT_AUTH_NO_ERROR = 0,
	AGENT_AUTH_TIMEOUT,
	AGENT_AUTH_NO_MATCH_IP,
	AGENT_AUTH_CHECK_OK
};

enum AGENT_AUTH_REFRESH_ERROR_CODE_E
{
	AGENT_AUTH_REFRESH_FAIL = 0,
	AGENT_AUTH_REFRESH_SUCCESS,
	AGENT_AUTH_REFRESH_TABLE_FULL
};

#if HAS_LIMIT_APP_ACCOUNT_LOGIN
enum _AGENT_APP_TYPE_
{
	AGENT_APP_NONE = 1<<0,
	AGENT_APP_ENMESH = 1<<1
} AGENT_APP_TYPE;
#endif

enum _AGENT_LOGIN_STATUS_
{
	AGENT_LOGIN_FAIL = 0,
	AGENT_LOGIN_OK = 1<<0,
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
	AGENT_LOGIN_OK_APP_ENMESH = AGENT_APP_ENMESH
#endif
} LOGIN_STATUS;
/**********************************/
/*  default value                 */
/**********************************/

/*---------------------------------------------------------------------*/
/* Function Prototypes                                                 */
/*---------------------------------------------------------------------*/
typedef int (*APP_AUTH_FUNC_T)(int nSock, HTTPS_CB *pkt);
void	App_AuthHook (APP_AUTH_FUNC_T pAuthCheck);
int		App_AuthCheck   (int nSock, HTTPS_CB *pkt);
int		App_CheckAdminTimeout(int sock);
T_BOOL	App_AdminTimeout(int sock, HTTPS_CB *pkt);
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
int 	Login_AuthRefresh (int sock, HTTPS_CB *pkt, int appType);
int 	Logout_Auth (int sock, HTTPS_CB *pkt, int appType);
#else
int 	Login_AuthRefresh (int sock, HTTPS_CB *pkt);
int 	Logout_Auth (int sock, HTTPS_CB *pkt);
#endif
int 	Flush_Login_Auth (int sock, HTTPS_CB *pkt);
int 	Flush_APP_Login_Auth (int sock, HTTPS_CB *pkt, int appType);
int 	saveLoginArraytoToken();

#endif /* Resrouce API */
#if __cplusplus
}
#endif
