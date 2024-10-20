/************************************************************************************************
 * Copyright (c) 2006-2009 Laboratorio di Sistemi di Elaborazione e Bioingegneria Informatica	*
 *                          Universita' Campus BioMedico - Italy								*
 *																								*
 * This program is free software; you can redistribute it and/or modify it under the terms		*
 * of the GNU General Public License as published by the Free Software Foundation; either		*
 * version 2 of the License, or (at your option) any later version.								*
 *																								*
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY				*
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A				*
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.						*
 *																								*
 * You should have received a copy of the GNU General Public License along with this			*
 * program; if not, write to the:																*
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,							*
 * MA  02111-1307, USA.																			*
 *																								*
 * -------------------------------------------------------------------------------------------- *
 * Project:  Capwap
 * Authors : Matteo Latini (mtylty@gmail.com)
 * 	     Donato Capitella (d.capitella@gmail.com)											*
 ************************************************************************************************/


#ifndef __CAPWAP_VendorPayloads__
#define __CAPWAP_VendorPayloads__

/***********************************************************************
 * Vendor specific payloads types
 * *********************************************************************/
//#define	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_UCI		1
//#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WUM		2

/* SENAO Vendor specific Payload */
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG				          1000
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG_RESULT		          1001
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WAITING_TIME                   1002
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CURRENT_STATION_INFO           1003
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WLAN_STATISTICS                1004
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SYSTEM_UPTIME		          1005
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_UNUSED1      		          1006
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_IP_CFG_INFO			          1007
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CAPWAP_TIMER			          1008
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC              1009
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC_INTERVAL	  1010
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_DEBUG_LOG                	  1011
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC_MAX_CLIENTS  1012
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CFG_CAP_INFO               1013
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_CURRENT_CFG_INFO           1014
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SITESURVEY			          1015
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SITESURVEY_RESULT              1016
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_PROXY_WTP_ADDRESS              1017
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_PROXY_RESPONSE_WTP_INFO        1018
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_PROXY_AC_ADDRESS               1019
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_RADIO_CHANNEL                  1020
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_KICKMAC						  1021
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_KICKMAC_RESULT				  1022
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_PORT_INFO				  1023
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_POE_INFO				  1024
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_TOPOLOGY				  1025
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_BACKGROUND_SITESURVEY          1026
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_WTP_PACKET_INTERVEL            1027
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_AUTO_TXPOWER                   1028
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_AUTO_TXPOWER_RESPONSE          1029
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SHELL_CMD                      1030
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SHELL_CMD_OUTPUT               1031
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MEMORY_INFO                    1032
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MEMORY_LOG_THRESHOLD           1033
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_STATION_STATISTIC_POLL_INTERVAL 1034
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_CLIENT_STATE_CHANGE_EVENT_ENABLE 1035
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_SWITCH_TRUNK_INFO		      1036
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MTU                            1037
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_ALLPY_CONFIG_INFO              1038
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_MESH_INFO                      1039
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_UTC_TIME                       1040
#define 	CW_MSG_ELEMENT_VENDOR_SPEC_PAYLOAD_AC_CFG_CAP_INFO                1041

/*************************************************************************
 *  SENAO WTP Cfg Messages
 *************************************************************************/

typedef struct
{
    int type;
    char *name;
    CWErrorCode(*getMsg)(int keyLen, void *keyPtr, int *valLen, void **valPtr);
    CWErrorCode(*setMsg)(int keyLen, void *keyPtr, int valLen, void *valPtr);
    CWErrorCode(*getNextKey)(int keyLen, void *keyPtr, int *nextkeyLen, void **nextkeyPtr);
    CWErrorCode(*getMsgDefault)(int keyLen, void *keyPtr, int *valLen, void **valPtr);
} CWWtpCfgMsgInfo;


#define CFG_MSG_INFO_ENTRY_START() {WTP_CFG_NONE, NULL, NULL, NULL, NULL}
#define CFG_MSG_INFO_ENTRY_END(wtpType) {WTP_CFG_##wtpType##_END, NULL, NULL, NULL, NULL}

#define CW_CFG_MSG_AP_INFO \
{ \
	CFG_MSG_INFO_ENTRY_START(), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_NAME, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LOCATION, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_IPV4, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_NUM, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_OPERATION_MODE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_WIRELESS_MODE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_COUNTRY_CODE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_CHANNEL_HT_MODE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_CHANNEL_EXT, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_CHANNEL, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_LIMITED_CLIENTS_ENABLE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_LIMITED_CLIENTS, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_TX_POWER, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_AGGRE_ENABLE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_AGGRE_FRAMES, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_AGGRE_MAXBYTES, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_WLAN_NUM, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_SSID, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_SUPPRESSED_SSID, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_STA_SEPARATION, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_ISOLATION, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_VLAN, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_SECURITY_MODE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_WEP_AUTH_TYPE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_WEP_KEY_LENGTH, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_WEP_DEF_KEY_ID, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_WEP_KEY, CWCfgMsgGetWepKeyNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_WPA_ENCRYPT_MODE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_WPA_PASSPHRASE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_WPA_GROUP_KEY_INT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_RADIUS_ADDR, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_RADIUS_PORT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_RADIUS_SECRET, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_RADIUS_ACC_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_RADIUS_ACC_ADDR, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_RADIUS_ACC_PORT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_RADIUS_ACC_SECRET, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_ACL_MODE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_ACL_MAC_LIST, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_DNS1, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_DNS2, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_ADMIN, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_PASSWORD_MD5, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_DATA_RATE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_RTSCTS_THRESHOLD, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_RADIUS_ACC_INTERIM_INT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_OBEY_REGULATORY_POWER, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_WEP_INPUT_METHOD, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_BAND_STREERING, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_FAST_HANDOVER_STATUS, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_FAST_HANDOVER_RSSI, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_DOWNLOAD_LIMIT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_UPLOAD_LIMIT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_ROAMING_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_ROAMING_ADV_SEARCH, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_GUEST_NETWORK_IP, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_GUEST_NETWORK_MASK, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_GUEST_NETWORK_DHCP_START, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_GUEST_NETWORK_DHCP_END, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_GUEST_NETWORK_WINS_SERVER, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_DISTANCE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LED_POWER, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LED_LAN, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LED_WLAN0, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LED_WLAN1, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LED_MESH, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_LAYER2_ISOLATION, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_ENABLE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_LOGIN_TYPE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_RADIUS_SERVER, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_REDIRECT_BEHAVIOR, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_WALLEDGARDEN, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_WALLEDGARDEN_PAGE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_RADIUS_PORT, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_RADIUS_SECRET, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_SESSION_TIMEOUT, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_SETIMEOUT_ENABLE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_IDLE_TIMEOUT, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_IDTIMEOUT_ENABLE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_ENABLE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_TIME, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_AUTH_TYPE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_EXTERNAL_SERVER, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_EXTERNAL_SECRET, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_SERVER, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_PORT, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_ACCOUNTING_SECRET, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_MANAGEMENT_VLAN_ID, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_AC_LIST, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_UAMFORMAT, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_LOCAL_AUTH, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LAN_PORT_NUM, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LAN_PORT_ENABLE, CWCfgMsgGetLanPortNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LAN_PORT_VLAN_ID, CWCfgMsgGetLanPortNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_PORT, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_HTTPS_ENABLE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_RADIUS_SECRET2, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_PORTAL_RADIUS_SERVER2, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_BANDSTREERING_MODE, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_BANDSTREERING_PERCENT_ENABLE, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_BANDSTREERING_RSSI_ENABLE, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_BANDSTREERING_RSSI, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_BANDSTREERING_PERCENT, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_NAS_ID_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_NAS_ID, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_NAS_PORT_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_NAS_PORT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LAN_PORT_VLAN_MODE, CWCfgMsgGetLanPortNextKey), \
    CFG_MSG_INFO_ENTRY(WTP_CFG_AP_MESH_ENABLE_TOTAL, NULL), \
    CFG_MSG_INFO_ENTRY(WTP_CFG_AP_MESH_ENABLE, CWCfgMsgGetRadioNextKey), \
    CFG_MSG_INFO_ENTRY(WTP_CFG_AP_MESH_ID, CWCfgMsgGetRadioNextKey), \
    CFG_MSG_INFO_ENTRY(WTP_CFG_AP_MESH_WPA_KEY, CWCfgMsgGetRadioNextKey), \
    CFG_MSG_INFO_ENTRY(WTP_CFG_AP_MESH_LINK_ROBUST_THRESHOLD, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_NAS_IP_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_NAS_IP, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_WPA_PMF_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_DOWNLOAD_PERUSER_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_UPLOAD_PERUSER_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_FAST_HANDOVER_STATUS, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_FAST_HANDOVER_RSSI, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_NETRORK_MODE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_LOGIN_TYPE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_RADIUS_SERVER, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_REDIRECT_BEHAVIOR, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_WALLEDGARDEN, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_WALLEDGARDEN_PAGE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_RADIUS_PORT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_RADIUS_SECRET, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_SESSION_TIMEOUT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_SETIMEOUT_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_IDLE_TIMEOUT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_IDTIMEOUT_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_TIME, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_AUTH_TYPE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_EXTERNAL_SERVER, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_EXTERNAL_SECRET, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_SERVER, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_PORT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_ACCOUNTING_SECRET, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_PORT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_HTTPS_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_RADIUS_SECRET2, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_RADIUS_SERVER2, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_UAMFORMAT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_PORTAL_LOCAL_AUTH, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_BANDSTREERING_MODE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_BANDSTREERING_PERCENT_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_BANDSTREERING_RSSI_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_BANDSTREERING_RSSI, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_BANDSTREERING_PERCENT, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LED_WLAN2, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LOG_REMOTE_SERVER_ENABLE, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LOG_REMOTE_SERVER_CONFIG, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_TIME_ZONE, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_LOG_TRAFFIC_ENABLE, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_DOWNLOAD_LIMIT_MODE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_UPLOAD_LIMIT_MODE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_L2_ISOLATION_WHITE_MAC_LIST, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_BIT_RATE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_RADIO_AX_ENABLE, CWCfgMsgGetRadioNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_HOTSPOT20_JSON, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_AP_WLAN_SUITEB_ENABLE, CWCfgMsgGetWlanNextKey), \
	CFG_MSG_INFO_ENTRY_END(AP) \
}

#define CW_CFG_MSG_SW_INFO \
{ \
	CFG_MSG_INFO_ENTRY_START(), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_NAME, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_LOCATION, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_IPV4, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_DNS1, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_DNS2, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_ADMIN, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_PASSWORD, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_PORT_NUM, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_PORT_NO, CWCfgMsgGetPortNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_PORT_SPEED_MODE, CWCfgMsgGetPortNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_PORT_FLOWCTL, CWCfgMsgGetPortNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_POE_POWER_BUDGET, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_POE_PORT_NUM, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_POE_PORT_NO, CWCfgMsgGetPoePortNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_POE_PORT_ENABLE, CWCfgMsgGetPoePortNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_POE_PORT_PRIORITY, CWCfgMsgGetPoePortNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_POE_PORT_POWER_LIMIT_TYPE, CWCfgMsgGetPoePortNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_POE_PORT_POWER_LIMIT, CWCfgMsgGetPoePortNextKey), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_AC_LIST, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_REDUNDANT_MANAGED_MAC_LIST, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_REDUDNANCY, NULL), \
	CFG_MSG_INFO_ENTRY(WTP_CFG_SW_PORT_DESCRIPTION, CWCfgMsgGetPortNextKey), \
	CFG_MSG_INFO_ENTRY_END(SW) \
}

extern CWWtpCfgMsgInfo gCfgMsgApInfo[];
extern CWWtpCfgMsgInfo gCfgMsgSwitchInfo[];

#define CFG_MSG_INIT_DO_NOTHING {keyLen=keyLen;}

#define CW_CFG_MSG_GET_IPV4_ADDR(_addr) \
	do { \
		unsigned int *_tmp; \
		CW_CREATE_OBJECT_ERR(_tmp, unsigned int, { \
			CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); \
			return CW_ERROR_OUT_OF_MEMORY; \
		}); \
		*_tmp = _addr; \
		*valLen = 4; \
		*valPtr = (void*) _tmp; \
	} while(0)

#define CW_CFG_MSG_SET_IPV4_ADDR(_addr) \
	do { \
		if(valLen < 4 || !valPtr)  { \
			CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid Value"); \
			return CW_ERROR_WRONG_ARG; \
		} \
		_addr = *((unsigned int*)valPtr); \
	} while(0)

#define CW_CFG_MSG_GET_VAL32(_val) \
	do { \
		int *_tmp; \
		CW_CREATE_OBJECT_ERR(_tmp, int, { \
			CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); \
			return CW_ERROR_OUT_OF_MEMORY; \
		}); \
		*_tmp = htonl((int)_val); \
		*valLen = 4; \
		*valPtr = (void*) _tmp; \
	} while(0)

#define CW_CFG_MSG_SET_VAL32(_val) \
	do { \
		if(valLen < 4 || !valPtr) { \
			CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid Value"); \
			return CW_ERROR_WRONG_ARG; \
		} \
		_val = ntohl(*((int*)valPtr)); \
	} while(0)

#define CW_CFG_MSG_GET_VAL16(_val) \
	do { \
		short *_tmp; \
		CW_CREATE_OBJECT_ERR(_tmp, short, { \
			CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); \
			return CW_ERROR_OUT_OF_MEMORY; \
		}); \
		*_tmp = htons((short)_val); \
		*valLen = 2; \
		*valPtr = (void*) _tmp;	\
	} while(0)

#define CW_CFG_MSG_SET_VAL16(_val) \
	do { \
		if(valLen < 2 || !valPtr) { \
			CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid Value"); \
			return CW_ERROR_WRONG_ARG; \
		} \
		_val = ntohs(*((short*)valPtr));	\
	} while(0)

#define CW_CFG_MSG_GET_VAL8(_val) \
	do { \
		char *_tmp; \
		CW_CREATE_OBJECT_ERR(_tmp, char, { \
			CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); \
			return CW_ERROR_OUT_OF_MEMORY; \
		}); \
		*_tmp = (char) _val; \
		*valLen = 1; \
		*valPtr = (void*) _tmp;	\
	} while(0)

#define CW_CFG_MSG_SET_VAL8(_val) \
	do { \
		if(valLen < 1 || !valPtr) { \
			CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid Value"); \
			return CW_ERROR_WRONG_ARG; \
		} \
		_val = *((char*)valPtr) \
	} while(0)

#define CW_CFG_MSG_GET_STR(_str) \
	do { \
		if(_str && (*valLen = strlen(_str))) { \
			*valPtr = (void*) _str; \
			_str = NULL; \
		} \
		else { \
			*valLen = 0; \
			*valPtr = NULL; \
			CW_FREE_OBJECT(_str); \
		} \
	} while(0)

#define CW_CFG_MSG_GET_STR_ERR(_str,_on_err) \
	do { \
		if(_str && (*valLen = strlen(_str))) { \
			char *_tmp; \
			CW_CREATE_STRING_FROM_STRING_ERR(_tmp, _str, _on_err); \
			*valPtr = (void*) _tmp; \
		} \
		else { \
			*valLen = 0; \
			*valPtr = NULL; \
		} \
	} while(0)

#define CW_CFG_MSG_SET_STR(_str) \
	do { \
		if(valLen && valPtr) { \
			CW_CREATE_STRING_FROM_STRING_LEN_ERR(_str, valPtr, valLen, { \
				CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); \
				return CW_ERROR_OUT_OF_MEMORY; \
			}); \
		} \
		else { \
			CW_CREATE_STRING_FROM_STRING_ERR(_str, "", { \
				CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); \
				return CW_ERROR_OUT_OF_MEMORY; \
			}); \
		} \
	} while(0)

#define CW_CFG_MSG_GET_PORT_INDEX(_maxPort) \
	if(keyLen < 1) { \
		CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid Key Length"); \
		return CW_ERROR_WRONG_ARG; \
	} \
	int port = ((unsigned char*)keyPtr)[0]; \
	if(port >= _maxPort) { \
		CWErrorRaise(CW_ERROR_OUT_OF_INDEX, NULL); \
		return CW_ERROR_OUT_OF_INDEX; \
	}

#define CW_CFG_MSG_GET_RADIO_INDEX(_maxRadio) \
	if(keyLen < 1) { \
		CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid Key Length"); \
		return CW_ERROR_WRONG_ARG; \
	} \
	int radioIdx = ((unsigned char*)keyPtr)[0]; \
	if(radioIdx >= _maxRadio) { \
		CWErrorRaise(CW_ERROR_OUT_OF_INDEX, NULL); \
		return CW_ERROR_OUT_OF_INDEX; \
	}

#define CW_CFG_MSG_GET_RADIO_WLAN_INDEX(_maxRadio,_maxWlan) \
	if(keyLen < 2) { \
		CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid Key Length"); \
		return CW_ERROR_WRONG_ARG; \
	} \
	int radioIdx = ((unsigned char*)keyPtr)[0]; \
	int wlanIdx = ((unsigned char*)keyPtr)[1]; \
	if(radioIdx >= _maxRadio || wlanIdx >= _maxWlan) { \
		CWErrorRaise(CW_ERROR_OUT_OF_INDEX, NULL); \
		return CW_ERROR_OUT_OF_INDEX; \
	}

#define CW_CFG_MSG_GET_RADIO_WLAN_WEPKEY_INDEX(_maxRadio,_maxWlan,_maxWepKey) \
	if(keyLen < 3) { \
		CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid Key Length"); \
		return CW_ERROR_WRONG_ARG; \
	} \
	int radioIdx = ((unsigned char*)keyPtr)[0]; \
	int wlanIdx = ((unsigned char*)keyPtr)[1]; \
	int wepKeyIdx = ((unsigned char*)keyPtr)[2]; \
	if(radioIdx >= _maxRadio || wlanIdx >= _maxWlan || wepKeyIdx >= _maxWepKey) { \
		CWErrorRaise(CW_ERROR_OUT_OF_INDEX, NULL); \
		return CW_ERROR_OUT_OF_INDEX; \
	}

#define CW_CFG_MSG_GETNEXT_PORT_INDEX(_maxPort) \
	do { \
		unsigned char *_key; \
		int port = (keyLen && keyPtr) ? ((unsigned char *)keyPtr)[0] : -1; \
		port++; \
		while(1) { \
			if(port >= _maxPort) { \
				*nextkeyLen = 0; \
				*nextkeyPtr = NULL; \
				return CW_ERROR_NONE; \
			} \
			CW_CREATE_ARRAY_ERR(_key, 1, unsigned char, { \
				CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); \
				return CW_ERROR_OUT_OF_MEMORY; \
			}); \
			_key[0] = (unsigned char)port; \
			*nextkeyLen = 1; \
			*nextkeyPtr = (void*)_key; \
			return CW_ERROR_SUCCESS; \
		} \
	} while(0)

#define CW_CFG_MSG_GETNEXT_RADIO_INDEX(_maxRadio) \
	do { \
		unsigned char *_key; \
		int radioIdx = (keyLen && keyPtr) ? ((unsigned char *)keyPtr)[0] : -1; \
		radioIdx++; \
		while(1) { \
			if(radioIdx >= _maxRadio) { \
				*nextkeyLen = 0; \
				*nextkeyPtr = NULL; \
				return CW_ERROR_NONE; \
			} \
			CW_CREATE_ARRAY_ERR(_key, 1, unsigned char, { \
				CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); \
				return CW_ERROR_OUT_OF_MEMORY; \
			}); \
			_key[0] = (unsigned char)radioIdx; \
			*nextkeyLen = 1; \
			*nextkeyPtr = (void*)_key; \
			return CW_ERROR_SUCCESS; \
		} \
	} while(0)

#define CW_CFG_MSG_GETNEXT_RADIO_WLAN_INDEX(_maxRadio,_maxWlan) \
	do { \
		unsigned char *_key; \
		int radioIdx = (keyLen && keyPtr) ? ((unsigned char *)keyPtr)[0] : 0; \
		int wlanIdx = (keyLen && keyPtr) ? ((unsigned char *)keyPtr)[1] : -1; \
		wlanIdx++; \
		while(1) { \
			if(radioIdx >= _maxRadio) { \
				*nextkeyLen = 0; \
				*nextkeyPtr = NULL; \
				return CW_ERROR_NONE; \
			} \
			if(wlanIdx >= _maxWlan) { \
				radioIdx++; \
				wlanIdx = 0; \
				continue; \
			} \
			CW_CREATE_ARRAY_ERR(_key, 2, unsigned char, { \
				CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); \
				return CW_ERROR_OUT_OF_MEMORY; \
			}); \
			_key[0] = (unsigned char)radioIdx; \
			_key[1] = (unsigned char)wlanIdx; \
			*nextkeyLen = 2; \
			*nextkeyPtr = (void*)_key; \
			return CW_ERROR_SUCCESS; \
		} \
	} while(0)

#define CW_CFG_MSG_GETNEXT_RADIO_WLAN_WEPKEY_INDEX(_maxRadio,_maxWlan,_maxWepKey) \
	do { \
		unsigned char *_key; \
		int radioIdx = (keyLen && keyPtr) ? ((unsigned char *)keyPtr)[0] : 0; \
		int wlanIdx = (keyLen && keyPtr) ? ((unsigned char *)keyPtr)[1] : 0; \
	 	int wepKeyIdx = (keyLen && keyPtr) ? ((unsigned char *)keyPtr)[2] : -1; \
		wepKeyIdx++; \
		while(1) { \
			if(radioIdx >= _maxRadio) { \
				*nextkeyLen = 0; \
				*nextkeyPtr = NULL; \
				return CW_ERROR_NONE; \
			} \
			if(wlanIdx >= _maxWlan) { \
				radioIdx++; \
				wlanIdx = 0; \
				wepKeyIdx = 0; \
				continue; \
			} \
			if(wepKeyIdx >= _maxWepKey) { \
				wlanIdx++; \
				wepKeyIdx = 0; \
				continue; \
			} \
			CW_CREATE_ARRAY_ERR(_key, 3, unsigned char, { \
				CWErrorRaise(CW_ERROR_OUT_OF_MEMORY, NULL); \
				return CW_ERROR_OUT_OF_MEMORY; \
			}); \
			_key[0] = (unsigned char)radioIdx; \
			_key[1] = (unsigned char)wlanIdx; \
			_key[2] = (unsigned char)wepKeyIdx; \
			*nextkeyLen = 3; \
			*nextkeyPtr = (void*)_key; \
			return CW_ERROR_SUCCESS; \
		} \
	} while(0)

/* WTP */
CWBool CWWTPGetWtpCfg(CWWtpCfgMsgList *cfgList);
CWBool CWWTPSaveWtpCfg(CWWtpCfgMsgList *cfgList);
CWBool CWWTPSaveBackgroundSitesurveyValues(CWBackgroundSitesurveyValues *pBgStSvy);
CWBool CWWTPSaveAutoTxpowerHealingValues(CWAutoTxPowerHealingValues *pTxPwHealingVal);

/* AC */
CWBool CWACSaveShellCmdOutput(int wtpIdx, CWStringValue *output);
CWBool CWACSaveMemoryInfo(int wtpIdx, CWMemoryInfo *memInfo);
CWBool CWACIsForceSyncCfg(int wtpIdx, int cfgType);
CWBool CWACIsForceWireMode(int wtpIdx);

CWBool CWACIsUnsupportedCfg(int wtpIdx, int cfgType, int keyLen, void *keyPtr);
CWBool CWACIsUnchangeableCfg(int wtpIdx, int cfgType, int keyLen, void *keyPtr);
CWBool CWACIsDBSavedCfg(int wtpIdx, int cfgType);
CWBool CWACSyncWtpCfg(int wtpIdx, CWWtpCfgMsgList *cfgList, CWWtpCfgMsgList *cfgDiffList);
CWBool CWACRollbackWtpCfg(int wtpIdx, CWWtpCfgMsgList *cfgList);
CWBool CWACUpdateWtpStationInfo(int wtpIdx, CWWTPStationInfo *staInfo);
CWBool CWACUpdateWtpStatisticsInfo(int wtpIdx, CWWTPStatisticsInfo *statInfo);
CWBool CWACUpdateStationStatistic(int wtpIdx, CWWTPStationInfo *staInfo);
CWBool CWACUpdateWtpCurCfgInfo(int wtpIdx, CWWTPCurrentCfgInfo *curCfgInfo);
CWBool CWACUpdateSwitchTopology(int wtpIdx, CWWTPSwitchTopologyInfo *topoInfo);
CWBool CWACUpdateSwitchPoeInfo(int wtpIdx, CWWTPSwitchPoeInfo *poeInfo);
CWBool CWACUpdateSwitchPortInfo(int wtpIdx, CWWTPSwitchPortInfo *portInfo);
CWBool CWACUpdateSwitchTrunkInfo(int wtpIdx, CWWTPSwitchTrunkInfo *trunkInfo);

CWBool CWParseVendorPayload(CWProtocolMessage *msg, int len, CWProtocolVendorSpecificValues *valPtr);
void CWDestroyVendorSpecificValues(CWProtocolVendorSpecificValues *valPtr);
CWBool CWACUpdateMeshInfo(int wtpIdx, CWWTPMeshInfo *meshInfo);
#endif
