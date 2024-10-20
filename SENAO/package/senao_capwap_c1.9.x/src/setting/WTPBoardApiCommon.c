#include "WTPBoardApiCommon.h"
#include <ctype.h>
#include <gconfig.h>
#include <sysCore.h>
#include <sysFile.h>
#include <sysAddr.h>
#include <api_tokens.h>

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

#define RLF_IS_SET(v, f) (v & (f))
#define RLF_SET(v, f)    (v |= (f))
#define RLF_EQ(v, f)    (v == (f))

#ifdef CW_DEF_AP_WIFI_IFACE_NUM
extern int configuration_wlan_traffic_shapping[WIFI_RADIO_NUM][CW_DEF_AP_WIFI_IFACE_NUM+1];
extern int configuration_wlan_nas[WIFI_RADIO_NUM][CW_DEF_AP_WIFI_IFACE_NUM+1];
#else
extern int configuration_wlan_traffic_shapping[WIFI_RADIO_NUM][8+1];
extern int configuration_wlan_nas[WIFI_RADIO_NUM][8+1];
#endif

void CWWTPBoardGetWtpCfgCap(CWWtpCfgCap cfgCap)
{
    CW_WTP_CFG_CAP_CLEAR(cfgCap);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_NAME);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LOCATION);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_IPV4);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_DNS1);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_DNS2);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_ADMIN);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_PASSWORD_MD5);
#if SUPPORT_MESH_SETTING
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_MESH_ENABLE_TOTAL);
#endif
#if !SUPPORT_COMBINED_SSID_SETTING /* Bandsteering is not supported by single-radio models */
    if (CWWTPBoardGetMaxRadio() > 1)
    {
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_BAND_STREERING);
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_BANDSTREERING_MODE);
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_BANDSTREERING_PERCENT_ENABLE);
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_BANDSTREERING_RSSI_ENABLE);
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_BANDSTREERING_PERCENT);
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_BANDSTREERING_RSSI);
    }
#endif
#if !SUPPORT_FAST_HANDOVER_INDEPENDENT_SETTING
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_FAST_HANDOVER_STATUS);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_FAST_HANDOVER_RSSI);
#endif
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_MANAGEMENT_VLAN_ID);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_AC_LIST);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_GUEST_NETWORK_IP);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_GUEST_NETWORK_MASK);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_GUEST_NETWORK_DHCP_START);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_GUEST_NETWORK_DHCP_END);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_GUEST_NETWORK_WINS_SERVER);

    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LOG_REMOTE_SERVER_ENABLE);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LOG_REMOTE_SERVER_CONFIG);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LOG_TRAFFIC_ENABLE);

    /*not support */
    //CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_TIME_ZONE);

    if (!IS_OUTDOOR_AP())
    {
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LED_POWER);
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LED_LAN);
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LED_WLAN0);
        if (CWWTPBoardGetMaxRadio() > 1)
        {
            CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LED_WLAN1);
            if(CWWTPBoardGetMaxRadio() > 2)
            {
                CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LED_WLAN2);
            }
        }
#if SUPPORT_MESH_SETTING
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LED_MESH);
#endif
    }
    if (CWWTPBoardGetMaxLanPortNum() > 0)
    {
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LAN_PORT_NUM);
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LAN_PORT_ENABLE);
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LAN_PORT_VLAN_ID);
        CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_LAN_PORT_VLAN_MODE);
    }
}

void CWWTPBoardGetRadioCfgCap(int radioIdx, CWWtpCfgCap cfgCap)
{
    CW_WTP_CFG_CAP_CLEAR(cfgCap);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_OPERATION_MODE);
//    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_WIRELESS_MODE); //replace by WTP_CFG_AP_RADIO_AX_ENABLE
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_COUNTRY_CODE);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_CHANNEL_HT_MODE);
#if SUPPORT_WLAN_EXTENSION_CHANNEL
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_CHANNEL_EXT);
#endif
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_CHANNEL);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_LIMITED_CLIENTS_ENABLE);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_LIMITED_CLIENTS);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_TX_POWER);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_BIT_RATE);
    CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_AX_ENABLE);
	if (CWWTPBoardConfigShowOnly() == CW_FALSE)
	{
#if SUPPORT_MESH_SETTING
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_MESH_ENABLE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_MESH_ID);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_MESH_WPA_KEY);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_MESH_LINK_ROBUST_THRESHOLD);
#endif
		/* 11ac radio is no longer allowed to change aggregation settings */
		if (!IS_11AC_RADIO(CWConvertRadioIdx(radioIdx)))
		{
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_AGGRE_ENABLE);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_AGGRE_FRAMES);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_AGGRE_MAXBYTES);
		}
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_DATA_RATE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_RTSCTS_THRESHOLD);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_OBEY_REGULATORY_POWER);
		if (IS_OUTDOOR_AP())
		{
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_DISTANCE);
		}
#if SUPPORT_FAST_HANDOVER_INDEPENDENT_SETTING
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_FAST_HANDOVER_STATUS);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_FAST_HANDOVER_RSSI);
#endif
#if !defined(CW_VENDOR_ATI) && (!SUPPORT_COMBINED_SSID_SETTING)
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_ENABLE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_LOGIN_TYPE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_RADIUS_SERVER);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_REDIRECT_BEHAVIOR);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_WALLEDGARDEN);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_WALLEDGARDEN_PAGE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_RADIUS_PORT);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_RADIUS_SECRET);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_SESSION_TIMEOUT);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_SETIMEOUT_ENABLE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_IDLE_TIMEOUT);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_IDTIMEOUT_ENABLE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_ENABLE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_TIME);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_AUTH_TYPE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_EXTERNAL_SERVER);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_EXTERNAL_SECRET);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_SERVER);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_PORT);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_SECRET);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_UAMFORMAT);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_LOCAL_AUTH);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_PORT);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_HTTPS_ENABLE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_RADIUS_SECRET2);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_RADIO_PORTAL_RADIUS_SERVER2);
#endif /* !CW_VENDOR_ATI */
	}
}

void CWWTPBoardGetWlanCfgCap(int radioIdx, int wlanIdx, CWWtpCfgCap cfgCap)
{
    CW_WTP_CFG_CAP_CLEAR(cfgCap);
	if (CWWTPBoardConfigShowOnly() == CW_FALSE)
	{
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_ENABLE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_SSID);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_SUPPRESSED_SSID);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_SECURITY_MODE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_STA_SEPARATION);
#if !SUPPORT_COMBINED_SSID_SETTING /* Bandsteering is not supported by single-radio models */
		if (wlanIdx == 0)
		{
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_WEP_AUTH_TYPE);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_WEP_KEY_LENGTH);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_WEP_DEF_KEY_ID);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_WEP_KEY);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_WEP_INPUT_METHOD);
		}
#endif
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_WPA_ENCRYPT_MODE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_WPA_PASSPHRASE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_WPA_GROUP_KEY_INT);
#if SUPPORT_WPA3
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_SUITEB_ENABLE);
#endif
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_ACL_MODE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_ACL_MAC_LIST);
#if !SUPPORT_ALL_ENCRYPTION_HAS_RADIUS_ACCOUNTING
		if (wlanIdx != GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
#endif
		{
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_LAYER2_ISOLATION);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_ISOLATION);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_VLAN);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_RADIUS_ADDR);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_RADIUS_PORT);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_RADIUS_SECRET);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_RADIUS_ACC_ENABLE);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_RADIUS_ACC_ADDR);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_RADIUS_ACC_PORT);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_RADIUS_ACC_SECRET);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_RADIUS_ACC_INTERIM_INT);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_DOWNLOAD_LIMIT);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_UPLOAD_LIMIT);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_DOWNLOAD_PERUSER_ENABLE);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_UPLOAD_PERUSER_ENABLE);
#if !defined(CW_VENDOR_ATI)
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_NAS_ID_ENABLE);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_NAS_ID);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_NAS_PORT_ENABLE);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_NAS_PORT);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_NAS_IP_ENABLE);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_NAS_IP);
#endif
#if !SUPPORT_FAST_ROAMING_PER_SSID
			if (wlanIdx == 0)
#else
			if (wlanIdx != GUEST_WLAN_IDX(CWConvertRadioIdx(radioIdx)))
#endif
			{
				CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_ROAMING_ENABLE);
				CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_ROAMING_ADV_SEARCH);
			}
		}

#if SUPPORT_COMBINED_SSID_SETTING /*only support SmartWrt: bandSteer/portal  per ssid*/
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_ENABLE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_LOGIN_TYPE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_RADIUS_SERVER);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_REDIRECT_BEHAVIOR);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_WALLEDGARDEN);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_WALLEDGARDEN_PAGE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_RADIUS_PORT);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_RADIUS_SECRET);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_SESSION_TIMEOUT);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_SETIMEOUT_ENABLE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_IDLE_TIMEOUT);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_IDTIMEOUT_ENABLE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_ENABLE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_TIME);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_AUTH_TYPE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_EXTERNAL_SERVER);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_EXTERNAL_SECRET);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_SERVER);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_PORT);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_SECRET);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_UAMFORMAT);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_LOCAL_AUTH);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_PORT);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_NETRORK_MODE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_HTTPS_ENABLE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_RADIUS_SECRET2);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_PORTAL_RADIUS_SERVER2);

		if (CWWTPBoardGetMaxRadio() > 1)
		{
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_BANDSTREERING_MODE);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_BANDSTREERING_PERCENT_ENABLE);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_BANDSTREERING_RSSI_ENABLE);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_BANDSTREERING_PERCENT);
			CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_BANDSTREERING_RSSI);
		}

		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_DOWNLOAD_LIMIT_MODE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_UPLOAD_LIMIT_MODE);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_L2_ISOLATION_WHITE_MAC_LIST);
		CW_WTP_CFG_CAP_ON(cfgCap, WTP_CFG_AP_WLAN_HOTSPOT20_JSON);
#endif /*support smartWrt*/
	}
}

#if SUPPORT_MAC_FILTER_NO_RELOAD
CWBool GetSectionName(char *cfg, char *buf, int length)
{
    int len = 0;
    char *cur, *pstr, *estr;

    if (cfg == NULL || buf == NULL)
    {
        return CW_FALSE;
    }

    cur = cfg;
    pstr = strchr(cur, '.')+1;
    estr = strchr(pstr, '.')+1;
    len = (estr-pstr);

    if (len < length)
    {
        snprintf(buf, len, "%s", pstr);
    }

    return CW_TRUE;
}

CWBool UpdateSectionNameList(char *cfg, char *list, int length)
{
    int len = 0;
    char name[31+1];

    if (cfg == NULL || list == NULL)
    {
        return CW_FALSE;
    }

    if (GetSectionName(cfg, name, sizeof(name)))
    {
        if (strstr(list, name)==NULL)
        {
            len = strlen(list);

            if (len < length)
            {
                if (len > 0)
                {
                    strncat(list, " ", length);
                }
                strncat(list, name, length);
            }
        }
    }
    return CW_TRUE;
}

char *GetAclSectionNameList()
{
    char *val, *cur, *next;
    static char buf[1023+1];

    CWDebugLog("%s %d", __FUNCTION__, __LINE__);

    if(!(val = CWCreateStringByCmd("uci changes")))
    {
        return 0;
    }

    memset(buf, 0, sizeof(buf));

    cur = val;

    while(1)
    {
        next = cur;

        while(*next != '\n' && *next != '\0')
        {
            next++;
        }

        if(*next == '\0')
        {
            /* last line */
            next = NULL;
        }
        else
        {
            *next = '\0';
        }

        if(!strncmp("wireless.", cur, 9) || !strncmp("-wireless.", cur, 10) || !strncmp("mesh.", cur, 5))
        {
            if(strstr(cur, ".macfilter") || strstr(cur, ".allowmaclist") || strstr(cur, ".denymaclist"))
            {
                UpdateSectionNameList(cur, buf, sizeof(buf));
            }
        }

        if(!next)
        {
            break;
        }

        cur = next + 1;
    }

    CW_FREE_OBJECT(val);

    return buf;
}
#endif

ReloadFlag_t GetDependentFlag()
{
    char *val, *cur, *next;
    DependentFlag_t flag = 0;

    if(!(val = CWCreateStringByCmd("uci changes")))
    {
        return 0;
    }

    cur = val;

    while(1)
    {
        next = cur;

        while(*next != '\n' && *next != '\0')
        {
            next++;
        }

        if(*next == '\0')
        {
            /* last line */
            next = NULL;
        }
        else
        {
            *next = '\0';
        }

        if (!strncmp("wireless.", cur, 9) || !strncmp("-wireless.", cur, 10))
        {
            if (strstr(cur, ".tc_"))
            {
                RLF_SET(flag, DPF_TRAFFIC_SHAPING);
            }
            else if (strstr(cur, ".nas"))
            {
                RLF_SET(flag, DPF_NAS);
            }
            else if (strstr(cur, "mesh") || strstr(cur, ".channel"))
            {
                RLF_SET(flag, DPF_MESH);
            }
        }

        if(!next)
        {
            break;
        }

        cur = next + 1;
    }

    CW_FREE_OBJECT(val);

    return flag;
}

ReloadFlag_t GetReloadFlag()
{
    char *val, *cur, *next;
    ReloadFlag_t flag = 0;

    if(!(val = CWCreateStringByCmd("uci changes")))
    {
        return 0;
    }

    cur = val;

    while(1)
    {
        next = cur;

        while(*next != '\n' && *next != '\0')
        {
            next++;
        }

        if(*next == '\0')
        {
            /* last line */
            next = NULL;
        }
        else
        {
            *next = '\0';
        }

        if(!strncmp("network.", cur, 8) || !strncmp("-network.", cur, 9))
        {
            RLF_SET(flag, RLF_NETWORK);
        }
        else if(!strncmp("dhcp.", cur, 5) || !strncmp("-dhcp.", cur, 6))
        {
            /* Guestnetwork must reload wureless script */
            RLF_SET(flag, RLF_DHCP);
        }
        else if(!strncmp("wireless.", cur, 9) || !strncmp("-wireless.", cur, 10) || !strncmp("mesh.", cur, 5))
        {
            if (strstr(cur, ".isolation") || strstr(cur, ".vlan_id"))
            {
                RLF_SET(flag, RLF_VLANISOLATION);
            }
#if SUPPORT_MAC_FILTER_NO_RELOAD
            else if(strstr(cur, ".macfilter") || strstr(cur, ".allowmaclist") || strstr(cur, ".denymaclist"))
            {
                RLF_SET(flag, RLF_MACFILTER);
            }
            else
#endif
            {
                RLF_SET(flag, RLF_WIRELESS);
            }
        }
	else if(!strncmp("portal.", cur, 7))
	{
	    RLF_SET(flag, RLF_PORTAL);
	}
        else if (!strncmp("fastroaming.", cur, 12) || !strncmp("-fastroaming.", cur, 13))
        {
            RLF_SET(flag, RLF_FASTROAMING);
        }
        else if (!strncmp("wifi_schedule.", cur, 14) || !strncmp("-wifi_schedule.", cur, 15))
        {
            RLF_SET(flag, RLF_WIRELESS_SCHEDULE);
        }
        else if (!strncmp("eccaptive.", cur, 10) || !strncmp("-eccaptive.", cur, 11))
        {
            RLF_SET(flag, RLF_ECCAPTIVE);
        }
        else if(!strncmp("system.", cur, 7) || !strncmp("-system.", cur, 8))
        {
            if(strstr(cur, ".@led") || strstr(cur, "_led."))
            {
                RLF_SET(flag, RLF_LED);
            }
            else
            {
                RLF_SET(flag, RLF_SYSTEM);
            }
        }

        if(!next)
        {
            break;
        }

        cur = next + 1;
    }

    CW_FREE_OBJECT(val);

    return flag;
}

#if 0
char *CWCreateStringByUci(const char *fmt, ...)
{
    char cmd[256], uci[128], *c, *buf = NULL, *str = NULL;
    int len;
    va_list va;

    va_start(va, fmt);

    vsnprintf(uci, sizeof(uci), fmt, va);

    va_end(va);

    sprintf(cmd, "uci show %s 2> /dev/null", uci);

    if(!(buf = CWCreateStringByCmd(cmd)))
    {
        return NULL;
    }

    c = buf;
    while(*c != '=' && *c != '\0')
    {
        c++;
    }
    /* check if buffer's content is empty */
    if(*c != '=')
    {
        CW_FREE_OBJECT(buf);
        CW_CREATE_OBJECT_SIZE_ERR(str, 1,
        {
            CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
            return NULL;
        });
        str[0] = '\0';
        return str;
    }

    c++;
    len = strlen(c);
    /* remove '\n' in the end */
    if(c[len - 1] == '\n')
    {
        c[len - 1] = '\0';
        len--;
    }

    CW_CREATE_OBJECT_SIZE_ERR(str, len + 1,
    {
        CW_FREE_OBJECT(buf);
        CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
        return NULL;
    });

    memcpy(str, c, len);
    str[len] = '\0';

    CW_FREE_OBJECT(buf);
    return str;
}
#else
char *CWCreateStringByUci(const char *fmt, ...)
{
    char cmd[256], uci[128], *buf = NULL;
    int len;
    va_list va;

    va_start(va, fmt);

    vsnprintf(uci, sizeof(uci), fmt, va);

    va_end(va);

    sprintf(cmd, "uci -q get %s 2> /dev/null", uci);

    if(!(buf = CWCreateStringByCmd(cmd)))
    {
        return NULL;
    }

    len = strlen(buf);
    /* remove '\n' in the end */
    if(buf[len - 1] == '\n')
    {
        buf[len - 1] = '\0';
        len--;
    }

    return buf;
}

char *CWCreateUciString(const char *pSrcStr)
{
    unsigned int addCount = 0;
    const char *pSrcChr;
    char *pDstStr, *pDstChr;

    if(pSrcStr == NULL)
    {
        return NULL;
    }

    for(pSrcChr = pSrcStr; *pSrcChr != '\0'; pSrcChr++)
    {
        if((*pSrcChr == '"') || (*pSrcChr == '`') || (*pSrcChr == '$') || (*pSrcChr == '\\') || (*pSrcChr == '*'))
        {
            addCount++;
        }
    }

    if(!addCount)
    {
        CW_CREATE_STRING_FROM_STRING_ERR(pDstStr, pSrcStr,
                                         CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                                         return NULL;
                                        );
        return pDstStr;
    }

    CW_CREATE_STRING_ERR(pDstStr, ((pSrcChr - pSrcStr) + addCount),
                         CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);
                         return NULL;
                        );
    pDstChr = pDstStr;
    for(pSrcChr = pSrcStr; *pSrcChr != '\0'; pSrcChr++)
    {
        if(*pSrcChr == '"')
        {
            *(pDstChr++) = '\\';
            *(pDstChr++) = '"';
        }
        else if(*pSrcChr == '`')
        {
            *(pDstChr++) = '\\';
            *(pDstChr++) = '`';
        }
        else if(*pSrcChr == '$')
        {
            *(pDstChr++) = '\\';
            *(pDstChr++) = '$';
        }
        else if(*pSrcChr == '\\')
        {
            *(pDstChr++) = '\\';
            *(pDstChr++) = '\\';
        }
        else if(*pSrcChr == '*')
        {
            *(pDstChr++) = '\\';
            *(pDstChr++) = '*';
        }
        else
        {
            *(pDstChr++) = *pSrcChr;
        }
    }

    *pDstChr = '\0';

    return pDstStr;
}
#endif

CWBool CWWTPBoardInitConfiguration()
{
    char *val, *c;
    int timeout = 10, vlanId;
    CWAcAddress acAddr;

    /* clear uci */
    CWWTPBoardCancelCfg();

    if(sysutil_check_file_existed("/etc/ewsconfig"))
    {
        /* read previous Force AC from ewsconfig */
        if(val = CWCreateStringByCmdStdout("cat /etc/ewsconfig | grep -w force_ac"))
        {
            if((c = strchr(val, '=')) != NULL)
            {
                c++;
                CWLog("Restore Force AC %s", c);
                CWWTPBoardSetForceAcAddress(c);
            }
            CW_FREE_OBJECT(val);
        }

        /* read previous AC from ewsconfig */
        if(val = CWCreateStringByCmdStdout("cat /etc/ewsconfig | grep -w ac"))
        {
            if((c = strchr(val, '=')) != NULL)
            {
                c++;
                CWLog("Restore AC %s", c);

                if(CWParseAcAddrString(c, &acAddr))
                {
                    CWWTPBoardSetAcAddress(&acAddr);
                }
            }
            CW_FREE_OBJECT(val);
        }

        /* determine ap's management vlan was enabled in last reboot by checking mngvlan flag in ewsconfig */
        if(val = CWCreateStringByCmdStdout("cat /etc/ewsconfig | grep -w ManagementVLANID"))
        {
            /* if mngvlan flag is found in ewsconfig, restore management vlan so that controller can discover this ap */
            if(val[0] != '\0' && (c = strchr(val, '=')))
            {
                c++;
                vlanId = atoi(c);

                CWLog("Restore Management VLAN to %d", vlanId);

                if(!(CWWTPBoardSetWlanIsolationCfg(0, 0, 1) && CWWTPBoardSetManageVlanCfg(vlanId) && CWWTPBoardApplyCfg()))
                {
                    return CW_FALSE;
                }
            }
            CW_FREE_OBJECT(val);
        }

        /* clear the ewsconfig once we have successfully restored it */
        CWSystem("rm -rf /etc/ewsconfig");
    }

    /* check if we will set to dhcp mode */
    if(!(val = CWCreateStringByUci("network.lan.proto")))
    {
        return CW_FALSE;
    }

    if(!strcmp(val, "dhcp"))
    {
        /* wait dhcp ready by polling lease_acquired in /tmp/state/network */
        while(timeout)
        {
            /* check if lease_acquired is found */
            if (sysutil_dhcp_ip_ready())
            {
                CWDebugLog("DHCP is ready");
                break;
            }

            CWDebugLog("Waiting DHCP ready for %d second...", timeout);

            CWWaitSec(1);

            timeout--;
        }
    }

    CW_FREE_OBJECT(val);

    return CW_TRUE;
}

int CWWTPBoardGetApplyCfgTime(CWBool *rejoin)
{
    CWBool reload = CW_FALSE;
    ReloadFlag_t flag;

    *rejoin = CW_FALSE;

    flag = GetReloadFlag();

    if(RLF_IS_SET(flag, RLF_NETWORK))
    {
        *rejoin = CW_TRUE;
        reload = CW_TRUE;
    }
    else if(RLF_IS_SET(flag, RLF_WIRELESS | RLF_FASTROAMING | RLF_ECCAPTIVE | RLF_TRAFFICSHAPPING | RLF_SYSTEM | RLF_DHCP |RLF_PORTAL | RLF_VLANISOLATION))
    {
        reload = CW_TRUE;
    }

    /* estimating time */
    return reload ? 600 : 0;
}

int CWWTPConvertKbpsToMbps(int input)
{
    int retVal = ( input + 999 ) / 1000;

    if ( retVal > 999 )
        retVal = 999;

    return retVal;
}

int CWWTPConvertMbpsToKbps(int input)
{
    int retVal = input * 1000;

    return retVal;
}

CWBool CWWTPBoardConfigurationTrafficShapingCfg(int radioIdx, int wlanIdx)
{
    int trafficShapingEnable = CW_FALSE;
    int downloadPerUserEnable = CW_FALSE, uploadPerUserEnable = CW_FALSE;
    int downLoadLimit = 0, upLoadLimit = 0;

    radioIdx = CWConvertRadioIdx(radioIdx);

    if (api_get_bool_option2(&trafficShapingEnable, "wireless.wifi%d_ssid_%d.tc_enabled", radioIdx, wlanIdx+1))
    {
        trafficShapingEnable = CW_FALSE;
    }

    if (trafficShapingEnable == CW_FALSE)
    {
        return CW_TRUE;
    }

    if (CWWTPBoardGetDownloadPerUserEnableCfg(radioIdx, wlanIdx, &downloadPerUserEnable) == CW_FALSE)
    {
        downloadPerUserEnable = CW_FALSE;
    }

    if (api_get_integer_option2(&downLoadLimit, "wireless.wifi%d_ssid_%d.tc_downlimit", radioIdx, wlanIdx+1))
    {
        downLoadLimit = 0;
    }

    if (downLoadLimit == 0)
    {
        if (api_get_integer_option2(&downLoadLimit, "wireless.wifi%d_ssid_%d.tc_downmaxlimit", radioIdx, wlanIdx+1))
        {
            downLoadLimit = 0;
        }
    }

    if (downloadPerUserEnable == CW_TRUE)
    {
        if (CWWTPBoardSetWlanPerUserDownloadLimitCfg(radioIdx, wlanIdx, downLoadLimit) == CW_FALSE)
            return CW_FALSE;
        if (CWWTPBoardSetWlanMaxDownloadLimitCfg(radioIdx, wlanIdx, 0) == CW_FALSE)
            return CW_FALSE;
    }
    else
    {
        if (CWWTPBoardSetWlanPerUserDownloadLimitCfg(radioIdx, wlanIdx, 0) == CW_FALSE)
            return CW_FALSE;
        if (CWWTPBoardSetWlanMaxDownloadLimitCfg(radioIdx, wlanIdx, downLoadLimit) == CW_FALSE)
            return CW_FALSE;
    }

    if (CWWTPBoardGetUploadPerUserEnableCfg(radioIdx, wlanIdx, &uploadPerUserEnable) == CW_FALSE)
    {
        uploadPerUserEnable = CW_FALSE;
    }

    if (api_get_integer_option2(&upLoadLimit, "wireless.wifi%d_ssid_%d.tc_uplimit", radioIdx, wlanIdx+1))
    {
        upLoadLimit = 0;
    }

    if (upLoadLimit == 0)
    {
        if (api_get_integer_option2(&upLoadLimit, "wireless.wifi%d_ssid_%d.tc_upmaxlimit", radioIdx, wlanIdx+1))
        {
            upLoadLimit = 0;
        }
    }

    if (uploadPerUserEnable == CW_TRUE)
    {
        if (CWWTPBoardSetWlanPerUserUploadLimitCfg(radioIdx, wlanIdx, upLoadLimit) == CW_FALSE)
            return CW_FALSE;
        if (CWWTPBoardSetWlanMaxUploadLimitCfg(radioIdx, wlanIdx, 0) == CW_FALSE)
            return CW_FALSE;
    }
    else
    {
        if (CWWTPBoardSetWlanPerUserUploadLimitCfg(radioIdx, wlanIdx, 0) == CW_FALSE)
            return CW_FALSE;
        if (CWWTPBoardSetWlanMaxUploadLimitCfg(radioIdx, wlanIdx, upLoadLimit) == CW_FALSE)
            return CW_FALSE;
    }
    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] trafficShapingEnable:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, trafficShapingEnable);
    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] downloadPerUserEnable:[%d] downLoadLimit:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, downloadPerUserEnable, downLoadLimit);
    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] uploadPerUserEnable:[%d] upLoadLimit:[%d]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, uploadPerUserEnable, upLoadLimit);
    return CW_TRUE;
}

CWBool CWWTPBoardConfigurationNASCfg(int radioIdx, int wlanIdx)
{
    int NasIdEnable = CW_FALSE, NasPortEnable = CW_FALSE, NasIpEnable = CW_FALSE;
    char *NasId = NULL;
    unsigned short NasPort = 0;
    unsigned int NasIp=0;

    if (CWWTPBoardGetWlanNasIdEnableCfg(radioIdx, wlanIdx, &NasIdEnable) == CW_FALSE)
    {
        NasIdEnable = CW_FALSE;
    }
    CW_CREATE_STRING_ERR(NasId, 48, return CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL);); // NAS ID MAX LENGTH OF HOSTAPD
    if (NasIdEnable == CW_TRUE)
    {
        if (CWWTPBoardGetWlanNasIdCfg(radioIdx, wlanIdx, &NasId) == CW_FALSE)
        {
            CW_FREE_OBJECT(NasId);
            return CW_FALSE;
        }
    }
    else
    {
        snprintf(NasId, 48, "");
    }
    if (CWWTPBoardSetWlanPortalNasIdCfg(radioIdx, wlanIdx, NasId) == CW_FALSE)
    {
        CW_FREE_OBJECT(NasId);
        return CW_FALSE;
    }
    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] NasID:[%s]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, NasId);
    CW_FREE_OBJECT(NasId);

    if (CWWTPBoardGetWlanNasPortEnableCfg(radioIdx, wlanIdx, &NasPortEnable) == CW_FALSE)
    {
        NasPortEnable = CW_FALSE;
    }
    if (NasPortEnable == CW_TRUE)
    {
        if (CWWTPBoardGetWlanNasPortCfg(radioIdx, wlanIdx, &NasPort) == CW_FALSE)
        {
            return CW_FALSE;
        }
    }
    else
    {
        NasPort = 0;
    }
    if (CWWTPBoardSetWlanPortalNasPortCfg(radioIdx, wlanIdx, NasPort) == CW_FALSE)
    {
        return CW_FALSE;
    }
    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] NasPort:[%u]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, NasPort);

    if (CWWTPBoardGetWlanNasIPEnableCfg(radioIdx, wlanIdx, &NasIpEnable) == CW_FALSE)
    {
        NasIpEnable = CW_FALSE;
    }
    if (NasIpEnable == CW_TRUE)
    {
        if (CWWTPBoardGetWlanNasIPCfg(radioIdx, wlanIdx, &NasIp) == CW_FALSE)
        {
            return CW_FALSE;
        }
    }
    else
    {
        NasIp = 0;
    }
    if (CWWTPBoardSetWlanPortalNasIPCfg(radioIdx, wlanIdx, NasIp) == CW_FALSE)
    {
        return CW_FALSE;
    }
    CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] NasIP:[%u.%u.%u.%u]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, CW_IPV4_PRINT_LIST(NasIp));

    return CW_TRUE;
}

CWBool CWWTPBoardConfigurationCfg()
{
    DependentFlag_t flag = 0;
    int radioIdx, wlanIdx, wlanMaxNum = 0;

#if SUPPORT_MESH_SETTING && SUPPORT_WLAN_5G_2_SETTING
    int mesh_channel, mesh_disabled, mesh_24g_disabled, mesh_5g_disabled, mesh_5g_2_disabled;
#endif

    CWDebugLog("%s %d", __FUNCTION__, __LINE__);

    flag = GetDependentFlag();

    CWDebugLog("%s %d flag:[%u]", __FUNCTION__, __LINE__, flag);

    if (RLF_IS_SET(flag, DPF_TRAFFIC_SHAPING) || RLF_IS_SET(flag, DPF_NAS))
    {
        for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
        {
            if (api_get_integer_option("functionlist.vendorlist.PORTAL_LIMIT_NUM",&wlanMaxNum))
            {
                wlanMaxNum = CWWTPBoardGetMaxRadioWlans(radioIdx);
            }
            for(wlanIdx = 0; wlanIdx < wlanMaxNum; wlanIdx++)
            {
                CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] traffic_shapping configuration:[%u]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, configuration_wlan_traffic_shapping[radioIdx][wlanIdx]);
                CWDebugLog("%s %d radioIdx:[%d] wlanIdx:[%d] NAS configuration:[%u]", __FUNCTION__, __LINE__, CWConvertRadioIdx(radioIdx), wlanIdx, configuration_wlan_nas[radioIdx][wlanIdx]);

                if (RLF_IS_SET(flag, DPF_TRAFFIC_SHAPING))
                {
                    if (configuration_wlan_traffic_shapping[radioIdx][wlanIdx] == CW_TRUE)
                    {
                        CWWTPBoardConfigurationTrafficShapingCfg(radioIdx, wlanIdx);
                    }
                }
                if (RLF_IS_SET(flag, DPF_NAS))
                {
                    if (configuration_wlan_nas[radioIdx][wlanIdx] == CW_TRUE)
                    {
                        CWWTPBoardConfigurationNASCfg(radioIdx, wlanIdx);
                    }
                }
            }
        }
    }

#if 0 //SUPPORT_MESH_SETTING && SUPPORT_WLAN_5G_2_SETTING
    //if (RLF_IS_SET(flag, DPF_MESH))
    {
        api_get_integer_option(MESH_WIFI_DISABLED_OPTION, &mesh_disabled);
        api_get_integer_option(WIRELESS_WIFI_24G_MESH_DISABLED, &mesh_24g_disabled);
        api_get_integer_option(WIRELESS_WIFI_5G_MESH_DISABLED, &mesh_5g_disabled);
        api_get_integer_option(WIRELESS_WIFI_5G_2_MESH_DISABLED, &mesh_5g_2_disabled);
        api_get_integer_option("mesh.wifi.5Gchannel", &mesh_channel);

        if (mesh_disabled == 0 && mesh_24g_disabled == 1)
        {
            if (mesh_channel >= 36 && mesh_channel < 100)
            {
                api_set_integer_option(WIRELESS_WIFI_5G_MESH_DISABLED, 1);
                api_set_integer_option(WIRELESS_WIFI_5G_2_MESH_DISABLED, 0);
            }
            else if (mesh_channel >= 100)
            {
                api_set_integer_option(WIRELESS_WIFI_5G_MESH_DISABLED, 0);
                api_set_integer_option(WIRELESS_WIFI_5G_2_MESH_DISABLED, 1);
            }
        }
        else
        {
            api_set_integer_option(WIRELESS_WIFI_5G_MESH_DISABLED, 1);
            api_set_integer_option(WIRELESS_WIFI_5G_2_MESH_DISABLED, 1);
        }
    }

#endif


    memset(configuration_wlan_traffic_shapping, 0, sizeof(configuration_wlan_traffic_shapping));
    memset(configuration_wlan_nas, 0, sizeof(configuration_wlan_nas));
    return CW_TRUE;
}

CWBool CWWTPBoardApplyCfg()
{
    char *val;
    CWBool dhcpWait = CW_FALSE;
    int radioIdx, wlanIdx;
    int enable;
    int timeout;
    int curWlanCount;
    int enabledWlanCount;
    int portal_type = 0;
    int vlan_enable = 0;
    int portal_vlan = 0;
    ReloadFlag_t flag = 0;
    int wlanMaxNum = 0;

    CWDebugLog("%s %d", __FUNCTION__, __LINE__);

    CWWTPBoardConfigurationCfg();
    
    flag = GetReloadFlag();

    CWDebugLog("%s %d flag:[%u]", __FUNCTION__, __LINE__, flag);

    /* Check the reload is required */

    if(RLF_IS_SET(flag, RLF_NETWORK))
    {
        /* check if we will being in dhcp mode */
        if(!(val = CWCreateStringByUci("network.lan.proto")))
        {
            return CW_FALSE;
        }

        if(!strcmp(val, "dhcp"))
        {
            dhcpWait = CW_TRUE;
        }

        CW_FREE_OBJECT(val);
    }

    if (RLF_IS_SET(flag, RLF_PORTAL) || RLF_IS_SET(flag, RLF_VLANISOLATION))
    {
        for (radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
        {
            if (api_get_integer_option("functionlist.vendorlist.PORTAL_LIMIT_NUM",&wlanMaxNum))
            {
                wlanMaxNum = CWWTPBoardGetMaxRadioWlans(radioIdx);
            }
            for (wlanIdx = 0; wlanIdx < wlanMaxNum; wlanIdx++)
            {
                if (RLF_IS_SET(flag, RLF_PORTAL))
                {
                    portal_type = 0;
                    if (CWWTPBoardGetWlanPortalEnableCfg(radioIdx, wlanIdx, &enable) && (enable == 1))
                    {
                        if (CWWTPBoardGetWlanPortalNetworkTypeCfg(radioIdx, wlanIdx, &portal_type) == CW_FALSE)
                            portal_type = 0;
                    }
                    CWWTPBoardSetWlanGuestNetworkTypeCfg(radioIdx, wlanIdx, portal_type);
                }
                if (RLF_IS_SET(flag, RLF_VLANISOLATION))
                {
                    vlan_enable = CW_FALSE;
                    portal_vlan = 0;
                    if (CWWTPBoardGetWlanIsolationCfg(radioIdx, wlanIdx, &vlan_enable) == CW_FALSE)
                    {
                        vlan_enable = CW_FALSE;
                    }
                    if (vlan_enable == CW_TRUE) 
                    {
                        if (CWWTPBoardGetWlanVlanCfg(radioIdx, wlanIdx, &portal_vlan) == CW_FALSE)
                        {
                            portal_vlan = 0;
                        }
                    }
                    else
                    {
                        portal_vlan = 0;
                    }
                    CWWTPBoardSetWlanPortalVlanTagCfg(radioIdx, wlanIdx, portal_vlan);
                }
            }
        }
    }

    if(RLF_EQ(flag, RLF_LED))
    {
        CWSystem("/etc/init.d/led start");
        CWSystem("uci commit");
    }
#if SUPPORT_MAC_FILTER_NO_RELOAD
    else if(RLF_EQ(flag, RLF_MACFILTER))
    {
        CWSystem("/sbin/setMacFilter.sh %s", GetAclSectionNameList());
        CWSystem("uci commit");
    }
#endif
    else
    {
        CWSystem("/sbin/luci-reload auto");
    }

    enabledWlanCount = 0;
    for(radioIdx = 0; radioIdx < CWWTPBoardGetMaxRadio(); radioIdx++)
    {
        if (CWWTPBoardGetRadioNoChannel(radioIdx) == CW_TRUE)
        {
            continue;
        }
        if (api_get_integer_option("functionlist.vendorlist.PORTAL_LIMIT_NUM",&wlanMaxNum))
        {
            wlanMaxNum = CWWTPBoardGetMaxRadioWlans(radioIdx);
        }
        curWlanCount = 0;
        for(wlanIdx = 0; wlanIdx < wlanMaxNum; wlanIdx++)
        {
            if(CWWTPBoardGetWlanEnableCfg(radioIdx, wlanIdx, &enable) && enable)
            {
                curWlanCount++;
                enabledWlanCount++;
            }
        }
    }

    CWWaitSec(1);

    if (enabledWlanCount > 2)
        timeout = (enabledWlanCount*5); /* wait 5 seconds for loading every one wireless interface */
    else
        timeout = 10; /* wait 10 seconds for loading wireless interface */
    do
    {
        if(!(val = CWCreateStringByCmd("iwconfig 2> /dev/null | grep -e '^ath' -c")))
        {
            return CW_FALSE;
        }

        curWlanCount = atoi(val);



        CW_FREE_OBJECT(val);

        CWLog("Wireless Interfaces Ready (%d/%d)", curWlanCount, enabledWlanCount);

        if(curWlanCount >= enabledWlanCount)
        {
            break;
        }

        CWWaitSec(1); /* wait 1 */

        timeout--;

        /* Sometimes wireless interface cannot be brought up, the workaround is to reboot */
        if(timeout == 0)
        {
            CWLog("Wireless interfaces cannot be brought up, system will reboot");
            CWSystem("reboot");
            return CW_FALSE;
        }
    }
    while(1);

    if(dhcpWait)
    {
        timeout = 10;

        /* wait dhcp ready by polling lease_acquired */
        do
        {
            if (sysutil_dhcp_ip_ready())
            {
                CWDebugLog("DHCP is ready");
                break;
            }

            CWDebugLog("Waiting DHCP ready for %d seconds...", timeout);

            CWWaitSec(1);
        }
        while(timeout--);
    }

    return CW_TRUE;
}

CWBool CWWTPBoardCancelCfg()
{
    char tmp_file[32];
    char uci_str[128];
    char *c;
    FILE *fp;

    CWDebugLog("%s", __FUNCTION__);

    unlink("/tmp/webpasswd");

    sprintf(tmp_file, "/tmp/cancelcfg.%x.tmp", (unsigned int) CWThreadSelf());
    CWSystem("uci changes > %s", tmp_file);

    fp = fopen(tmp_file, "r");
    if(!fp)
    {
        unlink(tmp_file);
        return CWErrorRaise(CW_ERROR_GENERAL, NULL);
    }
    while(fgets(uci_str, 128, fp) != NULL)
    {
        if((c = strchr(uci_str, '=')))
        {
            *c = '\0';
        }
        CWDebugLog("%s uci revert %s", __FUNCTION__, uci_str);
        CWSystem("uci revert %s", uci_str);
    }

    fclose(fp);
    unlink(tmp_file);

    return CW_TRUE;
}
