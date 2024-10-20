#include "WTPBoardApiNas.h"
#include "WTPBoardApiCommon.h"
#include <ctype.h>
#include <variable/api_wireless.h>

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

CWBool CWWTPBoardSetWlanNasIdEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(enable != 0)
        {
            CWDebugLog("Guest wlan cannot support NAS ID Enable");
        }
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        CWDebugLog("Invalid operation mode");
        return CW_TRUE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if (api_set_wifi_ifacex_nasid_enabled_option((index+wlanIdx), enable))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanNasIdEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int opMode = 0, index = 0, val = 0;

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        *enable = 0;    
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        CWDebugLog("Invalid operation mode");
        return CW_TRUE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if (api_get_wifi_ifacex_nasid_enabled_option((index+wlanIdx), &val))
    {
        if (api_set_wifi_ifacex_nasid_enabled_option((index+wlanIdx), val))
        {
            CWDebugLog("Set configuration fail");
        }
    }

    *enable = val;

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}
CWBool CWWTPBoardSetWlanNasPortEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(enable != 0)
        {
            CWDebugLog("Guest wlan cannot support NAS Port Enable");
        }
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        CWDebugLog("Invalid operation mode");
        return CW_TRUE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if (api_set_wifi_ifacex_nasport_enabled_option((index+wlanIdx), enable))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanNasPortEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int opMode = 0, index = 0, val = 0;

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        *enable = 0;    
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        CWDebugLog("Invalid operation mode");
        return CW_TRUE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if (api_get_wifi_ifacex_nasport_enabled_option((index+wlanIdx), &val))
    {
        if (api_set_wifi_ifacex_nasport_enabled_option((index+wlanIdx), val))
        {
            CWDebugLog("Set configuration fail");
        }
    }

    *enable = val;

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);
    
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanNasIdCfg(int radioIdx, int wlanIdx, char **pstr)
{
    int opMode = 0, index = 0;
    char *val = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    { 
        CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        CWDebugLog("Invalid operation mode");
        return CW_TRUE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", MAX_NASID_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (api_get_wifi_ifacex_nasid_option((index + wlanIdx), val, MAX_NASID_SIZE+1))
    {
        CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        CW_FREE_OBJECT(val);
    }
    else
    {
        *pstr = val;
    }

    CWDebugLog("%s %d radio:[%d] wlanIdx:[%d] %s", __FUNCTION__, __LINE__, radioIdx, wlanIdx, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanNasIdCfg(int radioIdx, int wlanIdx, char *pstr)
{
    int opMode = 0, index = 0;
    char *pUciStr = NULL;

    CWDebugLog("%s %d %d %s", __FUNCTION__, radioIdx, wlanIdx, pstr);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(pstr[0] != 0)
        {
            CWDebugLog("Guest wlan cannot support NAS ID");
        }
        return CW_TRUE;
    }

    CW_CREATE_STRING_FROM_STRING_ERR(pUciStr, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        CWDebugLog("Invalid operation mode");
        CW_FREE_OBJECT(pUciStr);
        return CW_TRUE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {   
        CWDebugLog("Get configuration fail");
        CW_FREE_OBJECT(pUciStr);
        return CW_TRUE;
    }

    if (api_set_wifi_ifacex_nasid_option((index+wlanIdx), pUciStr, strlen(pUciStr)))
    {
        CWDebugLog("Set configuration fail");
    }

    CW_FREE_OBJECT(pUciStr);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanNasPortCfg(int radioIdx, int wlanIdx, unsigned short *port)
{
    int opMode = 0, index = 0, val = 0;

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {   
        *port = 0;
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        CWDebugLog("Invalid operation mode");
        return CW_TRUE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if (api_get_wifi_ifacex_nasport_option((index+wlanIdx), &val))
    {
        if (api_set_wifi_ifacex_nasport_option((index+wlanIdx), val))
        {
            CWDebugLog("Set configuration fail");
        }
    }

    *port = val;

    CWDebugLog("%s %d %u", __FUNCTION__, __LINE__, *port);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanNasPortCfg(int radioIdx, int wlanIdx, unsigned short port)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %u", __FUNCTION__, radioIdx, wlanIdx, port);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(port)
        {
            CWDebugLog("Guest wlan cannot support NAS Port");
        }
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        CWDebugLog("Invalid operation mode");
        return CW_TRUE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if (api_set_wifi_ifacex_nasport_option((index+wlanIdx), (int*)port))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanNasIPEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(enable != 0)
        {
            CWDebugLog("Guest wlan cannot support NAS IP Enable");
        }
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        CWDebugLog("Invalid operation mode");
        return CW_TRUE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if (api_set_wifi_ifacex_nasip_enabled_option((index+wlanIdx), enable))
    {
        CWDebugLog("Set configuration fail");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanNasIPEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int opMode = 0, index = 0, val = 0;

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        *enable = 0;    
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        CWDebugLog("Invalid operation mode");
        return CW_TRUE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if (api_get_wifi_ifacex_nasip_enabled_option((index+wlanIdx), &val))
    {
        if (api_set_wifi_ifacex_nasip_enabled_option((index+wlanIdx), val))
        {
            CWDebugLog("Set configuration fail");
        }
    }

    *enable = val;

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanNasIPCfg(int radioIdx, int wlanIdx, unsigned int *addr)
{
    int opMode = 0, index = 0;
    char val[15+1]={0};

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        *addr = 0;
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode)) 
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if (opMode == -1) 
    {
        CWDebugLog("Invalid operation mode");
        return CW_TRUE;
    }

    if (api_get_wifi_ifacex_nasip_option((index+wlanIdx), val, sizeof(val)))
    {
        *addr = 0;
    }

    if(val[0])
    {
        *addr = inet_addr(val);
    }

    CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, val);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanNasIPCfg(int radioIdx, int wlanIdx, unsigned int addr)
{
    int opMode = 0, index = 0;
    char val[15+1];

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(addr)
        {
            CWDebugLog("Guest wlan cannot support NAS IP Address");
        }
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2) 
    {
        CWDebugLog("Invalid operation mode");
        return CW_TRUE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode)) 
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if(addr)
    {
        snprintf(val, sizeof(val), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
        if (api_set_wifi_ifacex_nasip_option((index+wlanIdx), val, sizeof(val)))
        {
            CWDebugLog("Set configuration fail");
        }
    }
    else
    {
        snprintf(val, sizeof(val), " ");
        if (api_set_wifi_ifacex_nasip_option((index+wlanIdx), val, sizeof(val)))
        {
            CWDebugLog("Set configuration fail");
        }
    }

    return CW_TRUE;
}
