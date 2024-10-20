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
;    File    : json_wan_setting.c
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
#include "lan_json_setting.h"

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
* NAME:    get_device_lan_settings_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool get_device_lan_settings_json_cb(struct json_object *jobj, ROUTER_LAN_SETTINGS_T *setting)
{
    if(NULL == jobj)
    {
        return FALSE;
    }

    /* Construct the packet content in json format. */
    json_object_object_add(jobj, "IPAddress", json_object_new_string(setting->router_ip_address));
    json_object_object_add(jobj, "SubnetMask", json_object_new_string(setting->router_subnet_mask));
    json_object_object_add(jobj, "DHCPServerEnabled", json_object_new_boolean(setting->dhcp_server_enabled));
    json_object_object_add(jobj, "DHCPServerStartIP", json_object_new_string(setting->dhcp_start));
    json_object_object_add(jobj, "DHCPServerEndIP", json_object_new_string(setting->dhcp_end));

    return TRUE;
}

/*****************************************************************
* NAME:    parse_blocked_client_list_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_blocked_client_list_json_cb(char *query_str, BLOCKED_CLIENT_LIST_T *setting,int *total_num)
{
    bool result, is_jobj;
    int i, count;
    struct json_object *jobj, *jobj_mac_info, *jarr_blocked_client_list;
    struct array_list *array_mac_info;

    result = TRUE;
    is_jobj = FALSE;

    /****************** Packet format ******************
     * {                                               *
     *   "Enabled" : "boolean",                        *
     *   "IsAllowList" : "boolean",                    *
     *   "BlockedClientList" : [                       *
     *     {                                           *
     *       "MacAddress" : "string",                  *
     *       "DeviceName" : "string"                   *
     *     },                                          *
     *     {                                           *
     *       "MacAddress" : "string",                  *
     *       "DeviceName" : "string"                   *
     *     }                                           *
     *   ]                                             *
     * }                                               *
     ***************************************************/

    if(NULL != query_str)
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            is_jobj = TRUE;

            result = senao_json_object_get_boolean(jobj, "Enabled", &(setting->enabled));
            /* If the parsing failed or the function is disabled, then the rest parsing is unnecessary */
            if((FALSE == result) || (FALSE == setting->enabled)) goto out;

            //result = senao_json_object_get_boolean(jobj, "IsAllowList", &(setting->is_allow_list));
            //if(FALSE == result) goto out;

            /* Get the content of array "MacInfo" and parse these content to             *
             * corresponding parameter in structure.                                     */
            jarr_blocked_client_list = json_object_object_get(jobj, "BlockedClientList");
            array_mac_info = json_object_get_array(jarr_blocked_client_list);

            count = (MAX_CLIENT_DEVICE < array_mac_info->length)?MAX_CLIENT_DEVICE:array_mac_info->length;
            *total_num = count;
            for(i = 0; i < count; i++)
            {
                jobj_mac_info = json_object_array_get_idx(jarr_blocked_client_list, i);

                senao_json_object_get_string(jobj_mac_info, "MacAddress", setting->mac[i]);
                senao_json_object_get_string(jobj_mac_info, "DeviceName", setting->device_name[i]);
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
