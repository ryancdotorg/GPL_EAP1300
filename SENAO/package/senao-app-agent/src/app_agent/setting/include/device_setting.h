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
;       By              Date     	Ver.   	Modification Description
;       --------------- -------- 	-----  	-------------------------------------
;		Jerry			2012/09/10			First commit
;****************************************************************************/
#ifndef _DEVICE_SETTING_H_
#define _DEVICE_SETTING_H_

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
//#include "hnap.h"
#include "../../appagents.h"
#include "deviceinfo.h"

/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/
#if SUPPORT_IPERF_THROUGHPUT_TEST
#define THROUGHPUT_TEST_RESULT_FILE           "/tmp/throughput_result"
#endif
#define PING_TEST_RESULT_FILE                 "/tmp/ping_result"
#define TRACE_ROUTE_RESULT_FILE               "/tmp/trace_route_result"

#if FOR_SC
#define AUTO_FIRMWARE_INIT_SCRIPT             "/etc/init.d/autofw"
#define FIRMWARE_INFO_LOG_FILE                "/tmp/FWinfo.log"
#define FIRMWARE_DOWNLOAD_STATUS_FILE         "/tmp/FirmwareDownloadStatus"
#define DOWNLOADED_FIRMWARE_FILE              "/tmp/firmware.img"

typedef enum _SC_AUTO_FIRMWARE_UPGRADE_
{
    SC_ENABLE_AUTO_FIRMWARE_UPGRADE=0,
    SC_CHECK_NEW_FIRMWARE,
    SC_CHECK_NEW_FIRMWARE_RESULT,
    SC_DOWNLOAD_NEW_FIRMWARE,
    SC_DOWNLOAD_FIRMWARE_STATUS,
    SC_EXECUTE_FIRMWARE_UPGRADE
} SC_AUTO_FIRMWARE_UPGRADE;
#endif
#define CHKNEWFW_SCRIPT             "/sbin/chkNewFW.sh"
#define UPGRADE_LIST             "/tmp/needUpgradeList"
#define FIRMWARE_INFO_LOG_FILE      "/tmp/new_fw_info"
#define FIRMWARE_IMAGE      "/tmp/download"
#define FIRMWARE_DOWNLOAD_STATUS      "/tmp/downloadStatus"
#define DOWNLOAD_FW_SCRIPT      "/sbin/doAutoFWupgrade.sh"
#define GEN_UPGRADELIST_LUA      "/usr/lib/lua/luci/gen_upgradeList.lua"
typedef enum _FIRMWARE_UPGRADE_
{
    ENABLE_AUTO_FIRMWARE_UPGRADE=0,
    CHECK_NEW_FIRMWARE,
    CHECK_NEW_FIRMWARE_RESULT,
    DOWNLOAD_NEW_FIRMWARE,
    DOWNLOAD_FIRMWARE_STATUS,
    EXECUTE_FIRMWARE_UPGRADE
} FIRMWARE_UPGRADE;


/*--------------------------------------------------------------------------*/
/*                                STRUCTURE                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
void get_IP_Port(int sock, int ipcam_port, char *ip_addr, char *port);
int login_cb(HTTPS_CB *pkt);
int change_login_pw_cb(HTTPS_CB *pkt);
int check_alive_cb(HTTPS_CB *pkt);
int reboot_cb(HTTPS_CB *pkt);
int reset_to_default_cb(HTTPS_CB *pkt);
int reboot_factory_cb(HTTPS_CB *pkt);
int get_device_settings_cb(HTTPS_CB *pkt);
int get_system_information_cb(HTTPS_CB *pkt);
int download_device_config_file_cb(HTTPS_CB *pkt);
int upgrade_fw_cb(HTTPS_CB *pkt);
int get_timezone_capability_cb(HTTPS_CB *pkt);
int get_systime_setting_cb(HTTPS_CB *pkt);
int set_systime_setting_cb(HTTPS_CB *pkt);
int get_http_port_cb(HTTPS_CB *pkt);
int set_http_port_cb(HTTPS_CB *pkt);
int get_fw_upgrade_url_cb(HTTPS_CB *pkt);
int get_config_upgrade_url_cb(HTTPS_CB *pkt);
#if HAS_EG_AUTO_FW_CHECK
int get_fw_release_info_cb(HTTPS_CB *pkt);
int set_auto_fw_upgrade_cb(HTTPS_CB *pkt);
int do_auto_fw_upgrade_cb(HTTPS_CB *pkt);
#endif
int set_device_led_action_cb(HTTPS_CB *pkt);
int set_sdcard_format_cb(HTTPS_CB *pkt);
int get_sdcard_format_status_cb(HTTPS_CB *pkt);
#if HAS_IPCAM
int set_ipcam_simple_nvr_cb(HTTPS_CB *pkt);
int get_rtsp_port_cb(HTTPS_CB *pkt);
int set_rtsp_port_cb(HTTPS_CB *pkt);
int get_sdcard_sync_cb(HTTPS_CB *pkt);
#endif
#if SUPPORT_IPERF_THROUGHPUT_TEST
int run_throughput_test_cb(HTTPS_CB *pkt);
int get_throughput_test_result_cb(HTTPS_CB *pkt);
#endif
int run_ping_test_cb(HTTPS_CB *pkt);
int get_ping_test_result_cb(HTTPS_CB *pkt);
int run_trace_route_cb(HTTPS_CB *pkt);
int get_trace_route_result_cb(HTTPS_CB *pkt);
int get_system_information_with_ip_cb(HTTPS_CB *pkt);
int get_mesh_connected_history_cb(HTTPS_CB *pkt);
int reboot_all_devices_cb(HTTPS_CB *pkt);
int reset_all_devices_cb(HTTPS_CB *pkt);
int update_mesh_node_info_cb(HTTPS_CB *pkt);
#if FOR_SC
int set_sc_fw_auto_upgrade_cb(HTTPS_CB *pkt);
int execute_sc_firmware_upgrade_cb(HTTPS_CB *pkt);
#endif
int get_mesh_node_simplify_info_cb(HTTPS_CB *pkt);
int execute_firmware_upgrade_cb(HTTPS_CB *pkt);
int do_manual_firmware_upgrade_cb(HTTPS_CB *pkt);
int set_fw_auto_upgrade_cb(HTTPS_CB *pkt);
int get_system_throughput_cb(HTTPS_CB *pkt);
int get_base_status_cb(HTTPS_CB *pkt);
int get_SN_number_cb(HTTPS_CB *pkt);
#if HAS_SPEEDTEST_THROUGHPUT_TEST
int run_speedtest_test_cb(HTTPS_CB *pkt);
int get_speedtest_test_result_cb(HTTPS_CB *pkt);
int find_speedtest_best_server(HTTPS_CB *pkt);
int get_speedtest_best_server_result_cb(HTTPS_CB *pkt);
#endif
int get_internet_status_cb(HTTPS_CB *pkt);
#endif

