#include "WTPBoardApiWireless_wn.h"
#include "WTPBoardApiCommon.h"
#include "WTPBoardApiNetwork.h"
#include <ctype.h>
#include <sysWlan.h>
#include <sysCore.h>
#include <wireless_tokens.h>
#include <variable/variable.h>
#include <variable/api_wireless.h>
#include <gconfig.h>
#include <variable/api_guest.h>
#include <json_ssid.h>

#if SUPPORT_WLAN_5G_2_SETTING
#include <sysFile.h>
#endif
#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

static char *wlanIfNameRadio0[] = {"ath0", "ath01", "ath02", "ath03", "ath04", "ath05", "ath06", "ath07", "ath27"};
#if SUPPORT_WLAN_5G_SETTING
static char *wlanIfNameRadio1[] = {"ath1", "ath11", "ath12", "ath13", "ath14", "ath15", "ath16", "ath17", "ath57"};
#if SUPPORT_WLAN_5G_2_SETTING
static char *wlanIfNameRadio2[] = {"ath4", "ath41", "ath42", "ath43", "ath44", "ath45", "ath46", "ath47", "ath67"};
#endif
#endif

static CWBool g_ac_support_tc_rate_unit = CW_FALSE;

#define WIFI_IF_ID(_radio)   (_radio)

static unsigned char radio2GChannel[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
#if SUPPORT_WLAN_5G_SETTING
#if SUPPORT_WLAN_5G_2_SETTING
static unsigned char radio5GChannel[] = {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140,
                                         149, 153, 157, 161, 165, 169, 173
                                        };
static unsigned char radio5G1Channel[] = {36, 40, 44, 48,
                                          52, 56, 60, 64
                                         };
#else
static unsigned char radio5GChannel[] = {36, 40, 44, 48,
                                         52, 56, 60, 64,
                                         100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140,
                                         149, 153, 157, 161, 165, 169, 173
                                        };
#endif
#endif

#ifdef CW_DEF_AP_WIFI_IFACE_NUM
int configuration_wlan_traffic_shapping[WIFI_RADIO_NUM][CW_DEF_AP_WIFI_IFACE_NUM+1]={0};
#else
int configuration_wlan_traffic_shapping[WIFI_RADIO_NUM][8+1]={0};
#endif

char *ltrim(char *str)
{
    int len = 0;
    char *p;

    if (str == NULL || *str == '\0') {
        return str;
    }

    p = str;
    while (*p != '\0' && isspace(*p)) {
        ++p;
        ++len;
    }
    memmove(str, p, strlen(str)-len+1);
    return str;
}

int CWWTPBoardGetMaxRadio()
{
    return WIFI_RADIO_NUM;
}

int CWWTPBoardConfigShowOnly()
{
	int i = 0;
	int qboost_enable = 0;

	for (i=0; i<CWWTPBoardGetMaxRadio(); i++)
	{
		if (CWWTPBoardGetRadioMode(i)!=SYS_OPM_AP)
		{
			//CWDebugLog("%s %d show only:[%d]", __FUNCTION__, __LINE__, CW_TRUE);
			return CW_TRUE;
		}
	}

	if (api_get_integer_option("wireless.wifi1.qboost_enable", &qboost_enable)) //TDMA Enable/Disable
	{
		//CWDebugLog("%s %d show only:[%d]", __FUNCTION__, __LINE__, CW_FALSE);
		return CW_FALSE;
	}

	if (qboost_enable == CW_TRUE)
	{
		//CWDebugLog("%s %d show only:[%d]", __FUNCTION__, __LINE__, CW_TRUE);
		return CW_TRUE;
	}

	//CWDebugLog("%s %d show only:[%d]", __FUNCTION__, __LINE__, CW_FALSE);
	return CW_FALSE;
}

int CWWTPBoardGetMaxRadioWlans(int radioIdx)
{
    int maxRadioWlans = 0;
#ifdef CW_DEF_AP_WIFI_IFACE_NUM
    maxRadioWlans = (CW_DEF_AP_WIFI_IFACE_NUM + 1); /* add 1 for guest network */
#else
    maxRadioWlans = (8 + 1); /* add 1 for guest network */
#endif
    //CWDebugLog("%s %d maxRadioWlans:[%d]", __FUNCTION__, __LINE__, maxRadioWlans);
    return maxRadioWlans;
}

int CWWTPBoardGetEncryptionCapabilities()
{
    return CW_ENC_CAP_TKIP_AES;
}

int CWWTPBoardGetMaxRadioWlansWepKeys(int radioIdx, int wlanIdx)
{
    return 4;
}

int CWWTPBoardGetRadioDisabled(int radioIdx)
{
    int disabled = 0;
    char *option = NULL;

    option = CWCreateString(WIRELESS_WIFIX_FORMAT, CWConvertRadioIdx(radioIdx), "disabled");

    if (api_get_wifi_disabled_option(option, &disabled))
    {
        disabled = 1;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    return disabled;
}

int CWWTPBoardGetRadioNoChannel(int radioIdx)
{
    int noChannel = 0;
    char *option = NULL;

    option = CWCreateString(WIRELESS_WIFIX_FORMAT, CWConvertRadioIdx(radioIdx), "nochannel");

    if (api_get_integer_option(option, &noChannel))
    {
        noChannel = 0;
    }

    CWDebugLog("%s %d radioIdx:[%d] noChannel:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), noChannel);
    if (option != NULL) CW_FREE_OBJECT(option);
    return noChannel;
}

int CWWTPBoardSetRadioNoChannel(int radioIdx, int noChannel)
{
    char *option = NULL;

    CWDebugLog("%s %d radioIdx:[%d] noChannel:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), noChannel);

    option = CWCreateString(WIRELESS_WIFIX_FORMAT, CWConvertRadioIdx(radioIdx), "nochannel");

    if (api_set_integer_option(option, noChannel))
    {
        if (option != NULL) CW_FREE_OBJECT(option);
        CWDebugLog("The nochannel value is invalid");
        return CW_TRUE;
    }
    
    if (option != NULL) CW_FREE_OBJECT(option);
    return CW_TRUE;
}

int CWWTPBoardGetRadioMode(int radioIdx)
{
    int opMode = 0;
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx), "opmode");

    if (api_get_wifi_opmode_option(option, &opMode))
    {
        opMode = -1;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    //CWDebugLog("%s %d RadioMode:[%d]", __FUNCTION__, __LINE__, opMode);
    return opMode;
}

CWBool CWWTPBoardGetWlanBssid(int radioIdx, int wlanIdx, CWMacAddress bssid)
{
    char *val, *c;
    unsigned int mac_tmp[6];

    if(!(val = CWCreateStringByCmd("ifconfig %s 2> /dev/null | grep HWaddr", WLAN_IF_NAME(CWConvertRadioIdx(radioIdx), wlanIdx))))
    {
        return CW_FALSE;
    }

    if(val[0] == '\0' || (c = strstr(val, "HWaddr ")) == NULL)
    {
        CW_ZERO_MEMORY(bssid, 6);
    }
    else
    {
        c += strlen("HWaddr ");

        if(sscanf(c, "%x:%x:%x:%x:%x:%x",
                  &mac_tmp[0], &mac_tmp[1], &mac_tmp[2], &mac_tmp[3], &mac_tmp[4], &mac_tmp[5]) != 6)
        {
            CW_FREE_OBJECT(val);
            CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
            return CW_TRUE;
        }

        bssid[0] = (unsigned char) mac_tmp[0];
        bssid[1] = (unsigned char) mac_tmp[1];
        bssid[2] = (unsigned char) mac_tmp[2];
        bssid[3] = (unsigned char) mac_tmp[3];
        bssid[4] = (unsigned char) mac_tmp[4];
        bssid[5] = (unsigned char) mac_tmp[5];
    }

    CW_FREE_OBJECT(val);

    CWDebugLog("%s %d bssid:[%02x:%02x:%02x:%02x:%02x:%02x]", __FUNCTION__, __LINE__, CW_MAC_PRINT_LIST(bssid));

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioMaxTxPower(int radioIdx, int *power)
{
    sysutil_get_wifix_maxTxPower(CWConvertRadioIdx(radioIdx), power);

    CWDebugLog("%s %d maxTxPower:[%d]", __FUNCTION__, __LINE__, *power);

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioOperationModeCfg(int radioIdx, int *mode)
{
    int disabled = 0, opMode = 0;

    disabled = CWWTPBoardGetRadioDisabled(radioIdx);
    opMode = CWWTPBoardGetRadioMode(radioIdx);

	if (CWWTPBoardConfigShowOnly() == CW_TRUE)
	{
		disabled = 1;
	}

    if (disabled == 0)
    {
        switch (opMode)
        {
            case SYS_OPM_AP:
                *mode = CW_RADIO_OPERATION_MODE_AP;
                break;
            case SYS_OPM_WDSAP:
            case SYS_OPM_CB:
            case SYS_OPM_WDSSTA:
			case SYS_OPM_WDSB:
				*mode = CW_RADIO_OPERATION_MODE_DISABLED;
                break;
            default:
                return CW_FALSE;
        }
    }
    else
    {
        *mode = CW_RADIO_OPERATION_MODE_DISABLED;
    }

    CWDebugLog("%s %d OperationMode:[%d]", __FUNCTION__, __LINE__, *mode);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioOperationModeCfg(int radioIdx, int mode)
{
    int disabled = 0, ErrorCode = CW_ERROR_NONE;
    char *disabled_option = NULL, *opmode_option = NULL;

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), mode);

    switch (mode)
    {
        case CW_RADIO_OPERATION_MODE_DISABLED:
            disabled = 1;
            disabled_option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx), "disabled");
            if (api_set_wifi_disabled_option(disabled_option, disabled))
            {
                ErrorCode = CW_ERROR_WRONG_ARG;
            }
            break;
        case CW_RADIO_OPERATION_MODE_AP:
            disabled = 0;
            disabled_option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx), "disabled");
            if (api_set_wifi_disabled_option(disabled_option, disabled))
            {
                ErrorCode = CW_ERROR_WRONG_ARG;
            }
            opmode_option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx), "opmode");
            if (api_set_wifi_opmode_option(opmode_option, SYS_OPM_AP))
            {
                ErrorCode = CW_ERROR_WRONG_ARG;
            }
            break;
        case CW_RADIO_OPERATION_MODE_AD_HOC:
            CWDebugLog("Do not support AD HOC");
            return CW_TRUE;
        default:
            CWDebugLog("Bad Radio Operation Mode");
            return CW_TRUE;

    }

    CW_FREE_OBJECT(disabled_option);
    CW_FREE_OBJECT(opmode_option);

    if (ErrorCode != CW_ERROR_NONE) 
    {
        CWDebugLog("Bad Radio Operation Mode");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioType(int radioIdx, CWRadioType *type)
{
    CWRadioType mode = 0;

    switch (CWConvertRadioIdx(radioIdx))
    {
#if SUPPORT_WLAN_24G_SETTING
    case 0:
        mode = (CW_802_DOT_11b | CW_802_DOT_11g | CW_802_DOT_11n);
#if HWMODE_AX
        mode = (mode | CW_802_DOT_11ax);
#endif
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case 1:
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
#endif
        mode = (CW_802_DOT_11a | CW_802_DOT_11n);
#if HWMODE_AC
        mode = (mode | CW_802_DOT_11ac);
#endif
#if HWMODE_AX
        mode = (mode | CW_802_DOT_11ax);
#endif
        break;
#endif
    default:
        mode = 0;
        break;
    }
    *type = mode;
    CWDebugLog("%s %d RadioType:[%d]", __FUNCTION__, __LINE__, *type);

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioMac(int radioIdx, CWMacAddress mac)
{
    char mac_buf[17+1];
    unsigned int mac_tmp[6];

    memset(mac_buf, 0, sizeof(mac_buf));
    memset(mac_tmp, 0, sizeof(mac_tmp));

    sysutil_get_wifixMacAddr(CWConvertRadioIdx(radioIdx), mac_buf, sizeof(mac_buf));

    if(sscanf(mac_buf, "%x:%x:%x:%x:%x:%x", &mac_tmp[0], &mac_tmp[1], &mac_tmp[2], &mac_tmp[3], &mac_tmp[4], &mac_tmp[5]) != 6)
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    mac[0] = (unsigned char) mac_tmp[0];
    mac[1] = (unsigned char) mac_tmp[1];
    mac[2] = (unsigned char) mac_tmp[2];
    mac[3] = (unsigned char) mac_tmp[3];
    mac[4] = (unsigned char) mac_tmp[4];
    mac[5] = (unsigned char) mac_tmp[5];

    CWDebugLog("%s radio %u %02x:%02x:%02x:%02x:%02x:%02x", __FUNCTION__, CWConvertRadioIdx(radioIdx), CW_MAC_PRINT_LIST(mac));

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioCountryCodeCfg(int radioIdx, int *country)
{
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"country");

    if (api_get_integer_option(option, country))
    {
        *country = 0;
    }

#if SUPPORT_JAPAN_BANDWIDTH_HT80
    *country = (*country == 4015)?392:*country;
#endif

    if (option != NULL) CW_FREE_OBJECT(option);
    CWDebugLog("%s %d radioIdx:[%d] country:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), *country);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioCountryCodeCfg(int radioIdx, int country)
{
    int countryIdx = 0;
    char *option = NULL;

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), country);

#if SUPPORT_JAPAN_BANDWIDTH_HT80
    country = (country == 392)?4015:country;
#endif

    if (!api_get_wifi_country_table_index(country, &countryIdx))
    {
        option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"country");
        if (api_set_wifi_country_option(option, countryIdx))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            CWDebugLog("Bad Country Index: %d", countryIdx);
            return CW_TRUE;
        }
    }
    else
    {
        CWDebugLog("Bad Country Code: %d", country);
        return CW_TRUE;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

#if SUPPORT_WLAN_5G_SETTING
#if SUPPORT_WLAN_5G_2_SETTING
    if (((country == 376) || (country == 414) || (country == 788) || (country == 818) || (country == 829)) && (CWConvertRadioIdx(radioIdx) == 1))
    {
        CWWTPBoardSetRadioNoChannel(radioIdx, 1);
        CWWTPBoardSetRadioNoChannel(2, 0);
    }
    else if (((country == 360) || (country == 586) || (country == 634) || (country == 830)) && (CWConvertRadioIdx(radioIdx) == 2))
    {
        CWWTPBoardSetRadioNoChannel(radioIdx, 1);
        CWWTPBoardSetRadioNoChannel(1, 0);
    }
    else
    {
        CWWTPBoardSetRadioNoChannel(1, 0);
        CWWTPBoardSetRadioNoChannel(2, 0);
    }
#endif
#endif
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioChannelHTModeCfg(int radioIdx, int *mode)
{
    int val = 0;
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"htmode");
    switch (CWConvertRadioIdx(radioIdx))
    {
    case 0:
        if (api_get_wifi_htmode_option(option, &val))
        {
            val = BANDWIDTH_20MHZ;
        }
        break;
#if SUPPORT_WLAN_5G_SETTING
    case 1:
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
#endif
        if (api_get_wifi_5g_htmode_option(option, &val))
        {
            val = BANDWIDTH_20MHZ;
        }
        break;
#endif
    default:
        if (option != NULL) CW_FREE_OBJECT(option);
        CWDebugLog("Invalid Radio Index");
        return CW_TRUE;
    }

    switch (val)
    {
    case BANDWIDTH_20MHZ:
        *mode = CW_RADIO_CHANNEL_HT_20_MHZ;
        break;
    case BANDWIDTH_40MHZ:
        *mode = CW_RADIO_CHANNEL_HT_40_MHZ;
        break;
    case BANDWIDTH_20MHZ_40MHZ:
        *mode = CW_RADIO_CHANNEL_HT_20_40_MHZ;
        break;
#if SUPPORT_WLAN_5G_SETTING
    case BANDWIDTH_80MHZ:
        *mode = CW_RADIO_CHANNEL_HT_80_MHZ;
        break;
#endif
    default:
        if (option != NULL) CW_FREE_OBJECT(option);
        CWDebugLog("Bad Channel HT Mode");
        return CW_TRUE;
    }

    if (option != NULL) CW_FREE_OBJECT(option);
    CWDebugLog("%s %d %d HTMode:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), *mode);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioChannelHTModeCfg(int radioIdx, int mode)
{
    int htmode = 0, ErrorCode = CW_ERROR_NONE;
    char *option = NULL;

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), mode);

    switch(mode)
    {
    case CW_RADIO_CHANNEL_HT_20_MHZ:
        htmode = BANDWIDTH_20MHZ;
        break;
    case CW_RADIO_CHANNEL_HT_20_40_MHZ:
        htmode = BANDWIDTH_20MHZ_40MHZ;
        break;
    case CW_RADIO_CHANNEL_HT_40_MHZ:
        htmode = BANDWIDTH_40MHZ;
        break;
#if SUPPORT_WLAN_5G_SETTING
    case CW_RADIO_CHANNEL_HT_80_MHZ:
        htmode = BANDWIDTH_80MHZ;
        break;
#endif
    default:
        CWDebugLog("Bad Channel HT Mode");
        return CW_TRUE;
    }

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"htmode");
    switch (CWConvertRadioIdx(radioIdx))
    {
    case 0:
        if (api_set_wifi_htmode_option(option, htmode))
        {
            ErrorCode = CW_ERROR_WRONG_ARG;
        }
        break;
#if SUPPORT_WLAN_5G_SETTING
    case 1:
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
#endif
        if (api_set_wifi_5g_htmode_option(option, htmode))
        {
            ErrorCode = CW_ERROR_WRONG_ARG;
        }
        break;
#endif
    default:
        ErrorCode = CW_ERROR_OUT_OF_INDEX;
        break;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    if (ErrorCode == CW_ERROR_WRONG_ARG)
    {
        CWDebugLog("Bad Channel HT Mode");
    }
    else if (ErrorCode == CW_ERROR_OUT_OF_INDEX) 
    {
        CWDebugLog("Invalid Radio Index");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioChannelExtCfg(int radioIdx, int *mode)
{
    int val = 0;
    char *option = NULL;

#if SUPPORT_WLAN_EXTENSION_CHANNEL
    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"ext_channel");
    if (api_get_wifi_extension_channel_option(option, &val))
    {
        val = 0;
    }
#else
    val = 0;
#endif

    switch(val)
    {
    case 1:
        *mode = CW_RADIO_EXTENSION_CHANNEL_LOWER;
        break;
    default:
        *mode = CW_RADIO_EXTENSION_CHANNEL_UPPER;
        break;
    }

    if (option != NULL) CW_FREE_OBJECT(option);
    CWDebugLog("%s %d %d ChannelExt:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), *mode);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioChannelExtCfg(int radioIdx, int mode)
{
#if SUPPORT_WLAN_EXTENSION_CHANNEL
    char *option = NULL;
#endif

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), mode);

#if SUPPORT_WLAN_EXTENSION_CHANNEL
    if ( mode > CW_RADIO_EXTENSION_CHANNEL_LOWER || mode < CW_RADIO_EXTENSION_CHANNEL_UPPER)
    {
        CWDebugLog("Bad Channel Ext");
        return CW_TRUE;
    }

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"ext_channel");
    if (api_set_wifi_extension_channel_option(option, mode))
    {
        if (option != NULL) CW_FREE_OBJECT(option);
        CWDebugLog("Bad Extension Channel");
        return CW_TRUE;
    }

    if (option != NULL) CW_FREE_OBJECT(option);
#endif

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioChannelCfg(int radioIdx, int *mode)
{
    int val = 0;
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"channel");
    switch (CWConvertRadioIdx(radioIdx))
    {
    case 0:
        if (api_get_wifi_channel_option(option, &val))
        {
            val = 0;
        }
        break;
#if SUPPORT_WLAN_5G_SETTING
    case 1:
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
#endif
        if (api_get_wifi_5g_channel_option(option, &val))
        {
            val = 0;
        }
        break;
#endif
    default:
        if (option != NULL) CW_FREE_OBJECT(option);
        CWDebugLog("Invalid Radio Index");
        return CW_TRUE;
    }

    *mode = (val == 0) ? CW_RADIO_CHANNEL_AUTO : val;

    if (option != NULL) CW_FREE_OBJECT(option); 
    CWDebugLog("%s %d RadioChannel:[%d]", __FUNCTION__, __LINE__, *mode);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioChannelCfg(int radioIdx, int mode)
{
    int channel = 0, ErrorCode = CW_ERROR_NONE;
    char *option = NULL;

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), mode);
#if 0 //SUPPORT_WLAN_5G_2_SETTING
    if ((radioIdx == 1) && (mode < 100))
    {
        radioIdx = 2;
    }
#endif
    if (CWWTPBoardGetRadioNoChannel(radioIdx) == CW_TRUE)
    {
        CWDebugLog("Invalid Radio Index");
        return CW_TRUE;
    }

    channel = (mode == CW_RADIO_CHANNEL_AUTO) ? 0 : mode;

#if 0 //SUPPORT_MESH_SETTING && SUPPORT_WLAN_5G_2_SETTING
    if ((radioIdx != 0))
        api_set_integer_option("mesh.wifi.5Gchannel", mode);
#endif

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"channel");

    switch (CWConvertRadioIdx(radioIdx))
    {
#if SUPPORT_WLAN_24G_SETTING
    case 0:
        if (api_set_wifi_channel_option(option, channel))
        {
            ErrorCode = CW_ERROR_WRONG_ARG;
        }
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case 1:
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
#endif
        if (api_set_wifi_5g_channel_option(option, channel))
        {
            ErrorCode = CW_ERROR_WRONG_ARG;
        }
        break;
#endif
    default:
        ErrorCode = CW_ERROR_OUT_OF_INDEX;
        break;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    if (ErrorCode == CW_ERROR_WRONG_ARG) 
    {
        CWDebugLog("Bad Radio Channel");
    }
    else if (ErrorCode == CW_ERROR_OUT_OF_INDEX)
    {
        CWDebugLog("Invalid Radio Index");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioWirelessModeCfg(int radioIdx, int *mode)
{
    int val = 0, ErrorCode = CW_ERROR_NONE;
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"hwmode");

    switch (CWConvertRadioIdx(radioIdx))
    {
#if SUPPORT_WLAN_24G_SETTING
    case 0:
        if (api_get_wifi_hwmode_option(option, &val))
        {
            val = 0;
        }
        switch (val)
        {
        case P24G_IEEE802_11B:
            *mode = CW_RADIO_WIRELESS_MODE_B;
            break;
        case P24G_IEEE802_11G:
            *mode = CW_RADIO_WIRELESS_MODE_G;
            break;
        case P24G_IEEE802_11BG:
            *mode = CW_RADIO_WIRELESS_MODE_BG;
            break;
        case P24G_IEEE802_11N:
            *mode = CW_RADIO_WIRELESS_MODE_N_2G;
            break;
        case P24G_IEEE802_11NG:
            *mode = CW_RADIO_WIRELESS_MODE_BGN;
            break;
#if HWMODE_AX
        case P24G_IEEE802_11AX:
            *mode = CW_RADIO_WIRELESS_MODE_AX;
            break;
#endif
        default:
            *mode = CW_RADIO_WIRELESS_MODE_B;
            ErrorCode = CW_ERROR_WRONG_ARG;
            break;
        }
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case 1:
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
#endif
        if (api_get_wifi_5g_hwmode_option(option, &val))
        {
            val = 0;
        }
        switch (val)
        {
        case P5G_IEEE802_11A:
            *mode = CW_RADIO_WIRELESS_MODE_A;
            break;
        case P5G_IEEE802_11N:
            *mode = CW_RADIO_WIRELESS_MODE_N_5G;
            break;
        case P5G_IEEE802_11NA:
            *mode = CW_RADIO_WIRELESS_MODE_AN;
            break;
        case P5G_IEEE802_11AC:
            *mode = CW_RADIO_WIRELESS_MODE_ACN_5G;
            break;
#if HWMODE_AX
        case P5G_IEEE802_11AX:
            *mode = CW_RADIO_WIRELESS_MODE_AX;
            break;
#endif
        default:
            *mode = CW_RADIO_WIRELESS_MODE_A;
            ErrorCode = CW_ERROR_WRONG_ARG;
            break;
        }
        break;
#endif
    default:
        ErrorCode = CW_ERROR_OUT_OF_INDEX;
        break;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    if (ErrorCode == CW_ERROR_WRONG_ARG) 
    {
        CWDebugLog("Bad Radio Mode");
        return CW_TRUE;
    }
    else if (ErrorCode == CW_ERROR_OUT_OF_INDEX)
    {
        CWDebugLog("Invalid Radio Index");
        return CW_TRUE;
    }

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), *mode);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioWirelessModeCfg(int radioIdx, int mode)
{
    int hwmode = 0, ErrorCode = CW_ERROR_NONE;
    char *option = NULL;

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), mode);

    switch(mode)
    {
        case CW_RADIO_WIRELESS_MODE_B:
            hwmode = P24G_IEEE802_11B;
            break;
        case CW_RADIO_WIRELESS_MODE_G:
            hwmode = P24G_IEEE802_11G;
            break;
        case CW_RADIO_WIRELESS_MODE_N_2G:
            hwmode = P24G_IEEE802_11N;
            break;
        case CW_RADIO_WIRELESS_MODE_BGN:
            hwmode = P24G_IEEE802_11NG;
            break;
        case CW_RADIO_WIRELESS_MODE_BG:
            hwmode = P24G_IEEE802_11BG;
            break;
#if SUPPORT_WLAN_5G_SETTING
        case CW_RADIO_WIRELESS_MODE_A:
            hwmode = P5G_IEEE802_11A;
            break;
        case CW_RADIO_WIRELESS_MODE_N_5G:
            hwmode = P5G_IEEE802_11N;
            break;
        case CW_RADIO_WIRELESS_MODE_AN:
            hwmode = P5G_IEEE802_11NA;
            break;
        case CW_RADIO_WIRELESS_MODE_ACN_5G:
            hwmode = P5G_IEEE802_11AC;
            break;
#if HWMODE_AX
        case CW_RADIO_WIRELESS_MODE_AX:
            if (radioIdx)
                hwmode = P5G_IEEE802_11AX;
            else
                hwmode = P24G_IEEE802_11AX;
            break;
#endif
#endif
        default:
            CWDebugLog("Bad Wireless Mode %d for radio %d", mode, CWConvertRadioIdx(radioIdx));
	    return CW_TRUE;
    }

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"hwmode");

    switch (CWConvertRadioIdx(radioIdx))
    {
#if SUPPORT_WLAN_24G_SETTING
    case 0:
        if (api_set_wifi_hwmode_option(option, hwmode))
        {
            ErrorCode = CW_ERROR_WRONG_ARG;
        }
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case 1:
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
#endif
        if (api_set_wifi_5g_hwmode_option(option, hwmode))
        {
            ErrorCode = CW_ERROR_WRONG_ARG;
        }
        break;
#endif
    default:
        ErrorCode = CW_ERROR_OUT_OF_INDEX;
        break;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    if (ErrorCode == CW_ERROR_WRONG_ARG) 
    {
        CWDebugLog("Bad Wireless Mode");
    }
    else if (ErrorCode == CW_ERROR_OUT_OF_INDEX)
    {
        CWDebugLog("Invalid Radio Index");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioLimitedClientsEnableCfg(int radioIdx, int *enable)
{
    int val = 0;
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"clientlimits_enable");
    if (api_get_bool_option(option, &val))
    {
        val = 0;
    }
    *enable = (val) ? CW_TRUE : CW_FALSE;
    if (option != NULL) CW_FREE_OBJECT(option);
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioLimitedClientsEnableCfg(int radioIdx, int enable)
{
    char *option = NULL;

    CWDebugLog("%s %d LimitedClientsEnable:[%d]", __FUNCTION__, CWConvertRadioIdx(radioIdx), enable);
    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"clientlimits_enable");
    if (api_set_bool_option(option, enable))
    {
        if (option != NULL) CW_FREE_OBJECT(option);
         CWDebugLog("Bad Limited Clients Enable");
         return CW_TRUE;
    }
    if (option != NULL) CW_FREE_OBJECT(option);
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioLimitedClientsCfg(int radioIdx, int *clients)
{
    int val = 0;
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"clientlimits_number");
    if (api_get_integer_option(option, &val))
    {
        val = 0;
    }
    *clients = val;
    if (option != NULL) CW_FREE_OBJECT(option);
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *clients);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioLimitedClientsCfg(int radioIdx, int clients)
{
    char *option = NULL;

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), clients);
    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"clientlimits_number");
    if (api_set_integer_option(option, clients))
    {
        if (option != NULL) CW_FREE_OBJECT(option);
        CWDebugLog("Bad Limited Clients Numbers");
        return CW_TRUE;
    }
    if (option != NULL) CW_FREE_OBJECT(option);
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioTxPowerCfg(int radioIdx, int *power)
{
    int val = 0;
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"txpower");

    switch (CWConvertRadioIdx(radioIdx))
    {
#if SUPPORT_WLAN_24G_SETTING
    case 0:
        if (api_get_wifi_txpower_option(option, &val))
        {
            val = 0;
        }
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case 1:
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
#endif
        if (api_get_wifi_5g_txpower_option(option, &val))
        {
            val = 0;
        }
        break;
#endif
    default:
        if (option != NULL) CW_FREE_OBJECT(option);
        CWDebugLog("Invalid Radio Index");
        return CW_TRUE;
    }
    *power = val;
    if (option != NULL) CW_FREE_OBJECT(option);
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *power);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioTxPowerCfg(int radioIdx, int power)
{
    int ErrorCode = CW_ERROR_NONE;
    char *option = NULL;

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), power);

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"txpower");

    switch (CWConvertRadioIdx(radioIdx))
    {
#if SUPPORT_WLAN_24G_SETTING
    case 0:
        if (api_set_wifi_txpower_option(option, power))
        {
            ErrorCode = CW_ERROR_WRONG_ARG;
        }
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case 1:
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
#endif
        if (api_set_wifi_5g_txpower_option(option, power))
        {
            ErrorCode = CW_ERROR_WRONG_ARG;
        }
        break;
#endif
    default:
        ErrorCode = CW_ERROR_OUT_OF_INDEX;
        break;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    if (ErrorCode == CW_ERROR_WRONG_ARG) 
    {
        CWDebugLog("Bad Tx Power");
    }
    else if (ErrorCode == CW_ERROR_OUT_OF_INDEX)
    {
        CWDebugLog("Invalid Radio Index");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioAggregationEnableCfg(int radioIdx, int *enable)
{
    int val = 0;
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"aggregation_enable");

    if (api_get_wifi_aggr_enabled_option(option, &val))
    {
        val = 0;
    }

    *enable = val;
    if (option != NULL) CW_FREE_OBJECT(option);
    CWDebugLog("%s %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioAggregationEnableCfg(int radioIdx, int enable)
{
    char *option = NULL;

    CWDebugLog("%s radioIdx:[%d] enable:[%d]", __FUNCTION__, CWConvertRadioIdx(radioIdx), enable);

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"aggregation_enable");

    if (api_set_wifi_aggr_enabled_option(option, enable))
    {
        if (option != NULL) CW_FREE_OBJECT(option);
        CWDebugLog("Bad Aggregation Enable");
        return CW_TRUE;
    }

    if (option != NULL) CW_FREE_OBJECT(option);
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioAggregationFramesCfg(int radioIdx, int *frames)
{
    int val = 0;
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"aggregation_frame");

    if (api_get_wifi_aggr_frame_option(option, &val))
    {
        val = 0;
    }

    *frames = val;
    if (option != NULL) CW_FREE_OBJECT(option);
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *frames);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioAggregationFramesCfg(int radioIdx, int frames)
{
    char *option = NULL;

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), frames);

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"aggregation_frame");

    if (api_set_wifi_aggr_frame_option(option, frames))
    {
        if (option != NULL) CW_FREE_OBJECT(option);
        CWDebugLog("Bad Aggregation Frames");
        return CW_TRUE;
    }

    if (option != NULL) CW_FREE_OBJECT(option);
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioAggregationMaxBytesCfg(int radioIdx, int *maxBytes)
{
    int val = 0;
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"aggregation_byte");

    if (api_get_wifi_aggr_byte_option(option, &val))
    {
        val = 0;
    }

    *maxBytes = val;
    if (option != NULL) CW_FREE_OBJECT(option);
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *maxBytes);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioAggregationMaxBytesCfg(int radioIdx, int maxBytes)
{
    char *option = NULL;

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), maxBytes);

    if (maxBytes == 0)
    {
        CWDebugLog("Aggregation maxBytes can't be 0");
        return CW_TRUE;
    }

#if HWMODE_AC || HWMODE_AX
    if (CWConvertRadioIdx(radioIdx) && (maxBytes!=65535))
    {
        CWDebugLog("Aggregation Max Bytes can't be changed on 11ac mode");
        return CW_TRUE;
    }
#endif

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"aggregation_byte");

    if (api_set_wifi_aggr_byte_option(option, maxBytes))
    {
        if (option != NULL) CW_FREE_OBJECT(option);
        CWDebugLog("Bad Aggregation Max Bytes");
        return CW_TRUE;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioWirelessModeAXEnableCfg(int radioIdx, int *enable)
{
    int val = 0;
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"hwmode");

    // only support 2.4g
    if ( CWConvertRadioIdx(radioIdx) != 0 )
    {
        CWDebugLog("Invalid Radio Index");
        *enable = CW_TRUE;
        return CW_TRUE;
    }

    if (api_get_wifi_hwmode_option(option, &val))
    {
        val = 0;
    }

    switch (val)
    {
#if HWMODE_AX
        case P24G_IEEE802_11AX:
            *enable = CW_TRUE;
            break;
#endif
        default:
            *enable = CW_FALSE;
            break;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioWirelessModeAXEnableCfg(int radioIdx, int enable)
{
    int hwmode = 0, ErrorCode = CW_ERROR_NONE;
    char *option = NULL;

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), enable);

    // only support 2.4g
    if ( CWConvertRadioIdx(radioIdx) != 0 )
    {
        CWDebugLog("Invalid Radio Index");
        return CW_TRUE;
    }

#if HWMODE_AX
    if ( enable )
    {
        hwmode = P24G_IEEE802_11AX;
    }
    else
#endif
    {
        hwmode = P24G_IEEE802_11NG;
    }

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"hwmode");

    if (api_set_wifi_hwmode_option(option, hwmode))
    {
        ErrorCode = CW_ERROR_WRONG_ARG;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    if (ErrorCode == CW_ERROR_WRONG_ARG)
    {
        CWDebugLog("Bad Wireless Mode [%d]", hwmode);
    }
    else if (ErrorCode == CW_ERROR_OUT_OF_INDEX)
    {
        CWDebugLog("Invalid Radio Index");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int val = 1;
    char *option = NULL;
    char *section = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"disabled");
        if (api_get_wifi_guest_disabled_option(option, &val))
        {
            val = 1;
        }
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_disabled_option_by_sectionname(OPM_AP, section, wlanIdx+1, &val))
        {
            val = 1;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    switch(val)
    {
    case 0:
        *enable = CW_TRUE;
        break;
    default:
        *enable = CW_FALSE;
        break;
    }

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    char *option = NULL;
    char *section = NULL;

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"disabled");
        if (api_set_wifi_guest_disabled_option(option, !enable))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            CWDebugLog("Bad Guest Wlan Enable");
            return CW_TRUE;
        }
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_disabled_option_by_sectionname(OPM_AP, section, wlanIdx+1, !enable))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            CWDebugLog("Bad Wlan Enable");
            return CW_TRUE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanSsidCfg(int radioIdx, int wlanIdx, char **pstr)
{
    char *ssid = NULL;
    char *option = NULL;
    char *section = NULL;

    CW_CREATE_STRING_ERR(ssid, MAX_SSID_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"ssid");
        CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] option:[%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, option);
        if (api_get_wifi_guest_ssid_option(option, ssid, MAX_SSID_SIZE))
        {
            snprintf(ssid,MAX_SSID_SIZE,"%s","");
        }
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] section:[%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, section);
        if (api_get_wifi_ifacex_ssid_option_by_sectionname(OPM_AP, section, wlanIdx+1, ssid, MAX_SSID_SIZE))
        {
            snprintf(ssid,MAX_SSID_SIZE,"%s","");
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    *pstr = ssid;

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] SSID:[%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanSsidCfg(int radioIdx, int wlanIdx, char *pstr)
{
    char *option = NULL;
    char *section = NULL;

    CWDebugLog("%s %d %d %s", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"ssid");
        if (api_set_wifi_guest_ssid_option(option, pstr, MAX_SSID_SIZE))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            CWDebugLog("Bad Wlan SSID");
            return CW_TRUE;
        }
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_ssid_option_by_sectionname(OPM_AP, section, wlanIdx+1, pstr, MAX_SSID_SIZE))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            CWDebugLog("Bad Wlan SSID");
            return CW_TRUE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanSuppressedSsidCfg(int radioIdx, int wlanIdx, int *enable)
{
    int val = 0;
    char *option = NULL;
    char *section = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"hidden");
        if (api_get_wifi_guest_hidden_option(option, &val))
        {
            val = 0;
        }
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_hidden_option_by_sectionname(OPM_AP, section, wlanIdx+1, &val))
        {
            val = 0;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    switch(val)
    {
        case 1:
            *enable = CW_TRUE;
            break;
        default:
            *enable = CW_FALSE;
            break;
    }

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanSuppressedSsidCfg(int radioIdx, int wlanIdx, int enable)
{
    char *option = NULL;
    char *section = NULL;

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"hidden");
        if (api_set_wifi_guest_hidden_option(option, enable))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            CWDebugLog("Bad Guest Wlan Hidden");
            return CW_TRUE;
        }
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_hidden_option_by_sectionname(OPM_AP, section, wlanIdx+1, enable))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            CWDebugLog("Bad Wlan Hidden");
            return CW_TRUE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanStationSeparationCfg(int radioIdx, int wlanIdx, int *enable)
{
    int val = 0;
    char *option = NULL;
    char *section = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"isolate");
        if (api_get_wifi_guest_isolate_option(option, &val))
        {
            val = 0;
        }
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_isolate_option_by_sectionname(OPM_AP, section, wlanIdx+1, &val))
        {
            val = 0;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    switch(val)
    {
    case 1:
        *enable = CW_TRUE;
        break;
    default:
        *enable = CW_FALSE;
        break;
    }

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanStationSeparationCfg(int radioIdx, int wlanIdx, int enable)
{
    char *option = NULL;
    char *section = NULL;

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"isolate");
        if (api_set_wifi_guest_isolate_option(option, enable))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            CWDebugLog("Bad Guest Wlan Client Separation");
            return CW_TRUE;
        }
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_isolate_option_by_sectionname(OPM_AP, section, wlanIdx+1, enable))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            CWDebugLog("Bad Wlan Client Separation");
            return CW_TRUE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanLayer2IsolationCfg(int radioIdx, int wlanIdx, int *enable)
{
    int val = 0;
    char *section = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        val = 0;
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_l2_isolation_option_by_sectionname(OPM_AP, section, wlanIdx+1, &val))
        {
            val = 0;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    switch(val)
    {
    case 1:
        *enable = CW_TRUE;
        break;
    default:
        *enable = CW_FALSE;
        break;
    }

    CWDebugLog("%s %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanLayer2IsolationCfg(int radioIdx, int wlanIdx, int enable)
{
    char *section = NULL;

    CWDebugLog("%s %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        CWDebugLog("Guest wlan cannot support L2 Isolation");
        return CW_TRUE;
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_l2_isolation_option_by_sectionname(OPM_AP, section, wlanIdx+1, enable))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            CWDebugLog("Bad wlan L2 Isolation");
            return CW_TRUE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanIsolationCfg(int radioIdx, int wlanIdx, int *enable)
{
    int val = 0;
    char *section = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        val = 0;
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_isolation_option_by_sectionname(OPM_AP, section, wlanIdx+1, &val))
        {
            val = 0;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    switch(val)
    {
    case 1:
        *enable = CW_TRUE;
        break;
    default:
        *enable = CW_FALSE;
        break;
    }

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanIsolationCfg(int radioIdx, int wlanIdx, int enable)
{
    char *section = NULL; 

    CWDebugLog("%s %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(enable)
        {
            CWDebugLog("Guest wlan cannot support Isolation");
        }
        return CW_TRUE;
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_isolation_option_by_sectionname(OPM_AP, section, wlanIdx+1, enable))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            CWDebugLog("Bad wlan Isolation");
            return CW_TRUE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanVlanCfg(int radioIdx, int wlanIdx, int *vlan)
{
    char *section = NULL;

    if (GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)) == wlanIdx)
    {
        *vlan = 1;
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_vlan_id_option_by_sectionname(OPM_AP, section, wlanIdx+1, vlan))
        {
            *vlan = 1;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] vlan:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *vlan);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanVlanCfg(int radioIdx, int wlanIdx, int vlan)
{
    char *section = NULL;

    CWDebugLog("%s %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, vlan);

    if(vlan <= 0 || vlan > 4094)
    {
        CWDebugLog("Invalid VLAN ID");
        return CW_TRUE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(vlan != 1)
        {
            CWDebugLog("Guest wlan cannot change VLAN ID");
            return CW_TRUE;
        }
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
    if (api_set_wifi_ifacex_vlan_id_option_by_sectionname(OPM_AP, section, wlanIdx+1, vlan))
    {
        if (section != NULL) CW_FREE_OBJECT(section);
        CWDebugLog("Bad wlan VLAN ID");
        return CW_TRUE;
    }
    if (section != NULL) CW_FREE_OBJECT(section);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanSecurityCfg(int radioIdx, int wlanIdx, int *mode)
{
    int encryption = 0;
    char *option = NULL;
    char *section = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"encryption");
        if (api_get_wifi_guest_encryption_option(option, &encryption))
        {
            encryption = 0;
        }
        if (option != NULL) CW_FREE_OBJECT(option);
        if(encryption >= 1 && encryption <= 3)
        {
            *mode = CW_WLAN_SECURITY_MODE_WPA_PSK;
        }
        else if(encryption >= 4 && encryption <= 6)
        {
            *mode = CW_WLAN_SECURITY_MODE_WPA2_PSK;
        }
        else if(encryption >= 7 && encryption <= 9)
        {
            *mode = CW_WLAN_SECURITY_MODE_WPA_PSK_MIXED;
        }
        else
        {
            *mode = CW_WLAN_SECURITY_MODE_NONE;
        }
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_encryption_option_by_sectionname(OPM_AP, section, wlanIdx+1, &encryption))
        {
            encryption = 0;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
        if(encryption >= 1 && encryption <= 2)
        {
            *mode = CW_WLAN_SECURITY_MODE_WEP;
        }
        else if(encryption >= 3 && encryption <= 5)
        {
            *mode = CW_WLAN_SECURITY_MODE_WPA_PSK;
        }
        else if(encryption >= 6 && encryption <= 8)
        {
            *mode = CW_WLAN_SECURITY_MODE_WPA2_PSK;
        }
        else if(encryption >= 9 && encryption <= 11)
        {
            *mode = CW_WLAN_SECURITY_MODE_WPA_PSK_MIXED;
        }
        else if(encryption >= 12 && encryption <= 14)
        {
            *mode = CW_WLAN_SECURITY_MODE_WPA;
        }
        else if(encryption >= 15 && encryption <= 17)
        {
            *mode = CW_WLAN_SECURITY_MODE_WPA2;
        }
        else if(encryption >= 18 && encryption <= 20)
        {
            *mode = CW_WLAN_SECURITY_MODE_WPA_MIXED;
        }
#if SUPPORT_WPA3
        else if(encryption == ENCRYPTION_OWE_CCMP)
        {
            *mode = CW_WLAN_SECURITY_MODE_OWE;
        }
        else if(encryption == WPA3_SAE_CCMP)
        {
            *mode = CW_WLAN_SECURITY_MODE_WPA3_PERSONAL;
        }
        else if(encryption == WPA3_SAE_MIXED_CCMP)
        {
            *mode = CW_WLAN_SECURITY_MODE_WPA2_WPA3_PERSONAL_MIXED;
        }
        else if(encryption == WPA3_EAP_CCMP)
        {
            *mode = CW_WLAN_SECURITY_MODE_WPA3;
        }
        else if(encryption == WPA3_EAP_MIXED_CCMP)
        {
            *mode = CW_WLAN_SECURITY_MODE_WPA3_MIXED;
        }
#endif
        else
        {
            *mode = CW_WLAN_SECURITY_MODE_NONE;
        }
    }
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *mode);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanSecurityNONECfg(int radioIdx, int wlanIdx)
{
    char *option = NULL;
    char *section = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"encryption");
        if (api_set_wifi_guest_encryption_option(option, GUEST_ENCRYPTION_NONE))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            return CW_FALSE;
        }
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_encryption_option_by_sectionname(OPM_AP, section, wlanIdx+1, ENCRYPTION_NONE))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanSecurityWEPCfg(int radioIdx, int wlanIdx, int auth)
{
    int sec_mode = 0, keyid = 0, auth_type = 0;
    char *section = NULL;

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] auth:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, auth);

    if(wlanIdx != 0)
    {
        CWDebugLog("Only WLAN0 can support WEP Security");
        return CW_TRUE;
    }

    sec_mode = (auth==WEP_AUTH_SHARED)?WEP_SHARED:WEP_OPEN;

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] sec_mode:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, sec_mode);

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
    if (api_set_wifi_ifacex_encryption_option_by_sectionname(OPM_AP, section, wlanIdx+1, sec_mode))
    {
        if (section != NULL) CW_FREE_OBJECT(section);
        return CW_FALSE;
    }
    /* Check WEP variables are present in UCI, if not, set to default value */
    /* Wep Auth */
    if (api_get_wifi_ifacex_wep_auth_option_by_sectionname(OPM_AP, section, wlanIdx+1, &auth_type))
    {
        if (api_set_wifi_ifacex_wep_auth_option_by_sectionname(OPM_AP, section, wlanIdx+1, auth))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
    }
    /* WepKeyIdx */
    if (api_get_wifi_ifacex_wepkey_id_option_by_sectionname(OPM_AP, section, wlanIdx+1, &keyid))
    {
        if (api_set_wifi_ifacex_wepkey_id_option_by_sectionname(OPM_AP, section, wlanIdx+1, 1))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
    }
    if (section != NULL) CW_FREE_OBJECT(section);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanSecurityWPACfg(int radioIdx, int wlanIdx, int mode, int type)
{
    int sec_mode = 0;
    char *encryption_option = NULL;
    char *wpa_enc_option = NULL;
    char *section = NULL;

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] mode:[%d] type:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, mode, type);

    switch(mode)
    {
    case CW_WLAN_SECURITY_MODE_WPA_PSK:
        if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
        {
            sec_mode = (type==WPA_ENC_CCMP)?GUEST_WPA_PSK_CCMP:(type==WPA_ENC_TKIP)?GUEST_WPA_PSK_TKIP:GUEST_WPA_PSK_TKIP_CCMP;
        }
        else
        {
            sec_mode = (type==WPA_ENC_CCMP)?WPA_PSK_CCMP:(type==WPA_ENC_TKIP)?WPA_PSK_TKIP:WPA_PSK_TKIP_CCMP;
        }
        break;
    case CW_WLAN_SECURITY_MODE_WPA2_PSK:
        if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
        {
            sec_mode = (type==WPA_ENC_CCMP)?GUEST_WPA2_PSK_CCMP:(type==WPA_ENC_TKIP)?GUEST_WPA2_PSK_TKIP:GUEST_WPA2_PSK_TKIP_CCMP;
        }
        else
        {
            sec_mode = (type==WPA_ENC_CCMP)?WPA2_PSK_CCMP:(type==WPA_ENC_TKIP)?WPA2_PSK_TKIP:WPA2_PSK_TKIP_CCMP;
        }
        break;
    case CW_WLAN_SECURITY_MODE_WPA_PSK_MIXED:
        if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
        {
            sec_mode = (type==WPA_ENC_CCMP)?GUEST_WPA_PSK_MIXED_CCMP:(type==WPA_ENC_TKIP)?GUEST_WPA_PSK_MIXED_TKIP:GUEST_WPA_PSK_MIXED_TKIP_CCMP;
        }
        else
        {
            sec_mode = (type==WPA_ENC_CCMP)?WPA_PSK_MIXED_CCMP:(type==WPA_ENC_TKIP)?WPA_PSK_MIXED_TKIP:WPA_PSK_MIXED_TKIP_CCMP;
        }
        break;
    case CW_WLAN_SECURITY_MODE_WPA:
        sec_mode = (type==WPA_ENC_CCMP)?WPA_EAP_CCMP:(type==WPA_ENC_TKIP)?WPA_EAP_TKIP:WPA_EAP_TKIP_CCMP;
        break;
    case CW_WLAN_SECURITY_MODE_WPA2:
        sec_mode = (type==WPA_ENC_CCMP)?WPA2_EAP_CCMP:(type==WPA_ENC_TKIP)?WPA2_EAP_TKIP:WPA2_EAP_TKIP_CCMP;
        break;
    case CW_WLAN_SECURITY_MODE_WPA_MIXED:
        sec_mode = (type==WPA_ENC_CCMP)?WPA_EAP_MIXED_CCMP:(type==WPA_ENC_TKIP)?WPA_EAP_MIXED_TKIP:WPA_EAP_MIXED_TKIP_CCMP;
        break;
#if SUPPORT_WPA3
    case CW_WLAN_SECURITY_MODE_OWE:
        sec_mode = ENCRYPTION_OWE_CCMP;
        break;
    case CW_WLAN_SECURITY_MODE_WPA3_PERSONAL:
        sec_mode = WPA3_SAE_CCMP;
        break;
    case CW_WLAN_SECURITY_MODE_WPA2_WPA3_PERSONAL_MIXED:
        sec_mode = WPA3_SAE_MIXED_CCMP;
        break;
    case CW_WLAN_SECURITY_MODE_WPA3:
        sec_mode = WPA3_EAP_CCMP;
        break;
    case CW_WLAN_SECURITY_MODE_WPA3_MIXED:
        sec_mode = WPA3_EAP_MIXED_CCMP;
        break;
#endif
    default:
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        encryption_option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"encryption");
        if (!api_set_wifi_guest_encryption_option(encryption_option, sec_mode))
        {
            wpa_enc_option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"wpa_enc");
            if (api_set_wifi_guest_wpa_enc_option(wpa_enc_option, type))
            {
                CW_FREE_OBJECT(encryption_option);
                CW_FREE_OBJECT(wpa_enc_option);
                return CW_FALSE;
            }
        }
        CW_FREE_OBJECT(encryption_option);
        CW_FREE_OBJECT(wpa_enc_option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (!api_set_wifi_ifacex_encryption_option_by_sectionname(OPM_AP, section, wlanIdx+1, sec_mode))
        {
            if (api_set_wifi_ifacex_wpa_enc_option_by_sectionname(OPM_AP, section, wlanIdx+1, type))
            {
                if (section != NULL) CW_FREE_OBJECT(section);
                return CW_FALSE;
            }
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanSecurityCfg(int radioIdx, int wlanIdx, int mode)
{
    int sec_mode = 0;
#if !SUPPORT_WPA3
    int hw_mode=0, auth_type = 0;
    char *option = NULL;
#endif
    char *section = NULL;

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] mode:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, mode);

#if SUPPORT_WPA3
    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] mode:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, mode);
    switch (mode)
    {
        case CW_WLAN_SECURITY_MODE_NONE:
            sec_mode = ENCRYPTION_NONE;
            break;
        case CW_WLAN_SECURITY_MODE_WPA2_PSK:
            sec_mode = WPA2_PSK_CCMP;
            break;
        case CW_WLAN_SECURITY_MODE_WPA2:
            sec_mode = WPA2_EAP_CCMP;
            break;
        case CW_WLAN_SECURITY_MODE_OWE:
            sec_mode = ENCRYPTION_OWE_CCMP;
            break;
        case CW_WLAN_SECURITY_MODE_WPA3_PERSONAL:
            sec_mode = WPA3_SAE_CCMP;
            break;
        case CW_WLAN_SECURITY_MODE_WPA2_WPA3_PERSONAL_MIXED:
            sec_mode = WPA3_SAE_MIXED_CCMP;
            break;
        case CW_WLAN_SECURITY_MODE_WPA3:
            sec_mode = WPA3_EAP_CCMP;
            break;
        case CW_WLAN_SECURITY_MODE_WPA3_MIXED:
            sec_mode = WPA3_EAP_MIXED_CCMP;
            break;
        default:
            CWDebugLog("Bad Wlan Security");
            return CW_TRUE;
    }
    if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        CWDebugLog("Guest wlan cannot support WPA Security");
        return CW_TRUE;
    }
    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
    if (api_set_wifi_ifacex_encryption_wpa_option_by_sectionname(OPM_AP, section, wlanIdx+1, sec_mode))
    {
        if (section != NULL) CW_FREE_OBJECT(section);
        return CW_FALSE;
    }
    if (section != NULL) CW_FREE_OBJECT(section);
#else
    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] mode:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, mode);
    switch (mode)
    {
        case CW_WLAN_SECURITY_MODE_NONE:
            CWWTPBoardSetWlanSecurityNONECfg(radioIdx, wlanIdx);
            break;
        case CW_WLAN_SECURITY_MODE_WEP:
            if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
            {
                CWDebugLog("Guest wlan cannot support WEP Security");
                return CW_TRUE;
            }
            section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
            if (api_get_wifi_ifacex_wep_auth_option_by_sectionname(OPM_AP, section, wlanIdx+1, &auth_type))
            {
                auth_type = WEP_AUTH_OPEN;
            }
            if (section != NULL) CW_FREE_OBJECT(section);
            CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] sec_mode:[%d] auth_type:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, sec_mode, auth_type);
            CWWTPBoardSetWlanSecurityWEPCfg(radioIdx, wlanIdx, auth_type);
            break;
        case CW_WLAN_SECURITY_MODE_WPA_PSK:
        case CW_WLAN_SECURITY_MODE_WPA2_PSK:
        case CW_WLAN_SECURITY_MODE_WPA_PSK_MIXED:
            if (!CWWTPBoardGetRadioWirelessModeCfg(radioIdx, &hw_mode))
            {
                hw_mode = (CWConvertRadioIdx(radioIdx))?CW_RADIO_WIRELESS_MODE_N_5G:CW_RADIO_WIRELESS_MODE_N_2G;
            }
            CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] mode:[%d], hw_mode:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, mode, hw_mode);
            if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
            {
                option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"wpa_enc");
                if (api_get_wifi_guest_wpa_enc_option(option, &auth_type))
                {
                    auth_type = (hw_mode==CW_RADIO_WIRELESS_MODE_N_5G)?WPA_ENC_CCMP:WPA_ENC_TKIP_CCMP;
                    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] auth_type:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, auth_type);
                }
                if (option != NULL) CW_FREE_OBJECT(option);
            }
            else
            {
                section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
                if (api_get_wifi_ifacex_wpa_enc_option_by_sectionname(OPM_AP, section, wlanIdx+1, &auth_type))
                {
                    switch (CWConvertRadioIdx(radioIdx))
                    {
#if SUPPORT_WLAN_24G_SETTING
                    case 0:
                        auth_type = (hw_mode==CW_RADIO_WIRELESS_MODE_N_2G)?WPA_ENC_CCMP:WPA_ENC_TKIP_CCMP;
                        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
                    case 1:
#if SUPPORT_WLAN_5G_2_SETTING
                    case 2:
#endif
                        auth_type = (hw_mode==CW_RADIO_WIRELESS_MODE_N_5G)?WPA_ENC_CCMP:WPA_ENC_TKIP_CCMP;
                        break;
#endif
                    default:
                        break;
                    }
                    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] auth_type:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, auth_type);
                }
                if (section != NULL) CW_FREE_OBJECT(section);
            }
            CWWTPBoardSetWlanSecurityWPACfg(radioIdx, wlanIdx, mode, auth_type);
            break;
        case CW_WLAN_SECURITY_MODE_WPA:
        case CW_WLAN_SECURITY_MODE_WPA2:
        case CW_WLAN_SECURITY_MODE_WPA_MIXED:
            if (!CWWTPBoardGetRadioWirelessModeCfg(radioIdx, &hw_mode))
            {
                hw_mode = (radioIdx)?CW_RADIO_WIRELESS_MODE_N_5G:CW_RADIO_WIRELESS_MODE_N_2G;
            }
            CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] mode:[%d], hw_mode:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, mode, hw_mode);
            if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
            {
                CWDebugLog("Guest wlan cannot support WPA Security");
                return CW_TRUE;
            }
            else
            {
                section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
                if (api_get_wifi_ifacex_wpa_enc_option_by_sectionname(OPM_AP, section, wlanIdx+1, &auth_type))
                {
                    switch (CWConvertRadioIdx(radioIdx))
                    {
#if SUPPORT_WLAN_24G_SETTING
                    case 0:
                        auth_type = (hw_mode==CW_RADIO_WIRELESS_MODE_N_2G)?WPA_ENC_CCMP:WPA_ENC_TKIP_CCMP;
                        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
                    case 1:
#if SUPPORT_WLAN_5G_2_SETTING
                    case 2:
#endif
                        auth_type = (hw_mode==CW_RADIO_WIRELESS_MODE_N_5G)?WPA_ENC_CCMP:WPA_ENC_TKIP_CCMP;
                        break;
#endif
                    default:
                        break;
                    }
                    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] auth_type:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, auth_type);
                }
                if (section != NULL) CW_FREE_OBJECT(section);
                CWWTPBoardSetWlanSecurityWPACfg(radioIdx, wlanIdx, mode, auth_type);
            }
            break;
        default:
            CWDebugLog("Bad Wlan Security");
            return CW_TRUE;
    }
#endif

    return CW_TRUE;
}

CWBool CWWTPBoardGetWepAuthTypeCfg(int radioIdx, int wlanIdx, int *type)
{
    int sec_mode = 0;
    char *section = NULL;

    if(wlanIdx != 0)
    {
        *type = CW_WEP_AUTH_TYPE_OPEN_SYSTEM;
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    if (api_get_wifi_ifacex_encryption_option_by_sectionname(OPM_AP, section, wlanIdx+1, &sec_mode))
    {
        sec_mode = 0;
    }

    switch(sec_mode)
    {
    case 1:
        *type = CW_WEP_AUTH_TYPE_OPEN_SYSTEM;
        break;
    case 2:
        *type = CW_WEP_AUTH_TYPE_SHARED_KEY;
        break;
    default:
        *type = CW_WEP_AUTH_TYPE_OPEN_SYSTEM;
        break;
    }
    if (section != NULL) CW_FREE_OBJECT(section);
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *type);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWepAuthTypeCfg(int radioIdx, int wlanIdx, int type)
{
    int encryption = 0, sec_mode = 0, auth_mode = 0;
    char *section = NULL;

    CWDebugLog("%s %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, type);

    if(wlanIdx != 0)
    {
        if(type != CW_WEP_AUTH_TYPE_OPEN_SYSTEM)
        {
            CWDebugLog("Only WLAN0 can support WEP Auth Type");
        }
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    if (api_get_wifi_ifacex_encryption_option_by_sectionname(OPM_AP, section, wlanIdx+1, &encryption))
    {
        encryption = 0;
    }

    CWDebugLog("%s %d %d encryption:[%d]", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, encryption);

    if(encryption == WEP_OPEN || encryption == WEP_SHARED)
    {
        switch(type)
        {
        case CW_WEP_AUTH_TYPE_OPEN_SYSTEM:
            sec_mode = WEP_OPEN;
            auth_mode = WEP_AUTH_OPEN;
            break;
        case CW_WEP_AUTH_TYPE_SHARED_KEY:
            sec_mode = WEP_SHARED;
            auth_mode = WEP_AUTH_SHARED;
            break;
        default:
            if (section != NULL) CW_FREE_OBJECT(section);
            CWDebugLog("Bad Auth Type");
            return CW_TRUE;
        }

        CWDebugLog("%s %d %d type:[%d] sec_mode:[%d] auth_mode:[%d]", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, type, sec_mode, auth_mode);
        if (!api_set_wifi_ifacex_encryption_option_by_sectionname(OPM_AP, section, wlanIdx+1, sec_mode))
        {
            if (api_set_wifi_ifacex_wep_auth_option_by_sectionname(OPM_AP, section, wlanIdx+1, auth_mode))
            {
                if (section != NULL) CW_FREE_OBJECT(section);
                return CW_FALSE;
            }
        }
    }

    if (section != NULL) CW_FREE_OBJECT(section);
    return CW_TRUE;
}

CWBool CWWTPBoardGetWepInputMethodCfg(int radioIdx, int wlanIdx, int *method)
{
    int keyIdx = 0;
    char val[34+1]={0};
    char *c = NULL, *section = NULL;

    *method = CW_WEP_INPUT_METHOD_HEX;

    if(wlanIdx != 0)
    {
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
    if (api_get_wifi_ifacex_wepkey_id_option_by_sectionname(OPM_AP, section, wlanIdx+1, &keyIdx))
    {
        keyIdx = 0;
    }

    if (!api_get_wifi_ifacex_wepkey_keyx_option_by_sectionname(OPM_AP, section, wlanIdx+1, keyIdx, val, sizeof(val)))
    {
        if ((c = strstr(val, "s:")))
        {
            *method = CW_WEP_INPUT_METHOD_ASCII;
        }
    }
    if (section != NULL) CW_FREE_OBJECT(section);
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *method);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWepInputMethodCfg(int radioIdx, int wlanIdx, int method)
{
    int keyIdx = 0;
    char val[34+1], wepkey[34+1];
    char *c = NULL, *section = NULL;

    CWDebugLog("%s %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, method);

    if(wlanIdx != 0)
    {
        if(method != CW_WEP_INPUT_METHOD_HEX)
        {
            CWDebugLog("Only WLAN0 can support WEP Input Method");
        }
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    for (keyIdx = 0; keyIdx<4; keyIdx++)
    {
        memset(val, 0, sizeof(val));
        if (!api_get_wifi_ifacex_wepkey_keyx_option_by_sectionname(OPM_AP, section, wlanIdx+1, keyIdx, val, sizeof(val)))
        {
            switch(method)
            {
            case CW_WEP_INPUT_METHOD_HEX:
                if ((c = strstr(val,"s:")))
                {
                    if (api_set_wifi_ifacex_wepkey_keyx_option_by_sectionname(OPM_AP, section, wlanIdx+1, keyIdx, c+2, sizeof(val)))
                    {
                        if (section != NULL) CW_FREE_OBJECT(section);
                        return CW_FALSE;
                    }
                }
                break;
            case CW_WEP_INPUT_METHOD_ASCII:
                if (strstr(val,"s:"))
                {
                    snprintf(wepkey, sizeof(wepkey), "%s", val);
                    if (api_set_wifi_ifacex_wepkey_keyx_option_by_sectionname(OPM_AP, section, wlanIdx+1, keyIdx, wepkey, sizeof(val)))
                    {
                        if (section != NULL) CW_FREE_OBJECT(section);
                        return CW_FALSE;
                    }
                }
                break;
            default:
                break;
            }
        }
    }
    if (section != NULL) CW_FREE_OBJECT(section);
    return CW_TRUE;
}

CWBool CWWTPBoardGetWepDefaultKeyIdCfg(int radioIdx, int wlanIdx, int *keyId)
{
    int val = 1;
    char *section = NULL;

    if(wlanIdx != 0)
    {
        *keyId = 0;
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    if (api_get_wifi_ifacex_wepkey_id_option_by_sectionname(OPM_AP, section, wlanIdx+1, &val))
    {
        if (api_set_wifi_ifacex_wepkey_id_option_by_sectionname(OPM_AP, section, wlanIdx+1, val))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
    }
    *keyId = val - 1;
    if (section != NULL) CW_FREE_OBJECT(section);
    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *keyId);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWepDefaultKeyIdCfg(int radioIdx, int wlanIdx, int keyId)
{
    char *section = NULL;

    CWDebugLog("%s %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, keyId);

    if(wlanIdx != 0)
    {
        if(keyId != 0)
        {
            CWDebugLog("Only WLAN0 can support WEP Default Key");
        }
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    if (api_set_wifi_ifacex_wepkey_id_option_by_sectionname(OPM_AP, section, wlanIdx+1, keyId+1))
    {
        if (section != NULL) CW_FREE_OBJECT(section);
        return CW_FALSE;
    }
    if (section != NULL) CW_FREE_OBJECT(section);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWepKeyLengthCfg(int radioIdx, int wlanIdx, int *len)
{
    int keyIdx = 0, key_len;
    char val[32+1], *section = NULL;

    *len = 64;

    if(wlanIdx != 0)
    {
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    if (!api_get_wifi_ifacex_wepkey_id_option_by_sectionname(OPM_AP, section, wlanIdx+1, &keyIdx))
    {
        if (!api_get_wifi_ifacex_wepkey_keyx_option_by_sectionname(OPM_AP, section, wlanIdx+1, keyIdx, val, sizeof(val)))
        {
            key_len = strlen(val);

            if (key_len % 16 == 0)
            {
                *len = 152;
            }
            else if (key_len % 13 == 0)
            {
                *len = 128;
            }
            else
            {
                *len = 64;
            }
        }
    }
    if (section != NULL) CW_FREE_OBJECT(section);
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *len);
    return CW_TRUE;

}

CWBool CWWTPBoardSetWepKeyLengthCfg(int radioIdx, int wlanIdx, int len)
{
    CWDebugLog("%s %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, len);

    if(wlanIdx != 0)
    {
        if(len != 0)
        {
            CWDebugLog("Only WLAN0 can support WEP Key Length");
        }
        return CW_TRUE;
    }

    if (api_check_wifi_iface_wepkey_length_option(NULL, len))
    {
        return CW_FALSE;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWepKeyCfg(int radioIdx, int wlanIdx, int keyIdx, char **pstr)
{
    char *val = NULL, *section = NULL;

    if(wlanIdx != 0)
    {
        return CW_TRUE;
    }

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", MAX_SECRET_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    api_get_wifi_ifacex_wepkey_keyx_option_by_sectionname(OPM_AP, section, wlanIdx+1, keyIdx+1, val, MAX_SECRET_SIZE+1);

    *pstr = val;

    CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWepKeyCfg(int radioIdx, int wlanIdx, int keyIdx, char *pstr)
{
    char *pUciStr = NULL;
    char *section = NULL;

    CWDebugLog("%s %d %d %d %s", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, keyIdx, pstr);

    if(wlanIdx != 0)
    {
        if(pstr[0] != 0)
        {
            CWDebugLog("Only WLAN0 can support WEP Key");
        }
        return CW_TRUE;
    }

    if (pstr != NULL)
    {
        CW_CREATE_STRING_FROM_STRING_ERR(pUciStr, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    }
    else
    {
        CW_CREATE_STRING_FROM_STRING_ERR(pUciStr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    }
    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
    if (api_set_wifi_ifacex_wepkey_keyx_option_by_sectionname(OPM_AP, section, wlanIdx+1, keyIdx+1, pUciStr, strlen(pUciStr)+1))
    {
        if (section != NULL) CW_FREE_OBJECT(section);
        CW_FREE_OBJECT(pUciStr);
        return CW_FALSE;
    }
    if (section != NULL) CW_FREE_OBJECT(section);
    CW_FREE_OBJECT(pUciStr);
    return CW_TRUE;
}

CWBool CWWTPBoardGetWpaEncryptionCfg(int radioIdx, int wlanIdx, int *mode)
{
    int old_mode = 0, val = 3;
    char *option = NULL;
    char *section = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"encryption");
        if (api_get_wifi_guest_encryption_option(option, &old_mode))
        {
            old_mode = 0;
        }
        if (old_mode >= GUEST_WPA_PSK_CCMP && old_mode <= GUEST_WPA_PSK_MIXED_TKIP_CCMP)
        {
            val = (old_mode-1)%3;
        }
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_encryption_option_by_sectionname(OPM_AP, section, wlanIdx+1, &old_mode))
        {
            old_mode = 0;
        }
        if (old_mode >= WPA_PSK_CCMP && old_mode <= WPA_EAP_MIXED_TKIP_CCMP)
        {
            val = old_mode%3;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    switch(val)
    {
    case 0:
        *mode = CW_WPA_ENCRPTION_AES;
        break;
    case 1:
        *mode = CW_WPA_ENCRPTION_TKIP;
        break;
    case 2:
        *mode = CW_WPA_ENCRPTION_TKIP_AES;
        break;
    default:
        *mode = CW_WPA_ENCRPTION_TKIP_AES;
        break;
    }

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *mode);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWpaEncryptionCfg(int radioIdx, int wlanIdx, int mode)
{
    int sec_mode = 0, enc_mode = 0, encryption = 0;
    char *encryption_option = NULL, *wpa_enc_option = NULL;
    char *section = NULL;

    CWDebugLog("%s %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, mode);

    if(mode < CW_WPA_ENCRPTION_TKIP_AES || mode > CW_WPA_ENCRPTION_AES)
    {
        CWDebugLog("Bad WPA Encryption mode");
        return CW_TRUE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        encryption_option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"encryption");
        if (api_get_wifi_guest_encryption_option(encryption_option, &encryption))
        {
            encryption = 0; 
        }
        
        CWDebugLog("%s %d %d %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, mode, encryption, sec_mode);

        if(encryption >= GUEST_WPA_PSK_CCMP && encryption <= GUEST_WPA_PSK_MIXED_TKIP_CCMP)
        {
            switch (encryption)
            {
            case GUEST_WPA_PSK_CCMP:
            case GUEST_WPA_PSK_TKIP:
            case GUEST_WPA_PSK_TKIP_CCMP:
                sec_mode = (mode==CW_WPA_ENCRPTION_TKIP)?GUEST_WPA_PSK_TKIP:(mode==CW_WPA_ENCRPTION_AES)?GUEST_WPA_PSK_CCMP:GUEST_WPA_PSK_TKIP_CCMP;
                break;
            case GUEST_WPA2_PSK_CCMP:
            case GUEST_WPA2_PSK_TKIP:
            case GUEST_WPA2_PSK_TKIP_CCMP:
                sec_mode = (mode==CW_WPA_ENCRPTION_TKIP)?GUEST_WPA2_PSK_TKIP:(mode==CW_WPA_ENCRPTION_AES)?GUEST_WPA2_PSK_CCMP:GUEST_WPA2_PSK_TKIP_CCMP;
                break;
            case GUEST_WPA_PSK_MIXED_CCMP:
            case GUEST_WPA_PSK_MIXED_TKIP:
            case GUEST_WPA_PSK_MIXED_TKIP_CCMP:
                sec_mode = (mode==CW_WPA_ENCRPTION_TKIP)?GUEST_WPA_PSK_MIXED_TKIP:(mode==CW_WPA_ENCRPTION_AES)?GUEST_WPA_PSK_MIXED_CCMP:GUEST_WPA_PSK_MIXED_TKIP_CCMP;
                break;
            default:
                break;
            }
        }
        else
        {
            CW_FREE_OBJECT(encryption_option);
            CWDebugLog("Bad WPA Encryption mode");
            return CW_TRUE;
        }
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_encryption_option_by_sectionname(OPM_AP, section, wlanIdx+1, &encryption))
        {
            encryption = 0;
        }
        
        CWDebugLog("%s %d %d %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, mode, encryption, sec_mode);

        if(encryption >= WPA_PSK_CCMP && encryption <= WPA_EAP_MIXED_TKIP_CCMP)
        {
            switch (encryption)
            {
            case WPA_PSK_CCMP:
            case WPA_PSK_TKIP:
            case WPA_PSK_TKIP_CCMP:
                sec_mode = (mode==CW_WPA_ENCRPTION_TKIP)?WPA_PSK_TKIP:(mode==CW_WPA_ENCRPTION_AES)?WPA_PSK_CCMP:WPA_PSK_TKIP_CCMP;
                break;
            case WPA2_PSK_CCMP:
            case WPA2_PSK_TKIP:
            case WPA2_PSK_TKIP_CCMP:
                sec_mode = (mode==CW_WPA_ENCRPTION_TKIP)?WPA2_PSK_TKIP:(mode==CW_WPA_ENCRPTION_AES)?WPA2_PSK_CCMP:WPA2_PSK_TKIP_CCMP;
                break;
            case WPA_PSK_MIXED_CCMP:
            case WPA_PSK_MIXED_TKIP:
            case WPA_PSK_MIXED_TKIP_CCMP:
                sec_mode = (mode==CW_WPA_ENCRPTION_TKIP)?WPA_PSK_MIXED_TKIP:(mode==CW_WPA_ENCRPTION_AES)?WPA_PSK_MIXED_CCMP:WPA_PSK_MIXED_TKIP_CCMP;
                break;
            case WPA_EAP_CCMP:
            case WPA_EAP_TKIP:
            case WPA_EAP_TKIP_CCMP:
                sec_mode = (mode==CW_WPA_ENCRPTION_TKIP)?WPA_EAP_TKIP:(mode==CW_WPA_ENCRPTION_AES)?WPA_EAP_CCMP:WPA_EAP_TKIP_CCMP;
                break;
            case WPA2_EAP_CCMP:
            case WPA2_EAP_TKIP:
            case WPA2_EAP_TKIP_CCMP:
                sec_mode = (mode==CW_WPA_ENCRPTION_TKIP)?WPA2_EAP_TKIP:(mode==CW_WPA_ENCRPTION_AES)?WPA2_EAP_CCMP:WPA2_EAP_TKIP_CCMP;
                break;
            case WPA_EAP_MIXED_CCMP:
            case WPA_EAP_MIXED_TKIP:
            case WPA_EAP_MIXED_TKIP_CCMP:
                sec_mode = (mode==CW_WPA_ENCRPTION_TKIP)?WPA_EAP_MIXED_TKIP:(mode==CW_WPA_ENCRPTION_AES)?WPA_EAP_MIXED_CCMP:WPA_EAP_MIXED_TKIP_CCMP;
                break;
            default:
                break;
            }
        }
        else
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            CWDebugLog("Bad WPA Encryption mode");
            return CW_TRUE;
        }
    }

    switch(mode)
    {
    case CW_WPA_ENCRPTION_TKIP:
        enc_mode = WPA_ENC_TKIP;
        break;
    case CW_WPA_ENCRPTION_AES:
        enc_mode = WPA_ENC_CCMP;
        break;
    default:
        enc_mode = WPA_ENC_TKIP_CCMP;
        break;
    }

    CWDebugLog("%s %d %d %d %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, mode, encryption, sec_mode, enc_mode);

    //CW_WPA_ENCRPTION_TKIP_AES = 0; CW_WPA_ENCRPTION_TKIP = 1; CW_WPA_ENCRPTION_AES = 2;
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(!api_set_wifi_guest_encryption_option(encryption_option, sec_mode))
        {
            wpa_enc_option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"wpa_enc");
            if (api_set_wifi_guest_wpa_enc_option(wpa_enc_option, enc_mode))
            {
                CW_FREE_OBJECT(encryption_option);
                CW_FREE_OBJECT(wpa_enc_option);
                return CW_FALSE;
            }
        }
        CW_FREE_OBJECT(encryption_option);
        CW_FREE_OBJECT(wpa_enc_option);
    }
    else
    {
        if (!api_set_wifi_ifacex_encryption_option_by_sectionname(OPM_AP, section, wlanIdx+1, sec_mode))
        {
            if (api_set_wifi_ifacex_wpa_enc_option_by_sectionname(OPM_AP, section, wlanIdx+1, enc_mode))
            {
                if (section != NULL) CW_FREE_OBJECT(section);
                return CW_FALSE;
            }
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWpaPassphraseCfg(int radioIdx, int wlanIdx, char **pstr)
{
    char *val, *option, *section;

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", MAX_SECRET_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"key");
        api_get_wifi_guest_wpa_key_option(option, val, MAX_SECRET_SIZE+1);
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        api_get_wifi_ifacex_wpakey_key_option_by_sectionname(OPM_AP, section, wlanIdx+1, val, MAX_SECRET_SIZE+1);
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    *pstr = val;

    CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, *pstr);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWpaPassphraseCfg(int radioIdx, int wlanIdx, char *pstr)
{
    char *pUciStr = NULL, *option = NULL, *section = NULL;
    int encryption = 0;

    CWDebugLog("%s %d %d %s", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(pUciStr, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"key");
        if (api_set_wifi_guest_wpa_key_option(option, pUciStr, strlen(pUciStr)))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            CW_FREE_OBJECT(pUciStr);
            return CW_FALSE;
        }
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_wpakey_key_option_by_sectionname(OPM_AP, section, wlanIdx+1, pUciStr, strlen(pUciStr)))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            CW_FREE_OBJECT(pUciStr);
            return CW_FALSE;
        }

#if SUPPORT_WPA3
        if (api_get_wifi_ifacex_encryption_option_by_sectionname(OPM_AP, section, wlanIdx+1, &encryption))
        {
            encryption = 0;
        }

        if(encryption == WPA3_SAE_CCMP || encryption == WPA3_SAE_MIXED_CCMP)
        {
            if (api_set_wifi_ifacex_wpakey_key_option_by_sectionname(OPM_AP, section, wlanIdx+1, pUciStr, strlen(pUciStr)))
            {
                if (section != NULL) CW_FREE_OBJECT(section);
                CW_FREE_OBJECT(pUciStr);
                return CW_FALSE;
            }
        }

#endif
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    CW_FREE_OBJECT(pUciStr);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWpaGroupKeyUpdateIntervalCfg(int radioIdx, int wlanIdx, int *interval)
{
    int val = 3600;
    char *option = NULL;
    char *section = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"wpa_group_rekey");
        if (api_get_wifi_guest_wpa_group_rekey_option(option, &val))
        {
            if (api_set_wifi_guest_wpa_group_rekey_option(option, val))
            {
                if (option != NULL) CW_FREE_OBJECT(option);
                return CW_FALSE;
            }
        }
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_wpa_group_rekey_option_by_sectionname(OPM_AP, section, wlanIdx+1, &val))
        {
            if (api_set_wifi_ifacex_wpa_group_rekey_option_by_sectionname(OPM_AP, section, wlanIdx+1, val))
            {
                if (section != NULL) CW_FREE_OBJECT(section);
                return CW_FALSE;
            }
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    *interval = val;

    CWDebugLog("%s %d interval:[%d]", __FUNCTION__, __LINE__, *interval);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWpaGroupKeyUpdateIntervalCfg(int radioIdx, int wlanIdx, int interval)
{
    char *option = NULL;
    char *section = NULL;

    CWDebugLog("%s %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, interval);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        option = CWCreateString("wireless.wifi%d_guest.%s",CWConvertRadioIdx(radioIdx),"wpa_group_rekey");
        if (api_set_wifi_guest_wpa_group_rekey_option(option, interval))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            return CW_FALSE;
        }
        if (option != NULL) CW_FREE_OBJECT(option);
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_wpa_group_rekey_option_by_sectionname(OPM_AP, section, wlanIdx+1, interval))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetSuiteBEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int val = 0;
    char *section = NULL;

#if SUPPORT_WPA3
    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
    if (api_get_wifi_ifacex_suiteb_option_by_sectionname(OPM_AP, section, wlanIdx+1, &val))
    {
        val = 0;
    }
    if (section != NULL) CW_FREE_OBJECT(section);

    if (val)
    {
        *enable = CW_TRUE;
    }
    else
    {
        *enable = CW_FALSE;
    }
#else
    CWDebugLog("Not support wpa3");
#endif

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetSuiteBEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    char *section = NULL;

#if SUPPORT_WPA3
    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
    if (api_set_wifi_ifacex_suiteb_option_by_sectionname(OPM_AP, section, wlanIdx+1, enable))
    {
        if (section != NULL) CW_FREE_OBJECT(section);
        CWDebugLog("Bad SuiteB Enable");
        return CW_TRUE;
    }
#else
    CWDebugLog("Not support wpa3");
#endif

    if (section != NULL) CW_FREE_OBJECT(section);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWpaRadiusAddressCfg(int radioIdx, int wlanIdx, unsigned int *addr)
{
    char val[15+1]={0};
    char *section = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        *addr = 0;
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    if (api_get_wifi_ifacex_auth_server_option_by_sectionname(OPM_AP, section, wlanIdx+1, val, sizeof(val)))
    {
        *addr = 0;
    }

    if(val[0])
    {
        *addr = inet_addr(val);
    }

    if (section != NULL) CW_FREE_OBJECT(section);
    CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, val);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWpaRadiusAddressCfg(int radioIdx, int wlanIdx, unsigned int addr)
{
    char val[15+1];
    char *section = NULL;

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(addr)
        {
            CWDebugLog("Guest wlan cannot support Radius Address");
        }
        return CW_TRUE;
    }

    if(addr)
    {
        snprintf(val, sizeof(val), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
    }
    else
    {
        snprintf(val, sizeof(val), " ");
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    if (api_set_wifi_ifacex_auth_server_option_by_sectionname(OPM_AP, section, wlanIdx+1, val, sizeof(val)))
    {
        if (section != NULL) CW_FREE_OBJECT(section);
        return CW_FALSE;
    }
    if (section != NULL) CW_FREE_OBJECT(section);
    return CW_TRUE;
}

CWBool CWWTPBoardGetWpaRadiusPortCfg(int radioIdx, int wlanIdx, unsigned short *port)
{
    int val = 1812;
    char *section = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        *port = 1812;
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    if (api_get_wifi_ifacex_auth_port_option_by_sectionname(OPM_AP, section, wlanIdx+1, &val))
    {
        if (api_set_wifi_ifacex_auth_port_option_by_sectionname(OPM_AP, section, wlanIdx+1, val))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
    }
    *port = val;
    if (section != NULL) CW_FREE_OBJECT(section);
    CWDebugLog("%s %d %u", __FUNCTION__, __LINE__, *port);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWpaRadiusPortCfg(int radioIdx, int wlanIdx, unsigned short port)
{
    char *section = NULL;

    CWDebugLog("%s %d %d %u", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, port);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(port)
        {
            CWDebugLog("Guest wlan cannot support Radius Port");
        }
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    if (api_set_wifi_ifacex_auth_port_option_by_sectionname(OPM_AP, section, wlanIdx+1, port))
    {
        if (section != NULL) CW_FREE_OBJECT(section);
        return CW_FALSE;
    }
    if (section != NULL) CW_FREE_OBJECT(section);
    return CW_TRUE;
}

CWBool CWWTPBoardGetWpaRadiusSecretCfg(int radioIdx, int wlanIdx, char **pstr)
{
    char *val = NULL, *section = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        return CW_TRUE;
    }

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", MAX_SECRET_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    api_get_wifi_ifacex_auth_secret_option_by_sectionname(OPM_AP, section, wlanIdx+1, val, MAX_SECRET_SIZE+1);

    *pstr = val;

    if (section != NULL) CW_FREE_OBJECT(section);

    CWDebugLog("%s %d radio:[%d] wlanIdx:[%d] %s", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWpaRadiusSecretCfg(int radioIdx, int wlanIdx, char *pstr)
{
    char *pUciStr = NULL, *section = NULL;

    CWDebugLog("%s %d %d %s", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(pstr[0] != 0)
        {
            CWDebugLog("Guest wlan cannot support Radius Secret");
        }
        return CW_TRUE;
    }

    CW_CREATE_STRING_FROM_STRING_ERR(pUciStr, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
    if (api_set_wifi_ifacex_auth_secret_option_by_sectionname(OPM_AP, section, wlanIdx+1, pUciStr, strlen(pUciStr)))
    {
        if (section != NULL) CW_FREE_OBJECT(section);
        CW_FREE_OBJECT(pUciStr);
        return CW_FALSE;
    }
    if (section != NULL) CW_FREE_OBJECT(section);
    CW_FREE_OBJECT(pUciStr);
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadiusAccountingEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int val = 0;
    char *section = NULL;

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        *enable = CW_FALSE;
        return CW_TRUE;
    }
#endif

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_get_wifix_guest_acct_enabled_option(CWConvertRadioIdx(radioIdx), &val))
        {
            if (api_set_wifix_guest_acct_enabled_option(CWConvertRadioIdx(radioIdx), val))
            {
                return CW_FALSE;
            }
        }
    }
    else
#endif
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_acct_enabled_option_by_sectionname(OPM_AP, section, wlanIdx+1, &val))
        {
            if (api_set_wifi_ifacex_acct_enabled_option_by_sectionname(OPM_AP, section, wlanIdx+1, val))
            {
                if (section != NULL) CW_FREE_OBJECT(section);
                return CW_FALSE;
            }
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    switch(val)
    {
    case 1:
        *enable = CW_TRUE;
        break;
    default:
        *enable = CW_FALSE;
        break;
    }

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadiusAccountingEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    char *section = NULL;

    CWDebugLog("%s %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(enable)
        {
            CWDebugLog("Guest wlan cannot support Radius Accounting Enable");
        }
        return CW_TRUE;
    }
#endif


#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_set_wifix_guest_acct_enabled_option(CWConvertRadioIdx(radioIdx), enable ? 1 : 0))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_acct_enabled_option_by_sectionname(OPM_AP, section, wlanIdx+1, enable ? 1 : 0))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadiusAccountingAddressCfg(int radioIdx, int wlanIdx, unsigned int *addr)
{
    char val[API_IPADDR_SIZE] = {0};
    char *section = NULL;

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        *addr = 0;
        return CW_TRUE;
    }
#endif

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_get_wifix_guest_acct_server_option(CWConvertRadioIdx(radioIdx), val, sizeof(val)))
        {
            *addr = 0;
        }
    }
    else
#endif
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_acct_server_option_by_sectionname(OPM_AP, section, wlanIdx+1, val, sizeof(val)))
        {
            *addr = 0;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    if(val[0])
    {
        *addr = inet_addr(val);
    }

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *addr);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadiusAccountingAddressCfg(int radioIdx, int wlanIdx, unsigned int addr)
{
    char val[API_IPADDR_SIZE] = {0};
    char *section = NULL;

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(addr)
        {
            CWDebugLog("Guest wlan cannot support Radius Accounting Address");
        }
        return CW_TRUE;
    }
#endif

    snprintf(val, API_IPADDR_SIZE, "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_set_wifix_guest_acct_server_option(CWConvertRadioIdx(radioIdx), val, API_IPADDR_SIZE))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_acct_server_option_by_sectionname(OPM_AP, section, wlanIdx+1, val, API_IPADDR_SIZE))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }
    return CW_TRUE;

}

CWBool CWWTPBoardGetRadiusAccountingPortCfg(int radioIdx, int wlanIdx, unsigned short *port)
{
    int val = 1813;
    char *section = NULL;

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        *port = 1813;
        return CW_TRUE;
    }
#endif

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_get_wifix_guest_acct_port_option(CWConvertRadioIdx(radioIdx), &val))
        {
            if (api_set_wifix_guest_acct_port_option(CWConvertRadioIdx(radioIdx), val))
            {
                return CW_FALSE;
            }
        }
    }
    else
#endif
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_acct_port_option_by_sectionname(OPM_AP, section, wlanIdx+1, &val))
        {
            if (api_set_wifi_ifacex_acct_port_option_by_sectionname(OPM_AP, section, wlanIdx+1, val))
            {
                if (section != NULL) CW_FREE_OBJECT(section);
                return CW_FALSE;
            }
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    *port = val;

    CWDebugLog("%s %d %u", __FUNCTION__, __LINE__, *port);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadiusAccountingPortCfg(int radioIdx, int wlanIdx, unsigned short port)
{
    char *section = NULL;

    CWDebugLog("%s %d %d %u", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, port);

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(port)
        {
            CWDebugLog("Guest wlan cannot support Radius Accounting Port");
        }
        return CW_TRUE;
    }
#endif

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_set_wifix_guest_acct_port_option(CWConvertRadioIdx(radioIdx), port))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_acct_port_option_by_sectionname(OPM_AP, section, wlanIdx+1, port))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadiusAccountingSecretCfg(int radioIdx, int wlanIdx, char **pstr)
{
    char *val = NULL, *section = NULL;

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        return CW_TRUE;
    }
#endif

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", MAX_SECRET_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        api_get_wifix_guest_acct_secret_option(CWConvertRadioIdx(radioIdx), val, MAX_SECRET_SIZE+1);
    }
    else
#endif
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        api_get_wifi_ifacex_acct_secret_option_by_sectionname(OPM_AP, section, wlanIdx+1, val, MAX_SECRET_SIZE+1);
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    *pstr = val;

    CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, *pstr);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadiusAccountingSecretCfg(int radioIdx, int wlanIdx, char *pstr)
{
    char *pUciStr = NULL, *section = NULL;

    CWDebugLog("%s %d %d %s", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, pstr);

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(pstr[0] != 0)
        {
            CWDebugLog("Guest wlan cannot support Radius Accounting Secret");
        }
        return CW_TRUE;
    }
#endif

    CW_CREATE_STRING_FROM_STRING_ERR(pUciStr, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_set_wifix_guest_acct_secret_option(CWConvertRadioIdx(radioIdx), pUciStr, strlen(pUciStr)))
        {
            CW_FREE_OBJECT(pUciStr);
            return CW_FALSE;
        }
    }
    else
#endif
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_acct_secret_option_by_sectionname(OPM_AP, section, wlanIdx+1, pUciStr, strlen(pUciStr)))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            CW_FREE_OBJECT(pUciStr);
            return CW_FALSE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    CW_FREE_OBJECT(pUciStr);

    return CW_TRUE;
}
//NOT_FINISH
CWBool CWWTPBoardIsolationConfig(void)
{
#if 0
    int radioIdx, wlanIdx, rdIdx, wlIdx;
    int enable, vId, vlanId, mngVid = 0;
    int networkId, cmpRes;
    char *val;

    CWWTPBoardGetManageVlanCfg(&mngVid);

    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
        {
            if(GUEST_WLAN_IDX(radioIdx) == wlanIdx)
            {
                continue;
            }

            CWWTPBoardGetWlanEnableCfg(radioIdx, wlanIdx, &enable);
            if(!enable)
            {
                continue;
            }

            CWWTPBoardGetWlanIsolationCfg(radioIdx, wlanIdx, &enable);
            if(!enable)
            {
                continue;
            }

            CWWTPBoardGetWlanVlanCfg(radioIdx, wlanIdx, &vlanId);

            /* check if the vlan is the same with management vlan */
            if(vlanId == mngVid)
            {
                CWDebugLog("(radio %d wlan %d) is added to bridge network of management vlan %d",
                           radioIdx, wlanIdx, vlanId);

                if(!(val = CWCreateStringByUci("wireless.w%d_index%d.network",
                                               radioIdx, UCI_WLAN_IDX(radioIdx, wlanIdx))))
                {
                    return CW_FALSE;
                }

                /* delete original bridge network that may be created for this wlan */
                if(atoi(val) == BRIDGE_NETWORK_ID(radioIdx, wlanIdx))
                {
                    CWSystem("uci delete network.%d", BRIDGE_NETWORK_ID(radioIdx, wlanIdx));
                }

                /* add to lan bridge */
                CWSystem("uci set wireless.w%d_index%d.network=lan", radioIdx, UCI_WLAN_IDX(radioIdx, wlanIdx));

                CW_FREE_OBJECT(val);
                continue;
            }

            networkId = 0;

            for(rdIdx = 0; rdIdx < CWWTPBoardGetMaxRadio(); rdIdx++)
            {
                for(wlIdx = 0; wlIdx < CWWTPBoardGetMaxRadioWlans(rdIdx); wlIdx++)
                {
                    if(GUEST_WLAN_IDX(rdIdx) == wlIdx)
                    {
                        continue;
                    }

                    CWWTPBoardGetWlanEnableCfg(rdIdx, wlIdx, &enable);
                    if(!enable)
                    {
                        continue;
                    }

                    CWWTPBoardGetWlanIsolationCfg(rdIdx, wlIdx, &enable);
                    if(!enable)
                    {
                        continue;
                    }

                    cmpRes = WLAN_CMP(rdIdx, wlIdx, radioIdx, wlanIdx);

                    if(cmpRes != 0)
                    {
                        CWWTPBoardGetWlanVlanCfg(rdIdx, wlIdx, &vId);
                        if(vId != vlanId)
                        {
                            continue;
                        }

                        if(cmpRes < 0)
                        {
                            /* the bridge network of this vlan should be already created, we can stop the looking */
                            CWDebugLog("(radio %d wlan %d) should be already added to the bridge network of vlan %d",
                                       rdIdx, wlIdx, vlanId);

                            rdIdx = CWWTPBoardGetMaxRadio(); /* trick to break the outside loop */
                            break;
                        }
                        else /* cmpRes > 0 */
                        {
                            CWDebugLog("(radio %d wlan %d) is added to bridge network id %d of vlan %d",
                                       rdIdx, wlIdx, networkId, vlanId);

                            /* add to bridge network which should be already created */
                            if(!(val = CWCreateStringByUci("wireless.w%d_index%d.network", rdIdx, UCI_WLAN_IDX(rdIdx, wlIdx))))
                            {
                                return CW_FALSE;
                            }

                            /* delete original bridge network that may be created for this wlan */
                            if(atoi(val) == BRIDGE_NETWORK_ID(rdIdx, wlIdx))
                            {
                                CWSystem("uci delete network.%d", BRIDGE_NETWORK_ID(rdIdx, wlIdx));
                            }
                            // add to bridge
                            CWSystem("uci set wireless.w%d_index%d.network=%d", rdIdx, UCI_WLAN_IDX(rdIdx, wlIdx), networkId);

                            CW_FREE_OBJECT(val);
                        }
                    }
                    else
                    {
                        /* the bridge network of this vlan does not exist, create the bridge network for it */
                        networkId = BRIDGE_NETWORK_ID(rdIdx, wlIdx);

                        CWDebugLog("(radio %d wlan %d) create bridge network id %d of vlan %d",
                                   rdIdx, wlIdx, networkId, vlanId);

                        // set master bridge
                        CWSystem("uci set network.%d=interface", networkId);
                        CWSystem("uci set network.%d.ifname=eth0.%d", networkId, vlanId);
                        CWSystem("uci set network.%d.type=bridge", networkId);
                        CWSystem("uci set network.%d.auto=1", networkId);
                        CWSystem("uci set network.%d.proto=none", networkId);
                        CWSystem("uci set wireless.w%d_index%d.network=%d", rdIdx, UCI_WLAN_IDX(rdIdx, wlIdx), networkId);
                    }
                }
            }
        }
    }
#endif
    return CW_TRUE;
}
//NOT_FINISH
CWBool CWWTPBoardGetRadioOperationalState(int radioIdx, CWRadioState *state, CWOperationalCause *cause)
{
    *state = ENABLED;
    *cause = OP_NORMAL;

    return CW_TRUE;
}
//NOT_FINISH
CWBool CWWTPBoardGetRadioDecryptErrorReport(int radioIdx, CWMacAddress **errorMacList, unsigned char *numEntries)
{
    *errorMacList = NULL;
    *numEntries = 0;

    return CW_TRUE;
}

//NOT_FINISH (NOT SUPPORT)
CWBool CWGetHealthyTxPowerValue(int i32MaxTxPower, int int32HealingTxpower, int *i32healthyTxpower)
{
    CWDebugLog("%s %d i32MaxTxPower:[%d]", __FUNCTION__, __LINE__, i32MaxTxPower);
#ifdef ATMWS_SERIES
    switch(int32HealingTxpower)
    {
        case 100: //%
            *i32healthyTxpower = i32MaxTxPower;
            break;
        case 50: //%
            *i32healthyTxpower = (i32MaxTxPower - 3);
            break;
        case 25: //%
            *i32healthyTxpower = (i32MaxTxPower - 6);
            break;
        case 10: //%
            *i32healthyTxpower = (i32MaxTxPower - 10);
            break;
        default:
            return CW_FALSE;
    }
#else
    switch(int32HealingTxpower)
    {
        case 100: //%
            *i32healthyTxpower = i32MaxTxPower;
            break;
        case 50: //%
            *i32healthyTxpower = (((i32MaxTxPower - 11) / 2) + 11);
            break;
        case 25: //%
            *i32healthyTxpower = (((i32MaxTxPower - 11) / 4) + 11);
            break;
        case 10: //%
            *i32healthyTxpower = (((i32MaxTxPower - 11) / 8) + 11);
            break;
        default:
            return CW_FALSE;
    }
#endif
    return CW_TRUE;
}

CWBool CWWTPBoardGetSitesurveyIwlist(CWRadioFreqType radioType, unsigned char *version, int *count, CWWTPSitesurvey **sitesurvey)
{
    int i = 0, radioIdx = 0;
    char tmpFile[64] = {0};
    FILE *fp;
    long fsize;
    char *buffer, *c;
    unsigned int bssid[6];
    unsigned int uint32CapCode;
    int int32RadioIdx = 0, int32WlanIdx = 0, int32SsidEnable = 0;
    CWBool bFound = CW_FALSE;

    if (!CWWTPBoardGetCapCode(&uint32CapCode))
    {
        CWDebugLog("%s %d Get capCode fail", __func__, __LINE__);
        return CW_FALSE;
    }

    if (!CWGetRadioIndex(radioType, uint32CapCode, &int32RadioIdx))
    {
        CWDebugLog("%s %d Get radioIdx fail", __func__, __LINE__);
        return CW_FALSE;
    }

    radioIdx = CWConvertRadioIdx(int32RadioIdx);

    for (int32WlanIdx = 0; int32WlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); int32WlanIdx++) 
    {
        if (CWWTPBoardGetWlanEnableCfg(radioIdx, int32WlanIdx, &int32SsidEnable)) 
        {
            if (int32SsidEnable) 
            {
                bFound = CW_TRUE;
                break;
            }
        }
    }

    if (bFound == CW_FALSE) 
    {
        return CW_FALSE;
    }

    switch (radioType) 
    {
#if SUPPORT_WLAN_24G_SETTING
    case CW_RADIOFREQTYPE_2G:
        sprintf(tmpFile, "/tmp/WLAN_SITE_SURVEY%s.%x.tmp", "2G", (unsigned int) CWThreadSelf());
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case CW_RADIOFREQTYPE_5G:
        sprintf(tmpFile, "/tmp/WLAN_SITE_SURVEY%s.%x.tmp", "5G", (unsigned int) CWThreadSelf());
        break;
#if SUPPORT_WLAN_5G_2_SETTING
    case CW_RADIOFREQTYPE_5G_1:
        sprintf(tmpFile, "/tmp/WLAN_SITE_SURVEY%s.%x.tmp", "5G_1", (unsigned int) CWThreadSelf());
        break;
#endif
#endif
    default:
        CWDebugLog("Bad Radio Frequency Type");
	return CW_TRUE;
    }

#if defined(CW_BOARD_EWS310AP) || defined(ATMWS_SERIES)
    if (radioType == CW_RADIOFREQTYPE_5G)
    {
        CWSystem("echo 1 >  /proc/sys/dev/%s/isscanning", 
                 WIFI_IF_NAME(radioIdx, int32WlanIdx));
    }
#endif

#if defined(CW_BOARD_EWS310AP) || defined(ATMWS_SERIES)
    CWSystem("iwlist %s scanning fast | tail -n +3 > %s",
             WLAN_IF_NAME(radioIdx, int32WlanIdx), tmpFile);
#else
    CWSystem("iwlist %s scanning | tail -n +3 > %s",
             WLAN_IF_NAME(radioIdx, int32WlanIdx), tmpFile);
#endif /* (CW_BOARD_EWS310AP) || (ATMWS_SERIES) */

#if defined(CW_BOARD_EWS310AP) || defined(ATMWS_SERIES)
    if (radioType == CW_RADIOFREQTYPE_5G)
    {
        CWSystem("echo 0 >  /proc/sys/dev/%s/isscanning", 
                 WIFI_IF_NAME(radioIdx, int32WlanIdx));
    }
#endif

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    *version = 0;

    fp = fopen(tmpFile, "r");
    if(!fp)
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        return CW_TRUE;
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }

    if(fsize < 0)
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(!fsize)
    {
        *count = 0;
        *sitesurvey = NULL;
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }

    CW_CREATE_OBJECT_SIZE_ERR(buffer, fsize + 1,
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    });

    if(fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CW_TRUE;
    }


    buffer[fsize] = '\0';
    *count = 0;
    c = buffer;
    while(*c != '\0')
    {
        if(*c == '\n')
        {
            (*count)++;
        }
        c++;
    }
    CW_FREE_OBJECT(buffer);

    CW_CREATE_ARRAY_ERR(*sitesurvey, *count, CWWTPSitesurvey,
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    });

    char tmpstr[256];
    char *pch, *ptr;
    i = 0;
    while(fgets(tmpstr, sizeof(tmpstr), fp) != NULL && i < *count)
    {
        //BSSID
        pch = strtok_r(tmpstr, "\t", &ptr);
        if(sscanf(pch, "%x:%x:%x:%x:%x:%x",
                  &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5])
           != 6)
        {
            CW_FREE_OBJECT(*sitesurvey);
            fclose(fp);
            unlink(tmpFile);
            CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
            return CW_TRUE;
        }
        else
        {
            (*sitesurvey)[i].bssid[0] = (unsigned char) bssid[0];
            (*sitesurvey)[i].bssid[1] = (unsigned char) bssid[1];
            (*sitesurvey)[i].bssid[2] = (unsigned char) bssid[2];
            (*sitesurvey)[i].bssid[3] = (unsigned char) bssid[3];
            (*sitesurvey)[i].bssid[4] = (unsigned char) bssid[4];
            (*sitesurvey)[i].bssid[5] = (unsigned char) bssid[5];
        }
        CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, (*sitesurvey)[i].bssid);
        //SSID
        pch = strtok_r(NULL, "\t", &ptr);
        strcpy((*sitesurvey)[i].ssid, pch);
        CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, (*sitesurvey)[i].ssid);
        //LEN
        pch = strtok_r(NULL, "\t", &ptr);
        (*sitesurvey)[i].ssidLen = atoi(pch);
        CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, (*sitesurvey)[i].ssidLen);
        //MODE
        pch = strtok_r(NULL, "\t", &ptr);
        if(!strcmp(pch, "Master"))
        {
            (*sitesurvey)[i].mode = CW_RADIO_OPERATION_MODE_AP;
        }
        else if(!strcmp(pch, " Ad-Hoc"))
        {
            (*sitesurvey)[i].mode = CW_RADIO_OPERATION_MODE_AD_HOC;
        }
        else
        {
            (*sitesurvey)[i].mode = CW_RADIO_OPERATION_MODE_DISABLED;
        }
        CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, (*sitesurvey)[i].mode);
        //CH
        pch = strtok_r(NULL, "\t", &ptr);
        (*sitesurvey)[i].chan = atoi(pch);
        CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, (*sitesurvey)[i].chan);
        //SIGNAL
        pch = strtok_r(NULL, "\t", &ptr);
        (*sitesurvey)[i].signal = atoi(pch);
        CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, (*sitesurvey)[i].signal);
        //ENC
        pch = strtok_r(NULL, "\t", &ptr);
        if(strstr(pch, "OPEN"))
        {
            (*sitesurvey)[i].enc = CW_WLAN_SECURITY_MODE_NONE;
        }
        else if(strstr(pch, "WEP"))
        {
            (*sitesurvey)[i].enc = CW_WLAN_SECURITY_MODE_WEP;
        }
        else if(strstr(pch, "WPA/WPA2-PSK"))
        {
            (*sitesurvey)[i].enc = CW_WLAN_SECURITY_MODE_WPA_PSK_MIXED;
        }
        else if(strstr(pch, "WPA2-PSK"))
        {
            (*sitesurvey)[i].enc = CW_WLAN_SECURITY_MODE_WPA2_PSK;
        }
        else if(strstr(pch, "WPA-PSK"))
        {
            (*sitesurvey)[i].enc = CW_WLAN_SECURITY_MODE_WPA_PSK;
        }
        else if(strstr(pch, "WPA/WPA2"))
        {
            (*sitesurvey)[i].enc = CW_WLAN_SECURITY_MODE_WPA_MIXED;
        }
        else if(strstr(pch, "WPA2"))
        {
            (*sitesurvey)[i].enc = CW_WLAN_SECURITY_MODE_WPA2;
        }
        else if(strstr(pch, "WPA"))
        {
            (*sitesurvey)[i].enc = CW_WLAN_SECURITY_MODE_WPA;
        }
        else
        {
            CWDebugLog("unknown sitesurvey security mode");
            (*sitesurvey)[i].enc = CW_WLAN_SECURITY_MODE_NONE;
        }
        CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, (*sitesurvey)[i].enc);
        //TYPE
        pch = strtok_r(NULL, "\t", &ptr);
        if(!strcmp(pch, "11a"))
        {
            (*sitesurvey)[i].type = CW_RADIO_WIRELESS_MODE_A;
        }
        else if(!strcmp(pch, "11a/n"))
        {
            (*sitesurvey)[i].type = CW_RADIO_WIRELESS_MODE_AN;
        }
        else if(!strcmp(pch, "11b"))
        {
            (*sitesurvey)[i].type = CW_RADIO_WIRELESS_MODE_B;
        }
        else if(!strcmp(pch, "11b/g"))
        {
            (*sitesurvey)[i].type = CW_RADIO_WIRELESS_MODE_BG;
        }
        else if(!strcmp(pch, "11g/n"))
        {
            (*sitesurvey)[i].type = CW_RADIO_WIRELESS_MODE_BGN;
        }
        else if(!strcmp(pch, "11ac"))
        {
            (*sitesurvey)[i].type = CW_RADIO_WIRELESS_MODE_ACN_5G;
        }
        else
        {
            (*sitesurvey)[i].type = 0;
        }
        i++;
        CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, (*sitesurvey)[i].type);
    }

    fclose(fp);
    unlink(tmpFile);

    return CW_TRUE;
}

CWBool CWWTPBoardGetSitesurveySecurity(int flag, int *tag)
{
    int security = 0;

    if ((flag & STD_OPN) == STD_OPN)
        security = CW_WLAN_SECURITY_MODE_NONE;
    else if ((flag & STD_WEP) == STD_WEP)
        security = CW_WLAN_SECURITY_MODE_WEP;
    else if ((flag & (STD_WPA2|STD_WPA|AUTH_PSK)) == (STD_WPA2|STD_WPA|AUTH_PSK))
        security = CW_WLAN_SECURITY_MODE_WPA_PSK_MIXED;
    else if ((flag & (STD_WPA2|STD_WPA)) == (STD_WPA2|STD_WPA))
        security = CW_WLAN_SECURITY_MODE_WPA_MIXED;
    else if ((flag & (STD_WPA2|AUTH_PSK)) == (STD_WPA2|AUTH_PSK))
        security = CW_WLAN_SECURITY_MODE_WPA2_PSK;
    else if ((flag & STD_WPA2) == STD_WPA2)
        security = CW_WLAN_SECURITY_MODE_WPA2;
    else if ((flag & (STD_WPA|AUTH_PSK)) == (STD_WPA|AUTH_PSK))
        security = CW_WLAN_SECURITY_MODE_WPA_PSK;
    else if ((flag & STD_WPA) == STD_WPA)
        security = CW_WLAN_SECURITY_MODE_WPA;
    else
    {
        security = CW_WLAN_SECURITY_MODE_NONE;
        CWDebugLog("unknown sitesurvey security mode");
    }

    *tag = security;

    //CWDebugLog("flag:[%d], security:[%d], tag:[%d]", flag, security, *tag);

    return CW_TRUE;
}

CWBool CWWTPBoardGetSitesurveyAirodump(CWRadioFreqType radioType, unsigned char *version, int *count, CWWTPSitesurvey **sitesurvey)
{
    int i, num = 0, security = 0, len = 0, flag = 0;
    char tmpFile[64] = {0}, airodumpFile[64] = {0};
    FILE *fp;
    long fsize;
    char *buffer, *c;
    unsigned int bssid[6];
    char tmpstr[256] = {0};
    char *pch, *ptr, *pval, *ptr2 = NULL;
    char *privacy, *cipher, *authentication;
    unsigned int uint32CapCode;
    int int32RadioIdx, radioIdx = 0;

    if(!CWWTPBoardGetCapCode(&uint32CapCode))
    {
        CWDebugLog("%s %d Get capCode fail", __func__, __LINE__);
        return CW_FALSE;
    }

    if(!CWGetRadioIndex(radioType, uint32CapCode, &int32RadioIdx))
    {
        CWDebugLog("%s %d Get radioIdx fail", __func__, __LINE__);
        return CW_FALSE;
    }

    radioIdx = CWConvertRadioIdx(int32RadioIdx);

    if (snprintf(airodumpFile, sizeof(airodumpFile), "/tmp/Background_Scanning/wifi%d_ap_list.txt", radioIdx) < 0)
    {
        CWDebugLog("%s %d File \"%s\" is not exist", __func__, __LINE__, airodumpFile);
        return CW_FALSE;
    }

    if (access(airodumpFile, R_OK) != 0)
    {
        CWDebugLog("%s %d File \"%s\" is not exist", __func__, __LINE__, airodumpFile);
        return CW_FALSE;
    }

    switch (radioType)
    {
#if SUPPORT_WLAN_24G_SETTING
        case CW_RADIOFREQTYPE_2G:
            sprintf(tmpFile, "/tmp/WLAN_SITE_SURVEY%s.%x.tmp", "2G", (unsigned int) CWThreadSelf());
            break;
#endif
#if SUPPORT_WLAN_5G_SETTING
        case CW_RADIOFREQTYPE_5G:
            sprintf(tmpFile, "/tmp/WLAN_SITE_SURVEY%s.%x.tmp", "5G", (unsigned int) CWThreadSelf());
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case CW_RADIOFREQTYPE_5G_1:
            sprintf(tmpFile, "/tmp/WLAN_SITE_SURVEY%s.%x.tmp", "5G_1", (unsigned int) CWThreadSelf());
            break;
#endif
#endif
        default:
            CWDebugLog("Bad Radio Frequency Type");
            return CW_TRUE;
    }

    CWSystem("tr -d '\\r' < %s > %s", airodumpFile, tmpFile);

    if (access(tmpFile, R_OK) != 0)
    {
        CWDebugLog("File \"%s\" is not exist", tmpFile);
        return CW_FALSE;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    *version = 0;

    fp = fopen(tmpFile, "r");

    if (!fp)
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        return CW_TRUE;
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }

    if (fsize < 0)
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if (!fsize)
    {
        *count = 0;
        *sitesurvey = NULL;
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }

    CW_CREATE_OBJECT_SIZE_ERR(buffer, fsize + 1,
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    });

    if (fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CW_TRUE;
    }

    buffer[fsize] = '\0';
    *count = 0;
    c = buffer;
    while (*c != '\0')
    {
        if (*c == '\n' && *(c+1) != '\n')
        {
            (*count)++;
        }
        c++;
    }
    CW_FREE_OBJECT(buffer);

    CW_CREATE_ARRAY_ERR(*sitesurvey, *count, CWWTPSitesurvey,
    {
        fclose(fp);
        unlink(tmpFile);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    i = 0;
    while (fgets(tmpstr, sizeof(tmpstr), fp) != NULL && i < *count)
    {
        //CWDebugLog("%s %d length:[%d] tmpstr:[%s]", __func__, __LINE__, strlen(tmpstr), tmpstr);

        if (strlen(tmpstr) < 17)
        {
            (*count)--;
            break;
        }
        num = 0;
        flag = 0;
        security = 0;
        len = strcspn(tmpstr,"\n");
        tmpstr[len] = '\0';

        pch = strtok_r(tmpstr, ",", &ptr);

        while (pch != NULL) 
        {
            pval = ltrim(pch);
            switch(num)
            {
                case 0:
                    // BSSID
                    if(sscanf(pch, "%x:%x:%x:%x:%x:%x", &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5]) != 6)
                    {
                        CW_FREE_OBJECT(*sitesurvey);
                        fclose(fp);
                        unlink(tmpFile);
                        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
                        return CW_TRUE;
                    }
                    else
                    {
                        (*sitesurvey)[i].bssid[0] = (unsigned char) bssid[0];
                        (*sitesurvey)[i].bssid[1] = (unsigned char) bssid[1];
                        (*sitesurvey)[i].bssid[2] = (unsigned char) bssid[2];
                        (*sitesurvey)[i].bssid[3] = (unsigned char) bssid[3];
                        (*sitesurvey)[i].bssid[4] = (unsigned char) bssid[4];
                        (*sitesurvey)[i].bssid[5] = (unsigned char) bssid[5];
                    }
                    break;
                case 3:
                    // Chan
                    (*sitesurvey)[i].chan = atoi(pval);
                    break;
                case 5:
                    // Privacy
                    privacy = strtok_r(pval, " ", &ptr2);
                    while (privacy != NULL) {
                        if (!strcmp(privacy, "WPA2"))
                            flag |= STD_WPA2;
                        else if (!strcmp(privacy, "WPA"))
                            flag |= STD_WPA;
                        else if (!strcmp(privacy, "WEP"))
                            flag |= STD_WEP;
                        else if (!strcmp(privacy, "OPN"))
                            flag |= STD_OPN;
                        privacy = strtok_r(NULL, " ", &ptr2);
                    }
                    break;
                case 6:
                    // Cipher
                    cipher = strtok_r(pval, " ", &ptr2);
                    while (cipher != NULL) {
                        if (!strcmp(pval, "WEP104"))
                            flag |= ENC_WEP104;
                        else if (!strcmp(pval, "WEP40"))
                            flag |= ENC_WEP40;
                        else if (!strcmp(pval, "CCMP"))
                            flag |= ENC_CCMP;
                        else if (!strcmp(pval, "WRAP"))
                            flag |= ENC_WRAP;
                        else if (!strcmp(pval, "TKIP"))
                            flag |= ENC_TKIP;
                        else if (!strcmp(pval, "WEP"))
                            flag |= ENC_WEP;
                        cipher = strtok_r(NULL, " ", &ptr2);
                    }
                    break;
                case 7:
                    // Authentication
                    authentication = strtok_r(pval, " ", &ptr2);
                    while (authentication != NULL) {
                        if (!strcmp(authentication, "MGT"))
                            flag |= AUTH_MGT;
                        else if (!strcmp(authentication, "PSK"))
                            flag |= AUTH_PSK;
                        else if (!strcmp(authentication, "ASK"))
                            flag |= AUTH_PSK;
                        else if (!strcmp(authentication, "OPN"))
                            flag |= AUTH_OPN;
                        authentication = strtok_r(NULL, " ", &ptr2);
                    }
                    break;
                case 8:
                    (*sitesurvey)[i].signal = atoi(pval);
                    break;
                case 12:
                    // Length
                    (*sitesurvey)[i].ssidLen = atoi(pval);
                    break;
                case 13:
                    // ESSID
                    strcpy((*sitesurvey)[i].ssid, pval);
                    break;
                case 15:
                    // Bandwidth
                    break;
                case 16:
                    // Mode
                    if(!strcmp(pval, "A"))
                        (*sitesurvey)[i].type = CW_RADIO_WIRELESS_MODE_A;
                    else if(!strcmp(pval, "AN"))
                        (*sitesurvey)[i].type = CW_RADIO_WIRELESS_MODE_AN;
#if 0
                    else if(!strcmp(pval, "11b"))
                        (*sitesurvey)[i].type = CW_RADIO_WIRELESS_MODE_B;
#endif
                    else if(!strcmp(pval, "BG"))
                        (*sitesurvey)[i].type = CW_RADIO_WIRELESS_MODE_BG;
                    else if(!strcmp(pval, "BGN"))
                        (*sitesurvey)[i].type = CW_RADIO_WIRELESS_MODE_BGN;
                    else if(!strcmp(pval, "AC"))
                        (*sitesurvey)[i].type = CW_RADIO_WIRELESS_MODE_ACN_5G;
#if 0
                    else if(!strcmp(pval, "11ax"))
                        (*sitesurvey)[i].type = CW_RADIO_WIRELESS_MODE_AX;
#endif
                    else
                        (*sitesurvey)[i].type = 0;
                    break;
                case 1:
                case 2:
                case 4:
                case 9:
                case 10:
                case 11:
                case 14:
                default:
                    break;
            }
            num++;
            pch = strtok_r(NULL, ",", &ptr);
        }
        CWWTPBoardGetSitesurveySecurity(flag, &security);
        (*sitesurvey)[i].enc = security;
        (*sitesurvey)[i].mode = CW_RADIO_OPERATION_MODE_AP;
        i++;
    }

    fclose(fp);
    unlink(tmpFile);

    return CW_TRUE;
}

CWBool CWWTPBoardGetSitesurvey(CWRadioFreqType radioType, unsigned char *version, int *count, CWWTPSitesurvey **sitesurvey)
{
    int backgroundscanEnable = 0;
#if SUPPORT_WLAN_5G_2_SETTING
    int radioIdx = 0, int32RadioIdx = 0;
#endif
    unsigned int uint32CapCode;

    if(!CWWTPBoardGetCapCode(&uint32CapCode))
    {
        CWDebugLog("%s %d Get capCode fail", __func__, __LINE__);
        return CW_FALSE;
    }


#if SUPPORT_WLAN_5G_SETTING
#if SUPPORT_WLAN_5G_2_SETTING
    if(!CWGetRadioIndex(radioType, uint32CapCode, &int32RadioIdx))
    {
        CWDebugLog("%s %d Get radioIdx fail", __func__, __LINE__);
        return CW_FALSE;
    }

    radioIdx = CWConvertRadioIdx(int32RadioIdx);

    if (CWWTPBoardGetRadioNoChannel(radioIdx) == CW_TRUE)
    {
        *count = 0;
        *sitesurvey = NULL;
        CWDebugLog("%s %d radioIdx:[%d] noChannel:[%d]", __FUNCTION__, __LINE__, radioIdx, CWWTPBoardGetRadioNoChannel(radioIdx));
        return CW_TRUE;
    }
#endif
#endif

    switch (radioType) 
    {
#if SUPPORT_WLAN_24G_SETTING
    case CW_RADIOFREQTYPE_2G:
        api_get_integer_option("wireless.wifi0.backgroundscanEnable", &backgroundscanEnable);
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case CW_RADIOFREQTYPE_5G:
        api_get_integer_option("wireless.wifi1.backgroundscanEnable", &backgroundscanEnable);
        break;
#if SUPPORT_WLAN_5G_2_SETTING
    case CW_RADIOFREQTYPE_5G_1:
        api_get_integer_option("wireless.wifi2.backgroundscanEnable", &backgroundscanEnable);
        break;
#endif
#endif
    default:
        backgroundscanEnable = 0;
        break;
    }
    
    if (backgroundscanEnable == 1) 
    {
        CWDebugLog("--- Get Sitesurvey By Airodump ---");
        return CWWTPBoardGetSitesurveyAirodump(radioType, version, count, sitesurvey);
    }
    else 
    {
        CWDebugLog("--- Get Sitesurvey By Iwlist ---");
        return CWWTPBoardGetSitesurveyIwlist(radioType, version, count, sitesurvey);
    }
}
//NOT_FINISH (NOT SUPPORT)
CWBool CWWTPBoardSetRadioAutoTxPower(CWRadioFreqType radioType, int int32HealingTxpower)
{
    unsigned int uint32CapCode;
    int int32RadioIdx, int32WlanIdx, int32SsidEnable = 0, int32MaxTxpower, int32Txpower = 0;

    switch (radioType)
    {
#if SUPPORT_WLAN_24G_SETTING
    case CW_RADIOFREQTYPE_2G:
        CWDebugLog("WTP auto txpower request radiotype: %s healing txpower: %d%%", "2G", int32HealingTxpower);
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case CW_RADIOFREQTYPE_5G:
        CWDebugLog("WTP auto txpower request radiotype: %s healing txpower: %d%%", "5G", int32HealingTxpower);
        break;
#endif
#if SUPPORT_WLAN_5G_2_SETTING
    case CW_RADIOFREQTYPE_5G_1:
        CWDebugLog("WTP auto txpower request radiotype: %s healing txpower: %d%%", "5G_1", int32HealingTxpower);
        break;
#endif
    default:
        CWDebugLog("Bad Radio Frequency Type");
        return CW_TRUE;
    }


    if(!CWWTPBoardGetCapCode(&uint32CapCode))
    {
        CWDebugLog("Set configuration fail");
        return CW_TRUE;
    }

    if(!CWGetRadioIndex(radioType, uint32CapCode, &int32RadioIdx))
    {
        CWDebugLog("Set configuration fail");
        return CW_TRUE;
    }

#if SUPPORT_WLAN_5G_SETTING
#if SUPPORT_WLAN_5G_2_SETTING
    if (CWWTPBoardGetRadioNoChannel(int32RadioIdx) == CW_TRUE)
    {
        CWDebugLog("%s %d radioIdx:[%d] noChannel:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(int32RadioIdx), CWWTPBoardGetRadioNoChannel(int32RadioIdx));
        return CW_TRUE;
    }
#endif
#endif

    if(!CWWTPBoardGetRadioMaxTxPower(int32RadioIdx, &int32MaxTxpower))
    {
        CWDebugLog("Set configuration fail");
        return CW_TRUE;
    }

    if(!CWGetHealthyTxPowerValue(int32MaxTxpower, int32HealingTxpower, &int32Txpower))
    {
        CWDebugLog("Set configuration fail");
        return CW_TRUE;
    }

    for(int32WlanIdx = 0; int32WlanIdx < CWWTPBoardGetMaxRadioWlans(int32RadioIdx); int32WlanIdx++)
    {
        if(CWWTPBoardGetWlanEnableCfg(int32RadioIdx, int32WlanIdx, &int32SsidEnable))
        {
            if(int32SsidEnable)
            {
                CWSystem("iwconfig %s txpower %d", WLAN_IF_NAME(CWConvertRadioIdx(int32RadioIdx), int32WlanIdx), int32Txpower);
            }
        }
    }
    return CW_TRUE;
}

CWBool CWWTPBoardKickmac(CWWTPKickmacInfo *kicks)
{
    int i = 0;
    char *section = NULL;
    char str[API_MAC_SIZE];
    char ifname[8];

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx((CWRadioFreqType) kicks->radio));

    if (api_get_wifi_ifacex_ifname_option_by_sectionname(OPM_AP, section, (kicks->wlan)+1, ifname, sizeof(ifname)))
    {
        if (section != NULL) CW_FREE_OBJECT(section);
        return CW_TRUE;
    }

    while(i < kicks->clientNum)
    {
        snprintf(str, sizeof(str), "%02x:%02x:%02x:%02x:%02x:%02x", kicks->client[i].mac[0], kicks->client[i].mac[1],
                kicks->client[i].mac[2], kicks->client[i].mac[3], kicks->client[i].mac[4], kicks->client[i].mac[5]);

        CWSystem("iwpriv %s kickmac %s", ifname, str);
        CWDebugLog("iwpriv %s kickmac %s", ifname, str);

        i++;
    }
    if (section != NULL) CW_FREE_OBJECT(section);
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanStations(int radioIdx, int wlanIdx, int *count, CWStation **station)
{
    int i;
    char tmpFile[128];
    FILE *fp;
    long fsize;
    char *buffer, *c;
    unsigned int mac[6];
    int ssid_disabled = 1;
    char sectionstr[128] = {0};

    //initial count, fix wtp crashed in portal + NAT.
    *count = 0;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        // guest ssid skip
        return CW_TRUE;
    }

    // check ssid.disabled
    sprintf(sectionstr, "wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
    if (api_get_wifi_ifacex_disabled_option_by_sectionname(OPM_AP, sectionstr, wlanIdx+1, &ssid_disabled))
    {
        ssid_disabled = 1;
    }
    // ssid.disabled=1 skip
    if (ssid_disabled == 1)
    {
        return CW_TRUE;
    }

    snprintf(tmpFile, sizeof(tmpFile), "/tmp/wtpsta.%x.tmp", (unsigned int) CWThreadSelf());

    CWSystem("wlanconfig %s list sta 2> /dev/null | grep : | awk '{print $1%s$7%s$8%s$6}' > %s",
             WLAN_IF_NAME(CWConvertRadioIdx(radioIdx), wlanIdx), "\"\\t\"", "\"\\t\"", "\"\\t\"", tmpFile);

    fp = fopen(tmpFile, "r");

    if(!fp)
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        return CW_TRUE;
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }

    if(fsize < 0)
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(!fsize)
    {
        *count = 0;
        *station = NULL;
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }

    CW_CREATE_OBJECT_SIZE_ERR(buffer, fsize + 1,
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    });

    if(fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    fclose(fp);
    unlink(tmpFile);

    buffer[fsize] = '\0';

    *count = 0;

    if(strstr(buffer, "Error") != NULL) 
    {
        CW_FREE_OBJECT(buffer);
        //CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    
    c = buffer;
    while(*c != '\0')
    {
        if(*c == '\n')
        {
            (*count)++;
        }
        c++;
    }

    CW_CREATE_ARRAY_ERR(*station, *count, CWStation,
    {
        CW_FREE_OBJECT(buffer);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    });

    i = 0;
    c = buffer;
    while(*c != '\0')
    {
        if(sscanf(c, "%x:%x:%x:%x:%x:%x\t%uKb\t%uKb\t%d",
                  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5],
                  &((*station)[i].txKB),
                  &((*station)[i].rxKB),
                  &((*station)[i].rssi)) != 9)
        {
            CW_FREE_OBJECT(*station);
            CW_FREE_OBJECT(buffer);
            CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
            return CW_TRUE;
        }
        (*station)[i].mac[0] = (unsigned char) mac[0];
        (*station)[i].mac[1] = (unsigned char) mac[1];
        (*station)[i].mac[2] = (unsigned char) mac[2];
        (*station)[i].mac[3] = (unsigned char) mac[3];
        (*station)[i].mac[4] = (unsigned char) mac[4];
        (*station)[i].mac[5] = (unsigned char) mac[5];
        (*station)[i].osTypeLen = 0;
        CW_CREATE_STRING_FROM_STRING_ERR((*station)[i].osType, "",
        {
            CW_FREE_OBJECT(buffer);
            CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
            return CW_TRUE;
        });
        (*station)[i].hostNameLen = 0;
        CW_CREATE_STRING_FROM_STRING_ERR((*station)[i].hostName, "",
        {
            CW_FREE_OBJECT(buffer);
            CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
            return CW_TRUE;
        });
        (*station)[i].address = 0;

        i++;

        while(*c != '\n')
        {
            c++;
        }
        c++;
    }
    CW_FREE_OBJECT(buffer);

    // fingerprint
    memset(tmpFile, 0, sizeof(tmpFile));
    sprintf(tmpFile, "/tmp/fingerprint_status_list_%s.%x.tmp",
            WLAN_IF_NAME(CWConvertRadioIdx(radioIdx), wlanIdx), (unsigned int) CWThreadSelf());

    CWSystem("cp -a /tmp/fingerprint_status_list_%s %s", WLAN_IF_NAME(CWConvertRadioIdx(radioIdx), wlanIdx), tmpFile);

    fp = fopen(tmpFile, "r");
    if(!fp)
    {
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        return CW_TRUE;
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }

    if(fsize < 0)
    {
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }

    if(!fsize)
    {
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }

    CW_CREATE_OBJECT_SIZE_ERR(buffer, fsize + 1,
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    });

    if(fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    fclose(fp);
    unlink(tmpFile);

    buffer[fsize] = '\0';

    // parse
    char osType[128] = {0};
    char hostName[128] = {0};
    char ip[15] = {0};
    char cmd[256] = {0};

    c = buffer;
    while(*c != '\0')
    {
        memset(osType, 0, sizeof(osType));
        memset(hostName, 0, sizeof(hostName));
        memset(ip, 0, sizeof(ip));
        memset(cmd, 0, sizeof(cmd));

        sscanf(c, "%x:%x:%x:%x:%x:%x|%[^|]|%[^|]|%[^\n]",
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5], ip, osType, hostName);

        for(i = 0; i < *count; i++)
        {
            if((*station)[i].mac[0] == (unsigned char) mac[0] &&
               (*station)[i].mac[1] == (unsigned char) mac[1] &&
               (*station)[i].mac[2] == (unsigned char) mac[2] &&
               (*station)[i].mac[3] == (unsigned char) mac[3] &&
               (*station)[i].mac[4] == (unsigned char) mac[4] &&
               (*station)[i].mac[5] == (unsigned char) mac[5])
            {
                CW_FREE_OBJECT((*station)[i].osType);
                (*station)[i].osTypeLen = strlen(osType);
                CW_CREATE_STRING_FROM_STRING_ERR((*station)[i].osType, osType,
                {
                    CW_FREE_OBJECT(buffer);
                    CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
                    return CW_TRUE;
                });

                CW_FREE_OBJECT((*station)[i].hostName);
                (*station)[i].hostNameLen = strlen(hostName);
                CW_CREATE_STRING_FROM_STRING_ERR((*station)[i].hostName, hostName,
                {
                    CW_FREE_OBJECT(buffer);
                    CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
                    return CW_TRUE;
                });

                if(ip[0] == '\0')
                {
                    (*station)[i].address = 0;
                }
                else
                {
                    if(strcmp(ip, "0.0.0.0") == 0)
                    {
                        sprintf(cmd, "finger_syncli send request %02x%02x%02x%02x%02x%02x update &", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                        CWSystem(cmd);
                    }
                    (*station)[i].address = inet_addr(ip);
                }
                break;
            }
        }

        while(*c != '\n')
        {
            c++;
        }
        c++;
    }

    CW_FREE_OBJECT(buffer);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanStatistics(int radioIdx, int wlanIdx, CWWlanStatistics *statistics)
{
    char tmpFile[32];
    FILE *fp;
    long fsize;
    char *buffer, *c, ifname[8];
    char *section = NULL;

    if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_get_wifi_guest_ifname_option(CWConvertRadioIdx(radioIdx), ifname, sizeof(ifname)))
        {
            return CW_FALSE;
        }
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_ifname_option_by_sectionname(OPM_AP, section, wlanIdx+1, ifname, sizeof(ifname)))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    sprintf(tmpFile, "/tmp/wtpstat.%x.tmp", (unsigned int) CWThreadSelf());

    CWSystem("ifconfig %s 2> /dev/null > %s", ifname, tmpFile);

    fp = fopen(tmpFile, "r");
    if(!fp)
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        return CW_TRUE;
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }
    if(fsize < 0)
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(!fsize)
    {
        CW_ZERO_MEMORY(statistics, sizeof(CWWlanStatistics));
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }

    CW_CREATE_OBJECT_SIZE_ERR(buffer, fsize + 1,
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    });

    if(fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    fclose(fp);
    unlink(tmpFile);

    buffer[fsize] = '\0';

    c = strstr(buffer, "RX packets:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    c += strlen("RX packets:");
    statistics->rxPkts = strtoul(c, NULL, 10);

    c = strstr(c, "errors:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    c += strlen("errors:");
    statistics->rxErrPkts = strtoul(c, NULL, 10);

    c = strstr(c, "dropped:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    c += strlen("dropped:");
    statistics->rxDrpPkts = strtoul(c, NULL, 10);

    c = strstr(c, "TX packets:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    c += strlen("TX packets:");
    statistics->txPkts = strtoul(c, NULL, 10);

    c = strstr(c, "errors:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    c += strlen("errors:");
    statistics->txErrPkts = strtoul(c, NULL, 10);

    c = strstr(c, "dropped:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    c += strlen("dropped:");
    statistics->txDrpPkts = strtoul(c, NULL, 10);

    c = strstr(c, "RX bytes:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    c += strlen("RX bytes:");
    statistics->rxBytes = strtoul(c, NULL, 10);

    c = strstr(c, "TX bytes:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    c += strlen("TX bytes:");
    statistics->txBytes = strtoul(c, NULL, 10);

    CW_FREE_OBJECT(buffer);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanAclModeCfg(int radioIdx, int wlanIdx, int *mode)
{
    char *section = NULL;
    int aclmode = 0;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_get_wifi_guest_macfilter_option(CWConvertRadioIdx(radioIdx), &aclmode))
        {
            aclmode = 0;
        }
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_macfilter_option_by_sectionname(OPM_AP, section, wlanIdx+1, &aclmode))
        {
            aclmode = 0;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    *mode = aclmode;

    CWDebugLog("%s %d radioIdx:%d wlanIdx:%d mode:%d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *mode);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanAclModeCfg(int radioIdx, int wlanIdx, int mode)
{
    int aclmode=0;
    char val[18*MAC_FILTER_NUM+1]={0};
    char section[32]={0};
    char option[64]={0};

    CWDebugLog("%s %d radioIdx:%d wlanIdx:%d mode:%d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, mode);

    if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_get_wifi_guest_macfilter_option(CWConvertRadioIdx(radioIdx), &aclmode))
        {
            aclmode = 0;
        }
        if (aclmode != mode)
        {
            switch (mode)
            {
                case 1:
                    api_get_wifi_guest_denymaclist_option(CWConvertRadioIdx(radioIdx), val, sizeof(val));
                    if (val[0] != '\0')
                    {
                        memset(option, 0, sizeof(option));
                        snprintf(option, sizeof(option), "wireless.wifi%d_guest.%s", CWConvertRadioIdx(radioIdx), "allowmaclist");
                        api_add_wifi_iface_maclist_option(option, val, strlen(val));
                    }
                    memset(option, 0, sizeof(option));
                    snprintf(option, sizeof(option), "wireless.wifi%d_guest.%s", CWConvertRadioIdx(radioIdx), "denymaclist");
                    api_delete_wifi_iface_maclist_option(option, 0);
                    break;
                case 2:
                    api_get_wifi_guest_allowmaclist_option(CWConvertRadioIdx(radioIdx), val, sizeof(val));
                    if (val[0] != '\0')
                    {
                        memset(option, 0, sizeof(option));
                        snprintf(option, sizeof(option), "wireless.wifi%d_guest.%s", CWConvertRadioIdx(radioIdx), "denymaclist");
                        api_add_wifi_iface_maclist_option(option, val, strlen(val));
                    }
                    memset(option, 0, sizeof(option));
                    snprintf(option, sizeof(option), "wireless.wifi%d_guest.%s", CWConvertRadioIdx(radioIdx), "allowmaclist");
                    api_delete_wifi_iface_maclist_option(option, 0);
                    break;
                default:
                    break;
            }
        }
        if (api_set_wifi_guest_macfilter_option(CWConvertRadioIdx(radioIdx), mode))
        {
            return CW_FALSE;
        }
    }
    else
    {
        if (CWConvertRadioIdx(radioIdx) > 0 && (RADIO_MODE != 5))
        {
            CWDebugLog("%s %d radioIdx:%d wlanIdx:%d synchronize with 2.4Ghz", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);
            return CW_TRUE;
        }

        snprintf(section, sizeof(section), "wireless.wifi%d_ssid", CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_macfilter_option_by_sectionname(OPM_AP, section, wlanIdx+1, &aclmode))
        {
            aclmode = 0;
        }
        if (aclmode != mode)
        {
            switch (mode)
            {
                case 1:
                    api_get_wifi_ifacex_denymaclist_option_by_sectionname(OPM_AP, section, wlanIdx+1, val, sizeof(val));
                    if (val[0] != '\0')
                        api_add_wifi_ifacex_allowmaclist_option_by_sectionname(OPM_AP, section, wlanIdx+1, val, strlen(val));
                    api_delete_wifi_ifacex_denymaclist_option_by_sectionname(OPM_AP, section, wlanIdx+1, 0);
                    break;
                case 2:
                    api_get_wifi_ifacex_allowmaclist_option_by_sectionname(OPM_AP, section, wlanIdx+1, val, sizeof(val));
                    if (val[0] != '\0')
                        api_add_wifi_ifacex_denymaclist_option_by_sectionname(OPM_AP, section, wlanIdx+1, val, strlen(val));
                    api_delete_wifi_ifacex_allowmaclist_option_by_sectionname(OPM_AP, section, wlanIdx+1, 0);
                    break;
                default:
                    break;
            }
        }
        if (api_set_wifi_ifacex_macfilter_option_by_sectionname(OPM_AP, section, wlanIdx+1, mode))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanAclMacListCfg(int radioIdx, int wlanIdx, int *count, CWMacAddress **macs)
{
    char *c;
    int i, len, aclmode;
    unsigned int mac[6];
    char val[18*MAC_FILTER_NUM+1];
    char section[32]={0};

    CWDebugLog("%s %d radioIdx:%d wlanIdx:%d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);

    memset(val, 0, sizeof(val));

    if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_get_wifi_guest_macfilter_option(CWConvertRadioIdx(radioIdx), &aclmode))
        {
            aclmode = 0;
        }
        switch (aclmode)
        {
            case 1:
                api_get_wifi_guest_allowmaclist_option(CWConvertRadioIdx(radioIdx), val, sizeof(val));
                break;
            case 2:
                api_get_wifi_guest_denymaclist_option(CWConvertRadioIdx(radioIdx), val, sizeof(val));
                break;
            default:
                break;
        }
    }
    else
    {
        snprintf(section, sizeof(section), "wireless.wifi%d_ssid", CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_macfilter_option_by_sectionname(OPM_AP, section, wlanIdx+1, &aclmode))
        {
            aclmode = 0;
        }
        switch (aclmode)
        {
            case 1:
                if (api_get_wifi_ifacex_allowmaclist_option_by_sectionname(OPM_AP, section, wlanIdx+1, val, sizeof(val)))
                {
                    *count = 0;
                    *macs = NULL;
                    return CW_TRUE;
                }
                break;
            case 2:
                if (api_get_wifi_ifacex_denymaclist_option_by_sectionname(OPM_AP, section, wlanIdx+1, val, sizeof(val)))
                {
                    *count = 0;
                    *macs = NULL;
                    return CW_TRUE;
                }
                break;
            default:
                    *count = 0;
                    *macs = NULL;
                    return CW_TRUE;
                break;
        }
    }

    if (val[0] == '\0')
    {
        *count = 0;
        *macs = NULL;
        return CW_TRUE;
    }

    len = strlen(val);
    while(val[len - 1] == ' ')
    {
        val[len - 1] = '\0';
        len--;
    }

    *count = 1;
    c = val;
    while(*c != '\0')
    {
        if (*c == ' ')
        {
            (*count)++;
        }
        c++;
    }

    CW_CREATE_ARRAY_ERR(*macs, *count, CWMacAddress,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    i = 0;
    c = val;
    do
    {
        if (sscanf(c, "%x:%x:%x:%x:%x:%x",
                   &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6)
        {
            CW_FREE_OBJECT(*macs);
            return CWErrorRaise(CW_ERROR_GENERAL, NULL);
        }

        (*macs)[i][0] = (unsigned char) mac[0];
        (*macs)[i][1] = (unsigned char) mac[1];
        (*macs)[i][2] = (unsigned char) mac[2];
        (*macs)[i][3] = (unsigned char) mac[3];
        (*macs)[i][4] = (unsigned char) mac[4];
        (*macs)[i][5] = (unsigned char) mac[5];

        while(*c != ' ' && *c != '\0')
        {
            c++;
        }

        if (*c == '\0')
        {
            break;
        }
        else
        {
            c++;
        }
        i++;
    }
    while(1);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanAclMacListCfg(int radioIdx, int wlanIdx, int count, CWMacAddress *macs)
{
    char *macArray = NULL;
    int i, len, aclmode;
    char section[32]={0}, option[64]={0};
    CWBool retv = CW_TRUE;

    CWDebugLog("%s %d radioIdx:%d wlanIdx:%d count:%d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, count);

    CW_CREATE_OBJECT_SIZE_ERR(macArray, 64 + (count * 18),
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);;
    });

    if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_get_wifi_guest_macfilter_option(CWConvertRadioIdx(radioIdx), &aclmode))
        {
            retv = CW_FALSE;
            goto CWWTPBoardSetWlanAclMacListCfg_End;
        }
        snprintf(option, sizeof(option), "wireless.wifi%d_guest.%s", CWConvertRadioIdx(radioIdx), (aclmode == 1) ? "allowmaclist" : "denymaclist");
    }
    else
    {
        snprintf(section, sizeof(section), "wireless.wifi%d_ssid", CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_macfilter_option_by_sectionname(OPM_AP, section, wlanIdx+1, &aclmode))
        {
            retv = CW_FALSE;
            goto CWWTPBoardSetWlanAclMacListCfg_End;
        }
        snprintf(option, sizeof(option), "wireless.wifi%d_ssid_%d.%s", CWConvertRadioIdx(radioIdx), (wlanIdx+1), (aclmode == 1) ? "allowmaclist" : "denymaclist");
    }

    if (count > 0)
    {
        len = 0;
        i = 0;
        while(i < count)
        {
            sprintf(&macArray[len], "%s%02x:%02x:%02x:%02x:%02x:%02x",
                    i == 0 ? "" : " ", CW_MAC_PRINT_LIST(macs[i]));
            i++;
            len = strlen(macArray);
        }
        api_lower2upper_mac(macArray);
        if (api_set_string_option(option, macArray, (64+(count*18))))
        {
            retv = CW_FALSE;
            goto CWWTPBoardSetWlanAclMacListCfg_End;
        }
    }
    else
    {
        if (api_delete_wifi_iface_maclist_option(option, 0))
        {
            retv = CW_FALSE;
            goto CWWTPBoardSetWlanAclMacListCfg_End;
        }
    }

CWWTPBoardSetWlanAclMacListCfg_End :

    CW_FREE_OBJECT(macArray);

    return retv;
}

CWBool CWWTPBoardGetRadioDataRateCfg(int radioIdx, int *rate)
{
    char val[8], *c, *option;
    int rateIdx = 0;

    memset(val, 0, sizeof(val));

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"rate");

    switch (CWConvertRadioIdx(radioIdx))
    {
#if SUPPORT_WLAN_24G_SETTING
    case 0:
        if (api_get_wifi_rate_option(option, &rateIdx))
        {
            rateIdx = 0;
        }
        if (api_fetch_wifi_datarate_option(rateIdx, val, sizeof(val)))
        {
            memset(val, 0, sizeof(val));
        }
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case 1:
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
#endif
        if (api_get_wifi_5g_rate_option(option, &rateIdx))
        {
            rateIdx = 0;
        }
        if (api_fetch_wifi_5g_datarate_option(rateIdx, val, sizeof(val)))
        {
            memset(val, 0, sizeof(val));
        }
        break;
#endif
    default:
        break;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    CWDebugLog("%s %d val:[%s]", __FUNCTION__, __LINE__, val);

    if(!strcmp("auto", val))
    {
        *rate = CW_RADIO_DATA_RATE_AUTO;
    }
    else if((c = strstr(val, "VHTMCS")))
    {
        *rate = atoi(c + 6) + CW_RADIO_DATA_RATE_MCS_MIN_AC;
    }
    else if((c = strstr(val, "mcs")))
    {
        *rate = atoi(c + 3) + 1;
    }
    else if((c = strstr(val, "M")))
    {
        *rate = atoi(val) * 1000;
        if((c = strstr(val, ".")))
        {
            *rate += atoi(c + 1) * 100;
        }
    }
    else
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    CWDebugLog("%s %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), *rate);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioDataRateCfg(int radioIdx, int rate)
{
    int rateIdx;
    char rateStr[8];
    char *option = NULL;

    CWDebugLog("%s %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), rate);

    if(rate == CW_RADIO_DATA_RATE_AUTO)
    {
        strcpy(rateStr, "auto");
    }
    else if(rate <= CW_RADIO_DATA_RATE_MCS_MAX)
    {
        sprintf(rateStr, "mcs%d", rate - 1);
    }
    else if(rate <= CW_RADIO_DATA_RATE_MCS_MAX_AC)
    {
        sprintf(rateStr, "VHTMCS%d", (rate - CW_RADIO_DATA_RATE_MCS_MIN_AC));
    }
    else if((rate % 1000) < 100)
    {
        sprintf(rateStr, "%dM", rate / 1000);
    }
    else
    {
        sprintf(rateStr, "%d.%dM", rate / 1000, (rate % 1000) / 100);
    }

    CWDebugLog("%s %d %d %d %s", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), rate, rateStr);

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"rate");

    switch (CWConvertRadioIdx(radioIdx))
    {
#if SUPPORT_WLAN_24G_SETTING
    case 0:
        if (api_match_wifi_datarate_option(rateStr, &rateIdx))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            return CW_FALSE;
        }
        if (api_set_wifi_rate_option(option, rateIdx))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            return CW_FALSE;
        }
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case 1:
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
#endif
        if (api_match_wifi_5g_datarate_option(rateStr, &rateIdx))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            return CW_FALSE;
        }
        if (api_set_wifi_5g_rate_option(option, rateIdx))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            return CW_FALSE;
        }
        break;
#endif
    default:
        break;
    }
    if (option != NULL) CW_FREE_OBJECT(option);
    CWDebugLog("%s %d %d %d %s %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), rate, rateStr, rateIdx);

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioRtsCtsThresholdCfg(int radioIdx, int *threshold)
{
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"rts");
    if (api_get_integer_option(option, threshold))
    {
        *threshold = 0;
    }
    if (option != NULL) CW_FREE_OBJECT(option);
    CWDebugLog("%s %d threshold:[%d]", __FUNCTION__, __LINE__, *threshold);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioRtsCtsThresholdCfg(int radioIdx, int threshold)
{
    char *option = NULL;

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), threshold);

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"rts");
    if (api_set_integer_option(option, threshold))
    {
        if (option != NULL) CW_FREE_OBJECT(option);
        return CW_FALSE;
    }
    if (option != NULL) CW_FREE_OBJECT(option);
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadiusAccountingIntermiIntervalCfg(int radioIdx, int wlanIdx, int *interval)
{
    int val = 600;
    char *section = NULL;

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        *interval = 600;
        return CW_TRUE;
    }
#endif

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_get_wifix_guest_acct_interval_option(CWConvertRadioIdx(radioIdx), &val))
        {
            if (api_set_wifix_guest_acct_interval_option(CWConvertRadioIdx(radioIdx), val))
            {
                CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, val);
                return CW_FALSE;
            }
        }
    }
    else
#endif
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_acct_interval_option_by_sectionname(OPM_AP, section, wlanIdx+1, &val))
        {
            if (api_set_wifi_ifacex_acct_interval_option_by_sectionname(OPM_AP, section, wlanIdx+1, val))
            {
                if (section != NULL) CW_FREE_OBJECT(section);
                CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, val);
                return CW_FALSE;
            }
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    *interval = val;
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *interval);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadiusAccountingIntermiIntervalCfg(int radioIdx, int wlanIdx, int interval)
{
    char *section = NULL;

    CWDebugLog("%s %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, interval);

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(interval != 0)
        {
            CWDebugLog("Guest wlan cannot support Radius Accounting Intermi Interval");
        }
        return CW_TRUE;
    }
#endif

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if (api_set_wifix_guest_acct_interval_option(CWConvertRadioIdx(radioIdx), interval))
        {
            return CW_FALSE;
        }
    }
    else
    {
        section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_acct_interval_option_by_sectionname(OPM_AP, section, wlanIdx+1, interval))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
        if (section != NULL) CW_FREE_OBJECT(section);
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioObeyRegulatoryPowerCfg(int radioIdx, int *enable)
{
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"obeyregpower");
    
    if (api_get_integer_option(option, enable))
    {
        *enable = 0;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    CWDebugLog("%s %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioObeyRegulatoryPowerCfg(int radioIdx, int enable)
{
    char *option = NULL;

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), enable);

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"obeyregpower");
    
    if (api_set_integer_option(option, enable))
    {
        if (option != NULL) CW_FREE_OBJECT(option);
        return CW_FALSE;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioBackgroundScanningCfg(int radioIdx, int *enable)
{
    char *option = NULL;

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"backgroundscanEnable");
    
    if (api_get_integer_option(option, enable))
    {
        *enable = 0;
        return CW_FALSE;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    CWDebugLog("%s %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioBackgroundScanningCfg(int radioIdx, int enable)
{
    char *option = NULL;

    CWDebugLog("%s %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), enable);

    option = CWCreateString("wireless.wifi%d.%s",CWConvertRadioIdx(radioIdx),"backgroundscanEnable");
    
#if SUPPORT_CAPWAP_NO_BACKGROUND_SCAN
    if (api_set_integer_option(option, 0))
#else
    if (api_set_integer_option(option, enable))
#endif
    {
        if (option != NULL) CW_FREE_OBJECT(option);
        return CW_FALSE;
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    return CW_TRUE;
}

CWBool CWWTPBoardGetBandSteeringCfg(int *enable)
{
    int val;

    if (api_get_wifi_band_steering_mode_option(WIRELESS_WIFI_BANDSTEER_OPTION, &val))
    {
        val = 0;
    }

    *enable = (val == 0) ? CW_FALSE : CW_TRUE ;

    CWDebugLog("%s %d enable:%d", __FUNCTION__, __LINE__, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetBandSteeringCfg(int enable)
{
    int val = 0;

    CWDebugLog("%s %d enable:%d", __FUNCTION__, __LINE__, enable);

    switch(enable)
    {
        case CW_FALSE:
            val = 0 ;
            break;
        case CW_TRUE:
            val = 2 ;
            break;
    }

    if (api_set_wifi_band_steering_mode_option(WIRELESS_WIFI_BANDSTEER_OPTION, val))
    {
        return CW_FALSE;
    }
#if SUPPORT_WLAN_5G_SETTING
    if (api_set_wifi_band_steering_mode_option(WIRELESS_WIFI_5G_BANDSTEER_OPTION, val))
    {
        return CW_FALSE;
    }
#if SUPPORT_WLAN_5G_2_SETTING
    if (api_set_wifi_band_steering_mode_option(WIRELESS_WIFI_5G_2_BANDSTEER_OPTION, val))
    {
        return CW_FALSE;
    }
#endif
#endif
    CWDebugLog("%s %d val:%d", __FUNCTION__, __LINE__, val);
    return CW_TRUE;
}

CWBool CWWTPBoardGetBandSteeringMode(int *mode)
{
    *mode = 0;

    if (api_get_wifi_band_steering_mode_option(WIRELESS_WIFI_BANDSTEER_OPTION, mode))
    {
        *mode = 0;
    }

    CWDebugLog("%s %d band steering mode:[%d]", __FUNCTION__, __LINE__, *mode);

    return CW_TRUE;
}

CWBool CWWTPBoardSetBandSteeringMode(int mode)
{
    char *check = NULL;
    int wifi1_check = CW_FALSE, wifi2_check = CW_FALSE;

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, mode);

    if(!(check = CWCreateStringByCmdStdout("/usr/sbin/sn_bandsteering %d %d", 1, 1)))
    {
        return CW_FALSE;
    }
    wifi1_check = atoi(check);
    CW_FREE_OBJECT(check);
    CWDebugLog("%s %d wifi1_check:%d", __FUNCTION__, __LINE__, wifi1_check);
    if(!(check = CWCreateStringByCmdStdout("/usr/sbin/sn_bandsteering %d %d", 2, 1)))
    {
        return CW_FALSE;
    }
    wifi2_check = atoi(check);
    CW_FREE_OBJECT(check);
    CWDebugLog("%s %d wifi2_check:%d", __FUNCTION__, __LINE__, wifi2_check);

    if (api_set_wifi_band_steering_mode_option(WIRELESS_WIFI_BANDSTEER_OPTION, mode))
    {
        return CW_FALSE;
    }
    if (api_set_bool_option("wireless.wifi0_ssid_1.bandsteer_en", (((wifi1_check==CW_FALSE)&&(wifi2_check==CW_FALSE))||(mode==CW_FALSE))?CW_FALSE:CW_TRUE))
    {
        return CW_FALSE;
    }
#if SUPPORT_WLAN_5G_SETTING
    if (api_set_wifi_band_steering_mode_option(WIRELESS_WIFI_5G_BANDSTEER_OPTION, mode))
    {
        return CW_FALSE;
    }
    if (api_set_bool_option("wireless.wifi1_ssid_1.bandsteer_en", ((wifi1_check==CW_FALSE)||(mode==CW_FALSE))?CW_FALSE:CW_TRUE))
    {
        return CW_FALSE;
    }
#if SUPPORT_WLAN_5G_2_SETTING
    if (api_set_wifi_band_steering_mode_option(WIRELESS_WIFI_5G_2_BANDSTEER_OPTION, mode))
    {
        return CW_FALSE;
    }
    if (api_set_bool_option("wireless.wifi2_ssid_1.bandsteer_en", ((wifi2_check==CW_FALSE)||(mode==CW_FALSE))?CW_FALSE:CW_TRUE))
    {
        return CW_FALSE;
    }
#endif
#endif
    return CW_TRUE;
}

CWBool CWWTPBoardGetBandSteeringPercentEnable(int *enable)
{
    *enable = 1;
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetBandSteeringPercentEnable(int enable)
{
    CWDebugLog("%s %d", __FUNCTION__, enable);

    return CW_TRUE;
}

CWBool CWWTPBoardGetBandSteeringRssiEnable(int *enable)
{
    *enable = 1;
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetBandSteeringRssiEnable(int enable)
{
    CWDebugLog("%s %d", __FUNCTION__, enable);

    return CW_TRUE;
}

CWBool CWWTPBoardGetBandSteeringRssi(int *rssi)
{
#if SUPPORT_WLAN_BANDSTEER_ENHANCE
    int val;

    if (api_get_wifi_band_steering_rssi_option(WIRELESS_WIFI_BANDSTEERRSSI_OPTION, &val))
    {
        val = 30;
    }

    *rssi = (val - 95) ;
#else
    *rssi = 0 ;
#endif
    CWDebugLog("%s %d band steering rssi:[%d]", __FUNCTION__, __LINE__, *rssi);

    return CW_TRUE;
}

CWBool CWWTPBoardSetBandSteeringRssi(int rssi)
{
    CWDebugLog("%s %d", __FUNCTION__, rssi);
#if SUPPORT_WLAN_BANDSTEER_ENHANCE
#if SUPPORT_WLAN_5G_2_SETTING
    int lbrssi=0;
    api_get_wifi_band_steering_rssi_option(WIRELESS_WIFI_BANDSTEERRSSI_OPTION, &lbrssi);

    rssi = rssi + 95;

    if (api_set_wifi_band_steering_rssi_option(WIRELESS_WIFI_BANDSTEERRSSI_OPTION, rssi))
        return CW_FALSE;
    if (api_set_wifi_band_steering_rssi_option(WIRELESS_WIFI_5G_BANDSTEERRSSI_OPTION, rssi))
        return CW_FALSE;
    if (api_set_wifi_band_steering_rssi_option(WIRELESS_WIFI_5G_2_BANDSTEERRSSI_OPTION, rssi))
        return CW_FALSE;
#else
    if (api_set_wifi_band_steering_rssi_option(WIRELESS_WIFI_BANDSTEERRSSI_OPTION, (rssi + 95)))
    {
        return CW_FALSE;
    }
#if SUPPORT_WLAN_5G_SETTING
    if (api_set_wifi_band_steering_rssi_option(WIRELESS_WIFI_5G_BANDSTEERRSSI_OPTION, (rssi + 95)))
    {
        return CW_FALSE;
    }
#endif
#endif
#endif
    return CW_TRUE;
}

CWBool CWWTPBoardGetBandSteeringPercent(int *present)
{
#if SUPPORT_WLAN_BANDSTEER_ENHANCE
    if (api_get_wifi_band_steering_percent_option(WIRELESS_WIFI_BANDSTEERPERSENT_OPTION, present))
    {
        *present = 75;
    }
#else
    *present = 0;
#endif
    CWDebugLog("%s %d band steering percent:[%d]", __FUNCTION__, __LINE__, *present);

    return CW_TRUE;
}

CWBool CWWTPBoardSetBandSteeringPercent(int present)
{
    CWDebugLog("%s %d", __FUNCTION__, present);
#if SUPPORT_WLAN_BANDSTEER_ENHANCE
    if (api_set_wifi_band_steering_percent_option(WIRELESS_WIFI_BANDSTEERPERSENT_OPTION, present))
    {
        return CW_FALSE;
    }
#if SUPPORT_WLAN_5G_SETTING
    if (api_set_wifi_band_steering_percent_option(WIRELESS_WIFI_5G_BANDSTEERPERSENT_OPTION, present))
    {
        return CW_FALSE;
    }
#if SUPPORT_WLAN_5G_2_SETTING
    if (api_set_wifi_band_steering_percent_option(WIRELESS_WIFI_5G_2_BANDSTEERPERSENT_OPTION, present))
    {
        return CW_FALSE;
    }
#endif
#endif
#endif
    return CW_TRUE;
}

CWBool CWWTPBoardGetFastHandoverStatusCfg(int *enable)
{
#if SUPPORT_WLAN_24G_SETTING
    if (api_get_integer_option(WIRELESS_WIFI_FASTHANDOVER_ENABLE_OPTION, enable))
#elif SUPPORT_WLAN_5G_SETTING
    if (api_get_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_ENABLE_OPTION, enable))
#endif
    {
        *enable = 0;
    }

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetFastHandoverStatusCfg(int enable)
{
    CWDebugLog("%s %u", __FUNCTION__, enable);
#if SUPPORT_WLAN_24G_SETTING
    if (api_set_integer_option(WIRELESS_WIFI_FASTHANDOVER_ENABLE_OPTION, enable))
    {
        return CW_FALSE;
    }
#endif
#if SUPPORT_WLAN_5G_SETTING
    if (api_set_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_ENABLE_OPTION, enable))
    {
        return CW_FALSE;
    }
#if SUPPORT_WLAN_5G_2_SETTING
    if (api_set_integer_option(WIRELESS_WIFI_5G_2_FASTHANDOVER_ENABLE_OPTION, enable))
    {
        return CW_FALSE;
    }
#endif
#endif
    return CW_TRUE;
}

CWBool CWWTPBoardGetFastHandoverRssiCfg(int *rssi)
{
#if SUPPORT_WLAN_24G_SETTING
    if (api_get_integer_option(WIRELESS_WIFI_FASTHANDOVER_RSSI_OPTION, rssi))
#elif SUPPORT_WLAN_5G_SETTING
    if (api_get_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_RSSI_OPTION, rssi))
#endif
    {
        *rssi = 0 ;
    }

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *rssi);
    return CW_TRUE;
}

CWBool CWWTPBoardSetFastHandoverRssiCfg(int rssi)
{
    CWDebugLog("%s %d", __FUNCTION__, rssi);
#if SUPPORT_WLAN_24G_SETTING
    if (api_set_integer_option(WIRELESS_WIFI_FASTHANDOVER_RSSI_OPTION, rssi))
    {
        return CW_FALSE;
    }
#endif
#if SUPPORT_WLAN_5G_SETTING
    if (api_set_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_RSSI_OPTION, rssi))
    {
        return CW_FALSE;
    }
#if SUPPORT_WLAN_5G_2_SETTING
    if (api_set_integer_option(WIRELESS_WIFI_5G_2_FASTHANDOVER_RSSI_OPTION, rssi))
    {
        return CW_FALSE;
    }
#endif
#endif

    return CW_TRUE;
}

// V1 Not Support
CWBool CWWTPBoardGetRadioFastHandoverStatusCfg(int radioIdx, int *enable)
{
    int Ret = API_RC_INTERNAL_ERROR;
    
    switch (CWConvertRadioIdx(radioIdx))
    {
#if SUPPORT_WLAN_24G_SETTING
    case 0:
        Ret = api_get_integer_option(WIRELESS_WIFI_FASTHANDOVER_ENABLE_OPTION, enable);
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case 1:
        Ret = api_get_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_ENABLE_OPTION, enable);
        break;
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
        Ret = api_get_integer_option(WIRELESS_WIFI_5G_2_FASTHANDOVER_ENABLE_OPTION, enable);
        break;
#endif
#endif
    default:
        break;
    }

    if (Ret != API_RC_SUCCESS)
    {
        *enable = CW_FALSE;
    }

    CWLog("%s %u %u", __FUNCTION__,CWConvertRadioIdx(radioIdx), *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioFastHandoverStatusCfg(int radioIdx, int enable)
{
    int Ret = API_RC_INTERNAL_ERROR;

    CWLog("%s %u %u", __FUNCTION__,CWConvertRadioIdx(radioIdx), enable);

    switch (CWConvertRadioIdx(radioIdx))
    {
    case 0:
        Ret = api_set_integer_option(WIRELESS_WIFI_FASTHANDOVER_ENABLE_OPTION, enable);
        break;
#if SUPPORT_WLAN_5G_SETTING
    case 1:
        Ret = api_set_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_ENABLE_OPTION, enable);
        break;
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
        Ret = api_set_integer_option(WIRELESS_WIFI_5G_2_FASTHANDOVER_ENABLE_OPTION, enable);
        break;
#endif
#endif
    default:
        break;
    }

    return (Ret!=API_RC_SUCCESS)?CW_FALSE:CW_TRUE;
}

// V1 Not Support
CWBool CWWTPBoardGetRadioFastHandoverRssiCfg(int radioIdx, int *rssi)
{
    int Ret = API_RC_INTERNAL_ERROR;

    switch (CWConvertRadioIdx(radioIdx))
    {
    case 0:
        Ret = api_get_integer_option(WIRELESS_WIFI_FASTHANDOVER_RSSI_OPTION, rssi);
        break;
#if SUPPORT_WLAN_5G_SETTING
    case 1:
        Ret = api_get_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_RSSI_OPTION, rssi);
        break;
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
        Ret = api_get_integer_option(WIRELESS_WIFI_5G_2_FASTHANDOVER_RSSI_OPTION, rssi);
        break;
#endif
#endif
    default:
        break;
    }

    if (Ret != API_RC_SUCCESS)
    {
        *rssi = -90;
    }

    CWLog("%s %u %d", __FUNCTION__,CWConvertRadioIdx(radioIdx), *rssi);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioFastHandoverRssiCfg(int radioIdx, int rssi)
{
    int Ret = API_RC_INTERNAL_ERROR;

    CWLog("%s %u %d ", __FUNCTION__,CWConvertRadioIdx(radioIdx), rssi);

    switch (CWConvertRadioIdx(radioIdx))
    {
    case 0:
        Ret = api_set_integer_option(WIRELESS_WIFI_FASTHANDOVER_RSSI_OPTION, rssi);
        break;
#if SUPPORT_WLAN_5G_SETTING
    case 1:
        Ret = api_set_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_RSSI_OPTION, rssi);
        break;
#if SUPPORT_WLAN_5G_2_SETTING
    case 2:
        Ret = api_set_integer_option(WIRELESS_WIFI_5G_2_FASTHANDOVER_RSSI_OPTION, rssi);
        break;
#endif
#endif
    default:
        break;
    }

    return (Ret!=API_RC_SUCCESS)?CW_FALSE:CW_TRUE;
}

CWBool CWWTPBoardGetDownloadLimitCfg(int radioIdx, int wlanIdx, int *limit)
{
    char *section, value[5] = {0};
    int tc_enable, tc_rate, tc_support_rate = CW_RATE_MBPS;

    CWDebugLog("%s %d radioIdx:%d wlanIdx:%d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);

    *limit = -1;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    api_get_wifi_ifacex_tc_enabled_option_by_sectionname(OPM_AP,section,wlanIdx+1, &tc_enable);

    if(tc_enable)
    {
        api_get_wifi_ifacex_tc_downlimit_option_by_sectionname(OPM_AP,section,wlanIdx+1, limit);
    }
    else
    {
        *limit = -1;

        if (section != NULL) CW_FREE_OBJECT(section);
            CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *limit);

        return CW_TRUE;
    }

    memset(value, 0, sizeof(value));
    api_get_string_option(SNTCD_RATE_UNIT_OPTION, value, sizeof(value));

    if ( strcasecmp(value,"Kbps") == 0 )
        tc_support_rate = CW_RATE_KBPS;
    else
        tc_support_rate = CW_RATE_MBPS;

    memset(value, 0, sizeof(value));
    api_get_sntcd_down_rate_unit_option_by_sectionname(section, wlanIdx+1, value, sizeof(value));

    if ( strcasecmp(value,"Kbps") == 0 )
        tc_rate = CW_RATE_KBPS;
    else
        tc_rate = CW_RATE_MBPS;

    if ( g_ac_support_tc_rate_unit == CW_FALSE && tc_support_rate == CW_RATE_KBPS )
    { // old ezMaster and new AP
        if ( tc_rate == CW_RATE_KBPS )
        {
            *limit = CWWTPConvertKbpsToMbps(*limit);
        }
    }

    if (section != NULL) CW_FREE_OBJECT(section);
        CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *limit);

    return CW_TRUE;
}

CWBool CWWTPBoardSetDownloadLimitCfg(int radioIdx, int wlanIdx, int limit)
{
    char *section, value[5] = {0};
    int tc_rate, tc_support_rate = CW_RATE_MBPS;

    CWDebugLog("%s %u %u limit:[%d]", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, limit);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(limit != -1)
        {
            CWDebugLog("Guest wlan cannot support Download Limit");
        }
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    if(limit == -1)
    {
        if (api_set_wifi_ifacex_tc_enabled_option_by_sectionname(OPM_AP, section, wlanIdx+1, 0))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
    }
    else
    {
        memset(value, 0, sizeof(value));
        api_get_string_option(SNTCD_RATE_UNIT_OPTION, value, sizeof(value));

        if ( strcasecmp(value,"Kbps") == 0 )
            tc_support_rate = CW_RATE_KBPS;
        else
            tc_support_rate = CW_RATE_MBPS;

        memset(value, 0, sizeof(value));
        api_get_sntcd_down_rate_unit_option_by_sectionname(section, wlanIdx+1, value, sizeof(value));

        if ( strcasecmp(value,"Kbps") == 0 )
            tc_rate = CW_RATE_KBPS;
        else
            tc_rate = CW_RATE_MBPS;

        if ( g_ac_support_tc_rate_unit == CW_FALSE && tc_support_rate == CW_RATE_KBPS )
        { // old ezMaster and new AP
            if ( tc_rate == CW_RATE_KBPS )
            {
                limit = CWWTPConvertMbpsToKbps(limit);
            }
        }

        if (api_set_wifi_ifacex_tc_enabled_option_by_sectionname(OPM_AP, section, wlanIdx+1, 1))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
        if (api_set_wifi_ifacex_tc_downlimit_option_by_sectionname(OPM_AP, section, wlanIdx+1, limit))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
    }
    if (section != NULL) CW_FREE_OBJECT(section);
    configuration_wlan_traffic_shapping[radioIdx][wlanIdx]=CW_TRUE;
    return CW_TRUE;
}

CWBool CWWTPBoardGetUploadLimitCfg(int radioIdx, int wlanIdx, int *limit)
{
    char *section, value[5] = {0};
    int tc_enable, tc_rate = CW_RATE_MBPS, tc_support_rate = CW_RATE_MBPS;

    CWDebugLog("%s %d radioIdx:%d wlanIdx:%d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);

    *limit=-1;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    api_get_wifi_ifacex_tc_enabled_option_by_sectionname(OPM_AP,section,wlanIdx+1, &tc_enable);

    if(tc_enable)
    {
        api_get_wifi_ifacex_tc_uplimit_option_by_sectionname(OPM_AP,section,wlanIdx+1, limit);
    }
    else
    {
        *limit = -1;

        if (section != NULL) CW_FREE_OBJECT(section);
            CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *limit);

        return CW_TRUE;
    }

    memset(value, 0, sizeof(value));
    api_get_string_option(SNTCD_RATE_UNIT_OPTION, value, sizeof(value));

    if ( strcasecmp(value,"Kbps") == 0 )
        tc_support_rate = CW_RATE_KBPS;
    else
        tc_support_rate = CW_RATE_MBPS;

    memset(value, 0, sizeof(value));
    api_get_sntcd_up_rate_unit_option_by_sectionname(section, wlanIdx+1, value, sizeof(value));

    if ( strcasecmp(value,"Kbps") == 0 )
        tc_rate = CW_RATE_KBPS;
    else
        tc_rate = CW_RATE_MBPS;

    if ( g_ac_support_tc_rate_unit == CW_FALSE && tc_support_rate == CW_RATE_KBPS )
    { // old ezMaster and new AP
        if ( tc_rate == CW_RATE_KBPS )
        {
            *limit = CWWTPConvertKbpsToMbps(*limit);
        }
    }

    if (section != NULL) CW_FREE_OBJECT(section);
        CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *limit);

    return CW_TRUE;
}

CWBool CWWTPBoardSetUploadLimitCfg(int radioIdx, int wlanIdx, int limit)
{
    char *section, value[5] = {0};
    int tc_rate, tc_support_rate = CW_RATE_MBPS;

    CWDebugLog("%s %u %u limit:[%d]", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, limit);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(limit != -1)
        {
            CWDebugLog("Guest wlan cannot support Upload Limit");
        }
        return CW_TRUE;
    }

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    if(limit == -1)
    {
        if (api_set_wifi_ifacex_tc_enabled_option_by_sectionname(OPM_AP, section, wlanIdx+1, 0))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
    }
    else
    {
        memset(value, 0, sizeof(value));
        api_get_string_option(SNTCD_RATE_UNIT_OPTION, value, sizeof(value));

        if ( strcasecmp(value,"Kbps") == 0 )
            tc_support_rate = CW_RATE_KBPS;
        else
            tc_support_rate = CW_RATE_MBPS;

        memset(value, 0, sizeof(value));
        api_get_sntcd_down_rate_unit_option_by_sectionname(section, wlanIdx+1, value, sizeof(value));

        if ( strcasecmp(value,"Kbps") == 0 )
            tc_rate = CW_RATE_KBPS;
        else
            tc_rate = CW_RATE_MBPS;

        if ( g_ac_support_tc_rate_unit == CW_FALSE && tc_support_rate == CW_RATE_KBPS )
        { // old ezMaster and new AP
            if ( tc_rate == CW_RATE_KBPS )
            {
                limit = CWWTPConvertMbpsToKbps(limit);
            }
        }

        if (api_set_wifi_ifacex_tc_enabled_option_by_sectionname(OPM_AP, section, wlanIdx+1, 1))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
        if (api_set_wifi_ifacex_tc_uplimit_option_by_sectionname(OPM_AP, section, wlanIdx+1, limit))
        {
            if (section != NULL) CW_FREE_OBJECT(section);
            return CW_FALSE;
        }
    }
    if (section != NULL) CW_FREE_OBJECT(section);
    configuration_wlan_traffic_shapping[radioIdx][wlanIdx]=CW_TRUE;
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanMaxDownloadLimitCfg(int radioIdx, int wlanIdx, int limit)
{
    char option[64]={0};

    CWDebugLog("%s %d radioIdx:[%u] wlanIdx:[%u] limit:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, limit);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(limit != -1)
        {
            CWDebugLog("Guest wlan cannot support Wlan Max Download Limit");
        }
        return CW_TRUE;
    }

    if (limit == -1) 
    {
        return CW_TRUE;
    }
    else
    {
        snprintf(option,sizeof(option),"wireless.wifi%d_ssid_%d.%s",CWConvertRadioIdx(radioIdx),wlanIdx+1,"tc_downmaxlimit");
        if (api_set_integer_option(option, limit))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPerUserDownloadLimitCfg(int radioIdx, int wlanIdx, int limit)
{
    char option[64]={0};

    CWDebugLog("%s %d radioIdx:[%u] wlanIdx:[%u] limit:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, limit);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(limit != -1)
        {
            CWDebugLog("Guest wlan cannot support Wlan Download Limit");
        }
        return CW_TRUE;
    }

    if (limit == -1) 
    {
        return CW_TRUE;
    }
    else
    {
        snprintf(option,sizeof(option),"wireless.wifi%d_ssid_%d.%s",CWConvertRadioIdx(radioIdx),wlanIdx+1,"tc_downlimit");
        if (api_set_integer_option(option, limit))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanMaxUploadLimitCfg(int radioIdx, int wlanIdx, int limit)
{
    char *option = NULL;

    CWDebugLog("%s %d radioIdx:[%u] wlanIdx:[%u] limit:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, limit);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(limit != -1)
        {
            CWDebugLog("Guest wlan cannot support Wlan Max Upload Limit");
        }
        return CW_TRUE;
    }

    if (limit == -1) 
    {
        return CW_TRUE;
    }
    else
    {
        option = CWCreateString("wireless.wifi%d_ssid_%d.%s",CWConvertRadioIdx(radioIdx),wlanIdx+1,"tc_upmaxlimit");
        if (api_set_integer_option(option, limit))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            return CW_FALSE;
        }
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanPerUserUploadLimitCfg(int radioIdx, int wlanIdx, int limit)
{
    char *option = NULL;

    CWDebugLog("%s %d radioIdx:[%u] wlanIdx:[%u] limit:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, limit);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        if(limit != -1)
        {
            CWDebugLog("Guest wlan cannot support Wlan Upload Limit");
        }
        return CW_TRUE;
    }

    if (limit == -1) 
    {
        return CW_TRUE;
    }
    else
    {
        option = CWCreateString("wireless.wifi%d_ssid_%d.%s",CWConvertRadioIdx(radioIdx),wlanIdx+1,"tc_uplimit");
        if (api_set_integer_option(option, limit))
        {
            if (option != NULL) CW_FREE_OBJECT(option);
            return CW_FALSE;
        }
    }

    if (option != NULL) CW_FREE_OBJECT(option);

    return CW_TRUE;
}

/*v1 not support per user*/
CWBool CWWTPBoardGetDownloadPerUserEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int downperuser=CW_FALSE;
    char *option = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {   
        *enable = CW_FALSE;
        CWDebugLog("Guest wlan cannot support Per Download Limit");
	return CW_TRUE;
    }

    option = CWCreateString("wireless.wifi%d_ssid_%d.%s",CWConvertRadioIdx(radioIdx),wlanIdx+1,"tc_downperuser");

    if (api_get_integer_option(option, &downperuser))
    {   
        downperuser = CW_FALSE;
    }

    *enable = downperuser;

    CWDebugLog("%s %d radioIdx:[%u] wlanIdx:[%u] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    if (option != NULL) CW_FREE_OBJECT(option);

    return CW_TRUE;
}

CWBool CWWTPBoardSetDownloadPerUserEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    char *option = NULL;

    CWDebugLog("%s %d radioIdx:[%u] wlanIdx:[%u] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        CWDebugLog("Guest wlan cannot support Per Download Limit");
	return CW_TRUE;
    }

    option = CWCreateString("wireless.wifi%d_ssid_%d.%s",CWConvertRadioIdx(radioIdx),wlanIdx+1,"tc_downperuser");

    if (api_set_integer_option(option, enable))
    {
        if (option != NULL) CW_FREE_OBJECT(option);
        return CW_FALSE;
    }

    configuration_wlan_traffic_shapping[radioIdx][wlanIdx]=CW_TRUE;

    if (option != NULL) CW_FREE_OBJECT(option);

    return CW_TRUE;
}

CWBool CWWTPBoardGetUploadPerUserEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int upperuser=CW_FALSE;
    char *option = NULL;

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        *enable = CW_FALSE;
        CWDebugLog("Guest wlan cannot support Per Upload Limit");
	return CW_TRUE;
    }

    option = CWCreateString("wireless.wifi%d_ssid_%d.%s",CWConvertRadioIdx(radioIdx),wlanIdx+1,"tc_upperuser");

    if (api_get_integer_option(option, &upperuser))
    {
        upperuser = CW_FALSE;
    }

    *enable = upperuser;

    CWDebugLog("%s %d radioIdx:[%u] wlanIdx:[%u] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    if (option != NULL) CW_FREE_OBJECT(option);

    return CW_TRUE;
}

CWBool CWWTPBoardSetUploadPerUserEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    char *option = NULL;

    CWDebugLog("%s %d radioIdx:[%u] wlanIdx:[%u] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        CWDebugLog("Guest wlan cannot support Per Upload Limit");
	return TRUE;
    }

    option = CWCreateString("wireless.wifi%d_ssid_%d.%s",CWConvertRadioIdx(radioIdx),wlanIdx+1,"tc_upperuser");

    if (api_set_integer_option(option, enable))
    {
        if (option != NULL) CW_FREE_OBJECT(option);
        return CW_FALSE;
    }

    configuration_wlan_traffic_shapping[radioIdx][wlanIdx]=CW_TRUE;

    if (option != NULL) CW_FREE_OBJECT(option);

    return CW_TRUE;
}


CWBool CWWTPBoardGetRoamingEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    char *section;
    int val = 0;

    *enable = CW_FALSE;

#if !SUPPORT_FAST_ROAMING_PER_SSID
    if(wlanIdx != 0)
    {
        return CW_TRUE;
    }
#endif

#if SUPPORT_FAST_ROAMING_PER_SSID
    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
    api_get_wifi_ifacex_fastroaming_enable_option_by_sectionname(OPM_AP, section, wlanIdx+1,&val);
    if (section != NULL) CW_FREE_OBJECT(section);
#else
    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            api_get_wifi_fastroaming_enable_option(WIRELESS_WIFI_FASTROAMING_ENABLE_OPTION, &val);
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            api_get_wifi_fastroaming_enable_option(WIRELESS_WIFI_5G_FASTROAMING_ENABLE_OPTION, &val);
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            api_get_wifi_fastroaming_enable_option(WIRELESS_WIFI_5G_2_FASTROAMING_ENABLE_OPTION, &val);
            break;
#endif
#endif
        default:
            break;
    }
#endif  

    *enable = (val==1) ? CW_TRUE : CW_FALSE;

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRoamingEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    char *section;

    CWDebugLog("%s %u %u %u", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);
#if !SUPPORT_FAST_ROAMING_PER_SSID
    if(wlanIdx != 0)
    {
        if(enable != CW_FALSE)
        {
            CWDebugLog("Only WLAN0 can support Roaming");
        }
        return CW_TRUE;
    }
#endif

#if SUPPORT_FAST_ROAMING_PER_SSID
    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
    if (api_set_wifi_ifacex_fastroaming_enable_option_by_sectionname(OPM_AP, section, wlanIdx+1, enable))
    {
        if (section != NULL) CW_FREE_OBJECT(section);
        return CW_FALSE;
    }
    if (section != NULL) CW_FREE_OBJECT(section);
#else
    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            if (api_set_wifi_fastroaming_enable_option(WIRELESS_WIFI_FASTROAMING_ENABLE_OPTION, enable))
            {
                return CW_FALSE;
            }
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            if (api_set_wifi_fastroaming_enable_option(WIRELESS_WIFI_5G_FASTROAMING_ENABLE_OPTION, enable))
            {
                return CW_FALSE;
            }
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            if (api_set_wifi_fastroaming_enable_option(WIRELESS_WIFI_5G_2_FASTROAMING_ENABLE_OPTION, enable))
            {
                return CW_FALSE;
            }
            break;
#endif
#endif
        default:
            break;
    }
#endif
    return CW_TRUE;
}
//NOT_FINISH (NOT_SUPPORT)
CWBool CWWTPBoardGetRoamingAdvSearchCfg(int radioIdx, int wlanIdx, int *enable)
{
    *enable = CW_FALSE;
    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);
    return CW_TRUE;
}
//NOT_FINISH (NOT_SUPPORT)
CWBool CWWTPBoardSetRoamingAdvSearchCfg(int radioIdx, int wlanIdx, int enable)
{
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioDistance(int radioIdx, int *distance)
{
#if SUPPORT_WLAN_OUTDOOR_DISTANCE

    int val = 0;

    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            api_get_wifi_iface_distance_option(WIRELESS_WIFI_DISTANCE_OPTION, &val);
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            api_get_wifi_iface_distance_option(WIRELESS_WIFI_5G_DISTANCE_OPTION, &val);
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            api_get_wifi_iface_distance_option(WIRELESS_WIFI_5G_2_DISTANCE_OPTION, &val);
            break;
#endif
#endif
        default:
            break;
    }

    *distance = val;

#else
    *distance = 0;
#endif
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *distance);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioDistance(int radioIdx, int distance)
{
    CWDebugLog("%s %u %u", __FUNCTION__, CWConvertRadioIdx(radioIdx), distance);

#if SUPPORT_WLAN_OUTDOOR_DISTANCE
    switch(CWConvertRadioIdx(radioIdx))
    {
        case 0:
            api_set_wifi_iface_distance_option(WIRELESS_WIFI_DISTANCE_OPTION, distance);
            break;
#if SUPPORT_WLAN_5G_SETTING
        case 1:
            api_set_wifi_iface_distance_option(WIRELESS_WIFI_5G_DISTANCE_OPTION, distance);
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case 2:
            api_set_wifi_iface_distance_option(WIRELESS_WIFI_5G_2_DISTANCE_OPTION, distance);
            break;
#endif
#endif
        default:
            break;
    }
#else
    if(distance != 0)
    {
        CWDebugLog("Set configuration fail");
        return CW_TRUE;
    }
#endif

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioCurrentTxPower(int radioIdx, int *power)
{
    int wlanIdx=0;
    char ifname[8]={0};
    char *section = NULL;

#if SUPPORT_WLAN_5G_SETTING
#if SUPPORT_WLAN_5G_2_SETTING
    if (CWWTPBoardGetRadioNoChannel(radioIdx) == CW_TRUE)
    {
        *power = 0;
        CWDebugLog("%s %d radioIdx:[%d] noChannel:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), CWWTPBoardGetRadioNoChannel(radioIdx));
        return CW_TRUE;
    }
#endif
#endif

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
    {
        api_get_wifi_ifacex_ifname_option_by_sectionname(OPM_AP, section, wlanIdx+1, ifname, sizeof(ifname));

        CWDebugLog("%s %d ifname:[%s]", __FUNCTION__, __LINE__, ifname);

        *power = sysutil_get_wifixTxpower(ifname);

        if (*power != 0)
        {
            break;
        }
    }

    CWDebugLog("%s radio:[%d] power:[%d]", __FUNCTION__, CWConvertRadioIdx(radioIdx), *power);
    if (section != NULL) CW_FREE_OBJECT(section);
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioCurrentChannel(int radioIdx, int *channel)
{
    if (CWWTPBoardGetRadioNoChannel(radioIdx) == CW_TRUE)
    {
        *channel = -1;
        CWDebugLog("%s %d radio %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), *channel);
        return CW_TRUE;
    }

    if (sysutil_check_current_wifix_disabled(CWConvertRadioIdx(radioIdx)))
    {
        *channel = -1;
    }
    else
    {
        *channel = sysutil_get_wifixCurrentChannel(CWConvertRadioIdx(radioIdx));
    }

    CWDebugLog("%s %d radio %u %u", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), *channel);

    return CW_TRUE;
}

CWBool CWWTPBoardGetNextFastScanChannel(int radioIdx, int *channel)
{
    int airodump = 0;
    char *val =  NULL;
    char tmpFile[64]={0};

    snprintf(tmpFile, sizeof(tmpFile), "/tmp/Background_Scanning/wifi%d_scan_next_channel", radioIdx);
                                                                
    airodump = (access(tmpFile, R_OK) == 0)?1:0;

    if (airodump) {
        if (!(val = CWCreateStringByCmdStdout("cat %s", tmpFile))) {
            return CW_FALSE;
        }
    }
    else {
        if (!(val = CWCreateStringByCmdStdout("cat /proc/sys/dev/%s/fastscan_next_chan", WIFI_IF_NAME(CWConvertRadioIdx(radioIdx))))) {
            return CW_FALSE;
        }
    }
    
    *channel = atoi(val);
    CWDebugLog("%s %d radio %u channel %u", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), *channel);
    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardSetFastScanDurationTime(int radioIdx, unsigned int duration)
{
    int airodump = 0, enable = 0;
    char *val =  NULL;
    char tmpOption[64]={0};

    snprintf(tmpOption, sizeof(tmpOption), "wireless.wifi%d.backgroundscanEnable", radioIdx);
    airodump = (api_get_integer_option(tmpOption, &enable) == 0)?1:0;

    if (airodump) 
    {
        memset(tmpOption, 0, sizeof(tmpOption));
        snprintf(tmpOption, sizeof(tmpOption), "wireless.wifi%d.switchChanInterval", radioIdx);
        if (api_set_integer_option(tmpOption, duration))
        {
            return CW_FALSE;
        }
    }
    else 
    {
        if (!(val = CWCreateStringByCmdStdout("`echo %u > /proc/sys/dev/%s/fastscan_duration`", duration, WIFI_IF_NAME(CWConvertRadioIdx(radioIdx))))) 
        {
            return CW_FALSE;
        }
    }
    CWSystem("echo 0 > /tmp/notify_fastscan_enable");

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetScanIwlist(CWRadioFreqType radioType, CWBool bDisplay, CWWTPSitesurveyInfo *sitesurveyInfo)
{
    int i = 0;
    char tmpFile1[64] = {0}, tmpFile2[64] = {0};
    FILE *fp=NULL;
    long fsize=0;
    char *buffer=NULL, *c=NULL;
    unsigned int bssid[6]={0};
    unsigned int uint32CapCode=0;
    int radioIdx = 0, int32RadioIdx = 0, int32WlanIdx = 0, int32SsidEnable = 0;
    CWBool bFound = CW_FALSE;

    if (!CWWTPBoardGetCapCode(&uint32CapCode))
    {
        CWDebugLog("%s %d Get capCode fail", __func__, __LINE__);
        return CW_FALSE;
    }

    if (!CWGetRadioIndex(radioType, uint32CapCode, &int32RadioIdx))
    {
        CWDebugLog("%s %d Get radioIdx fail", __func__, __LINE__);
        return CW_FALSE;
    }

    radioIdx = CWConvertRadioIdx(int32RadioIdx);

    for (int32WlanIdx = 0; int32WlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); int32WlanIdx++) 
    {
        if (CWWTPBoardGetWlanEnableCfg(radioIdx, int32WlanIdx, &int32SsidEnable)) 
        {
            if (int32SsidEnable) 
            {
                bFound = CW_TRUE;
                break;
            }
        }
    }

    if (bFound == CW_FALSE) 
    {
        return CW_FALSE;
    }

    if (bDisplay == CW_FALSE)
    {
        return CW_TRUE;
    }

    sitesurveyInfo->version = 1;

    sprintf(tmpFile1, "/tmp/Background_Scanning/wifi%d_displayscanResult", radioIdx);

    if(sys_check_file_existed(tmpFile1))
    {
        CWDebugLog("%s %d %s file existed", __func__, __LINE__, tmpFile1);
        fp = fopen(tmpFile1, "r");
    }
    else
    {
        CWDebugLog("%s %d %s file not existed", __func__, __LINE__, tmpFile1);
        return CW_TRUE;
    }
    if(!fp)
    {
        CWDebugLog("%s %d fp open fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        return CW_TRUE;
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        return CW_TRUE;
    }

    if(fsize < 0)
    {
        fclose(fp);
        CWDebugLog("%s %d fsize less than 0", __func__, __LINE__);
        return CW_TRUE;
    }

    if(!fsize)
    {
        sitesurveyInfo->infoCount = 0;
        sitesurveyInfo->info = NULL;
        fclose(fp);
        return CW_TRUE;
    }

    CW_CREATE_OBJECT_SIZE_ERR(buffer, fsize + 1,
    {
        fclose(fp);
        CWDebugLog("%s %d create fsize fail", __func__, __LINE__);
        return CW_TRUE;
    });

    if(fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        CWDebugLog("%s %d fread fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CW_FREE_OBJECT(buffer);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        return CW_TRUE;
    }

    buffer[fsize] = '\0';
    sitesurveyInfo->infoCount = 0;
    c = buffer;
    while(*c != '\0')
    {
        if(*c == '\n')
        {
            (sitesurveyInfo->infoCount)++;
        }
        c++;
    }
    CW_FREE_OBJECT(buffer);

    CW_CREATE_ZERO_ARRAY_ERR(sitesurveyInfo->info, sitesurveyInfo->infoCount, CWWTPSitesurvey,
    {
        fclose(fp);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    });

    char tmpstr[256]={0};
    char *pch=NULL, *ptr=NULL;
    i = 0;
    while(fgets(tmpstr, 256, fp) != NULL && i < sitesurveyInfo->infoCount)
    {
        //BSSID
        pch = strtok_r(tmpstr, "\t", &ptr);
        if(sscanf(pch, "%x:%x:%x:%x:%x:%x",
                  &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5])
           != 6)
        {
            CW_FREE_OBJECT(sitesurveyInfo->info);
            fclose(fp);
            CWDebugLog("%s %d Get sitesurveyInfo MAC[%d] error", __func__, __LINE__, i);
            return CW_TRUE;
        }
        else
        {
            sitesurveyInfo->info[i].bssid[0] = (unsigned char) bssid[0];
            sitesurveyInfo->info[i].bssid[1] = (unsigned char) bssid[1];
            sitesurveyInfo->info[i].bssid[2] = (unsigned char) bssid[2];
            sitesurveyInfo->info[i].bssid[3] = (unsigned char) bssid[3];
            sitesurveyInfo->info[i].bssid[4] = (unsigned char) bssid[4];
            sitesurveyInfo->info[i].bssid[5] = (unsigned char) bssid[5];
        }

        //SSID
        pch = strtok_r(NULL, "\t", &ptr);
        strcpy(sitesurveyInfo->info[i].ssid, pch);

        //LEN
        pch = strtok_r(NULL, "\t", &ptr);
        sitesurveyInfo->info[i].ssidLen = atoi(pch);

        //MODE
        pch = strtok_r(NULL, "\t", &ptr);
        if(!strcmp(pch, "Master"))
        {
            sitesurveyInfo->info[i].mode = CW_RADIO_OPERATION_MODE_AP;
        }
        else if(!strcmp(pch, " Ad-Hoc"))
        {
            sitesurveyInfo->info[i].mode = CW_RADIO_OPERATION_MODE_AD_HOC;
        }
        else
        {
            sitesurveyInfo->info[i].mode = CW_RADIO_OPERATION_MODE_DISABLED;
        }

        //CH
        pch = strtok_r(NULL, "\t", &ptr);
        sitesurveyInfo->info[i].chan = atoi(pch);

        //SIGNAL
        pch = strtok_r(NULL, "\t", &ptr);
        sitesurveyInfo->info[i].signal = atoi(pch);

        //ENC
        pch = strtok_r(NULL, "\t", &ptr);
        if(strstr(pch, "OPEN"))
        {
            sitesurveyInfo->info[i].enc = CW_WLAN_SECURITY_MODE_NONE;
        }
        else if(strstr(pch, "WEP"))
        {
            sitesurveyInfo->info[i].enc = CW_WLAN_SECURITY_MODE_WEP;
        }
        else if(strstr(pch, "WPA/WPA2-PSK"))
        {
            sitesurveyInfo->info[i].enc = CW_WLAN_SECURITY_MODE_WPA_PSK_MIXED;
        }
        else if(strstr(pch, "WPA2-PSK"))
        {
            sitesurveyInfo->info[i].enc = CW_WLAN_SECURITY_MODE_WPA2_PSK;
        }
        else if(strstr(pch, "WPA-PSK"))
        {
            sitesurveyInfo->info[i].enc = CW_WLAN_SECURITY_MODE_WPA_PSK;
        }
        else if(strstr(pch, "WPA/WPA2"))
        {
            sitesurveyInfo->info[i].enc = CW_WLAN_SECURITY_MODE_WPA_MIXED;
        }
        else if(strstr(pch, "WPA2"))
        {
            sitesurveyInfo->info[i].enc = CW_WLAN_SECURITY_MODE_WPA2;
        }
        else if(strstr(pch, "WPA"))
        {
            sitesurveyInfo->info[i].enc = CW_WLAN_SECURITY_MODE_WPA;
        }
        else
        {
            CWDebugLog("unknown sitesurvey security mode");
            sitesurveyInfo->info[i].enc = CW_WLAN_SECURITY_MODE_NONE;
        }

        //TYPE
        pch = strtok_r(NULL, "\t", &ptr);
        if(!strcmp(pch, "11a"))
        {
            sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_A;
        }
        else if(!strcmp(pch, "11a/n"))
        {
            sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_AN;
        }
        else if(!strcmp(pch, "11b"))
        {
            sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_B;
        }
        else if(!strcmp(pch, "11b/g"))
        {
            sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_BG;
        }
        else if(!strcmp(pch, "11g/n"))
        {
            sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_BGN;
        }
        else if(!strcmp(pch, "11ac"))
        {
            sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_ACN_5G;
        }
        else if(!strcmp(pch, "11ax"))
        {
            sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_AX;
        }
        else
        {
            sitesurveyInfo->info[i].type = 0;
        }

        //HTMODE insert a default value
        sitesurveyInfo->info[i].htmode = CW_RADIO_CHANNEL_HT_20_MHZ;

        i++;
    }

    fclose(fp);

    sprintf(tmpFile2, "/tmp/Background_Scanning/wifi%d_displayscanAdvanceResult", radioIdx);

    fp = fopen(tmpFile2, "r");
    if(!fp)
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    while(fgets(tmpstr, 256, fp) != NULL)
    {
        //BSSID
        pch = strtok_r(tmpstr, "\t", &ptr);
        if(sscanf(pch, "%x:%x:%x:%x:%x:%x",
                  &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5])
           != 6)
        {
            CW_FREE_OBJECT(sitesurveyInfo->info);
            fclose(fp);
            CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
            return CW_TRUE;
        }
        else
        {
            for(i = 0; i < sitesurveyInfo->infoCount; i++)
            {
                if((sitesurveyInfo->info[i].bssid[0] == (unsigned char) bssid[0])
                   && (sitesurveyInfo->info[i].bssid[1] == (unsigned char) bssid[1])
                   && (sitesurveyInfo->info[i].bssid[2] == (unsigned char) bssid[2])
                   && (sitesurveyInfo->info[i].bssid[3] == (unsigned char) bssid[3])
                   && (sitesurveyInfo->info[i].bssid[4] == (unsigned char) bssid[4])
                   && (sitesurveyInfo->info[i].bssid[5] == (unsigned char) bssid[5]))
                {
                    //HT
                    pch = strtok_r(NULL, "\t", &ptr);
                    if(strstr(pch, "HT20"))
                    {
                        sitesurveyInfo->info[i].htmode = CW_RADIO_CHANNEL_HT_20_MHZ;
                    }
                    else if(strstr(pch, "HT40PLUS"))
                    {
                        sitesurveyInfo->info[i].htmode = CW_RADIO_CHANNEL_HT_40_MHZ;
                        sitesurveyInfo->info[i].extch = CW_RADIO_EXTENSION_CHANNEL_UPPER;
                    }
                    else if(strstr(pch, "HT40MINUS"))
                    {
                        sitesurveyInfo->info[i].htmode = CW_RADIO_CHANNEL_HT_40_MHZ;
                        sitesurveyInfo->info[i].extch = CW_RADIO_EXTENSION_CHANNEL_LOWER;
                    }
                    else if(strstr(pch, "VHT80"))
                    {
                        sitesurveyInfo->info[i].htmode = CW_RADIO_CHANNEL_HT_80_MHZ;
                    }
                    else
                    {
                        CWDebugLog("unknown sitesurvey HT mode");
                    }

                    break;
                }
            }
        }

    }

    fclose(fp);

    return CW_TRUE;
}

CWBool CWWTPBoardGetScanAirodump(CWRadioFreqType radioType, CWBool bDisplay, CWWTPSitesurveyInfo *sitesurveyInfo)
{
    int i = 0, security = 0, len = 0, flag = 0;
    char tmpFile[64] = {0}, airodumpFile[64] = {0};
    FILE *fp;
    long fsize;
    char *buffer, *c;
    unsigned int bssid[6] = {0};
    char tmpstr[256] = {0};
    unsigned int uint32CapCode;
    int int32RadioIdx = 0, radioIdx = 0;

    char bssid_str[128]={0}, privacy_type[64]={0}, cipher_type[64]={0}, auth_type[64]={0}, ssid[128]={0}, chwidth[64]={0}, mode[64]={0};
    int channel=0, power=0, ssid_len=0;

    if (bDisplay == CW_FALSE) 
    {
        return CW_TRUE;
    }

    sitesurveyInfo->version = 1;

    if(!CWWTPBoardGetCapCode(&uint32CapCode))
    {
        CWDebugLog("%s %d Get capCode fail", __func__, __LINE__);
        return CW_FALSE;
    }

    if(!CWGetRadioIndex(radioType, uint32CapCode, &int32RadioIdx))
    {
        CWDebugLog("%s %d Get radioIdx fail", __func__, __LINE__);
        return CW_FALSE;
    }

    radioIdx = CWConvertRadioIdx(int32RadioIdx);

    if (snprintf(airodumpFile, sizeof(airodumpFile), "/tmp/Background_Scanning/wifi%d_ap_list.txt", radioIdx) < 0)
    {
        CWDebugLog("%s %d File \"%s\" is not exist", __func__, __LINE__, airodumpFile);
        return CW_FALSE;
    }

    if (access(airodumpFile, R_OK) != 0)
    {
        CWDebugLog("%s %d File \"%s\" is not exist", __func__, __LINE__, airodumpFile);
        return CW_FALSE;
    }

    switch (radioType)
    {
#if SUPPORT_WLAN_24G_SETTING
        case CW_RADIOFREQTYPE_2G:
            sprintf(tmpFile, "/tmp/WLAN_SITE_SURVEY%s.%x.tmp", "2G", (unsigned int) CWThreadSelf());
            break;
#endif
#if SUPPORT_WLAN_5G_SETTING
        case CW_RADIOFREQTYPE_5G:
            sprintf(tmpFile, "/tmp/WLAN_SITE_SURVEY%s.%x.tmp", "5G", (unsigned int) CWThreadSelf());
            break;
#if SUPPORT_WLAN_5G_2_SETTING
        case CW_RADIOFREQTYPE_5G_1:
            sprintf(tmpFile, "/tmp/WLAN_SITE_SURVEY%s.%x.tmp", "5G_1", (unsigned int) CWThreadSelf());
            break;
#endif
#endif
        default:
            CWDebugLog("Bad Radio Frequency Type");
            return CW_TRUE;
    }

    CWSystem("tr -d '\\r' < %s > %s", airodumpFile, tmpFile);

    if (access(tmpFile, R_OK) != 0)
    {
        CWDebugLog("File \"%s\" is not exist", tmpFile);
        return CW_FALSE;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    fp = fopen(tmpFile, "r");

    if (!fp)
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        return CW_TRUE;
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }

    if (fsize < 0)
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if (!fsize)
    {
        sitesurveyInfo->infoCount = 0;
        sitesurveyInfo->info = NULL;
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }

    CW_CREATE_OBJECT_SIZE_ERR(buffer, fsize + 1,
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    });

    if (fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        return CW_TRUE;
    }


    buffer[fsize] = '\0';
    sitesurveyInfo->infoCount = 0;
    c = buffer;
    while (*c != '\0')
    {
        if (*c == '\n' && *(c+1) != '\n')
        {
            (sitesurveyInfo->infoCount)++;
        }
        c++;
    }
    CW_FREE_OBJECT(buffer);

    CW_CREATE_ZERO_ARRAY_ERR(sitesurveyInfo->info, sitesurveyInfo->infoCount, CWWTPSitesurvey,
    {
        fclose(fp);
        unlink(tmpFile);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    i = 0;
    while (fgets(tmpstr, sizeof(tmpstr), fp) != NULL && i < sitesurveyInfo->infoCount)
    {
        flag = 0;
        security = 0;
        len = strcspn(tmpstr,"\n");
        tmpstr[len] = '\0';

        memset(bssid_str, 0, sizeof(bssid_str));
        memset(privacy_type, 0, sizeof(privacy_type));
        memset(cipher_type, 0, sizeof(cipher_type));
        memset(auth_type, 0, sizeof(auth_type));
        memset(chwidth, 0, sizeof(chwidth));
        memset(mode, 0, sizeof(mode));
        memset(ssid, 0, sizeof(ssid));
        channel = 0;
        power = 0;
        ssid_len = 0;

        if (sscanf(tmpstr, "%[^,],%*[^,],%*[^,],%d,%*d,%[^,],%[^,],%[^,],%d,%*d,%*d,%*[^,],%d,%*[^,],%[^,],%[^,],%s",
                            bssid_str,
                            &channel,
                            privacy_type,
                            cipher_type,
                            auth_type,
                            &power,
                            &ssid_len,
                            chwidth,
                            mode,
                            ssid) >= 9) // ssid can be empty
        {
            if(sscanf(bssid_str, "%x:%x:%x:%x:%x:%x", &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5]) != 6)
            {
                fclose(fp);
                unlink(tmpFile);
                CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
                return CW_TRUE;
            }
            else
            {
                sitesurveyInfo->info[i].bssid[0] = (unsigned char) bssid[0];
                sitesurveyInfo->info[i].bssid[1] = (unsigned char) bssid[1];
                sitesurveyInfo->info[i].bssid[2] = (unsigned char) bssid[2];
                sitesurveyInfo->info[i].bssid[3] = (unsigned char) bssid[3];
                sitesurveyInfo->info[i].bssid[4] = (unsigned char) bssid[4];
                sitesurveyInfo->info[i].bssid[5] = (unsigned char) bssid[5];
            }

            sitesurveyInfo->info[i].chan = channel;

            if (strstr(privacy_type, "WPA2"))
                flag |= STD_WPA2;
            else if (strstr(privacy_type, "WPA"))
                flag |= STD_WPA;
            else if (strstr(privacy_type, "WEP"))
                flag |= STD_WEP;
            else if (strstr(privacy_type, "OPN"))
                flag |= STD_OPN;

            if (strstr(cipher_type, "WEP104"))
                flag |= ENC_WEP104;
            else if (strstr(cipher_type, "WEP40"))
                flag |= ENC_WEP40;
            else if (strstr(cipher_type, "CCMP"))
                flag |= ENC_CCMP;
            else if (strstr(cipher_type, "WRAP"))
                flag |= ENC_WRAP;
            else if (strstr(cipher_type, "TKIP"))
                flag |= ENC_TKIP;
            else if (strstr(cipher_type, "WEP"))
                flag |= ENC_WEP;

            if (strstr(auth_type, "MGT"))
                flag |= AUTH_MGT;
            else if (strstr(auth_type, "PSK"))
                flag |= AUTH_PSK;
            else if (strstr(auth_type, "ASK"))
                flag |= AUTH_PSK;
            else if (strstr(auth_type, "OPN"))
                flag |= AUTH_OPN;

            sitesurveyInfo->info[i].signal = power;

            sitesurveyInfo->info[i].ssidLen = ssid_len;

            strcpy(sitesurveyInfo->info[i].ssid, ssid);

            if(strstr(chwidth, "HT20"))
            {
                sitesurveyInfo->info[i].htmode = CW_RADIO_CHANNEL_HT_20_MHZ;
            }
            else if(strstr(chwidth, "HT40+"))
            {
                sitesurveyInfo->info[i].htmode = CW_RADIO_CHANNEL_HT_40_MHZ;
                sitesurveyInfo->info[i].extch = CW_RADIO_EXTENSION_CHANNEL_UPPER;
            }
            else if(strstr(chwidth, "HT40-"))
            {
                sitesurveyInfo->info[i].htmode = CW_RADIO_CHANNEL_HT_40_MHZ;
                sitesurveyInfo->info[i].extch = CW_RADIO_EXTENSION_CHANNEL_LOWER;
            }
            else if(strstr(chwidth, "HT80"))
            {
                sitesurveyInfo->info[i].htmode = CW_RADIO_CHANNEL_HT_80_MHZ;
            }

            if(strstr(mode, "AN"))
                sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_AN;
            else if(!strstr(mode, "AC"))
                sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_ACN_5G;
#if 0
            else if(!strcmp(pval, "11ax"))
                sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_AX;
#endif
            else if(strstr(mode, "A"))
                sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_A;
            else if(strstr(mode, "BGN"))
                sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_BGN;
            else if(strstr(mode, "BG"))
                sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_BG;
#if 0
            else if(!strcmp(pval, "11b"))
                sitesurveyInfo->info[i].type = CW_RADIO_WIRELESS_MODE_B;
#endif
            else
                sitesurveyInfo->info[i].type = 0;
        }
        CWWTPBoardGetSitesurveySecurity(flag, &security);
        sitesurveyInfo->info[i].enc = security;
        sitesurveyInfo->info[i].mode = CW_RADIO_OPERATION_MODE_AP;
        i++;
    }

    fclose(fp);
    unlink(tmpFile);

    return CW_TRUE;
}

CWBool CWWTPBoardGetScan(CWBool bDisplay, CWWTPSitesurveyInfo *sitesurveyInfo)
{
    unsigned int uint32CapCode = 0;
    int int32RadioIdx = 0;
    int radioIdx = 0, backgroundscanEnable = 0;
    int j = 0;

    if (!CWWTPBoardGetCapCode(&uint32CapCode))
    {
        CWDebugLog("%s %d Get capCode fail", __func__, __LINE__);
        return CW_FALSE;
    }

    if (!CWGetRadioIndex(sitesurveyInfo->radio, uint32CapCode, &int32RadioIdx))
    {
        CWDebugLog("%s %d Get radioIdx fail", __func__, __LINE__);
        return CW_FALSE;
    }

#if SUPPORT_CAPWAP_NO_BACKGROUND_SCAN
    sitesurveyInfo->infoCount = 0;
    sitesurveyInfo->info = NULL;
    return CW_TRUE;
#endif

    radioIdx = CWConvertRadioIdx(int32RadioIdx);

#if SUPPORT_WLAN_5G_SETTING
#if SUPPORT_WLAN_5G_2_SETTING
    if (CWWTPBoardGetRadioNoChannel(radioIdx) == CW_TRUE) 
    {
        sitesurveyInfo->infoCount = 0;
        sitesurveyInfo->info = NULL;
        CWDebugLog("%s %d radioIdx:[%d] noChannel:[%d]", __FUNCTION__, __LINE__, radioIdx, CWWTPBoardGetRadioNoChannel(radioIdx));
        return CW_TRUE;
    }
#endif
#endif

    switch (radioIdx) 
    {
#if SUPPORT_WLAN_24G_SETTING
    case CW_RADIOFREQTYPE_2G:
        api_get_integer_option("wireless.wifi0.backgroundscanEnable", &backgroundscanEnable);
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case CW_RADIOFREQTYPE_5G:
        api_get_integer_option("wireless.wifi1.backgroundscanEnable", &backgroundscanEnable);
        break;
#if SUPPORT_WLAN_5G_2_SETTING
    case CW_RADIOFREQTYPE_5G_1:
        api_get_integer_option("wireless.wifi2.backgroundscanEnable", &backgroundscanEnable);
        break;
#endif
#endif
    default:
        backgroundscanEnable = 0;
        break;
    }

    if (backgroundscanEnable == 1)
    {
        CWDebugLog("%s %d --- Get Scan By Iwlist ---", __func__, __LINE__);
        CWWTPBoardGetScanIwlist(radioIdx, bDisplay, sitesurveyInfo);
        CWDebugLog("%s %d infoCount:%d", __func__, __LINE__, sitesurveyInfo->infoCount);
   
        if (sitesurveyInfo->infoCount > 0)
        {
            for (j = 0; j < sitesurveyInfo->infoCount; j++)
            {
                CWDebugLog("sitesurveyInfo[%d] bssid:[%02x:%02x:%02x:%02x:%02x:%02x] chan:%d signal:%d length:%d essid:%s bandwidth:%d wlmode:%d security:%d operation:%d htmode:[%d]",
                       j, CW_MAC_PRINT_LIST(sitesurveyInfo->info[j].bssid),
                       sitesurveyInfo->info[j].chan, sitesurveyInfo->info[j].signal,
                       sitesurveyInfo->info[j].ssidLen, sitesurveyInfo->info[j].ssid,
                       sitesurveyInfo->info[j].htmode, sitesurveyInfo->info[j].type,
                       sitesurveyInfo->info[j].enc, sitesurveyInfo->info[j].mode,
		       sitesurveyInfo->info[j].htmode);
            }
        }
    }

    return CW_TRUE;
}

//Get Channel Utilization for Auto Channel 
CWBool CWWTPBoardGetChannelUtilization(CWRadioFreqType radioType, int channel, unsigned char *chanUtil)
{

    unsigned int uint32CapCode, num = 0;
    int i32RadioIdx, i;
    unsigned char *radioChannel;
    char *val =  NULL;

    if(!CWWTPBoardGetCapCode(&uint32CapCode))
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }
    if(!CWGetRadioIndex(radioType, uint32CapCode, &i32RadioIdx))
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    switch (radioType)
    {
    case CW_RADIOFREQTYPE_2G:
        radioChannel = radio2GChannel;
        num = sizeof(radio2GChannel);
        break;
#if SUPPORT_WLAN_5G_SETTING
    case CW_RADIOFREQTYPE_5G:
        radioChannel = radio5GChannel;
        num = sizeof(radio5GChannel);
        break;
#if SUPPORT_WLAN_5G_2_SETTING
    case CW_RADIOFREQTYPE_5G_1:
        radioChannel = radio5G1Channel;
        num = sizeof(radio5G1Channel);
        break;
#endif
#endif
    default:
        CWDebugLog("Invalid Radio Type:%d", radioType);
        return CW_TRUE;
    }

    for(i = 0; i < num; i++)
    {
        if(channel == radioChannel[i])
        {
            chanUtil[(i * 4) + 0] = 1;

            if(!(val = CWCreateStringByCmdStdout("cat /proc/sys/dev/%s/last_rxclear", WIFI_IF_NAME(i32RadioIdx))))
            {
                return CW_FALSE;
            }
            else
            {
                chanUtil[(i * 4) + 1] = atoi(val);
                CW_FREE_OBJECT(val);
            }

            if(!(val = CWCreateStringByCmdStdout("cat /proc/sys/dev/%s/last_rxframe", WIFI_IF_NAME(i32RadioIdx))))
            {
                return CW_FALSE;
            }
            else
            {
                chanUtil[(i * 4) + 2] = atoi(val);
                CW_FREE_OBJECT(val);
            }

            if(!(val = CWCreateStringByCmdStdout("cat /proc/sys/dev/%s/last_txframe", WIFI_IF_NAME(i32RadioIdx))))
            {
                return CW_FALSE;
            }
            else
            {
                chanUtil[(i * 4) + 3] = atoi(val);
                CW_FREE_OBJECT(val);
            }
            break;
        }
    }

    if(i == num)
    {
        CWDebugLog("Invalid Channel:%d", channel);
    }

    CWDebugLog("radioType:%d Channel Utilization Not Support", radioType);
    return CW_TRUE;
}

CWBool CWWTPBoardSetScanDwellTime(int radioIdx, unsigned int min, unsigned int max)
{
    int airodump = 0, enable = 0;
    char *val =  NULL;
    char tmpOption[64]={0};

    snprintf(tmpOption, sizeof(tmpOption), "wireless.wifi%d.backgroundscanEnable", radioIdx);
    airodump = (api_get_integer_option(tmpOption, &enable) == 0)?1:0;

    if (airodump) 
    {
        memset(tmpOption, 0, sizeof(tmpOption));
        snprintf(tmpOption, sizeof(tmpOption), "wireless.wifi%d.mindwell", radioIdx);
        if (api_set_integer_option(tmpOption, min))
        {
            return CW_FALSE;
        }

        memset(tmpOption, 0, sizeof(tmpOption));
        snprintf(tmpOption, sizeof(tmpOption), "wireless.wifi%d.maxdwell", radioIdx);
        if (api_set_integer_option(tmpOption, max))
        {
            return CW_FALSE;
        }
    }
    else 
    {
        if (!(val = CWCreateStringByCmdStdout("`echo %u > /proc/sys/dev/%s/fastscan_active_dwell_min`", min, WIFI_IF_NAME(CWConvertRadioIdx(radioIdx)))))
        {
            return CW_FALSE;
        }

        CW_FREE_OBJECT(val);

        if (!(val = CWCreateStringByCmdStdout("`echo %u > /proc/sys/dev/%s/fastscan_active_dwell_max`", max, WIFI_IF_NAME(CWConvertRadioIdx(radioIdx)))))
        {
            return CW_FALSE;
        }

        CW_FREE_OBJECT(val);
    }

    return CW_TRUE;
}
#if 0
CWBool CWWTPBoardSetAutoChannelSelectionACS(CWBool enable)
{
    ///////////////* for auto channel phase 1 *//////////////////
    /*
    		usage: sn_acs rxclear_threshold noise_count chan_dwell_time polling_time radio_sequence radio
    		rxclear_threshold range: 30~100 percent
    		noise_count range: 2~10
    		chan_dwell_time: 10~100 seconds
    		polling_time: 3~30 seconds
    		radio_sequence: 1 = 2G wifi0 5G wifi1, 2 = 5G wifi0 2G wifi1
    		radio: 0~1, 0 = wifi0, 1 = wifi1
    		example: sn_acs 75 5 30 5 1 0
    		atmws600ap:
    		2G: sn_acs 75 5 30 5 2 1
    		5G: sn_acs 75 5 30 5 2 0
    		atmws900ap/1750ap:
    		2G: sn_acs 75 5 30 5 1 0
    		5G: sn_acs 75 5 30 5 1 1
    	*/
    unsigned int uint32CapCode;
    int int32SsidEnable;
    CWRadioFreqType radioType;
    int radioIdx;
    int channel;
    CWBool bBgStsrvy = CW_FALSE;

    if(enable)
    {
        CWWTPBoardGetCapCode(&uint32CapCode);

        for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
        {
            CWWTPBoardGetWlanEnableCfg(radioIdx, 0, &int32SsidEnable);
            if(int32SsidEnable == CW_TRUE)
            {
                if(CWGetRadioType(&radioType, uint32CapCode, radioIdx) == CW_TRUE)
                {
                    if(radioType == CW_RADIOFREQTYPE_2G)
                    {
                        bBgStsrvy = CWWTPCheck2gSitesurveyDoing();
                    }
                    else //if(radioType == CW_RADIOFREQTYPE_5G)
                    {
                        bBgStsrvy = CWWTPCheck5gSitesurveyDoing();
                    }
                    CWWTPBoardGetRadioChannelCfg(radioIdx, &channel);

                    if((channel == CW_RADIO_CHANNEL_AUTO) && (bBgStsrvy == CW_TRUE))
                    {
#if defined(CW_BOARD_EWS310AP) || defined(CW_BOARD_EWS210AP) || defined(CW_BOARD_ATMWS600AP) || defined(CW_BOARD_WAP310) || defined(CW_BOARD_WAP210)
                        CWDebugLog("sn_acs 90 6 600 10 2 %d >/dev/null 2>&1 &", (radioIdx ? 0 : 1));
                        CWSystem("sn_acs 90 6 600 10 2 %d >/dev/null 2>&1 &", (radioIdx ? 0 : 1));
#else
                        CWDebugLog("sn_acs 90 6 600 10 1 %d >/dev/null 2>&1 &", radioIdx);
                        CWSystem("sn_acs 90 6 600 10 1 %d >/dev/null 2>&1 &", radioIdx);
#if SUPPORT_WLAN_5G_2_SETTING
                        if (radioIdx == 1) {
                            CWDebugLog("sn_acs 90 6 600 10 1 %d >/dev/null 2>&1 &", 2);
                            CWSystem("sn_acs 90 6 600 10 1 %d >/dev/null 2>&1 &", 2);
                        }
#endif
#endif
                    }
                }
            }
        }
    }
    else
    {
        CWSystem("killall -9 sn_acs >/dev/null 2>&1");
    }
    ////////////////////////////////////////////////////////

    return CW_TRUE;
}
#else
CWBool CWWTPBoardSetRadioAutoChannelSelectionACS(int radioIdx, CWBool enable)
{
    ///////////////* for auto channel phase 1 *//////////////////
    /*
     * usage: sn_acs --rxth [rxclear_threshold] --rxthnc [rxclear_threshold_no_client] --n [noise_count] --wt [chan_dwell_time] --pt [polling_time] --rs [radio_sequence] --rt [radio]
     *
     * rxclear_threshold range: 30~100 percent   {default=90}
     * rxclear_threshold_no_client range: 30~100 percent  {default=50}
     * noise_count range: 2~10  {default=6}
     * chan_dwell_time: 60~1200 seconds  {default=600}
     * polling_time: 3~30 seconds   {default=10}
     * radio_sequence: [1/2] 1:=(2.4G/5G = wifi0/wifi1), 2:=(2.4G/5G = wifi1/wifi0)  {default=0}
     * radio: 0:wifi0, 1:= wifi1  {default=1}
     * example:
     *   sn_acs --rxth 90 --rxthnc 50 --n 6 --wt 600 --pt 10 --rs 1 --r 0
     */
    int wlanEnable = 0, wlanIdx = 0, wlanIdxEnable = 0, bFound = CW_FALSE;

    CWDebugLog("%s %d radioIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), enable);

    wlanEnable = (CWWTPBoardGetRadioNoChannel(radioIdx) == CW_FALSE)?CW_TRUE:CW_FALSE;

    if (wlanEnable == CW_TRUE)
    {
        for (wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
        {
            if(CWWTPBoardGetWlanEnableCfg(radioIdx, wlanIdx, &wlanIdxEnable))
            {
                if(wlanIdxEnable == 1)
                {
                    bFound = CW_TRUE;
                    break;
                }
            }
        }
    }

    CWDebugLog("%s %d radioIdx:[%d] enable:[%d] wlanEnable:[%d] bFound:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), enable, wlanEnable, bFound);

    if(enable && wlanEnable && bFound)
    {
        CWSystem("sn_acs --r %d --rxth 90 --rxthnc 50 --n 6 --wt 600 --pt 10 --rs 1 >/dev/null 2>&1 &", WIFI_IF_ID(CWConvertRadioIdx(radioIdx)));
    }
    else
    {
        CWSystem("ps |grep sn_acs |grep \"\\-\\-r %d\" |awk '{printf $1}'|xargs kill -9 >/dev/null 2>&1 &", WIFI_IF_ID(CWConvertRadioIdx(radioIdx)));
    }

    sleep(1);

    return CW_TRUE;
}
#endif
CWBool CWWTPBoardGetRadioCurrentAvailableChannelList(int i32RadioIdx, unsigned char *pAvalaibleChanCount, unsigned char **pAvailableChanList)
{
    unsigned int uint32CapCode;
    CWRadioFreqType radioType;
    int i32WlanIdx, i32SsidEnable;
    CWBool bFound = CW_FALSE;
    unsigned char u8SupportChanCount;
    unsigned char *pSupportChanList;
    char tmpFile1[64];
    FILE *fp;
    long fsize;
    char *buffer, *c;
    char tmpstr[256];
    char *pch;
    int i = 0;
    int cur_wlmode, cur_ht, cur_extch;

    *pAvalaibleChanCount = 0;
    *pAvailableChanList = NULL;

    if(CWWTPBoardGetCapCode(&uint32CapCode) == CW_FALSE)
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(CWGetRadioType(&radioType, uint32CapCode, i32RadioIdx) == CW_FALSE)
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

#if SUPPORT_WLAN_5G_SETTING
#if SUPPORT_WLAN_5G_2_SETTING
    if (CWWTPBoardGetRadioNoChannel(i32RadioIdx) == CW_TRUE)
    {
        CWDebugLog("%s %d radioIdx:[%d] noChannel:[%d]", __FUNCTION__, __LINE__, i32RadioIdx, CWWTPBoardGetRadioNoChannel(i32RadioIdx));
        return CW_TRUE;
    }
#endif
#endif

    for(i32WlanIdx = 0; i32WlanIdx < CWWTPBoardGetMaxRadioWlans(i32RadioIdx); i32WlanIdx++)
    {
        if(CWWTPBoardGetWlanEnableCfg(i32RadioIdx, i32WlanIdx, &i32SsidEnable))
        {
            if(i32SsidEnable)
            {
                bFound = CW_TRUE;
                break;
            }
        }
    }
    if(bFound == CW_FALSE)
    {
        return CW_TRUE;
    }

    switch (radioType)
    {
#if SUPPORT_WLAN_24G_SETTING
    case CW_RADIOFREQTYPE_2G:
        sprintf(tmpFile1, "/tmp/CHANNELlIST.%s.%x.tmp", "2G", (unsigned int) CWThreadSelf());
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case CW_RADIOFREQTYPE_5G:
        sprintf(tmpFile1, "/tmp/CHANNELlIST.%s.%x.tmp", "5G", (unsigned int) CWThreadSelf());
        break;
#endif
#if SUPPORT_WLAN_5G_2_SETTING
    case CW_RADIOFREQTYPE_5G_1:
        sprintf(tmpFile1, "/tmp/CHANNELlIST.%s.%x.tmp", "5G_1", (unsigned int) CWThreadSelf());
        break;
#endif
    default:
        CWDebugLog("Bad Radio Frequency Type");
        return CW_TRUE;
    }
    //CWDebugLog("%s %d i32RadioIdx:[%d] ifname:[%s]", __FUNCTION__, __LINE__, i32RadioIdx, WLAN_IF_NAME(i32RadioIdx, i32WlanIdx));
    if(IS_11AC_RADIO(i32RadioIdx))
    {
        CWSystem("wlanconfig %s list chan | cut -c -49 | sed -e \"s/^.\\{8\\}\\(...\\)...\\(....\\)\\(.\\+\\) Mhz \\(....\\).\\(.\\)..\\(.\\) ...\\(.\\) ....\\(...\\)/\\4,\\5,\\6,\\1,\\7,\\8,/g\" -e \"s/[ ]//g\" -e \"s/CL/L/g\" -e \"s/CU/U/g\" > %s", WLAN_IF_NAME(i32RadioIdx, i32WlanIdx), tmpFile1);
        CWSystem("wlanconfig %s list chan | cut -c 76-|sed -e \"s/^  *//g\"|grep ^Chan | sed -e \"s/^.\\{8\\}\\(...\\)...\\(....\\)\\(.\\+\\) Mhz \\(....\\).\\(.\\)..\\(.\\) ...\\(.\\) ....\\(...\\)/\\4,\\5,\\6,\\1,\\7,\\8,/g\" -e \"s/[ ]//g\" -e \"s/CL/L/g\" -e \"s/CU/U/g\" >> %s", WLAN_IF_NAME(i32RadioIdx, i32WlanIdx), tmpFile1);
    }
    else
    {
        CWSystem("wlanconfig %s list chan | cut -c -38 | sed -e \"s/^.\\{8\\}\\(...\\)...\\(....\\)\\(.\\)\\(.\\).* Mhz \\(....\\).\\(.\\)..\\(.*\\)$/\\5,\\6,\\7,\\1,/g\" -e \"s/[ ]//g\" -e \"s/CL/L/g\" -e \"s/CU/U/g\" > %s", WLAN_IF_NAME(i32RadioIdx, i32WlanIdx), tmpFile1);
        CWSystem("wlanconfig %s list chan | cut -c 39-|sed -e \"s/^  *//g\"|grep ^Chan | sed -e \"s/^.\\{8\\}\\(...\\)...\\(....\\)\\(.\\)\\(.\\).* Mhz \\(....\\).\\(.\\)..\\(.*\\)$/\\5,\\6,\\7,\\1,/g\" -e \"s/[ ]//g\" -e \"s/CL/L/g\" -e \"s/CU/U/g\" >> %s", WLAN_IF_NAME(i32RadioIdx, i32WlanIdx), tmpFile1);
    }
    //CWSystem("cat %s > /dev/console", tmpFile1);

    fp = fopen(tmpFile1, "r");
    if(!fp)
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        return CW_TRUE;
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        fclose(fp);
        unlink(tmpFile1);
        return CW_TRUE;
    }

    if(fsize < 0)
    {
        fclose(fp);
        unlink(tmpFile1);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(!fsize)
    {
        u8SupportChanCount = 0;
        pSupportChanList = NULL;
        fclose(fp);
        unlink(tmpFile1);
        return CW_TRUE;
    }

    CW_CREATE_OBJECT_SIZE_ERR(buffer, fsize + 1,
    {
        fclose(fp);
        unlink(tmpFile1);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    });

    if(fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile1);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile1);
        return CW_TRUE;
    }

    buffer[fsize] = '\0';
    u8SupportChanCount = 0;
    c = buffer;
    while(*c != '\0')
    {
        if(*c == '\n')
        {
            (u8SupportChanCount)++;
        }
        c++;
    }
    CW_FREE_OBJECT(buffer);

    CW_CREATE_ZERO_ARRAY_ERR(pSupportChanList, u8SupportChanCount, unsigned char,
    {
        fclose(fp);
        unlink(tmpFile1);
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        return CW_TRUE;
    });

    CWWTPBoardGetRadioWirelessModeCfg(i32RadioIdx, &cur_wlmode);
    CWWTPBoardGetRadioChannelHTModeCfg(i32RadioIdx, &cur_ht);
    CWWTPBoardGetRadioChannelExtCfg(i32RadioIdx, &cur_extch);

    i = 0;
    while(fgets(tmpstr, 256, fp) != NULL && i < u8SupportChanCount)
    {
        if((pch = CWStrtokSingle(tmpstr, ",")) == NULL)
        {
            continue;
        }
        else
        {
            //CWDebugLog("%s %d i32RadioIdx:[%d] pch:[%s]", __FUNCTION__, __LINE__, i32RadioIdx, pch);
            if(!strcmp(pch, "11ng"))
            {
                if(!((cur_wlmode == CW_RADIO_WIRELESS_MODE_B)
                     || (cur_wlmode == CW_RADIO_WIRELESS_MODE_G)
                     || (cur_wlmode == CW_RADIO_WIRELESS_MODE_N_2G)
                     || (cur_wlmode == CW_RADIO_WIRELESS_MODE_BGN)
                     || (cur_wlmode == CW_RADIO_WIRELESS_MODE_BG)
                    ))
                {
                    continue;
                }
            }
            else if(!strcmp(pch, "11g"))
            {
                if(!((cur_wlmode == CW_RADIO_WIRELESS_MODE_B)
                     || (cur_wlmode == CW_RADIO_WIRELESS_MODE_G)
                     || (cur_wlmode == CW_RADIO_WIRELESS_MODE_BG)
                    ))
                {
                    continue;
                }
            }
            else if(!strcmp(pch, "11b"))
            {
                if(!(cur_wlmode == CW_RADIO_WIRELESS_MODE_B))
                {
                    continue;
                }
            }
            else if(!strcmp(pch, "11g_pure"))
            {
                if(!(cur_wlmode == CW_RADIO_WIRELESS_MODE_G))
                {
                    continue;
                }
            }
            else if(!strcmp(pch, "11n_pure"))
            {
                if(!(cur_wlmode == CW_RADIO_WIRELESS_MODE_N_2G))
                {
                    continue;
                }
            }
            else if(!strcmp(pch, "11na"))
            {
                if(!((cur_wlmode == CW_RADIO_WIRELESS_MODE_A)
                     || (cur_wlmode == CW_RADIO_WIRELESS_MODE_N_5G)
                     || (cur_wlmode == CW_RADIO_WIRELESS_MODE_AN)
                     || (cur_wlmode == CW_RADIO_WIRELESS_MODE_ACN_5G)
                    ))
                {
                    continue;
                }
            }
            else if(!strcmp(pch, "11a"))
            {
                if(!(cur_wlmode == CW_RADIO_WIRELESS_MODE_A))
                {
                    continue;
                }
            }
            else if(!strcmp(pch, "11na_pure"))
            {
                if(!(cur_wlmode == CW_RADIO_WIRELESS_MODE_N_5G))
                {
                    continue;
                }
            }
            else if(IS_11AC_RADIO(i32RadioIdx) && !strcmp(pch, "11naac"))
            {
                if(!((cur_wlmode == CW_RADIO_WIRELESS_MODE_A)
                     || (cur_wlmode == CW_RADIO_WIRELESS_MODE_N_5G)
                     || (cur_wlmode == CW_RADIO_WIRELESS_MODE_AN)
                     || (cur_wlmode == CW_RADIO_WIRELESS_MODE_ACN_5G)
                    ))
                {
                    continue;
                }
            }
            else
            {
                CWDebugLog("*** unknown support wireless mode");
                continue;
            }
        }

        if((pch = CWStrtokSingle(NULL, ",")) == NULL)
        {
            continue;
        }
        else
        {
            //CWDebugLog("%s %d i32RadioIdx:[%d] pch:[%s]", __FUNCTION__, __LINE__, i32RadioIdx, pch);
            if(!strcmp(pch, "C"))
            {
                //
            }
            else if(!strcmp(pch, ""))
            {
                if(radioType == CW_RADIOFREQTYPE_2G)
                {
                    if(((cur_wlmode == CW_RADIO_WIRELESS_MODE_N_2G) || (cur_wlmode == CW_RADIO_WIRELESS_MODE_BGN))
                       && (cur_ht == CW_RADIO_CHANNEL_HT_20_MHZ)
                      )
                    {
                        continue;
                    }
                }
                else
                {
                    if(((cur_wlmode == CW_RADIO_WIRELESS_MODE_N_5G) || (cur_wlmode == CW_RADIO_WIRELESS_MODE_AN) || (cur_wlmode == CW_RADIO_WIRELESS_MODE_ACN_5G))
                       && ((cur_ht == CW_RADIO_CHANNEL_HT_20_MHZ))
                      )
                    {
                        continue;
                    }
                }
            }
            else
            {
                CWDebugLog("*** unknown support HT20 mode");
                continue;
            }
        }

        if((pch = CWStrtokSingle(NULL, ",")) == NULL)
        {
            continue;
        }
        else
        {
            //CWDebugLog("%s %d i32RadioIdx:[%d] pch:[%s]", __FUNCTION__, __LINE__, i32RadioIdx, pch);
            if(!strcmp(pch, "U"))
            {
                if(radioType == CW_RADIOFREQTYPE_2G)
                {
                    if(((cur_wlmode == CW_RADIO_WIRELESS_MODE_N_2G) || (cur_wlmode == CW_RADIO_WIRELESS_MODE_BGN))
                       && ((cur_ht == CW_RADIO_CHANNEL_HT_20_40_MHZ) || (cur_ht == CW_RADIO_CHANNEL_HT_40_MHZ) || (cur_ht == CW_RADIO_CHANNEL_HT_80_MHZ))
                       && (cur_extch == CW_RADIO_EXTENSION_CHANNEL_LOWER)
                      )
                    {
                        continue;
                    }
                }
                else
                {
                    if(((cur_wlmode == CW_RADIO_WIRELESS_MODE_N_5G) || (cur_wlmode == CW_RADIO_WIRELESS_MODE_AN) || (cur_wlmode == CW_RADIO_WIRELESS_MODE_ACN_5G))
                       && ((cur_ht == CW_RADIO_CHANNEL_HT_20_40_MHZ) || (cur_ht == CW_RADIO_CHANNEL_HT_40_MHZ) || (cur_ht == CW_RADIO_CHANNEL_HT_80_MHZ))
                       && (cur_extch == CW_RADIO_EXTENSION_CHANNEL_LOWER)
                      )
                    {
                        continue;
                    }
                }
            }
            else if(!strcmp(pch, "UL"))
            {
                //
            }
            else if(!strcmp(pch, "L"))
            {
                if(radioType == CW_RADIOFREQTYPE_2G)
                {
                    if(((cur_wlmode == CW_RADIO_WIRELESS_MODE_N_2G) || (cur_wlmode == CW_RADIO_WIRELESS_MODE_BGN))
                       && ((cur_ht == CW_RADIO_CHANNEL_HT_20_40_MHZ) || (cur_ht == CW_RADIO_CHANNEL_HT_40_MHZ) || (cur_ht == CW_RADIO_CHANNEL_HT_80_MHZ))
                       && (cur_extch == CW_RADIO_EXTENSION_CHANNEL_UPPER)
                      )
                    {
                        continue;
                    }
                }
                else
                {
                    if(((cur_wlmode == CW_RADIO_WIRELESS_MODE_N_5G) || (cur_wlmode == CW_RADIO_WIRELESS_MODE_AN) || (cur_wlmode == CW_RADIO_WIRELESS_MODE_ACN_5G))
                       && ((cur_ht == CW_RADIO_CHANNEL_HT_20_40_MHZ) || (cur_ht == CW_RADIO_CHANNEL_HT_40_MHZ) || (cur_ht == CW_RADIO_CHANNEL_HT_80_MHZ))
                       && (cur_extch == CW_RADIO_EXTENSION_CHANNEL_UPPER)
                      )
                    {
                        continue;
                    }
                }
            }
            else if(!strcmp(pch, ""))
            {
                if(radioType == CW_RADIOFREQTYPE_2G)
                {
                    if(((cur_wlmode == CW_RADIO_WIRELESS_MODE_N_2G) || (cur_wlmode == CW_RADIO_WIRELESS_MODE_BGN))
                       && (cur_ht == CW_RADIO_CHANNEL_HT_40_MHZ)
                      )
                    {
                        continue;
                    }
                }
                else
                {
                    if(((cur_wlmode == CW_RADIO_WIRELESS_MODE_N_5G) || (cur_wlmode == CW_RADIO_WIRELESS_MODE_AN) || (cur_wlmode == CW_RADIO_WIRELESS_MODE_ACN_5G))
                       && ((cur_ht == CW_RADIO_CHANNEL_HT_40_MHZ) || (cur_ht == CW_RADIO_CHANNEL_HT_80_MHZ))
                      )
                    {
                        continue;
                    }
                }
            }
            else
            {
                CWDebugLog("*** unknown support HT20/40/80 mode");
                continue;
            }
        }

        if((pch = CWStrtokSingle(NULL, ",")) == NULL)
        {
            continue;
        }
        else
        {
            //CWDebugLog("%s %d i32RadioIdx:[%d] pch:[%s]", __FUNCTION__, __LINE__, i32RadioIdx, pch);
            pSupportChanList[(*pAvalaibleChanCount)++] = (char)atoi(pch);
        }
        i++;
    }

    fclose(fp);
    unlink(tmpFile1);

    CW_CREATE_ZERO_ARRAY_ERR(*pAvailableChanList, *pAvalaibleChanCount, unsigned char,
    {
        CWDebugLog("%s %d Get configuration fail", __func__, __LINE__);
        CW_FREE_OBJECT(pSupportChanList);
        return CW_TRUE;
    });

    for(i = 0; i < *pAvalaibleChanCount; i++)
    {
        (*pAvailableChanList)[i] = pSupportChanList[i];
    }

    CW_FREE_OBJECT(pSupportChanList);

    CWDebugLog("%s %d i32RadioIdx:[%d] pAvalaibleChanCount:[%u]", __FUNCTION__, __LINE__, i32RadioIdx, *pAvalaibleChanCount);

    return CW_TRUE;
}

CWBool CWWTPBoardGetTxPowerByStrength(int maxTxPower, int strength, int *txPower)
{
    int range[2], value[2], i, ratio;

    if(strength <= 10)
    {
        range[0] = 0;
        range[1] = 10;
    }
    else if(strength <= 25)
    {
        range[0] = 10;
        range[1] = 25;
    }
    else if(strength <= 50)
    {
        range[0] = 25;
        range[1] = 50;
    }
    else
    {
        range[0] = 50;
        range[1] = 100;
    }

    for(i = 0; i < 2; i++)
    {
        switch(range[i])
        {
#ifdef CW_VENDOR_ATI
            case 100: //%
                value[i] = maxTxPower;
                break;
            case 50: //%
                value[i] = (maxTxPower - 3);
                break;
            case 25: //%
                value[i] = (maxTxPower - 6);
                break;
            case 10: //%
                value[i] = (maxTxPower - 10);
                break;
#else
            case 100: //%
                value[i] = maxTxPower;
                break;
            case 50: //%
                value[i] = (((maxTxPower - 11) / 2) + 11);
                break;
            case 25: //%
                value[i] = (((maxTxPower - 11) / 4) + 11);
                break;
            case 10: //%
                value[i] = (((maxTxPower - 11) / 8) + 11);
                break;
#endif
            default:
                value[i] = 0;
        }
    }

    ratio = ((value[1] - value[0]) * 100) / (range[1] - range[0]);
    *txPower = value[1] - (((range[1] - strength) * ratio) / 100);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioAutoTxPowerStrength(CWRadioFreqType radioType, int strength)
{
    unsigned int capCode;
    int radioIdx, wlanIdx, ssidEnable, maxTxpower, txPower = 0;

    switch (radioType) 
    {
#if SUPPORT_WLAN_24G_SETTING
    case CW_RADIOFREQTYPE_2G:
        CWDebugLog("WTP auto txpower request radiotype: %s healing txpower: %d%%", "2.4G", txPower);
        break;
#endif
#if SUPPORT_WLAN_5G_SETTING
    case CW_RADIOFREQTYPE_5G:
        CWDebugLog("WTP auto txpower request radiotype: %s healing txpower: %d%%", "5G", txPower);
        break;
#endif
#if SUPPORT_WLAN_5G_2_SETTING
    case CW_RADIOFREQTYPE_5G_1:
        CWDebugLog("WTP auto txpower request radiotype: %s healing txpower: %d%%", "5G_1", txPower);
        break;
#endif
    default:
        CWDebugLog("Bad Radio Frequency Type");
        return CW_TRUE;
    }

    if(!CWWTPBoardGetCapCode(&capCode))
    {
        return CW_FALSE;
    }

    if(!CWGetRadioIndex(radioType, capCode, &radioIdx))
    {
        return CW_FALSE;
    }

#if SUPPORT_WLAN_5G_SETTING
#if SUPPORT_WLAN_5G_2_SETTING
    if (CWWTPBoardGetRadioNoChannel(radioIdx) == CW_TRUE)
    {
        CWDebugLog("%s %d radioIdx:[%d] noChannel:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), CWWTPBoardGetRadioNoChannel(radioIdx));
        return CW_TRUE;
    }
#endif
#endif

    if(!CWWTPBoardGetRadioMaxTxPower(radioIdx, &maxTxpower))
    {
        return CW_FALSE;
    }

    if(!CWWTPBoardGetTxPowerByStrength(maxTxpower, strength, &txPower))
    {
        return CW_FALSE;
    }

    for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
    {
        if(CWWTPBoardGetWlanEnableCfg(radioIdx, wlanIdx, &ssidEnable) && ssidEnable)
        {
            CWSystem("iwconfig %s txpower %d", WLAN_IF_NAME(CWConvertRadioIdx(radioIdx), wlanIdx), txPower);
            break;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanWpaPMFEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int val = 0;
    char option[63+1];

    if (GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)) == wlanIdx)
    {
        snprintf(option,sizeof(option),"wireless.wifi%d_guest.ieee80211w_enabled",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_guest_pmf_enable_option(option, &val))
        {
            val = 0;
        }
    }
    else
    {
        snprintf(option,sizeof(option),"wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_get_wifi_ifacex_pmf_enable_option_by_sectionname(OPM_AP, option, wlanIdx+1, &val))
        {
            val = 0;
        }
    }

    switch(val)
    {
        case 1:
            *enable = CW_TRUE;
            break;
        default:
            *enable = CW_FALSE;
            break;
    }

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanWpaPMFEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    char option[63+1];

    CWDebugLog("%s %d %d %d", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, enable);

    if (GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)) == wlanIdx)
    {
        snprintf(option,sizeof(option),"wireless.wifi%d_guest.ieee80211w_enabled",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_guest_pmf_enable_option(option, enable))
        {
            return CW_FALSE;
        }
    }
    else
    {
        snprintf(option,sizeof(option),"wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));
        if (api_set_wifi_ifacex_pmf_enable_option_by_sectionname(OPM_AP, option, wlanIdx+1, enable))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

/*bandsteer wlan*/
CWBool CWWTPBoardGetWlanBandSteeringMode(int radioIdx, int wlanIdx, int *mode)
{
    int bandsteer_mode=0;
    char option[64]={0};

    snprintf(option,sizeof(option),"wireless.wifi%d_ssid_%d.bandsteer_en",CWConvertRadioIdx(radioIdx),wlanIdx+1);

    if (api_get_integer_option(option, &bandsteer_mode))
    {
        *mode = 0;
        if (api_set_integer_option(option, 0))
        {
            CWDebugLog("Set Configuration Fail!!!");
            return CW_TRUE;
        }
    }
    else
        *mode = bandsteer_mode;

    CWDebugLog("%s %d %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, bandsteer_mode, *mode);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanBandSteeringMode(int radioIdx, int wlanIdx, int mode)
{
    char option[64]={0};

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, mode);

    snprintf(option,sizeof(option),"wireless.wifi%d_ssid_%d.bandsteer_en",CWConvertRadioIdx(radioIdx),wlanIdx+1);

    if (api_set_integer_option(option, mode))
    {
        CWDebugLog("Set Configuration Fail!!!");
        return CW_TRUE;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanBandSteeringPercentEnable(int radioIdx, int wlanIdx, int *enable)
{
    int mode = 0;

    if (CWWTPBoardGetWlanBandSteeringMode(radioIdx,wlanIdx,&mode)==CW_TRUE)
        *enable = (mode==3)?CW_TRUE:CW_FALSE;
    else
        *enable = CW_FALSE;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanBandSteeringPercentEnable(int radioIdx, int wlanIdx, int enable)
{
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanBandSteeringRssiEnable(int radioIdx, int wlanIdx, int *enable)
{
    int mode = 0;

    if (CWWTPBoardGetWlanBandSteeringMode(radioIdx,wlanIdx,&mode)==CW_TRUE)
        *enable = (mode==2||mode==3)?CW_TRUE:CW_FALSE;
    else
        *enable = CW_FALSE;

    CWDebugLog("%s %d %d %d %d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanBandSteeringRssiEnable(int radioIdx, int wlanIdx, int enable)
{
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanBandSteeringRssi(int radioIdx, int wlanIdx, int *rssi)
{
#if SUPPORT_WLAN_BANDSTEER_ENHANCE
    int val=0;
    char option[64]={0};

    snprintf(option,sizeof(option),"wireless.wifi%d_ssid_%d.bandsteerrssi",CWConvertRadioIdx(radioIdx),wlanIdx+1);
    if (api_get_wifi_band_steering_rssi_option(option, &val))
    {
#if SUPPORT_WLAN_5G_2_SETTING
        val = 25;
#else
        val = 30;
#endif
        if (api_set_wifi_band_steering_rssi_option(option, val))
        {
            *rssi = 0 ;
            CWDebugLog("Set Configuration Fail!!!");
            return CW_TRUE;
        }
    }

    *rssi = (val - 95) ;
#else
    *rssi = 0 ;
#endif
    CWDebugLog("%s %d %d %d band steering rssi:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *rssi);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanBandSteeringRssi(int radioIdx, int wlanIdx, int rssi)
{
    CWDebugLog("%s %d %d %d band steering rssi:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, rssi);
#if SUPPORT_WLAN_BANDSTEER_ENHANCE
    char option[64]={0};

    snprintf(option,sizeof(option),"wireless.wifi%d_ssid_%d.bandsteerrssi",CWConvertRadioIdx(radioIdx),wlanIdx+1);

    if (api_set_wifi_band_steering_rssi_option(option, (rssi + 95)))
    {
        CWDebugLog("Set Configuration Fail!!!");
        return CW_TRUE;
    }
#endif

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanBandSteeringPercent(int radioIdx, int wlanIdx, int *present)
{
#if SUPPORT_WLAN_BANDSTEER_ENHANCE
    char option[64]={0};

    snprintf(option,sizeof(option),"wireless.wifi%d_ssid_%d.bandsteerpersent",CWConvertRadioIdx(radioIdx),wlanIdx+1);

    if (api_get_wifi_band_steering_percent_option(option, present))
    {
        *present = 75;
        if (api_set_wifi_band_steering_percent_option(option, 75))
        {
            CWDebugLog("Set Configuration Fail!!!");
            return CW_TRUE;
        }
    }
#else
    *present = 0;
#endif
    CWDebugLog("%s %d %d %d band steering percent:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, *present);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanBandSteeringPercent(int radioIdx, int wlanIdx, int present)
{
    CWDebugLog("%s %d %d %d band steering percent:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, present);

#if SUPPORT_WLAN_BANDSTEER_ENHANCE
    char option[64]={0};

    snprintf(option,sizeof(option),"wireless.wifi%d_ssid_%d.bandsteerpersent",CWConvertRadioIdx(radioIdx),wlanIdx+1);

    if (api_set_wifi_band_steering_percent_option(option, present))
    {
        CWDebugLog("Set Configuration Fail!!!");
        return CW_TRUE;
    }
#endif

    return CW_TRUE;
}

CWBool CWWTPBoardSetDownloadModeCfg(int radioIdx, int wlanIdx, CWRateMode mode)
{
    char value[5] = {0};
    char *section = NULL;

    if ( mode == CW_RATE_KBPS )
        snprintf(value, sizeof(value), "Kbps");
    else
        snprintf(value, sizeof(value), "Mbps");

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    api_set_sntcd_down_rate_unit_option_by_sectionname(section, wlanIdx+1, value, sizeof(value));

    CWLog("%s %u %u %u", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, mode);

    return CW_TRUE;
}

CWBool CWWTPBoardGetDownloadModeCfg(int radioIdx, int wlanIdx, CWRateMode *mode)
{
    char value[5] = {0};
    char *section = NULL;

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    api_get_sntcd_down_rate_unit_option_by_sectionname(section, wlanIdx+1, value, sizeof(value));

    if ( strcasecmp(value,"Kbps") == 0 )
        *mode = CW_RATE_KBPS;
    else
        *mode = CW_RATE_MBPS;

    CWLog("%s %u %u %u", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, *mode);

    return CW_TRUE;
}

CWBool CWWTPBoardSetUploadModeCfg(int radioIdx, int wlanIdx, CWRateMode mode)
{
    char value[5] = {0};
    char *section = NULL;

    if ( mode == CW_RATE_KBPS )
        snprintf(value, sizeof(value), "Kbps");
    else
        snprintf(value, sizeof(value), "Mbps");

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    api_set_sntcd_up_rate_unit_option_by_sectionname(section, wlanIdx+1, value, sizeof(value));

    CWLog("%s %u %u %u", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, mode);

    return CW_TRUE;
}

CWBool CWWTPBoardGetUploadModeCfg(int radioIdx, int wlanIdx, CWRateMode *mode)
{
    char value[5] = {0};
    char *section = NULL;

    section = CWCreateString("wireless.wifi%d_ssid",CWConvertRadioIdx(radioIdx));

    api_get_sntcd_up_rate_unit_option_by_sectionname(section, wlanIdx+1, value, sizeof(value));

    if ( strcasecmp(value,"Kbps") == 0 )
        *mode = CW_RATE_KBPS;
    else
        *mode = CW_RATE_MBPS;

    CWLog("%s %u %u %u", __FUNCTION__, CWConvertRadioIdx(radioIdx), wlanIdx, *mode);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanL2IsolateWhiteMacListCfg(int radioIdx, int wlanIdx, int *count, CWMacAddress **macs)
{
    char *c;
    int i, len;
    unsigned int mac[6];
    char val[18*MAC_FILTER_NUM+1];

    CWDebugLog("%s %d radioIdx:%d wlanIdx:%d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);

    memset(val, 0, sizeof(val));

    if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        *count = 0;
        *macs = NULL;
        return CW_TRUE;
        // only support ap now
    }
    else
    {
        if (api_get_l2_isolation_whitelist_option_by_sectionname("wifi_ssid", wlanIdx+1, val, sizeof(val)))
        {
            *count = 0;
            *macs = NULL;
            return CW_TRUE;
        }
    }

    if (val[0] == '\0')
    {
        *count = 0;
        *macs = NULL;
        return CW_TRUE;
    }

    len = strlen(val);
    while(val[len - 1] == ' ')
    {
        val[len - 1] = '\0';
        len--;
    }

    *count = 1;
    c = val;
    while(*c != '\0')
    {
        if (*c == ' ')
        {
            (*count)++;
        }
        c++;
    }

    CW_CREATE_ARRAY_ERR(*macs, *count, CWMacAddress,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    i = 0;
    c = val;
    do
    {
        if (sscanf(c, "%x:%x:%x:%x:%x:%x",
                   &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6)
        {
            CW_FREE_OBJECT(*macs);
            return CWErrorRaise(CW_ERROR_GENERAL, NULL);
        }

        (*macs)[i][0] = (unsigned char) mac[0];
        (*macs)[i][1] = (unsigned char) mac[1];
        (*macs)[i][2] = (unsigned char) mac[2];
        (*macs)[i][3] = (unsigned char) mac[3];
        (*macs)[i][4] = (unsigned char) mac[4];
        (*macs)[i][5] = (unsigned char) mac[5];

        while(*c != ' ' && *c != '\0')
        {
            c++;
        }

        if (*c == '\0')
        {
            break;
        }
        else
        {
            c++;
        }
        i++;
    }
    while(1);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanL2IsolateWhiteMacListCfg(int radioIdx, int wlanIdx, int count, CWMacAddress *macs)
{
    char *macArray = NULL;
    int i, len;
    CWBool retv = CW_TRUE;

    CWDebugLog("%s %d radioIdx:%d wlanIdx:%d count:%d", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, count);

    if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        return CW_TRUE;
    }

    CW_CREATE_OBJECT_SIZE_ERR(macArray, 64 + (count * 18),
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);;
    });

    if (count > 0)
    {
        len = 0;
        i = 0;
        while(i < count)
        {
            sprintf(&macArray[len], "%s%02x:%02x:%02x:%02x:%02x:%02x",
                    i == 0 ? "" : " ", CW_MAC_PRINT_LIST(macs[i]));
            i++;
            len = strlen(macArray);
        }
        api_lower2upper_mac(macArray);
        if ( api_set_l2_isolation_whitelist_option_by_sectionname("wifi_ssid", wlanIdx+1, macArray, (64+(count*18))) )
        {
            retv = CW_FALSE;
            goto CWWTPBoardSetWlanL2IsolateWhiteMacListCfg_End;
        }
    }
    else
    {
        if (api_delete_l2_isolation_whitelist_option_by_sectionname("wifi_ssid", wlanIdx+1))
        {
            retv = CW_FALSE;
            goto CWWTPBoardSetWlanL2IsolateWhiteMacListCfg_End;
        }
    }

CWWTPBoardSetWlanL2IsolateWhiteMacListCfg_End:

    CW_FREE_OBJECT(macArray);

    return retv;
}

CWBool CWWTPBoardGetSntcdModeCfg(CWRateMode *mode)
{
    char value[5] = {0};

    api_get_string_option(SNTCD_RATE_UNIT_OPTION, value, sizeof(value));

    if ( strcasecmp(value,"Kbps") == 0 )
        *mode = CW_RATE_KBPS;
    else
        *mode = CW_RATE_MBPS;

    return CW_TRUE;
}

CWBool CWWTPBoardGetAcSupportTrafficRateUnit()
{
    return g_ac_support_tc_rate_unit;
}

CWBool CWWTPBoradPreSetConfig(CWACInfoValues *acinfo)
{
    int infoIndex=0;
    int infoCount=0;

    if(!acinfo)
    {
        CWLog("acinfo is NULL");
        return CW_FALSE;
    }

    /*check capility*/
    if(!acinfo->cfgCapInfo)
    {
        /*Ac capability is not supported before version c1.9.29 */
        CWLog("cfgCapInfo is NULL");
        return CW_FALSE;
    }
    else
    {
        if( acinfo->cfgCapInfo->count == 0)
        {
            /*Ac capability is not supported before version c1.9.29 */
            CWLog("Not support ac capability");
            return CW_FALSE;
        }
        infoCount=acinfo->cfgCapInfo->count ;
    }

    for(infoIndex =0 ; infoIndex < infoCount; infoIndex++)
    {
        if ( CW_WTP_CFG_CAP_CHECK(acinfo->cfgCapInfo->cfgCap[infoIndex], WTP_CFG_AP_WLAN_DOWNLOAD_LIMIT_MODE) ||
             CW_WTP_CFG_CAP_CHECK(acinfo->cfgCapInfo->cfgCap[infoIndex], WTP_CFG_AP_WLAN_UPLOAD_LIMIT_MODE) )
        {
            g_ac_support_tc_rate_unit = CW_TRUE;
        }
    }
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioBitRateCfg(int radioIdx, int *rate)
{
    switch( CWConvertRadioIdx(radioIdx) )
    {
        case 0:
            api_get_wifi_min_rate_option(WIRELESS_WIFI_MIN_RATE_OPTION, rate);
            break;
        case 1:
            api_get_wifi_min_rate_option(WIRELESS_WIFI_5G_MIN_RATE_OPTION, rate);
            break;
        case 2:
            api_get_wifi_min_rate_option(WIRELESS_WIFI_5G_2_MIN_RATE_OPTION, rate);
            break;
        default:
            CWDebugLog("Unsupport radioIdx!!!");
            return CW_FALSE;
            break;
    }

    CWDebugLog("%s %d %d radio bit rate:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), *rate);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioBitRateCfg(int radioIdx, int rate)
{
    CWDebugLog("%s %d %d radio bit rate:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), rate);

    switch( CWConvertRadioIdx(radioIdx) )
    {
        case 0:
            api_set_wifi_min_rate_option(WIRELESS_WIFI_MIN_RATE_OPTION, rate);
            break;
        case 1:
            api_set_wifi_5g_min_rate_option(WIRELESS_WIFI_5G_MIN_RATE_OPTION, rate);
            break;
        case 2:
            api_set_wifi_5g_min_rate_option(WIRELESS_WIFI_5G_2_MIN_RATE_OPTION, rate);
            break;
        default:
            CWDebugLog("Unsupport radioIdx!!!");
            return CW_FALSE;
            break;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanHotspot20Josn(int radioIdx, int wlanIdx, char **jsonStr)
{
#if SUPPORT_HOTSPOT_SETTING
    char msg[10] = {0}, *val = NULL;
    int error_code = 0;
    ssid_cfg_st ssid_cfg_p;
    ssid_cfg_p.opmode = OPM_AP;
    ssid_cfg_p.idx = wlanIdx+1;

    CWDebugLog("%s %d %d %d wlan hotspot20:[**********]\n", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);
    api_get_wifix_section_name(OPM_AP, CWConvertRadioIdx(radioIdx), ssid_cfg_p.section);
    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", HOTSPOT_JSONSTR_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
    CWDebugLog("%s %d %d %d %d\n", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)));
    if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        CWDebugLog("%s %d %d %d wlan hotspot20:[**********]\n", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);
        //CW_CREATE_STRING_FROM_STRING_ERR(val, "\"hotspot20\":{}", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        //CWDebugLog("%s %d %d %d wlan hotspot20:[**********]\n", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);
        //sprintf(val, "\"hotspot20\":{}");
        //CWDebugLog("%s %d %d %d wlan hotspot20:[**********]\n", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);
        //*jsonStr = b64_encode(val,strlen(val));
        //CWDebugLog("%s %d %d %d wlan hotspot20:[**********]\n", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);

        //sprintf(val, "null");
        sprintf(val, "{\"operator_friendly_name\": \"\",\"domain\": \"\",\"roaming_consortium\": \"\",\"enable\": 0,\"venue\": {\"group\": 0,\"type\": 0,\"desc\": []},\"network\": {\"type\": 0,\"auth_type\": 0,\"auth_url\": \"\"},\"hessid\": \"\"}");
        *jsonStr = b64_encode(val,strlen(val));

        return CW_TRUE;
    }

    else
    {
        CWDebugLog("%s %d %d %d wlan hotspot20:[**********]\n", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);
        json_get_hotspot20(&ssid_cfg_p, val, HOTSPOT_JSONSTR_SIZE, error_code, msg);
        CWDebugLog("%s %d %d %d wlan hotspot20:[**********]\n", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);
        *jsonStr = b64_encode(val,strlen(val));

        //if(val != NULL)
        //    CW_FREE_OBJECT(val);
    }

#else
    char *val = NULL;

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", 1024, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    sprintf(val, "{\"enable\":0}");

    *jsonStr = b64_encode(val,strlen(val));

#endif
    //if(val != NULL)
    //    CW_FREE_OBJECT(val);

    return CW_TRUE;

}

CWBool CWWTPBoardSetWlanHotspot20Josn(int radioIdx, int wlanIdx, char *jsonStr)
{
#if SUPPORT_HOTSPOT_SETTING
    char msg[10] = {0}, *jsonStr_decode = NULL;
    int error_code = 0;
    ssid_cfg_st ssid_cfg_p;

    ssid_cfg_p.opmode = OPM_AP;
    ssid_cfg_p.idx = wlanIdx+1;

    if(jsonStr == NULL)
    {
        CWDebugLog("%s %d %d %d \n", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);
        return CW_TRUE;
    }
    api_get_wifix_section_name(OPM_AP, CWConvertRadioIdx(radioIdx), ssid_cfg_p.section);
    CWDebugLog("%s %d %d %d %d\n", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)));
    if (wlanIdx == GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
    {
        CWDebugLog("%s %d %d %d \n", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);
        CWDebugLog("%s %d %d %d GUEST_WLAN_IDX\n", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);
        return CW_TRUE;
    }
    else
    {
        jsonStr_decode = b64_decode(jsonStr, strlen(jsonStr));
        json_set_hotspot20(&ssid_cfg_p, jsonStr_decode, error_code, msg);
        CW_FREE_OBJECT(jsonStr_decode);
    }
#else
    CWDebugLog("%s %d %d %d \n", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx);
#endif

    return CW_TRUE;
}
/*end*/
