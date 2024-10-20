#ifndef _JSON_CHK_SSID_H_
#define _JSON_CHK_SSID_H_

#include <json_object.h>
#include <json_tokener.h>
#include <api_response.h>
static char * scheduling_days[] = {
        "su",
        "mo",
        "tu",
        "we",
        "th",
        "fr",
        "sa"
};
static int scheduling_days_length = (sizeof (scheduling_days) / sizeof (const char *));

int* _api_get_wifi_opmode_option(int *op_mode);
int json_chk_wireless_ssid(ResponseEntry *rep, char *query_str, char *radio, int idx);
int json_chk_wireless_band_steering(ResponseEntry *rep, char *query_str, char *radio, int idx);
int json_chk_wireless_traffic_shaping(ResponseEntry *rep, char *query_str, char *radio, int idx);
int json_chk_wireless_security(ResponseEntry *rep, char *query_str, char *radio, int idx);
int json_chk_wireless_encryption(ResponseEntry *rep, char *query_str, char *radio, int idx);
int json_chk_wireless_radius_server(ResponseEntry *rep, char *query_str, char *radio, int idx);
int json_chk_wireless_accounting_server(ResponseEntry *rep, char *query_str, char *radio, int idx);
int json_chk_wireless_captive_portal(ResponseEntry *rep, char *query_str, char *radio, int idx);
int json_chk_wireless_scheduling(ResponseEntry *rep, char *query_str, char *radio, int idx);
int json_chk_wireless_scheduling_days(ResponseEntry *rep, char *query_str, char *radio, int idx);
int json_chk_wireless_scheduling_day(ResponseEntry *rep, char *query_str, char *radio, int idx, int dayIdx);
int json_chk_wireless_l2_acl(ResponseEntry *rep, char *query_str, char *radio, int idx);
#endif
