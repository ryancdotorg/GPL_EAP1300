#ifndef _JSON_SYSTEM_H_
#define _JSON_SYSTEM_H_


#include <json_object.h>
#include <json_tokener.h>
#include <api_response.h>

int json_sys_revert(ResponseEntry *rep, char *query_str);
int json_mgm_fw_upgrade(ResponseEntry *rep, char *query_str);
int json_mgm_fw_upgrade_only(ResponseEntry *rep, char *query_str);
int json_mgm_drop_caches(ResponseEntry *rep, char *query_str);
int json_fw_upgrade_fromserver(ResponseEntry *rep, char *query_str);
int json_mgm_localupgradeimage(ResponseEntry *rep, struct json_object *jobj);
int json_get_auth_check(ResponseEntry *rep, struct json_object *jobj);
int burn_fw();
int burn_fw_only();
int json_mgm_ping(ResponseEntry *rep, char *query_str);
int json_get_mgm_ping(ResponseEntry *rep, struct json_object *jobj);
int json_mgm_traceroute(ResponseEntry *rep, char *query_str);
int json_get_mgm_traceroute(ResponseEntry *rep, struct json_object *jobj);
int json_get_sys_dev_capability(ResponseEntry *rep, struct json_object *jobj);
#if HAS_SENAO_PACKETEER
int json_get_sys_benu_tunnel_addr(ResponseEntry *rep, struct json_object *jobj);
int json_set_sys_benu_tunnel_addr(ResponseEntry *rep, char *query_str);
#endif
int json_get_lxc_ezm_backup_info(ResponseEntry *rep, struct json_object *jobj);
int json_get_lxc_ezm_backup_method(ResponseEntry *rep, struct json_object *jobj);
int json_post_lxc_ezm_backup_method(ResponseEntry *rep, char *query_str);
int json_get_lxc_ezm_restore(ResponseEntry *rep, struct json_object *jobj);
int json_get_sys_first_login(ResponseEntry *rep, struct json_object *jobj);
int json_post_sys_first_login(ResponseEntry *rep, char *query_str);
#endif
