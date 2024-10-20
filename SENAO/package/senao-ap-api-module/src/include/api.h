#ifndef _JSON_AP_API_H_
#define _JSON_AP_API_H_

#include <api_response.h>

#define API_PARSING_LEVEL 7
#define API_LEVEL_LENGTH  32
#define OK    0
#define FAIL -1
#define VARIABLE "change_id"  //table paring will skip this item
#define VARIABLE_OPMODE "ap|wds_ap_24g|wds_ap_5g|wds_bridge_24g|wds_bridge_5g|wds_sta_24g|wds_sta_5g|sta_ap|sta_24g|sta_5g|enjet|sta_ap_24g|sta_ap_5g|wds_ap_5g_2|wds_bridge_5g_2|sta_ap_5g_2"

typedef struct __HTTPEntry
{
    const char *query_string;
    const char *request_method;
    const char *body;
    const char (*api_uri_array)[API_LEVEL_LENGTH];
} HTTPEntry;

typedef struct __ApiEntry
{
    const char *uri[API_PARSING_LEVEL];
    const char *method;
    const int  group;
    int (*cb_function)(HTTPEntry packet, ResponseEntry *rep);
} ApiEntry;

#endif
