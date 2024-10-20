#ifndef _JSON_SSID_H_
#define _JSON_SSID_H_

#include <json_object.h>
#include <json_tokener.h>
#include <api_response.h>

#define SECTION_NAME_LENGTH 128
#define SSID_NAME_LENGTH 64
#define SSID_ENABLE_BANDS 13
#define BAND_STEER_TYPE_LENGTH 32
#define ACL_MACFILTER_LENGTH 16
#define ACL_MACLIST_LENGTH 2048
#define PASSPHRASE_LENGTH 65
#define IP_LENGTH 32
#define NAS_ID_LENGTH 33
#define RADIUS_SERVER_SECRET_LENGTH 64
#define ENCRYPTION_LENGTH 32
#define AUTH_TYPE_LENGTH 32
#define CIPERTYPE_LENGTH 32
#define QUERY_STR_LENGTH 1024

#define JSON_WIRELESS "wireless"
#define JSON_WIRELESS_WIFI_BANDSTEER_EN "bandsteer_en"
#define JSON_WIRELESS_WIFI_BANDSTEER_TYPE "bandsteer"
#define JSON_WIRELESS_WIFI_BANDSTEER_PERCENT "bandsteerpersent"
#define JSON_WIRELESS_WIFI_BANDSTEER_RSSI "bandsteerrssi"
#define JSON_WIRELESS_WIFI_ACL_ENABLE "acl_enable"
#define JSON_WIRELESS_WIFI_ACL_MACFILTER "macfilter"
#define JSON_WIRELESS_WIFI_ACL_MACLIST "maclist"

#define DESC_BAND_STEER_DISABLE "disable"
#define DESC_BAND_STEER_FORCE_5G "force_5g"
#define DESC_BAND_STEER_PREFER_5G "prefer_5g"
#define DESC_BAND_STEER_BAND_BALANCE "band_balance"
#define DESC_ACL_NONE "NONE"
#define DESC_ACL_ALLOW "ALLOW"
#define DESC_ACL_DENY "DENY"
#define DESC_ENCRYPTION_NONE "Disabled"
#define DESC_WPA_PSK "WPA-PSK"
#define DESC_WPA2_PSK "WPA2-PSK"
#define DESC_WPA_PSK_MIXED "WPA-PSK-Mixed"
#define DESC_WPA_EAP "WPA-Enterprise"
#define DESC_WPA2_EAP "WPA2-Enterprise"
#define DESC_WPA_EAP_MIXED "WPA Mixed-Enterprie"
#define DESC_WPA3_SAE "WPA3-SAE"
#define DESC_WPA3_SAE_MIXED "WPA3-SAE-Mixed"
#define DESC_WPA3_EAP "WPA3-Enterprise"
#define DESC_WPA3_EAP_MIXED "WPA3-Mixed-Enterprise"
#define DESC_OWE "OWE"
#define DESC_API_OPTION_ERROR "API_OPTION_ERROR"
#define DESC_TKIP "TKIP"
#define DESC_CCMP "AES"
#define DESC_TKIP_CCMP "TKIP + AES"
#define DESC_WEP "WEP"
#define DESC_OPEN "OPEN"
#define DESC_SHARED "SHARED"
#if SUPPORT_WLAN_5G_2_SETTING
#define RADIO_NUM 3
#else
#define RADIO_NUM 2
#endif

#define GET_SECTION_NAME(RADIO_IDX, OPMODE ,SECTION, RESPONSE_STATUS_PTR) {\
    assert (RADIO_IDX == 0 || RADIO_IDX == 1 || RADIO_IDX == 2);\
    if(_api_get_wifi_opmode_option(&OPMODE)) RET_GEN_ERRORMSG(RESPONSE_STATUS_PTR, API_INTERNAL_ERROR, "GET_SECTION_NAME");\
    if(api_get_wifix_section_name(OPMODE, RADIO_IDX, SECTION)) RET_GEN_ERRORMSG(RESPONSE_STATUS_PTR, API_INTERNAL_ERROR, "GET_SECTION_NAME");\
    }

typedef struct _ssid_cfg_st {
    int opmode;
    char section[32]; 
    int  idx; 
} ssid_cfg_st;

typedef struct FINGER_PRINT_DATA{
    char   mac[20];
    char   os[33];
    char   ip[20];
    char   device_name[32];
}Finger_Print;

typedef struct TRAFFIC_INFORMATION{
    int    traffic_id;
    char   down[16];
    char   up[16];
    char   assoc_time[16];
}TRAFFIC;
typedef struct VAP_INFORMATION{
    int             ssid;
    int             ssid_id;
    int             traffic_num;
    TRAFFIC         traffic[10];
}VAP;
typedef struct CLIENT_INFORMATION{
    char   mac[20];
    char   os[33];
    int    vap_num;
    VAP    vap[24];
    char   rssi[16];
    int    cur_ssid_id;
    char   rate[16];
    char   ip[20];
    char   device_name[32];
    int    channel;
    char   rx_byte[16];
    char   tx_byte[16];
    int    ssid_id;
}CLIENTINFO;

CLIENTINFO g_clientinfo[127];
typedef enum {
    FORCE_5G = 1,
    PREFER_5G,
    BAND_BALANCE
} BAND_STEER_TYPE;

int json_get_wifi_mgmt(ResponseEntry *rep, struct json_object *jobj);
int json_set_wifi_mgmt(ResponseEntry *rep, char *query_str);
int json_get_wireless_ssid(struct json_object *jobj, ssid_cfg_st *ssidCfg_p);
int json_get_wireless_band_steering(struct json_object *jobj, ssid_cfg_st *ssidCfg_p);
int json_get_wireless_traffic_shaping(struct json_object *jobj, ssid_cfg_st *ssidCfg_p);
int json_get_wireless_security(struct json_object *jobj, ssid_cfg_st *ssidCfg_p);
int json_get_wireless_encryption(struct json_object *jobj, ssid_cfg_st *ssidCfg_p);
int json_get_wireless_radius_server(struct json_object *jobj, ssid_cfg_st *ssidCfg_p);
int json_get_wireless_accounting_server(struct json_object *jobj, ssid_cfg_st *ssidCfg_p);
int json_get_wireless_guest_network(struct json_object *jobj, ssid_cfg_st *ssidCfg_p);
int json_get_wireless_scheduling(struct json_object *jobj, ssid_cfg_st *ssidCfg_p);
int json_get_wireless_scheduling_days(struct json_object *jobj, ssid_cfg_st *ssidCfg_p, int day);
int json_get_wireless_l2_acl(struct json_object *jobj, ssid_cfg_st *ssidCfg_p);
int json_get_client_info(ResponseEntry *rep, struct json_object *jobj);
int json_get_wifi_site_survey(ResponseEntry *rep, struct json_object *jobj);
int json_get_wireless_traffic_info(struct json_object *jobj, ssid_cfg_st *ssidCfg_p);
int json_set_wireless_ssid(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_set_wireless_band_steering(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_set_wireless_traffic_shaping(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_set_wireless_security(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_set_wireless_encryption(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_set_wireless_radius_server(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_set_wireless_accounting_server(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_set_wireless_guest_network(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_set_wireless_scheduling(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char* radio);
int json_set_wireless_scheduling_sync(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_set_wireless_scheduling_days(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char *radio);
int json_set_wireless_scheduling_day(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char *radio, int dayIdx);
int json_set_wireless_l2_acl(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p);
int json_wifi_site_survey(ResponseEntry *rep, char *query_str);
int get_wifi_ifacex_disabled(int radio, int index, ssid_cfg_st *ssidCfg_p);
int set_wifi_ifacex_disabled(int radio, int index, int disable, ssid_cfg_st *ssidCfg_p);
int json_get_hotspot20(ssid_cfg_st *ssidCfg_p, char *query_str, int str_size, int error_code, char* msg);
int json_set_hotspot20(ssid_cfg_st *ssidCfg_p, char *query_str, int error_code, char* msg);
int json_get_hotspot20_venue(int opmode, int idx, char *section, char *query_str, int str_size, int* error_code, char* msg);
int json_set_hotspot20_venue(int opmode, int idx, char *section, char *query_str, int* error_code, char* msg);
int json_get_hotspot20_osu(int opmode, int idx, char *section, char *query_str, int str_size, int* error_code, char* msg);
int json_set_hotspot20_osu(int opmode, int idx, char *section, char *query_str, int* error_code, char* msg);
int json_get_hotspot20_wan_metrics(int opmode, int idx, char* section, char *query_str, int str_size, int* error_code, char* msg);
int json_set_hotspot20_wan_metrics(int opmode, int idx, char* section, char *query_str, int* error_code, char* msg);
int json_get_hotspot20_network(int opmode, int idx, char* section, char *query_str, int str_size, int* error_code, char* msg);
int json_set_hotspot20_network(int opmode, int idx, char* section, char *query_str, int* error_code, char* msg);

#endif
