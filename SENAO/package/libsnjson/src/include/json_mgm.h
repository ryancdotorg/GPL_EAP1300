#ifndef _JSON_MGM_H_
#define _JSON_MGM_H_


#include <json_object.h>
#include <json_tokener.h>
#include <api_response.h>

int json_mgm_fw_upgrade(ResponseEntry *rep, char *query_str);
int json_fw_upgrade_fromserver(ResponseEntry *rep, char *query_str);
int json_mgm_localupgradeimage(ResponseEntry *rep, struct json_object *jobj);
int json_get_auth_check(ResponseEntry *rep, struct json_object *jobj);
int burn_fw();
int json_mgm_ping(ResponseEntry *rep, char *query_str);
int json_get_mgm_ping(ResponseEntry *rep, struct json_object *jobj);
int json_mgm_traceroute(ResponseEntry *rep, char *query_str);
int json_get_mgm_traceroute(ResponseEntry *rep, struct json_object *jobj);
int json_get_mgm_tools_iperf(ResponseEntry *rep, struct json_object *jobj);
int json_set_mgm_tools_iperf(ResponseEntry *rep, char *query_str);
int json_set_mgm_backup_config(ResponseEntry *rep,  char *query_str);
int json_set_mgm_restore_config(ResponseEntry *rep, char *query_str);
int json_set_mgm_reboot(ResponseEntry *rep, char *query_str);
int json_set_mgm_auto_reboot_cfg(ResponseEntry *rep, char *query_str);
int json_set_mgm_reset_to_default(ResponseEntry *rep, char *query_str);
int json_set_mgm_device_discovery(ResponseEntry *rep, char *query_str);
int json_get_mgm_device_discovery(ResponseEntry *rep, struct json_object *jobj);
#if SUPPORT_NETGEAR_FUNCTION
int json_get_mgm_mode(ResponseEntry *rep, struct json_object *jobj);
int json_set_mgm_mode(ResponseEntry *rep, char *query_str);
#endif
int json_get_mgm_led_list(ResponseEntry *rep, struct json_object *jobj);
int json_get_mgm_led_status(ResponseEntry *rep, struct json_object *jobj);
int json_set_mgm_led_cfg(ResponseEntry *rep, char *query_str);
#endif
