#include <sn_api.h>
#include <jwt_api.h>

#define G_GUEST 1 //0x01
#define G_ADMIN 2 //0x10

ApiEntry basicTable[] = 
{
    //  1       2                       3                   4                    5         6     7      METHOD  GROUP             callback_function
    {"version", "",                     "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_api_version},
    {"sys",     "login",                "",                 "",                  "",       "",   "",   "POST",  G_GUEST|G_ADMIN   ,post_sys_login},
    {"sys",     "account_username",     "",                 "",                  "",       "",   "",   "GET",   G_ADMIN           ,get_account_username},
    {"sys",     "change_account",       "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_change_account},
    {"sys",     "fw_upgrade",           "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_sys_fw_upgrade},
    {"sys",     "sys_info",             "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_sys_sys_info},
    {"sys",     "controlled_info",      "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_sys_controlled_info},
    {"sys",     "system_config",        "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_sys_system_config},
    {"sys",     "system_config",        "",                 "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_sys_system_config},
    {"sys",     "apply",                "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_sys_apply},
    {"sys",     "reload",               "",                 "",                  "",       "",   "",   "GET",   G_ADMIN           ,post_sys_reload},
    {"sys",     "revert",               "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_sys_revert},
    {"sys",     "dev_capability",       "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_dev_capability},
    {"sys",     "first_login",          "",                 "",                  "",       "",   "",   "GET",   G_ADMIN           ,get_first_login},
    {"sys",     "first_login",          "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_first_login},
#if HAS_SENAO_PACKETEER
    {"sys",     "benu_tunnel_addr",     "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_benu_tunnel_addr},
    {"sys",     "benu_tunnel_addr",     "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_benu_tunnel_addr},
#endif
    {"sys",     "syslog",               "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_sys_syslog},
    {"sys",     "syslog",               "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_sys_syslog},
    {"sys",     "syslog",               "",                 "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_sys_syslog},
    {"wifi",    "radio",                "24g",              "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_radio_24g},
    {"wifi",    "radio",                "24g",              "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_radio_24g},
    {"wifi",    "radio",                "24g",              "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_radio_24g},
    {"wifi",    "radio",                "5g",               "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_radio_5g},
    {"wifi",    "radio",                "5g",               "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_radio_5g},
    {"wifi",    "radio",                "5g",               "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_radio_5g},
    {"wifi",    "radio",                "5g_2",             "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_radio_5g_2},
    {"wifi",    "radio",                "5g_2",             "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_radio_5g_2},
    {"wifi",    "radio",                "5g_2",             "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_radio_5g_2},
    {"wifi",    "mesh",                 "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_mesh},
    {"wifi",    "mesh",                 "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_mesh},
    {"wifi",    "mesh",                 "throughput",       "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_mesh_throughput},
    {"wifi",    "mesh",                 "throughput",       "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_mesh_throughput},
    {"wifi",    "mesh",                 "mesh_info",        "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_mesh_mesh_info},
    {"wifi",    "mesh",                 "mesh_group_info",  "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_mesh_group_info},
    {"wifi",    "ssids",                "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_ssids},
#if !SUPPORT_SWOS_FUNCTION
    {"wifi",    "mgmt",                 "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_mgmt},
    {"wifi",    "mgmt",                 "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_mgmt},
    {"wifi",    "mgmt",                 "",                 "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_mgmt},
#endif
    {"wifi",    VARIABLE_OPMODE,        "wds_link",         "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_wds_link},
    {"wifi",    VARIABLE_OPMODE,        "wds_link",         "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_wds_link},
    {"wifi",    VARIABLE_OPMODE,        "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_sta_mode},
    {"wifi",    VARIABLE_OPMODE,        "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_sta_mode},
    {"wifi",    VARIABLE_OPMODE,        "",                 "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_sta_mode},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_ssid},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_ssid},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_ssid},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "security",          "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_ssid_security},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "security",          "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_ssid_security},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "security",          "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_ssid_security},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "guest_network",     "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_ssid_guestnetwork},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "guest_network",     "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_ssid_guestnetwork},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "guest_network",     "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_ssid_guestnetwork},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "captive_portal",    "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_ssid_captive_portal},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "captive_portal",    "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_ssid_captive_portal},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "captive_portal",    "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_ssid_captive_portal},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "hs20",              "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_ssid_hs20},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "hs20",              "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_ssid_hs20},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "accounting_server", "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_ssid_accountingserver},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "accounting_server", "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_ssid_accountingserver},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "accounting_server", "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_ssid_accountingserver},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "traffic_shaping",   "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_ssid_trafficshaping},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "traffic_shaping",   "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_ssid_trafficshaping},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "traffic_shaping",   "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_ssid_trafficshaping},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "band_steering",     "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_ssid_bandsteering},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "band_steering",     "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_ssid_bandsteering},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "band_steering",     "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_ssid_bandsteering},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "radius_server",     "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_ssid_radiusserver},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "radius_server",     "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_ssid_radiusserver},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "radius_server",     "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_ssid_radiusserver},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "scheduling",        "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_ssid_scheduling},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "scheduling",        "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_ssid_scheduling},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "scheduling",        "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_ssid_scheduling},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "l2_acl",            "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_ssid_l2_acl},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "l2_acl",            "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_ssid_l2_acl},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "l2_acl",            "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_ssid_l2_acl},
    {"wifi",    VARIABLE_OPMODE,        VARIABLE,           "kick",              "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_ssid_kick},
	{"wifi",    VARIABLE_OPMODE,        VARIABLE,           "traffic_info",      "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_ssid_traffic_info},
    {"wifi",    "wifi_client_info",     "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_client_info},
    {"wifi",    "site_survey",          "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_site_survey},
    {"wifi",    "site_survey",          "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_site_survey},
    {"wifi",    "channel_list",         "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_channel_list},
    {"wifi",    "channel_list",         "",                 "",                  "",       "",   "",   "POST",  G_GUEST|G_ADMIN   ,post_wifi_channel_list},
    {"wifi",    "guest_network",        "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_guest_network},
    {"wifi",    "guest_network",        "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_guest_network},
    {"wifi",    "guest_network",        "",                 "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_guest_network},
    {"wifi",    "scan",                 "chanutil",         "",                  "",       "",   "",   "GET",   G_ADMIN           ,get_wifi_scan_chanutil},
    {"wifi",    "scan",                 "ap_list",          "",                  "",       "",   "",   "GET",   G_ADMIN           ,get_wifi_scan_aplist},
    {"wifi",    "scan",                 "sta_list",         "",                  "",       "",   "",   "GET",   G_ADMIN           ,get_wifi_scan_stalist},
    {"wifi",    "vpn",                  "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_wifi_vpn_profile},
    {"wifi",    "vpn",                  "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_wifi_vpn_profile},
    {"wifi",    "vpn",                  "",                 "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_vpn_profile},
    {"wifi",    "delete_all_vpn",       "",                 "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_wifi_delete_all_vpn_profile},
    {"wifi",    "vpn_status",           "",                 "",                  "",       "",   "",   "GET",   G_ADMIN           ,get_wifi_vpn_status},
    {"net",     "ethernet",             "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_net_ethernet},
    {"net",     "ethernet",             "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_net_ethernet},
    {"net",     "ethernet",             "",                 "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_net_ethernet},
	{"net",     "ethernet",             "traffic_info",     "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_net_ethernet_traffic_info},
    {"net",     "ethernet",             "client_info",      "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_net_ethernet_client_info},
    {"net",     "linktest",             "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_linktest},
    {"net",     "linktest",            VARIABLE,                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_linktest},
    {"net",     "proxy",                "http",             "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_net_proxy_http},
    {"net",     "proxy",                "http",             "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_net_proxy_http},
    {"net",     "proxy",                "https",            "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_net_proxy_https},
    {"net",     "proxy",                "https",            "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_net_proxy_https},
    {"net",     "iptable_rules",        "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_net_iptable_rules},
    {"net",     "iptable_rules",        "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_net_iptable_rules},
    {"net",     "iptable_rules",        "",                 "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_net_iptable_rules},
    {"net",     "proxy",                "settings",         "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_net_proxy_settings},
    {"net",     "proxy",                "settings",         "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_net_proxy_settings},
    {"net",     "proxy",                "settings",         "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_net_proxy_settings},
    {"net",     "spanning_tree",        "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_net_spanning_tree},
    {"net",     "spanning_tree",        "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_net_spanning_tree},
    {"net",     "spanning_tree",        "",                 "",                  "",       "",   "",   "PATCH", G_ADMIN           ,patch_net_spanning_tree},
    {"net",     "internet_speedtest",   "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_net_internet_speedtest},
    {"net",     "internet_speedtest",   "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_net_internet_speedtest},
    {"auth",    "check",       	        "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_auth_check},
    {"mgm",     "fw_upgrade",           "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_fwupgrade},
    {"mgm",     "fw_upgrade_only",      "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_fwupgrade_only},
    {"mgm",     "drop_caches",          "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_drop_caches},
    {"mgm",     "ocu",                  "fw_check",         "",                  "",       "",   "",   "POST",  G_GUEST|G_ADMIN   ,post_mgm_fw_check},
    {"mgm",     "ocu",                  "fw_check",         "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_mgm_fw_check},
    {"mgm",     "ocu",                  "fw_download",      "",                  "",       "",   "",   "POST",  G_GUEST|G_ADMIN   ,post_mgm_fw_download},
    {"mgm",     "ocu",                  "fw_download",      "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_mgm_fw_download},
    {"mgm",     "tools",                "iperf",            "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_tools_iperf},
    {"mgm",     "tools",                "iperf",            "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_mgm_tools_iperf},
    {"mgm",     "backup_config",        "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_backup_config},
    {"mgm",     "backup_config",        "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_mgm_backup_config},
    {"mgm",     "restore_config",       "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_restore_config},
    {"mgm",     "local_upgrade_image",  "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_mgm_localupgradeimage},
    {"mgm",     "restore_image",        "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_mgm_restoreimage},
    {"mgm",     "reboot",    	        "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_reboot},
    {"mgm",     "auto_reboot_cfg",      "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_auto_reboot_cfg},
    {"mgm",     "auto_reboot_cfg",      "",                 "",                  "",       "",   "",   "GET",   G_ADMIN           ,get_mgm_auto_reboot_cfg},
    {"mgm",     "reset_to_default",     "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_reset_to_default},
    {"mgm",     "reset_with_key",       "",                 "",                  "",       "",   "",   "POST",  G_GUEST|G_ADMIN   ,post_mgm_reset_with_key},
    {"mgm",     "led_list",             "",                 "",                  "",       "",   "",   "GET",   G_ADMIN           ,get_mgm_led_list},
    {"mgm",     "led_status",           "",                 "",                  "",       "",   "",   "GET",   G_ADMIN           ,get_mgm_led_status},
    {"mgm",     "led_cfg",              "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_led_cfg},
    {"mgm",     "tools",                "ping",             "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_ping},
    {"mgm",     "tools",                "ping",             "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_mgm_ping},
    {"mgm",     "tools",                "traceroute",       "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_traceroute},
    {"mgm",     "tools",                "traceroute",       "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_mgm_traceroute},
    {"mgm",     "tools",                "nslookup",         "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_nslookup},
    {"mgm",     "tools",                "nslookup",         "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_mgm_nslookup},
    {"mgm",     "tools",                "device_discovery", "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_device_discovery},
    {"mgm",     "tools",                "device_discovery", "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_mgm_device_discovery},
#if !SUPPORT_SWOS_FUNCTION
    {"mgm",     "tools",                "gps",              "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_gps},
    {"mgm",     "tools",                "gps",              "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_mgm_gps},
#endif
#if SUPPORT_NETGEAR_FUNCTION
    {"mgm",     "mgm_mode",    	        "",                 "",                  "",       "",   "",   "GET",   G_ADMIN           ,get_mgm_mode},
    {"mgm",     "mgm_mode",    	        "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_mgm_mode},
#endif
    {"mgm",     "ssh",                  "",                 "",                  "",       "",   "",   "GET",   G_ADMIN           ,get_ssh_setting},
    {"mgm",     "ssh",                  "",                 "",                  "",       "",   "",   "POST",  G_ADMIN           ,post_ssh_setting},
#if !SUPPORT_SWOS_FUNCTION
    {"ezmcloud","info",                 "",                 "",                  "",       "",   "",   "GET",   G_GUEST|G_ADMIN   ,get_ezmcloud_info},
#endif
    {"lxc",     "ezm",                  "backup",           "info",              "",       "",   "",   "GET",   G_ADMIN           ,get_lxc_ezm_backup_info},
    {"lxc",     "ezm",                  "backup",           "method",            "",       "",   "",   "GET",   G_ADMIN           ,get_lxc_ezm_backup_method},
    {"lxc",     "ezm",                  "backup",           "method",            "",       "",   "",   "POST",  G_ADMIN           ,post_lxc_ezm_backup_method},
    {"lxc",     "ezm",                  "restore",          "",                  "",       "",   "",   "GET",   G_ADMIN           ,get_lxc_ezm_restore},
//    {"check",   "wifi",                 "radio",       "24g",          "",                 "",      "","POST",  G_ADMIN           ,check_wifi_radio_24g},
//    {"check",   "wifi",                 "radio",       "5g",           "",                 "",      "","POST",  G_ADMIN           ,check_wifi_radio_5g},
//    {"check",   "wifi",                 "radio",       "triband",      "5g",               VARIABLE,"","POST",  G_ADMIN           ,check_wifi_radio_triband_5g},
//    {"check",   "wifi",                 "mesh",        "",             "",                 "",      "","POST",  G_ADMIN           ,check_wifi_mesh},
//    {"check",   "wifi",                 "ssid",        VARIABLE,       "",                 "",      "","POST",  G_ADMIN           ,check_wifi_ssid},
//    {"check",   "wifi",                 "ssid",        VARIABLE,       "security",         "",      "","POST",  G_ADMIN           ,check_wifi_ssid_security},
//    {"check",   "wifi", "ssid",     VARIABLE,    "captive_portal",   "",                         "",   "POST",  G_ADMIN           ,check_wifi_ssid_captiveportal},
//    {"check",   "wifi", "ssid",     VARIABLE,    "accounting_server","",                         "",   "POST",  G_ADMIN           ,check_wifi_ssid_accountingserver},
//    {"check",   "wifi", "ssid",     VARIABLE,    "traffic_shaping",  "",                         "",   "POST",  G_ADMIN           ,check_wifi_ssid_trafficshaping},
//    {"check",   "wifi", "ssid",     VARIABLE,    "band_steering",    "",                         "",   "POST",  G_ADMIN           ,check_wifi_ssid_bandsteering},
//    {"check",   "wifi", "ssid",     VARIABLE,    "radius_server",    "",                         "",   "POST",  G_ADMIN           ,check_wifi_ssid_radiusserver},
//    {"check",   "wifi", "ssid",     VARIABLE,    "scheduling",       "",                         "",   "POST",  G_ADMIN           ,check_wifi_ssid_scheduling},
//    {"check",   "wifi", "ssid",     VARIABLE,    "l2_acl",           "",                         "",   "POST",  G_ADMIN           ,check_wifi_ssid_l2_acl},
//    {"check",   "net",  "ethernet", "",          "",                 "",                         "",   "POST",  G_ADMIN           ,check_net_ethernet},
//    {"check",   "sys",  "system_config","",      "",                 "",                         "",   "POST",  G_ADMIN           ,check_sys_system_config},
//    {"check",   "sys",  "fw_upgrade","",         "",                 "",                         "",   "POST",  G_ADMIN           ,check_sys_fw_upgrade}
};

//Global variable
char queryTable[256];
int basicTableLen = (sizeof(basicTable)/sizeof(basicTable[0]));
int triband_5g_idx=0; //Global value for /wifi/radio/triband/5g/{id} :URI_5
int ssid_idx=0; //Global value for /wifi/ssid_mode/{id} :URI_3
char ssid_mode[32]={0}; //Global value for /wifi/{ssid_mode}/{id} :URI_2

ssid_cfg_st ssidCfg;

#define SSID_MODE_AP "ap"
#define SSID_MODE_WDS_AP_24G "wds_ap_24g"
#define SSID_MODE_WDS_AP_5G "wds_ap_5g"
#define SSID_MODE_WDS_AP_5G_2 "wds_ap_5g_2"
#define SSID_MODE_ENJET "enjet"
#define SSID_MODE_STA_24G "sta_24g"
#define SSID_MODE_STA_5G "sta_5g"
#define SSID_MODE_WDS_STA_24G "wds_sta_24g"
#define SSID_MODE_WDS_STA_5G "wds_sta_5g"
#define SSID_MODE_WDS_BRIDGE_24G "wds_bridge_24g"
#define SSID_MODE_WDS_BRIDGE_5G "wds_bridge_5g"
#define SSID_MODE_WDS_BRIDGE_5G_2 "wds_bridge_5g_2"
#define SSID_MODE_STA_AP_24G "sta_ap_24g"
#define SSID_MODE_STA_AP_5G "sta_ap_5g"
#define SSID_MODE_STA_AP_5G_2 "sta_ap_5g_2"

/*-----------------------------------------------------------------------*/
/*                            FUNCTION                              */
/*-----------------------------------------------------------------------*/
//=================[Function from sysutil Start]==================
#define SYSTEM(format,args...) \
{ \
	const char *F[] = {format}; \
	char buf_for_SYSTEM[1024]; \
	sprintf(buf_for_SYSTEM, *F, ##args); \
	system(buf_for_SYSTEM); \
}
int sysinteract(char *output, int outputlen, char *fmt, ...)
{
	char cmd_for_sysinteract[256]={0};
	FILE *pipe;
	int c;
	//char cmd[256];
	int i;
	va_list ap;

	memset(cmd_for_sysinteract, 0, sizeof(cmd_for_sysinteract));
	va_start(ap, fmt);
	vsnprintf(cmd_for_sysinteract, sizeof(cmd_for_sysinteract), fmt, ap);
	va_end(ap);

	memset(output, 0, outputlen);
	if((pipe = popen(cmd_for_sysinteract, "r")) == NULL)
	{
		goto err;
	}

	for(i = 0; ((c = fgetc(pipe)) != EOF) && (i < outputlen - 1); i++)
	{
		output[i] = (char) c;
	}
	output[i] = '\0';

	pclose(pipe);

	if(strlen(output) == 0)
	{
		goto err;
	}

	return 0;

err:
	strcpy(output, "---");
	return -1;
}
char sysIsFileExisted(const char *filename)
{
	int rval;

    // Check file existence.
	rval = access(filename, 0);

	return rval ? 0 : 1;
}
//=================[Function from sysutil END]==================

void split(char **arr, char *str, const char *del) {
    char *s = strtok(str, del);
    while(s != NULL) {
        *arr++ = s;
        s = strtok(NULL, del);
    }
}

/*****************************************************************
* NAME:    get_wifi_radio_24g
* ---------------------------------------------------------------
* FUNCTION: parsing /wifi/radio/24g
* INPUT:    HTTPEntry packet
* OUTPUT:   
******************************************************************/
int get_api_version(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jobj;
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    jobj = newObjectFromStack(rep);
    json_get_version(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_sys_login(HTTPEntry packet, ResponseEntry *rep)
{
    char username[64]={0}, passwd[64]={0}, server_account[64]={0}, server_password[64]={0}, role[32]={0}, passwd_md5[64]={0}, buf[16]={0};
    int column=1, rval, max_column;
    struct json_object *jobj = NULL;
    struct json_object *jobj2 = NULL;
    ResponseStatus *res = rep->res;

    sysinteract(buf, sizeof(buf), "cat /etc/senao-openapi-server/senao-openapi-server.dav |wc -l");
    max_column = atoi(buf);
    if((jobj2 = json_tokener_parse(packet.body)))
    {
        senao_json_object_get_string(jobj2, "username", username);
        senao_json_object_get_string(jobj2, "password", passwd);
        json_object_put(jobj2);
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }

    for(column=1; column<=max_column; column++)
    {
        if (sysinteract(server_account, sizeof(server_account), "cat /etc/senao-openapi-server/senao-openapi-server.dav | awk -F':' 'NR==\"%d\"{ print $1 }'", column) < 0)
        {
            debug_print("Jason DEBUG %s[%d], Get server_account fail.\n", __FUNCTION__, __LINE__);
            genErrorMessage(rep->res, API_INVALID_ACCOUNT, "Error");
            return 0;
        }
        else
        {
            server_account[strlen(server_account)-1]='\0';
            if (strcmp(server_account, username) == 0)
            {
                if(sysinteract(server_password, sizeof(server_password), "cat /etc/senao-openapi-server/senao-openapi-server.dav | awk -F':' 'NR==\"%d\"{ print $2 }'", column)<0)
                {
                    debug_print("Jason DEBUG %s[%d], Get server_password fail.\n", __FUNCTION__, __LINE__);
                }
                else
                {
                    server_password[strlen(server_password)-1]='\0';
                    FILE *fptr;
                    fptr = fopen("/tmp/pwd.txt","w");
                    fprintf(fptr,"%s\n",passwd);
                    if(fptr) fclose(fptr);

                    if(sysinteract(passwd_md5, sizeof(passwd_md5), "md5sum /tmp/pwd.txt |  awk '{print $1}'")<0)
                    {
                        debug_print("Jason DEBUG %s[%d], Get passwd_md5 fail.\n", __FUNCTION__, __LINE__);
                    }
                    else
                    {
                        passwd_md5[strlen(passwd_md5)-1]='\0';
                        if (strcmp(server_password, passwd_md5) == 0)
                        {
                            if(sysinteract(role, sizeof(role), "cat /etc/senao-openapi-server/senao-openapi-server.dav | awk -F':' 'NR==\"%d\"{ print $3 }'", column)<0)
                            {
                                debug_print("Jason DEBUG %s[%d], Get role fail.\n", __FUNCTION__, __LINE__);
                            }
                            else
                            {
                                role[strlen(role)-1]='\0';
                                char *out = NULL;
                                encode_hs256(role, &out);
                                debug_print("cl %s:%d out[%s] ###\n", __FUNCTION__, __LINE__, out);
                                jobj = newObjectFromStack(rep);
                                json_set_login(rep, jobj, out);
                                free(out);
                                rep->jobj = jobj;
                                debug_print("Jason DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, out);
                                return 0;
                            }
                        }
                        else
                        {
                            genErrorMessage(rep->res, API_INVALID_ACCOUNT, "Error");
                            return 0;
                        }
                    }
                }
            }
        }
        memset(server_account, 0, sizeof(server_account));
    }
    genErrorMessage(rep->res, API_INVALID_ACCOUNT, "Error");
    return 0;
}

int get_account_username(HTTPEntry packet, ResponseEntry *rep)
{
    char username[32]={0};
    struct json_object *jobj;

    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    jobj = newObjectFromStack(rep);

    sysinteract(username, sizeof(username), "cat /etc/senao-openapi-server/senao-openapi-server.dav | awk -F':' 'NR==\"1\"{ print $1 }'");
    username[strlen(username)-1]='\0';
    json_object_object_add(jobj, "username", json_object_new_string(username));
    rep->jobj = jobj;

    return 0;
}

int post_change_account(HTTPEntry packet, ResponseEntry *rep)
{
    char *username=NULL, *passwd=NULL, tmp[10];
    struct json_object *jobj=NULL;
    ResponseStatus *res = rep->res;
#if SUPPORT_LXC_EZMASTER_ACCOUNT_SETTING
    char lxc_name[32];
    char running_status[32];
#endif

    if((jobj = jsonTokenerParseFromStack(rep, packet.body)))
    {
        senao_json_object_get_and_create_string(rep, jobj, "username", &username);
        senao_json_object_get_and_create_string(rep, jobj, "password", &passwd);
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }

    debug_print("Jason DEBUG username[%s] passwd[%s] \n", username, passwd);

    if ( strlen(username) < 0 || strlen(username) > 12 )
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "USERNAME LENGTH");
    }

    if (( strchr(username, '#') != NULL ) || ( strchr(username, '\'') != NULL ) ||
        ( strchr(username, '"') != NULL ) || ( strchr(username, '\\') != NULL ) ||
        ( strchr(username, '/') != NULL ) || ( strchr(username, ':') != NULL ) ||
        ( strchr(username, '&') != NULL ) || ( strchr(username, '[') != NULL ) ||
        ( strchr(username, ' ') != NULL ) || ( strchr(username, ']') != NULL ) ||
        ( strchr(username, '`') != NULL ))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "USERNAME CAN'T INCLUDE SPECIAL CHARACTERS");
    }

    if ( !strcmp(username, "root") )
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "USERNAME CAN'T USE ROOT");
    }

#if SUPPORT_COMPLEX_PASSWORD
    if (( strchr(passwd, '`') != NULL ) || ( strchr(passwd, '~') != NULL ) || ( strchr(passwd, '-') != NULL ) ||
        ( strchr(passwd, '_') != NULL ) || ( strchr(passwd, '=') != NULL ) || ( strchr(passwd, '+') != NULL ) ||
        ( strchr(passwd, '[') != NULL ) || ( strchr(passwd, ']') != NULL ) || ( strchr(passwd, '{') != NULL ) ||
        ( strchr(passwd, '}') != NULL ) || ( strchr(passwd, '\\') != NULL ) || ( strchr(passwd, '|') != NULL ) ||
        ( strchr(passwd, ';') != NULL ) || ( strchr(passwd, ':') != NULL ) || ( strchr(passwd, '\'') != NULL ) ||
        ( strchr(passwd, '"') != NULL ) || ( strchr(passwd, ',') != NULL ) || ( strchr(passwd, '<') != NULL ) ||
        ( strchr(passwd, '.') != NULL ) || ( strchr(passwd, '>') != NULL ) || ( strchr(passwd, '/') != NULL ) ||
        ( strchr(passwd, '?') != NULL ) || ( strchr(passwd, ' ') != NULL ))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "USERNAME CAN'T INCLUDE SPECIAL CHARACTERS");
    }
    if (!api_check_string_pattern4(passwd))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Password must contain at least one uppercase letter, one lowercase letter, and one number. Allowed symbols are !@#$%^&()*");
    }
#else
    if (!api_check_string_pattern3(passwd))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSWORD CAN'T INCLUDE SPECIAL CHARACTERS");
    }
#endif

#if SUPPORT_PASSWORD_LENGTH32
    if ( strlen(passwd) < 0 || strlen(passwd) > 32 )
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSWORD LENGTH");
    }
#else
    if ( strlen(passwd) < 0 || strlen(passwd) > 12 )
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSWORD LENGTH");
    }
#endif

    SYSTEM("/lib/auth.sh set_acct '%s' '%s' ; (echo '%s'; echo '%s') | passwd '%s'",
            username, passwd, passwd, passwd, username);

#if SUPPORT_NETGEAR_FUNCTION
    api_set_integer_option("system.firmware.first_login", 0);
#endif

#if SUPPORT_LXC_EZMASTER_ACCOUNT_SETTING
    memset(lxc_name, 0x00, sizeof(lxc_name));
    memset(running_status, 0x00, sizeof(running_status));

    api_get_string_option("lxc.lqsdk.name", lxc_name, sizeof(lxc_name));

    if(0 != strlen(lxc_name))
    {
        sysinteract(running_status, sizeof(running_status),
                "/usr/bin/lxc-info -n %s | grep RUNNING 2>/dev/null", lxc_name);

        if(0 != strlen(running_status))
        {
            SYSTEM("lxc-attach -n %s -- sh -c 'cd /usr/share/ezmaster/snweb; echo \"from django.contrib.auth.models import User; u = User.objects.get(username=\\\"admin\\\"); u.set_password(\\\"%s\\\"); u.save()\" | python3 manage.py shell' &", lxc_name, passwd);
fflush(stderr);
        }
    }
#endif

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int get_wifi_radio_24g(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jobj;
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    jobj = newObjectFromStack(rep);
    json_get_wireless_radio(rep, jobj, WIFI_RADIO_NAME_24G);
    rep->jobj = jobj;
    //openapi_response(rep);
    //json_object_put(jobj);
    return 0;
}
int post_wifi_radio_24g(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_wireless_radio(rep, packet.body, WIFI_RADIO_NAME_24G);
    //openapi_response(rep);
    return 0;
}
int patch_wifi_radio_24g(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_patch_wireless_radio(rep, packet.body, WIFI_RADIO_NAME_24G);
    //openapi_response(rep);
    return 0;
}
int get_wifi_radio_5g(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jobj;
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    jobj = newObjectFromStack(rep);
    json_get_wireless_radio(rep, jobj, WIFI_RADIO_NAME_5G);
    rep->jobj = jobj;
    //openapi_response(rep);
    //json_object_put(jobj);
    return 0;
}

int post_wifi_radio_5g(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_wireless_radio(rep, packet.body, WIFI_RADIO_NAME_5G);
    //openapi_response(rep);
    return 0;
}
int patch_wifi_radio_5g(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_patch_wireless_radio(rep, packet.body, WIFI_RADIO_NAME_5G);
    //openapi_response(rep);
    return 0;
}
int get_wifi_radio_5g_2(HTTPEntry packet, ResponseEntry *rep)
{

    struct json_object *jobj;
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    jobj = newObjectFromStack(rep);
    json_get_wireless_radio(rep, jobj, WIFI_RADIO_NAME_5G_2);
    rep->jobj = jobj;

    return 0;
}

int post_wifi_radio_5g_2(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_wireless_radio(rep, packet.body, WIFI_RADIO_NAME_5G_2);

    //openapi_response(rep);
    return 0;
}
int patch_wifi_radio_5g_2(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_patch_wireless_radio(rep, packet.body, WIFI_RADIO_NAME_5G_2);
    //openapi_response(rep);
    return 0;
}
int get_wifi_mesh(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_mesh(rep, jobj);
    rep->jobj = jobj;
    //openapi_response(rep);
    //json_object_put(jobj);
    return 0;
}

int post_wifi_mesh(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_mesh(rep, packet.body);
    //openapi_response(rep);
    return 0;
}

int get_wifi_mesh_mesh_info(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_mesh_info(rep, jobj);
    rep->jobj = jobj;
    //openapi_response(rep);
    //json_object_put(jobj);
    return 0;
}
int get_wifi_mesh_group_info(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jarr;
    jarr = newObjectArrayFromStack(rep);
    json_get_mesh_group_info(rep, jarr);
    rep->jobj = jarr;
    //openapi_response(rep);
    //json_object_put(jobj);
    return 0;
}

json_object * create_ssid_json_object(ssid_cfg_st *ssidCfg_p, ResponseEntry *rep)
{
    struct json_object *jobj, *jobj_client_dns_server, *jobj_band_steering, *jobj_traffic_shaping, *jobj_wireless_security, *jobj_wireless_encryption, *jobj_wireless_radius_server,*jobj_wireless_accounting_server, *jobj_captive_portal, *jobj_guest_network, *jobj_scheduling, *jobj_scheduling_days_all, *jobj_l2_acl, *jobj_debug;
    int i=0;
    struct json_object *jobj_scheduling_days[scheduling_days_length];
    char support_bandsteering[128]={0}, support_captive_portal[16]={0};

    jobj = newObjectFromStack(rep);
    jobj_client_dns_server = newObjectFromStack(rep);
    jobj_band_steering = newObjectFromStack(rep);
    jobj_traffic_shaping = newObjectFromStack(rep);
    jobj_wireless_security = newObjectFromStack(rep);
    jobj_wireless_encryption = newObjectFromStack(rep);
    jobj_wireless_radius_server = newObjectFromStack(rep);
    jobj_wireless_accounting_server = newObjectFromStack(rep);
    jobj_captive_portal = newObjectFromStack(rep);
    jobj_guest_network = newObjectFromStack(rep);

    //Display bandsteering info when SUPPORT_BANDSTEER
    sys_interact(support_bandsteering, sizeof(support_bandsteering), "uci get functionlist.functionlist.SUPPORT_BANDSTEER_SELECT_LIST 2>/dev/null");
    sys_interact(support_captive_portal, sizeof(support_captive_portal), "uci get apcontroller.capwap.enable 2>/dev/null");

    for (i=0;i<scheduling_days_length;i++)
    {
        jobj_scheduling_days[i] = newObjectFromStack(rep);
    }

    jobj_scheduling = newObjectFromStack(rep);
    jobj_scheduling_days_all = newObjectFromStack(rep);
    jobj_l2_acl = newObjectFromStack(rep);

    json_get_wireless_ssid(jobj, ssidCfg_p);

    // 2.4G              RADIO_MODE=2
    // 5G                RADIO_MODE=5
    // 2.4G + 5G         RADIO_MODE=2+5
    // 2.4G + 5G + 5G-2  RADIO_MODE=2+5+5
    // Don't show band steering group for single band project.
    if ( ssidCfg_p->opmode == OPM_AP && RADIO_MODE > 5)
    {
        if(atoi(support_bandsteering) == 1)
        {
            json_get_wireless_band_steering(jobj_band_steering, ssidCfg_p);
        }
        json_get_wireless_traffic_shaping(jobj_traffic_shaping, ssidCfg_p);
    }
    if ( ssidCfg_p->opmode == OPM_AP || ssidCfg_p->opmode == OPM_WDSAP ) 
    {
        json_get_wireless_traffic_shaping(jobj_traffic_shaping, ssidCfg_p);
    }

#if !SUPPORT_SWOS_FUNCTION
    json_get_client_dns_server(jobj_client_dns_server, ssidCfg_p);
#endif
    json_get_wireless_security(jobj_wireless_security, ssidCfg_p);
    json_get_wireless_encryption(jobj_wireless_encryption, ssidCfg_p);
    json_get_wireless_radius_server(jobj_wireless_radius_server, ssidCfg_p);

    json_get_wireless_accounting_server(jobj_wireless_accounting_server, ssidCfg_p);

    if ( ssidCfg_p->opmode == OPM_AP )
    {
        if(atoi(support_captive_portal) == 1)
        {
            json_get_wireless_captive_portal(jobj_captive_portal, ssidCfg_p);
        }
        else
        {
            json_get_wireless_guest_network(jobj_guest_network, ssidCfg_p);
        }
    }

    for (i=0;i<scheduling_days_length;i++)
    {
        json_get_wireless_scheduling_days(jobj_scheduling_days[i], ssidCfg_p, i);
    }
    json_get_wireless_scheduling(jobj_scheduling, ssidCfg_p);
    json_get_wireless_l2_acl(jobj_l2_acl, ssidCfg_p);

#if !SUPPORT_SWOS_FUNCTION
    json_object_object_add(jobj, "client_dns_server", jobj_client_dns_server);
#endif

    if ( ssidCfg_p->opmode == OPM_AP && RADIO_MODE > 5)
    {
        if(atoi(support_bandsteering) == 1)
        {
            json_object_object_add(jobj, "band_steering", jobj_band_steering);
        }
    }
    if ( ssidCfg_p->opmode == OPM_AP || ssidCfg_p->opmode == OPM_WDSAP ) 
    {
        json_object_object_add(jobj, "traffic_shaping", jobj_traffic_shaping);
    }
    json_object_object_add(jobj_wireless_security, "wpa", jobj_wireless_encryption);
    json_object_object_add(jobj_wireless_security, "radius_server", jobj_wireless_radius_server);
    json_object_object_add(jobj_wireless_security, "accounting_server", jobj_wireless_accounting_server);
    json_object_object_add(jobj, "wireless_security", jobj_wireless_security);

    if ( ssidCfg_p->opmode == OPM_AP )
    {
        if(atoi(support_captive_portal) == 1)
        {
            json_object_object_add(jobj, "captive_portal", jobj_captive_portal);
        }
        else
        {
            json_object_object_add(jobj, "guest_network", jobj_guest_network);
        }
    }

    for (i=0;i<scheduling_days_length;i++)
    {
        json_object_object_add(jobj_scheduling_days_all, scheduling_days[i], jobj_scheduling_days[i]);
    }
    json_object_object_add(jobj_scheduling, "days", jobj_scheduling_days_all);
    json_object_object_add(jobj, "scheduling", jobj_scheduling);
    json_object_object_add(jobj, "l2_acl", jobj_l2_acl);

    return jobj;
}

int post_wifi_mgmt(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    json_set_wifi_mgmt(rep, packet.body);
    //openapi_response(rep);
    return 0;
}

int patch_wifi_mgmt(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    json_patch_wifi_mgmt(rep, packet.body);
    //openapi_response(rep);
    return 0;
}

int get_wifi_mgmt(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_wifi_mgmt(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_wifi_sta_mode(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    get_cfg_name(packet, rep);
    //json_set_wifi_sta_mode(rep, packet.body, WIFI_RADIO_NAME_24G);
    json_set_wifi_sta_mode(rep, packet.body, &ssidCfg);   
    //openapi_response(rep);
    return 0;
}

int patch_wifi_sta_mode(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    get_cfg_name(packet, rep);
    //json_set_wifi_sta_mode(rep, packet.body, WIFI_RADIO_NAME_24G);
    json_patch_wifi_sta_mode(rep, packet.body, &ssidCfg);   
    //openapi_response(rep);
    return 0;
}


int get_wifi_sta_mode(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;

    get_cfg_name(packet, rep);
    jobj = newObjectFromStack(rep);
    json_get_wifi_sta_mode(rep, jobj, &ssidCfg);
    rep->jobj = jobj;


    return 0;
}

int post_wifi_wds_link(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    get_cfg_name(packet, rep);
    //json_set_wifi_sta_mode(rep, packet.body, WIFI_RADIO_NAME_24G);
    json_set_wifi_wds_link(rep, packet.body, &ssidCfg);   
    //openapi_response(rep);
    return 0;
}

int get_wifi_wds_link(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;

    get_cfg_name(packet, rep);
    jobj = newObjectFromStack(rep);
    json_get_wifi_wds_link(rep, jobj, &ssidCfg);
    rep->jobj = jobj;


    return 0;
}
int get_wifi_ssids(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jobj, *jobjArray[8];
    int i, index;
    char index_str[3] = {0};

    jobj = newObjectFromStack(rep);
    memset(packet.api_uri_array[1], 0, sizeof(packet.api_uri_array[1]));
    strcpy(packet.api_uri_array[1], SSID_MODE_AP);
    for(i=0; i < AP_WIFI_IFACE_NUM;i++)
    {
        memset(packet.api_uri_array[2], 0, sizeof(packet.api_uri_array[2]));
        index = i+1;
        sprintf(packet.api_uri_array[2], "%d",index);
        sprintf(index_str, "%d",index);
        get_cfg_name(packet, rep);
        jobjArray[i] = create_ssid_json_object(&ssidCfg, rep);
        json_object_object_add(jobj, index_str, jobjArray[i]);
    }
    rep->jobj = jobj;

    return 0;
}


int get_cfg_name(HTTPEntry packet, ResponseEntry *rep)
{
    memset(ssid_mode, 0, sizeof(ssid_mode));
    strncpy(ssid_mode, packet.api_uri_array[1], sizeof(ssid_mode));
    ssid_idx = atoi(packet.api_uri_array[2]);
    ssidCfg.idx = ssid_idx;

    int radio=0;
    debug_print("11111111111111areal111111111111Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);

    // 2.4G              RADIO_MODE=2
    // 5G                RADIO_MODE=5
    // 2.4G + 5G         RADIO_MODE=2+5
    // 2.4G + 5G + 5G-2  RADIO_MODE=2+5+5
    switch(RADIO_MODE)
    {
        case 2:
            radio = 0;
            break;
        case 5:
            radio = 1;
            break;
        default :
            radio = 0;
            break;
    }

    if(strcmp(ssid_mode,SSID_MODE_AP) == 0)
    {
        api_get_wifix_section_name(OPM_AP, radio, ssidCfg.section);
        ssidCfg.opmode = OPM_AP;
    }
    else if(strcmp(ssid_mode, SSID_MODE_WDS_AP_24G) == 0)
    {
        api_get_wifix_section_name(OPM_WDSAP, 0, ssidCfg.section);
        ssidCfg.opmode = OPM_WDSAP;
    }
    else if(strcmp(ssid_mode, SSID_MODE_WDS_AP_5G) == 0)
    {
        api_get_wifix_section_name(OPM_WDSAP, 1, ssidCfg.section);
        ssidCfg.opmode = OPM_WDSAP;
    }
    else if(strcmp(ssid_mode, SSID_MODE_WDS_AP_5G_2) == 0)
    {
        api_get_wifix_section_name(OPM_WDSAP, 2, ssidCfg.section);
        ssidCfg.opmode = OPM_WDSAP;
    }
    else if(strcmp(ssid_mode, SSID_MODE_ENJET) == 0)
    {
        ssidCfg.opmode = -1;
        ssidCfg.idx = 0;
    debug_print("11111111111111areal111111111111Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
        switch (radio)
        {
            case 0:
                strcpy(ssidCfg.section,"wireless.wifi0_enjet");
    debug_print("11111111111111areal111111111111Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
                break;
            case 1:
                strcpy(ssidCfg.section,"wireless.wifi1_enjet");
    debug_print("11111111111111areal111111111111Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
                break;
        }
    }
    else if(strcmp(ssid_mode, SSID_MODE_STA_24G) == 0)
    {
        ssidCfg.opmode = OPM_CB;
        strcpy(ssidCfg.section,"wireless.wifi0_sta");
    }
    else if(strcmp(ssid_mode, SSID_MODE_STA_5G) == 0)
    {
        ssidCfg.opmode = OPM_CB;
        strcpy(ssidCfg.section,"wireless.wifi1_sta");
    }
    else if(strcmp(ssid_mode, SSID_MODE_WDS_STA_24G) == 0)
    {
        ssidCfg.opmode = OPM_WDSSTA;
        strcpy(ssidCfg.section,"wireless.wifi0_wds_sta");
    }
    else if(strcmp(ssid_mode, SSID_MODE_WDS_STA_5G) == 0)
    {
        ssidCfg.opmode = OPM_WDSSTA;
        strcpy(ssidCfg.section,"wireless.wifi1_wds_sta");
    }
    else if(strcmp(ssid_mode, SSID_MODE_WDS_BRIDGE_24G) == 0)
    {
        ssidCfg.opmode = OPM_WDSB;
        strcpy(ssidCfg.section,"wireless.wifi0_wds");
    }
    else if(strcmp(ssid_mode, SSID_MODE_WDS_BRIDGE_5G) == 0)
    {
        ssidCfg.opmode = OPM_WDSB;
        strcpy(ssidCfg.section,"wireless.wifi1_wds");
    }
    else if(strcmp(ssid_mode, SSID_MODE_WDS_BRIDGE_5G_2) == 0)
    {
        ssidCfg.opmode = OPM_WDSB;
        strcpy(ssidCfg.section,"wireless.wifi2_wds");
    }
    else if(strcmp(ssid_mode, SSID_MODE_STA_AP_24G) == 0)
    {
        ssidCfg.opmode = OPM_RP;
        strcpy(ssidCfg.section,"wireless.wifi0_sta_ap");
    }
    else if(strcmp(ssid_mode, SSID_MODE_STA_AP_5G) == 0)
    {
        ssidCfg.opmode = OPM_RP;
        strcpy(ssidCfg.section,"wireless.wifi1_sta_ap");
    }
    else if(strcmp(ssid_mode, SSID_MODE_STA_AP_5G_2) == 0)
    {
        ssidCfg.opmode = OPM_RP;
        strcpy(ssidCfg.section,"wireless.wifi2_sta_ap");
    }
    else
    {
        ssidCfg.opmode = -1;
        debug_print("11111111111111areal111111111111Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    }


    return 0;
}


int get_wifi_ssid(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    int i=0;

    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }

    jobj = create_ssid_json_object(&ssidCfg, rep);
    rep->jobj = jobj;


    return 0;
}

int post_wifi_ssid(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_wireless_ssid(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}

int patch_wifi_ssid(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_patch_wireless_ssid(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}

int get_wifi_ssid_security(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj, *jobj_band_steering, *jobj_traffic_shaping, *jobj_wireless_security, *jobj_wireless_encryption, *jobj_wireless_radius_server,*jobj_wireless_accounting_server, *jobj_captive_portal, *jobj_scheduling, *jobj_l2_acl, *jobj_debug;
//    int i=0;
//
//    ssid_idx = atoi(packet.api_uri_array[2]);
//debug_print("Jason DEBUG %s[%d] ssid_idx=[%d]\n", __FUNCTION__, __LINE__, ssid_idx); 
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }

debug_print("Jason DEBUG %s[%d] \n", __FUNCTION__, __LINE__); 
    jobj_wireless_security = (struct josn_object *) newObjectFromStack(rep);
debug_print("Jason DEBUG %s[%d] \n", __FUNCTION__, __LINE__); 
    jobj_wireless_encryption = (struct josn_object *) newObjectFromStack(rep);
debug_print("Jason DEBUG %s[%d] \n", __FUNCTION__, __LINE__); 
    jobj_wireless_radius_server = (struct josn_object *) newObjectFromStack(rep);
debug_print("Jason DEBUG %s[%d] \n", __FUNCTION__, __LINE__); 
    jobj_wireless_accounting_server = (struct josn_object *) newObjectFromStack(rep);
debug_print("Jason DEBUG %s[%d] \n", __FUNCTION__, __LINE__); 

    json_get_wireless_security(jobj_wireless_security, &ssidCfg);
    json_get_wireless_encryption(jobj_wireless_encryption, &ssidCfg);
    json_get_wireless_radius_server(jobj_wireless_radius_server, &ssidCfg);
    json_get_wireless_accounting_server(jobj_wireless_accounting_server, &ssidCfg);
    //json_get_wireless_captive_portal(jobj_captive_portal, &ssidCfg);

    json_object_object_add(jobj_wireless_security, "wpa", jobj_wireless_encryption);
    json_object_object_add(jobj_wireless_security, "radius_server", jobj_wireless_radius_server);
    json_object_object_add(jobj_wireless_security, "accounting_server", jobj_wireless_accounting_server);
    //genErrorMessage(rep->res, 0,"get_wifi_ssid_security OK");
    rep->jobj = jobj_wireless_security;
    //openapi_response(rep);
    //json_object_put(jobj_wireless_accounting_server);
    //json_object_put(jobj_wireless_radius_server);
    //json_object_put(jobj_wireless_encryption);
    //json_object_put(jobj_wireless_security);
    //json_object_put(jobj_debug);
    return 0;
}

int post_wifi_ssid_security(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_wireless_security(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}

int patch_wifi_ssid_security(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_patch_wireless_security(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}

int get_wifi_ssid_guestnetwork(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jarr;

    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  || ssid_idx >7)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    jarr = newObjectFromStack(rep);

    json_get_wireless_guest_network(jarr, &ssidCfg);

    rep->jobj = jarr;
    //openapi_response(rep);
    //json_object_put(jarr);

    return 0;
}

int post_wifi_ssid_guestnetwork(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  || ssid_idx >7)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }

	json_set_wireless_guest_network(rep, packet.body, &ssidCfg);

    //openapi_response(rep);

    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    return 0;
}

int patch_wifi_ssid_guestnetwork(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    get_cfg_name(packet, rep);
    json_patch_wireless_guest_network(rep, packet.body, &ssidCfg);

    //openapi_response(rep);

    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    return 0;
}

int get_wifi_ssid_captive_portal(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jarr;

    get_cfg_name(packet, rep);
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    jarr = newObjectFromStack(rep);
    json_get_wireless_captive_portal(jarr, &ssidCfg);
    rep->jobj = jarr;
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    return 0;
}

int post_wifi_ssid_captive_portal(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);
    json_set_wireless_captive_portal(rep, packet.body, &ssidCfg, NULL);
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    return 0;
}

int patch_wifi_ssid_captive_portal(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);
    json_patch_wireless_captive_portal(rep, packet.body, &ssidCfg, NULL);
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    return 0;
}

int get_wifi_ssid_hs20(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jobj;
    int error_code = 0;
    char string[2048] = {0}, msg[10] = {0};
    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  || ssid_idx >7)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    // jobj = newObjectFromStack(rep);
    json_get_hotspot20(&ssidCfg, string, sizeof(string), error_code, msg);

    jobj = json_tokener_parse(string);

    rep->jobj = jobj;
    //openapi_response(rep);
    //json_object_put(jarr);

    return 0;
}

int post_wifi_ssid_hs20(HTTPEntry packet, ResponseEntry *rep)
{
    int error_code = 0;
    char string[128] = {0}, msg[10] = {0};
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  || ssid_idx >7)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }

    json_set_hotspot20(&ssidCfg, packet.body, error_code, msg);

    //openapi_response(rep);
    RET_GEN_ERRORMSG(rep->res, API_SUCCESS, NULL);
    return 0;
}

int get_wifi_ssid_accountingserver(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jobj;

    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//debug_print("Jason DEBUG %s[%d] packet.api_uri_array[2]=[%s]\n", __FUNCTION__, __LINE__, packet.api_uri_array[2]); 
//debug_print("Jason DEBUG %s[%d] ssid_idx=[%d]\n", __FUNCTION__, __LINE__, ssid_idx); 
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }

    jobj = newObjectFromStack(rep);

    json_get_wireless_accounting_server(jobj, &ssidCfg);

    rep->jobj = jobj;
    //openapi_response(rep);

    //json_object_put(jobj);

    return 0;
}

int post_wifi_ssid_accountingserver(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_wireless_accounting_server(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}

int patch_wifi_ssid_accountingserver(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);

    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_patch_wireless_accounting_server(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}

int get_wifi_ssid_traffic_info(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    get_cfg_name(packet, rep);
    jobj = newObjectFromStack(rep);
    json_get_wireless_traffic_info(jobj, &ssidCfg);
    rep->jobj = jobj;

    return 0;
}

int get_wifi_ssid_trafficshaping(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jobj;

    get_cfg_name(packet, rep);
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 

    jobj = newObjectFromStack(rep);

    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_get_wireless_traffic_shaping(jobj, &ssidCfg);

    rep->jobj = jobj;
    //openapi_response(rep);

    //json_object_put(jobj);

    return 0;
}

int post_wifi_ssid_trafficshaping(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    json_set_wireless_traffic_shaping(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}

int patch_wifi_ssid_trafficshaping(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    json_patch_wireless_traffic_shaping(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}

int get_wifi_ssid_bandsteering(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jobj;
    get_cfg_name(packet, rep);

//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }

    jobj = newObjectFromStack(rep);

    json_get_wireless_band_steering(jobj, &ssidCfg);

    rep->jobj = jobj;
    //openapi_response(rep);

    //json_object_put(jobj);

    return 0;
}

int post_wifi_ssid_bandsteering(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_wireless_band_steering(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}

int patch_wifi_ssid_bandsteering(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_patch_wireless_band_steering(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}

int get_wifi_ssid_radiusserver(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jobj;

    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }

    jobj = newObjectFromStack(rep);

    json_get_wireless_radius_server(jobj, &ssidCfg);

    rep->jobj = jobj;
    //openapi_response(rep);

    //json_object_put(jobj);

    return 0;
}


int post_wifi_ssid_radiusserver(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_wireless_radius_server(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}


int patch_wifi_ssid_radiusserver(HTTPEntry packet, ResponseEntry *rep)
{
    get_cfg_name(packet, rep);

    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_patch_wireless_radius_server(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}

int get_wifi_ssid_scheduling(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj_scheduling, *jobj_scheduling_days_all;
    int i=0;
    struct json_object *jobj_scheduling_days[scheduling_days_length];
    get_cfg_name(packet, rep);

//    ssid_idx = atoi(packet.api_uri_array[2]);
//
//    if (ssid_idx < 0  || ssid_idx >AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }

    for (i=0;i<scheduling_days_length;i++)
    {
        jobj_scheduling_days[i] = newObjectFromStack(rep);
    }

    jobj_scheduling = newObjectFromStack(rep);
    jobj_scheduling_days_all = newObjectFromStack(rep);

    for (i=0;i<scheduling_days_length;i++)
    {
        json_get_wireless_scheduling_days(jobj_scheduling_days[i], &ssidCfg, i);
    }
    json_get_wireless_scheduling(jobj_scheduling, &ssidCfg);

    for (i=0;i<scheduling_days_length;i++)
    {
        json_object_object_add(jobj_scheduling_days_all, scheduling_days[i], jobj_scheduling_days[i]);
    }
    json_object_object_add(jobj_scheduling, "days", jobj_scheduling_days_all);
    rep->jobj = jobj_scheduling;

    return 0;
}

int post_wifi_ssid_scheduling(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  || ssid_idx >AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    json_set_wireless_scheduling_sync(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}

int patch_wifi_ssid_scheduling(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    get_cfg_name(packet, rep);
    json_patch_wireless_scheduling_sync(rep, packet.body, &ssidCfg);
    //openapi_response(rep);
    return 0;
}


int post_wifi_ssid_kick(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    ssid_idx = atoi(packet.api_uri_array[2]);
    get_cfg_name(packet, rep);

    json_set_wireless_kick(rep, packet.body, &ssidCfg);
    return 0;

}

int post_wifi_ssid_l2_acl(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    ssid_idx = atoi(packet.api_uri_array[2]);
    get_cfg_name(packet, rep);
//    if (ssid_idx < 0  || ssid_idx >AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    json_set_wireless_l2_acl(rep, packet.body, &ssidCfg);
    return 0;
}

int patch_wifi_ssid_l2_acl(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    ssid_idx = atoi(packet.api_uri_array[2]);
    get_cfg_name(packet, rep);
    json_patch_wireless_l2_acl(rep, packet.body, &ssidCfg);
    return 0;
}

int get_wifi_ssid_l2_acl(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jobj;

    get_cfg_name(packet, rep);
//    ssid_idx = atoi(packet.api_uri_array[2]);
//    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
//    {
//        debug_print("[Senao API] Error!! ssid idx!\n");
//        return FAIL;
//    }
    jobj = newObjectFromStack(rep);
    json_get_wireless_l2_acl(jobj, &ssidCfg);
    rep->jobj = jobj;

    return 0;
}
int get_wifi_site_survey(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_wifi_site_survey(rep, jobj);
    rep->jobj = jobj;
    return 0;
}

int post_wifi_site_survey(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    //debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body);
    json_wifi_site_survey(rep, NULL);
    return 0;
}

int get_net_ethernet(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_ethernet(rep, jobj);
    rep->jobj = jobj;
    //openapi_response(rep);
    //json_object_put(jobj);
    return 0;
}

int get_net_ethernet_traffic_info(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_ethernet_traffic_info(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int get_net_ethernet_client_info(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_ethernet_client_info(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_net_ethernet(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_ethernet(rep, packet.body);
    //openapi_response(rep);
    return 0;
}

int patch_net_ethernet(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_patch_ethernet(rep, packet.body);
    //openapi_response(rep);
    return 0;
}

int post_linktest(HTTPEntry packet, ResponseEntry *rep)
{
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;
    debug_print("======= DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    jobj = newObjectFromStack(rep);
    json_post_linktest(rep, jobj);
    rep->jobj = jobj;
    //openapi_response(rep);
    return 0;
}

int get_linktest(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("======= DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    int resultPath;
    jobj = newObjectFromStack(rep);
    resultPath = (strcmp(packet.api_uri_array[2], "")) ? atoi(packet.api_uri_array[2]) : -1;
    debug_print("%s[%d] get result in resultpath:[%d]\n", __FUNCTION__, __LINE__, resultPath);
    json_get_linktest(rep, jobj, resultPath);
    rep->jobj = jobj;
    //openapi_response(rep);
    //json_object_put(jobj);
    return 0;
}

int get_sys_sys_info(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_sys_info(rep, jobj);
    rep->jobj = jobj;
    //openapi_response(rep);
    //json_object_put(jobj);
    return 0;
}

int get_sys_controlled_info(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_sys_controlled_info(rep, jobj);
    rep->jobj = jobj;
    //openapi_response(rep);
    //json_object_put(jobj);
    return 0;
}

int post_sys_system_config(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_sys_info(rep, packet.body);
    //openapi_response(rep);
    return 0;
}

int patch_sys_system_config(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    json_patch_sys_info(rep, packet.body);
    //openapi_response(rep);
    return 0;
}

int get_sys_syslog(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_sys_syslog(rep, jobj);
    rep->jobj = jobj;
    //openapi_response(rep);
    return 0;
}

int post_sys_syslog(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    json_set_sys_syslog(rep, packet.body);
    //openapi_response(rep);
    return 0;
}

int patch_sys_syslog(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_patch_sys_syslog(rep, packet.body);
    //openapi_response(rep);
    return 0;
}

int post_sys_fw_upgrade(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_fw_upgrade(rep, packet.body);

    return 0;
}

int post_sys_apply(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_sys_apply(rep, packet.body);

    return 0;
}
int post_sys_reload(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_post_sys_reload(rep, jobj);
    rep->jobj = jobj;

    return 0;
}
int post_sys_revert(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    json_sys_revert(rep, packet.body);

    return 0;
}
int get_dev_capability(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_sys_dev_capability(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int get_first_login(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_sys_first_login(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_first_login(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    json_post_sys_first_login(rep, packet.body);

    return 0;
}

#if HAS_SENAO_PACKETEER
int get_benu_tunnel_addr(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_sys_benu_tunnel_addr(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_benu_tunnel_addr(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_sys_benu_tunnel_addr(rep, packet.body);

    return 0;
}
#endif

int get_wifi_client_info(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_client_info(rep, jobj);
    rep->jobj = jobj;

    return 0;
}
int get_ezmcloud_info(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_ezmcloud_info(rep, jobj);
    rep->jobj = jobj;

    return 0;
}
int get_net_proxy_settings(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_net_proxy_settings(rep, jobj);
    rep->jobj = jobj;

    return 0;
}
int patch_net_proxy_settings(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    json_patch_net_proxy_settings(rep, packet.body);
    //openapi_response(rep);
    return 0;
}
int post_net_proxy_settings(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    json_set_net_proxy_settings(rep, packet.body);
    //openapi_response(rep);
    return 0;
}
int get_net_proxy_http(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_net_proxy_http(rep, jobj);
    rep->jobj = jobj;

    return 0;
}
int post_net_proxy_http(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_net_proxy_http(rep, packet.body);
    //openapi_response(rep);
    return 0;
}
int get_net_proxy_https(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_net_proxy_https(rep, jobj);
    rep->jobj = jobj;

    return 0;
}
int post_net_proxy_https(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_net_proxy_https(rep, packet.body);
    //openapi_response(rep);
    return 0;
}
int get_net_iptable_rules(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_net_iptable_rules(rep, jobj);
    rep->jobj = jobj;

    return 0;
}
int post_net_iptable_rules(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_net_iptable_rules(rep, packet.body);
    //openapi_response(rep);
    return 0;
}
int patch_net_iptable_rules(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_patch_net_iptable_rules(rep, packet.body);
    //openapi_response(rep);
    return 0;
}
int get_net_spanning_tree(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_net_spanning_tree(rep, jobj);
    rep->jobj = jobj;

    return 0;
}
int post_net_spanning_tree(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_net_spanning_tree(rep, packet.body);
    //openapi_response(rep);
    return 0;
}
int patch_net_spanning_tree(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_patch_net_spanning_tree(rep, packet.body);
    //openapi_response(rep);
    return 0;
}
int get_net_internet_speedtest(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_net_internet_speedtest(rep, jobj);
    rep->jobj = jobj;
    return 0;
}
int post_net_internet_speedtest(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    json_set_net_internet_speedtest(rep, packet.body);
    return 0;
}
int get_wifi_mesh_throughput(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    char *del = "%3A";
    char *result[16];
    char body[1024];
    char buf[64];

    memset(buf, 0, sizeof(buf));
    memset(result, 0, sizeof(result));
    memset(body, 0, sizeof(body));

    if (strlen(queryTable) != 0)
    {
        sscanf(queryTable, "%*[^:]%s", buf);
        sscanf(queryTable, "target_mac_addr=%s", queryTable);

        if (strlen(buf) != 0)
        {
            sprintf(body, "%s%s%s","{\"Source\":\"", queryTable, "\"}");
        }
        else
        {
            split(result, queryTable, del);
            sprintf(body, "%s%s:%s:%s:%s:%s:%s%s","{\"Source\":\"", result[0], result[1], result[2], result[3], result[4], result[5], "\"}");
        }
    }

    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_throughput_result(rep, body, jobj);
    rep->jobj = jobj;
    return 0;
}
int post_wifi_mesh_throughput(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    json_set_throughput(rep, packet.body);
    return 0;
}
int post_mgm_fwupgrade(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_mgm_fw_upgrade(rep, packet.body);
    return 0;
}

int post_mgm_fwupgrade_only(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_mgm_fw_upgrade_only(rep, packet.body);
    return 0;
}

int post_mgm_drop_caches(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body);
    json_mgm_drop_caches(rep, packet.body);
    return 0;
}

int get_mgm_localupgradeimage(HTTPEntry packet, ResponseEntry *rep)
{
	debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
        struct json_object *jobj;
        jobj = newObjectFromStack(rep);
        json_mgm_localupgradeimage(rep, jobj);
        rep->jobj = jobj;

	return 0;
}

int post_mgm_fw_check(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    //debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body);
    json_post_mgm_fw_check(rep, packet.body);
    return 0;
}

int get_mgm_fw_check(HTTPEntry packet, ResponseEntry *rep)
{
	debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
        struct json_object *jobj;
        jobj = newObjectFromStack(rep);
        json_get_mgm_fw_check(rep, jobj);
        rep->jobj = jobj;

	return 0;
}

int post_mgm_fw_download(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    //debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body);
    json_post_mgm_fw_download(rep, packet.body);
    return 0;
}

int get_mgm_fw_download(HTTPEntry packet, ResponseEntry *rep)
{
	debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
        struct json_object *jobj;
        jobj = newObjectFromStack(rep);
        json_get_mgm_fw_download(rep, jobj);
        rep->jobj = jobj;

	return 0;
}

int get_mgm_restoreimage(HTTPEntry packet, ResponseEntry *rep)
{
	debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
        struct json_object *jobj;
        jobj = newObjectFromStack(rep);
        json_mgm_restoreimage(rep, jobj);
        rep->jobj = jobj;

	return 0;
}

int get_auth_check(HTTPEntry packet, ResponseEntry *rep)
{
	debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
        struct json_object *jobj;
        jobj = newObjectFromStack(rep);
        json_get_auth_check(rep, jobj);
        rep->jobj = jobj;

	return 0;
}

int post_mgm_tools_iperf(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_set_mgm_tools_iperf(rep, packet.body);

    return 0;
}

int get_mgm_tools_iperf(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_mgm_tools_iperf(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_mgm_backup_config(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_set_mgm_backup_config(rep, packet.body);

    return 0;
}

int get_mgm_backup_config(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_mgm_backup_config(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_mgm_restore_config(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_set_mgm_restore_config(rep, packet.body);

    return 0;
}

int post_mgm_reboot(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body);
    json_set_mgm_reboot(rep, packet.body);

    return 0;
}

int post_mgm_auto_reboot_cfg(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body);
    json_set_mgm_auto_reboot_cfg(rep, packet.body);

    return 0;
}

int get_mgm_auto_reboot_cfg(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_mgm_auto_reboot_cfg(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_mgm_reset_to_default(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body);
    json_set_mgm_reset_to_default(rep, packet.body);

    return 0;
}

int post_mgm_reset_with_key(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body);
    json_set_mgm_reset_with_key(rep, packet.body);

    return 0;
}

int get_mgm_led_list(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jarr;
    jarr = newObjectArrayFromStack(rep);
    json_get_mgm_led_list(rep, jarr);
    rep->jobj = jarr;

    return 0;
}

int get_mgm_led_status(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jarr;
    jarr = newObjectArrayFromStack(rep);
    json_get_mgm_led_status(rep, jarr);
    rep->jobj = jarr;

    return 0;
}

int post_mgm_led_cfg(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body);
    json_set_mgm_led_cfg(rep, packet.body);

    return 0;
}

int post_mgm_ping(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body);
    json_mgm_ping(rep, packet.body);
    return 0;
}

int get_mgm_ping(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_mgm_ping(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_mgm_traceroute(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_mgm_traceroute(rep, packet.body);

    return 0;
}

int get_mgm_traceroute(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_mgm_traceroute(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_mgm_nslookup(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_mgm_nslookup(rep, packet.body);

    return 0;
}

int get_mgm_nslookup(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_mgm_nslookup(rep, jobj);
    rep->jobj = jobj;

    return 0;
}
int post_mgm_device_discovery(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_set_mgm_device_discovery(rep, packet.body);

    return 0;
}

int get_mgm_device_discovery(HTTPEntry packet, ResponseEntry *rep)
{    
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jarr;
    jarr = newObjectArrayFromStack(rep);
    json_get_mgm_device_discovery(rep, jarr);
    rep->jobj = jarr;

    return 0;
}
int post_mgm_gps(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_set_mgm_gps(rep, packet.body);

    return 0;
}

int get_mgm_gps(HTTPEntry packet, ResponseEntry *rep)
{    
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_mgm_gps(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

#if SUPPORT_NETGEAR_FUNCTION
int get_mgm_mode(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_mgm_mode(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_mgm_mode(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_mgm_mode(rep, packet.body);

    return 0;
}
#endif

int get_ssh_setting(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_ssh_setting(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_ssh_setting(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_set_ssh_setting(rep, packet.body);

    return 0;
}

int post_wifi_channel_list(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body);
    json_post_wifi_channel_list(rep, packet.body);

    return 0;
}

int get_wifi_channel_list(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_wifi_channel_list(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_wifi_guest_network(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_set_wifi_guest_network(rep, packet.body);

    return 0;
}

int patch_wifi_guest_network(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_patch_wifi_guest_network(rep, packet.body);

    return 0;
}

int get_wifi_guest_network(HTTPEntry packet, ResponseEntry *rep)
{    
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_wifi_guest_network(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int get_lxc_ezm_backup_info(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_lxc_ezm_backup_info(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int get_lxc_ezm_backup_method(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_lxc_ezm_backup_method(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_lxc_ezm_backup_method(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body);
    json_post_lxc_ezm_backup_method(rep, packet.body);

    return 0;
}

int get_lxc_ezm_restore(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_lxc_ezm_restore(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int get_wifi_scan_chanutil(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_wifi_scan_chanutil(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int get_wifi_scan_aplist(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_wifi_scan_aplist(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int get_wifi_scan_stalist(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_wifi_scan_stalist(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int get_wifi_vpn_profile(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_wifi_vpn_profile(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

int post_wifi_vpn_profile(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_set_wifi_vpn_profile(rep, packet.body);

    return 0;
}

int patch_wifi_vpn_profile(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_patch_wifi_vpn_profile(rep, packet.body);

    return 0;
}

int patch_wifi_delete_all_vpn_profile(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    debug_print("DEBUG %s[%d], ======ss [%s]\n", __FUNCTION__, __LINE__, packet.body); 
    json_delete_all_vpn_profile(rep, packet.body);

    return 0;
}

int get_wifi_vpn_status(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("=======Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    struct json_object *jobj;
    jobj = newObjectFromStack(rep);
    json_get_wifi_vpn_status(rep, jobj);
    rep->jobj = jobj;

    return 0;
}

#if 0
/*-----------------------------------------------------------------------*/
/*                                 CHECK                                 */
/*-----------------------------------------------------------------------*/

int check_wifi_radio_24g(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_chk_wireless_radio(rep, packet.body, WIFI_RADIO_NAME_24G);

    return 0;
}

int check_wifi_radio_5g(HTTPEntry packet, ResponseEntry *rep)
{
    //curl -v -k -X POST "https://192.168.1.1:4430/api/wifi/radio/5g" -H "accept: */*" -H "Content-Type: application/json" -H "Authorization: Basic YWRtaW46YWRtaW4=" -d"{\"enable\":true,\"channel\":108,\"ht_mode\":\"HT20\",\"tx_power\":0,\"client_limit\":0,\"min_bitrate\":18,\"rssi_threshold\":-80,\"opmode\":\"AP\",\"dfs_backup_channel\":0}"

    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_chk_wireless_radio(rep, packet.body, WIFI_RADIO_NAME_5G);

    return 0;
}

int check_wifi_radio_triband_5g(HTTPEntry packet, ResponseEntry *rep)
{

    //curl -v -k -X POST "https://192.168.1.1:4430/api/wifi/radio/triband/5g/1" -H "accept: */*" -H "Content-Type: application/json" -H "Authorization: Basic YWRtaW46YWRtaW4=" -d"{\"enable\":true,\"channel\":108,\"ht_mode\":\"HT20\",\"tx_power\":0,\"client_limit\":0,\"min_bitrate\":18,\"rssi_threshold\":-80,\"opmode\":\"AP\",\"dfs_backup_channel\":0}"
    //curl -v -k -X POST "https://192.168.1.1:4430/api/wifi/radio/triband/5g/2" -H "accept: */*" -H "Content-Type: application/json" -H "Authorization: Basic YWRtaW46YWRtaW4=" -d"{\"enable\":true,\"channel\":48,\"ht_mode\":\"HT40\",\"tx_power\":0,\"client_limit\":100,\"min_bitrate\":24,\"rssi_threshold\":-90,\"opmode\":\"AP\",\"dfs_backup_channel\":0}"
    
    debug_print("Jason DEBUG %s[%d]  api_uri_array[4] id[%s]\n", __FUNCTION__, __LINE__, packet.api_uri_array[4]); 
    triband_5g_idx = atoi(packet.api_uri_array[4]);
    switch(triband_5g_idx)
    {
        case 1:
            json_chk_wireless_radio(rep, packet.body, WIFI_RADIO_NAME_5G);
            break;
        case 2:
            json_chk_wireless_radio(rep, packet.body, WIFI_RADIO_NAME_5G_2);
            break;
        default:
            debug_print("[Senao API] Error!! triband 5g idx should be 1 or 2!\n");
            genErrorMessage(rep->res, API_UNKNOWN_ACTION, NULL);
            break;
    }
    return 0;
}

int check_wifi_mesh(HTTPEntry packet, ResponseEntry *rep)
{
    //curl -v -k -X POST "https://192.168.1.1:4430/api/wifi/mesh" -H "accept: */*" -H "Content-Type: application/json" -H "Authorization: Basic YWRtaW46YWRtaW4=" -d"{\"enable\":true,\"wifi_radio\":\"2_4G\",\"mesh_id\":\"1234\",\"mesh_pw\":\"0987654321\",\"mesh_rssi\":-70}"

    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_chk_mesh(rep, packet.body);
    return 0;
}
int check_wifi_ssid(HTTPEntry packet, ResponseEntry *rep)
{
    ssid_idx = atoi(packet.api_uri_array[2]);
    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
    {
        debug_print("[Senao API] Error!! ssid idx!\n");
        return FAIL;
    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_chk_wireless_ssid(rep, packet.body, &ssidCfg);

    return 0;
}

int check_wifi_ssid_security(HTTPEntry packet, ResponseEntry *rep)
{
    ssid_idx = atoi(packet.api_uri_array[2]);
    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
    {
        debug_print("[Senao API] Error!! ssid idx!\n");
        return FAIL;
    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    senao_json_validator(rep, packet.body, "security");
//    json_chk_wireless_security(rep, packet.body, WIFI_RADIO_NAME_24G, ssid_idx);

    return 0;
}

int check_wifi_ssid_captiveportal(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    ssid_idx = atoi(packet.api_uri_array[2]);
    if (ssid_idx < 0  || ssid_idx >7)
    {
        debug_print("[Senao API] Error!! ssid idx!\n");
        return FAIL;
    }
    json_chk_wireless_captive_portal(rep, packet.body, WIFI_RADIO_NAME_24G, ssid_idx);

    return 0;
}

int check_wifi_ssid_accountingserver(HTTPEntry packet, ResponseEntry *rep)
{
    ssid_idx = atoi(packet.api_uri_array[2]);
    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
    {
        debug_print("[Senao API] Error!! ssid idx!\n");
        return FAIL;
    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_chk_wireless_accounting_server(rep, packet.body, WIFI_RADIO_NAME_24G, ssid_idx);

    return 0;
}

int check_wifi_ssid_trafficshaping(HTTPEntry packet, ResponseEntry *rep)
{
    ssid_idx = atoi(packet.api_uri_array[2]);
    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
    {
        debug_print("[Senao API] Error!! ssid idx!\n");
        return FAIL;
    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    json_chk_wireless_traffic_shaping(rep, packet.body, WIFI_RADIO_NAME_24G, ssid_idx);

    return 0;
}

int check_wifi_ssid_bandsteering(HTTPEntry packet, ResponseEntry *rep)
{
    ssid_idx = atoi(packet.api_uri_array[2]);
    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
    {
        debug_print("[Senao API] Error!! ssid idx!\n");
        return FAIL;
    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_chk_wireless_band_steering(rep, packet.body, WIFI_RADIO_NAME_24G, ssid_idx);

    return 0;
}

int check_wifi_ssid_radiusserver(HTTPEntry packet, ResponseEntry *rep)
{
    ssid_idx = atoi(packet.api_uri_array[2]);
    if (ssid_idx < 0  ||  ssid_idx > AP_WIFI_IFACE_NUM)
    {
        debug_print("[Senao API] Error!! ssid idx!\n");
        return FAIL;
    }
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_chk_wireless_radius_server(rep, packet.body, WIFI_RADIO_NAME_24G, ssid_idx);

    return 0;
}

int check_wifi_ssid_scheduling(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    ssid_idx = atoi(packet.api_uri_array[2]);
    if (ssid_idx < 0  || ssid_idx >7)
    {
        debug_print("[Senao API] Error!! ssid idx!\n");
        return FAIL;
    }
    json_chk_wireless_scheduling(rep, packet.body, WIFI_RADIO_NAME_24G, ssid_idx);

    return 0;
}
int check_wifi_ssid_l2_acl(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    ssid_idx = atoi(packet.api_uri_array[2]);
    if (ssid_idx < 0  || ssid_idx >7)
    {
        debug_print("[Senao API] Error!! ssid idx!\n");
        return FAIL;
    }
    json_chk_wireless_l2_acl(rep, packet.body, WIFI_RADIO_NAME_24G, ssid_idx);
    return 0;
}

int check_net_ethernet(HTTPEntry packet, ResponseEntry *rep)
{
    //curl -v -k -X POST "https://192.168.1.1:4430/api/net/ethernet" -H "accept: */*" -H "Content-Type: application/json" -H "Authorization: Basic YWRtaW46YWRtaW4=" -d"{\"mode\":\"Static\",\"lacp_mode\":true,\"ip\":\"192.168.1.2\",\"mask\":\"255.255.255.0\",\"gateway\":\"192.168.1.3\",\"primary_dns\":\"192.168.1.4\",\"secondary_dns\":\"192.168.1.5\"}"
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_chk_ethernet(rep, packet.body);

    return 0;
}

int check_sys_system_config(HTTPEntry packet, ResponseEntry *rep)
{
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 
    json_chk_sys_info(rep, packet.body);

    return 0;
}

int check_sys_fw_upgrade(HTTPEntry packet, ResponseEntry *rep)
{
    //curl -v -k -X POST "https://192.168.1.1:4430/api/sys/system_config" -H "accept: */*" -H "Content-Type: application/json" -H "Authorization: Basic YWRtaW46YWRtaW4=" -d"{\"system_name\":\"LEO_MODEL\",\"time_zone\":\"UTC6 UTC-06:00 Mexico\",\"led_enable\":true,\"location\":\"abc\"}"

    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__); 

    return 0;
}
#endif

