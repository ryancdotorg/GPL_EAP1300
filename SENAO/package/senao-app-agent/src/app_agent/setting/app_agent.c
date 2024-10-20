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
  ;    Project :
  ;    Creator :
  ;    File    :
  ;    Abstract:
  ;
  ;       Modification History:
  ;       By              Date     Ver.   Modification Description
  ;       --------------- -------- -----  -------------------------------------
  ;
  ;****************************************************************************/

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "app_agent.h"
#include "json_setting.h"
#include "appagent_cfg_data.h"
#include "admin_cfg.h"
#include "sysString.h"
#include "sysUtilMisc.h"
#include "utility/sys_type.h"
#include "device_setting.h"
#include "wan_setting.h"
#include "lan_setting.h"
#include "wlan_setting.h"
#include "sitecom_setting.h"
#include "usb_setting.h"
#if HAS_MESH_JSON
#include "mesh_setting.h"
#endif
#include "account_setting.h"
#include "ddns_setting.h"
#if ROUTER_SUPPORT_IPCAM
#include "ipcam_setting.h"
#endif

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/

APP_AGENT_SERVER_SUPPORT_TABLE_T app_agent_server_support_table[] =
{
        /*--------------------------------- Device Setting --------------------------------*/
        {   M_POST,     "Login",                        login_cb,                          1},
        {   M_POST,     "ChangeLoginPw",                change_login_pw_cb,                1},
        {   M_GET,      "CheckAlive",                   check_alive_cb,                    1},
        {   M_GET,      "Reboot",                       reboot_cb,                         1},
        {   M_GET,      "RebootFactory",                reboot_factory_cb,                 1},
        {   M_POST,     "ResetToDefault",               reset_to_default_cb,               1},
        {   M_GET,      "GetDeviceSettings",            get_device_settings_cb,            1},
        {   M_GET,      "GetSystemInformation",         get_system_information_cb,         1},
        {   M_GET,      "DownloadDeviceConfigFile",     download_device_config_file_cb,    1},
        {   M_POST,     "UpgradeFW",                    upgrade_fw_cb,                     1},
        {   M_GET,      "GetFwUpgradeURL",              get_fw_upgrade_url_cb,             1},
        {   M_GET,      "GetConfigUpgradeURL",          get_config_upgrade_url_cb,         1},
#if APP_AGENT_SUPPORT_ENSHARE
        {   M_GET,      "GetDeviceStatus",              get_device_status_for_enshare_cb,  1},
#endif
        {   M_GET,      "GetSystemThroughput",          get_system_throughput_cb,          1},
#if HAS_EG_AUTO_FW_CHECK
        {   M_GET,      "GetFirmwareReleaseInfo",       get_fw_release_info_cb,            1},
        {   M_POST,     "SetAutoFirmwareUpdate",        set_auto_fw_upgrade_cb,            1},
        {   M_POST,     "DoAutoFirmwareUpdate",         do_auto_fw_upgrade_cb,             1},
#endif
        {   M_POST,     "SetDeviceLEDAction",           set_device_led_action_cb,          1},
        {   M_POST,     "SetSDcardFormat",              set_sdcard_format_cb,              1},
        {   M_GET,      "GetSDcardFormatStatus",        get_sdcard_format_status_cb,       1},
#if SUPPORT_IPERF_THROUGHPUT_TEST
        {   M_POST,     "RunThroughputTest",            run_throughput_test_cb,            1},
        {   M_GET,      "GetThroughputTestResult",      get_throughput_test_result_cb,     1},
#endif
#if HAS_SPEEDTEST_THROUGHPUT_TEST
        {   M_POST,     "RunSpeedtestTest",             run_speedtest_test_cb,             1},
        {   M_POST,     "GetSpeedtestTestResult",       get_speedtest_test_result_cb,      1},
#endif
        {   M_POST,     "RunPingTest",                  run_ping_test_cb,                  1},
        {   M_GET,      "GetPingTestResult",            get_ping_test_result_cb,           1},
        {   M_POST,     "RunTraceRoute",                run_trace_route_cb,                1},
        {   M_GET,      "GetTraceRouteResult",          get_trace_route_result_cb,         1},
#if SUPPORT_WAN_SETTING
        /*---------------------------------- WAN Setting ----------------------------------*/
        {   M_GET,      "GetIpv4WanStatus",             get_ipv4_wan_status_cb,            1},
        {   M_GET,      "GetIPv4WanSettings",           get_ipv4_wan_settings_cb,          1},
        {   M_POST,     "SetIPv4WanSettings",           set_ipv4_wan_settings_cb,          1},
        {   M_GET,      "GetWanStatus",                 get_wan_status_cb,                 1},
        {   M_GET,      "GetWanSettings",               get_wan_settings_cb,               1},
        {   M_POST,     "SetWanSettings",               set_wan_settings_cb,               1},
#if SUPPORT_IPV6_SETTING
        {   M_GET,      "GetIpv6WanStatus",             get_ipv6_wan_status_cb,            1},
        {   M_GET,      "GetIPv6WanSettings",           get_ipv6_wan_settings_cb,          1},
        {   M_POST,     "SetIPv6WanSettings",           set_ipv6_wan_settings_cb,          1},
#endif
#endif
        /*---------------------------------- LAN Setting ----------------------------------*/
        {   M_GET,      "GetLanSettings",               get_lan_settings,                  1},
        {   M_POST,     "SetLanSettings",               set_lan_settings,                  1},
        {   M_GET,      "GetClientStatus",              get_client_status,                 1},
        {   M_GET,      "GetLanAccessControlList",      get_lan_access_control_list_cb,    1},
        {   M_POST,     "SetLanAccessControlList",      set_lan_access_control_list_cb,    1},
        {   M_POST,     "AddLanAccessControlList",      add_lan_access_control_list_cb,    1},
        {   M_POST,     "DeleteLanAccessControlList",   delete_lan_access_control_list_cb, 1},
        {   M_GET,      "GetBlockedClientList",         get_blocked_client_list_cb,        1},
        {   M_POST,     "EditBlockedClientList",        edit_blocked_client_list_cb,       1},
        {   M_GET,      "DeleteBlockedClientList",      delete_blocked_client_list_cb,     1},
        /*------------------------ --------- WLAN Setting ---------------------------------*/
        {   M_GET,      "GetWLanRadios",                get_wlan_radios_cb,                1},
        {   M_GET,      "GetWLanStationStatus",         get_wlan_station_status_cb,        1},
        {   M_POST,     "SetWLanRadios",                set_wlan_radios_cb,                1},
        {   M_POST,     "GetWLanRadioSettings",         get_wlan_radio_settings_cb,        1},
        {   M_POST,     "GetWLanRadioSecurity",         get_wlan_radio_security_cb,        1},
        {   M_POST,     "GetAccessControlList",         get_access_control_list_cb,        1},
        {   M_POST,     "SetAccessControlList",         set_access_control_list_cb,        1},
        {   M_POST,     "AddAccessControlList",         add_access_control_list_cb,        1},
        {   M_POST,     "DeleteAccessControlList",      delete_access_control_list_cb,     1},
        {   M_GET,      "GetWLanSiteSurvey",            get_wlan_sitesurvey_cb,            1},
        {   M_POST,     "SetWLanConnection",            set_wlan_connection_cb,            1},
        {   M_POST,     "KickWirelessClientByMac",      kick_wireless_client_by_mac_cb,    0},
        {   M_POST,     "SetWLanRadioSettings",         set_wlan_radio_settings_cb,        1},
        {   M_POST,     "SetWLanRadioSecurity",         set_wlan_radio_security_cb,        1},
    /*-------------------------------- Sitecom Setting --------------------------------*/
#if 1//HAS_SC_AUTO_FW_CHECK
        {   M_GET,      "GetAutoFWupgradeStatus",       get_auto_fw_upgrade_status_cb,     1},
#if 0
        {   M_POST,     "SetAutoFWupgradeStatus",       set_auto_fw_upgrade_status_cb      1},
#endif
#endif
#if 0//HAS_SC_UTMPROXY_FUNCTION
        {   M_GET,      "GetSCSStatus",                 get_scs_status_cb                  1},
        {   M_POST,     "SetSCSStatus",                 set_scs_status_cb                  1},
#endif
#if HAS_STREAM_BOOST_SETTING
        {   M_GET,      "GetStreamBoost",               get_stream_boost_settings_cb,      1},
        {   M_POST,     "SetStreamBoost",               set_stream_boost_settings_cb,      1},
#endif
#if HAS_USB_SETTING
        /*---------------------------------- USB Setting ----------------------------------*/
        {   M_GET,      "GetUSBportSettings",           get_usb_port_settings_cb,          1},
        {   M_POST,     "SetUSBportSettings",           set_usb_port_settings_cb,          1},
#endif
#if APP_AGENT_SUPPORT_ENSHARE
        {   M_GET,      "GetStorageInfo",               get_storage_info_cb,                     1},
        {   M_POST,     "CheckIPLogin",                 check_ip_login_cb,                       0},
        {   M_POST,     "GenerateFileListByType",       generate_file_list_by_type_cb,           1},
        {   M_POST,     "CheckGenerateProcessByType",   check_generate_process_by_type_cb,       1},
        {   M_POST,     "GetFileListByType",            get_file_list_by_type_cb,                1},
        {   M_POST,     "AddFileIntoFileList",          add_file_into_file_list_cb,              1},
        {   M_POST,     "RenameFileInFileList",         rename_file_in_file_list_cb,             1},
        {   M_POST,     "DeleteFileFromFileList",       delete_file_from_file_list_cb,           1},
        {   M_POST,     "GetFileListUnderFolder",       get_file_list_under_folder_cb,           1},
        {   M_POST,     "GetFileListUnderFolderInFile", get_file_list_under_folder_in_file_cb,   1},
        {   M_POST,     "GetFolderPathByFileName",      get_folder_path_by_file_name_cb,         1},
        {   M_POST,     "DeleteFileByName",             delete_file_by_name_cb,                  1},
        {   M_POST,     "DeleteFileByFileName",         delete_file_by_file_name_cb,             1},
        {   M_POST,     "EditFilenameByName",           edit_filename_by_name_cb,                1},
        {   M_POST,     "AddFileIntoFavoriteList",      add_file_into_favorite_list_cb,          1},
        {   M_POST,     "DeleteFileFromFavoriteList",   delete_file_from_favorite_list_cb,       1},
        {   M_POST,     "AddFileIntoPublicList",        add_file_into_public_list_cb,            1},
        {   M_POST,     "DeleteFileFromPublicList",     delete_file_from_public_list_cb,         1},
        {   M_POST,     "ReloadDownsizedPicture",       reload_downsized_picture_cb,             1},
        {   M_POST,     "GetMusicInformation",          get_music_information_cb,                1},
        {   M_POST,     "SearchFileByName",             search_file_by_name_cb,                  1},
        {   M_POST,     "CreateFolder",                 create_folder_cb,                        1},
#endif
#if HAS_AP
		/*---------------------------------- AP Setting ----------------------------------*/
        {   M_GET,      "GetWanSettings",               get_ap_wan_settings_cb,            1},
        {   M_POST,     "SetWanSettings",               set_ap_wan_settings_cb,            1},
#endif
        {   M_GET,      "GetApWanAllSettings",          get_ap_wan_all_settings_cb,        1},
        {   M_POST,     "SetApWanAllSettings",          set_ap_wan_all_settings_cb,        1},
#if FOR_SC
        {   M_GET,      "GetSCWanSettings",             get_ap_wan_all_settings_cb,        1},
        {   M_POST,     "SetSCWanSettings",             set_ap_wan_all_settings_cb,        1},
#endif
        /*---------------------------------- TIME Setting ---------------------------------*/
        {   M_GET,      "GetTimeZoneCapability",        get_timezone_capability_cb,        1},
        {   M_GET,      "GetSysTimeSetting",            get_systime_setting_cb,            1},
        {   M_POST,     "SetSysTimeSetting",            set_systime_setting_cb,            1},
        /*---------------------------------- USER Setting ---------------------------------*/
        {   M_GET,      "GetUserList",                  get_user_list_cb,                  1},
        {   M_POST,     "UpdateUser",                   update_user_cb,                    1},
        {   M_POST,     "RemoveUser",                   remove_user_cb,                    1},
        //{   M_GET,      "GetAccountPassword",           get_account_password_cb,           1},
        {   M_POST,     "SetAccountPassword",           set_account_password_cb,           1},
#if FOR_SC
        //{   M_GET,      "GetSCAccountPassword",         get_account_password_cb,           1},
        {   M_POST,     "SetSCAccountPassword",         set_account_password_cb,           1},
#endif
        {   M_GET,      "RebootAllDevices",             reboot_all_devices_cb,             0},
        {   M_GET,      "ResetAllDevices",              reset_all_devices_cb,              0},
#if HAS_IPCAM
#if !APP_AGENT_SUPPORT_ENSHARE
        {   M_GET,      "GetDeviceStatus",              get_device_status_cb,              1},
#endif
        {   M_POST,     "SetOnvifDiscoveryMode",        set_onvif_discovery_mode_cb,       1},
        {   M_POST,     "SetOnvifScopes",               set_onvif_scopes_cb,               1},
        {   M_POST,     "SetIPCamSimpleNVR",            set_ipcam_simple_nvr_cb,           0},
        {   M_GET,      "GetRtspPortSettings",          get_rtsp_port_cb,                  1},
        {   M_POST,     "SetRtspPortSettings",          set_rtsp_port_cb,                  1},
        {   M_GET,      "GetSDcardSync",                get_sdcard_sync_cb,                0},
#endif
#if ROUTER_SUPPORT_IPCAM
        {   M_GET,      "GetIPCameraList",              get_ip_camera_list_cb,             1},
        {   M_GET,      "GetIPCamSambaFolderList",      get_ipcam_samba_folder_list_cb,    1},
        {   M_POST,     "GetIPCamFileListByDate",       get_ipcam_file_list_by_date_cb,    1},
        {   M_POST,     "GetPlayBackInfo",              get_play_back_info_cb,             1},
#endif
        /*---------------------------------- HTTP Setting ---------------------------------*/
        {   M_GET,      "GetHttpPortSettings",          get_http_port_cb,                  1},
        {   M_POST,     "SetHttpPortSettings",          set_http_port_cb,                  1},
        /*---------------------------------- UPNP Setting ---------------------------------*/
        {   M_GET,      "GetUpnpSettings",              get_upnp_settings_cb,              1},
        {   M_POST,     "SetUpnpSettings",              set_upnp_settings_cb,              1}, 
        /*---------------------------------- DDNS Setting ---------------------------------*/
        {   M_GET,      "GetDdnsSettings",              get_ddns_settings_cb,              1},
        {   M_POST,     "SetDdnsSettings",              set_ddns_settings_cb,              1},
#if HAS_ENGENIUS_DDNS
        {   M_POST,     "GetEnDdnsAliasNameAvailable", get_en_ddns_alias_name_available_cb,1},
#endif
        {   M_GET,      "GetDdnsProvider",              get_ddns_provider_cb,              1},
        {   M_GET,      "GetWifiInfo",                  get_wifiinfo_cb,                   1},
        {   M_POST,     "SetWifiInfo",                  set_wifiinfo_cb,                   1},
        {   M_GET,      "GetWifiOptList",               get_wifioptlist_cb,                1},
        {   M_GET,      "GetSystemInformationWithIP",   get_system_information_with_ip_cb, 1},
        {   M_GET,      "UpdateMeshNodesInfo",          update_mesh_node_info_cb,          1},
#if FOR_SC
        {   M_GET,      "GetSCWifiInfo",                get_wifiinfo_cb,                   1},
        {   M_POST,     "SetSCWifiInfo",                set_wifiinfo_cb,                   1},
        {   M_GET,      "GetSCStatus",                  get_system_information_with_ip_cb, 1},
        {   M_POST,     "SetSCFWAutoUpgrade",           set_sc_fw_auto_upgrade_cb,         1},
        {   M_GET,      "UpdateSCMeshNodesInfo",        update_mesh_node_info_cb,          1},
        {   M_POST,     "ExecuteSCFirmwareUpgrade",     execute_sc_firmware_upgrade_cb,    1},
#endif
        {   M_GET,      "MeshConnectedHistory",         get_mesh_connected_history_cb,     1},
        {   M_GET,      "GetSCMeshNodesInfo",           get_mesh_node_simplify_info_cb,    1},
        {   M_POST,     "ExecuteFirmwareUpgrade",       execute_firmware_upgrade_cb,       0},
        {   M_POST,     "DoManualFirmwareUpgrade",      do_manual_firmware_upgrade_cb,     0},
        {   M_POST,     "SetFWAutoUpgrade",             set_fw_auto_upgrade_cb,            0},
        {   M_GET,      "GetBaseStatus",                get_base_status_cb,                0},
        {   M_GET,      "GetMeshNodeSimplifyInfo",      get_mesh_node_simplify_info_cb,    0},
        {   M_GET,      "GetSNNumber",                  get_SN_number_cb,                  0},
        {   M_GET,      "GetInternetStatus",            get_internet_status_cb,            0},
        {   M_POST,     "SetCountryCode",               set_countrycode_cb,                1},
        /*---------------------------------------------------------------------------------*/
        {   -1,         NULL,                           NULL,                             -1}
};

#if HAS_MESH_JSON
APP_AGENT_SERVER_SUPPORT_TABLE_T app_agent_server_support_mesh_table[] =
{
	{   M_POST,     "SetMeshDeviceByWizard",               set_mesh_device_by_wizard_cb                ,0},
	{   M_POST,     "SetMeshNetworkProfile",               set_mesh_network_profile_cb                 ,0},
	{   M_POST,     "GetMeshBroadcastSettingStatus",       get_mesh_broadcast_setting_status_cb        ,0},
	{   M_POST,     "GetMeshSpecificOptionSetting",        get_mesh_specific_option_setting_cb         ,0},
	{   M_POST,     "GetMeshDeviceList",                   get_mesh_device_list_cb                     ,0},
	{   M_POST,     "GetMeshDeviceClientList",             get_mesh_device_client_list_cb              ,0},
	{   M_POST,     "InviteNewMeshDevice",                 invite_new_mesh_device_cb                   ,0},
	{   M_POST,     "SetMeshDeviceName",                   set_mesh_device_name_cb                     ,0},
	{   M_POST,     "ApplyMeshNetworkSettings",            apply_mesh_network_settings_cb              ,0},
	{   M_POST,     "GetMeshDeviceWirelessSettings",       get_mesh_device_wireless_settings_cb        ,0},
	{   M_POST,     "SetMeshDeviceWirelessSettings",       set_mesh_device_wireless_settings_cb        ,0},
	{   M_POST,     "SetMeshDeviceLEDAction",              set_mesh_device_led_action_cb               ,0},
	{   M_POST,     "GetMeshDeviceStatus",                 get_mesh_device_status_cb                   ,0},
	{   M_POST,     "DownloadMeshDeviceFirmware",          download_mesh_device_firmware_cb            ,0},
	{   M_POST,     "DownloadMeshDeviceFirmwareStatus",    download_mesh_device_firmware_status_cb     ,0},
	{   M_POST,     "DoMeshDeviceFirmwareUpgrade",         do_mesh_device_firmware_upgrade_cb          ,0},
	{   M_POST,     "DoMeshSingleDeviceFirmwareUpgrade",   do_mesh_single_device_firmware_upgrade_cb   ,0},
#if SUPPORT_IPERF_THROUGHPUT_TEST
    {   M_POST,     "RunMeshThroughputTest",               run_mesh_throughput_test_cb                 ,0},
	{   M_POST,     "GetMeshThroughputTestResult",         get_mesh_throughput_test_result_cb          ,0},
#endif
#if HAS_SPEEDTEST_THROUGHPUT_TEST
	{   M_POST,     "RunMeshInternetSpeedTest",            run_mesh_speedtest_cb                       ,0},
	{   M_POST,     "GetMeshInternetSpeedTestResult",      get_mesh_speedtest_result_cb                ,0},
	{   M_POST,     "FindSpeedTestBestServer",             find_mesh_speedtest_best_server_cb          ,0},
	{   M_POST,     "GetSpeedTestBestServerResult",        get_mesh_speedtest_best_server_result_cb    ,0},
#endif
	{   M_POST,     "RunMeshPingTest",                     run_mesh_ping_test_cb                       ,0},
	{   M_POST,     "GetMeshPingTestResult",               get_mesh_ping_test_result_cb                ,0},
	{   M_POST,     "RunMeshTraceRoute",                   run_mesh_trace_route_cb                     ,0},
	{   M_POST,     "GetMeshTraceRouteResult",             get_mesh_trace_route_result_cb              ,0},
    {   M_POST,     "GetMeshTraceRouteSimpleResult",       get_mesh_trace_route_simple_result_cb       ,0},
	{   M_POST,     "GetMeshDeviceNeighbors",              get_mesh_device_neighbors_cb                ,0},
	{   M_POST,     "SyncMeshRobustThreshold",             sync_mesh_robust_threshold_cb               ,0},
	{   M_POST,     "SyncMeshMSCConfigured",               sync_mesh_msc_configured_cb                 ,0},
	{   M_POST,     "GetMeshNodeInfo",                     get_mesh_node_info_cb                       ,0},
	{   M_POST,     "GetLoginMeshNodeInfo",                get_login_mesh_info_cb                      ,0},
    {   M_POST,     "SendSystemCmdToMeshDevice",           send_system_cmd_to_mesh_device_cb           ,0},
    {   M_POST,     "SendUciChangesToMeshDevice",          send_uci_changes_to_mesh_device_cb          ,0},
    {   M_POST,     "SetMeshDeviceReboot",                 set_mesh_device_reboot_cb                   ,0},
    {   M_POST,     "SetMeshWifiDisabled",                 set_mesh_wifi_disabled_cb                   ,0},
    {   M_POST,     "SetMeshWifiTriggerWps",               set_mesh_wifi_trigger_wps_cb                ,0},
    {   M_POST,     "CheckMeshWifiTriggerWps",             check_mesh_wifi_trigger_wps_cb              ,0},
    {   M_POST,     "SetMeshWifiLocation",                 set_mesh_wifi_Location_cb                   ,0},
    {   M_POST,     "SetMeshLedDisabled",                  set_mesh_led_disabled_cb                    ,0},
    {   M_POST,     "GetMeshBasicModeWifiInfo",            get_mesh_basic_mode_wifi_info_cb            ,0},
    {   M_POST,     "DeleteMeshDevice",                    delete_mesh_device_cb                       ,0},
    {   M_POST,     "MeshRebootDevice",                    mesh_reboot_device_cb                       ,0},
    {   M_POST,     "MeshResetDevice",                     mesh_reset_device_cb                        ,0},
    {   M_POST,     "SetMeshDeviceUSB",                    set_mesh_device_usb_cb                      ,0},
    {   M_POST,     "GetMeshDeviceUSB",                    get_mesh_device_usb_cb                      ,0},
    {   M_POST,     "GetMeshDeviceUSBInfo",                get_mesh_device_usb_info_cb                 ,0},
    {   M_POST,     "GetMeshHomeInfo",                     get_mesh_home_info_cb                       ,0},
    {   M_POST,     "MeshResetTargetDevice",               mesh_reset_target_device_cb                 ,0},
#if SUPPORT_PEOPLE_FUNCTION
	{   M_POST,     "GetMeshSimplePeopleInfo",             get_mesh_simple_people_info_cb              ,0},
	{   M_POST,     "GetMeshUserProfile",                  get_mesh_user_profile_cb                    ,0},
	{   M_POST,     "GetMeshUserProfileList",              get_mesh_user_profile_list_cb               ,0},
	{   M_POST,     "SetMeshUserProfile",                  set_mesh_user_profile_cb                    ,0},
	{   M_POST,     "SetMeshUserProfileList",              set_mesh_user_profile_list_cb               ,0},
	{   M_POST,     "DeleteMeshUserProfile",               delete_mesh_user_profile_cb                 ,0},
	{   M_POST,     "GetMeshClientProfile",                get_mesh_client_profile_cb                  ,0},
	{   M_POST,     "GetMeshClientProfileList",            get_mesh_client_profile_list_cb             ,0},
	{   M_POST,     "SetMeshClientProfile",                set_mesh_client_profile_cb                  ,0},
	{   M_POST,     "DeleteMeshClientProfile",             delete_mesh_client_profile_cb               ,0},
	{   M_POST,     "DeleteMeshClientProfileList",         delete_mesh_client_profile_list_cb          ,0},
#if SUPPORT_FIREWALL_URL_CATEGORY
	{   M_POST,     "GetMeshFirewallDefaultURLCategory",   get_mesh_firewall_default_url_category_cb   ,0},
#endif
	{   M_POST,     "GetMeshFirewallRule",                 get_mesh_firewall_rule_cb                   ,0},
	{   M_POST,     "GetMeshFirewallRuleList",             get_mesh_firewall_rule_list_cb              ,0},
	{   M_POST,     "SetMeshFirewallRule",                 set_mesh_firewall_rule_cb                   ,0},
	{   M_POST,     "DeleteMeshFirewallRule",              delete_mesh_firewall_rule_cb                ,0},
	{   M_POST,     "BlockMeshUser",                       block_mesh_user_cb                          ,0},
	{   M_POST,     "BlockMeshClient",                     block_mesh_client_cb                        ,0},
	{   M_POST,     "ResetAllBlockedMeshClient",           reset_all_blocked_mesh_client_cb            ,0},
#endif
	{   M_POST,     "EzSetupBcastPkts",                    ezsetup_broadcast_pkts_cb                   ,0},
	{   M_POST,     "EzSetupGetCandidates",                ezsetup_get_candidates_cb                   ,0},
	{   M_POST,     "EzSetupAddMeshNotes",                 ezsetup_add_mesh_nodes_cb                   ,0},
	{   M_POST,     "EzSetupGetFailMeshNodes",             ezsetup_get_mesh_fail_nodes_cb              ,0},
    // {   M_POST,     "SetMeshDeviceIbeacon",                set_mesh_device_ibeacon_cb                  ,0},
    // {   M_POST,     "GetMeshDeviceIbeacon",                get_mesh_device_ibeacon_cb                  ,0},
    {   M_POST,     "GetMeshDeviceClientInfo",             get_mesh_divece_client_info_cb              ,0},
#if HAS_MESH_STATIC_ROUTE
    {   M_POST,     "SetMeshAllowMacList",                 set_mesh_allow_mac_list_cb                  ,0},
    {   M_POST,     "InformMeshStaticRoute",               inform_mesh_static_route                    ,0},
#endif
	{   -1,         NULL,                                  NULL                                        , -1}
};
#endif
/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
#if HAS_REDIRECT_DEVICE_SETTING
/*****************************************************************
* NAME:    redirect_to_target_device
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
char *redirect_to_target_device(HTTPS_CB *pkt, char *target_ip)
{
	char buf_payload[65536];
	char buf_result[65536];
	char *replace_str=NULL;
	char *query_str;
	char ipcam_app_agent_port[16] = {0};
#if HAS_IPCAM
	char init_IP_Port[64]={0};
	char final_IP_Port[64]={0};
#endif

	api_get_string_option("xrelayd.xrelayd.ipcam_app_agent_port", ipcam_app_agent_port, sizeof(ipcam_app_agent_port));

    memset(buf_payload, 0x00, sizeof(buf_payload));
    memset(buf_result, 0x00, sizeof(buf_result));

	if (pkt->method == M_POST)
	{
		query_str = get_env(&pkt->envcfg, "QUERY_STRING");
		sysCheckStringOnWeb(query_str, buf_payload);
		replace_str = str_replace(buf_payload, "\n", "");

		if(ipcam_app_agent_port[0] == '\0')
		{
			sysutil_interact(buf_result, sizeof(buf_result),
				"app_client -i %s -m POST -a %s -e 1 -p \"%s\"",
				target_ip, pkt->json_action, replace_str);
		}
		else
		{
			sysutil_interact(buf_result, sizeof(buf_result),
				"app_client -i %s -m POST -a %s -e 1 -P %s -p \"%s\"",
				target_ip, pkt->json_action, ipcam_app_agent_port, replace_str);
		}

		free(replace_str);
	}
	else
	{
		if(ipcam_app_agent_port[0] == '\0')
		{
			sysutil_interact(buf_result, sizeof(buf_result),
				"app_client -i %s -m GET -a %s -e 1",
				target_ip, pkt->json_action);
		}
		else
		{
			sysutil_interact(buf_result, sizeof(buf_result),
				"app_client -i %s -m GET -a %s -e 1 -P %s",
				target_ip, pkt->json_action, ipcam_app_agent_port);
		}
	}

#if HAS_IPCAM
	if(strstr(buf_result, target_ip))
	{
		char ip_addr[24]={0}, port[8]={0};

		if(strstr(buf_result, "5540"))	/* get video setting */
		{
			sprintf(init_IP_Port, "%s:5540", target_ip);
			printf("\n[app_agent] tmp_IP_Port=(%s)\n", init_IP_Port);
			get_IP_Port(pkt->fd, 5540, ip_addr, port);
			sprintf(final_IP_Port, "%s:%s", ip_addr, port);
			printf("\n[app_agent] final_IP_Port=(%s)\n", final_IP_Port);
			replace_str = str_replace(buf_result, init_IP_Port, final_IP_Port);
		}
		else if(strstr(buf_result, "9000"))	/* GetFileListByDate */
		{
			sprintf(init_IP_Port, "%s:9000", target_ip);
			printf("\n[app_agent] tmp_IP_Port=(%s)\n", init_IP_Port);
			get_IP_Port(pkt->fd, 9000, ip_addr, port);
			printf("\n[app_agent] final_IP_Port=(%s:%s)\n", ip_addr, port);
			replace_str = str_replace(buf_result, target_ip, ip_addr);
			if (strcmp(port, "9000")!=0)
			{
				char replace_port[32]={0};

				memset(buf_payload, 0x00, sizeof(buf_payload));
				strcpy(buf_payload, replace_str);
				free(replace_str);

				sprintf(replace_port, "\"FileListDownPort\": %s", port);
				replace_str = str_replace(buf_payload, "\"FileListDownPort\": 9000", replace_port);
			}
		}
	}
	else
#endif
		replace_str = strdup(buf_result);

	return replace_str;
}
#endif
/*****************************************************************
 * NAME:    app_agent_process
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
void app_agent_process(int s, HTTPS_CB *pkt)
{
    bool matched=FALSE;
    int i=0;
    char *action;
    char *return_str=ERROR_STR;
    int nAuth, login_result, refresh_result;
    APP_AGENT_SERVER_SUPPORT_TABLE_T *app_agent_table_ptr=NULL;
#if HAS_REDIRECT_DEVICE_SETTING
	char targeIP[16]={0};
#endif
    int line;
    int existed;
    int removed;
    char ip_addr[32];
    char ip_md5[128];
    char buf[128];
    char first_record_ip[128];
    char *ptr;
    FILE *fp;

    if(NULL == pkt)
    {
        return;
    }

    if(strlen(pkt->json_action))
    {
        action = pkt->json_action;
    }

    if(!strlen(action))
    {
        return;
    }

#if HAS_MESH_JSON
    if(pkt->mesh)
    {
        app_agent_table_ptr = app_agent_server_support_mesh_table;
    }
    else
#endif
    {
        app_agent_table_ptr = app_agent_server_support_table;
    }
    /*check corresponding admin time accroding to peer ip*/
    if (AGENT_AUTH_TIMEOUT == (nAuth = App_AuthCheck (s, pkt))
#if HAS_MESH_JSON
       /* The authentication checking is not necessary if the API belongs to MESH. */
    && (0 == pkt->senao_app_client)
#endif
       )
    {
        //Time out!
        if(0 != strcasecmp(action, "Login"))
        {
            sprintf(pkt->json_action, "Login");
            send_simple_response(pkt, ERROR_STR);
            return;
        }
    }

    /*if not timeout, it may exist corresponding ip and admin time or no information in data structure.
     *So we must force to direct "Login" pkt to check login function, then deal with no match ip pkt to response login fail.
     */
    if(0 == strcasecmp(action, "Login") || 0 == strcasecmp(action, "ForceLogin"))
    {
        if(M_POST == pkt->method)
        {
            login_result=login_cb(pkt);
        }
        else
        {
            send_simple_response(pkt, ERROR_HTTP_METHOD_STR);
            return;
        }

        if(AGENT_LOGIN_FAIL != login_result)
        {
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
            refresh_result=Login_AuthRefresh(s, pkt, login_result);
#else
            refresh_result=Login_AuthRefresh(s, pkt);
#endif
            return_str = OK_STR;
        }

        if(AGENT_LOGIN_FAIL == login_result || AGENT_AUTH_REFRESH_FAIL == refresh_result)
        {
            DBprintf("auth refrsh failed\n");
            login_result = FALSE;
            return_str = ERROR_STR;
        }
        else if(AGENT_AUTH_REFRESH_TABLE_FULL == refresh_result)
        {
            DBprintf("auth table full\n");

            if(0 == strcasecmp(action, "ForceLogin"))
            {
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
                Flush_APP_Login_Auth(s, pkt, login_result);
                refresh_result=Login_AuthRefresh(s, pkt, login_result);
#else
                Flush_Login_Auth(s, pkt);
                refresh_result=Login_AuthRefresh(s, pkt);
#endif

                if(AGENT_AUTH_REFRESH_FAIL == refresh_result)
                {
                    DBprintf("auth refrsh failed\n");
                    login_result = FALSE;
                    return_str = ERROR_STR;
                }
                else if(AGENT_AUTH_REFRESH_SUCCESS == refresh_result)
                {
                    login_result = TRUE;
                }
            }
            else
            {
                login_result = FALSE;
                return_str = ERROR_FULL_ACCOUNT_WITH_KICK_STR;
            }
        }
        else if(AGENT_AUTH_REFRESH_SUCCESS == refresh_result)
        {
            login_result = TRUE;
        }

#if HAS_ENGENIUS
	//authentication process finished, response fail or more information
	(TRUE == login_result)?login_response_json_cb(pkt):send_simple_response(pkt, return_str);
#else
        //authentication process finished, send the success response
        send_simple_response(pkt, return_str);
#endif
#if HAS_ENGENIUS
	if (TRUE == login_result)
#endif
    {
        fp = NULL;
        ptr = NULL;
        line = 0;
        existed = FALSE;
        removed = FALSE;
        memset(first_record_ip, 0x00, sizeof(first_record_ip));

        sprintf(ip_addr, "%s", sysutil_get_peername(s));
        sysutil_string_to_md5(ip_addr, strlen(ip_addr), ip_md5);

        if(NULL != (fp = fopen(APP_AGENT_LOGIN_IP_FILE_NAME, "a+")))
        {
            while(NULL != fgets(buf, sizeof(buf), fp))
            {
                if(0 == strlen(first_record_ip))
                {
                    sprintf(first_record_ip, "%s", buf);
                }

                if(ptr = strchr(buf, '\n'))
                {
                    *ptr = '\0';
                }

                if(0 == strcmp(buf, ip_md5))
                {
                    existed = TRUE;
                }
            }

            if(FALSE == existed)
            {
                sysutil_get_file_line_num(APP_AGENT_LOGIN_IP_FILE_NAME, &line);

                // Write the IP into the file
                // and remove the first record if the records of the file is larger than 10.
                if(line >= 10)
                {
                    removed = TRUE;
                }
                fprintf(fp, "%s\n", ip_md5);
            }

            fclose(fp);

            // Remove the first record.
            if(TRUE == removed)
            {
                sysutil_remove_string_from_file(first_record_ip, APP_AGENT_LOGIN_IP_FILE_NAME);
            }
        }
    }

	return;
    }
    else if(0 == strcasecmp(action, "Logout"))
    {
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
        login_result = logout_cb(pkt);

        refresh_result=Logout_Auth(s, pkt, login_result);
#else
        refresh_result=Logout_Auth(s, pkt);
#endif
        send_simple_response(pkt, (AGENT_AUTH_NO_ERROR == refresh_result)?OK_STR:ERROR_STR);

        return;
    }
    else if(0 == strcasecmp(action, "FlushAllLoginAccount"))
    {
        refresh_result=Flush_Login_Auth(s, pkt);

        send_simple_response(pkt, (AGENT_AUTH_NO_ERROR == refresh_result)?OK_STR:ERROR_STR);

        return;
    }
	else if(1 == pkt->senao_app_client)
	{
		for(i = 0; NULL != app_agent_table_ptr[i].method; i++){
			if ((0 == app_agent_table_ptr[i].needAuth))
			{
				if(0 == strcasecmp(action, app_agent_table_ptr[i].method)){
					if (app_agent_table_ptr[i].http_method == pkt->method)
						app_agent_table_ptr[i].handler_cb(pkt);
					else
						send_simple_response(pkt, ERROR_HTTP_METHOD_STR);
					return;
				}
			}
		}
#if HAS_MESH_JSON
		 /* This API does not belong to MESH. */
		if(pkt->mesh)
			goto no_matched_api;
#endif
	}

    if (AGENT_AUTH_NO_MATCH_IP == nAuth)
    {
        DBprintf("%s ---> table No Match ip\n",__FUNCTION__);

        if(0 != strcasecmp(action, "Login"))
        {
            sprintf(pkt->json_action, "Login");
        }
        send_simple_response(pkt, ERROR_STR);
        return;
    }

    if (AGENT_AUTH_CHECK_OK == nAuth)
    {
        /*refresh login time, when receiving every pkt.*/
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
        Login_AuthRefresh(s, pkt, AGENT_LOGIN_OK);
#else
        Login_AuthRefresh(s, pkt);
#endif

        for(i = 0; NULL != app_agent_table_ptr[i].method; i++)
        {
            if(0 == strcasecmp(action, app_agent_table_ptr[i].method))
            {
                /* Make sure that the HTTP method is correct. */
                if(app_agent_table_ptr[i].http_method == pkt->method)
                {
                    app_agent_table_ptr[i].handler_cb(pkt);
                }
                else
                {
                    send_simple_response(pkt, ERROR_HTTP_METHOD_STR);
                }

                matched = TRUE;
                break;
            }
        }

#if HAS_REDIRECT_DEVICE_SETTING
        if(FALSE == matched)
        {
            char *ret = NULL;

#if HAS_IPCAM
			api_get_string_option("xrelayd.xrelayd.conn_sec_ip", targeIP, sizeof(targeIP));
#else
			sprintf(targeIP,"%s",DEVICE_IPADDR);
#endif
            ret = redirect_to_target_device(pkt, targeIP);

            if(NULL == strstr(ret, "ERROR_HOST_UNREACHABLE"))
            {
                basic_json_response(pkt, ret);
                free(ret);

                return;
            }

            free(ret);
        }
#endif
    }

    if(FALSE == matched)
    {
no_matched_api:
        send_simple_response(pkt, ERROR_API_NOT_SUPPORTED_STR);
    }
}

/*****************************************************************
 * NAME:    send_simple_response
 * ---------------------------------------------------------------
 * FUNCTION:  Simple HTTP response packet format
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int send_simple_response(HTTPS_CB *pkt, char *result)
{
    if(NULL == pkt)
    {
        return -1;
    }

    if(pkt->json)
    {
        simple_json_response(pkt, result);
    }

    return 0;
}
