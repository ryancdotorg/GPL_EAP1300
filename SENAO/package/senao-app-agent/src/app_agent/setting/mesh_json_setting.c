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
;    File    : mesh_json_setting.c
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
#include "json.h"
#include "sysString.h"
#include "deviceinfo.h"
#include "mesh_setting.h"
#include "mesh_json_setting.h"
#include "json_setting.h"
#include "device_json_setting.h"

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
/*****************************************************************
* NAME:    disable_broadcast_in_query_str
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool disable_broadcast_in_query_str(char *query_str, char *result_query_str)
{
    bool result;
    struct json_object *jobj;
    struct json_object *jobj_broadcast;

    result = FALSE;

    if(query_str)
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            json_object_object_del(jobj, "Broadcast");
            json_object_object_add(jobj, "Broadcast", json_object_new_boolean(0));

            sprintf(result_query_str, "%s", json_object_to_json_string(jobj));

            /* Free object */
            json_object_put(jobj);
        }
    }

    return result;
}

/*****************************************************************
* NAME:    get_json_mesh_account_json
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool get_json_mesh_account(char *query_str, char *user_name, char *password)
{
    bool result;
    struct json_object *jobj;

    result = FALSE;

    if(query_str)
    {
        if((jobj = json_tokener_parse(query_str)))
        {
            if(senao_json_object_get_string(jobj, "MeshAdminUsername", user_name))
            {
                if(senao_json_object_get_string(jobj, "MeshAdminPassword", password))
                {
                    result = TRUE;
                }
            }

            /* Free object */
            json_object_put(jobj);
        }
    }

    return result;
}

/*****************************************************************
* NAME:    parse_json_mesh_wireless_setting
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_json_mesh_wireless_setting(char *query_str, char *name, WLAN_RADIOS_T *setting, char **return_str)
{
    char band[8];
    struct json_object *jobj;
    struct json_object *jobj_wlan;
    struct json_object *jobj_wlan_obj;

    if((jobj = json_tokener_parse(query_str)))
    {
        *return_str = OK_STR;

        jobj_wlan = json_object_object_get(jobj, name);

        if(FALSE == senao_json_object_get_string(jobj_wlan, "Band", band))
        {
            *return_str = ERROR_BAD_BAND_STR;
        }
        if(FALSE == senao_json_object_get_string(jobj_wlan, "SSID", setting->ssid))
        {
            *return_str = ERROR_BAD_SSID_STR;
        }
        if(FALSE == senao_json_object_get_integer(jobj_wlan, "Channel", &(setting->channel)))
        {
            *return_str = ERROR_BAD_CHANNEL_STR;
        }
        if(FALSE == senao_json_object_get_string(jobj_wlan, "Type", setting->type))
        {
            *return_str = ERROR_TYPE_NOT_SUPPORTED_STR;
        }
        if(FALSE == senao_json_object_get_string(jobj_wlan, "Key", setting->key))
        {
            *return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
        }

        /* Free object */
        json_object_put(jobj);
    }

    return 0;
}

/*****************************************************************
* NAME:    parse_json_wireless_setting
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_json_wireless_setting(char *query_str, char *name, WLAN_RADIOS_T setting[], char **return_str)
{
    int i = 0, count = 0;
    bool result = FALSE;
    char radio_band[8];
    struct json_object *jobj;
    struct json_object *jarr;
    struct json_object *jarr_obj;
    struct array_list *array_setting;

    if((NULL == query_str) || (0 == strlen(query_str)))
    {
        return result;
    }

    if((jobj = json_tokener_parse(query_str)))
    {
        memset(radio_band, 0x00, sizeof(radio_band));
        *return_str = OK_STR;
        result = TRUE;

        jarr = json_object_object_get(jobj, name);
        array_setting = json_object_get_array(jarr);

        count = array_setting->length;

        for(i = 0; i < count; i++)
        {
            jarr_obj = json_object_array_get_idx(jarr, i);

            if(FALSE == senao_json_object_get_string(jarr_obj, "Band", radio_band))
            {
                *return_str = ERROR_BAD_BAND_STR;
                result = FALSE;
            }

            if(0 == strcasecmp(WLAN_RADIO_2_4_GHZ_STR, radio_band))
            {
                senao_json_object_get_boolean(jarr_obj, "Enabled", &(setting[0].radio_enabled));
                if(FALSE == senao_json_object_get_string(jarr_obj, "SSID", setting[0].ssid))
                {
                    *return_str = ERROR_BAD_SSID_STR;
                    result = FALSE;
                }
                senao_json_object_get_boolean(jarr_obj, "SSIDBroadcast", &(setting[0].ssid_broadcast));
                senao_json_object_get_integer(jarr_obj, "ChannelWidth", &(setting[0].channel_width));
                if(FALSE == senao_json_object_get_integer(jarr_obj, "Channel", &(setting[0].channel)))
                {
                    *return_str = ERROR_BAD_CHANNEL_STR;
                    result = FALSE;
                }

                senao_json_object_get_boolean(jarr_obj, "SecurityEnabled", &(setting[0].authentication_enabled));
                if(FALSE == senao_json_object_get_string(jarr_obj, "Type", setting[0].type))
                {
                    *return_str = ERROR_TYPE_NOT_SUPPORTED_STR;
                    result = FALSE;
                }
                if(FALSE == senao_json_object_get_string(jarr_obj, "Encryption", setting[0].encryption))
                {
                    *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                    result = FALSE;
                }
                if(FALSE == senao_json_object_get_string(jarr_obj, "Key", setting[0].key))
                {
                    *return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                    result = FALSE;
                }
                if(FALSE == senao_json_object_get_string(jarr_obj, "RadiusIP", setting[0].radius_ip1))
                {
                    result = FALSE;
                }
                if(0 != strlen(setting[0].radius_ip1))
                {
                    senao_json_object_get_integer(jarr_obj, "RadiusPort", &(setting[0].radius_port1));
                    if(FALSE == senao_json_object_get_string(jarr_obj, "RadiusSecret", setting[0].radius_secret1))
                    {
                        *return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        result = FALSE;
                    }
                }
            }
#ifdef WIFI_5G_IF
            else if(0 == strcasecmp(WLAN_RADIO_5_GHZ_STR, radio_band))
            {
                senao_json_object_get_boolean(jarr_obj, "Enabled", &(setting[1].radio_enabled));
                if(FALSE == senao_json_object_get_string(jarr_obj, "SSID", setting[1].ssid))
                {
                    *return_str = ERROR_BAD_BAND_STR;
                    result = FALSE;
                }
                senao_json_object_get_boolean(jarr_obj, "SSIDBroadcast", &(setting[1].ssid_broadcast));
                senao_json_object_get_integer(jarr_obj, "ChannelWidth", &(setting[1].channel_width));
                if(FALSE == senao_json_object_get_integer(jarr_obj, "Channel", &(setting[1].channel)))
                {
                    *return_str = ERROR_BAD_CHANNEL_STR;
                    result = FALSE;
                }

                senao_json_object_get_boolean(jarr_obj, "SecurityEnabled", &(setting[1].authentication_enabled));
                if(FALSE == senao_json_object_get_string(jarr_obj, "Type", setting[1].type))
                {
                    *return_str = ERROR_TYPE_NOT_SUPPORTED_STR;
                    result = FALSE;
                }
                if(FALSE == senao_json_object_get_string(jarr_obj, "Encryption", setting[1].encryption))
                {
                    *return_str = ERROR_ENCRYPTION_NOT_SUPPORTED_STR;
                    result = FALSE;
                }
                if(FALSE == senao_json_object_get_string(jarr_obj, "Key", setting[1].key))
                {
                    *return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                    result = FALSE;
                }
                if(FALSE == senao_json_object_get_string(jarr_obj, "RadiusIP", setting[1].radius_ip1))
                {
                    result = FALSE;
                }
                if(0 != strlen(setting[1].radius_ip1))
                {
                    senao_json_object_get_integer(jarr_obj, "RadiusPort", &(setting[1].radius_port1));
                    if(FALSE == senao_json_object_get_string(jarr_obj, "RadiusSecret", setting[1].radius_secret1))
                    {
                        *return_str = ERROR_ILLEGAL_KEY_VALUE_STR;
                        result = FALSE;
                    }
                }
            }
#endif
            else
            {
                *return_str = ERROR_BAD_BAND_STR;
                result = FALSE;
            }
        }

        /* Free object */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
* NAME:    parse_json_mesh_network_settings_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_json_mesh_network_settings_cb(char *query_str, WAN_SETTINGS_T *wan_setting, char **return_str)
{
    int i = 0, count = 0;
    bool result = FALSE;
    char radio_band[8];
    struct json_object *jobj;
    struct json_object *jobj_internet;
    struct json_object *jobj_dns;
    struct json_object *jarr;
    struct json_object *jarr_obj;

    if((NULL == query_str) || (0 == strlen(query_str)))
    {
        return result;
    }

    if((jobj = json_tokener_parse(query_str)))
    {
        *return_str = OK_STR;

        jobj_internet = json_object_object_get(jobj, "InternetSettings");

        if(FALSE == senao_json_object_get_string(jobj_internet, "ConnectionType", wan_setting->type))
        {
            *return_str = ERROR_STR;
            goto out;
        }
        if(FALSE == senao_json_object_get_string(jobj_internet, "IPAddress", wan_setting->ip_address))
        {
            *return_str = ERROR_IP_ADDRESS_STR;
            goto out;
        }
        if(FALSE == senao_json_object_get_string(jobj_internet, "SubnetMask", wan_setting->subnet_mask))
        {
            *return_str = ERROR_SUBNET_MASK_STR;
            goto out;
        }
        if(FALSE == senao_json_object_get_string(jobj_internet, "Gateway", wan_setting->gateway))
        {
            *return_str = ERROR_GATEWAY_STR;
            goto out;
        }
        if(FALSE == senao_json_object_get_string(jobj_internet, "UserName", wan_setting->user_name))
        {
            *return_str = ERROR_USER_NAME_STR;
            goto out;
        }
        if(FALSE == senao_json_object_get_string(jobj_internet, "Password", wan_setting->password))
        {
            *return_str = ERROR_PASSWORD_STR;
            goto out;
        }

        jobj_dns = json_object_object_get(jobj_internet, "DNS");

        if(FALSE == senao_json_object_get_string(jobj_dns, "Primary", wan_setting->dns_primary))
        {
            *return_str = ERROR_DNS1_IP_ADDRESS_STR;
            goto out;
        }
        if(FALSE == senao_json_object_get_string(jobj_dns, "Secondary", wan_setting->dns_secondary))
        {
            *return_str = ERROR_DNS2_IP_ADDRESS_STR;
            goto out;
        }

        result = TRUE;
    }

out:
    if(jobj)
    {
        /* Free object */
        json_object_put(jobj);
    }

    return result;
}

/*****************************************************************
* NAME:    get_json_mesh_node_info_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_mesh_node_info_cb(HTTPS_CB *pkt, MESH_DEVICE_SETTINGS_T setting, MESH_DEVICE_NEIGHBORS_T neighbor, MESH_GUEST_CLIENT_INFO_T gClientInfo, char client_list[][32], char *result)
{
    int i;
    struct json_object *jobj;
    struct json_object *jobj_device;
    struct json_object *jobj_guestClient;
    struct json_object *jarr_obj;
#if SUPPORT_AP_MESH_PROJECT
    struct json_object *jarr, *jarr_mesh_link,*jarr_mesh_learn_table;
    struct json_object *jobj_mesh_group_info, *jobj_mesh_learn_table, *jobj_mesh_link;
#else
    struct json_object *jarr;
#endif
    char api_result[64];
    char buf[256]={0};
    char interface_name[32];
    char mastermac[128]={0};
    char fw_ver1[256]={0}, fw_ver2[256]={0};
    int wifi0=0, wifi1=0, light=0, TQ=255;
    int cnt;
    char upTime[128]={0};
    int days=0, hours=0, minutes=0, seconds=0;

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

#if SUPPORT_AP_MESH_PROJECT
/***************** mesh_group_info **************************/

    jobj_mesh_group_info = json_object_new_object();

    json_object_object_add(jobj_mesh_group_info, "MacAddress", json_object_new_string(setting.mac));
    json_object_object_add(jobj_mesh_group_info, "MeshRole", json_object_new_string(setting.mesh_role));
    json_object_object_add(jobj_mesh_group_info, "Mesh_Channel", json_object_new_int(setting.mesh_channel));
    json_object_object_add(jobj_mesh_group_info, "Robust_Enable", json_object_new_boolean(setting.enable));
    json_object_object_add(jobj_mesh_group_info, "Robust_Threshold", json_object_new_int(setting.threshold));
    json_object_object_add(jobj_mesh_group_info, "Gw_Mac_Addr", json_object_new_string(setting.gw_mac_addr));
    json_object_object_add(jobj_mesh_group_info, "NextHopMac", json_object_new_string(setting.next_hop_mac));
    json_object_object_add(jobj_mesh_group_info, "LANMacAddress", json_object_new_string(setting.lan_mac));
    json_object_object_add(jobj_mesh_group_info, "MeshController", json_object_new_string(setting.mesh_controller));

    jarr_mesh_learn_table = json_object_new_array();
    i = 0;
    while(0 != strlen(neighbor.mac[i]))
    {
        jobj_mesh_learn_table = json_object_new_object();
        json_object_object_add(jobj_mesh_learn_table, "Mac_Addr", json_object_new_string(neighbor.mac[i]));
        json_object_object_add(jobj_mesh_learn_table, "Learn_Rssi", json_object_new_int(neighbor.rssi[i]));
        json_object_object_add(jobj_mesh_learn_table, "Link_Flags", json_object_new_string(neighbor.flag[i]));
        json_object_array_add(jarr_mesh_learn_table, jobj_mesh_learn_table);
        i++;
    }
    json_object_object_add(jobj_mesh_group_info, "mesh_learn_table", jarr_mesh_learn_table);

    jarr_mesh_link = json_object_new_array();
    i = 0;
    while(0 != strlen(setting.link_mac_addr[i]))
    {
        jobj_mesh_link = json_object_new_object();
        json_object_object_add(jobj_mesh_link, "Mac_Addr", json_object_new_string(setting.link_mac_addr[i]));
        json_object_object_add(jobj_mesh_link, "Rssi", json_object_new_int(neighbor.rssi[i]));
        json_object_object_add(jobj_mesh_link, "Tq_Val", json_object_new_int(setting.tq_val[i]));
        json_object_object_add(jobj_mesh_link, "Tx_Datarate", json_object_new_int(atoi(setting.tx_rate[i])));
        json_object_object_add(jobj_mesh_link, "Rx_Datarate", json_object_new_int(atoi(setting.rx_rate[i])));
        json_object_array_add(jarr_mesh_link, jobj_mesh_link);
        i++;
    }
    json_object_object_add(jobj_mesh_group_info, "mesh_link", jarr_mesh_link);

    json_object_object_add(jobj, "MeshGroupInfo", jobj_mesh_group_info);

/***************** mesh_group_info end**************************/
#endif

    jobj_device = json_object_new_object();

    json_object_object_add(jobj_device, "DeviceType", json_object_new_int(setting.device_type));
    json_object_object_add(jobj_device, "DeviceName", json_object_new_string(setting.device_name));
    json_object_object_add(jobj_device, "LocationName", json_object_new_string(setting.location_name));
    json_object_object_add(jobj_device, "PN", json_object_new_string(setting.product_name));
    json_object_object_add(jobj_device, "MeshRole", json_object_new_string(setting.mesh_role));
    json_object_object_add(jobj_device, "RootHopCount", json_object_new_int(setting.root_hop_count));
    json_object_object_add(jobj_device, "LANIPAddress", json_object_new_string(setting.lan_ip_address));
    json_object_object_add(jobj_device, "LANMacAddress", json_object_new_string(setting.lan_mac));
    json_object_object_add(jobj_device, "MacAddress", json_object_new_string(setting.mac));
    json_object_object_add(jobj_device, "UID", json_object_new_string(setting.uid));
    json_object_object_add(jobj_device, "MeshController", json_object_new_string(setting.mesh_controller));
    json_object_object_add(jobj_device, "NextHopRssi", json_object_new_int(setting.next_hop_rssi));
    json_object_object_add(jobj_device, "NextHopMac", json_object_new_string(setting.next_hop_mac));
    json_object_object_add(jobj_device, "NextHopTrueRssi", json_object_new_int(setting.next_hop_true_rssi));
    json_object_object_add(jobj_device, "BatmanSt", json_object_new_int(setting.bat_st));
    json_object_object_add(jobj_device, "RPSt", json_object_new_int(setting.rp_st));
    json_object_object_add(jobj_device, "RPMAC", json_object_new_string(setting.rpap_mac));
    json_object_object_add(jobj_device, "TrueWANMAC", json_object_new_string(setting.true_wan_mac));

    if (api_get_integer_option("wireless.wifi0_ssid_1.disabled", &wifi0))
    {
        wifi0 = 1;
    }

    if (api_get_integer_option("wireless.wifi1_ssid_1.disabled", &wifi1))
    {
        wifi1 = 1;
    }

    if (wifi0 ==1 && wifi1 == 1)
    {
        json_object_object_add(jobj_device, "WiFiStatus", json_object_new_int(0));
    }
    else
    {
        json_object_object_add(jobj_device, "WiFiStatus", json_object_new_int(1));
    }

    if (api_get_integer_option("system.@system[0].basic_led_status", &light))
    {
        light = 0;
    }

    json_object_object_add(jobj_device, "LedStatus", json_object_new_int(light));

    sysutil_interact(buf, sizeof(buf), "cat /etc/version | grep Firmware | awk '{print $4}'");
    buf[strlen(buf)-1]=0;
    json_object_object_add(jobj_device, "FullFwVersion", json_object_new_string(buf));

    sysutil_interact(interface_name, sizeof(interface_name), "getinfo mesh_ifname | tr -d '\n'");
    sysutil_interact(buf, sizeof(buf), "iwlist %s chan | grep 'Current' | awk {'print $2'}| tr -d '\n'", interface_name);
    json_object_object_add(jobj_device, "MeshChannel", json_object_new_int(atoi(buf)));

    if(sysutil_check_file_existed("/tmp/currentNextHopSpeed"))
    {
       sysutil_interact(buf, sizeof(buf), "cat /tmp/currentNextHopSpeed |awk {'print $2'}| tr -d '\n'");
       json_object_object_add(jobj_device, "NextHopMeshSpeed", json_object_new_string(buf));
    }
    else
    {
        json_object_object_add(jobj_device, "NextHopMeshSpeed", json_object_new_string(""));
    }

#if HAS_MESH_EZSETUP_FOR_AP_MODE
    json_object_object_add(jobj_device, "WANMacAddress", json_object_new_string(setting.lan_mac));
#else
    sysutil_interact(buf, sizeof(buf), "setconfig -g 7");
    buf[strlen(buf)-1]=0;
    json_object_object_add(jobj_device, "WANMacAddress", json_object_new_string(buf));
#endif

    sysutil_interact(buf, sizeof(buf), "cat /etc/version | grep Firmware | awk '{print $4}'");
    sscanf(buf,"%[^.].%[^.].%*s\n",fw_ver1,fw_ver2);
    sprintf(buf,"%s.%s", fw_ver1, fw_ver2);
    json_object_object_add(jobj_device, "FwVersion", json_object_new_string(buf));

    if (strcmp(setting.mesh_controller, "master") == 0)
    {
        json_object_object_add(jobj_device, "TQ", json_object_new_int(TQ));
        json_object_object_add(jobj_device, "NextHopTQ", json_object_new_int(TQ));
    }
    else
    {
#if SUPPORT_BATMAN_2019
        sysutil_interact(mastermac, sizeof(mastermac), "batctl gwl -H | grep \"=>\|^*\" | awk '{print $2}' | grep -E \"^[a-f0-9:]{17}$\"");
#else
        sysutil_interact(mastermac, sizeof(mastermac), "batctl gwl -H | grep \"=>\" | awk '{print $2}' | grep -E \"^[a-f0-9:]{17}$\"");
#endif
        mastermac[strlen(mastermac)-1]=0;
#if SUPPORT_BATMAN_2019

        sysutil_interact(buf, sizeof(buf), "batctl o | grep \"*\" | grep ^%s |awk '{print $4}'| awk -F \"(\" '{print $2}'|awk -F \")\" '{print $1}'",mastermac);
#else
        sysutil_interact(buf, sizeof(buf), "batctl o | grep ^%s |awk '{print $3}'| awk -F \"(\" '{print $2}'|awk -F \")\" '{print $1}'",mastermac);
#endif
        buf[strlen(buf)-1]=0;

        json_object_object_add(jobj_device, "TQ", json_object_new_int(atoi(buf)));
        json_object_object_add(jobj_device, "NextHopTQ", json_object_new_int(setting.nexthop_tq_val));
    }

	if (sysutil_get_uptime(upTime, sizeof(upTime)))
	{
		sscanf(upTime, "%d days ,%d hours ,%d minutes ,%d seconds", &days, &hours, &minutes, &seconds);
		if(days > 0)
				sprintf(upTime, "%d days %d hours %d min %ld sec", days, hours, minutes, seconds);
		else if(hours > 0)
				sprintf(upTime, "%d hours %d min %ld sec", hours, minutes, seconds);
		else if(minutes > 0)
				sprintf(upTime, "%d min %ld sec", minutes, seconds);
		else
				sprintf(upTime, "%ld sec", seconds);
	}

	json_object_object_add(jobj_device, "UpTime", json_object_new_string(upTime));

	sysutil_interact(buf, sizeof(buf), "iwlist ath0 chan | grep Current | awk {'print $2'}");
	json_object_object_add(jobj_device, "24g_channel", json_object_new_int(atoi(buf)));

    i = 0;
    jarr = json_object_new_array();

    while(0 != strlen(client_list[i]))
    {
        json_object_array_add(jarr, json_object_new_string(client_list[i]));
        i++;
    }

    json_object_object_add(jobj_device, "DevicesConnectedNumber", json_object_new_int(i));

    json_object_object_add(jobj, "MeshDevice", jobj_device);

    json_object_object_add(jobj, "DeviceList", jarr);

    i = 0;
    jarr = json_object_new_array();

    while(0 != strlen(neighbor.mac[i]))
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "MAC", json_object_new_string(neighbor.mac[i]));
        json_object_object_add(jarr_obj, "RSSI", json_object_new_int(neighbor.rssi[i]));
        json_object_object_add(jarr_obj, "Flag", json_object_new_string(neighbor.flag[i]));

        json_object_array_add(jarr, jarr_obj);
        i++;
    }

    json_object_object_add(jobj, "Neighbors", jarr);

    jobj_guestClient = json_object_new_object();

    i = 0;
    jarr = json_object_new_array();

    while(0 != strlen(gClientInfo.guest24_mac[i]))
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "MAC", json_object_new_string(gClientInfo.guest24_mac[i]));
        json_object_object_add(jarr_obj, "IP", json_object_new_string(gClientInfo.guest24_ip[i]));
        json_object_object_add(jarr_obj, "HostName", json_object_new_string(gClientInfo.guest24_hostname[i]));

        json_object_array_add(jarr, jarr_obj);
        i++;
    }

    json_object_object_add(jobj_guestClient, "twoG", jarr);
    i = 0;
    jarr = json_object_new_array();

    while(0 != strlen(gClientInfo.guest5_mac[i]))
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "MAC", json_object_new_string(gClientInfo.guest5_mac[i]));
        json_object_object_add(jarr_obj, "IP", json_object_new_string(gClientInfo.guest5_ip[i]));
        json_object_object_add(jarr_obj, "HostName", json_object_new_string(gClientInfo.guest5_hostname[i]));

        json_object_array_add(jarr, jarr_obj);
        i++;
    }
    json_object_object_add(jobj_guestClient, "fiveG", jarr);

#if HAS_WLAN_5G_2_SETTING
    i = 0;
    jarr = json_object_new_array();

    while(0 != strlen(gClientInfo.guest5_2_mac[i]))
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "MAC", json_object_new_string(gClientInfo.guest5_2_mac[i]));
        json_object_object_add(jarr_obj, "IP", json_object_new_string(gClientInfo.guest5_2_ip[i]));
        json_object_object_add(jarr_obj, "HostName", json_object_new_string(gClientInfo.guest5_2_hostname[i]));

        json_object_array_add(jarr, jarr_obj);
        i++;
    }
    json_object_object_add(jobj_guestClient, "fiveG2", jarr);
#endif

    json_object_object_add(jobj, "GuestClientList", jobj_guestClient);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    get_json_mesh_basic_wifi_info_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_mesh_basic_wifi_info_cb(HTTPS_CB *pkt, char *result)
{
    int i;
    struct json_object *jobj;
    char api_result[64];
    int light=0;
    int wifi0=0;
    int wifi1=0;
    char buf[8]={0};
    char usb_settings[8]={0};
    int samba=0, dlna=0, printsrv=0;

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    //jobj_device = json_object_new_object();

    if (api_get_integer_option("system.@system[0].basic_led_status", &light))
    {
        light = 0;
    }

    if (api_get_integer_option("wireless.wifi0_ssid_1.disabled", &wifi0))
    {
        wifi0 = 1;
    }

    if (api_get_integer_option("wireless.wifi1_ssid_1.disabled", &wifi1))
    {
        wifi1 = 1;
    }

    if (light == 1)
    {
        json_object_object_add(jobj, "LedStatus", json_object_new_string("1"));
    }
    else
    {
        json_object_object_add(jobj, "LedStatus", json_object_new_string("0"));
    }

    if (wifi0 ==1 && wifi1 == 1)
    {
        json_object_object_add(jobj, "WiFiStatus", json_object_new_string("0"));
    }
    else
    {
        json_object_object_add(jobj, "WiFiStatus", json_object_new_string("1"));
    }

    if (api_get_integer_option("samba.@samba[0].enable", &samba))
    {
        samba = 0;
    }

    if (api_get_integer_option("minidlna.config.enabled", &dlna))
    {
        dlna = 0;
    }

    if (api_get_integer_option("airprint.config.enabled", &printsrv))
    {
        printsrv = 0;
    }

    sprintf(usb_settings, "%d%d%d", samba, dlna, printsrv);

    json_object_object_add(jobj, "USBSettings", json_object_new_string(usb_settings));

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);

    return 0;
}
/*****************************************************************
* NAME:    get_json_mesh_usb_settings_info_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_mesh_usb_settings_info_cb(HTTPS_CB *pkt, char *result)
{
    int i;
    struct json_object *jobj;
    char api_result[64];
    char usb_settings[8]={0};
    char buf[8]={0};
    int samba=0, dlna=0, printsrv=0;
    char hidden[8]={0};

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);

    json_object_object_add(jobj, api_result, json_object_new_string(result));

    //strcpy(usb_settings, "010");
    if (api_get_integer_option("samba.@samba[0].enable", &samba))
    {
        samba = 0;
    }

    if (api_get_integer_option("minidlna.config.enabled", &dlna))
    {
        dlna = 0;
    }

    if (api_get_integer_option("airprint.config.enabled", &printsrv))
    {
        printsrv = 0;
    }

    sprintf(usb_settings, "%d%d%d", samba, dlna, printsrv);
    api_get_string_option("wireless.wifi0_ssid_1.hidden", hidden, sizeof(hidden));

    json_object_object_add(jobj, "USBSettings", json_object_new_string(usb_settings));
    json_object_object_add(jobj, "HiddenSSID", json_object_new_string((strlen(hidden) == 0)?"0":hidden));

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);
}
/*****************************************************************
* NAME:    get_json_mesh_usb_device_info_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_mesh_usb_device_info_cb(HTTPS_CB *pkt, char *result)
{
    int i;
    struct json_object *jobj;
    char api_result[64];
    char buf[64]={0};

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);

    json_object_object_add(jobj, api_result, json_object_new_string(result));

    sysutil_interact(buf, sizeof(buf), "[ -e /tmp/storage_vendor ] && cat /tmp/storage_vendor | tr -d \"\n\"");
	json_object_object_add(jobj, "vendor", (strcmp(buf, "") == 0)?json_object_new_string(""):json_object_new_string(buf));

	sysutil_interact(buf, sizeof(buf), "[ -e /tmp/storage_model ] && cat /tmp/storage_model | tr -d \"\n\"");
	json_object_object_add(jobj, "model", (strcmp(buf, "") == 0)?json_object_new_string(""):json_object_new_string(buf));

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);
}
/*****************************************************************
* NAME:    get_json_mesh_home_info_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_mesh_home_info_cb(HTTPS_CB *pkt, char *result)
{

	if(NULL == pkt)
	{
		return -1;
	}

	int wifi0=0;
	int wifi1=0;
	int wifi0_guest=0;
	int wifi1_guest=0;
	int active=0;
	int active_number=0;
	char buf[8]={0};
	struct json_object *jobj;
	char api_result[64];
	int internet_status=0;

	jobj = json_object_new_object();

	memset(api_result, 0x00, sizeof(api_result));
	sprintf(api_result, "%sResult", pkt->json_action);
	json_object_object_add(jobj, api_result, json_object_new_string(result));

	if (api_get_integer_option("wireless.wifi0_ssid_1.disabled", &wifi0))
	{
		wifi0 = 1;
	}

	if (api_get_integer_option("wireless.wifi1_ssid_1.disabled", &wifi1))
	{
		wifi1 = 1;
	}

	if (api_get_integer_option("wireless.wifi0_guest.disabled", &wifi0_guest))
	{
		wifi0_guest = 1;
	}

	if (api_get_integer_option("wireless.wifi1_guest.disabled", &wifi1_guest))
	{
		wifi1_guest = 1;
	}

	//batctl o -H |grep "no batman" | wc -l
	//batctl o -H |grep "No batman *" | wc -l
#if SUPPORT_BATMAN_2019
	sysutil_interact(buf, sizeof(buf), "batctl o -H | grep \"*\" | grep \"No batman *\" | wc -l");
#else
	sysutil_interact(buf, sizeof(buf), "batctl o -H |grep \"No batman *\" | wc -l");
#endif
	active=atoi(buf);
#if SUPPORT_BATMAN_2019
	sysutil_interact(buf, sizeof(buf), "batctl o -H | grep \"*\" | wc -l");
#else
	sysutil_interact(buf, sizeof(buf), "batctl o -H | wc -l");
#endif
	active_number=atoi(buf);

	json_object_object_add(jobj, "WirelessStatus", json_object_new_string((wifi0 & wifi1) ? "0" : "1"));
	json_object_object_add(jobj, "WirelessGuestNetworkStatus", json_object_new_string((wifi0_guest & wifi1_guest) ? "0" : "1"));

	if (active==0)
	{
		json_object_object_add(jobj, "RoomsActiveNumber", json_object_new_int(active_number+1));
	}
	else
	{
		json_object_object_add(jobj, "RoomsActiveNumber", json_object_new_int(1));
	}
	sysutil_interact(buf, sizeof(buf), "lua /usr/lib/lua/luci/people.lua refresh; cat /tmp/people_active");
	json_object_object_add(jobj, "PeopleActiveNumber", json_object_new_string(buf));

	if(sysutil_check_file_existed("/tmp/internet_status"))
	{
		sysutil_interact(buf, sizeof(buf), "cat /tmp/internet_status");
		json_object_object_add(jobj, "CurrentInternetStatus", json_object_new_int(atoi(buf)));
	}
	else
	{
		json_object_object_add(jobj, "CurrentInternetStatus", json_object_new_int(0));
	}

	basic_json_response(pkt, (char *)json_object_to_json_string(jobj));
	json_object_put(jobj);
}

/*****************************************************************
* NAME:    parse_json_mesh_device_list
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_json_mesh_device_list(char *buf, MESH_DEVICE_SETTINGS_T setting[], char **return_str)
{
    int i, j;
    bool matched;
    char *ptr, *ptr_start;
    char buf_mac[4096];
    char buf_signal[8];
    char mesh_info[256];
    char tmp_mesh_info[256];
    char device_type[16];
    char mesh_mac_table[MAX_MESH_DEVICES][32];
    int mesh_signal_table[MAX_MESH_DEVICES];
    struct json_object *jobj;

    if(0 == strlen(buf))
    {
        return FALSE;
    }

    i = 0;
    memset(buf_mac, 0x00, sizeof(buf_mac));
    memset(mesh_mac_table, 0x00, sizeof(mesh_mac_table));

#if 0
    /* Get all the MESH members from B.A.T.M.A.N. */
    /******************************** MESH connection status *******************************
     * [B.A.T.M.A.N. adv 2013.1.0, MainIF/MAC: ath60/88:dc:96:17:49:66 (bat0)]
     *   Originator      last-seen (#/255)           Nexthop [outgoingIF]:   Potential nexthops ...
     * No batman nodes in range ...
     ***************************************************************************************/
    /******************************** MESH connection status *******************************
     * [B.A.T.M.A.N. adv 2013.1.0, MainIF/MAC: ath60/88:dc:96:17:49:66 (bat0)]
     *   Originator      last-seen (#/255)           Nexthop [outgoingIF]:   Potential nexthops ...
     * 00:02:6f:fb:a0:11    2.676s   (255) 00:02:6f:fb:a0:11 [     ath60]: 88:dc:96:3b:da:af (225) 88:dc:96:3b:da:b1 (225) 00:02:6f:fb:a0:11 (255)
     * 88:dc:96:3b:da:af    0.100s   (255) 88:dc:96:3b:da:af [     ath60]: 00:02:6f:fb:a0:11 (225) 88:dc:96:3b:da:b1 (225) 88:dc:96:3b:da:af (255)
     * 88:dc:96:3b:da:b1    1.584s   (255) 88:dc:96:3b:da:b1 [     ath60]: 00:02:6f:fb:a0:11 (225) 88:dc:96:3b:da:af (225) 88:dc:96:3b:da:b1 (255)
     ***************************************************************************************/
    sys_interact(buf_mac, sizeof(buf_mac), "batctl o");
    if(0 != strcmp("---", buf_mac))
    {
        ptr_start = strchr(buf_mac, '\n') + 1;
        ptr_start = strchr(ptr_start, '\n') + 1;

        ptr_start = strtok(ptr_start, "\n");

        do
        {
            sscanf(ptr_start, "%s %*s (%[^')']) %*s", mesh_mac_table[i], buf_signal);
            mesh_signal_table[i] = (int)((atoi(buf_signal)*100)/255);
            i++;
        } while((ptr_start = strtok(NULL, "\n")));

        /* Add the DUT itself */
        sysinteract(mesh_mac_table[i], sizeof(mesh_mac_table[i]),
                "ifconfig %s | grep \"HWaddr\" | awk \'{print $5}\'",
                apCfgGetIntValue(WLAN_R1_ALL_WDS_NAWDS_ENABLE_TOK)?ATH_WDSBRIDGE_DEV:ATH2_WDSBRIDGE_DEV);

        if(NULL != (ptr = strchr(mesh_mac_table[i], '\n')))
        {
            *ptr = '\0';
            mesh_signal_table[i] = 100;
        }
    }
#endif

    /* Get the information of all MESH members from A.L.F.R.E.D. */
    i = 0;
    ptr_start = buf;
    ptr_start = strtok(ptr_start, "\n");

    do
    {
        matched = FALSE;
        memset(mesh_info, 0x00, sizeof(mesh_info));
        memset(tmp_mesh_info, 0x00, sizeof(tmp_mesh_info));
        memset(device_type, 0x00, sizeof(device_type));

        /* Format : */
        /* { "88:dc:96:17:49:66", "{\"DeviceType\":\"Router\",\"DeviceName\":\"\",\"ModelName\":\"EMR3000\",\"IPAddress\":\"192.168.18.35\",\"UID\":\"0777755\"}" } */
        sscanf(ptr_start, "%*s \"%[^'\"']\", \"%[^\n]\n", setting[i].mac, tmp_mesh_info);
        /* Remove the last character '"' */
        ptr = strrchr(tmp_mesh_info, '\"');
        *ptr = '\0';

        sysutil_remove_escape_from_string(tmp_mesh_info, mesh_info, 1);

#if 0
        /* Check if this device is still alive. */
        for(j = 0; j < MAX_MESH_DEVICES; j++)
        {
            if(0 != strlen(mesh_mac_table[j]))
            {
                if(0 == strcasecmp(mesh_mac_table[j], setting[i].mac))
                {
                    matched = TRUE;
                    setting[i].wifi_strength = mesh_signal_table[j];
                    break;
                }
            }
            else
            {
                break;
            }
        }
#endif

        if(jobj = json_tokener_parse(mesh_info))
        {
            setting[i].existed = TRUE;

            senao_json_object_get_string(jobj, "DeviceType", device_type);
            senao_json_object_get_string(jobj, "DeviceName", setting[i].device_name);
            senao_json_object_get_string(jobj, "ModelName", setting[i].model_name);
            senao_json_object_get_string(jobj, "IPAddress", setting[i].ip_address);
            senao_json_object_get_string(jobj, "LANIPAddress", setting[i].lan_ip_address);
            senao_json_object_get_string(jobj, "UID", setting[i].uid);

            if(0 == strcmp("Router", device_type))
            {
                setting[i].device_type = 1;
            }
            else if(0 == strcmp("AP-Camera", device_type))
            {
                setting[i].device_type = 3;
            }
            else if(0 == strcmp("AP", device_type))
            {
                setting[i].device_type = 2;
            }

            /* Free object */
            json_object_put(jobj);

            i++;
        }
    } while((ptr_start = strtok(NULL, "\n")));

    return TRUE;
}

/*****************************************************************
* NAME:    get_json_mesh_specific_option_setting_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_mesh_specific_option_setting_cb(HTTPS_CB *pkt, char *value, char *result)
{
    int i;
    struct json_object *jobj;
    char api_result[64];

    if(NULL == pkt)
    {
        return -1;
    }

    i = 0;

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));
    json_object_object_add(jobj, "SettingValue", json_object_new_string(value));

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    get_json_mesh_device_list_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_mesh_device_list_cb(HTTPS_CB *pkt, MESH_DEVICE_SETTINGS_T setting[], char *result)
{
    int i;
    struct json_object *jobj;
    struct json_object *jarr;
    struct json_object *jarr_obj;
    struct json_object *jstr;
    char api_result[64];

    if(NULL == pkt)
    {
        return -1;
    }

    i = 0;

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    jarr = json_object_new_array();

    while(TRUE == setting[i].existed)
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "DeviceType", json_object_new_int(setting[i].device_type));
        json_object_object_add(jarr_obj, "DeviceName", json_object_new_string(setting[i].device_name));
        json_object_object_add(jarr_obj, "ModelName", json_object_new_string(setting[i].model_name));
        json_object_object_add(jarr_obj, "IPAddress", json_object_new_string(setting[i].ip_address));
        json_object_object_add(jarr_obj, "LANIPAddress", json_object_new_string(setting[i].lan_ip_address));
        json_object_object_add(jarr_obj, "WifiStrength", json_object_new_int(setting[i].wifi_strength));
        json_object_object_add(jarr_obj, "MacAddress", json_object_new_string(setting[i].mac));
        json_object_object_add(jarr_obj, "UID", json_object_new_string(setting[i].uid));

        json_object_array_add(jarr, jarr_obj);
        i++;
    }

    json_object_object_add(jobj, "MeshDevices", jarr);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    parse_json_mesh_device_mac_info_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int parse_json_mesh_device_mac_info_cb(char *buf, MESH_DEVICE_MAC_INFO_T setting[])
{
    int i;
    char *ptr, *ptr_start;
    char mac_info[256];
    char tmp_mac_info[256];
    struct json_object *jobj;

    if(0 == strlen(buf))
    {
        return FALSE;
    }

    /* Get the information of all MESH members from A.L.F.R.E.D. */
    i = 0;
    ptr_start = buf;
    ptr_start = strtok(ptr_start, "\n");

    do
    {
        memset(mac_info, 0x00, sizeof(mac_info));
        memset(tmp_mac_info, 0x00, sizeof(tmp_mac_info));

        /* Format : */
        /* { "88:dc:96:17:49:66", "{\"LanMac\":\"88:DC:96:17:49:65\",\"BatMac\":\"36:02:CC:73:94:8E\"}" } */
        sscanf(ptr_start, "%*s \"%[^'\"']\", \"%[^\n]\n", setting[i].primary_mac, tmp_mac_info);

        /* Remove the last character '"' */
        ptr = strrchr(tmp_mac_info, '"');
        *ptr = '\0';

        sysutil_remove_escape_from_string(tmp_mac_info, mac_info, 1);

        if(jobj = json_tokener_parse(mac_info))
        {
            senao_json_object_get_string(jobj, "LanMac", setting[i].lan_mac);
            senao_json_object_get_string(jobj, "BatMac", setting[i].bat_mac);

            /* Free object */
            json_object_put(jobj);

            i++;
        }
    } while((ptr_start = strtok(NULL, "\n")));

    return TRUE;
}

/*****************************************************************
* NAME:    parse_json_mesh_device_client_mac_list_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int parse_json_mesh_device_client_mac_list_cb(char *buf, MESH_DEVICE_MAC_INFO_T mac_info[], MESH_DEVICE_CLIENT_MAC_LIST_T setting[])
{
    struct json_object *jobj, *jobj_vis;
    struct json_object *jarr_vis, *jarr_clients;
    struct array_list *array_vis, *array_clients;
    struct json_object *jstr_primary, *jstr_client;
    int i, j, k, l, num_vis, num_client;
    char mac[32];

    if(0 == strlen(buf))
    {
        return FALSE;
    }

    /* Get the all network nodes in this MESH environment from batadv-vis. */
    /* The format of batadv-vis -f jsondoc
    {
      "source_version" : "2015.1",
      "algorithm" : 4,
      "vis" : [
        { "primary" : "88:dc:96:17:49:66",
          "neighbors" : [
             { "router" : "88:dc:96:17:49:66",
               "neighbor" : "88:dc:96:3b:da:b1",
               "metric" : "1.527" },
             { "router" : "88:dc:96:17:49:66",
               "neighbor" : "88:dc:96:18:52:01",
               "metric" : "1.128" }
          ],
          "clients" : [
            "88:dc:96:17:49:65",
            "36:02:cc:73:94:8e"
          ]
        }
      ]
    }
    */
    if(jobj = json_tokener_parse(buf))
    {
        jarr_vis = json_object_object_get(jobj, "vis");

        array_vis = json_object_get_array(jarr_vis);

        num_vis = array_vis->length;

        for(i = 0; i < num_vis; i++)
        {
            jobj_vis = json_object_array_get_idx(jarr_vis, i);

            jstr_primary = json_object_object_get(jobj_vis, "primary");
            sprintf(setting[i].primary_mac, "%s", json_object_get_string(jstr_primary));

            for(j = 0; j < MAX_MESH_DEVICES; j++)
            {
                if(0 == strcasecmp(setting[i].primary_mac, mac_info[j].primary_mac))
                {
                    break;
                }
            }

            jarr_clients = json_object_object_get(jobj_vis, "clients");

            array_clients = json_object_get_array(jarr_clients);

            num_client = array_clients->length;

            l = 0;

            for(k = 0; k < num_client; k++)
            {
                jstr_client = json_object_array_get_idx(jarr_clients, k);
                sprintf(mac, "%s", json_object_get_string(jstr_client));
                if((0 != strcasecmp(mac, mac_info[j].lan_mac)) &&
                   (0 != strcasecmp(mac, mac_info[j].bat_mac)))
                {
                    sprintf(setting[i].client_mac_list[l], "%s", mac);
                    l++;
                }
            }
        }

        /* Free object */
        json_object_put(jobj);
    }

    return TRUE;
}

/*****************************************************************
* NAME:    get_json_mesh_device_client_list_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_mesh_device_client_list_cb(HTTPS_CB *pkt, MESH_DEVICE_CLIENT_MAC_LIST_T setting[], char *result)
{
    struct json_object *jobj;
    struct json_object *jarr, *jarr_obj;
    struct json_object *jarr_client_list;
    struct json_object *jstr;
    char api_result[64];
    int i, j;

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    jarr = json_object_new_array();

    while(0 != strlen(setting[i].primary_mac))
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "PrimaryMAC", json_object_new_string(setting[i].primary_mac));

        jarr_client_list = json_object_new_array();

        for(j = 0; j <= 128; j++)
        {
            if(0 != strlen(setting[i].client_mac_list[j]))
            {
                json_object_array_add(jarr_client_list, json_object_new_string(setting[i].client_mac_list[j]));
            }
            else
            {
                break;
            }
        }

        json_object_object_add(jarr_obj, "ClientList", jarr_client_list);

        json_object_array_add(jarr, jarr_obj);
        i++;
    }

    json_object_object_add(jobj, "MeshDeviceClientList", jarr);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);

    return 0;
}
/*****************************************************************
* NAME:    get_json_mesh_device_wireless_settings_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_mesh_device_wireless_settings_cb(HTTPS_CB *pkt, char *op_mesh_band,
        WLAN_RADIO_SETTINGS_T setting[], WLAN_RADIO_SECURITY_T security[], char *result)
{
    struct json_object *jobj;
    struct json_object *jarr;
    struct json_object *jarr_obj;
    struct json_object *jstr;
    char api_result[64];

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    json_object_object_add(jobj, "OperatingMeshBand", json_object_new_string(op_mesh_band));

    jarr = json_object_new_array();

    jarr_obj = json_object_new_object();

    json_object_object_add(jarr_obj, "Band", json_object_new_string(WLAN_RADIO_2_4_GHZ_STR));
    json_object_object_add(jarr_obj, "Enabled", json_object_new_boolean(setting[0].enabled));
    json_object_object_add(jarr_obj, "SSID", json_object_new_string(setting[0].ssid));
    json_object_object_add(jarr_obj, "SSIDBroadcast", json_object_new_boolean(setting[0].ssid_broadcast));
    json_object_object_add(jarr_obj, "MacAddress", json_object_new_string((setting[0].mac)?setting[0].mac:""));
    json_object_object_add(jarr_obj, "ChannelWidth", json_object_new_int(setting[0].channel_width));
    json_object_object_add(jarr_obj, "Channel", json_object_new_int(setting[0].channel));
    json_object_object_add(jarr_obj, "SecurityEnabled", json_object_new_boolean(security[0].enabled));
    json_object_object_add(jarr_obj, "SecurityType", json_object_new_string(security[0].type));
    json_object_object_add(jarr_obj, "Encryption", json_object_new_string(security[0].encryption));
    json_object_object_add(jarr_obj, "Key", json_object_new_string(security[0].key));
    json_object_object_add(jarr_obj, "RadiusIP", json_object_new_string(security[0].radius_ip1));
    json_object_object_add(jarr_obj, "RadiusPort", json_object_new_int(security[0].radius_port1));
    json_object_object_add(jarr_obj, "RadiusSecret", json_object_new_string(security[0].radius_secret1));

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
    json_object_object_add(jarr_obj, "SecurityType", json_object_new_string(security[1].type));
    json_object_object_add(jarr_obj, "Encryption", json_object_new_string(security[1].encryption));
    json_object_object_add(jarr_obj, "Key", json_object_new_string(security[1].key));
    json_object_object_add(jarr_obj, "RadiusIP", json_object_new_string(security[1].radius_ip1));
    json_object_object_add(jarr_obj, "RadiusPort", json_object_new_int(security[1].radius_port1));
    json_object_object_add(jarr_obj, "RadiusSecret", json_object_new_string(security[1].radius_secret1));

    json_object_array_add(jarr, jarr_obj);
#endif

    json_object_object_add(jobj, "WirelessSettings", jarr);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    get_json_mesh_device_status_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_device_mesh_wireless_settings_json_cb(struct json_object *jobj, MESH_WIRELESS_SETTINGS_T *setting)
{
    if(NULL == jobj)
    {
        return FALSE;
    }

    /* Construct the packet content in json format. */
    json_object_object_add(jobj, "MESHEnabled", json_object_new_boolean(setting->enabled));
    json_object_object_add(jobj, "MESHConfigured", json_object_new_boolean(setting->configured));
    json_object_object_add(jobj, "MESHDeviceName", json_object_new_string(setting->device_name));
    json_object_object_add(jobj, "MESHOperationBand", json_object_new_string(setting->operation_band));
    json_object_object_add(jobj, "MESHSSID", json_object_new_string(setting->ssid));
    json_object_object_add(jobj, "MESHChannel", json_object_new_int(setting->channel));
    json_object_object_add(jobj, "MESHMac", json_object_new_string(setting->mac));
    json_object_object_add(jobj, "MESHSecurityType", json_object_new_string(setting->type));
    json_object_object_add(jobj, "MESHKey", json_object_new_string(setting->key));

    return TRUE;
}

/*****************************************************************
* NAME:    get_json_mesh_device_status_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_mesh_device_status_cb(HTTPS_CB *pkt, DEVICE_ALL_SETTING_T *setting, char *result)
{
    struct json_object *jobj;
    struct json_object *jobj_minor;
    struct json_object *jarr;
    char api_result[64];

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    json_object_object_add(jobj, "OperationMode", json_object_new_int(setting->operation_mode));

    jobj_minor = json_object_new_object();
    get_device_device_settings_json_cb(jobj_minor, &setting->device_setting);
    json_object_object_add(jobj, "DeviceSettings", jobj_minor);

    jobj_minor = json_object_new_object();
    get_device_lan_settings_json_cb(jobj_minor, &setting->lan_setting);
    json_object_object_add(jobj, "LanSettings", jobj_minor);

    jarr = json_object_new_array();
    get_device_wireless_settings_json_cb(jarr, setting->wlan_setting, setting->wlan_security);
    json_object_object_add(jobj, "WirelessSettings", jarr);

    jobj_minor = json_object_new_object();
    get_device_mesh_wireless_settings_json_cb(jobj_minor, &setting->mesh_wlan_setting);
    json_object_object_add(jobj, "MeshWirelessSettings", jobj_minor);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    parse_json_mesh_device_neighbors
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_json_mesh_device_neighbors(char *buf, MESH_DEVICE_NEIGHBORS_T setting[], char **return_str)
{
    int i, j;
    bool matched, result;
    char *ptr, *ptr_start;
    char ifname[8];
    char buf_mac[4096];
    char mesh_info[256];
    char tmp_mesh_info[256];
    char mesh_mac_table[MAX_MESH_DEVICES][32];
    int count_node;
    int mesh24g_disabled;
    struct json_object *jobj;
    struct json_object *jobj_mac;
    struct json_object *jobj_tq;
    struct json_object *jarr;
    struct json_object *jarr_obj;
    struct array_list *array_node;

    if(0 == strlen(buf))
    {
        return FALSE;
    }

    i = 0;
    result = FALSE;
    count_node = 0;

    memset(buf_mac, 0x00, sizeof(buf_mac));
    memset(mesh_mac_table, 0x00, sizeof(mesh_mac_table));

    /* Get all the MESH members from B.A.T.M.A.N. */
    /******************************** MESH connection status *******************************
     * [B.A.T.M.A.N. adv 2013.1.0, MainIF/MAC: ath60/88:dc:96:17:49:66 (bat0)]
     *   Originator      last-seen (#/255)           Nexthop [outgoingIF]:   Potential nexthops ...
     * No batman nodes in range ...
     ***************************************************************************************/
    /******************************** MESH connection status *******************************
     * [B.A.T.M.A.N. adv 2013.1.0, MainIF/MAC: ath60/88:dc:96:17:49:66 (bat0)]
     *   Originator      last-seen (#/255)           Nexthop [outgoingIF]:   Potential nexthops ...
     * 00:02:6f:fb:a0:11    2.676s   (255) 00:02:6f:fb:a0:11 [     ath60]: 88:dc:96:3b:da:af (225) 88:dc:96:3b:da:b1 (225) 00:02:6f:fb:a0:11 (255)
     ***************************************************************************************/
    sysutil_interact(buf_mac, sizeof(buf_mac), "batctl o");

    if(0 != strcmp("---", buf_mac))
    {
        ptr_start = strchr(buf_mac, '\n') + 1;
        ptr_start = strchr(ptr_start, '\n') + 1;

        ptr_start = strtok(ptr_start, "\n");

        do
        {
            sscanf(ptr_start, "%s %*[^'\n']\n", mesh_mac_table[i]);
            i++;
        } while((ptr_start = strtok(NULL, "\n")));

        /* Add the DUT itself */
        api_get_wifi_mesh_disabled_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, &mesh24g_disabled);
        api_get_wifi_mesh_ifname_option((mesh24g_disabled)?1:0, ifname, sizeof(ifname));

        sysutil_interact(mesh_mac_table[i], sizeof(mesh_mac_table[i]),
                "ifconfig %s | awk \'/HWaddr/{print $5}\'", ifname);

        if(NULL != (ptr = strchr(mesh_mac_table[i], '\n')))
        {
            *ptr = '\0';
        }
    }

    /* Get the information from A.L.F.R.E.D. */
    i = 0;
    ptr_start = buf;
    ptr_start = strtok(ptr_start, "\n");

    do
    {
        matched = FALSE;
        memset(mesh_info, 0x00, sizeof(mesh_info));
        memset(tmp_mesh_info, 0x00, sizeof(tmp_mesh_info));

        /* Format : */
        /* { "8a:dc:96:17:49:66", "{\"Node\":[[\"02:aa:bb:cc:dd:13\",\"94\"]]}" }, */
        sscanf(ptr_start, "%*s \"%[^'\"']\", \"%[^\n]\n", setting[i].mesh_device_mac, tmp_mesh_info);
        /* Remove the last character '"' */
        ptr = strrchr(tmp_mesh_info, '\"');
        *ptr = '\0';

        sysutil_remove_escape_from_string(tmp_mesh_info, mesh_info, 1);

        /* Check if this device is still alive. */
        for(j = 0; j < MAX_MESH_DEVICES; j++)
        {
            if(0 != strlen(mesh_mac_table[j]))
            {
                if(0 == strcasecmp(mesh_mac_table[j], setting[i].mesh_device_mac))
                {
                    matched = TRUE;
                    break;
                }
            }
            else
            {
                break;
            }
        }

#if 0
        if((TRUE == matched) && (jobj = json_tokener_parse(mesh_info)))
        {
            result = TRUE;

            jarr = json_object_object_get(jobj, "Node");
            array_node = json_object_get_array(jarr);

            count_node = array_node->length;

            for(j = 0; j < count_node; j++)
            {
                jarr_obj = json_object_array_get_idx(jarr, j);

                char tmp_buf[64];
                char tq[8];

                memset(tmp_buf, 0x00, sizeof(tmp_buf));
                memset(tq, 0x00, sizeof(tq));
                sprintf(tmp_buf, "%s", json_object_get_string(jarr_obj));

                sscanf(tmp_buf, "[ \"%[^\"]\", \"%[^\"]\"", setting[i].mac[j], tq);
                setting[i].tq[j] = atoi(tq);
            }

            i++;
        }
#else
        if((TRUE == matched) && (jobj = json_tokener_parse(mesh_info)))
        {
            result = TRUE;

            jarr = json_object_object_get(jobj, "Node");
            array_node = json_object_get_array(jarr);

            count_node = array_node->length;

            for(j = 0; j < count_node; j++)
            {
                jarr_obj = json_object_array_get_idx(jarr, j);

                if(json_type_array == json_object_get_type(jarr_obj))
                {
                    jobj_mac = json_object_array_get_idx(jarr_obj, MESH_LINK_INFORMATION_MAC);
                    sprintf(setting[i].mac[j], "%s", json_object_get_string(jobj_mac));

                    jobj_tq = json_object_array_get_idx(jarr_obj, MESH_LINK_INFORMATION_TQ);
                    setting[i].tq[j] = json_object_get_int(jobj_tq);
                }
                else
                {
                    return FALSE;
                }
            }

            /* Free object */
            json_object_put(jobj);

            i++;
        }
#endif
    } while((ptr_start = strtok(NULL, "\n")));

    return result;
}

/*****************************************************************
* NAME:    get_json_mesh_device_neighbors_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_json_mesh_device_neighbors_cb(HTTPS_CB *pkt, MESH_DEVICE_NEIGHBORS_T setting[], char *result)
{
    int i, j;
    struct json_object *jobj;
    struct json_object *jarr, *jarr_obj;
    struct json_object *jarr_link, *jarr_link_obj;
    struct json_object *jstr;
    char api_result[64];

    if(NULL == pkt)
    {
        return -1;
    }

    i = 0;

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    jarr = json_object_new_array();

    while(strlen(setting[i].mesh_device_mac))
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "MeshDeviceMAC", json_object_new_string(setting[i].mesh_device_mac));

        jarr_link = json_object_new_array();

        j = 0;

        while(strlen(setting[i].mac[j]))
        {
            jarr_link_obj = json_object_new_object();

            json_object_object_add(jarr_link_obj, "MAC", json_object_new_string(setting[i].mac[j]));
            json_object_object_add(jarr_link_obj, "TQ", json_object_new_int(setting[i].tq[j]));

            json_object_array_add(jarr_link, jarr_link_obj);
            j++;
        }

        json_object_object_add(jarr_obj, "MeshLinkInformation", jarr_link);

        json_object_array_add(jarr, jarr_obj);
        i++;
    }

    json_object_object_add(jobj, "MeshDeviceNeighbors", jarr);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    /* Free object */
    json_object_put(jobj);

    return 0;
}

#if SUPPORT_PEOPLE_FUNCTION
/*****************************************************************
* NAME:    get_mesh_simple_people_info_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_simple_people_info_json_cb(HTTPS_CB *pkt, MESH_USER_PROFILE_T *users, MESH_CLIENT_PROFILE_T *clients, MESH_FIREWALL_RULE_T *rules, char *result)
{
    int i, j;
    struct json_object *jobj;
    struct json_object *jarr, *jarr_obj;
    struct json_object *jarr_item;
    char api_result[64];

    if(NULL == pkt)
    {
        return -1;
    }

    i = 0;

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    jarr = json_object_new_array();

    i = 0;

    while(0 != strlen(users[i].name) && i < MAX_CLIENT_PERSON)
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "Index", json_object_new_int(users[i].index));
        json_object_object_add(jarr_obj, "Name", json_object_new_string(users[i].name));
        json_object_object_add(jarr_obj, "Block", json_object_new_boolean(users[i].block));
        json_object_object_add(jarr_obj, "BlockActivated", json_object_new_boolean(users[i].block_activated));
        json_object_object_add(jarr_obj, "ScheduleBlockActivated", json_object_new_boolean(users[i].schedule_block_activated));

        i++;

        json_object_array_add(jarr, jarr_obj);
    }

    json_object_object_add(jobj, "Users", jarr);

    jarr = json_object_new_array();

    i = 0;

    while(0 != strlen(clients[i].mac) && i < MAX_CLIENT_DEVICE)
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "Index", json_object_new_int(clients[i].index));
        json_object_object_add(jarr_obj, "Name", json_object_new_string(clients[i].name));
        json_object_object_add(jarr_obj, "IP", json_object_new_string(clients[i].ip));
        json_object_object_add(jarr_obj, "Mac", json_object_new_string(clients[i].mac));
        json_object_object_add(jarr_obj, "Block", json_object_new_boolean(clients[i].block));
        json_object_object_add(jarr_obj, "IsGuest", json_object_new_boolean(clients[i].is_guest));
        json_object_object_add(jarr_obj, "ScheduleBlock", json_object_new_boolean(clients[i].schedule_block));
        json_object_object_add(jarr_obj, "BlockActivated", json_object_new_boolean(clients[i].block_activated));
        json_object_object_add(jarr_obj, "ScheduleBlockActivated", json_object_new_boolean(clients[i].schedule_block_activated));
        json_object_object_add(jarr_obj, "Status", json_object_new_boolean(clients[i].status));
        json_object_object_add(jarr_obj, "OwnerIndex", json_object_new_int(clients[i].owner_index));
        json_object_object_add(jarr_obj, "Location", json_object_new_string(clients[i].connected_host_name));
        json_object_object_add(jarr_obj, "LocationMAC", json_object_new_string(clients[i].connected_host_mac));

        i++;

        json_object_array_add(jarr, jarr_obj);
    }

    json_object_object_add(jobj, "Clients", jarr);

    jarr = json_object_new_array();

    i = 0;

    while(0 != strlen(rules[i].name) && i < MAX_FIREWALL_EBTABLES_RULE)
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "Index", json_object_new_int(rules[i].index));
        json_object_object_add(jarr_obj, "Name", json_object_new_string(rules[i].name));

        i++;

        json_object_array_add(jarr, jarr_obj);
    }

    json_object_object_add(jobj, "Rules", jarr);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    /* Free object */
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    get_mesh_user_profile_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_user_profile_json_cb(HTTPS_CB *pkt, MESH_USER_PROFILE_T setting, char *result)
{
    int i;
    struct json_object *jobj;
    struct json_object *jarr;
    char api_result[64];

    if(NULL == pkt)
    {
        return -1;
    }

    i = 0;

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    json_object_object_add(jobj, "Index", json_object_new_int(setting.index));
    json_object_object_add(jobj, "Name", json_object_new_string(setting.name));
    json_object_object_add(jobj, "Block", json_object_new_boolean(setting.block));
    json_object_object_add(jobj, "BlockActivated", json_object_new_boolean(setting.block_activated));
    json_object_object_add(jobj, "ScheduleBlockActivated", json_object_new_boolean(setting.schedule_block_activated));

    jarr = json_object_new_array();

    i = 0;

    while(0 != setting.device_index[i])
    {
        json_object_array_add(jarr, json_object_new_int(setting.device_index[i]));
        i++;
    }

    json_object_object_add(jobj, "Devices", jarr);

    jarr = json_object_new_array();

    i = 0;

    while(0 != strlen(setting.device_mac[i]))
    {
        json_object_array_add(jarr, json_object_new_string(setting.device_mac[i]));
        i++;
    }

    json_object_object_add(jobj, "DeviceMACs", jarr);

    jarr = json_object_new_array();

    i = 0;

    while(0 != setting.rule_index[i] && i < MAX_FIREWALL_EBTABLES_RULE)
    {
        json_object_array_add(jarr, json_object_new_int(setting.rule_index[i]));
        i++;
    }

    json_object_object_add(jobj, "Rules", jarr);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    /* Free object */
    json_object_put(jobj);

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_user_profile_list_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_user_profile_list_json_cb(HTTPS_CB *pkt, MESH_USER_PROFILE_T *setting, char *result)
{
    int i, j;
    struct json_object *jobj;
    struct json_object *jarr, *jarr_obj;
    struct json_object *jarr_item;
    char api_result[64];

    if(NULL == pkt)
    {
        return -1;
    }

    i = 0;

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    jarr = json_object_new_array();

    while(0 != strlen(setting[i].name))
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "Index", json_object_new_int(setting[i].index));
        json_object_object_add(jarr_obj, "Name", json_object_new_string(setting[i].name));
        json_object_object_add(jarr_obj, "Block", json_object_new_boolean(setting[i].block));
        json_object_object_add(jarr_obj, "BlockActivated", json_object_new_boolean(setting[i].block_activated));
        json_object_object_add(jarr_obj, "ScheduleBlockActivated", json_object_new_boolean(setting[i].schedule_block_activated));


        jarr_item = json_object_new_array();

        j = 0;

        while(0 != setting[i].device_index[j])
        {
            json_object_array_add(jarr_item, json_object_new_int(setting[i].device_index[j]));
            j++;
        }

        json_object_object_add(jarr_obj, "Devices", jarr_item);

        jarr_item = json_object_new_array();

        j = 0;

        while(0 != setting[i].rule_index[j] && j < MAX_FIREWALL_EBTABLES_RULE)
        {
            json_object_array_add(jarr_item, json_object_new_int(setting[i].rule_index[j]));
            j++;
        }

        json_object_object_add(jarr_obj, "Rules", jarr_item);

        i++;

        json_object_array_add(jarr, jarr_obj);
    }

    json_object_object_add(jobj, "Users", jarr);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    /* Free object */
    json_object_put(jobj);

	return 0;
}

/*****************************************************************
* NAME:    parse_mesh_user_profile_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int parse_mesh_user_profile_json_cb(char *query_str, MESH_USER_PROFILE_T *setting, char **return_str)
{
    int i = 0, count = 0;
    bool result = FALSE;
    struct json_object *jobj;
    struct json_object *jarr;
    struct json_object *jstr;
    struct json_object *jint;
    struct array_list *array_setting;

    if((NULL == query_str) || (0 == strlen(query_str)))
    {
        return result;
    }

    if((jobj = json_tokener_parse(query_str)))
    {
        senao_json_object_get_integer(jobj, "Index", &setting->index);
        senao_json_object_get_string(jobj, "Name", setting->name);

        jarr = json_object_object_get(jobj, "Devices");
        array_setting = json_object_get_array(jarr);

        count = array_setting->length;

        for(i = 0; i < count; i++)
        {
            jstr = json_object_array_get_idx(jarr, i);
            sprintf(setting->device_mac[i], "%s\0", json_object_get_string(jstr));
        }

        jarr = json_object_object_get(jobj, "Rules");
        array_setting = json_object_get_array(jarr);

        count = array_setting->length;

        for(i = 0; i < count; i++)
        {
            jint = json_object_array_get_idx(jarr, i);
            setting->rule_index[i] = json_object_get_int(jint);
        }

        /* Free object */
        json_object_put(jobj);

        *return_str = OK_STR;
        result = TRUE;
    }

    return result;
}

/*****************************************************************
* NAME:    parse_mesh_user_profile_list_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int parse_mesh_user_profile_list_json_cb(char *query_str, MESH_USER_PROFILE_T *setting, char **return_str)
{
    int i = 0, j = 0, user_count = 0, count = 0;
    bool result = FALSE;
    struct json_object *jobj;
    struct json_object *jobj_user;
    struct json_object *jarr_user;
    struct json_object *jarr;
    struct json_object *jstr;
    struct json_object *jint;
    struct array_list *array_setting;

    if((NULL == query_str) || (0 == strlen(query_str)))
    {
        return result;
    }

    if((jobj = json_tokener_parse(query_str)))
    {
        jarr_user = json_object_object_get(jobj, "Users");
        array_setting = json_object_get_array(jarr_user);

        user_count = array_setting->length;

        for(i = 0; i < user_count; i++)
        {
            jobj_user = json_object_array_get_idx(jarr_user, i);

            senao_json_object_get_integer(jobj_user, "Index", &setting[i].index);
            senao_json_object_get_string(jobj_user, "Name", setting[i].name);

            jarr = json_object_object_get(jobj_user, "Devices");
            array_setting = json_object_get_array(jarr);

            count = array_setting->length;

            for(j = 0; j < count; j++)
            {
                jstr = json_object_array_get_idx(jarr, j);
                sprintf(setting[i].device_mac[j], "%s\0", json_object_get_string(jstr));
            }

#if 0
            jarr = json_object_object_get(jobj_user, "Rules");
            array_setting = json_object_get_array(jarr);

            count = array_setting->length;

            for(j = 0; j < count; j++)
            {
                jint = json_object_array_get_idx(jarr, j);
                setting[i].rule_index[j] = json_object_get_int(jint);
            }
#endif
        }

        /* Free object */
        json_object_put(jobj);

        *return_str = OK_STR;
        result = TRUE;
    }

    return result;
}

/*****************************************************************
* NAME:    get_mesh_client_profile_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_client_profile_json_cb(HTTPS_CB *pkt, MESH_CLIENT_PROFILE_T setting, char *result)
{
    int i;
    struct json_object *jobj;
    struct json_object *jarr;
    char api_result[64];

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    json_object_object_add(jobj, "Index", json_object_new_int(setting.index));
    json_object_object_add(jobj, "Name", json_object_new_string(setting.name));
    json_object_object_add(jobj, "Status", json_object_new_boolean(setting.status));
    json_object_object_add(jobj, "HighQuality", json_object_new_boolean(setting.high_quality));
    json_object_object_add(jobj, "Block", json_object_new_boolean(setting.block));
    json_object_object_add(jobj, "StartDate", json_object_new_string(setting.start_date));
    json_object_object_add(jobj, "BlockTime", json_object_new_string(setting.block_time));
    json_object_object_add(jobj, "IP", json_object_new_string(setting.ip));
    json_object_object_add(jobj, "MAC", json_object_new_string(setting.mac));

    if(0 != strlen(setting.owner))
    {
        json_object_object_add(jobj, "Owner", json_object_new_string(setting.owner));
        json_object_object_add(jobj, "OwnerIndex", json_object_new_int(setting.owner_index));
        json_object_object_add(jobj, "OwnerBlock", json_object_new_boolean(setting.owner_block));

        jarr = json_object_new_array();

        i = 0;

        while(0 != setting.owner_rule_index[i])
        {
            json_object_array_add(jarr, json_object_new_int(setting.owner_rule_index[i]));
            i++;
        }

        json_object_object_add(jobj, "OwnerRules", jarr);
    }

    json_object_object_add(jobj, "ScheduleBlock", json_object_new_boolean(setting.schedule_block));
    json_object_object_add(jobj, "FromHour", json_object_new_int(setting.from_hour));
    json_object_object_add(jobj, "FromMinute", json_object_new_int(setting.from_minute));
    json_object_object_add(jobj, "ToHour", json_object_new_int(setting.to_hour));
    json_object_object_add(jobj, "ToMinute", json_object_new_int(setting.to_minute));
    json_object_object_add(jobj, "WeekDay", json_object_new_string(setting.week_day));

    json_object_object_add(jobj, "BlockActivated", json_object_new_boolean(setting.block_activated));
    json_object_object_add(jobj, "ScheduleBlockActivated", json_object_new_boolean(setting.schedule_block_activated));

    json_object_object_add(jobj, "ConnectedHostName", json_object_new_string(setting.connected_host_name));

    jarr = json_object_new_array();

    i = 0;

    while(0 != setting.rule_index[i] && i < MAX_FIREWALL_EBTABLES_RULE)
    {
        json_object_array_add(jarr, json_object_new_int(setting.rule_index[i]));
        i++;
    }

    json_object_object_add(jobj, "Rules", jarr);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    /* Free object */
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    get_mesh_client_profile_list_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_client_profile_list_json_cb(HTTPS_CB *pkt, MESH_CLIENT_PROFILE_T *setting, char *result)
{
    int i, j;
    struct json_object *jobj;
    struct json_object *jarr, *jarr_obj;
    struct json_object *jarr_item;
    char api_result[64];

    if(NULL == pkt)
    {
        return -1;
    }

    i = 0;

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    jarr = json_object_new_array();

    while(0 != strlen(setting[i].mac))
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "Index", json_object_new_int(setting[i].index));
        json_object_object_add(jarr_obj, "Name", json_object_new_string(setting[i].name));
        json_object_object_add(jarr_obj, "Status", json_object_new_boolean(setting[i].status));
        json_object_object_add(jarr_obj, "HighQuality", json_object_new_boolean(setting[i].high_quality));
        json_object_object_add(jarr_obj, "Block", json_object_new_boolean(setting[i].block));
        json_object_object_add(jarr_obj, "BlockActivated", json_object_new_boolean(setting[i].block_activated));
        json_object_object_add(jarr_obj, "ScheduleBlock", json_object_new_boolean(setting[i].schedule_block));
        json_object_object_add(jarr_obj, "ScheduleBlockActivated", json_object_new_boolean(setting[i].schedule_block_activated));
        json_object_object_add(jarr_obj, "IP", json_object_new_string(setting[i].ip));
        json_object_object_add(jarr_obj, "MAC", json_object_new_string(setting[i].mac));

        if(0 != strlen(setting[i].owner))
        {
            json_object_object_add(jarr_obj, "Owner", json_object_new_string(setting[i].owner));
            json_object_object_add(jarr_obj, "OwnerIndex", json_object_new_int(setting[i].owner_index));

            jarr_item = json_object_new_array();

            j = 0;

            while(0 != setting[i].owner_rule_index[j])
            {
                json_object_array_add(jarr_item, json_object_new_int(setting[i].owner_rule_index[j]));
                j++;
            }

            json_object_object_add(jarr_obj, "OwnerRules", jarr_item);
        }

        jarr_item = json_object_new_array();

        j = 0;

        while(0 != setting[i].rule_index[j] && j < MAX_FIREWALL_EBTABLES_RULE)
        {
            json_object_array_add(jarr_item, json_object_new_int(setting[i].rule_index[j]));
            j++;
        }

        json_object_object_add(jarr_obj, "Rules", jarr_item);

        i++;

        json_object_array_add(jarr, jarr_obj);
    }

    json_object_object_add(jobj, "Clients", jarr);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    /* Free object */
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    parse_mesh_client_profile_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int parse_mesh_client_profile_json_cb(char *query_str, MESH_CLIENT_PROFILE_T *setting, char **return_str)
{
    int i = 0;
    bool result = FALSE;
    struct json_object *jobj;
    struct json_object *jint;
    struct json_object *jarr;
    struct array_list *array_setting;

    if((NULL == query_str) || (0 == strlen(query_str)))
    {
        return result;
    }

    if((jobj = json_tokener_parse(query_str)))
    {
        senao_json_object_get_integer(jobj, "Index", &setting->index);
        senao_json_object_get_string(jobj, "Name", setting->name);
        senao_json_object_get_string(jobj, "MAC", setting->mac);
        senao_json_object_get_integer(jobj, "OwnerIndex", &setting->owner_index);
        senao_json_object_get_boolean(jobj, "HighQuality", &setting->high_quality);

        jarr = json_object_object_get(jobj, "Rules");
        array_setting = json_object_get_array(jarr);

        for(i = 0; i < array_setting->length; i++)
        {
            jint = json_object_array_get_idx(jarr, i);
            setting->rule_index[i] = json_object_get_int(jint);
        }

        /* Free object */
        json_object_put(jobj);

        *return_str = OK_STR;
        result = TRUE;
    }

    return result;
}

/*****************************************************************
* NAME:    delete_mesh_client_profile_list_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int delete_mesh_client_profile_list_json_cb(char *query_str, int *index)
{
    int i = 0, count = 0;
    bool result = FALSE;
    struct json_object *jobj;
    struct json_object *jint;
    struct json_object *jarr;
    struct array_list *array_setting;

    if((NULL == query_str) || (0 == strlen(query_str)))
    {
        return result;
    }

    if((jobj = json_tokener_parse(query_str)))
    {
        jarr = json_object_object_get(jobj, "Indexes");
        array_setting = json_object_get_array(jarr);

        count = array_setting->length;

        for(i = 0; i < count; i++)
        {
            jint = json_object_array_get_idx(jarr, i);
            index[i] = json_object_get_int(jint);
        }

        /* Free object */
        json_object_put(jobj);

        result = TRUE;
    }

    return result;
}

#if SUPPORT_FIREWALL_URL_CATEGORY
/*****************************************************************
* NAME:    get_mesh_firewall_default_url_category_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_firewall_default_url_category_json_cb(HTTPS_CB *pkt)
{
    struct json_object *jobj;
    struct json_object *jarr;
    char api_result[64];

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(OK_STR));

    jarr = json_object_new_array();

    json_object_array_add(jarr, json_object_new_string("facebook"));
    json_object_array_add(jarr, json_object_new_string("flickr"));
    json_object_array_add(jarr, json_object_new_string("instagram"));
    json_object_array_add(jarr, json_object_new_string("twitter"));

    json_object_object_add(jobj, "SocialMediaList", jarr);

    jarr = json_object_new_array();

    json_object_array_add(jarr, json_object_new_string("bing"));
    json_object_array_add(jarr, json_object_new_string("google"));
    json_object_array_add(jarr, json_object_new_string("yahoo"));
    json_object_array_add(jarr, json_object_new_string("microsoft"));

    json_object_object_add(jobj, "SearchEngineList", jarr);

    jarr = json_object_new_array();

    json_object_array_add(jarr, json_object_new_string("hbo"));
    json_object_array_add(jarr, json_object_new_string("mlb"));
    json_object_array_add(jarr, json_object_new_string("netflix"));
    json_object_array_add(jarr, json_object_new_string("spotify"));
    json_object_array_add(jarr, json_object_new_string("vimeo"));
    json_object_array_add(jarr, json_object_new_string("youtube"));
    json_object_array_add(jarr, json_object_new_string("vevo"));

    json_object_object_add(jobj, "VideoMediaList", jarr);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    /* Free object */
    json_object_put(jobj);

    return 0;
}
#endif

/*****************************************************************
* NAME:    get_mesh_firewall_rule_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_firewall_rule_json_cb(HTTPS_CB *pkt, MESH_FIREWALL_RULE_T setting, char *result)
{
    int i;
    struct json_object *jobj;
    struct json_object *jarr;
    char api_result[64];

    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    json_object_object_add(jobj, "Index", json_object_new_int(setting.index));
    json_object_object_add(jobj, "Name", json_object_new_string(setting.name));
    json_object_object_add(jobj, "BlockAll", json_object_new_boolean(setting.block_all));

    json_object_object_add(jobj, "UrlBlockEnable", json_object_new_boolean(setting.url_block_enable));

#if SUPPORT_FIREWALL_URL_CATEGORY
        json_object_object_add(jobj, "BlockSocialMediaEnable", json_object_new_boolean(setting.block_social_media_enable));
        json_object_object_add(jobj, "BlockSearchEngineEnable", json_object_new_boolean(setting.block_search_engine_enable));
        json_object_object_add(jobj, "BlockVideoMediaEnable", json_object_new_boolean(setting.block_video_media_enable));
        json_object_object_add(jobj, "BlockCustomURLEnable", json_object_new_boolean(setting.block_custom_url_enable));

        jarr = json_object_new_array();
        i = 0;

        while(strlen(setting.block_social_media_list[i]))
        {
            json_object_array_add(jarr, json_object_new_string(setting.block_social_media_list[i]));
            i++;
        }

        json_object_object_add(jobj, "BlockSocialMediaList", jarr);

        jarr = json_object_new_array();
        i = 0;

        while(strlen(setting.block_search_engine_list[i]))
        {
            json_object_array_add(jarr, json_object_new_string(setting.block_search_engine_list[i]));
            i++;
        }

        json_object_object_add(jobj, "BlockSearchEngineList", jarr);

        jarr = json_object_new_array();
        i = 0;

        while(strlen(setting.block_video_media_list[i]))
        {
            json_object_array_add(jarr, json_object_new_string(setting.block_video_media_list[i]));
            i++;
        }

        json_object_object_add(jobj, "BlockVideoMediaList", jarr);

        jarr = json_object_new_array();
        i = 0;

        while(strlen(setting.block_custom_url_list[i]))
        {
            json_object_array_add(jarr, json_object_new_string(setting.block_custom_url_list[i]));
            i++;
        }

        json_object_object_add(jobj, "BlockCustomURLList", jarr);
#else
    jarr = json_object_new_array();

    i = 0;

    while(strlen(setting.block_item[i]))
    {
        json_object_array_add(jarr, json_object_new_string(setting.block_item[i]));
        i++;
    }

    json_object_object_add(jobj, "BlockItems", jarr);
#endif

    json_object_object_add(jobj, "ScheduleBlockEnable", json_object_new_boolean(setting.schedule_block_enable));
    json_object_object_add(jobj, "BlockAllTime", json_object_new_boolean(setting.all_time));
    json_object_object_add(jobj, "FromHour", json_object_new_int(setting.from_hour));
    json_object_object_add(jobj, "FromMinute", json_object_new_int(setting.from_minute));
    json_object_object_add(jobj, "ToHour", json_object_new_int(setting.to_hour));
    json_object_object_add(jobj, "ToMinute", json_object_new_int(setting.to_minute));
    json_object_object_add(jobj, "WeekDay", json_object_new_string(setting.week_day));
    json_object_object_add(jobj, "Summary", json_object_new_string(setting.summary));

    jarr = json_object_new_array();

    i = 0;

    while(0 != setting.related_user[i])
    {
        json_object_array_add(jarr, json_object_new_int(setting.related_user[i]));
        i++;
    }

    json_object_object_add(jobj, "RelatedUsers", jarr);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    /* Free object */
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    get_mesh_firewall_rule_list_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_firewall_rule_list_json_cb(HTTPS_CB *pkt, MESH_FIREWALL_RULE_T *setting, char *result)
{
    int i, j;
    struct json_object *jobj;
    struct json_object *jarr, *jarr_obj;
    struct json_object *jarr_item;
    char api_result[64];

    if(NULL == pkt)
    {
        return -1;
    }

    i = 0;

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    jarr = json_object_new_array();

    while(strlen(setting[i].name))
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "Index", json_object_new_int(setting[i].index));
        json_object_object_add(jarr_obj, "Name", json_object_new_string(setting[i].name));
        json_object_object_add(jarr_obj, "BlockAll", json_object_new_boolean(setting[i].block_all));

        json_object_object_add(jarr_obj, "UrlBlockEnable", json_object_new_boolean(setting[i].url_block_enable));

#if SUPPORT_FIREWALL_URL_CATEGORY
        json_object_object_add(jarr_obj, "BlockSocialMediaEnable", json_object_new_boolean(setting[i].block_social_media_enable));
        json_object_object_add(jarr_obj, "BlockSearchEngineEnable", json_object_new_boolean(setting[i].block_search_engine_enable));
        json_object_object_add(jarr_obj, "BlockVideoMediaEnable", json_object_new_boolean(setting[i].block_video_media_enable));
        json_object_object_add(jarr_obj, "BlockCustomURLEnable", json_object_new_boolean(setting[i].block_custom_url_enable));

        jarr_item = json_object_new_array();

        j = 0;

        while(strlen(setting[i].block_social_media_list[j]))
        {
            json_object_array_add(jarr_item, json_object_new_string(setting[i].block_social_media_list[j]));
            j++;
        }

        json_object_object_add(jarr_obj, "BlockSocialMediaList", jarr_item);

        jarr_item = json_object_new_array();

        j = 0;

        while(strlen(setting[i].block_search_engine_list[j]))
        {
            json_object_array_add(jarr_item, json_object_new_string(setting[i].block_search_engine_list[j]));
            j++;
        }

        json_object_object_add(jarr_obj, "BlockSearchEngineList", jarr_item);

        jarr_item = json_object_new_array();

        j = 0;

        while(strlen(setting[i].block_video_media_list[j]))
        {
            json_object_array_add(jarr_item, json_object_new_string(setting[i].block_video_media_list[j]));
            j++;
        }

        json_object_object_add(jarr_obj, "BlockVideoMediaList", jarr_item);

        jarr_item = json_object_new_array();

        j = 0;

        while(strlen(setting[i].block_custom_url_list[j]))
        {
            json_object_array_add(jarr_item, json_object_new_string(setting[i].block_custom_url_list[j]));
            j++;
        }

        json_object_object_add(jarr_obj, "BlockCustomURLList", jarr_item);
#else
        jarr_item = json_object_new_array();

        j = 0;

        while(strlen(setting[i].block_item[j]))
        {
            json_object_array_add(jarr_item, json_object_new_string(setting[i].block_item[j]));
            j++;
        }

        json_object_object_add(jarr_obj, "BlockItems", jarr_item);
#endif

        json_object_object_add(jarr_obj, "ScheduleBlockEnable", json_object_new_boolean(setting[i].schedule_block_enable));
        json_object_object_add(jarr_obj, "BlockAllTime", json_object_new_boolean(setting[i].all_time));
        json_object_object_add(jarr_obj, "FromHour", json_object_new_int(setting[i].from_hour));
        json_object_object_add(jarr_obj, "FromMinute", json_object_new_int(setting[i].from_minute));
        json_object_object_add(jarr_obj, "ToHour", json_object_new_int(setting[i].to_hour));
        json_object_object_add(jarr_obj, "ToMinute", json_object_new_int(setting[i].to_minute));
        json_object_object_add(jarr_obj, "WeekDay", json_object_new_string(setting[i].week_day));
        json_object_object_add(jarr_obj, "Summary", json_object_new_string(setting[i].summary));

        jarr_item = json_object_new_array();

        j = 0;

        while(0 != setting[i].related_user[j])
        {
            json_object_array_add(jarr_item, json_object_new_int(setting[i].related_user[j]));
            j++;
        }

        json_object_object_add(jarr_obj, "RelatedUsers", jarr_item);

        json_object_array_add(jarr, jarr_obj);
        i++;
    }

    json_object_object_add(jobj, "Rules", jarr);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    /* Free object */
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    parse_mesh_firewall_rule_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int parse_mesh_firewall_rule_json_cb(char *query_str, MESH_FIREWALL_RULE_T *setting, char **return_str)
{
    int i = 0, count = 0;
    bool result = FALSE;
    struct json_object *jobj;
    struct json_object *jstr;
    struct json_object *jint;
    struct json_object *jarr;
    struct array_list *array_setting;

    if((NULL == query_str) || (0 == strlen(query_str)))
    {
        return result;
    }

    if((jobj = json_tokener_parse(query_str)))
    {
        senao_json_object_get_integer(jobj, "Index", &setting->index);
        senao_json_object_get_string(jobj, "Name", setting->name);
        senao_json_object_get_boolean(jobj, "BlockAll", &setting->block_all);
        senao_json_object_get_boolean(jobj, "UrlBlockEnable", &setting->url_block_enable);

#if SUPPORT_FIREWALL_URL_CATEGORY
        senao_json_object_get_boolean(jobj, "BlockSocialMediaEnable", &setting->block_social_media_enable);
        senao_json_object_get_boolean(jobj, "BlockSearchEngineEnable", &setting->block_search_engine_enable);
        senao_json_object_get_boolean(jobj, "BlockVideoMediaEnable", &setting->block_video_media_enable);
        senao_json_object_get_boolean(jobj, "BlockCustomURLEnable", &setting->block_custom_url_enable);

        jarr = json_object_object_get(jobj, "BlockSocialMediaList");
        array_setting = json_object_get_array(jarr);

        count = array_setting->length;

        for(i = 0; i < count; i++)
        {
            jstr = json_object_array_get_idx(jarr, i);
            sprintf(setting->block_social_media_list[i], "%s", json_object_get_string(jstr));
        }

        jarr = json_object_object_get(jobj, "BlockSearchEngineList");
        array_setting = json_object_get_array(jarr);

        count = array_setting->length;

        for(i = 0; i < count; i++)
        {
            jstr = json_object_array_get_idx(jarr, i);
            sprintf(setting->block_search_engine_list[i], "%s", json_object_get_string(jstr));
        }

        jarr = json_object_object_get(jobj, "BlockVideoMediaList");
        array_setting = json_object_get_array(jarr);

        count = array_setting->length;

        for(i = 0; i < count; i++)
        {
            jstr = json_object_array_get_idx(jarr, i);
            sprintf(setting->block_video_media_list[i], "%s", json_object_get_string(jstr));
        }

        jarr = json_object_object_get(jobj, "BlockCustomURLList");
        array_setting = json_object_get_array(jarr);

        count = array_setting->length;

        for(i = 0; i < count; i++)
        {
            jstr = json_object_array_get_idx(jarr, i);
            sprintf(setting->block_custom_url_list[i], "%s", json_object_get_string(jstr));
        }
#else
        jarr = json_object_object_get(jobj, "BlockItems");
        array_setting = json_object_get_array(jarr);

        count = array_setting->length;

        for(i = 0; i < count; i++)
        {
            jstr = json_object_array_get_idx(jarr, i);
            sprintf(setting->block_item[i], "%s", json_object_get_string(jstr));
        }
#endif

        senao_json_object_get_boolean(jobj, "ScheduleBlockEnable", &setting->schedule_block_enable);
        senao_json_object_get_boolean(jobj, "BlockAllTime", &setting->all_time);
        senao_json_object_get_integer(jobj, "FromHour", &setting->from_hour);
        senao_json_object_get_integer(jobj, "FromMinute", &setting->from_minute);
        senao_json_object_get_integer(jobj, "ToHour", &setting->to_hour);
        senao_json_object_get_integer(jobj, "ToMinute", &setting->to_minute);
        senao_json_object_get_string(jobj, "WeekDay", setting->week_day);
        senao_json_object_get_string(jobj, "Summary", setting->summary);

        jarr = json_object_object_get(jobj, "RelatedUsers");
        array_setting = json_object_get_array(jarr);

        count = array_setting->length;

        for(i = 0; i < count; i++)
        {
            jint = json_object_array_get_idx(jarr, i);
            setting->related_user[i] =  json_object_get_int(jint);
        }

        /* Free object */
        json_object_put(jobj);

        *return_str = OK_STR;
        result = TRUE;
    }

    return result;
}
#endif

/*****************************************************************
* NAME:    ezsetup_get_candidates_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int ezsetup_get_candidates_json_cb(HTTPS_CB *pkt, char *result)
{
	struct json_object *jobj;
	struct json_object *jarr;
	struct json_object *jarr_obj;

	char api_result[64]={0}, buf[512]={0}, escape_buf[700]={0}, target_ip[32]={0}, master_r_domain[32]={0};
	char *mesh_mac, *ptr = NULL;
    char BluetoothID_tmp[1024]={0}, BluetoothID[32]={0}, RegularDomain[16]={0};
	int  eth_status=0, device_type=0;
	int ez_counter=0, ez_setup_flag=1; //flag 1 support scanning method
	char ez_buf[128]={0}, ez_vendor[32]={0}, ez_id_buf[64]={0}, ez_sn_buf[12]={0};
	FILE *fp = NULL;
	char ez_mesh_mac[18]={0}, ez_mesh[3]={0}, ez_sn[16]={0}, ez_cfg[3]={0}, ez_model[4]={0}, ez_domain[3]={0};


	if(NULL == pkt)
	{
		return -1;
	}

	jobj = json_object_new_object();

    sysutil_interact(master_r_domain, sizeof(master_r_domain), "setconfig -g 4");
    master_r_domain[strlen(master_r_domain)-1]=0;

    json_object_object_add(jobj, "MasterRegularDomain", json_object_new_string(master_r_domain));

	if(ez_setup_flag)
	{
		jarr = json_object_new_array();

		if(sysutil_check_file_existed(EzSetup_SCAN_RESULT))
		{
			SYSTEM("cp /tmp/ezsetup_scan_result /tmp/ezsetup_scan_result_bak");
			if(sysutil_check_file_existed("/tmp/temp_mesh_scan_mac"))
			{
				SYSTEM("rm -f /tmp/temp_mesh_scan_mac");
			}
			if(NULL != (fp = fopen(EzSetup_SCAN_RESULT, "r")))
			{
				ez_counter=0;
				while(NULL != fgets(buf, sizeof(buf), fp))
				{
					sysCheckSingleQuoteOnString(buf, escape_buf);
					sysutil_interact(ez_buf,sizeof(ez_buf), "echo '%s', |awk '{split($0,a,\"\t\"); printf a[1]\" \"a[10]\" \"a[11]\" \"a[12]\" \"a[13]\" \"a[14]}'", escape_buf);
					sscanf(ez_buf, "%s %s %s %s %s %s",
					ez_mesh_mac, ez_mesh, ez_cfg, ez_sn, ez_model, ez_domain);
					if(strcmp(ez_mesh, "U") == 0)
					{
						printf("**** result %s, [%s], cfg[%s], sn[%s], model[%s], domain[%s]\n", ez_mesh_mac, ez_mesh, ez_cfg, ez_sn, ez_model, ez_domain);
						if(strcmp(ez_cfg, "0") == 0)
						{
							SYSTEM("echo %s >> /tmp/temp_mesh_scan_mac", ez_mesh_mac);
							jarr_obj = json_object_new_object();
							json_object_object_add(jarr_obj, "MeshMac", json_object_new_string(ez_mesh_mac));
							api_get_string_option("sysProductInfo.model.venderName", ez_vendor, sizeof(ez_vendor));

							sprintf(ez_sn_buf, "%d", (atoi(ez_sn) % 1000000));

							if(atoi(ez_model) == 2 || atoi(ez_model) == 5)
                            {
                                sprintf(ez_id_buf, "%s%06d", "EMRSetup", atoi(ez_sn_buf));
                            }
                            else
                            {
                                sprintf(ez_id_buf, "%s%06d", ez_vendor, atoi(ez_sn_buf));
                            }

							json_object_object_add(jarr_obj, "BluetoothID", json_object_new_string(ez_id_buf));
							json_object_object_add(jarr_obj, "RegularDomain", json_object_new_string(ez_domain));
							json_object_object_add(jarr_obj, "DeviceType", json_object_new_int(atoi(ez_model)));
							json_object_array_add(jarr, jarr_obj);

							result=OK_STR;
						}
					}
					ez_counter++;
				}
				fclose(fp);
			}
		}
		SYSTEM("echo \"iwlist " EzSetup_24g_SCANNING_TMP " scanning | tail -n +3 > " EzSetup_SCAN_RESULT"\" > /tmp/ezsetup_candidate_scan.sh");

		SYSTEM("sh /tmp/ezsetup_candidate_scan.sh &");

#if 0 //using scanning result instead
		if(sysutil_check_file_existed("/tmp/temp_mesh_mac"))
		{
			SYSTEM("rm -rf /tmp/temp_mesh_mac");
		}

		SYSTEM("wlanconfig "EzSetup_24g_MSC_TMP" list | grep -v \"^No\" |awk -F\" \" '{ print $1 }' > /tmp/temp_mesh_mac");

		if(sysutil_check_file_existed("/tmp/temp_mesh_mac"))
		{
			sysutil_interact(buf, sizeof(buf), "cat /tmp/temp_mesh_mac");
			if(strlen(buf) > 0)
			{
				mesh_mac = strtok(buf, "\n");
			}
			while (mesh_mac != NULL)
			{
				if (strstr(mesh_mac, ":"))
				{
					memset(ez_buf, 0x00, sizeof(ez_buf));
					if(sysutil_check_file_existed("/tmp/temp_mesh_scan_mac"))
					{
						sysutil_interact(ez_buf, sizeof(ez_buf), "cat /tmp/temp_mesh_scan_mac | grep -i %s", mesh_mac);
					}
					if(strlen(ez_buf))
					{
						printf("****** WE FOUND WDS mesh_mac %s, do nothing\n", mesh_mac);
					}
					else
					{
						printf("****** NOT FOUND WDS mesh_mac %s, add to list\n", mesh_mac);
						memset(target_ip, 0x00, sizeof(target_ip));
						sysutil_interact(target_ip, sizeof(target_ip), "/sbin/mesh.sh mac_to_ipv6_unique_local %s", mesh_mac);
						if(NULL != (ptr = strchr(target_ip, '\n')))
							*ptr = '\0';

						memset(BluetoothID_tmp, 0x00, sizeof(BluetoothID_tmp));
						memset(BluetoothID, 0x00, sizeof(BluetoothID));

						sysutil_interact(BluetoothID_tmp, sizeof(BluetoothID_tmp), "app_client -m GET -i %s -a GetSNNumber -e 1", target_ip);
						if(NULL != (ptr = strchr(BluetoothID_tmp, '\n')))
							*ptr = '\0';

						get_json_string_from_query(BluetoothID_tmp, BluetoothID, "MeshDeviceLocation");
						get_json_integer_from_query(BluetoothID_tmp, &eth_status, "EthStatus");
						get_json_string_from_query(BluetoothID_tmp, RegularDomain, "RegularDomain");
						get_json_integer_from_query(BluetoothID_tmp, &device_type, "DeviceType");

						jarr_obj = json_object_new_object();

						json_object_object_add(jarr_obj, "MeshMac", json_object_new_string(mesh_mac));
						json_object_object_add(jarr_obj, "BluetoothID", json_object_new_string(BluetoothID));
						json_object_object_add(jarr_obj, "EthStatus", json_object_new_int(eth_status));
						json_object_object_add(jarr_obj, "RegularDomain", json_object_new_string(RegularDomain));
						json_object_object_add(jarr_obj, "DeviceType", json_object_new_int(device_type));

						json_object_array_add(jarr, jarr_obj);

						result=OK_STR;
					}
				}
				mesh_mac = strtok(NULL, "\n");
			}
		}
#endif

		json_object_object_add(jobj, "MeshNodeList", jarr);
	}

	memset(api_result, 0x00, sizeof(api_result));
	sprintf(api_result, "%sResult", pkt->json_action);
	json_object_object_add(jobj, api_result, json_object_new_string(result));

	basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

	json_object_put(jobj);

	return 0;
}
/*****************************************************************
* NAME:    ezsetup_get_candidates_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_json_ezsetup_add_mesh_nodes(char *query_str, char *ipList, char *FailList, char **return_str)
{
	struct json_object *jobj;
	struct json_object *jarr;
	struct json_object *jarr_obj;
	struct array_list *array_setting;

	int i = 0, count = 0, j = 0, k=0;
	bool result = FALSE;
	char mac[32], target_ip[32];
	char *ptr = NULL;
	char BluetoothID_tmp[1024], BluetoothID[32], GetSNNumberResult[32];

	if((NULL == query_str) || (0 == strlen(query_str)))
	{
		return result;
	}

	if((jobj = json_tokener_parse(query_str)))
	{
		*return_str = OK_STR;
		result = TRUE;

		jarr = json_object_object_get(jobj, "MeshNodeList");
		array_setting = json_object_get_array(jarr);
		count = array_setting->length;

		for(i = 0; i < count; i++)
		{
			jarr_obj = json_object_array_get_idx(jarr, i);
			memset(mac, 0x00, sizeof(mac));

			if(FALSE == senao_json_object_get_string(jarr_obj, "MeshMac", mac))
			{
				*return_str = ERROR_STR;
				result = FALSE;
			}
			else
			{
				if (strlen(mac)!=0)
				{
					memset(target_ip, 0x00, sizeof(target_ip));
					sysutil_interact(target_ip, sizeof(target_ip), "/sbin/mesh.sh mac_to_ipv6_unique_local %s", mac);
					if(NULL != (ptr = strchr(target_ip, '\n')))
						*ptr = '\0';
/*
					memset(BluetoothID_tmp, 0x00, sizeof(BluetoothID_tmp));
					memset(BluetoothID, 0x00, sizeof(BluetoothID));
					memset(GetSNNumberResult, 0x00, sizeof(GetSNNumberResult));
					sysutil_interact(BluetoothID_tmp, sizeof(BluetoothID_tmp), "app_client -m GET -i %s -a GetSNNumber -e 1", target_ip);
					if(NULL != (ptr = strchr(BluetoothID_tmp, '\n')))
						*ptr = '\0';
					get_json_string_from_query(BluetoothID_tmp, BluetoothID, "MeshDeviceLocation");
					get_json_string_from_query(BluetoothID_tmp, GetSNNumberResult, "GetSNNumberResult");
*/
					/* check all devices are alive before add mesh nodes */
//					if ((strcmp("OK", GetSNNumberResult) == 0) && (strcmp("", BluetoothID) != 0) )
					{
						if (j == 0)
						{
							j+= sprintf(ipList+j, "%s", target_ip);
						}
						else
						{
							j+= sprintf(ipList+j, " %s", target_ip);
						}
					}
//					else
//					{
//						k+= sprintf(FailList+k, "%s%s", (k==0)?"":" ", mac);
//						*return_str = ERROR_STR;
//						result = FALSE;
//					}
				}
			}
		}

		/* Free object */
		json_object_put(jobj);
	}

	return result;
}
/*****************************************************************
 * NAME:    get_ibeacon_settings_result_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:  
 * Modify:  
 ******************************************************************/
int get_ibeacon_settings_result_json_cb(HTTPS_CB *pkt, char *result, int action)
{
    struct json_object *jobj, *jarr_obj, *jarr;
    char api_result[64]={0};
    char buf[256]={0};

    char minor[32]={0}, uuid[256]={0}, major[32]={0}, enable[32]={0}, text[2048]={0}, mac[128]={0};
    char *ptr;
    FILE *fp;
    char deviceType[32]={0}, locationName[64]={0};


    if(NULL == pkt)
    {
        return FALSE;
    }

    memset(deviceType, 0x00, sizeof(deviceType));
    sysutil_interact(deviceType, sizeof(deviceType), "batctl gw |awk '{ print $1 }' | tr -d '\n'x");

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    if (action == GET_ALL_MESH_IBEACON_SETTINGS)
    {
        if (0 == strcmp(deviceType, "server"))
        {
            if(NULL != "/tmp/mesh_ibeacon_settings")
            {
                if(NULL != (fp = fopen("/tmp/mesh_ibeacon_settings", "r")))
                {
                    jarr = json_object_new_array();

                    while(NULL != fgets(buf, sizeof(buf), fp))
                    {
                        jarr_obj = json_object_new_object();
                        /* Remove the character '\n' */
                        if(NULL != (ptr = strrchr(buf, '\n')))
                        {
                            *ptr = '\0';
                        }

                        //sysutil_interact(buf,sizeof(buf), "echo \"%s\", |awk '{split($0,a,\"\t\"); printf a[1]\" \"a[2]\" \"a[3]\" \"a[4]\" \"a[5]\" \"a[6]}'", buf);
                        sscanf(buf,"%s\t%s\t%s\t%s\t%[^\t]\t%s",enable, uuid, major, minor, text, mac);
                        json_object_object_add(jarr_obj, "Enable", json_object_new_string(enable));
                        json_object_object_add(jarr_obj, "UUID", json_object_new_string(uuid));
                        json_object_object_add(jarr_obj, "Major", json_object_new_string(major));
                        json_object_object_add(jarr_obj, "Minor", json_object_new_string(minor));
                        json_object_object_add(jarr_obj, "Text", json_object_new_string(text));
                        json_object_object_add(jarr_obj, "MAC", json_object_new_string(mac));

                        sys_interact(locationName, sizeof(locationName), "lua /usr/lib/lua/luci/get_location_form_mac.lua %s | tr -d '\\n'",mac);
                        json_object_object_add(jarr_obj, "LocationName", json_object_new_string(locationName));

                        json_object_array_add(jarr, jarr_obj);
                    }
                    fclose(fp);
                    json_object_object_add(jobj, "DeviceList", jarr);
                    result = OK_STR;
                }
            }
            else
            {
                result = ERROR_STR;
            }
        }
        else
        {
            result = OK_STR;
        }
    }

    json_object_object_add(jobj, api_result, json_object_new_string(result));
    json_object_object_add(jobj, "ResponseAction", json_object_new_int(action));
    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));
    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return TRUE;
}

/*****************************************************************
* NAME:    get_mesh_divece_client_info_json_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_divece_client_info_json_cb(HTTPS_CB *pkt, MESH_CLIENT_INFO_T ClientInfo,MESH_GUEST_CLIENT_INFO_T gClientInfo, char *result)
{
    int i;
    struct json_object *jobj;
    //struct json_object *jobj_device;
    struct json_object *jobj_guestClient;
    struct json_object *jobj_Client;
    struct json_object *jarr;
    struct json_object *jarr_obj;
    char api_result[64];
    
    if(NULL == pkt)
    {
        return -1;
    }

    jobj = json_object_new_object();

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    //json_object_object_add(jobj_device, "DevicesConnectedNumber", json_object_new_int(i));

    //######################################Client#####################################//
    jobj_Client = json_object_new_object();

    i = 0;
    jarr = json_object_new_array();

    while(0 != strlen(ClientInfo.client24_mac[i]))
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "MAC", json_object_new_string(ClientInfo.client24_mac[i]));
        json_object_object_add(jarr_obj, "IP", json_object_new_string(ClientInfo.client24_ip[i]));
        json_object_object_add(jarr_obj, "HostName", json_object_new_string(ClientInfo.client24_hostname[i]));

        json_object_array_add(jarr, jarr_obj);
        i++;
    }

    json_object_object_add(jobj_Client, "_2G", jarr);
    i = 0;
    jarr = json_object_new_array();

    while(0 != strlen(ClientInfo.client5_mac[i]))
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "MAC", json_object_new_string(ClientInfo.client5_mac[i]));
        json_object_object_add(jarr_obj, "IP", json_object_new_string(ClientInfo.client5_ip[i]));
        json_object_object_add(jarr_obj, "HostName", json_object_new_string(ClientInfo.client5_hostname[i]));

        json_object_array_add(jarr, jarr_obj);
        i++;
    }
    json_object_object_add(jobj_Client, "_5G", jarr);

    json_object_object_add(jobj, "ClientList", jobj_Client);

    //######################################Guest Client#####################################//
    jobj_guestClient = json_object_new_object();

    i = 0;
    jarr = json_object_new_array();

    while(0 != strlen(gClientInfo.guest24_mac[i]))
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "MAC", json_object_new_string(gClientInfo.guest24_mac[i]));
        json_object_object_add(jarr_obj, "IP", json_object_new_string(gClientInfo.guest24_ip[i]));
        json_object_object_add(jarr_obj, "HostName", json_object_new_string(gClientInfo.guest24_hostname[i]));

        json_object_array_add(jarr, jarr_obj);
        i++;
    }

    json_object_object_add(jobj_guestClient, "_2G", jarr);
    i = 0;
    jarr = json_object_new_array();

    while(0 != strlen(gClientInfo.guest5_mac[i]))
    {
        jarr_obj = json_object_new_object();

        json_object_object_add(jarr_obj, "MAC", json_object_new_string(gClientInfo.guest5_mac[i]));
        json_object_object_add(jarr_obj, "IP", json_object_new_string(gClientInfo.guest5_ip[i]));
        json_object_object_add(jarr_obj, "HostName", json_object_new_string(gClientInfo.guest5_hostname[i]));

        json_object_array_add(jarr, jarr_obj);
        i++;
    }
    json_object_object_add(jobj_guestClient, "_5G", jarr);

    json_object_object_add(jobj, "GuestClientList", jobj_guestClient);

    basic_json_response(pkt, (char *)json_object_to_json_string(jobj));

    json_object_put(jobj);

    return 0;
}

/*****************************************************************
 * NAME:    get_mesh_trace_route_result_json_cb
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
int get_mesh_trace_route_result_json_cb(HTTPS_CB *pkt, char *result_file_path, char *result)
{
    struct json_object *jobj;
    struct json_object *jarr;
    struct json_object *jobj_trace_info;
    char buf[256];
    char trace_time_1[256];
    char api_result[64];
    char *ptr;
    FILE *fp;
    char mac[32];
    char trace_time[256];
    
    if(NULL == pkt)
    {
        return FALSE;
    }

    fp = NULL;

    /* Construct the packet content in json format. */
    jobj = json_object_new_object();

    if(NULL != result_file_path)
    {
        if(NULL != (fp = fopen(result_file_path, "r")))
        {
            jarr = json_object_new_array();

            while(NULL != fgets(buf, sizeof(buf), fp))
            {            
                memset(mac, 0x00, sizeof(mac));
                memset(trace_time, 0x00, sizeof(trace_time));
                memset(trace_time_1, 0x00, sizeof(trace_time_1));

                sscanf(buf, "%*s %s %[^\n]\n", mac, trace_time);

                sysutil_interact(trace_time_1,sizeof(trace_time_1), "echo \"%s\" | awk '{for (i=1; i<=NF;i++) printf \"%%s%%s\", ($i==\"*\")?\"*, \":$i, (i==NF)?\"\":($i==\"ms\")?\", \":\"\"}'", trace_time);
               
                jobj_trace_info = json_object_new_object();

                json_object_object_add(jobj_trace_info, "MAC", json_object_new_string(mac));
                json_object_object_add(jobj_trace_info, "TraceTime", json_object_new_string(trace_time_1));

                json_object_array_add(jarr, jobj_trace_info);
            }

            fclose(fp);
            json_object_object_add(jobj, "TestResult", jarr);
        }
    }
    else
    {
        result = ERROR_STR;
    }

    memset(api_result, 0x00, sizeof(api_result));
    sprintf(api_result, "%sResult", pkt->json_action);
    json_object_object_add(jobj, api_result, json_object_new_string(result));

    /* Store packet content into buffer and send it out */
    http_store_data_to_buffer(HTTP_RESPONSE_CODE_200_TEMPLATE);
    http_store_data_to_buffer(HTTP_JSON_RESPONSE_HEADER_TEMPLATE);
    http_store_data_to_buffer("%s\n", json_object_to_json_string(jobj));

    http_send_stored_data(pkt->fd);

    /* Free obj */
    json_object_put(jobj);

    return TRUE;
}
#if HAS_MESH_STATIC_ROUTE
/*****************************************************************
* NAME:    parse_json_allow_mac_list_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool parse_json_allow_mac_list_cb(char *query_str, MESH_ALLOW_MACLIST_T *setting, char **return_str)
{
    struct json_object *jstr;
    struct json_object *jobj;
    struct json_object *jarr;
    struct json_object *jarr_obj;
    struct array_list *array_setting;
    int i = 0, jcount = 0;
    bool result = FALSE;
    char *ptr = NULL;

    if ((NULL == query_str) || (0 == strlen(query_str))) {
        return result;
    }

    if ((jobj = json_tokener_parse(query_str))) {
        jarr = json_object_object_get(jobj, "AllowMac");
        array_setting = json_object_get_array(jarr);
        jcount = array_setting->length;

        for (i = 0; i < jcount; i++) {
            jstr = json_object_array_get_idx(jarr, i);
            sprintf(setting->aw_mac[i], "%s\0", json_object_get_string(jstr));
        }
        setting->aw_num = jcount;

        /* Free object */
        json_object_put(jobj);

        *return_str = OK_STR;
        result = TRUE;
    }

    return result;
}
#endif

