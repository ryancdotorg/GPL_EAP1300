#include <api_common.h>
#include <sys_common.h>
#include <api_wireless.h>
#include <variable.h>
#include <api_lan.h>
#include <wireless_tokens.h>
//#include <integer_check.h>
#include <json_object.h>
#include <json_tokener.h>
#include <json_chk_ssid.h>
#include <json_common.h>
#include <variable/api_schedule.h>
#include <unistd.h>

#define SECTION_NAME_LENGTH 128
#define SSID_NAME_LENGTH 64
#define SSID_ENABLE_BANDS 13
#define BAND_STEER_TYPE_LENGTH 32
#define ACL_MACFILTER_LENGTH 16
#define ACL_MACLIST_LENGTH 1024
#define PASSPHRASE_LENGTH 64
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
#define DESC_WPA_PSK_MIXED "WPA-PSK Mixed"
#define DESC_WPA_EAP "WPA-Enterprise"
#define DESC_WPA2_EAP "WPA2-Enterprise"
#define DESC_WPA_EAP_MIXED "WPA Mixed-Enterprie"
#define DESC_API_OPTION_ERROR "API_OPTION_ERROR"
#define DESC_TKIP "TKIP"
#define DESC_CCMP "AES"
#define DESC_TKIP_CCMP "TKIP + AES"
#if SUPPORT_WLAN_5G_2_SETTING
#define RADIO_NUM 3
#else
#define RADIO_NUM 2
#endif

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

int* _api_get_wifi_opmode_option(int *op_mode){
    *op_mode = OPM_AP;
    return 0;
}
int json_chk_wireless_ssid(ResponseEntry *rep, char *query_str, char *radio, int idx)
{
    return 0;
}
int json_chk_wireless_band_steering(ResponseEntry *rep, char *query_str, char *radio, int idx)
{
    return 0;
}
int json_chk_wireless_traffic_shaping(ResponseEntry *rep, char *query_str, char *radio, int idx)
{
    return 0;
}
int json_chk_wireless_security(ResponseEntry *rep, char *query_str, char *radio, int idx)
{
    return 0;
}
int json_chk_wireless_encryption(ResponseEntry *rep, char *query_str, char *radio, int idx)
{
    return 0;
}
int json_chk_wireless_radius_server(ResponseEntry *rep, char *query_str, char *radio, int idx)
{
    return 0;
}
int json_chk_wireless_accounting_server(ResponseEntry *rep, char *query_str, char *radio, int idx)
{
    return 0;
}
int json_chk_wireless_captive_portal(ResponseEntry *rep, char *query_str, char *radio, int idx)
{
    struct json_object *jobj;
    char section[SECTION_NAME_LENGTH] = {0}, *guest_network=NULL, *ext_url=NULL;
    char *walled_garden=NULL, *auth_type_str=NULL ;
    int i, radio_idx = 0, opmode = 0, auth_type = 0, session_timeout = 0, idle_timeout = 0;
    bool enable;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if(!senao_json_object_get_boolean(jobj, "enable", &enable))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE,"ENABLE");
            }
            if(!senao_json_object_get_and_create_string(rep, jobj, "opmode", &guest_network))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE,"OPMODE");
            }
            if(!senao_json_object_get_and_create_string(rep, jobj, "auth_type", &auth_type_str))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE,"AUTH_TYPE");
            }
            if(!senao_json_object_get_and_create_string(rep, jobj, "external_splash_url", &ext_url))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE,"EXT_URL");
            }
            if(!senao_json_object_get_integer(jobj, "session_timeout", &session_timeout))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE,"SESSION_TIMEOUT");
            }
            if(!senao_json_object_get_integer(jobj, "idle_timeout", &idle_timeout))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE,"IDLE_TIMEOUT");
            }
            if(!senao_json_object_get_and_create_string(rep, jobj, "walled_garden", &walled_garden))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE,"WALLED_GARDEN");
            }
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_NUMBER_OF_ARGUMENTS, NULL);
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_NUMBER_OF_ARGUMENTS, NULL);
    }
    if (strcasecmp(guest_network, "nat") != 0 && strcasecmp(guest_network, "bridge") != 0)
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"GUEST_NETWORK");
    }

    if (_api_get_wifi_opmode_option(&opmode))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_NUMBER_OF_ARGUMENTS, NULL);
    }

    if (strcmp(auth_type_str, "splash") != 0 && strcmp(auth_type_str, "cloud-radius") != 0 && strcmp(auth_type_str, "social-login") != 0 && strcmp(auth_type_str, "customer-radius") != 0 && strcmp(auth_type_str, "3rd-party") != 0)
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"AUTH_TYPE");
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
int json_chk_wireless_scheduling(ResponseEntry *rep, char *query_str, char *radio, int idx)
{
    return 0;
}
int json_chk_wireless_scheduling_days(ResponseEntry *rep, char *query_str, char *radio, int idx)
{
    return 0;
}
int json_chk_wireless_scheduling_day(ResponseEntry *rep, char *query_str, char *radio, int idx, int dayIdx)
{
    return 0;
}
int json_chk_wireless_l2_acl(ResponseEntry *rep, char *query_str, char *radio, int idx)
{
    return 0;
}
