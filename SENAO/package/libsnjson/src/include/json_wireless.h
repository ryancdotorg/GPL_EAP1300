#ifndef _JSON_WIRELESS_H_
#define _JSON_WIRELESS_H_


#include <json_object.h>
#include <json_tokener.h>
#include <api_response.h>
#include <utility/sys_common.h>


int json_get_wireless_radio(ResponseEntry *rep, struct json_object *jarr, char *radio);
int json_set_wireless_radio(ResponseEntry *rep, char *query_str, char *radio);
int json_get_mesh(ResponseEntry *rep, struct json_object *jarr);
int json_set_mesh(ResponseEntry *rep, char *query_str);
int json_get_ethernet(ResponseEntry *rep, struct json_object *jarr);
int json_set_ethernet(ResponseEntry *rep, char *query_str);
int json_get_ethernet_traffic_info(ResponseEntry *rep, struct json_object *jarr);
int json_post_linktest(ResponseEntry *rep, struct json_object *jobj);
int json_get_linktest(ResponseEntry *rep, struct json_object *jobj, int resultPath);
int json_get_sys_controlled_info(ResponseEntry *rep, struct json_object *jobj);
int json_get_sys_info(ResponseEntry *rep, struct json_object *jarr);
int json_set_sys_info(ResponseEntry *rep, char *query_str);
int json_fw_upgrade(ResponseEntry *rep, char *query_str);
int json_sys_apply(ResponseEntry *rep, char *query_str);
int32_t check_wifix_channel(wlan_info *wlanInfo, char *wifix, int country_code, int green_mode, int ch);
int json_get_wifi_vpn_profile(ResponseEntry *rep, struct json_object *jobj);
int json_set_wifi_vpn_profile(ResponseEntry *rep, char *query_str);
int json_delete_all_vpn_profile(ResponseEntry *rep, char *query_str);
int json_get_wifi_vpn_status(ResponseEntry *rep, struct json_object *jobj);

#endif
