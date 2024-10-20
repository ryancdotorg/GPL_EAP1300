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
;    File    : wlan_setting.c
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
#include "gconfig.h"
#include "opmodes.h"
#include "app_agent.h"
#include "wlan_setting.h"
#include "mesh_setting.h"
#include "device_setting.h"
#include "mesh_json_setting.h"
#include "deviceinfo.h"
#include "variable/api_wan.h"
#include "variable/api_mesh.h"
#include "utility/sys_common.h"
#include <unistd.h>
#include "json.h"

/*--------------------------------------------------------------------------*/
/*                                DEFINITION                                */
/*--------------------------------------------------------------------------*/
typedef struct _APP_MESH_SETTING_
{
    bool used;
    char operating_mesh_band[8];
    WLAN_RADIOS_T setting;
} APP_MESH_SETTING;

#if HAS_MESH_STATIC_ROUTE
#define MESH_ALLOW_MAC_LIST    "/sbin/mesh_allow_mac_list.sh"
#define MESH_SR_INFORM_BH      "/sbin/MeshInformSR_bh.sh"
#endif
/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*                             GLOBAL VARIABLE                              */
/*--------------------------------------------------------------------------*/
extern SECURITY_TYPE security_type[];
extern ENCRYPTION_TYPE encryption_type[];
APP_MESH_SETTING global_mesh_setting;

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
/*****************************************************************
* NAME:    remove_colon_from_mac
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
void remove_colon_from_mac(char *src, char *dest)
{
    int i, j;

    for(i = 0, j = 0; '\0' != src[i]; i++)
    {
        if(':' != src[i])
        {
            dest[j] = src[i];
            j++;
        }
    }
}

/*****************************************************************
* NAME:    hex_to_dec
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int hex_to_dec(char hex)
{
    int dec;

    dec = -1;

    if('0' <= hex && '9' >= hex)
    {
        dec = atoi(&hex);
    }
    else if('a' == hex || 'A' == hex)
    {
        dec = 10;
    }
    else if('b' == hex || 'B' == hex)
    {
        dec = 11;
    }
    else if('c' == hex || 'C' == hex)
    {
        dec = 12;
    }
    else if('d' == hex || 'D' == hex)
    {
        dec = 13;
    }
    else if('e' == hex || 'E' == hex)
    {
        dec = 14;
    }
    else if('f' == hex || 'F' == hex)
    {
        dec = 15;
    }

    return dec;
}

/*****************************************************************
* NAME:    ipv6_to_mac
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool ipv6_to_mac(char *ipv6, char *mac)
{
    int  i;
    int  diff;
    int  mac_value[6];
    bool res;
    char *ptr1, *ptr2;

    i = 0;
    res = FALSE;
    memset(mac_value, 0x00, sizeof(mac_value));

    //printf "fc80::%x:%x:%x:%x\n" 0x"`printf %x $((0x${1}^0x02))`"${2} 0x"${3}ff" 0x"fe${4}" 0x"${5}${6}"
    ptr1 = strchr(ipv6, ':');

    while(NULL != ptr1)
    {
        i++;
        ptr1 = strchr(ptr1 + 1, ':');
    }

    if(5 == i)
    {
        ptr1 = strstr(ipv6, "::") + 2;
        ptr2 = strchr(ptr1, ':');
        diff = ptr2 - ptr1;

        if(1 == diff)
        {
            mac_value[1] = hex_to_dec(*(ptr1));
        }
        else if(2 == diff)
        {
            mac_value[1] = hex_to_dec(*ptr1) * 16 + hex_to_dec(*(ptr1 + 1));
        }
        else
        {
            if(3 == diff)
            {
                mac_value[0] = hex_to_dec(*ptr1);
                ptr1 += 1;
            }
            else if(4 == diff)
            {
                mac_value[0] = hex_to_dec(*ptr1) * 16 + hex_to_dec(*(ptr1 + 1));
                ptr1 += 2;
            }

            mac_value[1] = hex_to_dec(*ptr1) * 16 + hex_to_dec(*(ptr1 + 1));
        }

        mac_value[0] ^= 0x02;

        ptr1 = ptr2 + 1;
        ptr2 = strchr(ptr1, ':');
        diff = ptr2 - ptr1;

        if(3 == diff)
        {
            mac_value[2] = hex_to_dec(*(ptr1));
        }
        else if(4 == diff)
        {
            mac_value[2] = hex_to_dec(*ptr1) * 16 + hex_to_dec(*(ptr1 + 1));
        }

        ptr1 = ptr2 + 3;
        ptr2 = strchr(ptr1, ':');
        diff = ptr2 - ptr1;

        mac_value[3] = hex_to_dec(*ptr1) * 16 + hex_to_dec(*(ptr1 + 1));

        ptr1 = ptr2 + 1;
        ptr2 = strchr(ptr1, '\0');
        diff = ptr2 - ptr1;

        if(1 == diff)
        {
            mac_value[5] = hex_to_dec(*(ptr1));
        }
        else if(2 == diff)
        {
            mac_value[5] = hex_to_dec(*ptr1) * 16 + hex_to_dec(*(ptr1 + 1));
        }
        else
        {
            if(3 == diff)
            {
                mac_value[4] = hex_to_dec(*ptr1);
                ptr1 += 1;
            }
            else if(4 == diff)
            {
                mac_value[4] = hex_to_dec(*ptr1) * 16 + hex_to_dec(*(ptr1 + 1));
                ptr1 += 2;
            }

            mac_value[5] = hex_to_dec(*ptr1) * 16 + hex_to_dec(*(ptr1 + 1));
        }

        if(mac_value[0] >= 0 && mac_value[1] >= 0 && mac_value[2] >= 0
                && mac_value[3] >= 0 && mac_value[4] >= 0 && mac_value[5] >= 0)
        {
            sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
                    mac_value[0], mac_value[1], mac_value[2], mac_value[3], mac_value[4], mac_value[5]);

            res = TRUE;
        }
    }

    return res;
}

/*****************************************************************
* NAME:    mac_to_ipv6
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool mac_to_ipv6(char *mac, char *ipv6)
{
    bool res;
    int mac_value[6], i;
    char mac_address[32];

    res = FALSE;

    if(NULL != mac && 0 != strlen(mac) && NULL != ipv6)
    {
        i = 0;
        memset(mac_value, 0x00, sizeof(mac_value));
        memset(mac_address, 0x00, sizeof(mac_address));

        remove_colon_from_mac(mac, mac_address);

        mac_value[0] = hex_to_dec(mac_address[0]) * 16 + hex_to_dec(mac_address[1]);
        mac_value[1] = hex_to_dec(mac_address[2]) * 16 + hex_to_dec(mac_address[3]);
        mac_value[2] = hex_to_dec(mac_address[4]) * 16 + hex_to_dec(mac_address[5]);
        mac_value[3] = hex_to_dec(mac_address[6]) * 16 + hex_to_dec(mac_address[7]);
        mac_value[4] = hex_to_dec(mac_address[8]) * 16 + hex_to_dec(mac_address[9]);
        mac_value[5] = hex_to_dec(mac_address[10]) * 16 + hex_to_dec(mac_address[11]);

        //printf "fc00::%x:%x:%x:%x\n" 0x"`printf %x $((0x${1}^0x02))`"${2} 0x"${3}ff" 0x"fe${4}" 0x"${5}${6}"
        i = sprintf(ipv6, "fc00::");

        if(0 == (mac_value[0] ^ 0x02))
        {
            if(0 != mac_value[1])
            {
                i += sprintf(ipv6+i, "%x:", mac_value[1]);
            }
        }
        else
        {
            i += sprintf(ipv6+i, "%x%02x:", mac_value[0] ^ 0x02, mac_value[1]);
        }

        if(0 == mac_value[2])
        {
            i += sprintf(ipv6+i, "ff:fe%02x:", mac_value[3]);
        }
        else
        {
            i += sprintf(ipv6+i, "%xff:fe%02x:", mac_value[2], mac_value[3]);
        }


        if(0 == mac_value[4])
        {
            if(0 == mac_value[5])
            {
                i += sprintf(ipv6+i, "0");
            }
            else
            {

                i += sprintf(ipv6+i, "%x", mac_value[5]);
            }
        }
        else
        {
            i += sprintf(ipv6+i, "%x%02x", mac_value[4], mac_value[5]);
        }

        res = TRUE;
    }

    return res;
}

/*****************************************************************
* NAME:    is_mesh_function_enabled
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool is_mesh_function_enabled()
{
	int mesh24g_disabled=0,mesh5g_disabled=0;
#if HAS_WLAN_5G_2_SETTING
	int mesh5g_2_disabled=0;
#endif
	api_get_wifi_mesh_disabled_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, &mesh24g_disabled);
	api_get_wifi_mesh_disabled_option(WIRELESS_WIFI_5G_MESH_DISABLED_OPTION, &mesh5g_disabled);
#if HAS_WLAN_5G_2_SETTING
	api_get_wifi_mesh_disabled_option("wireless.wifi2_mesh.disabled", &mesh5g_2_disabled);
#endif
	if (!mesh24g_disabled || !mesh5g_disabled
#if HAS_WLAN_5G_2_SETTING
	|| !mesh5g_2_disabled
#endif
	)
		return TRUE;
	return FALSE;
}

/*****************************************************************
* NAME:    __is_mesh_function_enabled
* ---------------------------------------------------------------
* FUNCTION: improve is_mesh_function_enabled
* INPUT:    
* OUTPUT:  2: 5G mesh enable, 1: 2.4G mesh enable, 0:mesh dsabled
* Author:   
* Modify:   
******************************************************************/
int __is_mesh_function_enabled()
{
    int mesh24g_disabled=0, mesh5g_disabled=0;

    api_get_wifi_mesh_disabled_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, &mesh24g_disabled);
    api_get_wifi_mesh_disabled_option(WIRELESS_WIFI_5G_MESH_DISABLED_OPTION, &mesh5g_disabled);

    if (!mesh5g_disabled)
        return 2; // 5G
    else if(!mesh24g_disabled)
        return 1; // 2.4G
    else
        return 0; // disable
}

/*****************************************************************
* NAME:    check_mesh_account
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool check_mesh_account(char *query_str)
{
	bool result=0;
	char user_name[64],ssid[64],password[128],key[128];

	memset(user_name,0x0,sizeof(user_name));
	memset(password,0x0,sizeof(password));
	get_json_mesh_account(query_str, user_name, password);

	memset(ssid,0x0,sizeof(ssid));
	api_get_wifi_mesh_ssid_option(WIRELESS_WIFI_MESH_SSID_OPTION, ssid, sizeof(ssid));

	//check the user_name is 2.4G or not.
	if (!strcmp(user_name,ssid)){
		memset(key,0x0,sizeof(key));
		api_get_wifi_mesh_wpa_key_option(WIRELESS_WIFI_MESH_AESKEY_OPTION, key, sizeof(key));
		if(!strcmp(password,key))
			result = TRUE;
	}

	memset(ssid,0x0,sizeof(ssid));
	api_get_wifi_mesh_ssid_option(WIRELESS_WIFI_5G_MESH_SSID_OPTION, ssid, sizeof(ssid));

	//check the user_name is 5G or not
	if (!strcmp(user_name,ssid)){
		memset(key,0x0,sizeof(key));
		api_get_wifi_mesh_wpa_key_option(WIRELESS_WIFI_5G_MESH_AESKEY_OPTION, key, sizeof(key));
		if(!strcmp(password,key))
			result = TRUE;
	}
	return result;
}

/*****************************************************************
* NAME:    check_temp_account
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool check_temp_account(char *query_str)
{
    bool result = 0;
    char user_name[65], ssid[65], password[129], key[129];

    memset(user_name, 0x0, sizeof(user_name));
    memset(password, 0x0, sizeof(password));
	get_json_mesh_account(query_str, user_name, password);

    memset(ssid, 0x0, sizeof(ssid));
    memset(key, 0x0, sizeof(key));
    sysutil_interact(ssid, sizeof(ssid), "cat /proc/mesh_temp");

    if ((ssid != 0) && (strlen(ssid) > 0))
    {
        ssid[strlen(ssid)-1] = '\0';
    }

	if (!strcmp(user_name, ssid))
	{
		if (api_get_string_option("wireless.wifi0_mesh.t_esPwd", key, sizeof(key)))
		{
			api_get_string_option("wireless.wifi1_mesh.t_esPwd", key, sizeof(key));
		}
		if (!strcmp(password, key))
			result = TRUE;
	}

    return result;
}

/*****************************************************************
* NAME:    check_mesh_private_mac
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool check_mesh_private_mac(char *target)
{
    bool result;
    int mesh24g_disabled;
    char buf[32];
    char interface_name[32];
    char *ptr;

    result = FALSE;

    memset(interface_name, 0x00, sizeof(interface_name));
    api_get_wifi_mesh_disabled_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, &mesh24g_disabled);
    //mesh24g_disabled =0, get 2.4GHz interface name
    //mesh24g_disabled =1, get 5GHz interface name
    api_get_wifi_mesh_ifname_option((mesh24g_disabled)?1:0,interface_name,sizeof(interface_name));

#if HAS_WLAN_5G_2_SETTING
	if(mesh24g_disabled == 1)
	{
		sysutil_interact(interface_name, sizeof(interface_name), "getinfo mesh_ifname | tr -d \"\n\"");
	}
#endif

    memset(buf, 0x00, sizeof(buf));
    sysutil_interact(buf, sizeof(buf),
            "ifconfig %s | awk \'/HWaddr/{print $5}\'", interface_name);

    ptr = strchr(buf, '\n');

    if(NULL != ptr)
    {
        *ptr = '\0';
    }

    if(0 == strcasecmp(buf, target))
    {
        result = TRUE;
    }

    return result;
}

/*****************************************************************
* NAME:    check_mesh_private_ip
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool check_mesh_private_ip(char *target_ip)
{
    bool result;
    char buf[32];
    char *ptr;

    result = FALSE;

    memset(buf, 0x00, sizeof(buf));
    sysutil_interact(buf, sizeof(buf),
            "ifconfig %s:avahi | grep \"inet addr\" | awk \'{print $2}\' | awk -F: \'{print $2}\'", BRG_DEV);

    ptr = strchr(buf, '\n');

    if(NULL != ptr)
    {
        *ptr = '\0';
    }

    if(0 == strcmp(buf, target_ip))
    {
        result = TRUE;
    }

    return result;
}

/*****************************************************************
* NAME:    check_mesh_lan_ip
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool check_mesh_lan_ip(char *target_ip)
{
    bool result;
    char buf[32];
    char *ptr;

    result = FALSE;

    memset(buf, 0x00, sizeof(buf));
    sysutil_interact(buf, sizeof(buf),
            "ifconfig %s | grep \"inet addr\" | awk \'{print $2}\' | awk -F: \'{print $2}\'", BRG_DEV);

    ptr = strchr(buf, '\n');

    if(NULL != ptr)
    {
        *ptr = '\0';
    }

    if(0 == strcmp(buf, target_ip))
    {
        result = TRUE;
    }

    return result;
}

/*****************************************************************
* NAME:    check_mesh_private_ipv6
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool check_mesh_private_ipv6(char *target_ip)
{
    bool result;
    char buf[32];
    char *ptr;

    result = FALSE;

    memset(buf, 0x00, sizeof(buf));
    sysutil_interact(buf, sizeof(buf),
            "ifconfig %s | grep \"inet6 addr\" | awk \'{print $3}\' | awk -F \"/\" \'{print $1}\'", BRG_DEV);

    ptr = strchr(buf, '\n');

    if(NULL != ptr)
    {
        *ptr = '\0';
    }

    if(0 == strcasecmp(buf, target_ip))
    {
        result = TRUE;
    }

    return result;
}

/*****************************************************************
* NAME:    check_mesh_global_ipv6
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
bool check_mesh_global_ipv6(char *target_ip)
{
    bool result;
    char buf[128];
    char *ptr;

    result = FALSE;

    memset(buf, 0x00, sizeof(buf));
    sysutil_interact(buf, sizeof(buf),
            "ifconfig %s | grep \"inet6 addr: fc00\" | awk \'/Scope:Global/{print $3}\' | awk -F \"/\" \'{print $1}\'", BRG_DEV);

    ptr = strtok(buf, "\n");

    do
    {
        if(0 == strcasecmp(ptr, target_ip))
        {
            result = TRUE;
            break;
        }
    } while(NULL != (ptr = strtok(NULL, "\n")));

    return result;
}

/*****************************************************************
 * NAME:    check_mac_is_in_list
 * ---------------------------------------------------------------
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * Author:
 * Modify:
 ******************************************************************/
bool check_mac_is_in_list(char *target_mac, char mac_list[][32])
{
	bool result;
	int i;

	result = FALSE;

	for(i = 0; i < MAX_CLIENT_DEVICE && 0 != strlen(mac_list[i]); i++)
	{
		if(0 == strcasecmp(target_mac, mac_list[i]))
		{
			result = TRUE;
			break;
		}
	}

	return result;
}


/*****************************************************************
* NAME:    broadcast_to_mesh_device
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
void broadcast_to_mesh_device(HTTPS_CB *pkt)
{
    int i, count;
	int mesh24g_disabled=0,mesh5g_disabled=0;
    char buf_mac[32];
    char buf_payload[1024];
    char buf_mesh_list[4096];
    char buf_query_str[4096];
    char buf_result[4096];
    char buf_result_count_file[64];
    char buf_result_success_file[64];
    char buf_count[8];
	char ifname[6];
    char *ptr;
    char *query_str;
    char *return_str;
    MESH_DEVICE_SETTINGS_T setting[32];

    memset(buf_mac, 0x00, sizeof(buf_mac));
    memset(buf_payload, 0x00, sizeof(buf_payload));
    memset(buf_mesh_list, 0x00, sizeof(buf_mesh_list));
    memset(buf_query_str, 0x00, sizeof(buf_query_str));
    memset(setting, 0x00, sizeof(setting));
    memset(buf_result, 0x00, sizeof(buf_result));
    memset(buf_result_count_file, 0x00, sizeof(buf_result_count_file));
    memset(buf_result_success_file, 0x00, sizeof(buf_result_success_file));
	memset(ifname,0x0,sizeof(ifname));

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");
    disable_broadcast_in_query_str(query_str, buf_query_str);
    sysutil_check_string_format(buf_query_str, buf_payload, 5);

    sprintf(buf_result_count_file, MESH_BROADCAST_COUNT_FILE_FORMAT, pkt->json_action);
    sprintf(buf_result_success_file, MESH_BROADCAST_SUCCESS_FILE_FORMAT, pkt->json_action);

    sys_find_proc("alfred", &count);

    if(count > 0)
    {
        sysutil_interact(buf_mesh_list, sizeof(buf_mesh_list), "alfred -r %d", ALFRED_SYSTEM_INFORMATION);

        parse_json_mesh_device_list(buf_mesh_list, setting, &return_str);

		api_get_wifi_mesh_disabled_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, &mesh24g_disabled);
		api_get_wifi_mesh_disabled_option(WIRELESS_WIFI_5G_MESH_DISABLED_OPTION, &mesh5g_disabled);
		//mesh24g_disabled =0, get 2.4GHz ifname
		//mesh24g_disabled =1, get 5GHz ifname
		api_get_wifi_mesh_ifname_option((mesh24g_disabled)?1:0,ifname,sizeof(ifname));

		sysutil_interact(buf_mac, sizeof(buf_mac), "ifconfig %s | grep \"HWaddr\" | awk \'{print $5}\'",ifname);

        if(NULL != (ptr = strchr(buf_mac, '\n')))
        {
            *ptr = '\0';
        }

        if(sysutil_check_file_existed(buf_result_count_file))
        {
            SYSTEM("rm -f %s", buf_result_count_file);
        }

        if(sysutil_check_file_existed(buf_result_success_file))
        {
            SYSTEM("rm -f %s", buf_result_success_file);
        }

        for(i = 0; i < MAX_MESH_DEVICES; i++)
        {
            if(TRUE == setting[i].existed)
            {
                if(0 != strcasecmp(buf_mac, setting[i].mac))
                {
                    sysutil_interact(buf_result, sizeof(buf_result),
                            "app_client -i %s -m POST -a mesh/%s -e 1 -p \"%s\" &",
                            setting[i].ip_address, pkt->json_action, buf_payload);

                    /* Check the reply. If the reply is OK, then increase the success count. */
                    if(TRUE == check_app_client_reply(buf_result, pkt->json_action))
                    {
                        if(sysutil_check_file_existed(buf_result_count_file))
                        {
                            memset(buf_count, 0x00, sizeof(buf_count));
                            sysutil_interact(buf_count, sizeof(buf_count), "cat %s", buf_result_count_file);

                            count = atoi(buf_count);
                        }
                        else
                        {
                            count = 0;
                        }

                        SYSTEM("echo %d > %s", count + 1, buf_result_count_file);
                    }
                }
            }
            else
            {
                break;
            }
        }

        count = 0;

        if(sysutil_check_file_existed(buf_result_count_file))
        {
            memset(buf_count, 0x00, sizeof(buf_count));
            sysutil_interact(buf_count, sizeof(buf_count), "cat %s", buf_result_count_file);

            count = atoi(buf_count);

            SYSTEM("rm -f %s", buf_result_count_file);
        }

        /* Case 1 : Only one device in this MESH. */
        /* Case 2 : i contains all the MESH devices, including the sender itself. */
        if((1 == i) || (count == (i - 1)))
        {
            SYSTEM("touch %s", buf_result_success_file);
        }
    }

    return;
}

/*****************************************************************
* NAME:    redirect_to_target_mesh
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
void redirect_to_target_mesh(HTTPS_CB *pkt, char *target_ip)
{
    char buf_payload[4096];
    char buf_result[4096];
    char *query_str;

    memset(buf_payload, 0x00, sizeof(buf_payload));
    memset(buf_result, 0x00, sizeof(buf_result));

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");
    sysCheckStringOnApp(query_str, buf_payload);

    sysutil_interact(buf_result, sizeof(buf_result),
            "app_client -i %s -m POST -a mesh/%s -e 1 -p \"%s\"",
            target_ip, pkt->json_action, buf_payload);

    basic_json_response(pkt, buf_result);

    return;
}

/*****************************************************************
* NAME:    redirect_to_target_mesh_device_no_response
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
char *redirect_to_target_mesh_device_no_response(HTTPS_CB *pkt, char *target_ip)
{
    char buf_payload[65536];
    char buf_result[65536];
    char *replace_str=NULL;
    char *query_str;

    memset(buf_payload, 0x00, sizeof(buf_payload));
    memset(buf_result, 0x00, sizeof(buf_result));

    if (pkt->method == M_POST)
    {
        query_str = get_env(&pkt->envcfg, "QUERY_STRING");
        sysCheckStringOnWeb(query_str, buf_payload);
        replace_str = str_replace(buf_payload, "\n", "");

        sysutil_interact(buf_result, sizeof(buf_result),
            "app_client -i %s -m POST -a mesh/%s -e 1 -p \"%s\"",
            target_ip, pkt->json_action, replace_str);

        free(replace_str);
    }
    else
    {
        sysutil_interact(buf_result, sizeof(buf_result),
            "app_client -i %s -m GET -a mesh/%s -e 1",
            target_ip, pkt->json_action);
    }

    replace_str = strdup(buf_result);

    return replace_str;
}

/*****************************************************************
* NAME:    redirect_to_target
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
void redirect_to_target(HTTPS_CB *pkt, char *target_ip)
{
    char buf_payload[4096];
    char buf_result[4096];
    char *query_str;

    memset(buf_payload, 0x00, sizeof(buf_payload));
    memset(buf_result, 0x00, sizeof(buf_result));

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");
    sysCheckStringOnApp(query_str, buf_payload);

#if 0
    sysutil_interact(buf_result, sizeof(buf_result),
            "app_client -i %s -m POST -a %s -e 1 -p \"%s\"",
            target_ip, pkt->json_action, buf_payload);
#else
    APPAGENT_SYSTEM("app_client -i %s -m POST -a %s -e 1 -p \"%s\" &",
            target_ip, pkt->json_action, buf_payload);
#endif

    //basic_json_response(pkt, buf_result);

    return;
}

/*****************************************************************
* NAME:    set_mesh_network_profile_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
void set_mesh_setting(char *operating_mesh_band, WLAN_RADIOS_T *setting)
{
    if (!strcmp(operating_mesh_band,"2.4GHZ"))
    {
        if(setting->radio_enabled)
        {
            api_set_wifi_mesh_disabled_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, 0);
            api_set_wifi_mesh_disabled_option(WIRELESS_WIFI_5G_MESH_DISABLED_OPTION,1);
        }
        else
        {
            api_set_wifi_mesh_disabled_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, 1);
        }
        api_set_wifi_mesh_ssid_option(WIRELESS_WIFI_MESH_SSID_OPTION, setting->ssid, sizeof(setting->ssid));
        api_set_wifi_channel_option(WIRELESS_WIFI_CHANNEL_OPTION, setting->channel);
        //api_set_wifi_mesh_encryption_option(WIRELESS_WIFI_MESH_NAWDS_ENCR_OPTION, setting->type, sizeof(setting->type));
        api_set_string_option(WIRELESS_WIFI_MESH_ENCRYPTION_OPTION, setting->type, strlen(setting->type));
        api_set_wifi_mesh_wpa_key_option(WIRELESS_WIFI_MESH_AESKEY_OPTION, setting->key, sizeof(setting->key));
        api_set_wifi_opmode_option(WIRELESS_WIFI_OPMODE_OPTION, SYS_OPM_WDSAP);
        api_set_bool_option(ALFRED_WIFI_ALFRED_DISABLED_OPTION, 0);
    }
#if SUPPORT_WLAN_5G_SETTING
    else if (!strcmp(operating_mesh_band,"5GHZ"))
    {
        if(setting->radio_enabled)
        {
            api_set_wifi_mesh_disabled_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, 1);
            api_set_wifi_mesh_disabled_option(WIRELESS_WIFI_5G_MESH_DISABLED_OPTION,0);
        }
        else
        {
            api_set_wifi_mesh_disabled_option(WIRELESS_WIFI_5G_MESH_DISABLED_OPTION,1);
        }
        api_set_wifi_mesh_ssid_option(WIRELESS_WIFI_5G_MESH_SSID_OPTION, setting->ssid, sizeof(setting->ssid));
        api_set_wifi_5g_channel_option(WIRELESS_WIFI_5G_CHANNEL_OPTION, setting->channel);
        //api_set_wifi_mesh_encryption_option(WIRELESS_WIFI_5G_MESH_NAWDS_ENCR_OPTION, setting->type, sizeof(setting->type));
        api_set_string_option(WIRELESS_WIFI_5G_MESH_ENCRYPTION_OPTION, setting->type, strlen(setting->type));
        api_set_wifi_mesh_wpa_key_option(WIRELESS_WIFI_5G_MESH_AESKEY_OPTION, setting->key, sizeof(setting->key));
        api_set_wifi_opmode_option(WIRELESS_WIFI_5G_OPMODE_OPTION, SYS_OPM_WDSAP);
        api_set_bool_option(ALFRED_WIFI_5G_ALFRED_DISABLED_OPTION, 0);
    }
#endif
    else
    {
        api_set_wifi_mesh_disabled_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, 1);
        api_set_wifi_mesh_disabled_option(WIRELESS_WIFI_5G_MESH_DISABLED_OPTION, 1);
    }
}

/*****************************************************************
* NAME:    set_mesh_device_by_wizard_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_mesh_device_by_wizard_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	int isReboot = 0;
	int op_mode;
	char operating_mesh_band[16];
	char mesh_ssid[64];
	WLAN_RADIOS_T wlan_setting[2];
	WLAN_RADIOS_T mesh_setting;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = OK_STR;
	memset(&wlan_setting, 0x0, sizeof(wlan_setting));
	memset(&mesh_setting, 0x0, sizeof(mesh_setting));
	memset(mesh_ssid, 0x0, sizeof(mesh_ssid));

	if(NULL == query_str)
		goto send_pkt;

	if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
		goto send_pkt;
	}

	if(get_json_integer_from_query(query_str, &op_mode, "SystemOperationMode"))
	{
		return_str = ERROR_SYSTEM_OPERATION_MODE_STR;
		goto send_pkt;
	}

	//WirelessSettings
	get_json_string_from_query(query_str, operating_mesh_band, "OperatingMeshBand");
	parse_json_wireless_setting(query_str, "WirelessSettings", wlan_setting, &return_str);

	if(!((return_str != ERROR_BAD_BAND_STR) &&
		 (return_str != ERROR_BAD_SSID_STR) &&
		 (return_str != ERROR_ILLEGAL_KEY_VALUE_STR)))
	{
		goto send_pkt;
	}

	/* fill the default setting of WIZARD in wlan_setting to fulfil the requirement of set_wireless_setting() */
	wlan_setting[0].radio_id = WIRELESS_RADIO_2_4_GHZ;
	wlan_setting[0].ssid_id = 0;
	wlan_setting[0].radio_enabled = 1;
	wlan_setting[0].ssid_broadcast = 1;
	wlan_setting[0].channel_width = 0;
	wlan_setting[0].channel = 0;

	wlan_setting[0].authentication_enabled = 1;
	sprintf(wlan_setting[0].type, WLAN_WPA2_PSK_STR);
	sprintf(wlan_setting[0].encryption, WLAN_CIPHER_AES_STR);

	set_wireless_setting(&wlan_setting[0], &return_str, &isReboot);

#if SUPPORT_WLAN_5G_SETTING
	wlan_setting[1].radio_id = WIRELESS_RADIO_5_GHZ;
	wlan_setting[1].ssid_id = 0;
	wlan_setting[1].radio_enabled = 1;
	wlan_setting[1].ssid_broadcast = 1;
	wlan_setting[1].channel_width = 80; // 80MHz as default.
	wlan_setting[1].channel = 0;

	wlan_setting[1].authentication_enabled = 1;
	sprintf(wlan_setting[1].type, WLAN_WPA2_PSK_STR);
	sprintf(wlan_setting[1].encryption, WLAN_CIPHER_AES_STR);

	if(OK_STR == return_str)
	{
		set_wireless_setting(&wlan_setting[1], &return_str, &isReboot);
	}
#endif

	if(OK_STR == return_str)
	{
		if(0 == strcasecmp(WLAN_RADIO_2_4_GHZ_STR, operating_mesh_band))
		{
			memcpy(&mesh_setting, &wlan_setting[0], sizeof(WLAN_RADIOS_T));

			memset(mesh_setting.ssid, 0x00, sizeof(mesh_setting.ssid));
			snprintf(mesh_setting.ssid, 32 + 1, "Mesh_%s", wlan_setting[0].ssid);
			mesh_setting.channel = 1; // Set the first setting as default channel
		}
#if SUPPORT_WLAN_5G_SETTING
		else if(0 == strcasecmp(WLAN_RADIO_5_GHZ_STR, operating_mesh_band))
		{
			memcpy(&mesh_setting, &wlan_setting[1], sizeof(WLAN_RADIOS_T));

			memset(mesh_setting.ssid, 0x00, sizeof(mesh_setting.ssid));
			snprintf(mesh_setting.ssid, 32 + 1, "Mesh_%s", wlan_setting[1].ssid);
			mesh_setting.channel = 36; // Set the first setting as default channel
		}
#endif

		memset(mesh_setting.type, 0x00, sizeof(mesh_setting.type));
		strcpy(mesh_setting.type, "aes");

		set_mesh_setting(operating_mesh_band, &mesh_setting);
	}

	if(isReboot)
	{
		APPAGENT_SYSTEM("uci commit");
		APPAGENT_SYSTEM("reboot &");
	}
	else
	{
		APPAGENT_SYSTEM("luci-reload auto network &");
	}

send_pkt:
	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    set_mesh_network_profile_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_mesh_network_profile_cb(HTTPS_CB *pkt)
{
	bool result;
	bool matched;
	bool broadcast;
	int action_type;
	char *query_str;
	char *return_str;
	char operating_mesh_band[16];
	WLAN_RADIOS_T setting;
	char target_ip[32];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	result = FALSE;
	return_str = ERROR_STR;

	if(NULL == query_str)
		goto send_pkt;

	if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
		goto send_pkt;
	}

	if(get_json_integer_from_query(query_str, &action_type, "ActionType"))
	{
		return_str = ERROR_ACTION_TYPE_STR;
		goto send_pkt;
	}

	 switch(action_type)
	{
		case MESH_PROFILE_ACTION_DELETE:
		if(FALSE == is_mesh_function_enabled())
		{
			return_str = ERROR_MESH_DISABLED_STR;
			goto send_pkt;
		}
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshIP");

		/* Check if this packet is for this DUT. */
		matched = check_mesh_private_ip(target_ip);

		if(matched)
		{
			// mesh options reset to default.
			APPAGENT_SYSTEM("sh /etc/revert_mesh_options.sh wifi0_mesh");
			APPAGENT_SYSTEM("sh /etc/revert_mesh_options.sh wifi1_mesh");
			return_str = OK_STR;
		}
		else{
			/* Send to target MESH device */
			redirect_to_target_mesh(pkt, target_ip);
			return 0;
		}
		break;

		case MESH_PROFILE_ACTION_MODIFY:
		if(FALSE == is_mesh_function_enabled())
		{
			return_str = ERROR_MESH_DISABLED_STR;
			goto send_pkt;
		}
		get_json_boolean_from_query(query_str, &broadcast, "Broadcast");

		if(broadcast)
		{
			/* Send to all MESH devices. */
			broadcast_to_mesh_device(pkt);
		}

		case MESH_PROFILE_ACTION_SET:
		memset(operating_mesh_band, 0x0, sizeof(operating_mesh_band));
		get_json_string_from_query(query_str, operating_mesh_band, "OperatingMeshBand");

		parse_json_mesh_wireless_setting(query_str, "MeshWirelessSettings", &setting, &return_str);
		if((return_str != ERROR_BAD_BAND_STR) &&
			(return_str != ERROR_BAD_SSID_STR) &&
			(return_str != ERROR_BAD_CHANNEL_STR) &&
			(return_str != ERROR_TYPE_NOT_SUPPORTED_STR) &&
			(return_str != ERROR_ILLEGAL_KEY_VALUE_STR))
		{
			return_str = OK_STR;
			setting.radio_enabled = 1;
		}
		else
			 goto send_pkt;
		// The encryption of MESH only AES or none
		if (!strcmp(setting.type,"WPA2-PSK"))
		{
			memset(setting.type,0x0,sizeof(setting.type));
			strcpy(setting.type,"aes");
		}
		else if (!strcmp(setting.type,"NONE")){
			memset(setting.type,0x0,sizeof(setting.type));
			strcpy(setting.type,"none");

		}
		else{
			return_str = ERROR_TYPE_NOT_SUPPORTED_STR;
			goto send_pkt;
		}

		break;

		default:
		return_str = ERROR_ACTION_TYPE_STR;
		break;
	}

	if(action_type != MESH_PROFILE_ACTION_MODIFY){
		set_mesh_setting(operating_mesh_band, &setting);

		APPAGENT_SYSTEM("luci-reload auto network &");
	}
	else{
		/* If the action is Modify, then keep this setting until the ApplyMeshNetworkSetting is called.*/
		memset(&global_mesh_setting, 0x00, sizeof(APP_MESH_SETTING));
		global_mesh_setting.used = TRUE;
		sprintf(global_mesh_setting.operating_mesh_band, "%s", operating_mesh_band);
		memcpy(&(global_mesh_setting.setting), &setting, sizeof(WLAN_RADIOS_T));
	}
	/* Send response packet */
send_pkt:
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_broadcast_setting_status_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_broadcast_setting_status_cb(HTTPS_CB *pkt)
{
    char *query_str;
    char *return_str;
    char target_api[64];
    char target_api_count_file[64];
    char target_api_success_file[64];

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");
    return_str = ERROR_STR;

    if(FALSE == is_mesh_function_enabled())
    {
        return_str = ERROR_MESH_DISABLED_STR;
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        return_str = ERROR_UNAUTHORIZED_STR;
    }
    else
    {
        memset(target_api, 0x00, sizeof(target_api));
        memset(target_api_count_file, 0x00, sizeof(target_api_count_file));
        memset(target_api_success_file, 0x00, sizeof(target_api_success_file));

        get_json_string_from_query(query_str, target_api, "TargetAPI");

        if(0 != strlen(target_api))
        {
            sprintf(target_api_count_file, MESH_BROADCAST_COUNT_FILE_FORMAT, target_api);
            sprintf(target_api_success_file, MESH_BROADCAST_SUCCESS_FILE_FORMAT, target_api);

            if(sysutil_check_file_existed(target_api_success_file))
            {
                return_str = OK_STR;

                SYSTEM("rm -f %s", target_api_success_file);
            }
            else if(sysutil_check_file_existed(target_api_count_file))
            {
                return_str = ERROR_PROCESS_IS_RUNNING_STR;
            }
        }
    }

    send_simple_response(pkt, return_str);

    return 0;
}

#define MESH_ROOT_HC_PATH "/tmp/mesh_root_hc"
/*****************************************************************
* NAME:    get_mesh_node_info_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_node_info_cb(HTTPS_CB *pkt)
{
	int i, j;
	int mesh24g_disabled;
	int idle_time;
    int wifix = 0, mesh_num = 0;
    char ifname[16]={0};
	char *query_str;
	char *ptr;
	char buf[4096];
    char buf2[1024];
	char interface_name[32];
    char mesh_role[8]={0};
	char if_name[8];
	char client_list[128][32];
	MESH_DEVICE_SETTINGS_T setting;
	MESH_DEVICE_NEIGHBORS_T neighbor;
    char mesh_controller[8]={0};
    int no_mesh=0, bat_st=0, rp_st=0;
    char mastermac[128]={0};
    MESH_GUEST_CLIENT_INFO_T gClientInfo;
    int disabled;
#if HAS_WLAN_5G_2_SETTING
	int mesh5g_2_disabled=0;
#endif
#if HAS_SUPPORT_GUEST_NETWORK_BY_SSID
	char guest_status[16]={0};
#endif

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	ptr = NULL;

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(&setting, 0x00, sizeof(setting));
		memset(&neighbor, 0x00, sizeof(neighbor));
        memset(&gClientInfo, 0x00, sizeof(gClientInfo));
		memset(interface_name, 0x00, sizeof(interface_name));
		memset(if_name, 0x00, sizeof(if_name));
		memset(client_list, 0x00, sizeof(client_list));

/************************ Mesh_Group_info *************************/

//        api_get_mesh_disabled_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, &wifix);
        uci_get_wifix_mesh_disabled_option(0, &i);
        if(i == 0) wifix=0;
        uci_get_wifix_mesh_disabled_option(1, &i);
        if(i == 0) wifix=1;
        uci_get_wifix_mesh_disabled_option(2, &i);
        if(i == 0) wifix=2;
        api_get_wifi_mesh_ifname_option(wifix, ifname, sizeof(ifname));
        setting.mesh_channel = 0;
        uci_get_wifix_mesh_channel_option(wifix, &setting.mesh_channel);

        memset(buf, 0, sizeof(buf));
#if SUPPORT_BATMAN_2019
        sys_interact(buf, sizeof(buf), "batctl o -H | grep \"*\" |wc -l");
#else
        sys_interact(buf, sizeof(buf), "batctl o -H |wc -l");
#endif
        mesh_num = atoi(buf);

        /* ---------------mesh_robust----------------*/
        /* enable */
        memset(buf, 0, sizeof(buf));
        sys_interact(buf, sizeof(buf), "cat /proc/mesh_robust |awk {'printf $1'}");
        setting.enable = 0;
        setting.enable = (atoi(buf) == 1?true:false);

        /* threshold */
        memset(buf, 0, sizeof(buf));
        if (wifix == 0)
            sys_interact(buf, sizeof(buf), "cat /proc/mesh_robust |awk {'printf $2'}");
        else
            sys_interact(buf, sizeof(buf), "cat /proc/mesh_robust |awk {'printf $3'}");
        setting.threshold = atoi(buf);

        /* ---------------mesh_link----------------*/
        /* link_mac_addr */
        memset(buf, 0x00, sizeof(buf));
        sys_interact(buf, sizeof(buf), "batctl o |tail -n +3|awk {'print $1'}");
        i = 0;
        ptr = strtok(buf, "\n");
        while(ptr != NULL)
        {
            sscanf(ptr, "%s", setting.link_mac_addr[i]);
            i++;
            ptr = strtok(NULL, "\n");
        }

        /* tq_val */
        memset(buf, 0x00, sizeof(buf));
#if SUPPORT_BATMAN_2019
        sys_interact(buf, sizeof(buf), "batctl o |tail -n +3 |grep \"*\" |awk -F '(' '{print $2}'|awk -F ')' '{print $1}'");
#else
        sys_interact(buf, sizeof(buf), "batctl o |tail -n +3 |awk -F '(' '{print $2}'|awk -F ')' '{print $1}'");
#endif
        i = 0;
        ptr = strtok(buf, "\n");
        while(ptr != NULL)
        {
            sscanf(ptr, "%d", &setting.tq_val[i]);
            i++;
            ptr = strtok(NULL, "\n");
        }

        /* tx_datarate ＆ rx_datarate*/
        memset(buf, 0x00, sizeof(buf));
        sys_interact(buf, sizeof(buf), "wlanconfig %s list |tail -n +2", ifname);
        i = 0;
        ptr = strtok(buf, "\n");
        while(ptr != NULL)
        {
            sscanf(ptr, "%*s %*s %*s %s %s", setting.tx_rate[i], setting.rx_rate[i]);
            i++;
            ptr = strtok(NULL, "\n");
        }

        /* gw_mac_addr */
#if SUPPORT_BATMAN_2019
        sys_interact(setting.gw_mac_addr, sizeof(setting.gw_mac_addr), "batctl gwl | grep \"=>\|^*\" | awk -F" " '{ print $2 }' | tr -d '\n'");
#else
        sys_interact(setting.gw_mac_addr, sizeof(setting.gw_mac_addr), "batctl gwl |grep = |awk {'printf $2'} | tr -d '\n'");
#endif

/*********************** Mesh_Group_info End***********************/

		/* Device type : 1 - Router, 2 - AP-Server, 3 - AP-client, 4 - AP-Camera */
		api_get_string_option(SYSTEM_SYSTEM_SECTION".opmode", buf, sizeof(buf));

		if(0 == strcmp(buf, "ar"))
		{
			setting.device_type = 1;
		}
		else
		{
			memset(buf, 0x00, sizeof(buf));
			api_get_string_option("mesh.wifi.role", buf, sizeof(buf));

			if(0 == strlen(buf))
			{
				sprintf(buf, "client");
			}

			if(0 == strcmp(buf, "server"))
			{
				setting.device_type = 2;
			}
			else
			{
				setting.device_type = 3;
			}
#if HAS_IPCAM
			setting.device_type = 4;
#endif
		}

		sysutil_interact(setting.lan_ip_address, sizeof(setting.lan_ip_address),
		        "ifconfig br-lan | awk \'/inet addr/{print substr($2,6)}\'");

		ptr = strchr(setting.lan_ip_address, '\n');
		if(NULL != ptr)
		{
			*ptr = '\0';
		}
        api_get_string_option("sysProductInfo.model.modelName", setting.product_name, sizeof(setting.product_name));
        //In order to make mac address information of bottom label consistent,EMR3500 shows WAN mac address.
        if((0 == strcmp(setting.product_name,"EMR3500")) || (0 == strcmp(setting.product_name,"ESR530")) )
        {
            sysutil_interact(setting.lan_mac, sizeof(setting.lan_mac), "ifconfig %s | awk \'/HWaddr/{print $5}\' | tr -d \"\n\"","eth0");
        }
        else
        {
            sysutil_interact(setting.lan_mac, sizeof(setting.lan_mac), "ifconfig %s | awk \'/HWaddr/{print $5}\' | tr -d \"\n\"","br-lan");
        }
        //for no ether device
        if(17 != strlen(setting.lan_mac))
		{
            sysutil_interact(setting.lan_mac, sizeof(setting.lan_mac), "ifconfig %s | awk \'/HWaddr/{print $5}\' | tr -d \"\n\"","br-lan");
		}
		sysutil_interact(setting.true_wan_mac, sizeof(setting.true_wan_mac), "ifconfig %s | awk \'/HWaddr/{print $5}\' | tr -d \"\n\"","eth0");
        //for no ether device
        if(17 != strlen(setting.true_wan_mac))
		{
            sysutil_interact(setting.true_wan_mac, sizeof(setting.true_wan_mac), "ifconfig %s | awk \'/HWaddr/{print $5}\' | tr -d \"\n\"","br-lan");
		}
        sysutil_interact(buf, sizeof(buf), "batctl o 2>/dev/null | wc -l");
        bat_st=atoi(strtok(buf, "\n"));
        if (bat_st>1)
        {
            setting.bat_st=1;
        }

        api_get_string_option("mesh.wifi.supportRP", buf, sizeof(buf));
        if(atoi(buf) == 1)
        {
            if(strcmp(ifname, "ath32") == 0)
            {
                sysutil_interact(setting.rpap_mac, sizeof(setting.rpap_mac), "ifconfig %s | awk \'/HWaddr/{print $5}\'| tr -d '\n'","ath28");
                {
                    setting.rp_st=0;
                }
            }
            if(strcmp(ifname, "ath35") == 0)
            {
                sysutil_interact(setting.rpap_mac, sizeof(setting.rpap_mac), "ifconfig %s | awk \'/HWaddr/{print $5}\'| tr -d '\n'","ath58");
                {
                    setting.rp_st=1;
                }
            }
            if(strcmp(ifname, "ath37") == 0)
            {
                sysutil_interact(setting.rpap_mac, sizeof(setting.rpap_mac), "ifconfig %s | awk \'/HWaddr/{print $5}\'| tr -d '\n'","ath78");
                {
                    setting.rp_st=2;
                }
            }
        }
        else
        {
            sysutil_interact(interface_name, sizeof(interface_name), "getinfo mesh_ifname | tr -d '\n'");
            if(0 == strcmp(interface_name, "ath35"))
            {
                sysutil_interact(setting.rpap_mac, sizeof(setting.rpap_mac), "ifconfig %s | awk \'/HWaddr/{print $5}\'| tr -d '\n'","ath58");

                sysutil_interact(buf, sizeof(buf), "iwconfig %s 2>/dev/null | grep 'Access Point' | awk -F \" \" '{printf $6}'","ath59");
                if(0 != strcmp(buf, "Not-Associated"))
                {
                    setting.rp_st=1;
                }
            }
            else
            {
                sysutil_interact(setting.rpap_mac, sizeof(setting.rpap_mac), "ifconfig %s | awk \'/HWaddr/{print $5}\'| tr -d '\n'","ath68");

                sysutil_interact(buf, sizeof(buf), "iwconfig %s 2>/dev/null | grep 'Access Point' | awk -F \" \" '{printf $6}'","ath69");
                if(0 != strcmp(buf, "Not-Associated"))
                {
                    setting.rp_st=1;
                }
            }
        }

	uci_get_mesh_device_name_option(setting.device_name, sizeof(setting.device_name));
	uci_get_mesh_location_option(setting.location_name, sizeof(setting.location_name));
	sysutil_interact(mesh_role, sizeof(mesh_role), "batctl gw | awk \'{print $1}\'");
        mesh_role[strlen(mesh_role)-1] = '\0';
        sprintf(setting.mesh_role, mesh_role);

#if SUPPORT_AP_MESH_PROJECT
        if (strcmp(setting.mesh_role, "server") != 0)
        {
            sprintf(mesh_controller, "slave");
        }
        sprintf(setting.mesh_controller, mesh_controller);
#else //SUPPORT_ROUTER_MESH_PROJECT
        if(api_get_string_option("mesh.wifi.controller", mesh_controller, sizeof(mesh_controller)))
        {
            sprintf(mesh_controller, "slave");
        }
        sprintf(setting.mesh_controller, mesh_controller);
#endif

		api_get_wifi_mesh_disabled_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, &mesh24g_disabled);
		//mesh24g_disabled =0, get 2.4GHz interface name
		//mesh24g_disabled =1, get 5GHz interface name
		api_get_wifi_mesh_ifname_option((mesh24g_disabled)?1:0, interface_name, sizeof(interface_name));
#if HAS_WLAN_5G_2_SETTING
//		api_get_wifi_mesh_disabled_option("wireless.wifi2_mesh.disabled", &mesh5g_2_disabled);
//		if(mesh5g_2_disabled==0)
//		{
//			api_get_string_option("wireless.wifi2_mesh.ifname", interface_name, sizeof(interface_name));
//		}
		sysutil_interact(interface_name, sizeof(interface_name), "getinfo mesh_ifname | tr -d '\n'");
#endif

		sysutil_interact(setting.mac, sizeof(setting.mac),
		        "ifconfig %s | awk \'/HWaddr/{print $5}\'",interface_name);

        setting.root_hop_count = -1;
        if (sysutil_check_file_existed(MESH_ROOT_HC_PATH))
        {
            int _len = 0;
            memset(buf, 0x00, sizeof(buf));
    		sysutil_interact(buf, sizeof(buf), "cat "MESH_ROOT_HC_PATH);
            _len = strlen(buf);
            if (_len != 0)
            {
                buf[_len] = '\0';
                setting.root_hop_count = atoi(buf);
            }
        }

		ptr = strchr(setting.mac, '\n');
		if(NULL != ptr)
		{
			*ptr = '\0';
		}

		sysutil_interact(setting.uid, sizeof(setting.uid), "setconfig -g 35");

		ptr = strchr(setting.uid, '\n');
		if(NULL != ptr)
		{
			*ptr = '\0';
		}

        //get next hop's mac
#if SUPPORT_BATMAN_2019
        sysutil_interact(buf, sizeof(buf), "batctl o -H | grep \"*\" | grep \"No batman *\" | wc -l | tr -d '\n'");
#else
        sysutil_interact(buf, sizeof(buf), "batctl o -H |grep \"No batman *\" | wc -l | tr -d '\n'");
#endif
        no_mesh=atoi(buf);

        if (no_mesh==0) //if has mesh devices
        {
#if SUPPORT_AP_MESH_PROJECT
            sprintf(mesh_controller, setting.mesh_role);
#else //SUPPORT_ROUTER_MESH_PROJECT
            api_get_string_option("mesh.wifi.controller", mesh_controller, sizeof(mesh_controller));
#endif

            if(0 == strlen(mesh_controller))
            {
                api_get_string_option("mesh.wifi.role", mesh_controller, sizeof(mesh_controller));
            }
            if(0 == strcmp(mesh_controller, "server") || 0 == strcmp(mesh_controller, "master")) //if self is master, set next hop's rssi = 100
            {
                setting.next_hop_rssi = 100;
                setting.rp_st=1;
            }
            else
            {
#if SUPPORT_BATMAN_2019
                sysutil_interact(mastermac, sizeof(mastermac), "batctl gwl -H | grep \"=>\|^*\" | awk '{print $2}' | grep -E \"^[a-f0-9:]{17}$\""); //get master's mac
#else
                sysutil_interact(mastermac, sizeof(mastermac), "batctl gwl -H | grep \"=>\" | awk '{print $2}' | grep -E \"^[a-f0-9:]{17}$\""); //get master's mac
#endif
                mastermac[strlen(mastermac)-1]=0;
                
                if(strlen(mastermac)==0)
                {
                    if(sysutil_check_file_existed("/tmp/masterMac"))
                    {
                        sysutil_interact(mastermac, sizeof(mastermac), "cat /tmp/masterMac | awk '{printf tolower($0)}'");
                    }
                }

                //sysutil_interact(setting.next_hop_mac, sizeof(setting.next_hop_mac),"batctl tr %s  | awk 'NR>1'| grep '1:'| awk '{print $2}'| tr -d \"\n\"",mastermac); //get next hop's mac to gw.

				if(sysutil_check_file_existed("/tmp/nextHopAPMAC"))
                {
                    sysutil_interact(setting.next_hop_mac, sizeof(setting.next_hop_mac), "cat /tmp/nextHopAPMAC | awk -F , '{print $2}'| tr -d \"\n\"");
                }
                else
                {
#if SUPPORT_BATMAN_2019
                    sysutil_interact(setting.next_hop_mac, sizeof(setting.next_hop_mac), "batctl o -H | grep \"* %s\" |  awk -F ')' '{print $2}' | awk {'print $1'}| grep -E \"^[a-f0-9:]{17}$\" | tr -d \"\n\"",mastermac);
#else
                    sysutil_interact(setting.next_hop_mac, sizeof(setting.next_hop_mac), "batctl o | grep ^%s |awk '{print $4}' | grep -E \"^[a-f0-9:]{17}$\" | tr -d \"\n\"",mastermac);
#endif
                    if(strlen(setting.next_hop_mac)==0) //if TQ is <= 99
                    {
                        sysutil_interact(setting.next_hop_mac, sizeof(setting.next_hop_mac), "batctl o | grep ^%s |awk '{print $5}' | grep -E \"^[a-f0-9:]{17}$\" | tr -d \"\n\"",mastermac);
                    }
                    //printf("-------------------FUNCTION:%s-------LINE:%d-------next_hop_mac:%s-----------\n", __FUNCTION__, __LINE__, setting.next_hop_mac);
                }

                memset(buf, 0x00, sizeof(buf));
                if(sysutil_check_file_existed("/tmp/current_color"))
                {
                    char current_color[100];
                    char nexthop_rssi[100];
                    char myCurHopMAC[100];
                    sysutil_interact(current_color, sizeof(current_color), "cat /tmp/current_color | tr -d \"\n\"");
                    sysutil_interact(myCurHopMAC, sizeof(myCurHopMAC), "cat /tmp/myCurHopMAC | tr -d \"\n\"");
                    sysutil_interact(nexthop_rssi, sizeof(nexthop_rssi),"wlanconfig %s nawds learning | grep %s| awk '{print $3}'| tr -d \"\n\"", interface_name, myCurHopMAC); //get next hop's rssi to gw.
                    if(0 == strcmp(current_color,"white"))
                    {
                        if((0 == strcmp(setting.product_name,"EMR5000")) || (0 == strcmp(setting.product_name,"ESR580")) )
                        {
                            if(atoi(nexthop_rssi) > 35)
                            {
                                sprintf(buf,"50");//Too close
                            }
                            else
                            {
                                sprintf(buf,"30");//Optimum range
                            }
                        }
                        else
                        {
                            if(atoi(nexthop_rssi) > 45)
                            {
                                sprintf(buf,"50");//Too close
                            }
                            else
                            {
                                sprintf(buf,"30");//Optimum range
                            }
                        }
                    }
                    if(0 == strcmp(current_color,"orange"))
                    {
                        sprintf(buf,"5");//Too far
                    }
                    setting.next_hop_true_rssi = atoi(nexthop_rssi);
                }
                else
                {
                    sysutil_interact(buf, sizeof(buf),"wlanconfig %s nawds learning | grep %s| awk '{print $3}'| tr -d \"\n\"", interface_name, setting.next_hop_mac); //get next hop's rssi to gw.
                }
                setting.next_hop_rssi = atoi(buf);
                //printf("-------------------FUNCTION:%s-------LINE:%d-------next_hop_rssi:%d-----------\n", __FUNCTION__, __LINE__, setting.next_hop_rssi);
            }
        }


        /* nexthop_tq_val */
        memset(buf, 0x00, sizeof(buf));
        sys_interact(buf, sizeof(buf), "batctl o|awk -F ') ' {'print $1'}|grep %s|awk -F '(' '{print $2}'| tr -d \"\n\"", setting.next_hop_mac);
        setting.nexthop_tq_val = atoi(buf);


		memset(buf, 0x00, sizeof(buf));
		sysutil_interact(buf, sizeof(buf),
		        "wlanconfig %s nawds learning | tail -32 | grep -v \"00:00:00:00:00:00\"", interface_name);

		i = 0;
		ptr = strtok(buf, "\n");

		while(ptr != NULL)
		{
			sscanf(ptr, "%*s %s %d [%[^]]\n", neighbor.mac[i], &neighbor.rssi[i], neighbor.flag[i]);

			i++;
			ptr = strtok(NULL, "\n");
		}

		memset(buf, 0x00, sizeof(buf));
		api_get_string_option("wireless.wifi0_ssid_1.ifname", if_name, sizeof(if_name));
		sysutil_interact(buf, sizeof(buf), "wlanconfig %s list  | grep -v ADDR | awk \'{print $1, $9}\'", if_name);

		i = 0;
		ptr = strtok(buf, "\n");

		while(ptr != NULL)
		{
			sscanf(ptr, "%s %d\n", client_list[i], &idle_time);

			if(idle_time > 60)
			{
				memset(client_list[i], 0x00, sizeof(client_list[0]));
				ptr = strtok(NULL, "\n");

				continue;
			}

			i++;
			ptr = strtok(NULL, "\n");
		}

		memset(if_name, 0x00, sizeof(if_name));
		memset(buf, 0x00, sizeof(buf));
		api_get_string_option("wireless.wifi1_ssid_1.ifname", if_name, sizeof(if_name));
		sysutil_interact(buf, sizeof(buf), "wlanconfig %s list  | grep -v ADDR | awk \'{print $1, $9}\'", if_name);

		ptr = strtok(buf, "\n");

		while(ptr != NULL)
		{
			sscanf(ptr, "%s %d\n", client_list[i], &idle_time);

			if(idle_time > 60)
			{
				memset(client_list[i], 0x00, sizeof(client_list[0]));
				ptr = strtok(NULL, "\n");

				continue;
			}

			i++;
			ptr = strtok(NULL, "\n");
		}

#if HAS_WLAN_5G_2_SETTING
		memset(if_name, 0x00, sizeof(if_name));
		memset(buf, 0x00, sizeof(buf));
		api_get_string_option("wireless.wifi2_ssid_1.ifname", if_name, sizeof(if_name));
		sysutil_interact(buf, sizeof(buf), "wlanconfig %s list  | grep -v ADDR | awk \'{print $1, $9}\'", if_name);

		ptr = strtok(buf, "\n");

		while(ptr != NULL)
		{
			sscanf(ptr, "%s %d\n", client_list[i], &idle_time);

			if(idle_time > 60)
			{
				memset(client_list[i], 0x00, sizeof(client_list[0]));
				ptr = strtok(NULL, "\n");

				continue;
			}

			i++;
			ptr = strtok(NULL, "\n");
		}

		//get guest 5G-2 client's info
		memset(if_name, 0x00, sizeof(if_name));
		memset(buf, 0x00, sizeof(buf));

		api_get_integer_option("wireless.wifi2_guest.disabled", &disabled);

		if(!disabled)
		{
			api_get_string_option("wireless.wifi2_guest.ifname", if_name, sizeof(if_name));
			sysutil_interact(buf, sizeof(buf),"wlanconfig %s list|awk 'NR>1 {print $1}'", if_name);
		}

		i = 0;
		ptr = strtok(buf, "\n");

		while(NULL != ptr)
		{
			memset(buf2, 0x00, sizeof(buf2));
			sysutil_interact(buf2, sizeof(buf2),"cat /tmp/dhcp.leases | grep %s", ptr);
			sscanf(buf2, "%*s %s %s %s %*[^\n]\n", gClientInfo.guest5_2_mac[i], gClientInfo.guest5_2_ip[i], gClientInfo.guest5_2_hostname[i]);

			i++;
			ptr = strtok(NULL, "\n");
		}
#endif

//get guest 5G client's info
        memset(if_name, 0x00, sizeof(if_name));
        memset(buf, 0x00, sizeof(buf));

#if HAS_SUPPORT_GUEST_NETWORK_BY_SSID
		memset(guest_status, 0x00, sizeof(guest_status));
		api_get_string_option("wireless.wifi1_ssid_2.guest_network", guest_status, sizeof(guest_status));
		if (strcmp(guest_status, "Enable")==0)
		{
			disabled=0;
		}
		else
		{
			disabled=1;
		}
#else
        api_get_integer_option("wireless.wifi1_guest.disabled", &disabled);
#endif

        if(!disabled)
        {
#if HAS_SUPPORT_GUEST_NETWORK_BY_SSID
			api_get_string_option("wireless.wifi1_ssid_2.ifname", if_name, sizeof(if_name));
#else
            api_get_string_option("wireless.wifi1_guest.ifname", if_name, sizeof(if_name));
#endif
            sysutil_interact(buf, sizeof(buf),"wlanconfig %s list|awk 'NR>1 {print $1}'", if_name);
        }

        i = 0;
        ptr = strtok(buf, "\n");

        while(ptr != NULL)
        {
            sysutil_interact(buf2, sizeof(buf2),"cat /tmp/dhcp.leases | grep %s", ptr);
            sscanf(buf2, "%*s %s %s %s %*[^\n]\n", gClientInfo.guest5_mac[i], gClientInfo.guest5_ip[i], gClientInfo.guest5_hostname[i]);

            i++;
            ptr = strtok(NULL, "\n");
        }
//get guest 24G client's info

        memset(if_name, 0x00, sizeof(if_name));
        memset(buf, 0x00, sizeof(buf));

#if HAS_SUPPORT_GUEST_NETWORK_BY_SSID
		memset(guest_status, 0x00, sizeof(guest_status));
		api_get_string_option("wireless.wifi0_ssid_2.guest_network", guest_status, sizeof(guest_status));
		if (strcmp(guest_status, "Enable")==0)
		{
			disabled=0;
		}
		else
		{
			disabled=1;
		}
#else
        api_get_integer_option("wireless.wifi0_guest.disabled", &disabled);
#endif
        
        if(!disabled)
        {
#if HAS_SUPPORT_GUEST_NETWORK_BY_SSID
			api_get_string_option("wireless.wifi0_ssid_2.ifname", if_name, sizeof(if_name));
#else
            api_get_string_option("wireless.wifi0_guest.ifname", if_name, sizeof(if_name));
#endif
            sysutil_interact(buf, sizeof(buf),"wlanconfig %s list|awk 'NR>1 {print $1}'", if_name);
        }
        i = 0;
        ptr = strtok(buf, "\n");

        while(ptr != NULL)
        {
            sysutil_interact(buf2, sizeof(buf2),"cat /tmp/dhcp.leases | grep %s", ptr);
            sscanf(buf2, "%*s %s %s %s %*[^\n]\n", gClientInfo.guest24_mac[i], gClientInfo.guest24_ip[i], gClientInfo.guest24_hostname[i]);

            i++;
            ptr = strtok(NULL, "\n");
        }
        /*For update roomlist infomation for wired connection*/
        APPAGENT_SYSTEM("[ -e \"/tmp/update_roomlist\" ] && rm -rf /tmp/update_roomlist");
		get_json_mesh_node_info_cb(pkt, setting, neighbor, gClientInfo, client_list, OK_STR);
	}

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_specific_option_setting_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_specific_option_setting_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char option_name[128];
	char value[128];

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
        memset(option_name, 0x00, sizeof(option_name));
		get_json_string_from_query(query_str, option_name, "OptionName");

		if(0 < strlen(option_name))
		{
			memset(value, 0x00, sizeof(value));

			api_get_string_option(option_name, value, sizeof(value));

			get_json_mesh_specific_option_setting_cb(pkt, value, OK_STR);
		}
		else
		{
			send_simple_response(pkt, ERROR_STR);
		}
	}

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_device_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_device_list_cb(HTTPS_CB *pkt)
{
	int count;
	char *query_str;
	char *return_str;
	char buf[4096];
	MESH_DEVICE_SETTINGS_T setting[32];

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = OK_STR;

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		sys_find_proc("alfred", &count);

		if(count > 0)
		{
			memset(buf, 0x00, sizeof(buf));
			memset(setting, 0x00, sizeof(setting));

			sys_interact(buf, sizeof(buf), "alfred -r %d", ALFRED_SYSTEM_INFORMATION);

			parse_json_mesh_device_list(buf, setting, &return_str);

			return_str = OK_STR;

			get_json_mesh_device_list_cb(pkt, setting, return_str);
		}
		else
		{
			send_simple_response(pkt, ERROR_STR);
		}
	}

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_device_client_list_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_device_client_list_cb(HTTPS_CB *pkt)
{
	int count;
	char *query_str;
	char buf[1024*32];
	MESH_DEVICE_MAC_INFO_T mac_info[MAX_MESH_DEVICES];
	MESH_DEVICE_CLIENT_MAC_LIST_T mac_client_list[MAX_MESH_DEVICES];

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(buf, 0x00, sizeof(buf));
		memset(mac_info, 0x00, sizeof(mac_info));
		memset(mac_client_list, 0x00, sizeof(mac_client_list));

		sys_find_proc("alfred", &count);

		if(count > 0)
		{
			sys_interact(buf, sizeof(buf), "alfred -r %d", ALFRED_INTERFACE_INFORMATION);

			if(parse_json_mesh_device_mac_info_cb(buf, mac_info))
			{
				memset(buf, 0x00, sizeof(buf));

				sys_interact(buf, sizeof(buf), "batadv-vis -f jsondoc");

				if(parse_json_mesh_device_client_mac_list_cb(buf, mac_info, mac_client_list))
				{
					get_json_mesh_device_client_list_cb(pkt, mac_client_list, OK_STR);
				}
			}
		}
		else
		{
			send_simple_response(pkt, ERROR_STR);
		}
	}

	return 0;
}

/*****************************************************************
* NAME:    invite_new_mesh_device_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int invite_new_mesh_device_cb(HTTPS_CB *pkt)
{
	int  disabled;
	int  broadcast;
	int  radio;
	char *query_str;
	char *return_str;

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(NULL == query_str)
	{
		send_simple_response(pkt, return_str);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		radio = -1;

        disabled = 0;
		api_get_wifi_mesh_disabled_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, &disabled);
		if(0 == disabled)
		{
			radio = 0;
		}

		api_get_wifi_mesh_disabled_option(WIRELESS_WIFI_5G_MESH_DISABLED_OPTION, &disabled);
		if(0 == disabled)
		{
			radio = 1;
		}

		if(-1 != radio)
		{
			get_json_boolean_from_query(query_str, &broadcast, "Broadcast");

			if(broadcast)
			{
				/* Send to all MESH devices. */
				broadcast_to_mesh_device(pkt);
			}

			/* TODO : Command to broadcast invite packets. */
			return_str = OK_STR;
		}

	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    set_mesh_device_name_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_mesh_device_name_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char target_ip[32];
	char device_name[32];

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshIP");

		if(check_mesh_private_ip(target_ip))
		{
			get_json_string_from_query(query_str, device_name, "MeshDeviceName");

			api_set_wifi_mesh_device_name_option(WIRELESS_WIFI_MESH_MESHDEVICENAME_OPTION, device_name);
			api_set_wifi_mesh_device_name_option(WIRELESS_WIFI_5G_MESH_MESHDEVICENAME_OPTION, device_name);

			send_simple_response(pkt, OK_STR);

			APPAGENT_SYSTEM("luci-reload auto network &");
		}
		else
		{
			redirect_to_target_mesh(pkt, target_ip);
		}
	}

	return 0;
}
/*****************************************************************
* NAME:    apply_mesh_network_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int apply_mesh_network_settings_cb(HTTPS_CB *pkt)
{
	bool broadcast;
	char *query_str;
	char *return_str;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(NULL == query_str)
	{
		return_str = ERROR_STR;
	}
	else if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		get_json_boolean_from_query(query_str, &broadcast, "Broadcast");

		if(broadcast)
		{
			/* Send to all MESH devices. */
			broadcast_to_mesh_device(pkt);
		}

		if(TRUE == global_mesh_setting.used)
		{
			set_mesh_setting(global_mesh_setting.operating_mesh_band, &global_mesh_setting.setting);
		}

		APPAGENT_SYSTEM("luci-reload auto network &");

		return_str = OK_STR;
	}

	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_wlan_settings
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool get_mesh_wlan_settings(MESH_WIRELESS_SETTINGS_T *setting, char **result)
{
	int mesh_band,encryption,mesh24g_disabled=0,mesh5g_disabled=0;
	char *ptr,buf_mac[32],ifname[6],encry_type[10],encry_key[64];
	char *device_name_option=NULL, *ssid_option=NULL, *channel_option=NULL, *enc_option=NULL, *key_option=NULL;

	if(NULL == setting)
		return FALSE;

	setting->enabled = is_mesh_function_enabled();
	api_get_wifi_mesh_disabled_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, &mesh24g_disabled);
	api_get_wifi_mesh_disabled_option(WIRELESS_WIFI_5G_MESH_DISABLED_OPTION, &mesh5g_disabled);

	setting->configured=0;

	//mesh24g_disabled =0, get 2.4GHz
	//mesh24g_disabled =1, get 5GHz
	if (!mesh24g_disabled){
		sprintf(setting->operation_band,"2.4GHZ");
		device_name_option = WIRELESS_WIFI_MESH_MESHDEVICENAME_OPTION;
		ssid_option = WIRELESS_WIFI_MESH_SSID_OPTION;
		channel_option = WIRELESS_WIFI_CHANNEL_OPTION;
		enc_option = WIRELESS_WIFI_MESH_ENCRYPTION_OPTION;
		key_option = WIRELESS_WIFI_MESH_AESKEY_OPTION;
	}
#if SUPPORT_WLAN_5G_SETTING
	else{
		sprintf(setting->operation_band,"5GHZ");
		device_name_option = WIRELESS_WIFI_5G_MESH_MESHDEVICENAME_OPTION;
		ssid_option = WIRELESS_WIFI_5G_MESH_SSID_OPTION;
		channel_option = WIRELESS_WIFI_5G_CHANNEL_OPTION;
		enc_option = WIRELESS_WIFI_5G_MESH_ENCRYPTION_OPTION;
		key_option = WIRELESS_WIFI_5G_MESH_AESKEY_OPTION;
	}
#endif

	api_get_wifi_iface_ssid_option(device_name_option,setting->device_name, sizeof(setting->device_name));

	api_get_wifi_iface_ssid_option(ssid_option,setting->ssid, sizeof(setting->ssid));

	api_get_wifi_channel_option(channel_option,&(setting->channel));

	memset(ifname,0x0,sizeof(ifname));
	api_get_wifi_mesh_ifname_option((mesh24g_disabled)?1:0,ifname,sizeof(ifname));
	sysutil_interact(buf_mac, sizeof(buf_mac), "ifconfig %s | awk \'/HWaddr/{print $5}\'",ifname);
	if(NULL != (ptr = strchr(buf_mac, '\n')))
		*ptr = '\0';
	sprintf(setting->mac, "%s", buf_mac);

	memset(encry_type,0x0,sizeof(encry_type));
	api_get_string_option(enc_option,encry_type,sizeof(encry_type));
	if(!strcmp(encry_type,"aes")){
		sprintf(setting->type,"WPA2-PSK");

		memset(encry_key,0x0,sizeof(encry_key));
		api_get_string_option(key_option,encry_key,sizeof(encry_key));
		sprintf(setting->key, "%s", encry_key);
	}
	else
		sprintf(setting->type,"NONE");

	return TRUE;
}

/*****************************************************************
* NAME:    get_mesh_device_wireless_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_device_wireless_settings_cb(HTTPS_CB *pkt)
{
	bool matched=FALSE;
	char *query_str;
	char *return_str;
    char target_ip[32];
	char op_mesh_band[16];
    WLAN_RADIO_SETTINGS_T setting[2];
    WLAN_RADIO_SECURITY_T security[2];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
    matched = FALSE;
    return_str = NULL;

	if(NULL == query_str){
		send_simple_response(pkt, ERROR_STR);
        	return -1;
	}

	if(FALSE == is_mesh_function_enabled())
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	else if(FALSE == check_mesh_account(query_str))
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshIP");
		matched = check_mesh_private_ip(target_ip) || (0 == strcmp(target_ip, "127.0.0.1"));

        if(matched)
        {
            memset(&setting, 0x00, sizeof(setting));
	    // only show the first set of SSID
            setting[0].radio_id = 0;
            setting[0].ssid_id = 0;
	    	get_wlan_options(&setting[0],&return_str);

#if SUPPORT_WLAN_5G_SETTING
			if(NULL == return_str)
            {	// only show the first set of SSID
				setting[1].radio_id = 1;
				setting[1].ssid_id = 0;
                get_wlan_options(&setting[1],&return_str);
            }
#endif
			memset(&security, 0, sizeof(security));
            security[0].radio_id = 0;
            security[0].ssid_id = 0;
            get_wlan_security_options(&security[0], &return_str);
#if SUPPORT_WLAN_5G_SETTING
			security[1].radio_id = 1;
			security[1].ssid_id = 0;
			get_wlan_security_options(&security[1], &return_str);
#endif
            if(NULL != return_str)
                send_simple_response(pkt, return_str);
            else
            {
                return_str = OK_STR;
				get_json_mesh_device_wireless_settings_cb(pkt, op_mesh_band, setting, security, return_str);
            }
        }
        else
            redirect_to_target_mesh(pkt, target_ip);
    }

    return 0;
}

/*****************************************************************
* NAME:    set_mesh_device_wireless_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_mesh_device_wireless_settings_cb(HTTPS_CB *pkt)
{
	bool result;
	char *query_str;
	char *return_str;
	char target_ip[32];
	bool matched;
	int  isReboot;
	WLAN_RADIOS_T setting[2];

	if(NULL == pkt)
		return -1;

	result = FALSE;
	return_str = ERROR_STR;
	matched = FALSE;
	isReboot = 0;
	memset(&setting, 0, sizeof(setting));
	memset(target_ip, 0x00, sizeof(target_ip));

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return 0;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
		return 0;
	}

	if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
		return 0;
	}

	/* Change the wireless settings if the target IP is the same as the device's IP. */
	get_json_string_from_query(query_str, target_ip, "TargetMeshIP");

	matched = check_mesh_private_ip(target_ip);

	result = parse_json_wireless_setting(query_str, "WirelessSettings", setting, &return_str);

	if(matched & result)
	{
		setting[0].radio_id = 0;
		setting[0].ssid_id = 0;
		set_wireless_setting(&setting[0], &return_str, &isReboot);

		if(OK_STR == return_str)
		{
			setting[1].radio_id = 1;
			setting[1].ssid_id = 0;
			set_wireless_setting(&setting[1], &return_str, &isReboot);
		}
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	if(OK_STR == return_str)
	{
		if(isReboot)
		{
			APPAGENT_SYSTEM("uci commit");
			APPAGENT_SYSTEM("reboot &");
		}
		else
		{
			APPAGENT_SYSTEM("luci-reload auto wireless &");
		}
	}

	return 0;
}

/*****************************************************************
* NAME:    set_mesh_device_led_action_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_mesh_device_led_action_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char target_ip[32];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	memset(target_ip, 0x00, sizeof(target_ip));

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		get_json_string_from_query(query_str, target_ip, "TargetMeshIP");

		if(check_mesh_private_ip(target_ip))
		{
			set_device_led_action_cb(pkt);
		}
		else
		{
			redirect_to_target_mesh(pkt, target_ip);
		}
	}

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_device_status_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_device_status_cb(HTTPS_CB *pkt)
{
	bool matched;
	char *query_str;
	char *return_str;
    	char target_ip[32];
    	int  operation_mode;
    	DEVICE_ALL_SETTING_T setting;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
    	matched = FALSE;
    	return_str = NULL;

	if(NULL == query_str){
		send_simple_response(pkt, ERROR_STR);
        	return -1;
	}

	if(FALSE == check_mesh_account(query_str))
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshIP");
		matched = check_mesh_private_ip(target_ip) || (0 == strcmp(target_ip, "127.0.0.1"));

        if(matched)
        {
		    // The device's role in mesh network.
			// 1=Router.
			// 2=AP.
			setting.operation_mode=2;

            memset(&setting, 0x00, sizeof(setting));
            get_device_settings(&setting.device_setting);
			get_lan_options(&setting.lan_setting);
			// only show the first set of SSID
            setting.wlan_setting[0].radio_id = 0;
            setting.wlan_setting[0].ssid_id = 0;
			get_wlan_options(&setting.wlan_setting[0],&return_str);

#if SUPPORT_WLAN_5G_SETTING
			if(NULL == return_str)
            {	// only show the first set of SSID
                setting.wlan_setting[1].radio_id = 1;
                setting.wlan_setting[1].ssid_id = 0;
                get_wlan_options(&setting.wlan_setting[1],&return_str);
            }
#endif
			setting.wlan_security[0].radio_id = 0;
			setting.wlan_security[0].ssid_id = 0;
			get_wlan_security_options(&setting.wlan_security[0], &return_str);
#if SUPPORT_WLAN_5G_SETTING
			setting.wlan_security[1].radio_id = 1;
			setting.wlan_security[1].ssid_id = 0;
			get_wlan_security_options(&setting.wlan_security[1], &return_str);
#endif
            get_mesh_wlan_settings(&setting.mesh_wlan_setting, &return_str);

            if(NULL != return_str)
                send_simple_response(pkt, return_str);
            else
            {
                return_str = OK_STR;
                get_json_mesh_device_status_cb(pkt, &setting, return_str);
            }
        }
        else
            redirect_to_target_mesh(pkt, target_ip);
    }

    return 0;
}


/*****************************************************************
* NAME:    get_mesh_device_wan_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_device_wan_settings_cb(HTTPS_CB *pkt)
{
	bool result;
	char *query_str;
	char *return_str;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	result = FALSE;
	return_str = OK_STR;

	if(NULL == query_str)
		return_str = ERROR_STR;

	if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}
/*****************************************************************
* NAME:    set_mesh_device_wan_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_mesh_device_wan_settings_cb(HTTPS_CB *pkt)
{
	bool result;
	char *query_str;
	char *return_str;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	result = FALSE;
	return_str = OK_STR;

	if(NULL == query_str)
		return_str = ERROR_STR;

	if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    download_mesh_device_firmware_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int download_mesh_device_firmware_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	char target_ip[32];
	char ip[128];
	char file_path_buf[128];
	int  port;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshIP");

		/* If the target MESH IP does not exist, then the packet is for the DUT itself. */
		if(0 == strlen(target_ip) || check_mesh_lan_ip(target_ip))
		{
			get_json_string_from_query(query_str, ip, "IP");
			get_json_integer_from_query(query_str, &port, "Port");
			get_json_string_from_query(query_str, file_path_buf, "FilePath");

			APPAGENT_SYSTEM("wget --no-check-certificate -O/tmp/firmware.img http://%s:%d/%s &",
			        ip, port, file_path_buf);

			return_str = OK_STR;
		}
		else
		{
			/* Send to target MESH device */
			redirect_to_target_mesh(pkt, target_ip);
			return 0;
		}
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    download_mesh_device_firmware_status_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int download_mesh_device_firmware_status_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	char buf[128];
	char target_ip[32];
	long total_size;
	long download_size;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshIP");

		/* If the target MESH IP does not exist, then the packet is for the DUT itself. */
		if(0 == strlen(target_ip) || check_mesh_lan_ip(target_ip))
		{
			memset(buf, 0x00, sizeof(buf));

			if(sysutil_check_file_existed("/tmp/firmware.img"))
			{
				return_str = ERROR_DOWNLOAD_FAILED_STR;

				/* 123 root      1528 S    wget -O/tmp/firmware.img http://192.168.0.1:9000/usb_admin/sda1/firmware.img */
				sysutil_interact(buf, sizeof(buf), "ps | grep wget | grep -v grep | awk '{print $6}'");

				if((0 != strlen(buf)) && (NULL != strstr(buf, "/tmp/firmware.img")))
				{
					return_str = ERROR_PROCESS_IS_RUNNING_STR;
				}
				else
				{
					memset(buf, 0x00, sizeof(buf));
					get_json_string_from_query(query_str, buf, "FileSize");
					total_size = atol(buf);

					/* -rw-r--r--    1 root     root      17825796 Aug 22 07:00 /tmp/firmware.img */
					memset(buf, 0x00, sizeof(buf));
					sysutil_interact(buf, sizeof(buf), "ls -l /tmp/firmware.img | awk '{print $5}'");
					download_size = atol(buf);

					if(download_size == total_size)
					{
						return_str = OK_STR;
					}
				}
			}
		}
		else
		{
			/* Send to target MESH device */
			redirect_to_target_mesh(pkt, target_ip);
			return 0;
		}
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    do_mesh_device_firmware_upgrade_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int do_mesh_device_firmware_upgrade_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	char *ptr;
	char buf[256];
	char buf_payload[1024];
	char ip[64];
    char action[32];
    char new_ver[64];
    char ori_ver[64];
    char *saveptr1, *saveptr2, *ptr_ori, *ptr_new;
    bool new_firmware;
    new_firmware = FALSE;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;
	ptr = NULL;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
        get_json_string_from_query(query_str, action, "ActionFrom");
		if(sysutil_check_file_existed("/tmp/firmware.img"))
		{
			memset(buf_payload, 0x00, sizeof(buf_payload));
			sysCheckStringOnApp(query_str, buf_payload);

			memset(buf, 0x00, sizeof(buf));
			sysutil_interact(buf, sizeof(buf), "batctl o -H | awk '{print $1}'");

			ptr = strtok(buf, "\n");

			do
			{
				memset(ip, 0x00, sizeof(ip));
				mac_to_ipv6(ptr, ip);

				APPAGENT_SYSTEM("app_client -i %s -m POST -a %s -e 1 -p \"%s\" &",
						ip,
						"mesh/DoMeshSingleDeviceFirmwareUpgrade",
						buf_payload);

				APPAGENT_SYSTEM("sleep 1");
			} while(NULL != (ptr = strtok(NULL, "\n")));

			APPAGENT_SYSTEM("sleep 1");
            if (strcmp(action, "1") == 0) //action from GUI
            {
                APPAGENT_SYSTEM("/sbin/sysupgrade -v /tmp/firmware.img &");
            }
            else
            {
                if(sysutil_check_file_existed("/tmp/FWinfo.log"))
                {
                    sys_interact(new_ver, sizeof(new_ver), "cat /tmp/FWinfo.log  | awk -F\":\" '{print $3}' |awk -F'\"' '{print $2}' | cut -c '2-'");
    			    sys_interact(ori_ver, sizeof(ori_ver), "cat /etc/version | grep Firmware | awk 'BEGIN{FS= \" \"} {print $4}'");

                    new_ver[strlen(new_ver)-1]=0;
                    ori_ver[strlen(ori_ver)-1]=0;

                    ptr_new = strtok_r(new_ver, ".", &saveptr1);   
                    ptr_ori = strtok_r(ori_ver, ".", &saveptr2);                 

                    while (ptr_ori != NULL && ptr_new != NULL)
                    {
                        if (atoi(ptr_ori) > atoi(ptr_new))
                        {
                            new_firmware = FALSE;
                            break;
                        }
                        else if (atoi(ptr_ori) < atoi(ptr_new))
                        {
                            new_firmware = TRUE;
                            break;
                        }

                        ptr_new = strtok_r(saveptr1, ".", &saveptr1);
                        ptr_ori = strtok_r(saveptr2, ".", &saveptr2);
                    }
                    if(new_firmware == TRUE) 
                    {

                        APPAGENT_SYSTEM("/sbin/sysupgrade -v /tmp/firmware.img &");
                    }
                }
            }

			return_str = OK_STR;
		}
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    do_mesh_single_device_firmware_upgrade_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int do_mesh_single_device_firmware_upgrade_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		if(sysutil_check_file_existed("/tmp/firmware.img"))
		{
			return_str = OK_STR;

			APPAGENT_SYSTEM("/sbin/sysupgrade -v /tmp/firmware.img &");
		}
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

#if SUPPORT_IPERF_THROUGHPUT_TEST
/*****************************************************************
* NAME:    run_mesh_throughput_test_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int run_mesh_throughput_test_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char source[32];
    char buf[64]={0};
    char buf2[4]={0};

    memset(source, 0x00, sizeof(source));

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else
    {
        get_json_string_from_query(query_str, source, "Source");

        sysutil_interact(buf2, sizeof(buf2), "[ $(echo %s | grep -E \"^[a-fA-F0-9:]{17}$\") ] && echo -n 1 || echo -n 0", source);
        if(atoi(buf2))
        {
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\\n'", source);
        }
        else
        {
            sprintf(buf, "%s", source);
        }

        if(check_mesh_global_ipv6(buf))
        {
            run_throughput_test_cb(pkt);
        }
        else
        {
            redirect_to_target_mesh(pkt, buf);
        }
    }

    return 0;
}

/*****************************************************************
* NAME:    get_mesh_throughput_test_result_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_throughput_test_result_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char source[32];
    char buf[64]={0};
    char buf2[4]={0};

    memset(source, 0x00, sizeof(source));

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else
    {
        get_json_string_from_query(query_str, source, "Source");

        sysutil_interact(buf2, sizeof(buf2), "[ $(echo %s | grep -E \"^[a-fA-F0-9:]{17}$\") ] && echo -n 1 || echo -n 0", source);
        if(atoi(buf2))
        {
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\\n'", source);
        }
        else
        {
            sprintf(buf, "%s", source);
        }

        if(check_mesh_global_ipv6(buf))
        {
            get_throughput_test_result_cb(pkt);
        }
        else
        {
            redirect_to_target_mesh(pkt, buf);
        }
    }

    return 0;
}
#endif

#if HAS_SPEEDTEST_THROUGHPUT_TEST
/*****************************************************************
* NAME:    run_mesh_speedtest_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  20170103 stevenlin
* Modify:
******************************************************************/
int run_mesh_speedtest_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char target_mac[32];
    char buf[64];

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else
	{
        memset(target_mac, 0x00, sizeof(target_mac));
        get_json_string_from_query(query_str, target_mac, "TargetMeshMac");

        if(0 == strlen(target_mac) || check_mesh_private_mac(target_mac))
        {
            run_speedtest_test_cb(pkt);
        }
        else
        {
            memset(buf, 0x00, sizeof(buf));
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\\n'", target_mac);
            redirect_to_target_mesh(pkt, buf);
        }
	}

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_speedtest_result_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  20170103 stevenlin
* Modify:
******************************************************************/
int get_mesh_speedtest_result_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char target_mac[32];
    char buf[64];

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else
    {
        memset(target_mac, 0x00, sizeof(target_mac));
        get_json_string_from_query(query_str, target_mac, "TargetMeshMac");

        if(0 == strlen(target_mac) || check_mesh_private_mac(target_mac))
        {
            get_speedtest_test_result_cb(pkt);
        }
        else
        {
            memset(buf, 0x00, sizeof(buf));
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\\n'", target_mac);
            redirect_to_target_mesh(pkt, buf);
        }
    }

    return 0;
}

/*****************************************************************
* NAME:    find_mesh_speedtest_best_server_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  20200926 Jim.Yang
* Modify:
******************************************************************/
int find_mesh_speedtest_best_server_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    //char *query_str = (char *)pkt->body;
    char target_mac[32];
    char buf[64];

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else
	{
        memset(target_mac, 0x00, sizeof(target_mac));
        get_json_string_from_query(query_str, target_mac, "TargetMeshMac");

        if(0 == strlen(target_mac) || check_mesh_private_mac(target_mac))
        {
            find_speedtest_best_server(pkt);
        }
        else
        {
            memset(buf, 0x00, sizeof(buf));
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\\n'", target_mac);
            redirect_to_target_mesh(pkt, buf);
        }
	}

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_speedtest_best_server_result_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  20200926 Jim.Yang
* Modify:
******************************************************************/
int get_mesh_speedtest_best_server_result_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    //char *query_str = (char *)pkt->body;
    char target_mac[32];
    char buf[64];

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else
    {
        memset(target_mac, 0x00, sizeof(target_mac));
        get_json_string_from_query(query_str, target_mac, "TargetMeshMac");
        if(0 == strlen(target_mac) || check_mesh_private_mac(target_mac))
        {
            get_speedtest_best_server_result_cb(pkt);
        }
        else
        {
            memset(buf, 0x00, sizeof(buf));
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\\n'", target_mac);
            redirect_to_target_mesh(pkt, buf);
        }
    }

    return 0;
}
#endif

/*****************************************************************
* NAME:    is_mesh_ping_test_in_process
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool is_mesh_ping_test_in_process(char *file_path)
{
    bool result_p, result_f;
    char buf[256];

    result_p = FALSE;
    result_f = FALSE;
    memset(buf, 0x00, sizeof(buf));

    /* 1156 root      1068 S    batctl p 8a:dc:96:17:49:66 -c 16 */
    sysutil_interact(buf, sizeof(buf), "ps | grep batctl | awk '{print $6}' | grep p | grep -v grep");

    if(0 != strlen(buf))
    {
        result_p = TRUE;
    }

    if(sysutil_check_file_existed(file_path))
    {
        memset(buf, 0x00, sizeof(buf));

        sysutil_interact(buf, sizeof(buf), "cat %s | grep rtt", file_path);

        if(0 == strlen(buf))
        {
            result_f = TRUE;
        }
    }

    /* ping is in process and the result is not completely written into file. */
    return (result_p && result_f);
}

/*****************************************************************
* NAME:    run_mesh_ping_test_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int run_mesh_ping_test_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *return_str;
    char mac[32];
    char mesh_mac[32];
    char source[32];
    char destination[32];
    char destination_mac[32];
    char result_file[128];
    int count;

    count = 0;
    return_str = ERROR_STR;
    memset(mac, 0x00, sizeof(mac));
    memset(mesh_mac, 0x00, sizeof(mesh_mac));
    memset(source, 0x00, sizeof(source));
    memset(destination, 0x00, sizeof(destination));
    memset(result_file, 0x00, sizeof(result_file));

    if(get_json_string_from_query(query_str, mesh_mac, "MeshMAC"))
    {
        remove_colon_from_mac(mesh_mac, mac);
        sprintf(result_file, MESH_PING_TEST_RESULT_FILE"_%s", mac);
    }

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else if(is_mesh_ping_test_in_process(result_file))
    {
        send_simple_response(pkt, ERROR_PROCESS_IS_RUNNING_STR);
    }
    else
    {
        get_json_string_from_query(query_str, source, "Source");

        if(check_mesh_global_ipv6(source))
        {
            if(sysutil_check_file_existed(result_file))
            {
                SYSTEM("rm -f %s", result_file);
            }

            if((get_json_string_from_query(query_str, destination, "Destination")) &&
               (get_json_integer_from_query(query_str, &count, "NumberOfPing")))
            {
                if(ipv6_to_mac(destination, destination_mac))
                {
                    APPAGENT_SYSTEM("batctl p -c %d %s > %s 2>&1 &", count, destination_mac, result_file);

                    return_str = OK_STR;
                }
            }

            send_simple_response(pkt, return_str);
        }
        else
        {
            redirect_to_target_mesh(pkt, source);
        }
    }

    return 0;
}

/*****************************************************************
* NAME:    get_mesh_ping_test_result_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_ping_test_result_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *result;
    char mac[32];
    char mesh_mac[32];
    char source[32];
    char result_file[128];

    result = ERROR_STR;
    memset(mac, 0x00, sizeof(mac));
    memset(mesh_mac, 0x00, sizeof(mesh_mac));
    memset(source, 0x00, sizeof(source));
    memset(result_file, 0x00, sizeof(result_file));

    if(get_json_string_from_query(query_str, mesh_mac, "MeshMAC"))
    {
        remove_colon_from_mac(mesh_mac, mac);
        sprintf(result_file, MESH_PING_TEST_RESULT_FILE"_%s", mac);
    }

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else if(is_mesh_ping_test_in_process(result_file))
    {
        send_simple_response(pkt, ERROR_PROCESS_IS_RUNNING_STR);
    }
    else
    {
        get_json_string_from_query(query_str, source, "Source");

        if(check_mesh_global_ipv6(source))
        {
            if(sysutil_check_file_existed(result_file))
            {
                result = OK_STR;
            }

            get_specific_test_result_json_cb(pkt, result_file, result);
        }
        else
        {
            redirect_to_target_mesh(pkt, source);
        }
    }

    return 0;
}

/*****************************************************************
* NAME:    is_mesh_trace_route_in_process
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool is_mesh_trace_route_in_process(char *file_path, char *destination)
{
    bool result_tr, result_f;
    char buf[256];

    result_tr = FALSE;
    result_f = FALSE;
    memset(buf, 0x00, sizeof(buf));

    /* 1156 root      1068 S    batctl p 8a:dc:96:17:49:66 -c 16 */
    sysutil_interact(buf, sizeof(buf), "ps | grep batctl | awk '{print $6}' | grep tr | grep -v grep");

    if(0 != strlen(buf))
    {
        result_tr = TRUE;
    }

    if(sysutil_check_file_existed(file_path))
    {
        memset(buf, 0x00, sizeof(buf));

        sysutil_interact(buf, sizeof(buf), "cat %s | awk '{print $2}' | grep \"%s\"", file_path, destination);

        if(0 == strlen(buf))
        {
            result_f = TRUE;
        }
    }

    /* ping is in process and the result is not completely written into file. */
    return (result_tr && result_f);
}

/*****************************************************************
* NAME:    run_mesh_trace_route_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int run_mesh_trace_route_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *return_str;
    char mac[32];
    char mesh_mac[32];
    char source[32];
    char source_mac[32];
    char destination[32];
    char destination_mac[32];
    char result_file[128];

    return_str = ERROR_STR;
    memset(mac, 0x00, sizeof(mac));
    memset(mesh_mac, 0x00, sizeof(mesh_mac));
    memset(source, 0x00, sizeof(source));
    memset(source_mac, 0x00, sizeof(source_mac));
    memset(destination, 0x00, sizeof(destination));
    memset(destination_mac, 0x00, sizeof(destination_mac));
    memset(result_file, 0x00, sizeof(result_file));

    if(get_json_string_from_query(query_str, mesh_mac, "MeshMAC"))
    {
        remove_colon_from_mac(mesh_mac, mac);
        sprintf(result_file, MESH_TRACE_ROUTE_RESULT_FILE"_%s", mac);
    }

    get_json_string_from_query(query_str, destination, "Destination");

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else if(is_mesh_trace_route_in_process(result_file, destination))
    {
        send_simple_response(pkt, ERROR_PROCESS_IS_RUNNING_STR);
    }
    else
    {
        get_json_string_from_query(query_str, source_mac, "Source");

        if(strchr(source_mac, ':') && NULL == strstr(source_mac, "::"))
        {
            mac_to_ipv6(source_mac, source);
        }
        else
        {
            sprintf(source, "%s", source_mac);
        }

        if(check_mesh_global_ipv6(source))
        {
            if(sysutil_check_file_existed(result_file))
            {
                SYSTEM("rm -f %s", result_file);
            }

            if(!strchr(destination, ':'))
            {
                ipv6_to_mac(destination, destination_mac);
            }
            else
            {
                sprintf(destination_mac, "%s", destination);
            }

            if(0 != strlen(destination_mac))
            {
                APPAGENT_SYSTEM("batctl tr %s > %s 2>&1 &", destination_mac, result_file);
                return_str = OK_STR;
            }

            send_simple_response(pkt, return_str);
        }
        else
        {
            redirect_to_target_mesh(pkt, source);
        }
    }

    return 0;
}

/*****************************************************************
* NAME:    get_mesh_trace_route_result_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_trace_route_result_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *result;
    char mac[32];
    char mesh_mac[32];
    char source[32];
    char destination[32];
    char result_file[128];

    result = ERROR_STR;
    memset(mac, 0x00, sizeof(mac));
    memset(mesh_mac, 0x00, sizeof(mesh_mac));
    memset(source, 0x00, sizeof(source));
    memset(destination, 0x00, sizeof(destination));
    memset(result_file, 0x00, sizeof(result_file));

    if(get_json_string_from_query(query_str, mesh_mac, "MeshMAC"))
    {
        remove_colon_from_mac(mesh_mac, mac);
        sprintf(result_file, MESH_TRACE_ROUTE_RESULT_FILE"_%s", mac);
    }

    get_json_string_from_query(query_str, destination, "Destination");

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else if(is_mesh_trace_route_in_process(result_file, destination))
    {
        send_simple_response(pkt, ERROR_PROCESS_IS_RUNNING_STR);
    }
    else
    {
        get_json_string_from_query(query_str, source, "Source");

        if(check_mesh_global_ipv6(source))
        {
            if(sysutil_check_file_existed(result_file))
            {
                result = OK_STR;
            }

            get_specific_test_result_json_cb(pkt, result_file, result);
        }
        else
        {
            redirect_to_target_mesh(pkt, source);
        }
    }

    return 0;
}

/*****************************************************************
* NAME:    get_mesh_device_neighbors_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_device_neighbors_cb(HTTPS_CB *pkt)
{
    char *query_str;
    char *return_str;
    MESH_DEVICE_NEIGHBORS_T setting[32];
    char buf[4096];

    if(NULL == pkt)
    {
        return -1;
    }

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");
    return_str = ERROR_STR;

    if(NULL == query_str)
    {
        send_simple_response(pkt, ERROR_STR);
        return -1;
    }

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else
    {
        memset(buf, 0x00, sizeof(buf));
        memset(setting, 0x00, sizeof(setting));

        if(FINDPROC("alfred") > 0)
        {
            sysutil_interact(buf, sizeof(buf), "alfred -r %d", ALFRED_WDSLINK_INFORMATION);

            parse_json_mesh_device_neighbors(buf, setting, &return_str);

            return_str = OK_STR;

            get_json_mesh_device_neighbors_cb(pkt, setting, return_str);
        }
        else
        {
            send_simple_response(pkt, ERROR_STR);
        }
    }

    return 0;
}

/*****************************************************************
* NAME:    sync_mesh_robust_threshold_cb
* ---------------------------------------------------------------
* FUNCTION: sync mesh-robust RSSI threshold
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int sync_mesh_robust_threshold_cb(HTTPS_CB *pkt)
{
    char *query_str;
    char *return_str;
    int  new_robust_thd = 0, ori_robust_thd = 0;
    int  robust_en = 1;

    if (NULL == pkt)
    {
        goto send_pkt;
    }

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");

    if (NULL == query_str)
    {
        return_str = ERROR_STR;
        goto send_pkt;
    }

    if (FALSE == is_mesh_function_enabled())
    {
        return_str = ERROR_MESH_DISABLED_STR;
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        return_str = ERROR_UNAUTHORIZED_STR;
    }
    else
    {
        get_json_integer_from_query(query_str, &new_robust_thd, "LinkRobustThreshold");
        api_get_wifi_mesh_link_robust_threshold_option(WIRELESS_WIFI_MESH_LINK_ROBUST_THRESHOLD_OPTION, &ori_robust_thd);

        if ((new_robust_thd != 0) && (ori_robust_thd != new_robust_thd))
        {
            api_set_wifi_mesh_link_robust_threshold_option(WIRELESS_WIFI_MESH_LINK_ROBUST_THRESHOLD_OPTION, new_robust_thd);
            api_set_wifi_mesh_link_robust_threshold_option(WIRELESS_WIFI_5G_MESH_LINK_ROBUST_THRESHOLD_OPTION, new_robust_thd);
            APPAGENT_SYSTEM("echo \"*/1 * * * * /sbin/Mesh_robust.sh %d %d\" >> /tmp/etc/crontabs/root", robust_en, new_robust_thd);
            APPAGENT_SYSTEM("/etc/init.d/cron reload");
            //APPAGENT_SYSTEM("uci commit wireless");
        }

        return_str = OK_STR;
    }

send_pkt:

    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    sync_mesh_msc_configured_cb
* ---------------------------------------------------------------
* FUNCTION: sync mesh settings through mesh easy-setup
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int sync_mesh_msc_configured_cb(HTTPS_CB *pkt)
{
    char *query_str;
    char *return_str;
    char mesh_id[65]={0}, mesh_pwd[129]={0}, tmp_chan[10]={0}, tmp_country[8]={0}, tmp_htmode[10]={0}, tmp_cur_chan[8]={0};
    int  mesh_chan = 0, _conn_type = 0, meshbh=0;
    char buf[16]={0}, _orig_chan[5]={0};
#if HAS_WLAN_5G_2_SETTING
	int  isHB=0;
#endif
    FILE *fp;
    struct json_object *jobj;

    if (NULL == pkt)
    {
        goto send_pkt;
    }

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");

    if (NULL == query_str)
    {
        return_str = ERROR_STR;
        goto send_pkt;
    }

    if (FALSE == check_temp_account(query_str))
    {
        return_str = ERROR_UNAUTHORIZED_STR;
    }
    else
    {
        if (get_json_string_from_query(query_str, mesh_id, "ID") != 1)
            return_str = ERROR_STR;
        if (get_json_string_from_query(query_str, mesh_pwd, "Passwd") != 1)
            return_str = ERROR_STR;
        if (get_json_string_from_query(query_str, tmp_chan, "Chan") != 1)
            return_str = ERROR_STR;
        if (get_json_string_from_query(query_str, tmp_country, "Country") != 1)
            return_str = ERROR_STR;
        if (get_json_integer_from_query(query_str, &_conn_type, "ConnType") != 1)
            return_str = ERROR_STR;
		get_json_string_from_query(query_str, tmp_htmode, "HTmode");
		get_json_string_from_query(query_str, tmp_cur_chan, "CurChan");
#if HAS_WLAN_5G_2_SETTING
		if(get_json_integer_from_query(query_str, &meshbh, "MeshBH"))
		{
			isHB=(meshbh==1)?1:0;
		}
#endif

		APPAGENT_SYSTEM("uci set wireless.wifi0.country='%s'", tmp_country);
		APPAGENT_SYSTEM("uci set wireless.wifi1.country='%s'", tmp_country);
#if HAS_WLAN_5G_2_SETTING
		APPAGENT_SYSTEM("uci set wireless.wifi2.country='%s'", tmp_country);
#endif

		if(strlen(tmp_cur_chan))
		{
			APPAGENT_SYSTEM("echo %s > /tmp/ez_cur_chan", tmp_cur_chan);
#if HAS_WLAN_5G_2_SETTING
			isHB=(atoi(tmp_cur_chan) >= 100)?1:0;
#endif
		}

		if(strcmp("auto_5G", tmp_chan)!=0)
		{
			mesh_chan = atoi(tmp_chan);
#if HAS_WLAN_5G_2_SETTING
			isHB=(mesh_chan >= 100)?1:0;
#endif
		}
        if ((strcmp("auto_5G", tmp_chan)!=0) && mesh_chan < 36)
        {
            APPAGENT_SYSTEM("uci set wireless.wifi0.channel_config_enable=%d", 1);
#if SUPPORT_MESH_AUTO_CHAN
            api_get_string_option("wireless.wifi0.channel", _orig_chan, sizeof(_orig_chan));
            if (strcmp(_orig_chan, "auto")!=0 && strcmp(_orig_chan, "0")!=0)
            {
                APPAGENT_SYSTEM("uci set wireless.wifi0.channel=%d", mesh_chan);
            }
#else
            APPAGENT_SYSTEM("uci set wireless.wifi0.channel=%d", mesh_chan);
#endif
            APPAGENT_SYSTEM("uci set wireless.wifi0_mesh.disabled=%d", 0);
            APPAGENT_SYSTEM("uci set network.sys.mesh_configured=%d", 1);
			if (strlen(tmp_htmode)>0)
			{
				APPAGENT_SYSTEM("uci set wireless.wifi0.htmode='%s'", tmp_htmode);
			}

        }
#if SUPPORT_WLAN_5G_SETTING
#if HAS_WLAN_5G_2_SETTING
        else
        {
            APPAGENT_SYSTEM("uci set wireless.wifi%d.channel_config_enable=%d", (isHB)?1:2, 1);
#if SUPPORT_MESH_AUTO_CHAN
            if (strcmp("auto_5G", tmp_chan)!=0)
            {
                APPAGENT_SYSTEM("uci set wireless.wifi%d.channel=%d", (isHB)?1:2, mesh_chan);
				if (strlen(tmp_htmode)>0)
				{
					APPAGENT_SYSTEM("uci set wireless.wifi%d.htmode='%s'", (isHB)?1:2, tmp_htmode);
				}
            }
            else
            {
                APPAGENT_SYSTEM("uci set wireless.wifi%d.channel=0", (isHB)?1:2);
				if (strlen(tmp_htmode)>0)
				{
					APPAGENT_SYSTEM("uci set wireless.wifi1.htmode='%s'", tmp_htmode);
					APPAGENT_SYSTEM("uci set wireless.wifi2.htmode='%s'", tmp_htmode);
				}
            }
#else
            APPAGENT_SYSTEM("uci set wireless.wifi%d.channel=%d", (isHB)?1:2, mesh_chan);
			if (strlen(tmp_htmode)>0)
			{
				APPAGENT_SYSTEM("uci set wireless.wifi%d.htmode='%s'", (isHB)?1:2, tmp_htmode);
			}
#endif
            APPAGENT_SYSTEM("uci set wireless.wifi%d_mesh.disabled=%d", (isHB)?1:2, 0);
            APPAGENT_SYSTEM("uci set network.sys.mesh_configured_5g=%d", 1);
        }
#else
        else
        {
            APPAGENT_SYSTEM("uci set wireless.wifi1.channel_config_enable=%d", 1);
#if SUPPORT_MESH_AUTO_CHAN
            api_get_string_option("wireless.wifi1.channel", _orig_chan, sizeof(_orig_chan));
            if (strcmp(_orig_chan, "auto")!=0 && strcmp(_orig_chan, "0")!=0 && (strcmp("auto_5G", tmp_chan)!=0))
            {
                APPAGENT_SYSTEM("uci set wireless.wifi1.channel=%d", mesh_chan);
            }
            else
            {
                APPAGENT_SYSTEM("uci set wireless.wifi1.channel=0");
            }
#else
            APPAGENT_SYSTEM("uci set wireless.wifi1.channel=%d", mesh_chan);
#endif
            APPAGENT_SYSTEM("uci set wireless.wifi1_mesh.disabled=%d", 0);
            APPAGENT_SYSTEM("uci set network.sys.mesh_configured_5g=%d", 1);
			if (strlen(tmp_htmode)>0)
			{
				APPAGENT_SYSTEM("uci set wireless.wifi1.htmode='%s'", tmp_htmode);
			}
        }
#endif
#endif
            // delete easy-setup tmp pwd
            APPAGENT_SYSTEM("[ -n \"$(uci show wireless.wifi0_mesh | grep t_esPwd)\" ] && uci delete wireless.wifi0_mesh.t_esPwd");
#if SUPPORT_WLAN_5G_SETTING
            APPAGENT_SYSTEM("[ -n \"$(uci show wireless.wifi1_mesh | grep t_esPwd)\" ] && uci delete wireless.wifi1_mesh.t_esPwd");
#endif
#if HAS_WLAN_5G_2_SETTING
			APPAGENT_SYSTEM("[ -n \"$(uci show wireless.wifi2_mesh | grep t_esPwd)\" ] && uci delete wireless.wifi2_mesh.t_esPwd");
#endif


        // set mesh-ID
        if (_conn_type)
        {
            APPAGENT_SYSTEM("uci set wireless.wifi0_mesh.Mesh_id=%s", mesh_id);
#if SUPPORT_WLAN_5G_SETTING
            APPAGENT_SYSTEM("uci set wireless.wifi1_mesh.Mesh_id=%s", mesh_id);
#endif
#if HAS_WLAN_5G_2_SETTING
			APPAGENT_SYSTEM("uci set wireless.wifi2_mesh.Mesh_id=%s", mesh_id);
#endif
        }
        else
        {
            APPAGENT_SYSTEM("uci set wireless.wifi0_mesh.ssid=%s", mesh_id);
#if SUPPORT_WLAN_5G_SETTING
            APPAGENT_SYSTEM("uci set wireless.wifi1_mesh.ssid=%s", mesh_id);
#endif
#if HAS_WLAN_5G_2_SETTING
			APPAGENT_SYSTEM("uci set wireless.wifi2_mesh.ssid=%s", mesh_id);
#endif
        }

        // set common settings
        APPAGENT_SYSTEM("uci set wireless.wifi0_mesh.aeskey=%s", mesh_pwd);
        APPAGENT_SYSTEM("uci set wireless.wifi0_mesh.MeshEzBroCast=%d", 0);
#if SUPPORT_WLAN_5G_SETTING
        APPAGENT_SYSTEM("uci set wireless.wifi1_mesh.aeskey=%s", mesh_pwd);
        APPAGENT_SYSTEM("uci set wireless.wifi1_mesh.MeshEzBroCast=%d", 0);
#endif
#if HAS_WLAN_5G_2_SETTING
		APPAGENT_SYSTEM("uci set wireless.wifi2_mesh.aeskey=%s", mesh_pwd);
		APPAGENT_SYSTEM("uci set wireless.wifi2_mesh.MeshEzBroCast=%d", 0);
#endif

        // enable EnMesh config
#if HAS_MESH_EZSETUP_FOR_AP_MODE
        APPAGENT_SYSTEM("uci set network.sys.EnMesh=%d", 1);
        if(sysutil_check_file_existed("/tmp/ezsetup_temp_link"))
        {
            APPAGENT_SYSTEM("rm -rf /tmp/ezsetup_temp_link");
        }
#endif
        APPAGENT_SYSTEM("uci set mesh.wifi.role='client'");
        APPAGENT_SYSTEM("uci set mesh.wifi.controller='slave'");

        // enable mesh netowrk
        APPAGENT_SYSTEM("uci set mesh.wifi.disabled=%d", 0);
        sys_interact(buf, sizeof(buf), "[ -x '/usr/shc/mode_setting' ] && echo -n 1 || echo -n 0");
        if (strcmp(buf, "1") == 0)
        {
            if(sysutil_check_file_existed("/tmp/ch2apclient"))
            {
                system("rm -rf /tmp/ch2apclient");
            }
			fp = fopen("/tmp/ch2apclient", "w");
			if(fp)
			{
				fprintf(fp, "mode_setting ap\n");
				fprintf(fp, "uci set wireless.wifi0_ssid_1.nobeacon=0\n");
				fprintf(fp, "uci set wireless.wifi1_ssid_1.nobeacon=0\n");
#if HAS_WLAN_5G_2_SETTING
				fprintf(fp, "uci set wireless.wifi2_ssid_1.nobeacon=0\n");
#endif
				fprintf(fp, "uci set bluetooth.@bt_app[0].configured=1\n");
				fprintf(fp, "uci commit wireless\n");
				fprintf(fp, "uci commit bluetooth\n");
				fprintf(fp, "/sbin/autoFwUpgrade.sh\n");
			}
			fclose(fp);
			if(sysutil_check_file_existed("/tmp/ch2apclient"))
			{
				APPAGENT_SYSTEM("sh /tmp/ch2apclient &");
			}
        }
        else
        {
            // reload wireless
            APPAGENT_SYSTEM("luci-reload auto &");
        }
        return_str = OK_STR;
    }

send_pkt:

    jobj = json_object_new_object();

    sprintf(buf, "%sResult", pkt->json_action);
    json_object_object_add(jobj, buf, json_object_new_string(return_str));

    sprintf(buf, "SupportRP", pkt->json_action);
    json_object_object_add(jobj, buf, json_object_new_int((sysutil_check_file_existed("/sbin/setRPConnection.sh"))?1:0));

    basic_json_response(pkt,  (char *)json_object_to_json_string(jobj));
    json_object_put(jobj);

    return 0;
}

/*****************************************************************
* NAME:    get_login_mesh_info_cb
* ---------------------------------------------------------------
* FUNCTION: api to prevent httpd hang on login when get mesh global info
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_login_mesh_info_cb(HTTPS_CB *pkt)
{
    char *query_str;
    char *return_str;
    char user_name[64]={0}, password[128]={0};

    if (NULL == pkt)
    {
        goto send_pkt;
    }

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");

    if (NULL == query_str)
    {
        return_str = ERROR_STR;
        goto send_pkt;
    }

    if (FALSE == is_mesh_function_enabled())
    {
        return_str = ERROR_MESH_DISABLED_STR;
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        return_str = ERROR_UNAUTHORIZED_STR;
    }
    else
    {
        get_json_mesh_account(query_str, user_name, password);
        APPAGENT_SYSTEM("/sbin/mesh.sh get_mesh_global_node_info %s %s &", user_name, password);
        return_str = OK_STR;
    }

send_pkt:

    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    send_system_cmd_to_mesh_device_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int send_system_cmd_to_mesh_device_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char target_ip[32];
	char cmd[40960];

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshIP");

		if(check_mesh_global_ipv6(target_ip))
		{
            get_json_string_from_query(query_str, cmd, "SystemCmd");
            APPAGENT_SYSTEM("%s &",cmd);
            send_simple_response(pkt, OK_STR);
		}
		else
		{
			redirect_to_target_mesh(pkt, target_ip);
		}
	}

	return 0;
}
/*****************************************************************
* NAME:    send_uci_changes_to_mesh_device_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author: 20160726 stevenlin
* Modify:
******************************************************************/
int send_uci_changes_to_mesh_device_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char target_ip[32];
	char uci_changes[2048];

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshIP");

        printf("IP:%s\n", target_ip);

		if(check_mesh_global_ipv6(target_ip))
		{
            get_json_string_from_query(query_str, uci_changes, "UciChanges");
            APPAGENT_SYSTEM("printf %s > /tmp/mesh_server_config_data &", uci_changes);
            send_simple_response(pkt, OK_STR);
		}
		else
		{
			redirect_to_target_mesh(pkt, target_ip);
		}
	}

	return 0;
}
/*****************************************************************
* NAME:    set_mesh_device_reboot_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_mesh_device_reboot_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char target_ip[32];
    char buf[32];
#if HAS_IPCAM
	char xrelayd_ip[128];
#endif

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshMac");


        printf("IP:%s\n", target_ip);

		if(check_mesh_private_mac(target_ip) || strcmp("ff:ff:ff:ff:ff:ff", target_ip) == 0)
		{
            send_simple_response(pkt, OK_STR);
#if HAS_IPCAM
            memset(xrelayd_ip, 0x00, sizeof(xrelayd_ip));
            api_get_string_option("xrelayd.xrelayd.conn_sec_ip", xrelayd_ip, sizeof(xrelayd_ip));
            APPAGENT_SYSTEM("app_client -i %s -m GET -a Reboot -e 1", xrelayd_ip);
#endif
            APPAGENT_SYSTEM("reboot &");
		}
        else
		{
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\n'", target_ip);
			redirect_to_target_mesh(pkt, buf);
		}
	}

	return 0;
}

/*****************************************************************
* NAME:    set_mesh_wifi_trigger_wps_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_mesh_wifi_disabled_cb(HTTPS_CB *pkt)
{
    char *query_str;
	char target_ip[32];
	char wireless_switch[8];
    int disabled = 0;
    char buf[32];

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshMac");


        printf("IP:%s\n", target_ip);

		if(check_mesh_private_mac(target_ip))
		{
			get_json_string_from_query(query_str, wireless_switch, "WifiSwitch");

            disabled=atoi(wireless_switch);

            send_simple_response(pkt, OK_STR);

            if (disabled==1) 
            {
                APPAGENT_SYSTEM("uci set wireless.wifi1_ssid_1.disabled=%s", "0");
                APPAGENT_SYSTEM("uci set wireless.wifi0_ssid_1.disabled=%s", "0");
            }
            else
            {
                APPAGENT_SYSTEM("uci set wireless.wifi1_ssid_1.disabled=%s", "1");
                APPAGENT_SYSTEM("uci set wireless.wifi0_ssid_1.disabled=%s", "1");
            }            

            APPAGENT_SYSTEM("uci commit wireless");

			APPAGENT_SYSTEM("luci-reload auto network &");
		}
		else
		{
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\n'", target_ip);
			redirect_to_target_mesh(pkt, buf);
		}
	}

	return 0;
}

/*****************************************************************
* NAME:    set_mesh_wifi_trigger_wps_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_mesh_wifi_trigger_wps_cb(HTTPS_CB *pkt)
{
    char *query_str;
	char target_ip[32];
    char buf[32];
	char ifname[8], ifname2[8];
#if HAS_WLAN_5G_2_SETTING
	char ifname3[8];
#endif

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshMac");

        printf("IP:%s\n", target_ip);

		if(check_mesh_private_mac(target_ip))
		{
			if(sysutil_check_file_existed("/tmp/wps-running"))
			{
				send_simple_response(pkt, ERROR_PROCESS_IS_RUNNING_STR);
			}
			else
			{
				api_get_string_option("wireless.wifi0_ssid_1.ifname", ifname, sizeof(ifname));
				api_get_string_option("wireless.wifi1_ssid_1.ifname", ifname2, sizeof(ifname2));
#if HAS_WLAN_5G_2_SETTING
				api_get_string_option("wireless.wifi2_ssid_1.ifname", ifname3, sizeof(ifname3));
				APPAGENT_SYSTEM("wps_cli %s both_pbc %s %s", ifname, ifname2, ifname3);
#else
				APPAGENT_SYSTEM("wps_cli %s both_pbc %s", ifname, ifname2);
#endif
				send_simple_response(pkt, OK_STR);
			}
		}
		else
		{
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\n'", target_ip);
			redirect_to_target_mesh(pkt, buf);
		}
	}

	return 0;
}

/*****************************************************************
* NAME:    check_mesh_wifi_trigger_wps_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int check_mesh_wifi_trigger_wps_cb(HTTPS_CB *pkt)
{
    char *query_str;
    char *return_str;
    char *ptr;
    char target_ip[128];
    char buf[256];
    char buf_payload[1024];
    char buf_result[1024];
    int no_mesh=0;


    if(NULL == pkt)
    {
        return -1;
    }

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");
    return_str = OK_STR;

    if(NULL == query_str)
    {
        send_simple_response(pkt, ERROR_STR);
        return -1;
    }

    if(FALSE == is_mesh_function_enabled())
    {
        return_str = ERROR_MESH_DISABLED_STR;
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        return_str = ERROR_UNAUTHORIZED_STR;
    }
    else
    {
        memset(buf, 0x00, sizeof(buf));
        api_get_string_option("mesh.wifi.controller", buf, sizeof(buf));

        if(0 == strcmp(buf, "master"))
        {
            return_str = OK_STR;
#if SUPPORT_BATMAN_2019
            sysutil_interact(buf, sizeof(buf), "batctl o -H | grep \"*\" | grep \"No batman *\" | wc -l");
#else
            sysutil_interact(buf, sizeof(buf), "batctl o -H |grep \"No batman *\" | wc -l");
#endif
            no_mesh=atoi(buf);

            if (no_mesh==0)
            {
                sys_interact(buf, sizeof(buf), "batctl o -H | awk '{print $1}'");

                ptr = strtok(buf, "\n");

                do
                {
                    mac_to_ipv6(ptr, target_ip);
                    memset(buf_payload, 0x00, sizeof(buf_payload));
                    memset(buf_result, 0x00, sizeof(buf_result));

                    sysCheckStringOnApp(query_str, buf_payload);

                    sysutil_interact(buf_result, sizeof(buf_result),
                            "app_client -i %s -m POST -a mesh/%s -e 1 -p \"%s\"",
                            target_ip, pkt->json_action, buf_payload);


                    if(NULL != strstr(buf_result, ERROR_PROCESS_IS_RUNNING_STR))
                    {
                        return_str = ptr;
                        break;
                    }
                } while(NULL != (ptr = strtok(NULL, "\n")));
            }

            if(OK_STR == return_str && sysutil_check_file_existed("/tmp/wps-running"))
            {
#if HAS_WLAN_5G_2_SETTING
				sys_interact(buf, sizeof(buf), "getinfo mesh_mac | tr -d \'\n\'");
#else
                sys_interact(buf, sizeof(buf), "ifconfig |grep ath35 | awk \'{print $5}\' | tr -d \'\n\'");
#endif
                return_str = buf;
            }
        }
        else
        {
            if(sysutil_check_file_existed("/tmp/wps-running"))
            {
                return_str = ERROR_PROCESS_IS_RUNNING_STR;
            }
            else
            {
                return_str = OK_STR;
            }
        }
    }

    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    set_mesh_wifi_Location_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_mesh_wifi_Location_cb(HTTPS_CB *pkt)
{
    char *query_str;
	char target_ip[32];
	char location[64];
    char buf[32], counter[4]={0};
    int i=0;

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshMac");
        get_json_string_from_query(query_str, location, "Location");

        printf("IP:%s\n", target_ip);

        sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s", target_ip);
        sysutil_interact(counter, sizeof(counter), "uci show mesh | grep rooms | grep MacAddress | wc -l");

        for (i=0; i < atoi(counter);i++)
        {
            api_get_string_option2(buf, sizeof(buf), "mesh.@rooms[%d].MacAddress", i);

            if (strcmp(target_ip, buf) == 0)
            {
                APPAGENT_SYSTEM("uci set mesh.@rooms[%d].DeviceName=\"%s\"", i, location);
                APPAGENT_SYSTEM("uci commit mesh");
                break;
            }
            else if (strcmp("", buf) == 0)
            {
                APPAGENT_SYSTEM("uci set mesh.@rooms[%d].DeviceName=\"%s\"", i, location);
                APPAGENT_SYSTEM("uci set mesh.@rooms[%d].MacAddress=\"%s\"", i, target_ip);
                APPAGENT_SYSTEM("uci commit mesh");
                break;
            }
        }

		if(check_mesh_private_mac(target_ip))
		{
			api_set_string_option("wireless.wifi1_mesh.MeshDeviceLocation",location,sizeof(location));
			APPAGENT_SYSTEM("uci commit wireless");
			if(sysutil_check_file_existed("/etc/config/samba"))
			{
				APPAGENT_SYSTEM("/etc/init.d/samba restart &");
			}
			if(sysutil_check_file_existed("/etc/config/minidlna"))
			{
				APPAGENT_SYSTEM("/etc/init.d/minidlna restart &");
			}
			send_simple_response(pkt, OK_STR);
		}
		else
		{
			sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\n'", target_ip);
			redirect_to_target_mesh(pkt, buf);
		}
	}

	return 0;
}

/*****************************************************************
* NAME:    set_mesh_wifi_Location_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_mesh_led_disabled_cb(HTTPS_CB *pkt)
{
    char *query_str;
	char target_ip[32];
	char brightness[8];
    char buf[32];


	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));

        get_json_string_from_query(query_str, target_ip, "TargetMeshMac");


        printf("IP:%s\n", target_ip);

		if(check_mesh_private_mac(target_ip))
		{
            send_simple_response(pkt, OK_STR);

			get_json_string_from_query(query_str, brightness, "Brightness");

            APPAGENT_SYSTEM("uci set system.@system[0].basic_led_status=%s", brightness);
            APPAGENT_SYSTEM("uci set system.@system[0].ethernet_led_status=%s", brightness);

            /*APPAGENT_SYSTEM("echo %s > /proc/Lan_Led", brightness);
            APPAGENT_SYSTEM("echo %s > /proc/Wan_Led", brightness);*/
            APPAGENT_SYSTEM("uci commit system");

			APPAGENT_SYSTEM("/usr/shc/led-internet");
		}
		else
		{
			sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\n'", target_ip);
			redirect_to_target_mesh(pkt, buf);
		}
	}

	return 0;
}
/*****************************************************************
* NAME:    get_mesh_basic_mode_wifi_info_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_basic_mode_wifi_info_cb(HTTPS_CB *pkt)
{
    char *query_str;
	char target_ip[32];
    int white=0;
    char buf[32];
	
	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshMac");


        printf("IP:%s\n", target_ip);

		if(check_mesh_private_mac(target_ip))
		{
			get_json_mesh_basic_wifi_info_cb(pkt, OK_STR);
		}
        else
		{
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\n'", target_ip);
			redirect_to_target_mesh(pkt, buf);
		}
	}

	return 0;
}
/*****************************************************************
* NAME:    delete_mesh_device_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   MeshDeviceLocation
******************************************************************/
int delete_mesh_device_cb(HTTPS_CB *pkt)
{
    char *query_str;
	char target_ip[32];
    char buf[32];
    int i=0;
	
	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshMac");

        //sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s", target_ip);

        //printf("IP:%s\n", target_ip);
        
        for (i=0; i < 32;i++) 
        {
            api_get_string_option2(buf, sizeof(buf), "mesh.@rooms[%d].MacAddress", i);

            if (strcmp(target_ip, buf) == 0)
            {
                APPAGENT_SYSTEM("uci delete mesh.@rooms[%d]", i);
                APPAGENT_SYSTEM("uci commit mesh");
                break;
            }
        }

        send_simple_response(pkt, OK_STR);
		// if(check_mesh_private_mac(target_ip))
		// {
		// 	//get_json_mesh_basic_wifi_info_cb(pkt, OK_STR);
  //           send_simple_response(pkt, OK_STR);
  //           APPAGENT_SYSTEM("uci set wireless.wifi1_mesh.Mesh_id=%s", "00000000");
  //           APPAGENT_SYSTEM("uci set wireless.wifi1_mesh.aeskey=%s", "0000000000");

  //           APPAGENT_SYSTEM("uci commit wireless");

		// 	APPAGENT_SYSTEM("luci-reload auto network &");

		// }
		// else
		// {
  //           sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\n'", target_ip);
		// 	redirect_to_target_mesh(pkt, buf);
		// }
	}
}
/*****************************************************************
* NAME:    mesh_reboot_device_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   MeshRebootDevice
******************************************************************/

int mesh_reboot_device_cb(HTTPS_CB *pkt)
{
    char *query_str;
    char *return_str;
    int  new_robust_thd = 0, ori_robust_thd = 0;
    int  robust_en = 1;

    if (NULL == pkt)
    {
        goto send_pkt;
    }

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");

    if (NULL == query_str)
    {
        return_str = ERROR_STR;
        goto send_pkt;
    }

    if (FALSE == is_mesh_function_enabled())
    {
        return_str = ERROR_MESH_DISABLED_STR;
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        return_str = ERROR_UNAUTHORIZED_STR;
    }
    else
    {
        send_simple_response(pkt, OK_STR);
        APPAGENT_SYSTEM("echo reboot | at now + 2 minutes");
        return_str = OK_STR;
    }

send_pkt:

    send_simple_response(pkt, return_str);
}
/*****************************************************************
* NAME:    mesh_reset_device_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   MeshResetDevice
******************************************************************/
int mesh_reset_device_cb(HTTPS_CB *pkt)
{
    char *query_str;
    char *return_str;
    int  new_robust_thd = 0, ori_robust_thd = 0;
    int  robust_en = 1;

    if (NULL == pkt)
    {
        goto send_pkt;
    }

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");

    if (NULL == query_str)
    {
        return_str = ERROR_STR;
        goto send_pkt;
    }

    if (FALSE == is_mesh_function_enabled())
    {
        return_str = ERROR_MESH_DISABLED_STR;
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        return_str = ERROR_UNAUTHORIZED_STR;
    }
    else
    {
        APPAGENT_SYSTEM("rm /overlay/* -rf; echo reboot | at now + 2 minutes");
        return_str = OK_STR;
    }

send_pkt:

    send_simple_response(pkt, return_str);
}
/*****************************************************************
* NAME:    set_mesh_device_usb_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_mesh_device_usb_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char target_ip[32]={0};
    char buf[32]={0};
    char usb_settings[8]={0};

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshMac");

		if(check_mesh_private_mac(target_ip))
		{
			get_json_string_from_query(query_str, usb_settings, "USBSettings");
			if(sysutil_check_file_existed("/etc/config/samba"))
			{
				APPAGENT_SYSTEM("uci set samba.@samba[0].enable=%c", usb_settings[0]);
				APPAGENT_SYSTEM("/etc/init.d/samba restart &");
			}

			if(sysutil_check_file_existed("/etc/init.d/airprint"))
			{
				APPAGENT_SYSTEM("uci set airprint.config.enabled=%c", usb_settings[2]);
				APPAGENT_SYSTEM("/etc/init.d/airprint restart &");
			}
			if(sysutil_check_file_existed("/etc/config/minidlna"))
			{
				APPAGENT_SYSTEM("uci set minidlna.config.enabled=%c", usb_settings[1]);
				APPAGENT_SYSTEM("/etc/init.d/minidlna restart &");
			}
			send_simple_response(pkt, OK_STR);
		}
        else
		{
			sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\n'", target_ip);
			redirect_to_target_mesh(pkt, buf);
		}
	}

	return 0;
}
/*****************************************************************
* NAME:    get_mesh_device_usb_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_device_usb_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char target_ip[32];
    char buf[32];

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshMac");


        printf("IP:%s\n", target_ip);

		if(check_mesh_private_mac(target_ip))
		{
            get_json_mesh_usb_settings_info_cb(pkt, OK_STR);
		}
        else
		{
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\n'", target_ip);
			redirect_to_target_mesh(pkt, buf);
		}
	}

	return 0;
}
/*****************************************************************
* NAME:    get_mesh_device_usb_info_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_device_usb_info_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char target_ip[32];
    char buf[32];

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshMac");


        printf("IP:%s\n", target_ip);

		if(check_mesh_private_mac(target_ip))
		{
            get_json_mesh_usb_device_info_cb(pkt, OK_STR);
		}
        else
		{
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\n'", target_ip);
			redirect_to_target_mesh(pkt, buf);
		}
	}

	return 0;
}
/*****************************************************************
* NAME:    get_mesh_home_info_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_home_info_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char target_ip[32];
    char buf[64];

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		get_json_mesh_home_info_cb(pkt, OK_STR);
	}

	return 0;
}

/*****************************************************************
* NAME:    mesh_reset_target_device_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int mesh_reset_target_device_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char target_ip[32];
    char buf[32];
#if HAS_IPCAM
	char xrelayd_ip[128];
#endif

	if(NULL == pkt)
	{
		return -1;
	}

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == query_str)
	{
		send_simple_response(pkt, ERROR_STR);
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(target_ip, 0x00, sizeof(target_ip));
		get_json_string_from_query(query_str, target_ip, "TargetMeshMac");
//        printf("IP:%s\n", target_ip);
		if(check_mesh_private_mac(target_ip))
		{
#if HAS_IPCAM
            memset(xrelayd_ip, 0x00, sizeof(xrelayd_ip));
            api_get_string_option("xrelayd.xrelayd.conn_sec_ip", xrelayd_ip, sizeof(xrelayd_ip));
            APPAGENT_SYSTEM("app_client -i %s -m POST -a ResetToDefault -e 1", xrelayd_ip);
#endif
            APPAGENT_SYSTEM("rm /overlay/* -rf; echo '** SYSTEM is going reboot **' > /dev/console;reboot");
            send_simple_response(pkt, OK_STR);
		}
        else
		{
			APPAGENT_SYSTEM("/etc/remove_mesh_client_profile %s &", target_ip);
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\n'", target_ip);
			redirect_to_target_mesh(pkt, buf);
		}
	}
	return 0;
}

#if SUPPORT_PEOPLE_FUNCTION
/*****************************************************************
* NAME:    get_mesh_simple_people_info_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_simple_people_info_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char buf[128];
	MESH_USER_PROFILE_T users[MAX_CLIENT_PERSON];
	MESH_CLIENT_PROFILE_T clients[MAX_CLIENT_DEVICE*2];
	MESH_CLIENT_PROFILE_T tmp_clients[MAX_CLIENT_DEVICE*2];
	MESH_FIREWALL_RULE_T rules[MAX_FIREWALL_EBTABLES_RULE];
	int i, j, k, index, boolean;
	bool user, client, rule;
	FILE *fp;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		if(sysutil_check_file_existed("/tmp/clientRefresh.sh"))
		{
			APPAGENT_SYSTEM("/tmp/clientRefresh.sh &");
		}

		user = 0, client = 0, rule = 0;
		memset(users, 0x00, sizeof(users));
		memset(clients, 0x00, sizeof(clients));
		memset(tmp_clients, 0x00, sizeof(tmp_clients));
		memset(rules, 0x00, sizeof(rules));

		get_json_boolean_from_query(query_str, &user, "User");
		get_json_boolean_from_query(query_str, &client, "Client");
		get_json_boolean_from_query(query_str, &rule, "Rule");

		if(user)
		{
			for(i = 1, j = 0; i <= MAX_CLIENT_PERSON; i++)
			{
				users[j].index = i;
				api_get_string_option2(users[j].name, sizeof(users[j].name), CLIENT_PERSON_NAME_OPTION, i);

				if(strlen(users[j].name))
				{
					boolean = 0;
					api_get_bool_option2(&boolean, CLIENT_PERSON_BLOCK_OPTION, i);
					users[j].block = boolean;

					boolean = 0;
					api_get_bool_option2(&boolean, CLIENT_PERSON_BLOCK_ACTIVATED_OPTION, i);
					users[j].block_activated = boolean;

					boolean = 0;
					api_get_bool_option2(&boolean, CLIENT_PERSON_SCH_BLOCK_ACTIVATED_OPTION, i);
					users[j].schedule_block_activated = boolean;

					j++;
				}
			}
		}

		if(client)
		{
			/* cat /tmp/finger_device_list */
			/*
			1       00:aa:bb:cc:dd:e0       192.168.0.104   Android android-1234567890123456        11111_HUDDLE    -83     3Kb      37Kb
			1       00:aa:bb:cc:dd:e1       192.168.0.102   Apple iOS       iPhone          11111_HUDDLE    -36     43300Kb 1941Kb
			1       00:aa:bb:cc:dd:e2       192.168.0.106   Windows 7/Vista/Server 2008     Desk-PC 11111_HUDDLE    -56     5Kb      14Kb
			*/
			if(NULL != (fp = fopen("/tmp/finger_device_list", "r")))
			{
				i = 0;

				while(NULL != fgets(buf, sizeof(buf), fp))
				{
					int client_status=0;
					sscanf(buf, "%d\t%s\t%s\t%*[^\t]\t%s\t%*[^\n]\n",&client_status, tmp_clients[i].mac, tmp_clients[i].ip, tmp_clients[i].name);

					if(api_check_ip_addr(tmp_clients[i].ip))
					{
						tmp_clients[i].is_guest = api_check_is_guest_network(tmp_clients[i].ip);
					}

					memset(buf, 0x00, sizeof(buf));

					if(tmp_clients[i].is_guest)
					{
						sys_interact(buf, sizeof(buf),
								"lua /usr/lib/lua/luci/location_guest.lua %s | tr -d '\\n'",
								tmp_clients[i].mac);
					}
					else
					{
						sys_interact(buf, sizeof(buf),
								"lua /usr/lib/lua/luci/location.lua %s | tr -d '\\n'",
								tmp_clients[i].mac);
					}

					sscanf(buf, "%[^\t]\t%s",
							tmp_clients[i].connected_host_name, tmp_clients[i].connected_host_mac);

					tmp_clients[i].status = client_status;
					i++;
				}
                fclose(fp);
			}

			for(i = 1, j = 0; i <= MAX_CLIENT_DEVICE; i++)
			{
				api_get_string_option2(clients[j].mac, sizeof(clients[j].mac), CLIENT_DEVICE_MAC_OPTION, i);

				if(0 == strlen(clients[j].mac))
				{
					continue;
				}

				for(k = 0; k < MAX_CLIENT_DEVICE; k++)
				{
					if(0 == strcasecmp(tmp_clients[k].mac, clients[j].mac))
					{
						memcpy(&clients[j], &tmp_clients[k], sizeof(MESH_CLIENT_PROFILE_T));
						memset(&tmp_clients[k], 0x00, sizeof(MESH_CLIENT_PROFILE_T));
						break;
					}
				}

				clients[j].index = i;
				api_get_string_option2(clients[j].name, sizeof(clients[j].name), CLIENT_DEVICE_NAME_OPTION, i);
				api_get_integer_option2(&clients[j].owner_index, CLIENT_DEVICE_OWNER_INDEX_OPTION, i);

				boolean = 0;
				api_get_bool_option2(&boolean, CLIENT_DEVICE_BLOCK_OPTION, i);
				clients[j].block = boolean;

				boolean = 0;
				api_get_bool_option2(&boolean, CLIENT_DEVICE_SCHEDULE_BLOCK_OPTION, i);
				clients[j].schedule_block = boolean;

				boolean = 0;
				api_get_bool_option2(&boolean, CLIENT_DEVICE_BLOCK_ACTIVATED_OPTION, i);
				clients[j].block_activated = boolean;

				boolean = 0;
				api_get_bool_option2(&boolean, CLIENT_DEVICE_SCH_BLOCK_ACTIVATED_OPTION, i);
				clients[j].schedule_block_activated = boolean;

				j++;
			}

			for(i = 0, j = 0; i < MAX_CLIENT_DEVICE*2; i++)
			{
				if(0 == strlen(clients[i].mac))
				{
					for(; j < MAX_CLIENT_DEVICE; j++)
					{
						if(0 != strlen(tmp_clients[j].mac))
						{
							break;
						}
					}

					/* No extra device */
					if(MAX_CLIENT_DEVICE == j)
					{
						break;
					}

					memcpy(&clients[i], &tmp_clients[j], sizeof(MESH_CLIENT_PROFILE_T));
					j++;
				}
			}
		}

		if(rule)
		{
			for(i = 1, j = 0; i <= MAX_FIREWALL_EBTABLES_RULE; i++)
			{
				rules[j].index = i;
				api_get_string_option2(rules[j].name, sizeof(rules[j].name), SN_FIREWALL_EBTABLES_RULE_NAME_OPTION, i);

				if(strlen(rules[j].name))
				{
					j++;
				}
			}
		}

		get_mesh_simple_people_info_json_cb(pkt, users, clients, rules, OK_STR);
	}

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_user_profile_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_user_profile_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char buf[128];
	int device_index[MAX_CLIENT_DEVICE];
	int rule_index[MAX_FIREWALL_EBTABLES_RULE];
	int boolean;
	int i;
	MESH_USER_PROFILE_T setting;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(&setting, 0x00, sizeof(setting));

		get_json_integer_from_query(query_str, &setting.index, "Index");

		if(0 != setting.index)
		{
			api_get_string_option2(setting.name, sizeof(setting.name), CLIENT_PERSON_NAME_OPTION, setting.index);

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_PERSON_BLOCK_OPTION, setting.index);
			setting.block = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_PERSON_BLOCK_ACTIVATED_OPTION, setting.index);
			setting.block_activated = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_PERSON_SCH_BLOCK_ACTIVATED_OPTION, setting.index);
			setting.schedule_block_activated = boolean;

			api_get_integer_list_option2(setting.device_index, MAX_CLIENT_DEVICE, CLIENT_PERSON_DEVICES_OPTION, setting.index);

			for(i = 0; i < MAX_CLIENT_DEVICE && setting.device_index[i] > 0; i++)
			{
				api_get_string_option2(setting.device_mac[i], 32, CLIENT_DEVICE_MAC_OPTION, setting.device_index[i]);
			}

			api_get_integer_list_option2(setting.rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_PERSON_RULES_OPTION, setting.index);
		}

		get_mesh_user_profile_json_cb(pkt, setting, OK_STR);
	}

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_user_profile_list_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_user_profile_list_cb(HTTPS_CB *pkt)
{
	char *query_str;
	MESH_USER_PROFILE_T setting[MAX_CLIENT_PERSON];
	int i, j;
	int boolean;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(setting, 0x00, sizeof(setting));

		for(i = 1, j = 0; i <= MAX_CLIENT_PERSON; i++)
		{
			api_get_string_option2(setting[j].name, sizeof(setting[j].name), CLIENT_PERSON_NAME_OPTION, i);
			if(0 == strlen(setting[j].name))
			{
				continue;
			}

			setting[j].index = i;

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_PERSON_BLOCK_OPTION, i);
			setting[j].block = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_PERSON_BLOCK_ACTIVATED_OPTION, i);
			setting[j].block_activated = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_PERSON_SCH_BLOCK_ACTIVATED_OPTION, i);
			setting[j].schedule_block_activated = boolean;

			api_get_integer_list_option2(setting[j].device_index, MAX_CLIENT_DEVICE, CLIENT_PERSON_DEVICES_OPTION, i);
			api_get_integer_list_option2(setting[j].rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_PERSON_RULES_OPTION, i);

			j++;
		}

		get_mesh_user_profile_list_json_cb(pkt, setting, OK_STR);
	}

	return 0;
}

/*****************************************************************
* NAME:    set_mesh_user_profile_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int set_mesh_user_profile_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	MESH_USER_PROFILE_T setting;
	bool existed;
	int i, j, k, l;
	int empty_index;
	char name[32];
	char buf[256];
	int device_index[MAX_CLIENT_DEVICE];
	int owner_rule_index[MAX_FIREWALL_EBTABLES_RULE];
	int rule_index[MAX_FIREWALL_EBTABLES_RULE];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;
	empty_index = 0;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		memset(&setting, 0x00, sizeof(setting));
		parse_mesh_user_profile_json_cb(query_str, &setting, &return_str);

		if(0 == setting.index)
		{
			for(i = 1; i <= MAX_CLIENT_PERSON; i++)
			{
				memset(buf, 0x00, sizeof(buf));
				api_get_string_option2(buf, sizeof(buf), CLIENT_PERSON_NAME_OPTION, i);

				if(0 == strlen(buf))
				{
					setting.index = i;
					break;
				}
			}
		}

		if(setting.index > 0 && setting.index <= MAX_CLIENT_PERSON)
		{
			if(0 != strlen(setting.name))
			{
				api_set_string_option2(setting.name, sizeof(setting.name), CLIENT_PERSON_NAME_OPTION, setting.index);
			}
#if 1
			/* Reset the CLIENT_PERSON_DEVICES_OPTION */
			memset(device_index, 0x00, sizeof(device_index));
			memset(owner_rule_index, 0x00, sizeof(owner_rule_index));
			api_get_integer_list_option2(device_index, MAX_CLIENT_DEVICE, CLIENT_PERSON_DEVICES_OPTION, setting.index);
			api_get_integer_list_option2(owner_rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_PERSON_RULES_OPTION, setting.index);
			for(i = 0; 0 != device_index[i] && i < MAX_CLIENT_DEVICE; i++)
			{
				api_delete_option2("", CLIENT_DEVICE_OWNER_OPTION, device_index[i]);
				api_delete_option2("", CLIENT_DEVICE_OWNER_INDEX_OPTION, device_index[i]);

				// Remove the original firewall rules which belong to the owner
				memset(rule_index, 0x00, sizeof(rule_index));
				api_get_integer_list_option2(rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_DEVICE_RULES_OPTION, device_index[i]);

				for(j = 0; j < MAX_FIREWALL_EBTABLES_RULE && owner_rule_index[j]; j++)
				{
					for(k = 0; k < MAX_FIREWALL_EBTABLES_RULE && rule_index[k]; k++)
					{
						if(owner_rule_index[j] == rule_index[k])
						{
							api_del_integer_list2(rule_index[k], CLIENT_DEVICE_RULES_OPTION, device_index[i]);
						}
					}
				}
			}
			api_delete_option2("", CLIENT_PERSON_DEVICES_OPTION, setting.index);

			for(i = 0; 0 != strlen(setting.device_mac[i]) && i < MAX_CLIENT_DEVICE; i++)
			{
				existed = FALSE;
				empty_index = 0;

				/* Search all the devices. */
				for(j = 1; j <= MAX_CLIENT_DEVICE; j++)
				{
					memset(buf, 0x00, sizeof(buf));
					api_get_string_option2(buf, sizeof(buf), CLIENT_DEVICE_MAC_OPTION, j);
					if(0 == strlen(buf))
					{
						if(0 == empty_index)
						{
							empty_index = j;
						}

						continue;
					}

					if(0 == strcasecmp(buf, setting.device_mac[i]))
					{
						api_set_string_option2(setting.name, sizeof(setting.name), CLIENT_DEVICE_OWNER_OPTION, j);
						api_set_integer_option2(setting.index, CLIENT_DEVICE_OWNER_INDEX_OPTION, j);

						/* Check if the device belongs to the user originally. */
						existed = FALSE;

						for(k = 0; (k < MAX_CLIENT_DEVICE) && (0 != device_index[k]); k++)
						{
							if(j == device_index[k])
							{
								existed = TRUE;
								break;
							}
						}

						/* The device does not belongs to the user originally. */
						if(FALSE == existed)
						{
							/* Reset the setting of firewall rule to new user's rules. */
							api_delete_option2("", CLIENT_DEVICE_RULES_OPTION, j);

							for(k = 0; 0 != setting.rule_index[k] && k < MAX_FIREWALL_EBTABLES_RULE; k++)
							{
								api_add_integer_list2(setting.rule_index[k], CLIENT_DEVICE_RULES_OPTION, j);
							}
						}
						else
						{
							/* Keep the firewall rules which belong to the client and reset the user's new rules. */
							memset(rule_index, 0x00, sizeof(rule_index));
							api_get_integer_list_option2(rule_index, MAX_CLIENT_PERSON, CLIENT_PERSON_RULES_OPTION, setting.index);

							/* Remove the rule which is deleted from user's rule list. */
							for(k = 0; 0 != rule_index[k] && k < MAX_FIREWALL_EBTABLES_RULE; k++)
							{
								existed = FALSE;

								for(l = 0; 0 != setting.rule_index[l] && l < MAX_FIREWALL_EBTABLES_RULE; l++)
								{
									if(rule_index[k] == setting.rule_index[l])
									{
										existed = TRUE;
										break;
									}
								}

								if(FALSE == existed)
								{
									api_del_integer_list2(setting.rule_index[k], CLIENT_DEVICE_RULES_OPTION, j);
								}
							}

							memset(rule_index, 0x00, sizeof(rule_index));
							api_get_integer_list_option2(rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_DEVICE_RULES_OPTION, j);

							/* Set the rule which is added into user's rule list. */
							for(k = 0; 0 != setting.rule_index[k] && k < MAX_FIREWALL_EBTABLES_RULE; k++)
							{
								existed = FALSE;

								for(l = 0; 0 != rule_index[l] && l < MAX_FIREWALL_EBTABLES_RULE; l++)
								{
									if(setting.rule_index[k] == rule_index[l])
									{
										existed = TRUE;
										break;
									}
								}

								if(FALSE == existed)
								{
									api_add_integer_list2(setting.rule_index[k], CLIENT_DEVICE_RULES_OPTION, j);
								}
							}
						}

						existed = TRUE;

						break;
					}
				}

				if(FALSE == existed)
				{
					j = empty_index;

					memset(buf, 0x00, sizeof(buf));
					memset(name, 0x00, sizeof(name));
					sys_interact(buf, sizeof(buf), "cat /tmp/finger_device_list | grep -i \"%s\"", setting.device_mac[i]);
					sscanf(buf, "%*s\t%*s\t%*s\t%*[^\t]\t%[^\t]\t%*[^\n]\n", name);

					api_set_string_option2(name, sizeof(name), CLIENT_DEVICE_NAME_OPTION, j);
					api_set_string_option2(setting.device_mac[i], sizeof(setting.device_mac[i]), CLIENT_DEVICE_MAC_OPTION, j);
					api_set_integer_option2(setting.index, CLIENT_DEVICE_OWNER_INDEX_OPTION, j);

					api_set_string_option2(setting.name, sizeof(setting.name), CLIENT_DEVICE_OWNER_OPTION, j);

					for(k = 0; 0 != setting.rule_index[k] && k < MAX_FIREWALL_EBTABLES_RULE; k++)
					{
						api_add_integer_list2(setting.rule_index[k], CLIENT_DEVICE_RULES_OPTION, j);
					}
				}

				if(j <= MAX_CLIENT_DEVICE)
				{
					api_add_integer_list2(j, CLIENT_PERSON_DEVICES_OPTION, setting.index);
				}
			}

			/* Reset the CLIENT_PERSON_RULES_OPTION */
			api_delete_option2("", CLIENT_PERSON_RULES_OPTION, setting.index);

			for(i = 0; 0 != setting.rule_index[i] && i < MAX_FIREWALL_EBTABLES_RULE; i++)
			{
				api_add_integer_list2(setting.rule_index[i], CLIENT_PERSON_RULES_OPTION, setting.index);
			}
#else
			/* Reset the CLIENT_PERSON_DEVICES_OPTION */
			memset(device_index, 0x00, sizeof(device_index));
			api_get_integer_list_option2(device_index, MAX_CLIENT_DEVICE, CLIENT_PERSON_DEVICES_OPTION, setting.index);
			for(i = 0; 0 != device_index[i] && i < MAX_CLIENT_DEVICE; i++)
			{
				APPAGENT_SYSTEM("uci delete "CLIENT_DEVICE_OWNER_OPTION, device_index[i]);
				APPAGENT_SYSTEM("uci delete "CLIENT_DEVICE_OWNER_INDEX_OPTION, device_index[i]);
			}
			APPAGENT_SYSTEM("uci delete "CLIENT_PERSON_DEVICES_OPTION, setting.index);

			for(i = 0; 0 != strlen(setting.device_mac[i]) && i < MAX_CLIENT_DEVICE; i++)
			{
				existed = FALSE;
				empty_index = 0;

				/* Search all the devices. */
				for(j = 1; j <= MAX_CLIENT_DEVICE; j++)
				{
					memset(buf, 0x00, sizeof(buf));
					api_get_string_option2(buf, sizeof(buf), CLIENT_DEVICE_MAC_OPTION, j);
					if(0 == strlen(buf))
					{
						if(0 == empty_index)
						{
							empty_index = j;
						}

						continue;
					}

					if(0 == strcasecmp(buf, setting.device_mac[i]))
					{
						existed = TRUE;

						memset(buf, 0x00, sizeof(buf));
						sysCheckSingleQuoteOnString(setting.name, buf);
						APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_OWNER_OPTION"='%s'", j, buf);
						APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_OWNER_INDEX_OPTION"=%d", j, setting.index);
						/* Reset the setting of firewall rule to new user's rules. */
						APPAGENT_SYSTEM("uci delete "CLIENT_DEVICE_RULES_OPTION, j);
						break;
					}
				}

				if(FALSE == existed)
				{
					j = empty_index;
					
					memset(buf, 0x00, sizeof(buf));
					memset(name, 0x00, sizeof(name));
					sys_interact(buf, sizeof(buf), "cat /tmp/finger_device_list | grep -i \"%s\"", setting.device_mac[i]);
					sscanf(buf, "%*s\t%*s\t%*s\t%*[^\t]\t%[^\t]\t%*[^\n]\n", name);

					memset(buf, 0x00, sizeof(buf));
					sysCheckSingleQuoteOnString(name, buf);
					APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_NAME_OPTION"='%s'", j, buf);
					APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_MAC_OPTION"=%s", j, setting.device_mac[i]);
					APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_OWNER_INDEX_OPTION"=%d", j, setting.index);

					memset(buf, 0x00, sizeof(buf));
					sysCheckSingleQuoteOnString(setting.name, buf);
					APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_OWNER_OPTION"='%s'", j, buf);
				}

				if(j <= MAX_CLIENT_DEVICE)
				{
					APPAGENT_SYSTEM("uci add_list "CLIENT_PERSON_DEVICES_OPTION"=%d", setting.index, j);

					for(k = 0; 0 != setting.rule_index[k]; k++)
					{
						APPAGENT_SYSTEM("uci add_list "CLIENT_DEVICE_RULES_OPTION"=%d", j, setting.rule_index[k]);
					}
				}
			}

			/* Reset the CLIENT_PERSON_RULES_OPTION */
			APPAGENT_SYSTEM("uci delete "CLIENT_PERSON_RULES_OPTION, setting.index);

			for(i = 0; 0 != setting.rule_index[i]; i++)
			{
				APPAGENT_SYSTEM("uci add_list "CLIENT_PERSON_RULES_OPTION"=%d", setting.index, setting.rule_index[i]);
			}
#endif

			APPAGENT_SYSTEM("uci commit client");
			APPAGENT_SYSTEM("luci-reload client firewall sfe qos upnpd sncrontab nat-traversal &");

			return_str = OK_STR;
		}
		else
		{
			return_str = ERROR_FULL_TABLE_STR;
		}
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    set_mesh_user_profile_list_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int set_mesh_user_profile_list_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	MESH_USER_PROFILE_T setting[MAX_CLIENT_PERSON];
	bool existed;
	int i, j, k, l;
	int index, empty_index;
	char name[32];
	char buf[256];
	int device_index[MAX_CLIENT_DEVICE];
	int device_index2[MAX_CLIENT_DEVICE];
	int rule_index[MAX_FIREWALL_EBTABLES_RULE];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;
	empty_index = 0;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		memset(&setting, 0x00, sizeof(setting));
		parse_mesh_user_profile_list_json_cb(query_str, setting, &return_str);

		for(i = 0; setting[i].index; i++)
		{
			if(0 != strlen(setting[i].name))
			{
				api_set_string_option2(setting[i].name, sizeof(setting[i].name), CLIENT_PERSON_NAME_OPTION, setting[i].index);
			}

			memset(rule_index, 0x00, sizeof(rule_index));
			api_get_integer_list_option2(rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_PERSON_RULES_OPTION, setting[i].index);

			/* Get the devices which belong to the user. Remove their owner and owner index. */
			memset(device_index, 0x00, sizeof(device_index));
			api_get_integer_list_option2(device_index, MAX_CLIENT_DEVICE, CLIENT_PERSON_DEVICES_OPTION, setting[i].index);

			for(j = 0; 0 != device_index[j] && j < MAX_CLIENT_DEVICE; j++)
			{
				memset(buf, 0x00, sizeof(buf));
				api_get_string_option2(buf, sizeof(buf), CLIENT_DEVICE_MAC_OPTION, device_index[j]);
				existed = FALSE;

				/* If the device exists in the setting table, its owner and owner index are deleted.
				 * If the device does not exists in the setting table, its all configuration in uci are deleted. */
				for(k = 0; setting[k].index && (FALSE == existed); k++)
				{
					for(l = 0; strlen(setting[k].device_mac[l]); l++)
					{
						if(0 == strcasecmp(buf, setting[k].device_mac[l]))
						{
							existed = TRUE;
							break;
						}
					}
				}

				if(TRUE == existed)
				{
					api_delete_option2("", CLIENT_DEVICE_OWNER_OPTION, device_index[j]);
					api_delete_option2("", CLIENT_DEVICE_OWNER_INDEX_OPTION, device_index[j]);
				}
				else
				{
					api_delete_option2("", CLIENT_DEVICE_NAME_OPTION, device_index[j]);
					api_delete_option2("", CLIENT_DEVICE_OWNER_OPTION, device_index[j]);
					api_delete_option2("", CLIENT_DEVICE_OWNER_INDEX_OPTION, device_index[j]);
					api_delete_option2("", CLIENT_DEVICE_MAC_OPTION, device_index[j]);
					api_delete_option2("", CLIENT_DEVICE_BLOCK_OPTION, device_index[j]);
					api_delete_option2("", CLIENT_DEVICE_START_DATE_OPTION, device_index[j]);
					api_delete_option2("", CLIENT_DEVICE_BLOCK_TIME_OPTION, device_index[j]);
					api_delete_option2("", CLIENT_DEVICE_RULES_OPTION, device_index[j]);
					api_delete_option2("", CLIENT_DEVICE_BLOCK_ACTIVATED_OPTION, device_index[j]);
					api_delete_option2("", CLIENT_DEVICE_SCH_BLOCK_ACTIVATED_OPTION, device_index[j]);
				}
			}

			/* Reset the CLIENT_PERSON_DEVICES_OPTION */
			api_delete_option2("", CLIENT_PERSON_DEVICES_OPTION, setting[i].index);

			for(j = 0; 0 != strlen(setting[i].device_mac[j]) && j < MAX_CLIENT_DEVICE; j++)
			{
				existed = FALSE;
				empty_index = 0;

				/* If the device exists in uci, its configuration is updated.
                 * If the device does not exist in uci, its configuration is set to an empty section. */
				for(k = 1; k <= MAX_CLIENT_DEVICE; k++)
				{
					memset(buf, 0x00, sizeof(buf));
					api_get_string_option2(buf, sizeof(buf), CLIENT_DEVICE_MAC_OPTION, k);
					if(0 == strlen(buf))
					{
						if(0 == empty_index)
						{
							empty_index = k;
						}

						continue;
					}

					if(0 == strcasecmp(buf, setting[i].device_mac[j]))
					{
						existed = TRUE;
                        index = 0;

						/* Remove the device from the original user's list. */
						api_get_integer_option2(&index, CLIENT_DEVICE_OWNER_INDEX_OPTION, k);
						if(index)
						{
							api_del_integer_list2(k, CLIENT_PERSON_DEVICES_OPTION, index);
						}

						api_set_string_option2(setting[i].name, sizeof(setting[i].name), CLIENT_DEVICE_OWNER_OPTION, k);
						api_set_integer_option2(setting[i].index, CLIENT_DEVICE_OWNER_INDEX_OPTION, k);

						api_delete_option2("", CLIENT_DEVICE_RULES_OPTION, k);

#if 0
						for(l = 0; 0 != setting[i].rule_index[l]; l++)
						{
							APPAGENT_SYSTEM("uci add_list "CLIENT_DEVICE_RULES_OPTION"=%d", k, setting[i].rule_index[l]);
						}
#else
						for(l = 0; 0 != rule_index[l] && l < MAX_FIREWALL_EBTABLES_RULE; l++)
						{
							api_add_integer_list2(rule_index[l], CLIENT_DEVICE_RULES_OPTION, k);
						}
#endif

						break;
					}
				}

				if(FALSE == existed)
				{
					k = empty_index;

					memset(buf, 0x00, sizeof(buf));
					memset(name, 0x00, sizeof(name));
					sys_interact(buf, sizeof(buf), "cat /tmp/finger_device_list | grep -i \"%s\"", setting[i].device_mac[j]);
					sscanf(buf, "%*s\t%*s\t%*s\t%*[^\t]\t%[^\t]\t%*[^\n]\n", name);

					api_set_string_option2(name, sizeof(name), CLIENT_DEVICE_NAME_OPTION, k);
					api_set_string_option2(setting[i].device_mac[j], sizeof(setting[i].device_mac[j]), CLIENT_DEVICE_MAC_OPTION, k);
					api_set_integer_option2(setting[i].index, CLIENT_DEVICE_OWNER_INDEX_OPTION, k);

					api_set_string_option2(setting[i].name, sizeof(setting[i].name), CLIENT_DEVICE_OWNER_OPTION, k);

#if 0
					for(l = 0; 0 != setting[i].rule_index[l]; l++)
					{
						APPAGENT_SYSTEM("uci add_list "CLIENT_DEVICE_RULES_OPTION"=%d", k, setting[i].rule_index[l]);
					}
#else
						for(l = 0; 0 != rule_index[l] && l < MAX_FIREWALL_EBTABLES_RULE; l++)
						{
							api_add_integer_list2(rule_index[l], CLIENT_DEVICE_RULES_OPTION, k);
						}
#endif
				}

				if(k <= MAX_CLIENT_DEVICE)
				{
					api_add_integer_list2(k, CLIENT_PERSON_DEVICES_OPTION, setting[i].index);
				}
			}

#if 0
			/* Reset the CLIENT_PERSON_RULES_OPTION */
			APPAGENT_SYSTEM("uci delete "CLIENT_PERSON_RULES_OPTION, setting[i].index);

			for(j = 0; 0 != setting[i].rule_index[j]; j++)
			{
				api_add_integer_list2(setting[i].rule_index[j], CLIENT_PERSON_RULES_OPTION, setting[i].index);
			}
#endif
		}

		APPAGENT_SYSTEM("uci commit client");
		APPAGENT_SYSTEM("luci-reload client firewall sfe qos upnpd sncrontab nat-traversal &");
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    delete_mesh_user_profile_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int delete_mesh_user_profile_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	int  i, j, k, index;
	int  device_index[MAX_CLIENT_DEVICE];
	int  rule_index[MAX_FIREWALL_EBTABLES_RULE];
	int  rule_index2[MAX_FIREWALL_EBTABLES_RULE];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		get_json_integer_from_query(query_str, &index, "Index");

		memset(device_index, 0x00, sizeof(device_index));
		memset(rule_index, 0x00, sizeof(rule_index));
		api_get_integer_list_option2(device_index, MAX_CLIENT_DEVICE, CLIENT_PERSON_DEVICES_OPTION, index);
		api_get_integer_list_option2(rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_PERSON_RULES_OPTION, index);

		for(i = 0; 0 != device_index[i] && i < MAX_CLIENT_DEVICE; i++)
		{
			api_delete_option2("", CLIENT_DEVICE_OWNER_INDEX_OPTION, device_index[i]);
			api_delete_option2("", CLIENT_DEVICE_OWNER_OPTION, device_index[i]);

			/* Get the original rules and remove the rules which are in user's rules. */
			memset(rule_index2, 0x00, sizeof(rule_index2));
			api_get_integer_list_option2(rule_index2, MAX_FIREWALL_EBTABLES_RULE, CLIENT_DEVICE_RULES_OPTION, device_index[i]);

			for(j = 0; 0 != rule_index[j] && j < MAX_FIREWALL_EBTABLES_RULE; j++)
			{
				for(k = 0; 0 != rule_index2[k] && k < MAX_FIREWALL_EBTABLES_RULE; k++)
				{
					if(rule_index2[k] == rule_index[j])
					{
						api_del_integer_list2(rule_index[j], CLIENT_DEVICE_RULES_OPTION, device_index[i]);
						break;
					}
				}
			}

			memset(rule_index2, 0x00, sizeof(rule_index2));
			api_get_integer_list_option2(rule_index2, MAX_FIREWALL_EBTABLES_RULE, CLIENT_DEVICE_RULES_OPTION, device_index[i]);
			for(j = 0; 0 == rule_index2[j] && j < MAX_FIREWALL_EBTABLES_RULE; j++);
			if(MAX_FIREWALL_EBTABLES_RULE == j)
			{
				api_delete_option2("", CLIENT_DEVICE_RULES_OPTION, device_index[i]);
			}
		}

		api_delete_option2("", CLIENT_PERSON_NAME_OPTION, index);
		api_delete_option2("", CLIENT_PERSON_BLOCK_OPTION, index);
		api_delete_option2("", CLIENT_PERSON_BLOCK_ACTIVATED_OPTION, index);
		api_delete_option2("", CLIENT_PERSON_SCH_BLOCK_ACTIVATED_OPTION, index);
		api_delete_option2("", CLIENT_PERSON_START_DATE_OPTION, index);
		api_delete_option2("", CLIENT_PERSON_BLOCK_TIME_OPTION, index);
		api_delete_option2("", CLIENT_PERSON_DEVICES_OPTION, index);
		api_delete_option2("", CLIENT_PERSON_RULES_OPTION, index);
		api_delete_option2("", "client.Person%d.peopleIdx", index);

		APPAGENT_SYSTEM("uci commit client");
		APPAGENT_SYSTEM("luci-reload client firewall sfe qos upnpd sncrontab nat-traversal &");

		return_str = OK_STR;
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_client_profile_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_client_profile_cb(HTTPS_CB *pkt)
{
	char *query_str;
	int boolean;
	int atf_enable;
	int index;
#if 0
	int i;
#endif
	char *result;
	char mac1[32];
	char mac2[32];
	char buf[256];
	char *ptr;
	char mac_list[MAX_CLIENT_DEVICE][32];
	MESH_CLIENT_PROFILE_T setting;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	result = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(&setting, 0x00, sizeof(setting));
		memset(buf, 0x00, sizeof(buf));
		memset(mac1, 0x00, sizeof(mac1));
		memset(mac2, 0x00, sizeof(mac2));

		get_json_string_from_query(query_str, mac1, "MAC");

		for(index = 1; index <= MAX_CLIENT_DEVICE; index++)
		{
			api_get_string_option2(mac2, sizeof(mac2), CLIENT_DEVICE_MAC_OPTION, index);

			if(0 == strcasecmp(mac1, mac2))
			{
				break;
			}
		}

		if(index <= MAX_CLIENT_DEVICE)
		{
			setting.index = index;
			api_get_string_option2(setting.name, sizeof(setting.name), CLIENT_DEVICE_NAME_OPTION, index);

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_DEVICE_BLOCK_OPTION, index);
			setting.block = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_DEVICE_SCHEDULE_BLOCK_OPTION, index);
			setting.schedule_block = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_DEVICE_BLOCK_ACTIVATED_OPTION, index);
			setting.block_activated = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_DEVICE_SCH_BLOCK_ACTIVATED_OPTION, index);
			setting.schedule_block_activated = boolean;


			if(setting.block)
			{
				memset(buf, 0x00, sizeof(buf));
				api_get_string_option2(buf, sizeof(buf), CLIENT_DEVICE_START_DATE_OPTION, index);
				sys_interact(setting.start_date, sizeof(setting.start_date), "date -d @%s +\"%%Y-%%m-%%d %%H:%%M\"", buf);

				ptr = strchr(setting.start_date, '\n');

				if(NULL != ptr)
				{
					*ptr = '\0';
				}

				api_get_string_option2(setting.block_time, sizeof(setting.block_time), CLIENT_DEVICE_BLOCK_TIME_OPTION, index);
			}

			api_get_integer_option2(&setting.from_hour, CLIENT_DEVICE_FROM_HOUR_OPTION, index);
			api_get_integer_option2(&setting.from_minute, CLIENT_DEVICE_FROM_MINUTE_OPTION, index);
			api_get_integer_option2(&setting.to_hour, CLIENT_DEVICE_TO_HOUR_OPTION, index);
			api_get_integer_option2(&setting.to_minute, CLIENT_DEVICE_TO_MINUTE_OPTION, index);
			api_get_string_option2(setting.week_day, sizeof(setting.week_day), CLIENT_DEVICE_WEEK_DAY_OPTION, index);

			api_get_string_option2(setting.mac, sizeof(setting.mac), CLIENT_DEVICE_MAC_OPTION, index);
			api_get_integer_option2(&setting.owner_index, CLIENT_DEVICE_OWNER_INDEX_OPTION, index);
			api_get_string_option2(setting.owner, sizeof(setting.owner), CLIENT_DEVICE_OWNER_OPTION, index);
			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_PERSON_BLOCK_OPTION, setting.owner_index);
			setting.owner_block = boolean;

			api_get_integer_list_option2(setting.owner_rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_PERSON_RULES_OPTION, setting.owner_index);

			api_get_integer_list_option2(setting.rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_DEVICE_RULES_OPTION, index);

			atf_enable = 0;
			api_get_bool_option("airtime_fairness.atf.enabled", &atf_enable);

			if(atf_enable)
			{
				api_get_string_length_32_list_option("airtime_fairness.atf.vip_list", mac_list, MAX_CLIENT_DEVICE);

#if 1
				setting.high_quality = check_mac_is_in_list(setting.mac, mac_list);
#else
				for(i = 0; i < MAX_CLIENT_DEVICE && 0 != strlen(mac_list[i]); i++)
				{
					if(0 == strcasecmp(setting.mac, mac_list[i]))
					{
						setting.high_quality = TRUE;
						break;
					}
				}
#endif
			}

			memset(buf, 0x00, sizeof(buf));
			sys_interact(buf, sizeof(buf), "cat /tmp/finger_device_list | grep -i \"%s\"", mac1);

			/* 1   aa:bb:cc:dd:ee:e1   192.168.0.2   Apple iOS   iPhone   ---   ---    ---   ---   Huddle7453 */
			if(strlen(buf))
			{
				sscanf(buf, "%*s\t%*s\t%s\t%*[^\t]\t%*[^\t]\t%*[^\t]\t%*s\t%*s\t%*s\t%[^\n]\n",
						setting.ip, setting.connected_host_name);
				setting.status = atoi(&buf[0]);
			}

			result = OK_STR;
		}
		else
		{
			/* cat /tmp/finger_device_list */
			/* 1       00:aa:bb:cc:dd:e1       192.168.0.100   Apple iOS       iPhone          HUDDLE    -36     43300Kb 1941Kb   Huddle7453 */
			sys_interact(buf, sizeof(buf), "cat /tmp/finger_device_list | grep -i \"%s\"", mac1);
			sscanf(buf, "%*s\t%s\t%s\t%*[^\t]\t%[^\t]\t%*[^\t]\t%*s\t%*s\t%*s\t%[^\n]\n",
					setting.mac, setting.ip, setting.name, setting.connected_host_name);

			setting.status = atoi(&buf[0]);

			result = OK_STR;
		}

		get_mesh_client_profile_json_cb(pkt, setting, result);
	}

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_client_profile_list_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_client_profile_list_cb(HTTPS_CB *pkt)
{
	char *query_str;
	int i, j, k;
	int boolean;
	int atf_enable;
	char *result;
	char buf[128];
	char mac_list[MAX_CLIENT_DEVICE][32];
	MESH_CLIENT_PROFILE_T setting[MAX_CLIENT_DEVICE];
	MESH_CLIENT_PROFILE_T tmp_setting[MAX_CLIENT_DEVICE];
	FILE *fp;

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	result = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(setting, 0x00, sizeof(setting));
		memset(tmp_setting, 0x00, sizeof(tmp_setting));
		memset(mac_list, 0x00, sizeof(mac_list));

		atf_enable = 0;
		api_get_bool_option("airtime_fairness.atf.enabled", &atf_enable);

		if(atf_enable)
		{
			api_get_string_length_32_list_option("airtime_fairness.atf.vip_list", mac_list, MAX_CLIENT_DEVICE);
		}

		/* cat /tmp/finger_device_list */
		/*
		1       00:aa:bb:cc:dd:e0       192.168.0.104   Android android-1234567890123456        11111_HUDDLE    -83     3Kb      37Kb
		1       00:aa:bb:cc:dd:e1       192.168.0.102   Apple iOS       iPhone          11111_HUDDLE    -36     43300Kb 1941Kb
		1       00:aa:bb:cc:dd:e2       192.168.0.106   Windows 7/Vista/Server 2008     Desk-PC 11111_HUDDLE    -56     5Kb      14Kb
		*/
		if(NULL != (fp = fopen("/tmp/finger_device_list", "r")))
		{
			i = 0;

			while(NULL != fgets(buf, sizeof(buf), fp))
			{
				sscanf(buf, "%*s\t%s\t%s\t%*[^\t]\t%s\t%*[^\n]\n",
				        tmp_setting[i].mac, tmp_setting[i].ip, tmp_setting[i].name);
				tmp_setting[i].status = 1;
				i++;
			}

			fclose(fp);
		}

		for(i = 1, j = 0, k = 0; i <= MAX_CLIENT_DEVICE; i++)
		{
			memset(setting[j].mac, 0x00, sizeof(setting[j].mac));
			api_get_string_option2(setting[j].mac, sizeof(setting[j].mac), CLIENT_DEVICE_MAC_OPTION, i);

			if(0 == strlen(setting[j].mac))
			{
				continue;
			}

			setting[j].index = i;

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_DEVICE_BLOCK_OPTION, i);
			setting[j].block = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_DEVICE_SCHEDULE_BLOCK_OPTION, i);
			setting[j].schedule_block = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_DEVICE_BLOCK_ACTIVATED_OPTION, i);
			setting[j].block_activated = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_DEVICE_SCH_BLOCK_ACTIVATED_OPTION, i);
			setting[j].schedule_block_activated = boolean;

			api_get_string_option2(setting[j].name, sizeof(setting[j].name), CLIENT_DEVICE_NAME_OPTION, i);
			api_get_integer_option2(&setting[j].owner_index, CLIENT_DEVICE_OWNER_INDEX_OPTION, i);
			api_get_string_option2(setting[j].owner, sizeof(setting[j].owner), CLIENT_DEVICE_OWNER_OPTION, i);

			api_get_integer_list_option2(setting[j].owner_rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_PERSON_RULES_OPTION, setting[j].owner_index);

			api_get_integer_list_option2(setting[j].rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_DEVICE_RULES_OPTION, j+1);

			k = 0;

			for(k = 0; k < MAX_CLIENT_DEVICE; k++)
			{
				if(0 == strcasecmp(tmp_setting[k].mac, setting[j].mac))
				{
					sprintf(setting[j].ip, "%s\0", tmp_setting[k].ip);
					setting[j].status = 1;

					memset(&tmp_setting[k], 0x00, sizeof(MESH_CLIENT_PROFILE_T));
					break;
				}
			}

			if(atf_enable)
			{
#if 1
				setting[j].high_quality = check_mac_is_in_list(setting[j].mac, mac_list);
#else
				for(k = 0; k < MAX_CLIENT_DEVICE && 0 != strlen(mac_list[k]); k++)
				{
					if(0 == strcasecmp(setting[j].mac, mac_list[k]))
					{
						setting[j].high_quality = TRUE;
						break;
					}
				}
#endif
			}

			j++;
		}

		for(i = 0, j = 0; i < MAX_CLIENT_DEVICE; i++)
		{
			if(0 == strlen(setting[i].mac))
			{
				for(; j < MAX_CLIENT_DEVICE; j++)
				{
					if(0 != strlen(tmp_setting[j].mac))
					{
						break;
					}
				}

				/* No extra device */
				if(MAX_CLIENT_DEVICE == j)
				{
					break;
				}

				memcpy(&setting[i], &tmp_setting[j], sizeof(MESH_CLIENT_PROFILE_T));
				j++;
			}
		}

		result = OK_STR;

		get_mesh_client_profile_list_json_cb(pkt, setting, result);
	}

	return 0;
}

/*****************************************************************
* NAME:    set_mesh_client_profile_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int set_mesh_client_profile_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	MESH_CLIENT_PROFILE_T setting;
	bool existed;
	int i, j;
	int empty_index;
	int atf_enable;
	int device_index[MAX_CLIENT_DEVICE];
	char buf[64];
	char mac_list[MAX_CLIENT_DEVICE][32];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;
	existed = FALSE;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		memset(&setting, 0x00, sizeof(setting));
		memset(mac_list, 0x00, sizeof(mac_list));

		parse_mesh_client_profile_json_cb(query_str, &setting, &return_str);

		if(0 == setting.index)
		{
			for(i = 1, empty_index = 0; i <= MAX_CLIENT_DEVICE; i++)
			{
				memset(buf, 0x00, sizeof(buf));
				api_get_string_option2(buf, sizeof(buf), CLIENT_DEVICE_MAC_OPTION, i);

				if(0 == strlen(buf))
				{
					if(0 == empty_index)
					{
						empty_index = i;
					}

					continue;
				}

				if(0 == strcasecmp(buf, setting.mac))
				{
					break;
				}
			}

			if(i <= MAX_CLIENT_DEVICE)
			{
				setting.index = i;
			}
			else
			{
				setting.index = empty_index;
			}
		}

		if(setting.index <= MAX_CLIENT_DEVICE)
		{
			api_set_string_option2(setting.name, sizeof(setting.name), CLIENT_DEVICE_NAME_OPTION, setting.index);
			api_set_string_option2(setting.mac, sizeof(setting.mac), CLIENT_DEVICE_MAC_OPTION, setting.index);

			if(0 != setting.owner_index)
			{
				for(i = 1; i <= MAX_CLIENT_DEVICE; i++)
				{
					memset(device_index, 0x00, sizeof(device_index));
					api_get_integer_list_option2(device_index, MAX_CLIENT_DEVICE, CLIENT_PERSON_DEVICES_OPTION, i);

					for(j = 0; j < MAX_CLIENT_DEVICE && 0 != device_index[j]; j++)
					{
						if(device_index[j] == setting.index)
						{
							existed = TRUE;
							break;
						}
					}

					if(existed == TRUE)
					{
						if(i != setting.owner_index)
						{
							api_del_integer_list2(setting.index, CLIENT_PERSON_DEVICES_OPTION, i);
							existed = FALSE;
						}

						break;
					}
				}

				if(FALSE == existed)
				{
					api_add_integer_list2(setting.index, CLIENT_PERSON_DEVICES_OPTION, setting.owner_index);
				}

				api_get_string_option2(setting.owner, sizeof(setting.owner), CLIENT_PERSON_NAME_OPTION, setting.owner_index);
				api_set_integer_option2(setting.owner_index, CLIENT_DEVICE_OWNER_INDEX_OPTION, setting.index);
				api_set_string_option2(setting.owner, sizeof(setting.owner), CLIENT_DEVICE_OWNER_OPTION, setting.index);
			}

			existed = FALSE;
			atf_enable = 0;
			api_get_bool_option("airtime_fairness.atf.enabled", &atf_enable);

			if(atf_enable)
			{
				api_get_string_length_32_list_option("airtime_fairness.atf.vip_list", mac_list, MAX_CLIENT_DEVICE);

#if 1
				existed = check_mac_is_in_list(setting.mac, mac_list);
#else
				for(i = 0; i < MAX_CLIENT_DEVICE && 0 != strlen(mac_list[i]); i++)
				{
					if(0 == strcasecmp(setting.mac, mac_list[i]))
					{
						existed = TRUE;
						break;
					}
				}
#endif
			}

			if(TRUE == setting.high_quality)
			{
				api_set_bool_option("airtime_fairness.atf.enabled", 1);

				if(FALSE == existed)
				{
					api_add_string_list2(setting.mac, sizeof(setting.mac), "airtime_fairness.atf.vip_list");
					api_set_bool_option("airtime_fairness.atf.parameter_changed", 1);
				}
			}
			else
			{
				if(TRUE == existed)
				{
					api_del_string_list2(setting.mac, sizeof(setting.mac), "airtime_fairness.atf.vip_list");
					api_set_bool_option("airtime_fairness.atf.parameter_changed", 1);
				}
			}

			api_delete_option2("", CLIENT_DEVICE_RULES_OPTION, setting.index);

			for(i = 0; i < MAX_FIREWALL_EBTABLES_RULE && 0 != setting.rule_index[i]; i++)
			{
				api_add_integer_list2(setting.rule_index[i], CLIENT_DEVICE_RULES_OPTION, setting.index);
			}

			APPAGENT_SYSTEM("uci commit client");
			APPAGENT_SYSTEM("uci commit airtime_fairness");
			APPAGENT_SYSTEM("cat /etc/config/airtime_fairness");
			APPAGENT_SYSTEM("uci show airtime_fairness");
			APPAGENT_SYSTEM("luci-reload client firewall sfe qos upnpd sncrontab nat-traversal &");
		}
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    delete_mesh_client_profile_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int delete_mesh_client_profile_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	int i, j;
	int index;
	int device_index[MAX_CLIENT_DEVICE];
	char mac[32];
	char mac_list[MAX_CLIENT_DEVICE][32];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		memset(mac, 0x00, sizeof(mac));
		memset(mac_list, 0x00, sizeof(mac_list));
		get_json_integer_from_query(query_str, &index, "Index");

		if(index >= 1 && index <= MAX_CLIENT_DEVICE)
		{
			api_get_string_option2(mac, sizeof(mac), CLIENT_DEVICE_MAC_OPTION, index);

			api_delete_option2("", CLIENT_DEVICE_NAME_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_OWNER_INDEX_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_OWNER_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_IP_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_MAC_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_BLOCK_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_START_DATE_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_BLOCK_TIME_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_RULES_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_SCHEDULE_BLOCK_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_ALL_TIME_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_FROM_HOUR_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_TO_HOUR_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_FROM_MINUTE_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_TO_MINUTE_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_WEEK_DAY_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_BLOCK_ACTIVATED_OPTION, index);
			api_delete_option2("", CLIENT_DEVICE_SCH_BLOCK_ACTIVATED_OPTION, index);

			for(i = 1; i <= MAX_CLIENT_PERSON; i++)
			{
				memset(device_index, 0x00, sizeof(device_index));
				api_get_integer_list_option2(device_index, MAX_CLIENT_DEVICE, CLIENT_PERSON_DEVICES_OPTION, i);

				for(j = 0; 0 != device_index[j] && j < MAX_CLIENT_DEVICE; j++)
				{
					if(device_index[j] == index)
					{
						api_del_integer_list2(index, CLIENT_PERSON_DEVICES_OPTION, i);
					}
				}
			}

			api_get_string_length_32_list_option("airtime_fairness.atf.vip_list", mac_list, MAX_CLIENT_DEVICE);

#if 1
			if(TRUE == check_mac_is_in_list(mac, mac_list))
			{
				api_del_string_list2(mac, sizeof(mac), "airtime_fairness.atf.vip_list");
				api_set_bool_option("airtime_fairness.atf.parameter_changed", 1);
			}
#else
			for(i = 0; i < MAX_CLIENT_DEVICE && 0 != strlen(mac_list[i]); i++)
			{
				if(0 == strcasecmp(mac, mac_list[i]))
				{
					APPAGENT_SYSTEM("uci del_list airtime_fairness.atf.vip_list=%s", mac);
					break;
				}
			}
#endif

			return_str = OK_STR;
			APPAGENT_SYSTEM("uci commit client");
			APPAGENT_SYSTEM("uci commit airtime_fairness");
			APPAGENT_SYSTEM("luci-reload client firewall sfe qos upnpd sncrontab nat-traversal &");
		}
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    delete_mesh_client_profile_list_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int delete_mesh_client_profile_list_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	int i, j, k;
	int index[MAX_CLIENT_DEVICE];
	int device_index[MAX_CLIENT_DEVICE];
	char mac[32];
	char mac_list[MAX_CLIENT_DEVICE][32];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		memset(index, 0x00, sizeof(index));
		memset(mac_list, 0x00, sizeof(mac_list));

		delete_mesh_client_profile_list_json_cb(query_str, index);

		api_get_string_length_32_list_option("airtime_fairness.atf.vip_list", mac_list, MAX_CLIENT_DEVICE);

		for(i = 0; index[i] >= 1 && index[i] <= MAX_CLIENT_DEVICE; i++)
		{
			memset(mac, 0x00, sizeof(mac));
			api_get_string_option2(mac, sizeof(mac), CLIENT_DEVICE_MAC_OPTION, index[i]);

			api_delete_option2("", CLIENT_DEVICE_NAME_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_OWNER_INDEX_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_OWNER_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_IP_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_MAC_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_BLOCK_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_START_DATE_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_BLOCK_TIME_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_RULES_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_SCHEDULE_BLOCK_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_FROM_HOUR_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_TO_HOUR_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_FROM_MINUTE_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_TO_MINUTE_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_WEEK_DAY_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_BLOCK_ACTIVATED_OPTION, index[i]);
			api_delete_option2("", CLIENT_DEVICE_SCH_BLOCK_ACTIVATED_OPTION, index[i]);

			for(j = 1; j <= MAX_CLIENT_PERSON; j++)
			{
				memset(device_index, 0x00, sizeof(device_index));
				api_get_integer_list_option2(device_index, MAX_CLIENT_DEVICE, CLIENT_PERSON_DEVICES_OPTION, j);

				for(k = 0; 0 != device_index[k] && k < MAX_CLIENT_DEVICE; k++)
				{
					if(device_index[k] == index[i])
					{
						api_del_integer_list2(index[i], CLIENT_PERSON_DEVICES_OPTION, j);
					}
				}
			}

#if 1
			if(TRUE == check_mac_is_in_list(mac, mac_list))
			{
				api_del_string_list2(mac, sizeof(mac), "airtime_fairness.atf.vip_list");
				api_set_bool_option("airtime_fairness.atf.parameter_changed", 1);
			}
#else
			for(j = 0; j < MAX_CLIENT_DEVICE && 0 != strlen(mac_list[j]); j++)
			{
				if(0 == strcasecmp(mac, mac_list[j]))
				{
					APPAGENT_SYSTEM("uci del_list airtime_fairness.atf.vip_list=%s", mac);
					break;
				}
			}
#endif
		}
		return_str = OK_STR;
		APPAGENT_SYSTEM("uci commit client");
		APPAGENT_SYSTEM("uci commit airtime_fairness");
		APPAGENT_SYSTEM("luci-reload client firewall sfe qos upnpd sncrontab nat-traversal &");
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

#if SUPPORT_FIREWALL_URL_CATEGORY
/*****************************************************************
* NAME:    get_mesh_firewall_default_url_category_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_firewall_default_url_category_cb(HTTPS_CB *pkt)
{
	get_mesh_firewall_default_url_category_json_cb(pkt);

	return 0;
}
#endif

/*****************************************************************
* NAME:    get_mesh_firewall_rule_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_firewall_rule_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	MESH_FIREWALL_RULE_T setting;
	int i, j, k;
	int boolean;
	int rule_index[MAX_FIREWALL_EBTABLES_RULE];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(&setting, 0x00, sizeof(setting));

		get_json_integer_from_query(query_str, &setting.index, "Index");

		if(setting.index > 0 && setting.index <= MAX_FIREWALL_EBTABLES_RULE)
		{
			api_get_string_option2(setting.name, sizeof(setting.name), SN_FIREWALL_EBTABLES_RULE_NAME_OPTION, setting.index);

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_BLOCK_ALL_OPTION, setting.index);
			setting.block_all = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_SCHEDULE_BLOCK_ENABLE_OPTION, setting.index);
			setting.schedule_block_enable = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_URL_BLOCK_ENABLE_OPTION, setting.index);
			setting.url_block_enable = boolean;

#if SUPPORT_FIREWALL_URL_CATEGORY
			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_BLOCK_SOCIAL_MEDIA_ENABLE_OPTION, setting.index);
			setting.block_social_media_enable = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_BLOCK_SEARCH_ENGINE_ENABLE_OPTION, setting.index);
			setting.block_search_engine_enable = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_BLOCK_VIDEO_MEDIA_ENABLE_OPTION, setting.index);
			setting.block_video_media_enable = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_BLOCK_CUSTOM_URL_ENABLE_OPTION, setting.index);
			setting.block_custom_url_enable = boolean;

			api_get_string_length_32_list_option2(setting.block_social_media_list, MAX_FIREWALL_BLOCK_ITEM, SN_FIREWALL_EBTABLES_RULE_BLOCK_SOCIAL_MEDIA_LIST_OPTION, setting.index);
			api_get_string_length_32_list_option2(setting.block_search_engine_list, MAX_FIREWALL_BLOCK_ITEM, SN_FIREWALL_EBTABLES_RULE_BLOCK_SEARCH_ENGINE_LIST_OPTION, setting.index);
			api_get_string_length_32_list_option2(setting.block_video_media_list, MAX_FIREWALL_BLOCK_ITEM, SN_FIREWALL_EBTABLES_RULE_BLOCK_VIDEO_MEDIA_LIST_OPTION, setting.index);
			api_get_string_length_32_list_option2(setting.block_custom_url_list, MAX_FIREWALL_BLOCK_ITEM, SN_FIREWALL_EBTABLES_RULE_BLOCK_CUSTOM_URL_LIST_OPTION, setting.index);
#else
			api_get_string_length_32_list_option2(setting.block_item, MAX_FIREWALL_BLOCK_ITEM, SN_FIREWALL_EBTABLES_RULE_BLOCK_ITEM_OPTION, setting.index);
#endif

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_ALL_TIME_OPTION, setting.index);
			setting.all_time = boolean;

			api_get_integer_option2(&setting.from_hour, SN_FIREWALL_EBTABLES_RULE_FROM_HOUR_OPTION, setting.index);
			api_get_integer_option2(&setting.from_minute, SN_FIREWALL_EBTABLES_RULE_FROM_MINUTE_OPTION, setting.index);
			api_get_integer_option2(&setting.to_hour, SN_FIREWALL_EBTABLES_RULE_TO_HOUR_OPTION, setting.index);
			api_get_integer_option2(&setting.to_minute, SN_FIREWALL_EBTABLES_RULE_TO_MINUTE_OPTION, setting.index);
			api_get_string_option2(setting.week_day, sizeof(setting.week_day), SN_FIREWALL_EBTABLES_RULE_WEEK_DAY_OPTION, setting.index);
			api_get_string_option2(setting.summary, sizeof(setting.summary), SN_FIREWALL_EBTABLES_RULE_SUMMARY_OPTION, setting.index);

			k = 0;
			for(i = 1; i <= MAX_CLIENT_PERSON; i++)
			{
				memset(rule_index, 0x00, sizeof(rule_index));
				api_get_integer_list_option2(rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_PERSON_RULES_OPTION, i);

				for(j = 0; 0 != rule_index[j] && j < MAX_FIREWALL_EBTABLES_RULE; j++)
				{
					if(rule_index[j] == setting.index)
					{
						setting.related_user[k] = i;
						k++;
						break;
					}
				}
			}

			return_str = OK_STR;
		}

		get_mesh_firewall_rule_json_cb(pkt, setting, return_str);
	}

	return 0;
}

/*****************************************************************
* NAME:    get_mesh_firewall_rule_list_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_firewall_rule_list_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	MESH_FIREWALL_RULE_T setting[MAX_FIREWALL_EBTABLES_RULE];
	int i, j, k, l, m;
	int boolean;
	int rule_index[MAX_FIREWALL_EBTABLES_RULE];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		memset(&setting, 0x00, sizeof(setting));

		for(i = 1, j = 0; i <= MAX_FIREWALL_EBTABLES_RULE; i++)
		{
			memset(&setting[j].name[0], 0x00, sizeof(setting[j].name));
			api_get_string_option2(&setting[j].name[0], sizeof(setting[j].name), SN_FIREWALL_EBTABLES_RULE_NAME_OPTION, i);

			/* Check empty setting. */
			if(0 == strlen(setting[j].name))
			{
				continue;
			}

			setting[j].index = i;

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_BLOCK_ALL_OPTION, i);
			setting[j].block_all = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_SCHEDULE_BLOCK_ENABLE_OPTION, i);
			setting[j].schedule_block_enable = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_URL_BLOCK_ENABLE_OPTION, i);
			setting[j].url_block_enable = boolean;

#if SUPPORT_FIREWALL_URL_CATEGORY
			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_BLOCK_SOCIAL_MEDIA_ENABLE_OPTION, i);
			setting[j].block_social_media_enable = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_BLOCK_SEARCH_ENGINE_ENABLE_OPTION, i);
			setting[j].block_search_engine_enable = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_BLOCK_VIDEO_MEDIA_ENABLE_OPTION, i);
			setting[j].block_video_media_enable = boolean;

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_BLOCK_CUSTOM_URL_ENABLE_OPTION, i);
			setting[j].block_custom_url_enable = boolean;

			api_get_string_length_32_list_option2(setting[j].block_social_media_list, MAX_FIREWALL_BLOCK_ITEM, SN_FIREWALL_EBTABLES_RULE_BLOCK_SOCIAL_MEDIA_LIST_OPTION, i);
			api_get_string_length_32_list_option2(setting[j].block_search_engine_list, MAX_FIREWALL_BLOCK_ITEM, SN_FIREWALL_EBTABLES_RULE_BLOCK_SEARCH_ENGINE_LIST_OPTION, i);
			api_get_string_length_32_list_option2(setting[j].block_video_media_list, MAX_FIREWALL_BLOCK_ITEM, SN_FIREWALL_EBTABLES_RULE_BLOCK_VIDEO_MEDIA_LIST_OPTION, i);
			api_get_string_length_32_list_option2(setting[j].block_custom_url_list, MAX_FIREWALL_BLOCK_ITEM, SN_FIREWALL_EBTABLES_RULE_BLOCK_CUSTOM_URL_LIST_OPTION, i);
#else
			api_get_string_length_32_list_option2(setting[j].block_item, MAX_FIREWALL_BLOCK_ITEM, SN_FIREWALL_EBTABLES_RULE_BLOCK_ITEM_OPTION, i);
#endif

			boolean = 0;
			api_get_bool_option2(&boolean, SN_FIREWALL_EBTABLES_RULE_ALL_TIME_OPTION, i);
			setting[j].all_time = boolean;

			api_get_integer_option2(&setting[j].from_hour, SN_FIREWALL_EBTABLES_RULE_FROM_HOUR_OPTION, i);
			api_get_integer_option2(&setting[j].from_minute, SN_FIREWALL_EBTABLES_RULE_FROM_MINUTE_OPTION, i);
			api_get_integer_option2(&setting[j].to_hour, SN_FIREWALL_EBTABLES_RULE_TO_HOUR_OPTION, i);
			api_get_integer_option2(&setting[j].to_minute, SN_FIREWALL_EBTABLES_RULE_TO_MINUTE_OPTION, i);
			api_get_string_option2(setting[j].week_day, sizeof(setting[j].week_day), SN_FIREWALL_EBTABLES_RULE_WEEK_DAY_OPTION, i);
			api_get_string_option2(setting[j].summary, sizeof(setting[j].summary), SN_FIREWALL_EBTABLES_RULE_SUMMARY_OPTION, i);

			for(k = 1, m = 0; k <= MAX_CLIENT_PERSON; k++)
			{
				memset(rule_index, 0x00, sizeof(rule_index));
				api_get_integer_list_option2(rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_PERSON_RULES_OPTION, k);

				for(l = 0; 0 != rule_index[l] && l < MAX_FIREWALL_EBTABLES_RULE; l++)
				{
					if(rule_index[l] == setting[j].index)
					{
						setting[j].related_user[m] = k;
						m++;
						break;
					}
				}
			}

			j++;
		}

		return_str = OK_STR;

		get_mesh_firewall_rule_list_json_cb(pkt, setting, return_str);
	}

	return 0;
}

/*****************************************************************
* NAME:    set_mesh_firewall_rule_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int set_mesh_firewall_rule_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	MESH_FIREWALL_RULE_T setting;
	int i, j, k, l;
	int empty_index;
	bool existed;
	char buf[32];
	int rule_index[MAX_FIREWALL_EBTABLES_RULE];
	int rule_index2[MAX_FIREWALL_EBTABLES_RULE];
	int device_index[MAX_CLIENT_DEVICE];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		existed = FALSE;
		empty_index = -1;
		memset(&setting, 0x00, sizeof(setting));

		if(parse_mesh_firewall_rule_json_cb(query_str, &setting, &return_str))
		{
			if(0 == setting.index)
			{
				for(i = 1; i <= MAX_FIREWALL_EBTABLES_RULE; i++)
				{
					memset(buf, 0x00, sizeof(buf));
					api_get_string_option2(buf, sizeof(buf), SN_FIREWALL_EBTABLES_RULE_NAME_OPTION, i);

					if(0 == strlen(buf))
					{
						if(0 > empty_index)
						{
							empty_index = i;
						}
					}

					if(0 == strcmp(setting.name, buf))
					{
						existed = TRUE;
					}
				}

				if(TRUE == existed)
				{
					i = MAX_FIREWALL_EBTABLES_RULE + 1;
					return_str = ERROR_RULE_EXISTED_STR;
				}
				else if(0 < empty_index)
				{
					i = empty_index;
				}
			}
			else
			{
				i = setting.index;
			}

			if(i <= MAX_FIREWALL_EBTABLES_RULE)
			{
				api_set_string_option2(setting.name, sizeof(setting.name), SN_FIREWALL_EBTABLES_RULE_NAME_OPTION, i);
				api_set_integer_option2(setting.schedule_block_enable, SN_FIREWALL_EBTABLES_RULE_SCHEDULE_BLOCK_ENABLE_OPTION, i);
				api_set_integer_option2(setting.all_time, SN_FIREWALL_EBTABLES_RULE_ALL_TIME_OPTION, i);
				api_set_integer_option2(setting.from_hour, SN_FIREWALL_EBTABLES_RULE_FROM_HOUR_OPTION, i);
				api_set_integer_option2(setting.to_hour, SN_FIREWALL_EBTABLES_RULE_TO_HOUR_OPTION, i);
				api_set_integer_option2(setting.from_minute, SN_FIREWALL_EBTABLES_RULE_FROM_MINUTE_OPTION, i);
				api_set_integer_option2(setting.to_minute, SN_FIREWALL_EBTABLES_RULE_TO_MINUTE_OPTION, i);
				api_set_string_option2(setting.week_day, sizeof(setting.week_day), SN_FIREWALL_EBTABLES_RULE_WEEK_DAY_OPTION, i);

				api_set_integer_option2(setting.url_block_enable, SN_FIREWALL_EBTABLES_RULE_URL_BLOCK_ENABLE_OPTION, i);

#if SUPPORT_FIREWALL_URL_CATEGORY
				api_set_integer_option2(setting.block_social_media_enable, SN_FIREWALL_EBTABLES_RULE_BLOCK_SOCIAL_MEDIA_ENABLE_OPTION, i);
				api_set_integer_option2(setting.block_search_engine_enable, SN_FIREWALL_EBTABLES_RULE_BLOCK_SEARCH_ENGINE_ENABLE_OPTION, i);
				api_set_integer_option2(setting.block_video_media_enable, SN_FIREWALL_EBTABLES_RULE_BLOCK_VIDEO_MEDIA_ENABLE_OPTION, i);
				api_set_integer_option2(setting.block_custom_url_enable, SN_FIREWALL_EBTABLES_RULE_BLOCK_CUSTOM_URL_ENABLE_OPTION, i);

				api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_ITEM_OPTION, i);

				api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_SOCIAL_MEDIA_LIST_OPTION, i);
				for(j = 0; 0 != strlen(setting.block_social_media_list[j]) && j < MAX_FIREWALL_BLOCK_ITEM; j++)
				{
					api_add_string_list2(setting.block_social_media_list[j], sizeof(setting.block_social_media_list[j]), SN_FIREWALL_EBTABLES_RULE_BLOCK_SOCIAL_MEDIA_LIST_OPTION, i);

					if(setting.block_social_media_enable)
					{
						api_add_string_list2(setting.block_social_media_list[j], sizeof(setting.block_social_media_list[j]), SN_FIREWALL_EBTABLES_RULE_BLOCK_ITEM_OPTION, i);
					}
				}

				api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_SEARCH_ENGINE_LIST_OPTION, i);
				for(j = 0; 0 != strlen(setting.block_search_engine_list[j]) && j < MAX_FIREWALL_BLOCK_ITEM; j++)
				{
					api_add_string_list2(setting.block_search_engine_list[j], sizeof(setting.block_search_engine_list[j]), SN_FIREWALL_EBTABLES_RULE_BLOCK_SEARCH_ENGINE_LIST_OPTION, i);

					if(setting.block_search_engine_enable)
					{
						api_add_string_list2(setting.block_search_engine_list[j], sizeof(setting.block_search_engine_list[j]), SN_FIREWALL_EBTABLES_RULE_BLOCK_ITEM_OPTION, i);
					}
				}

				api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_VIDEO_MEDIA_LIST_OPTION, i);
				for(j = 0; 0 != strlen(setting.block_video_media_list[j]) && j < MAX_FIREWALL_BLOCK_ITEM; j++)
				{
					api_add_string_list2(setting.block_video_media_list[j], sizeof(setting.block_video_media_list[j]), SN_FIREWALL_EBTABLES_RULE_BLOCK_VIDEO_MEDIA_LIST_OPTION, i);

					if(setting.block_video_media_enable)
					{
						api_add_string_list2(setting.block_video_media_list[j], sizeof(setting.block_video_media_list[j]), SN_FIREWALL_EBTABLES_RULE_BLOCK_ITEM_OPTION, i);
					}
				}

				api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_CUSTOM_URL_LIST_OPTION, i);
				for(j = 0; 0 != strlen(setting.block_custom_url_list[j]) && j < MAX_FIREWALL_BLOCK_ITEM; j++)
				{
					api_add_string_list2(setting.block_custom_url_list[j], sizeof(setting.block_custom_url_list[j]), SN_FIREWALL_EBTABLES_RULE_BLOCK_CUSTOM_URL_LIST_OPTION, i);

					if(setting.block_custom_url_enable)
					{
						api_add_string_list2(setting.block_custom_url_list[j], sizeof(setting.block_custom_url_list[j]), SN_FIREWALL_EBTABLES_RULE_BLOCK_ITEM_OPTION, i);
					}
				}
#else
				api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_ITEM_OPTION, i);
				for(j = 0; 0 != strlen(setting.block_item[j]) && j < MAX_FIREWALL_BLOCK_ITEM; j++)
				{
					api_add_string_list2(setting.block_item[j], sizeof(setting.block_item[j]), SN_FIREWALL_EBTABLES_RULE_BLOCK_ITEM_OPTION, i);
				}
#endif

				api_set_integer_option2(setting.block_all, SN_FIREWALL_EBTABLES_RULE_BLOCK_ALL_OPTION, i);
				api_set_string_option2(setting.summary, sizeof(setting.summary), SN_FIREWALL_EBTABLES_RULE_SUMMARY_OPTION, i);

				/* Remove the rule from original connected user and set it for the new user. */
				for(j = 1; j <= MAX_CLIENT_PERSON; j++)
				{
					memset(rule_index, 0x00, sizeof(rule_index));
					api_get_integer_list_option2(rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_PERSON_RULES_OPTION, j);

					for(k = 0; 0 != rule_index[k] && k < MAX_FIREWALL_EBTABLES_RULE; k++)
					{
						if(rule_index[k] == i)
						{
							api_del_integer_list2(i, CLIENT_PERSON_RULES_OPTION, j);
							/* Get the devices which belong to the user.
							 * Delete the rule from the device's option. */
							memset(device_index, 0x00, sizeof(device_index));
							api_get_integer_list_option2(device_index, MAX_CLIENT_DEVICE, CLIENT_PERSON_DEVICES_OPTION, j);

							for(l = 0; 0 != device_index[l] && l < MAX_CLIENT_DEVICE; l++)
							{
								api_del_integer_list2(i, CLIENT_DEVICE_RULES_OPTION, device_index[l]);
							}
							break;
						}
					}
				}

				for(j = 0; 0 != setting.related_user[j] && j < MAX_CLIENT_PERSON; j++)
				{
					api_add_integer_list2(i, CLIENT_PERSON_RULES_OPTION, setting.related_user[j]);

					/* Get the devices which belong to the user.
					 * Check if the rule exists in the device's option. */
					memset(device_index, 0x00, sizeof(device_index));
					api_get_integer_list_option2(device_index, MAX_CLIENT_DEVICE, CLIENT_PERSON_DEVICES_OPTION, setting.related_user[j]);

					for(k = 0; 0 != device_index[k] && k < MAX_CLIENT_DEVICE; k++)
					{
						existed = FALSE;
						memset(rule_index2, 0x00, sizeof(rule_index2));
						api_get_integer_list_option2(rule_index2, MAX_FIREWALL_EBTABLES_RULE, CLIENT_DEVICE_RULES_OPTION, device_index[k]);

						for(l = 0; 0 != rule_index2[l] && l < MAX_FIREWALL_EBTABLES_RULE; l++)
						{
							if(i == rule_index2[l])
							{
								existed = TRUE;
								break;
							}
						}

						if(FALSE == existed)
						{
							api_add_integer_list2(i, CLIENT_DEVICE_RULES_OPTION, device_index[k]);
						}
					}
				}

				return_str = OK_STR;
				APPAGENT_SYSTEM("uci commit client");
				APPAGENT_SYSTEM("uci commit sn_firewall");
				APPAGENT_SYSTEM("luci-reload client firewall sfe qos upnpd sncrontab nat-traversal &");
			}
			else
			{
				if(ERROR_RULE_EXISTED_STR != return_str)
				{
					return_str = ERROR_FULL_RULE_STR;
				}
			}
		}
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    delete_mesh_firewall_rule_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int delete_mesh_firewall_rule_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	int  index;
	int  i, j;
	int  rule_index[MAX_FIREWALL_EBTABLES_RULE];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		get_json_integer_from_query(query_str, &index, "Index");

		if(index >= 1 && index <= MAX_FIREWALL_EBTABLES_RULE)
		{
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_NAME_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_SOURCE_MAC_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_ALL_TIME_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_FROM_HOUR_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_TO_HOUR_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_FROM_MINUTE_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_TO_MINUTE_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_WEEK_DAY_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_SCHEDULE_BLOCK_ENABLE_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_URL_BLOCK_ENABLE_OPTION, index);
#if SUPPORT_FIREWALL_URL_CATEGORY
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_SOCIAL_MEDIA_ENABLE_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_SEARCH_ENGINE_ENABLE_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_VIDEO_MEDIA_ENABLE_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_CUSTOM_URL_ENABLE_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_SOCIAL_MEDIA_LIST_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_SEARCH_ENGINE_LIST_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_VIDEO_MEDIA_LIST_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_CUSTOM_URL_LIST_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_ITEM_OPTION, index);
#else
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_ITEM_OPTION, index);
#endif
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_BLOCK_ALL_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_SUMMARY_OPTION, index);
			api_delete_option2("", SN_FIREWALL_EBTABLES_RULE_ACTIVE_OPTION, index);

			for(i = 1; i <= MAX_CLIENT_PERSON; i++)
			{
				memset(rule_index, 0x00, sizeof(rule_index));
				api_get_integer_list_option2(rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_PERSON_RULES_OPTION, i);

				for(j = 0; 0 != rule_index[j] && j < MAX_FIREWALL_EBTABLES_RULE; j++)
				{
					if(rule_index[j] == index)
					{
						api_del_integer_list2(index, CLIENT_PERSON_RULES_OPTION, i);
					}
				}
			}

			for(i = 1; i <= MAX_CLIENT_DEVICE; i++)
			{
				memset(rule_index, 0x00, sizeof(rule_index));
				api_get_integer_list_option2(rule_index, MAX_FIREWALL_EBTABLES_RULE, CLIENT_DEVICE_RULES_OPTION, i);

				for(j = 0; 0 != rule_index[j] && j < MAX_FIREWALL_EBTABLES_RULE; j++)
				{
					if(rule_index[j] == index)
					{
						api_del_integer_list2(index, CLIENT_DEVICE_RULES_OPTION, i);
					}
				}
			}

			return_str = OK_STR;
			APPAGENT_SYSTEM("uci commit client");
			APPAGENT_SYSTEM("uci commit sn_firewall");
			APPAGENT_SYSTEM("luci-reload client firewall sfe qos upnpd sncrontab nat-traversal &");
		}
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    block_mesh_user_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int block_mesh_user_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	char *ptr;
	int i;
	int index;
	int boolean;
	int device_index[MAX_CLIENT_DEVICE];
	bool block;
	char buf[32];
	char block_time[32];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		index = 0;
		memset(buf, 0x00, sizeof(buf));
		memset(block_time, 0x00, sizeof(block_time));
		memset(device_index, 0x00, sizeof(device_index));

		get_json_integer_from_query(query_str, &index, "Index");
		get_json_boolean_from_query(query_str, &block, "Block");
		get_json_string_from_query(query_str, block_time, "BlockTime");

		api_get_integer_list_option2(device_index, MAX_CLIENT_DEVICE, CLIENT_PERSON_DEVICES_OPTION, index);

		if(block)
		{
			sys_interact(buf, sizeof(buf), "date +%%s");

			if(ptr = strchr(buf, '\n'))
			{
				*ptr = '\0';
			}

			api_set_integer_option2(1, CLIENT_PERSON_BLOCK_OPTION, index);
			api_set_string_option2(buf, sizeof(buf), CLIENT_PERSON_START_DATE_OPTION, index);
			api_set_string_option2(block_time, sizeof(block_time), CLIENT_PERSON_BLOCK_TIME_OPTION, index);
			api_set_integer_option2(1, CLIENT_PERSON_BLOCK_ACTIVATED_OPTION, index);

			for(i = 0; i < MAX_CLIENT_DEVICE && 0 != device_index[i]; i++)
			{
				api_set_integer_option2(1, CLIENT_DEVICE_BLOCK_ACTIVATED_OPTION, device_index[i]);
			}
		}
		else
		{
			api_set_integer_option2(0, CLIENT_PERSON_BLOCK_OPTION, index);
			api_delete_option2("", CLIENT_PERSON_START_DATE_OPTION, index);
			api_delete_option2("", CLIENT_PERSON_BLOCK_TIME_OPTION, index);
			api_delete_option2("", CLIENT_PERSON_BLOCK_ACTIVATED_OPTION, index);

			for(i = 0; i < MAX_CLIENT_DEVICE && 0 != device_index[i]; i++)
			{
				boolean = 0;
				api_get_bool_option2(&boolean, CLIENT_DEVICE_BLOCK_OPTION, device_index[i]);

				if(0 == boolean)
				{
					api_delete_option2("", CLIENT_DEVICE_BLOCK_ACTIVATED_OPTION, device_index[i]);
				}
			}
		}

		return_str = OK_STR;
		APPAGENT_SYSTEM("uci commit client");
		APPAGENT_SYSTEM("luci-reload client firewall sfe qos upnpd sncrontab nat-traversal &");
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    block_mesh_client_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int block_mesh_client_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	char *ptr;
	int i;
	int existed;
	int empty_index;
	int from_hour;
	int from_minute;
	int to_hour;
	int to_minute;
	bool block;
	bool schedule_block;
	bool all_time;
	char buf[32];
	char mac[32];
	char mac_buf[32];
	char name[32];
	char name_buf[256];
	char block_time[32];
	char week_day[32];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		existed = 0, empty_index = 0;
		memset(buf, 0x00, sizeof(buf));
		memset(mac, 0x00, sizeof(mac));
		memset(name, 0x00, sizeof(name));
		memset(block_time, 0x00, sizeof(block_time));

		get_json_string_from_query(query_str, mac, "MAC");
		get_json_boolean_from_query(query_str, &block, "Block");
		get_json_boolean_from_query(query_str, &schedule_block, "ScheduleBlock");


		for(i = 1; i <= MAX_CLIENT_DEVICE; i++)
		{
			memset(mac_buf, 0x00, sizeof(mac_buf));
			api_get_string_option2(mac_buf, sizeof(mac_buf), CLIENT_DEVICE_MAC_OPTION, i);

			if(0 == strlen(mac_buf))
			{
				if(0 == empty_index)
				{
					empty_index = i;
				}

				continue;
			}

			if(0 == strcmp(mac, mac_buf))
			{
				existed = 1;
				break;
			}
		}

		if(block)
		{
			get_json_string_from_query(query_str, block_time, "BlockTime");

			if(0 == existed)
			{
				i = empty_index;
				memset(buf, 0x00, sizeof(buf));

				sys_interact(name_buf, sizeof(name_buf), "cat /tmp/finger_device_list | grep \"%s\"", mac);
				sscanf(name_buf, "%*s\t%*s\t%*s\t%*[^\t]\t%[^\t]\t%*[^\n]\n", name);
				api_set_string_option2(name, sizeof(name), CLIENT_DEVICE_NAME_OPTION, i);
				api_set_string_option2(mac, sizeof(mac), CLIENT_DEVICE_MAC_OPTION, i);
			}

			memset(buf, 0x00, sizeof(buf));
			sys_interact(buf, sizeof(buf), "date +%%s");

			if(ptr = strchr(buf, '\n'))
			{
				*ptr = '\0';
			}

			if(MAX_CLIENT_DEVICE >= i)
			{
				api_set_integer_option2(1, CLIENT_DEVICE_BLOCK_OPTION, i);
				api_set_string_option2(buf, sizeof(buf), CLIENT_DEVICE_START_DATE_OPTION, i);
				api_set_string_option2(block_time, sizeof(block_time), CLIENT_DEVICE_BLOCK_TIME_OPTION, i);
				api_set_integer_option2(1, CLIENT_DEVICE_BLOCK_ACTIVATED_OPTION, i);
				return_str = OK_STR;
			}
		}
		else
		{
			if(existed && (MAX_CLIENT_DEVICE >= i))
			{
				api_set_integer_option2(0, CLIENT_DEVICE_BLOCK_OPTION, i);
				api_delete_option2("", CLIENT_DEVICE_BLOCK_ACTIVATED_OPTION, i);
				api_delete_option2("", CLIENT_DEVICE_START_DATE_OPTION, i);
				api_delete_option2("", CLIENT_DEVICE_BLOCK_TIME_OPTION, i);
				return_str = OK_STR;
			}
		}

		if(schedule_block)
		{
			if(0 == existed)
			{
				i = empty_index;
				memset(buf, 0x00, sizeof(buf));

				sys_interact(name_buf, sizeof(name_buf), "cat /tmp/finger_device_list | grep \"%s\"", mac);
				sscanf(name_buf, "%*s\t%*s\t%*s\t%*[^\t]\t%[^\t]\t%*[^\n]\n", name);
				api_set_string_option2(name, sizeof(name), CLIENT_DEVICE_NAME_OPTION, i);
				api_set_string_option2(mac, sizeof(mac), CLIENT_DEVICE_MAC_OPTION, i);
			}

			if(MAX_CLIENT_DEVICE >= i)
			{
				get_json_integer_from_query(query_str, &from_hour, "FromHour");
				get_json_integer_from_query(query_str, &from_minute, "FromMinute");
				get_json_integer_from_query(query_str, &to_hour, "ToHour");
				get_json_integer_from_query(query_str, &to_minute, "ToMinute");
				get_json_string_from_query(query_str, week_day, "WeekDay");

				api_set_integer_option2(1, CLIENT_DEVICE_NAME_OPTION, i);
				api_set_integer_option2(from_hour, CLIENT_DEVICE_FROM_HOUR_OPTION, i);
				api_set_integer_option2(from_minute, CLIENT_DEVICE_FROM_MINUTE_OPTION, i);
				api_set_integer_option2(to_hour, CLIENT_DEVICE_TO_HOUR_OPTION, i);
				api_set_integer_option2(to_minute, CLIENT_DEVICE_TO_MINUTE_OPTION, i);
				api_set_string_option2(week_day, sizeof(week_day), CLIENT_DEVICE_WEEK_DAY_OPTION, i);

				return_str = OK_STR;
			}
		}
		else
		{
			if(existed && (MAX_CLIENT_DEVICE >= i))
			{
				api_set_integer_option2(0, CLIENT_DEVICE_SCHEDULE_BLOCK_OPTION, i);

				return_str = OK_STR;
			}
		}

		if(0 == block && 0 == schedule_block)
		{
			return_str = OK_STR;
		}

		if(OK_STR == return_str)
		{
			APPAGENT_SYSTEM("uci commit client");
			APPAGENT_SYSTEM("luci-reload client firewall sfe qos upnpd sncrontab nat-traversal &");
		}
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}

/*****************************************************************
* NAME:    reset_all_blocked_mesh_client_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int reset_all_blocked_mesh_client_cb(HTTPS_CB *pkt)
{
	char *query_str;
	char *return_str;
	int i;
	int boolean;
	int reset;
	char mac[32];

	if(NULL == pkt)
		return -1;

	query_str = get_env(&pkt->envcfg, "QUERY_STRING");
	return_str = ERROR_STR;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		reset = 0;

		for(i = 1; i <= MAX_CLIENT_DEVICE; i++)
		{
			memset(mac, 0x00, sizeof(mac));
			api_get_string_option2(mac, sizeof(mac), CLIENT_DEVICE_MAC_OPTION, i);

			if(0 == strlen(mac))
			{
				continue;
			}

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_DEVICE_BLOCK_OPTION, i);

			if(boolean)
			{
				APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_BLOCK_OPTION"=0", i);
				APPAGENT_SYSTEM("uci delete "CLIENT_DEVICE_BLOCK_ACTIVATED_OPTION, i);
				APPAGENT_SYSTEM("uci delete "CLIENT_DEVICE_START_DATE_OPTION, i);
				APPAGENT_SYSTEM("uci delete "CLIENT_DEVICE_BLOCK_TIME_OPTION, i);

				reset = 1;
			}

			boolean = 0;
			api_get_bool_option2(&boolean, CLIENT_DEVICE_SCHEDULE_BLOCK_OPTION, i);

			if(boolean)
			{
				APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_SCHEDULE_BLOCK_OPTION"=0", i);
				APPAGENT_SYSTEM("uci delete "CLIENT_DEVICE_BLOCK_ACTIVATED_OPTION, i);

				reset = 1;
			}
		}

		if(1 == reset)
		{
			APPAGENT_SYSTEM("uci commit client");
			APPAGENT_SYSTEM("luci-reload client firewall sfe qos upnpd sncrontab nat-traversal &");
		}

		return_str = OK_STR;
	}

	/* Send response packet */
	send_simple_response(pkt, return_str);

	return 0;
}
#endif
/*****************************************************************
* NAME:    ezsetup_broadcast_pkts_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int ezsetup_broadcast_pkts_cb(HTTPS_CB *pkt)
{
	char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
	char *return_str = ERROR_STR;
	struct json_object *jobj;
	int disabled=1;
	int ez_counter=0, ez_setup_flag=1; //flag 1 support scanning method
	char buf[256]={0}, ez_buf[128]={0};
	FILE *fp = NULL;
	char ez_mesh_mac[18]={0}, ez_mesh[3]={0}, ez_sn[16]={0}, ez_cfg[3]={0}, ez_model[4]={0}, ez_domain[3]={0};

	if(NULL == pkt)
	{
		return -1;
	}

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		if(sysutil_check_file_existed(EzSetup_ADD_SLAVE_FLAG))
		{
			APPAGENT_SYSTEM("rm %s", EzSetup_ADD_SLAVE_FLAG);
			if(sysutil_check_file_existed(EzSetup_SCAN_RESULT))
			{
				APPAGENT_SYSTEM("rm %s", EzSetup_SCAN_RESULT);
			}
		}

		APPAGENT_SYSTEM("iwlist %s scanning | tail -n +3 > %s", EzSetup_24g_SCANNING_TMP, EzSetup_SCAN_RESULT);

		APPAGENT_SYSTEM("sh /sbin/MeshBroadcast_cfg.sh 0 1 &");

		if(sysutil_check_file_existed(EzSetup_SCAN_RESULT))
		{
			return_str = OK_STR;
		}
	}

	jobj = json_object_new_object();

	sprintf(buf, "%sResult", pkt->json_action);
	json_object_object_add(jobj, buf, json_object_new_string(return_str));

	if(OK_STR == return_str)
	{
		sprintf(buf, "%sSupportScan", pkt->json_action);
		json_object_object_add(jobj, buf, json_object_new_int(1));
	}

	basic_json_response(pkt,  (char *)json_object_to_json_string(jobj));
	json_object_put(jobj);

	return 0;
}
/*****************************************************************
* NAME:    ezsetup_get_candidates_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int ezsetup_get_candidates_cb(HTTPS_CB *pkt)
{
	char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
	char *return_str;
	int disabled=1, numOfLines=0;

	if(NULL == pkt)
		return -1;

	if(FALSE == is_mesh_function_enabled())
	{
		send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
	}
	else
	{
		return_str = ERROR_DEVICE_NOT_FOUND_STR;

		if(sysutil_check_file_existed(EzSetup_ADD_SLAVE_FLAG))
		{
			APPAGENT_SYSTEM("rm %s", EzSetup_ADD_SLAVE_FLAG);
		}

		if(pkt->json)
		{
			ezsetup_get_candidates_json_cb(pkt, return_str);
		}
	}

	return 0;
}
/*****************************************************************
* NAME:    ezsetup_add_mesh_nodes_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int ezsetup_add_mesh_nodes_cb(HTTPS_CB *pkt)
{
	struct json_object *jobj;

	struct json_object *ez_jobj;
	struct json_object *ez_jarr;
	struct json_object *ez_jarr_obj;
	struct array_list *ez_array_setting;

	char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
	char *return_str;
	char ipList[512]={0}, FailList[512]={0}, name[64]={0}, mesh_id[64]={0}, password[128]={0}, buf[32]={0};
	char ez_buf[256]={0}, ez_chan[10]={0}, ez_htmode[10]={0};
	int disabled=1, type=1;
	int ez_count=0, ez_i=0, ez_meshbh=0, ez_getbschan=0;
	char ez_mac[32]={0}, ez_country[8]={0}, ez_cur_chan[8]={0}, ez_model[32]={0};
	FILE *fp = NULL;

	if(FALSE == is_mesh_function_enabled())
	{
		return_str = ERROR_MESH_DISABLED_STR;
	}
	else if(FALSE == check_mesh_account(query_str))
	{
		return_str = ERROR_UNAUTHORIZED_STR;
	}
	else
	{
		return_str = OK_STR;

		if(!sysutil_check_file_existed(EzSetup_ADD_SLAVE_FLAG))
		{

			if(sysutil_check_file_existed("/tmp/ez_add_mesh_mac"))
			{
				SYSTEM("rm -f /tmp/ez_add_mesh_mac");
			}
			if((ez_jobj = json_tokener_parse(query_str)))
			{
				ez_jarr = json_object_object_get(ez_jobj, "MeshNodeList");
				ez_array_setting = json_object_get_array(ez_jarr);
				ez_count = ez_array_setting->length;

				for(ez_i = 0; ez_i < ez_count; ez_i++)
				{
					ez_jarr_obj = json_object_array_get_idx(ez_jarr, ez_i);
					memset(ez_mac, 0x00, sizeof(ez_mac));

					senao_json_object_get_string(ez_jarr_obj, "MeshMac", ez_mac);
					if (strlen(ez_mac)!=0)
					{
						printf("**** We Got [%s] to Mesh Add List.\n", ez_mac);
						SYSTEM("echo %s >> /tmp/ez_add_mesh_mac", ez_mac);
					}
#if 1
					sysutil_interact(ez_model, sizeof(ez_model), "[ -e \"/tmp/ezsetup_scan_result_bak\" ] && cat /tmp/ezsetup_scan_result_bak | grep %s | awk {'printf $11'}", ez_mac);
#else
					senao_json_object_get_string(ez_jarr_obj, "DeviceType", &ez_model);
#endif
					if(atoi(ez_model) != 2)
					{
						ez_getbschan=1;
#if 1
						printf("**** [%s] TYPE [%d] NOT support DFS channel.\n", ez_mac, atoi(ez_model));
#else
						printf("**** [%s] TYPE [%d] NOT support DFS channel.\n", ez_mac, ez_model);
#endif
					}
				}
			}

#if HAS_WLAN_5G_2_SETTING
			if(ez_getbschan == 1)
			{
				api_get_integer_option(WIRELESS_WIFI_5G_MESH_DISABLED_OPTION, &disabled);
				if (disabled == 0)
				{
					api_get_string_option("wireless.wifi1.channel", ez_chan, sizeof(ez_chan));
				}
				else
				{
					api_get_string_option("wireless.wifi2.channel", ez_chan, sizeof(ez_chan));
				}
				if(strcmp(ez_chan,"auto") != 0)
				{
					if(atoi(ez_chan) > 48 && atoi(ez_chan) < 149)
					{
						printf("* ez_chan [%d]\n", atoi(ez_chan));
						return_str = ERROR_DFS_CHANNEL_STR;
					}
				}
			}
			if(strcmp(return_str, ERROR_DFS_CHANNEL_STR) != 0)
			{
				return_str = ERROR_PROCESS_IS_RUNNING_STR;
				sysCheckStringOnApp(query_str, ez_buf);
				APPAGENT_SYSTEM("/sbin/MeshAddMeshNodes.sh %s\"%s\" &", (ez_getbschan)?"mesh_getbschan ":"", ez_buf);
			}
#else
			return_str = ERROR_PROCESS_IS_RUNNING_STR;
			sysCheckStringOnApp(query_str, ez_buf);
			APPAGENT_SYSTEM("/sbin/MeshAddMeshNodes.sh %s\"%s\" &", (ez_getbschan)?"mesh_getbschan ":"", ez_buf);
#endif
		}
		else
		{
			parse_json_ezsetup_add_mesh_nodes(query_str, ipList, FailList, &return_str);
			if (return_str == OK_STR)
			{
				get_json_mesh_account(query_str, mesh_id, password);
				sysutil_interact(name, sizeof(name), "cat /proc/mesh_temp");
				if ((name != 0) && (strlen(name) > 0))
				{
					name[strlen(name)-1] = '\0';
				}
				else
				{
					type = 0;
				}

				api_get_integer_option(WIRELESS_WIFI_MESH_DISABLED_OPTION, &disabled);
				if (disabled == 0)
				{
					api_get_string_option("wireless.wifi0.channel", ez_chan, sizeof(ez_chan));
					api_get_string_option("wireless.wifi0.htmode", ez_htmode, sizeof(ez_htmode));
					ez_meshbh=0;
#if HAS_WLAN_5G_2_SETTING
					sysutil_interact(ez_cur_chan, sizeof(ez_cur_chan), "iwlist `getinfo mesh_ifname` chan| awk /Current/{'print $2'}");
#endif
				}
#if SUPPORT_WLAN_5G_SETTING
				else
				{
					api_get_integer_option(WIRELESS_WIFI_5G_MESH_DISABLED_OPTION, &disabled);
					if (disabled == 0)
					{
						api_get_string_option("wireless.wifi1.channel", ez_chan, sizeof(ez_chan));
						api_get_string_option("wireless.wifi1.htmode", ez_htmode, sizeof(ez_htmode));
						ez_meshbh=1;
#if HAS_WLAN_5G_2_SETTING
						sysutil_interact(ez_cur_chan, sizeof(ez_cur_chan), "iwlist `getinfo mesh_ifname` chan| awk /Current/{'print $2'}");
#endif
					}
#if HAS_WLAN_5G_2_SETTING
					api_get_integer_option("wireless.wifi2_mesh.disabled", &disabled);
					if (disabled == 0)
					{
						api_get_string_option("wireless.wifi2.channel", ez_chan, sizeof(ez_chan));
						api_get_string_option("wireless.wifi2.htmode", ez_htmode, sizeof(ez_htmode));
						ez_meshbh=2;
						sysutil_interact(ez_cur_chan, sizeof(ez_cur_chan), "iwlist `getinfo mesh_ifname` chan| awk /Current/{'print $2'}");
					}
#endif
					if((strcmp(ez_chan, "auto")==0)?1:0)
					{
						sprintf(ez_chan, "auto_5G");
					}
					if((strcmp(ez_chan, "0")==0)?1:0)
					{
						sprintf(ez_chan, "auto_5G");
					}
				}
#endif
				api_get_string_option("wireless.wifi1.country", ez_country, sizeof(ez_country));
				fp = fopen("/tmp/ezsetup_mesh_profile", "w");
				if(fp)
				{
					fprintf(fp, "name %s\n", name);
					fprintf(fp, "mesh_id %s\n", mesh_id);
					fprintf(fp, "password %s\n", password);
					fprintf(fp, "ez_chan %s\n", ez_chan);
					fprintf(fp, "ez_type %d\n", type);
					fprintf(fp, "ez_country %s\n", ez_country);
					fprintf(fp, "ez_htmode %s\n", ez_htmode);
					fprintf(fp, "ez_meshbh %d\n", ez_meshbh);
					fprintf(fp, "ez_cur_chan %s\n", ez_cur_chan);
					fclose(fp);
				}

				APPAGENT_SYSTEM("/sbin/MeshAddMeshNodes.sh mesh_profile %s &", ipList);
//				APPAGENT_SYSTEM("app_client -m POST -M '%s' -a mesh/SyncMeshMSCConfigured -e 1 -p "
//								"'{\"MeshAdminUsername\":\"%s\",\"MeshAdminPassword\":\"1s2h3a4m5b6a7l8l9a0\","
//								"\"ID\":\"%s\",\"Passwd\":\"%s\",\"Chan\":\"%s\",\"ConnType\":%d,\"Country\":\"%s\"}' &",
//								ipList, name, mesh_id, password, ez_chan, type, ez_country);

				// /sbin/MeshBroadcast_cfg.sh auto stop both 2.4g and 5g tmp wds
//				APPAGENT_SYSTEM("echo 'sleep 20;/sbin/MeshBroadcast_cfg.sh 0 0' > /tmp/ezsetup_broadcast_stop");
//				APPAGENT_SYSTEM("sh /tmp/ezsetup_broadcast_stop &");
			}
		}
	}

send_pkt:
	//send_simple_response(pkt, return_str);
	jobj = json_object_new_object();
	sprintf(buf, "%sResult", pkt->json_action);
	json_object_object_add(jobj, buf, json_object_new_string(return_str));
	json_object_object_add(jobj, "FailList", json_object_new_string(FailList));
	basic_json_response(pkt,  (char *)json_object_to_json_string(jobj));
	json_object_put(jobj);

	return 0;
}
/*****************************************************************
* NAME:    ezsetup_get_mesh_fail_nodes_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int ezsetup_get_mesh_fail_nodes_cb(HTTPS_CB *pkt)
{
	char FailList[512]={0}, buf[32]={0};
	struct json_object *jobj, *jarr;

	jobj = json_object_new_object();
	jarr = json_object_new_array();

	if(sysutil_check_file_existed("/sbin/GetRetryMeshNodes.sh"))
	{
		sysutil_interact(FailList, sizeof(FailList), "/sbin/GetRetryMeshNodes.sh");
		if(strlen(FailList))
		{
			char *mac = strtok(FailList, " ");
			while (mac != NULL)
			{
				json_object_array_add(jarr, json_object_new_string(mac));
				mac = strtok(NULL, " ");
			}
		}
	}

	sprintf(buf, "%sResult", pkt->json_action);
	json_object_object_add(jobj, buf, json_object_new_string(OK_STR));
	json_object_object_add(jobj, "FailList", jarr);
	basic_json_response(pkt, (char *)json_object_to_json_string(jobj));
	json_object_put(jobj);
}
/*****************************************************************
* NAME:    set_mesh_device_ibeacon_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int set_mesh_device_ibeacon_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *return_str;
    char *ptr, *ptr2;
    int enabled=1;
    char advertising[8]={0};
    char uuid[128]={0};
    char major[32]={0};
    char minor[32]={0};
    char text[1024]={0};
    char target_mac[32]={0};
    char buf[256]={0};
    char buf1[256]={0};


    char ori_ver[64];
    char *saveptr1, *saveptr2, *ptr_ori, *ptr_new;
    char uuid_nonhyphen_part[128]={0};
    char uuid_nonhyphen[128]={0};
    char uuid_set[256]={0};
    int j = 0;
    int hci_st=0;

    if(NULL == pkt)
        return -1;

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else
    {
        memset(target_mac, 0x00, sizeof(target_mac));
        get_json_string_from_query(query_str, target_mac, "TargetMeshMac");

        if(0 == strlen(target_mac) || check_mesh_private_mac(target_mac))
        {
            memset(advertising, 0x00, sizeof(advertising));
            get_json_string_from_query(query_str, advertising, "Advertising");
            enabled=atoi(advertising);

            APPAGENT_SYSTEM("uci set ibeacon.@ibeacon[0].enable=%d", enabled);
            if(enabled)
            {
                sysutil_interact(buf1, sizeof(buf1), "hciconfig |wc -l");
                hci_st=atoi(buf1);
                if(hci_st < 1)
                {
                    APPAGENT_SYSTEM("sh /sbin/Ibeacon_Init.sh; sh /sbin/ibeacon_start.sh &");
                }
                else
                {
                    APPAGENT_SYSTEM("sh /sbin/ibeacon_start.sh &");
                }
            }
            else
            {
                APPAGENT_SYSTEM("sh /sbin/ibeacon_stop.sh &");
            }

            memset(uuid, 0x00, sizeof(uuid));
            get_json_string_from_query(query_str, uuid, "UUID");

            ptr_new = strtok_r(uuid, "-", &saveptr1);

            //ex: 0f99d832-6436-48de-adaa-343536373839               

            while (ptr_new != NULL)
            {
                j +=sprintf(uuid_nonhyphen + j,"%s", ptr_new);
                ptr_new = strtok_r(saveptr1, "-", &saveptr1);
            }

            sysutil_interact(uuid_set, sizeof(uuid_set), "echo %s | sed 's/.\\{2\\}/& /g' | tr -d '\n'",uuid_nonhyphen);

            uuid_set[strlen(uuid_set)-1] = '\0';

            APPAGENT_SYSTEM("uci set ibeacon.@ibeacon[0].uuid='%s'", uuid_set);

            memset(major, 0x00, sizeof(major));
            get_json_string_from_query(query_str, major, "Major");
            APPAGENT_SYSTEM("uci set ibeacon.@ibeacon[0].major=%s", major);

            memset(minor, 0x00, sizeof(minor));
            get_json_string_from_query(query_str, minor, "Minor");
            APPAGENT_SYSTEM("uci set ibeacon.@ibeacon[0].minor=%s", minor);

            memset(text, 0x00, sizeof(text));
            get_json_string_from_query(query_str, text, "Text");
            APPAGENT_SYSTEM("uci set ibeacon.@ibeacon[0].text='%s'", text);
             

            APPAGENT_SYSTEM("uci commit ibeacon");

            send_simple_response(pkt, OK_STR);
            APPAGENT_SYSTEM("luci-reload auto &");
        }
        else
        {
            memset(buf, 0x00, sizeof(buf));
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\\n'", target_mac);
            redirect_to_target_mesh(pkt, buf);
        }        
    }

    return 0;
}
/*****************************************************************
* NAME:    get_mesh_device_ibeacon_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_mesh_device_ibeacon_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *return_str;
    char *ptr, *ptr2;
    int enabled=1;
    char uuid[128]={0};
    char major[32]={0}, minor[32]={0}, text[1024]={0}, advertising[8]={0};

    char target_mac[32]={0};
    char buf[128]={0};
    char deviceType[32]={0};

    int i=0, j=0, no_mesh=0;
    char redirectIP[1024]={0}, ip[512]={0};
    IBEACON_SETTINGS action;
    char *result;
    result = ERROR_STR;

    if(NULL == pkt)
        return -1;

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else
    {
        memset(deviceType, 0x00, sizeof(deviceType));
        sysutil_interact(deviceType, sizeof(deviceType), "batctl gw |awk '{ print $1 }' | tr -d '\n'x");

        if (0 == strcmp(deviceType, "server"))
        {
#if SUPPORT_BATMAN_2019
            sysutil_interact(buf, sizeof(buf), "batctl o -H | grep \"*\" | grep \"No batman *\" | wc -l");
#else
            sysutil_interact(buf, sizeof(buf), "batctl o -H |grep \"No batman *\" | wc -l");
#endif
            no_mesh=atoi(buf);

            if (no_mesh==0)
            {
                memset(buf, 0x00, sizeof(buf));
                sysutil_interact(buf, sizeof(buf), "batctl o -H | awk '{print $1}'");

                ptr = strtok(buf, "\n");

                do
                {
                    memset(ip, 0x00, sizeof(ip));
                    mac_to_ipv6(ptr, ip);

#if 0
                    if (i==0)
                    {
                        j += sprintf(redirectIP + j,"%s", ip);
                    }
                    else
                    {
                        j += sprintf(redirectIP + j, ",%s", ip);
                    }
                    i++;
#else
                    ptr2 = redirect_to_target_mesh_device_no_response(pkt, ip);
                    //printf("===0== [%s] : ptr2 [%s]\n", __func__, ptr2);
                    if(NULL != ptr2)
                    {
                        //printf("===1== [%s] : ptr2 [%s]\n", __func__, ptr2);
                        if(NULL != strstr(ptr2, ERROR_STR))
                        {
                            //printf("===2== [%s] : ptr2 [%s]\n", __func__, ptr2);
                            result = ERROR_STR;
                            break;
                        }
                        else
                        {
                            //printf("===3== [%s] : ptr2 [%s]\n", __func__, ptr2);
                            result = OK_STR;
                        }
                        free(ptr2);
                    }
#endif
                } while(NULL != (ptr = strtok(NULL, "\n")));

                //redirect_to_target_mesh(pkt, redirectIP);
            }
            else
            {
                result = OK_STR;
            }
        }
        else
        {
            result = ERROR_STR;
        }

        get_json_integer_from_query(query_str, (int *)&action, "Action");

        switch(action)
        {
            case COLLECT_IBEACON_SETTING:
                if(sysutil_check_file_existed("/tmp/mesh_ibeacon_settings"))
                {
                    APPAGENT_SYSTEM("rm %s", "/tmp/mesh_ibeacon_settings");
                }
                if(sysutil_check_file_existed("/sbin/mesh_ibeacon_setting.sh"))
                {
                    APPAGENT_SYSTEM("/sbin/mesh_ibeacon_setting.sh genfile &");
                }

                break;
            case GET_ALL_MESH_IBEACON_SETTINGS:
                if(OK_STR == result && !sysutil_check_file_existed("/tmp/mesh_ibeacon_settings"))
                {
                    result = ERROR_STR;
                }

                break;
        }

        get_ibeacon_settings_result_json_cb(pkt, result, action);
    }

    return 0;
}

/*****************************************************************
* NAME:    get_mesh_divece_client_info_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:  20170927 JimYang
* Modify:
******************************************************************/
int get_mesh_divece_client_info_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char target_mac[32];
    char buf[4096];
    char buf2[1024];
    char if_name[8];
    int i;
    char *ptr;
    int disabled;
#if HAS_SUPPORT_GUEST_NETWORK_BY_SSID
	char guest_status[16]={0};
#endif

    MESH_CLIENT_INFO_T ClientInfo;
    MESH_GUEST_CLIENT_INFO_T gClientInfo;

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else
    {
        memset(target_mac, 0x00, sizeof(target_mac));
        get_json_string_from_query(query_str, target_mac, "TargetMeshMac");

        if(0 == strlen(target_mac) || check_mesh_private_mac(target_mac))
        {
            memset(&ClientInfo, 0x00, sizeof(ClientInfo));
            memset(&gClientInfo, 0x00, sizeof(gClientInfo));
            //######################################Client######################################//
            //get guest 5G client's info
            memset(if_name, 0x00, sizeof(if_name));
            memset(buf, 0x00, sizeof(buf));
            memset(&gClientInfo, 0x00, sizeof(gClientInfo));
            api_get_string_option("wireless.wifi1_ssid_1.ifname", if_name, sizeof(if_name));
            sysutil_interact(buf, sizeof(buf),"wlanconfig %s list|awk 'NR>1 {print $1}'", if_name);

            i = 0;
            ptr = strtok(buf, "\n");

            while(ptr != NULL)
            {
                sysutil_interact(buf2, sizeof(buf2),"cat /tmp/dhcp.leases | grep %s", ptr);
                sscanf(buf2, "%*s %s %s %s %*[^\n]\n", ClientInfo.client5_mac[i], ClientInfo.client5_ip[i], ClientInfo.client5_hostname[i]);

                i++;
                ptr = strtok(NULL, "\n");
            }

            //get guest 24G client's info
            memset(if_name, 0x00, sizeof(if_name));
            memset(buf, 0x00, sizeof(buf));
            api_get_string_option("wireless.wifi0_ssid_1.ifname", if_name, sizeof(if_name));
            sysutil_interact(buf, sizeof(buf),"wlanconfig %s list|awk 'NR>1 {print $1}'", if_name);
            i = 0;
            ptr = strtok(buf, "\n");

            while(ptr != NULL)
            {
                sysutil_interact(buf2, sizeof(buf2),"cat /tmp/dhcp.leases | grep %s", ptr);
                sscanf(buf2, "%*s %s %s %s %*[^\n]\n", ClientInfo.client24_mac[i], ClientInfo.client24_ip[i], ClientInfo.client24_hostname[i]);

                i++;
                ptr = strtok(NULL, "\n");
            }
            //######################################Guest Client######################################//
            //get guest 5G client's info
            memset(if_name, 0x00, sizeof(if_name));
            memset(buf, 0x00, sizeof(buf));
            memset(&gClientInfo, 0x00, sizeof(gClientInfo));

#if HAS_SUPPORT_GUEST_NETWORK_BY_SSID
			memset(guest_status, 0x00, sizeof(guest_status));
			api_get_string_option("wireless.wifi1_ssid_2.guest_network", guest_status, sizeof(guest_status));
			if (strcmp(guest_status, "Enable")==0)
			{
				disabled=0;
			}
			else
			{
				disabled=1;
			}
#else
            api_get_integer_option("wireless.wifi1_guest.disabled", &disabled);
#endif
            if(!disabled)
            {
#if HAS_SUPPORT_GUEST_NETWORK_BY_SSID
				api_get_string_option("wireless.wifi1_ssid_2.ifname", if_name, sizeof(if_name));
#else
                api_get_string_option("wireless.wifi1_guest.ifname", if_name, sizeof(if_name));
#endif
                sysutil_interact(buf, sizeof(buf),"wlanconfig %s list|awk 'NR>1 {print $1}'", if_name);
            }

            i = 0;
            ptr = strtok(buf, "\n");

            while(ptr != NULL)
            {
                sysutil_interact(buf2, sizeof(buf2),"cat /tmp/dhcp.leases | grep %s", ptr);
                sscanf(buf2, "%*s %s %s %s %*[^\n]\n", gClientInfo.guest5_mac[i], gClientInfo.guest5_ip[i], gClientInfo.guest5_hostname[i]);

                i++;
                ptr = strtok(NULL, "\n");
            }

            //get guest 24G client's info
            memset(if_name, 0x00, sizeof(if_name));
            memset(buf, 0x00, sizeof(buf));
#if HAS_SUPPORT_GUEST_NETWORK_BY_SSID
			memset(guest_status, 0x00, sizeof(guest_status));
			api_get_string_option("wireless.wifi0_ssid_2.guest_network", guest_status, sizeof(guest_status));
			if (strcmp(guest_status, "Enable")==0)
			{
				disabled=0;
			}
			else
			{
				disabled=1;
			}
#else
            api_get_integer_option("wireless.wifi0_guest.disabled", &disabled);
#endif
            if(!disabled)
            {
#if HAS_SUPPORT_GUEST_NETWORK_BY_SSID
				api_get_string_option("wireless.wifi0_ssid_2.ifname", if_name, sizeof(if_name));
#else
                api_get_string_option("wireless.wifi0_guest.ifname", if_name, sizeof(if_name));
#endif
                sysutil_interact(buf, sizeof(buf),"wlanconfig %s list|awk 'NR>1 {print $1}'", if_name);
            }
            i = 0;
            ptr = strtok(buf, "\n");

            while(ptr != NULL)
            {
                sysutil_interact(buf2, sizeof(buf2),"cat /tmp/dhcp.leases | grep %s", ptr);
                sscanf(buf2, "%*s %s %s %s %*[^\n]\n", gClientInfo.guest24_mac[i], gClientInfo.guest24_ip[i], gClientInfo.guest24_hostname[i]);

                i++;
                ptr = strtok(NULL, "\n");
            }

            get_mesh_divece_client_info_json_cb(pkt, ClientInfo, gClientInfo, OK_STR);
            //get_json_mesh_node_info_cb
            //get_speedtest_test_result_cb(pkt);
        }
        else
        {
            memset(buf, 0x00, sizeof(buf));
            sys_interact(buf, sizeof(buf), "/sbin/mesh.sh mac_to_ipv6_unique_local %s | tr -d '\\n'", target_mac);
            redirect_to_target_mesh(pkt, buf);
        }
    }

    return 0;
}

/*****************************************************************
* NAME:    get_mesh_trace_route_simple_result_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_mesh_trace_route_simple_result_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *result;
    char mac[32];
    char mesh_mac[32];
    char source[32];
    char source_mac[32];
    char destination[32];
    char result_file[128];

    result = ERROR_STR;
    memset(mac, 0x00, sizeof(mac));
    memset(mesh_mac, 0x00, sizeof(mesh_mac));
    memset(source, 0x00, sizeof(source));
    memset(source_mac, 0x00, sizeof(source_mac));
    memset(destination, 0x00, sizeof(destination));
    memset(result_file, 0x00, sizeof(result_file));

    if(get_json_string_from_query(query_str, mesh_mac, "MeshMAC"))
    {
        remove_colon_from_mac(mesh_mac, mac);
        sprintf(result_file, MESH_TRACE_ROUTE_RESULT_FILE"_%s", mac);
    }

    get_json_string_from_query(query_str, destination, "Destination");

    if(FALSE == is_mesh_function_enabled())
    {
        send_simple_response(pkt, ERROR_MESH_DISABLED_STR);
    }
    else if(FALSE == check_mesh_account(query_str))
    {
        send_simple_response(pkt, ERROR_UNAUTHORIZED_STR);
    }
    else if(is_mesh_trace_route_in_process(result_file, destination))
    {
        send_simple_response(pkt, ERROR_PROCESS_IS_RUNNING_STR);
    }
    else
    {
        get_json_string_from_query(query_str, source_mac, "Source");

        if(strchr(source_mac, ':'))
        {
            mac_to_ipv6(source_mac, source);
        }
        else
        {
            sprintf(source, "%s", source_mac);
        }

        if(check_mesh_global_ipv6(source))
        {
            if(sysutil_check_file_existed(result_file))
            {
                result = OK_STR;
            }

            get_mesh_trace_route_result_json_cb(pkt, result_file, result);
        }
        else
        {
            redirect_to_target_mesh(pkt, source);
        }
    }

    return 0;
}

#if HAS_MESH_STATIC_ROUTE
/*****************************************************************
* NAME:    set_mesh_allow_mac_list_cb
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:  set mesh allow mac list
* Author:  20171106 Leonard
* Modify:
******************************************************************/
int set_mesh_allow_mac_list_cb(HTTPS_CB *pkt)
{
    char *query_str;
    char *return_str;
    char if_buf[8]={0};
    int  i, ret=0, mesh_sr_en=0;
    MESH_ALLOW_MACLIST_T setting;

    /* check pkt */
    if (pkt == NULL) {
        goto send_pkt;
    }

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");
    return_str = ERROR_STR;

    if (query_str == NULL) {
        return_str = ERROR_STR;
        goto send_pkt;
    }

    if (check_mesh_account(query_str) == FALSE) {
        return_str = ERROR_UNAUTHORIZED_STR;
        goto send_pkt;
    }

    ret = __is_mesh_function_enabled();
    if (ret == 0) {
        return_str = ERROR_MESH_DISABLED_STR;
        goto send_pkt;
    }
    else if (ret == 2) {
        sprintf(if_buf, "%s", "ath35");
    }
    else if (ret == 1) {
        sprintf(if_buf, "%s", "ath32");
    }

    /* parse parameters */
    if (get_json_integer_from_query(query_str, &mesh_sr_en, "StaticRoute") != 1)
        return_str = ERROR_STR;

    /* produce script */
    APPAGENT_SYSTEM("echo \"wlanconfig %s nawds flushallowlist\" > %s", if_buf, MESH_ALLOW_MAC_LIST); // flush earlier allow flag

    memset(&setting, 0x00, sizeof(setting));

    if (parse_json_allow_mac_list_cb(query_str, &setting, &return_str) == TRUE) {
        for (i=0; i < setting.aw_num; i++) {
            APPAGENT_SYSTEM("echo \"wlanconfig %s nawds allow-repeater %s\" >> %s", if_buf, setting.aw_mac[i], MESH_ALLOW_MAC_LIST);
        }
    }
    if (mesh_sr_en) { // delay for enable static-route mode
        APPAGENT_SYSTEM("echo \"sleep 15\" >> %s", MESH_ALLOW_MAC_LIST);
    }

    //APPAGENT_SYSTEM("echo \"echo %d > /proc/mesh_robust\" >> %s", (!mesh_sr_en)? 1:0, MESH_ALLOW_MAC_LIST);
    APPAGENT_SYSTEM("echo \"echo %d > /proc/mesh_static_route\" >> %s", mesh_sr_en, MESH_ALLOW_MAC_LIST);

    APPAGENT_SYSTEM("sh %s &", MESH_ALLOW_MAC_LIST);

send_pkt:

    /* Send response packet */
    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    inform_mesh_static_route
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:  slave inform gateway topology changes
* Author:  20171205 Leonard
* Modify:
******************************************************************/
int inform_mesh_static_route(HTTPS_CB *pkt)
{
    char *query_str;
    char *return_str;
    char mesh_id[64]={0};
    char mesh_key[64]={0};
    int  ret=0, mesh_sr_en=0;

    /* check pkt */
    if (pkt == NULL) {
        goto send_pkt;
    }

    query_str = get_env(&pkt->envcfg, "QUERY_STRING");
    return_str = ERROR_STR;

    if (query_str == NULL) {
        return_str = ERROR_STR;
        goto send_pkt;
    }

    if (check_mesh_account(query_str) == FALSE) {
        return_str = ERROR_UNAUTHORIZED_STR;
        goto send_pkt;
    }

    ret = __is_mesh_function_enabled();
    if (ret == 0) {
        return_str = ERROR_MESH_DISABLED_STR;
        goto send_pkt;
    }

    /* parse parameters */
    if (get_json_integer_from_query(query_str, &mesh_sr_en, "StaticRoute") != 1)
        return_str = ERROR_STR;

    /* execute script */
    if (sysutil_check_file_existed(MESH_SR_INFORM_BH)) {
        query_str=get_env(&pkt->envcfg, "QUERY_STRING");
        APPAGENT_SYSTEM("sh %s %d &", MESH_SR_INFORM_BH, mesh_sr_en);
    }

send_pkt:

    /* Send response packet */
    send_simple_response(pkt, return_str);

    return 0;
}
#endif
