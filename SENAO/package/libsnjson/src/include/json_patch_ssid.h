#ifndef _JSON_PATCH_SSID_H_
#define _JSON_PATCH_SSID_H_

int json_patch_wireless_ssid(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_patch_wireless_band_steering(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_patch_wireless_traffic_shaping(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_patch_wireless_security(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_patch_wireless_encryption(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_patch_wireless_radius_server(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_patch_wireless_accounting_server(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_patch_wireless_guest_network(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_patch_wireless_scheduling(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char* radio);
int json_patch_wireless_scheduling_sync(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_patch_wireless_scheduling_days(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char *radio);
int json_patch_wireless_scheduling_day(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char *radio, int dayIdx);
int json_patch_wireless_l2_acl(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
#endif
