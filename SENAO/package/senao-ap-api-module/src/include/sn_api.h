#ifndef _JSON_SENAO_API_H_
#define _JSON_SENAO_API_H_

#include <fcgi_stdio.h>
#include <wireless_tokens.h>
#include <json_wireless.h>
#include <json_ssid.h>
#include <json_chk_wireless.h>
#include <json_chk_ssid.h>
#include <json_object.h>
#include <json_tokener.h>
#include <api_json_check.h>
#include <variable/api_wireless.h>
#include <api.h>

int get_api_version(HTTPEntry packet, ResponseEntry *rep);
int post_sys_login(HTTPEntry packet, ResponseEntry *rep);
int get_account_username(HTTPEntry packet, ResponseEntry *rep);
int post_change_account(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_radio_24g(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_radio_24g(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_radio_24g(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_radio_5g(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_radio_5g(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_radio_5g(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_radio_5g_2(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_radio_5g_2(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_radio_5g_2(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_mesh(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_mesh(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_mesh_mesh_info(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_mesh_group_info(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_mgmt(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_mgmt(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_mgmt(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_sta_mode(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_sta_mode(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_sta_mode(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_wds_link(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_wds_link(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_ssids(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_ssid(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_ssid(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_ssid(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_ssid_security(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_ssid_security(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_ssid_security(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_ssid_guestnetwork(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_ssid_guestnetwork(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_ssid_guestnetwork(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_ssid_captive_portal(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_ssid_captive_portal(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_ssid_captive_portal(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_ssid_hs20(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_ssid_hs20(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_ssid_accountingserver(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_ssid_accountingserver(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_ssid_accountingserver(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_ssid_trafficshaping(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_ssid_trafficshaping(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_ssid_trafficshaping(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_ssid_bandsteering(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_ssid_bandsteering(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_ssid_bandsteering(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_ssid_radiusserver(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_ssid_radiusserver(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_ssid_radiusserver(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_ssid_scheduling(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_ssid_scheduling(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_ssid_scheduling(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_ssid_l2_acl(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_ssid_l2_acl(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_ssid_l2_acl(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_ssid_kick(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_ssid_traffic_info(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_site_survey(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_site_survey(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_channel_list(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_channel_list(HTTPEntry packet, ResponseEntry *rep);
int get_net_ethernet(HTTPEntry packet, ResponseEntry *rep);
int post_net_ethernet(HTTPEntry packet, ResponseEntry *rep);
int patch_net_ethernet(HTTPEntry packet, ResponseEntry *rep);
int get_net_ethernet_traffic_info(HTTPEntry packet, ResponseEntry *rep);
int get_net_ethernet_client_info(HTTPEntry packet, ResponseEntry *rep);
int post_linktest(HTTPEntry packet, ResponseEntry *rep);
int get_linktest(HTTPEntry packet, ResponseEntry *rep);
int get_sys_sys_info(HTTPEntry packet, ResponseEntry *rep);
int get_sys_controlled_info(HTTPEntry packet, ResponseEntry *rep);
int post_sys_system_config(HTTPEntry packet, ResponseEntry *rep);
int patch_sys_system_config(HTTPEntry packet, ResponseEntry *rep);
int get_sys_syslog(HTTPEntry packet, ResponseEntry *rep);
int post_sys_syslog(HTTPEntry packet, ResponseEntry *rep);
int patch_sys_syslog(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_client_info(HTTPEntry packet, ResponseEntry *rep);
int post_sys_fw_upgrade(HTTPEntry packet, ResponseEntry *rep);
int post_sys_apply(HTTPEntry packet, ResponseEntry *rep);
int post_sys_reload(HTTPEntry packet, ResponseEntry *rep);
int post_sys_revert(HTTPEntry packet, ResponseEntry *rep);
int get_ezmcloud_info(HTTPEntry packet, ResponseEntry *rep);
int get_net_proxy_settings(HTTPEntry packet, ResponseEntry *rep);
int patch_net_proxy_settings(HTTPEntry packet, ResponseEntry *rep);
int post_net_proxy_settings(HTTPEntry packet, ResponseEntry *rep);
int get_net_proxy_http(HTTPEntry packet, ResponseEntry *rep);
int get_net_proxy_https(HTTPEntry packet, ResponseEntry *rep);
int post_net_proxy_http(HTTPEntry packet, ResponseEntry *rep);
int post_net_proxy_https(HTTPEntry packet, ResponseEntry *rep);
int get_net_iptable_rules(HTTPEntry packet, ResponseEntry *rep);
int post_net_iptable_rules(HTTPEntry packet, ResponseEntry *rep);
int patch_net_iptable_rules(HTTPEntry packet, ResponseEntry *rep);
int get_net_spanning_tree(HTTPEntry packet, ResponseEntry *rep);
int post_net_spanning_tree(HTTPEntry packet, ResponseEntry *rep);
int patch_net_spanning_tree(HTTPEntry packet, ResponseEntry *rep);
int get_net_internet_speedtest(HTTPEntry packet, ResponseEntry *rep);
int post_net_internet_speedtest(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_mesh_throughput(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_mesh_throughput(HTTPEntry packet, ResponseEntry *rep);
int get_auth_check(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_fwupgrade(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_fwupgrade_only(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_drop_caches(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_localupgradeimage(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_restoreimage(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_tools_iperf(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_tools_iperf(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_backup_config(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_backup_config(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_restore_config(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_reboot(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_auto_reboot_cfg(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_auto_reboot_cfg(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_reset_to_default(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_reset_with_key(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_led_list(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_led_status(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_led_cfg(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_ping(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_ping(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_traceroute(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_traceroute(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_nslookup(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_nslookup(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_device_discovery(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_device_discovery(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_gps(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_gps(HTTPEntry packet, ResponseEntry *rep);
int get_dev_capability(HTTPEntry packet, ResponseEntry *rep);
int get_first_login(HTTPEntry packet, ResponseEntry *rep);
int post_first_login(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_fw_check(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_fw_check(HTTPEntry packet, ResponseEntry *rep);
int get_mgm_fw_download(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_fw_download(HTTPEntry packet, ResponseEntry *rep);
#if SUPPORT_NETGEAR_FUNCTION
int get_mgm_mode(HTTPEntry packet, ResponseEntry *rep);
int post_mgm_mode(HTTPEntry packet, ResponseEntry *rep);
#endif
int get_ssh_setting(HTTPEntry packet, ResponseEntry *rep);
int post_ssh_setting(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_guest_network(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_guest_network(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_guest_network(HTTPEntry packet, ResponseEntry *rep);
#if HAS_SENAO_PACKETEER
int get_benu_tunnel_addr(HTTPEntry packet, ResponseEntry *rep);
int post_benu_tunnel_addr(HTTPEntry packet, ResponseEntry *rep);
#endif
int get_lxc_ezm_backup_info(HTTPEntry packet, ResponseEntry *rep);
int get_lxc_ezm_backup_method(HTTPEntry packet, ResponseEntry *rep);
int post_lxc_ezm_backup_method(HTTPEntry packet, ResponseEntry *rep);
int get_lxc_ezm_restore(HTTPEntry packet, ResponseEntry *rep);
int check_wifi_radio_24g(HTTPEntry packet, ResponseEntry *rep);
int check_wifi_radio_5g(HTTPEntry packet, ResponseEntry *rep);
int check_wifi_radio_triband_5g(HTTPEntry packet, ResponseEntry *rep);
int check_wifi_mesh(HTTPEntry packet, ResponseEntry *rep);
int check_wifi_ssid(HTTPEntry packet, ResponseEntry *rep);
int check_wifi_ssid_security(HTTPEntry packet, ResponseEntry *rep);
int check_wifi_ssid_captiveportal(HTTPEntry packet, ResponseEntry *rep);
int check_wifi_ssid_accountingserver(HTTPEntry packet, ResponseEntry *rep);
int check_wifi_ssid_trafficshaping(HTTPEntry packet, ResponseEntry *rep);
int check_wifi_ssid_bandsteering(HTTPEntry packet, ResponseEntry *rep);
int check_wifi_ssid_radiusserver(HTTPEntry packet, ResponseEntry *rep);
int check_wifi_ssid_scheduling(HTTPEntry packet, ResponseEntry *rep);
int check_wifi_ssid_l2_acl(HTTPEntry packet, ResponseEntry *rep);
int check_net_ethernet(HTTPEntry packet, ResponseEntry *rep);
int check_sys_system_config(HTTPEntry packet, ResponseEntry *rep);
int check_sys_fw_upgrade(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_scan_chanutil(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_scan_aplist(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_scan_stalist(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_vpn_profile(HTTPEntry packet, ResponseEntry *rep);
int post_wifi_vpn_profile(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_vpn_profile(HTTPEntry packet, ResponseEntry *rep);
int patch_wifi_delete_all_vpn_profile(HTTPEntry packet, ResponseEntry *rep);
int get_wifi_vpn_status(HTTPEntry packet, ResponseEntry *rep);
#endif
