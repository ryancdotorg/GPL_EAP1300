/****************************************************************************
;
;   (C) Unpublished Work of Senao Networks, Inc.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF SENAO INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) SENAO EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN SENAO WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF SENAO.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;----------------------------------------------------------------------------
;
;    Project : app_agent
;    Creator : Jerry Chen
;    File    : wlan_setting.c
;    Abstract:
;
;       Modification History:
;       By              Date        Ver.    Modification Description
;       --------------- --------    -----   -------------------------------------
;       Jerry           2012/09/10          First commit
;****************************************************************************/

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "wlan_setting.h"
#include "app_agent.h"
#include "product.h"
#include "sysCore.h"
#include "sysUtilMisc.h"
#include "sysWlan.h"
#include <variable/api_wireless.h>
#include <check/wireless_check.h>
#include <variable/api_guest.h>
/*-------------------------------------------------------------------------*/
/*                           DEFINITIONS                                   */
/*-------------------------------------------------------------------------*/
#define ATHEROS_SUPP_CHAN_PATH	"/mnt/chanlist"
/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/
extern T_WLANTokInfo wlan_ra_Tokens[];
/*--------------------------------------------------------------------------*/
/*                             CONSTANT STRING                              */
/*--------------------------------------------------------------------------*/
const char *wlan_mode_str[] =
{
    [0] = WLAN_802_11_BG_STR,
    [1] = WLAN_802_11_B_STR,
    [2] = WLAN_802_11_A_STR,
    [3] = WLAN_802_11_ABG_STR,
    [4] = WLAN_802_11_G_STR,
    [5] = WLAN_802_11_ABGN_STR,
    [6] = WLAN_802_11_N_STR,
    [7] = WLAN_802_11_GN_STR,
    [8] = WLAN_802_11_AN_STR,
    [9] = WLAN_802_11_BGN_STR,
    [10] = WLAN_802_11_AGN_STR,
    [11] = WLAN_802_11_N_STR,
    [13] = WLAN_802_11_AC_STR
};

const int wifi_2g_hwmode_mapping[][2] =
{
    { P24G_IEEE802_11B, WIRELESSMODE_24G_B },
    { P24G_IEEE802_11G, WIRELESSMODE_24G_G },
    { P24G_IEEE802_11N, WIRELESSMODE_24G_N },
    { P24G_IEEE802_11BG, WIRELESSMODE_24G_BG },
#if 0
    { WIRELESSMODE_24G_GN, P24G_IEEE802_11NG },
#endif
    { P24G_IEEE802_11NG, WIRELESSMODE_24G_BGN }
};

#if SUPPORT_WLAN_5G_SETTING
const int wifi_5g_hwmode_mapping[][2] =
{
    { P5G_IEEE802_11A, WIRELESSMODE_5G_A },
    { P5G_IEEE802_11N, WIRELESSMODE_5G_N },
    { P5G_IEEE802_11NA, WIRELESSMODE_5G_AN },
    { P5G_IEEE802_11AC, WIRELESSMODE_5G_AC }
};
#endif

const int wifi_security_mapping[][3] =
{
    { ENCRYPTION_NONE, WLAN_AUTH_OPEN, WLAN_ENC_NONE },
    { WEP_OPEN, WLAN_AUTH_OPEN, WLAN_ENC_WEP },
    { WEP_SHARED, WLAN_AUTH_SHARED, WLAN_ENC_WEP },

    { WPA_PSK_CCMP, WLAN_AUTH_WPAPSK, WLAN_ENC_AES },
    { WPA_PSK_TKIP, WLAN_AUTH_WPAPSK, WLAN_ENC_TKIP },
    { WPA_PSK_TKIP_CCMP, WLAN_AUTH_WPAPSK, WLAN_ENC_TKIPAES },

    { WPA2_PSK_CCMP, WLAN_AUTH_WPA2PSK, WLAN_ENC_AES },
    { WPA2_PSK_TKIP, WLAN_AUTH_WPA2PSK, WLAN_ENC_TKIP },
    { WPA2_PSK_TKIP_CCMP, WLAN_AUTH_WPA2PSK, WLAN_ENC_TKIPAES },

    { WPA_PSK_MIXED_CCMP, WLAN_AUTH_WPA1PSKWPA2PSK, WLAN_ENC_AES },
    { WPA_PSK_MIXED_TKIP, WLAN_AUTH_WPA1PSKWPA2PSK, WLAN_ENC_TKIP },
    { WPA_PSK_MIXED_TKIP_CCMP, WLAN_AUTH_WPA1PSKWPA2PSK, WLAN_ENC_TKIPAES },

    { WPA_EAP_CCMP, WLAN_AUTH_WPA, WLAN_ENC_AES },
    { WPA_EAP_TKIP, WLAN_AUTH_WPA, WLAN_ENC_TKIP },
    { WPA_EAP_TKIP_CCMP, WLAN_AUTH_WPA, WLAN_ENC_TKIPAES },

    { WPA2_EAP_CCMP, WLAN_AUTH_WPA2, WLAN_ENC_AES },
    { WPA2_EAP_TKIP, WLAN_AUTH_WPA2, WLAN_ENC_TKIP },
    { WPA2_EAP_TKIP_CCMP, WLAN_AUTH_WPA2, WLAN_ENC_TKIPAES },

    { WPA_EAP_MIXED_CCMP, WLAN_AUTH_WPA1WPA2, WLAN_ENC_AES },
    { WPA_EAP_MIXED_TKIP, WLAN_AUTH_WPA1WPA2, WLAN_ENC_TKIP },
    { WPA_EAP_MIXED_TKIP_CCMP, WLAN_AUTH_WPA1WPA2, WLAN_ENC_TKIPAES },

    { ENCRYPTION_END, WLAN_AUTH_WEPAUTO, WLAN_ENC_WEP }, //todo
};

const int wifi_guest_security_mapping[][3] =
{
    { GUEST_ENCRYPTION_NONE, WLAN_AUTH_OPEN, WLAN_ENC_NONE },

    { GUEST_WPA_PSK_CCMP, WLAN_AUTH_WPAPSK, WLAN_ENC_AES },
    { GUEST_WPA_PSK_TKIP, WLAN_AUTH_WPAPSK, WLAN_ENC_TKIP },
    { GUEST_WPA_PSK_TKIP_CCMP, WLAN_AUTH_WPAPSK, WLAN_ENC_TKIPAES },

    { GUEST_WPA2_PSK_CCMP, WLAN_AUTH_WPA2PSK, WLAN_ENC_AES },
    { GUEST_WPA2_PSK_TKIP, WLAN_AUTH_WPA2PSK, WLAN_ENC_TKIP },
    { GUEST_WPA2_PSK_TKIP_CCMP, WLAN_AUTH_WPA2PSK, WLAN_ENC_TKIPAES },

    { GUEST_WPA_PSK_MIXED_CCMP, WLAN_AUTH_WPA1PSKWPA2PSK, WLAN_ENC_AES },
    { GUEST_WPA_PSK_MIXED_TKIP, WLAN_AUTH_WPA1PSKWPA2PSK, WLAN_ENC_TKIP },
    { GUEST_WPA_PSK_MIXED_TKIP_CCMP, WLAN_AUTH_WPA1PSKWPA2PSK, WLAN_ENC_TKIPAES },

    { GUEST_ENCRYPTION_END, WLAN_AUTH_WEPAUTO, WLAN_ENC_WEP } //todo
};

WLAN_RADIO_MODE wifi_2g_mode[] =
{
    { P24G_IEEE802_11B, WLAN_802_11_B_STR },
    { P24G_IEEE802_11G, WLAN_802_11_G_STR },
    { P24G_IEEE802_11N, WLAN_802_11_N_STR },
    { P24G_IEEE802_11BG, WLAN_802_11_BG_STR },
#if 0                                                  
    { WIRELESSMODE_24G_GN, WLAN_802_11_GN_STR },
#endif                                                 
    { P24G_IEEE802_11NG, WLAN_802_11_BGN_STR }
};

#if SUPPORT_WLAN_5G_SETTING
WLAN_RADIO_MODE wifi_5g_mode[] =
{
    { P5G_IEEE802_11A, WLAN_802_11_A_STR },
    { P5G_IEEE802_11N, WLAN_802_11_N_STR },
    { P5G_IEEE802_11NA, WLAN_802_11_AN_STR },
    { P5G_IEEE802_11AC, WLAN_802_11_AC_STR }
};
#endif

SECURITY_TYPE security_type[] =
{
    { WLAN_WEP_OPEN_STR,            WLAN_AUTH_OPEN                  },
    { WLAN_WEP_AUTO_STR,            WLAN_AUTH_WEPAUTO               },
    { WLAN_WEP_SHARED_STR,          WLAN_AUTH_SHARED                },
    { WLAN_WPA_PSK_STR,             WLAN_AUTH_WPAPSK                },
    { WLAN_WPA_RADIUS_STR,          WLAN_AUTH_WPA                   },
    { WLAN_WPA2_PSK_STR,            WLAN_AUTH_WPA2PSK               },
    { WLAN_WPA2_RADIUS_STR,         WLAN_AUTH_WPA2                  },
    { WLAN_WPAORWPA2_RADIUS_STR,    WLAN_AUTH_WPA1WPA2              },
    { WLAN_WPAORWPA2_PSK_STR,       WLAN_AUTH_WPA1PSKWPA2PSK        }
};

// WPAORWPA2-PSK
ENCRYPTION_TYPE encryption_type[] =
{
    { WLAN_WEP_64_STR,              WLAN_ENC_WEP,                   64},
    { WLAN_WEP_128_STR,             WLAN_ENC_WEP,                   128},
    { WLAN_CIPHER_TKIP_STR,         WLAN_ENC_TKIP,                  0},
    { WLAN_CIPHER_AES_STR,          WLAN_ENC_AES,                   0},
    { WLAN_CIPHER_TKIPORAES_STR,    WLAN_ENC_TKIPAES,               0}
};

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
/*****************************************************************
* NAME:    reolad_module
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int reolad_module()
{
    SYSTEM("sysconf_cli applychanges checkAllModules &");
}

/*****************************************************************
* NAME:    get_radio_id
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_radio_id(HTTP_PACKET_CONTENT_FORMAT format, char *query_str)
{
    char *radio_id;
    int is5g;

    radio_id = NULL;
    is5g = -1;

    if(JSON_FORMAT == format)
        is5g = get_json_radio_id(query_str);

    return is5g;
}

/*****************************************************************
* NAME:    get_ssid_id
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_ssid_id(HTTP_PACKET_CONTENT_FORMAT format, char *query_str)
{
    int ssid_id;

    ssid_id = -1;

    if(JSON_FORMAT == format)
        ssid_id = get_json_ssid_id(query_str);

    return ssid_id;
}

/*****************************************************************
* NAME: get_wlan_authentication
* ---------------------------------------------------------------
* FUNCTION: Get corresponding authentication.
* INPUT:
* OUTPUT:
* Author:
* Modify:
****************************************************************/
int get_wlan_authentication(char *auth_type_str, int *authentication)
{
    int i = 0;

    for(i = 0; i < T_NUM_OF_ELEMENTS(security_type); i++)
    {
        if((strcmp(security_type[i].security_str, auth_type_str) == 0))
        {
            *authentication = security_type[i].security_mode;
            break;
        }
    }

    return (i >= T_NUM_OF_ELEMENTS(security_type))?false:true;
}

/*****************************************************************
* NAME: get_wlan_encryption
* ---------------------------------------------------------------
* FUNCTION: Get corresponding encryption.
* INPUT:
* OUTPUT:
* Author:
* Modify:
****************************************************************/
int get_wlan_encryption(char *encryption_type_str, int *encryption)
{
    int i = 0;

    for(i = 0; i < T_NUM_OF_ELEMENTS(encryption_type); i++)
    {
        if((strcmp(encryption_type[i].encryption_str, encryption_type_str) == 0))
        {
            *encryption = encryption_type[i].encryption_mode;
            break;
        }
    }

    return (i >= T_NUM_OF_ELEMENTS(encryption_type))?false:true;
}

/*****************************************************************
* NAME: is_valid_wlan_security
* ---------------------------------------------------------------
* FUNCTION: Check if encryption and authentication are set correctly.
* INPUT:
* OUTPUT:
* Author:
* Modify:
****************************************************************/
int is_valid_wlan_security(int encryption, int authentication)
{
    bool result = false;

    switch(encryption)
    {
        case WLAN_ENC_WEP:
            if((authentication == WLAN_AUTH_OPEN) || (authentication == WLAN_AUTH_WEPAUTO) || (authentication == WLAN_AUTH_SHARED))
            {
                result = true;
            }
            break;
        case WLAN_ENC_TKIP:
            if((authentication == WLAN_AUTH_WPA) || (authentication == WLAN_AUTH_WPAPSK))
            {
                result = true;
            }
            break;
        case WLAN_ENC_AES:
            if((authentication == WLAN_AUTH_WPA2) || (authentication == WLAN_AUTH_WPA2PSK))
            {
                result = true;
            }
            break;
        case WLAN_ENC_TKIPAES:
            if((authentication == WLAN_AUTH_WPA1WPA2) || (authentication == WLAN_AUTH_WPA1PSKWPA2PSK))
            {
                result = true;
            }
            break;
        default:
            break;
    }

    return result;
}

/*****************************************************************
* NAME:    get_hwmode_convert
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_hwmode_convert(int wifix, int hwMode, int *newHwMode)
{
    int j=0;

#if SUPPORT_WLAN_5G_SETTING
    if (wifix) 
    {
        for (j=0; j<T_NUM_OF_ELEMENTS(wifi_5g_hwmode_mapping); j++) 
        {
            if (hwMode==wifi_5g_hwmode_mapping[j][0]) 
            {
                *newHwMode=wifi_5g_hwmode_mapping[j][1];
                return 0;
            }
        }
    }
    else 
#endif
    {
        for (j=0; j<T_NUM_OF_ELEMENTS(wifi_2g_hwmode_mapping); j++) 
        {
            if (hwMode==wifi_2g_hwmode_mapping[j][0]) 
            {
                *newHwMode=wifi_2g_hwmode_mapping[j][1];
                return 0;
            }
        }
    }
    return -1;
}

/*****************************************************************
* NAME:    get_security_convert
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_security_convert(int security, int *encryption, int *authentication)
{
    int j=0;

    for (j=0; j<T_NUM_OF_ELEMENTS(wifi_security_mapping); j++)
    {
        if (security==wifi_security_mapping[j][0]) 
        {
            *authentication=wifi_security_mapping[j][1];
            *encryption=wifi_security_mapping[j][2];
            return 0;
        }
    }
    return -1;
}

/*****************************************************************
* NAME:    get_guest_security_convert
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_guest_security_convert(int security, int *encryption, int *authentication)
{
    int j=0;

    for (j=0; j<T_NUM_OF_ELEMENTS(wifi_security_mapping); j++)
    {
        if (security==wifi_guest_security_mapping[j][0]) 
        {
            *authentication=wifi_guest_security_mapping[j][1];
            *encryption=wifi_guest_security_mapping[j][2];
            return 0;
        }
    }
    return -1;
}

/*****************************************************************
* NAME:    get_hwmode_id
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_wifi_hwmode_id(int wifix, char *hwModeStr, int *hwModeInt)
{
    int j=0;

#if SUPPORT_WLAN_5G_SETTING
    if (wifix) 
    {
        for (j = 0; j < T_NUM_OF_ELEMENTS(wifi_5g_mode); j++) 
        {
            if(!strcmp(wifi_5g_mode[j].modeName, hwModeStr))
            {
                *hwModeInt = wifi_5g_mode[j].modeId;
                return 0;
            }
        }

    }
    else
#endif
    {
        for (j = 0; j < T_NUM_OF_ELEMENTS(wifi_2g_mode); j++) 
        {
            if(!strcmp(wifi_2g_mode[j].modeName, hwModeStr))
            {
                *hwModeInt = wifi_2g_mode[j].modeId;
                return 0;
            }
        }
    }
    return -1;
}

/*****************************************************************
* NAME: get_wifi_security
* ---------------------------------------------------------------
* FUNCTION: Get corresponding encryption.
* INPUT:
* OUTPUT:
* Author:
* Modify:
****************************************************************/
int get_wifi_security(int encryption, int authentication, int *security)
{
    int j=0;

    for (j=0; j<T_NUM_OF_ELEMENTS(wifi_security_mapping); j++)
    {
        if (encryption==wifi_security_mapping[j][2]) 
        {
            if (authentication==wifi_security_mapping[j][1]) 
            {
                *security = wifi_security_mapping[j][0];
                return 0;
            }
            
        }
    }
    return -1;
}

/*****************************************************************
* NAME: get_wifi_guest_security
* ---------------------------------------------------------------
* FUNCTION: Get corresponding encryption.
* INPUT:
* OUTPUT:
* Author:
* Modify:
****************************************************************/
int get_wifi_guest_security(int encryption, int authentication, int *security)
{
    int j=0;

    for (j=0; j<T_NUM_OF_ELEMENTS(wifi_guest_security_mapping); j++)
    {
        if (encryption==wifi_guest_security_mapping[j][2]) 
        {
            if (authentication==wifi_guest_security_mapping[j][1]) 
            {
                *security = wifi_guest_security_mapping[j][0];
                return 0;
            }
            
        }
    }
    return -1;
}

/*****************************************************************
* NAME: get_wlan_wep_key_length
* ---------------------------------------------------------------
* FUNCTION: Get corresponding encryption.
* INPUT:
* OUTPUT:
* Author:
* Modify:
****************************************************************/
int get_wlan_wep_key_length(char *encryption_type_str)
{
    int i = 0;

    for(i = 0; i < 2; i++)
    {
        if((strcmp(encryption_type[i].encryption_str, encryption_type_str) == 0))
        {
            return encryption_type[i].wep_key_length;
        }
    }

    return 0;
}

/*****************************************************************
* NAME: get_wlan_wep_key_type
* ---------------------------------------------------------------
* FUNCTION: get wep key type
* INPUT:
* OUTPUT:
* Author:
* Modify:
****************************************************************/
int get_wlan_wep_key_type(int wep, int length)
{
    return ((wep==64)&&(length==5)||(wep==128)&&(length==13))?1:0;
}

/*****************************************************************
* NAME:    get_wlan_radios_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_wlan_radios_cb(HTTPS_CB *pkt){

    if(NULL == pkt)
        return -1;

    if(pkt->json)
        get_wlan_radios_json_cb(pkt);

    return 0;
}

/*****************************************************************
* NAME:    get_wlan_settings
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_wlan_settings(HTTPS_CB *pkt, WLAN_RADIO_SETTINGS_T *settings, char **result){

    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    int guest_flag=0, index=0, disabled=0, hwMode=0, newHwMode=0, channel=0, htMode=0, hidden=0;
    char macAddr[17+1], ssid[31+1];

    HTTP_PACKET_CONTENT_FORMAT pkt_format;

    if(NULL == pkt || 0 == strlen(query_str))
    {
        *result = ERROR_STR;
        return -1;
    }

    if(pkt->json)
        pkt_format = JSON_FORMAT;

    /* Extra setting from packet. */
    settings->radio_id = get_radio_id(pkt_format, query_str);
    settings->ssid_id = get_ssid_id(pkt_format, query_str);

    if(-1 == settings->radio_id)
    {
        *result = ERROR_BAD_RADIOID_STR;
        return -1;
    }

#if SUPPORT_WLAN_5G_SETTING
    // 2.4GHZ SSID==1: main   SSID==2: GUEST     
    // 5GHZ   SSID==3: main   SSID==4: GUEST
#if APP_AGENT_SUPPORT_ENSHARE
#else
    if(settings->radio_id == 1)
        settings->ssid_id = settings->ssid_id - (WIFI_SSID_NUM+1);
#endif
#endif
    if (settings->ssid_id < 0 || settings->ssid_id > WIFI_SSID_NUM)
    {
        *result = ERROR_BAD_SSIDID_STR;
        return -1;
    }

    guest_flag = (settings->ssid_id == WIFI_SSID_NUM)?1:0;

    memset(ssid, 0, sizeof(ssid));
    memset(macAddr, 0, sizeof(macAddr));

    if (settings->radio_id == 0) 
    {
        api_get_wifi_hwmode_option(WIRELESS_WIFI_HWMODE_OPTION, &hwMode);
        get_hwmode_convert(0, hwMode, &newHwMode);
        sysutil_get_wifixMacAddr(0, macAddr, sizeof(macAddr));
        //api_get_wifi_channel_option(WIRELESS_WIFI_CHANNEL_OPTION, &channel);
        channel = sysutil_get_wifixChannel(WIFI_24G_IF);
        api_get_wifi_htmode_option(WIRELESS_WIFI_HTMODE_OPTION, &htMode);
    }
#if SUPPORT_WLAN_5G_SETTING
    else if (settings->radio_id == 1) 
    {
        api_get_wifi_5g_hwmode_option(WIRELESS_WIFI_5G_HWMODE_OPTION, &hwMode);
        get_hwmode_convert(1, hwMode, &newHwMode);
        sysutil_get_wifixMacAddr(1, macAddr, sizeof(macAddr));
        channel = sysutil_get_wifixChannel(WIFI_5G_IF);
        //api_get_wifi_5g_channel_option(WIRELESS_WIFI_5G_CHANNEL_OPTION, &channel);
        api_get_wifi_5g_htmode_option(WIRELESS_WIFI_5G_HTMODE_OPTION, &htMode);
    }
#endif
    if (guest_flag) 
    {
#if SUPPORT_WLAN_5G_SETTING
        if (1 == settings->radio_id)
        {
            api_get_wifi_guest_disabled_option(WIRELESS_WIFI_5G_GUEST_DISABLED_OPTION, &disabled);
            api_get_wifi_guest_ssid_option(WIRELESS_WIFI_5G_GUEST_SSID_OPTION, ssid, sizeof(ssid));
            api_get_wifi_guest_hidden_option(WIRELESS_WIFI_5G_GUEST_HIDDEN_OPTION, &hidden);
        }
        else
#endif
        {
            api_get_wifi_guest_disabled_option(WIRELESS_WIFI_GUEST_DISABLED_OPTION, &disabled);
            api_get_wifi_guest_ssid_option(WIRELESS_WIFI_GUEST_SSID_OPTION, ssid, sizeof(ssid));
            api_get_wifi_guest_hidden_option(WIRELESS_WIFI_GUEST_HIDDEN_OPTION, &hidden);
        }
    }
    else 
    {
        index = (settings->radio_id == 1)?(settings->ssid_id+WIFI_SSID_NUM):settings->ssid_id;
        api_get_wifi_ifacex_disabled_option(index, &disabled);
        api_get_wifi_ifacex_ssid_option(index, ssid, sizeof(ssid));
        api_get_wifi_ifacex_hidden_option(index, &hidden);
    }
    settings->enabled = disabled?false:true;
    sprintf(settings->mode,"%s",wlan_mode_str[newHwMode]);
    sprintf(settings->mac,"%s",macAddr);
    sprintf(settings->ssid,"%s",ssid);
    settings->ssid_broadcast = hidden?false:true;
    settings->channel_width = (htMode==BANDWIDTH_80MHZ)?80:(htMode==BANDWIDTH_20MHZ_40MHZ)?20:(htMode==BANDWIDTH_40MHZ)?40:20;
    settings->channel = channel;

    return 0;
}
#if HAS_ENGENIUS && SUPPORT_CONFIG_HAS_SECTIONNAME
/*****************************************************************
* NAME:    get_wlan_options
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_wlan_options(WLAN_RADIO_SETTINGS_T *settings, char **result)
{
	// Enabled : boolean,
	// SSID : string,
	// SSIDBroadcast : boolean,
	// Channel : integer,
	// ChannelWidth : integer,
	// MacAddress : string,	

	char ifname[6];
	char buf_mac[32];
	char channel[8];
	char *ptr;
	int disable=0;
	int htmode,hidden;
	
	memset(ifname,0x0,sizeof(ifname));
	memset(buf_mac,0x0,sizeof(buf_mac));
	memset(channel,0x0,sizeof(channel));

    if(0 == settings->radio_id)
    {
		api_get_wifi_disabled_option(WIRELESS_WIFI0_SSID_1_DISABLED_OPTION,&disable);
		(disable == 1)?(settings->enabled=0):(settings->enabled=1);
		api_get_wifi_iface_ssid_option(WIRELESS_WIFI0_SSID_1_SSID_OPTION,settings->ssid, sizeof(settings->ssid));
		api_get_wifi_iface_hidden_option(WIRELESS_WIFI0_SSID_1_HIDDEN_OPTION, &hidden);
		settings->ssid_broadcast = (0 == hidden)?true:false;
		api_get_string_option(WIRELESS_WIFI_CHANNEL_OPTION, channel, sizeof(channel));
		settings->channel = (0 == strcmp("auto", channel))?0:atoi(channel);
		api_get_wifi_htmode_option(WIRELESS_WIFI_HTMODE_OPTION, &htmode);
		settings->channel_width = (0 == htmode)?20:(1 == htmode)?40:/* (2 == htmode) */0;
		api_get_string_option(WIRELESS_WIFI0_SSID_1_IFNAME_OPTION,ifname,sizeof(ifname));
		sysutil_interact(buf_mac, sizeof(buf_mac), "ifconfig %s | grep \"HWaddr\" | awk \'{print $5}\'",ifname);
		if(NULL != (ptr = strchr(buf_mac, '\n')))
            		*ptr = '\0';
		sprintf(settings->mac,"%s",buf_mac);
    }
#if SUPPORT_WLAN_5G_SETTING
    else if(1 == settings->radio_id)
    {
		api_get_wifi_disabled_option(WIRELESS_WIFI1_SSID_1_DISABLED_OPTION,&disable);
		(disable == 1)?(settings->enabled=0):(settings->enabled=1);
		api_get_wifi_iface_ssid_option(WIRELESS_WIFI1_SSID_1_SSID_OPTION,settings->ssid, sizeof(settings->ssid));
		api_get_wifi_iface_hidden_option(WIRELESS_WIFI1_SSID_1_HIDDEN_OPTION, &hidden);
		settings->ssid_broadcast = (0 == hidden)?true:false;
		api_get_string_option(WIRELESS_WIFI_5G_CHANNEL_OPTION,channel, sizeof(channel));
		settings->channel = (0 == strcmp("auto", channel))?0:atoi(channel);
		api_get_wifi_5g_htmode_option(WIRELESS_WIFI_5G_HTMODE_OPTION, &htmode);
		settings->channel_width = (0 == htmode)?20:(1 == htmode)?40:(2 == htmode)?0:/* (3 == htmode) */80;
		api_get_string_option(WIRELESS_WIFI1_SSID_1_IFNAME_OPTION,ifname,sizeof(ifname));
		sysutil_interact(buf_mac, sizeof(buf_mac), "ifconfig %s | grep \"HWaddr\" | awk \'{print $5}\'",ifname);
		if(NULL != (ptr = strchr(buf_mac, '\n')))
            		*ptr = '\0';
		sprintf(settings->mac,"%s",buf_mac);
    }
#endif
    return 0;
}
#endif

/*****************************************************************
* NAME:    get_wlan_radio_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_wlan_radio_settings_cb(HTTPS_CB *pkt){

    char *result;
    WLAN_RADIO_SETTINGS_T settings;

    if(NULL == pkt)
        return -1;

    result = NULL;

    memset(&settings, 0, sizeof(settings));

    get_wlan_settings(pkt, &settings, &result);

    if(NULL != result)
    {
        send_simple_response(pkt, result);
    }
    else
    {
        result = OK_STR;

        if(pkt->json)
            get_wlan_radio_settings_json_cb(pkt, &settings, result);
    }

    return 0;
}
/*****************************************************************
* NAME:    set_wlan_radio_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_wlan_radio_settings_cb(HTTPS_CB *pkt){

    /*******************************************************************
     * SetWLanRadioSettingsResult type:                                *
     * OK, ERROR, REBOOT, ERROR_BAD_RADIOID, ERROR_BAD_SSIDID,         *
     * ERROR_BAD_CHANNEL, ERROR_BAD_CHANNEL_WIDTH, ERROR_BAD_MODE      *
     *******************************************************************/
   bool result = TRUE;
    int ApiResult = 0;
    int guest_flag = 0; 
    int index = 0, hwmode = 0, security = 0, encryption = 0, authentication = 0, wep_key_type = -1, wep_key_length = 0;
    int isReboot = 0;
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char ssid[31+1];
    WLAN_RADIOS_T settings;
    char *return_str;
    HTTP_PACKET_CONTENT_FORMAT pkt_format;

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if(NULL == pkt || 0 == strlen(query_str))
    {
        goto send_pkt;
    }

    memset(&settings, 0, sizeof(settings));
    memset(ssid, 0, sizeof(ssid));

    if(pkt->json)
    {
        pkt_format = JSON_FORMAT;
        result &= parse_json_wlan_radio_settings(query_str, &settings, return_str);
    }

    if(TRUE != result) goto send_pkt;

    /* 1. check value */
    if(settings.radio_id == -1){
        return_str = ERROR_BAD_RADIOID_STR;
        goto send_pkt;
    }

    if(settings.ssid_id == -1){
        return_str = ERROR_BAD_SSIDID_STR;
        goto send_pkt;
    }

    guest_flag = (settings.ssid_id == WIFI_SSID_NUM)?1:0; 

    if (guest_flag == 1) 
    {
#if SUPPORT_WLAN_5G_SETTING
        if (1 == settings.radio_id)
        {
            api_set_wifi_guest_disabled_option(WIRELESS_WIFI_5G_GUEST_DISABLED_OPTION, (settings.radio_enabled==true)?0:1);
            api_set_integer_option(DHCP_GUEST2_IGNORE_OPTION, (settings.radio_enabled==true)?0:1);
        }
        else
#endif
        {
            api_set_wifi_guest_disabled_option(WIRELESS_WIFI_GUEST_DISABLED_OPTION, (settings.radio_enabled==true)?0:1);
            api_set_integer_option(DHCP_GUEST1_IGNORE_OPTION, (settings.radio_enabled==true)?0:1);
        }
        if (settings.radio_enabled==false) {
            // isReboot = 1;
            goto reload_setting;
        }
#if SUPPORT_WLAN_5G_SETTING
        if (1 == settings.radio_id)
        {
            ApiResult = api_set_wifi_guest_ssid_option(WIRELESS_WIFI_5G_GUEST_SSID_OPTION, settings.ssid, sizeof(settings.ssid));
        }
        else
#endif
        {
            ApiResult = api_set_wifi_guest_ssid_option(WIRELESS_WIFI_GUEST_SSID_OPTION, settings.ssid, sizeof(settings.ssid));
        }
        if (ApiResult)
        {
            return_str = ERROR_BAD_SSID_STR;
            goto send_pkt;
        }
#if SUPPORT_WLAN_5G_SETTING
        if (1 == settings.radio_id)
        {
            api_set_wifi_guest_hidden_option(WIRELESS_WIFI_5G_GUEST_HIDDEN_OPTION, (settings.ssid_broadcast==true)?0:1);
        }
        else
#endif
        {
            api_set_wifi_guest_hidden_option(WIRELESS_WIFI_GUEST_HIDDEN_OPTION, (settings.ssid_broadcast==true)?0:1);
        }
    }
    else 
    {
#if SUPPORT_WLAN_5G_SETTING
        if (1 == settings.radio_id)
        {
            api_set_wifi_disabled_option(WIRELESS_WIFI_5G_DISABLED_OPTION, (settings.radio_enabled==true)?0:1);
        }
        else
#endif
        {
            api_set_wifi_disabled_option(WIRELESS_WIFI_DISABLED_OPTION, (settings.radio_enabled==true)?0:1);
        }
        index = (settings.radio_id == 1)?(settings.ssid_id+WIFI_SSID_NUM):settings.ssid_id;
        api_set_wifi_ifacex_disabled_option(index, (settings.radio_enabled==true)?0:1);
        if (settings.radio_enabled==false) {
            // isReboot = 1;
            goto reload_setting;
        }
        if (api_set_wifi_ifacex_ssid_option(index, settings.ssid, sizeof(settings.ssid)))
        {
            return_str = ERROR_BAD_SSID_STR;
            goto send_pkt;
        }
        api_set_wifi_ifacex_hidden_option(index, (settings.ssid_broadcast == true) ? 0 : 1); 
    }
#if APP_AGENT_SUPPORT_ENSHARE
        switch(settings.channel_width)
        {
            case 20:
                settings.channel_width = 0;
                break;
            case 40:
                settings.channel_width = 1;
                break;
            case 60:
                settings.channel_width = 2;
                break;
            case 80:
                settings.channel_width = 3;
                break;
        }
#endif
#if SUPPORT_WLAN_5G_SETTING
    if(settings.radio_id == 1)
    {
        get_wifi_hwmode_id(1, settings.mode, &hwmode);
        if (api_set_wifi_5g_hwmode_option(WIRELESS_WIFI_5G_HWMODE_OPTION, hwmode))
        {
            return_str = ERROR_BAD_MODE_STR;
            goto send_pkt;
        }
        settings.channel_width = (settings.channel_width==2)?BANDWIDTH_80MHZ:settings.channel_width;
        if (api_set_wifi_5g_htmode_option(WIRELESS_WIFI_5G_HTMODE_OPTION, settings.channel_width))
        {
            goto send_pkt;
        }
        if (api_set_wifi_5g_channel_option(WIRELESS_WIFI_5G_CHANNEL_OPTION, settings.channel))
        {
            return_str = ERROR_BAD_CHANNEL_STR;
            goto send_pkt;
        }
    }
    else
#endif
    {
        get_wifi_hwmode_id(0, settings.mode, &hwmode);
        if (api_set_wifi_hwmode_option(WIRELESS_WIFI_HWMODE_OPTION, hwmode))
        {
            return_str = ERROR_BAD_MODE_STR;
            goto send_pkt;
        }

        if (api_set_wifi_htmode_option(WIRELESS_WIFI_HTMODE_OPTION, settings.channel_width))
        {
            goto send_pkt;
        }
        if (api_set_wifi_channel_option(WIRELESS_WIFI_CHANNEL_OPTION, settings.channel))
        {
            return_str = ERROR_BAD_CHANNEL_STR;
            goto send_pkt;
        }
    }


    if (!guest_flag) 
    {
        memset(ssid, 0, sizeof(ssid));
        api_get_wifi_ifacex_ssid_option(index, ssid, sizeof(ssid));
        if ((settings.authentication_enabled == true) || strcmp(ssid, settings.ssid)) // If the wifi setting is changed, then wps status must be set as "Configurated".
            api_set_wifi_ifacex_wps_status_option(index, 2);
    }
    if(TRUE == result)
    {
        return_str = OK_STR;
    }

reload_setting:
    if(result)
    {
        if(isReboot){
            APPAGENT_SYSTEM("uci commit");
            APPAGENT_SYSTEM("reboot &");
            return_str = REBOOT_STR;
        }
        else{
            APPAGENT_SYSTEM("uci commit");
            APPAGENT_SYSTEM("/etc/init.d/network reload &");
            APPAGENT_SYSTEM("/etc/init.d/led reload &");
        }
    }

send_pkt:
    send_simple_response(pkt, return_str);

    return 0;
}
/*****************************************************************
* NAME:    get_wlan_security
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_wlan_security(HTTPS_CB *pkt, WLAN_RADIO_SECURITY_T *settings, char **result)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");

    HTTP_PACKET_CONTENT_FORMAT pkt_format;

    int index=0, guest_flag=0, security=0, encrypton=0, authentication=0, wep_key_length=0, wep_key_id=0, wpa_group_rekey=0, auth_server_port=0;
    char wep_key_string[255+1], wpa_key_string[127+1], auth_server_ipaddr[15+1], auth_server_secret[127+1];

    if(NULL == pkt || 0 == strlen(query_str))
    {
        *result = ERROR_STR;
        return -1;
    }

    if(pkt->json)
        pkt_format = JSON_FORMAT;

    /* Extra setting from packet. */
    settings->radio_id = get_radio_id(pkt_format, query_str);
    settings->ssid_id = get_ssid_id(pkt_format, query_str);

    if(-1 == settings->radio_id)
    {
        *result = ERROR_BAD_RADIOID_STR;
        return -1;
    }
#if SUPPORT_WLAN_5G_SETTING
    // 2.4GHZ SSID==1: main   SSID==2: GUEST     
    // 5GHZ   SSID==3: main   SSID==4: GUEST
#if APP_AGENT_SUPPORT_ENSHARE
#else
    if(settings->radio_id == 1) 
        settings->ssid_id = settings->ssid_id - (WIFI_SSID_NUM+1);
#endif
#endif
    if(settings->ssid_id < 0 || settings->ssid_id > WIFI_SSID_NUM)
    {
        *result = ERROR_BAD_SSIDID_STR;
        return -1;
    }

    guest_flag = (settings->ssid_id == WIFI_SSID_NUM)?1:0;

    if (guest_flag) 
    {
#if SUPPORT_WLAN_5G_SETTING
        if (1 == settings->radio_id)
        {
            api_get_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTIONFAKE_OPTION, &security);
        }
        else
#endif
        {
            api_get_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTIONFAKE_OPTION, &security);
        }
        get_guest_security_convert(security, &encrypton, &authentication);
    }
    else 
    {
        index = (settings->radio_id == 1)?(settings->ssid_id+WIFI_SSID_NUM):settings->ssid_id;
        api_get_wifi_ifacex_encryption_option(index, &security);
        get_security_convert(security, &encrypton, &authentication);
    } 

    settings->enabled = (security > 0)?true:false;
    settings->typeIdx = authentication;
    settings->enable_8021x = ((authentication==WLAN_AUTH_WPA)||
                              (authentication==WLAN_AUTH_WPA2)||
                              (authentication==WLAN_AUTH_WPA1WPA2))?true:false;

    if(security > 0) // If wireless security is enabled
    {
        switch(encrypton)
        {
            case WLAN_ENC_WEP:
                api_get_wifi_ifacex_wepkey_length_option(index, &wep_key_length);
                api_get_wifi_ifacex_wepkey_id_option(index, &wep_key_id);
                api_get_wifi_ifacex_wepkey_keyx_option(index, wep_key_id, wep_key_string, sizeof(wep_key_string));
                sprintf(settings->key, "%s", wep_key_string);
                sprintf(settings->encryption, "%s", (64 == wep_key_length)?encryption_type[0].encryption_str:encryption_type[1].encryption_str);
                break;
            case WLAN_ENC_TKIP:
                sprintf(settings->encryption, "%s", encryption_type[2].encryption_str);
                if(guest_flag) 
                {
#if SUPPORT_WLAN_5G_SETTING
                    if (1 == settings->radio_id)
                    {
                        api_get_wifi_guest_wpa_group_rekey_option(WIRELESS_WIFI_5G_GUEST_WPA_GROUP_REKEY_OPTION, &wpa_group_rekey);
                    }
                    else
#endif
                    {
                        api_get_wifi_guest_wpa_group_rekey_option(WIRELESS_WIFI_GUEST_WPA_GROUP_REKEY_OPTION, &wpa_group_rekey);
                    }
                }
                else 
                    api_get_wifi_ifacex_wpa_group_rekey_option(index, &wpa_group_rekey);
                settings->key_renewal = wpa_group_rekey;
                break;
            case WLAN_ENC_AES:
                sprintf(settings->encryption, "%s", encryption_type[3].encryption_str);
                if(guest_flag) 
                {
#if SUPPORT_WLAN_5G_SETTING
                    if (1 == settings->radio_id)
                    {
                        api_get_wifi_guest_wpa_group_rekey_option(WIRELESS_WIFI_5G_GUEST_WPA_GROUP_REKEY_OPTION, &wpa_group_rekey);
                    }
                    else
#endif
                    {
                        api_get_wifi_guest_wpa_group_rekey_option(WIRELESS_WIFI_GUEST_WPA_GROUP_REKEY_OPTION, &wpa_group_rekey);
                    }
                }
                else 
                    api_get_wifi_ifacex_wpa_group_rekey_option(index, &wpa_group_rekey);
                settings->key_renewal = wpa_group_rekey;
                break;
            case WLAN_ENC_TKIPAES:
                sprintf(settings->encryption, "%s", encryption_type[4].encryption_str);
                if(guest_flag) 
                {
#if SUPPORT_WLAN_5G_SETTING
                    if (1 == settings->radio_id)
                    {
                        api_get_wifi_guest_wpa_group_rekey_option(WIRELESS_WIFI_5G_GUEST_WPA_GROUP_REKEY_OPTION, &wpa_group_rekey);
                    }
                    else
#endif
                    {
                        api_get_wifi_guest_wpa_group_rekey_option(WIRELESS_WIFI_GUEST_WPA_GROUP_REKEY_OPTION, &wpa_group_rekey);
                    }
                }
                else 
                    api_get_wifi_ifacex_wpa_group_rekey_option(index, &wpa_group_rekey);
                settings->key_renewal = wpa_group_rekey;
                break;
            default:
                break;
        }

        switch(settings->typeIdx)
        {
            case WLAN_AUTH_OPEN:
            case WLAN_AUTH_WEPAUTO:
            case WLAN_AUTH_SHARED:
            case WLAN_AUTH_WPA:
            case WLAN_AUTH_WPA2:
            case WLAN_AUTH_WPA1WPA2:
                if(settings->enable_8021x)
                {
                    api_get_wifi_ifacex_auth_server_option(index, auth_server_ipaddr, sizeof(auth_server_ipaddr));
                    api_get_wifi_ifacex_auth_port_option(index, &auth_server_port);
                    api_get_wifi_ifacex_auth_secret_option(index, auth_server_secret, sizeof(auth_server_secret));
                    sprintf(settings->radius_ip1, "%s", auth_server_ipaddr);
                    settings->radius_port1 = auth_server_port;
                    sprintf(settings->radius_secret1, "%s", auth_server_secret);
                }
                break;
            case WLAN_AUTH_WPAPSK:
            case WLAN_AUTH_WPA2PSK:
            case WLAN_AUTH_WPA1PSKWPA2PSK:
                if (guest_flag)
                {
#if SUPPORT_WLAN_5G_SETTING
                    if (1 == settings->radio_id)
                    {
                        api_get_wifi_guest_wpa_key_option(WIRELESS_WIFI_5G_GUEST_KEY_OPTION, wpa_key_string, sizeof(wpa_key_string));
                    }
                    else
#endif
                    {
                        api_get_wifi_guest_wpa_key_option(WIRELESS_WIFI_GUEST_KEY_OPTION, wpa_key_string, sizeof(wpa_key_string));
                    }
                }
                else
                    api_get_wifi_ifacex_wpakey_key_option(index, wpa_key_string, sizeof(wpa_key_string)); 
                sprintf(settings->key, "%s", wpa_key_string);
                break;
            default:
                break;
        }
        sprintf(settings->type, "%s", security_type[settings->typeIdx].security_str);
    }
    else // if disable wlan_encryption_type
    {
        if(settings->enable_8021x)
        {
            api_get_wifi_ifacex_auth_server_option(index, auth_server_ipaddr, sizeof(auth_server_ipaddr));
            api_get_wifi_ifacex_auth_port_option(index, &auth_server_port);
            api_get_wifi_ifacex_auth_secret_option(index, auth_server_secret, sizeof(auth_server_secret));
            sprintf(settings->radius_ip1, "%s", auth_server_ipaddr);
            settings->radius_port1 = auth_server_port;
            sprintf(settings->radius_secret1, "%s", auth_server_secret);
        }
    }
    return 0;
}
#if HAS_ENGENIUS && SUPPORT_CONFIG_HAS_SECTIONNAME
int get_wlan_security_options(WLAN_RADIO_SECURITY_T *settings, char **result)
{
    int key_id=0,security=0, encrypton=0, authentication=0,wep_key_id=0, wpa_group_rekey=0, auth_server_port=0;
    char option[64],wep_key_string[127+1], wpa_key_string[127+1], auth_server_ipaddr[15+1], auth_server_secret[127+1];
    char *enc_option=NULL, *ssid_option=NULL, *ssid_key_option=NULL, *ssid_key_id_option=NULL, *wpa_rekey_option=NULL;
    char *auth_server_option=NULL, *auth_port_option=NULL, *auth_secret_option=NULL;
	
	if(0 == settings->radio_id){
		enc_option=WIRELESS_WIFI0_SSID_1_ENCRYPTION_OPTION;
		ssid_option=WIRELESS_WIFI0_SSID_1_OPTION;
		ssid_key_option=WIRELESS_WIFI0_SSID_1_KEY_OPTION;
		ssid_key_id_option=WIRELESS_WIFI0_SSID_1_KEY_ID_OPTION;
		wpa_rekey_option=WIRELESS_WIFI0_SSID_1_WPA_GROUP_REKEY_OPTION;
		auth_server_option=WIRELESS_WIFI0_SSID_1_AUTH_SERVER_OPTION;
		auth_port_option=WIRELESS_WIFI0_SSID_1_AUTH_PORT_OPTION;
		auth_secret_option=WIRELESS_WIFI0_SSID_1_AUTH_SECRET_OPTION;
	}
#if SUPPORT_WLAN_5G_SETTING
	else if(1 == settings->radio_id){
		enc_option=WIRELESS_WIFI1_SSID_1_ENCRYPTION_OPTION;
		ssid_option=WIRELESS_WIFI1_SSID_1_OPTION;
		ssid_key_option=WIRELESS_WIFI1_SSID_1_KEY_OPTION;
		ssid_key_id_option=WIRELESS_WIFI1_SSID_1_KEY_ID_OPTION;
		wpa_rekey_option=WIRELESS_WIFI1_SSID_1_WPA_GROUP_REKEY_OPTION;
		auth_server_option=WIRELESS_WIFI1_SSID_1_AUTH_SERVER_OPTION;
		auth_port_option=WIRELESS_WIFI1_SSID_1_AUTH_PORT_OPTION;
		auth_secret_option=WIRELESS_WIFI1_SSID_1_AUTH_SECRET_OPTION;
	}
#endif
	// SecurityEnabled : boolean,
	// WPSEnabled : boolean,
	// SecurityType : string,
	// Encryption : string,
	// Key : string,
	// RadiusIP1 : string,
	// RadiusPort1 : integer,
	// RadiusSecret1 : string

	api_get_wifi_iface_encryption_option(enc_option, &security);
	get_security_convert(security, &encrypton, &authentication);
	settings->enabled = (security > 0)?true:false;
	settings->typeIdx = authentication;
	settings->enable_8021x = ((authentication==WLAN_AUTH_WPA)||
		(authentication==WLAN_AUTH_WPA2)||
		(authentication==WLAN_AUTH_WPA1WPA2))?true:false;
	// If wireless security is enabled
	if(security > 0) {
		switch(encrypton){
			case WLAN_ENC_WEP:
			api_get_wifi_iface_wepkey_id_option(ssid_key_id_option,&key_id);

			memset(option,0x0,sizeof(option));
			memset(wep_key_string,0x0,sizeof(wep_key_string));
			sprintf(option,"%s.%s%d",ssid_option,"key",key_id);
			api_get_wifi_iface_wepkey_key_option(option,wep_key_string,sizeof(wep_key_string));
			sprintf(settings->key, "%s", wep_key_string);
			sprintf(settings->encryption, "%s", (64 == strlen(wep_key_string))?encryption_type[0].encryption_str:encryption_type[1].encryption_str);
			break;

			case WLAN_ENC_TKIP:
			sprintf(settings->encryption, "%s", encryption_type[2].encryption_str);
			api_get_wifi_iface_wpa_group_rekey_option(wpa_rekey_option,&wpa_group_rekey);
			settings->key_renewal = wpa_group_rekey;
			break;

			case WLAN_ENC_AES:
			sprintf(settings->encryption, "%s", encryption_type[3].encryption_str);
			api_get_wifi_iface_wpa_group_rekey_option(wpa_rekey_option,&wpa_group_rekey);
			settings->key_renewal = wpa_group_rekey;
			break;

			case WLAN_ENC_TKIPAES:
			sprintf(settings->encryption, "%s", encryption_type[4].encryption_str);
			api_get_wifi_iface_wpa_group_rekey_option(wpa_rekey_option,&wpa_group_rekey);
			settings->key_renewal = wpa_group_rekey;
			break;

			default:
				break;
		}

		switch(settings->typeIdx){
			case WLAN_AUTH_OPEN:
			case WLAN_AUTH_WEPAUTO:
			case WLAN_AUTH_SHARED:
			case WLAN_AUTH_WPA:
			case WLAN_AUTH_WPA2:
			case WLAN_AUTH_WPA1WPA2:
			if(settings->enable_8021x){
				api_get_wifi_iface_auth_server_option(auth_server_option, auth_server_ipaddr, sizeof(auth_server_ipaddr));
				api_get_wifi_iface_auth_port_option(auth_port_option , &auth_server_port);
				api_get_wifi_iface_auth_secret_option(auth_secret_option, auth_server_secret, sizeof(auth_server_secret));

				sprintf(settings->radius_ip1, "%s", auth_server_ipaddr);
				settings->radius_port1 = auth_server_port;
				sprintf(settings->radius_secret1, "%s", auth_server_secret);
			}
			break;

			case WLAN_AUTH_WPAPSK:
			case WLAN_AUTH_WPA2PSK:
			case WLAN_AUTH_WPA1PSKWPA2PSK:
			api_get_wifi_iface_wpakey_key_option(ssid_key_option, wpa_key_string, sizeof(wpa_key_string));
			sprintf(settings->key, "%s", wpa_key_string);
			break;

			default:
				break;
			}
		sprintf(settings->type, "%s", security_type[settings->typeIdx].security_str);
	}
	else // if disable wlan_encryption_type
	{
		if(settings->enable_8021x){
			api_get_wifi_iface_auth_server_option(auth_server_option, auth_server_ipaddr, sizeof(auth_server_ipaddr));
			api_get_wifi_iface_auth_port_option(auth_port_option , &auth_server_port);
			api_get_wifi_iface_auth_secret_option(auth_secret_option, auth_server_secret, sizeof(auth_server_secret));

			sprintf(settings->radius_ip1, "%s", auth_server_ipaddr);
			settings->radius_port1 = auth_server_port;
			sprintf(settings->radius_secret1, "%s", auth_server_secret);
		}
	}
	return 0;
}
#endif
/*****************************************************************
* NAME:    get_wlan_radio_security_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_wlan_radio_security_cb(HTTPS_CB *pkt){

    WLAN_RADIO_SECURITY_T settings;
    char *result;
    result = NULL;
    memset(&settings, 0, sizeof(settings));

    get_wlan_security(pkt, &settings, &result);

    if(NULL != result)
        send_simple_response(pkt, result);
    else
    {
        result = OK_STR;

        if(pkt->json)
            get_wlan_radio_security_json_cb(pkt, &settings, result);
    }
    return 0;
}

/*****************************************************************
* NAME:    set_wlan_radio_security_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_wlan_radio_security_cb(HTTPS_CB *pkt){

    /*******************************************************************
     * SetWLanRadioSecurityResult type:                                *
     * OK, ERROR, REBOOT, ERROR_BAD_RADIOID, ERROR_BAD_SSIDID,         *
     * ERROR_TYPE, ERROR_ENCRYPTION, ERROR_ILLEAGAL_KEY                *
     * ERROR_KEY_RENEWAL, ERROR_RADIUS_VALUE                           *
     *******************************************************************/
    bool result = TRUE;
    int ApiResult = 0;
    int guest_flag = 0; 
    int index = 0, hwmode = 0, security = 0, encryption = 0, authentication = 0, wep_key_type = -1, wep_key_length = 0;
    int isReboot = 0;
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char ssid[31+1];
    WLAN_RADIOS_T settings;
    char *return_str;
    HTTP_PACKET_CONTENT_FORMAT pkt_format;

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if(NULL == pkt || 0 == strlen(query_str))
    {
        goto send_pkt;
    }

    memset(&settings, 0, sizeof(settings));
    memset(ssid, 0, sizeof(ssid));

    if(pkt->json)
    {
        pkt_format = JSON_FORMAT;
        result &= parse_json_wlan_radio_security(query_str, &settings, return_str);
    }

    if(TRUE != result) goto send_pkt;

    /* 1. check value */
    if(settings.radio_id == -1){
        return_str = ERROR_BAD_RADIOID_STR;
        goto send_pkt;
    }

    if(settings.ssid_id == -1){
        return_str = ERROR_BAD_SSIDID_STR;
        goto send_pkt;
    }

    guest_flag = (settings.ssid_id == WIFI_SSID_NUM)?1:0; 

    if (guest_flag == 1) 
    {
#if SUPPORT_WLAN_5G_SETTING
        if (1 == settings.radio_id)
        {
            api_set_wifi_guest_disabled_option(WIRELESS_WIFI_5G_GUEST_DISABLED_OPTION, (settings.radio_enabled==true)?0:1);
            api_set_integer_option(DHCP_GUEST2_IGNORE_OPTION, (settings.radio_enabled==true)?0:1);
        }
        else
#endif
        {
            api_set_wifi_guest_disabled_option(WIRELESS_WIFI_GUEST_DISABLED_OPTION, (settings.radio_enabled==true)?0:1);
            api_set_integer_option(DHCP_GUEST1_IGNORE_OPTION, (settings.radio_enabled==true)?0:1);
        }
        if (settings.radio_enabled==false) {
            // isReboot = 1;
            goto reload_setting;
        }
    }
    else 
    {
#if SUPPORT_WLAN_5G_SETTING
        if (1 == settings.radio_id)
        {
            api_set_wifi_disabled_option(WIRELESS_WIFI_5G_DISABLED_OPTION, (settings.radio_enabled==true)?0:1);
        }
        else
#endif
        {
            api_set_wifi_disabled_option(WIRELESS_WIFI_DISABLED_OPTION, (settings.radio_enabled==true)?0:1);
        }
        index = (settings.radio_id == 1)?(settings.ssid_id+WIFI_SSID_NUM):settings.ssid_id;
        api_set_wifi_ifacex_disabled_option(index, (settings.radio_enabled==true)?0:1);
        if (settings.radio_enabled==false) {
            // isReboot = 1;
            goto reload_setting;
        }
    }

        if (get_wlan_authentication(settings.type, &authentication) < 0)
        {
            return_str = ERROR_TYPE_NOT_SUPPORTED_STR;
            goto send_pkt;
        }
        if (get_wlan_encryption(settings.encryption, &encryption) < 0)
        {
            return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
            goto send_pkt;
        }
        if (guest_flag == 1) 
        {
            if (get_wifi_guest_security(encryption, authentication, &security) < 0)
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }

            switch (security) 
            {
                case GUEST_ENCRYPTION_NONE:
                    break;
                case GUEST_WPA_PSK_CCMP:
                case GUEST_WPA_PSK_TKIP:
                case GUEST_WPA_PSK_TKIP_CCMP:
                case GUEST_WPA2_PSK_CCMP:
                case GUEST_WPA2_PSK_TKIP:
                case GUEST_WPA2_PSK_TKIP_CCMP:
                case GUEST_WPA_PSK_MIXED_CCMP:
                case GUEST_WPA_PSK_MIXED_TKIP:
                case GUEST_WPA_PSK_MIXED_TKIP_CCMP:
                    if (api_set_wifi_guest_wpa_key_option(WIRELESS_WIFI_GUEST_KEY_OPTION, settings.key, sizeof(settings.key)))
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    break;
                default:
                    break;
            }
#if SUPPORT_WLAN_5G_SETTING
            if (1 == settings.radio_id)
            {
                ApiResult = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTIONFAKE_OPTION, security);
            }
            else
#endif
            {
                ApiResult = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTIONFAKE_OPTION, security);
            }
            if (ApiResult)
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
#if SUPPORT_WLAN_5G_SETTING
            if (1 == settings.radio_id)
            {
                ApiResult = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_OPTION, security);
            }
            else
#endif
            {
                ApiResult = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTION_OPTION, security);
            }
            if (ApiResult)
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
#if SUPPORT_WLAN_5G_SETTING
            if (1 == settings.radio_id)
            {
                ApiResult = api_set_bool_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_DISABLED_OPTION, 0);
            }
            else
#endif
            {
                ApiResult = api_set_bool_option(WIRELESS_WIFI_GUEST_ENCRYPTION_DISABLED_OPTION, 0);
            }
            if (ApiResult)
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
        }
        else
        {
            if (get_wifi_security(encryption, authentication, &security) < 0)
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
            switch (security) 
            {
                case ENCRYPTION_NONE:
                    if(settings.enable_8021x) 
                    {
                        ;
                    }
                    break;
                case WEP_OPEN:
                case WEP_SHARED:
                    wep_key_length = get_wlan_wep_key_length(settings.encryption);
                    wep_key_type = get_wlan_wep_key_type(wep_key_length, strlen(settings.key));
                    if (!api_check_wep_key_is_valid(wep_key_length, settings.key))
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    if (api_set_wifi_ifacex_wepkey_type_option(index, wep_key_type))
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    if (api_set_wifi_ifacex_wepkey_length_option(index, wep_key_length))
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    if (api_set_wifi_ifacex_wepkey_id_option(index, 1)) 
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    if (api_set_wifi_ifacex_wepkey_keyx_option(index, 1, settings.key, sizeof(settings.key))) 
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    break;
                case WPA_PSK_CCMP:
                case WPA_PSK_TKIP:
                case WPA_PSK_TKIP_CCMP:
                case WPA2_PSK_CCMP:
                case WPA2_PSK_TKIP:
                case WPA2_PSK_TKIP_CCMP:
                case WPA_PSK_MIXED_CCMP:
                case WPA_PSK_MIXED_TKIP:
                case WPA_PSK_MIXED_TKIP_CCMP:
                    if (api_set_wifi_ifacex_wpakey_key_option(index, settings.key, sizeof(settings.key)))
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    break;
                case WPA_EAP_CCMP:
                case WPA_EAP_TKIP:
                case WPA_EAP_TKIP_CCMP:
                case WPA2_EAP_CCMP:
                case WPA2_EAP_TKIP:
                case WPA2_EAP_TKIP_CCMP:
                case WPA_EAP_MIXED_CCMP:
                case WPA_EAP_MIXED_TKIP:
                case WPA_EAP_MIXED_TKIP_CCMP:
                    break;
                default:
                    break;
            }
            if (api_set_wifi_ifacex_encryption_option(index, security))
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
            if (api_set_wifi_ifacex_encryption_name_option(index, "encryptionFake", security))
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
            if (api_set_wifi_ifacex_sction_integer_option(index, "encrDisabled", 0))
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
            if (security != WPA2_PSK_CCMP) // WPS 2.0 only support WPS2-PSK (AES)
                api_set_wifi_ifacex_wps_enable_option(index, 0);
        }
    if (!guest_flag) 
    {
        memset(ssid, 0, sizeof(ssid));
        api_get_wifi_ifacex_ssid_option(index, ssid, sizeof(ssid));
    }
    if(TRUE == result)
    {
        return_str = OK_STR;
    }

reload_setting:
    if(result)
    {
        if(isReboot){
            APPAGENT_SYSTEM("uci commit");
            APPAGENT_SYSTEM("reboot &");
            return_str = REBOOT_STR;
        }
        else{
            APPAGENT_SYSTEM("uci commit");
            APPAGENT_SYSTEM("/etc/init.d/network reload &");
            APPAGENT_SYSTEM("/etc/init.d/led reload &");
        }
    }

send_pkt:
    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    set_wireless_setting
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
/*******************************************************************
 * SetWLanRadios type:                                             *
 * OK, ERROR, REBOOT, ERROR_BAD_RADIOID, ERROR_BAD_SSIDID,         *
 * ERROR_BAD_CHANNEL, ERROR_BAD_MODE,                              *
 * ERROR_BAD_SSID, ERROR_TYPE_NOT_SUPPORTED,                       *
 * ERROR_ENCRYPTION_NOT_SUPPORTED, ERROR_ILLEGAL_KEY_VALUE         *
 *******************************************************************/
int set_wireless_setting(WLAN_RADIOS_T *setting, char **return_str, int *isReboot)
{
    int result = 0;
    int guest_flag = 0;
    int index = 0, hwmode = 0;
    int security = 0, encryption = 0, authentication = 0, wep_key_type = -1, wep_key_length = 0;
    char ssid[32];

    // The default value in return packet is ERROR.
    *return_str = ERROR_STR;

    memset(ssid, 0, sizeof(ssid));

    /* 1. check value */
    if (-1 == setting->radio_id)
    {
        *return_str = ERROR_BAD_RADIOID_STR;
        goto out;
    }

    if (-1 == setting->ssid_id)
    {
        *return_str = ERROR_BAD_SSIDID_STR;
        goto out;
    }

    guest_flag = (WIFI_SSID_NUM == setting->ssid_id)?1:0;

    if (1 == guest_flag)
    {
#if SUPPORT_WLAN_5G_SETTING
        if (1 == setting->radio_id)
        {
            api_set_wifi_guest_disabled_option(WIRELESS_WIFI_5G_GUEST_DISABLED_OPTION, (true == setting->radio_enabled)?0:1);
            api_set_integer_option(DHCP_GUEST2_IGNORE_OPTION, (true == setting->radio_enabled)?0:1);
        }
        else
#endif
        {
            api_set_wifi_guest_disabled_option(WIRELESS_WIFI_GUEST_DISABLED_OPTION, (true == setting->radio_enabled)?0:1);
            api_set_integer_option(DHCP_GUEST1_IGNORE_OPTION, (true == setting->radio_enabled)?0:1);
        }

        if (false == setting->radio_enabled)
        {
            *isReboot = 1;
            *return_str = OK_STR;
            goto out;
        }

#if SUPPORT_WLAN_5G_SETTING
        if (1 == setting->radio_id)
        {
            result = api_set_wifi_guest_ssid_option(WIRELESS_WIFI_5G_GUEST_SSID_OPTION, setting->ssid, sizeof(setting->ssid));
        }
        else
#endif
        {
            result = api_set_wifi_guest_ssid_option(WIRELESS_WIFI_GUEST_SSID_OPTION, setting->ssid, sizeof(setting->ssid));
        }

        if (result)
        {
            *return_str = ERROR_BAD_SSID_STR;
            goto out;
        }

#if SUPPORT_WLAN_5G_SETTING
        if (1 == setting->radio_id)
        {
            api_set_wifi_guest_hidden_option(WIRELESS_WIFI_5G_GUEST_HIDDEN_OPTION, (true == setting->ssid_broadcast)?0:1);
        }
        else
#endif
        {
            api_set_wifi_guest_hidden_option(WIRELESS_WIFI_GUEST_HIDDEN_OPTION, (true == setting->ssid_broadcast)?0:1);
        }
    }
    else
    {
#if SUPPORT_WLAN_5G_SETTING
        if (1 == setting->radio_id)
        {
            api_set_wifi_disabled_option(WIRELESS_WIFI_5G_DISABLED_OPTION, (true == setting->radio_enabled)?0:1);
        }
        else
#endif
        {
            api_set_wifi_disabled_option(WIRELESS_WIFI_DISABLED_OPTION, (true == setting->radio_enabled)?0:1);
        }

        index = (1 == setting->radio_id)?(setting->ssid_id+WIFI_SSID_NUM):setting->ssid_id;
        api_set_wifi_ifacex_disabled_option(index, (true == setting->radio_enabled)?0:1);

        if (false == setting->radio_enabled)
        {
            *isReboot = 1;
            *return_str = OK_STR;
            goto out;
        }

        if (api_set_wifi_ifacex_ssid_option(index, setting->ssid, sizeof(setting->ssid)))
        {
            *return_str = ERROR_BAD_SSID_STR;
            goto out;
        }

        api_set_wifi_ifacex_hidden_option(index, (true == setting->ssid_broadcast)?0:1);
    }

#if SUPPORT_WLAN_5G_SETTING
    if(1 == setting->radio_id)
    {
        if(0 != strlen(setting->mode))
        {
            get_wifi_hwmode_id(1, setting->mode, &hwmode);

            if (api_set_wifi_5g_hwmode_option(WIRELESS_WIFI_5G_HWMODE_OPTION, hwmode))
            {
                *return_str = ERROR_BAD_MODE_STR;
                goto out;
            }
        }

#if HAS_ENGENIUS
        setting->channel_width = (80 == setting->channel_width)?BANDWIDTH_80MHZ:
                                 (40 == setting->channel_width)?BANDWIDTH_40MHZ:
                                 (20 == setting->channel_width)?BANDWIDTH_20MHZ:
                                 BANDWIDTH_20MHZ_40MHZ;
#else
        setting->channel_width = (2 == setting->channel_width)?BANDWIDTH_80MHZ:setting->channel_width;
#endif

        if (api_set_wifi_5g_htmode_option(WIRELESS_WIFI_5G_HTMODE_OPTION, setting->channel_width))
        {
            goto out;
        }

        if(0 != setting->channel)
        {
            if (api_set_wifi_5g_channel_option(WIRELESS_WIFI_5G_CHANNEL_OPTION, setting->channel))
            {
                *return_str = ERROR_BAD_CHANNEL_STR;
                goto out;
            }
        }
    }
    else
#endif
    {
        if(0 != strlen(setting->mode))
        {
            get_wifi_hwmode_id(0, setting->mode, &hwmode);

            if (api_set_wifi_hwmode_option(WIRELESS_WIFI_HWMODE_OPTION, hwmode))
            {
                *return_str = ERROR_BAD_MODE_STR;
                goto out;
            }
        }

#if HAS_ENGENIUS
        setting->channel_width = (80 == setting->channel_width)?BANDWIDTH_80MHZ:
                                 (40 == setting->channel_width)?BANDWIDTH_40MHZ:
                                 (20 == setting->channel_width)?BANDWIDTH_20MHZ:
                                 BANDWIDTH_20MHZ_40MHZ;
#endif
        if (api_set_wifi_htmode_option(WIRELESS_WIFI_HTMODE_OPTION, setting->channel_width))
        {
            goto out;
        }

        if(0 != setting->channel)
        {
            if (api_set_wifi_channel_option(WIRELESS_WIFI_CHANNEL_OPTION, setting->channel))
            {
                *return_str = ERROR_BAD_CHANNEL_STR;
                goto out;
            }
        }
    }

    if (true == setting->authentication_enabled)
    {
        if (get_wlan_authentication(setting->type, &authentication) < 0)
        {
            *return_str = ERROR_TYPE_NOT_SUPPORTED_STR;
            goto out;
        }

        if (get_wlan_encryption(setting->encryption, &encryption) < 0)
        {
            *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
            goto out;
        }

        if (1 == guest_flag)
        {
            if (get_wifi_guest_security(encryption, authentication, &security) < 0)
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }

            switch (security)
            {
                case GUEST_ENCRYPTION_NONE:
                    break;
                case GUEST_WPA_PSK_CCMP:
                case GUEST_WPA_PSK_TKIP:
                case GUEST_WPA_PSK_TKIP_CCMP:
                case GUEST_WPA2_PSK_CCMP:
                case GUEST_WPA2_PSK_TKIP:
                case GUEST_WPA2_PSK_TKIP_CCMP:
                case GUEST_WPA_PSK_MIXED_CCMP:
                case GUEST_WPA_PSK_MIXED_TKIP:
                case GUEST_WPA_PSK_MIXED_TKIP_CCMP:
                    if (api_set_wifi_guest_wpa_key_option(WIRELESS_WIFI_GUEST_KEY_OPTION, setting->key, sizeof(setting->key)))
                    {
                        *return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto out;
                    }
                    break;
                default:
                    break;
            }

#if SUPPORT_WLAN_5G_SETTING
            if (1 == setting->radio_id)
            {
                result = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTIONFAKE_OPTION, security);
            }
            else
#endif
            {
                result = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTIONFAKE_OPTION, security);
            }

            if (result)
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }

#if SUPPORT_WLAN_5G_SETTING
            if (1 == setting->radio_id)
            {
                result = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_OPTION, security);
            }
            else
#endif
            {
                result = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTION_OPTION, security);
            }

            if (result)
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }

#if SUPPORT_WLAN_5G_SETTING
            if (1 == setting->radio_id)
            {
                result = api_set_bool_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_DISABLED_OPTION, 0);
            }
            else
#endif
            {
                result = api_set_bool_option(WIRELESS_WIFI_GUEST_ENCRYPTION_DISABLED_OPTION, 0);
            }

            if (result)
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }
        }
        else
        {
            if (get_wifi_security(encryption, authentication, &security) < 0)
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }

            switch (security)
            {
                case ENCRYPTION_NONE:
                    if (setting->enable_8021x || (0 != strlen(setting->radius_ip1)))
                    {
                        if (api_check_ip_addr(setting->radius_ip1))
                        {
                            api_set_wifi_ifacex_auth_server_option(index, setting->radius_ip1, sizeof(setting->radius_ip1));
                            api_set_wifi_ifacex_auth_port_option(index, setting->radius_port1);
                            api_set_wifi_ifacex_auth_secret_option(index, setting->radius_secret1, sizeof(setting->radius_secret1));
                        }
                    }
                    break;
                case WEP_OPEN:
                case WEP_SHARED:
                    wep_key_length = get_wlan_wep_key_length(setting->encryption);
                    wep_key_type = get_wlan_wep_key_type(wep_key_length, strlen(setting->key));

                    if (!api_check_wep_key_is_valid(wep_key_length, setting->key))
                    {
                        *return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto out;
                    }

                    if (api_set_wifi_ifacex_wepkey_type_option(index, wep_key_type))
                    {
                        *return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto out;
                    }

                    if (api_set_wifi_ifacex_wepkey_length_option(index, wep_key_length))
                    {
                        *return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto out;
                    }

                    if (api_set_wifi_ifacex_wepkey_id_option(index, 1))
                    {
                        *return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto out;
                    }

                    if (api_set_wifi_ifacex_wepkey_keyx_option(index, 1, setting->key, sizeof(setting->key)))
                    {
                        *return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto out;
                    }
                    break;
                case WPA_PSK_CCMP:
                case WPA_PSK_TKIP:
                case WPA_PSK_TKIP_CCMP:
                case WPA2_PSK_CCMP:
                case WPA2_PSK_TKIP:
                case WPA2_PSK_TKIP_CCMP:
                case WPA_PSK_MIXED_CCMP:
                case WPA_PSK_MIXED_TKIP:
                case WPA_PSK_MIXED_TKIP_CCMP:
                    if (api_set_wifi_ifacex_wpakey_key_option(index, setting->key, sizeof(setting->key)))
                    {
                        *return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto out;
                    }
                    break;
                case WPA_EAP_CCMP:
                case WPA_EAP_TKIP:
                case WPA_EAP_TKIP_CCMP:
                case WPA2_EAP_CCMP:
                case WPA2_EAP_TKIP:
                case WPA2_EAP_TKIP_CCMP:
                case WPA_EAP_MIXED_CCMP:
                case WPA_EAP_MIXED_TKIP:
                case WPA_EAP_MIXED_TKIP_CCMP:
                    api_set_wifi_ifacex_auth_server_option(index, setting->radius_ip1, sizeof(setting->radius_ip1));
                    api_set_wifi_ifacex_auth_port_option(index, setting->radius_port1);
                    api_set_wifi_ifacex_auth_secret_option(index, setting->radius_secret1, sizeof(setting->radius_secret1));
                    break;
                default:
                    break;
            }

            if (api_set_wifi_ifacex_encryption_option(index, security))
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }

            if (api_set_wifi_ifacex_encryption_name_option(index, "encryptionFake", security))
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }

            if (api_set_wifi_ifacex_sction_integer_option(index, "encrDisabled", 0))
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }

            if (WPA2_PSK_CCMP != security) // WPS 2.0 only support WPS2-PSK (AES)
            {
                api_set_wifi_ifacex_wps_enable_option(index, 0);
            }
        }
    }
    else
    {
        if (1 == guest_flag)
        {
#if SUPPORT_WLAN_5G_SETTING
            if (1 == setting->radio_id)
            {
                result = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTIONFAKE_OPTION, ENCRYPTION_NONE);
            }
            else
#endif
            {
                result = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTIONFAKE_OPTION, ENCRYPTION_NONE);
            }

            if (result)
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }

#if SUPPORT_WLAN_5G_SETTING
            if (1 == setting->radio_id)
            {
                result = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_OPTION, ENCRYPTION_NONE);
            }
            else
#endif
            {
                result = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTION_OPTION, ENCRYPTION_NONE);
            }

            if (result)
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }

#if SUPPORT_WLAN_5G_SETTING
            if (1 == setting->radio_id)
            {
                result = api_set_bool_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_DISABLED_OPTION, 1);
            }
            else
#endif
            {
                result = api_set_bool_option(WIRELESS_WIFI_GUEST_ENCRYPTION_DISABLED_OPTION, 1);
            }

            if (result)
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }
        }
        else
        {
            if (api_set_wifi_ifacex_encryption_option(index, ENCRYPTION_NONE))
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }

            if (api_set_wifi_ifacex_encryption_name_option(index, "encryptionFake", ENCRYPTION_NONE))
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }

            if (api_set_wifi_ifacex_sction_integer_option(index, "encrDisabled", 1))
            {
                *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto out;
            }
        }
    }

    if (!guest_flag)
    {
        memset(ssid, 0, sizeof(ssid));
        api_get_wifi_ifacex_ssid_option(index, ssid, sizeof(ssid));

        // If the wifi setting is changed, then wps status must be set as "Configurated".
        if ((true == setting->authentication_enabled) || strcmp(ssid, setting->ssid))
        {
            api_set_wifi_ifacex_wps_status_option(index, 2);
        }
    }

    if(API_RC_SUCCESS == result)
    {
        *return_str = OK_STR;
    }

out:
    return 0;
}
/*****************************************************************
* NAME:    set_wlan_radios_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
/*******************************************************************
 * SetWLanRadios type:                                             *
 * OK, ERROR, REBOOT, ERROR_BAD_RADIOID, ERROR_BAD_SSIDID,         *
 * ERROR_BAD_CHANNEL, ERROR_BAD_MODE,                              *
 * ERROR_BAD_SSID, ERROR_TYPE_NOT_SUPPORTED,                       *
 * ERROR_ENCRYPTION_NOT_SUPPORTED, ERROR_ILLEGAL_KEY_VALUE         * 
 *******************************************************************/
int set_wlan_radios_cb(HTTPS_CB *pkt){
    bool result = TRUE;
    int ApiResult = 0;
    int guest_flag = 0; 
    int index = 0, hwmode = 0, security = 0, encryption = 0, authentication = 0, wep_key_type = -1, wep_key_length = 0;
    int isReboot = 0;
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char ssid[31+1];
    WLAN_RADIOS_T settings;
    char *return_str;
    HTTP_PACKET_CONTENT_FORMAT pkt_format;

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if(NULL == pkt || 0 == strlen(query_str))
    {
        goto send_pkt;
    }

    memset(&settings, 0, sizeof(settings));
    memset(ssid, 0, sizeof(ssid));

    if(pkt->json)
    {
        pkt_format = JSON_FORMAT;
        result &= parse_json_wlan_radios(query_str, &settings, return_str);
    }

    if(TRUE != result) goto send_pkt;

    /* 1. check value */
    if(settings.radio_id == -1){
        return_str = ERROR_BAD_RADIOID_STR;
        goto send_pkt;
    }

    if(settings.ssid_id == -1){
        return_str = ERROR_BAD_SSIDID_STR;
        goto send_pkt;
    }

    guest_flag = (settings.ssid_id == WIFI_SSID_NUM)?1:0; 

    if (guest_flag == 1) 
    {
#if SUPPORT_WLAN_5G_SETTING
        if (1 == settings.radio_id)
        {
            api_set_wifi_guest_disabled_option(WIRELESS_WIFI_5G_GUEST_DISABLED_OPTION, (settings.radio_enabled==true)?0:1);
            api_set_integer_option(DHCP_GUEST2_IGNORE_OPTION, (settings.radio_enabled==true)?0:1);
        }
        else
#endif
        {
            api_set_wifi_guest_disabled_option(WIRELESS_WIFI_GUEST_DISABLED_OPTION, (settings.radio_enabled==true)?0:1);
            api_set_integer_option(DHCP_GUEST1_IGNORE_OPTION, (settings.radio_enabled==true)?0:1);
        }
        if (settings.radio_enabled==false) {
            isReboot = 1;
            goto reload_setting;
        }
#if SUPPORT_WLAN_5G_SETTING
        if (1 == settings.radio_id)
        {
            ApiResult = api_set_wifi_guest_ssid_option(WIRELESS_WIFI_5G_GUEST_SSID_OPTION, settings.ssid, sizeof(settings.ssid));
        }
        else
#endif
        {
            ApiResult = api_set_wifi_guest_ssid_option(WIRELESS_WIFI_GUEST_SSID_OPTION, settings.ssid, sizeof(settings.ssid));
        }
        if (ApiResult)
        {
            return_str = ERROR_BAD_SSID_STR;
            goto send_pkt;
        }
#if SUPPORT_WLAN_5G_SETTING
        if (1 == settings.radio_id)
        {
            api_set_wifi_guest_hidden_option(WIRELESS_WIFI_5G_GUEST_HIDDEN_OPTION, (settings.ssid_broadcast==true)?0:1);
        }
        else
#endif
        {
            api_set_wifi_guest_hidden_option(WIRELESS_WIFI_GUEST_HIDDEN_OPTION, (settings.ssid_broadcast==true)?0:1);
        }
    }
    else 
    {
#if SUPPORT_WLAN_5G_SETTING
        if (1 == settings.radio_id)
        {
            api_set_wifi_disabled_option(WIRELESS_WIFI_5G_DISABLED_OPTION, (settings.radio_enabled==true)?0:1);
        }
        else
#endif
        {
            api_set_wifi_disabled_option(WIRELESS_WIFI_DISABLED_OPTION, (settings.radio_enabled==true)?0:1);
        }
        index = (settings.radio_id == 1)?(settings.ssid_id+WIFI_SSID_NUM):settings.ssid_id;
        api_set_wifi_ifacex_disabled_option(index, (settings.radio_enabled==true)?0:1);
        if (settings.radio_enabled==false) {
            isReboot = 1;
            goto reload_setting;
        }
        if (api_set_wifi_ifacex_ssid_option(index, settings.ssid, sizeof(settings.ssid)))
        {
            return_str = ERROR_BAD_SSID_STR;
            goto send_pkt;
        }
        api_set_wifi_ifacex_hidden_option(index, (settings.ssid_broadcast == true) ? 0 : 1); 
    }

#if SUPPORT_WLAN_5G_SETTING
    if(settings.radio_id == 1)
    {
        get_wifi_hwmode_id(1, settings.mode, &hwmode);
        if (api_set_wifi_5g_hwmode_option(WIRELESS_WIFI_5G_HWMODE_OPTION, hwmode))
        {
            return_str = ERROR_BAD_MODE_STR;
            goto send_pkt;
        }
        settings.channel_width = (settings.channel_width==2)?BANDWIDTH_80MHZ:settings.channel_width;
        if (api_set_wifi_5g_htmode_option(WIRELESS_WIFI_5G_HTMODE_OPTION, settings.channel_width))
        {
            goto send_pkt;
        }
        if (api_set_wifi_5g_channel_option(WIRELESS_WIFI_5G_CHANNEL_OPTION, settings.channel))
        {
            return_str = ERROR_BAD_CHANNEL_STR;
            goto send_pkt;
        }
    }
    else
#endif
    {
        get_wifi_hwmode_id(0, settings.mode, &hwmode);
        if (api_set_wifi_hwmode_option(WIRELESS_WIFI_HWMODE_OPTION, hwmode))
        {
            return_str = ERROR_BAD_MODE_STR;
            goto send_pkt;
        }
        if (api_set_wifi_htmode_option(WIRELESS_WIFI_HTMODE_OPTION, settings.channel_width))
        {
            goto send_pkt;
        }
        if (api_set_wifi_channel_option(WIRELESS_WIFI_CHANNEL_OPTION, settings.channel))
        {
            return_str = ERROR_BAD_CHANNEL_STR;
            goto send_pkt;
        }
    }

    if(settings.authentication_enabled == true)
    {
        if (get_wlan_authentication(settings.type, &authentication) < 0)
        {
            return_str = ERROR_TYPE_NOT_SUPPORTED_STR;
            goto send_pkt;
        }
        if (get_wlan_encryption(settings.encryption, &encryption) < 0)
        {
            return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
            goto send_pkt;
        }
        if (guest_flag == 1) 
        {
            if (get_wifi_guest_security(encryption, authentication, &security) < 0)
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }

            switch (security) 
            {
                case GUEST_ENCRYPTION_NONE:
                    break;
                case GUEST_WPA_PSK_CCMP:
                case GUEST_WPA_PSK_TKIP:
                case GUEST_WPA_PSK_TKIP_CCMP:
                case GUEST_WPA2_PSK_CCMP:
                case GUEST_WPA2_PSK_TKIP:
                case GUEST_WPA2_PSK_TKIP_CCMP:
                case GUEST_WPA_PSK_MIXED_CCMP:
                case GUEST_WPA_PSK_MIXED_TKIP:
                case GUEST_WPA_PSK_MIXED_TKIP_CCMP:
                    if (api_set_wifi_guest_wpa_key_option(WIRELESS_WIFI_GUEST_KEY_OPTION, settings.key, sizeof(settings.key)))
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    break;
                default:
                    break;
            }
#if SUPPORT_WLAN_5G_SETTING
            if (1 == settings.radio_id)
            {
                ApiResult = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTIONFAKE_OPTION, security);
            }
            else
#endif
            {
                ApiResult = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTIONFAKE_OPTION, security);
            }
            if (ApiResult)
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
#if SUPPORT_WLAN_5G_SETTING
            if (1 == settings.radio_id)
            {
                ApiResult = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_OPTION, security);
            }
            else
#endif
            {
                ApiResult = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTION_OPTION, security);
            }
            if (ApiResult)
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
#if SUPPORT_WLAN_5G_SETTING
            if (1 == settings.radio_id)
            {
                ApiResult = api_set_bool_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_DISABLED_OPTION, 0);
            }
            else
#endif
            {
                ApiResult = api_set_bool_option(WIRELESS_WIFI_GUEST_ENCRYPTION_DISABLED_OPTION, 0);
            }
            if (ApiResult)
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
        }
        else
        {
            if (get_wifi_security(encryption, authentication, &security) < 0)
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
            switch (security) 
            {
                case ENCRYPTION_NONE:
                    if(settings.enable_8021x) 
                    {
                        api_set_wifi_ifacex_auth_server_option(index, settings.radius_ip1, sizeof(settings.radius_ip1));
                        api_set_wifi_ifacex_auth_port_option(index, settings.radius_port1);
                        api_set_wifi_ifacex_auth_secret_option(index, settings.radius_secret1, sizeof(settings.radius_secret1));
                    }
                    break;
                case WEP_OPEN:
                case WEP_SHARED:
                    wep_key_length = get_wlan_wep_key_length(settings.encryption);
                    wep_key_type = get_wlan_wep_key_type(wep_key_length, strlen(settings.key));
                    if (!api_check_wep_key_is_valid(wep_key_length, settings.key))
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    if (api_set_wifi_ifacex_wepkey_type_option(index, wep_key_type))
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    if (api_set_wifi_ifacex_wepkey_length_option(index, wep_key_length))
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    if (api_set_wifi_ifacex_wepkey_id_option(index, 1)) 
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    if (api_set_wifi_ifacex_wepkey_keyx_option(index, 1, settings.key, sizeof(settings.key))) 
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    break;
                case WPA_PSK_CCMP:
                case WPA_PSK_TKIP:
                case WPA_PSK_TKIP_CCMP:
                case WPA2_PSK_CCMP:
                case WPA2_PSK_TKIP:
                case WPA2_PSK_TKIP_CCMP:
                case WPA_PSK_MIXED_CCMP:
                case WPA_PSK_MIXED_TKIP:
                case WPA_PSK_MIXED_TKIP_CCMP:
                    if (api_set_wifi_ifacex_wpakey_key_option(index, settings.key, sizeof(settings.key)))
                    {
                        return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        goto send_pkt;
                    }
                    break;
                case WPA_EAP_CCMP:
                case WPA_EAP_TKIP:
                case WPA_EAP_TKIP_CCMP:
                case WPA2_EAP_CCMP:
                case WPA2_EAP_TKIP:
                case WPA2_EAP_TKIP_CCMP:
                case WPA_EAP_MIXED_CCMP:
                case WPA_EAP_MIXED_TKIP:
                case WPA_EAP_MIXED_TKIP_CCMP:
                    api_set_wifi_ifacex_auth_server_option(index, settings.radius_ip1, sizeof(settings.radius_ip1));
                    api_set_wifi_ifacex_auth_port_option(index, settings.radius_port1);
                    api_set_wifi_ifacex_auth_secret_option(index, settings.radius_secret1, sizeof(settings.radius_secret1));
                    break;
                default:
                    break;
            }
            if (api_set_wifi_ifacex_encryption_option(index, security))
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
            if (api_set_wifi_ifacex_encryption_name_option(index, "encryptionFake", security))
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
            if (api_set_wifi_ifacex_sction_integer_option(index, "encrDisabled", 0))
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
            if (security != WPA2_PSK_CCMP) // WPS 2.0 only support WPS2-PSK (AES)
                api_set_wifi_ifacex_wps_enable_option(index, 0);
        }
    }
    else
    {
        if (guest_flag == 1) 
        {
#if SUPPORT_WLAN_5G_SETTING
            if (1 == settings.radio_id)
            {
                ApiResult = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTIONFAKE_OPTION, ENCRYPTION_NONE);
            }
            else
#endif
            {
                ApiResult = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTIONFAKE_OPTION, ENCRYPTION_NONE);
            }
            if (ApiResult)
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }

#if SUPPORT_WLAN_5G_SETTING
            if (1 == settings.radio_id)
            {
                ApiResult = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_OPTION, ENCRYPTION_NONE);
            }
            else
#endif
            {
                ApiResult = api_set_wifi_guest_encryption_option(WIRELESS_WIFI_GUEST_ENCRYPTION_OPTION, ENCRYPTION_NONE);
            }
            if (ApiResult)
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }

#if SUPPORT_WLAN_5G_SETTING
            if (1 == settings.radio_id)
            {
                ApiResult = api_set_bool_option(WIRELESS_WIFI_5G_GUEST_ENCRYPTION_DISABLED_OPTION, 1);
            }
            else
#endif
            {
                ApiResult = api_set_bool_option(WIRELESS_WIFI_GUEST_ENCRYPTION_DISABLED_OPTION, 1);
            }

            if (ApiResult)
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
        }
        else
        {
            if (api_set_wifi_ifacex_encryption_option(index, ENCRYPTION_NONE))
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
            if (api_set_wifi_ifacex_encryption_name_option(index, "encryptionFake", ENCRYPTION_NONE))
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
            if (api_set_wifi_ifacex_sction_integer_option(index, "encrDisabled", 1))
            {
                return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                goto send_pkt;
            }
        }
    }

    if (!guest_flag) 
    {
        memset(ssid, 0, sizeof(ssid));
        api_get_wifi_ifacex_ssid_option(index, ssid, sizeof(ssid));
        if ((settings.authentication_enabled == true) || strcmp(ssid, settings.ssid)) // If the wifi setting is changed, then wps status must be set as "Configurated".
            api_set_wifi_ifacex_wps_status_option(index, 2);
    }
    if(TRUE == result)
    {
        return_str = OK_STR;
    }

reload_setting:
    if(result)
    {
        if(isReboot){
            APPAGENT_SYSTEM("uci commit");
            APPAGENT_SYSTEM("reboot &");
            return_str = REBOOT_STR;
        }
        else{
            APPAGENT_SYSTEM("uci commit");
            APPAGENT_SYSTEM("/etc/init.d/network reload &");
            APPAGENT_SYSTEM("/etc/init.d/led reload &");
        }
    }

send_pkt:
    send_simple_response(pkt, return_str);

    return 0;
}
/*****************************************************************
* NAME:    get_access_control_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_access_control_list_cb(HTTPS_CB *pkt){

#if 0
    char *query_str="{\n" \
                    "  \"RadioID\" : \"2.4GHZ\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    int i,MacEnabled=0;
    char mac_list_tmp[18*MAC_FILTER_NUM+1];
    object_list_info maclist[MAC_FILTER_NUM];
    char *result;
    char tok[] = {0};
    HTTP_PACKET_CONTENT_FORMAT pkt_format;
    ACCESS_CONTROL_LIST_T settings;
    //macfiltering_t m;

    memset(&settings, 0, sizeof(settings));
    result = NULL;
    if(NULL == pkt || 0 == strlen(query_str))
    {
        result = ERROR_STR;
        return -1;
    }

    if(pkt->json)
    {
        pkt_format = JSON_FORMAT;
    }

    settings.radio_id = get_radio_id(pkt_format, query_str);

    if(-1 == settings.radio_id)
    {
        result = ERROR_BAD_RADIOID_STR;
        send_simple_response(pkt, result);
        return 0;
    }

    /* get value from tokens to struct */
    if(0 == settings.radio_id) // 2.4GHZ
    {
        api_get_wifi_ifacex_macfilter_option(0,&MacEnabled);
        settings.enable_macfilter = (MacEnabled == 0) ? 0 : 1 ;
        memset(&mac_list_tmp, 0, sizeof(mac_list_tmp));
        memset(&maclist, 0, sizeof(maclist));
        api_get_wifi_ifacex_maclist_option(settings.radio_id, mac_list_tmp, sizeof(mac_list_tmp));
        char *delim = " ";
        api_get_list_option_info(delim, mac_list_tmp, maclist, MAC_FILTER_NUM);

        for(i = 0; i < MAC_FILTER_NUM; i++){
            settings.is_enable[i] = (strlen(maclist[i].string)==0) ? 0 : 1;
            sprintf(settings.mac[i], "%s", maclist[i].string);
        }
    }

#if SUPPORT_WLAN_5G_SETTING
    else if(1 == settings.radio_id) // 5GHZ
    {
        api_get_wifi_ifacex_macfilter_option(1,&MacEnabled);
        settings.enable_macfilter = (MacEnabled == 0) ? 0 : 1 ;
        memset(&mac_list_tmp, 0, sizeof(mac_list_tmp));
        memset(&maclist, 0, sizeof(maclist));
        api_get_wifi_ifacex_maclist_option(settings.radio_id, mac_list_tmp, sizeof(mac_list_tmp));
        char *delim = " ";
        api_get_list_option_info(delim, mac_list_tmp, maclist, MAC_FILTER_NUM);

        for(i = 0; i < MAC_FILTER_NUM; i++){
            settings.is_enable[i] = (strlen(maclist[i].string)==0) ? 0 : 1;
            sprintf(settings.mac[i], "%s", maclist[i].string);
        }
    }
#endif

    result = OK_STR;

    if(pkt->json)
    {
        get_access_control_list_json_cb(pkt, &settings);
    }

    return 0;
}

/*****************************************************************
* NAME:    set_access_control_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_access_control_list_cb(HTTPS_CB *pkt){
    bool result = TRUE;
    char *return_str;
    int wps_enable;
    ACCESS_CONTROL_LIST_T settings;
    HTTP_PACKET_CONTENT_FORMAT pkt_format;
    int isReboot = 0;
#if 0
    char *query_str="{\n" \
                    "  \"RadioID\" : \"2.4GHZ\",\n" \
                    "  \"EnabledMacFilter\" : \"true\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

#if HAS_WIRELESS_MUST_REBOOT
    isReboot = 1;
#endif

    if(NULL == pkt || 0 == strlen(query_str))
    {
        goto send_pkt;
    }

    if(pkt->json)
    {
        pkt_format = JSON_FORMAT;
    }

    settings.radio_id = get_radio_id(pkt_format, query_str);
    if(-1 == settings.radio_id){
        return_str = ERROR_BAD_RADIOID_STR;
        goto send_pkt;
    }

    if(pkt->json)
    {
        result &= parse_set_access_control_list_json_cb(query_str, &settings, return_str);
    }

    if(TRUE != result) goto send_pkt;


    if(0 == settings.radio_id) // 2.4GHZ
    {
        api_set_wifi_ifacex_macfilter_option(0,settings.enable_macfilter);
        api_get_wifi_ifacex_wps_enable_option(0,&wps_enable);
        if(settings.enable_macfilter == ENABLE && wps_enable == 1){
            api_set_wifi_ifacex_wps_enable_option(0,0);             //turn off wps
        }
    }

#if SUPPORT_WLAN_5G_SETTING
    else if(1 == settings.radio_id)  // 5GHZ
    {
        api_set_wifi_ifacex_macfilter_option(1,settings.enable_macfilter);
        api_get_wifi_ifacex_wps_enable_option(1,&wps_enable);
        if(settings.enable_macfilter == ENABLE && wps_enable == 1){
            api_set_wifi_ifacex_wps_enable_option(1,0);             //turn off wps
        }
    }
#endif

    /* Save new token value */
    if(result)
    {
        return_str = REBOOT_STR;
        APPAGENT_SYSTEM("uci commit");
        APPAGENT_SYSTEM("/etc/init.d/network reload &");
    }

send_pkt:
    /* Send response packet */
    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    add_access_control_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int add_access_control_list_cb(HTTPS_CB *pkt){

    bool result = TRUE;
    char *return_str;
    int radio_id = 0,num=0;
    int valid_num, i, j;
    char mac_exist[18*MAC_FILTER_NUM+1]={0};
    object_list_info maclist[MAC_FILTER_NUM];
    //char macArray[MAX_MAC_ADDR_LEN_IN_BYTE];
    char mac[20] = {0};
    char mac_tmp_input[20] = {0};
    char mac_tmp[20] = {0};
    char comment[20] = {0};
    //macfiltering_t m;
    HTTP_PACKET_CONTENT_FORMAT pkt_format;

#if 0
    char *query_str="{\n" \
                    "  \"RadioID\" : \"2.4GHZ\",\n" \
                    "  \"MacAddress\" : \"00:00:00:00:00:06\",\n" \
                    "  \"Comment\" : \"TESTAAA\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif

    //memset(&m, 0, sizeof(m));
    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if(NULL == pkt || 0 == strlen(query_str))
    {
        goto send_pkt;
    }

    if(pkt->json)
    {
        pkt_format = JSON_FORMAT;
    }

    radio_id = get_radio_id(pkt_format, query_str);

    if(pkt->json)
    {
        result &= parse_add_access_control_list_json_cb(query_str, mac, &comment, return_str);
    }

    if(TRUE != result) goto send_pkt;

    /* check */
    if(-1 == radio_id){
        return_str = ERROR_BAD_RADIOID_STR;
        goto send_pkt;
    }
    if(!api_check_mac(mac)){
        return_str = ERROR_MAC_ADDRESS_STR;
        goto send_pkt;
    }
    if(strlen(comment)>16)
    {
        return_str = ERROR_COMMENT_STR;
        goto send_pkt;
    }    

    /* if input has semicolon, remove it 
    if(strlen(mac) == 17){
        j = 0;
        for(i=0; mac[i]!=0; i++){
            if(mac[i] != ':'){
                mac_tmp_input[j] = mac[i];
                j++;
            }
        }
        strcpy(mac, mac_tmp_input);
    }
    */
    sysutil_lower2upper_mac(mac);

    if(radio_id == 0) // 2.4GHZ
    {
        memset(&mac_exist, 0, sizeof(mac_exist));
        api_get_wifi_ifacex_maclist_option(radio_id, mac_exist, sizeof(mac_exist));

    	if (!api_check_redundancy_mac_filterlist(mac_exist, mac)){
            return_str = ERROR_DUPLICATE_MAC_ADDRESS_STR;
            goto send_pkt;
        }

        char *delim = " ";
        num = api_get_list_option_info(delim, mac_exist, maclist, MAC_FILTER_NUM);

        if (num == MAC_FILTER_NUM){
            return_str = ERROR_FULL_TABLE_STR;
            goto send_pkt;
        }
    }
#if SUPPORT_WLAN_5G_SETTING
    else if(radio_id == 1) // 5GHZ
    {
        memset(&mac_exist, 0, sizeof(mac_exist));
        api_get_wifi_ifacex_maclist_option(radio_id, mac_exist, sizeof(mac_exist));

        if (!api_check_redundancy_mac_filterlist(mac_exist, mac)){
            return_str = ERROR_DUPLICATE_MAC_ADDRESS_STR;
            goto send_pkt;
        }

        char *delim = " ";
        num = api_get_list_option_info(delim, mac_exist, maclist, MAC_FILTER_NUM);

        if (num == MAC_FILTER_NUM){
            return_str = ERROR_FULL_TABLE_STR;
            goto send_pkt;
        }
    }
#endif

    /* set value */

    api_add_wifi_ifacex_maclist_option(radio_id, mac, sizeof(mac));

    if(TRUE == result)
    {
        return_str = OK_STR;
    }


    /* Save new token value */
    if(result)
    {
        return_str = REBOOT_STR;
        APPAGENT_SYSTEM("uci commit");
        APPAGENT_SYSTEM("/etc/init.d/network reload &");
    }


send_pkt:
    /* Send response packet */
    send_simple_response(pkt, return_str);

    return 0;
}
/*****************************************************************
* NAME:    delete_access_control_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int delete_access_control_list_cb(HTTPS_CB *pkt){

    char del_action[640] = {0}; // 32 mac address
    char tmp_string[640] = {0};
    char *del_mac;
    bool result = TRUE;
    char *return_str;
    int radio_id = 0;
    int i, j;
    char tok[30] = {0};
    char mac[20] = {0};
    char mac_tmp_input[20] = {0};
    char mac_tmp[20] = {0};
    //macfiltering_t m;
    HTTP_PACKET_CONTENT_FORMAT pkt_format;

#if 0
    char *query_str="{\n" \
                    "  \"RadioID\" : \"2.4GHZ\",\n" \
                    "  \"DeleteMacAddress\" : \"00:00:00:00:00:06\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif

    //memset(&m, 0, sizeof(m));
    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if(NULL == pkt || 0 == strlen(query_str))
    {
        goto send_pkt;
    }

    if(pkt->json)
    {
        pkt_format = JSON_FORMAT;
    }

    radio_id = get_radio_id(pkt_format, query_str);

    if(pkt->json)
    {
        result &= parse_del_access_control_list_json_cb(query_str, del_action, return_str);
    }

    if(TRUE != result) goto send_pkt;

    if(-1 == radio_id){
        return_str = ERROR_BAD_RADIOID_STR;
        goto send_pkt;
    }

    RemoveSpaces(del_action);

    /* check query is smaller than array */
    if(strlen(del_action) >= sizeof(del_action)-1)
    {
        goto send_pkt;
    }
    /*check*/
    if(0 == strcmp(del_action,"DeleteAll")){
    }
    else{
        strcpy(tmp_string, del_action);
        /* check */
        del_mac = strtok(tmp_string, ",");
        while(del_mac != NULL)
        {
            if(!api_check_mac(del_mac)){
                return_str = ERROR_MAC_ADDRESS_STR;
                goto send_pkt;
            }
            del_mac = strtok(NULL, ",");
        }
    }
    /*set*/
    api_delete_wifi_ifacex_maclist_option(radio_id, del_action, sizeof(del_action));

    /* Save new token value */
    if(result)
    {
        return_str = REBOOT_STR;
        APPAGENT_SYSTEM("uci commit");
        APPAGENT_SYSTEM("/etc/init.d/network reload");
    }


send_pkt:
    /* Send response packet */
    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    get_wlan_station_status_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_wlan_station_status_cb(HTTPS_CB *pkt){
	WLAN_STATION_STATUS setting;
	int rssi_value=(-83);
        char opmode[20]={0},ssid[64]={0},encryption[20]={0},status[20]={0};
        char ifname[20]={0},command[128]={0},response[128]={0},command_2[128]={0},response_2[20]={0},command_3[256]={0},response_3[20]={0};
        
        if(NULL == pkt)
                return -1;
        
        //only show 2.4GHz CB mode informations
        api_get_string_option(WIRELESS_WIFI_OPMODE_OPTION,opmode,sizeof(opmode));
        
        if(strcmp(opmode,"sta"))
        {
                send_simple_response(pkt,ERROR_WIFI_DISABLED_STR);
                return 0;
        }
         memset(&setting, 0, sizeof(setting));
        api_get_string_option(WIRELESS_WIFI0_STA_SSID_OPTION,ssid,sizeof(ssid));
        strcpy(setting.ssid,ssid);
	
        api_get_string_option(WIRELESS_WIFI0_STA_ENCRYPTION_OPTION,opmode,sizeof(opmode));
        if(strstr(opmode,"ccmp"))
                strcpy(setting.encryptionType, "AES");
        else if(strstr(opmode,"tkip"))
                strcpy(setting.encryptionType, "TKIP");
        else if(strstr(opmode,"wep"))
                strcpy(setting.encryptionType, "WEP");
	else
                strcpy(setting.encryptionType, "NONE");

        api_get_string_option(WIRELESS_WIFI0_STA_IFNAME_OPTION,ifname,sizeof(ifname));
        sprintf(command,"iwconfig %s | grep \"Access Point:\"| awk '{print $6}'",ifname);
	sysutil_interact(response, sizeof(response),command);

        if(strstr(response,"Not-Associated"))
                strcpy(setting.status,"DISCONNECTED");
        else{
                strcpy(setting.status,"CONNECTED");
                sprintf(command_3,"iwlist %s displayscan | grep \"%s\" |awk \'{print $6}\'",ifname,response);
                sysutil_interact(response_3, sizeof(response_3),command_3);
                rssi_value=atoi(response_3);
        }

        sprintf(command_2,"iwlist %s channel | grep \"Current\" | awk \'{print $2}\'",ifname);
        sysutil_interact(response_2, sizeof(response_2),command_2);
        setting.channel=atoi(response_2);

        if (rssi_value >= -50)
                        setting.signalStrength = 100;
        else if (rssi_value >= -80)/* between -50 ~ -80dbm*/
                        setting.signalStrength = (UINT)(24 + ((rssi_value + 80) * 26)/10);
        else if (rssi_value >= -90)/* between -80 ~ -90dbm*/
                        setting.signalStrength = (UINT)(((rssi_value + 90) * 26)/10);
        else/* < -84 dbm*/
                        setting.signalStrength = 2;//0;
        
        get_wlan_station_status_json_cb(pkt, &setting,OK_STR);
        return 0;
}

/*****************************************************************
* NAME:    get_wlan_sitesurvey_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_wlan_sitesurvey_cb(HTTPS_CB *pkt){
        WLAN_SITESURVEY_T setting[MAX_SITE_SURVEY_INFO_NUM]; 
        char *result=OK_STR;
        char ifname[20]={0},command[128]={0},buf[256]={0},macAddr[32]={0};
        char SSID[32]={0},detSSID[64]={0},mode[10]={0},auth[32]={0},type[20]={0};
	int signalStrength=0,index=0,len=0,channel=0,j=0,k=0;
        FILE *fp;

	if(NULL == pkt)
	    return -1;

        memset(&setting, 0, sizeof(setting));
	APPAGENT_SYSTEM("rm -rf /tmp/displayscan.txt");        
        api_get_string_option(WIRELESS_WIFI0_STA_IFNAME_OPTION,ifname,sizeof(ifname));
        sprintf(command,"iwlist %s displayscan >> /tmp/displayscan.txt &",ifname);
        APPAGENT_SYSTEM(command);
        sleep(1);
        fp = fopen("/tmp/displayscan.txt", "r");

        if(fp == NULL){
             send_simple_response(pkt,ERROR_STR);
             return 0;
        }
        while (fgets(buf, sizeof(buf), fp) != NULL){
                index++;

                if(index < 3)
                        continue;
                if( index > (MAX_SITE_SURVEY_INFO_NUM + 2))
                        break;

                //BSSID           	SSID    LEN        MODE  	CH	SIGNAL	ENC          TYPE
                //AC:86:74:26:CC:14	ADSL   4           Master	1           -90	OPEN      11g/n
		if(sscanf(buf,"%s %s %d %s %d %d %s %s %[^\r\n]",macAddr,SSID,&len,mode,&channel,&signalStrength,auth,type))
                {
			if(!strcmp(SSID,"0"))
			{
				//02:CA:FE:CA:CA:40   	0	Ad-Hoc	1	-85	OPEN	     11g/n
				index--;
				continue;
			}
                       	sysCheckStringOnWeb(SSID, detSSID);
			setting[index-3].channel=channel;
                        strcpy(setting[index-3].ssid,detSSID);
			strcpy(setting[index-3].bssid,macAddr);
#if 0
        "none":"None", 
        "wep-shared":"WEP Shared", 
        "wep-open":"WEP Open", 
        "psk+tkip":"WPA/PSK TKIP", 
        "psk+ccmp":"WPA/PSK AES", 
        "psk+tkip+ccmp":"WPA/PSK TKIP+AES", 
        "psk2+tkip":"WPA2/PSK TKIP", 
        "psk2+ccmp":"WPA2/PSK AES", 
        "psk2+tkip+ccmp":"WPA2/PSK TKIP+AES", 
        "psk-mixed+tkip+ccmp":"WPA/WPA2-PSK TKIP+AES", 
        "psk-mixed+tkip":"WPA/WPA2-PSK TKIP", 
        "psk-mixed+ccmp":"WPA/WPA2-PSK AES", 
        "wpa+tkip+ccmp":"WPA TKIP+AES", 
        "wpa+tkip":"WPA TKIP", 
        "wpa+ccmp":"WPA AES", 
        "wpa2+tkip+ccmp":"WPA2 TKIP+AES", 
        "wpa2+tkip":"WPA2 TKIP", 
        "wpa2+ccmp":"WPA2 AES", 
        "wpa-mixed+tkip+ccmp":"WPA/WPA2 TKIP+AES", 
        "wpa-mixed+tkip":"WPA/WPA2 TKIP", 
        "wpa-mixed+ccmp":"WPA/WPA2 AES"
#endif
			if(!strcmp(auth,"WEP"))
                        {
                                strcpy(setting[index-3].authType,"WEP");
				strcpy(setting[index-3].encryptionType,"Shared");
                        }
 			if(!strcmp(auth,"WPA/WPA2-PSK"))
                        {
                                strcpy(setting[index-3].authType,"WPAPSKWPA2PSK");
				strcpy(setting[index-3].encryptionType,"TKIPSAES");
                        }
 			if(!strcmp(auth,"WPA-PSK"))
                        {
                                strcpy(setting[index-3].authType,"WPAPSK");
				strcpy(setting[index-3].encryptionType,"TKIP");
                        }
 			if(!strcmp(auth,"WPA2-PSK"))
                        {
                                strcpy(setting[index-3].authType,"WPA2PSK");
				strcpy(setting[index-3].encryptionType,"AES");
                        }
 			if(!strcmp(auth,"mixed WPA/WPA2 - PSK"))
                        {
                                strcpy(setting[index-3].authType,"WPAPSKWPA2PSK");
				strcpy(setting[index-3].encryptionType,"TKIPSAES");
                        }
			else
                        {
                                strcpy(setting[index-3].authType,"OPEN");
                                strcpy(setting[index-3].encryptionType,"NONE");
                        }

			if (signalStrength >= -50)
				setting[index-3].signalStrength = 100;
			else if (signalStrength >= -80)/* between -50 ~ -80dbm*/
				setting[index-3].signalStrength = (UINT)(24 + ((signalStrength + 80) * 26)/10);
			else if (signalStrength >= -90)/* between -80 ~ -90dbm*/
				setting[index-3].signalStrength = (UINT)(((signalStrength + 90) * 26)/10);
			else/* < -84 dbm*/
				setting[index-3].signalStrength = 2;//0;

			//this for loop translate 11b/g/n to 11bgn
			for(j=0;j<sizeof(type);j++){
				if(type[j]!='\\'&& type[j]!='/'){
					setting[index-3].wlanMode[k]=type[j];
					k++;
				}
			}
			k=0;
	        }
        }
        fclose(fp);
        get_wlan_site_survey_json_cb(pkt, &setting, result,(index-3));
	return 0;
}

int set_wlan_connection_cb(HTTPS_CB *pkt)
{
        bool result = TRUE;
        char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");    
        char *return_str=OK_STR; 
	WLAN_CONNECT_SETTINGS_T settings;
        HTTP_PACKET_CONTENT_FORMAT pkt_format;
        char opmode[20]={0},key_option[32]={0},tmp[30]={0};
        
        if((NULL == pkt) || (NULL == query_str))
		send_simple_response(pkt,ERROR_STR);
        
        //only show 2.4GHz CB mode informations
        api_get_string_option(WIRELESS_WIFI_OPMODE_OPTION,opmode,sizeof(opmode));
        
        if(strcmp(opmode,"sta"))
        {
                send_simple_response(pkt,ERROR_WIFI_DISABLED_STR);
                return 0;
        }
        memset(&settings, 0, sizeof(settings));
	get_json_string_from_query(query_str, tmp, "SSID");
	get_json_string_from_query(query_str, settings.ssid, "SSID");
        get_json_string_from_query(query_str, settings.bssid, "BSSID");
        get_json_string_from_query(query_str, settings.encryptionType, "EncryptionType");
        get_json_string_from_query(query_str, settings.authType, "AuthType");
        get_json_integer_from_query(query_str,&settings.keyIndex, "KeyIndex");
        get_json_string_from_query(query_str, settings.password, "Password");

        api_set_string_option(WIRELESS_WIFI0_STA_SSID_OPTION,settings.ssid,sizeof(settings.ssid));
        api_set_string_option(WIRELESS_WIFI0_STA_KEY_OPTION,settings.password,sizeof(settings.password));
        api_set_integer_option(WIRELESS_WIFI0_STA_TC_DOWNPERUSER_OPTION,0);
        api_set_integer_option(WIRELESS_WIFI0_STA_TC_UPPERUSER_OPTION,0);
        
        if(!strcmp(settings.encryptionType,"WPA-PSK"))//WPA + TKIP
        {
                api_set_string_option(WIRELESS_WIFI0_STA_ENCRYPTION_OPTION,"wpa+tkip",sizeof("wpa+tkip"));
                api_set_string_option(WIRELESS_WIFI0_STA_KEY_OPTION,settings.password,sizeof(settings.password));
        }
        if(!strcmp(settings.encryptionType,"WPA2-PSK"))//WPA2 + AES
        {
                api_set_string_option(WIRELESS_WIFI0_STA_ENCRYPTION_OPTION,"wpa2+ccmp",sizeof("wpa2+ccmp"));
                api_set_string_option(WIRELESS_WIFI0_STA_KEY_OPTION,settings.password,sizeof(settings.password));
        }        
      
        if(!strcmp(settings.encryptionType,"WEP"))
        {
                if(!strcmp(settings.authType,"OPEN"))
                        api_set_string_option(WIRELESS_WIFI0_STA_ENCRYPTION_OPTION,"wep-open",sizeof("wep-open"));
                else
                        api_set_string_option(WIRELESS_WIFI0_STA_ENCRYPTION_OPTION,"wep-shared",sizeof("wep-shared"));

                api_set_integer_option(WIRELESS_WIFI0_STA_KEY_ID,settings.keyIndex);
                sprintf(key_option,"wireless.wifi0_sta.key%d",settings.keyIndex);
                api_set_string_option(key_option,settings.password,sizeof(settings.password));
        }

        if(!strcmp(settings.encryptionType, "DISABLE"))
                api_set_string_option(WIRELESS_WIFI0_STA_ENCRYPTION_OPTION,"none",sizeof("none"));

	APPAGENT_SYSTEM("uci commit &");
	APPAGENT_SYSTEM("luci-reload auto &");
        send_simple_response(pkt,return_str);
        return 0;
}

int get_wifiinfo_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char buf[64];

    if(NULL == pkt)
        return -1;

    get_json_wifiinfo_cb(pkt, OK_STR);

    return 0;
}

int set_wifiinfo_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");    
    char *return_str=OK_STR;
    int guest_switch=1;
    char buf[128]={0};
    char key[128]={0};
    char guest_key[128]={0};

    if(NULL == pkt || 0 == strlen(query_str))
    {
        send_simple_response(pkt,ERROR_STR);
    }

	memset(buf, 0x00, sizeof(buf));
	memset(key, 0x00, sizeof(key));
	memset(guest_key, 0x00, sizeof(guest_key));
	get_json_string_from_query(query_str, buf, "SSID");
	get_json_string_from_query(query_str, key, "Password");
	get_json_string_from_query(query_str, guest_key, "GuestNetworkPassword");

#if HAS_WLAN_5G_2_SETTING
	api_set_string_option("wireless.wifi2_ssid_1.ssid",buf,sizeof(buf));
#endif
	api_set_string_option("wireless.wifi1_ssid_1.ssid",buf,sizeof(buf));
	api_set_string_option("wireless.wifi0_ssid_1.ssid",buf,sizeof(buf));

	get_json_integer_from_query(query_str,&guest_switch, "GuestNetworkSwitch");
	guest_switch = (1 == guest_switch)?0:1;
	api_set_integer_option("wireless.wifi0_guest.disabled", guest_switch);
	api_set_integer_option("wireless.wifi1_guest.disabled", guest_switch);
	api_set_integer_option("dhcp.guest.ignore", guest_switch);
	// api_set_integer_option("dhcp.guest2.ignore", guest_switch);
#if HAS_WLAN_5G_2_SETTING
	api_set_integer_option("wireless.wifi2_guest.disabled", guest_switch);
	//api_set_integer_option("dhcp.guest3.ignore", guest_switch);
#endif

	memset(buf, 0x00, sizeof(buf));
	get_json_string_from_query(query_str, buf, "GuestNetworkSSID");
#if HAS_WLAN_5G_2_SETTING
	api_set_string_option("wireless.wifi2_guest.ssid",buf,sizeof(buf));
#endif
	api_set_string_option("wireless.wifi1_guest.ssid",buf,sizeof(buf));
	api_set_string_option("wireless.wifi0_guest.ssid",buf,sizeof(buf));

    //2.4G
    memset(buf, 0x00, sizeof(buf));
	get_json_string_from_query(query_str, buf, "Wifi24gBandwith");
	if(strlen(buf))
	{
		api_set_string_option("wireless.wifi0.htmode",buf,sizeof(buf));
	}
	memset(buf, 0x00, sizeof(buf));
	get_json_string_from_query(query_str, buf, "Wifi24gChannel");
	if(strlen(buf))
	{
#if SUPPORT_MESH_AUTO_CHAN
		api_set_string_option("wireless.wifi0.channel", (0 == strcmp(buf,"0"))?"auto":buf, sizeof(buf));
#else
		api_set_string_option("wireless.wifi0.channel", buf, sizeof(buf));
#endif
	}
    memset(buf, 0x00, sizeof(buf));
	get_json_string_from_query(query_str, buf, "Wifi24gEncrType");

	if(strlen(buf))
	{
		api_set_integer_option("wireless.wifi0_ssid_1.encrDisabled", (0 == strcmp(buf, "none"))?1:0);
		api_set_string_option("wireless.wifi0_ssid_1.encryption",buf,sizeof(buf));
		api_set_string_option("wireless.wifi0_ssid_1.encryptionFake1",buf,sizeof(buf));
		if(strcmp(buf,"none") != 0)
		{
			api_set_string_option("wireless.wifi0_ssid_1.key",key,sizeof(key));
		}
        api_set_integer_option("wireless.wifi0_ssid_1.fastroamingEnable", (0 == strcmp(buf, "none"))?0:1);
	}
	else
	{
		api_set_string_option("wireless.wifi0_ssid_1.encryption","psk2+ccmp",strlen("psk2+ccmp"));
		api_set_string_option("wireless.wifi0_ssid_1.encryptionFake","psk2+ccmp",strlen("psk2+ccmp"));

		api_set_string_option("wireless.wifi0_ssid_1.key",key,sizeof(key));
	}
	//2.4G Guest
    memset(buf, 0x00, sizeof(buf));
	get_json_string_from_query(query_str, buf, "Wifi24gGuestEncrType");

	if(strlen(buf))
	{
		api_set_integer_option("wireless.wifi0_guest.encrDisabled", (0 == strcmp(buf, "none"))?1:0);
		api_set_string_option("wireless.wifi0_guest.encryption",buf,sizeof(buf));

		if(strcmp(buf,"none") != 0)
		{
			api_set_string_option("wireless.wifi0_guest.key",guest_key,sizeof(guest_key));
		}
	}
	else
	{
		api_set_string_option("wireless.wifi0_guest.key",guest_key,sizeof(guest_key));
	}

    //5G
    memset(buf, 0x00, sizeof(buf));
	get_json_string_from_query(query_str, buf, "Wifi5gBandwith");
	if(strlen(buf))
	{
		api_set_string_option("wireless.wifi1.htmode",buf,sizeof(buf));
	}
    memset(buf, 0x00, sizeof(buf));
	get_json_string_from_query(query_str, buf, "Wifi5gChannel");
	if(strlen(buf))
	{
#if SUPPORT_MESH_AUTO_CHAN
		api_set_string_option("wireless.wifi1.channel", (0 == strcmp(buf,"0"))?"auto":buf, sizeof(buf));
#else
		api_set_string_option("wireless.wifi1.channel", buf, sizeof(buf));
#endif
	}
    memset(buf, 0x00, sizeof(buf));
	get_json_string_from_query(query_str, buf, "Wifi5gEncrType");
	if(strlen(buf))
	{
		api_set_integer_option("wireless.wifi1_ssid_1.encrDisabled", (0 == strcmp(buf, "none"))?1:0);
		api_set_string_option("wireless.wifi1_ssid_1.encryption",buf,sizeof(buf));
		api_set_string_option("wireless.wifi1_ssid_1.encryptionFake",buf,sizeof(buf));
		if(strcmp(buf,"none") != 0)
		{
			api_set_string_option("wireless.wifi1_ssid_1.key",key,sizeof(key));
		}
        api_set_integer_option("wireless.wifi1_ssid_1.fastroamingEnable", (0 == strcmp(buf, "none"))?0:1);
	}
	else
	{
		api_set_string_option("wireless.wifi1_ssid_1.encryption","psk2+ccmp",strlen("psk2+ccmp"));
		api_set_string_option("wireless.wifi1_ssid_1.encryptionFake","psk2+ccmp",strlen("psk2+ccmp"));
		api_set_string_option("wireless.wifi1_ssid_1.key",key,sizeof(key));
	}
	//5G Guest
    memset(buf, 0x00, sizeof(buf));
	get_json_string_from_query(query_str, buf, "Wifi5gGuestEncrType");
	if(strlen(buf))
	{
		api_set_integer_option("wireless.wifi1_guest.encrDisabled", (0 == strcmp(buf, "none"))?1:0);
		api_set_string_option("wireless.wifi1_guest.encryption",buf,sizeof(buf));

		if(strcmp(buf,"none") != 0)
		{
			api_set_string_option("wireless.wifi1_guest.key",guest_key,sizeof(guest_key));
		}
	}
	else
	{
		api_set_string_option("wireless.wifi1_guest.key",guest_key,sizeof(guest_key));
	}

#if HAS_WLAN_5G_2_SETTING
	//5G-2
	memset(buf, 0x00, sizeof(buf));
	get_json_string_from_query(query_str, buf, "Wifi5g2Bandwith");
	if(strlen(buf))
	{
		api_set_string_option("wireless.wifi2.htmode",buf,sizeof(buf));
	}
	memset(buf, 0x00, sizeof(buf));
	get_json_string_from_query(query_str, buf, "Wifi5g2Channel");
	if(strlen(buf))
	{
#if SUPPORT_MESH_AUTO_CHAN
		api_set_string_option("wireless.wifi2.channel",(0 == strcasecmp("0", buf))?"auto":buf,sizeof(buf));
#else
		api_set_string_option("wireless.wifi2.channel",buf,sizeof(buf));
#endif
	}
	memset(buf, 0x00, sizeof(buf));
	get_json_string_from_query(query_str, buf, "Wifi5g2EncrType");
	if(strlen(buf))
	{
		api_set_integer_option("wireless.wifi2_ssid_1.encrDisabled", (0 == strcmp(buf,"none"))?1:0);
		api_set_string_option("wireless.wifi2_ssid_1.encryption",buf,sizeof(buf));
		api_set_string_option("wireless.wifi2_ssid_1.encryptionFake",buf,sizeof(buf));
		if(0 != strcmp(buf,"none"))
		{
			api_set_string_option("wireless.wifi2_ssid_1.key",key,sizeof(key));
		}
        api_set_integer_option("wireless.wifi2_ssid_1.fastroamingEnable", (0 == strcmp(buf, "none"))?0:1);
	}
	else
	{
		api_set_string_option("wireless.wifi2_ssid_1.encryption","psk2+ccmp",strlen("psk2+ccmp"));
		api_set_string_option("wireless.wifi2_ssid_1.encryptionFake","psk2+ccmp",strlen("psk2+ccmp"));
		api_set_string_option("wireless.wifi2_ssid_1.key",key,sizeof(key));
	}
	//5G-2 Guest
	memset(buf, 0x00, sizeof(buf));
	get_json_string_from_query(query_str, buf, "Wifi5g2GuestEncrType");
	if(strlen(buf))
	{
		api_set_integer_option("wireless.wifi2_guest.encrDisabled", (0 == strcmp(buf,"none"))?1:0);
		api_set_string_option("wireless.wifi2_guest.encryption",buf,sizeof(buf));

		if(0 != strcmp(buf,"none"))
		{
			api_set_string_option("wireless.wifi2_guest.key",guest_key,sizeof(guest_key));
		}
	}
	else
	{
		api_set_string_option("wireless.wifi2_guest.key",guest_key,sizeof(guest_key));
	}
#endif

    APPAGENT_SYSTEM("luci-reload auto wireless dhcp &");

	send_simple_response(pkt, OK_STR);

    return 0;
}

int get_wifioptlist_cb(HTTPS_CB *pkt)
{
	char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
	if(NULL == pkt)
	{
		return -1;
	}

	get_json_wifioptlist_cb(pkt, OK_STR);

	return 0;
}
/*****************************************************************
* NAME:    kick_wireless_client_by_mac_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int kick_wireless_client_by_mac_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *return_str;
    char mac[32];
    char buf[64];
    int iv;

    iv = 0;
    return_str = ERROR_STR;
    memset(mac, 0x00, sizeof(mac));
    memset(buf, 0x00, sizeof(buf));

    get_json_string_from_query(query_str, mac, "MAC");

    if(0 != strlen(mac))
    {
        sysutil_interact(buf, sizeof(buf), "wlanconfig %s list sta | grep \"%s\"", WIFI_24G_IF, mac);

        if(0 != strlen(buf))
        {
            APPAGENT_SYSTEM("iwpriv %s kickmac %s", WIFI_24G_IF, mac);
        }

        memset(buf, 0x00, sizeof(buf));
        sysutil_interact(buf, sizeof(buf), "wlanconfig %s list sta | grep \"%s\"", WIFI_5G_IF, mac);

        if(0 != strlen(buf))
        {
            APPAGENT_SYSTEM("iwpriv %s kickmac %s", WIFI_5G_IF, mac);
        }

        return_str = OK_STR;
    }

    send_simple_response(pkt, return_str);

    return 0;
}

int set_countrycode_cb(HTTPS_CB *pkt)
{
	char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
	char *return_str;
	char country[6]={0};
	char *reloadwifi_str="/tmp/reloadwifichan.sh";
	char disable_band[6]={0}, domain[4]={0};
	char fcc_dfsCertified[6]={0}, etsi_dfsCertified[6]={0}, int_dfsCertified[6]={0};
	char ori_wifi1_country[6]={0}, ori_wifi1_disable_band[6]={0}, ori_wifi1_chan[6]={0};
#if HAS_WLAN_5G_2_SETTING
	char ori_wifi2_country[6]={0}, ori_wifi2_disable_band[6]={0}, ori_wifi2_chan[6]={0};
	int highBandCountry=0;
	char split_low[6]={0}, split_high[6]={0};
#endif
	int greenmode=1, outdoor=0, dfsCertified=0;
	FILE *fp;

	if(NULL == pkt || 0 == strlen(query_str))
	{
		return_str = ERROR_STR;
	}
	else
	{
		return_str = OK_STR;
		api_get_string_option("wireless.wifi1.country", ori_wifi1_country, sizeof(ori_wifi1_country));
		sysutil_interact(ori_wifi1_disable_band, sizeof(ori_wifi1_disable_band), "iwpriv wifi1 g_disable_band | awk -F\":\" {'printf $2'}");
		//iwlist ath1 chan | awk '/Current/{print $2}'
		sysutil_interact(ori_wifi1_chan, sizeof(ori_wifi1_chan), "iwlist ath1 chan | awk '/Current/{printf $2}'");
#if HAS_WLAN_5G_2_SETTING
		api_get_string_option("wireless.wifi2.country", ori_wifi2_country, sizeof(ori_wifi2_country));
		sysutil_interact(ori_wifi2_disable_band, sizeof(ori_wifi2_disable_band), "iwpriv wifi2 g_disable_band | awk -F\":\" {'printf $2'}");
		sysutil_interact(ori_wifi2_chan, sizeof(ori_wifi2_chan), "iwlist ath4 chan | awk '/Current/{printf $2}'");
#endif

		get_json_string_from_query(query_str, country, "Country");
		APPAGENT_SYSTEM("uci set wireless.wifi0.country='%s'", country);
		APPAGENT_SYSTEM("uci set wireless.wifi1.country='%s'", country);
#if HAS_WLAN_5G_2_SETTING
		APPAGENT_SYSTEM("uci set wireless.wifi2.country='%s'", country);
		api_get_string_option("wireless.wifi1.split_band", split_high, sizeof(split_high));
		api_get_string_option("wireless.wifi2.split_band", split_low, sizeof(split_low));
		if(atoi(country) == 360 || atoi(country) == 458 || atoi(country) == 586 || atoi(country) == 634)
		{
			highBandCountry=1;
		}
#endif
		sysutil_interact(domain, sizeof(domain), "setconfig -g 4");

		api_get_string_option("sysProductInfo.model.fccDfsCertified", fcc_dfsCertified, sizeof(fcc_dfsCertified));
		api_get_string_option("sysProductInfo.model.etsiDfsCertified", etsi_dfsCertified, sizeof(etsi_dfsCertified));
		api_get_string_option("sysProductInfo.model.intDfsCertified", int_dfsCertified, sizeof(int_dfsCertified));
		api_get_integer_option("sysProductInfo.model.outdoor", &outdoor);

		dfsCertified=(atoi(domain)==0) ? atoi(fcc_dfsCertified) : (atoi(domain)==1) ? atoi(etsi_dfsCertified) : atoi(int_dfsCertified);
		sysutil_interact(disable_band, sizeof(disable_band), "/bin/sh /lib/wifi/RegularDomain.sh %s %d %d %d %d | awk {'printf $1'}", country, atoi(domain), greenmode, outdoor, dfsCertified);

		fp = fopen(reloadwifi_str, "w");
		if(fp)
		{
			fprintf(fp, "iwpriv wifi0 disable_band %d\n", atoi(disable_band));
			fprintf(fp, "iwpriv wifi0 setCountryID %s\n", country);
			fprintf(fp, "max_htmode=$(getHTModeList.sh 0 | awk -F',' {'print $NF'})\n");
			fprintf(fp, "uci set wireless.wifi0.htmode=$max_htmode\n");
			fprintf(fp, "echo \"wifi0 htmode[$max_htmode]\" > /dev/console\n");
			fprintf(fp, "sleep 1\n");
#if HAS_WLAN_5G_2_SETTING
			if((atoi(disable_band) & 12) == 0)
			{
				fprintf(fp, "uci set wireless.wifi1.nochannel=\"1\"\n");
			}
			else
			{
				fprintf(fp, "iwpriv wifi1 disable_band %d\n", (atoi(disable_band) & 12));
			}
#else
			fprintf(fp, "iwpriv wifi1 disable_band %d\n", atoi(disable_band));
#endif
			fprintf(fp, "iwpriv wifi1 setCountryID %s\n", country);
			fprintf(fp, "max_htmode1=$(getHTModeList.sh 1 | awk -F',' {'print $NF'})\n");
#if HAS_WLAN_5G_2_SETTING
			fprintf(fp, "sleep 1\n");
			if((atoi(disable_band) & 3) == 0)
			{
				fprintf(fp, "uci set wireless.wifi2.nochannel=\"1\"\n");
			}
			else
			{
				fprintf(fp, "iwpriv wifi2 disable_band %d\n", (atoi(disable_band) & 3));
			}
			fprintf(fp, "iwpriv wifi2 setCountryID %s\n", country);
			fprintf(fp, "max_htmode2=$(getHTModeList.sh 2 | awk -F',' {'print $NF'})\n");
			fprintf(fp, "[ -z \"$(iwlist ath1 chan | grep Current)\" ] && max_htmode1=$max_htmode2\n");
			fprintf(fp, "[ -z \"$(iwlist ath4 chan | grep Current)\" ] && max_htmode2=$max_htmode1\n");
			fprintf(fp, "uci set wireless.wifi2.htmode=$max_htmode2\n");
			fprintf(fp, "echo \"wifi2 htmode[$max_htmode2]\" > /dev/console\n");
#endif
			fprintf(fp, "echo \"wifi1 htmode[$max_htmode1]\" > /dev/console\n");
			fprintf(fp, "uci set wireless.wifi1.htmode=$max_htmode1\n");
			fprintf(fp, "uci set wireless.wifi0.channel='auto'\n");
			fprintf(fp, "uci set wireless.wifi1.channel='auto'\n");
#if HAS_WLAN_5G_2_SETTING
			fprintf(fp, "uci set wireless.wifi2.channel='auto'\n");
#endif
			//restore original country and disable_band for WDS connection
			fprintf(fp, "iwpriv wifi1 disable_band %s\n", ori_wifi1_disable_band);
			fprintf(fp, "iwpriv wifi1 setCountryID %s\n", ori_wifi1_country);
			fprintf(fp, "[ -n \"%s\" ] && iwconfig ath1 chan %s\n", ori_wifi1_chan, (strlen(ori_wifi1_chan))?ori_wifi1_chan:"0");
#if HAS_WLAN_5G_2_SETTING
			fprintf(fp, "iwpriv wifi2 disable_band %s\n", ori_wifi2_disable_band);
			fprintf(fp, "iwpriv wifi2 setCountryID %s\n", ori_wifi2_country);
			fprintf(fp, "[ -n \"%s\" ] && iwconfig ath4 chan %s\n", ori_wifi2_chan, (strlen(ori_wifi2_chan))?ori_wifi2_chan:"0");
			if(highBandCountry)
			{
				if(atoi(split_low) == 1 || atoi(split_high) == 8)
				{
					fprintf(fp, "uci set wireless.wifi1.split_band=8\n");
					fprintf(fp, "uci set wireless.wifi2.split_band=3\n");
				}
				fprintf(fp, "[ \"ath37\" == \"$(getinfo mesh_ifname)\" ] && echo ath37 > /tmp/ori_mesh_ifname\n");
				fprintf(fp, "uci set wireless.wifi1_mesh.disabled=0\n");
				fprintf(fp, "uci set wireless.wifi2_mesh.disabled=1\n");
			}
			else
			{
				if(atoi(split_low) == 1 || atoi(split_high) == 8)
				{
					fprintf(fp, "uci set wireless.wifi1.split_band=12\n");
					fprintf(fp, "uci set wireless.wifi2.split_band=1\n");
				}
				fprintf(fp, "[ \"ath35\" == \"$(getinfo mesh_ifname)\" ] && echo ath35 > /tmp/ori_mesh_ifname\n");
				fprintf(fp, "uci set wireless.wifi1_mesh.disabled=1\n");
				fprintf(fp, "uci set wireless.wifi2_mesh.disabled=0\n");
			}
#endif
			fprintf(fp, "[ -e \"/tmp/mesh_sync_client_md5\" ] && rm /tmp/mesh_sync_client_md5\n");
			fprintf(fp, "luci-reload auto wireless\n");
			fclose(fp);
		}
		if(sysutil_check_file_existed(reloadwifi_str))
		{
			APPAGENT_SYSTEM("sh %s &", reloadwifi_str);
		}
	}

	send_simple_response(pkt, return_str);
	return 0;
}
