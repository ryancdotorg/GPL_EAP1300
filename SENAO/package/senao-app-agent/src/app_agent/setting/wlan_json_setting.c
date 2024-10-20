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
;    Creator :
;    File    : json_wlan_setting.c
;    Abstract:
;
;       Modification History:
;       By              Date        Ver.    Modification Description
;       --------------- --------    -----   -------------------------------------
;                       2015/12/18          First commit
;****************************************************************************/

/*--------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                           */
/*--------------------------------------------------------------------------*/
#include "wlan_json_setting.h"

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
/*****************************************************************
* NAME:    get_json_mesh_device_wireless_settings_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool get_device_wireless_settings_json_cb(struct json_object *jarr, WLAN_RADIO_SETTINGS_T setting[], WLAN_RADIO_SECURITY_T security[])
{
    struct json_object *jarr_obj;
    struct json_object *jstr;

    if(NULL == jarr)
    {
        return FALSE;
    }

    jarr_obj = json_object_new_object();

    json_object_object_add(jarr_obj, "Band", json_object_new_string(WLAN_RADIO_2_4_GHZ_STR));
    json_object_object_add(jarr_obj, "Enabled", json_object_new_boolean(setting[0].enabled));
    json_object_object_add(jarr_obj, "SSID", json_object_new_string(setting[0].ssid));
    json_object_object_add(jarr_obj, "SSIDBroadcast", json_object_new_boolean(setting[0].ssid_broadcast));
    json_object_object_add(jarr_obj, "MacAddress", json_object_new_string((setting[0].mac)?setting[0].mac:""));
    json_object_object_add(jarr_obj, "ChannelWidth", json_object_new_int(setting[0].channel_width));
    json_object_object_add(jarr_obj, "Channel", json_object_new_int(setting[0].channel));
    json_object_object_add(jarr_obj, "SecurityEnabled", json_object_new_boolean(security[0].enabled));
    json_object_object_add(jarr_obj, "WPSEnabled", json_object_new_boolean(security[0].wps_enabled));
    json_object_object_add(jarr_obj, "SecurityType", json_object_new_string(security[0].type));
    json_object_object_add(jarr_obj, "Encryption", json_object_new_string(security[0].encryption));
    json_object_object_add(jarr_obj, "Key", json_object_new_string(security[0].key));
    json_object_object_add(jarr_obj, "RadiusIP1", json_object_new_string(security[0].radius_ip1));
    json_object_object_add(jarr_obj, "RadiusPort1", json_object_new_int(security[0].radius_port1));
    json_object_object_add(jarr_obj, "RadiusSecret1", json_object_new_string(security[0].radius_secret1));

    json_object_array_add(jarr, jarr_obj);

#if SUPPORT_WLAN_5G_SETTING
    jarr_obj = NULL;
    jarr_obj = json_object_new_object();

    json_object_object_add(jarr_obj, "Band", json_object_new_string(WLAN_RADIO_5_GHZ_STR));
    json_object_object_add(jarr_obj, "Enabled", json_object_new_boolean(setting[1].enabled));
    json_object_object_add(jarr_obj, "SSID", json_object_new_string(setting[1].ssid));
    json_object_object_add(jarr_obj, "SSIDBroadcast", json_object_new_boolean(setting[1].ssid_broadcast));
    json_object_object_add(jarr_obj, "MacAddress", json_object_new_string((setting[1].mac)?setting[1].mac:""));
    json_object_object_add(jarr_obj, "ChannelWidth", json_object_new_int(setting[1].channel_width));
    json_object_object_add(jarr_obj, "Channel", json_object_new_int(setting[1].channel));
    json_object_object_add(jarr_obj, "SecurityEnabled", json_object_new_boolean(security[1].enabled));
    json_object_object_add(jarr_obj, "WPSEnabled", json_object_new_boolean(security[1].wps_enabled));
    json_object_object_add(jarr_obj, "SecurityType", json_object_new_string(security[1].type));
    json_object_object_add(jarr_obj, "Encryption", json_object_new_string(security[1].encryption));
    json_object_object_add(jarr_obj, "Key", json_object_new_string(security[1].key));
    json_object_object_add(jarr_obj, "RadiusIP1", json_object_new_string(security[1].radius_ip1));
    json_object_object_add(jarr_obj, "RadiusPort1", json_object_new_int(security[1].radius_port1));
    json_object_object_add(jarr_obj, "RadiusSecret1", json_object_new_string(security[1].radius_secret1));

    json_object_array_add(jarr, jarr_obj);
#endif

    return TRUE;
}
/*****************************************************************
* NAME:    parse_json_wlan_radio_settings
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_json_wlan_radio_settings(char *query_str, WLAN_RADIOS_T *settings, char *return_str)
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
#if APP_AGENT_SUPPORT_ENSHARE
#else
                if(is5g)
                    ssid_id = ssid_id - (WIFI_SSID_NUM+1);
#endif
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

            result = senao_json_object_get_boolean(jobj, "Enabled", &(settings->radio_enabled));
            
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
* NAME:    parse_json_wlan_radio_security
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_json_wlan_radio_security(char *query_str, WLAN_RADIOS_T *settings, char *return_str)
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
#if APP_AGENT_SUPPORT_ENSHARE
#else
                if(is5g)
                    ssid_id = ssid_id - (WIFI_SSID_NUM+1);
#endif
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

            result = senao_json_object_get_boolean(jobj, "Enabled", &(settings->radio_enabled));
            
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
