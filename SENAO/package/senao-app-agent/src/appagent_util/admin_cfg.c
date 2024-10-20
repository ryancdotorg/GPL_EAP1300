#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "admin_cfg.h"
#include "ostypes.h"

/**********************************/
/* Data sturcture definition      */
/**********************************/
static KNL_SEM_ID      AdminCfg_Sem;
/*---------------------------------------------------------------------*/
/* Global variables                                                    */
/*---------------------------------------------------------------------*/
static int AdminCfg_SemCreatedOK=0;
static Current_ADMIN_CFG_T *pAdmincfg;
static Current_ADMIN_CFG_T gAdmincfg;

/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_Init         
*---------------------------------------------------------------
* FUNCTION: Init the AdminCfg group parameter
*
* INPUT:    None 
* OUTPUT:   None 
* RETURN:   None                                               
* NOTE:     The routine is called by the root task.  We initial
*           all the config parameter
*---------------------------------------------------------------*/
void  AdminCfg_Init()
{
	//int i=0;
	//******************************
	// Initialize the variables
	//******************************
	if(KNL_SEM_CREATE("admin_cfg", &AdminCfg_Sem,0) != OK)
	{
		AdminCfg_SemCreatedOK=0;
		return;
	}
	else
	{
		AdminCfg_SemCreatedOK=1;
	}

	if(KNL_SEM_LOCK( AdminCfg_Sem) != OK) return;
	//printf("!!!AdminCfg_Sem %d\n",AdminCfg_Sem);

	// Jerry
	// printf("--------- admin.cfg init -------------------\n");

	pAdmincfg=&gAdmincfg;
	/* set it to zero */
#if 0//SUPPORT_IPV6_SETTING
	memset(pAdmincfg->admin_ip, 0, sizeof(pAdmincfg->admin_ip));
#else
	//pAdmincfg->admin_ip=0;
#endif
	//pAdmincfg->admin_time=0;	
	//pAdmincfg->dmz_ip=0;
	memset(pAdmincfg->multi_login, 0, sizeof(pAdmincfg->multi_login));

	KNL_SEM_UNLOCK(AdminCfg_Sem);

	return;
}
#if 0
/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_AdminTime
*---------------------------------------------------------------
* FUNCTION: Gets the user configuration variables.              
* INPUT:    None
* OUTPUT:   None
* RETURN:       
* NOTE:    
---------------------------------------------------------------*/
INT32 AdminCfg_AdminTime(UINT32 *atime)
{
	*atime=pAdmincfg->admin_time;

	return UAP_NO_ERROR;
}
/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_SetAdminTime
*---------------------------------------------------------------
* FUNCTION: Gets the user configuration variables.              
* INPUT:    None
* OUTPUT:   None
* RETURN:       
* NOTE:    
---------------------------------------------------------------*/
INT32 AdminCfg_SetAdminTime(UINT32 atime)
{
	pAdmincfg->admin_time=atime;

	return UAP_NO_ERROR;
}
/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_AdminIp
*---------------------------------------------------------------
* FUNCTION: Gets the user configuration variables.              
* INPUT:    None
* OUTPUT:   None
* RETURN:       
* NOTE:    
---------------------------------------------------------------*/
INT32 AdminCfg_AdminIp(UINT32 *ip)
{
	*ip=pAdmincfg->admin_ip;
	return UAP_NO_ERROR;
}
/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_SetAdminIp
*---------------------------------------------------------------
* FUNCTION: Gets the user configuration variables.              
* INPUT:    None
* OUTPUT:   None
* RETURN:       
---------------------------------------------------------------*/
INT32 AdminCfg_SetAdminIp(UINT32 ip)
{
	pAdmincfg->admin_ip=ip;

	return UAP_NO_ERROR;
}
#endif
/* 2013-02-21 Derek: add for app_agent support multi-login*/
/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_GetHasLoginTime
*---------------------------------------------------------------
* FUNCTION: Gets the adminIpArray login time.              
* INPUT:    UINT32 *time, int arrayintry
* OUTPUT:   UAP_NO_ERROR
* Author:   leonard
* Modify: 	2010-0730
---------------------------------------------------------------*/
#if 1//SUPPORT_IPV6_SETTING
INT32 AdminCfg_GetMultiLoginTime(LONG *time, int arrayintry)
{
	*time=pAdmincfg->multi_login[arrayintry].admin_time;

	return UAP_NO_ERROR;
}
#else
INT32 AdminCfg_GetMultiLoginTime(UINT32 *time, int arrayintry)
{
	*time=pAdmincfg->multi_login[arrayintry].admin_time;

	return UAP_NO_ERROR;
}
#endif
/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_SetHasLoginTime
*---------------------------------------------------------------
* FUNCTION: Sets the adminIpArray login time.              
* INPUT:    UINT32 time, int arrayintry
* OUTPUT:   UAP_NO_ERROR
* Author:   leonard
* Modify: 	2010-0730  
---------------------------------------------------------------*/
#if 1//SUPPORT_IPV6_SETTING
INT32 AdminCfg_SetMultiLoginTime(LONG time, int arrayintry)
{
	pAdmincfg->multi_login[arrayintry].admin_time=time;

	return UAP_NO_ERROR;
}
#else
INT32 AdminCfg_SetMultiLoginTime(UINT32 time, int arrayintry)
{
	pAdmincfg->multi_login[arrayintry].admin_time=time;

	return UAP_NO_ERROR;
}
#endif
/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_GetHasLoginIp
*---------------------------------------------------------------
* FUNCTION: Gets the adminIpArray login ip.              
* INPUT:    UINT32 *ip, int arrayintry
* OUTPUT:   UAP_NO_ERROR
* Author:   leonard
* Modify: 	2010-0730   
---------------------------------------------------------------*/
#if 1//SUPPORT_IPV6_SETTING
INT32 AdminCfg_GetHasLoginIp(UINT8 *ip, int arrayintry)
{
	strcpy(ip, pAdmincfg->multi_login[arrayintry].admin_ip);

	return UAP_NO_ERROR;
}
#else
INT32 AdminCfg_GetHasLoginIp(UINT32 *ip, int arrayintry)
{
	*ip=pAdmincfg->multi_login[arrayintry].admin_ip;

	return UAP_NO_ERROR;
}
#endif
/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_SetHasLoginIp
*---------------------------------------------------------------
* FUNCTION: Sets the adminIpArray login ip.              
* INPUT:    UINT32 ip
* OUTPUT:   UAP_NO_ERROR / UAP_END_OF_TABLE
* Author:   leonard
* Modify: 	2010-0730        
---------------------------------------------------------------*/
#if 1//SUPPORT_IPV6_SETTING
INT32 AdminCfg_SetHasLoginIp(UINT8 *ip)
{
	int i;
	int j=1;
	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
	{
		if(strlen(pAdmincfg->multi_login[i].admin_ip)==0)
		{
			sprintf(pAdmincfg->multi_login[i].admin_ip, "%s", ip);
			return UAP_NO_ERROR;
			break;
		}
		else
		{
			j++;
		}
	}
	if(j==NUM_APPAGENT_CONCURENT_LOGIN)
	{
		printf("Connection number is full\n");
		return UAP_END_OF_TABLE;
	}
}
#else
INT32 AdminCfg_SetHasLoginIp(UINT32 ip)
{
	int i;
	int j=0;
	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
	{
		//printf("%s[%d] adminIpArray[%d]: 0x%x\n",__FUNCTION__,__LINE__, i, pAdmincfg->multi_login[i].admin_ip);
		if(pAdmincfg->multi_login[i].admin_ip==0xffffffff || pAdmincfg->multi_login[i].admin_ip==0)
		{
			pAdmincfg->multi_login[i].admin_ip=ip;
			return UAP_NO_ERROR;
			break;
		}
		else
		{
			j++;
		}
	}
	if(j==NUM_APPAGENT_CONCURENT_LOGIN)
	{
		printf("Connection number is full\n");
		return UAP_END_OF_TABLE;
	}
}
#endif
/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_HasValidIndex
*---------------------------------------------------------------
* FUNCTION:               
* INPUT:    UINT32 ip
* OUTPUT:   UAP_NO_ERROR / UAP_END_OF_TABLE
* Author:   nelson
* Modify: 	2010-0901        
---------------------------------------------------------------*/
#if 1//SUPPORT_IPV6_SETTING
INT32 AdminCfg_HasValidIndex()
{
	int i,j=0;
	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
	{
		//printf("%s[%d] adminIpArray[%d]: 0x%x\n",__FUNCTION__,__LINE__, i, pAdmincfg->adminIpArray[i][0]);
		if(strlen(pAdmincfg->multi_login[i].admin_ip)==0)
		{
			return UAP_NO_ERROR;
			break;
		}
		else
		{
			j++;
		}
	}
	if(j==NUM_APPAGENT_CONCURENT_LOGIN)
	{
		printf("Connection number is full\n");
		return UAP_END_OF_TABLE;
	}
}
#else
INT32 AdminCfg_HasValidIndex(UINT32 ip)
{
	int i,j=0;
	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
	{
		//printf("%s[%d] adminIpArray[%d]: 0x%x\n",__FUNCTION__,__LINE__, i, pAdmincfg->adminIpArray[i][0]);
		if(pAdmincfg->multi_login[i].admin_ip==0xffffffff || pAdmincfg->multi_login[i].admin_ip==0)
		{
			return UAP_NO_ERROR;
			break;
		}
		else
		{
			j++;
		}
	}
	if(j==NUM_APPAGENT_CONCURENT_LOGIN)
	{
		printf("Connection number is full\n");
		return UAP_END_OF_TABLE;
	}
}
#endif
/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_HasValidIp
*---------------------------------------------------------------
* FUNCTION:               
* INPUT:    UINT32 ip
* OUTPUT:   TRUE / FALSE
* Author:   nelson
* Modify: 	2010-0901        
---------------------------------------------------------------*/
INT32 AdminCfg_CheckArrayHasValidIp(UINT8 *ip)
{
	DBprintf("\n%s ArrayIp: %s\n",__FUNCTION__, ip);
	if(strlen(ip) == 0)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_refreshAdminIpArray
*---------------------------------------------------------------
* FUNCTION: Clear the memory of array when timeout.              
* INPUT:    UINT32 admin_timeout
* OUTPUT:   None
* Author:   leonard
* Modify: 	2010-0730       
---------------------------------------------------------------*/
void AdminCfg_refreshAdminIpArray(LONG admin_timeout)
{
	int j;
    LONG kticks=0, admin_time=0;
	UINT8 has_login_ip[64]={0};

    kticks=KNL_TICKS();
    for(j=0; j < NUM_APPAGENT_CONCURENT_LOGIN; j++)
    {
		AdminCfg_GetHasLoginIp(has_login_ip, j);
		if(AdminCfg_CheckArrayHasValidIp(has_login_ip) == TRUE)
		{
			AdminCfg_GetMultiLoginTime(&admin_time, j);
			//if(admin_time > 0)
			{
				DBprintf("\n %s ==> kticks[%ld]-admin_time[%ld]=idlel[%ld] max_idle[%ld]\n",__FUNCTION__,kticks,admin_time, kticks-admin_time, admin_timeout*60);
				if((kticks - admin_time) > admin_timeout * 60 )
				{
					memset(&pAdmincfg->multi_login[j], 0, sizeof(pAdmincfg->multi_login[j]));
					//AdminCfg_SetMultiLoginTime(0, j);
					DBprintf("AdminIpArray memory set 0\n");
				}
			}		
		}
    }
}

/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_SetMultiAdminTimeout
*---------------------------------------------------------------
* FUNCTION: Clear the memory of current ip entry.
* INPUT:    UINT32 currentIp
* OUTPUT:   UAP_UNINITIALIZED / UAP_NO_ERROR
* Author:   Nelson
* Modify: 	2010-0914       
---------------------------------------------------------------*/
#if 1//SUPPORT_IPV6_SETTING
INT32 AdminCfg_SetMultiAdminTimeout(UINT8 *currentIp)
{
	int i, kticks;
	UINT8   has_login_ip[64];
	LONG  admin_time;

	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
	{
		AdminCfg_GetHasLoginIp(has_login_ip ,i);
		if(strcmp(has_login_ip, currentIp)==0)
		{
			memset(&pAdmincfg->multi_login[i], 0, sizeof(pAdmincfg->multi_login[i]));
			return UAP_UNINITIALIZED;
		}
	}
	return UAP_NO_ERROR;
}
#else
INT32 AdminCfg_SetMultiAdminTimeout(UINT32 currentIp)
{
	int i, kticks;
	UINT32 	has_login_ip;
	UINT32  admin_time;

	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
	{
		AdminCfg_GetHasLoginIp(&has_login_ip ,i);
		if(has_login_ip == currentIp)
		{
			memset(&pAdmincfg->multi_login[i], 0xffffffff, sizeof(pAdmincfg->multi_login[i]));
			AdminCfg_SetMultiLoginTime(0, i);
			return UAP_UNINITIALIZED;
		}
	}
	return UAP_NO_ERROR;
}
#endif

/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_ResetMultiAdmin
*---------------------------------------------------------------
* FUNCTION: Clear the memory of current ip entry.
* INPUT:    UINT32 currentIp, int index
* OUTPUT:   UAP_UNINITIALIZED / UAP_NO_ERROR
* Author:   Nelson
* Modify: 	2010-0914
---------------------------------------------------------------*/
INT32 AdminCfg_ResetMultiAdmin(UINT8 *currentIp, int index)
{
    if(0 == strcmp(currentIp, pAdmincfg->multi_login[index].admin_ip))
    {
        memset(&pAdmincfg->multi_login[index], 0x00, sizeof(pAdmincfg->multi_login[index]));
        return UAP_NO_ERROR;
    }

    return UAP_BAD_INDEX;
}

/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_ClearMultiAdmin
*---------------------------------------------------------------
* FUNCTION: Clear the memory of current ip entry.
* INPUT:    UINT32 currentIp, int index
* OUTPUT:   UAP_UNINITIALIZED / UAP_NO_ERROR
* Author:   Nelson
* Modify: 	2010-0914
---------------------------------------------------------------*/
INT32 AdminCfg_ClearMultiAdmin()
{
    int i = 0;

    for(i = 0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
    {
        if(strlen(pAdmincfg->multi_login[i].admin_ip))
        {
            memset(&pAdmincfg->multi_login[i], 0x00, sizeof(pAdmincfg->multi_login[i]));
        }
    }

    return UAP_NO_ERROR;
}

#if HAS_LIMIT_APP_ACCOUNT_LOGIN
/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_GetHasLoginAppCount
*---------------------------------------------------------------
* FUNCTION: Gets the number of login APP
* INPUT:    int appType, int *count
* OUTPUT:   UAP_NO_ERROR
* Author:   leonard
* Modify:   2010-0730
---------------------------------------------------------------*/
INT32 AdminCfg_GetHasLoginAppCount(int appType, int *count)
{
	int i;

	*count = 0;

	/* Reset the TIMEOUT connections first. */
	AdminCfg_refreshAdminIpArray(APPAGENT_CONNECTION_TIMEOUT_MINUTE);

	for(i=0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
	{
		if(0 != strlen(pAdmincfg->multi_login[i].admin_ip))
		{
			if(appType & pAdmincfg->multi_login[i].app_type)
			{
				*count += 1;
			}
		}
	}

	return UAP_NO_ERROR;
}

/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_GetMultiLoginAppType
*---------------------------------------------------------------
* FUNCTION: Gets the APP type
* INPUT:    int *appType, int index
* OUTPUT:   UAP_NO_ERROR
* Author:   leonard
* Modify:   2010-0730
---------------------------------------------------------------*/
INT32 AdminCfg_GetMultiLoginAppType(int *appType, int index)
{
	if(0 != strlen(pAdmincfg->multi_login[index].admin_ip))
	{
		*appType = pAdmincfg->multi_login[index].app_type;
	}

	return UAP_NO_ERROR;
}

/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_SetMultiLoginAppType
*---------------------------------------------------------------
* FUNCTION: Sets the APP type
* INPUT:    int appType, int index
* OUTPUT:   UAP_NO_ERROR
* Author:   leonard
* Modify:   2010-0730
---------------------------------------------------------------*/
INT32 AdminCfg_SetMultiLoginAppType(int appType, int index)
{
	if(0 != strlen(pAdmincfg->multi_login[index].admin_ip))
	{
		pAdmincfg->multi_login[index].app_type = appType;
	}

	return UAP_NO_ERROR;
}

/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_AddMultiLoginAppType
*---------------------------------------------------------------
* FUNCTION: Adds the APP type
* INPUT:    int appType, int index
* OUTPUT:   UAP_NO_ERROR
* Author:   leonard
* Modify:   2010-0730
---------------------------------------------------------------*/
INT32 AdminCfg_AddMultiLoginAppType(int appType, int index)
{
	if(0 != strlen(pAdmincfg->multi_login[index].admin_ip))
	{
		pAdmincfg->multi_login[index].app_type |= appType;
	}

	return UAP_NO_ERROR;
}
#endif

/*--------------------------------------------------------------
* ROUTINE NAME - AdminCfg_ShowLoginIPInfo
*---------------------------------------------------------------
* FUNCTION: Gets the number of login APP
* INPUT:    int appType, int *count
* OUTPUT:   UAP_NO_ERROR
* Author:   leonard
* Modify:   2010-0730
---------------------------------------------------------------*/
INT32 AdminCfg_ShowLoginIPInfo()
{
	int i;
	LONG kticks = 0, admin_time = 0;

	kticks = KNL_TICKS();

	for(i = 0; i < NUM_APPAGENT_CONCURENT_LOGIN; i++)
	{
		AdminCfg_GetMultiLoginTime(&admin_time, i);

		DBprintf("===== [%s - %d] : ip [%15s] app_type [%08x] idle time [%ld]\n",
				__func__, __LINE__,
				pAdmincfg->multi_login[i].admin_ip,
				pAdmincfg->multi_login[i].app_type,
				(admin_time)?(kticks - admin_time):0);
	}

	return UAP_NO_ERROR;
}
/******************************************************************
<<END>>
******************************************************************/
