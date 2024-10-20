#include "WTPBoardApiNas.h"
#include "WTPBoardApiCommon.h"
#include <ctype.h>
#include <variable/api_wireless.h>

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

#ifdef CW_DEF_AP_WIFI_IFACE_NUM
int configuration_wlan_nas[WIFI_RADIO_NUM][CW_DEF_AP_WIFI_IFACE_NUM+1]={0};
#else
int configuration_wlan_nas[WIFI_RADIO_NUM][8+1]={0};
#endif

CWBool CWWTPBoardSetWlanNasIdEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    CWDebugLog("%s %d %d %d", __FUNCTION__,CWConvertRadioIdx(radioIdx),wlanIdx,enable);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(enable != 0)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support NAS ID Enable");
        }
        return CW_TRUE;
    }

    switch (CWConvertRadioIdx(radioIdx))
    {
        case 0:
            if (api_set_wifi_ifacex_nasid_enabled_option_by_sectionname(OPM_AP, "wireless.wifi0_ssid", wlanIdx+1, enable))
            {
                return CW_TRUE;
            }
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            if (api_set_wifi_ifacex_nasid_enabled_option_by_sectionname(OPM_AP, "wireless.wifi1_ssid", wlanIdx+1, enable))
            {
                return CW_TRUE;
            }
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            if (api_set_wifi_ifacex_nasid_enabled_option_by_sectionname(OPM_AP, "wireless.wifi2_ssid", wlanIdx+1, enable))
            {
                return CW_TRUE;
            }
            break;
#endif
#endif
        default:
            return CW_TRUE;
    }
    configuration_wlan_nas[radioIdx][wlanIdx]=CW_TRUE;
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanNasIdEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int val = 0;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        *enable = 0;
        return CW_TRUE;
    }

    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            if(api_get_wifi_ifacex_nasid_enabled_option_by_sectionname(OPM_AP,"wireless.wifi0_ssid",(wlanIdx+1),&val))
            {
                api_set_wifi_ifacex_nasid_enabled_option_by_sectionname(OPM_AP, "wireless.wifi0_ssid", wlanIdx+1,val);
            }
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            if(api_get_wifi_ifacex_nasid_enabled_option_by_sectionname(OPM_AP,"wireless.wifi1_ssid",(wlanIdx+1),&val))
            {
                api_set_wifi_ifacex_nasid_enabled_option_by_sectionname(OPM_AP, "wireless.wifi1_ssid", wlanIdx+1,val);
            }
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            if(api_get_wifi_ifacex_nasid_enabled_option_by_sectionname(OPM_AP,"wireless.wifi2_ssid",(wlanIdx+1),&val))
            {
                api_set_wifi_ifacex_nasid_enabled_option_by_sectionname(OPM_AP, "wireless.wifi2_ssid", wlanIdx+1,val);
            }
            break;
#endif
#endif
        default:
            CWDebugLog("Invalid radio mode");
            break;
    }

    *enable = val;

    CWDebugLog("%s %d %d %d", __FUNCTION__,CWConvertRadioIdx(radioIdx),wlanIdx,*enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanNasPortEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    CWDebugLog("%s %d %d %d", __FUNCTION__,CWConvertRadioIdx(radioIdx),wlanIdx,enable);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(enable != 0)
        {
            CWDebugLog("Guest wlan cannot support NAS Port Enable");
        }
        return CW_TRUE;
    }

    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            if (api_set_wifi_ifacex_nasport_enabled_option_by_sectionname(OPM_AP, "wireless.wifi0_ssid", wlanIdx+1, enable))
            {
                return CW_TRUE;
            }
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            if (api_set_wifi_ifacex_nasport_enabled_option_by_sectionname(OPM_AP, "wireless.wifi1_ssid", wlanIdx+1, enable))
            {
                return CW_TRUE;
            }
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            if (api_set_wifi_ifacex_nasport_enabled_option_by_sectionname(OPM_AP, "wireless.wifi2_ssid", wlanIdx+1, enable))
            {
                return CW_TRUE;
            }
            break;
#endif
#endif
        default:
            return CW_TRUE;
    }
    configuration_wlan_nas[radioIdx][wlanIdx]=CW_TRUE;
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanNasPortEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int val = 0;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        *enable = 0;
        return CW_TRUE;
    }

    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            if(api_get_wifi_ifacex_nasport_enabled_option_by_sectionname(OPM_AP,"wireless.wifi0_ssid",(wlanIdx+1),&val))
            {
                api_set_wifi_ifacex_nasport_enabled_option_by_sectionname(OPM_AP, "wireless.wifi0_ssid", wlanIdx+1,val);
            }
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            if(api_get_wifi_ifacex_nasport_enabled_option_by_sectionname(OPM_AP,"wireless.wifi1_ssid",(wlanIdx+1),&val))
            {
                api_set_wifi_ifacex_nasport_enabled_option_by_sectionname(OPM_AP, "wireless.wifi1_ssid", wlanIdx+1,val);
            }
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            if(api_get_wifi_ifacex_nasport_enabled_option_by_sectionname(OPM_AP,"wireless.wifi2_ssid",(wlanIdx+1),&val))
            {
                api_set_wifi_ifacex_nasport_enabled_option_by_sectionname(OPM_AP, "wireless.wifi2_ssid", wlanIdx+1,val);
            }
            break;
#endif
#endif
        default:
            CWDebugLog("Invalid radio mode");
            break;
    }

    *enable = val;

    CWDebugLog("%s %d %d %d", __FUNCTION__,CWConvertRadioIdx(radioIdx),wlanIdx,*enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanNasIPEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    CWDebugLog("%s %d %d %d", __FUNCTION__,CWConvertRadioIdx(radioIdx),wlanIdx,enable);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(enable != 0)
        {
            CWDebugLog("Guest wlan cannot support NAS IP Enable");
        }
        return CW_TRUE;
    }

    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            if (api_set_wifi_ifacex_nasip_enabled_option_by_sectionname(OPM_AP, "wireless.wifi0_ssid", wlanIdx+1, enable))
            {
                return CW_TRUE;
            }
            break;

#if SUPPORT_WLAN_5G_SETTING
        case 1:
            if (api_set_wifi_ifacex_nasip_enabled_option_by_sectionname(OPM_AP, "wireless.wifi1_ssid", wlanIdx+1, enable))
            {
                return CW_TRUE;
            }
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            if (api_set_wifi_ifacex_nasip_enabled_option_by_sectionname(OPM_AP, "wireless.wifi2_ssid", wlanIdx+1, enable))
            {
                return CW_TRUE;
            }
            break;
#endif
#endif
        default:
            return CW_TRUE;
    }
    configuration_wlan_nas[radioIdx][wlanIdx]=CW_TRUE;
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanNasIPEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int val = 0;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        *enable = 0;
        return CW_TRUE;
    }

    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            if(api_get_wifi_ifacex_nasip_enabled_option_by_sectionname(OPM_AP,"wireless.wifi0_ssid",(wlanIdx+1),&val))
            {
                api_set_wifi_ifacex_nasip_enabled_option_by_sectionname(OPM_AP, "wireless.wifi0_ssid", wlanIdx+1,val);
            }
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            if(api_get_wifi_ifacex_nasip_enabled_option_by_sectionname(OPM_AP,"wireless.wifi1_ssid",(wlanIdx+1),&val))
            {
                api_set_wifi_ifacex_nasip_enabled_option_by_sectionname(OPM_AP, "wireless.wifi1_ssid", wlanIdx+1,val);
            }
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            if(api_get_wifi_ifacex_nasip_enabled_option_by_sectionname(OPM_AP,"wireless.wifi2_ssid",(wlanIdx+1),&val))
            {
                api_set_wifi_ifacex_nasip_enabled_option_by_sectionname(OPM_AP, "wireless.wifi2_ssid", wlanIdx+1,val);
            }
            break;
#endif
#endif
        default:
            CWDebugLog("Invalid radio mode");
            break;
    }

    *enable = val;

    CWDebugLog("%s %d %d %d", __FUNCTION__,CWConvertRadioIdx(radioIdx),wlanIdx,*enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanNasIdCfg(int radioIdx, int wlanIdx, char *pstr)
{
    char *pUciStr = NULL;

    CWDebugLog("%s %d %d %s", __FUNCTION__,CWConvertRadioIdx(radioIdx),wlanIdx,pstr);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(pstr[0] != 0)
        {
            CWDebugLog("Guest wlan cannot support NAS ID");
        }
        return CW_TRUE;
    }

    CW_CREATE_STRING_FROM_STRING_ERR(pUciStr, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            if (api_set_wifi_ifacex_nasid_option_by_sectionname(OPM_AP, "wireless.wifi0_ssid", wlanIdx+1, pUciStr, strlen(pUciStr)))
            {
                CW_FREE_OBJECT(pUciStr);
                return CW_TRUE;
            }
            break;

#if SUPPORT_WLAN_5G_SETTING
        case 1:
            if (api_set_wifi_ifacex_nasid_option_by_sectionname(OPM_AP, "wireless.wifi1_ssid", wlanIdx+1, pUciStr, strlen(pUciStr)))
            {
                CW_FREE_OBJECT(pUciStr);
                return CW_TRUE;
            }
            break;

#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            if (api_set_wifi_ifacex_nasid_option_by_sectionname(OPM_AP, "wireless.wifi2_ssid", wlanIdx+1, pUciStr, strlen(pUciStr)))
            {
                CW_FREE_OBJECT(pUciStr);
                return CW_TRUE;
            }
            break;
#endif
#endif
        default:
            CW_FREE_OBJECT(pUciStr);
            return CW_TRUE;
    }

    CW_FREE_OBJECT(pUciStr);
    configuration_wlan_nas[radioIdx][wlanIdx]=CW_TRUE;
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanNasIdCfg(int radioIdx, int wlanIdx, char **pstr)
{
    char *val = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        return CW_TRUE;
    }

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", MAX_NASID_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            if(api_get_wifi_ifacex_nasid_option_by_sectionname(OPM_AP,"wireless.wifi0_ssid",(wlanIdx+1),val,MAX_NASID_SIZE+1))
            {
                CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
                CW_FREE_OBJECT(val);
            }
            else
            {
                    *pstr = val;
            }
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            if(api_get_wifi_ifacex_nasid_option_by_sectionname(OPM_AP,"wireless.wifi1_ssid",(wlanIdx+1),val,MAX_NASID_SIZE+1))
            {
                CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
                CW_FREE_OBJECT(val);
            }
            else
            {
                    *pstr = val;
            }
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            if(api_get_wifi_ifacex_nasid_option_by_sectionname(OPM_AP,"wireless.wifi2_ssid",(wlanIdx+1),val,MAX_NASID_SIZE+1))
            {
                CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
                CW_FREE_OBJECT(val);
            }
            else
            {
                    *pstr = val;
            }
            break;
#endif
#endif
        default:
            CWDebugLog("Invalid radio mode");
            break;
    }

    if(*pstr)
    	CWDebugLog("%s %d %d %s", __FUNCTION__,CWConvertRadioIdx(radioIdx),wlanIdx,*pstr);
    else
	CWDebugLog("%s %d %d NULL", __FUNCTION__,CWConvertRadioIdx(radioIdx),wlanIdx);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanNasPortCfg(int radioIdx, int wlanIdx, unsigned short port)
{
    CWDebugLog("%s %d %d %u", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, port);

    if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (port)
        {
            CWDebugLog("Guest wlan cannot support NAS Port");
        }
        return CW_TRUE;
    }

    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            if (api_set_wifi_ifacex_nasport_option_by_sectionname(OPM_AP, "wireless.wifi0_ssid", wlanIdx+1, (int*)port))
            {
                CWDebugLog("Set configuration fail");
                return CW_TRUE;
            }
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            if (api_set_wifi_ifacex_nasport_option_by_sectionname(OPM_AP, "wireless.wifi1_ssid", wlanIdx+1, (int*)port))
            {
                CWDebugLog("Set configuration fail");
                return CW_TRUE;
            }
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            if (api_set_wifi_ifacex_nasport_option_by_sectionname(OPM_AP, "wireless.wifi2_ssid", wlanIdx+1, (int*)port))
            {
                CWDebugLog("Set configuration fail");
                return CW_TRUE;
            }
            break;
#endif
#endif
        default:
            CWDebugLog("Invalid radio mode");
            return CW_TRUE;
    }
    configuration_wlan_nas[radioIdx][wlanIdx]=CW_TRUE;
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanNasPortCfg(int radioIdx, int wlanIdx, unsigned short *port)
{
    int val = 0;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        *port = 0;
        return CW_TRUE;
    }

    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            if(api_get_wifi_ifacex_nasport_option_by_sectionname(OPM_AP,"wireless.wifi0_ssid",(wlanIdx+1),&val))
            {
                api_set_wifi_ifacex_nasport_option_by_sectionname(OPM_AP, "wireless.wifi0_ssid", wlanIdx+1,val);
            }
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            if(api_get_wifi_ifacex_nasport_option_by_sectionname(OPM_AP,"wireless.wifi1_ssid",(wlanIdx+1),&val))
            {
                api_set_wifi_ifacex_nasport_option_by_sectionname(OPM_AP, "wireless.wifi1_ssid", wlanIdx+1,val);
            }
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            if(api_get_wifi_ifacex_nasport_option_by_sectionname(OPM_AP,"wireless.wifi2_ssid",(wlanIdx+1),&val))
            {
                api_set_wifi_ifacex_nasport_option_by_sectionname(OPM_AP, "wireless.wifi2_ssid", wlanIdx+1,val);
            }
            break;
#endif
#endif
        default:
            CWDebugLog("Invalid radio mode");
            break;
    }

    *port = val;

    CWDebugLog("%s %d %d %u", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, *port);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanNasIPCfg(int radioIdx, int wlanIdx, unsigned int addr)
{
    char val[15+1]={0};

    CWDebugLog("%s %d %d (%u.%u.%u.%u)", __FUNCTION__,CWConvertRadioIdx(radioIdx), wlanIdx, CW_IPV4_PRINT_LIST(addr));

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(addr)
        {
            CWDebugLog("Guest wlan cannot support NAS IP Address");
        }
        return CW_TRUE;
    }

    if(addr)
        snprintf(val, sizeof(val), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
    else
        snprintf(val, sizeof(val), " ");

    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            if (api_set_wifi_ifacex_nasip_option_by_sectionname(OPM_AP, "wireless.wifi0_ssid", wlanIdx+1, val, sizeof(val)))
            {
                CWDebugLog("Set configuration fail");
                return CW_TRUE;

            }
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            if (api_set_wifi_ifacex_nasip_option_by_sectionname(OPM_AP, "wireless.wifi1_ssid", wlanIdx+1, val, sizeof(val)))
            {
                CWDebugLog("Set configuration fail");
                return CW_TRUE;
            }
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            if (api_set_wifi_ifacex_nasip_option_by_sectionname(OPM_AP, "wireless.wifi2_ssid", wlanIdx+1, val, sizeof(val)))
            {
                CWDebugLog("Set configuration fail");
                return CW_TRUE;
            }
            break;
#endif
#endif
        default:
            CWDebugLog("Invalid radio mode");
            return CW_TRUE;
    }
    configuration_wlan_nas[radioIdx][wlanIdx]=CW_TRUE;
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanNasIPCfg(int radioIdx, int wlanIdx, unsigned int *addr)
{
    char val[15+1]={0};

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        *addr = 0;
        return CW_TRUE;
    }

    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            if(api_get_wifi_ifacex_nasip_option_by_sectionname(OPM_AP,"wireless.wifi0_ssid",(wlanIdx+1),val ,sizeof(val)))
            {
                *addr = 0;
            }
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            if(api_get_wifi_ifacex_nasip_option_by_sectionname(OPM_AP,"wireless.wifi1_ssid",(wlanIdx+1),val ,sizeof(val)))
            {
                *addr = 0;
            }
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            if(api_get_wifi_ifacex_nasip_option_by_sectionname(OPM_AP,"wireless.wifi2_ssid",(wlanIdx+1),val ,sizeof(val)))
            {
                *addr = 0;
            }
            break;
#endif
#endif
        default:
            *addr = 0;
            CWDebugLog("Invalid radio mode");
            return CW_TRUE;
    }

    if(val[0])
        *addr = inet_addr(val);
    else
	*addr = 0;

    CWDebugLog("%s %d %d %s", __FUNCTION__,CWConvertRadioIdx(radioIdx), wlanIdx,val);

    return CW_TRUE;
}

