#ifndef _JSON_CHK_WIRELESS_H_
#define _JSON_CHK_WIRELESS_H_


#include <json_object.h>
#include <json_tokener.h>
#include <api_response.h>

int json_chk_wireless_radio(ResponseStatus *res, char *query_str, char *radio);
int json_chk_mesh(ResponseStatus *res, char *query_str);
int json_chk_ethernet(ResponseStatus *res, char *query_str);
int json_chk_sys_info(ResponseStatus *res, char *query_str);
int json_chk_fw_upgrade(ResponseStatus *res, char *query_str);
#endif
