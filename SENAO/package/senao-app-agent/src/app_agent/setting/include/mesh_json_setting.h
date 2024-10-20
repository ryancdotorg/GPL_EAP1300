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
;    File    : mesh_json_setting.h
;    Abstract:
;
;       Modification History:
;       By              Date     	Ver.   	Modification Description
;       --------------- -------- 	-----  	-------------------------------------
;		Jerry			2012/09/10			First commit
;****************************************************************************/

#ifndef _MESH_JSON_SETTING_H_
#define _MESH_JSON_SETTING_H_

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "gconfig.h"
#include "../../appagents.h"

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                                STRUCTURE                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
bool get_json_mesh_account(char *query_str, char *user_name, char *password);
bool parse_json_mesh_wireless_setting(char *query_str, char *name, WLAN_RADIOS_T *setting, char **return_str);
bool parse_json_wireless_setting(char *query_str, char *name, WLAN_RADIOS_T setting[], char **return_str);
bool parse_json_mesh_network_settings_cb(char *query_str, WAN_SETTINGS_T *wan_setting, char **return_str);
int get_json_mesh_specific_option_setting_cb(HTTPS_CB *pkt, char *value, char *result);
int get_json_mesh_device_list_cb(HTTPS_CB *pkt, MESH_DEVICE_SETTINGS_T setting[], char *result);
int get_json_mesh_device_wireless_settings_cb(HTTPS_CB *pkt, char *op_mesh_band,
        WLAN_RADIO_SETTINGS_T setting[], WLAN_RADIO_SECURITY_T security[], char *result);
bool parse_json_mesh_device_neighbors(char *buf, MESH_DEVICE_NEIGHBORS_T setting[], char **return_str);
int get_json_mesh_device_neighbors_cb(HTTPS_CB *pkt, MESH_DEVICE_NEIGHBORS_T setting[], char *result);
int get_json_mesh_node_info_cb(HTTPS_CB *pkt, MESH_DEVICE_SETTINGS_T setting, MESH_DEVICE_NEIGHBORS_T neighbor, MESH_GUEST_CLIENT_INFO_T gClientInfo, char client_list[][32], char *result);
int get_json_mesh_basic_wifi_info_cb(HTTPS_CB *pkt, char *result);
int get_json_mesh_usb_device_info_cb(HTTPS_CB *pkt, char *result);
int get_json_mesh_usb_settings_info_cb(HTTPS_CB *pkt, char *result);
int get_json_mesh_home_info_cb(HTTPS_CB *pkt, char *result);
#if SUPPORT_PEOPLE_FUNCTION
int get_mesh_simple_people_info_json_cb(HTTPS_CB *pkt, MESH_USER_PROFILE_T *users, MESH_CLIENT_PROFILE_T *clients, MESH_FIREWALL_RULE_T *rules, char *result);
int get_mesh_user_profile_json_cb(HTTPS_CB *pkt, MESH_USER_PROFILE_T setting, char *result);
int get_mesh_user_profile_list_json_cb(HTTPS_CB *pkt, MESH_USER_PROFILE_T *setting, char *result);
int parse_mesh_user_profile_json_cb(char *query_str, MESH_USER_PROFILE_T *setting, char **return_str);
int parse_mesh_user_profile_list_json_cb(char *query_str, MESH_USER_PROFILE_T *setting, char **return_str);
int get_mesh_client_profile_json_cb(HTTPS_CB *pkt, MESH_CLIENT_PROFILE_T setting, char *result);
int get_mesh_client_profile_list_json_cb(HTTPS_CB *pkt, MESH_CLIENT_PROFILE_T *setting, char *result);
int parse_mesh_client_profile_json_cb(char *query_str, MESH_CLIENT_PROFILE_T *setting, char **return_str);
int delete_mesh_client_profile_list_json_cb(char *query_str, int *index);
#if SUPPORT_FIREWALL_URL_CATEGORY
int get_mesh_firewall_default_url_category_json_cb(HTTPS_CB *pkt);
#endif
int get_mesh_firewall_rule_json_cb(HTTPS_CB *pkt, MESH_FIREWALL_RULE_T setting, char *result);
int get_mesh_firewall_rule_list_json_cb(HTTPS_CB *pkt, MESH_FIREWALL_RULE_T *setting, char *result);
int parse_mesh_firewall_rule_json_cb(char *query_str, MESH_FIREWALL_RULE_T *setting, char **return_str);
#endif
int ezsetup_get_candidates_json_cb(HTTPS_CB *pkt, char *result);
bool parse_json_ezsetup_add_mesh_nodes(char *query_str, char *ipList, char *FailList, char **return_str);
int get_mesh_trace_route_result_json_cb(HTTPS_CB *pkt, char *result_file_path, char *result);
// int get_ibeacon_settings_result_json_cb(HTTPS_CB *pkt, char *result, int action);
int get_mesh_divece_client_info_json_cb(HTTPS_CB *pkt, MESH_CLIENT_INFO_T ClientInfo,MESH_GUEST_CLIENT_INFO_T gClientInfo, char *result);
#if HAS_MESH_STATIC_ROUTE
bool parse_json_allow_mac_list_cb(char *query_str, MESH_ALLOW_MACLIST_T *setting, char **return_str);
#endif
#endif
