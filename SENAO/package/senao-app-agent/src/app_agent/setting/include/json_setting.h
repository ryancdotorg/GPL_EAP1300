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
;    File    : device_setting.h
;    Abstract: HNAP methods for device management
;
;       Modification History:
;       By              Date        Ver.    Modification Description
;       --------------- --------    -----   -------------------------------------
;       Jerry           2012/09/10          First commit
;****************************************************************************/
/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "../../appagents.h"
#include "device_setting.h"
#include "sitecom_setting.h"
#include "deviceinfo.h"
#include "app_agent.h"
#include "variable/api_wan.h"
#include "variable/api_ipv6.h"
/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/
#define HTTP_JSON_RESPONSE_HEADER_TEMPLATE  "Server: Mathopd/1.5p6\r\n" \
                                            "Connection: Close\r\n" \
                                            "Content-Type: text/html; charset=utf-8\r\n\r\n"

/*--------------------------------------------------------------------------*/
/*                                STRUCTURE                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
int basic_json_response(HTTPS_CB *pkt, char *result);
int simple_json_response(HTTPS_CB *pkt, char *result);
bool check_app_client_reply(char *payload, char *action);

/*----------------------------- DEVICE SETTING -----------------------------*/
int login_json_cb(char *query_str, void *priv);
int change_login_pw_json_cb(char *query_str, void *priv);
int get_device_settings_json_cb(HTTPS_CB *pkt, DEVICE_SETTING_T *setting, char *result);
int get_system_information_json_cb(HTTPS_CB *pkt, SYSTEM_INFO_T *setting, char *result);
int get_timezone_capability_json_cb(HTTPS_CB *pkt);
int get_systime_setting_json_cb(HTTPS_CB *pkt, TIME_SETTINGS_T *settings, char *result);
int download_device_config_file_json_cb(HTTPS_CB *pkt, char *fileName, char *result);
bool parse_upgrade_fw_json_cb(char *query_str, UPGRADE_FW *settings);
int get_fw_release_info_json_cb(HTTPS_CB *pkt, char *setting, char *result);
int get_device_status_cb(HTTPS_CB *pkt);
int set_onvif_discovery_mode_cb(HTTPS_CB *pkt);
int set_onvif_scopes_cb(HTTPS_CB *pkt);
int get_rtsp_port_json_cb(HTTPS_CB *pkt, int rtsp_port, char *result);
bool parse_json_rtsp_port_json_cb(char *query_str, int *rtsp_port);
/*------------------------------- WAN SETTING -------------------------------*/
int get_ipv4_wan_status_json_cb(HTTPS_CB *pkt, WAN_SETTINGS_T *setting, char *wan_status, char *result);
int get_ipv4_wan_settings_json_cb(HTTPS_CB *pkt, WAN_SETTINGS_T *setting, char *result);
int parse_wan_settings_json_cb(char *query_str, WAN_SETTINGS_T *setting, char **result_str);
int parse_ipv4_wan_settings_json_cb(char *query_str, WAN_SETTINGS_T *setting, char **result_str);
int get_lan_settings_json_cb(HTTPS_CB *pkt, ROUTER_LAN_SETTINGS_T *setting, char *result);
int parse_lan_settings_json_cb(char *query_str, ROUTER_LAN_SETTINGS_T *setting, char *result_str);
int get_client_status_json_cb(HTTPS_CB *pkt, char *result);
#if SUPPORT_IPV6_SETTING
int get_ipv6_wan_status_json_cb(HTTPS_CB *pkt, WAN_IPV6_SETTINGS_T *setting, char *result);
int get_ipv6_wan_settings_json_cb(HTTPS_CB *pkt, WAN_IPV6_SETTINGS_T *setting, char *result);
bool parse_ipv6_wan_settings_json_cb(char *query_str, WAN_IPV6_SETTINGS_T *settings, char **return_str);
#endif
int get_json_radio_id(char *query_str);
int get_json_ssid_id(char *query_str);
void get_wlan_radios_json_cb(HTTPS_CB *pkt);
int get_wlan_radio_settings_json_cb(HTTPS_CB *pkt, WLAN_RADIO_SETTINGS_T *setting, char *result);
int get_wlan_radio_security_json_cb(HTTPS_CB *pkt, WLAN_RADIO_SECURITY_T *setting, char *result);
bool parse_json_wlan_radios(char *query_str, WLAN_RADIOS_T *settings, char *return_str);
bool get_auto_fw_upgrade_status_json_cb(HTTPS_CB *pkt, bool enabled, char *result);
#if HAS_STREAM_BOOST_SETTING
bool get_stream_boost_settings_json_cb(HTTPS_CB *pkt, STREAM_BOOST_DATA *setting, char *result);
bool parse_stream_boost_settings_json_cb(char *query_str, STREAM_BOOST_DATA *setting, char *return_str);
#endif
void get_user_list_json_cb(HTTPS_CB *pkt, ACCOUNT_SETTING_T *account_array);
/*------------------------------- DDNS Setting ------------------------------*/
int get_ddns_settings_json_cb(HTTPS_CB *pkt, SYSTEM_DDNS_INFO_T *setting, char *result);
/*------------------------------- UPNP Setting ------------------------------*/
int get_upnp_settings_json_cb(HTTPS_CB *pkt, SYSTEM_UPNP_INFO_T *setting, char *result);


int get_system_throughput_json_cb(HTTPS_CB *pkt, char *download_rate, char *upload_rate, char *result);
int get_device_status_for_enshare_cb(HTTPS_CB *pkt);
void get_blocked_client_list_json_cb(HTTPS_CB *pkt, BLOCKED_CLIENT_LIST_T *setting);
