#include "WTPBoardApiPortal.h"
#include "WTPBoardApiCommon.h"
#include <ctype.h>
#include <sysWlan.h>
#include <wireless_tokens.h>
#include <variable/variable.h>
#include <variable/api_wireless.h>
#include <gconfig.h>

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

CWBool CWWTPBoardSetPortalEnableCfg(int radioIdx, int enable)
{
    CWDebugLog("%s %d", __FUNCTION__, enable);

    if (api_set_bool_option(WIRELESS_WIFI_PORTALENABLE_OPTION, enable))
    {
        CWDebugLog("Set configuration fail");
        return CW_TRUE;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalEnableCfg(int radioIdx, int *enable)
{  
 
    if (api_get_bool_option(WIRELESS_WIFI_PORTALENABLE_OPTION, enable))
    {
        *enable = 0;
    }

    CWDebugLog("%s %d", __FUNCTION__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalLoginTypeCfg(int radioIdx, int type)
{
    CWDebugLog("%s %d", __FUNCTION__, type);

    if (api_set_integer_option(WIRELESS_WIFI_PORTALLOGINTYPE_OPTION, type))
    {
        CWDebugLog("Set configuration fail");
        return CW_TRUE;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalLoginTypeCfg(int radioIdx, int *type)
{
    if (api_get_integer_option(WIRELESS_WIFI_PORTALLOGINTYPE_OPTION, type))
    {
        *type = 0;
    }

    CWDebugLog("%s %d", __FUNCTION__, *type);

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalRadiusCfg(int radioIdx, unsigned int *addr)
{
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, SERVER_IPADDR_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(WIRELESS_WIFI_PORTALRADIUSSERVER_OPTION, val, SERVER_IPADDR_LENGTH))
    {
        *addr = 0;
    }
    else
    {
        if(val[0])
        {
            *addr = inet_addr(val);
        }
        else
        {
            *addr = 0;
        }
    }

    CW_FREE_OBJECT(val);

    CWDebugLog("%s %u", __FUNCTION__, *addr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalRadiusCfg(int radioIdx, unsigned int addr)
{
    char *val = NULL;

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

    CW_CREATE_STRING_ERR(val, SERVER_IPADDR_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (addr)
        snprintf(val, SERVER_IPADDR_LENGTH, "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
    else
        snprintf(val, SERVER_IPADDR_LENGTH, "0.0.0.0");

    if (api_set_string_option(WIRELESS_WIFI_PORTALRADIUSSERVER_OPTION, val, SERVER_IPADDR_LENGTH))
    {
        CW_FREE_OBJECT(val);
        CWDebugLog("Set configuration fail");
        return CW_TRUE;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalRedirectCfg(int radioIdx, char **pstr)
{
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, URL_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(WIRELESS_WIFI_PORTALUSERURL_OPTION, val, URL_LENGTH))
    {
        snprintf(val, URL_LENGTH, "%s", "");
    }

    *pstr = val;

    CWDebugLog("%s %s", __FUNCTION__, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalRedirectCfg(int radioIdx,  char *pstr)
{
    char *val = NULL;

    CWDebugLog("%s %s", __FUNCTION__, pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_set_string_option(WIRELESS_WIFI_PORTALUSERURL_OPTION, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalWalledGardenCfg(int radioIdx, int enable)
{
    CWDebugLog("%s %d", __FUNCTION__, enable);

    if (api_set_bool_option(WIRELESS_WIFI_PORTALWALLEDGARDENENABLE_OPTION, enable))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalWalledGardenCfg(int radioIdx, int *enable)
{
    if (api_get_bool_option(WIRELESS_WIFI_PORTALWALLEDGARDENENABLE_OPTION, enable))
    {
        *enable = 0;
    }

    CWDebugLog("%s %d", __FUNCTION__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalWalledGardenPageCfg(int radioIdx,  char **pstr)
{
    char *val = NULL;

    CWDebugLog("%s ", __FUNCTION__);

    CW_CREATE_STRING_ERR(val, URL_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(WIRELESS_WIFI_PORTALWALLEDGARDENPAGES_OPTION, val, URL_LENGTH))
    {
        snprintf(val, URL_LENGTH, "%s", "");
    }

    *pstr = val;

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalWalledGardenPageCfg(int radioIdx, char *pstr)
{
    char *val = NULL;

    CWDebugLog("%s %s", __FUNCTION__, pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_set_string_option(WIRELESS_WIFI_PORTALWALLEDGARDENPAGES_OPTION, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalRadiusPortCfg(int radioIdx, unsigned short *port)
{
    int val = 0;

    if (api_get_integer_option(WIRELESS_WIFI_PORTALRADIUSPORT_OPTION, &val))
    {
    	val = 1812;
    }

    *port = (unsigned short) val;

    CWDebugLog("%s %u", __FUNCTION__, *port);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalRadiusPortCfg(int radioIdx, unsigned short port)
{
    int val = 0;

    CWDebugLog("%s %u", __FUNCTION__, port);

    val = (int) port;

    if (api_set_integer_option(WIRELESS_WIFI_PORTALRADIUSPORT_OPTION, val))
    {
        CWDebugLog("Set configuration fail");;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalRadiusSecretCfg(int radioIdx, char **pstr)
{
    char *val = NULL;

    CWDebugLog("%s ", __FUNCTION__ );

    CW_CREATE_STRING_ERR(val, RADIUS_SECRET_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(WIRELESS_WIFI_PORTALRADIUSSECRET_OPTION, val, RADIUS_SECRET_LENGTH))
    {
        snprintf(val, sizeof(URL_LENGTH), "%s", "testing123");
    }

    *pstr = val;

    CWDebugLog("%s %s", __FUNCTION__, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalRadiusSecretCfg(int radioIdx, char *pstr)
{
    char *val = NULL;

    CWDebugLog("%s %s", __FUNCTION__, pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_set_string_option(WIRELESS_WIFI_PORTALRADIUSSECRET_OPTION, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalSessionTimeoutCfg(int radioIdx, int time)
{
    int val = time * 60;

    CWDebugLog("%s %d", __FUNCTION__, time);

    if (api_set_integer_option(WIRELESS_WIFI_PORTALSESSIONTIMEOUT_OPTION, val))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalSessionTimeoutCfg(int radioIdx,  int *time)
{
    int val = 0;

    CWDebugLog("%s ", __FUNCTION__);

    if (api_get_integer_option(WIRELESS_WIFI_PORTALSESSIONTIMEOUT_OPTION, &val))
    {
        val = 60;
    }
    *time = (val)?(val/60):0;

    CWDebugLog("%s %d", __FUNCTION__, *time);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalSessionTimeoutEnableCfg(int radioIdx, int enable)
{
    CWDebugLog("%s %d", __FUNCTION__, enable);

    if (api_set_bool_option(WIRELESS_WIFI_PORTALSESSIONTIMEOUTENABLE_OPTION, enable))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalSessionTimeoutEnableCfg(int radioIdx,  int *enable)
{
    if (api_get_bool_option(WIRELESS_WIFI_PORTALSESSIONTIMEOUTENABLE_OPTION, enable))
    {
        *enable = 0;
    }

    CWDebugLog("%s %d", __FUNCTION__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalIdleTimeoutCfg(int radioIdx, int time)
{
    CWDebugLog("%s %d", __FUNCTION__, time);

    int val = time * 60;

    if (api_set_integer_option(WIRELESS_WIFI_PORTALIDLETIMEOUT_OPTION, val))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalIdleTimeoutCfg(int radioIdx, int *time)
{
    int val = 0;

    if (api_get_integer_option(WIRELESS_WIFI_PORTALIDLETIMEOUT_OPTION, &val))
    {
        val = 0;
    }

    *time = (val)?(val/60):0;

    CWDebugLog("%s %d", __FUNCTION__, *time);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalIdleTimeoutEnableCfg(int radioIdx, int enable)
{
    CWDebugLog("%s %d", __FUNCTION__, enable);
    if (api_set_bool_option(WIRELESS_WIFI_PORTALIDLETIMEOUTENABLE_OPTION, enable))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}
CWBool CWWTPBoardGetPortalIdleTimeoutEnableCfg(int radioIdx,  int *enable)
{
    if (api_get_bool_option(WIRELESS_WIFI_PORTALIDLETIMEOUTENABLE_OPTION, enable))
    {
        *enable = 0;
    }

    CWDebugLog("%s %d", __FUNCTION__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalAccountingEnableCfg(int radioIdx,  int enable)
{
    CWDebugLog("%s %d", __FUNCTION__, enable);
    if (api_set_bool_option(WIRELESS_WIFI_PORTALACCOUNTINGENABLE_OPTION, enable))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalAccountingEnableCfg(int radioIdx, int *enable)
{
    if (api_get_bool_option(WIRELESS_WIFI_PORTALACCOUNTINGENABLE_OPTION, enable))
    {
        *enable = 0;
    }

    CWDebugLog("%s %d", __FUNCTION__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalAccountingIntervalCfg(int radioIdx,  int time)
{
    CWDebugLog("%s %d", __FUNCTION__, time);
    if (api_set_integer_option(WIRELESS_WIFI_PORTALACCOUNTINGINTERVAL_OPTION, time))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalAccountingIntervalCfg(int radioIdx, int *time)
{
    if (api_get_integer_option(WIRELESS_WIFI_PORTALACCOUNTINGINTERVAL_OPTION, time))
    {
        *time = 0;
    }

    CWDebugLog("%s %d", __FUNCTION__, *time);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalAuthTypeCfg(int radioIdx, int type)
{
    CWDebugLog("%s %d", __FUNCTION__, type);

    if (api_set_integer_option(WIRELESS_WIFI_PORTALAUTHTYPE_OPTION, type))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalAuthTypeCfg(int radioIdx,  int *type)
{
    if (api_get_integer_option(WIRELESS_WIFI_PORTALAUTHTYPE_OPTION, type))
    {
        *type = 0;
    }

    CWDebugLog("%s %d", __FUNCTION__, *type);

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalExternalServerCfg(int radioIdx,  char **pstr)
{
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, SERVER_IPADDR_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(WIRELESS_WIFI_PORTALEXTERNALSERVER_OPTION, val, SERVER_IPADDR_LENGTH))
    {
        snprintf(val, SERVER_IPADDR_LENGTH, "%s", "");
    }

    *pstr = val;

    CWDebugLog("%s %s", __FUNCTION__, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalExternalServerCfg(int radioIdx,  char *pstr)
{
    char *val = NULL;

    CWDebugLog("%s %s", __FUNCTION__, pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_set_string_option(WIRELESS_WIFI_PORTALEXTERNALSERVER_OPTION, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalExternalSecretCfg(int radioIdx, char **pstr)
{
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, RADIUS_SECRET_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(WIRELESS_WIFI_PORTALEXTERNALSECRET_OPTION, val, RADIUS_SECRET_LENGTH))
    {
        snprintf(val, sizeof(RADIUS_SECRET_LENGTH), "%s", "testing123");
    }

    *pstr = val;

    CWDebugLog("%s %s", __FUNCTION__, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalExternalSecretCfg(int radioIdx, char *pstr)
{
    char *val = NULL;

    CWDebugLog("%s %s", __FUNCTION__, pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_set_string_option(WIRELESS_WIFI_PORTALEXTERNALSECRET_OPTION, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalAccountingServerCfg(int radioIdx, unsigned int *addr)
{
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, SERVER_IPADDR_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(WIRELESS_WIFI_PORTALACCOUNTINGSERVER_OPTION, val, SERVER_IPADDR_LENGTH))
    {
        *addr = 0;
    }
    else
    {
        if(val[0])
        {
            *addr = inet_addr(val);
        }
        else
        {
            *addr = 0;
        }
    }

    CW_FREE_OBJECT(val);

    CWDebugLog("%s %lu", __FUNCTION__, *addr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalAccountingServerCfg(int radioIdx, unsigned int addr)
{
    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

    char *val = NULL;

    CW_CREATE_STRING_ERR(val, SERVER_IPADDR_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (addr)
        snprintf(val, SERVER_IPADDR_LENGTH, "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
    else
        snprintf(val, SERVER_IPADDR_LENGTH, "0.0.0.0");

    if (api_set_string_option(WIRELESS_WIFI_PORTALACCOUNTINGSERVER_OPTION, val, SERVER_IPADDR_LENGTH))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalAccountingPortCfg(int radioIdx, unsigned short *port)
{
    int val = 0;

    if (api_get_integer_option(WIRELESS_WIFI_PORTALACCOUNTINGPORT_OPTION, &val))
    {
        val = 1813;
    }

    *port = (unsigned short) val;

    CWDebugLog("%s %u", __FUNCTION__, *port);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalAccountingPortCfg(int radioIdx,  unsigned short port)
{
    CWDebugLog("%s %u", __FUNCTION__, port);

    int val = 0;

    val = (int) port;

    if (api_set_integer_option(WIRELESS_WIFI_PORTALACCOUNTINGPORT_OPTION, val))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalAccountingSecretCfg(int radioIdx, char **pstr)
{
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, RADIUS_SECRET_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(WIRELESS_WIFI_PORTALACCOUNTINGSECRET_OPTION, val, RADIUS_SECRET_LENGTH))
    {
        snprintf(val, sizeof(RADIUS_SECRET_LENGTH), "%s", "testing123");
    }
    *pstr = val;

    CWDebugLog("%s  %s", __FUNCTION__, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalAccountingSecretCfg(int radioIdx, char *pstr)
{
    char *val = NULL;

    CWDebugLog("%s %s", __FUNCTION__, pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_set_string_option(WIRELESS_WIFI_PORTALACCOUNTINGSECRET_OPTION, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}
CWBool CWWTPBoardGetPortalUamformatCfg(int radioIdx,  char **pstr)
{
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, URL_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(WIRELESS_WIFI_PORTALUAMFORMAT_OPTION, val, URL_LENGTH))
    {
        snprintf(val, sizeof(URL_LENGTH), "%s", "sn.captivePortal.login");
    }

    *pstr = val;

    CWDebugLog("%s %s", __FUNCTION__, *pstr);
    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalUamformatCfg(int radioIdx, char *pstr)
{
    char *val = NULL;

    CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_set_string_option(WIRELESS_WIFI_PORTALUAMFORMAT_OPTION, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalLocalAuthCfg(int radioIdx, char **pstr)
{
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, AUTH_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(WIRELESS_WIFI_PORTALLOCALAUTH_OPTION, val, AUTH_LENGTH))
    {
        snprintf(val, sizeof(URL_LENGTH), "%s", "sn.captivePortal.auth");
    }

    *pstr = val;

    CWDebugLog("%s %s", __FUNCTION__, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalLocalAuthCfg(int radioIdx,  char *pstr)
{
    char *val = NULL;

    CWDebugLog("%s %s", __FUNCTION__, pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_set_string_option(WIRELESS_WIFI_PORTALLOCALAUTH_OPTION, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalPortCfg(int radioIdx,  unsigned short *port)
{
    int val = 0;

    if (api_get_integer_option(WIRELESS_WIFI_PORTALPORT_OPTION, &val))
    {
        val = 0;
    }

    *port = (unsigned short) val;

    CWDebugLog("%s %u", __FUNCTION__, *port);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalPortCfg(int radioIdx, unsigned short port)
{
    CWDebugLog("%s %u", __FUNCTION__, port);

    int val = 0;

    val = (int) port;

    if (api_set_integer_option(WIRELESS_WIFI_PORTALPORT_OPTION, val))
    { 
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalHttpsEnableCfg(int radioIdx, int *enable)
{
    if (api_get_bool_option(WIRELESS_WIFI_PORTALHTTPSENABLE_OPTION, enable))
    {
        *enable = 0;
    }

    CWDebugLog("%s %d", __FUNCTION__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalHttpsEnableCfg(int radioIdx, int enable)
{
    CWDebugLog("%s %d", __FUNCTION__, enable);
    if (api_set_bool_option(WIRELESS_WIFI_PORTALHTTPSENABLE_OPTION, enable))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalRadiusSecret2Cfg(int radioIdx,  char **pstr)
{
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, RADIUS_SECRET_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(WIRELESS_WIFI_PORTALRADIUSSECRET2_OPTION, val, RADIUS_SECRET_LENGTH))
    {
        snprintf(val, RADIUS_SECRET_LENGTH, "%s", "");
    }

    *pstr = val;

    CWDebugLog("%s %s", __FUNCTION__, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalRadiusSecret2Cfg(int radioIdx, char *pstr)
{
    char *val = NULL;

    CWDebugLog("%s %s", __FUNCTION__, pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    if (api_set_string_option(WIRELESS_WIFI_PORTALRADIUSSECRET2_OPTION, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetPortalRadius2Cfg(int radioIdx,  unsigned int *addr)
{
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, SERVER_IPADDR_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    if (api_get_string_option(WIRELESS_WIFI_PORTALRADIUSSERVER2_OPTION, val, SERVER_IPADDR_LENGTH))
    {
        *addr = 0;
    }
    else
    {
        if(val[0])
        {
            *addr = inet_addr(val);
        }
        else
        {
            *addr = 0;
        }
    }

    CW_FREE_OBJECT(val);

    CWDebugLog("%s %d", __FUNCTION__, *addr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetPortalRadius2Cfg(int radioIdx,  unsigned int addr)
{
    char *val = NULL;

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

    CW_CREATE_STRING_ERR(val, SERVER_IPADDR_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (addr)
        snprintf(val, SERVER_IPADDR_LENGTH, "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
    else
        snprintf(val, SERVER_IPADDR_LENGTH, "0.0.0.0");

    if (api_set_string_option(WIRELESS_WIFI_PORTALRADIUSSERVER2_OPTION, val, SERVER_IPADDR_LENGTH))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

#if SUPPORT_COMBINED_SSID_SETTING
/*  *******************___BASE ON SSID___*******************  */
CWBool CWWTPBoardGetWlanPortalEnableCfg(int radioIdx, int wlanIdx , int *enable)
{
    char option[64]={0};

    snprintf(option, sizeof(option),"portal.ssid_%d.enable",wlanIdx+1);
    
    if (api_get_bool_option(option, enable))
        *enable = CW_FALSE;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalEnableCfg(int radioIdx, int wlanIdx , int enable)
{
    char option[64]={0};

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    snprintf(option,sizeof(option),"portal.ssid_%d.enable",wlanIdx+1);

    if (api_set_bool_option(option, enable))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalLoginTypeCfg(int radioIdx, int wlanIdx , int *type)
{
    char option[64]={0};

    snprintf(option,sizeof(option),"portal.ssid_%d.loginType",wlanIdx+1);

    if (api_get_integer_option(option, type))
        *type = 0;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *type);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalLoginTypeCfg(int radioIdx, int wlanIdx , int type)
{
    char option[64]={0};
    
    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, type);

    snprintf(option,sizeof(option),"portal.ssid_%d.loginType",wlanIdx+1);
    
    if (api_set_integer_option(option, type))
    {
        CWDebugLog("Set configuration fail");
    }
    
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalRadiusCfg(int radioIdx, int wlanIdx , unsigned int *addr)
{
    char option[64]={0};
    char val[15+1]={0};

    snprintf(option,sizeof(option),"portal.ssid_%d.radiusServer",wlanIdx+1);

    api_get_wifi_iface_auth_server_option(option, val, sizeof(val));

    if(val[0])
        *addr = inet_addr(val);
    else
        *addr = 0;

    CWDebugLog("%s %d %d %d %u", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *addr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalRadiusCfg(int radioIdx, int wlanIdx , unsigned int addr)
{
    char option[64]={0};
    char val[15+1]={0};

    CWDebugLog("%s %d %d %d %u.%u.%u.%u", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, CW_IPV4_PRINT_LIST(addr));

    snprintf(option,sizeof(option),"portal.ssid_%d.radiusServer",wlanIdx+1);

    if(addr)
    {
        snprintf(val, sizeof(val), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
        if(api_set_wifi_iface_auth_server_option(option, val, sizeof(val)))
        {
            CWDebugLog("Set configuration fail");
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalRedirectCfg(int radioIdx, int wlanIdx , char **pstr)
{
    char option[64]={0};
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, URL_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    snprintf(option,sizeof(option),"portal.ssid_%d.userurl",wlanIdx+1);

    if (api_get_string_option(option, val, URL_LENGTH))
        snprintf(val, URL_LENGTH, "%s", "");

    *pstr = val;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalRedirectCfg(int radioIdx, int wlanIdx , char *pstr)
{
    char option[64]={0};
    char *val = NULL;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

    if (pstr == NULL || (!strcmp(pstr,"")))
        return CW_TRUE;

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (val == NULL)
        snprintf(val, URL_LENGTH, "%s", "");

    snprintf(option,sizeof(option),"portal.ssid_%d.userurl",wlanIdx+1);

    if (api_set_string_option(option, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalWalledGardenCfg(int radioIdx, int wlanIdx , int *enable)
{
    char option[64]={0};

    snprintf(option,sizeof(option),"portal.ssid_%d.walledGardenEnable",wlanIdx+1);

    if (api_get_bool_option(option, enable))
        *enable = 0;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalWalledGardenCfg(int radioIdx, int wlanIdx , int enable)
{
    char option[64]={0};

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    snprintf(option,sizeof(option),"portal.ssid_%d.walledGardenEnable",wlanIdx+1);

    if (api_set_bool_option(option, enable))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalWalledGardenPageCfg(int radioIdx, int wlanIdx , char **pstr)
{
    char option[64]={0};
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, URL_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    snprintf(option,sizeof(option),"portal.ssid_%d.walledGardenPages",wlanIdx+1);

    if (api_get_string_option(option, val, URL_LENGTH))
        snprintf(val, URL_LENGTH, "%s", "");

    *pstr = val;

    if(val)
        CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalWalledGardenPageCfg(int radioIdx, int wlanIdx , char *pstr)
{
    char option[64]={0};
    char *val = NULL;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

    if (pstr == NULL || (!strcmp(pstr,"")))
        return CW_TRUE;

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (val == NULL)
        snprintf(val, URL_LENGTH, "%s", "");

    snprintf(option,sizeof(option),"portal.ssid_%d.walledGardenPages",wlanIdx+1);

    if (api_set_string_option(option, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalRadiusPortCfg(int radioIdx, int wlanIdx , unsigned short *port)
{
    int val = 0;
    char option[64]={0};

    snprintf(option,sizeof(option),"portal.ssid_%d.radiusPort",wlanIdx+1);

    if (api_get_integer_option(option, &val))
    	val = 1812;

    *port = (unsigned short) val;

    CWDebugLog("%s %d %d %d %u", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *port);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalRadiusPortCfg(int radioIdx, int wlanIdx , unsigned short port)
{
    int val = 0;
    char option[64]={0};

    CWDebugLog("%s %d %d %d %u", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, port);

    snprintf(option,sizeof(option),"portal.ssid_%d.radiusPort",wlanIdx+1);

    val = (int) port;

    if (api_set_integer_option(option, val))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalRadiusSecretCfg(int radioIdx, int wlanIdx , char **pstr)
{
    char *val = NULL;
    char option[64]={0}; 

    CW_CREATE_STRING_ERR(val, RADIUS_SECRET_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    snprintf(option,sizeof(option),"portal.ssid_%d.radiusSecret",wlanIdx+1);

    if (api_get_string_option(option, val, RADIUS_SECRET_LENGTH))
        snprintf(val, RADIUS_SECRET_LENGTH, "%s", "testing123");

    *pstr = val;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalRadiusSecretCfg(int radioIdx, int wlanIdx , char *pstr)
{
    char *val = NULL;
    char option[64]={0};

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

    if (pstr == NULL || (!strcmp(pstr,"")))
        return CW_TRUE;

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (val == NULL)
        snprintf(val, RADIUS_SECRET_LENGTH, "%s", "");

    snprintf(option,sizeof(option),"portal.ssid_%d.radiusSecret",wlanIdx+1);

    if (api_set_string_option(option, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalSessionTimeoutCfg(int radioIdx, int wlanIdx , int time)
{
    int val = time * 60;
    char option[64]={0};

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, time);

    snprintf(option,sizeof(option),"portal.ssid_%d.sessionTimeout",wlanIdx+1);

    if (api_set_integer_option(option, val))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalSessionTimeoutCfg(int radioIdx, int wlanIdx , int *time)
{
    int val = 0;
    char option[64]={0};

    snprintf(option,sizeof(option),"portal.ssid_%d.sessionTimeout",wlanIdx+1);

    if (api_get_integer_option(option, &val))
        val = 60;

    *time = (val)?(val/60):0;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *time);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalSessionTimeoutEnableCfg(int radioIdx, int wlanIdx , int *enable)
{
    char option[64]={0};

    snprintf(option,sizeof(option),"portal.ssid_%d.sessionTimeoutEnable",wlanIdx+1);
    
    if (api_get_bool_option(option, enable))
        *enable = 0;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalSessionTimeoutEnableCfg(int radioIdx, int wlanIdx , int enable)
{
    char option[64]={0};

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    snprintf(option,sizeof(option),"portal.ssid_%d.sessionTimeoutEnable",wlanIdx+1);

    if (api_set_bool_option(option, enable))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalIdleTimeoutCfg(int radioIdx, int wlanIdx , int *time)
{
    char option[64]={0};
    int val = 0;

    snprintf(option,sizeof(option),"portal.ssid_%d.idleTimeout",wlanIdx+1);

    if (api_get_integer_option(option, &val))
        val = 0;

    *time = (val)?(val/60):0;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *time);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalIdleTimeoutCfg(int radioIdx, int wlanIdx , int time)
{
    int val = time * 60;
    char option[64]={0};

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, time);
    
    snprintf(option,sizeof(option),"portal.ssid_%d.idleTimeout",wlanIdx+1);

    if (api_set_integer_option(option, val))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalIdleTimeoutEnableCfg(int radioIdx, int wlanIdx , int *enable)
{
    char option[64]={0};

    snprintf(option,sizeof(option),"portal.ssid_%d.idleTimeoutEnable",wlanIdx+1);

    if (api_get_bool_option(option, enable))
        *enable = 0;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalIdleTimeoutEnableCfg(int radioIdx, int wlanIdx , int enable)
{
    char option[64]={0};

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    snprintf(option,sizeof(option),"portal.ssid_%d.idleTimeoutEnable",wlanIdx+1);
    
    if (api_set_bool_option(option, enable))
    {
        CWDebugLog("Set configuration fail");
    }
    
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalAccountingEnableCfg(int radioIdx, int wlanIdx , int *enable)
{
    char option[64]={0};

    snprintf(option,sizeof(option),"portal.ssid_%d.accountingEnable",wlanIdx+1);

    if (api_get_bool_option(option, enable))
        *enable = 0;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalAccountingEnableCfg(int radioIdx, int wlanIdx , int enable)
{
    char option[64]={0};

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    snprintf(option,sizeof(option),"portal.ssid_%d.accountingEnable",wlanIdx+1);
    
    if (api_set_bool_option(option, enable))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalAccountingIntervalCfg(int radioIdx, int wlanIdx , int *time)
{
    char option[64]={0};

    snprintf(option,sizeof(option),"portal.ssid_%d.accountingInterval",wlanIdx+1);
    
    if (api_get_integer_option(option, time))
        *time = 0;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *time);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalAccountingIntervalCfg(int radioIdx, int wlanIdx , int time)
{
    char option[64]={0};

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, time);

    snprintf(option,sizeof(option),"portal.ssid_%d.accountingInterval",wlanIdx+1);

    if (api_set_integer_option(option, time))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalAuthTypeCfg(int radioIdx, int wlanIdx , int *type)
{
    char option[64]={0};

    snprintf(option,sizeof(option),"portal.ssid_%d.authType",wlanIdx+1);

    if (api_get_integer_option(option, type))
        *type = 0;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *type);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalAuthTypeCfg(int radioIdx, int wlanIdx , int type)
{
    char option[64]={0};

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, type);

    snprintf(option,sizeof(option),"portal.ssid_%d.authType",wlanIdx+1);

    if (api_set_integer_option(option, type))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalExternalServerCfg(int radioIdx, int wlanIdx , char **pstr)
{
    char option[64]={0};
    char *val = NULL;

    snprintf(option,sizeof(option),"portal.ssid_%d.externalServer",wlanIdx+1);

    CW_CREATE_STRING_ERR(val, URL_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(option, val, URL_LENGTH))
        snprintf(val, URL_LENGTH, "%s", "");

    *pstr = val;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalExternalServerCfg(int radioIdx, int wlanIdx , char *pstr)
{
    char option[64]={0};
    char *val = NULL;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

    if (pstr == NULL || (!strcmp(pstr,"")))
        return CW_TRUE;

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (val == NULL)
        snprintf(val, URL_LENGTH, "%s", "");

    snprintf(option,sizeof(option),"portal.ssid_%d.externalServer",wlanIdx+1);

    if (api_set_string_option(option, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;

}

CWBool CWWTPBoardGetWlanPortalExternalSecretCfg(int radioIdx, int wlanIdx , char **pstr)
{
    char option[64]={0};
    char *val = NULL;

    snprintf(option,sizeof(option),"portal.ssid_%d.externalSecret",wlanIdx+1);

    CW_CREATE_STRING_ERR(val, RADIUS_SECRET_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(option, val, RADIUS_SECRET_LENGTH))
        snprintf(val, RADIUS_SECRET_LENGTH, "%s", "testing123");

    *pstr = val;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalExternalSecretCfg(int radioIdx, int wlanIdx , char *pstr)
{
    char option[64]={0};
    char *val = NULL;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

    if (pstr == NULL || (!strcmp(pstr,"")))
        return CW_TRUE;

    snprintf(option,sizeof(option),"portal.ssid_%d.externalSecret",wlanIdx+1);

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (val == NULL)
        snprintf(val, RADIUS_SECRET_LENGTH, "%s", "");

    if (api_set_string_option(option, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;;
}

CWBool CWWTPBoardGetWlanPortalAccountingServerCfg(int radioIdx, int wlanIdx , unsigned int *addr)
{
    char option[64]={0};
    char *val = NULL;

    snprintf(option,sizeof(option),"portal.ssid_%d.accountingServer",wlanIdx+1);

    CW_CREATE_STRING_ERR(val, SERVER_IPADDR_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(option, val, SERVER_IPADDR_LENGTH))
        *addr = 0;
    else
    {
        if(val[0])
            *addr = inet_addr(val);
        else
            *addr = 0;
    }

    CW_FREE_OBJECT(val);

    CWDebugLog("%s %d %d %d %lu", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *addr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalAccountingServerCfg(int radioIdx, int wlanIdx , unsigned int addr)
{
    char option[64]={0};
    char *val = NULL;

    CWDebugLog("%s %d %d %d %u.%u.%u.%u", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, CW_IPV4_PRINT_LIST(addr));

    snprintf(option,sizeof(option),"portal.ssid_%d.accountingServer",wlanIdx+1);

    CW_CREATE_STRING_ERR(val, SERVER_IPADDR_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (addr)
        snprintf(val, SERVER_IPADDR_LENGTH, "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
    else
        snprintf(val, SERVER_IPADDR_LENGTH, "0.0.0.0");

    if (api_set_string_option(option, val, SERVER_IPADDR_LENGTH))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalAccountingPortCfg(int radioIdx, int wlanIdx , unsigned short *port)
{
    char option[64]={0};
    int val = 0;

    snprintf(option,sizeof(option),"portal.ssid_%d.accountingPort",wlanIdx+1);

    if (api_get_integer_option(option, &val))
        val = 1813;

    *port = (unsigned short) val;

    CWDebugLog("%s %d %d %d %u", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *port);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalAccountingPortCfg(int radioIdx, int wlanIdx , unsigned short port)
{
    char option[64]={0};
    int val = 0;

    CWDebugLog("%s %d %d %d %u", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, port);

    snprintf(option,sizeof(option),"portal.ssid_%d.accountingPort",wlanIdx+1);
    
    val = (int) port;

    if (api_set_integer_option(option, val))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalAccountingSecretCfg(int radioIdx, int wlanIdx , char **pstr)
{
    char option[64]={0};
    char *val = NULL;

    snprintf(option,sizeof(option),"portal.ssid_%d.accountingSecret",wlanIdx+1);

    CW_CREATE_STRING_ERR(val, RADIUS_SECRET_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(option, val, RADIUS_SECRET_LENGTH))
        snprintf(val, RADIUS_SECRET_LENGTH, "%s", "testing123");

    *pstr = val;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalAccountingSecretCfg(int radioIdx, int wlanIdx , char *pstr)
{
    char option[64]={0};
    char *val = NULL;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

    if (pstr == NULL || (!strcmp(pstr,"")))
        return CW_TRUE;

    snprintf(option,sizeof(option),"portal.ssid_%d.accountingSecret",wlanIdx+1);

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (val == NULL)
        snprintf(val, RADIUS_SECRET_LENGTH, "%s", "");

    if (api_set_string_option(option, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalUamformatCfg(int radioIdx, int wlanIdx , char **pstr)
{
    char option[64]={0};
    char *val = NULL;

    snprintf(option,sizeof(option),"portal.ssid_%d.uamformat",wlanIdx+1);

    CW_CREATE_STRING_ERR(val, URL_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(option, val, URL_LENGTH))
        snprintf(val, URL_LENGTH, "%s", "sn.captivePortal.login");

    *pstr = val;

    CWDebugLog("%s %d %d %d %s", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalUamformatCfg(int radioIdx, int wlanIdx , char *pstr)
{
    char *val = NULL;
    char option[64]={0};

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

    if (pstr == NULL || (!strcmp(pstr,"")))
        return CW_TRUE;

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (val == NULL)
        snprintf(val, URL_LENGTH, "%s", "");

    snprintf(option,sizeof(option),"portal.ssid_%d.uamformat",wlanIdx+1);

    if (api_set_string_option(option, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalLocalAuthCfg(int radioIdx, int wlanIdx , char **pstr)
{
    char *val = NULL;
    char option[64]={0};

    snprintf(option,sizeof(option),"portal.ssid_%d.localAuth",wlanIdx+1);

    CW_CREATE_STRING_ERR(val, AUTH_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(option, val, AUTH_LENGTH))
        snprintf(val, AUTH_LENGTH, "%s", "sn.captivePortal.auth");

    *pstr = val;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalLocalAuthCfg(int radioIdx, int wlanIdx , char *pstr)
{
    char *val = NULL;
    char option[64]={0};

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

    if (pstr == NULL || (!strcmp(pstr,"")))
        return CW_TRUE;

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (val == NULL)
        snprintf(val, AUTH_LENGTH, "%s", "");

    snprintf(option,sizeof(option),"portal.ssid_%d.localAuth",wlanIdx+1);

    if (api_set_string_option(option, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalPortCfg(int radioIdx, int wlanIdx , unsigned short *port)
{
    char option[64]={0};
    int val = 0;

    snprintf(option,sizeof(option),"portal.ssid_%d.port",wlanIdx+1);

    if (api_get_integer_option(option, &val))
        val = 0;

    *port = (unsigned short) val;

    CWDebugLog("%s %d %d %d %u", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *port);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalPortCfg(int radioIdx, int wlanIdx , unsigned short port)
{
    char option[64]={0};
    int val = 0;

    CWDebugLog("%s %d %d %d %u", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, port);

    snprintf(option,sizeof(option),"portal.ssid_%d.port",wlanIdx+1);

    val = (int) port;

    if (api_set_integer_option(option, val))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalHttpsEnableCfg(int radioIdx, int wlanIdx , int *enable)
{
    char option[64]={0};

    snprintf(option,sizeof(option),"portal.ssid_%d.httpsEnable",wlanIdx+1);

    if (api_get_bool_option(option, enable))
        *enable = 0;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalHttpsEnableCfg(int radioIdx, int wlanIdx , int enable)
{
    char option[64]={0};

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    snprintf(option,sizeof(option),"portal.ssid_%d.httpsEnable",wlanIdx+1);

    if (api_set_bool_option(option, enable))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalRadiusSecret2Cfg(int radioIdx, int wlanIdx , char **pstr)
{
    char option[64]={0};
    char *val = NULL;

    snprintf(option,sizeof(option),"portal.ssid_%d.radiusSecret2",wlanIdx+1);

    CW_CREATE_STRING_ERR(val, RADIUS_SECRET_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(option, val, RADIUS_SECRET_LENGTH))
        snprintf(val, RADIUS_SECRET_LENGTH, "%s", "");

    *pstr = val;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalRadiusSecret2Cfg(int radioIdx, int wlanIdx , char *pstr)
{
    char option[64]={0};
    char *val = NULL;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

    if (pstr == NULL || (!strcmp(pstr,"")))
        return CW_TRUE;

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (val == NULL)
        snprintf(val, RADIUS_SECRET_LENGTH, "%s", "");

    snprintf(option,sizeof(option),"portal.ssid_%d.radiusSecret2",wlanIdx+1);

    if (api_set_string_option(option, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalRadius2Cfg(int radioIdx, int wlanIdx , unsigned int *addr)
{
    char option[64]={0};
    char *val = NULL;

    snprintf(option,sizeof(option),"portal.ssid_%d.radiusServer2",wlanIdx+1);

    CW_CREATE_STRING_ERR(val, SERVER_IPADDR_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_string_option(option, val, SERVER_IPADDR_LENGTH))
        *addr = 0;
    else
    {
        if(val[0])
            *addr = inet_addr(val);
        else
            *addr = 0;
    }

    CW_FREE_OBJECT(val);

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *addr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalRadius2Cfg(int radioIdx, int wlanIdx , unsigned int addr)
{
    char option[64]={0};
    char *val = NULL;

    CWDebugLog("%s %d %d %d %u.%u.%u.%u", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, CW_IPV4_PRINT_LIST(addr));

    snprintf(option,sizeof(option),"portal.ssid_%d.radiusServer2",wlanIdx+1);

    CW_CREATE_STRING_ERR(val, SERVER_IPADDR_LENGTH, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (addr)
        snprintf(val, SERVER_IPADDR_LENGTH, "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
    else
        snprintf(val, SERVER_IPADDR_LENGTH, "0.0.0.0");

    if (api_set_string_option(option, val, SERVER_IPADDR_LENGTH))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalNetworkTypeCfg(int radioIdx, int wlanIdx , int *type)
{
    char option[64]={0};
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, 16, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    snprintf(option,sizeof(option),"portal.%s_%d.guest_network","ssid",wlanIdx+1);

    if (api_get_string_option(option, val, 16))
        *type = 0;
    else
        *type = (strcmp("NAT",val)==0)?3:(strcmp("Bridge",val)==0)?2:(strcmp("Enable",val)==0)?1:0;

    CW_FREE_OBJECT(val);

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *type);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalNetworkTypeCfg(int radioIdx, int wlanIdx , int type)
{
    char option[64]={0};
    char *val = NULL;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, type);

    snprintf(option,sizeof(option),"portal.%s_%d.guest_network","ssid",wlanIdx+1);

    CW_CREATE_STRING_ERR(val, 16, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    switch (type)
    {
    case 1:
        snprintf(val,16,"Enable");
        break;
    case 2:
        snprintf(val,16,"Bridge");
        break;
    case 3:
        snprintf(val,16,"NAT");
        break;
    default:
        snprintf(val,16,"Disable");
        break;
    }

    if((api_set_string_option(option, val, strlen(val))))
    {
        CWDebugLog("Set configuration fail");
    }

    CWDebugLog("%s %d %d %d type:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, type);

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanGuestNetworkTypeCfg(int radioIdx, int wlanIdx , int *type)
{
    char option[64]={0};
    char *val = NULL;

    CW_CREATE_STRING_ERR(val, 16, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    snprintf(option,sizeof(option),"wireless.wifi%d_ssid_%d.guest_network",CWConvertRadioIdx(radioIdx),wlanIdx+1);

    if (api_get_string_option(option, val, 16))
        *type = 0;
    else
        *type = (strcmp("NAT",val)==0)?3:(strcmp("Bridge",val)==0)?2:(strcmp("Enable",val)==0)?1:0;

    CW_FREE_OBJECT(val);

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *type);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanGuestNetworkTypeCfg(int radioIdx, int wlanIdx , int type)
{
    char option[64]={0};
    char *val = NULL;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, type);

    snprintf(option,sizeof(option),"wireless.wifi%d_ssid_%d.guest_network",CWConvertRadioIdx(radioIdx),wlanIdx+1);

    CW_CREATE_STRING_ERR(val, 16, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    switch (type) 
    {
    case 1:
        snprintf(val,16,"Enable");
        break;
    case 2:
        snprintf(val,16,"Bridge");
        break;
    case 3:
        snprintf(val,16,"NAT");
        break;
    default:
        snprintf(val,16,"Disable");
        break;
    }

    if((api_set_string_option(option, val, strlen(val))))
    {
        CWDebugLog("Set configuration fail");
    }

    CWDebugLog("%s %d %d %d type:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, type);

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalNasIdCfg(int radioIdx, int wlanIdx , char *pstr)
{
    char option[64]={0};
    char *val = NULL;

    CWDebugLog("%s %d %d %d [%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(val, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (val == NULL)
        snprintf(val, 128, "%s", "");

    snprintf(option,sizeof(option),"portal.ssid_%d.nasid",wlanIdx+1);

    if (api_set_string_option(option, val, strlen(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalNasPortCfg(int radioIdx, int wlanIdx , unsigned short port)
{
    char option[64]={0};
    int val = 0;

    CWDebugLog("%s %d %d %d [%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, port);

    val = (int) port;

    snprintf(option,sizeof(option),"portal.ssid_%d.nasport",wlanIdx+1);

    if (api_set_integer_option(option, val))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalNasIPCfg(int radioIdx, int wlanIdx, unsigned int addr)
{
    char option[64]={0};
    char val[15+1];

    CWDebugLog("%s %d %d %d [%u.%u.%u.%u]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, CW_IPV4_PRINT_LIST(addr));

    snprintf(option,sizeof(option),"portal.ssid_%d.nasip",wlanIdx+1);

    if (addr)
        snprintf(val, sizeof(val), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
    else
        snprintf(val, sizeof(val), "");

    if (api_set_string_option(option, val, sizeof(val)))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanPortalVlanTagCfg(int radioIdx, int wlanIdx , int *vlan)
{
    char option[64]={0};
    int val = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);

    if (GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)) == wlanIdx)
    {
        return CW_TRUE;
    }

    snprintf(option, sizeof(option), "portal.ssid_%d.vlantag", wlanIdx+1);

    if (api_get_integer_option(option, &val))
    {
        val = 0;
    }

    *vlan = val;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *vlan);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPortalVlanTagCfg(int radioIdx, int wlanIdx, int vlan)
{
    char option[64]={0};

    CWDebugLog("%s %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, vlan);

    if (GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)) == wlanIdx)
    {
        return CW_TRUE;
    }

    snprintf(option, sizeof(option), "portal.ssid_%d.vlantag", wlanIdx+1);

    CWDebugLog("%s %d %d %s=%d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, option, vlan);

    if (api_set_integer_option(option, vlan))
    {
        CWDebugLog("Set configuration fail");
    }
    return CW_TRUE;
}
#endif
