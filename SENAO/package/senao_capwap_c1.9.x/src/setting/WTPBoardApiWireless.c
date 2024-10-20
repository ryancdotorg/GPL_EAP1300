#include "WTPBoardApiWireless.h"
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

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

static char *wlanIfNameRadio0[] = {"ath0", "ath01", "ath02", "ath03", "ath04", "ath05", "ath06", "ath07", "ath27"};
static char *wlanIfNameRadio1[] = {"ath1", "ath11", "ath12", "ath13", "ath14", "ath15", "ath16", "ath17", "ath57"};
#if SUPPORT_WLAN_5G_2_SETTING
static char *wlanIfNameRadio2[] = {"ath4", "ath41", "ath42", "ath43", "ath44", "ath45", "ath46", "ath47", "ath67"};
#endif

#define WIFI_IF_ID(_radio)   (_radio)

static unsigned char radio2GChannel[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
static unsigned char radio5GChannel[] = {36, 40, 44, 48,
                                         52, 56, 60, 64,
                                         100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140,
                                         149, 153, 157, 161, 165, 169, 173
                                        };

int CWWTPBoardGetMaxRadio()
{
    return WIFI_RADIO_NUM;
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

#if SUPPORT_WLAN_5G_SETTING
    if (api_get_wifi_disabled_option((radioIdx) ? WIRELESS_WIFI_5G_DISABLED_OPTION : WIRELESS_WIFI_DISABLED_OPTION, &disabled))
#else
    if (api_get_wifi_disabled_option(WIRELESS_WIFI_DISABLED_OPTION, &disabled))
#endif
    {
        return -1;
    }

    return disabled;
}

int CWWTPBoardGetRadioMode(int radioIdx)
{
    int opMode = 0;

#if SUPPORT_WLAN_5G_SETTING
    if (api_get_wifi_opmode_option((radioIdx) ? WIRELESS_WIFI_5G_OPMODE_OPTION : WIRELESS_WIFI_OPMODE_OPTION, &opMode))
#else
    if (api_get_wifi_opmode_option(WIRELESS_WIFI_OPMODE_OPTION, &opMode))
#endif
    {
        return -1;
    }
    //CWDebugLog("%s %d RadioMode:[%d]", __FUNCTION__, __LINE__, opMode);
    return opMode;
}

CWBool CWWTPBoardGetWlanBssid(int radioIdx, int wlanIdx, CWMacAddress bssid)
{
    char *val, *c;
    unsigned int mac_tmp[6];

    if(!(val = CWCreateStringByCmd("ifconfig %s 2> /dev/null | grep HWaddr", WLAN_IF_NAME(radioIdx, wlanIdx))))
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
            return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
    sysutil_get_wifix_maxTxPower(radioIdx, power);

    CWDebugLog("%s %d maxTxPower:[%d]", __FUNCTION__, __LINE__, *power);
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioOperationModeCfg(int radioIdx, int *mode)
{
    int disabled = 0, opMode = 0;

    disabled = CWWTPBoardGetRadioDisabled(radioIdx);
    opMode = CWWTPBoardGetRadioMode(radioIdx);

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
    int disabled = 0;
    CWDebugLog("%s %d %d", __FUNCTION__, radioIdx, mode);

    switch (mode)
    {
        case CW_RADIO_OPERATION_MODE_DISABLED:
            disabled = 1;
#if SUPPORT_WLAN_5G_SETTING
            api_set_wifi_opmode_option((radioIdx) ? WIRELESS_WIFI_5G_DISABLED_OPTION : WIRELESS_WIFI_DISABLED_OPTION, disabled);
#else
            api_set_wifi_disabled_option(WIRELESS_WIFI_DISABLED_OPTION, disabled);
#endif
            break;
        case CW_RADIO_OPERATION_MODE_AP:
            disabled = 0;
#if SUPPORT_WLAN_5G_SETTING
            api_set_wifi_disabled_option((radioIdx) ? WIRELESS_WIFI_5G_DISABLED_OPTION : WIRELESS_WIFI_DISABLED_OPTION, disabled);
            api_set_wifi_opmode_option((radioIdx) ? WIRELESS_WIFI_5G_OPMODE_OPTION : WIRELESS_WIFI_OPMODE_OPTION, SYS_OPM_AP);
#else
            api_set_wifi_disabled_option(WIRELESS_WIFI_DISABLED_OPTION, disabled);
            api_set_wifi_opmode_option(WIRELESS_WIFI_OPMODE_OPTION, SYS_OPM_AP);
#endif
            break;
        case CW_RADIO_OPERATION_MODE_AD_HOC:
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Do not support AD HOC");
        default:
            return CWErrorRaise(CW_ERROR_WRONG_ARG, "Bad Radio Operation Mode");;

    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioType(int radioIdx, CWRadioType *type)
{
    *type = (radioIdx == 0 ? (CW_802_DOT_11b | CW_802_DOT_11g | CW_802_DOT_11n) :
#if HWMODE_AC
             (CW_802_DOT_11a | CW_802_DOT_11n | CW_802_DOT_11ac));
#else
             (CW_802_DOT_11a | CW_802_DOT_11n));
#endif
    CWDebugLog("%s %d RadioType:[%d]", __FUNCTION__, __LINE__, *type);
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioMac(int radioIdx, CWMacAddress mac)
{
    char mac_buf[17+1];
    unsigned int mac_tmp[6];

    memset(mac_buf, 0, sizeof(mac_buf));
    memset(mac_tmp, 0, sizeof(mac_tmp));

    sysutil_get_wifixMacAddr(radioIdx, mac_buf, sizeof(mac_buf));

    if(sscanf(mac_buf, "%x:%x:%x:%x:%x:%x", &mac_tmp[0], &mac_tmp[1], &mac_tmp[2], &mac_tmp[3], &mac_tmp[4], &mac_tmp[5]) != 6)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    mac[0] = (unsigned char) mac_tmp[0];
    mac[1] = (unsigned char) mac_tmp[1];
    mac[2] = (unsigned char) mac_tmp[2];
    mac[3] = (unsigned char) mac_tmp[3];
    mac[4] = (unsigned char) mac_tmp[4];
    mac[5] = (unsigned char) mac_tmp[5];

    CWDebugLog("%s radio %u %02x:%02x:%02x:%02x:%02x:%02x", __FUNCTION__, radioIdx, CW_MAC_PRINT_LIST(mac));

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioCountryCodeCfg(int radioIdx, int *country)
{
#if SUPPORT_WLAN_5G_SETTING
    if (api_get_integer_option((radioIdx)?WIRELESS_WIFI_5G_COUNTRY_OPTION:WIRELESS_WIFI_COUNTRY_OPTION, country))
#else
    if (api_get_wifi_country_option(WIRELESS_WIFI_COUNTRY_OPTION, country))
#endif
    {
        return CW_FALSE;
    }
#if SUPPORT_JAPAN_BANDWIDTH_HT80
    *country = (*country == 4015)?392:*country;
#endif

    CWDebugLog("%s %d country:[%d]", __FUNCTION__, __LINE__, *country);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioCountryCodeCfg(int radioIdx, int country)
{
    int countryIdx;

    CWDebugLog("%s %d %d", __FUNCTION__, radioIdx, country);

    api_get_wifi_country_table_index(country, &countryIdx);

#if SUPPORT_JAPAN_BANDWIDTH_HT80
    country = (country == 392)?4015:country;
#endif

#if SUPPORT_WLAN_5G_SETTING
    if (api_set_wifi_country_option((radioIdx)?WIRELESS_WIFI_5G_COUNTRY_OPTION:WIRELESS_WIFI_COUNTRY_OPTION, countryIdx))
#else
    if (api_set_wifi_country_option(WIRELESS_WIFI_COUNTRY_OPTION, countryIdx))
#endif
    {
        return CW_FALSE;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioChannelHTModeCfg(int radioIdx, int *mode)
{
    int val = 0;

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_get_wifi_5g_htmode_option(WIRELESS_WIFI_5G_HTMODE_OPTION, &val))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_get_wifi_htmode_option(WIRELESS_WIFI_HTMODE_OPTION, &val))
        {
            return CW_FALSE;
        }
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
            return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    CWDebugLog("%s %d HTMode:[%d]", __FUNCTION__, __LINE__, *mode);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioChannelHTModeCfg(int radioIdx, int mode)
{
    int val = 0;

    CWDebugLog("%s %d %d", __FUNCTION__, radioIdx, mode);

    switch(mode)
    {
        case CW_RADIO_CHANNEL_HT_20_MHZ:
            val = BANDWIDTH_20MHZ;
            break;
        case CW_RADIO_CHANNEL_HT_20_40_MHZ:
            val = BANDWIDTH_20MHZ_40MHZ;
            break;
        case CW_RADIO_CHANNEL_HT_40_MHZ:
            val = BANDWIDTH_40MHZ;
            break;
#if SUPPORT_WLAN_5G_SETTING
        case CW_RADIO_CHANNEL_HT_80_MHZ:
            val = BANDWIDTH_80MHZ;
            break;
#endif
        default:
            return CWErrorRaise(CW_ERROR_WRONG_ARG, "Bad Channel HT Mode");
    }

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_set_wifi_5g_htmode_option(WIRELESS_WIFI_5G_HTMODE_OPTION, val))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_wifi_htmode_option(WIRELESS_WIFI_HTMODE_OPTION, val))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioChannelExtCfg(int radioIdx, int *mode)
{
    int val;
#if SUPPORT_WLAN_EXTENSION_CHANNEL
#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_get_wifi_extension_channel_option(WIRELESS_WIFI_5G_EXTENSION_CHANNEL_OPTION, &val))
        {
            val = 0;
        }
    }
    else
#endif
    {
        if (api_get_wifi_extension_channel_option(WIRELESS_WIFI_EXTENSION_CHANNEL_OPTION, &val))
        {
            val = 0;
        }
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

    CWDebugLog("%s %d ChannelExt:[%d]", __FUNCTION__, __LINE__, *mode);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioChannelExtCfg(int radioIdx, int mode)
{
    CWDebugLog("%s %d %d", __FUNCTION__, radioIdx, mode);
#if SUPPORT_WLAN_EXTENSION_CHANNEL
    if ( mode > CW_RADIO_EXTENSION_CHANNEL_LOWER || mode < CW_RADIO_EXTENSION_CHANNEL_UPPER)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "Bad Channel Ext");
    }

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_set_wifi_extension_channel_option(WIRELESS_WIFI_5G_EXTENSION_CHANNEL_OPTION, mode)) 
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_wifi_extension_channel_option(WIRELESS_WIFI_EXTENSION_CHANNEL_OPTION, mode)) 
        {
            return CW_FALSE;
        }
    }
#endif
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioChannelCfg(int radioIdx, int *mode)
{
    int val;
#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_get_wifi_5g_channel_option(WIRELESS_WIFI_5G_CHANNEL_OPTION, &val))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_get_wifi_channel_option(WIRELESS_WIFI_CHANNEL_OPTION, &val))
        {
            return CW_FALSE;
        }
    }

    *mode = (val == 0) ? CW_RADIO_CHANNEL_AUTO : val;
    CWDebugLog("%s %d RadioChannel:[%d]", __FUNCTION__, __LINE__, *mode);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioChannelCfg(int radioIdx, int mode)
{
    int val = 0;
    CWDebugLog("%s %d %d", __FUNCTION__, radioIdx, mode);

    val = (mode == CW_RADIO_CHANNEL_AUTO) ? 0 : mode;
#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx) 
    {
        if (api_set_wifi_5g_channel_option(WIRELESS_WIFI_5G_CHANNEL_OPTION, val))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_wifi_channel_option(WIRELESS_WIFI_CHANNEL_OPTION, val))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioWirelessModeCfg(int radioIdx, int *mode)
{
    int val = 0;

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_get_wifi_5g_hwmode_option(WIRELESS_WIFI_5G_HWMODE_OPTION, &val))
        {
            return CW_FALSE;
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
            default:
                return CWErrorRaise(CW_ERROR_GENERAL, NULL);
        }
    }
    else
#endif
    {
        if (api_get_wifi_hwmode_option(WIRELESS_WIFI_HWMODE_OPTION, &val))
        {
            return CW_FALSE;
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
            default:
                return CWErrorRaise(CW_ERROR_GENERAL, NULL);
        }
    }
    CWDebugLog("%s %d %d", __FUNCTION__, radioIdx, *mode);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioWirelessModeCfg(int radioIdx, int mode)
{
    int val = 0;

    CWDebugLog("%s %d %d", __FUNCTION__, radioIdx, mode);

    switch(mode)
    {
        case CW_RADIO_WIRELESS_MODE_B:
            val = P24G_IEEE802_11B;
            break;
        case CW_RADIO_WIRELESS_MODE_G:
            val = P24G_IEEE802_11G;
            break;
        case CW_RADIO_WIRELESS_MODE_N_2G:
            val = P24G_IEEE802_11N;
            break;
        case CW_RADIO_WIRELESS_MODE_BGN:
            val = P24G_IEEE802_11NG;
            break;
        case CW_RADIO_WIRELESS_MODE_BG:
            val = P24G_IEEE802_11BG;
            break;
#if SUPPORT_WLAN_5G_SETTING
        case CW_RADIO_WIRELESS_MODE_A:
            val = P5G_IEEE802_11A;
            break;
        case CW_RADIO_WIRELESS_MODE_N_5G:
            val = P5G_IEEE802_11N;
            break;
        case CW_RADIO_WIRELESS_MODE_AN:
            val = P5G_IEEE802_11NA;
            break;
        case CW_RADIO_WIRELESS_MODE_ACN_5G:
            val = P5G_IEEE802_11AC;
            break;
#endif
        default:
            return CWErrorRaise(CW_ERROR_WRONG_ARG, "Bad Wireless Mode %d for radio %d", mode, radioIdx);
    }
#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_set_wifi_5g_hwmode_option(WIRELESS_WIFI_5G_HWMODE_OPTION, val))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_wifi_hwmode_option(WIRELESS_WIFI_HWMODE_OPTION, val))
        {
            return CW_FALSE;
        }
    }
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioLimitedClientsEnableCfg(int radioIdx, int *enable)
{
    int val = 0;

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_get_integer_option(WIRELESS_WIFI_5G_CLIENTLIMIT_ENABLE_OPTION, &val))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_get_integer_option(WIRELESS_WIFI_CLIENTLIMIT_ENABLE_OPTION, &val))
        {
            return CW_FALSE;
        }
    }

    *enable = (val) ? CW_TRUE : CW_FALSE;

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioLimitedClientsEnableCfg(int radioIdx, int enable)
{
    CWDebugLog("%s %d LimitedClientsEnable:[%d]", __FUNCTION__, radioIdx, enable);

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_set_bool_option(WIRELESS_WIFI_5G_CLIENTLIMIT_ENABLE_OPTION, enable))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_bool_option(WIRELESS_WIFI_CLIENTLIMIT_ENABLE_OPTION, enable))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioLimitedClientsCfg(int radioIdx, int *clients)
{
    int val = 0;

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_get_integer_option(WIRELESS_WIFI_5G_CLIENTLIMIT_NUMBER_OPTION, &val))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_get_integer_option(WIRELESS_WIFI_CLIENTLIMIT_NUMBER_OPTION, &val))
        {
            return CW_FALSE;
        }
    }
    *clients = val;
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *clients);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioLimitedClientsCfg(int radioIdx, int clients)
{
    CWDebugLog("%s %d %d", __FUNCTION__, radioIdx, clients);

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_set_integer_option(WIRELESS_WIFI_5G_CLIENTLIMIT_NUMBER_OPTION, clients))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_integer_option(WIRELESS_WIFI_CLIENTLIMIT_NUMBER_OPTION, clients))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioTxPowerCfg(int radioIdx, int *power)
{
    int val = 0;

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_get_wifi_5g_txpower_option(WIRELESS_WIFI_5G_TXPOWER_OPTION, &val))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_get_wifi_txpower_option(WIRELESS_WIFI_TXPOWER_OPTION, &val))
        {
            return CW_FALSE;
        }
    }
    *power = val;
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *power);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioTxPowerCfg(int radioIdx, int power)
{

    CWDebugLog("%s %d %d", __FUNCTION__, radioIdx, power);

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_set_wifi_5g_txpower_option(WIRELESS_WIFI_5G_TXPOWER_OPTION, power))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_wifi_txpower_option(WIRELESS_WIFI_TXPOWER_OPTION, power))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioAggregationEnableCfg(int radioIdx, int *enable)
{
    int val = 0;

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_get_wifi_aggr_enabled_option(WIRELESS_WIFI_5G_AGGR_ENABLED_OPTION, &val))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_get_wifi_aggr_enabled_option(WIRELESS_WIFI_AGGR_ENABLED_OPTION, &val))
        {
            return CW_FALSE;
        }
    }

    *enable = val;
    CWDebugLog("%s %d %d %d", __FUNCTION__, __LINE__, radioIdx, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioAggregationEnableCfg(int radioIdx, int enable)
{
    CWDebugLog("%s radioIdx:[%d] enable:[%d]", __FUNCTION__, radioIdx, enable);

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_set_wifi_aggr_enabled_option(WIRELESS_WIFI_5G_AGGR_ENABLED_OPTION, enable))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_wifi_aggr_enabled_option(WIRELESS_WIFI_AGGR_ENABLED_OPTION, enable))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioAggregationFramesCfg(int radioIdx, int *frames)
{
    int val = 0;

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_get_wifi_aggr_frame_option(WIRELESS_WIFI_5G_AGGR_FRAME_OPTION, &val))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_get_wifi_aggr_frame_option(WIRELESS_WIFI_AGGR_FRAME_OPTION, &val))
        {
            return CW_FALSE;
        }
    }

    *frames = val;
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *frames);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioAggregationFramesCfg(int radioIdx, int frames)
{

    CWDebugLog("%s %d %d", __FUNCTION__, radioIdx, frames);

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_set_wifi_aggr_frame_option(WIRELESS_WIFI_5G_AGGR_FRAME_OPTION, frames))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_wifi_aggr_frame_option(WIRELESS_WIFI_AGGR_FRAME_OPTION, frames))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioAggregationMaxBytesCfg(int radioIdx, int *maxBytes)
{
    int val = 0;

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
        if (api_get_wifi_aggr_byte_option(WIRELESS_WIFI_5G_AGGR_BYTE_OPTION, &val))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_get_wifi_aggr_byte_option(WIRELESS_WIFI_AGGR_BYTE_OPTION, &val))
        {
            return CW_FALSE;
        }
    }

    *maxBytes = val;
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *maxBytes);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioAggregationMaxBytesCfg(int radioIdx, int maxBytes)
{
    CWDebugLog("%s %d %d", __FUNCTION__, radioIdx, maxBytes);

    if (maxBytes == 0)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "Aggregation maxBytes can't be 0");
    }
#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
#if HWMODE_AC || HWMODE_AX
        if (maxBytes != 65535)
        {
            return CWErrorRaise(CW_ERROR_WRONG_ARG, "Aggregation Max Bytes can't be changed on 11ac mode");
        }
#endif
        if (api_set_wifi_aggr_byte_option(WIRELESS_WIFI_5G_AGGR_BYTE_OPTION, maxBytes))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_wifi_aggr_byte_option(WIRELESS_WIFI_AGGR_BYTE_OPTION, maxBytes))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int opMode = 0, index = 0, val = 1;

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
#if SUPPORT_WLAN_5G_SETTING
        if (radioIdx)
        {
            if (api_get_wifi_guest_disabled_option(WIRELESS_WIFI_5G_GUEST_DISABLED_OPTION, &val))
            {
                return CW_FALSE;
            }
        }
        else
#endif
        {
            if (api_get_wifi_guest_disabled_option(WIRELESS_WIFI_GUEST_DISABLED_OPTION, &val))
            {
                return CW_FALSE;
            }
        }
    }
    else
    {
        if (api_get_wifi_ifacex_disabled_option((index + wlanIdx), &val))
        {
            return CW_FALSE;
        }
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

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, enable);

    int opMode = 0, index = 0;

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
#if SUPPORT_WLAN_5G_SETTING
        if (radioIdx)
        {
            if (api_set_wifi_guest_disabled_option(WIRELESS_WIFI_5G_GUEST_DISABLED_OPTION, !enable))
            {
                return CW_FALSE;
            }
        }
        else
#endif
        {
            if (api_set_wifi_guest_disabled_option(WIRELESS_WIFI_GUEST_DISABLED_OPTION, !enable))
            {
                return CW_FALSE;
            }
        }
    }
    else
    {
        if (api_set_wifi_ifacex_disabled_option((index + wlanIdx), !enable))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanSsidCfg(int radioIdx, int wlanIdx, char **pstr)
{
    int opMode = 0, index = 0;
    char *ssid = NULL;

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    CW_CREATE_STRING_ERR(ssid, MAX_SSID_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
#if SUPPORT_WLAN_5G_SETTING
        if (radioIdx)
        {
            if (api_get_string_option(WIRELESS_WIFI_5G_GUEST_SSID_OPTION, ssid, MAX_SSID_SIZE+1))
            {
                return CW_FALSE;
            }
        }
        else
#endif
        {
            if (api_get_string_option(WIRELESS_WIFI_GUEST_SSID_OPTION, ssid, MAX_SSID_SIZE+1))
            {
                return CW_FALSE;
            }
        }

    }
    else
    {
        if (api_get_wifi_ifacex_ssid_option((index + wlanIdx), ssid, MAX_SSID_SIZE+1))
        {
            return CW_FALSE;
        }
    }

    *pstr = ssid;
    CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, *pstr);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanSsidCfg(int radioIdx, int wlanIdx, char *pstr)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %s", __FUNCTION__, radioIdx, wlanIdx, pstr);

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
#if SUPPORT_WLAN_5G_SETTING
        if (radioIdx)
        {
            if (api_set_string_option(WIRELESS_WIFI_5G_GUEST_SSID_OPTION, pstr, MAX_SSID_SIZE))
            {
                return CW_FALSE;
            }
        }
        else
#endif
        {
            if (api_set_string_option(WIRELESS_WIFI_GUEST_SSID_OPTION, pstr, MAX_SSID_SIZE))
            {
                return CW_FALSE;
            }
        }
        return CW_TRUE;
    }
    else
    {
        if (api_set_wifi_ifacex_ssid_option((index + wlanIdx), pstr, MAX_SSID_SIZE))
        {
            return CW_FALSE;
        }        
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanSuppressedSsidCfg(int radioIdx, int wlanIdx, int *enable)
{
    int opMode = 0, index = 0, val = 0;

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
#if SUPPORT_WLAN_5G_SETTING
        if (radioIdx)
        {
            if (api_get_integer_option(WIRELESS_WIFI_5G_GUEST_HIDDEN_OPTION, &val))
            {
                return CW_FALSE;
            }
        }
        else
#endif
        {
            if (api_get_integer_option(WIRELESS_WIFI_GUEST_HIDDEN_OPTION, &val))
            {
                return CW_FALSE;
            }
        }
    }
    else
    {
        if (api_get_wifi_ifacex_hidden_option((index+wlanIdx), &val))
        {
            return CW_FALSE;
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
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanSuppressedSsidCfg(int radioIdx, int wlanIdx, int enable)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, enable);

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
#if SUPPORT_WLAN_5G_SETTING
        if (radioIdx)
        {
            if (api_set_integer_option(WIRELESS_WIFI_5G_GUEST_HIDDEN_OPTION, enable))
            {
                return CW_FALSE;
            }
        }
        else
#endif
        {
            if (api_set_integer_option(WIRELESS_WIFI_GUEST_HIDDEN_OPTION, enable))
            {
                return CW_FALSE;
            }
        }
    }
    else
    {
        if (api_set_wifi_ifacex_hidden_option((index+wlanIdx), enable))
        {
            return CW_FALSE;
        }
    }
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanStationSeparationCfg(int radioIdx, int wlanIdx, int *enable)
{
    int opMode = 0, index = 0, val;

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
#if SUPPORT_WLAN_5G_SETTING
        if (radioIdx)
        {
            if (api_get_integer_option(WIRELESS_WIFI_5G_GUEST_ISOLATE_OPTION, &val))
            {
                return CW_FALSE;
            }
        }
        else
#endif
        {
            if (api_get_integer_option(WIRELESS_WIFI_GUEST_ISOLATE_OPTION, &val))
            {
                return CW_FALSE;
            }
        }
    }
    else
    {
        if (api_get_wifi_ifacex_isolate_option((index+wlanIdx), &val))
        {
            return CW_FALSE;
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
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanStationSeparationCfg(int radioIdx, int wlanIdx, int enable)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, enable);

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
#if SUPPORT_WLAN_5G_SETTING
        if (radioIdx)
        {
            if (api_set_integer_option(WIRELESS_WIFI_5G_GUEST_ISOLATE_OPTION, enable))
            {
                return CW_FALSE;
            }
        }
        else
#endif
        {
            if (api_set_integer_option(WIRELESS_WIFI_GUEST_ISOLATE_OPTION, enable))
            {
                return CW_FALSE;
            }
        }
    }
    else
    {
        if (api_set_wifi_ifacex_isolate_option((index+wlanIdx), enable))
        {
            return CW_FALSE;
        }
    }
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanLayer2IsolationCfg(int radioIdx, int wlanIdx, int *enable)
{
    int opMode = 0, index = 0, val = 0;

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        val = 0;
    }
    else
    {
        if (api_get_wifi_ifacex_l2_isolation_option((index+wlanIdx), &val))
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

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanLayer2IsolationCfg(int radioIdx, int wlanIdx, int enable)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, enable);

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        return CW_TRUE;
    }
    else
    {
        if (api_set_wifi_ifacex_l2_isolation_option((index+wlanIdx), enable))
        {
            return CW_FALSE;
        }
    }
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanIsolationCfg(int radioIdx, int wlanIdx, int *enable)
{
    int opMode = 0, index = 0, val;

    if (GUEST_WLAN_IDX(radioIdx) == wlanIdx)
    {
        *enable = CW_FALSE;
    }
    else
    {
        opMode = CWWTPBoardGetRadioMode(radioIdx);

        if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
        {
            return CW_FALSE;
        }

        if (api_get_wifi_ifacex_isolation_option((index+wlanIdx), &val))
        {
            return CW_FALSE;
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
    }
    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanIsolationCfg(int radioIdx, int wlanIdx, int enable)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(enable)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Isolation");
        }
        return CW_TRUE;
    }
    else
    {
        opMode = CWWTPBoardGetRadioMode(radioIdx);

        if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
        {
            return CW_FALSE;
        }

        if (api_set_wifi_ifacex_isolation_option((index+wlanIdx), enable))
        {
            return CW_FALSE;
        }
    }
    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanVlanCfg(int radioIdx, int wlanIdx, int *vlan)
{
    int opMode = 0, index = 0;

    if (GUEST_WLAN_IDX(radioIdx) == wlanIdx)
    {
        *vlan = 1;
    }
    else
    {
        opMode = CWWTPBoardGetRadioMode(radioIdx);

        if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
        {
            return CW_FALSE;
        }

        if (api_get_wifi_ifacex_vlan_id_option((index+wlanIdx), vlan))
        {
            return CW_FALSE;
        }
    }
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *vlan);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanVlanCfg(int radioIdx, int wlanIdx, int vlan)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, vlan);

    if(vlan <= 0 || vlan > 4094)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid VLAN ID");
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(vlan != 1)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot change VLAN ID");
        }
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if (api_set_wifi_ifacex_vlan_id_option((index+wlanIdx), vlan))
    {
        return CW_FALSE;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanSecurityCfg(int radioIdx, int wlanIdx, int *mode)
{
    int opMode = 0, index = 0, encryption = 0;

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
#if SUPPORT_WLAN_5G_SETTING
        if (radioIdx)
        {
            if (api_get_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_OPTION, &encryption))
            {
                return CW_FALSE;
            }
        }
        else
#endif
        {
            if (api_get_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTION_OPTION, &encryption))
            {
                return CW_FALSE;
            }
        }
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
        if (api_get_wifi_ifacex_encryption_option((index+wlanIdx), &encryption))
        {
            return CW_FALSE;
        }
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
        else
        {
            *mode = CW_WLAN_SECURITY_MODE_NONE;
        }
    }
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *mode);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanSecurityNONECfg(int radioIdx, int wlanIdx, int index)
{
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        switch(radioIdx)
        {
#if SUPPORT_WLAN_5G_SETTING
            case 1:
                api_set_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_OPTION, GUEST_ENCRYPTION_NONE);
                break;
#endif
            default:
                api_set_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTION_OPTION, GUEST_ENCRYPTION_NONE);
                break;
        }
    }
    else
    {
        if (api_set_wifi_ifacex_encryption_option((index+wlanIdx), ENCRYPTION_NONE))
        {
            return CW_FALSE;
        }
    }
}

CWBool CWWTPBoardSetWlanSecurityWEPCfg(int radioIdx, int wlanIdx, int index, int auth)
{
    int sec_mode = 0, keyid = 0, auth_type = 0;

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] index:[%d] auth:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, index, auth);

    if(wlanIdx != 0)
    {
        return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Only WLAN0 can support WEP Security");
    }

    sec_mode = (auth==WEP_AUTH_SHARED)?WEP_SHARED:WEP_OPEN;

    if (api_set_wifi_ifacex_encryption_option((index+wlanIdx), sec_mode))
    {
        return CW_FALSE;
    }

    /* Check WEP variables are present in UCI, if not, set to default value */
    /* Wep Auth */
    if (api_get_wifi_ifacex_wep_auth_option((index+wlanIdx), &auth_type))
    {
        api_set_wifi_ifacex_wep_auth_option((index+wlanIdx), auth);
    }
    /* WepKeyIdx */
    if (api_get_wifi_ifacex_wepkey_id_option((index+wlanIdx), &keyid))
    {
        api_set_wifi_ifacex_wepkey_id_option((index+wlanIdx), 1);
    }
}

CWBool CWWTPBoardSetWlanSecurityWPACfg(int radioIdx, int wlanIdx, int index, int mode, int type)
{
    int sec_mode = 0;

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] index:[%d] mode:[%d] type:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, index, mode, type);

    switch(mode)
    {
        case CW_WLAN_SECURITY_MODE_WPA_PSK:
            if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
            {
                sec_mode = (type==WPA_ENC_CCMP)?GUEST_WPA_PSK_CCMP:(type==WPA_ENC_TKIP)?GUEST_WPA_PSK_TKIP:GUEST_WPA_PSK_TKIP_CCMP;
            }
            else
            {
                sec_mode = (type==WPA_ENC_CCMP)?WPA_PSK_CCMP:(type==WPA_ENC_TKIP)?WPA_PSK_TKIP:WPA_PSK_TKIP_CCMP;
            }
            break;
        case CW_WLAN_SECURITY_MODE_WPA2_PSK:
            if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
            {
                sec_mode = (type==WPA_ENC_CCMP)?GUEST_WPA2_PSK_CCMP:(type==WPA_ENC_TKIP)?GUEST_WPA2_PSK_TKIP:GUEST_WPA2_PSK_TKIP_CCMP;
            }
            else
            {
                sec_mode = (type==WPA_ENC_CCMP)?WPA2_PSK_CCMP:(type==WPA_ENC_TKIP)?WPA2_PSK_TKIP:WPA2_PSK_TKIP_CCMP;
            }
            break;
        case CW_WLAN_SECURITY_MODE_WPA_PSK_MIXED:
            if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
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
        default:
            return CW_FALSE;
    }

    //CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] index:[%d] sec_mode:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, index, sec_mode);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        switch(radioIdx)
        {
#if SUPPORT_WLAN_5G_SETTING
            case 1:
		if (api_set_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_OPTION, sec_mode))
		{
                    return CW_FALSE;
		}
                if (api_set_wifi_guest_wpa_enc_option(WIRELESS_WIFI_5G_GUEST_WPA_ENC_OPTION, type))
                {
                    return CW_FALSE;
                }
                break;
#endif
            default:
		if (api_set_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTION_OPTION, sec_mode))
		{
                    return CW_FALSE;
		}
                if (api_set_wifi_guest_wpa_enc_option(WIRELESS_WIFI_GUEST_WPA_ENC_OPTION, type))
                {
                    return CW_FALSE;
                }
                break;
        }
    }
    else
    {
        if (api_set_wifi_ifacex_encryption_option((index+wlanIdx), sec_mode))
        {
            return CW_FALSE;
        }
        if (api_set_wifi_ifacex_wpa_enc_option((index+wlanIdx), type))
        {
            return CW_FALSE;
        }
    }
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanSecurityCfg(int radioIdx, int wlanIdx, int mode)
{

    int opMode = 0, index = 0, sec_mode = 0, auth_type = 0, hw_mode=0;

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] mode:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, mode);

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    switch (mode)
    {
        case CW_WLAN_SECURITY_MODE_NONE:
            CWWTPBoardSetWlanSecurityNONECfg(radioIdx, wlanIdx, index);
            break;
        case CW_WLAN_SECURITY_MODE_WEP:
            if (wlanIdx == GUEST_WLAN_IDX(radioIdx))
            {
                return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support WEP Security");
            }
            if (api_get_wifi_ifacex_wep_auth_option((index+wlanIdx), &auth_type))
            {
                auth_type = WEP_AUTH_OPEN;
            }
            CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] sec_mode:[%d] auth_type:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, sec_mode, auth_type);
            CWWTPBoardSetWlanSecurityWEPCfg(radioIdx, wlanIdx, index, auth_type);
            break;
        case CW_WLAN_SECURITY_MODE_WPA_PSK:
        case CW_WLAN_SECURITY_MODE_WPA2_PSK:
        case CW_WLAN_SECURITY_MODE_WPA_PSK_MIXED:
            if (!CWWTPBoardGetRadioWirelessModeCfg(radioIdx, &hw_mode))
            {
                hw_mode = (radioIdx)?CW_RADIO_WIRELESS_MODE_N_5G:CW_RADIO_WIRELESS_MODE_N_2G;
            }
            CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] mode:[%d], hw_mode:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, mode, hw_mode);
            if (wlanIdx == GUEST_WLAN_IDX(radioIdx))
            {
                switch(radioIdx)
                {
#if SUPPORT_WLAN_5G_SETTING
                    case 1:
                        if (api_get_wifi_guest_wpa_enc_option(WIRELESS_WIFI_5G_GUEST_WPA_ENC_OPTION, &auth_type))
                        {
                            auth_type = WPA_ENC_TKIP_CCMP;
                            CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] auth_type:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, auth_type);
                        }
                        break;
#endif
                    default:
                        if (api_get_wifi_guest_wpa_enc_option(WIRELESS_WIFI_GUEST_WPA_ENC_OPTION, &auth_type))
                        {
                            auth_type = WPA_ENC_TKIP_CCMP;
                            CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] auth_type:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, auth_type);
                        }
                        break;
                }
            }
            else
            {
                if (api_get_wifi_ifacex_wpa_enc_option((index+wlanIdx), &auth_type))
                {
                    auth_type = WPA_ENC_TKIP_CCMP;
                    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] auth_type:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, auth_type);
                }
            }
#if SUPPORT_WLAN_5G_SETTING
            if (radioIdx)
                auth_type = (hw_mode == CW_RADIO_WIRELESS_MODE_N_5G) ? WPA_ENC_CCMP : auth_type;
            else
#endif
                auth_type = (hw_mode == CW_RADIO_WIRELESS_MODE_N_2G) ? WPA_ENC_CCMP : auth_type;
            CWWTPBoardSetWlanSecurityWPACfg(radioIdx, wlanIdx, index, mode, auth_type);
            break;
        case CW_WLAN_SECURITY_MODE_WPA:
        case CW_WLAN_SECURITY_MODE_WPA2:
        case CW_WLAN_SECURITY_MODE_WPA_MIXED:
            if (!CWWTPBoardGetRadioWirelessModeCfg(radioIdx, &hw_mode))
            {
                hw_mode = (radioIdx)?CW_RADIO_WIRELESS_MODE_N_5G:CW_RADIO_WIRELESS_MODE_N_2G;
            }
            CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] mode:[%d], hw_mode:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, mode, hw_mode);
            if (wlanIdx == GUEST_WLAN_IDX(radioIdx))
            {
                return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support WPA Security");
            }
            else
            {
                if (api_get_wifi_ifacex_wpa_enc_option((index+wlanIdx), &auth_type))
                {
                    auth_type = WPA_ENC_TKIP_CCMP;
                    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] auth_type:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, auth_type);
                }
#if SUPPORT_WLAN_5G_SETTING
                if (radioIdx)
                    auth_type = (hw_mode == CW_RADIO_WIRELESS_MODE_N_5G) ? WPA_ENC_CCMP : auth_type;
                else
#endif
                    auth_type = (hw_mode == CW_RADIO_WIRELESS_MODE_N_2G) ? WPA_ENC_CCMP : auth_type;

                CWWTPBoardSetWlanSecurityWPACfg(radioIdx, wlanIdx, index, mode, auth_type);
            }
            break;
        default:
            return CWErrorRaise(CW_ERROR_WRONG_ARG, "Bad Wlan Security");
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWepAuthTypeCfg(int radioIdx, int wlanIdx, int *type)
{
    int opMode = 0, index = 0, sec_mode = 0;

    if(wlanIdx != 0)
    {
        *type = CW_WEP_AUTH_TYPE_OPEN_SYSTEM;
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (api_get_wifi_ifacex_encryption_option((index+wlanIdx), &sec_mode))
    {
        return CW_FALSE;
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

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *type);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWepAuthTypeCfg(int radioIdx, int wlanIdx, int type)
{
    int opMode = 0, index = 0, encryption = 0, sec_mode = 0, auth_mode = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, type);

    if(wlanIdx != 0)
    {
        if(type != CW_WEP_AUTH_TYPE_OPEN_SYSTEM)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Only WLAN0 can support WEP Auth Type");
        }
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (api_get_wifi_ifacex_encryption_option((index+wlanIdx), &encryption))
    {
        return CW_FALSE;
    }

    CWDebugLog("%s %d %d encryption:[%d]", __FUNCTION__, radioIdx, wlanIdx, encryption);

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
                return CWErrorRaise(CW_ERROR_WRONG_ARG, "Bad Auth Type");
        }

        CWDebugLog("%s %d %d type:[%d] sec_mode:[%d] auth_mode:[%d]", __FUNCTION__, radioIdx, wlanIdx, type, sec_mode, auth_mode);
        if (!api_set_wifi_ifacex_encryption_option((index+wlanIdx), sec_mode))
        {
            if (api_set_wifi_ifacex_wep_auth_option((index+wlanIdx), auth_mode))
            {
                return CW_FALSE;
            }
        }
        else
        {
            return CW_FALSE;
        }
    }
    return CW_TRUE;
}

CWBool CWWTPBoardGetWepInputMethodCfg(int radioIdx, int wlanIdx, int *method)
{
    int opMode = 0, index = 0, keyIdx = 0;
    char val[34+1]={0}, *c = NULL, option[API_OPTION_SIZE];

    *method = CW_WEP_INPUT_METHOD_HEX;

    if(wlanIdx != 0)
    {
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    api_get_wifi_ifacex_wepkey_id_option((index+wlanIdx), &keyIdx);

    sprintf(option, WIRELESS_WIFI_IFACEX_OPTION_FORMAT"%d", (index+wlanIdx), "key", keyIdx);

    api_get_string_option(option, val, sizeof(val));

    if ((c = strstr(val, "s:")))
    {
        *method = CW_WEP_INPUT_METHOD_ASCII;
    }

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *method);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWepInputMethodCfg(int radioIdx, int wlanIdx, int method)
{
    int opMode = 0, index = 0, keyIdx;
    char val[34+1], wepkey[34+1], *c, option[API_OPTION_SIZE];

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, method);

    if(wlanIdx != 0)
    {
        if(method != CW_WEP_INPUT_METHOD_HEX)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Only WLAN0 can support WEP Input Method");
        }
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    for (keyIdx = 0; keyIdx<4; keyIdx++)
    {
        memset(val, 0, sizeof(val));

        sprintf(option, WIRELESS_WIFI_IFACEX_OPTION_FORMAT"%d", (index+wlanIdx), "key", keyIdx);

        if (api_get_string_option(option, val, sizeof(val)))
        {
            return CW_FALSE;
        }
        switch(method)
        {
            case CW_WEP_INPUT_METHOD_HEX:
                if ((c = strstr(val,"s:")))
                {
                    if (api_set_wifi_ifacex_wepkey_keyx_option((index+wlanIdx), keyIdx, c+2, sizeof(val)))
                    {
                        return CW_FALSE;
                    }
                }
                break;
            case CW_WEP_INPUT_METHOD_ASCII:
                if (strstr(val,"s:"))
                {
                    snprintf(wepkey, sizeof(wepkey), "%s", val);
                    if (api_set_wifi_ifacex_wepkey_keyx_option((index+wlanIdx), keyIdx, wepkey, sizeof(val)))
                    {
                        return CW_FALSE;
                    }
                }
                break;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWepDefaultKeyIdCfg(int radioIdx, int wlanIdx, int *keyId)
{
    int opMode = 0, index = 0, val = 1;

    if(wlanIdx != 0)
    {
        *keyId = 0;
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (api_get_wifi_ifacex_wepkey_id_option((index+wlanIdx), &val))
    {
        if (api_set_wifi_ifacex_wepkey_id_option((index+wlanIdx), val))
        {
            return CW_FALSE;
        }
    }

    *keyId = val - 1;

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *keyId);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWepDefaultKeyIdCfg(int radioIdx, int wlanIdx, int keyId)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, keyId);

    if(wlanIdx != 0)
    {
        if(keyId != 0)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Only WLAN0 can support WEP Default Key");
        }
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (api_set_wifi_ifacex_wepkey_id_option((index+wlanIdx), keyId + 1))
    {
        return CW_FALSE;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWepKeyLengthCfg(int radioIdx, int wlanIdx, int *len)
{
    int opMode = 0, index = 0, keyIdx = 0, key_len;
    char val[32+1], *c;

    *len = 64;

    if(wlanIdx != 0)
    {
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (!api_get_wifi_ifacex_wepkey_id_option((index+wlanIdx), &keyIdx))
    {
        if (!api_get_wifi_ifacex_wepkey_keyx_option((index+wlanIdx), keyIdx, val, sizeof(val)))
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

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *len);
    return CW_TRUE;

}

CWBool CWWTPBoardSetWepKeyLengthCfg(int radioIdx, int wlanIdx, int len)
{
    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, len);

    if(wlanIdx != 0)
    {
        if(len != 0)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Only WLAN0 can support WEP Key Length");
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
    int opMode = 0, index = 0;
    char *val = NULL, *c = NULL;

    if(wlanIdx != 0)
    {
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", MAX_SECRET_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    api_get_wifi_ifacex_wepkey_keyx_option((index+wlanIdx), keyIdx+1, val, MAX_SECRET_SIZE+1);

    *pstr = val;

    CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, *pstr);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWepKeyCfg(int radioIdx, int wlanIdx, int keyIdx, char *pstr)
{
    int opMode = 0, index = 0;
    char *pUciStr = NULL;
    char *c = NULL;
    char val[64];
    char wepkey[API_STRING_SIZE]={0};
    int j = 0;

    CWDebugLog("%s %d %d %d %s", __FUNCTION__, radioIdx, wlanIdx, keyIdx, pstr);

    if(wlanIdx != 0)
    {
        if(pstr[0] != 0)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Only WLAN0 can support WEP Key");
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

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        CW_FREE_OBJECT(pUciStr);
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        CW_FREE_OBJECT(pUciStr);
        return CW_FALSE;
    }

    if (api_set_wifi_ifacex_wepkey_keyx_option((index+wlanIdx), keyIdx+1, pUciStr, strlen(pUciStr)+1))
    {
        CW_FREE_OBJECT(pUciStr);
        return CW_FALSE;
    }

    CW_FREE_OBJECT(pUciStr);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWpaEncryptionCfg(int radioIdx, int wlanIdx, int *mode)
{
    int opMode = 0, index = 0, old_mode = 0, val = 3;

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        switch(radioIdx)
        {
#if SUPPORT_WLAN_5G_SETTING
            case 1:
                api_get_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_OPTION, &old_mode);
                break;
#endif
            default:
                api_get_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTION_OPTION, &old_mode);
                break;
        }
        if (old_mode >= GUEST_WPA_PSK_CCMP && old_mode <= GUEST_WPA_PSK_MIXED_TKIP_CCMP)
        {
            val = (old_mode-1)%3;
        }

    }
    else
    {
        opMode = CWWTPBoardGetRadioMode(radioIdx);

        if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
        {
            return CW_FALSE;
        }

        if (api_get_wifi_ifacex_encryption_option((index+wlanIdx), &old_mode))
        {
            return CW_FALSE;
        }

        if (old_mode >= WPA_PSK_CCMP && old_mode <= WPA_EAP_MIXED_TKIP_CCMP)
        {
            val = old_mode%3;
        }
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
    int opMode = 0, index = 0, sec_mode = 0, enc_mode = 0, encryption = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, mode);

    if(mode < CW_WPA_ENCRPTION_TKIP_AES || mode > CW_WPA_ENCRPTION_AES)
    {
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "Bad WPA Encryption mode");
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        switch(radioIdx)
        {
#if SUPPORT_WLAN_5G_SETTING
            case 1:
                api_get_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_OPTION, &encryption);
                break;
#endif
            default:
                api_get_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTION_OPTION, &encryption);
                break;
        }

        CWDebugLog("%s %d %d %d %d %d", __FUNCTION__, radioIdx, wlanIdx, mode, encryption, sec_mode);

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
            }
        }
        else
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Bad WPA Encryption mode");
        }
    }
    else
    {
        opMode = CWWTPBoardGetRadioMode(radioIdx);

        if(!sys_get_wifi_iface_index(&index, radioIdx, opMode))
        {
            return CW_FALSE;
        }

        if(api_get_wifi_ifacex_encryption_option((index+wlanIdx), &encryption))
        {
            return CW_FALSE;
        }

        CWDebugLog("%s %d %d %d %d %d", __FUNCTION__, radioIdx, wlanIdx, mode, encryption, sec_mode);

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
            }
        }
        else
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Bad WPA Encryption mode");
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

    CWDebugLog("%s %d %d %d %d %d %d", __FUNCTION__, radioIdx, wlanIdx, mode, encryption, sec_mode, enc_mode);

    //CW_WPA_ENCRPTION_TKIP_AES = 0; CW_WPA_ENCRPTION_TKIP = 1; CW_WPA_ENCRPTION_AES = 2;
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        switch(radioIdx)
        {
#if SUPPORT_WLAN_5G_SETTING
            case 1:
                if (api_set_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_OPTION, sec_mode))
                {
                    return CW_FALSE;
                }
                if (api_set_wifi_guest_wpa_enc_option(WIRELESS_WIFI_5G_GUEST_WPA_ENC_OPTION, enc_mode))
                {
                    return CW_FALSE;
                }
                break;
#endif
            default:
                if (api_set_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTION_OPTION, sec_mode))
                {
                    return CW_FALSE;
                }
                if (api_set_wifi_guest_wpa_enc_option(WIRELESS_WIFI_GUEST_WPA_ENC_OPTION, enc_mode))
                {
                    return CW_FALSE;
                }
                break;
        }
    }
    else
    {
        if (api_set_wifi_ifacex_encryption_option((index+wlanIdx), sec_mode))
        {
            return CW_FALSE;
        }
        if (api_set_wifi_ifacex_wpa_enc_option((index+wlanIdx), enc_mode))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWpaPassphraseCfg(int radioIdx, int wlanIdx, char **pstr)
{
    int opMode = 0, index = 0;
    char *val;

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", MAX_SECRET_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        switch(radioIdx)
        {
#if SUPPORT_WLAN_5G_SETTING
            case 1:
                api_get_wifi_guest_wpa_key_option(WIRELESS_WIFI_5G_GUEST_KEY_OPTION, val, MAX_SECRET_SIZE+1);
                break;
#endif
            default:
                api_get_wifi_guest_wpa_key_option(WIRELESS_WIFI_GUEST_KEY_OPTION, val, MAX_SECRET_SIZE+1);
                break;
        }
    }
    else
    {
        opMode = CWWTPBoardGetRadioMode(radioIdx);

        if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
        {
            return CW_FALSE;
        }

        api_get_wifi_ifacex_wpakey_key_option((index + wlanIdx), val, MAX_SECRET_SIZE+1);
    }

    *pstr = val;

    CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, *pstr);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWpaPassphraseCfg(int radioIdx, int wlanIdx, char *pstr)
{
    int opMode = 0, index = 0;
    char *pUciStr = NULL;

    CWDebugLog("%s %d %d %s", __FUNCTION__, radioIdx, wlanIdx, pstr);

    CW_CREATE_STRING_FROM_STRING_ERR(pUciStr, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        switch(radioIdx)
        {
#if SUPPORT_WLAN_5G_SETTING
            case 1:
                api_set_wifi_guest_wpa_key_option(WIRELESS_WIFI_5G_GUEST_KEY_OPTION, pUciStr, strlen(pUciStr));
                break;
#endif
            default:
                api_set_wifi_guest_wpa_key_option(WIRELESS_WIFI_GUEST_KEY_OPTION, pUciStr, strlen(pUciStr));
                break;
        }
    }
    else
    {
        opMode = CWWTPBoardGetRadioMode(radioIdx);

        if (opMode != 0 && opMode != 2)
        {
            CW_FREE_OBJECT(pUciStr);
            return CW_FALSE;
        }

        if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
        {
            CW_FREE_OBJECT(pUciStr);
            return CW_FALSE;
        }

        if (api_set_wifi_ifacex_wpakey_key_option((index+wlanIdx), pUciStr, strlen(pUciStr)))
        {
            CW_FREE_OBJECT(pUciStr);
            return CW_FALSE;
        }
    }

    CW_FREE_OBJECT(pUciStr);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWpaGroupKeyUpdateIntervalCfg(int radioIdx, int wlanIdx, int *interval)
{
    int opMode = 0, index = 0, val = 3600;

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        switch(radioIdx)
        {
#if SUPPORT_WLAN_5G_SETTING
            case 1:
                if (api_get_wifi_guest_wpa_group_rekey_option(WIRELESS_WIFI_5G_GUEST_WPA_GROUP_REKEY_OPTION, &val))
                {
                    if (api_set_wifi_guest_wpa_group_rekey_option(WIRELESS_WIFI_5G_GUEST_WPA_GROUP_REKEY_OPTION, val))
                    {
                        return CW_FALSE;
                    }
                }
                break;
#endif
            default:
                if (api_get_wifi_guest_wpa_group_rekey_option(WIRELESS_WIFI_GUEST_WPA_GROUP_REKEY_OPTION, &val))
                {
                    if (api_set_wifi_guest_wpa_group_rekey_option(WIRELESS_WIFI_GUEST_WPA_GROUP_REKEY_OPTION, val))
                    {
                        return CW_FALSE;
                    }
                }
                break;
        }
    }
    else
    {
        opMode = CWWTPBoardGetRadioMode(radioIdx);

        if (opMode == -1)
        {
            return CW_FALSE;
        }

        if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
        {
            return CW_FALSE;
        }

        if (api_get_wifi_ifacex_wpa_group_rekey_option((index+wlanIdx), &val))
        {
            if (api_set_wifi_ifacex_wpa_group_rekey_option((index+wlanIdx), val))
            {
                return CW_FALSE;
            }
        }
    }

    *interval = val;

    CWDebugLog("%s %d interval:[%d]", __FUNCTION__, __LINE__, *interval);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWpaGroupKeyUpdateIntervalCfg(int radioIdx, int wlanIdx, int interval)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, interval);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        switch(radioIdx)
        {
#if SUPPORT_WLAN_5G_SETTING
            case 1:
                if (api_set_wifi_guest_wpa_group_rekey_option(WIRELESS_WIFI_5G_GUEST_WPA_GROUP_REKEY_OPTION, interval))
                {
                    return CW_FALSE;
                }
                break;
#endif
            default:
                if (api_set_wifi_guest_wpa_group_rekey_option(WIRELESS_WIFI_GUEST_WPA_GROUP_REKEY_OPTION, interval))
                {
                    return CW_FALSE;
                }
                break;
        }
    }
    else
    {
        opMode = CWWTPBoardGetRadioMode(radioIdx);

        if (opMode != 0 && opMode != 2)
        {
            return CW_FALSE;
        }

        if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
        {
            return CW_FALSE;
        }

        if (api_set_wifi_ifacex_wpa_group_rekey_option((index+wlanIdx), interval))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWpaRadiusAddressCfg(int radioIdx, int wlanIdx, unsigned int *addr)
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
        return CW_FALSE;
    }

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (api_get_wifi_ifacex_auth_server_option((index+wlanIdx), val, sizeof(val)))
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

CWBool CWWTPBoardSetWpaRadiusAddressCfg(int radioIdx, int wlanIdx, unsigned int addr)
{
    int opMode = 0, index = 0;
    char val[15+1];

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(addr)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Radius Address");
        }
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(addr)
    {
        snprintf(val, sizeof(val), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
        if (api_set_wifi_ifacex_auth_server_option((index+wlanIdx), val, sizeof(val)))
        {
            return CW_FALSE;
        }
    }
    else
    {
        snprintf(val, sizeof(val), " ");
        if (api_set_wifi_ifacex_auth_server_option((index+wlanIdx), val, sizeof(val)))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWpaRadiusPortCfg(int radioIdx, int wlanIdx, unsigned short *port)
{
    int opMode = 0, index = 0, val = 1812;

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        *port = 1812;
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (api_get_wifi_ifacex_auth_port_option((index+wlanIdx), &val))
    {
        if (api_set_wifi_ifacex_auth_port_option((index+wlanIdx), val))
        {
            return CW_FALSE;
        }
    }

    *port = val;

    CWDebugLog("%s %d %u", __FUNCTION__, __LINE__, *port);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWpaRadiusPortCfg(int radioIdx, int wlanIdx, unsigned short port)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %u", __FUNCTION__, radioIdx, wlanIdx, port);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(port)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Radius Port");
        }
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (api_set_wifi_ifacex_auth_port_option((index+wlanIdx), port))
    {
        return CW_FALSE;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWpaRadiusSecretCfg(int radioIdx, int wlanIdx, char **pstr)
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
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", MAX_SECRET_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    api_get_wifi_ifacex_auth_secret_option((index + wlanIdx), val, MAX_SECRET_SIZE+1);

    *pstr = val;

    CWDebugLog("%s %d radio:[%d] wlanIdx:[%d] %s", __FUNCTION__, __LINE__, radioIdx, wlanIdx, *pstr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWpaRadiusSecretCfg(int radioIdx, int wlanIdx, char *pstr)
{
    int opMode = 0, index = 0;
    char *pUciStr = NULL;

    CWDebugLog("%s %d %d %s", __FUNCTION__, radioIdx, wlanIdx, pstr);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(pstr[0] != 0)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Radius Secret");
        }
        return CW_TRUE;
    }

    CW_CREATE_STRING_FROM_STRING_ERR(pUciStr, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        CW_FREE_OBJECT(pUciStr);
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        CW_FREE_OBJECT(pUciStr);
        return CW_FALSE;
    }

    if (api_set_wifi_ifacex_auth_secret_option((index+wlanIdx), pUciStr, strlen(pUciStr)))
    {
        CW_FREE_OBJECT(pUciStr);
        return CW_FALSE;
    }

    CW_FREE_OBJECT(pUciStr);

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadiusAccountingEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int opMode = 0, index = 0, val = 0;

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        *enable = CW_FALSE;
        return CW_TRUE;
    }
#endif

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if (api_get_wifix_guest_acct_enabled_option(radioIdx, &val))
        {
            if (api_set_wifix_guest_acct_enabled_option(radioIdx, val))
            {
                return CW_FALSE;
            }
        }
    }
    else
#endif
    {
        if (api_get_wifi_ifacex_acct_enabled_option((index + wlanIdx), &val))
        {
            if (api_set_wifi_ifacex_acct_enabled_option((index+wlanIdx), val))
            {
                return CW_FALSE;
            }
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

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadiusAccountingEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, enable);

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(enable)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Radius Accounting Enable");
        }
        return CW_TRUE;
    }
#endif
    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if (api_set_wifix_guest_acct_enabled_option(radioIdx, enable ? 1 : 0))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_wifi_ifacex_acct_enabled_option((index+wlanIdx), enable ? 1 : 0))
        {
            return CW_FALSE;
        }
    }
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadiusAccountingAddressCfg(int radioIdx, int wlanIdx, unsigned int *addr)
{
    char val[API_IPADDR_SIZE] = {0};
    int opMode = 0, index = 0;

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        *addr = 0;
        return CW_TRUE;
    }
#endif

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if (api_get_wifix_guest_acct_server_option(radioIdx, val, sizeof(val)))
        {
            *addr = 0;
        }
    }
    else
#endif
    {
        if (api_get_wifi_ifacex_acct_server_option((index+wlanIdx), val, sizeof(val)))
        {
            *addr = 0;
        }
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
    int opMode = 0, index = 0;

    char val[API_IPADDR_SIZE] = {0};

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(addr)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Radius Accounting Address");
        }
        return CW_TRUE;
    }
#endif

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    snprintf(val, API_IPADDR_SIZE, "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if (api_set_wifix_guest_acct_server_option(radioIdx, val, API_IPADDR_SIZE))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_wifi_ifacex_acct_server_option((index+wlanIdx), val, API_IPADDR_SIZE))
        {
            return CW_FALSE;
        }
    }
    return CW_TRUE;

}

CWBool CWWTPBoardGetRadiusAccountingPortCfg(int radioIdx, int wlanIdx, unsigned short *port)
{
    int opMode = 0, index = 0, val = 1813;

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        *port = 1813;
        return CW_TRUE;
    }
#endif

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if (api_get_wifix_guest_acct_port_option(radioIdx, &val))
        {
            if (api_set_wifix_guest_acct_port_option(radioIdx, val))
            {
                return CW_FALSE;
            }
        }
    }
    else
#endif
    {
        if (api_get_wifi_ifacex_acct_port_option((index+wlanIdx), &val))
        {
            if (api_set_wifi_ifacex_acct_port_option((index+wlanIdx), val))
            {
                return CW_FALSE;
            }
        }
    }

    *port = val;

    CWDebugLog("%s %d %u", __FUNCTION__, __LINE__, *port);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadiusAccountingPortCfg(int radioIdx, int wlanIdx, unsigned short port)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %u", __FUNCTION__, radioIdx, wlanIdx, port);

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(port)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Radius Accounting Port");
        }
        return CW_TRUE;
    }
#endif

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if (api_set_wifix_guest_acct_port_option(radioIdx, port))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_wifi_ifacex_acct_port_option((index+wlanIdx), port))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadiusAccountingSecretCfg(int radioIdx, int wlanIdx, char **pstr)
{
    int opMode = 0, index = 0;
    char *val = NULL;

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        CW_CREATE_STRING_FROM_STRING_ERR(*pstr, "", return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););
        return CW_TRUE;
    }
#endif
    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    CW_CREATE_STRING_FROM_STRING_LEN_ERR(val, "", MAX_SECRET_SIZE, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        api_get_wifix_guest_acct_secret_option(radioIdx, val, MAX_SECRET_SIZE+1);
    }
    else
#endif
    {
        api_get_wifi_ifacex_acct_secret_option((index + wlanIdx), val, MAX_SECRET_SIZE+1);
    }

    *pstr = val;

    CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, *pstr);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadiusAccountingSecretCfg(int radioIdx, int wlanIdx, char *pstr)
{
    int opMode = 0, index = 0;
    char *pUciStr = NULL;

    CWDebugLog("%s %d %d %s", __FUNCTION__, radioIdx, wlanIdx, pstr);

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(pstr[0] != 0)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Radius Accounting Secret");
        }
        return CW_TRUE;
    }
#endif

    CW_CREATE_STRING_FROM_STRING_ERR(pUciStr, pstr, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL););

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode != 0 && opMode != 2)
    {
        CW_FREE_OBJECT(pUciStr);
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        CW_FREE_OBJECT(pUciStr);
        return CW_FALSE;
    }

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if (api_set_wifix_guest_acct_secret_option(radioIdx, pUciStr, strlen(pUciStr)))
        {
            CW_FREE_OBJECT(pUciStr);
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_set_wifi_ifacex_acct_secret_option((index+wlanIdx), pUciStr, strlen(pUciStr)))
        {
            CW_FREE_OBJECT(pUciStr);
            return CW_FALSE;
        }
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

CWBool CWWTPBoardGetSitesurvey(CWRadioFreqType radioType, unsigned char *version, int *count, CWWTPSitesurvey **sitesurvey)
{
    int i;
    char tmpFile[64];
    FILE *fp;
    long fsize;
    char *buffer, *c;
    unsigned int bssid[6];

    /////////////////////////////////////////////////////////////
    unsigned int uint32CapCode;
    int int32RadioIdx, int32WlanIdx, int32SsidEnable = 0;
    CWBool bFound = CW_FALSE;

    if(!CWWTPBoardGetCapCode(&uint32CapCode))
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    if(!CWGetRadioIndex(radioType, uint32CapCode, &int32RadioIdx))
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    for(int32WlanIdx = 0; int32WlanIdx < CWWTPBoardGetMaxRadioWlans(int32RadioIdx); int32WlanIdx++)
    {
        if(CWWTPBoardGetWlanEnableCfg(int32RadioIdx, int32WlanIdx, &int32SsidEnable))
        {
            if(int32SsidEnable)
            {
                bFound = CW_TRUE;
                break;
            }
        }
    }
    if(bFound == CW_FALSE)
    {
        return CW_FALSE;
    }
    /////////////////////////////////////////////////////////////

    sprintf(tmpFile, "/tmp/WLAN_SITE_SURVEY%sG.%x.tmp", ((radioType == CW_RADIOFREQTYPE_2G) ? "2" : "5"), (unsigned int) CWThreadSelf());

#if defined(CW_BOARD_EWS310AP) || defined(ATMWS_SERIES)
    if(radioType == CW_RADIOFREQTYPE_5G)
    {
        CWSystem("echo 1 >  /proc/sys/dev/%s/isscanning", WIFI_IF_NAME(int32RadioIdx));
    }
#endif

#if defined(CW_BOARD_EWS310AP) || defined(ATMWS_SERIES)
    CWSystem("iwlist %s scanning fast | tail -n +3 > %s",
             WLAN_IF_NAME(int32RadioIdx, int32WlanIdx), tmpFile);
#else
    CWSystem("iwlist %s scanning | tail -n +3 > %s",
             WLAN_IF_NAME(int32RadioIdx, int32WlanIdx), tmpFile);
#endif /* (CW_BOARD_EWS310AP) || (ATMWS_SERIES) */

#if defined(CW_BOARD_EWS310AP) || defined(ATMWS_SERIES)
    if(radioType == CW_RADIOFREQTYPE_5G)
    {
        CWSystem("echo 0 >  /proc/sys/dev/%s/isscanning", WIFI_IF_NAME(int32RadioIdx));
    }
#endif

    *version = 0;

    fp = fopen(tmpFile, "r");
    if(!fp)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(fsize < 0)
    {
        fclose(fp);
        unlink(tmpFile);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    if(fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
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
            return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
//NOT_FINISH (NOT SUPPORT)
CWBool CWWTPBoardSetRadioAutoTxPower(CWRadioFreqType radioType, int int32HealingTxpower)
{
    unsigned int uint32CapCode;
    int int32RadioIdx, int32WlanIdx, int32SsidEnable = 0, int32MaxTxpower, int32Txpower = 0;

    CWDebugLog("WTP auto txpower request radiotype: %s healing txpower: %d%%", ((radioType == CW_RADIOFREQTYPE_2G) ? "2G" : "5G"), int32HealingTxpower);

    if(!CWWTPBoardGetCapCode(&uint32CapCode))
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(!CWGetRadioIndex(radioType, uint32CapCode, &int32RadioIdx))
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(!CWWTPBoardGetRadioMaxTxPower(int32RadioIdx, &int32MaxTxpower))
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(!CWGetHealthyTxPowerValue(int32MaxTxpower, int32HealingTxpower, &int32Txpower))
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    for(int32WlanIdx = 0; int32WlanIdx < CWWTPBoardGetMaxRadioWlans(int32RadioIdx); int32WlanIdx++)
    {
        if(CWWTPBoardGetWlanEnableCfg(int32RadioIdx, int32WlanIdx, &int32SsidEnable))
        {
            if(int32SsidEnable)
            {
                CWSystem("iwconfig %s txpower %d", WLAN_IF_NAME(int32RadioIdx, int32WlanIdx), int32Txpower);
            }
        }
    }
    return CW_TRUE;
}

CWBool CWWTPBoardKickmac(CWWTPKickmacInfo *kicks)
{
    int i = 0;
    int wifix;
    char option[API_OPTION_SIZE];
    char str[API_MAC_SIZE];
    char ifname[8];

    wifix = (((CWRadioFreqType) kicks->radio == CW_RADIOFREQTYPE_2G) ? AP_WIFI_24G_IFACE_NO : AP_WIFI_5G_IFACE_NO);
    
    sprintf(option, "wireless.@wifi-iface[%d].ifname", wifix);

    api_get_string_option(option, ifname, sizeof(ifname));

    while(i < kicks->clientNum)
    {
        sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", kicks->client[i].mac[0], kicks->client[i].mac[1],
                kicks->client[i].mac[2], kicks->client[i].mac[3], kicks->client[i].mac[4], kicks->client[i].mac[5]);
            
        CWSystem("iwpriv %s kickmac %s", ifname, str);

        CWDebugLog("iwpriv %s kickmac %s", ifname, str);

        i++;
    }

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

    sprintf(tmpFile, "/tmp/wtpsta.%x.tmp", (unsigned int) CWThreadSelf());

    CWSystem("wlanconfig %s list sta 2> /dev/null | grep : | awk '{print $1%s$7%s$8%s$6}' > %s",
             WLAN_IF_NAME(radioIdx, wlanIdx), "\"\\t\"", "\"\\t\"", "\"\\t\"", tmpFile);

    fp = fopen(tmpFile, "r");
    if(!fp)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        fclose(fp);
        unlink(tmpFile);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(fsize < 0)
    {
        fclose(fp);
        unlink(tmpFile);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    if(fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    fclose(fp);
    unlink(tmpFile);

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

    CW_CREATE_ARRAY_ERR(*station, *count, CWStation,
    {
        CW_FREE_OBJECT(buffer);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    i = 0;
    c = buffer;
    while(*c != '\0' && i < *count)
    {
        if(sscanf(c, "%x:%x:%x:%x:%x:%x\t%uKb\t%uKb\t%d",
                  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5],
                  &((*station)[i].txKB),
                  &((*station)[i].rxKB),
                  &((*station)[i].rssi)) != 9)
        {
            CW_FREE_OBJECT(*station);
            CW_FREE_OBJECT(buffer);
            return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        });
        (*station)[i].hostNameLen = 0;
        CW_CREATE_STRING_FROM_STRING_ERR((*station)[i].hostName, "",
        {
            CW_FREE_OBJECT(buffer);
            return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
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
            WLAN_IF_NAME(radioIdx, wlanIdx), (unsigned int) CWThreadSelf());

    CWSystem("cp -a /tmp/fingerprint_status_list_%s %s", WLAN_IF_NAME(radioIdx, wlanIdx), tmpFile);

    fp = fopen(tmpFile, "r");
    if(!fp)
    {
        return CW_TRUE;
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    if(fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    fclose(fp);
    unlink(tmpFile);

    buffer[fsize] = '\0';

    // parse
    char osType[128] = {0};
    char hostName[128] = {0};
    char ip[15] = {0};

    c = buffer;
    while(*c != '\0')
    {
        memset(osType, 0, sizeof(osType));
        memset(hostName, 0, sizeof(hostName));
        memset(ip, 0, sizeof(ip));

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
                    return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                });

                CW_FREE_OBJECT((*station)[i].hostName);
                (*station)[i].hostNameLen = strlen(hostName);
                CW_CREATE_STRING_FROM_STRING_ERR((*station)[i].hostName, hostName,
                {
                    CW_FREE_OBJECT(buffer);
                    return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                });

                if(ip[0] == '\0')
                {
                    (*station)[i].address = 0;
                }
                else
                {
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
    int opMode = 0, index = 0;

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if (api_get_wifi_guest_ifname_option(radioIdx, ifname, sizeof(ifname)))
        {
            return CW_FALSE;
        }
    }
    else
    {
        if (api_get_wifi_ifacex_ifname_option((index+wlanIdx), ifname, sizeof(ifname)))
        {
            return CW_FALSE;
        }
    }

    sprintf(tmpFile, "/tmp/wtpstat.%x.tmp", (unsigned int) CWThreadSelf());

    CWSystem("ifconfig %s 2> /dev/null > %s", ifname, tmpFile);

    fp = fopen(tmpFile, "r");
    if(!fp)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        fclose(fp);
        unlink(tmpFile);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(fsize < 0)
    {
        fclose(fp);
        unlink(tmpFile);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    if(fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    fclose(fp);
    unlink(tmpFile);

    buffer[fsize] = '\0';

    c = strstr(buffer, "RX packets:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    c += strlen("RX packets:");
    statistics->rxPkts = strtoul(c, NULL, 10);

    c = strstr(c, "errors:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    c += strlen("errors:");
    statistics->rxErrPkts = strtoul(c, NULL, 10);

    c = strstr(c, "dropped:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    c += strlen("dropped:");
    statistics->rxDrpPkts = strtoul(c, NULL, 10);

    c = strstr(c, "TX packets:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    c += strlen("TX packets:");
    statistics->txPkts = strtoul(c, NULL, 10);

    c = strstr(c, "errors:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    c += strlen("errors:");
    statistics->txErrPkts = strtoul(c, NULL, 10);

    c = strstr(c, "dropped:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    c += strlen("dropped:");
    statistics->txDrpPkts = strtoul(c, NULL, 10);

    c = strstr(c, "RX bytes:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    c += strlen("RX bytes:");
    statistics->rxBytes = strtoul(c, NULL, 10);

    c = strstr(c, "TX bytes:");
    if(!c)
    {
        CW_FREE_OBJECT(buffer);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    c += strlen("TX bytes:");
    statistics->txBytes = strtoul(c, NULL, 10);

    CW_FREE_OBJECT(buffer);

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanAclModeCfg(int radioIdx, int wlanIdx, int *mode)
{
    int opMode = 0, index = 0;

    *mode = 0 ;

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        api_get_wifi_guest_macfilter_option(radioIdx, mode);
    }
    else
    {
        api_get_wifi_ifacex_macfilter_option((index+wlanIdx), mode);
    }

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *mode);

    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanAclModeCfg(int radioIdx, int wlanIdx, int mode)
{
    int opMode = 0, index = 0, aclmode = 0;
    char val[18*MAC_FILTER_NUM+1] = { 0 };

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    CWDebugLog("%s %d radioIdx:%d wlanIdx:%d mode:%d", __FUNCTION__, __LINE__, radioIdx, wlanIdx, mode);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        api_get_wifi_guest_macfilter_option(radioIdx, &aclmode);
        if ( aclmode != mode )
	{
            switch (mode)
            {
                case 1:
                    api_get_wifi_guest_denymaclist_option(radioIdx, val, sizeof(val));
                    if(val[0] != '\0')
                    {
                        api_add_wifi_ifacex_allowmaclist_option((100+radioIdx), val, strlen(val));
                    }
                    api_delete_wifi_ifacex_denymaclist_option((100+radioIdx), 0);
                    break;
                case 2:
                    api_get_wifi_guest_allowmaclist_option(radioIdx, val, sizeof(val));
                    if(val[0] != '\0')
                    {
                        api_add_wifi_ifacex_denymaclist_option((100+radioIdx), val, strlen(val));
                    }
                    api_delete_wifi_ifacex_allowmaclist_option((100+radioIdx), 0);
                    break;
            }
        }
        if (api_set_wifi_guest_macfilter_option(radioIdx, mode))
        {
            return CW_FALSE;
        }
    }
    else
    {
        api_get_wifi_ifacex_macfilter_option((index+wlanIdx), &aclmode);
        if (aclmode != mode)
        {
            switch (mode)
            {
                case 1:
                    api_get_wifi_ifacex_denymaclist_option((index+wlanIdx), val, sizeof(val));
                    if(val[0] != '\0')
                    {
                        api_add_wifi_ifacex_allowmaclist_option((index+wlanIdx), val, strlen(val));
                    }
                    api_delete_wifi_ifacex_denymaclist_option((index+wlanIdx), 0);
                    break;
                case 2:
                    api_get_wifi_ifacex_allowmaclist_option((index+wlanIdx), val, sizeof(val));
                    if(val[0] != '\0')
                    {
                        api_add_wifi_ifacex_denymaclist_option((index+wlanIdx), val, strlen(val));
                    }
                    api_delete_wifi_ifacex_allowmaclist_option((index+wlanIdx), 0);
                    break;
            }
        }
        if (api_set_wifi_ifacex_macfilter_option((index+wlanIdx), mode))
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
    int opMode = 0, index = 0;
    char val[18*MAC_FILTER_NUM+1];

    CWDebugLog("%s %d radioIdx:%d wlanIdx:%d", __FUNCTION__, __LINE__, radioIdx, wlanIdx);

    memset(&val, 0, sizeof(val));

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        api_get_wifi_guest_macfilter_option(radioIdx, &aclmode);

        switch (aclmode)
        {
            case 1:
                api_get_wifi_guest_allowmaclist_option(radioIdx, val, sizeof(val));
                break;
            case 2:
                api_get_wifi_guest_denymaclist_option(radioIdx, val, sizeof(val));
                break;
        }
    }
    else
    {
        api_get_wifi_ifacex_macfilter_option((index+wlanIdx), &aclmode);

        switch (aclmode)
        {
            case 1:
                api_get_wifi_ifacex_allowmaclist_option((index+wlanIdx), val, sizeof(val));
                break;
            case 2:
                api_get_wifi_ifacex_denymaclist_option((index+wlanIdx), val, sizeof(val));
                break;
        }
    }

    if(val[0] == '\0')
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
        if(*c == ' ')
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
        if(sscanf(c, "%x:%x:%x:%x:%x:%x",
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

        if(*c == '\0')
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
    char *cmd;
    int i, len, aclmode;
    int opMode = 0, index = 0;

    CWDebugLog("%s %d radioIdx:%d wlanIdx:%d", __FUNCTION__, __LINE__, radioIdx, wlanIdx);

    CW_CREATE_OBJECT_SIZE_ERR(cmd, 64 + (count * 18),
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);;
    });

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if (api_get_wifi_guest_macfilter_option(radioIdx, &aclmode))
        {
            return CW_FALSE;
        }

        sprintf(cmd, "uci set wireless.wifi%d_guest.%s=\"", radioIdx, (aclmode == 1) ? "allowmaclist" : "denymaclist");
    }
    else
    {
        if (api_get_wifi_ifacex_macfilter_option((index+wlanIdx), &aclmode))
        {
            return CW_FALSE;
        }

        sprintf(cmd, "uci set wireless.@wifi-iface[%d].%s=\"", (index+wlanIdx), (aclmode == 1) ? "allowmaclist" : "denymaclist");     
    }

    len = strlen(cmd);
    i = 0;
    while(i < count)
    {
        sprintf(&cmd[len], "%s%02x:%02x:%02x:%02x:%02x:%02x",
                i == 0 ? "" : " ", CW_MAC_PRINT_LIST(macs[i]));
        i++;
        len = strlen(cmd);
    }

    strcpy(&cmd[len], "\"");

    CWSystem(cmd);

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioDataRateCfg(int radioIdx, int *rate)
{
    char val[8], *c;
    int rateIdx = 0;

    memset(val, 0, sizeof(val));
#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx == 1) {
        if (api_get_wifi_5g_rate_option(WIRELESS_WIFI_5G_RATE_OPTION, &rateIdx))
        {
            rateIdx = 0;
        }
        if (api_fetch_wifi_5g_datarate_option(rateIdx, val, sizeof(val)))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_get_wifi_rate_option(WIRELESS_WIFI_RATE_OPTION, &rateIdx))
        {
            rateIdx = 0;
        }
        if (api_fetch_wifi_datarate_option(rateIdx, val, sizeof(val)))
        {
            return CW_FALSE;
        }
    }

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
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    CWDebugLog("%s %d %d %d", __FUNCTION__, __LINE__, radioIdx, *rate);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioDataRateCfg(int radioIdx, int rate)
{
    int rateIdx;
    char rateStr[8];

    CWDebugLog("%s %d %d %d", __FUNCTION__, __LINE__, radioIdx, rate);

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

    CWDebugLog("%s %d %d %d %s", __FUNCTION__, __LINE__, radioIdx, rate, rateStr);

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx == 1)
    {
        if (api_match_wifi_5g_datarate_option(rateStr, &rateIdx))
        {
            return CW_FALSE;
        }
        if (api_set_wifi_5g_rate_option(WIRELESS_WIFI_5G_RATE_OPTION, rateIdx))
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
        if (api_match_wifi_datarate_option(rateStr, &rateIdx))
        {
            return CW_FALSE;
        }
        if (api_set_wifi_rate_option(WIRELESS_WIFI_RATE_OPTION, rateIdx))
        {
            return CW_FALSE;
        }
    }

    CWDebugLog("%s %d %d %d %s %d", __FUNCTION__, __LINE__, radioIdx, rate, rateStr, rateIdx);

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioRtsCtsThresholdCfg(int radioIdx, int *threshold)
{
#if SUPPORT_WLAN_5G_SETTING
    if (api_get_integer_option((radioIdx == 0) ? WIRELESS_WIFI_RTS_OPTION : WIRELESS_WIFI_5G_RTS_OPTION , threshold))
    {
        *threshold = 0;
    }
#else
    if (api_get_integer_option(WIRELESS_WIFI_RTS_OPTION, threshold))
    {
        *threshold = 0;
    }
#endif

    CWDebugLog("%s %d threshold:[%d]", __FUNCTION__, __LINE__, *threshold);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioRtsCtsThresholdCfg(int radioIdx, int threshold)
{
    CWDebugLog("%s %d %d", __FUNCTION__, radioIdx, threshold);

#if SUPPORT_WLAN_5G_SETTING
    if (api_set_integer_option((radioIdx == 0) ? WIRELESS_WIFI_RTS_OPTION : WIRELESS_WIFI_5G_RTS_OPTION, threshold))
    {
        return CW_FALSE;
    }
#else
    if (api_set_integer_option(WIRELESS_WIFI_RTS_OPTION, threshold))
    {
        return CW_FALSE;
    }
#endif

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadiusAccountingIntermiIntervalCfg(int radioIdx, int wlanIdx, int *interval)
{
    int opMode = 0, index = 0, val = 600;

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        *interval = 600;
        return CW_TRUE;
    }
#endif

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

#if SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if (api_get_wifix_guest_acct_interval_option(radioIdx, &val))
        {
            if (api_set_wifix_guest_acct_interval_option(radioIdx, val))
            {
                CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, val);
                return CW_FALSE;
            }
        }
    }
    else
#endif
    {
        if (api_get_wifi_ifacex_acct_interval_option((index+wlanIdx), &val))
        {
            if (api_set_wifi_ifacex_acct_interval_option((index+wlanIdx), val))
            {
                CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, val);
                return CW_FALSE;
            }
        }
    }

    *interval = val;

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *interval);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadiusAccountingIntermiIntervalCfg(int radioIdx, int wlanIdx, int interval)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, interval);

#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(interval != 0)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Radius Accounting Intermi Interval");
        }
        return CW_TRUE;
    }
#endif

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (opMode != 0 && opMode != 2)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if (api_set_wifix_guest_acct_interval_option(radioIdx, interval))
        {
            return CW_FALSE;
        }
    }
    else
    {
        if (api_set_wifi_ifacex_acct_interval_option((index+wlanIdx), interval))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioObeyRegulatoryPowerCfg(int radioIdx, int *enable)
{
    if (api_get_integer_option(WIRELESS_WIFI_OBEYREGPOWER_OPTION, enable))
    {
        *enable = 0;
    }

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioObeyRegulatoryPowerCfg(int radioIdx, int enable)
{
    CWDebugLog("%s %d %d", __FUNCTION__, radioIdx, enable);

    if (api_set_integer_option(WIRELESS_WIFI_OBEYREGPOWER_OPTION, enable ? 1 : 0))
    {
        return CW_FALSE;
    }
#if SUPPORT_WLAN_5G_SETTING
    if (api_set_integer_option(WIRELESS_WIFI_5G_OBEYREGPOWER_OPTION, enable ? 1 : 0))
    {
        return CW_FALSE;
    }
#endif

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

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetBandSteeringCfg(int enable)
{
    int val = 0;

    CWDebugLog("%s %d", __FUNCTION__, enable);

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
#endif
    return CW_TRUE;
}

CWBool CWWTPBoardGetBandSteeringMode(int *mode)
{
    *mode = 0;

    api_get_wifi_band_steering_mode_option(WIRELESS_WIFI_BANDSTEER_OPTION, mode);

    CWDebugLog("%s %d band steering mode:[%d]", __FUNCTION__, __LINE__, *mode);

    return CW_TRUE;
}

CWBool CWWTPBoardSetBandSteeringMode(int mode)
{
    CWDebugLog("%s %d", __FUNCTION__, mode);

    if (api_set_wifi_band_steering_mode_option(WIRELESS_WIFI_BANDSTEER_OPTION, mode))
    {
        return CW_FALSE;
    }
#if SUPPORT_WLAN_5G_SETTING
    if (api_set_wifi_band_steering_mode_option(WIRELESS_WIFI_5G_BANDSTEER_OPTION, mode))
    {
        return CW_FALSE;
    }
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
#endif
#endif
    return CW_TRUE;
}

CWBool CWWTPBoardGetFastHandoverStatusCfg(int *enable)
{
    if (api_get_integer_option(WIRELESS_WIFI_FASTHANDOVER_ENABLE_OPTION, enable))
    {
        *enable = 0;
    }

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetFastHandoverStatusCfg(int enable)
{
    CWDebugLog("%s %u", __FUNCTION__, enable);

    if (api_set_integer_option(WIRELESS_WIFI_FASTHANDOVER_ENABLE_OPTION, enable))
    {
        return CW_FALSE;
    }
#if SUPPORT_WLAN_5G_SETTING
    if (api_set_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_ENABLE_OPTION, enable))
    {
        return CW_FALSE;
    }
#endif
    return CW_TRUE;
}

CWBool CWWTPBoardGetFastHandoverRssiCfg(int *rssi)
{
    if (api_get_integer_option(WIRELESS_WIFI_FASTHANDOVER_RSSI_OPTION, rssi))
    {
        *rssi = 0 ;
    }

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *rssi);
    return CW_TRUE;
}

CWBool CWWTPBoardSetFastHandoverRssiCfg(int rssi)
{
    CWDebugLog("%s %d", __FUNCTION__, rssi);

    if (api_set_integer_option(WIRELESS_WIFI_FASTHANDOVER_RSSI_OPTION, rssi))
    {
        return CW_FALSE;
    }
#if SUPPORT_WLAN_5G_SETTING
    if (api_set_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_RSSI_OPTION, rssi))
    {
        return CW_FALSE;
    }
#endif

    return CW_TRUE;
}

// V1 Not Support
CWBool CWWTPBoardGetRadioFastHandoverStatusCfg(int radioIdx, int *enable)
{
    int ret = API_RC_INTERNAL_ERROR;

    switch (radioIdx)
    {
    case 0:
        ret = api_get_integer_option(WIRELESS_WIFI_FASTHANDOVER_ENABLE_OPTION, enable);
        break;
#if SUPPORT_WLAN_5G_SETTING
    case 1:
        ret = api_get_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_ENABLE_OPTION, enable);
        break;
#endif
    default:
        break;
    }

    if (ret != API_RC_SUCCESS)
    {
        *enable = CW_FALSE;
    }

    CWLog("%s %u %u", __FUNCTION__,radioIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioFastHandoverStatusCfg(int radioIdx, int enable)
{
    CWLog("%s %u %u", __FUNCTION__,radioIdx, enable);

    switch (radioIdx)
    {
    case 0:
        api_set_integer_option(WIRELESS_WIFI_FASTHANDOVER_ENABLE_OPTION, enable);
        break;
#if SUPPORT_WLAN_5G_SETTING
    case 1:
        api_set_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_ENABLE_OPTION, enable);
        break;
#endif
    default:
        break;
    }

    return CW_TRUE;
}

// V1 Not Support
CWBool CWWTPBoardGetRadioFastHandoverRssiCfg(int radioIdx, int *rssi)
{
    int ret = API_RC_INTERNAL_ERROR;

    switch (radioIdx)
    {
    case 0:
        ret = api_get_integer_option(WIRELESS_WIFI_FASTHANDOVER_RSSI_OPTION, rssi);
        break;
#if SUPPORT_WLAN_5G_SETTING
    case 1:
        ret = api_get_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_RSSI_OPTION, rssi);
        break;
#endif
    default:
        break;
    }

    if (ret != API_RC_SUCCESS)
    {
        *rssi = -90;
    }

    CWLog("%s %u %d", __FUNCTION__,radioIdx, *rssi);

    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioFastHandoverRssiCfg(int radioIdx, int rssi)
{
    CWLog("%s %u %d ", __FUNCTION__,radioIdx, rssi);

    switch (radioIdx)
    {
    case 0:
        api_set_integer_option(WIRELESS_WIFI_FASTHANDOVER_RSSI_OPTION, rssi);
        break;
#if SUPPORT_WLAN_5G_SETTING
    case 1:
        api_set_integer_option(WIRELESS_WIFI_5G_FASTHANDOVER_RSSI_OPTION, rssi);
        break;
#endif
    default:
        break;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetDownloadLimitCfg(int radioIdx, int wlanIdx, int *limit)
{
    int opMode=0, index=0;
    int tc_enable;

    *limit = -1;

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    api_get_wifi_ifacex_tc_enabled_option((index+wlanIdx), &tc_enable);

    if(tc_enable)
    {
        api_get_wifi_ifacex_tc_downlimit_option((index+wlanIdx), limit);
    }
    else
    {
        *limit = -1;
    }

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *limit);
    return CW_TRUE;
}

CWBool CWWTPBoardSetDownloadLimitCfg(int radioIdx, int wlanIdx, int limit)
{
    int opMode=0, index=0;

    CWDebugLog("%s %u %u limit:[%d]", __FUNCTION__, radioIdx, wlanIdx, limit);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(limit != -1)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Download Limit");
        }
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(limit == -1)
    {
        if (api_set_wifi_ifacex_tc_enabled_option((index+wlanIdx), 0))
        {
            return CW_FALSE;                
        }
    }
    else
    {
        if (api_set_wifi_ifacex_tc_enabled_option((index+wlanIdx), 1))
        {
            return CW_FALSE;                
        }

        if (api_set_wifi_ifacex_tc_downlimit_option((index+wlanIdx), limit))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetUploadLimitCfg(int radioIdx, int wlanIdx, int *limit)
{
    int opMode=0, index=0;
    int tc_enable;

    *limit = -1;

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    api_get_wifi_ifacex_tc_enabled_option((index+wlanIdx), &tc_enable);

    if(tc_enable)
    {
        api_get_wifi_ifacex_tc_uplimit_option((index+wlanIdx), limit);
    }
    else
    {
        *limit = -1;
    }

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *limit);
    return CW_TRUE;
}

CWBool CWWTPBoardSetUploadLimitCfg(int radioIdx, int wlanIdx, int limit)
{
    int opMode=0, index=0;

    CWDebugLog("%s %u %u limit:[%d]", __FUNCTION__, radioIdx, wlanIdx, limit);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        if(limit != -1)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Upload Limit");
        }
        return CW_TRUE;
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if(limit == -1)
    {
        if (api_set_wifi_ifacex_tc_enabled_option((index+wlanIdx), 0))
        {
            return CW_FALSE;                
        }
    }
    else
    {
        if (api_set_wifi_ifacex_tc_enabled_option((index+wlanIdx), 1))
        {
            return CW_FALSE;                
        }

        if (api_set_wifi_ifacex_tc_uplimit_option((index+wlanIdx), limit))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}

/*v1 not support per user*/
CWBool CWWTPBoardGetDownloadPerUserEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int opMode=0, index=0;

    *enable = CW_FALSE;

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Download Limit");
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (api_get_wifi_ifacex_tc_downperuser_option((index+wlanIdx), enable))
    {
        return CW_FALSE;                
    }

    CWLog("%s %u %u %u", __FUNCTION__, radioIdx, wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetDownloadPerUserEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    int opMode=0, index=0;

    CWLog("%s %u %u %u", __FUNCTION__, radioIdx, wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Download Limit");
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (api_set_wifi_ifacex_tc_downperuser_option((index+wlanIdx), enable))
    {
        return CW_FALSE;                
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetUploadPerUserEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int opMode=0, index=0;

    *enable = CW_FALSE;

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Upload Limit");
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (api_get_wifi_ifacex_tc_upperuser_option((index+wlanIdx), enable))
    {
        return CW_FALSE;                
    }

    CWLog("%s %u %u %u", __FUNCTION__, radioIdx, wlanIdx, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetUploadPerUserEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    int opMode=0, index=0;

    CWLog("%s %u %u %u", __FUNCTION__, radioIdx, wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
        return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Guest wlan cannot support Upload Limit");
    }

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    if (api_set_wifi_ifacex_tc_upperuser_option((index+wlanIdx), enable))
    {
        return CW_FALSE;                
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetRoamingEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int opMode=0, index=0, val = 0;

    *enable = CW_FALSE;

    if(wlanIdx != 0)
    {
        return CW_TRUE;
    }

#if SUPPORT_FAST_ROAMING_PER_SSID
    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }
#endif

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
#if SUPPORT_FAST_ROAMING_PER_SSID
        api_get_wifi_ifacex_fastroaming_enable_option((index+wlanIdx), &val);
#else
        api_get_wifi_fastroaming_enable_option(WIRELESS_WIFI_5G_FASTROAMING_ENABLE_OPTION, &val);
#endif
    }
    else
#endif
    {
#if SUPPORT_FAST_ROAMING_PER_SSID
        api_get_wifi_ifacex_fastroaming_enable_option((index+wlanIdx), &val);
#else
        api_get_wifi_fastroaming_enable_option(WIRELESS_WIFI_FASTROAMING_ENABLE_OPTION, &val);
#endif
    }

    *enable = (val==1) ? CW_TRUE : CW_FALSE;

    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRoamingEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    int opMode=0, index=0;

    CWDebugLog("%s %u %u %u", __FUNCTION__, radioIdx, wlanIdx, enable);

    if(wlanIdx != 0)
    {
        if(enable != CW_FALSE)
        {
            return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "Only WLAN0 can support Roaming");
        }
        return CW_TRUE;
    }

#if SUPPORT_FAST_ROAMING_PER_SSID
    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }
#endif

#if SUPPORT_WLAN_5G_SETTING
    if (radioIdx)
    {
#if SUPPORT_FAST_ROAMING_PER_SSID
        if (api_set_wifi_ifacex_fastroaming_enable_option((index+wlanIdx), enable))
#else
        if (api_set_wifi_fastroaming_enable_option(WIRELESS_WIFI_5G_FASTROAMING_ENABLE_OPTION, enable))
#endif
        {
            return CW_FALSE;
        }
    }
    else
#endif
    {
#if SUPPORT_FAST_ROAMING_PER_SSID
        if (api_set_wifi_ifacex_fastroaming_enable_option((index+wlanIdx), enable))
#else
        if (api_set_wifi_fastroaming_enable_option(WIRELESS_WIFI_FASTROAMING_ENABLE_OPTION, enable))
#endif
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}
//NOT_FINISH (NOT_SUPPORT)
CWBool CWWTPBoardGetRoamingAdvSearchCfg(int radioIdx, int wlanIdx, int *enable)
{
    *enable = CW_FALSE;
    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, *enable);
    return CW_TRUE;
}
//NOT_FINISH (NOT_SUPPORT)
CWBool CWWTPBoardSetRoamingAdvSearchCfg(int radioIdx, int wlanIdx, int enable)
{
    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioDistance(int radioIdx, int *distance)
{
#if defined(CW_BOARD_EWS650AP) || defined(CW_BOARD_EWS660AP) || defined(CW_BOARD_EWS860AP) || defined(CW_BOARD_WAP660) || defined(CW_BOARD_WAP860)

    int val = 0, opMode = 0, index = 0;

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    api_get_wifi_iface_distance_option((radioIdx == 1) ? WIRELESS_WIFI_5G_DISTANCE_OPTION : WIRELESS_WIFI_DISTANCE_OPTION, &val);

    *distance = val;

#else
    *distance = 0;
#endif
    CWDebugLog("%s %d %d", __FUNCTION__, __LINE__, *distance);
    return CW_TRUE;
}

CWBool CWWTPBoardSetRadioDistance(int radioIdx, int distance)
{
    CWDebugLog("%s %u %u", __FUNCTION__, radioIdx, distance);

#if defined(CW_BOARD_EWS650AP) || defined(CW_BOARD_EWS660AP) || defined(CW_BOARD_EWS860AP) || defined(CW_BOARD_WAP660) || defined(CW_BOARD_WAP860)
    api_set_wifi_iface_distance_option((radioIdx == 1) ? WIRELESS_WIFI_5G_DISTANCE_OPTION : WIRELESS_WIFI_DISTANCE_OPTION, distance);
#else
    if(distance != 0)
    {
        return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, NULL);
    }
#endif

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioCurrentTxPower(int radioIdx, int *power)
{
    char ifname[8];
    int opMode = 0, index = 0;
    int wlanIdx;

    opMode = CWWTPBoardGetRadioMode(radioIdx);

    if (opMode == -1)
    {
        return CW_FALSE;
    }

    if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
    {
        return CW_FALSE;
    }

    for(wlanIdx = 0; wlanIdx < CWWTPBoardGetMaxRadioWlans(radioIdx); wlanIdx++)
    {
        api_get_wifi_ifacex_ifname_option(((index+wlanIdx)), ifname, sizeof(ifname));

        CWDebugLog("%s %d ifname:[%s]", __FUNCTION__, __LINE__, ifname);

        *power = sysutil_get_wifixTxpower(ifname);

        if (*power != 0)
        {
            break;
        }
    }

    CWDebugLog("%s radio:[%d] power:[%d]", __FUNCTION__, radioIdx, *power);

    return CW_TRUE;
}

CWBool CWWTPBoardGetRadioCurrentChannel(int radioIdx, int *channel)
{
    int val = 0;

    if (sysutil_check_current_wifix_disabled(radioIdx))
    {
#if SUPPORT_WLAN_5G_SETTING
        if (radioIdx)
        {
            if (api_get_wifi_5g_channel_option(WIRELESS_WIFI_5G_CHANNEL_OPTION, &val))
            {
                val = 0;
            }
        }
        else
#endif
        {
            if (api_get_wifi_channel_option(WIRELESS_WIFI_CHANNEL_OPTION, &val))
            {
                val = 0;
            }
        }
        *channel = val;
    }
    else
    {
        *channel = sysutil_get_wifixCurrentChannel(radioIdx);
    }

    if (*channel == 0)
    {
        *channel = (radioIdx)?36:1;
    }

    CWDebugLog("%s %d radio %u %u", __FUNCTION__, __LINE__, radioIdx, *channel);

    return CW_TRUE;
}

CWBool CWWTPBoardGetNextFastScanChannel(int radioIdx, int *channel)
{
    char *val =  NULL;
    if(!(val = CWCreateStringByCmdStdout("cat /proc/sys/dev/%s/fastscan_next_chan", WIFI_IF_NAME(radioIdx))))
    {
        return CW_FALSE;
    }

    *channel = atoi(val);

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardSetFastScanDurationTime(int radioIdx, unsigned int duration)
{
    char *val =  NULL;
    if(!(val = CWCreateStringByCmdStdout("`echo %u > /proc/sys/dev/%s/fastscan_duration`", duration, WIFI_IF_NAME(radioIdx))))
    {
        return CW_FALSE;
    }
    CWSystem("echo 0 > /tmp/notify_fastscan_enable");

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

CWBool CWWTPBoardGetScan(CWBool bDisplay, CWWTPSitesurveyInfo *sitesurveyInfo)
{
    int i;
    char tmpFile1[64], tmpFile2[64];
    FILE *fp;
    long fsize;
    char *buffer, *c;
    unsigned int bssid[6];
#if SUPPORT_WLAN_5G_2_SETTING
    char *wlan5G_2_if = NULL;
    unsigned int wifi1ChNum=0, wifi2ChNum=0; 
    static unsigned int scanCount=0;
#endif
    /////////////////////////////////////////////////////////////
    unsigned int uint32CapCode;
    int int32RadioIdx, int32WlanIdx, int32SsidEnable;
    CWBool bFound = CW_FALSE;

    if(!CWWTPBoardGetCapCode(&uint32CapCode))
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    if(!CWGetRadioIndex(sitesurveyInfo->radio, uint32CapCode, &int32RadioIdx))
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    for(int32WlanIdx = 0; int32WlanIdx < CWWTPBoardGetMaxRadioWlans(int32RadioIdx); int32WlanIdx++)
    {
        if(CWWTPBoardGetWlanEnableCfg(int32RadioIdx, int32WlanIdx, &int32SsidEnable))
        {
            if(int32SsidEnable)
            {
                bFound = CW_TRUE;
                break;
            }
        }
    }
#if SUPPORT_WLAN_5G_2_SETTING
    if(sitesurveyInfo->radio == CW_RADIOFREQTYPE_5G)
    {
	wifi1ChNum = atoi(CWCreateStringByCmdStdout("iwlist %s frequency | tail -n +2 | grep -c Channel", WLAN_IF_NAME(int32RadioIdx, int32WlanIdx)));
	if(wlan5G_2_if = CWCreateStringByCmdStdout("sh /sbin/getWifiFirstIF 2"))
	{
		CWSystem("echo %s >  /tmp/wlan5G_2_if", wlan5G_2_if);
		wifi2ChNum = atoi(CWCreateStringByCmdStdout("iwlist %s frequency | tail -n +2 | grep -c Channel", wlan5G_2_if));
	}
    }
#endif
    if(bFound == CW_FALSE)
    {
        return CW_FALSE;
    }
    /////////////////////////////////////////////////////////////

    sprintf(tmpFile1, "/tmp/WLAN_SITE_SURVEY%sG.%x.tmp", ((sitesurveyInfo->radio == CW_RADIOFREQTYPE_2G) ? "2" : "5"), (unsigned int) CWThreadSelf());
    sprintf(tmpFile2, "/tmp/WLAN_SITE_SURVEY%sG_adv.%x.tmp", ((sitesurveyInfo->radio == CW_RADIOFREQTYPE_2G) ? "2" : "5"), (unsigned int) CWThreadSelf());

    if(sitesurveyInfo->radio == CW_RADIOFREQTYPE_5G)
    {
        CWSystem("`echo 1 >  /proc/sys/dev/%s/isscanning`", WIFI_IF_NAME(int32RadioIdx));
#if SUPPORT_WLAN_5G_2_SETTING
	CWSystem("`echo 1 >  /proc/sys/dev/%s/isscanning`", WIFI_5G_2_IF_NAME);
#endif
    }

    if(bDisplay)
    {
        CWSystem("iwlist %s displayscan normal | tail -n +3 > %s",
                 WLAN_IF_NAME(int32RadioIdx, int32WlanIdx), tmpFile1);
        CWSystem("iwlist %s displayscan advance | tail -n +3 > %s",
                 WLAN_IF_NAME(int32RadioIdx, int32WlanIdx), tmpFile2);
#if SUPPORT_WLAN_5G_2_SETTING
	if(sitesurveyInfo->radio == CW_RADIOFREQTYPE_5G)
	{
		CWSystem("iwlist %s displayscan normal | tail -n +3 >> %s",
                 wlan5G_2_if, tmpFile1);
	        CWSystem("iwlist %s displayscan advance | tail -n +3 >> %s",
                 wlan5G_2_if, tmpFile2);
	}
#endif
    }
    else
    {
#if SUPPORT_WLAN_5G_2_SETTING
	if(sitesurveyInfo->radio == CW_RADIOFREQTYPE_5G)
        {		
		CWSystem("echo %d >  /tmp/wifi1ChNum", wifi1ChNum);
		CWSystem("echo %d >  /tmp/wifi2ChNum", wifi2ChNum);
		CWSystem("echo %d >  /tmp/scanCount", scanCount);
		
		scanCount = scanCount % (wifi1ChNum + wifi2ChNum);
		if(scanCount < wifi1ChNum)
		{
		        CWSystem("iwlist %s scanning active_quiet | tail -n +3 > %s",
				WLAN_IF_NAME(int32RadioIdx, int32WlanIdx), tmpFile1);
			CWSystem("echo %s >  /tmp/scanIf", WLAN_IF_NAME(int32RadioIdx, int32WlanIdx));
		}
		else
		{
			CWSystem("iwlist %s scanning active_quiet | tail -n +3 > %s",
                                wlan5G_2_if, tmpFile1);
			CWSystem("echo %s >  /tmp/scanIf", wlan5G_2_if);
		}
		scanCount++;
	}
	else
	{
		CWSystem("iwlist %s scanning active_quiet | tail -n +3 > %s",
			WLAN_IF_NAME(int32RadioIdx, int32WlanIdx), tmpFile1);	
	}
#else
        CWSystem("iwlist %s scanning active_quiet | tail -n +3 > %s",
                 WLAN_IF_NAME(int32RadioIdx, int32WlanIdx), tmpFile1);
#endif
    }

    if(sitesurveyInfo->radio == CW_RADIOFREQTYPE_5G)
    {
        CWSystem("`echo 0 >  /proc/sys/dev/%s/isscanning`", WIFI_IF_NAME(int32RadioIdx));
#if SUPPORT_WLAN_5G_2_SETTING
        CWSystem("`echo 0 >  /proc/sys/dev/%s/isscanning`", WIFI_5G_2_IF_NAME);
#endif

    }

    if(bDisplay == CW_FALSE)
    {
        unlink(tmpFile1);
        unlink(tmpFile2);
        return CW_TRUE;
    }

    sitesurveyInfo->version = 1;

    fp = fopen(tmpFile1, "r");
    if(!fp)
    {
        unlink(tmpFile1);
        unlink(tmpFile2);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        fclose(fp);
        unlink(tmpFile1);
        unlink(tmpFile2);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        fclose(fp);
        unlink(tmpFile1);
        unlink(tmpFile2);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(fsize < 0)
    {
        fclose(fp);
        unlink(tmpFile1);
        unlink(tmpFile2);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(!fsize)
    {
        sitesurveyInfo->infoCount = 0;
        sitesurveyInfo->info = NULL;
        fclose(fp);
        unlink(tmpFile1);
        unlink(tmpFile2);
        return CW_TRUE;
    }

    CW_CREATE_OBJECT_SIZE_ERR(buffer, fsize + 1,
    {
        fclose(fp);
        unlink(tmpFile1);
        unlink(tmpFile2);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    if(fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile1);
        unlink(tmpFile2);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile1);
        unlink(tmpFile2);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
        unlink(tmpFile1);
        unlink(tmpFile2);
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    char tmpstr[256];
    char *pch, *ptr;
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
            unlink(tmpFile1);
            unlink(tmpFile2);
            return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
        else
        {
            sitesurveyInfo->info[i].type = 0;
        }
        i++;
    }

    fclose(fp);
    unlink(tmpFile1);

    fp = fopen(tmpFile2, "r");
    if(!fp)
    {
        unlink(tmpFile2);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
            unlink(tmpFile2);
            return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
    unlink(tmpFile2);

    return CW_TRUE;
}

CWBool CWWTPBoardGetChannelUtilization(CWRadioFreqType radioType, int channel, unsigned char *chanUtil)
{
    unsigned int uint32CapCode;
    int i32RadioIdx, i;
    unsigned char *radioChannel;
    char *val =  NULL;

    if(!CWWTPBoardGetCapCode(&uint32CapCode))
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    if(!CWGetRadioIndex(radioType, uint32CapCode, &i32RadioIdx))
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    radioChannel = ((radioType == CW_RADIOFREQTYPE_2G) ? radio2GChannel : radio5GChannel);
    for(i = 0; i < ((radioType == CW_RADIOFREQTYPE_2G) ? sizeof(radio2GChannel) : sizeof(radio5GChannel)); i++)
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

    if(i == ((radioType == CW_RADIOFREQTYPE_2G) ? sizeof(radio2GChannel) : sizeof(radio5GChannel)))
    {
        return CW_FALSE;
    }

    return CW_TRUE;
}

CWBool CWWTPBoardSetScanDwellTime(int radioIdx, unsigned int min, unsigned int max)
{
    char *val =  NULL;

    if(!(val = CWCreateStringByCmdStdout("`echo %u > /proc/sys/dev/%s/fastscan_active_dwell_min`", min, WIFI_IF_NAME(radioIdx))))
    {
        return CW_FALSE;
    }


    CW_FREE_OBJECT(val);

    if(!(val = CWCreateStringByCmdStdout("`echo %u > /proc/sys/dev/%s/fastscan_active_dwell_max`", max, WIFI_IF_NAME(radioIdx))))
    {
        return CW_FALSE;
    }

    CW_FREE_OBJECT(val);

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
    int wlanEnable = 0;

    CWDebugLog("%s %d radioIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, radioIdx, enable);

    if(enable && CWWTPBoardGetWlanEnableCfg(radioIdx, 0, &wlanEnable) && wlanEnable)
    {
        CWSystem("sn_acs --r %d --rxth 90 --rxthnc 50 --n 6 --wt 600 --pt 10 --rs 1 >/dev/null 2>&1 &", WIFI_IF_ID(radioIdx));
    }
    else
    {
        CWSystem("ps |grep sn_acs |grep \"\\-\\-r %d\" |awk '{printf $1}'|xargs kill -9 >/dev/null 2>&1 &", WIFI_IF_ID(radioIdx));
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
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(CWGetRadioType(&radioType, uint32CapCode, i32RadioIdx) == CW_FALSE)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

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

    sprintf(tmpFile1, "/tmp/CHANNELlIST.%sG.%x.tmp", ((radioType == CW_RADIOFREQTYPE_2G) ? "2" : "5"), (unsigned int) CWThreadSelf());
    if(IS_11AC_RADIO(i32RadioIdx))
    {
        CWSystem("wlanconfig %s list chan | cut -c -49 | sed -e \"s/^.\\{8\\}\\(...\\)...\\(....\\)\\(.\\+\\) Mhz \\(....\\).\\(.\\)..\\(.\\) ...\\(.\\) ....\\(...\\)/\\4,\\5,\\6,\\1,\\7,\\8,/g\" -e \"s/[ ]//g\" -e \"s/CL/L/g\" -e \"s/CU/U/g\" > %s", WLAN_IF_NAME(i32RadioIdx, i32WlanIdx), tmpFile1);
        CWSystem("wlanconfig %s list chan | cut -c 50-|sed -e \"s/^  *//g\"|grep ^Chan | sed -e \"s/^.\\{8\\}\\(...\\)...\\(....\\)\\(.\\+\\) Mhz \\(....\\).\\(.\\)..\\(.\\) ...\\(.\\) ....\\(...\\)/\\4,\\5,\\6,\\1,\\7,\\8,/g\" -e \"s/[ ]//g\" -e \"s/CL/L/g\" -e \"s/CU/U/g\" >> %s", WLAN_IF_NAME(i32RadioIdx, i32WlanIdx), tmpFile1);
    }
    else
    {
        CWSystem("wlanconfig %s list chan | cut -c -38 | sed -e \"s/^.\\{8\\}\\(...\\)...\\(....\\)\\(.\\)\\(.\\).* Mhz \\(....\\).\\(.\\)..\\(.*\\)$/\\5,\\6,\\7,\\1,/g\" -e \"s/[ ]//g\" -e \"s/CL/L/g\" -e \"s/CU/U/g\" > %s", WLAN_IF_NAME(i32RadioIdx, i32WlanIdx), tmpFile1);
        CWSystem("wlanconfig %s list chan | cut -c 39-|sed -e \"s/^  *//g\"|grep ^Chan | sed -e \"s/^.\\{8\\}\\(...\\)...\\(....\\)\\(.\\)\\(.\\).* Mhz \\(....\\).\\(.\\)..\\(.*\\)$/\\5,\\6,\\7,\\1,/g\" -e \"s/[ ]//g\" -e \"s/CL/L/g\" -e \"s/CU/U/g\" >> %s", WLAN_IF_NAME(i32RadioIdx, i32WlanIdx), tmpFile1);
    }
    //CWSystem("cat %s > /dev/ttyS0", tmpFile1);

    fp = fopen(tmpFile1, "r");
    if(!fp)
    {
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(fseek(fp, 0L, SEEK_END) != 0)
    {
        fclose(fp);
        unlink(tmpFile1);
        unlink(tmpFile2);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    else
    {
        fsize = ftell(fp);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        fclose(fp);
        unlink(tmpFile1);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(fsize < 0)
    {
        fclose(fp);
        unlink(tmpFile1);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    if(fread(buffer, 1, fsize, fp) != fsize)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile1);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }

    if(fseek(fp, 0L, SEEK_SET) != 0)
    {
        CW_FREE_OBJECT(buffer);
        fclose(fp);
        unlink(tmpFile1);
        CWDebugLog("%s %d fseek error", __func__, __LINE__);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
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
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
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
            //CWDebugLog ("%s",pch);
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
            //CWDebugLog ("%s",pch);
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
            //CWDebugLog ("%s",pch);
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
            //CWDebugLog ("%s",pch);
            pSupportChanList[(*pAvalaibleChanCount)++] = (char)atoi(pch);
        }
        i++;
    }

    fclose(fp);
    unlink(tmpFile1);

    CW_CREATE_ZERO_ARRAY_ERR(*pAvailableChanList, *pAvalaibleChanCount, unsigned char,
    {
        return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
    });

    for(i = 0; i < *pAvalaibleChanCount; i++)
    {
        (*pAvailableChanList)[i] = pSupportChanList[i];
    }

    CW_FREE_OBJECT(pSupportChanList);

    CWDebugLog("%s radio %u", __FUNCTION__, i32RadioIdx);

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

    CWDebugLog("WTP auto txpower request radiotype: %s healing txpower: %d%%",
               ((radioType == CW_RADIOFREQTYPE_2G) ? "2.4G" : "5G"), txPower);

    if(!CWWTPBoardGetCapCode(&capCode))
    {
        return CW_FALSE;
    }

    if(!CWGetRadioIndex(radioType, capCode, &radioIdx))
    {
        return CW_FALSE;
    }

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
            CWSystem("iwconfig %s txpower %d", WLAN_IF_NAME(radioIdx, wlanIdx), txPower);
            break;
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetWlanWpaPMFEnableCfg(int radioIdx, int wlanIdx, int *enable)
{
    int opMode = 0, index = 0, val = 0;

    if (GUEST_WLAN_IDX(radioIdx) == wlanIdx)
    {
#if SUPPORT_WLAN_5G_SETTING
        if (radioIdx) 
        {
            if (api_get_wifi_guest_pmf_enable_option(WIRELESS_WIFI_5G_GUEST_PMF_ENABLE_OPTION, &val))
            {
                val = 0;
            }
        }
        else
#endif
        {
            if (api_get_wifi_guest_pmf_enable_option(WIRELESS_WIFI_GUEST_PMF_ENABLE_OPTION, &val))
            {
                val = 0;
            }
        }
    }
    else
    {
        opMode = CWWTPBoardGetRadioMode(radioIdx);
    
        if (!sys_get_wifi_iface_index(&index, radioIdx, opMode)) 
        {
            return CW_FALSE;
        }
    
        if (api_get_wifi_ifacex_pmf_enable_option((index+wlanIdx), &val))
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

    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] enable:[%d]", __FUNCTION__, __LINE__, radioIdx, wlanIdx, *enable);
    return CW_TRUE;
}

CWBool CWWTPBoardSetWlanWpaPMFEnableCfg(int radioIdx, int wlanIdx, int enable)
{
    int opMode = 0, index = 0;

    CWDebugLog("%s %d %d %d", __FUNCTION__, radioIdx, wlanIdx, enable);

    if(wlanIdx == GUEST_WLAN_IDX(radioIdx))
    {
#if SUPPORT_WLAN_5G_SETTING
        if (radioIdx) 
        {
            if (api_set_wifi_guest_pmf_enable_option(WIRELESS_WIFI_5G_GUEST_PMF_ENABLE_OPTION, enable))
            {
                return CW_FALSE;
            }
        }
        else
#endif
        {
            if (api_set_wifi_guest_pmf_enable_option(WIRELESS_WIFI_GUEST_PMF_ENABLE_OPTION, enable))
            {
                return CW_FALSE;
            }
        }
    }
    else
    {
        opMode = CWWTPBoardGetRadioMode(radioIdx);

        if (!sys_get_wifi_iface_index(&index, radioIdx, opMode))
        {
            return CW_FALSE;
        }

        if (api_set_wifi_ifacex_pmf_enable_option((index+wlanIdx), enable))
        {
            return CW_FALSE;
        }
    }

    return CW_TRUE;
}
