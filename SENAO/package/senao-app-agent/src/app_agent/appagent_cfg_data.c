#include "inet_sup.h"
#include "appagents.h"
#include "appagent_cfg.h"
#include "appagent_cfg_data.h"
#include "admin_cfg.h"
#if SUPPORT_IPV6_SETTING
#include <linux/in6.h>
#endif
#include <sysUtilMisc.h>

/************************
 Definition
 ************************/
#define APPS_HTTP_LISTEN_PORTS 2
#define SENAO_HTTP_PRIVATE_PORT 8080
#define APP_AGENT_LOGIN_FILE_PATH	"/tmp/app_agent_login_info"

/************************
 Variables
 ************************/
static APP_AUTH_FUNC_T pApp_AuthCheck = 0;

/*******************************
  IP mount table definitions
 *******************************/
UINT16  HttpPorts[APPS_HTTP_LISTEN_PORTS] =
{
    //<< APC_WEB_TABLE
    SENAO_HTTP_PRIVATE_PORT,
    //>> APC_WEB_TABLE
};

HTTP_CFG_DATA_T  HttpCfgData_Map =
{
    //<< APC_WEB_DATA
    APPS_HTTP_LISTEN_PORTS,         // Listen port number
    HTTP_CFG_ENABLED,               // http status
    HttpPorts                       // http default ports
    //>> APC_WEB_DATA
};

/*--------------------------------------------------------------
* ROUTINE NAME - saveLoginArraytoFile
*---------------------------------------------------------------
* FUNCTION:	save login ip array to app_agent_login_info
*
* INPUT:
*
* RETURN:	TRUE or FALSE
* 			
---------------------------------------------------------------*/
int saveLoginArraytoToken()
{
#if 0
	int i;
	UINT8 has_login_ip[64]={0};
	LONG admin_time=0;
	char ip_tok[32]={0}, time_tok[32]={0};

	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
	{
		sprintf(ip_tok,APPAGENT_LOGIN_IP_TOK"%d",(i+1));
		sprintf(time_tok,APPAGENT_LOGIN_TIME_TOK"%d",(i+1));
#if 1//SUPPORT_IPV6_SETTING
		AdminCfg_GetHasLoginIp(has_login_ip ,i);
		AdminCfg_GetMultiLoginTime(&admin_time, i);
#else
		AdminCfg_GetHasLoginIp(&has_login_ip ,i);
		AdminCfg_GetMultiLoginTime(&admin_time, i);
#endif
		apCfgSetValueByStr(ip_tok,has_login_ip);
		apCfgSetLongValue(time_tok,admin_time);

		//DBprintf("%s, has_login_ip[%d]=%s admin_time=%ld admin_timeout=%d\n",__FUNCTION__,i,has_login_ip,admin_time,apCfgGetIntValue(HTTP_REMOTE_IDLETIME_TOK));
	}
#endif
	return TRUE;
}

/*--------------------------------------------------------------
* ROUTINE NAME - Login_AuthRefresh
*---------------------------------------------------------------
* FUNCTION:	After checking username and passwd ok, save session IP
* 			and initial login time to IP array.
*
* INPUT:    sock, pkt
*
* RETURN:   
*   		AGENT_AUTH_REFRESH_FAIL			--	not save.
*   		AGENT_AUTH_REFRESH_SUCCESS		--	save OK.
*   		AGENT_AUTH_REFRESH_TABLE_FULL	--	IP array full.
---------------------------------------------------------------*/
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
int Login_AuthRefresh (int sock, HTTPS_CB *pkt, int appType)
#else
int Login_AuthRefresh (int sock, HTTPS_CB *pkt)
#endif
{
	int i, j, hasLogin = 0, refresh_result=AGENT_AUTH_REFRESH_FAIL;
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
	int app_type, app_count, app_count_max;
#endif
	UINT8 has_login_ip[64]={0}, ipstr[64]={0};

#if !SUPPORT_IPV6_SETTING
	int len=0;
	struct sockaddr_in peer;
#endif

#if SUPPORT_IPV6_SETTING
	sprintf(ipstr, "%s", sysutil_get_peername(sock));
#else
	len = sizeof(struct sockaddr_in);
	getpeername(sock, (struct sockaddr *)&peer, &len);
	sprintf(ipstr, "%s", inet_ntoa(peer.sin_addr));
#endif

	DBprintf("===== [%s - %d] : Show all Login IPs' information before checking\n", __func__, __LINE__);
	AdminCfg_ShowLoginIPInfo();

#if HAS_LIMIT_APP_ACCOUNT_LOGIN
	if(AGENT_APP_NONE < appType)
	{
		AdminCfg_GetHasLoginAppCount(appType, &app_count);

		switch(appType)
		{
			case AGENT_APP_ENMESH:
				app_count_max = NUM_APPAGENT_CONCURENT_LOGIN_APP_ENMESH;
				break;
			default:
				app_count_max = NUM_APPAGENT_CONCURENT_LOGIN;
				break;
		}
	}
#endif

	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
	{
#if 1//SUPPORT_IPV6_SETTING
		AdminCfg_GetHasLoginIp(has_login_ip ,i);
		if(strcmp(has_login_ip, ipstr)==0)
#else
		AdminCfg_GetHasLoginIp(&has_login_ip ,i);
		if(has_login_ip == ntohl(peer.sin_addr.s_addr))
#endif
		{
			AdminCfg_SetMultiLoginTime(KNL_TICKS(), i);
			DBprintf("%s refresh has_login_ip[%d]:%s time\n",__FUNCTION__,i,has_login_ip);
			hasLogin = 1;
			refresh_result = AGENT_AUTH_REFRESH_SUCCESS;

#if HAS_LIMIT_APP_ACCOUNT_LOGIN
			if(AGENT_APP_NONE < appType)
			{
				AdminCfg_GetMultiLoginAppType(&app_type, i);

				if(!(app_type & appType))
				{
					if(app_count >= app_count_max)
					{
						return AGENT_AUTH_REFRESH_TABLE_FULL;
					}

					AdminCfg_AddMultiLoginAppType(appType, i);
				}
			}
#endif
			break;
		}
	}

	/*if find no match ip, we will try to find an empty space and store current login ip*/
	if(hasLogin == 0)
	{
		/*first, we should refresh table status, release already timeout ip to others.*/
		//AdminCfg_refreshAdminIpArray((T_LONG)apCfgGetIntValue(HTTP_REMOTE_IDLETIME_TOK));
		//todo
		AdminCfg_refreshAdminIpArray(APPAGENT_CONNECTION_TIMEOUT_MINUTE);   //hardcode timeout 10 minute.

#if 1//SUPPORT_IPV6_SETTING
		if(AdminCfg_HasValidIndex()== UAP_END_OF_TABLE)
#else
		if(AdminCfg_HasValidIndex(ntohl(peer.sin_addr.s_addr))== UAP_END_OF_TABLE)
#endif
		{
			DBprintf("Login NUM table is full\n");
			refresh_result = AGENT_AUTH_REFRESH_TABLE_FULL;
		}
		else
		{
#if 1//SUPPORT_IPV6_SETTING
			AdminCfg_SetHasLoginIp(ipstr);
			for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
			{
				AdminCfg_GetHasLoginIp(has_login_ip, i);
				if(strcmp(has_login_ip, ipstr)==0)
				{
					AdminCfg_SetMultiLoginTime(KNL_TICKS(), i);
					DBprintf("%s save current ip[%s] and time to table\n",__FUNCTION__,ipstr);

#if HAS_LIMIT_APP_ACCOUNT_LOGIN
					if(AGENT_APP_NONE < appType)
					{
						if(app_count >= app_count_max)
						{
							AdminCfg_ResetMultiAdmin(has_login_ip, i);
							return AGENT_AUTH_REFRESH_TABLE_FULL;
						}

						AdminCfg_AddMultiLoginAppType(appType, i);
					}
#endif
					hasLogin = 1;
					refresh_result = AGENT_AUTH_REFRESH_SUCCESS;
					break;
				}
			}
#else
			AdminCfg_SetHasLoginIp(ntohl(peer.sin_addr.s_addr));
			for(j=0; j < NUM_APPAGENT_CONCURENT_LOGIN; j++)
			{
				AdminCfg_GetHasLoginIp(&has_login_ip ,j);
				if(has_login_ip == ntohl(peer.sin_addr.s_addr))
				{
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
					AdminCfg_AddMultiLoginAppType(appType, j);
#endif
					AdminCfg_SetMultiLoginTime(KNL_TICKS(), j);
					DBprintf("%s save current ip[0x%X] and time to table\n",__FUNCTION__,ntohl(peer.sin_addr.s_addr));
					hasLogin = 1;
					refresh_result = AGENT_AUTH_REFRESH_SUCCESS;
					break;
				}
			}
#endif
		}
	}

	if(refresh_result == AGENT_AUTH_REFRESH_SUCCESS)
	{
		saveLoginArraytoToken();
	}
	
	if(refresh_result != AGENT_AUTH_REFRESH_SUCCESS)
	{
		DBprintf("%s refrsh error!!\n",__FUNCTION__);
	}

	DBprintf("===== [%s - %d] : Show all Login IPs' information after checking\n", __func__, __LINE__);
	AdminCfg_ShowLoginIPInfo();

	return refresh_result;
}

/*--------------------------------------------------------------
 * ROUTINE NAME - Logout_Auth
 *---------------------------------------------------------------
 * FUNCTION:	Clear session IP from the IP array.
 *
 * INPUT:    sock, pkt
 *
 * RETURN:
 *   		AGENT_AUTH_NO_ERROR
 *   		AGENT_AUTH_NO_MATCH_IP
 ---------------------------------------------------------------*/
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
int Logout_Auth (int sock, HTTPS_CB *pkt, int appType)
#else
int Logout_Auth (int sock, HTTPS_CB *pkt)
#endif
{
    int i;
    UINT8 has_login_ip[64], ipstr[64];
#if !SUPPORT_IPV6_SETTING
    int len=0;
    struct sockaddr_in peer;
#endif
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
    int app_type;
#endif

    memset(has_login_ip, 0x00, sizeof(has_login_ip));
    memset(ipstr, 0x00, sizeof(ipstr));

#if SUPPORT_IPV6_SETTING
    sprintf(ipstr, "%s", sysutil_get_peername(sock));
#else
    len = sizeof(struct sockaddr_in);
    getpeername(sock, (struct sockaddr *)&peer, &len);
    sprintf(ipstr, "%s", inet_ntoa(peer.sin_addr));
#endif

    for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
    {
        AdminCfg_GetHasLoginIp(has_login_ip ,i);
        if(strcmp(has_login_ip, ipstr)==0)
        {
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
            AdminCfg_GetMultiLoginAppType(&app_type, i);

            if(app_type & appType)
            {
                AdminCfg_SetMultiLoginAppType((app_type & ~appType), i);
            }

            AdminCfg_GetMultiLoginAppType(&app_type, i);
            if(!app_type)
#endif
            {
                AdminCfg_ResetMultiAdmin(has_login_ip, i);
                DBprintf("%s clear has_login_ip[%s]\n",__func__,has_login_ip);
            }

            return AGENT_AUTH_NO_ERROR;
        }
    }

    return AGENT_AUTH_NO_MATCH_IP;
}

/*--------------------------------------------------------------
 * ROUTINE NAME - Flush_Login_Auth
 *---------------------------------------------------------------
 * FUNCTION:	Flush the session IP array.
 *
 * INPUT:    sock, pkt
 *
 * RETURN:
 *   		AGENT_AUTH_NO_ERROR
 ---------------------------------------------------------------*/
int Flush_Login_Auth (int sock, HTTPS_CB *pkt)
{
    int i;
    UINT8 has_login_ip[64], ipstr[64];

    memset(has_login_ip, 0x00, sizeof(has_login_ip));
    memset(ipstr, 0x00, sizeof(ipstr));

    DBprintf("===== [%s - %d] : Show all Login IPs' information before flushing\n", __func__, __LINE__);
    AdminCfg_ShowLoginIPInfo();
    sprintf(ipstr, "%s", sysutil_get_peername(sock));

    for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
    {
        AdminCfg_GetHasLoginIp(has_login_ip ,i);

        if(0 != strlen(has_login_ip))
        {
            AdminCfg_ResetMultiAdmin(has_login_ip, i);
            DBprintf("%s clear has_login_ip[%s]\n",__func__,has_login_ip);
        }
    }

    DBprintf("===== [%s - %d] : Show all Login IPs' information after flushing\n", __func__, __LINE__);
    AdminCfg_ShowLoginIPInfo();
    return AGENT_AUTH_NO_ERROR;
}

/*--------------------------------------------------------------
 * ROUTINE NAME - Flush_APP_Login_Auth
 *---------------------------------------------------------------
 * FUNCTION: Clear session IP which belongs to the assigned APP
 *           from the IP array.
 *
 * INPUT:    sock, pkt
 *
 * RETURN:
 *           AGENT_AUTH_NO_ERROR
 ---------------------------------------------------------------*/
int Flush_APP_Login_Auth (int sock, HTTPS_CB *pkt, int appType)
{
    int i, matched;
    UINT8 has_login_ip[64];
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
    int app_type;
#endif

    DBprintf("===== [%s - %d] : Show all Login IPs' information before flushing\n", __func__, __LINE__);
    AdminCfg_ShowLoginIPInfo();

    for(i = 0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
    {
        matched = FALSE;
        memset(has_login_ip, 0x00, sizeof(has_login_ip));
        AdminCfg_GetHasLoginIp(has_login_ip ,i);

        if(0 != strlen(has_login_ip))
        {
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
            AdminCfg_GetMultiLoginAppType(&app_type, i);

            if(app_type & appType)
            {
                AdminCfg_SetMultiLoginAppType((app_type & ~appType), i);
                matched = TRUE;
            }

            AdminCfg_GetMultiLoginAppType(&app_type, i);
            if(TRUE == matched && !app_type)
#endif
            {
                AdminCfg_ResetMultiAdmin(has_login_ip, i);
                DBprintf("%s clear has_login_ip[%s]\n",__func__,has_login_ip);
            }
        }
    }

    DBprintf("===== [%s - %d] : Show all Login IPs' information after flushing\n", __func__, __LINE__);
    AdminCfg_ShowLoginIPInfo();

    return AGENT_AUTH_NO_ERROR;
}


/*****************************************************************
* NAME: App_AdminTimeout
* ---------------------------------------------------------------
* FUNCTION:	reset admin_time/admin_ip.
* INPUT:	sock, pkt
* OUTPUT: 
*			TRUE	--	clean OK.
*			FALSE	--	no match IP, not clean.
* Author:	Derek Yu 2013-02-23
* Modify:   
****************************************************************/
#if 1//SUPPORT_IPV6_SETTING
T_BOOL App_AdminTimeout(int sock, HTTPS_CB *pkt)
{
	UINT8 login_ip[64]={0}, ipstr[64]={0};
	int i=0;
	T_BOOL nRet=FALSE;

#if !SUPPORT_IPV6_SETTING
	int len=0;
	struct sockaddr_in peer;
#endif

#if SUPPORT_IPV6_SETTING
	sprintf(ipstr, "%s", sysutil_get_peername(sock));
#else
	len = sizeof(struct sockaddr_in);
	getpeername(sock, (struct sockaddr *)&peer, &len);
	sprintf(ipstr, "%s", inet_ntoa(peer.sin_addr));
#endif

	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
	{
		AdminCfg_GetHasLoginIp(login_ip ,i);

		if(strcmp(ipstr, login_ip)==0)
		{
			AdminCfg_SetMultiAdminTimeout(login_ip);
			nRet=TRUE;
		}
	}

	return nRet;
}
#else
T_BOOL App_AdminTimeout(int sock, HTTPS_CB *pkt)
{
	UINT32 login_ip = 0xffffffff;
	struct sockaddr_in peer;
	int len=0,i=0;

	T_BOOL nRet=FALSE;
	T_CHAR *paction;

#if 0
    if(pkt->json)
    {
        paction = pkt->json_action;
    }
#endif
	
	/* login pkt did not check timeout, but other pkt need to check timeout. */
	//if(strcasecmp(paction,"Login") !=0)
	{
		len = sizeof(struct sockaddr_in);
		getpeername(sock, (struct sockaddr *)&peer, &len);
	
		for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
		{
			AdminCfg_GetHasLoginIp(&login_ip ,i);
	
			if(ntohl(peer.sin_addr.s_addr)==login_ip)
			{
				AdminCfg_SetMultiAdminTimeout(login_ip);
				nRet=TRUE;
			}
		}
	}

	return nRet;
}
#endif

/*****************************************************************
* NAME: App_CheckAdminTimeout
* ---------------------------------------------------------------
* FUNCTION:	Check admin time before doing any request
* INPUT:	sock
* OUTPUT: 
* 			AGENT_AUTH_NO_ERROR		--	session IP exist in IP array and not timeout.
*			AGENT_AUTH_TIMEOUT		--	session IP login timeout.
*           AGENT_AUTH_NO_MATCH_IP	--	session IP doesn't exist in IP array.
* Author:	Derek Yu 2013-02-23
* Modify:   
****************************************************************/
#if 1//SUPPORT_IPV6_SETTING
int App_CheckAdminTimeout(int sock)
{
	LONG kticks=0,admin_time=0, admin_timeout=0;;
	//UINT32  admin_time=0, admin_timeout=0;
	UINT8 has_login_ip[64]={0}, ipstr[64]={0};
	int i=0;

#if !SUPPORT_IPV6_SETTING
	int len=0;
	struct sockaddr_in peer;
#endif

#if SUPPORT_IPV6_SETTING
	sprintf(ipstr, "%s", sysutil_get_peername(sock));
#else
	len = sizeof(struct sockaddr_in);
	getpeername(sock, (struct sockaddr *)&peer, &len);
	sprintf(ipstr, "%s", inet_ntoa(peer.sin_addr));
#endif

	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
    {
		AdminCfg_GetHasLoginIp(has_login_ip ,i);
		
		if(strcmp(ipstr,has_login_ip)==0)
		{
			AdminCfg_GetMultiLoginTime(&admin_time, i);
			break;
		}
	}

	if(admin_time==0)
	{
		return AGENT_AUTH_NO_MATCH_IP;
	}

	kticks=KNL_TICKS();

	//todo
	admin_timeout=APPAGENT_CONNECTION_TIMEOUT_MINUTE;//(apCfgGetIntValue(HTTP_REMOTE_IDLETIME_TOK)==0)?86400:(T_LONG)apCfgGetIntValue(HTTP_REMOTE_IDLETIME_TOK);
	
	DBprintf("%s =====> kticks[%ld]-admin_time[%ld]=idlel[%ld] max_idle[%ld]\n",__FUNCTION__,kticks,admin_time, kticks-admin_time, admin_timeout*60);

	if(kticks - admin_time > admin_timeout * 60)
	{
		DBprintf("%s ---> Timeout\n",__FUNCTION__);
		return AGENT_AUTH_TIMEOUT;
	}

	return AGENT_AUTH_NO_ERROR;
}
#else
int App_CheckAdminTimeout(int sock)
{
	LONG kticks;
	LONG admin_time=0, admin_timeout=0;	
	UINT8 has_login_ip[64] = {0};
	struct sockaddr_in peer;
	int len=0,i=0;

	len = sizeof(struct sockaddr_in);
	getpeername(sock, (struct sockaddr *)&peer, &len);

	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
    {
		AdminCfg_GetHasLoginIp(&has_login_ip ,i);

		if(ntohl(peer.sin_addr.s_addr)==has_login_ip)
		{
			AdminCfg_GetMultiLoginTime(&admin_time, i);
			break;
		}
	}

	if(admin_time==0)
	{
		DBprintf("%s ---> table No Match ip\n",__FUNCTION__);
		return AGENT_AUTH_NO_MATCH_IP;
	}

	kticks=KNL_TICKS();

	//todo
	admin_timeout=APPAGENT_CONNECTION_TIMEOUT_MINUTE;//(apCfgGetIntValue(HTTP_REMOTE_IDLETIME_TOK)==0)?86400:apCfgGetIntValue(HTTP_REMOTE_IDLETIME_TOK);
	
	DBprintf("%s =====> kticks[%d]-admin_time[%d]=idlel[%d] max_idle[%d]\n",__FUNCTION__,kticks,admin_time, kticks-admin_time, admin_timeout*60);

	if(kticks - admin_time > admin_timeout * 60)
	{
		DBprintf("%s ---> Timeout\n",__FUNCTION__);
		return AGENT_AUTH_TIMEOUT;
	}

	return AGENT_AUTH_NO_ERROR;
}
#endif

/*--------------------------------------------------------------
* ROUTINE NAME - AppAuthCheck
*---------------------------------------------------------------
* FUNCTION: This routine returns the authentication result of
*           the login IP and login time
*
* INPUT:    nSock -- socket of this session,
*           pkt   -- packet content of ths session 
*
* RETURN:	AGENT_AUTH_TIMEOUT		--	IP array has aleady existed this session IP and its timeout.
*   		AGENT_AUTH_NO_MATCH_IP	--	Can't find session IP in IP array. It must new comer.
*   		AGENT_AUTH_CHECK_OK		--	IP array has aleady existed this session IP and its not timeout. 
---------------------------------------------------------------*/
static int AppAuthCheck (int sock, HTTPS_CB *pkt)
{
//<< Preserve Begin
	//struct sockaddr_in peer;
	int i,len,auth;
	//char *path;

	//UINT32  admin_time, admin_timeout;
	//UINT32  admin_ip=0;
	//UINT32  remote_ip=0;
	//UINT32  remote_status=0;
	//int kticks;

	//T_IPADDR lanIp,netmask;
	//T_UINT32 reg0, reg1, reg2, reg3;
	//T_BOOL  sameSubnet=0, omit=0;

#if 1//SUPPORT_IPV6_SETTING
	UINT8 login_ip[64]={0};
#else
	//UINT32 login_ip=0xffffffff;
	//len = sizeof(struct sockaddr_in);
	//getpeername(sock, (struct sockaddr *)&peer, &len);
#endif
	//T_BOOL hasLoginFlag = FALSE;
	//ULONG ip;
	char ipstr[64]={0};
	struct sockaddr_storage addr;
	int port;
    //UINT32  remote_port;
	
	len = sizeof(addr);
	getpeername(sock, (struct sockaddr *)&addr, &len);

	if(addr.ss_family == AF_INET)
	{
		struct sockaddr_in *s = (struct sockaddr_in *)&addr;
		port = ntohs(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
	}
#if SUPPORT_IPV6_SETTING
	else
	{
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		port = (s->sin6_port);
		inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
	}
#endif
	DBprintf("peer ip[%s], peer port[%d] peer family[%s]\n",ipstr, port, (addr.ss_family == AF_INET)?"AF_INET":"AF_INET6");

	if((auth=App_CheckAdminTimeout(sock)) == AGENT_AUTH_TIMEOUT)
    {
		/* timeout */
		if(App_AdminTimeout(sock, pkt) == TRUE)
		{
			/*update login array to file*/
			saveLoginArraytoToken();
		}
		else
		{
			DBprintf("-----> Not clean login time!!\n");
		}
		return AGENT_AUTH_TIMEOUT;
    }

	// Refresh timer every time we re-entered
	//AdminCfg_AdminIp(&admin_ip);    
 	//printf("get adminip() 0x%x\n",admin_ip);

	/* Refresh corresponding admin time accroding to peer ip */
	if(auth == AGENT_AUTH_NO_MATCH_IP)
	{
		return AGENT_AUTH_NO_MATCH_IP;
	}

	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
	{
#if	1//SUPPORT_IPV6_SETTING
		AdminCfg_GetHasLoginIp(login_ip ,i);
		if(strcmp(login_ip, ipstr)==0)
#else
		AdminCfg_GetHasLoginIp(&login_ip ,i);
		if(login_ip == ntohl(peer.sin_addr.s_addr))
#endif
		{
			AdminCfg_SetMultiLoginTime(KNL_TICKS(), i);
			break;
		}
	}


	return AGENT_AUTH_CHECK_OK;
}

/*--------------------------------------------------------------
* ROUTINE NAME - App_AuthCheck
*---------------------------------------------------------------
* FUNCTION: This routine returns the authentication result of
*           the login IP and login time
*
* INPUT:    nSock -- socket of this session,
*           pkt   -- packet content of ths session 
*
* RETURN:	AGENT_AUTH_TIMEOUT		--	IP array has aleady existed this session IP and its timeout.
*   		AGENT_AUTH_NO_MATCH_IP	--	Can't find session IP in IP array. It must new comer.
*   		AGENT_AUTH_CHECK_OK		--	IP array has aleady existed this session IP and its not timeout.
---------------------------------------------------------------*/
int App_AuthCheck (int nSock, HTTPS_CB *pkt)
{
	int auth;

	auth = ((*pApp_AuthCheck)(nSock, pkt));
	return auth;
}

/*--------------------------------------------------------------
* ROUTINE NAME - App_AuthHook
*---------------------------------------------------------------
* FUNCTION: This routine hooks the authentication check routines
*
* INPUT:    pAuthCheck -- authentication check routine   
---------------------------------------------------------------*/
void App_AuthHook(APP_AUTH_FUNC_T pAuthCheck)
{
    pApp_AuthCheck = pAuthCheck;
}

//>> FUNCTION END
/*--------------------------------------------------------------
* ROUTINE NAME - AuthCfg_Init
*---------------------------------------------------------------
* FUNCTION: Init the user information to default
*
* INPUT:    None
* OUTPUT:   None
* RETURN:   None
* NOTE:     Don't change the following code, Please change data
*           Settings above
---------------------------------------------------------------*/
void AuthCfg_Init()
{
    //AdminCfg_SetAdminIp(0);
	// Hook authentication check
	App_AuthHook(AppAuthCheck);
	return;
}


/*--------------------------------------------------------------
* ROUTINE NAME - HttpCfgData_Init
*---------------------------------------------------------------
* FUNCTION:
*   Initialize the http configuration variables.  This routine 
*   should only be caled in the 'root' task(application 
*   initialization).  This routine will open the configuration
*   file and copy to the information controll area, then based
*   on the settings, set to the system interface.  If the file
*   opened file, the settings of pDeault will be used and
*   create a new file to write the pDeault as the new settings.
*   
---------------------------------------------------------------*/
void HttpCfgData_Init()
{
    HttpCfg_Init(&HttpCfgData_Map);
    return;
}

