/****************************************************************************
;
;   (C) Unpublished Work of Senao Networks, Inc.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF SENAO INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) SENAO EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN SENAO WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF SENAO.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;----------------------------------------------------------------------------
;
;    Project : app_agent
;    Creator : Jerry Chen
;    File    : json_setting.c
;    Abstract:
;
;       Modification History:
;       By              Date        Ver.    Modification Description
;       --------------- --------    -----   -------------------------------------
;       Jerry           2012/09/10          First commit
;****************************************************************************/

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "ipcam_samba_setting.h"
#include "appagent_cfg_data.h"
#include "app_agent.h"
#include "json_setting.h"
#include "json.h"
#include "device_setting.h"
#include "wlan_setting.h"
#include "sysCommon.h"
#include "sysCore.h"
#include "sysFile.h"
#include "variable/api_sys.h"
#include "gconfig.h"
#include "deviceinfo.h"
#include <senao-sysutil/regx.h>
#if HAS_IPCAM
#include <variable/api_ipcam.h>
#include "utility/sys_common.h"
#endif
#include "ddns_setting.h"

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/
#if SUPPORT_WAN_SETTING
extern WAN_CONNECT_TYPE wanConnectTypeArr[];
extern int wanConnectTypeArrSize;
char *getIPv4WanPara[] = {
    "IPAddress",
    "SubnetMask",
    "Gateway",
    "PrimaryDNS",
    "SecondaryDNS",
    "Hostname",
    "MacAddress",
    "CloneMac",
    "Username",
    "Password",
    "Service",
    "MTU",
    "ConnectionType",
    "IdleTime",
    "ConnectionId",
    "IsIPDynamic",
    "DefaultGateway",
    "L2TPGateway",
    "PPTPGateway",
    "MultiWanBridgePort",
    "DSLiteConfigurationManual",
    "AFTRIPv6Address",
    "B4IPv4Address",
    "Ipv6WanAddress",
    "Ipv6WanDefaultGateway"
};
#endif
char *supported_mode_2_4_G[] = {
    WLAN_802_11_B_STR,
    WLAN_802_11_G_STR,
    WLAN_802_11_N_STR,
    WLAN_802_11_BG_STR,
    WLAN_802_11_BGN_STR
};

char *supported_mode_5_G[] = {
    WLAN_802_11_A_STR,
    WLAN_802_11_N_STR,
    WLAN_802_11_AN_STR,
    WLAN_802_11_AC_STR
};

char *security_wep_mode[] = {
    WLAN_WEP_OPEN_STR, 
    WLAN_WEP_SHARED_STR
};

char *wep_auth_type[] = {
    WLAN_WEP_64_STR, 
    WLAN_WEP_128_STR
};

char *security_wpa_mode[] = {
    WLAN_WPA_PSK_STR,
    WLAN_WPA_RADIUS_STR,
    WLAN_WPA2_PSK_STR,
    WLAN_WPA2_RADIUS_STR,
    WLAN_WPAORWPA2_PSK_STR,
    WLAN_WPAORWPA2_RADIUS_STR
};

char *cipher_type[] = {
    WLAN_CIPHER_TKIP_STR,
    WLAN_CIPHER_AES_STR,
    WLAN_CIPHER_TKIPORAES_STR
};

char *getWLANPara[] = {
    "Type",
    "Encryption",
    "Key",
    "Enable802.1x",
    "RadiusIP",
    "RadiusPort",
    "RadiusPassword"
};

enum{
    AGENT_SUPPORT_HTTPS_ONLY=0,
    AGENT_SUPPORT_HTTPS_AND_HTTP_ENCRPT,
    AGENT_SUPPORT_HTTPS_AND_HTTP,
    AGENT_SUPPORT_HTTP_ENCRPT_ONLY,
    AGENT_SUPPORT_HTTP_ONLY
};

#define ADD_JSON_OBJECT_NEW_STRING(objname, inputStr, objstr)  (json_object_object_add(objname, inputStr,  json_object_new_string(objstr)))
#define ADD_JSON_OBJECT_NEW_INT(objname, inputInt, objint) (json_object_object_add(objname, inputInt,  json_object_new_int(objint)))
#define ADD_JSON_OBJECT_NEW_BOOL(objname, inputInt, objbool) (json_object_object_add(objname, inputInt,  json_object_new_boolean(objbool)))

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/

/*****************************************************************
* NAME:    basic_json_response
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int basic_json_response(HTTPS_CB *pkt, char *result)
{
    if(NULL == pkt)
    {
        return -1;
    }

    /* Construct the response packet */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer("%s\n", result);

    http_send_stored_data(pkt->fd);

    return 0;
}

/*****************************************************************
* NAME:    senao_json_object_get_boolean
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool senao_json_object_get_boolean(struct json_object *jobj, char *obj_name, bool *res)
{
    struct json_object *jobj_bool;
    char boolean_str[16];
    bool result;

    *res = FALSE;
    result = FALSE;

    if((jobj_bool = json_object_object_get(jobj, obj_name)))
    {
        sprintf(boolean_str, "%s", json_object_get_string(jobj_bool));

        /* Get the setting successfully. */
        result = TRUE;

        if(!strcasecmp("true", boolean_str))
        {
            *res = TRUE;
        }
        else if(!strcasecmp("Enable", boolean_str))
        {
            *res = TRUE;
        }

        /* Free obj */
        json_object_put(jobj_bool);
    }

    return result;
}

/*****************************************************************
* NAME:    senao_json_object_get_string
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool senao_json_object_get_string(struct json_object *jobj, char *obj_name, char *res)
{
    struct json_object *jobj_str;
    bool result;

    result = FALSE;

    if((jobj_str = json_object_object_get(jobj, obj_name)))
    {
        sprintf(res, "%s", json_object_get_string(jobj_str));

        /* Get the setting successfully. */
        result = TRUE;

        /* Free obj */
        json_object_put(jobj_str);
    }

    return result;
}

/*****************************************************************
* NAME:    senao_json_object_get_integer
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool senao_json_object_get_integer(struct json_object *jobj, char *obj_name, int *res)
{
    struct json_object *jobj_int;
    bool result;

    result = FALSE;

    if((jobj_int = json_object_object_get(jobj, obj_name)))
    {
        *res = json_object_get_int(jobj_int);

        /* Get the setting successfully. */
        result = TRUE;

        /* Free obj */
        json_object_put(jobj_int);
    }

    return result;
}

/*****************************************************************
* NAME:    get_json_boolean_from_query
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_boolean_from_query(char *query_str, bool *output, char *boolean_name)
{
    struct json_object *jobj, *jobj_boolean;
    int ret = 0;

    if(NULL != query_str)
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            if(senao_json_object_get_boolean(jobj, boolean_name, output))
            {
                printf("APP agent [%s] : %s = %d\n", __func__, boolean_name, *output);
            }
            else
            {
                printf("APP agent [%s] : Can't get boolean %s!\n", __func__, boolean_name);
                ret = -1;
            }

            /* Free obj */
            json_object_put(jobj);
        }
    }
    else
    {
        printf("APP agent [%s] : NULL == query_str!\n", __func__);
        ret = -1;
    }

    return ret;
}

/*****************************************************************
* NAME:    get_json_string_from_query
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_string_from_query(char *query_str, char *outbuf, char *string_name)
{
    struct json_object *jobj, *jobj_string;
    int ret = 0;

    if(NULL != query_str)
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            if((jobj_string = json_object_object_get(jobj, string_name)))
            {
                ret = 1;
                sprintf(outbuf, "%s", json_object_get_string(jobj_string));
                printf("%s: %s = %s\n", __FUNCTION__, string_name, outbuf);
                /* Free obj */
                json_object_put(jobj_string);
            }
            else
            {
                printf("%s: Can't get string %s!\n", __FUNCTION__, string_name);
                ret = -1;
            }
            /* Free obj */
            json_object_put(jobj);
        }
    }
    else
    {
        printf("%s: NULL == query_str!\n", __FUNCTION__);
        ret = -1;
    }

    return ret;
}

/*****************************************************************
* NAME:    get_json_integer_from_query
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_integer_from_query(char *query_str, int *output, char *integer_name)
{
    struct json_object *jobj, *jobj_integer;
    int ret = 0;

    if(NULL != query_str)
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            if((jobj_integer = json_object_object_get(jobj, integer_name)))
            {
                ret = 1;
                *output = json_object_get_int(jobj_integer);
                printf("APP agent [%s] : %s = %d\n", __func__, integer_name, *output);
                /* Free obj */
                json_object_put(jobj_integer);
            }
            else
            {
                printf("APP agent [%s] : Can't get ingeter %s!\n", __func__, integer_name);
                ret = -1;
            }
            /* Free obj */
            json_object_put(jobj);
        }
    }
    else
    {
        printf("APP agent [%s] : NULL == query_str!\n", __func__);
        ret = -1;
    }

    return ret;
}

/*****************************************************************
* NAME:    simple_json_response
* ---------------------------------------------------------------
* FUNCTION:  JSON response packet format of setting functions
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int simple_json_response(HTTPS_CB *pkt, char *result)
{
    struct json_object *jobj;
    struct json_object *jstr_result;
    char buf[64];

    if(NULL == pkt)
    {
        return -1;
    }
    sprintf(buf, "%sResult", pkt->json_action);

    /* Construct the response packet */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);

    jobj = json_object_new_object();

    jstr_result = json_object_new_string(result);
    json_object_object_add(jobj, buf, jstr_result);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    // Note : Remove this when use send_simple_response().
    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jstr_result);
    json_object_put(jobj);
    return 0;
}

/*****************************************************************
* NAME:    check_app_client_reply
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool check_app_client_reply(char *payload, char *action)
{
    bool result;
    char buf_action[64];
    char buf_result[32];

    result = FALSE;
    memset(buf_action, 0x00, sizeof(buf_action));
    memset(buf_result, 0x00, sizeof(buf_result));

    sprintf(buf_action, "%sResult", action);

    get_json_string_from_query(payload, buf_result, buf_action);

    if(0 == strcmp(buf_result, OK_STR))
    {
        result = TRUE;
    }

    return result;
}

/*----------------------------- DEVICE SETTING -----------------------------*/

/*****************************************************************
* NAME:    login_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int login_json_cb(char *query_str, void *priv)
{
    int result = AGENT_LOGIN_FAIL;
#if HAS_LIMIT_APP_ACCOUNT_LOGIN
    int app_type = AGENT_APP_NONE;
#endif
    struct json_object *jobj, *jstr_username, *jstr_password;
    char username[31+1], password[31+1];

    jstr_username = NULL;
    jstr_password = NULL;

    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    if(query_str)
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            if((jstr_username = json_object_object_get(jobj, "AdminUsername")) &&
               (jstr_password = json_object_object_get(jobj, "AdminPassword")))
            {
                snprintf(username, sizeof(username), "%s", json_object_get_string(jstr_username));
                snprintf(password, sizeof(password), "%s", json_object_get_string(jstr_password));
                if (sysutil_check_account(username, password))
                {
                    APPAGENT_SYSTEM("printf %s > /tmp/user",username);
                    result = AGENT_LOGIN_OK;
                }
#if HAS_IPCAM
				else if (!strcmp("192.168.99.99_ipcam",username)
					&& !strcmp("ipcam",password)){
					result = AGENT_LOGIN_OK;
				}
#endif
                /* Free obj */
                if(jstr_username)
                {
                    json_object_put(jstr_username);
                }
                if(jstr_password)
                {
                    json_object_put(jstr_password);
                }
            }

#if HAS_LIMIT_APP_ACCOUNT_LOGIN
            if(AGENT_LOGIN_OK == result)
            {
                senao_json_object_get_integer(jobj, "APPType", &app_type);

                switch(app_type)
                {
                    case AGENT_APP_NONE:
                        break;
                    case AGENT_APP_ENMESH:
                        result = AGENT_LOGIN_OK_APP_ENMESH;
                        break;
                    default:
                        break;
                }
            }
#endif

            /* Free obj */
            json_object_put(jobj);
        }
    }

    return result;
}

/*****************************************************************
* NAME:    login_response_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
#if HAS_MESH_JSON && HAS_IPCAM
bool login_response_json_cb(HTTPS_CB *pkt)
{
	struct json_object *jobj, *jobj_response;
	struct json_object *jobj_support_function, *jobj_enable_function, *jobj_func_support, *jobj_func_enable;

	char *ptr = NULL;
	char model_name[128]={0}, fversion[15+1]={0}, uid[32]={0}, device_type[10]={0}, mac[32]={0}, response[1024]={0};
	bool mic=0, speaker=0, eventAction=0, scheduleAction=0, rtspAuth=0, timeLapse=0, upDownConfig=0, motion=0, privacy=0, dnr=0, mobile=0, rtspAuthEnable=0;
	int eventActionType=0, eventStorageDes=0, scheduleActionType=0, scheduleStorageDes=0, cloud=0;
	int appAgentService = AGENT_SUPPORT_HTTPS_ONLY;
    char apiversion[16]={0}, fw_tmp[16]={0}, fw_ver[32]={0},username[16]={0},authority[2]={0};
    char *fw_string;
    char xrelayd_ip[128];
	char ipcam_app_agent_port[16] = {0};

	if(NULL == pkt)
		return -1;

	jobj = json_object_new_object();

    sysutil_interact(username, sizeof(username), "cat /tmp/user");
    sysutil_interact(authority, sizeof(authority), "/lib/auth.sh get_authority %s",username);

    if (strcmp(authority,"0") == 0)  // Adminstrator
        ADD_JSON_OBJECT_NEW_STRING(jobj, "LoginResult", OK_STR);
    else                             // Guest Account
        ADD_JSON_OBJECT_NEW_STRING(jobj, "LoginResult", GUEST_STR);

	api_get_string_option(SYSPRODUCTINFO_MODEL_MODELNAME_OPTION, model_name, sizeof(model_name));
	ADD_JSON_OBJECT_NEW_STRING(jobj, "ModelName", model_name);

	sysutil_get_firmware_version_info(fversion, sizeof(fversion));
#if HAS_EG_AUTO_FW_CHECK
    int i=0;
    fw_string = strtok(fversion, ".");
    strcat(fw_ver,"v");

    while(NULL != fw_string)
    {
        if (i<2)
        {
            sprintf(fw_tmp,"%s.",fw_string);
            strcat(fw_ver,fw_tmp);
        }
        else if (i==2)
        {
            sprintf(fw_tmp,"%s",fw_string);
            strcat(fw_ver,fw_tmp);
        }
        i++;
        fw_string = strtok (NULL, ".");;
    }
    strcat(fw_ver,"-c");
    sysutil_interact(fw_tmp, sizeof(fw_tmp), "cat /etc/version_capwap");
    strcat(fw_ver,fw_tmp);

    // "FirmwareVersion":"v0.425.1534-c1.8.57" firmware_ver_capwap_ver
	ADD_JSON_OBJECT_NEW_STRING(jobj, "FirmwareVersion", fw_ver);
#else
    ADD_JSON_OBJECT_NEW_STRING(jobj, "FirmwareVersion", fversion);
#endif

	sysutil_interact(uid, sizeof(uid), "cat /etc/UID.conf");
	if(strlen(uid)==1)
	{
		strcpy(uid,"0000000");
		uid[strlen(uid)]='\0';
	}
	if(NULL != (ptr = strchr(uid, '\n')))
		*ptr = '\0';
	ADD_JSON_OBJECT_NEW_STRING(jobj, "UID", uid);

	sysutil_interact(device_type, sizeof(device_type), "setconfig -g 5");
	device_type[strcspn(device_type, "\n")] = '\0';
	device_type[strcspn(device_type, " ")] = '\0';
	ADD_JSON_OBJECT_NEW_STRING(jobj, "DeviceType", device_type);

#if SUPPORT_WLAN_5G_SETTING
	ADD_JSON_OBJECT_NEW_INT(jobj, "WirelessCapability", 2);
#else
	ADD_JSON_OBJECT_NEW_INT(jobj, "WirelessCapability", 1);
#endif

	ADD_JSON_OBJECT_NEW_INT(jobj, "MeshNetworkStatus", 0);

	ADD_JSON_OBJECT_NEW_STRING(jobj, "MeshAdminUsername", "Mesh_Engenius_5566");

	ADD_JSON_OBJECT_NEW_STRING(jobj, "MeshAdminPassword", "Mesh_passwd");

	ADD_JSON_OBJECT_NEW_STRING(jobj, "2.4GHzSupportedChannelList", "1,2,3,4,5,6,7,8,9,10,11");

	ADD_JSON_OBJECT_NEW_STRING(jobj, "5GHzSupportedChannelList", "36,40,44,48,149,153,157,161");

    if (strcmp(authority,"0") == 0)  // Adminstrator
        ADD_JSON_OBJECT_NEW_STRING(jobj, "Privilege", ADMINISTRATOR_USER_STR);
    else                             // Guest Account
        ADD_JSON_OBJECT_NEW_STRING(jobj, "Privilege", GUEST_USER_STR);

	ADD_JSON_OBJECT_NEW_STRING(jobj, "Token", "");

	sysutil_interact(mac, sizeof(mac), "ifconfig br-lan | grep \"HWaddr\" | awk \'{print $5}\'");
	if(NULL != (ptr = strchr(mac, '\n')))
		*ptr = '\0';
	ADD_JSON_OBJECT_NEW_STRING(jobj, "MacAddress", mac);

#if HAS_WLAN_DONGLE
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "WlanSupport", TRUE);
#else
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "WlanSupport", FALSE);
#endif

	api_get_string_option("xrelayd.xrelayd.conn_sec_ip", xrelayd_ip, sizeof(xrelayd_ip));
	api_get_string_option("xrelayd.xrelayd.ipcam_app_agent_port", ipcam_app_agent_port, sizeof(ipcam_app_agent_port));

	if(ipcam_app_agent_port[0] == '\0')
		sysutil_interact(response, sizeof(response), "app_client -i %s -m POST -a Login -e 1 -p \'{\"AdminUsername\":\"senao_mesh_ipcam\",\"AdminPassword\":\"ipcam\"}\'", xrelayd_ip);
	else
		sysutil_interact(response, sizeof(response), "app_client -i %s -m POST -a Login -e 1 -P %s -p \'{\"AdminUsername\":\"senao_mesh_ipcam\",\"AdminPassword\":\"ipcam\"}\'", xrelayd_ip, ipcam_app_agent_port);
	if(NULL != (ptr = strchr(response, '\n')))
		*ptr = '\0';
	//printf("app_client response: %s", response);
    get_json_string_from_query(response, apiversion, "APIVersion");
    ADD_JSON_OBJECT_NEW_STRING(jobj, "APIVersion", apiversion);

	get_json_boolean_from_query(response, &mic, "MicSupport");
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "MicSupport", mic);

	get_json_boolean_from_query(response, &speaker, "SpeakerSupport");
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "SpeakerSupport", speaker);

	//0: HTTPS only
	//1: HTTP with JSON encryption and HTTPS
	//2: HTTP without JSON encryption and HTTPS (Reserve)
	api_get_bool_option("app_agent.agent.enable_aes", &appAgentService);
	ADD_JSON_OBJECT_NEW_INT(jobj, "AgentCapability", appAgentService);

	get_json_boolean_from_query(response, &eventAction, "EventActionEnable");
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "EventActionEnable", eventAction);

	get_json_integer_from_query(response, &eventActionType, "EventActionType");
	ADD_JSON_OBJECT_NEW_INT(jobj, "EventActionType", eventActionType);

	get_json_integer_from_query(response, &eventStorageDes, "EventStorageDestination");
	ADD_JSON_OBJECT_NEW_INT(jobj, "EventStorageDestination", eventStorageDes);

	get_json_boolean_from_query(response, &scheduleAction, "ScheduleActionEnable");
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "ScheduleActionEnable", scheduleAction);

	get_json_integer_from_query(response, &scheduleActionType, "ScheduleActionType");
	ADD_JSON_OBJECT_NEW_INT(jobj, "ScheduleActionType", scheduleActionType);

	get_json_integer_from_query(response, &scheduleStorageDes, "ScheduleStorageDestination");
	ADD_JSON_OBJECT_NEW_INT(jobj, "ScheduleStorageDestination", scheduleStorageDes);

	jobj_response = json_tokener_parse(response);
	jobj_support_function = json_object_object_get(jobj_response,"APPSupportedFunction");
	jobj_func_support = json_object_new_object();

	senao_json_object_get_boolean(jobj_support_function, "RtspAuthorization", &rtspAuth);
	ADD_JSON_OBJECT_NEW_BOOL(jobj_func_support, "RtspAuthorization", rtspAuth);

	senao_json_object_get_boolean(jobj_support_function, "TimeLapse", &timeLapse);
	ADD_JSON_OBJECT_NEW_BOOL(jobj_func_support, "TimeLapse", timeLapse);

	senao_json_object_get_boolean(jobj_support_function, "UpDownConfigFile", &upDownConfig);
	ADD_JSON_OBJECT_NEW_BOOL(jobj_func_support, "UpDownConfigFile", upDownConfig);

	senao_json_object_get_boolean(jobj_support_function, "MotionDetection", &motion);
	ADD_JSON_OBJECT_NEW_BOOL(jobj_func_support, "MotionDetection", motion);

	senao_json_object_get_boolean(jobj_support_function, "PrivacyMask", &privacy);
	ADD_JSON_OBJECT_NEW_BOOL(jobj_func_support, "PrivacyMask", privacy);

	senao_json_object_get_boolean(jobj_support_function, "3DNR", &dnr);
	ADD_JSON_OBJECT_NEW_BOOL(jobj_func_support, "3DNR", dnr);

	senao_json_object_get_boolean(jobj_support_function, "MobileController", &mobile);
	ADD_JSON_OBJECT_NEW_BOOL(jobj_func_support, "MobileController", mobile);

	json_object_object_add(jobj, "APPSupportedFunction", jobj_func_support);

	jobj_enable_function = json_object_object_get(jobj_response,"EnabledFunction");
	jobj_func_enable = json_object_new_object();

	senao_json_object_get_boolean(jobj_enable_function, "RtspAuthorization", &rtspAuthEnable);
	ADD_JSON_OBJECT_NEW_BOOL(jobj_func_enable, "RtspAuthorization", rtspAuthEnable);

	senao_json_object_get_integer(jobj_enable_function, "CloudService", &cloud);
	ADD_JSON_OBJECT_NEW_INT(jobj_func_enable, "CloudService", cloud);

	json_object_object_add(jobj, "EnabledFunction", jobj_func_enable);


    /* Store packet content into buffer and send it out */
	http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
	http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
	http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

	http_send_stored_data(pkt->fd);

	/* Free obj */
	json_object_put(jobj);
	json_object_put(jobj_response);
	json_object_put(jobj_support_function);
	json_object_put(jobj_enable_function);
	json_object_put(jobj_func_support);
	json_object_put(jobj_func_enable);

	return 0;
}
#elif HAS_MESH_JSON
bool login_response_json_cb(HTTPS_CB *pkt)
{
	struct json_object *jobj;
	struct json_object *jstr_result;
//    int wifi0=0;
//    int wifi1=0;
//    int wifi0_guest=0;
//    int wifi1_guest=0;
//    int active=0;
//    int active_number=0;
    int channel=0;
    int country=0;
    int domain=0;
    int server_type=0;
    char buf[8]={0};
    char api_result[64]={0};
    char mesh_key[64]={0};
    char mesh_id[64]={0};
    char mesh_role[32]={0};
    char mesh_controller[32]={0};
    char model_name[128]={0};
    char UID[32]={0};
    char fw_version[16]={0};

    if(NULL == pkt)
		return -1;
    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string("OK"));

//    if (api_get_integer_option("wireless.wifi0_ssid_1.disabled", &wifi0))
//    {
//        wifi0 = 1;
//    }
//
//    if (api_get_integer_option("wireless.wifi1_ssid_1.disabled", &wifi1))
//    {
//        wifi1 = 1;
//    }
//
//    if (api_get_integer_option("wireless.wifi0_guest.disabled", &wifi0_guest))
//    {
//        wifi0_guest = 1;
//    }
//
//    if (api_get_integer_option("wireless.wifi1_guest.disabled", &wifi1_guest))
//    {
//        wifi1_guest = 1;
//    }
	api_get_string_option("wireless.wifi1_mesh.Mesh_id", mesh_id, sizeof(mesh_id));
    api_get_string_option("wireless.wifi1_mesh.aeskey", mesh_key, sizeof(mesh_key));
    api_get_string_option("mesh.wifi.role", mesh_role, sizeof(mesh_role));
    if(api_get_string_option("mesh.wifi.controller", mesh_controller, sizeof(mesh_controller)))
    {
        sprintf(mesh_controller, "slave");
    }
    sysutil_interact(UID, sizeof(UID), "setconfig -g 35 | tr -d '\n'");

#if SUPPORT_CAPWAP_CONTROL
    memset(fw_version, 0, sizeof (fw_version));
    sys_get_firmware_version_info(fw_version, 5);     //ignore the last number
#else
    sysutil_interact(fw_version, sizeof(fw_version), "cat /etc/version | grep Firmware | awk '{print $4}' | tr -d '\n'");
#endif

    //batctl o -H |grep "no batman" | wc -l
    //batctl o -H |grep "No batman *" | wc -l
//    sysutil_interact(buf, sizeof(buf), "batctl o -H |grep \"No batman *\" | wc -l | tr -d '\n'");
//    active=atoi(buf);
//    sysutil_interact(buf, sizeof(buf), "batctl o -H | wc -l | tr -d '\n'");
//    active_number=atoi(buf);
//
//    if (wifi0 ==1 && wifi1 == 1)
//    {
//        json_object_object_add(jobj, "WirelessStatus", json_object_new_string("0"));
//    }
//    else
//    {
//        json_object_object_add(jobj, "WirelessStatus", json_object_new_string("1"));
//    }
//
//    if (wifi0_guest == 1 && wifi1_guest == 1)
//    {
//        json_object_object_add(jobj, "WirelessGuestNetworkStatus", json_object_new_string("0"));
//    }
//    else
//    {
//        json_object_object_add(jobj, "WirelessGuestNetworkStatus", json_object_new_string("1"));
//    }
//
//    if (active==0)
//    {
//        json_object_object_add(jobj, "RoomsActiveNumber", json_object_new_int(active_number+1));
//    }
//    else
//    {
//        json_object_object_add(jobj, "RoomsActiveNumber", json_object_new_int(1));
//    }

    api_get_string_option("wireless.wifi1.channel", buf, sizeof(buf));
    json_object_object_add(jobj, "WiFiChannel", json_object_new_int(atoi(buf)));

    api_get_integer_option("wireless.wifi1.country", &country);
    json_object_object_add(jobj, "CountryCode", json_object_new_int(country));

    sysutil_interact(buf, sizeof(buf), "setconfig -g 4 | tr -d '\n'");
    domain=atoi(buf);
    json_object_object_add(jobj, "Domain", json_object_new_int(domain));

    if(0 == api_get_integer_option("senao-openapi-server.server.enable", &server_type))
    {
        json_object_object_add(jobj, "srv_type", json_object_new_int(server_type));
    }

//    APPAGENT_SYSTEM("lua /usr/lib/lua/luci/people.lua refresh");
//
//    sysutil_interact(buf, sizeof(buf), "cat /tmp/people_active");
//
//    json_object_object_add(jobj, "PeopleActiveNumber", json_object_new_string(buf));

    json_object_object_add(jobj, "MeshKey", json_object_new_string(mesh_key));
    json_object_object_add(jobj, "MeshID", json_object_new_string(mesh_id));
    json_object_object_add(jobj, "ModelType", json_object_new_int(1));
    json_object_object_add(jobj, "MeshRole", json_object_new_string(mesh_role));
    json_object_object_add(jobj, "MeshController", json_object_new_string(mesh_controller));

    api_get_string_option(SYSPRODUCTINFO_MODEL_MODELNAME_OPTION, model_name, sizeof(model_name));
    json_object_object_add(jobj, "ModelName", json_object_new_string(model_name));
    json_object_object_add(jobj, "UID", json_object_new_string(UID));
    json_object_object_add(jobj, "FirmwareVersion", json_object_new_string(fw_version));

	/* Store packet content into buffer and send it out */
	http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
	http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
	http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

	http_send_stored_data(pkt->fd);

	json_object_put(jobj);

    //if has 'rooms_list.lua', update mesh history config table
    if(sysutil_check_file_existed("/usr/lib/lua/luci/rooms_list.lua"))
    {
        system("lua /usr/lib/lua/luci/rooms_list.lua &");
    }

	return 0;
}
#elif HAS_IPCAM
bool login_response_json_cb(HTTPS_CB *pkt, int login_result)
{
	struct json_object *jobj;
	struct json_object *jobj_tmp;
	struct json_object *jobj_minor;
	struct json_object *jstr_result;
	struct json_object *jobj_func_support;
	struct json_object *jbool_func_support;
	struct json_object *jstr_privilege, *jstr_token;
	struct json_object *jstr_mac, *jstr_model_name;
	struct json_object *jstr_uid;
	struct json_object *jstr_fw_version;
	struct json_object *jobj_app_support_function;
	struct json_object *jobj_enabled_function;
	bool mic=0,speaker=0,eventaction=0,scheduleactionenable=0,rtspauthorization=0,timelapse=0,updownconfigfile=0,en_rtspauthorization=0, motion=0, privacy=0, dnr=0, mobile=0, rtspAuthEnable=0;
	int eventactiontype=0,eventstoragedestination=0,scheduleactiontype=0,schedulestoragedestination=0,cloud=0;
	char username[31+1],model_name[64], mac[32], uid[32],fversion[15+1],response[1024];
	char *result = NULL, *privilege = NULL,*ptr = NULL,*str_ptr=NULL;
	char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
	char xrelayd_ip[128];
#if HAS_SSL
	int appAgentService=AGENT_SUPPORT_HTTPS_ONLY;
#endif

	if(NULL == pkt)
		return -1;
	
	memset(username, 0, sizeof(username));
	get_json_string_from_query(query_str, username, "AdminUsername");
	if(!strcmp(username,"192.168.99.99_ipcam"))
	{
		send_simple_response(pkt,OK_STR);
		return 0;
	}
	privilege = ADMINISTRATOR_USER_STR;
	result = OK_STR;
	memset(response, 0, sizeof(response));
	api_get_string_option("xrelayd.xrelayd.conn_sec_ip", xrelayd_ip, sizeof(xrelayd_ip));
	sysutil_interact(response, sizeof(response), "app_client -i %s -m POST -a Login -e 1 -p \'{\"AdminUsername\":\"senao_mesh_ipcam\",\"AdminPassword\":\"ipcam\"}\'", xrelayd_ip);
	if(NULL != (ptr = strchr(response, '\n')))
		*ptr = '\0';
//printf(" *** response(%d)=[%s]",strlen(response),response);

	/* Construct the packet content in json format. */
	jobj = json_object_new_object();

	jstr_result = json_object_new_string("OK");
	json_object_object_add(jobj, "LoginResult", jstr_result);

	jstr_privilege = json_object_new_string(privilege);
	json_object_object_add(jobj, "Privilege", jstr_privilege);
    
	jstr_token = json_object_new_string("");
	json_object_object_add(jobj, "Token", jstr_token);
   
	memset(mac,0x0,sizeof(mac));
	sysutil_interact(mac, sizeof(mac), "ifconfig br-lan | grep \"HWaddr\" | awk \'{print $5}\'"); 
	if(NULL != (ptr = strchr(mac, '\n')))
		*ptr = '\0';
        jstr_mac = json_object_new_string(mac);
        json_object_object_add(jobj, "MacAddress", jstr_mac);
    
	memset(model_name,0x0,sizeof(model_name));
	api_get_string_option(SYSPRODUCTINFO_MODEL_MODELNAME_OPTION,model_name,sizeof(model_name));
	jstr_model_name = json_object_new_string(model_name);
	json_object_object_add(jobj, "ModelName", jstr_model_name);
 
	memset(uid, 0x00, sizeof(uid));
	sysutil_interact(uid, sizeof(uid), "cat /etc/UID.conf");
	if(strlen(uid)==1){
		strcpy(uid,"0000000");
		uid[strlen(uid)]='\0';
	}
	if(NULL != (ptr = strchr(uid, '\n')))
		*ptr = '\0';
	jstr_uid = json_object_new_string(uid);
	json_object_object_add(jobj, "UID", jstr_uid);
	
#if HAS_WLAN_DONGLE
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "WlanSupport", TRUE);
#else
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "WlanSupport", FALSE);
#endif

	get_json_boolean_from_query(response, &mic, "MicSupport");
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "MicSupport",mic);

	get_json_boolean_from_query(response, &speaker, "SpeakerSupport");
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "SpeakerSupport",speaker);

	//0: HTTPS only
	//1: HTTP with JSON encryption and HTTPS
	//2: HTTP without JSON encryption and HTTPS (Reserve) 
	api_get_bool_option("app_agent.agent.enable_aes",&appAgentService);
	ADD_JSON_OBJECT_NEW_INT(jobj, "AgentCapability", appAgentService);

	memset(fversion,0x0,sizeof(fversion));
	sysutil_get_firmware_version_info(fversion, sizeof(fversion));
	jstr_fw_version = json_object_new_string(fversion);
	json_object_object_add(jobj, "FirmwareVersion", jstr_fw_version);

	get_json_boolean_from_query(response, &eventaction, "EventActionEnable");
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "EventActionEnable",eventaction);

	get_json_integer_from_query(response, &eventactiontype, "EventActionType");
	ADD_JSON_OBJECT_NEW_INT(jobj, "EventActionType",eventactiontype);

	get_json_integer_from_query(response, &eventstoragedestination, "EventStorageDestination");
	ADD_JSON_OBJECT_NEW_INT(jobj, "EventStorageDestination",eventstoragedestination);

	get_json_boolean_from_query(response, &scheduleactionenable, "ScheduleActionEnable");
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "ScheduleActionEnable",scheduleactionenable);

	get_json_integer_from_query(response, &scheduleactiontype, "ScheduleActionType");
	ADD_JSON_OBJECT_NEW_INT(jobj, "ScheduleActionType",scheduleactiontype);

	get_json_integer_from_query(response, &schedulestoragedestination, "ScheduleStorageDestination");
	ADD_JSON_OBJECT_NEW_INT(jobj, "ScheduleStorageDestination",schedulestoragedestination);

	jobj_tmp = json_tokener_parse(response);
	jobj_app_support_function = json_object_object_get(jobj_tmp,"APPSupportedFunction");
	senao_json_object_get_boolean(jobj_app_support_function, "RtspAuthorization",&rtspauthorization);
	senao_json_object_get_boolean(jobj_app_support_function, "TimeLapse",&timelapse);
	senao_json_object_get_boolean(jobj_app_support_function, "UpDownConfigFile",&updownconfigfile);
	senao_json_object_get_boolean(jobj_app_support_function, "MotionDetection", &motion);
	senao_json_object_get_boolean(jobj_app_support_function, "PrivacyMask", &privacy);
	senao_json_object_get_boolean(jobj_app_support_function, "3DNR", &dnr);
	senao_json_object_get_boolean(jobj_app_support_function, "MobileController", &mobile);
	jobj_minor = json_object_new_object();
	json_object_object_add(jobj_minor, "RtspAuthorization",json_object_new_boolean(rtspauthorization));
	json_object_object_add(jobj_minor, "TimeLapse",json_object_new_boolean(timelapse));
	json_object_object_add(jobj_minor, "UpDownConfigFile",json_object_new_boolean(updownconfigfile));
	ADD_JSON_OBJECT_NEW_BOOL(jobj_minor, "MotionDetection", motion);
	ADD_JSON_OBJECT_NEW_BOOL(jobj_minor, "PrivacyMask", privacy);
	ADD_JSON_OBJECT_NEW_BOOL(jobj_minor, "3DNR", dnr);
	ADD_JSON_OBJECT_NEW_BOOL(jobj_minor, "MobileController", mobile);
	json_object_object_add(jobj, "APPSupportedFunction", jobj_minor);

	jobj_enabled_function = json_object_object_get(jobj_tmp,"EnabledFunction");
	senao_json_object_get_boolean(jobj_enabled_function, "RtspAuthorization",&en_rtspauthorization);
	senao_json_object_get_integer(jobj_enabled_function, "CloudService", &cloud);
	jobj_minor = json_object_new_object();
	json_object_object_add(jobj_minor, "RtspAuthorization",json_object_new_boolean(en_rtspauthorization));
	ADD_JSON_OBJECT_NEW_INT(jobj_minor, "CloudService", cloud);
	json_object_object_add(jobj, "EnabledFunction", jobj_minor);

	/* Store packet content into buffer and send it out */
	http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
	http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
	http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));
	http_send_stored_data(pkt->fd);

	/* Free obj */
	json_object_put(jstr_result);
	json_object_put(jobj_func_support);
	json_object_put(jbool_func_support);
	json_object_put(jstr_privilege);
	json_object_put(jstr_token);
	json_object_put(jstr_mac);
	json_object_put(jstr_model_name);
	json_object_put(jstr_uid);
	json_object_put(jobj);

	return 0;
}
#endif

#if HAS_IPCAM
int get_device_status_cb(HTTPS_CB *pkt)
{
	struct json_object *jobj;
	bool mic=0,speaker=0,eventaction=0,scheduleactionenable=0;
	int eventactiontype=0,eventstoragedestination=0,scheduleactiontype=0,schedulestoragedestination=0;
	char username[31+1],model_name[64],wanMac[32], uid[32],fversion[15+1],response[1024],wantype[10],wanIP[32],wanMask[32],productSN[20],opmode[64];
	char *result = NULL,*ptr = NULL,*str_ptr=NULL;
	char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
	char xrelayd_ip[128];
	char ipcam_app_agent_port[16] = {0};
#if HAS_SSL
	int appAgentService=AGENT_SUPPORT_HTTPS_ONLY;
#endif

	if(NULL == pkt)
		return -1;
	
	memset(username, 0, sizeof(username));
	get_json_string_from_query(query_str, username, "AdminUsername");

	if(!strcmp(username,"192.168.99.99_ipcam"))
	{
		send_simple_response(pkt,OK_STR);
		return 0;
	}
	result = OK_STR;
	memset(response, 0, sizeof(response));
	api_get_string_option("xrelayd.xrelayd.conn_sec_ip", xrelayd_ip, sizeof(xrelayd_ip));
	api_get_string_option("xrelayd.xrelayd.ipcam_app_agent_port", ipcam_app_agent_port, sizeof(ipcam_app_agent_port));
	if(ipcam_app_agent_port[0] == '\0')
		sysutil_interact(response, sizeof(response), "app_client -i %s -m POST -a Login -e 1 -p \'{\"AdminUsername\":\"senao_mesh_ipcam\",\"AdminPassword\":\"ipcam\"}\'", xrelayd_ip);
	else
		sysutil_interact(response, sizeof(response), "app_client -i %s -m POST -a Login -e 1 -P %s -p \'{\"AdminUsername\":\"senao_mesh_ipcam\",\"AdminPassword\":\"ipcam\"}\'", xrelayd_ip, ipcam_app_agent_port);
	if(NULL != (ptr = strchr(response, '\n')))
		*ptr = '\0';
	//printf(" *** response(%d)=[%s]",strlen(response),response);

	/* Construct the packet content in json format. */
	jobj = json_object_new_object();

	json_object_object_add(jobj, "GetDeviceStatusResult",json_object_new_string("OK"));

	memset(model_name,0x0,sizeof(model_name));
	api_get_string_option(SYSPRODUCTINFO_MODEL_MODELNAME_OPTION,model_name,sizeof(model_name));
	json_object_object_add(jobj, "ModelName",json_object_new_string(model_name));

	memset(productSN,0x0,sizeof(productSN));
        sysutil_interact(productSN, sizeof(productSN),"setconfig -g 0"); 
        productSN[strcspn(productSN, "\n") - 1] = '\0';
	json_object_object_add(jobj, "ProductSN",json_object_new_string(productSN));

	memset(fversion,0x0,sizeof(fversion));
	sysutil_get_firmware_version_info(fversion, sizeof(fversion));
	json_object_object_add(jobj, "FirmwareVersion",json_object_new_string(fversion));

        memset(opmode,0x0,sizeof(opmode));
        sysutil_interact(opmode, sizeof(opmode),"opmode.sh r | tail -n 1"); 
        opmode[strcspn(opmode, "\n") - 1] = '\0';
	json_object_object_add(jobj, "OperationMode",json_object_new_int(atoi(opmode)));

	memset(wantype,0x0,sizeof(wantype));
	api_get_string_option(NETWORK_LAN_PROTO_OPTION,wantype,sizeof(wantype));
	json_object_object_add(jobj, "WanType",json_object_new_string(wantype));

	memset(wanIP,0x0,sizeof(wanIP));
	sysutil_get_lan_ipaddr(wanIP, sizeof(wanIP)); 
	json_object_object_add(jobj, "WanIPAddress",json_object_new_string(wanIP));

	memset(wanMask,0x0,sizeof(wanMask));
	sysutil_get_lan_netmask(wanMask, sizeof(wanMask));
        json_object_object_add(jobj, "WanSubnetMask",json_object_new_string(wanMask));
  
	memset(wanMac,0x0,sizeof(wanMac));
	sysutil_interact(wanMac, sizeof(wanMac), "ifconfig br-lan | grep \"HWaddr\" | awk \'{print $5}\'"); 
	if(NULL != (ptr = strchr(wanMac, '\n')))
		*ptr = '\0';
	json_object_object_add(jobj, "WanMacAddress",json_object_new_string(wanMac));
		
#if HAS_WLAN_DONGLE
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "WlanSupport", TRUE);
#else
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "WlanSupport", FALSE);
#endif

	get_json_boolean_from_query(response, &mic, "MicSupport");
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "MicSupport",mic);

	get_json_boolean_from_query(response, &speaker, "SpeakerSupport");
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "SpeakerSupport",speaker);

	get_json_boolean_from_query(response, &eventaction, "EventActionEnable");
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "EventActionEnable",eventaction);

	get_json_integer_from_query(response, &eventactiontype, "EventActionType");
	ADD_JSON_OBJECT_NEW_INT(jobj, "EventActionType",eventactiontype);

	get_json_integer_from_query(response, &eventstoragedestination, "EventStorageDestination");
	ADD_JSON_OBJECT_NEW_INT(jobj, "EventStorageDestination",eventstoragedestination);

	get_json_boolean_from_query(response, &scheduleactionenable, "ScheduleActionEnable");
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "ScheduleActionEnable",scheduleactionenable);

	get_json_integer_from_query(response, &scheduleactiontype, "ScheduleActionType");
	ADD_JSON_OBJECT_NEW_INT(jobj, "ScheduleActionType",scheduleactiontype);

	get_json_integer_from_query(response, &schedulestoragedestination, "ScheduleStorageDestination");
	ADD_JSON_OBJECT_NEW_INT(jobj, "ScheduleStorageDestination",schedulestoragedestination);

	/* Store packet content into buffer and send it out */
	http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
	http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
	http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));
	http_send_stored_data(pkt->fd);

	/* Free obj */
	json_object_put(jobj);
	return 0;
}

int set_onvif_discovery_mode_cb(HTTPS_CB *pkt)
{
        char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
        bool result = FALSE,discovery=0;
        struct json_object *jobj;

        if(NULL == pkt)
                return -1;
        
        if(query_str)
        {
                if(jobj = json_tokener_parse(query_str))
                {
                        if(senao_json_object_get_boolean(jobj, "DiscoveryMode",&discovery))
			{
                                result = TRUE;
				api_set_bool_option("wsdiscovery.wsdiscovery.onvif_discovery_mode",discovery);
				system("uci commit");
			}
                }
        }
 
send_pkt:
        send_simple_response(pkt, (TRUE == result)?OK_STR:ERROR_STR);
        
        return 0;
}

int set_onvif_scopes_cb(HTTPS_CB *pkt)
{
        char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
        char option_name[64]={0},option_value[64]={0},name[20]={0};
        bool result = FALSE,discovery=0;
        int index=0;
        struct json_object *jobj;

        if(NULL == pkt)
                return -1;
        
        if(query_str)
        {
                if((jobj = json_tokener_parse(query_str)))
                {
                        for(index=1;index < 11; index++ ){
                        memset(option_name,0x0,sizeof (option_name));
                        memset(option_value,0x0,sizeof (option_value));
			memset(name,0x0,sizeof(name));
                        
                        sprintf(name,"Scopes_%02d",index);
                        if(senao_json_object_get_string(jobj,name,option_value)){
                                result = TRUE;
				sprintf(option_name,"wsdiscovery.wsdiscovery.onvif_scopes_%02d",index);
                                if(strlen(option_value))
                                        api_set_string_option(option_name,option_value,sizeof(option_value));
				else
					api_delete_option(option_name, "");
                        }
//printf("\n *** option (%s=%s,%s)\n",option_name,option_value,name);
                        }
                }
        }
        system("uci commit");
        sleep(1);
        system("wsdiscovery 1");
        
send_pkt:
        send_simple_response(pkt, (TRUE == result)?OK_STR:ERROR_STR);
        
        return 0;
}

#endif

/*****************************************************************
* NAME:    change_login_pw_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int change_login_pw_json_cb(char *query_str, void *priv)
{
    bool result, is_jobj;
    struct json_object *jobj, *jobj_oldpw, *jobj_newpw;
    char oldpw[127+1], newpw[127+1];

    result = TRUE;
    is_jobj = FALSE;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_oldpw = json_object_object_get(jobj, "OldPw")))
            {
                if(strlen(json_object_get_string(jobj_oldpw)) < PASSWORD_MIN_SIZE || strlen(json_object_get_string(jobj_oldpw)) > PASSWORD_MAX_SIZE)
                {
                    result = FALSE;
                    strcpy((char *)priv, "ERROR_OLD_PW");
                    goto out;
                }
                sprintf(oldpw, "%s", json_object_get_string(jobj_oldpw));

                /* Free obj */
                json_object_put(jobj_oldpw);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_newpw = json_object_object_get(jobj, "NewPw")))
            {
                if(strlen(json_object_get_string(jobj_newpw)) < PASSWORD_MIN_SIZE || strlen(json_object_get_string(jobj_newpw)) > PASSWORD_MAX_SIZE)
                {
                    result = FALSE;
                    strcpy((char *)priv, "ERROR_NEW_PW");
                    goto out;
                }
                sprintf(newpw, "%s", json_object_get_string(jobj_newpw));

                /* Free obj */
                json_object_put(jobj_newpw);
            }
            else
            {
                result = FALSE;
                goto out;
            }
            if (!sysutil_check_account("admin", oldpw))
            {
                result = FALSE;
                goto out;
            }
            if (sysutil_change_account("admin", newpw) < 0)
            {
                result = FALSE;
                goto out;
            }
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
* NAME:    get_device_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_device_settings_json_cb(HTTPS_CB *pkt, DEVICE_SETTING_T *setting, char *result)
{
    struct json_object *jobj;
    struct json_object *jstr_result, *jstr_vendor_name, *jstr_model_name;
    struct json_object *jstr_model_desc, *jstr_product_sn, *jstr_hw_version;
    struct json_object *jstr_lan_mac, *jstr_wlan_mac;
#if SUPPORT_WAN_SETTING
    struct json_object *jstr_wan_mac;
#endif

    if(NULL == pkt)
    {
        return -1;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string(result);
    json_object_object_add(jobj, "GetDeviceSettingsResult", jstr_result);

    jstr_vendor_name = json_object_new_string(setting->vendor_name);
    json_object_object_add(jobj, "VendorName", jstr_vendor_name);

    jstr_model_name = json_object_new_string(setting->model_name);
    json_object_object_add(jobj, "ModelName", jstr_model_name);

    jstr_model_desc = json_object_new_string(setting->model_description);
    json_object_object_add(jobj, "ModelDescription", jstr_model_desc);

    jstr_product_sn = json_object_new_string(setting->product_sn);
    json_object_object_add(jobj, "ProductSN", jstr_product_sn);

    jstr_hw_version = json_object_new_string(setting->hardware_version);
    json_object_object_add(jobj, "HardwareVersion", jstr_hw_version);
#if SUPPORT_WAN_SETTING
    jstr_wan_mac = json_object_new_string(setting->wan_mac);
    json_object_object_add(jobj, "WanMacAddress", jstr_wan_mac);
#endif
    jstr_lan_mac = json_object_new_string(setting->lan_mac);
    json_object_object_add(jobj, "LanMacAddress", jstr_lan_mac);

    jstr_wlan_mac = json_object_new_string(setting->wlan_mac);
    json_object_object_add(jobj, "WlanMacAddress", jstr_wlan_mac);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jstr_result);
    json_object_put(jstr_vendor_name);
    json_object_put(jstr_model_name);
    json_object_put(jstr_model_desc);
    json_object_put(jstr_product_sn);
    json_object_put(jstr_hw_version);
#if SUPPORT_WAN_SETTING
    json_object_put(jstr_wan_mac);
#endif
    json_object_put(jstr_lan_mac);
    json_object_put(jstr_wlan_mac);
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    get_system_information_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_system_information_json_cb(HTTPS_CB *pkt, SYSTEM_INFO_T *setting, char *result)
{
    struct json_object *jobj;
    struct json_object *jstr_result, *jstr_fw_version, *jstr_fw_version_gui, *jstr_fw_date;
    struct json_object *jstr_local_time, *jstr_uptime;
    struct json_object *jstr_http, *jstr_https;
    if(NULL == pkt)
    {
        return -1;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string(result);
    json_object_object_add(jobj, "GetSystemInformationResult", jstr_result);

    jstr_fw_version = json_object_new_string(setting->firmware_version);
    json_object_object_add(jobj, "FirmwareVersion", jstr_fw_version);

    jstr_fw_version_gui = json_object_new_string(setting->firmware_version);
    json_object_object_add(jobj, "FirmwareVersionGui", jstr_fw_version_gui);

    jstr_fw_date = json_object_new_string(setting->build_date);
    json_object_object_add(jobj, "FirmwareDate", jstr_fw_date);

    jstr_local_time = json_object_new_string(setting->local_time);
    json_object_object_add(jobj, "Time", jstr_local_time);

    jstr_uptime = json_object_new_string(setting->uptime);
    json_object_object_add(jobj, "UpTime", jstr_uptime);

    jstr_http = json_object_new_string("true");
    json_object_object_add(jobj, "SupportHttp", jstr_http);  // not do
    jstr_https = json_object_new_string("true");
        json_object_object_add(jobj, "SupportHttps", jstr_https);


    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jstr_result);
    json_object_put(jstr_fw_version);
    json_object_put(jstr_fw_version_gui);
    json_object_put(jstr_fw_date);
    json_object_put(jstr_local_time);
    json_object_put(jstr_uptime);
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    download_device_config_file_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int download_device_config_file_json_cb(HTTPS_CB *pkt, char *fileName, char *result)
{
    struct json_object *jobj;
    struct json_object *jstr_result, *jstr_config_path;

    if(NULL == pkt)
    {
        return -1;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string(result);
    json_object_object_add(jobj, "DownloadDeviceConfigFileResult", jstr_result);

    jstr_config_path = json_object_new_string(fileName);
    json_object_object_add(jobj, "ConfigFilePath", jstr_config_path);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jstr_result);
    json_object_put(jstr_config_path);
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    parse_upgrade_fw_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool parse_upgrade_fw_json_cb(char *query_str, UPGRADE_FW *settings)
{
    bool result, is_jobj;
    struct json_object *jobj, *jobj_action, *jobj_url;

    result = TRUE;
    is_jobj = FALSE;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_action = json_object_object_get(jobj, "Action")))
            {
                sprintf(settings->action, "%s", json_object_get_string(jobj_action));

                /* Free obj */
                json_object_put(jobj_action);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_url = json_object_object_get(jobj, "Url")))
            {
                if(strlen(json_object_get_string(jobj_url)) >= sizeof(settings->url))
                {
                    result = FALSE;
                    goto out;
                }

                sprintf(settings->url, "%s", json_object_get_string(jobj_url));

                /* Free obj */
                json_object_put(jobj_url);
            }
            else
            {
                result = FALSE;
                goto out;
            }
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
* NAME:    get_fw_release_info_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_fw_release_info_json_cb(HTTPS_CB *pkt, char *setting, char *result)
{
    struct json_object *jobj, *jobj_available, *jobj_setting, *jobj_data;

	int verCheck = 0;
	char fversion[16]={0};


    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    ADD_JSON_OBJECT_NEW_STRING(jobj, "GetFirmwareReleaseInfoResult", result);

	api_get_integer_option(SYSTEM_FIRMWARE_VERSION_CHECK_OPTION, &verCheck);
    ADD_JSON_OBJECT_NEW_BOOL(jobj, "autoUpdate", verCheck);

	sysutil_get_firmware_version_info(fversion, sizeof(fversion));
    ADD_JSON_OBJECT_NEW_STRING(jobj, "current", fversion);

    if (result==OK_STR)
    {
        jobj_available = json_object_new_object();

        /* {"model":"EDS1130","version":"v1.0.4","id":"120","file_size":"20316256",
           "release_date":"2014-11-27","change_log":"Fix push message not working on iOS platform.",
           "md5sum":"3960c4973d012e410e63889bf8da031d"} */

        jobj_setting = json_tokener_parse(setting);

        if((jobj_data = json_object_object_get(jobj_setting, "version")))
        {
            ADD_JSON_OBJECT_NEW_STRING(jobj_available, "version", json_object_get_string(jobj_data));
            /* Free obj */
            json_object_put(jobj_data);
        }
        else
        {
            ADD_JSON_OBJECT_NEW_STRING(jobj_available, "version", "");
        }

        if((jobj_data = json_object_object_get(jobj_setting, "id")))
        {
            ADD_JSON_OBJECT_NEW_STRING(jobj_available, "id", json_object_get_string(jobj_data));
            /* Free obj */
            json_object_put(jobj_data);
        }
        else
        {
            ADD_JSON_OBJECT_NEW_STRING(jobj_available, "id", "");
        }

        if((jobj_data = json_object_object_get(jobj_setting, "file_size")))
        {
            ADD_JSON_OBJECT_NEW_STRING(jobj_available, "size", json_object_get_string(jobj_data));
            /* Free obj */
            json_object_put(jobj_data);
        }
        else
        {
            ADD_JSON_OBJECT_NEW_STRING(jobj_available, "size", "");
        }

        if((jobj_data = json_object_object_get(jobj_setting, "release_date")))
        {
            ADD_JSON_OBJECT_NEW_STRING(jobj_available, "date", json_object_get_string(jobj_data));
            /* Free obj */
            json_object_put(jobj_data);
        }
        else
        {
            ADD_JSON_OBJECT_NEW_STRING(jobj_available, "date", "");
        }

        if((jobj_data = json_object_object_get(jobj_setting, "change_log")))
        {
            ADD_JSON_OBJECT_NEW_STRING(jobj_available, "logs", json_object_get_string(jobj_data));
            /* Free obj */
            json_object_put(jobj_data);
        }
        else
        {
            ADD_JSON_OBJECT_NEW_STRING(jobj_available, "logs", "");
        }
        json_object_object_add(jobj, "available", jobj_available);
    }

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);
	if (result==OK_STR)
		json_object_put(jobj_available);

    return 0;
}

/*****************************************************************
* NAME:    get_timezone_capability_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_timezone_capability_json_cb(HTTPS_CB *pkt)
{
	struct json_object *jobj, *jarr_timezone, *jobj_timezone;
	int i, timezone_size=0;
	char cur_time[32]={0};

	jobj = json_object_new_object();

	ADD_JSON_OBJECT_NEW_STRING(jobj, "GetTimeZoneCapabilityResult", OK_STR);
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "ManualDst", TRUE);

	sys_interact(cur_time, sizeof(cur_time), "date \"+%%Y-%%m-%%d %%H:%%M %%Z%%z\" | tr -d \"\n\"");
	ADD_JSON_OBJECT_NEW_STRING(jobj, "CurrentTime", cur_time);

	jarr_timezone = json_object_new_array();

	api_get_system_timezone_size(&timezone_size);

	for(i=0; i<timezone_size-1; i++)
	{
		jobj_timezone = json_object_new_object();

		ADD_JSON_OBJECT_NEW_INT(jobj_timezone, "ID", i);
		ADD_JSON_OBJECT_NEW_STRING(jobj_timezone, "Name", api_timezone_table[i].zonename);

		json_object_array_add(jarr_timezone, jobj_timezone);
	}
	json_object_object_add(jobj, "TimeZones", jarr_timezone);

	http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
	http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
	http_store_data_to_buffer("%s\r\n", json_object_to_json_string(jobj));

	http_send_stored_data(pkt->fd);

	/* Free obj */
	json_object_put(jobj);
	json_object_put(jarr_timezone);
	json_object_put(jobj_timezone);

	return 0;
}

/*****************************************************************
* NAME:    get_systime_setting_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_systime_setting_json_cb(HTTPS_CB *pkt, TIME_SETTINGS_T *settings, char *result)
{
	char buf[32]={0};
	struct json_object *jobj;

	jobj = json_object_new_object();

    ADD_JSON_OBJECT_NEW_STRING(jobj, "GetSysTimeSettingResult", result);

	ADD_JSON_OBJECT_NEW_INT(jobj, "SupportMethod", 3);
	ADD_JSON_OBJECT_NEW_INT(jobj, "NtpUsedIndex", settings->ntp_used_index);
	ADD_JSON_OBJECT_NEW_INT(jobj, "TimeZoneID", settings->timezone_id);
	ADD_JSON_OBJECT_NEW_STRING(jobj, "NtpServer", settings->ntp_server_addr);

#if HAS_WAN_AUTO_DETECTION
	api_get_string_option("system.ntp.auto_detect", buf, sizeof(buf));
	ADD_JSON_OBJECT_NEW_INT(jobj, "AutoDetection", atoi(buf));
#endif

	ADD_JSON_OBJECT_NEW_INT(jobj, "Year", settings->year);
	ADD_JSON_OBJECT_NEW_INT(jobj, "Month", settings->month);
	ADD_JSON_OBJECT_NEW_INT(jobj, "Day", settings->day);
	ADD_JSON_OBJECT_NEW_INT(jobj, "Hour", settings->hour);
	ADD_JSON_OBJECT_NEW_INT(jobj, "Minute", settings->minute);
	ADD_JSON_OBJECT_NEW_INT(jobj, "Second", settings->sec);
    ADD_JSON_OBJECT_NEW_STRING(jobj, "CurrentSysTime", settings->current_sys_time);

	ADD_JSON_OBJECT_NEW_INT(jobj, "DayLightSavingEn", settings->daylight_saving_en);

	http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
	http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
	http_store_data_to_buffer("%s\r\n", json_object_to_json_string(jobj));

	http_send_stored_data(pkt->fd);

	/* Free obj */
	json_object_put(jobj);

	return 0;
}
/*---------------------------------------------------------------------------*/

/*------------------------------- WAN SETTING -------------------------------*/
#if SUPPORT_WAN_SETTING
/*****************************************************************
* NAME:    get_ipv4_wan_status_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_ipv4_wan_status_json_cb(HTTPS_CB *pkt, WAN_SETTINGS_T *setting, char *wan_status, char *result)
{
    struct json_object *jobj;
    int showDSLite = 0;

    if(NULL == pkt)
    {
        return -1;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();
#if SUPPORT_IPV6_DSLITE
    if(setting->typeIdx == WAN_CONNECTION_TYPE_DSLITE && isIfExisted(DSLITE_DEV)){
        T_CHAR buf[256];
        static T_CHAR lanIp[48];
        if(0 == sysutil_interact(buf, sizeof(buf), "ip -6 tunnel show %s", DSLITE_DEV)){
            // dsltun: ip/ipv6 remote 2001:db8::1 local 2001:db8::254 dev eth2.2 encaplimit 4 hoplimit 64 tclass 0x00 flowlabel 0x00000 (flowinfo 0x00000000)
            sscanf(buf, "%*s %*s %*s %s %*s %*s %*s %*s %*s %*d %*s %*d %*s %*s %*s %*s %*s %*s", lanIp);
            if(strcmp(lanIp, "---") != 0 || strcmp(lanIp, "none") != 0 || strcmp(lanIp, "") != 0)
            {
                showDSLite = 1;
            }
        }
    }
#endif

    json_object_object_add(jobj, "GetIpv4WanStatusResult", json_object_new_string(result));
    json_object_object_add(jobj, "Type", json_object_new_string(setting->type));
    json_object_object_add(jobj, "Status", json_object_new_string(wan_status));
    json_object_object_add(jobj, "IPAddress", json_object_new_string(setting->ip_address));
    json_object_object_add(jobj, "SubnetMask", json_object_new_string(setting->subnet_mask));
    json_object_object_add(jobj, "Gateway", json_object_new_string(setting->gateway));
    json_object_object_add(jobj, "MacAddress", json_object_new_string(setting->mac_address));
    json_object_object_add(jobj, "PrimaryDNS", json_object_new_string(setting->dns_primary));
    json_object_object_add(jobj, "SecondaryDNS", json_object_new_string(setting->dns_secondary));
#if 0
    if(showDSLite == 1){
        json_object_object_add(jobj, "AFTRIPv6Address", json_object_new_string(setting->ipv6_ds_aftr_ip6));
        json_object_object_add(jobj, "DSLiteConfigurationManual", json_object_new_boolean(setting->ipv6_ds_type));       
    }
    else{
        json_object_object_add(jobj, "AFTRIPv6Address", json_object_new_string("NULL"));
        json_object_object_add(jobj, "DSLiteConfigurationManual", json_object_new_string("NULL"));
    }
#endif
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return 0;

}
/*****************************************************************
* NAME:    get_ipv4_wan_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_ipv4_wan_settings_json_cb(HTTPS_CB *pkt, WAN_SETTINGS_T *setting, char *result)
{
    int i, max; 
    struct json_object *jobj;

    if(NULL == pkt)
    {
        return -1;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    json_object_object_add(jobj, "GetIPv4WanSettingsResult", json_object_new_string(result));
    json_object_object_add(jobj, "Type", json_object_new_string(setting->type));
	switch (setting->typeIdx)
	{
    case WAN_STATIC:
        for(i = 0, max = T_NUM_OF_ELEMENTS(getIPv4WanPara); i < max; i++){
            if(strcmp(getIPv4WanPara[i],"IPAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ip_address));
            }
            else if(strcmp(getIPv4WanPara[i],"SubnetMask") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->subnet_mask));
            }
            else if(strcmp(getIPv4WanPara[i],"Gateway") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->gateway));
            }
            else if(strcmp(getIPv4WanPara[i],"PrimaryDNS") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->dns_primary));
            }
            else if(strcmp(getIPv4WanPara[i],"SecondaryDNS") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->dns_secondary));
            }
            else{
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string("NULL"));
            }
        }
		break;
    case WAN_DHCP:
        for(i = 0, max = T_NUM_OF_ELEMENTS(getIPv4WanPara); i < max; i++){   
            if(strcmp(getIPv4WanPara[i],"Hostname") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->hostname));
            }
            else if(strcmp(getIPv4WanPara[i],"MacAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->mac_address));
            }
            else if(strcmp(getIPv4WanPara[i],"CloneMac") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_boolean(setting->clonedMacStatus));
            }
            else{
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string("NULL"));
            }
        }
		break;
    case WAN_DSLITE:
        for(i = 0, max = T_NUM_OF_ELEMENTS(getIPv4WanPara); i < max; i++){   
            /*if(strcmp(getIPv4WanPara[i],"DSLiteConfigurationManual") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_boolean(setting->ipv6_ds_type));
            }*/
            if(strcmp(getIPv4WanPara[i],"AFTRIPv6Address") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ipv6_ds_aftr_ip6));
            }
            /*else if(strcmp(getIPv4WanPara[i],"B4IPv4Address") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ipv6_ds_b4_ip4));
            }*/
#if SUPPORT_IPV6_SETTING
            else if(strcmp(getIPv4WanPara[i],"Ipv6WanAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ipv6_wan_address));
            }
            else if(strcmp(getIPv4WanPara[i],"Ipv6WanDefaultGateway") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ipv6_wan_default_gw));
            }
#endif
            else{
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string("NULL"));
            }
        }
        break;
    case WAN_PPTP:
        for(i = 0, max = T_NUM_OF_ELEMENTS(getIPv4WanPara); i < max; i++){
            if(strcmp(getIPv4WanPara[i],"IsIPDynamic") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_boolean(setting->ip_mode));
            }
            else if(strcmp(getIPv4WanPara[i],"Hostname") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->hostname));
            }
            else if(strcmp(getIPv4WanPara[i],"MacAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->mac_address));
            }
            else if(strcmp(getIPv4WanPara[i],"Username") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->user_name));
            }
            else if(strcmp(getIPv4WanPara[i],"Password") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->password));
            }
            else if(strcmp(getIPv4WanPara[i],"PPTPGateway") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->service_ip));
            }
            else if(strcmp(getIPv4WanPara[i],"IPAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ip_address));
            }
            else if(strcmp(getIPv4WanPara[i],"CloneMac") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_boolean(setting->clonedMacStatus));
            }
            else if(strcmp(getIPv4WanPara[i],"SubnetMask") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->subnet_mask));
            }
            else if(strcmp(getIPv4WanPara[i],"DefaultGateway") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->gateway));
            }
            else if(strcmp(getIPv4WanPara[i],"ConnectionId") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->conId));
            }
            else if(strcmp(getIPv4WanPara[i],"MTU") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->mtu));
            }
            else if(strcmp(getIPv4WanPara[i],"IdleTime") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->max_idle_time));
            }
            else if(strcmp(getIPv4WanPara[i],"ConnectionType") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->connType));
            }
            else{
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string("NULL"));
            }
        }
        break;
    case WAN_PPPOE:
        for(i = 0, max = T_NUM_OF_ELEMENTS(getIPv4WanPara); i < max; i++){
            if(strcmp(getIPv4WanPara[i],"Username") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->user_name));
            }
            else if(strcmp(getIPv4WanPara[i],"Password") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->password));
            }
            else if(strcmp(getIPv4WanPara[i],"Service") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->service_name));
            }
            else if(strcmp(getIPv4WanPara[i],"MTU") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->mtu));
            }
            else if(strcmp(getIPv4WanPara[i],"ConnectionType") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->connType));
            }
            else if(strcmp(getIPv4WanPara[i],"IdleTime") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->max_idle_time));
            }
            else{
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string("NULL"));
            }
        }
		break;
    case WAN_L2TP:
    case 99: //WAN_CONNECTION_TYPE_L2TP_RU
        for(i = 0, max = T_NUM_OF_ELEMENTS(getIPv4WanPara); i < max; i++){
            if(strcmp(getIPv4WanPara[i],"IsIPDynamic") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_boolean(setting->ip_mode));
            }
            else if(strcmp(getIPv4WanPara[i],"Hostname") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->hostname));
            }
            else if(strcmp(getIPv4WanPara[i],"MacAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->mac_address));
            }
            else if(strcmp(getIPv4WanPara[i],"Username") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->user_name));
            }
            else if(strcmp(getIPv4WanPara[i],"Password") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->password));
            }
            else if(strcmp(getIPv4WanPara[i],"L2TPGateway") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->service_ip));
            }
            else if(strcmp(getIPv4WanPara[i],"IPAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ip_address));
            }
            else if(strcmp(getIPv4WanPara[i],"CloneMac") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_boolean(setting->clonedMacStatus));
            }
            else if(strcmp(getIPv4WanPara[i],"SubnetMask") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->subnet_mask));
            }
            else if(strcmp(getIPv4WanPara[i],"DefaultGateway") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->gateway));
            }
            else if(strcmp(getIPv4WanPara[i],"MTU") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->mtu));
            }
            else if(strcmp(getIPv4WanPara[i],"IdleTime") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->max_idle_time));
            }
            else if(strcmp(getIPv4WanPara[i],"ConnectionType") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->connType));
            }
#if 0//HAS_WAN1_L2TP_FOR_RUSSIA
    		else if(strcmp(getIPv4WanPara[i],"MultiWanBridgePort") == 0 && apCfgGetIntValue(WAN1_L2TP_RU_ENABLE_TOK))
    		{
#if HAS_IPTV_SETTINGS
                json_object_object_add(jobj, getIPv4WanPara[i],
#if HAS_IPTV_MULTI_SETTINGS
                                       json_object_new_string(setting->multi_wan_bridge_port)
#else
                                       json_object_new_int(setting->wan_bridge_port)
#endif
                                       );
#endif
            }
#endif
            else{
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string("NULL"));
            }
        }
        break;
    }

#if 0
    /* assign the parametes value which are part of selected wan type or fill the "NULL" */
	switch (setting->typeIdx)
	{
    case 0:
    printf("\n--%s------------------dw-------------------->%s[%d]:%s%d\n",__TIME__,__FUNCTION__,__LINE__,"JsontypeIdx",setting->typeIdx);
        for(i = 0, max = T_NUM_OF_ELEMENTS(getIPv4WanPara); i < max; i++){
            if(strcmp(getIPv4WanPara[i],"IPAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ip_address));
            }
            else if(strcmp(getIPv4WanPara[i],"SubnetMask") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->subnet_mask));
            }
            else if(strcmp(getIPv4WanPara[i],"Gateway") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->gateway));
            }
            else if(strcmp(getIPv4WanPara[i],"PrimaryDNS") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->dns_primary));
            }
            else if(strcmp(getIPv4WanPara[i],"SecondaryDNS") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->dns_secondary));
            }
            else{
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string("NULL"));
            }
        }
		break;
    case 1:
        for(i = 0, max = T_NUM_OF_ELEMENTS(getIPv4WanPara); i < max; i++){   
            if(strcmp(getIPv4WanPara[i],"Hostname") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->hostname));
            }
            else if(strcmp(getIPv4WanPara[i],"MacAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->mac_address));
            }
            else if(strcmp(getIPv4WanPara[i],"CloneMac") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_boolean(setting->clonedMacStatus));
            }
            else{
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string("NULL"));
            }
        }
		break;

#if HAS_WAN_PPPOE || HAS_WAN_3G
    case WAN_CONNECTION_TYPE_PPPOE:
        for(i = 0, max = T_NUM_OF_ELEMENTS(getIPv4WanPara); i < max; i++){
            if(strcmp(getIPv4WanPara[i],"Username") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->user_name));
            }
            else if(strcmp(getIPv4WanPara[i],"Password") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->password));
            }
            else if(strcmp(getIPv4WanPara[i],"Service") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->service_name));
            }
            else if(strcmp(getIPv4WanPara[i],"MTU") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->mtu));
            }
            else if(strcmp(getIPv4WanPara[i],"ConnectionType") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->connType));
            }
            else if(strcmp(getIPv4WanPara[i],"IdleTime") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->max_idle_time));
            }
            else{
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string("NULL"));
            }
        }
		break;
#endif

#if HAS_WAN_PPTP
    case WAN_CONNECTION_TYPE_PPTP:
        for(i = 0, max = T_NUM_OF_ELEMENTS(getIPv4WanPara); i < max; i++){
            if(strcmp(getIPv4WanPara[i],"IsIPDynamic") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_boolean(setting->ip_mode));
            }
            else if(strcmp(getIPv4WanPara[i],"Hostname") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->hostname));
            }
            else if(strcmp(getIPv4WanPara[i],"MacAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->mac_address));
            }
            else if(strcmp(getIPv4WanPara[i],"Username") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->user_name));
            }
            else if(strcmp(getIPv4WanPara[i],"Password") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->password));
            }
            else if(strcmp(getIPv4WanPara[i],"PPTPGateway") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->service_ip));
            }
            else if(strcmp(getIPv4WanPara[i],"IPAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ip_address));
            }
            else if(strcmp(getIPv4WanPara[i],"CloneMac") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_boolean(setting->clonedMacStatus));
            }
            else if(strcmp(getIPv4WanPara[i],"SubnetMask") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->subnet_mask));
            }
            else if(strcmp(getIPv4WanPara[i],"DefaultGateway") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->gateway));
            }
            else if(strcmp(getIPv4WanPara[i],"ConnectionId") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->conId));
            }
            else if(strcmp(getIPv4WanPara[i],"MTU") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->mtu));
            }
            else if(strcmp(getIPv4WanPara[i],"IdleTime") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->max_idle_time));
            }
            else if(strcmp(getIPv4WanPara[i],"ConnectionType") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->connType));
            }
            else{
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string("NULL"));
            }
        }
        break;
#endif
#if HAS_WAN_L2TP
    case WAN_CONNECTION_TYPE_L2TP:
    case 99: //WAN_CONNECTION_TYPE_L2TP_RU
        for(i = 0, max = T_NUM_OF_ELEMENTS(getIPv4WanPara); i < max; i++){
            if(strcmp(getIPv4WanPara[i],"IsIPDynamic") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_boolean(setting->ip_mode));
            }
            else if(strcmp(getIPv4WanPara[i],"Hostname") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->hostname));
            }
            else if(strcmp(getIPv4WanPara[i],"MacAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->mac_address));
            }
            else if(strcmp(getIPv4WanPara[i],"Username") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->user_name));
            }
            else if(strcmp(getIPv4WanPara[i],"Password") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->password));
            }
            else if(strcmp(getIPv4WanPara[i],"L2TPGateway") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->service_ip));
            }
            else if(strcmp(getIPv4WanPara[i],"IPAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ip_address));
            }
            else if(strcmp(getIPv4WanPara[i],"CloneMac") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_boolean(setting->clonedMacStatus));
            }
            else if(strcmp(getIPv4WanPara[i],"SubnetMask") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->subnet_mask));
            }
            else if(strcmp(getIPv4WanPara[i],"DefaultGateway") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->gateway));
            }
            else if(strcmp(getIPv4WanPara[i],"MTU") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->mtu));
            }
            else if(strcmp(getIPv4WanPara[i],"IdleTime") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->max_idle_time));
            }
            else if(strcmp(getIPv4WanPara[i],"ConnectionType") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_int(setting->connType));
            }
#if HAS_WAN1_L2TP_FOR_RUSSIA
    		else if(strcmp(getIPv4WanPara[i],"MultiWanBridgePort") == 0 && apCfgGetIntValue(WAN1_L2TP_RU_ENABLE_TOK))
    		{
#if HAS_IPTV_SETTINGS
                json_object_object_add(jobj, getIPv4WanPara[i],
#if HAS_IPTV_MULTI_SETTINGS
                                       json_object_new_string(setting->multi_wan_bridge_port)
#else
                                       json_object_new_int(setting->wan_bridge_port)
#endif
                                       );
#endif
            }
#endif
            else{
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string("NULL"));
            }
        }
        break;
#endif
#if SUPPORT_IPV6_DSLITE
    case WAN_CONNECTION_TYPE_DSLITE:
        for(i = 0, max = T_NUM_OF_ELEMENTS(getIPv4WanPara); i < max; i++){   
            if(strcmp(getIPv4WanPara[i],"DSLiteConfigurationManual") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_boolean(setting->ipv6_ds_type));
            }
            else if(strcmp(getIPv4WanPara[i],"AFTRIPv6Address") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ipv6_ds_aftr_ip6));
            }
            else if(strcmp(getIPv4WanPara[i],"B4IPv4Address") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ipv6_ds_b4_ip4));
            }
#if SUPPORT_IPV6_SETTING
#if HAS_WAN_DEV
            else if(strcmp(getIPv4WanPara[i],"Ipv6WanAddress") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ipv6_wan_address));
            }
            else if(strcmp(getIPv4WanPara[i],"Ipv6WanDefaultGateway") == 0){
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string(setting->ipv6_wan_default_gw));
            }
#endif
#endif
            else{
                json_object_object_add(jobj, getIPv4WanPara[i], json_object_new_string("NULL"));
            }
        }
        break;
#endif


	}
#endif
    /* Store packet content into buffer and send it out */
	http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
	http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));
	http_send_stored_data(pkt->fd);
	/* Free obj */
	json_object_put(jobj);

	return 0;
}

/*****************************************************************
* NAME:    parse_ipv4_wan_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int parse_ipv4_wan_settings_json_cb(char *query_str, WAN_SETTINGS_T *setting, char **result_str)
{
    bool result, is_jobj;
    int i=0;
    struct json_object *jobj;
    struct json_object *jobj_type,  *jobj_ipaddress, *jobj_subnetmask, *jobj_gateway, *jobj_pri, *jobj_sec;

    result = TRUE;
    is_jobj = FALSE;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_type = json_object_object_get(jobj, "Type")))
            {
                sprintf(setting->type, "%s", json_object_get_string(jobj_type));
                /* Free obj */
                json_object_put(jobj_type);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            /* TYPE */
            for(i=0;i<wanConnectTypeArrSize;i++)
            {
                if(strcmp(wanConnectTypeArr[i].name, setting->type)==0)
				{
        			strcpy(setting->type, wanConnectTypeArr[i].name);
        			setting->typeIdx=wanConnectTypeArr[i].type;
					break;
        		}
            }
            switch(setting->typeIdx)
            {
            case WAN_STATIC:
                if((jobj_ipaddress = json_object_object_get(jobj, "IPAddress")))
                {
                    sprintf(setting->ip_address, "%s", json_object_get_string(jobj_ipaddress));
                    /* Free obj */
                    json_object_put(jobj_ipaddress);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if((jobj_subnetmask = json_object_object_get(jobj, "SubnetMask")))
                {
                    sprintf(setting->subnet_mask, "%s", json_object_get_string(jobj_subnetmask));

                    /* Free obj */
                    json_object_put(jobj_subnetmask);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if((jobj_gateway = json_object_object_get(jobj, "Gateway")))
                {
                    sprintf(setting->gateway, "%s", json_object_get_string(jobj_gateway));

                    /* Free obj */
                    json_object_put(jobj_gateway);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if((jobj_pri = json_object_object_get(jobj, "PrimaryDNS")))
                {
                    sprintf(setting->dns_primary, "%s", json_object_get_string(jobj_pri));

                    /* Free obj */
                    json_object_put(jobj_pri);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if((jobj_sec = json_object_object_get(jobj, "SecondaryDNS")))
                {
                    sprintf(setting->dns_secondary, "%s", json_object_get_string(jobj_sec));

                    /* Free obj */
                    json_object_put(jobj_sec);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }
                break;
            case WAN_DHCP:
                if(json_object_object_get(jobj, "Hostname"))
    			{
    				strcpy(setting->hostname, json_object_get_string(json_object_object_get(jobj, "Hostname")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
    			if(json_object_object_get(jobj, "MacAddress"))
    			{
    				strcpy(setting->mac_address, json_object_get_string(json_object_object_get(jobj, "MacAddress")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
    			if(json_object_object_get(jobj, "CloneMac"))
    			{
                     result = senao_json_object_get_boolean(jobj, "CloneMac", &(setting->clonedMacStatus));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                break;
#if 1//HAS_WAN_PPPOE || HAS_WAN_3G
            case WAN_PPPOE:
                if(json_object_object_get(jobj, "Username"))
    			{
    				strcpy(setting->user_name, json_object_get_string(json_object_object_get(jobj, "Username")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "Password"))
    			{
    				strcpy(setting->password, json_object_get_string(json_object_object_get(jobj, "Password")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "Service"))
    			{
    				strcpy(setting->service_name, json_object_get_string(json_object_object_get(jobj, "Service")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
    			if(json_object_object_get(jobj, "MTU"))
    			{
                    if(!regxMatch(INTEGER_REGX, json_object_get_string(json_object_object_get(jobj, "MTU")))){
                        *result_str = ERROR_MTU_STR;
                        result = FALSE;
                        goto out;
                    }
                    setting->mtu = json_object_get_int(json_object_object_get(jobj, "MTU"));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "ConnectionType"))
    			{
                    setting->connType = json_object_get_int(json_object_object_get(jobj, "ConnectionType"));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "IdleTime"))
    			{
                    if(!regxMatch(INTEGER_REGX, json_object_get_string(json_object_object_get(jobj, "IdleTime")))){
                        *result_str = ERROR_IDLE_TIME_STR;
                        result = FALSE;
                        goto out;
                    }
                    setting->max_idle_time = json_object_get_int(json_object_object_get(jobj, "IdleTime"));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                break;
#endif
#if 1//HAS_WAN_PPTP | HAS_WAN_L2TP
            case WAN_PPTP:
            case WAN_L2TP:
#if HAS_WAN1_L2TP_FOR_RUSSIA
            case 99: //WAN_CONNECTION_TYPE_L2TP_RU
#endif
#if 1//HAS_WAN_PPTP
                if(setting->typeIdx == WAN_PPTP)
                {
                    if(json_object_object_get(jobj, "ConnectionId"))
        			{
                        setting->conId = json_object_get_int(json_object_object_get(jobj, "ConnectionId"));
        			}
        			else
                    {
                        result = FALSE;
                        goto out;
                    }
                    if(json_object_object_get(jobj, "PPTPGateway"))
                    {
                        strcpy(setting->service_ip, json_object_get_string(json_object_object_get(jobj, "PPTPGateway")));
                    }
                    else
                    {
                        result = FALSE;
                        goto out;
                    }
                }
#endif
#if 1//HAS_WAN_L2TP
                if(setting->typeIdx == WAN_L2TP
#if HAS_WAN1_L2TP_FOR_RUSSIA
                   || setting->typeIdx == 99
#endif
                   )
                {
                    if(json_object_object_get(jobj, "L2TPGateway"))
        			{
        				strcpy(setting->service_ip, json_object_get_string(json_object_object_get(jobj, "L2TPGateway")));
        			}
        			else
                    {
                        result = FALSE;
                        goto out;
                    }
#if HAS_WAN1_L2TP_FOR_RUSSIA
                    if(json_object_object_get(jobj, "MultiWanBridgePort"))
        			{
        				strcpy(setting->multi_wan_bridge_port, json_object_get_string(json_object_object_get(jobj, "MultiWanBridgePort")));
                        //reverseStr(setting->multi_wan_bridge_port); // The input value is port 4321 but The token set port is 1234, so reverse it
        			}
        			else
                    {
                        result = FALSE;
                        goto out;
                    }
#endif
                }
#endif
                if((jobj_ipaddress = json_object_object_get(jobj, "IPAddress")))
                {
                    sprintf(setting->ip_address, "%s", json_object_get_string(jobj_ipaddress));

                    /* Free obj */
                    json_object_put(jobj_ipaddress);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if((jobj_subnetmask = json_object_object_get(jobj, "SubnetMask")))
                {
                    sprintf(setting->subnet_mask, "%s", json_object_get_string(jobj_subnetmask));

                    /* Free obj */
                    json_object_put(jobj_subnetmask);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if(json_object_object_get(jobj, "Username"))
    			{
    				strcpy(setting->user_name, json_object_get_string(json_object_object_get(jobj, "Username")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
    			if(json_object_object_get(jobj, "Password"))
    			{
    				strcpy(setting->password, json_object_get_string(json_object_object_get(jobj, "Password")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
    			if(json_object_object_get(jobj, "Service"))
    			{
    				strcpy(setting->service_name, json_object_get_string(json_object_object_get(jobj, "Service")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
    			if(json_object_object_get(jobj, "MTU"))
    			{
                    if(!regxMatch(INTEGER_REGX, json_object_get_string(json_object_object_get(jobj, "MTU")))){
                        *result_str = ERROR_MTU_STR;
                        result = FALSE;
                        goto out;
                    }
                    setting->mtu = json_object_get_int(json_object_object_get(jobj, "MTU"));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "ConnectionType"))
    			{
                    setting->connType = json_object_get_int(json_object_object_get(jobj, "ConnectionType"));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "IdleTime"))
    			{
                    if(!regxMatch(INTEGER_REGX, json_object_get_string(json_object_object_get(jobj, "IdleTime")))){
                        *result_str = ERROR_IDLE_TIME_STR;
                        result = FALSE;
                        goto out;
                    }
                    setting->max_idle_time = json_object_get_int(json_object_object_get(jobj, "IdleTime"));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "IsIPDynamic"))
    			{
                    result = senao_json_object_get_boolean(jobj, "IsIPDynamic", &(setting->IsIPDynamic));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "DefaultGateway"))
    			{
    				strcpy(setting->gateway, json_object_get_string(json_object_object_get(jobj, "DefaultGateway")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "Hostname"))
    			{
    				strcpy(setting->hostname, json_object_get_string(json_object_object_get(jobj, "Hostname")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "MacAddress"))
    			{
    				strcpy(setting->mac_address, json_object_get_string(json_object_object_get(jobj, "MacAddress")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "CloneMac"))
    			{
                    result = senao_json_object_get_boolean(jobj, "CloneMac", &(setting->clonedMacStatus));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                break;
#endif
#if 1//SUPPORT_IPV6_DSLITE
            case WAN_DSLITE:
				if(json_object_object_get(jobj, "DSLiteConfigurationManual"))
    			{
                    result = senao_json_object_get_boolean(jobj, "DSLiteConfigurationManual", (bool *)&(setting->ipv6_ds_type));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
				if(json_object_object_get(jobj, "AFTRIPv6Address"))
    			{
    				strcpy(setting->ipv6_ds_aftr_ip6, json_object_get_string(json_object_object_get(jobj, "AFTRIPv6Address")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
				if(json_object_object_get(jobj, "B4IPv4Address"))
    			{
    				strcpy(setting->ipv6_ds_b4_ip4, json_object_get_string(json_object_object_get(jobj, "B4IPv4Address")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                break;
#endif
            }
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}
/*****************************************************************
* NAME:    parse_ipv4_wan_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int parse_wan_settings_json_cb(char *query_str, WAN_SETTINGS_T *setting, char **result_str)
{
    bool result, is_jobj;
    int i=0;
    struct json_object *jobj;
    struct json_object *jobj_type,  *jobj_ipaddress, *jobj_subnetmask, *jobj_gateway, *jobj_pri, *jobj_sec, *jobj_dns;

    result = TRUE;
    is_jobj = FALSE;
    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_type = json_object_object_get(jobj, "Type")))
            {
                sprintf(setting->type, "%s", json_object_get_string(jobj_type));
                /* Free obj */
                json_object_put(jobj_type);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            /* TYPE */
            for(i=0;i<wanConnectTypeArrSize;i++)
            {
                if(strcmp(wanConnectTypeArr[i].name, setting->type)==0)
				{
        			strcpy(setting->type, wanConnectTypeArr[i].name);
        			setting->typeIdx=wanConnectTypeArr[i].type;
					break;
        		}
            }

            switch(setting->typeIdx)
            {
            case WAN_STATIC:
                if((jobj_ipaddress = json_object_object_get(jobj, "IPAddress")))
                {
                    sprintf(setting->ip_address, "%s", json_object_get_string(jobj_ipaddress));
                    /* Free obj */
                    json_object_put(jobj_ipaddress);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if((jobj_subnetmask = json_object_object_get(jobj, "SubnetMask")))
                {
                    sprintf(setting->subnet_mask, "%s", json_object_get_string(jobj_subnetmask));

                    /* Free obj */
                    json_object_put(jobj_subnetmask);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if((jobj_gateway = json_object_object_get(jobj, "Gateway")))
                {
                    sprintf(setting->gateway, "%s", json_object_get_string(jobj_gateway));

                    /* Free obj */
                    json_object_put(jobj_gateway);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                jobj_dns = json_object_object_get(jobj, "DNS");

                if((jobj_pri = json_object_object_get(jobj_dns, "Primary")))
                {
                    sprintf(setting->dns_primary, "%s", json_object_get_string(jobj_pri));

                    /* Free obj */
                    json_object_put(jobj_pri);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }
                if((jobj_sec = json_object_object_get(jobj_dns, "Secondary")))
                {
                    sprintf(setting->dns_secondary, "%s", json_object_get_string(jobj_sec));

                    /* Free obj */
                    json_object_put(jobj_sec);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }
                break;
            case WAN_DHCP:
                if(json_object_object_get(jobj, "Hostname"))
    			{
    				strcpy(setting->hostname, json_object_get_string(json_object_object_get(jobj, "Hostname")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
    			if(json_object_object_get(jobj, "MacAddress"))
    			{
    				strcpy(setting->mac_address, json_object_get_string(json_object_object_get(jobj, "MacAddress")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
    			if(json_object_object_get(jobj, "CloneMac"))
    			{
                     result = senao_json_object_get_boolean(jobj, "CloneMac", &(setting->clonedMacStatus));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                break;
#if 1//HAS_WAN_PPPOE || HAS_WAN_3G
            case WAN_PPPOE:
                if(json_object_object_get(jobj, "Username"))
    			{
    				strcpy(setting->user_name, json_object_get_string(json_object_object_get(jobj, "Username")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "Password"))
    			{
    				strcpy(setting->password, json_object_get_string(json_object_object_get(jobj, "Password")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "Service"))
    			{
    				strcpy(setting->service_name, json_object_get_string(json_object_object_get(jobj, "Service")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
    			if(json_object_object_get(jobj, "MTU"))
    			{
                    if(!regxMatch(INTEGER_REGX, json_object_get_string(json_object_object_get(jobj, "MTU")))){
                        *result_str = ERROR_MTU_STR;
                        result = FALSE;
                        goto out;
                    }
                    setting->mtu = json_object_get_int(json_object_object_get(jobj, "MTU"));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "ConnectionType"))
    			{
                    setting->connType = json_object_get_int(json_object_object_get(jobj, "ConnectionType"));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "IdleTime"))
    			{
                    if(!regxMatch(INTEGER_REGX, json_object_get_string(json_object_object_get(jobj, "IdleTime")))){
                        *result_str = ERROR_IDLE_TIME_STR;
                        result = FALSE;
                        goto out;
                    }
                    setting->max_idle_time = json_object_get_int(json_object_object_get(jobj, "IdleTime"));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                break;
#endif
#if 1//HAS_WAN_PPTP | HAS_WAN_L2TP
            case WAN_PPTP:
            case WAN_L2TP:
#if HAS_WAN1_L2TP_FOR_RUSSIA
            case 99: //WAN_CONNECTION_TYPE_L2TP_RU
#endif
#if 1//HAS_WAN_PPTP
                if(setting->typeIdx == WAN_PPTP)
                {
                    if(json_object_object_get(jobj, "ConnectionId"))
        			{
                        setting->conId = json_object_get_int(json_object_object_get(jobj, "ConnectionId"));
        			}
        			else
                    {
                        result = FALSE;
                        goto out;
                    }
                    if(json_object_object_get(jobj, "PPTPGateway"))
                    {
                        strcpy(setting->service_ip, json_object_get_string(json_object_object_get(jobj, "PPTPGateway")));
                    }
                    else
                    {
                        result = FALSE;
                        goto out;
                    }
                }
#endif
#if 1//HAS_WAN_L2TP
                if(setting->typeIdx == WAN_L2TP
#if HAS_WAN1_L2TP_FOR_RUSSIA
                   || setting->typeIdx == 99
#endif
                   )
                {
                    if(json_object_object_get(jobj, "L2TPGateway"))
        			{
        				strcpy(setting->service_ip, json_object_get_string(json_object_object_get(jobj, "L2TPGateway")));
        			}
        			else
                    {
                        result = FALSE;
                        goto out;
                    }
#if HAS_WAN1_L2TP_FOR_RUSSIA
                    if(json_object_object_get(jobj, "MultiWanBridgePort"))
        			{
        				strcpy(setting->multi_wan_bridge_port, json_object_get_string(json_object_object_get(jobj, "MultiWanBridgePort")));
                        //reverseStr(setting->multi_wan_bridge_port); // The input value is port 4321 but The token set port is 1234, so reverse it
        			}
        			else
                    {
                        result = FALSE;
                        goto out;
                    }
#endif
                }
#endif
                if((jobj_ipaddress = json_object_object_get(jobj, "IPAddress")))
                {
                    sprintf(setting->ip_address, "%s", json_object_get_string(jobj_ipaddress));

                    /* Free obj */
                    json_object_put(jobj_ipaddress);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if((jobj_subnetmask = json_object_object_get(jobj, "SubnetMask")))
                {
                    sprintf(setting->subnet_mask, "%s", json_object_get_string(jobj_subnetmask));

                    /* Free obj */
                    json_object_put(jobj_subnetmask);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if(json_object_object_get(jobj, "Username"))
    			{
    				strcpy(setting->user_name, json_object_get_string(json_object_object_get(jobj, "Username")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
    			if(json_object_object_get(jobj, "Password"))
    			{
    				strcpy(setting->password, json_object_get_string(json_object_object_get(jobj, "Password")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
    			if(json_object_object_get(jobj, "Service"))
    			{
    				strcpy(setting->service_name, json_object_get_string(json_object_object_get(jobj, "Service")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
    			if(json_object_object_get(jobj, "MTU"))
    			{
                    if(!regxMatch(INTEGER_REGX, json_object_get_string(json_object_object_get(jobj, "MTU")))){
                        *result_str = ERROR_MTU_STR;
                        result = FALSE;
                        goto out;
                    }
                    setting->mtu = json_object_get_int(json_object_object_get(jobj, "MTU"));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "ConnectionType"))
    			{
                    setting->connType = json_object_get_int(json_object_object_get(jobj, "ConnectionType"));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "IdleTime"))
    			{
                    if(!regxMatch(INTEGER_REGX, json_object_get_string(json_object_object_get(jobj, "IdleTime")))){
                        *result_str = ERROR_IDLE_TIME_STR;
                        result = FALSE;
                        goto out;
                    }
                    setting->max_idle_time = json_object_get_int(json_object_object_get(jobj, "IdleTime"));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "IsIPDynamic"))
    			{
                    result = senao_json_object_get_boolean(jobj, "IsIPDynamic", &(setting->IsIPDynamic));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "DefaultGateway"))
    			{
    				strcpy(setting->gateway, json_object_get_string(json_object_object_get(jobj, "DefaultGateway")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "Hostname"))
    			{
    				strcpy(setting->hostname, json_object_get_string(json_object_object_get(jobj, "Hostname")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "MacAddress"))
    			{
    				strcpy(setting->mac_address, json_object_get_string(json_object_object_get(jobj, "MacAddress")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                if(json_object_object_get(jobj, "CloneMac"))
    			{
                    result = senao_json_object_get_boolean(jobj, "CloneMac", &(setting->clonedMacStatus));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                break;
#endif
#if 1//SUPPORT_IPV6_DSLITE
            case WAN_DSLITE:
				if(json_object_object_get(jobj, "DSLiteConfigurationManual"))
    			{
                    result = senao_json_object_get_boolean(jobj, "DSLiteConfigurationManual", (bool *)&(setting->ipv6_ds_type));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
				if(json_object_object_get(jobj, "AFTRIPv6Address"))
    			{
    				strcpy(setting->ipv6_ds_aftr_ip6, json_object_get_string(json_object_object_get(jobj, "AFTRIPv6Address")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
				if(json_object_object_get(jobj, "B4IPv4Address"))
    			{
    				strcpy(setting->ipv6_ds_b4_ip4, json_object_get_string(json_object_object_get(jobj, "B4IPv4Address")));
    			}
    			else
                {
                    result = FALSE;
                    goto out;
                }
                break;
#endif
            }
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}

#if SUPPORT_IPV6_SETTING
/*****************************************************************
* NAME:    get_ipv6_wan_status_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_ipv6_wan_status_json_cb(HTTPS_CB *pkt, WAN_IPV6_SETTINGS_T *setting, char *result)
{
    struct json_object *jobj;
    struct json_object *jstr_result;
    struct json_object *jstr_type;
    struct json_object *jstr_status;
    struct json_object *jstr_ipv6_addr;
    struct json_object *jstr_tunnel_link_local;
    struct json_object *jstr_default_gw;
    struct json_object *jstr_lan_ipv6_addr;
    struct json_object *jstr_lan_link_local;
    struct json_object *jstr_primary_dns;
    struct json_object *jstr_secondary_dns;
    struct json_object *jbool_enable_dhcpPD;
    struct json_object *jstr_wan_link_local;
    struct json_object *jstr_null;

    if(NULL == pkt)
    {
        return -1;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_null = json_object_new_string(NULL_STR);

    jstr_result = json_object_new_string(result);
    json_object_object_add(jobj, "GetIpv6WanStatusResult", jstr_result);

    jstr_type = json_object_new_string(setting->type);
    json_object_object_add(jobj, WAN_TYPE_STR, jstr_type);

    jstr_status = json_object_new_string(setting->status);

    jstr_ipv6_addr = json_object_new_string(setting->ipv6_addr);

    jstr_tunnel_link_local = json_object_new_string(setting->tunnel_link_local);

    jstr_default_gw = json_object_new_string(setting->default_gw);

    jstr_lan_ipv6_addr = json_object_new_string(setting->lan_ipv6_addr);

    jstr_lan_link_local = json_object_new_string(setting->lan_link_local);

    jstr_primary_dns = json_object_new_string(setting->primary_dns);

    jstr_secondary_dns = json_object_new_string(setting->secondary_dns);

    jbool_enable_dhcpPD = json_object_new_boolean(setting->enable_dhcpPD);

    jstr_wan_link_local = json_object_new_string(setting->wan_link_local);

    switch(setting->type_index)
    {
        case IPV6_WAN_STATIC:
            json_object_object_add(jobj, WAN_STATUS_STR, jstr_status);
            json_object_object_add(jobj, WAN_IPV6_ADDRESS_STR, jstr_ipv6_addr);
            json_object_object_add(jobj, WAN_TUNNEL_LINK_LOCAL_STR, jstr_null);
            json_object_object_add(jobj, WAN_DEFAULT_GATEWAY_STR, jstr_default_gw);
            json_object_object_add(jobj, WAN_LAN_IPV6_ADDRESS_STR, jstr_lan_ipv6_addr);
            json_object_object_add(jobj, WAN_LAN_LINK_LOCAL_STR, jstr_lan_link_local);
            json_object_object_add(jobj, WAN_PRIMARY_DNS_STR, jstr_primary_dns);
            json_object_object_add(jobj, WAN_SECONDARY_DNS_STR, jstr_secondary_dns);
            json_object_object_add(jobj, WAN_ENABLE_DHCPPD_STR, jstr_null);
            json_object_object_add(jobj, WAN_WAN_LINK_LOCAL_STR, jstr_null);
            break;
        case IPV6_WAN_PPPOE:
            json_object_object_add(jobj, WAN_STATUS_STR, jstr_status);
            json_object_object_add(jobj, WAN_IPV6_ADDRESS_STR, jstr_ipv6_addr);
            json_object_object_add(jobj, WAN_TUNNEL_LINK_LOCAL_STR, jstr_null);
            json_object_object_add(jobj, WAN_DEFAULT_GATEWAY_STR, jstr_default_gw);
            json_object_object_add(jobj, WAN_LAN_IPV6_ADDRESS_STR, jstr_lan_ipv6_addr);
            json_object_object_add(jobj, WAN_LAN_LINK_LOCAL_STR, jstr_lan_link_local);
            json_object_object_add(jobj, WAN_PRIMARY_DNS_STR, jstr_primary_dns);
            json_object_object_add(jobj, WAN_SECONDARY_DNS_STR, jstr_secondary_dns);
            json_object_object_add(jobj, WAN_ENABLE_DHCPPD_STR, jbool_enable_dhcpPD);
            json_object_object_add(jobj, WAN_WAN_LINK_LOCAL_STR, jstr_null);
            break;
        case IPV6_WAN_AUTOCONFIGURATION:
            json_object_object_add(jobj, WAN_STATUS_STR, jstr_status);
            json_object_object_add(jobj, WAN_IPV6_ADDRESS_STR, jstr_ipv6_addr);
            json_object_object_add(jobj, WAN_TUNNEL_LINK_LOCAL_STR, jstr_null);
            json_object_object_add(jobj, WAN_DEFAULT_GATEWAY_STR, jstr_default_gw);
            json_object_object_add(jobj, WAN_LAN_IPV6_ADDRESS_STR, jstr_lan_ipv6_addr);
            json_object_object_add(jobj, WAN_LAN_LINK_LOCAL_STR, jstr_lan_link_local);
            json_object_object_add(jobj, WAN_PRIMARY_DNS_STR, jstr_primary_dns);
            json_object_object_add(jobj, WAN_SECONDARY_DNS_STR, jstr_secondary_dns);
            json_object_object_add(jobj, WAN_ENABLE_DHCPPD_STR, jstr_null);
            json_object_object_add(jobj, WAN_WAN_LINK_LOCAL_STR, jstr_null);
            break;
        case IPV6_WAN_6RD:
            json_object_object_add(jobj, WAN_STATUS_STR, jstr_status);
            json_object_object_add(jobj, WAN_IPV6_ADDRESS_STR, jstr_null);
            json_object_object_add(jobj, WAN_TUNNEL_LINK_LOCAL_STR, jstr_tunnel_link_local);
            json_object_object_add(jobj, WAN_DEFAULT_GATEWAY_STR, jstr_default_gw);
            json_object_object_add(jobj, WAN_LAN_IPV6_ADDRESS_STR, jstr_lan_ipv6_addr);
            json_object_object_add(jobj, WAN_LAN_LINK_LOCAL_STR, jstr_lan_link_local);
            json_object_object_add(jobj, WAN_PRIMARY_DNS_STR, jstr_primary_dns);
            json_object_object_add(jobj, WAN_SECONDARY_DNS_STR, jstr_secondary_dns);
            json_object_object_add(jobj, WAN_ENABLE_DHCPPD_STR, jstr_null);
            json_object_object_add(jobj, WAN_WAN_LINK_LOCAL_STR, jstr_null);
            break;
        case IPV6_WAN_LINK_LOCAL_ONLY:
            json_object_object_add(jobj, WAN_STATUS_STR, jstr_null);
            json_object_object_add(jobj, WAN_IPV6_ADDRESS_STR, jstr_null);
            json_object_object_add(jobj, WAN_TUNNEL_LINK_LOCAL_STR, jstr_null);
            json_object_object_add(jobj, WAN_DEFAULT_GATEWAY_STR, jstr_null);
            json_object_object_add(jobj, WAN_LAN_IPV6_ADDRESS_STR, jstr_null);
            json_object_object_add(jobj, WAN_LAN_LINK_LOCAL_STR, jstr_lan_link_local);
            json_object_object_add(jobj, WAN_PRIMARY_DNS_STR, jstr_null);
            json_object_object_add(jobj, WAN_SECONDARY_DNS_STR, jstr_null);
            json_object_object_add(jobj, WAN_ENABLE_DHCPPD_STR, jstr_null);
            json_object_object_add(jobj, WAN_WAN_LINK_LOCAL_STR, jstr_wan_link_local);
            break;
        default:
            break;
    }

    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jstr_null);
    json_object_put(jstr_result);
    json_object_put(jstr_type);
    json_object_put(jstr_status);
    json_object_put(jstr_ipv6_addr);
    json_object_put(jstr_tunnel_link_local);
    json_object_put(jstr_default_gw);
    json_object_put(jstr_lan_ipv6_addr);
    json_object_put(jstr_lan_link_local);
    json_object_put(jstr_primary_dns);
    json_object_put(jstr_secondary_dns);
    json_object_put(jbool_enable_dhcpPD);
    json_object_put(jstr_wan_link_local);
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    get_ipv6_wan_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_ipv6_wan_settings_json_cb(HTTPS_CB *pkt, WAN_IPV6_SETTINGS_T *setting, char *result)
{
    struct json_object *jobj;
    struct json_object *jstr_result;
    struct json_object *jstr_type;
    struct json_object *jbool_use_link_local;
    struct json_object *jstr_ipv6_addr;
    struct json_object *jint_prefix_length;
    struct json_object *jstr_default_gw;
    struct json_object *jstr_primary_dns;
    struct json_object *jstr_secondary_dns;
    struct json_object *jstr_lan_ipv6_addr;
    struct json_object *jbool_enable_auto_assignment;
    struct json_object *jint_auto_config_type;
    struct json_object *jstr_start;
    struct json_object *jstr_end;
    struct json_object *jint_ra_life_time;
    struct json_object *jint_dhcpv6_life_time;
    struct json_object *jbool_is_ip_dynamic;
    struct json_object *jstr_username;
    struct json_object *jstr_password;
    struct json_object *jstr_service;
    struct json_object *jint_reconnect_mode;
    struct json_object *jint_mtu;
    struct json_object *jbool_enable_auto_dns;
    struct json_object *jbool_enable_dhcpPD;
    struct json_object *jbool_6rd_type;
    struct json_object *jstr_prefix;
    struct json_object *jint_mask_length;
    struct json_object *jstr_relay;
    struct json_object *jstr_lan_link_local;
    struct json_object *jstr_wan_link_local;
    struct json_object *jstr_ipv4_addr;
    struct json_object *jstr_null;

    if(NULL == pkt)
    {
        return -1;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_null = json_object_new_string(NULL_STR);

    jstr_result = json_object_new_string(result);
    json_object_object_add(jobj, "GetIpv6WanSettingResult", jstr_result);

    jstr_type = json_object_new_string(setting->type);
    json_object_object_add(jobj, WAN_TYPE_STR, jstr_type);

    jbool_use_link_local = json_object_new_boolean(setting->use_link_local);
    jstr_ipv6_addr = json_object_new_string(setting->ipv6_addr);
    jint_prefix_length = json_object_new_int(setting->prefix_length);
    jstr_default_gw = json_object_new_string(setting->default_gw);
    jstr_primary_dns = json_object_new_string(setting->primary_dns);
    jstr_secondary_dns = json_object_new_string(setting->secondary_dns);
    jstr_lan_ipv6_addr = json_object_new_string(setting->lan_ipv6_addr);
    jbool_enable_auto_assignment = json_object_new_boolean(setting->enable_auto_assignment);
    jint_auto_config_type = json_object_new_int(setting->auto_config_type);
    jstr_start = json_object_new_string(setting->start);
    jstr_end = json_object_new_string(setting->end);
    jint_ra_life_time = json_object_new_int(setting->ra_life_time);
    jint_dhcpv6_life_time = json_object_new_int(setting->dhcpv6_life_time);
    jbool_is_ip_dynamic = json_object_new_boolean(setting->is_ip_dynamic);
    jstr_username = json_object_new_string(setting->username);
    jstr_password = json_object_new_string(setting->password);
    jstr_service = json_object_new_string(setting->service);
    jint_reconnect_mode = json_object_new_int(setting->reconnect_mode);
    jint_mtu = json_object_new_int(setting->mtu);
    jbool_enable_auto_dns = json_object_new_boolean(setting->enable_auto_dns);
    jbool_enable_dhcpPD = json_object_new_boolean(setting->enable_dhcpPD);
    jbool_6rd_type = json_object_new_boolean(setting->_6rd_type);
    jstr_prefix = json_object_new_string(setting->prefix);
    jint_mask_length = json_object_new_int(setting->mask_length);
    jstr_relay = json_object_new_string(setting->relay);
    jstr_wan_link_local = json_object_new_string(setting->wan_link_local);
    jstr_lan_link_local = json_object_new_string(setting->lan_link_local);
    jstr_ipv4_addr = json_object_new_string(setting->ipv4_addr);

    switch(setting->type_index)
    {
        case IPV6_WAN_STATIC:
            json_object_object_add(jobj, WAN_USE_LINK_LOCAL_STR,         jstr_null);
            json_object_object_add(jobj, WAN_IPV6_ADDRESS_STR,           jstr_ipv6_addr);
            json_object_object_add(jobj, WAN_PREFIX_LENGTH_STR,          jint_prefix_length);
            json_object_object_add(jobj, WAN_DEFAULT_GATEWAY_STR,        jstr_default_gw);
            json_object_object_add(jobj, WAN_PRIMARY_DNS_STR,            jstr_primary_dns);
            json_object_object_add(jobj, WAN_SECONDARY_DNS_STR,          jstr_secondary_dns);
            json_object_object_add(jobj, WAN_LAN_IPV6_ADDRESS_STR,       jstr_lan_ipv6_addr);
            json_object_object_add(jobj, WAN_ENABLE_AUTO_ASSIGNMENT_STR, jbool_enable_auto_assignment);
            json_object_object_add(jobj, WAN_AUTO_CONFIG_TYPE_STR,       jint_auto_config_type);
            if(IPV6_WAN_AUTO_CONFIG_STATEFUL_DHCPV6 == setting->auto_config_type)
            {
                json_object_object_add(jobj, WAN_IP_START_STR,               jstr_start);
                json_object_object_add(jobj, WAN_IP_END_STR,                 jstr_end);
                json_object_object_add(jobj, WAN_DHCPV6_LIFETIME_STR,        jint_dhcpv6_life_time);
            }
            else
            {
                json_object_object_add(jobj, WAN_IP_START_STR,               jstr_null);
                json_object_object_add(jobj, WAN_IP_END_STR,                 jstr_null);
                json_object_object_add(jobj, WAN_DHCPV6_LIFETIME_STR,        jstr_null);
            }
            if(IPV6_WAN_AUTO_CONFIG_STATEFUL_DHCPV6 != setting->auto_config_type)
            {
                json_object_object_add(jobj, WAN_RA_LIFETIME_STR,            jint_ra_life_time);
            }
            else
            {
                json_object_object_add(jobj, WAN_RA_LIFETIME_STR,            jstr_null);
            }
            json_object_object_add(jobj, WAN_IS_IP_DYNAMIC_STR,          jstr_null);
            json_object_object_add(jobj, WAN_USERNAME_STR,               jstr_null);
            json_object_object_add(jobj, WAN_PASSWORD_STR,               jstr_null);
            json_object_object_add(jobj, WAN_SERVICE_STR,                jstr_null);
            json_object_object_add(jobj, WAN_RECONNECT_MODE_STR,         jstr_null);
            json_object_object_add(jobj, WAN_MTU_STR,                    jstr_null);
            json_object_object_add(jobj, WAN_ENABLE_AUTO_DNS_STR,        jstr_null);
            json_object_object_add(jobj, WAN_ENABLE_DHCPPD_STR,          jstr_null);
            json_object_object_add(jobj, WAN_6RD_TYPE_STR,               jstr_null);
            json_object_object_add(jobj, WAN_PREFIX_STR,                 jstr_null);
            json_object_object_add(jobj, WAN_MASK_LENGTH_STR,            jstr_null);
            json_object_object_add(jobj, WAN_RELAY_STR,                  jstr_null);
            json_object_object_add(jobj, WAN_WAN_LINK_LOCAL_STR,         jstr_null);
            json_object_object_add(jobj, WAN_LAN_LINK_LOCAL_STR,         jstr_lan_link_local);
            json_object_object_add(jobj, WAN_IPV4_ADDRESS_STR,           jstr_null);
            break;
        case IPV6_WAN_PPPOE:
            json_object_object_add(jobj, WAN_USE_LINK_LOCAL_STR,         jstr_null);
            json_object_object_add(jobj, WAN_IPV6_ADDRESS_STR,           jstr_ipv6_addr);
            json_object_object_add(jobj, WAN_PREFIX_LENGTH_STR,          jstr_null);
            json_object_object_add(jobj, WAN_DEFAULT_GATEWAY_STR,        jstr_null);
            json_object_object_add(jobj, WAN_PRIMARY_DNS_STR,            jstr_primary_dns);
            json_object_object_add(jobj, WAN_SECONDARY_DNS_STR,          jstr_secondary_dns);
            json_object_object_add(jobj, WAN_LAN_IPV6_ADDRESS_STR,       jstr_lan_ipv6_addr);
            json_object_object_add(jobj, WAN_ENABLE_AUTO_ASSIGNMENT_STR, jbool_enable_auto_assignment);
            json_object_object_add(jobj, WAN_AUTO_CONFIG_TYPE_STR,       jint_auto_config_type);
            if(IPV6_WAN_AUTO_CONFIG_STATEFUL_DHCPV6 == setting->auto_config_type)
            {
                json_object_object_add(jobj, WAN_IP_START_STR,               jstr_start);
                json_object_object_add(jobj, WAN_IP_END_STR,                 jstr_end);
                json_object_object_add(jobj, WAN_DHCPV6_LIFETIME_STR,        jint_dhcpv6_life_time);
            }
            else
            {
                json_object_object_add(jobj, WAN_IP_START_STR,               jstr_null);
                json_object_object_add(jobj, WAN_IP_END_STR,                 jstr_null);
                json_object_object_add(jobj, WAN_DHCPV6_LIFETIME_STR,        jstr_null);
            }
            if(IPV6_WAN_AUTO_CONFIG_STATEFUL_DHCPV6 != setting->auto_config_type)
            {
                json_object_object_add(jobj, WAN_RA_LIFETIME_STR,            jint_ra_life_time);
            }
            else
            {
                json_object_object_add(jobj, WAN_RA_LIFETIME_STR,            jstr_null);
            }
            json_object_object_add(jobj, WAN_IS_IP_DYNAMIC_STR,          jstr_null);
            json_object_object_add(jobj, WAN_USERNAME_STR,               jstr_username);
            json_object_object_add(jobj, WAN_PASSWORD_STR,               jstr_password);
            json_object_object_add(jobj, WAN_SERVICE_STR,                jstr_service);
            json_object_object_add(jobj, WAN_RECONNECT_MODE_STR,         jstr_null);
            json_object_object_add(jobj, WAN_MTU_STR,                    jint_mtu);
            json_object_object_add(jobj, WAN_ENABLE_AUTO_DNS_STR,        jbool_enable_auto_dns);
            json_object_object_add(jobj, WAN_ENABLE_DHCPPD_STR,          jbool_enable_dhcpPD);
            json_object_object_add(jobj, WAN_6RD_TYPE_STR,               jstr_null);
            json_object_object_add(jobj, WAN_PREFIX_STR,                 jstr_null);
            json_object_object_add(jobj, WAN_MASK_LENGTH_STR,            jstr_null);
            json_object_object_add(jobj, WAN_RELAY_STR,                  jstr_null);
            json_object_object_add(jobj, WAN_WAN_LINK_LOCAL_STR,         jstr_null);
            json_object_object_add(jobj, WAN_LAN_LINK_LOCAL_STR,         jstr_lan_link_local);
            json_object_object_add(jobj, WAN_IPV4_ADDRESS_STR,           jstr_null);
            break;
        case IPV6_WAN_AUTOCONFIGURATION:
            json_object_object_add(jobj, WAN_USE_LINK_LOCAL_STR,         jstr_null);
            json_object_object_add(jobj, WAN_IPV6_ADDRESS_STR,           jstr_null);
            json_object_object_add(jobj, WAN_PREFIX_LENGTH_STR,          jstr_null);
            json_object_object_add(jobj, WAN_DEFAULT_GATEWAY_STR,        jstr_null);
            json_object_object_add(jobj, WAN_PRIMARY_DNS_STR,            jstr_primary_dns);
            json_object_object_add(jobj, WAN_SECONDARY_DNS_STR,          jstr_secondary_dns);
            json_object_object_add(jobj, WAN_LAN_IPV6_ADDRESS_STR,       jstr_lan_ipv6_addr);
            json_object_object_add(jobj, WAN_ENABLE_AUTO_ASSIGNMENT_STR, jbool_enable_auto_assignment);
            json_object_object_add(jobj, WAN_AUTO_CONFIG_TYPE_STR,       jint_auto_config_type);
            json_object_object_add(jobj, WAN_IP_START_STR,               jstr_start);
            json_object_object_add(jobj, WAN_IP_END_STR,                 jstr_end);
            json_object_object_add(jobj, WAN_RA_LIFETIME_STR,            jint_ra_life_time);
            json_object_object_add(jobj, WAN_DHCPV6_LIFETIME_STR,        jint_dhcpv6_life_time);
            json_object_object_add(jobj, WAN_IS_IP_DYNAMIC_STR,          jstr_null);
            json_object_object_add(jobj, WAN_USERNAME_STR,               jstr_null);
            json_object_object_add(jobj, WAN_PASSWORD_STR,               jstr_null);
            json_object_object_add(jobj, WAN_SERVICE_STR,                jstr_null);
            json_object_object_add(jobj, WAN_RECONNECT_MODE_STR,         jstr_null);
            json_object_object_add(jobj, WAN_MTU_STR,                    jstr_null);
            json_object_object_add(jobj, WAN_ENABLE_AUTO_DNS_STR,        jbool_enable_auto_dns);
            json_object_object_add(jobj, WAN_ENABLE_DHCPPD_STR,          jbool_enable_dhcpPD);
            json_object_object_add(jobj, WAN_6RD_TYPE_STR,               jstr_null);
            json_object_object_add(jobj, WAN_PREFIX_STR,                 jstr_null);
            json_object_object_add(jobj, WAN_MASK_LENGTH_STR,            jstr_null);
            json_object_object_add(jobj, WAN_RELAY_STR,                  jstr_null);
            json_object_object_add(jobj, WAN_WAN_LINK_LOCAL_STR,         jstr_null);
            json_object_object_add(jobj, WAN_LAN_LINK_LOCAL_STR,         jstr_lan_link_local);
            json_object_object_add(jobj, WAN_IPV4_ADDRESS_STR,           jstr_null);
            break;
        case IPV6_WAN_6RD:
            json_object_object_add(jobj, WAN_USE_LINK_LOCAL_STR,         jstr_null);
            json_object_object_add(jobj, WAN_IPV6_ADDRESS_STR,           jstr_null);
            json_object_object_add(jobj, WAN_PREFIX_LENGTH_STR,          jint_prefix_length);
            json_object_object_add(jobj, WAN_DEFAULT_GATEWAY_STR,        jstr_null);
            json_object_object_add(jobj, WAN_PRIMARY_DNS_STR,            jstr_primary_dns);
            json_object_object_add(jobj, WAN_SECONDARY_DNS_STR,          jstr_secondary_dns);
            json_object_object_add(jobj, WAN_LAN_IPV6_ADDRESS_STR,       jstr_null);
            json_object_object_add(jobj, WAN_ENABLE_AUTO_ASSIGNMENT_STR, jbool_enable_auto_assignment);
            json_object_object_add(jobj, WAN_AUTO_CONFIG_TYPE_STR,       jint_auto_config_type);
            json_object_object_add(jobj, WAN_IP_START_STR,               jstr_start);
            json_object_object_add(jobj, WAN_IP_END_STR,                 jstr_end);
            json_object_object_add(jobj, WAN_RA_LIFETIME_STR,            jint_ra_life_time);
            json_object_object_add(jobj, WAN_DHCPV6_LIFETIME_STR,        jint_dhcpv6_life_time);
            json_object_object_add(jobj, WAN_IS_IP_DYNAMIC_STR,          jstr_null);
            json_object_object_add(jobj, WAN_USERNAME_STR,               jstr_null);
            json_object_object_add(jobj, WAN_PASSWORD_STR,               jstr_null);
            json_object_object_add(jobj, WAN_SERVICE_STR,                jstr_null);
            json_object_object_add(jobj, WAN_RECONNECT_MODE_STR,         jstr_null);
            json_object_object_add(jobj, WAN_MTU_STR,                    jstr_null);
            json_object_object_add(jobj, WAN_ENABLE_AUTO_DNS_STR,        jstr_null);
            json_object_object_add(jobj, WAN_ENABLE_DHCPPD_STR,          jstr_null);
            json_object_object_add(jobj, WAN_6RD_TYPE_STR,               jstr_null);
            json_object_object_add(jobj, WAN_PREFIX_STR,                 jstr_prefix);
            json_object_object_add(jobj, WAN_MASK_LENGTH_STR,            jint_mask_length);
            json_object_object_add(jobj, WAN_RELAY_STR,                  jstr_relay);
            json_object_object_add(jobj, WAN_WAN_LINK_LOCAL_STR,         jstr_null);
            json_object_object_add(jobj, WAN_LAN_LINK_LOCAL_STR,         jstr_lan_link_local);
            json_object_object_add(jobj, WAN_IPV4_ADDRESS_STR,           jstr_ipv4_addr);
            break;
        case IPV6_WAN_LINK_LOCAL_ONLY:
            json_object_object_add(jobj, WAN_USE_LINK_LOCAL_STR,         jstr_null);
            json_object_object_add(jobj, WAN_IPV6_ADDRESS_STR,           jstr_null);
            json_object_object_add(jobj, WAN_PREFIX_LENGTH_STR,          jstr_null);
            json_object_object_add(jobj, WAN_DEFAULT_GATEWAY_STR,        jstr_null);
            json_object_object_add(jobj, WAN_PRIMARY_DNS_STR,            jstr_null);
            json_object_object_add(jobj, WAN_SECONDARY_DNS_STR,          jstr_null);
            json_object_object_add(jobj, WAN_LAN_IPV6_ADDRESS_STR,       jstr_null);
            json_object_object_add(jobj, WAN_ENABLE_AUTO_ASSIGNMENT_STR, jstr_null);
            json_object_object_add(jobj, WAN_AUTO_CONFIG_TYPE_STR,       jstr_null);
            json_object_object_add(jobj, WAN_IP_START_STR,               jstr_null);
            json_object_object_add(jobj, WAN_IP_END_STR,                 jstr_null);
            json_object_object_add(jobj, WAN_RA_LIFETIME_STR,            jstr_null);
            json_object_object_add(jobj, WAN_DHCPV6_LIFETIME_STR,        jstr_null);
            json_object_object_add(jobj, WAN_IS_IP_DYNAMIC_STR,          jstr_null);
            json_object_object_add(jobj, WAN_USERNAME_STR,               jstr_null);
            json_object_object_add(jobj, WAN_PASSWORD_STR,               jstr_null);
            json_object_object_add(jobj, WAN_SERVICE_STR,                jstr_null);
            json_object_object_add(jobj, WAN_RECONNECT_MODE_STR,         jstr_null);
            json_object_object_add(jobj, WAN_MTU_STR,                    jstr_null);
            json_object_object_add(jobj, WAN_ENABLE_AUTO_DNS_STR,        jstr_null);
            json_object_object_add(jobj, WAN_ENABLE_DHCPPD_STR,          jstr_null);
            json_object_object_add(jobj, WAN_6RD_TYPE_STR,               jstr_null);
            json_object_object_add(jobj, WAN_PREFIX_STR,                 jstr_null);
            json_object_object_add(jobj, WAN_MASK_LENGTH_STR,            jstr_null);
            json_object_object_add(jobj, WAN_RELAY_STR,                  jstr_null);
            json_object_object_add(jobj, WAN_WAN_LINK_LOCAL_STR,         jstr_wan_link_local);
            json_object_object_add(jobj, WAN_LAN_LINK_LOCAL_STR,         jstr_lan_link_local);
            json_object_object_add(jobj, WAN_IPV4_ADDRESS_STR,           jstr_null);
            break;
        default:
            break;
    }

    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jstr_null);
    json_object_put(jstr_result);
    json_object_put(jstr_type);
    json_object_put(jbool_use_link_local);
    json_object_put(jstr_ipv6_addr);
    json_object_put(jint_prefix_length);
    json_object_put(jstr_default_gw);
    json_object_put(jstr_primary_dns);
    json_object_put(jstr_secondary_dns);
    json_object_put(jstr_lan_ipv6_addr);
    json_object_put(jbool_enable_auto_assignment);
    json_object_put(jint_auto_config_type);
    json_object_put(jstr_start);
    json_object_put(jstr_end);
    json_object_put(jint_ra_life_time);
    json_object_put(jint_dhcpv6_life_time);
    json_object_put(jbool_is_ip_dynamic);
    json_object_put(jstr_username);
    json_object_put(jstr_password);
    json_object_put(jstr_service);
    json_object_put(jint_reconnect_mode);
    json_object_put(jint_mtu);
    json_object_put(jbool_enable_auto_dns);
    json_object_put(jbool_enable_dhcpPD);
    json_object_put(jbool_6rd_type);
    json_object_put(jstr_prefix);
    json_object_put(jint_mask_length);
    json_object_put(jstr_relay);
    json_object_put(jstr_lan_link_local);
    json_object_put(jstr_wan_link_local);
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    parse_ipv6_wan_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_ipv6_wan_settings_json_cb(char *query_str, WAN_IPV6_SETTINGS_T *settings, char **return_str)
{
    bool result, is_jobj;
    struct json_object *jobj;
    struct json_object *jobj_type;
    struct json_object *jobj_ipv6_addr;
    struct json_object *jobj_prefix_length;
    struct json_object *jobj_default_gw;
    struct json_object *jobj_primary_dns;
    struct json_object *jobj_secondary_dns;
    struct json_object *jobj_lan_ipv6_addr;
    struct json_object *jobj_auto_config_type;
    struct json_object *jobj_start;
    struct json_object *jobj_end;
    struct json_object *jobj_ra_life_time;
    struct json_object *jobj_dhcpv6_life_time;
    struct json_object *jobj_username;
    struct json_object *jobj_password;
    struct json_object *jobj_service;
    struct json_object *jobj_reconnect_mode;
    struct json_object *jobj_mtu;
    struct json_object *jobj_prefix;
    struct json_object *jobj_mask_length;
    struct json_object *jobj_relay;

    result = TRUE;
    is_jobj = FALSE;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_type = json_object_object_get(jobj, WAN_TYPE_STR)))
            {
                sprintf(settings->type, "%s", json_object_get_string(jobj_type));

                /* Free obj */
                json_object_put(jobj_type);
            }
            else
            {
                *return_str = ERROR_STR;
                result = FALSE;
                goto out;
            }

            result = senao_json_object_get_boolean(jobj, WAN_USE_LINK_LOCAL_STR, &(settings->use_link_local));
            if(FALSE == result) goto out;

            if((jobj_ipv6_addr = json_object_object_get(jobj, WAN_IPV6_ADDRESS_STR)))
            {
                sprintf(settings->ipv6_addr, "%s", json_object_get_string(jobj_ipv6_addr));

                /* Free obj */
                json_object_put(jobj_ipv6_addr);
            }
            else
            {
                *return_str = ERROR_STR;
                result = FALSE;
                goto out;
            }

            if((jobj_prefix_length = json_object_object_get(jobj, WAN_PREFIX_LENGTH_STR)))
            {
                settings->prefix_length = json_object_get_int(jobj_prefix_length);

                /* Free obj */
                json_object_put(jobj_prefix_length);
            }
            else
            {
                *return_str = ERROR_STR;
                result = FALSE;
                goto out;
            }

            if((jobj_default_gw = json_object_object_get(jobj, WAN_DEFAULT_GATEWAY_STR)))
            {
                sprintf(settings->default_gw, "%s", json_object_get_string(jobj_default_gw));

                /* Free obj */
                json_object_put(jobj_default_gw);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_primary_dns = json_object_object_get(jobj, WAN_PRIMARY_DNS_STR)))
            {
                sprintf(settings->primary_dns, "%s", json_object_get_string(jobj_primary_dns));

                /* Free obj */
                json_object_put(jobj_primary_dns);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_secondary_dns = json_object_object_get(jobj, WAN_SECONDARY_DNS_STR)))
            {
                sprintf(settings->secondary_dns, "%s", json_object_get_string(jobj_secondary_dns));

                /* Free obj */
                json_object_put(jobj_secondary_dns);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_lan_ipv6_addr = json_object_object_get(jobj, WAN_LAN_IPV6_ADDRESS_STR)))
            {
                sprintf(settings->lan_ipv6_addr, "%s", json_object_get_string(jobj_lan_ipv6_addr));

                /* Free obj */
                json_object_put(jobj_lan_ipv6_addr);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            result = senao_json_object_get_boolean(jobj, WAN_ENABLE_AUTO_ASSIGNMENT_STR, &(settings->enable_auto_assignment));
            if(FALSE == result) goto out;


            if((jobj_auto_config_type = json_object_object_get(jobj, WAN_AUTO_CONFIG_TYPE_STR)))
            {
                settings->auto_config_type = json_object_get_int(jobj_auto_config_type);

                /* Free obj */
                json_object_put(jobj_auto_config_type);
            }
            else
            {
                *return_str = ERROR_STR;
                result = FALSE;
                goto out;
            }

            if((jobj_start = json_object_object_get(jobj, WAN_IP_START_STR)))
            {
                sprintf(settings->start, "%s", json_object_get_string(jobj_start));

                /* Free obj */
                json_object_put(jobj_start);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_end = json_object_object_get(jobj, WAN_IP_END_STR)))
            {
                sprintf(settings->end, "%s", json_object_get_string(jobj_end));

                /* Free obj */
                json_object_put(jobj_end);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_ra_life_time = json_object_object_get(jobj, WAN_RA_LIFETIME_STR)))
            {
                settings->ra_life_time = json_object_get_int(jobj_ra_life_time);

                /* Free obj */
                json_object_put(jobj_ra_life_time);
            }
            else
            {
                *return_str = ERROR_STR;
                result = FALSE;
                goto out;
            }

            if((jobj_dhcpv6_life_time = json_object_object_get(jobj, WAN_DHCPV6_LIFETIME_STR)))
            {
                settings->dhcpv6_life_time = json_object_get_int(jobj_dhcpv6_life_time);

                /* Free obj */
                json_object_put(jobj_dhcpv6_life_time);
            }
            else
            {
                *return_str = ERROR_STR;
                result = FALSE;
                goto out;
            }

            result = senao_json_object_get_boolean(jobj, WAN_IS_IP_DYNAMIC_STR, &(settings->is_ip_dynamic));
            if(FALSE == result) goto out;

            if((jobj_username = json_object_object_get(jobj, WAN_USERNAME_STR)))
            {
                sprintf(settings->username, "%s", json_object_get_string(jobj_username));

                /* Free obj */
                json_object_put(jobj_username);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_password = json_object_object_get(jobj, WAN_PASSWORD_STR)))
            {
                sprintf(settings->password, "%s", json_object_get_string(jobj_password));

                /* Free obj */
                json_object_put(jobj_password);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_service = json_object_object_get(jobj, WAN_SERVICE_STR)))
            {
                sprintf(settings->service, "%s", json_object_get_string(jobj_service));

                /* Free obj */
                json_object_put(jobj_service);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_reconnect_mode = json_object_object_get(jobj, WAN_RECONNECT_MODE_STR)))
            {
                settings->reconnect_mode = json_object_get_int(jobj_reconnect_mode);

                /* Free obj */
                json_object_put(jobj_reconnect_mode);
            }
            else
            {
                *return_str = ERROR_STR;
                result = FALSE;
                goto out;
            }

            if((jobj_mtu = json_object_object_get(jobj, WAN_MTU_STR)))
            {
                settings->mtu = json_object_get_int(jobj_mtu);

                /* Free obj */
                json_object_put(jobj_mtu);
            }
            else
            {
                *return_str = ERROR_STR;
                result = FALSE;
                goto out;
            }

            result = senao_json_object_get_boolean(jobj, WAN_ENABLE_AUTO_DNS_STR, &(settings->enable_auto_dns));
            if(FALSE == result) goto out;

            result = senao_json_object_get_boolean(jobj, WAN_ENABLE_DHCPPD_STR, &(settings->enable_dhcpPD));
            if(FALSE == result) goto out;

            result = senao_json_object_get_boolean(jobj, WAN_6RD_TYPE_STR, &(settings->_6rd_type));
            if(FALSE == result) goto out;

            if((jobj_prefix = json_object_object_get(jobj, WAN_PREFIX_STR)))
            {
                sprintf(settings->prefix, "%s", json_object_get_string(jobj_prefix));

                /* Free obj */
                json_object_put(jobj_prefix);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_mask_length = json_object_object_get(jobj, WAN_MASK_LENGTH_STR)))
            {
                settings->mask_length = json_object_get_int(jobj_mask_length);

                /* Free obj */
                json_object_put(jobj_mask_length);
            }
            else
            {
                *return_str = ERROR_STR;
                result = FALSE;
                goto out;
            }

            if((jobj_relay = json_object_object_get(jobj, WAN_RELAY_STR)))
            {
                sprintf(settings->relay, "%s", json_object_get_string(jobj_relay));

                /* Free obj */
                json_object_put(jobj_relay);
            }
            else
            {
                result = FALSE;
                goto out;
            }
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}
#endif
#endif

#if HAS_AP
/*****************************************************************
* NAME:    get_ap_wan_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_ap_wan_settings_json_cb(HTTPS_CB *pkt,AP_WAN_SETTINGS_T *setting,char *result)
{
    int i;
    struct json_object *jobj,*jstr_dns;
    char buf[32]={0};
    char api_result[64];

    if(NULL == pkt)
        return -1;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));
    json_object_object_add(jobj, "WanType", json_object_new_string(setting->type));
    json_object_object_add(jobj, "Username", json_object_new_string(setting->username));
    json_object_object_add(jobj, "Password", json_object_new_string(setting->password));
    json_object_object_add(jobj, "ConnectionType", json_object_new_int(setting->connection_type));

#if HAS_WAN_AUTO_DETECTION
	api_get_string_option("mesh.wifi.autodetect", buf, sizeof(buf));
	json_object_object_add(jobj, "AutoDetection", json_object_new_int(atoi(buf)));

    api_get_string_option((atoi(buf) == 1)?"system.@system[0].opmode":"mesh.wifi.man_opmode", buf, sizeof(buf));
    if (strcmp(buf, "ap")==0)
    {
        json_object_object_add(jobj, "RoutingMode", json_object_new_int(1));
    }
    else
    {
        json_object_object_add(jobj, "RoutingMode", json_object_new_int(0));
    }
#endif

    json_object_object_add(jobj, "IPAddress", json_object_new_string(setting->ip_address));
    json_object_object_add(jobj, "SubnetMask", json_object_new_string(setting->subnet_mask));
    if(strcmp(setting->type,"PPTP")==0 || strcmp(setting->type,"L2TP")==0)
    {
        json_object_object_add(jobj, "Gateway", json_object_new_string(setting->gui_gw));

        json_object_object_add(jobj, "ServerGatewayOpt", json_object_new_string(setting->servergatewayOpt));
        json_object_object_add(jobj, "IPAddressOpt", json_object_new_string(setting->gui_ipaddr_opt));
        json_object_object_add(jobj, "SubnetMaskOpt", json_object_new_string(setting->gui_netmaskOpt));
        json_object_object_add(jobj, "GatewayOpt", json_object_new_string(setting->gui_gwOpt));
    }
    else
    {
        json_object_object_add(jobj, "Gateway", json_object_new_string(setting->gateway));
    }

    jstr_dns = json_object_new_object();
    json_object_object_add(jstr_dns, "Primary",json_object_new_string(setting->dns_primary));
    json_object_object_add(jstr_dns, "Secondary",json_object_new_string(setting->dns_secondary));
    json_object_object_add(jobj, "DNS", jstr_dns);

    json_object_object_add(jobj, "IPType", json_object_new_string(setting->iptype));
    json_object_object_add(jobj, "ServerGateway", json_object_new_string(setting->gateway));

    sysutil_interact(buf, sizeof(buf), "ifconfig eth0 |grep \"inet addr\" |awk -F \":\" {'print $2'} | awk -F \" \" {'print $1'} | tr -d \"\n\"");
    json_object_object_add(jobj, "WANIPAddress", json_object_new_string(buf));


    /* Store packet content into buffer and send it out */
	http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
	http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));
	http_send_stored_data(pkt->fd);
	/* Free obj */
	json_object_put(jobj);

	return 0;
}


/*****************************************************************
* NAME:    set_ap_wan_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT: 
* Author:   
* Modify:   
******************************************************************/
int set_ap_wan_settings_json_cb(char *query_str,AP_WAN_SETTINGS_T *setting, char *result_str)
{
    bool result=TRUE;
    struct json_object *jobj,*jobj_dns;
	char *return_str=OK_STR,type[10]={0};

	if(NULL == query_str)
		return FALSE;
	if(jobj = json_tokener_parse(query_str))
    {
		if(FALSE == senao_json_object_get_string(jobj, "WanType",type))
		{
			return_str = ERROR_STR;
			result=FALSE;
		}
		else
		{
            if(strcmp(type, "Static IP")==0)
				strcpy(setting->type, "static");
            else if(strcmp(type, "PPTP")==0)
                strcpy(setting->type, "pptp");
            else if(strcmp(type, "L2TP")==0)
                strcpy(setting->type, "l2tp");
            else if(strcmp(type, "PPPoE")==0)
                strcpy(setting->type, "pppoe");
			 else
				strcpy(setting->type, "dhcp");
		}
		if(FALSE == senao_json_object_get_string(jobj, "IPAddress",setting->ip_address))
		{
			return_str = ERROR_IP_ADDRESS_STR;
			result=FALSE;
		}
		if(FALSE == senao_json_object_get_string(jobj, "SubnetMask",setting->subnet_mask))
		{
			return_str = ERROR_SUBNET_MASK_STR;
			result=FALSE;
		}
		if(FALSE == senao_json_object_get_string(jobj, "Gateway",setting->gateway))
		{
			return_str = ERROR_GATEWAY_STR;
			result=FALSE;
		}
        if(FALSE == senao_json_object_get_string(jobj, "Username",setting->username))
		{
			return_str = ERROR_USER_NAME_STR;
			result=FALSE;
		}
        if(FALSE == senao_json_object_get_string(jobj, "Password",setting->password))
		{
			return_str = ERROR_PASSWORD_STR;
			result=FALSE;
		}
        senao_json_object_get_string(jobj, "IPAddressType",setting->iptype);
        senao_json_object_get_string(jobj, "ServerGateway",setting->servergateway);
		senao_json_object_get_integer(jobj, "RoutingMode",&(setting->opmode));
		senao_json_object_get_integer(jobj, "ConnectionType",&(setting->connection_type));
		if(jobj_dns = json_object_object_get(jobj,"DNS"))
        {
			// DNS may be empty.
			senao_json_object_get_string(jobj_dns, "Primary",setting->dns_primary);
			senao_json_object_get_string(jobj_dns, "Secondary",setting->dns_secondary);
		}
		/* Free obj */
		json_object_put(jobj);
		json_object_put(jobj_dns);
	}
	return result;
}

#endif
/*-------------------------------------------------------------------------------*/

/*------------------------------- LAN SETTING -------------------------------*/
/*****************************************************************
* NAME:    get_lan_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_lan_settings_json_cb(HTTPS_CB *pkt, ROUTER_LAN_SETTINGS_T *setting, char *result)
{
    struct json_object *jobj;

    if(NULL == pkt)
    {
        return -1;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    json_object_object_add(jobj, "GetLanSettingsResult", json_object_new_string(result));
    json_object_object_add(jobj, "IPAddress", json_object_new_string(setting->router_ip_address));
    json_object_object_add(jobj, "SubnetMask", json_object_new_string(setting->router_subnet_mask));
    json_object_object_add(jobj, "DHCPServerEnabled", json_object_new_boolean(setting->dhcp_server_enabled));
    json_object_object_add(jobj, "DHCPLeaseTime", json_object_new_string(setting->dhcp_leasetime));
    json_object_object_add(jobj, "DHCPStartIP", json_object_new_string(setting->dhcp_start));
    json_object_object_add(jobj, "DHCPEndIP", json_object_new_string(setting->dhcp_end));
    json_object_object_add(jobj, "DomainName", json_object_new_string(setting->domain_name));

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    parse_lan_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int parse_lan_settings_json_cb(char *query_str, ROUTER_LAN_SETTINGS_T *setting, char *result_str)
{
    bool result, is_jobj;
    struct json_object *jobj;
    struct json_object *jobj_ipaddress, *jobj_subnetmask, *jobj_dhcp_leasetime, *jobj_start, *jobj_end, *jobj_domain;
    bool dhcp_server_enabled = 0;

    result = TRUE;
    is_jobj = FALSE;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_ipaddress = json_object_object_get(jobj, "IPAddress")))
            {
                sprintf(setting->router_ip_address, "%s", json_object_get_string(jobj_ipaddress));

                /* Free obj */
                json_object_put(jobj_ipaddress);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_subnetmask = json_object_object_get(jobj, "SubnetMask")))
            {
                sprintf(setting->router_subnet_mask, "%s", json_object_get_string(jobj_subnetmask));

                /* Free obj */
                json_object_put(jobj_subnetmask);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            result = senao_json_object_get_boolean(jobj, "DHCPServerEnabled", (bool *)&dhcp_server_enabled);
            setting->dhcp_server_enabled = dhcp_server_enabled;
            #if APP_AGENT_SUPPORT_ENSHARE
            #else
            if((jobj_dhcp_leasetime = json_object_object_get(jobj, "DHCPLeaseTime")))
            {
                sprintf(setting->dhcp_leasetime, "%s", json_object_get_string(jobj_dhcp_leasetime));
                /* Free obj */
                json_object_put(jobj_dhcp_leasetime);
            }
            else
            {
                result = FALSE;
                goto out;
            }
            if((jobj_start = json_object_object_get(jobj, "DHCPStartIP")))
            {
                sprintf(setting->dhcp_start, "%s", json_object_get_string(jobj_start));
                /* Free obj */
                json_object_put(jobj_start);
            }
            else
            {
                result = FALSE;
                goto out;
            }
            if((jobj_end = json_object_object_get(jobj, "DHCPEndIP")))
            {
                sprintf(setting->dhcp_end, "%s", json_object_get_string(jobj_end));
                /* Free obj */
                json_object_put(jobj_end);
            }
            else
            {
                result = FALSE;
                goto out;
            }
            if((jobj_domain = json_object_object_get(jobj, "DomainName")))
            {
                if(strlen(json_object_get_string(jobj_domain)) >= sizeof(setting->domain_name)-1)
                {
                    result = FALSE;
                    goto out;
                }
                sprintf(setting->domain_name, "%s", json_object_get_string(jobj_domain));

                /* Free obj */
                json_object_put(jobj_domain);
            }
            else
            {
                result = FALSE;
                goto out;
            }
            #endif
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
* NAME:    get_lan_access_control_list_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
void get_lan_access_control_list_json_cb(HTTPS_CB *pkt, ACCESS_CONTROL_LIST_T *setting)
{
    int i;
    struct json_object *jobj;
    struct json_object *jarr_access_list, *jobj_access_client;

    jobj = json_object_new_object();

    json_object_object_add(jobj, "GetLanAccessControlListResult", json_object_new_string(OK_STR));
    json_object_object_add(jobj, "EnabledMacFilter", json_object_new_boolean(setting->enable_macfilter));
    json_object_object_add(jobj, "TrafficType", json_object_new_boolean(setting->traffic_type));

    jarr_access_list = json_object_new_array();

    for(i = 0; i < MAC_FILTER_NUM; i++)
    {
        jobj_access_client = json_object_new_object();

        if(setting->is_enable[i] == 0){
            continue;
        }

        json_object_object_add(jobj_access_client, "MacAddress", json_object_new_string(setting->mac[i]));
        json_object_object_add(jobj_access_client, "Comment", json_object_new_string(setting->comment[i]));

        json_object_array_add(jarr_access_list, jobj_access_client);
    }

    json_object_object_add(jobj, "ClientList", jarr_access_list);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj_access_client);
    json_object_put(jarr_access_list);
    json_object_put(jobj);
}

/*****************************************************************
* NAME:    parse_set_lan_access_control_list_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool parse_set_lan_access_control_list_json_cb(char *query_str, ACCESS_CONTROL_LIST_T *setting, char *result_str)
{
    bool result, is_jobj;
    struct json_object *jobj;

    result = TRUE;
    is_jobj = FALSE;

    /***************** Packet format *******************
     * {                                               *
     *   "EnabledMacFilter" : "boolean",               *
     *   "TrafficType" : "boolean"                     * 
     * }                                               * 
     ***************************************************/

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            result = senao_json_object_get_boolean(jobj, "EnabledMacFilter", &(setting->enable_macfilter));
            if(FALSE == result) goto out;

            result = senao_json_object_get_boolean(jobj, "TrafficType", &(setting->traffic_type));
            if(FALSE == result) goto out;
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
* NAME:    parse_add_lan_access_control_list_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool parse_add_lan_access_control_list_json_cb(char *query_str, char *mac, char *comment, char *result_str)
{
    bool result, is_jobj;
    struct json_object *jobj;
    struct json_object *jobj_add_mac, *jobj_comment;

    result = TRUE;
    is_jobj = FALSE;

    /***************** Packet format *******************
     * {                                               * 
     *   "MacAddress" : "string",                      *
     *   "Comment" : "string"                          *
     * }                                               *
     ***************************************************/

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_add_mac = json_object_object_get(jobj, "MacAddress")))
            {
                sprintf(mac, "%s", json_object_get_string(jobj_add_mac));

                /* Free obj */
                json_object_put(jobj_add_mac);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_comment = json_object_object_get(jobj, "Comment")))
            {
                sprintf(comment, "%s", json_object_get_string(jobj_comment));

                /* Free obj */
                json_object_put(jobj_comment);
            }
            else
            {
                result = FALSE;
                goto out;
            }
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
* NAME:    parse_del_lan_access_control_list_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool parse_del_lan_access_control_list_json_cb(char *query_str, char *del_action, char *result_str)
{
    bool result, is_jobj;
    struct json_object *jobj;
    struct json_object *jobj_del_mac;

    result = TRUE;
    is_jobj = FALSE;

    /***************** Packet format *******************
     * {                                               * 
     *   "DeleteMacAddress" : "string"                 *
     * }                                               *
     ***************************************************/

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_del_mac = json_object_object_get(jobj, "DeleteMacAddress")))
            {
                sprintf(del_action, "%s", json_object_get_string(jobj_del_mac));

                /* Free obj */
                json_object_put(jobj_del_mac);
            }
            else
            {
                result = FALSE;
                goto out;
            }
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
* NAME:    get_json_radio_id
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_json_radio_id(char *query_str)
{
    struct json_object *jobj, *jobj_radio_id;
    char *radio_id;
    int is5g;

    radio_id = NULL;
    is5g = -1;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            if((jobj_radio_id = json_object_object_get(jobj, "RadioID")))
            {
                if (0 == strcmp(WLAN_RADIO_2_4_GHZ_STR, json_object_get_string(jobj_radio_id)))
                {
                    is5g = 0;
                }
#if SUPPORT_WLAN_5G_SETTING
                else if(0 == strcmp(WLAN_RADIO_5_GHZ_STR, json_object_get_string(jobj_radio_id)))
                {
                    is5g = 1;
                }
#endif

                /* Free obj */
                json_object_put(jobj_radio_id);
            }

            /* Free obj */
            json_object_put(jobj);
        }
    }
    return is5g;
}

/*****************************************************************
* NAME: get_json_ssid_id
* ---------------------------------------------------------------
* FUNCTION: Get the SSID ID in query_str.
* INPUT:
* OUTPUT:
* Author:
* Modify:
****************************************************************/
int get_json_ssid_id(char *query_str)
{
    struct json_object *jobj, *jobj_ssid_id;
    char ssid_id_str[4];
    int ssid_id = -1;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            if((jobj_ssid_id = json_object_object_get(jobj, "SSIDID")))
            {
                ssid_id = json_object_get_int(jobj_ssid_id);
                /* Make ssid_id's beginning to 0 */
                ssid_id--;
                /* Free obj */
                json_object_put(jobj_ssid_id);
            }

            /* Free obj */
            json_object_put(jobj);
        }
    }
    return ssid_id;
}

/*****************************************************************
* NAME:    get_wlan_radios_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
void get_wlan_radios_json_cb(HTTPS_CB *pkt)
{
    int i, j;
    char buf[4]={0};
    char chan_list_2g[63+1]={0};
#if SUPPORT_WLAN_5G_SETTING
    char chan_list_5g[127+1]={0};
#endif
    char *ch_string;
    char *channel_i;

    FILE *fp = NULL;

    struct json_object *jobj;
    struct json_object *jobj_radio_infos, *jarr_radio_info, *jobj_radio_info;
    struct json_object *jarr_support_mode;
    struct json_object *jarr_channel;
    struct json_object *jobj_support_security, *jarr_security_info, *jobj_security_info;
    struct json_object *jarr_encryption;
    int wlan_encryption;
    int wlan_authentication;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();
    json_object_object_add(jobj, "GetWLanRadiosResult", json_object_new_string(OK_STR));

    jobj_radio_infos = json_object_new_object();
    jarr_radio_info = json_object_new_array();
    jobj_radio_info = json_object_new_object();
    json_object_object_add(jobj_radio_info, "RadioID", json_object_new_string(WLAN_RADIO_2_4_GHZ_STR));
    json_object_object_add(jobj_radio_info, "Frequency", json_object_new_int(2));
#ifdef WIFI_SSID_NUM    
    json_object_object_add(jobj_radio_info, "SSIDNum", json_object_new_int(WIFI_SSID_NUM));
#endif

    jarr_support_mode = json_object_new_array();
    for (i=0;i<T_NUM_OF_ELEMENTS(supported_mode_2_4_G);i++){
        json_object_array_add(jarr_support_mode, json_object_new_string(supported_mode_2_4_G[i]));
    }
    json_object_object_add(jobj_radio_info, "SupportedModes", jarr_support_mode);

    sysutil_get_channel_list(0, chan_list_2g, sizeof(chan_list_2g));
    jarr_channel = json_object_new_array();
    ch_string = strtok(chan_list_2g, ",");
    while(NULL != ch_string){
        channel_i = ch_string;
        json_object_array_add(jarr_channel, json_object_new_int(atoi(channel_i)));
        ch_string = strtok(NULL, ",");
    }
    json_object_object_add(jobj_radio_info, "Channels", jarr_channel);

    jobj_support_security = json_object_new_object();
    jarr_security_info = json_object_new_array();

    for (i=0;i<T_NUM_OF_ELEMENTS(security_wep_mode);i++){
        jobj_security_info = json_object_new_object();
        json_object_object_add(jobj_security_info, "SecurityType", json_object_new_string(security_wep_mode[i]));

        jarr_encryption = json_object_new_array();
        for (j=0;j<T_NUM_OF_ELEMENTS(wep_auth_type);j++){
            json_object_array_add(jarr_encryption, json_object_new_string(wep_auth_type[j]));
        }

        json_object_object_add(jobj_security_info, "Encryptions", jarr_encryption);
        json_object_array_add(jarr_security_info, jobj_security_info);
    }

    for (i=0;i<T_NUM_OF_ELEMENTS(security_wpa_mode);i++){
        get_wlan_authentication(security_wpa_mode[i], &wlan_authentication);
        jobj_security_info = json_object_new_object();
        json_object_object_add(jobj_security_info, "SecurityType", json_object_new_string(security_wpa_mode[i]));

        jarr_encryption = json_object_new_array();
        for (j=0;j<T_NUM_OF_ELEMENTS(cipher_type);j++){
            get_wlan_encryption(cipher_type[j], &wlan_encryption);
            if(is_valid_wlan_security(wlan_encryption, wlan_authentication)){
                json_object_array_add(jarr_encryption, json_object_new_string(cipher_type[j]));
            }
        }

        json_object_object_add(jobj_security_info, "Encryptions", jarr_encryption);
        json_object_array_add(jarr_security_info, jobj_security_info);
    }

    json_object_object_add(jobj_support_security, "SecurityInfo", jarr_security_info);
    json_object_object_add(jobj_radio_info, "SupportedSecurity", jobj_support_security);
    json_object_array_add(jarr_radio_info, jobj_radio_info);

#if SUPPORT_WLAN_5G_SETTING
    // 5G
    jobj_radio_info = json_object_new_object();
    json_object_object_add(jobj_radio_info, "RadioID", json_object_new_string(WLAN_RADIO_5_GHZ_STR));
    json_object_object_add(jobj_radio_info, "Frequency", json_object_new_int(5));
#ifdef WIFI_SSID_NUM
    json_object_object_add(jobj_radio_info, "SSIDNum", json_object_new_int(WIFI_SSID_NUM));
#endif
    jarr_support_mode = json_object_new_array();
    for (i=0;i<T_NUM_OF_ELEMENTS(supported_mode_5_G);i++){
        json_object_array_add(jarr_support_mode, json_object_new_string(supported_mode_5_G[i]));
    }
    json_object_object_add(jobj_radio_info, "SupportedModes", jarr_support_mode);

    sysutil_get_channel_list(1, chan_list_5g, sizeof(chan_list_5g));
    jarr_channel = json_object_new_array();
    ch_string = strtok(chan_list_5g, ",");
    while(NULL != ch_string){
        channel_i = ch_string;
        json_object_array_add(jarr_channel, json_object_new_int(atoi(channel_i)));
        ch_string = strtok(NULL, ",");
    }
    json_object_object_add(jobj_radio_info, "Channels", jarr_channel);

    jobj_support_security = json_object_new_object();
    jarr_security_info = json_object_new_array();
    for (i=0;i<T_NUM_OF_ELEMENTS(security_wep_mode);i++){
        jobj_security_info = json_object_new_object();

        json_object_object_add(jobj_security_info, "SecurityType", json_object_new_string(security_wep_mode[i]));

        jarr_encryption = json_object_new_array();
        for (j=0;j<T_NUM_OF_ELEMENTS(wep_auth_type);j++){
            json_object_array_add(jarr_encryption, json_object_new_string(wep_auth_type[j]));
        }

        json_object_object_add(jobj_security_info, "Encryptions", jarr_encryption);
        json_object_array_add(jarr_security_info, jobj_security_info);
    }

    for (i=0;i<T_NUM_OF_ELEMENTS(security_wpa_mode);i++){
        get_wlan_authentication(security_wpa_mode[i], &wlan_authentication);
        jobj_security_info = json_object_new_object();
        json_object_object_add(jobj_security_info, "SecurityType", json_object_new_string(security_wpa_mode[i]));

        jarr_encryption = json_object_new_array();
        for (j=0;j<T_NUM_OF_ELEMENTS(cipher_type);j++){
            get_wlan_encryption(cipher_type[j], &wlan_encryption);
            if(is_valid_wlan_security(wlan_encryption, wlan_authentication)){
                json_object_array_add(jarr_encryption, json_object_new_string(cipher_type[j]));
            }
        }
        json_object_object_add(jobj_security_info, "Encryptions", jarr_encryption);
        json_object_array_add(jarr_security_info, jobj_security_info);
    }

    json_object_object_add(jobj_support_security, "SecurityInfo", jarr_security_info);
    json_object_object_add(jobj_radio_info, "SupportedSecurity", jobj_support_security);
    json_object_array_add(jarr_radio_info, jobj_radio_info);
#endif

    json_object_object_add(jobj_radio_infos, "RadioInfo", jarr_radio_info);
    json_object_object_add(jobj, "RadioInfos", jobj_radio_infos);
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\r\n", json_object_to_json_string(jobj));
    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jarr_encryption);
    json_object_put(jobj_security_info);
    json_object_put(jarr_security_info);
    json_object_put(jobj_support_security);
    json_object_put(jarr_channel);
    json_object_put(jarr_support_mode);
    json_object_put(jobj_radio_info);
    json_object_put(jarr_radio_info);
    json_object_put(jobj_radio_infos);
    json_object_put(jobj);
}
/*****************************************************************
* NAME:    get_access_control_list_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
void get_access_control_list_json_cb(HTTPS_CB *pkt, ACCESS_CONTROL_LIST_T *setting)
{
    int i;
    struct json_object *jobj;
    struct json_object *jarr_access_list, *jobj_access_client;

    jobj = json_object_new_object();

    json_object_object_add(jobj, "GetAccessControlListResult", json_object_new_string(OK_STR));
    json_object_object_add(jobj, "EnabledMacFilter", json_object_new_boolean(setting->enable_macfilter));

    jarr_access_list = json_object_new_array();

    for(i = 0; i < MAC_FILTER_NUM; i++)
    {
        jobj_access_client = json_object_new_object();
        
        if(setting->is_enable[i] == 0){
            continue;
        } 
        json_object_object_add(jobj_access_client, "MacAddress", json_object_new_string(setting->mac[i]));
        json_object_object_add(jobj_access_client, "Comment", json_object_new_string(setting->comment[i]));

        json_object_array_add(jarr_access_list, jobj_access_client);
    }

    json_object_object_add(jobj, "ClientList", jarr_access_list);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj_access_client);
    json_object_put(jarr_access_list);
    json_object_put(jobj);
}

/*****************************************************************
* NAME:    parse_set_access_control_list_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool parse_set_access_control_list_json_cb(char *query_str, ACCESS_CONTROL_LIST_T *setting, char *result_str)
{
    bool result, is_jobj;
    struct json_object *jobj;

    result = TRUE;
    is_jobj = FALSE;

    /***************** Packet format *******************
     * {                                               *
     *   "EnabledMacFilter" : "boolean",               * 
     * }                                               * 
     ***************************************************/

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            result = senao_json_object_get_boolean(jobj, "EnabledMacFilter", &(setting->enable_macfilter));
            if(FALSE == result) goto out;
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}
/*****************************************************************
* NAME:    parse_add_access_control_list_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool parse_add_access_control_list_json_cb(char *query_str, char *mac, char *comment, char *result_str)
{
    bool result, is_jobj;
    struct json_object *jobj;
    struct json_object *jobj_add_mac, *jobj_comment;

    result = TRUE;
    is_jobj = FALSE;

    /***************** Packet format *******************
     * {                                               * 
     *   "MacAddress" : "string",                      *
     *   "Comment" : "string"                          *
     * }                                               *
     ***************************************************/

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_add_mac = json_object_object_get(jobj, "MacAddress")))
            {
                sprintf(mac, "%s", json_object_get_string(jobj_add_mac));

                /* Free obj */
                json_object_put(jobj_add_mac);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_comment = json_object_object_get(jobj, "Comment")))
            {
                sprintf(comment, "%s", json_object_get_string(jobj_comment));

                /* Free obj */
                json_object_put(jobj_comment);
            }
            else
            {
                result = FALSE;
                goto out;
            }
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}
/*****************************************************************
* NAME:    parse_del_access_control_list_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool parse_del_access_control_list_json_cb(char *query_str, char *del_action, char *result_str)
{
    bool result, is_jobj;
    struct json_object *jobj;
    struct json_object *jobj_del_mac;

    result = TRUE;
    is_jobj = FALSE;

    /***************** Packet format *******************
     * {                                               * 
     *   "DeleteMacAddress" : "string"                 *
     * }                                               *
     ***************************************************/

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_del_mac = json_object_object_get(jobj, "DeleteMacAddress")))
            {
                sprintf(del_action, "%s", json_object_get_string(jobj_del_mac));

                /* Free obj */
                json_object_put(jobj_del_mac);
            }
            else
            {
                result = FALSE;
                goto out;
            }
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}
/*****************************************************************
* NAME:    get_wlan_radio_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_wlan_radio_settings_json_cb(HTTPS_CB *pkt, WLAN_RADIO_SETTINGS_T *setting, char *result)
{
    struct json_object *jobj;

    if(NULL == pkt)
        return -1;

    jobj = json_object_new_object();
    json_object_object_add(jobj, "GetWLanRadioSettingsResult", json_object_new_string(result));
    json_object_object_add(jobj, "Enabled", json_object_new_boolean(setting->enabled));
    json_object_object_add(jobj, "Mode", json_object_new_string(setting->mode));
    json_object_object_add(jobj, "MacAddress", json_object_new_string((setting->mac)?setting->mac:""));
    json_object_object_add(jobj, "SSID", json_object_new_string(setting->ssid));
    json_object_object_add(jobj, "SSIDBroadcast", json_object_new_boolean(setting->ssid_broadcast));
    json_object_object_add(jobj, "ChannelWidth", json_object_new_int(setting->channel_width));
    json_object_object_add(jobj, "Channel", json_object_new_int(setting->channel));

    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    get_wlan_radio_security_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_wlan_radio_security_json_cb(HTTPS_CB *pkt, WLAN_RADIO_SECURITY_T *setting, char *result)
{
    struct json_object *jobj;
    int i, max;

    if(NULL == pkt)
    {
        return -1;
    }
    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    json_object_object_add(jobj, "GetWLanRadioSecurityResult", json_object_new_string(result));
    json_object_object_add(jobj, "Enabled", json_object_new_boolean(setting->enabled));

    /* initial all parameters to "NULL" */
    for(i = 0, max = T_NUM_OF_ELEMENTS(getWLANPara); i < max; i++){
        json_object_object_add(jobj, getWLANPara[i], json_object_new_string("NULL"));
    }

    if(setting->enabled == WLAN_ENC_NONE)
    {
            json_object_object_add(jobj, "Enable802.1x", json_object_new_boolean(setting->enable_8021x));
            json_object_object_add(jobj, "RadiusIP", json_object_new_string(setting->radius_ip1));
            json_object_object_add(jobj, "RadiusPort", json_object_new_int(setting->radius_port1));
            json_object_object_add(jobj, "RadiusPassword", json_object_new_string(setting->radius_secret1));
    }
    else
    {
        json_object_object_add(jobj, "Type", json_object_new_string(setting->type));
        json_object_object_add(jobj, "Encryption", json_object_new_string(setting->encryption));

        switch(setting->typeIdx)
        {
        case WLAN_AUTH_OPEN:
        case WLAN_AUTH_WEPAUTO:
        case WLAN_AUTH_SHARED:
            json_object_object_add(jobj, "Key", json_object_new_string(setting->key)); // Key for WEP, WPA, WPA2
            json_object_object_add(jobj, "Enable802.1x", json_object_new_boolean(setting->enable_8021x));
            json_object_object_add(jobj, "RadiusIP", json_object_new_string(setting->radius_ip1));
            json_object_object_add(jobj, "RadiusPort", json_object_new_int(setting->radius_port1));
            json_object_object_add(jobj, "RadiusPassword", json_object_new_string(setting->radius_secret1));
            break;

        case WLAN_AUTH_WPA:
        case WLAN_AUTH_WPA2:
        case WLAN_AUTH_WPA1WPA2:
            json_object_object_add(jobj, "RadiusIP", json_object_new_string(setting->radius_ip1));
            json_object_object_add(jobj, "RadiusPort", json_object_new_int(setting->radius_port1));
            json_object_object_add(jobj, "RadiusPassword", json_object_new_string(setting->radius_secret1));
            break;

        case WLAN_AUTH_WPAPSK:
        case WLAN_AUTH_WPA2PSK:
        case WLAN_AUTH_WPA1PSKWPA2PSK:
            json_object_object_add(jobj, "Key", json_object_new_string(setting->key)); // Key for WEP, WPA, WPA2
            break;
        }
    }

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));
    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    parse_json_wlan_radios
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool parse_json_wlan_radios(char *query_str, WLAN_RADIOS_T *settings, char *return_str)
{
    bool result, is_jobj;
    struct json_object *jobj, *jobj_radio_id, *jobj_ssid_id, *jobj_mode, *jobj_ssid, *jobj_channel_width, *jobj_channel;
    struct json_object *jobj_type, *jobj_encryption, *jobj_key;
    struct json_object *jobj_radius_ip, *jobj_radius_port, *jobj_radius_pw;
    int is5g = -1;
    int ssid_id;

    result = TRUE;
    is_jobj = FALSE;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_radio_id = json_object_object_get(jobj, "RadioID")))
            {
                if (0 == strcmp(WLAN_RADIO_2_4_GHZ_STR, json_object_get_string(jobj_radio_id)))
                {
                    is5g = 0;
                }
#if SUPPORT_WLAN_5G_SETTING
                else if(0 == strcmp(WLAN_RADIO_5_GHZ_STR, json_object_get_string(jobj_radio_id)))
                {
                    is5g = 1;
                }
#endif

                settings->radio_id = is5g;

                /* Free obj */
                json_object_put(jobj_radio_id);
            }

            if(-1 == is5g)
            {
                result = FALSE;
                goto out;
            }

            if((jobj_ssid_id = json_object_object_get(jobj, "SSIDID")))
            {
                ssid_id = json_object_get_int(jobj_ssid_id);

                /* Make ssid_id's beginning to 0 */
                ssid_id--;
                // 2.4GHZ SSID==1: main   SSID==2: GUEST     
                // 5GHZ   SSID==3: main   SSID==4: GUEST
#if SUPPORT_WLAN_5G_SETTING
                if(is5g)
                    ssid_id = ssid_id - (WIFI_SSID_NUM+1);
#endif

                if(ssid_id < 0 || ssid_id > 1)
                    ssid_id = -1;

                settings->ssid_id = ssid_id;

                /* Free obj */
                json_object_put(jobj_ssid_id);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            result = senao_json_object_get_boolean(jobj, "RadioEnabled", &(settings->radio_enabled));
            
            if(FALSE == result) goto out;

            if((jobj_mode = json_object_object_get(jobj, "Mode")))
            {
                sprintf(settings->mode, "%s", json_object_get_string(jobj_mode));

                /* Free obj */
                json_object_put(jobj_mode);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_ssid = json_object_object_get(jobj, "SSID")))
            {
                sprintf(settings->ssid, "%s", json_object_get_string(jobj_ssid));

                /* Free obj */
                json_object_put(jobj_ssid);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            result = senao_json_object_get_boolean(jobj, "SSIDBroadcast", &(settings->ssid_broadcast));
            if(FALSE == result) goto out;

            if((jobj_channel_width = json_object_object_get(jobj, "ChannelWidth")))
            {
                settings->channel_width = json_object_get_int(jobj_channel_width);

                /* Free obj */
                json_object_put(jobj_channel_width);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_channel = json_object_object_get(jobj, "Channel")))
            {
                settings->channel = json_object_get_int(jobj_channel);

                /* Free obj */
                json_object_put(jobj_channel);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            result = senao_json_object_get_boolean(jobj, "AuthenticationEnabled", &(settings->authentication_enabled));
            if(FALSE == result) goto out;

            if((jobj_type = json_object_object_get(jobj, "Type")))
            {
                sprintf(settings->type, "%s", json_object_get_string(jobj_type));

                /* Free obj */
                json_object_put(jobj_type);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_encryption = json_object_object_get(jobj, "Encryption")))
            {
                sprintf(settings->encryption, "%s", json_object_get_string(jobj_encryption));

                /* Free obj */
                json_object_put(jobj_encryption);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_key = json_object_object_get(jobj, "Key")))
            {
                sprintf(settings->key, "%s", json_object_get_string(jobj_key));

                /* Free obj */
                json_object_put(jobj_key);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            result = senao_json_object_get_boolean(jobj, "Enable802.1x", &(settings->enable_8021x));

            if((jobj_radius_ip = json_object_object_get(jobj, "RadiusIP")))
            {
                sprintf(settings->radius_ip1, "%s", json_object_get_string(jobj_radius_ip));

                /* Free obj */
                json_object_put(jobj_radius_ip);
            }
            else
            {
                result = FALSE;
                goto out;
            }
            if((jobj_radius_port = json_object_object_get(jobj, "RadiusPort")))
            {
                settings->radius_port1 = json_object_get_int(jobj_radius_port);

                /* Free obj */
                json_object_put(jobj_radius_port);
            }
            else
            {
                result = FALSE;
                goto out;
            }
            if((jobj_radius_pw = json_object_object_get(jobj, "RadiusPassword")))
            {
                sprintf(settings->radius_secret1, "%s", json_object_get_string(jobj_radius_pw));

                /* Free obj */
                json_object_put(jobj_radius_pw);
            }
            else
            {
                result = FALSE;
                goto out;
            }
        }
    }
#if 0
    printf("[%s][%d] settings->radio_id:[%d]\n", __FUNCTION__, __LINE__, settings->radio_id);
    printf("[%s][%d] settings->radio_enabled:[%u]\n", __FUNCTION__, __LINE__, settings->radio_enabled);
    printf("[%s][%d] settings->mode:[%s]\n", __FUNCTION__, __LINE__, settings->mode);
    printf("[%s][%d] settings->ssid:[%s]\n", __FUNCTION__, __LINE__, settings->ssid);
    printf("[%s][%d] settings->ssid_broadcast:[%u]\n", __FUNCTION__, __LINE__, settings->ssid_broadcast); 
    printf("[%s][%d] settings->channel_width:[%d]\n", __FUNCTION__, __LINE__, settings->channel_width);
    printf("[%s][%d] settings->channel:[%d]\n", __FUNCTION__, __LINE__, settings->channel);
    printf("[%s][%d] settings->authentication_enabled:[%u]\n", __FUNCTION__, __LINE__, settings->authentication_enabled);
    printf("[%s][%d] settings->type:[%s]\n", __FUNCTION__, __LINE__, settings->type);
    printf("[%s][%d] settings->encryption:[%s]\n", __FUNCTION__, __LINE__, settings->encryption);
    printf("[%s][%d] settings->key:[%s]\n", __FUNCTION__, __LINE__, settings->key);
    printf("[%s][%d] settings->enable_8021x:[%u]\n", __FUNCTION__, __LINE__, settings->enable_8021x);
    printf("[%s][%d] settings->radius_ip1:[%s]\n", __FUNCTION__, __LINE__, settings->radius_ip1);
    printf("[%s][%d] settings->radius_port1:[%d]\n", __FUNCTION__, __LINE__, settings->radius_port1);
    printf("[%s][%d] settings->radius_secret1:[%s]\n", __FUNCTION__, __LINE__, settings->radius_secret1);
#endif
out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
* NAME:    get_client_status_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_client_status_json_cb(HTTPS_CB *pkt, char *result)
{
    struct json_object *jobj, *jobj_client_list, *jarr_client_list;
    struct json_object *jstr_result;
    struct json_object *jstr_hostname, *jstr_ip, *jstr_mac, *jstr_exp_time;
    char hostName[31+1], ipaddr[15+1], macaddr[17+1], exptime[15+1], buf[31+1], date[63+1];
    char fingerprint[127+1];
    char *pch;
    char opmode[3]={0};
    int i=0, clientNum=0, times=0, sec=0, min=0, hour=0, day=0, hourtens=0, hourones=0, mintens=0, minones=0, sectens=0, secones=0, j=0;

    if(NULL == pkt)
        return -1;

    memset(hostName, 0, sizeof(hostName));
    memset(ipaddr, 0, sizeof(ipaddr));
    memset(macaddr, 0, sizeof(macaddr));
    memset(exptime, 0, sizeof(exptime));
    memset(buf, 0, sizeof(buf));
    memset(date, 0, sizeof(date));
    memset(fingerprint, 0, sizeof(fingerprint));

    jstr_hostname = NULL;
    jstr_ip = NULL;
    jstr_mac = NULL;
    jstr_exp_time = NULL;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();
    jarr_client_list = json_object_new_array();

    jstr_result = json_object_new_string(result);
    json_object_object_add(jobj, "GetClientStatusResult", jstr_result);

#if APP_AGENT_SUPPORT_ENSHARE
    api_get_string_option("system.@system[0].opmode", opmode, sizeof(opmode));

    if ( strcmp(opmode,"ap") == 0) 
    {

       if (sysutil_get_file_line_num("/tmp/finger_device_list", &clientNum)) 
        {
            //printf("[%s][%d] clientNum:[%d]\n", __FUNCTION__, __LINE__, clientNum); 
            for (i=1;i<=clientNum;i++) 
            {
                
                sysutil_interact(fingerprint, sizeof(fingerprint), "awk 'NR==%d' /tmp/finger_device_list",i);

                pch = strtok(fingerprint,"\t");
                while ( pch != NULL)
                {
                    if( j==1 ){
                        strcpy(macaddr,pch);
                    }
                    else if( j==2 ){
                        strcpy(ipaddr,pch);
                    }
                    else if( j==4){
                        strcpy(hostName,pch);
                    }
                    pch = strtok(NULL,"\t");
                    j++;
                }
                j = 0;
                jobj_client_list = json_object_new_object();

                //sysutil_get_dhcpd_client_macaddr(i, macaddr, sizeof(macaddr));
                //printf("[%s][%d] macaddr:[%s]\n", __FUNCTION__, __LINE__, macaddr); 
                jstr_mac = json_object_new_string(macaddr);
                json_object_object_add(jobj_client_list, "MacAddress", jstr_mac);

                //sysutil_get_dhcpd_client_ipaddr(i, ipaddr, sizeof(ipaddr));
                //printf("[%s][%d] ipaddr:[%s]\n", __FUNCTION__, __LINE__, ipaddr); 
                jstr_ip = json_object_new_string(ipaddr);
                json_object_object_add(jobj_client_list, "IPAddress", jstr_ip);

                //sysutil_get_dhcpd_client_hostname(i, hostName, sizeof(hostName));
                //printf("[%s][%d] hostName:[%s]\n", __FUNCTION__, __LINE__, hostName); 
                jstr_hostname = json_object_new_string(hostName);
                json_object_object_add(jobj_client_list, "Hostname", jstr_hostname);
                jstr_exp_time = json_object_new_string("");
                json_object_object_add(jobj_client_list, "ExpirationTime", jstr_exp_time);
                json_object_array_add(jarr_client_list,jobj_client_list);
            }
        }
    }
    else
    {
#ifdef DHCP_LEASES
        if (sysutil_get_file_line_num(DHCP_LEASES, &clientNum)) 
        {
            //printf("[%s][%d] clientNum:[%d]\n", __FUNCTION__, __LINE__, clientNum); 
            for (i=1;i<=clientNum;i++) 
            {
                jobj_client_list = json_object_new_object();

                sysutil_get_dhcpd_client_macaddr(i, macaddr, sizeof(macaddr));
                //printf("[%s][%d] macaddr:[%s]\n", __FUNCTION__, __LINE__, macaddr); 
                jstr_mac = json_object_new_string(macaddr);
                json_object_object_add(jobj_client_list, "MacAddress", jstr_mac);

                sysutil_get_dhcpd_client_ipaddr(i, ipaddr, sizeof(ipaddr));
                //printf("[%s][%d] ipaddr:[%s]\n", __FUNCTION__, __LINE__, ipaddr); 
                jstr_ip = json_object_new_string(ipaddr);
                json_object_object_add(jobj_client_list, "IPAddress", jstr_ip);

                sysutil_get_dhcpd_client_hostname(i, hostName, sizeof(hostName));
                //printf("[%s][%d] hostName:[%s]\n", __FUNCTION__, __LINE__, hostName); 
                jstr_hostname = json_object_new_string(hostName);
                json_object_object_add(jobj_client_list, "Hostname", jstr_hostname);

                sysutil_get_dhcpd_client_exptime(i, exptime, sizeof(exptime));
                //printf("[%s][%d] exptime:[%s]\n", __FUNCTION__, __LINE__, exptime); 
                sysutil_interact(buf, sizeof(buf), "%s", "date +%s");
                buf[strlen(buf)-1]='\0';
                //printf("[%s][%d] buf:[%s]\n", __FUNCTION__, __LINE__, buf);

                if (strcmp(buf,"") && strcmp(exptime,""))
                {
                    if (atoi(exptime)==0) 
                    {
                        snprintf(date, sizeof(date), " %s", "Forever");
                    }
                    else 
                    {
                        times = atoi(exptime) - atoi(buf); 
                        //printf("[%s][%d] times:[%d]\n", __FUNCTION__, __LINE__, times); 
                        if (times > 0)
                            sec = times%60;
                        if (times > 60)
                            min = (times/60)%60;
                        if (times > 3600)
                            hour = (times/(3600))%24;
                        if (times > 3600*24)
                            day = times/(3600*24);
                        //printf("[%s][%d] sec:[%d]\n", __FUNCTION__, __LINE__, sec); 
                        //printf("[%s][%d] min:[%d]\n", __FUNCTION__, __LINE__, min); 
                        //printf("[%s][%d] hour:[%d]\n", __FUNCTION__, __LINE__, hour); 
                        //printf("[%s][%d] day:[%d]\n", __FUNCTION__, __LINE__, day); 

                        secones = sec%10;
                        sectens = sec/10;
                        minones = min%10;
                        mintens = min/10;
                        hourones = hour%10;
                        hourtens = hour/10;
                        //printf("[%s][%d] secones:[%d]\n", __FUNCTION__, __LINE__, secones); 
                        //printf("[%s][%d] sectens:[%d]\n", __FUNCTION__, __LINE__, sectens); 
                        //printf("[%s][%d] minones:[%d]\n", __FUNCTION__, __LINE__, minones); 
                        //printf("[%s][%d] mintens:[%d]\n", __FUNCTION__, __LINE__, mintens); 
                        //printf("[%s][%d] hourones:[%d]\n", __FUNCTION__, __LINE__, hourones); 
                        //printf("[%s][%d] hourtens:[%d]\n", __FUNCTION__, __LINE__, hourtens);

                        snprintf(date, sizeof(date), "%d day %d%d:%d%d:%d%d", day, hourtens, hourones, mintens, minones, sectens, secones);
                    }
                }
                //printf("[%s][%d] date:[%s]\n", __FUNCTION__, __LINE__, date); 
                jstr_exp_time = json_object_new_string(date);
                json_object_object_add(jobj_client_list, "ExpirationTime", jstr_exp_time);
                json_object_array_add(jarr_client_list,jobj_client_list);
            }
        }
#endif
    }
#else
#ifdef DHCP_LEASES
    if (sysutil_get_file_line_num(DHCP_LEASES, &clientNum)) 
    {
        //printf("[%s][%d] clientNum:[%d]\n", __FUNCTION__, __LINE__, clientNum); 
        for (i=1;i<=clientNum;i++) 
        {
            jobj_client_list = json_object_new_object();

            sysutil_get_dhcpd_client_macaddr(i, macaddr, sizeof(macaddr));
            //printf("[%s][%d] macaddr:[%s]\n", __FUNCTION__, __LINE__, macaddr); 
            jstr_mac = json_object_new_string(macaddr);
            json_object_object_add(jobj_client_list, "MacAddress", jstr_mac);

            sysutil_get_dhcpd_client_ipaddr(i, ipaddr, sizeof(ipaddr));
            //printf("[%s][%d] ipaddr:[%s]\n", __FUNCTION__, __LINE__, ipaddr); 
            jstr_ip = json_object_new_string(ipaddr);
            json_object_object_add(jobj_client_list, "IPAddress", jstr_ip);

            sysutil_get_dhcpd_client_hostname(i, hostName, sizeof(hostName));
            //printf("[%s][%d] hostName:[%s]\n", __FUNCTION__, __LINE__, hostName); 
            jstr_hostname = json_object_new_string(hostName);
            json_object_object_add(jobj_client_list, "Hostname", jstr_hostname);

            sysutil_get_dhcpd_client_exptime(i, exptime, sizeof(exptime));
            //printf("[%s][%d] exptime:[%s]\n", __FUNCTION__, __LINE__, exptime); 
            sysutil_interact(buf, sizeof(buf), "%s", "date +%s");
            buf[strlen(buf)-1]='\0';
            //printf("[%s][%d] buf:[%s]\n", __FUNCTION__, __LINE__, buf);

            if (strcmp(buf,"") && strcmp(exptime,""))
            {
                if (atoi(exptime)==0) 
                {
                    snprintf(date, sizeof(date), " %s", "Forever");
                }
                else 
                {
                    times = atoi(exptime) - atoi(buf); 
                    //printf("[%s][%d] times:[%d]\n", __FUNCTION__, __LINE__, times); 
                    if (times > 0)
                        sec = times%60;
                    if (times > 60)
                        min = (times/60)%60;
                    if (times > 3600)
                        hour = (times/(3600))%24;
                    if (times > 3600*24)
                        day = times/(3600*24);
                    //printf("[%s][%d] sec:[%d]\n", __FUNCTION__, __LINE__, sec); 
                    //printf("[%s][%d] min:[%d]\n", __FUNCTION__, __LINE__, min); 
                    //printf("[%s][%d] hour:[%d]\n", __FUNCTION__, __LINE__, hour); 
                    //printf("[%s][%d] day:[%d]\n", __FUNCTION__, __LINE__, day); 

                    secones = sec%10;
                    sectens = sec/10;
                    minones = min%10;
                    mintens = min/10;
                    hourones = hour%10;
                    hourtens = hour/10;
                    //printf("[%s][%d] secones:[%d]\n", __FUNCTION__, __LINE__, secones); 
                    //printf("[%s][%d] sectens:[%d]\n", __FUNCTION__, __LINE__, sectens); 
                    //printf("[%s][%d] minones:[%d]\n", __FUNCTION__, __LINE__, minones); 
                    //printf("[%s][%d] mintens:[%d]\n", __FUNCTION__, __LINE__, mintens); 
                    //printf("[%s][%d] hourones:[%d]\n", __FUNCTION__, __LINE__, hourones); 
                    //printf("[%s][%d] hourtens:[%d]\n", __FUNCTION__, __LINE__, hourtens);

                    snprintf(date, sizeof(date), "%d day %d%d:%d%d:%d%d", day, hourtens, hourones, mintens, minones, sectens, secones);
                }
            }
            //printf("[%s][%d] date:[%s]\n", __FUNCTION__, __LINE__, date); 
            jstr_exp_time = json_object_new_string(date);
            json_object_object_add(jobj_client_list, "ExpirationTime", jstr_exp_time);
            json_object_array_add(jarr_client_list,jobj_client_list);
        }
    }
#endif
#endif
    json_object_object_add(jobj, "ClientStatuses", jarr_client_list);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jstr_result);
    if(jstr_hostname)
        json_object_put(jstr_hostname);

    if(jstr_ip)
        json_object_put(jstr_ip);

    if(jstr_mac)
        json_object_put(jstr_mac);

    if(jstr_exp_time)
        json_object_put(jstr_exp_time);

    json_object_put(jobj_client_list);
    json_object_put(jarr_client_list);
    json_object_put(jobj);

    return 0;
}

/*-------------------------------- Sitecom Setting --------------------------------*/
#if 1//HAS_SC_AUTO_FW_CHECK
/*****************************************************************
* NAME:    get_auto_fw_upgrade_status_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool get_auto_fw_upgrade_status_json_cb(HTTPS_CB *pkt, bool enabled, char *result)
{
    struct json_object *jobj, *jstr_result, *jbool_enabled;

    if(NULL == pkt)
    {
        return -1;
    }
    jobj = json_object_new_object();

    jstr_result = json_object_new_string(result);
    json_object_object_add(jobj, "GetAutoFWupgradeStatusResult", jstr_result);

    jbool_enabled = json_object_new_boolean(enabled);
    json_object_object_add(jobj, "Enabled", jbool_enabled);

    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jstr_result);
    json_object_put(jbool_enabled);
    json_object_put(jobj);

    return 0;

}
#if 0
/*****************************************************************
* NAME:    parse_auto_fw_upgrade_status_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool parse_auto_fw_upgrade_status_json_cb(char *query_str, bool *enabled, char *return_str)
{
    bool result, is_jobj;
    struct json_object *jobj;

    result = TRUE;
    is_jobj = FALSE;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            result = senao_json_object_get_boolean(jobj, "Enabled", enabled);
            if(FALSE == result) goto out;
        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}
#endif
#endif // HAS_SC_AUTO_FW_CHECK

#if HAS_STREAM_BOOST_SETTING
/*****************************************************************
* NAME:    get_stream_boost_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool get_stream_boost_settings_json_cb(HTTPS_CB *pkt, STREAM_BOOST_DATA *setting, char *result)
{
    struct json_object *jobj, *jstr_result, *jbool_enabled;

    if(NULL == pkt)
    {
        return -1;
    }
    jobj = json_object_new_object();

    jstr_result = json_object_new_string(result);
    json_object_object_add(jobj, "GetStreamBoostResult", jstr_result);
    printf("[%s][%d] setting->enabled:[%u]\n", __FUNCTION__, __LINE__, setting->enabled); 
    jbool_enabled = json_object_new_boolean(setting->enabled);
    json_object_object_add(jobj, "Enabled", jbool_enabled);

    printf("[%s][%d] jobj:[%s]\n", __FUNCTION__, __LINE__, json_object_get_string(jobj));
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));
    printf("[%s][%d] jobj:[%s]\n", __FUNCTION__, __LINE__, json_object_get_string(jobj));
    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jstr_result);
    json_object_put(jbool_enabled);
    json_object_put(jobj);

    return 0;

}

/*****************************************************************
* NAME:    parse_stream_boost_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool parse_stream_boost_settings_json_cb(char *query_str, STREAM_BOOST_DATA *setting, char *return_str)
{
    bool result, is_jobj;
    struct json_object *jobj;
    bool enabled = 0;
    int upLimit = 0, downLimit = 0;

    result = TRUE;
    is_jobj = FALSE;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if (senao_json_object_get_boolean(jobj, "Enabled", (bool *)&enabled))
            {
                setting->enabled = enabled;
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if(json_object_object_get(jobj, "UpLimit"))
            {
                setting->upLimit = json_object_get_int(json_object_object_get(jobj, "UpLimit"));
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if(json_object_object_get(jobj, "DownLimit"))
            {
                setting->downLimit = json_object_get_int(json_object_object_get(jobj, "DownLimit"));
            }
            else
            {
                result = FALSE;
                goto out;
            }

        }
    }

out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}
#endif

/*-------------------------------------------------------------------------------*/

/*----------------------------- IPCAM SAMBA SETTING -----------------------------*/
/*****************************************************************
* NAME:    parse_json_ipcam_samba_settings
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_json_ipcam_samba_client(char *query_str, IPCAM_SAMBA_SETTINGS_T *settings, char *return_str)
{
    bool result, is_jobj;
    struct json_object *jobj, *jobj_username, *jobj_password;
	struct json_object *jobj_record, *jobj_overwrite;
    struct json_object *jobj_samba_server, *jobj_samba_folder_path;
	struct json_object *jobj_samba_username, *jobj_samba_password;

    result = TRUE;
    is_jobj = FALSE;

	if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_username = json_object_object_get(jobj, "AdminUsername")))
            {
                sprintf(settings->username, "%s", json_object_get_string(jobj_username));
                /* Free obj */
                json_object_put(jobj_username);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_password = json_object_object_get(jobj, "AdminPassword")))
            {
				sprintf(settings->password, "%s", json_object_get_string(jobj_password));

                /* Free obj */
                json_object_put(jobj_password);
            }
            else
            {
                //return_str = HNAP_ERROR_CHANNEL_WIDTH_STR;
                result = FALSE;
                goto out;
            }

            if((jobj_record = json_object_object_get(jobj, "EnableRecord")))
            {
                settings->record = json_object_get_int(jobj_record);

                /* Free obj */
                json_object_put(jobj_record);
            }
            else
            {
                //return_str = HNAP_ERROR_CHANNEL_STR;
                result = FALSE;
                goto out;
            }

            if((jobj_overwrite = json_object_object_get(jobj, "EnableOverwrite")))
            {
                settings->overwrite = json_object_get_int(jobj_overwrite);

                /* Free obj */
                json_object_put(jobj_overwrite);
            }
            else
            {
                //return_str = HNAP_ERROR_CHANNEL_STR;
                result = FALSE;
                goto out;
            }

			if((jobj_samba_server = json_object_object_get(jobj, "SambaServer")))
            {
                sprintf(settings->samba_server, "%s", json_object_get_string(jobj_samba_server));

                /* Free obj */
                json_object_put(jobj_samba_server);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_samba_folder_path = json_object_object_get(jobj, "SambaFolderPath")))
            {
				sprintf(settings->samba_folder_path, "%s", json_object_get_string(jobj_samba_folder_path));

                /* Free obj */
                json_object_put(jobj_samba_folder_path);
            }
            else
            {
                //return_str = HNAP_ERROR_CHANNEL_WIDTH_STR;
                result = FALSE;
                goto out;
            }

			if((jobj_samba_username = json_object_object_get(jobj, "SambaServerUsername")))
            {
                sprintf(settings->samba_username, "%s", json_object_get_string(jobj_samba_username));

                /* Free obj */
                json_object_put(jobj_samba_username);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_samba_password = json_object_object_get(jobj, "SambaServerPassword")))
            {
				sprintf(settings->samba_password, "%s", json_object_get_string(jobj_samba_password));

                /* Free obj */
                json_object_put(jobj_samba_password);
            }
            else
            {
                //return_str = HNAP_ERROR_CHANNEL_WIDTH_STR;
                result = FALSE;
                goto out;
            }
        }
	}

out:

    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
* NAME:    parse_json_ipcam_motion_detect
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_json_ipcam_motion_detect(char *query_str, IPCAM_SAMBA_SETTINGS_T *settings, char *return_str)
{
    bool result, is_jobj;
    struct json_object *jobj, *jobj_username, *jobj_password;
	struct json_object *jobj_motion_detect;

    result = TRUE;
    is_jobj = FALSE;

	if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            if((jobj_username = json_object_object_get(jobj, "AdminUsername")))
            {
                sprintf(settings->username, "%s", json_object_get_string(jobj_username));
                /* Free obj */
                json_object_put(jobj_username);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jobj_password = json_object_object_get(jobj, "AdminPassword")))
            {
				sprintf(settings->password, "%s", json_object_get_string(jobj_password));

                /* Free obj */
                json_object_put(jobj_password);
            }
            else
            {
                //return_str = HNAP_ERROR_CHANNEL_WIDTH_STR;
                result = FALSE;
                goto out;
            }

            if((jobj_motion_detect = json_object_object_get(jobj, "EnableMotionDetect")))
            {
                settings->motion_detect = json_object_get_int(jobj_motion_detect);

                /* Free obj */
                json_object_put(jobj_motion_detect);
            }
            else
            {
                //return_str = HNAP_ERROR_CHANNEL_STR;
                result = FALSE;
                goto out;
            }
        }
	}

out:

    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }

    return result;
}
/*--------------- -----------------------------------------------------------*/
/*----------------------------- ACCOUNT SETTING -----------------------------*/
void get_user_list_json_cb(HTTPS_CB *pkt, ACCOUNT_SETTING_T *account_array)

{
	int i = 0;
    struct json_object *jobj, *jstr_result, *jarr_userlist, *jobj_user, *jstr_username, *jstr_privilege;

	/* Construct the packet content in json format. */
	jobj = json_object_new_object();

    jstr_result = json_object_new_string(OK_STR);

	jarr_userlist = json_object_new_array();

	while(strlen(account_array[i].user_name) > 0)
	{
            jobj_user = json_object_new_object();
            jstr_username = json_object_new_string(account_array[i].user_name);
			switch (account_array[i].privilege_level)
			{
            case 0:
                jstr_privilege = json_object_new_string(ADMINISTRATOR_USER_STR);
                break;
            case 1:
                jstr_privilege = json_object_new_string(GUEST_USER_STR);
                break;
            case 2:
                jstr_privilege = json_object_new_string(ONVIF_USER_STR);
                break;
            default:
                jstr_privilege = json_object_new_string(GUEST_USER_STR);
                printf("%s: Incorrect privilege level %d\n", __FUNCTION__, account_array[i].privilege_level);
			}
            json_object_object_add(jobj_user, "Name", jstr_username);
            json_object_object_add(jobj_user, "Privilege", jstr_privilege);

            json_object_array_add(jarr_userlist, jobj_user);
	    i++;
    }

    json_object_object_add(jobj, "GetUserListResult", jstr_result);
	json_object_object_add(jobj, "UserList", jarr_userlist);

	http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
	http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
	http_store_data_to_buffer("%s\r\n", json_object_to_json_string(jobj));

	http_send_stored_data(pkt->fd);

	/* Free obj */
	json_object_put(jobj);
	json_object_put(jstr_result);
	json_object_put(jarr_userlist);
	json_object_put(jobj_user);
	json_object_put(jstr_username);
	json_object_put(jstr_privilege);
}

/*****************************************************************
* NAME:    get_http_port_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_http_port_json_cb(HTTPS_CB *pkt,int http_port,int second_http_port, char *result)
{
        struct json_object *jobj, *jobj_http;

        if(NULL == pkt)
                return -1;
        jobj = json_object_new_object();
        ADD_JSON_OBJECT_NEW_STRING(jobj, "GetHttpPortSettingsResult", result);
        jobj_http = json_object_new_object();
        ADD_JSON_OBJECT_NEW_INT(jobj_http, "HttpPort",http_port);
        ADD_JSON_OBJECT_NEW_INT(jobj_http, "SecondHttpPort",second_http_port);
        json_object_object_add(jobj, "Http", jobj_http);

        /* Store packet content into buffer and send it out */
        http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
        http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
        http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));
        http_send_stored_data(pkt->fd);
        /* Free obj */
        json_object_put(jobj_http);
        json_object_put(jobj);

        return 0;
}

/*****************************************************************
* NAME:    set_http_port_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool set_http_port_json_cb(char *query_str, int *http_port, int *second_http_port)
{
    bool result=TRUE;
    struct json_object *jobj, *jobj_http;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            if(jobj_http = json_object_object_get(jobj, "Http"))
            {
                if((FALSE == senao_json_object_get_integer(jobj_http, "HttpPort", http_port))
                        ||(FALSE == senao_json_object_get_integer(jobj_http, "SecondHttpPort", second_http_port)))
                {
                            result = FALSE;
                            goto out;
                }
            }
        }
        else
        {
            result = FALSE;
            goto out;
        }
    }
out:
	/* Free obj */
	json_object_put(jobj_http);
	json_object_put(jobj);
	return result;
}


/*****************************************************************
* NAME:    get_fw_upgrade_url_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_fw_upgrade_url_json_cb(HTTPS_CB *pkt, char *fw_upgrade_url)
{
    struct json_object *jobj, *jobj_ddns;
    struct json_object *jstr_result, *jstr_fw_upgrade_url;

    if(NULL == pkt)
        return -1;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string(OK_STR);
    json_object_object_add(jobj, "GetFwUpgradeURLResult", jstr_result);

    jstr_fw_upgrade_url = json_object_new_string(fw_upgrade_url);
    json_object_object_add(jobj, "GetFwUpgradeUrl", jstr_fw_upgrade_url);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jstr_result);
    json_object_put(jstr_fw_upgrade_url);
    json_object_put(jobj);
    return 0;
}

/*****************************************************************
* NAME:    get_config_upgrade_url_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_config_upgrade_url_json_cb(HTTPS_CB *pkt, char *config_upgrade_url)
{
    struct json_object *jobj, *jobj_ddns;
    struct json_object *jstr_result, *jstr_config_upgrade_url;

    if(NULL == pkt)
        return -1;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string(OK_STR);
    json_object_object_add(jobj, "GetConfigUpgradeURLResult", jstr_result);

    jstr_config_upgrade_url = json_object_new_string(config_upgrade_url);
    json_object_object_add(jobj, "ConfigUpgradeURL", jstr_config_upgrade_url);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jstr_result);
    json_object_put(jstr_config_upgrade_url);
    json_object_put(jobj);
    return 0;
}

/*****************************************************************
 * NAME:    get_upnp_settings_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_upnp_settings_json_cb(HTTPS_CB *pkt, SYSTEM_UPNP_INFO_T *setting, char *result)
{
    struct json_object *jobj, *jobj_upnp;

    if(NULL == pkt)
        return -1;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();
    json_object_object_add(jobj, "GetUpnpSettingsResult",json_object_new_string(result));
	
    jobj_upnp = json_object_new_object();
    json_object_object_add(jobj_upnp, "Name", json_object_new_string(setting->name));
    json_object_object_add(jobj_upnp, "EnableService",json_object_new_int(setting->enableService));
#if HAS_NAT_TRAVERSAL
    json_object_object_add(jobj_upnp, "EnableTraversal", json_object_new_int(setting->enableTraversal));
#endif

    json_object_object_add(jobj, "UPnP", jobj_upnp);
    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));
    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj_upnp);
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
 * NAME:    parse_json_upnp_settings
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
bool parse_json_upnp_settings(char *query_str, SYSTEM_UPNP_INFO_T *settings)
{
    bool result= TRUE, is_jobj= FALSE;
    struct json_object *jobj_upnp,*jobj;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;
            if((jobj_upnp = json_object_object_get(jobj, "UPnP")))
            {
                sprintf(settings->name, "%s", json_object_get_string(json_object_object_get(jobj_upnp, "Name")));
                settings->enableService = json_object_get_int(json_object_object_get(jobj_upnp, "EnableService"));
#if HAS_NAT_TRAVERSAL
                settings->enableTraversal = json_object_get_int(json_object_object_get(jobj_upnp, "EnableTraversal"));
#endif
            }
            else
            {
                result = FALSE;
                goto out;
            }
        }
    }

out:

    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
        json_object_put(jobj_upnp);
    }
    return result;
}

/*****************************************************************
* NAME:    get_rtsp_port_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_rtsp_port_json_cb(HTTPS_CB *pkt, int rtsp_port, char *result)
{
    struct json_object *jobj;

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    ADD_JSON_OBJECT_NEW_STRING(jobj, "GetRtspPortSettingsResult", result);
	ADD_JSON_OBJECT_NEW_INT(jobj, "RtspPort", rtsp_port);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    parse_json_rtsp_port_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_json_rtsp_port_json_cb(char *query_str, int *rtsp_port)
{
    bool result, is_jobj;
    struct json_object *jobj, *jobj_port;

    result = TRUE;
    is_jobj = FALSE;
    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;
            if((jobj_port = json_object_object_get(jobj, "RtspPort")))
			{
				*rtsp_port = json_object_get_int(jobj_port);
				/* Free obj */
				json_object_put(jobj_port);
			}
			else
			{
				result = FALSE;
				goto out;
			}
        }
        else
        {
            result = FALSE;
            goto out;
        }
    }
out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj);
    }
    return result;
}

/*------------------------------- DDNS Setting ------------------------------*/

/*****************************************************************
 * NAME:    get_ddns_settings_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_ddns_settings_json_cb(HTTPS_CB *pkt, SYSTEM_DDNS_INFO_T *setting, char *result)
{
    struct json_object *jobj, *jobj_ddns;

    if(NULL == pkt)
        return -1;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();
    json_object_object_add(jobj, "GetDdnsSettingsResult",json_object_new_string(result));

    jobj_ddns = json_object_new_object();
    json_object_object_add(jobj_ddns, "Enable",json_object_new_int(setting->enable));
    json_object_object_add(jobj_ddns, "Provider",json_object_new_string(setting->provider));
    json_object_object_add(jobj_ddns, "Host",json_object_new_string(setting->host));
    json_object_object_add(jobj_ddns, "UserName",json_object_new_string(setting->userName));
    json_object_object_add(jobj_ddns, "Password",json_object_new_string(setting->password));

#if HAS_ENGENIUS_DDNS
    json_object_object_add(jobj_ddns, "DefaultName",json_object_new_string(setting->defaultName));
    json_object_object_add(jobj_ddns, "AliasName",json_object_new_string(setting->aliasName));
    json_object_object_add(jobj_ddns, "RefreshTime",json_object_new_int(setting->refreshTime));
#endif

    json_object_object_add(jobj, "Ddns", jobj_ddns);
    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);
    return 0;
}


/*****************************************************************
 * NAME:    parse_json_ddns_settings
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
bool parse_json_ddns_settings(char *query_str, SYSTEM_DDNS_INFO_T *settings)
{
    bool result, is_jobj;
    struct json_object *jobj_ddns, *jobj, *jint_enable, *jstr_provider, *jstr_host, *jstr_userName, *jstr_password;
    struct json_object *jint_refresh_time, *jstr_alias_name;
    result = TRUE;
    is_jobj = FALSE;
    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;
            if((jobj_ddns = json_object_object_get(jobj, "Ddns")))
            {
                if((jint_enable = json_object_object_get(jobj_ddns, "Enable")))
                {
                    settings->enable= json_object_get_int(jint_enable);
                    /* Free obj */
                    json_object_put(jint_enable);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if((jstr_provider = json_object_object_get(jobj_ddns, "Provider")))
                {
                    sprintf(settings->provider, "%s", json_object_get_string(jstr_provider));
                    /* Free obj */
                    json_object_put(jstr_provider);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if((jstr_host = json_object_object_get(jobj_ddns, "Host")))
                {
                    sprintf(settings->host, "%s", json_object_get_string(jstr_host));
                    /* Free obj */
                    json_object_put(jstr_host);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if((jstr_userName = json_object_object_get(jobj_ddns, "UserName")))
                {
                    sprintf(settings->userName, "%s", json_object_get_string(jstr_userName));
                    /* Free obj */
                    json_object_put(jstr_userName);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }

                if((jstr_password = json_object_object_get(jobj_ddns, "Password")))
                {
                    sprintf(settings->password, "%s", json_object_get_string(jstr_password));
                    /* Free obj */
                    json_object_put(jstr_password);
                }
                else
                {
                    result = FALSE;
                    goto out;
                }
            }
            else
            {
                result = FALSE;
                goto out;
            }
#if HAS_ENGENIUS_DDNS
            if((jint_refresh_time = json_object_object_get(jobj_ddns, "RefreshTime")))
            {
                settings->refreshTime= json_object_get_int(jint_refresh_time);
                /* Free obj */
                json_object_put(jint_refresh_time);
            }
            else
            {
                result = FALSE;
                goto out;
            }

            if((jstr_alias_name = json_object_object_get(jobj_ddns, "AliasName")))
            {
                sprintf(settings->aliasName, "%s", json_object_get_string(jstr_alias_name));
                /* Free obj */
                json_object_put(jstr_alias_name);
            }
            else
            {
                result = FALSE;
                goto out;
            }
#endif
        }
    }
out:
    if(is_jobj)
    {
        /* Free obj */
        json_object_put(jobj_ddns);
        json_object_put(jobj);
    }
    return result;
}

#if HAS_ENGENIUS_DDNS
/*****************************************************************
* NAME:    get_engenius_ddns_info_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_engenius_ddns_info_json_cb(HTTPS_CB *pkt, bool enabled, char *ddns_str, char *result)
{
    struct json_object *jobj;
    struct json_object *jstr_result, *jbool_enable, *jstr_ddns;

    if(NULL == pkt)
    {
        return -1;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    jstr_result = json_object_new_string(result);
    json_object_object_add(jobj, "GetEnGeniusDDNSInfoResult", jstr_result);

    jbool_enable = json_object_new_boolean(enabled);
    json_object_object_add(jobj, "Enabled", jbool_enable);

    jstr_ddns = json_object_new_string(ddns_str);
    json_object_object_add(jobj, "EnGeniusDDNSName", jstr_ddns);

    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jstr_result);
	json_object_put(jbool_enable);
    json_object_put(jstr_ddns);
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    parse_alias_name_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_alias_name_json_cb(char *query_str, char *alias_name)
{
    bool result;
    struct json_object *jobj, *jobj_alias_name;

    result = FALSE;

    if(0 != strlen(query_str))
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            if((jobj_alias_name = json_object_object_get(jobj, "AliasName")))
            {
                sprintf(alias_name, "%s", json_object_get_string(jobj_alias_name));

                result = TRUE;

                /* Free obj */
                json_object_put(jobj_alias_name);
            }
            /* Free obj */
            json_object_put(jobj);
        }
    }

    return result;
}

/*****************************************************************
* NAME:    get_en_ddns_alias_name_available_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
void get_en_ddns_alias_name_available_json_cb(HTTPS_CB *pkt, bool result, char *check_result)
{
    struct json_object *jobj;

    if(NULL == pkt)
        return;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();
    json_object_object_add(jobj, "GetEnDdnsAliasNameAvailableResult",json_object_new_string((result)?OK_STR:ERROR_STR));

	if(!strcmp(check_result, "OK"))
                json_object_object_add(jobj, "CheckAliasNameResult",json_object_new_string("OK"));
	else if(!strcmp(check_result, "FAILED"))
                json_object_object_add(jobj, "CheckAliasNameResult",json_object_new_string("NA"));
	else
                json_object_object_add(jobj, "CheckAliasNameResult",json_object_new_string("TIMEOUT"));

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return;
}
#endif

/*****************************************************************
 * NAME:    get_ddns_provider_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_ddns_provider_json_cb(HTTPS_CB *pkt, char *result)
{
	struct json_object *jobj,*jarr_ddns_type,*jarr_ddns_refresh_time;
	int i;
	char **ddns_type=NULL;
	int *ddns_refresh_time=NULL;
	char refresh_time[8];

	if(NULL == pkt)
                return -1;

	ddns_type = all_ddns_type;
	ddns_refresh_time = en_ddns_refresh_time;

	/* Construct the packet content in json format. */
        jobj = json_object_new_object();
        json_object_object_add(jobj, "GetDdnsProviderResult",json_object_new_string(result));
#if HAS_ENGENIUS_DDNS
	jarr_ddns_type = json_object_new_array();
	for (i=0;i<DDNS_TYPE_MAX;i++)
	{
		if(SUPPORT_DDNS_TYPE_BITMAP & (1 << i))
			json_object_array_add(jarr_ddns_type,
			        json_object_new_string(ddns_type[i]));
	}
	json_object_object_add(jobj, "SupportProvider", jarr_ddns_type);

	jarr_ddns_refresh_time = json_object_new_array();
	for (i=0;i<EN_DDNS_REFRESH_TIME_NUM;i++)
	{
		sprintf(refresh_time, "%d", ddns_refresh_time[i]);
		json_object_array_add(jarr_ddns_refresh_time,
                        json_object_new_string(refresh_time));
	}

	json_object_object_add(jobj, "EnDdnsRefreshTime", jarr_ddns_refresh_time);
#endif

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jarr_ddns_type);
    json_object_put(jarr_ddns_refresh_time);
    json_object_put(jobj);
    return 0;
}

/*****************************************************************
* NAME:    get_wlan_station_status_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_wlan_station_status_json_cb(HTTPS_CB *pkt, WLAN_STATION_STATUS *setting, char *result)
{
    struct json_object *jobj;

    if(NULL == pkt)
    {
        return -1;
    }
    jobj = json_object_new_object();

    ADD_JSON_OBJECT_NEW_STRING(jobj, "GetWLanStationStatusResult", result);
    ADD_JSON_OBJECT_NEW_INT(jobj, "Channel", setting->channel);
    ADD_JSON_OBJECT_NEW_STRING(jobj, "SSID", setting->ssid);
    ADD_JSON_OBJECT_NEW_STRING(jobj, "EncryptionType", setting->encryptionType);
    ADD_JSON_OBJECT_NEW_STRING(jobj, "Status", setting->status);
    ADD_JSON_OBJECT_NEW_INT(jobj, "SignalStrength", setting->signalStrength);
            
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    get_wlan_site_survey_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_wlan_site_survey_json_cb(HTTPS_CB *pkt, WLAN_SITESURVEY_T *setting, char *result, int site_survey_ssid_num)
{
        struct json_object *jobj, *jobj_site_survey_infos, *jobj_site_survey_info;
        struct json_object *jarr_site_survey_info;
        int counter;
        
        if(NULL == pkt)
                return -1;

        jobj = json_object_new_object();
        ADD_JSON_OBJECT_NEW_STRING(jobj, "GetWLanSiteSurveyResult", result);
        jobj_site_survey_infos = json_object_new_object();
        jarr_site_survey_info = json_object_new_array();

	for (counter=0; counter<site_survey_ssid_num; counter++)
	{
                jobj_site_survey_info = json_object_new_object();
                ADD_JSON_OBJECT_NEW_INT(jobj_site_survey_info, "Channel", setting[counter].channel);
                ADD_JSON_OBJECT_NEW_STRING(jobj_site_survey_info, "BSSID", setting[counter].bssid);
                ADD_JSON_OBJECT_NEW_STRING(jobj_site_survey_info, "SSID", setting[counter].ssid);
                ADD_JSON_OBJECT_NEW_STRING(jobj_site_survey_info, "EncryptionType", setting[counter].encryptionType);
                ADD_JSON_OBJECT_NEW_STRING(jobj_site_survey_info, "AuthType", setting[counter].authType);
                ADD_JSON_OBJECT_NEW_INT(jobj_site_survey_info, "SignalStrength", setting[counter].signalStrength);
                ADD_JSON_OBJECT_NEW_STRING(jobj_site_survey_info, "WlanMode", setting[counter].wlanMode);
                json_object_array_add(jarr_site_survey_info, jobj_site_survey_info);
	}
        json_object_object_add(jobj_site_survey_infos, "SiteSurveyInfo", jarr_site_survey_info);
        json_object_object_add(jobj, "SiteSurveyInfos", jobj_site_survey_infos);

        http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
        http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
        http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));
        http_send_stored_data(pkt->fd);

        /* Free obj */
        json_object_put(jarr_site_survey_info);
	json_object_put(jobj_site_survey_info);
	json_object_put(jobj_site_survey_infos);
        json_object_put(jobj);

        return 0;
}

/*****************************************************************
* NAME:    get_json_wifiinfo_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_wifiinfo_cb(HTTPS_CB *pkt, char *result)
{
    int i;
    struct json_object *jobj;
    int wifi0_guest=0;
    int wifi1_guest=0;
    char api_result[128]={0};
    char buf[128]={0};
    char buf2[128]={0};
    char ifname[8]={0};
    int key_index=0;
#if HAS_WLAN_5G_2_SETTING
	int wifi1_nochannel=1;
	int wifi2_nochannel=1;
	int wifi1_disabled=1;
	int wifi2_disabled=1;
	int bandsteerHBpersent=0;
#endif

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    sysutil_interact(buf, sizeof(buf), "setconfig -g 4");
    json_object_object_add(jobj, "regularDomain", json_object_new_int(atoi(buf)));

    api_get_string_option("wireless.wifi1_ssid_1.ssid", buf, sizeof(buf));
    json_object_object_add(jobj, "WifiSsid", json_object_new_string(buf));

    api_get_string_option("wireless.wifi1_ssid_1.encryption", buf, sizeof(buf));

    if (strcmp(buf, "wep-open")==0)
    {
        api_get_integer_option("wireless.wifi1_ssid_1.key_id", &key_index);
        api_get_string_option2(buf, sizeof(buf), "wireless.wifi1_ssid_1.key%d", key_index);

        if (strstr(buf, "s:"))
        {
            strcpy(buf, buf+strlen("s:"));
        }
    }
    else
    {
        api_get_string_option("wireless.wifi1_ssid_1.key", buf, sizeof(buf));
    }

    json_object_object_add(jobj, "WifiKey", json_object_new_string(buf));

    if (api_get_integer_option("wireless.wifi0_guest.disabled", &wifi0_guest))
    {
        wifi0_guest = 1;
    }
    
    if (api_get_integer_option("wireless.wifi1_guest.disabled", &wifi1_guest))
    {
        wifi1_guest = 1;
    }

    if (wifi0_guest == 1 && wifi1_guest == 1)
    {
        json_object_object_add(jobj, "WirelessGuestNetworkStatus", json_object_new_string("0"));
    }
    else
    {
        json_object_object_add(jobj, "WirelessGuestNetworkStatus", json_object_new_string("1"));
    }

    api_get_string_option("wireless.wifi1_guest.ssid", buf, sizeof(buf));
    json_object_object_add(jobj, "WifiGuestSsid", json_object_new_string(buf));

    api_get_string_option("wireless.wifi1_guest.key", buf, sizeof(buf));
    json_object_object_add(jobj, "WifiGuestKey", json_object_new_string(buf));

	//2.4G
	api_get_string_option("wireless.wifi0_ssid_1.ifname", ifname, sizeof(ifname));

	api_get_string_option("wireless.wifi0.htmode", buf, sizeof(buf));
    sys_interact(buf2, sizeof(buf2),"/sbin/getHTModeList.sh 0 | tr -d '\\n'");
    if(strstr(buf2,buf) == NULL)
    {
        sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh get24GNowHTMode | tr -d '\\n'");
    }
	json_object_object_add(jobj, "Wifi24gBandwith", json_object_new_string(buf));

    api_get_string_option("wireless.wifi0.channel", buf, sizeof(buf));
	json_object_object_add(jobj, "Wifi24gChannel", json_object_new_string(buf));
    api_get_string_option("wireless.wifi0_ssid_1.encryption", buf, sizeof(buf));
	json_object_object_add(jobj, "Wifi24gEncrType", json_object_new_string(buf));
    api_get_string_option("wireless.wifi0_guest.encryption", buf, sizeof(buf));
	json_object_object_add(jobj, "Wifi24gGuestEncrType", json_object_new_string(buf));

	//5G
#if HAS_WLAN_5G_2_SETTING
	api_get_integer_option("wireless.wifi1.nochannel", &wifi1_nochannel);
	api_get_integer_option("wireless.wifi1_ssid_1.disabled", &wifi1_disabled);
#endif
	{
		api_get_string_option("wireless.wifi1_ssid_1.ifname", ifname, sizeof(ifname));

		api_get_string_option("wireless.wifi1.htmode", buf, sizeof(buf));
#if HAS_WLAN_5G_2_SETTING
		if(wifi1_disabled == 0 && wifi1_nochannel == 0)
#endif
		{
			sys_interact(buf2, sizeof(buf2),"/sbin/getHTModeList.sh 1 | tr -d '\\n'");
			if(strstr(buf2,buf) == NULL)
			{
				sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh get5GNowHTMode | tr -d '\\n'");
			}
		}
		json_object_object_add(jobj, "Wifi5gBandwith", json_object_new_string(buf));

		api_get_string_option("wireless.wifi1.channel", buf, sizeof(buf));
		json_object_object_add(jobj, "Wifi5gChannel", json_object_new_string(buf));

#if HAS_WLAN_5G_2_SETTING
		if(wifi1_disabled == 0 && wifi1_nochannel == 0)
#endif
		{
			//This is for DFS channel detection
			sysutil_interact(buf, sizeof(buf), "iwlist ath1 chan |grep Current| awk '{print $2}' | tr -d \"\n\"");
			json_object_object_add(jobj, "Wifi5gChannelCurrent", json_object_new_string(buf));
		}
#if HAS_WLAN_5G_2_SETTING
		else
		{
			json_object_object_add(jobj, "Wifi5gChannelCurrent", json_object_new_string(buf));
		}
#endif

		api_get_string_option("wireless.wifi1_ssid_1.encryption", buf, sizeof(buf));
		json_object_object_add(jobj, "Wifi5gEncrType", json_object_new_string(buf));
		api_get_string_option("wireless.wifi1_guest.encryption", buf, sizeof(buf));
		json_object_object_add(jobj, "Wifi5gGuestEncrType", json_object_new_string(buf));
	}


#if HAS_WLAN_5G_2_SETTING
	//5G - band 2
	api_get_integer_option("wireless.wifi2.nochannel", &wifi2_nochannel);
	api_get_integer_option("wireless.wifi2_ssid_1.disabled", &wifi2_disabled);
	{
		api_get_string_option("wireless.wifi2.htmode", buf, sizeof(buf));
		if(wifi2_disabled == 0 && wifi2_nochannel == 0)
		{
			sysutil_interact(buf2, sizeof(buf2),"/sbin/getHTModeList.sh 2 | tr -d '\n'");
			if(strstr(buf2,buf) == NULL)
			{
				sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh get5G2NowHTMode | tr -d '\n'");
			}
		}
		json_object_object_add(jobj, "Wifi5g2Bandwith", json_object_new_string(buf));

		api_get_string_option("wireless.wifi2.channel", buf, sizeof(buf));
		json_object_object_add(jobj, "Wifi5g2Channel", json_object_new_string(buf));
		api_get_string_option("wireless.wifi2_ssid_1.encryption", buf, sizeof(buf));
		json_object_object_add(jobj, "Wifi5g2EncrType", json_object_new_string(buf));
		api_get_string_option("wireless.wifi2_guest.encryption", buf, sizeof(buf));
		json_object_object_add(jobj, "Wifi5g2GuestEncrType", json_object_new_string(buf));
	}

	sprintf(buf, "%d%d", (wifi1_nochannel == 0)?1:0, (wifi2_nochannel == 0)?1:0);
	json_object_object_add(jobj, "5GRadioStatus", json_object_new_string(buf));

	if(strcmp(buf, "00") != 0)
	{
		if(strcmp(buf, "10") == 0) //high band only country
		{
			api_get_integer_option("wireless.wifi1.bandsteerHBpersent", &bandsteerHBpersent);
		}
		else
		{
			api_get_integer_option("wireless.wifi2.bandsteerHBpersent", &bandsteerHBpersent);
		}
		if(bandsteerHBpersent)
		{
			json_object_object_add(jobj, "MeshBackhaul", json_object_new_int((100-bandsteerHBpersent)));
		}
		else
		{
			json_object_object_add(jobj, "MeshBackhaul", json_object_new_int(0));
		}
	}
#else
	json_object_object_add(jobj, "5GRadioStatus", json_object_new_string("00"));
#endif

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);

    return 0;
}

void translate_string_to_integer_array(char *str, int integer[], char *sep)
{
    char *ptr;
    int i;

    i = 0;

    ptr = strtok(str, sep);
    
    while(NULL != ptr)
    {
        integer[i] = atoi(ptr);
        ptr = strtok(NULL, sep);
        i++;
    }

    return;
}

void remove_weather_channel(char *src, char *dest)
{
    int i, j, k;
    int ch[16], ch_weather[16], ch_weather_hz[16], ch_final[16];
    char buf[64];
    char *chan_freq_arr[] = { "5500", "5520", "5540", "5560", "5580", "5600", "5620", "5640", "5660", "5680", "5700"};
    char *chan_arr[] = { "100", "104", "108", "112", "116", "120", "124", "128", "132", "136", "140"};

    memset(ch, 0x00, sizeof(ch));
    memset(ch_weather, 0x00, sizeof(ch_weather));
    memset(ch_weather_hz, 0x00, sizeof(ch_weather_hz));
    memset(ch_final, 0x00, sizeof(ch_final));
    memset(buf, 0x00, sizeof(buf));

    translate_string_to_integer_array(src, ch, ",");

    memset(buf, 0x00, sizeof(buf));
    sysutil_interact(buf, sizeof(buf), "/sbin/getDFSDetectChannel");
    if(strlen(buf))
    {
        translate_string_to_integer_array(buf, ch_weather_hz, "\n");

        for(i = 0; i < 11 && ch_weather_hz[i]; i++)
        {
            for(j = 0; j < 11; j++)
            {
                if(ch_weather_hz[i] == atoi(chan_freq_arr[j]))
                {
                    ch_weather[i] = atoi(chan_arr[j]);
                    break;
                }
            }
        }

        for(i = 0; i < 11 && ch_weather[i]; i++)
        {
            for(j = 0; j < 11; j++)
            {
                if(ch[j] == ch_weather[i])
                {
                    ch[j] = 0;
                }
            }
        }

        for(i = 0, j = 0; i < 11; i++)
        {
            if(0 != ch[i])
            {
                ch_final[j] = ch[i];
                j++;
            }
        }

        for(i = 0, j = 0; i < 11 && ch_final[i]; i++)
        {
            j += sprintf(dest+j, "%d%s", ch_final[i], (ch_final[i+1])?",":"");
        }
    }
    else
    {
        for(i = 0, j = 0; i < 11 && ch[i]; i++)
        {
            j += sprintf(dest+j, "%d%s", ch[i], (ch[i+1])?",":"");
        }   
    }
}

int get_json_wifioptlist_cb(HTTPS_CB *pkt, char *result)
{
	if(NULL == pkt)
	{
		return -1;
	}

	struct json_object *jobj;
	struct json_object *jobj_24GChannelList;
	struct json_object *jobj_5GChannelList;
	struct json_object *jobj_5G2ChannelList;
	char api_result[64]={0};
	char buf[256]={0};
    char buf1[256]={0};
	char tmp[256]={0};
	char ifname[8]={0};
#if HAS_WLAN_5G_2_SETTING
	int wifi1_nochannel=1;
	int wifi2_nochannel=1;
	int wifi1_disabled=1;
	int wifi2_disabled=1;
#endif

	jobj = json_object_new_object();

	sprintf(api_result, "%sResult", pkt->json_action);
	json_object_object_add(jobj, api_result, json_object_new_string(result));

	//2.4G
	api_get_string_option("wireless.wifi0_ssid_1.ifname", ifname, sizeof(ifname));
	sysutil_interact(buf, sizeof(buf), "iwlist %s channel | grep GHz | awk '{printf \"%%d,\", $2}' | tr -d \"\n\"", ifname);
	strncpy(tmp, buf, strlen(buf) - 1);
	sprintf(buf, "auto,%s", tmp);
	json_object_object_add(jobj, "Wifi24gChannelList", json_object_new_string(buf));
    sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh 0 | tr -d '\\n'");
    // buf[strcspn(buf, "\n") - 1] = '\0';
	json_object_object_add(jobj, "Wifi24gBandwidthList", json_object_new_string(buf));
	sprintf(buf, "none,psk2+ccmp");
	json_object_object_add(jobj, "Wifi24gEncrTypeList", json_object_new_string(buf));
	sprintf(buf, "none,psk2+ccmp");
	json_object_object_add(jobj, "Wifi24gGuestEncrTypeList", json_object_new_string(buf));

    jobj_24GChannelList = json_object_new_object();

    //sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh get24GChannelListByHT | tr -d '\n'");
    memset(buf, 0x00, sizeof(buf));
    memset(tmp, 0x00, sizeof(tmp));
    sys_get_bandwidth_channel_list(0, 20, ",", buf, sizeof(buf));
    strncpy(tmp, buf, strlen(buf));
    sprintf(buf, "auto,%s", tmp);
    json_object_object_add(jobj_24GChannelList, "HT20", json_object_new_string(buf));
    
    //sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh get24GChannelListByHT | tr -d '\n'");
    memset(buf, 0x00, sizeof(buf));
    memset(tmp, 0x00, sizeof(tmp));
    sys_get_bandwidth_channel_list(0, 40, ",", buf, sizeof(buf));
    strncpy(tmp, buf, strlen(buf));
    sprintf(buf, "auto,%s", tmp);
    json_object_object_add(jobj_24GChannelList, "HT20_40", json_object_new_string(buf));
    
    //sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh get24GChannelListByHT | tr -d '\n'");
    memset(buf, 0x00, sizeof(buf));
    memset(tmp, 0x00, sizeof(tmp));
    sys_get_bandwidth_channel_list(0, 40, ",", buf, sizeof(buf));
    strncpy(tmp, buf, strlen(buf));
    sprintf(buf, "auto,%s", tmp);
    json_object_object_add(jobj_24GChannelList, "HT40", json_object_new_string(buf));

    json_object_object_add(jobj, "Wifi24gChannelListAll", jobj_24GChannelList);

	//5G
	memset(buf, 0x00, sizeof(buf));
	memset(tmp, 0x00, sizeof(tmp));

#if HAS_WLAN_5G_2_SETTING
	api_get_integer_option("wireless.wifi1.nochannel", &wifi1_nochannel);
	api_get_integer_option("wireless.wifi1_ssid_1.disabled", &wifi1_disabled);
	if(wifi1_disabled == 0 && wifi1_nochannel == 0)
#endif
	{
		api_get_string_option("wireless.wifi1_ssid_1.ifname", ifname, sizeof(ifname));
		sysutil_interact(buf, sizeof(buf), "iwlist %s channel | grep GHz | awk '{printf \"%%d,\", $2}' | tr -d \"\n\"", ifname);
		strncpy(tmp, buf, strlen(buf) - 1);
#if SUPPORT_MESH_AUTO_CHAN
		sprintf(buf, "auto,%s", tmp);
#else
		sprintf(buf, "%s", tmp);
#endif
		json_object_object_add(jobj, "Wifi5gChannelList", json_object_new_string(buf));
		sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh 1 | tr -d '\\n'");
    // buf[strcspn(buf, "\n") - 1] = '\0';
		json_object_object_add(jobj, "Wifi5gBandwidthList", json_object_new_string(buf));
		sprintf(buf, "none,psk2+ccmp");
		json_object_object_add(jobj, "Wifi5gEncrTypeList", json_object_new_string(buf));
		sprintf(buf, "none,psk2+ccmp");
		json_object_object_add(jobj, "Wifi5gGuestEncrTypeList", json_object_new_string(buf));

		jobj_5GChannelList = json_object_new_object();

		//sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh get5GChannelListByHT 20 | tr -d '\n'");
        memset(buf, 0x00, sizeof(buf));
        memset(tmp, 0x00, sizeof(tmp));
        sys_get_bandwidth_channel_list(1, 20, ",", buf, sizeof(buf));
        remove_weather_channel(buf, tmp);
        sprintf(buf, "auto,%s", tmp);
		json_object_object_add(jobj_5GChannelList, "HT20", json_object_new_string(buf));

		//sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh get5GChannelListByHT 40 | tr -d '\n'");
        memset(buf, 0x00, sizeof(buf));
        memset(tmp, 0x00, sizeof(tmp));
        sys_get_bandwidth_channel_list(1, 40, ",", buf, sizeof(buf));
        remove_weather_channel(buf, tmp);
        sprintf(buf, "auto,%s", tmp);
		json_object_object_add(jobj_5GChannelList, "HT40", json_object_new_string(buf));

		//sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh get5GChannelListByHT 80 | tr -d '\n'");
        memset(buf, 0x00, sizeof(buf));
        memset(tmp, 0x00, sizeof(tmp));
        sys_get_bandwidth_channel_list(1, 80, ",", buf, sizeof(buf));
        remove_weather_channel(buf, tmp);
        sprintf(buf, "auto,%s", tmp);
		json_object_object_add(jobj_5GChannelList, "HT80", json_object_new_string(buf));

		json_object_object_add(jobj, "Wifi5gChannelListAll", jobj_5GChannelList);
	}

#if HAS_WLAN_5G_2_SETTING
	//5G-2
	memset(buf, 0x00, sizeof(buf));
	memset(tmp, 0x00, sizeof(tmp));

	api_get_integer_option("wireless.wifi2.nochannel", &wifi2_nochannel);
	api_get_integer_option("wireless.wifi2_ssid_1.disabled", &wifi2_disabled);
	if(wifi2_disabled == 0 && wifi2_nochannel == 0)
	{
		api_get_string_option("wireless.wifi2_ssid_1.ifname", ifname, sizeof(ifname));
		sysutil_interact(buf, sizeof(buf), "iwlist %s channel | grep GHz | awk '{printf \"%%d,\", $2}' | tr -d \"\n\"", ifname);
		strncpy(tmp, buf, strlen(buf) - 1);
#if SUPPORT_MESH_AUTO_CHAN
		sprintf(buf, "auto,%s", tmp);
#else
		sprintf(buf, "%s", tmp);
#endif
		json_object_object_add(jobj, "Wifi5g2ChannelList", json_object_new_string(buf));
		sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh 2 | tr -d '\n'");
		json_object_object_add(jobj, "Wifi5g2BandwidthList", json_object_new_string(buf));
		json_object_object_add(jobj, "Wifi5g2EncrTypeList", json_object_new_string("none,psk2+ccmp"));
		json_object_object_add(jobj, "Wifi5g2GuestEncrTypeList", json_object_new_string("none,psk2+ccmp"));

		jobj_5G2ChannelList = json_object_new_object();

		//sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh get5G2ChannelListByHT 20 | tr -d '\n'");
        memset(buf, 0x00, sizeof(buf));
        memset(tmp, 0x00, sizeof(tmp));
        sys_get_bandwidth_channel_list(2, 20, ",", buf, sizeof(buf));
        strncpy(tmp, buf, strlen(buf));
        sprintf(buf, "auto,%s", tmp);
		json_object_object_add(jobj_5G2ChannelList, "HT20", json_object_new_string(buf));
		
        //sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh get5G2ChannelListByHT 40 | tr -d '\n'");
        memset(buf, 0x00, sizeof(buf));
        memset(tmp, 0x00, sizeof(tmp));
        sys_get_bandwidth_channel_list(2, 40, ",", buf, sizeof(buf));
        strncpy(tmp, buf, strlen(buf));
        sprintf(buf, "auto,%s", tmp);
		json_object_object_add(jobj_5G2ChannelList, "HT40", json_object_new_string(buf));

		//sys_interact(buf, sizeof(buf),"/sbin/getHTModeList.sh get5G2ChannelListByHT 80 | tr -d '\n'");
        memset(buf, 0x00, sizeof(buf));
        memset(tmp, 0x00, sizeof(tmp));
        sys_get_bandwidth_channel_list(2, 80, ",", buf, sizeof(buf));
        strncpy(tmp, buf, strlen(buf));
        sprintf(buf, "auto,%s", tmp);
		json_object_object_add(jobj_5G2ChannelList, "HT80", json_object_new_string(buf));

		json_object_object_add(jobj, "Wifi5g2ChannelListAll", jobj_5G2ChannelList);
	}
#endif

	basic_json_response(pkt, (char *)json_object_to_json_string(jobj));
	json_object_put(jobj);

	return 0;
}
/*****************************************************************
* NAME:    get_json_sc_account_password_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
/*int get_json_sc_account_password_cb(HTTPS_CB *pkt, char *result)
{
    int i;
    struct json_object *jobj;
    char api_result[128]={0};
    char buf[256]={0};
    char password[128]={0};
    char account[128]={0};

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    sysutil_interact(buf, sizeof(buf), "cat /etc/webpasswd | tr -d '\n'");

    sscanf(buf, "%[^:]:%[^:]:%*s",account , password);

    json_object_object_add(jobj, "Account", json_object_new_string(account));
    json_object_object_add(jobj, "Password", json_object_new_string(password));
    
    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);

    return 0;
}*/
/*****************************************************************
 * NAME:    get_system_throughput_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_system_throughput_json_cb(HTTPS_CB *pkt, char *download_rate, char *upload_rate, char *result)
{
    struct json_object *jobj;

    if(NULL == pkt)
    {
        return -1;
    }

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    ADD_JSON_OBJECT_NEW_STRING(jobj, "GetSystemThroughputResult", result);
    ADD_JSON_OBJECT_NEW_STRING(jobj, "DownloadRate", download_rate);
    ADD_JSON_OBJECT_NEW_STRING(jobj, "UploadRate", upload_rate);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return 0;
}

int get_device_status_for_enshare_cb(HTTPS_CB *pkt)
{
	struct json_object *jobj;
	bool mic=0,speaker=0,eventaction=0,scheduleactionenable=0;
	int eventactiontype=0,eventstoragedestination=0,scheduleactiontype=0,schedulestoragedestination=0;
	char model_name[64],wanMac[32], uid[32],fversion[15+1],wantype[10],wanIP[32],wanMask[32],productSN[20],opmode[64],lanIP[32],lanMask[32],lanMac[32];
        char wifi_24g_ssid[64], wifi_24g_key[64], wifi_24g_encryption[32], wifi_5g_ssid[64], wifi_5g_key[64], wifi_5g_encryption[32];
        char wan_interface[5]={0};
	char *result = NULL,*ptr = NULL,*str_ptr=NULL;
	char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
        char opmode_tmp[3]={0};
#if HAS_SSL
	int appAgentService=AGENT_SUPPORT_HTTPS_ONLY;
#endif

	if(NULL == pkt)
		return -1;
	
	result = OK_STR;
//printf(" *** response(%d)=[%s]",strlen(response),response);

	/* Construct the packet content in json format. */
        api_get_string_option("system.@system[0].opmode", opmode_tmp, sizeof(opmode_tmp));
        if ( strcmp(opmode_tmp,"ar") == 0) 
        {
            api_get_string_option("network.wan.ifname", wan_interface, sizeof(wan_interface));
        }
        else
        {
            sysutil_interact(wan_interface,sizeof(wan_interface), "cat /tmp/wandev");
        }

	jobj = json_object_new_object();

	json_object_object_add(jobj, "GetDeviceStatusResult",json_object_new_string("OK"));

	memset(model_name,0x0,sizeof(model_name));
	api_get_string_option(SYSPRODUCTINFO_MODEL_MODELNAME_OPTION,model_name,sizeof(model_name));
	json_object_object_add(jobj, "ModelName",json_object_new_string(model_name));

	memset(productSN,0x0,sizeof(productSN));
        sysutil_interact(productSN, sizeof(productSN),"setconfig -g 0"); 
        productSN[strcspn(productSN, "\n") - 1] = '\0';
	json_object_object_add(jobj, "ProductSN",json_object_new_string(productSN));

	memset(fversion,0x0,sizeof(fversion));
	sysutil_get_firmware_version_info(fversion, sizeof(fversion));
	json_object_object_add(jobj, "FirmwareVersion",json_object_new_string(fversion));

        memset(opmode,0x0,sizeof(opmode));
        sysutil_interact(opmode, sizeof(opmode),"opmode.sh r | tail -n 1"); 
        opmode[strcspn(opmode, "\n") - 1] = '\0';
	json_object_object_add(jobj, "OperationMode",json_object_new_int(atoi(opmode)));

    memset(wanMac,0x0,sizeof(wanMac));
    sysutil_get_interface_mac(wan_interface, wanMac, sizeof(wanMac));
    json_object_object_add(jobj, "WanMacAddress",json_object_new_string(wanMac));

	memset(wantype,0x0,sizeof(wantype));
	api_get_string_option(NETWORK_WAN_PROTO_OPTION,wantype,sizeof(wantype));
	json_object_object_add(jobj, "WanType",json_object_new_string(wantype));

    if(0 == strcmp("pppoe", wantype))
    {
        memset(wan_interface, 0x00, sizeof(wan_interface));
        sprintf(wan_interface, "pppoe-wan");
    }

	memset(wanIP,0x0,sizeof(wanIP));
        sysutil_get_interface_ipaddr(wan_interface, wanIP, sizeof(wanIP));
        //sysutil_get_wan_ipaddr(wanIP, sizeof(wanIP)); 
	json_object_object_add(jobj, "WanIPAddress",json_object_new_string(wanIP));

	memset(wanMask,0x0,sizeof(wanMask));
	//sysutil_get_wan_netmask(wanMask, sizeof(wanMask));
        sysutil_get_interface_netmask(wan_interface, wanMask, sizeof(wanMask));
        json_object_object_add(jobj, "WanSubnetMask",json_object_new_string(wanMask));

	memset(lanIP,0x0,sizeof(lanIP));
	sysutil_get_lan_ipaddr(lanIP, sizeof(lanIP)); 
	json_object_object_add(jobj, "LanIPAddress",json_object_new_string(lanIP));

	memset(lanMask,0x0,sizeof(lanMask));
	sysutil_get_lan_netmask(lanMask, sizeof(lanMask));
        json_object_object_add(jobj, "LanSubnetMask",json_object_new_string(lanMask));
  
	memset(lanMac,0x0,sizeof(lanMac));
	sysutil_interact(lanMac, sizeof(lanMac), "ifconfig br-lan | grep \"HWaddr\" | awk \'{print $5}\'"); 
	if(NULL != (ptr = strchr(lanMac, '\n')))
		*ptr = '\0';
	json_object_object_add(jobj, "LanMacAddress",json_object_new_string(lanMac));

      	api_get_string_option(WIRELESS_WIFI24G_SSID, wifi_24g_ssid, sizeof(wifi_24g_ssid));
	json_object_object_add(jobj, "2.4GHzSSID",json_object_new_string(wifi_24g_ssid));

      	api_get_string_option(WIRELESS_WIFI24G_KEY, wifi_24g_key, sizeof(wifi_24g_key));
	json_object_object_add(jobj, "2.4GHzKEY",json_object_new_string(wifi_24g_key));

        api_get_string_option(WIRELESS_WIFI24G_ENCRYPTION, wifi_24g_encryption, sizeof(wifi_24g_encryption));
	json_object_object_add(jobj, "2.4GHzSecurity",json_object_new_string(wifi_24g_encryption));

        api_get_string_option(WIRELESS_WIFI5G_SSID, wifi_5g_ssid, sizeof(wifi_5g_ssid));
	json_object_object_add(jobj, "5GHzSSID",json_object_new_string(wifi_5g_ssid));

      	api_get_string_option(WIRELESS_WIFI5G_KEY, wifi_5g_key, sizeof(wifi_5g_key));
	json_object_object_add(jobj, "5GHzKEY",json_object_new_string(wifi_5g_key));

        api_get_string_option(WIRELESS_WIFI5G_ENCRYPTION, wifi_5g_encryption, sizeof(wifi_5g_encryption));
	json_object_object_add(jobj, "5GHzSecurity",json_object_new_string(wifi_5g_encryption));


#if HAS_WLAN_DONGLE
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "WlanSupport", TRUE);
#else
	ADD_JSON_OBJECT_NEW_BOOL(jobj, "WlanSupport", FALSE);
#endif

        /* Store packet content into buffer and send it out */
	http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
	http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
	http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));
	http_send_stored_data(pkt->fd);

	/* Free obj */
	json_object_put(jobj);
	return 0;
}

/*****************************************************************
* NAME:    get_blocked_client_list_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
void get_blocked_client_list_json_cb(HTTPS_CB *pkt, BLOCKED_CLIENT_LIST_T *setting)
{
    int i;
    struct json_object *jobj;
    struct json_object *jarr_blocked_list, *jobj_blocked_client;

    jobj = json_object_new_object();

    json_object_object_add(jobj, "GetBlockedClientListResult", json_object_new_string(OK_STR));
    json_object_object_add(jobj, "Enabled", json_object_new_boolean(setting->enabled));
    //json_object_object_add(jobj, "IsAllowList", json_object_new_boolean(setting->is_allow_list));

    jarr_blocked_list = json_object_new_array();

    for(i = 0; i < MAX_CLIENT_DEVICE; i++)
    {
        jobj_blocked_client = json_object_new_object();

        if((0 == strlen(setting->mac[i])) || (0 == strcmp(setting->mac[i], "00:00:00:00:00:00")))
        {
            break;
        }

        json_object_object_add(jobj_blocked_client, "MacAddress", json_object_new_string(setting->mac[i]));
        json_object_object_add(jobj_blocked_client, "DeviceName", json_object_new_string(setting->device_name[i]));

        json_object_array_add(jarr_blocked_list, jobj_blocked_client);
    }

    json_object_object_add(jobj, "BlockedClientList", jarr_blocked_list);

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);
}
