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
;    File    : mesh_setting.h
;    Abstract:
;
;       Modification History:
;       By              Date     	Ver.   	Modification Description
;       --------------- -------- 	-----  	-------------------------------------
;		Jerry			2012/09/10			First commit
;****************************************************************************/

#ifndef _MESH_SETTING_H_
#define _MESH_SETTING_H_

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "gconfig.h"
#include "../../appagents.h"
#include "variable/api_wan.h"

/*--------------------------------------------------------------------------*/
/*                               DEFINITIONS                                */
/*--------------------------------------------------------------------------*/
#define MESH_BROADCAST_COUNT_FILE_FORMAT            "/tmp/%s"
#define MESH_BROADCAST_SUCCESS_FILE_FORMAT          "/tmp/%s_success"
#define MESH_PING_TEST_RESULT_FILE                  "/tmp/mesh_ping_result"
#define MESH_TRACE_ROUTE_RESULT_FILE                "/tmp/mesh_trace_route_result"
#define EzSetup_SCAN_RESULT                         "/tmp/ezsetup_scan_result"
#define EzSetup_NO_SCANNING                         "/tmp/ezsetup_not_support_scan"
#define EzSetup_ADD_SLAVE_FLAG                      "/tmp/ezsetup_slave_add_mesh_nodes"
#define FORCE_MESH_EZ_SCAN                          0
#define EzSetup_24g_MSC_TMP                         "ath33"
#define EzSetup_24g_SCANNING_TMP                    "ath36"
#define EzSetup_5g_MSC_TMP                          "ath34"
#define MESH_5g_IFNAME                              "ath35"

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                                STRUCTURE                                 */
/*--------------------------------------------------------------------------*/
#if HAS_MESH_JSON
typedef enum _MESH_SYSTEM_OPERATION_MODE_
{
	MESH_SYSTEM_ROUTER_MODE=1,
	MESH_SYSTEM_AP_MODE
} MESH_SYSTEM_OPERATION_MODE;

typedef enum _MESH_PROFILE_ACTION_TYPE_
{
	MESH_PROFILE_ACTION_DELETE=0,
	MESH_PROFILE_ACTION_SET,
	MESH_PROFILE_ACTION_MODIFY
} MESH_PROFILE_ACTION_TYPE;

typedef enum _MESH_FIRMWARE_UPGRADE_ACTION_TYPE_
{
	MESH_FIRMWARE_UPGRADE_ACTION_AUTO_ENABLE=1,
	MESH_FIRMWARE_UPGRADE_ACTION_EXECUTE_AUTO,
	MESH_FIRMWARE_UPGRADE_ACTION_EXECUTE_MANUAL
} MESH_FIRMWARE_UPGRADE_ACTION_TYPE;
#endif

#if 1//HAS_ALFRED
typedef enum _ALFRED_INFORMATION_OPTION_
{
	ALFRED_SYSTEM_INFORMATION=64,
	ALFRED_INTERFACE_INFORMATION,
	ALFRED_WDSLINK_INFORMATION
} ALFRED_INFORMATION_OPTION;

typedef enum _MESH_LINK_INFORMATION_TYPE_
{
    MESH_LINK_INFORMATION_MAC=0,
    MESH_LINK_INFORMATION_TQ,
} MESH_LINK_INFORMATION_TYPE;

#endif

typedef enum _IBEACON_SETTINGS_
{
    COLLECT_IBEACON_SETTING=0,
    GET_ALL_MESH_IBEACON_SETTINGS
} IBEACON_SETTINGS;

#if HAS_MESH_STATIC_ROUTE
typedef struct _MESH_ALLOW_MACLIST_
{
    int  aw_num;
    char aw_mac[32][32];
    bool aw_flag;
} MESH_ALLOW_MACLIST_T;
#endif
/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
int set_mesh_device_by_wizard_cb(HTTPS_CB *pkt);
int set_mesh_network_profile_cb(HTTPS_CB *pkt);
int get_mesh_broadcast_setting_status_cb(HTTPS_CB *pkt);
int get_mesh_specific_option_setting_cb(HTTPS_CB *pkt);
int get_mesh_device_list_cb(HTTPS_CB *pkt);
int get_mesh_device_client_list_cb(HTTPS_CB *pkt);
int get_mesh_device_neighbors_cb(HTTPS_CB *pkt);
int invite_new_mesh_device_cb(HTTPS_CB *pkt);
int set_mesh_device_name_cb(HTTPS_CB *pkt);
int apply_mesh_network_settings_cb(HTTPS_CB *pkt);
int get_mesh_device_wireless_settings_cb(HTTPS_CB *pkt);
int set_mesh_device_wireless_settings_cb(HTTPS_CB *pkt);
int set_mesh_device_led_action_cb(HTTPS_CB *pkt);
int get_mesh_device_status_cb(HTTPS_CB *pkt);
int get_mesh_device_wan_settings_cb(HTTPS_CB *pkt);
int set_mesh_device_wan_settings_cb(HTTPS_CB *pkt);
int download_mesh_device_firmware_cb(HTTPS_CB *pkt);
int download_mesh_device_firmware_status_cb(HTTPS_CB *pkt);
int do_mesh_device_firmware_upgrade_cb(HTTPS_CB *pkt);
int do_mesh_single_device_firmware_upgrade_cb(HTTPS_CB *pkt);
#if SUPPORT_IPERF_THROUGHPUT_TEST
int run_mesh_throughput_test_cb(HTTPS_CB *pkt);
int get_mesh_throughput_test_result_cb(HTTPS_CB *pkt);
#endif
int run_mesh_ping_test_cb(HTTPS_CB *pkt);
int get_mesh_ping_test_result_cb(HTTPS_CB *pkt);
int run_mesh_trace_route_cb(HTTPS_CB *pkt);
int get_mesh_trace_route_result_cb(HTTPS_CB *pkt);
int get_mesh_trace_route_simple_result_cb(HTTPS_CB *pkt);
int sync_mesh_robust_threshold_cb(HTTPS_CB *pkt);
int sync_mesh_msc_configured_cb(HTTPS_CB *pkt);
int get_mesh_node_info_cb(HTTPS_CB *pkt);
int get_login_mesh_info_cb(HTTPS_CB *pkt);
int send_system_cmd_to_mesh_device_cb(HTTPS_CB *pkt);
int send_uci_changes_to_mesh_device_cb(HTTPS_CB *pkt);
int set_mesh_wifi_disabled_cb(HTTPS_CB *pkt);
int set_mesh_device_reboot_cb(HTTPS_CB *pkt);
int set_mesh_wifi_trigger_wps_cb(HTTPS_CB *pkt);
int check_mesh_wifi_trigger_wps_cb(HTTPS_CB *pkt);
int set_mesh_wifi_Location_cb(HTTPS_CB *pkt);
int set_mesh_led_disabled_cb(HTTPS_CB *pkt);
int get_mesh_basic_mode_wifi_info_cb(HTTPS_CB *pkt);
int delete_mesh_device_cb(HTTPS_CB *pkt);
int mesh_reboot_device_cb(HTTPS_CB *pkt);
int mesh_reset_device_cb(HTTPS_CB *pkt);
int set_mesh_device_usb_cb(HTTPS_CB *pkt);
int get_mesh_device_usb_cb(HTTPS_CB *pkt);
int get_mesh_device_usb_info_cb(HTTPS_CB *pkt);
int get_mesh_home_info_cb(HTTPS_CB *pkt);
#if SUPPORT_PEOPLE_FUNCTION
int get_mesh_simple_people_info_cb(HTTPS_CB *pkt);
int get_mesh_user_profile_cb(HTTPS_CB *pkt);
int get_mesh_user_profile_list_cb(HTTPS_CB *pkt);
int set_mesh_user_profile_cb(HTTPS_CB *pkt);
int set_mesh_user_profile_list_cb(HTTPS_CB *pkt);
int delete_mesh_user_profile_cb(HTTPS_CB *pkt);
int get_mesh_client_profile_cb(HTTPS_CB *pkt);
int get_mesh_client_profile_list_cb(HTTPS_CB *pkt);
int set_mesh_client_profile_cb(HTTPS_CB *pkt);
int delete_mesh_client_profile_cb(HTTPS_CB *pkt);
int delete_mesh_client_profile_list_cb(HTTPS_CB *pkt);
#if SUPPORT_FIREWALL_URL_CATEGORY
int get_mesh_firewall_default_url_category_cb(HTTPS_CB *pkt);
#endif
int get_mesh_firewall_rule_cb(HTTPS_CB *pkt);
int get_mesh_firewall_rule_list_cb(HTTPS_CB *pkt);
int set_mesh_firewall_rule_cb(HTTPS_CB *pkt);
int delete_mesh_firewall_rule_cb(HTTPS_CB *pkt);
int block_mesh_user_cb(HTTPS_CB *pkt);
int block_mesh_client_cb(HTTPS_CB *pkt);
int reset_all_blocked_mesh_client_cb(HTTPS_CB *pkt);
#endif
#if HAS_SPEEDTEST_THROUGHPUT_TEST
int run_mesh_speedtest_cb(HTTPS_CB *pkt);
int get_mesh_speedtest_result_cb(HTTPS_CB *pkt);
int find_mesh_speedtest_best_server_cb(HTTPS_CB *pkt);
int get_mesh_speedtest_best_server_result_cb(HTTPS_CB *pkt);
#endif
int mesh_reset_target_device_cb(HTTPS_CB *pkt);
int ezsetup_broadcast_pkts_cb(HTTPS_CB *pkt);
int ezsetup_get_candidates_cb(HTTPS_CB *pkt);
int ezsetup_add_mesh_nodes_cb(HTTPS_CB *pkt);
int ezsetup_get_mesh_fail_nodes_cb(HTTPS_CB *pkt);
// int set_mesh_device_ibeacon_cb(HTTPS_CB *pkt);
// int get_mesh_device_ibeacon_cb(HTTPS_CB *pkt);
int get_mesh_divece_client_info_cb(HTTPS_CB *pkt);
#if HAS_MESH_STATIC_ROUTE
int set_mesh_allow_mac_list_cb(HTTPS_CB *pkt);
int inform_mesh_static_route(HTTPS_CB *pkt);
#endif
#endif
