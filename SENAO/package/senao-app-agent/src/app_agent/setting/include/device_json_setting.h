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
;    Creator :
;    File    : device_json_setting.h
;    Abstract:
;
;       Modification History:
;       By              Date            Ver.   	Modification Description
;       --------------- --------        -----   -------------------------------------
;                       2015/12/18              First commit
;****************************************************************************/
#ifndef _DEVICE_JSON_SETTING_H_
#define _DEVICE_JSON_SETTING_H_

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "json.h"
#include "deviceinfo.h"
#include "../../appagents.h"

/*--------------------------------------------------------------------------*/
/*                                STRUCTURE                                 */
/*--------------------------------------------------------------------------*/
struct device_info
{
    char deviceType[8];
    int  rssi;
    char mac[32];
    char uid[16];
    char deviceName[128];
    char locationName[128];
    char meshRole[16];
    char meshController[16];
    int  tq;
    char lANIPAddress[32];
    char LANMacAddress[32];
    int  wifi_status;
    int  dev_connect_number;
    int  rootHopCount;
    int  led_status;
    char FullFwVersion[32];
    char WANMacAddress[32];
    int  NextHopRssi;
    char NextHopMac[32];
    char TrueWANMAC[32];
    MESH_GUEST_CLIENT_INFO_T gClientInfo;
};
/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
bool get_device_device_settings_json_cb(struct json_object *jobj, DEVICE_SETTING_T *setting);
bool parse_device_led_action_cb(char *query_str, int *type, int *action);
#if SUPPORT_IPERF_THROUGHPUT_TEST
int get_throughput_test_result_json_cb(HTTPS_CB *pkt, char *upload, char *download, char *result);
#endif
int get_specific_test_result_json_cb(HTTPS_CB *pkt, char *result_file_path, char *result);
int get_json_system_information_with_ip_cb(HTTPS_CB *pkt, char *result);
int get_json_mesh_connected_history_cb(HTTPS_CB *pkt, char *result);
#if FOR_SC
int execute_sc_firmware_upgrade_json_cb(HTTPS_CB *pkt, bool boolean, int percentage, char *file_size, char *result);
#endif
int get_json_mesh_node_simplify_info_cb(HTTPS_CB *pkt, char *result);
int execute_firmware_upgrade_json_cb(HTTPS_CB *pkt, bool boolean, int percentage, char *file_size, char *result, char *release_date, char *change_log, char *version);
int get_new_firmware_result_json_cb(HTTPS_CB *pkt, char *result, int action);
int get_download_firmware_result_json_cb(HTTPS_CB *pkt, char *result, int action);
int get_json_base_status_cb(HTTPS_CB *pkt, char *result, int AutoFwEnable, char *ExternalIP, char *FwVersion, char *MainIP, char *ProductCode, char *upTime);
int get_json_SN_number_cb(HTTPS_CB *pkt, char *result, char *SN_number, char *r_domain, char *VenderName, char *MeshDeviceLocation, int eth_status);
#if HAS_SPEEDTEST_THROUGHPUT_TEST
int get_speedtest_test_result_json_cb(HTTPS_CB *pkt, char *result, SPEEDTEST_RESULT_T *test_result);
int get_speedtest_best_server_result_json_cb(HTTPS_CB *pkt, char *result, char *best_server_result);
#endif
int get_json_internet_status_cb(HTTPS_CB *pkt, int status);
#endif
