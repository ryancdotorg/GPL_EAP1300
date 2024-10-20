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
;    File    : lan_setting.c
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
#include "lan_setting.h"
#include "app_agent.h"
#include "api_tokens.h"
#include "sysCore.h"
#include "sysAddr.h"
#include "variable/api_lan.h"
#include "variable/api_dhcp.h"
#include "admin_cfg.h"

/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/
dhcpLeaseTimeData_t dhcpLeaseTimeDataArr[] = {
    { "Half hour",    "30m" },
    { "One hour",     "1h"  },
    { "Two hours",    "2h"  },
    { "Half day",     "12h" },
    { "One day",      "1d" },
    { "Two days",     "2d" },
    { "One week",     "7d"  },
    { "Two weeks",    "14d" },
    { "Forever",      "infinite" }
};
/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/

/*****************************************************************
* NAME:    isDuplicateMac
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool isDuplicateMac(BLOCKED_CLIENT_LIST_T *setting)
{
    bool result;
    int i, j;

    result = FALSE;

    for(i = 0; i < MAX_CLIENT_DEVICE; i++)
    {
        if((TRUE == result) || (0 == strlen(setting->mac[i])))
        {
            break;
        }

        for(j = i+1; j < MAX_CLIENT_DEVICE; j++)
        {
            if(0 == strlen(setting->mac[j]))
            {
                break;
            }

            if(0 == strcmp(setting->mac[i], setting->mac[j]))
            {
                result = TRUE;
                break;
            }
        }
    }

    return result;
}

/*****************************************************************
* NAME:    get_lan_settings
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_lan_settings(HTTPS_CB *pkt)
{
    char *result;
    char host[17+1], netmask[17+1];
    char start_addr[17+1], end_addr[17+1];
    char lease_time[7+1], domain_name[31+1];
    int ignore = 0, start = 0, limit = 0, i = 0;
    ROUTER_LAN_SETTINGS_T setting;

    if(NULL == pkt)
        return -1;

    memset(&setting, 0, sizeof(setting));

    memset(host, 0, sizeof(host));
    sysutil_get_lan_ipaddr(host, sizeof(host));
    sprintf(setting.router_ip_address, "%s", host);

    memset(netmask, 0, sizeof(netmask));
    sysutil_get_lan_netmask(netmask, sizeof(netmask));
    sprintf(setting.router_subnet_mask, "%s", netmask);

    api_get_bool_option(DHCP_LAN_IGNORE_OPTION, &ignore);
    setting.dhcp_server_enabled = !ignore;

    memset(lease_time, 0, sizeof(lease_time));
    api_get_dhcp_release(DHCP_LAN_LEASETIME_OPTION, lease_time, sizeof(lease_time));
    for (i=0; i<T_NUM_OF_ELEMENTS(dhcpLeaseTimeDataArr); i++)
    {
        if (!strcmp(dhcpLeaseTimeDataArr[i].config, lease_time))
        {
            sprintf(setting.dhcp_leasetime, "%s", dhcpLeaseTimeDataArr[i].title);
            break;
        }
    }

    api_get_dhcp_range_start(DHCP_LAN_START_OPTION, &start);
    memset(start_addr, 0, sizeof(start_addr));
    api_get_dhcp_start_addr(host, start, start_addr, sizeof(start_addr));
    sprintf(setting.dhcp_start, "%s", start_addr);

    api_get_dhcp_range_limit(DHCP_LAN_LIMIT_OPTION, &limit);
    memset(end_addr, 0, sizeof(end_addr));
    api_get_dhcp_end_addr(start_addr, netmask, limit, end_addr, sizeof(end_addr));
    sprintf(setting.dhcp_end, "%s", end_addr);

    memset(domain_name, 0, sizeof(domain_name));
    api_get_string_option(DHCP_DNSMASQ_DOMAIN_OPTION, domain_name, sizeof(domain_name));
    sprintf(setting.domain_name, "%s", domain_name);

    result = OK_STR;

    if(pkt->json)
        get_lan_settings_json_cb(pkt, &setting, result);

    return 0;
}

/*****************************************************************
* NAME:    get_lan_options
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool get_lan_options(ROUTER_LAN_SETTINGS_T *setting)
{
	char host[17+1], netmask[17+1];
	char start_addr[17+1], end_addr[17+1];
	int ignore = 0, start = 0, limit = 0;

	if(NULL == setting)
		return FALSE;

	memset(host, 0, sizeof(host));
	sysutil_get_lan_ipaddr(host, sizeof(host));
	sprintf(setting->router_ip_address, "%s", host);

	memset(netmask, 0, sizeof(netmask));
	sysutil_get_lan_netmask(netmask, sizeof(netmask));
	sprintf(setting->router_subnet_mask, "%s", netmask);

	api_get_bool_option(DHCP_LAN_IGNORE_OPTION, &ignore);
	setting->dhcp_server_enabled = !ignore;

	if(setting->dhcp_server_enabled)
	{
		api_get_dhcp_range_start(DHCP_LAN_START_OPTION, &start);
		memset(start_addr, 0, sizeof(start_addr));
		api_get_dhcp_start_addr(host, start, start_addr, sizeof(start_addr));
		sprintf(setting->dhcp_start, "%s", start_addr);

		api_get_dhcp_range_limit(DHCP_LAN_LIMIT_OPTION, &limit);
		memset(end_addr, 0, sizeof(end_addr));
		api_get_dhcp_end_addr(start_addr, netmask, limit, end_addr, sizeof(end_addr));
		sprintf(setting->dhcp_end, "%s", end_addr);
	}

	return TRUE;
}

#if 1
/*****************************************************************
* NAME:    get_client_status
* ---------------------------------------------------------------
* FUNCTION:
* INPUT:
* OUTPUT:
* Author:
* Modify:
******************************************************************/
int get_client_status(HTTPS_CB *pkt){
    char *result;
    if(NULL == pkt)
        return -1;

    result = OK_STR;

    if(pkt->json)
        get_client_status_json_cb(pkt, result);

    return 0;
}
#endif
/*****************************************************************
* NAME:    set_lan_settings
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_lan_settings(HTTPS_CB *pkt)
{
    char lanAddr[17+1], leaseTime[7+1];
    char *return_str;
    bool result = TRUE;
    int i = 0;
    int isReboot = 0;
    int ignore = 0;
    int start = 0;
    int limit = 0;
    ROUTER_LAN_SETTINGS_T setting;
#if SUPPORT_WAN_SETTING
    char wanAddr[17+1], wanMask[17+1];
#endif
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    printf("[%s][%d] query_str:[%s]\n", __FUNCTION__, __LINE__, query_str); 

    if(NULL == pkt || 0 == strlen(query_str))
        goto send_pkt;

    memset(&setting, 0, sizeof(setting));

    if(pkt->json)
        result &= parse_lan_settings_json_cb(query_str, &setting, return_str);

    if(TRUE != result) goto send_pkt;
 
    memset(lanAddr, 0, sizeof(lanAddr));
    memset(leaseTime, 0, sizeof(leaseTime));
#if SUPPORT_WAN_SETTING
    memset(wanAddr, 0, sizeof(wanAddr));
    memset(wanMask, 0, sizeof(wanMask));
#endif

    /* Check value */
    if(!api_check_ip_addr(setting.router_ip_address))
    {
        return_str = ERROR_IP_ADDRESS_STR;
        goto send_pkt;
    }

    if(!api_check_mask_addr(setting.router_subnet_mask))
    {
        return_str = ERROR_SUBNET_MASK_STR;
        goto send_pkt;
    }
#if APP_AGENT_SUPPORT_ENSHARE
#else
#if SUPPORT_WAN_SETTING
    sysutil_get_wan_ipaddr(wanAddr, sizeof(wanAddr));
    sysutil_get_wan_netmask(wanMask, sizeof(wanMask));

    sprintf(wanAddr,"%s", strcmp(wanAddr, PPP_FAKE_WAN_IP) == 0 ? "---" : 
#if HAS_WAN_3G
                          strcmp(wanAddr, PPP_FAKE_3G_WAN_IP) == 0 ? "---" : 
#endif
#if HAS_WAN_L2TP
                          strcmp(wanAddr, PPP_FAKE_L2TP_WAN_IP) == 0 ? "---" : 
#endif
                          wanAddr);

    sprintf(wanMask,"%s", strcmp(wanAddr, PPP_FAKE_WAN_IP) == 0 ? "---" : 
#if HAS_WAN_3G
                          strcmp(wanAddr, PPP_FAKE_3G_WAN_IP) == 0 ? "---" : 
#endif
#if HAS_WAN_L2TP
                          strcmp(wanAddr, PPP_FAKE_L2TP_WAN_IP) == 0 ? "---" : 
#endif
                          wanMask);
#endif
#endif
    if(!api_check_subnetIPwithNetAndBroadcast(setting.router_ip_address, setting.router_subnet_mask, setting.router_ip_address,1))
    {
        return_str = ERROR_IP_ADDRESS_STR;
        goto send_pkt;
    }
#if 0
    if(api_check_is_two_same_subnet(setting.router_ip_address, wanAddr, setting.router_subnet_mask, wanMask) && 0 != strcmp(wanAddr, "---"))
    {
        return_str = ERROR_IP_ADDRESS_STR;
        goto send_pkt;
    }
#endif
#if APP_AGENT_SUPPORT_ENSHARE
#else
    if(!api_check_ip_addr(setting.dhcp_start))
    {
        return_str = ERROR_DHCP_START_ADDRESS_STR;
        goto send_pkt;
    }

    if(api_check_is_device_ipaddr(setting.dhcp_start))
    {
        return_str = ERROR_DHCP_START_ADDRESS_STR;
        goto send_pkt;
    }

    if(!api_check_is_same_subnet(setting.dhcp_start, setting.router_ip_address, setting.router_subnet_mask))
    {
        return_str = ERROR_DHCP_START_ADDRESS_STR;
        goto send_pkt;
    }

    if(!api_check_ip_addr(setting.dhcp_end))
    {
        return_str = ERROR_DHCP_END_ADDRESS_STR;
        goto send_pkt;
    }

    if(api_check_is_device_ipaddr(setting.dhcp_end))
    {
        return_str = ERROR_DHCP_END_ADDRESS_STR;
        goto send_pkt;
    }

    if(!api_check_is_same_subnet(setting.dhcp_end, setting.router_ip_address, setting.router_subnet_mask))
    {
        return_str = ERROR_DHCP_END_ADDRESS_STR;
        goto send_pkt;
    }

    if(api_get_dhcp_limit(setting.dhcp_end, setting.dhcp_start, setting.router_subnet_mask, &limit))
    {
        return_str = ERROR_DHCP_START_ADDRESS_STR;
        goto send_pkt;
    }

    if(api_check_ipaddr_in_range(setting.router_ip_address, setting.dhcp_start, setting.dhcp_end))
    { // lan ip can't in dhcp range.
        return_str = ERROR_DHCP_START_ADDRESS_STR;
        goto send_pkt;
    }

    if(strlen(setting.domain_name) == 0)
    {
        return_str = ERROR_DOMAIN_NAME_STR;
        goto send_pkt;
    }

    if (!api_check_domain_name(setting.domain_name))
    {
        return_str = ERROR_DOMAIN_NAME_STR;
        goto send_pkt;
    }
#endif

    /* Set the setting into /etc/config */
    sysutil_get_lan_ipaddr(lanAddr, sizeof(lanAddr));
    if(strcmp(setting.router_ip_address, lanAddr))
        isReboot = 1;

    api_set_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION, setting.router_ip_address, sizeof(setting.router_ip_address));
    api_set_lan_netmask_option(NETWORK_LAN_NETMASK_OPTION, setting.router_subnet_mask, sizeof(setting.router_subnet_mask));
    ignore = setting.dhcp_server_enabled;
    api_set_bool_option(DHCP_LAN_IGNORE_OPTION, !ignore);

    #if APP_AGENT_SUPPORT_ENSHARE
    #else
    for (i=0; i<T_NUM_OF_ELEMENTS(dhcpLeaseTimeDataArr); i++)
    {
        if (!strcmp(dhcpLeaseTimeDataArr[i].title, setting.dhcp_leasetime))
        {
            snprintf(leaseTime, sizeof(leaseTime), "%s", dhcpLeaseTimeDataArr[i].config);
            break;
        }
    }
    api_set_string_option(DHCP_LAN_LEASETIME_OPTION, leaseTime, sizeof(leaseTime));
    api_get_dhcp_start(setting.dhcp_start, &start);
    api_set_integer_option(DHCP_LAN_START_OPTION, start);
    api_set_integer_option(DHCP_LAN_LIMIT_OPTION, limit);
    api_set_string_option(DHCP_DNSMASQ_DOMAIN_OPTION, setting.domain_name, sizeof(setting.domain_name));
    #endif
#if 0
    /* Save new token value */
    if(result)
    {
        if(isReboot)
        {
            send_simple_response(pkt, REBOOT_STR);
            SYSTEM("uci commit");
            SYSTEM("reboot &");
            return 0;
        }    
        else
        {
            SYSTEM("uci commit");
            SYSTEM("ubus call network reload");
        }
    }
#endif
    if(TRUE == result)
    {
        return_str = OK_STR;
    }
send_pkt:
    /* Send response packet */
    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    get_lan_access_control_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_lan_access_control_list_cb(HTTPS_CB *pkt){

    int i, enable_macfilter, traffic_type;
    char *result;
    char tok[] = {0};
    char macaddr[18+1]={0}, comment[512+1]={0};
    ACCESS_CONTROL_LIST_T settings;
    //macfiltering_t m;

    memset(&settings, 0, sizeof(settings));
    result = NULL;
    if(NULL == pkt){
        return -1;
    }

    /* get value from tokens to struct */
    api_get_integer_option(NETWORK_LAN_MACFILTERING_ENABLE_OPTION,&enable_macfilter);
    settings.enable_macfilter = enable_macfilter;                     // MAC Filter enable : enable is 1(true), disable is 0(false)
    api_get_integer_option(NETWORK_LAN_MACFILTERING_POLICY_OPTION,&traffic_type);
    settings.traffic_type = !traffic_type;                            // MAC Filter mode : Deny is 1(true), Allow is 0(false)
    
    for(i = 0; i < NUM_LAN_MAC_FILTERS; i++){
        memset(&macaddr, 0, sizeof(macaddr));
        memset(&comment, 0, sizeof(comment));
        api_get_access_control_list_macaddr_option(i+1, macaddr, sizeof(macaddr));
        api_get_access_control_list_comment_option(i+1, comment, sizeof(comment));
        if (0 != strlen(macaddr)){
            settings.is_enable[i] = 1;
            strcpy(settings.mac[i], macaddr);
            strcpy(settings.comment[i], comment);
        }
    }

    result = OK_STR;

    if(pkt->json)
    {
        get_lan_access_control_list_json_cb(pkt, &settings);
    }

    return 0;
}

/*****************************************************************
* NAME:    set_lan_access_control_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_lan_access_control_list_cb(HTTPS_CB *pkt){
    int i, j=1, target;
    char test[18+1];
    char rule_section[10];
    char macaddr[18+1]={0}, comment[20]={0};
    char rule_name_format[18]="MAC filtering %d";
    char rule_name[18]={0};
    char src[4]="lan", src_mac[18+1]={0}, rule_comment[20]={0};
    bool result = TRUE;
    char *return_str;

    ACCESS_CONTROL_LIST_T settings;
#if 0
    char *query_str="{\n" \
                    "  \"EnabledMacFilter\" : \"true\",\n" \
                    "  \"TrafficType\" : \"true\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if(NULL == pkt || 0 == strlen(query_str))
    {
        goto send_pkt;
    }

    if(pkt->json)
    {
        result &= parse_set_lan_access_control_list_json_cb(query_str, &settings, return_str);
    }

    if(TRUE != result) goto send_pkt;

    api_set_integer_option(NETWORK_LAN_MACFILTERING_ENABLE_OPTION, settings.enable_macfilter);
    api_set_integer_option(NETWORK_LAN_MACFILTERING_POLICY_OPTION, !settings.traffic_type);
    for (i = 0; i < NUM_LAN_MAC_FILTERS; i++){    
        api_del_lan_access_control_list_section(i+1);
    }

    if (settings.enable_macfilter == 1){
        api_set_sn_firewall_to_firewall_rule(!settings.traffic_type);
    }

    if(result)
    {    
        return_str = OK_STR;
        APPAGENT_SYSTEM("uci commit");
        APPAGENT_SYSTEM("ubus call network reload &");
    }

send_pkt:
    /* Send response packet */
    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    add_lan_access_control_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int add_lan_access_control_list_cb(HTTPS_CB *pkt){

    bool result = TRUE;
    char *return_str;
    int i, j=0,enable_macfilter=0 , traffic_type=0, available_index;
    char sn_macaddr[18+1]={0},sn_comment[20]={0}, rule_section[10];
    char rule_name_format[18]="MAC filtering %d";
    char rule_name[18]={0};
    char src[4]="lan", src_mac[18+1]={0}, rule_comment[20]={0};
    char mac[20] = {0};
    char exist_macaddr[20]={0};
    char comment[20] = {0};
    //macfiltering_t m;

#if 0
    char *query_str="{\n" \
                    "  \"MacAddress\" : \"00:00:00:00:00:06\",\n" \
                    "  \"Comment\" : \"TESTAAA\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif

    //memset(&m, 0, sizeof(m));
    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if(NULL == pkt || 0 == strlen(query_str))
    {
        goto send_pkt;
    }

    if(pkt->json)
    {
        result &= parse_add_lan_access_control_list_json_cb(query_str, mac, &comment, return_str);
    }

    if(TRUE != result) goto send_pkt;

    /* check */
    if(!api_check_mac(mac)){
        return_str = ERROR_MAC_ADDRESS_STR;
        goto send_pkt;
    }
    if(strlen(comment)>16)
    {
        return_str = ERROR_COMMENT_STR;
        goto send_pkt;
    }    
    sysutil_lower2upper_mac(mac);
    for(i = 0; i < NUM_LAN_MAC_FILTERS; i++){
        memset(&exist_macaddr, 0, sizeof(exist_macaddr));
        api_get_access_control_list_macaddr_option(i+1, exist_macaddr, sizeof(exist_macaddr));       
        if (0 == strcmp(exist_macaddr, mac)){
            return_str = ERROR_DUPLICATE_MAC_ADDRESS_STR;
            goto send_pkt;            
        }
        if (strlen(exist_macaddr) != 0){
            j++;
        }
        else{
           available_index = i+1;           /* find available index to add */
        }
        if (j == NUM_LAN_MAC_FILTERS){
            return_str = ERROR_FULL_TABLE_STR;
            goto send_pkt;
        }
    }
    /* set value */
    api_get_integer_option(NETWORK_LAN_MACFILTERING_ENABLE_OPTION,&enable_macfilter);
    api_get_integer_option(NETWORK_LAN_MACFILTERING_POLICY_OPTION,&traffic_type);

    api_set_sn_firewall_rule_src_mac_option(available_index, mac, sizeof(mac));
    api_set_sn_firewall_rule_comment_option(available_index, comment, sizeof(comment));
             
    for (i = 0; i < NUM_LAN_MAC_FILTERS; i++){    
        api_del_lan_access_control_list_section(i+1);
    }
    /* if enable sn_firewall --> firewall */
    if (enable_macfilter == 1){
        api_set_sn_firewall_to_firewall_rule(traffic_type);
    }
    if(result)
    {    
        return_str = OK_STR;
        APPAGENT_SYSTEM("uci commit");
        APPAGENT_SYSTEM("ubus call network reload &");
    }
send_pkt:
    /* Send response packet */
    send_simple_response(pkt, return_str);

    return 0;
}

/*****************************************************************
* NAME:    delete_lan_access_control_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int delete_lan_access_control_list_cb(HTTPS_CB *pkt){

    char del_action[640] = {0}; // 32 mac address
    char tmp_string[640] = {0};
    char *del_mac;
    bool result = TRUE;
    char *return_str;
    int i, j;
    char sn_macaddr[18+1]={0}, sn_comment[20]={0};
    char tok[30] = {0};
    char mac[20] = {0};
    char mac_tmp_input[20] = {0};
    char mac_tmp[20] = {0};

    int enable_macfilter=0, traffic_type=0; 
    char rule_name_format[18]="MAC filtering %d";
    char rule_section[10], rule_name[18]={0};
    char src[4]="lan", src_mac[18+1]={0}, rule_comment[20]={0};
    char comment[20] = {0};

#if 0
    char *query_str="{\n" \
                    "  \"DeleteMacAddress\" : \"00:00:00:00:00:06\"\n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if(NULL == pkt || 0 == strlen(query_str))
    {
        goto send_pkt;
    }

    if(pkt->json)
    {
        result &= parse_del_lan_access_control_list_json_cb(query_str, del_action, return_str);
    }

    if(TRUE != result) goto send_pkt;

    RemoveSpaces(del_action);

    /* check query is smaller than array */
    if(strlen(del_action) >= sizeof(del_action)-1)
    {
        goto send_pkt;
    }

    /*check*/
    if(0 == strcmp(del_action,"DeleteAll")){
    }
    else{
        strcpy(tmp_string, del_action);
        /* check */
        del_mac = strtok(tmp_string, ",");
        while(del_mac != NULL)
        {
            if(!api_check_mac(del_mac)){
                return_str = ERROR_MAC_ADDRESS_STR;
                goto send_pkt;
            }
            del_mac = strtok(NULL, ",");
        }
    }

    for (i = 0; i < NUM_LAN_MAC_FILTERS; i++){    
        api_del_lan_access_control_list_section(i+1);
    }

    /* delete all */
    if(0 == strcmp(del_action,"DeleteAll")){
        for(i = 0; i < NUM_LAN_MAC_FILTERS; i++){
            api_del_sn_firewall_mac_option(i+1);
            api_del_sn_firewall_comment_option(i+1);
        }
    }
    /* delete single or multi client list */
    else
    {
        strcpy(tmp_string, del_action);
        /* set value */
        del_mac = strtok(del_action, ",");
        while(del_mac != NULL)
        {
            api_lower2upper_mac(del_mac);
            for(i = 0; i < NUM_LAN_MAC_FILTERS; i++){
                memset(&sn_macaddr, 0, sizeof(sn_macaddr));
                api_get_access_control_list_macaddr_option(i+1, sn_macaddr, sizeof(sn_macaddr));      //get sn_firewall mac
                if(strcmp(sn_macaddr, del_mac) == 0){
                    api_del_sn_firewall_mac_option(i+1);
                    api_del_sn_firewall_comment_option(i+1);
                }
            }

            del_mac = strtok(NULL, ",");
        }
    }

    /* set value to firewall*/
    api_get_integer_option(NETWORK_LAN_MACFILTERING_ENABLE_OPTION,&enable_macfilter);
    api_get_integer_option(NETWORK_LAN_MACFILTERING_POLICY_OPTION,&traffic_type);

    /* if enable sn_firewall --> firewall */
    if (enable_macfilter == 1){
        api_set_sn_firewall_to_firewall_rule(traffic_type);
    }

    /* Save new token value */
    if(result)
    {    
        return_str = OK_STR;
        APPAGENT_SYSTEM("uci commit");
        APPAGENT_SYSTEM("ubus call network reload &");
    }

send_pkt:
    /* Send response packet */
    send_simple_response(pkt, return_str);

    return 0;
}

#if HAS_AP
/*****************************************************************
* NAME:    get_ap_wan_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_ap_wan_settings_cb(HTTPS_CB *pkt){
    char *result = OK_STR;
    char lanIP[17+1], netmask[17+1],lanGW[17+1],dns1[15+1],dns2[15+1],type[10];
    AP_WAN_SETTINGS_T setting;

    if(NULL == pkt)
        return -1;

	memset(type,0x0,sizeof(type));
	memset(&setting,0x0, sizeof(setting));
	api_get_string_option(NETWORK_LAN_PROTO_OPTION,type,sizeof(type));
	memset(lanIP,0x0, sizeof(lanIP));
	memset(netmask,0x0, sizeof(netmask));
	memset(lanGW,0x0, sizeof(lanGW));
	memset(dns1,0x0, sizeof(dns1));
	memset(dns2,0x0, sizeof(dns2));

	if(!strcmp(type,"dhcp"))
	{
	strcpy(setting.type,"DHCP");
    	sysutil_get_lan_ipaddr(lanIP, sizeof(lanIP));
    	sprintf(setting.ip_address, "%s", lanIP);

    	sysutil_get_lan_netmask(netmask, sizeof(netmask));
    	sprintf(setting.subnet_mask, "%s", netmask);

    	sysutil_get_lan_gateway(lanGW, sizeof(lanGW));
    	sprintf(setting.gateway, "%s", lanGW);

    	sysutil_get_lan_dns(1, dns1, sizeof(dns1));
    	sysutil_get_lan_dns(2, dns2, sizeof(dns2));
    	sprintf(setting.dns_primary, "%s", dns1);
    	sprintf(setting.dns_secondary, "%s", dns2);
	}
	else{
	strcpy(setting.type,"Static IP");
	api_get_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION,lanIP,sizeof(lanIP));
	sprintf(setting.ip_address, "%s", lanIP);
	api_get_lan_netmask_option(NETWORK_LAN_NETMASK_OPTION,netmask,sizeof(netmask));
	sprintf(setting.subnet_mask, "%s", netmask);
	api_get_lan_gateway_option(NETWORK_LAN_GATEWAY_OPTION,lanGW,sizeof(lanGW));
	sprintf(setting.gateway, "%s", lanGW);
	api_get_lan_dns_option(NETWORK_LAN_DNS_OPTION,1,dns1,sizeof(dns1));
	sprintf(setting.dns_primary, "%s", dns1);
	api_get_lan_dns_option(NETWORK_LAN_DNS_OPTION,2,dns2,sizeof(dns2));
	sprintf(setting.dns_secondary, "%s", dns2);
	}
    /* Store packet contentfinto buffer and send it out */
#if HAS_AP
    if(pkt->json)
        get_ap_wan_settings_json_cb(pkt, &setting, result);
#endif
    return 0;
}

/*****************************************************************
* NAME:    set_ap_wan_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_ap_wan_settings_cb(HTTPS_CB *pkt)
{
	char lanAddr[17+1], leaseTime[7+1],dns[128];
	char *return_str=OK_STR;
	bool result=TRUE;
	AP_WAN_SETTINGS_T setting;
	char *query_str= get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == pkt || 0 == strlen(query_str))
	{
		return_str = ERROR_STR; 
		return FALSE;
	}
	memset(&setting, 0, sizeof(setting));
#if HAS_AP
	if(pkt->json)
		result = set_ap_wan_settings_json_cb(query_str, &setting, return_str);
#endif
	if(TRUE != result) 
		goto send_pkt;

	api_set_lan_type_option(NETWORK_LAN_PROTO_OPTION,setting.type,sizeof(setting.type));

	if(strcasecmp(setting.type,"dhcp")){
		if(!api_check_ip_addr(setting.ip_address)){
        		return_str = ERROR_IP_ADDRESS_STR;
        		goto send_pkt;
    		}
		if(!api_check_mask_addr(setting.subnet_mask))
		{
        		return_str = ERROR_SUBNET_MASK_STR;
        		goto send_pkt;
		}
		if(!api_check_ip_addr(setting.gateway)){
			return_str = ERROR_GATEWAY_STR;
			goto send_pkt;
		}
		if((strlen(setting.dns_primary) > 0) && !api_check_dns_addr(setting.dns_primary)){
			return_str = ERROR_STR;
			goto send_pkt;
		}
		if((strlen(setting.dns_secondary) > 0) && !api_check_dns_addr(setting.dns_secondary)){
			return_str = ERROR_STR;
			goto send_pkt;
		}
		api_set_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
		api_set_lan_netmask_option(NETWORK_LAN_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
		api_set_lan_gateway_option(NETWORK_LAN_GATEWAY_OPTION, setting.gateway,sizeof(setting.gateway));
		memset(dns,0x0,sizeof(dns));
		if(strlen(setting.dns_secondary) != 0){
		   strcpy(dns,setting.dns_primary);
		   strcat(dns," ");
		   strcat(dns,setting.dns_secondary);
		   api_set_string_option(NETWORK_LAN_DNS_OPTION, dns, sizeof(dns));
		}
		else
		   api_set_string_option(NETWORK_LAN_DNS_OPTION, setting.dns_primary, sizeof(setting.dns_primary));
	}

	if(result){
		if(send_simple_response(pkt, OK_STR) == 0)
		{
			system("uci commit");
		}
		return 0;
	}

send_pkt:
	/* Send response packet */
	send_simple_response(pkt,ERROR_STR);
	return 0;
}

#endif

int get_upnp_settings_cb(HTTPS_CB *pkt)
{
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    char *result_str=OK_STR;
    SYSTEM_UPNP_INFO_T setting;

    if(NULL == pkt)
	return -1;

    memset(&setting, 0, sizeof(setting));
        api_get_string_option("upnpd.config.friendly_name",setting.name,sizeof(setting.name));
        api_get_bool_option("upnpd.config.enable_upnp",&(setting.enableService));
#if HAS_NAT_TRAVERSAL
        api_get_bool_option("nat-traversal.nattraversal.upnpc_enabled",&(setting.enableTraversal));
#endif
    if(pkt->json)
        get_upnp_settings_json_cb(pkt, &setting, result_str);
}

int set_upnp_settings_cb(HTTPS_CB *pkt)
{
        char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
        bool result = FALSE;
#if HAS_IPCAM
        char hostname[31+1]={0};
#endif
        SYSTEM_UPNP_INFO_T settings;

        if(NULL == pkt)
                return -1;

        memset(&settings,0x0,sizeof(settings));
        result = parse_json_upnp_settings(query_str, &settings);
	if(TRUE == result)
        {
                api_set_string_option("upnpd.config.friendly_name",settings.name,sizeof(settings.name));
                api_set_bool_option("upnpd.config.enable_upnp",settings.enableService);
#if HAS_NAT_TRAVERSAL
                api_set_bool_option("nat-traversal.nattraversal.upnpc_enabled",settings.enableTraversal);
#endif
                system("uci commit");
        }   
	APPAGENT_SYSTEM("luci-reload upnpd &");
        send_simple_response(pkt, (TRUE == result)?OK_STR:ERROR_STR);

        return 0;
}

/*****************************************************************
* NAME:    get_ap_wan_all_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_ap_wan_all_settings_cb(HTTPS_CB *pkt)
{
    char *result = OK_STR;
    char lanIP[17+1], netmask[17+1],lanGW[17+1],dns1[15+1],dns2[15+1],type[10],username[128],password[128],gateway[32],service[128],wantype[128],iptype[8];
    char opMode[8], ifName[16], buf[32],gui_ipaddr[32],gui_netmask[32],gui_gw[32];

    AP_WAN_SETTINGS_T setting;
    int mtu,connection_type,idletime,connectionid;
    if(NULL == pkt)
        return -1;

	memset(type,0x0,sizeof(type));
	memset(&setting,0x0, sizeof(setting));
	api_get_string_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,type,sizeof(type));
	memset(lanIP,0x0, sizeof(lanIP));
	memset(netmask,0x0, sizeof(netmask));
	memset(lanGW,0x0, sizeof(lanGW));
	memset(dns1,0x0, sizeof(dns1));
	memset(dns2,0x0, sizeof(dns2));
    memset(username,0x0, sizeof(username));
	memset(password,0x0, sizeof(password));
    memset(gateway,0x0, sizeof(gateway));
	memset(service,0x0, sizeof(service));
	memset(ifName,0x0,sizeof(ifName));
	memset(opMode,0x0,sizeof(opMode));
	memset(buf,0x0,sizeof(buf));
    memset(iptype,0x0, sizeof(iptype));
    memset(gui_ipaddr,0x0, sizeof(gui_ipaddr));
    memset(gui_netmask,0x0, sizeof(gui_netmask));
    memset(gui_gw,0x0, sizeof(gui_gw));

	if(strcmp(type,"dhcp")==0)
    {
        strcpy(setting.type,"DHCP");

        api_get_string_option("system.@system[0].opmode",opMode,sizeof(opMode));

        if(0 == strcmp(opMode, "ar"))
        {
            api_get_string_option(NETWORK_WAN_IFNAME_OPTION,ifName,sizeof(ifName));

            if(0 != strlen(ifName))
            {
                sysutil_get_interface_ipaddr(ifName, lanIP, sizeof(lanIP));
                sprintf(setting.ip_address, "%s", lanIP);

                sysutil_get_interface_netmask(ifName, netmask, sizeof(netmask));
                sprintf(setting.subnet_mask, "%s", netmask);

                sysutil_get_interface_gateway(ifName, lanGW, sizeof(lanGW));
                sprintf(setting.gateway, "%s", lanGW);

#if SUPPORT_WAN_SETTING
                sysutil_get_wan_dns(1, dns1, sizeof(dns1));
                sysutil_get_wan_dns(2, dns2, sizeof(dns2));
#else
                sysutil_get_interface_dns(ifName, 1, dns1, sizeof(dns1));
                sysutil_get_interface_dns(ifName, 2, dns2, sizeof(dns2));
#endif
                sprintf(setting.dns_primary, "%s", dns1);
                sprintf(setting.dns_secondary, "%s", dns2);
            }
        }
        else
        {
            sysutil_get_lan_ipaddr(lanIP, sizeof(lanIP));
            sprintf(setting.ip_address, "%s", lanIP);

            sysutil_get_lan_netmask(netmask, sizeof(netmask));
            sprintf(setting.subnet_mask, "%s", netmask);

            sysutil_get_lan_gateway(lanGW, sizeof(lanGW));
            sprintf(setting.gateway, "%s", lanGW);

            sysutil_get_lan_dns(1, dns1, sizeof(dns1));
            sysutil_get_lan_dns(2, dns2, sizeof(dns2));
            sprintf(setting.dns_primary, "%s", dns1);
            sprintf(setting.dns_secondary, "%s", dns2);
        }

        sprintf(setting.username, "%s", "");
        sprintf(setting.password, "%s", "");
        setting.mtu=0;
        setting.connection_type=0;
	}
    else if (strcmp(type,"pppoe")==0)
    {
        api_get_integer_option(NETWORK_WAN_MTU_OPTION,&mtu);
        setting.mtu=mtu;
        api_get_string_option(NETWORK_WAN_USERNAME_OPTION,username,sizeof(username));
        sprintf(setting.username, "%s", username);
        api_get_string_option(NETWORK_WAN_PASSWORD_OPTION,password,sizeof(password));
        sprintf(setting.password, "%s", password);
        api_get_integer_option(NETWORK_WAN_CONNTYPE_OPTION,&connection_type);
        setting.connection_type=connection_type;
        api_get_string_option(NETWORK_WAN_SERVICE_OPTION,service,sizeof(service));
        sprintf(setting.service, "%s", service);
        sprintf(setting.gateway, "%s", "");
#if SUPPORT_WAN_SETTING
        sprintf(ifName, "pppoe-wan");
        sysutil_get_interface_ipaddr(ifName, setting.ip_address, sizeof(setting.ip_address));
        sysutil_get_interface_netmask(ifName, setting.subnet_mask, sizeof(setting.subnet_mask));
        sysutil_get_interface_gateway(ifName, setting.gateway, sizeof(setting.gateway));

        sysutil_get_wan_dns(1, dns1, sizeof(dns1));
        sysutil_get_wan_dns(2, dns2, sizeof(dns2));
        sprintf(setting.dns_primary, "%s", dns1);
        sprintf(setting.dns_secondary, "%s", dns2);
#endif
        strcpy(setting.type,"PPPoE");
    }
    else if(strcmp(type,"l2tp")==0)
    { 
        sprintf(ifName, "l2tp-l2tp");
        sysutil_get_interface_ipaddr(ifName, setting.ip_address, sizeof(setting.ip_address));
        sysutil_get_interface_netmask(ifName, setting.subnet_mask, sizeof(setting.subnet_mask));
        sysutil_get_interface_gateway(ifName, setting.gateway, sizeof(setting.gateway));

        sysutil_get_interface_dns("l2tp", 1, setting.dns_primary, sizeof(setting.dns_primary));
        sysutil_get_interface_dns("l2tp", 2, setting.dns_secondary, sizeof(setting.dns_secondary));    

    	api_get_integer_option(NETWORK_WAN_L2TP_MTU_OPTION,&mtu);
        setting.mtu=mtu;
        api_get_string_option(NETWORK_WAN_L2TP_USERNAME_OPTION,username,sizeof(username));
        sprintf(setting.username, "%s", username);
        api_get_string_option(NETWORK_WAN_L2TP_PASSWORD_OPTION,password,sizeof(password));
        sprintf(setting.password, "%s", password);
        api_get_integer_option(NETWORK_WAN_L2TP_CONNTYPE_OPTION,&connection_type);        
        setting.connection_type=0;


        
        api_get_string_option(NETWORK_WAN_L2TP_GATEWAY_OPTION,setting.servergatewayOpt,sizeof(setting.servergatewayOpt));

        api_get_string_option(NETWORK_WAN_L2TP_GUI_IPADDR_OPTION,setting.gui_ipaddr_opt,sizeof(setting.gui_ipaddr_opt));

        api_get_string_option(NETWORK_WAN_L2TP_GUI_NETMASK_OPTION,setting.gui_netmaskOpt,sizeof(setting.gui_netmaskOpt));

        api_get_string_option(NETWORK_WAN_L2TP_GUI_GATEWAY_OPTION,setting.gui_gwOpt,sizeof(setting.gui_gwOpt));





        api_get_string_option(NETWORK_WAN_L2TP_IPTYPE_OPTION,iptype,sizeof(iptype));
        sprintf(setting.iptype, "%s", iptype);

        //api_get_string_option(NETWORK_WAN_L2TP_GUI_IPADDR_OPTION,gui_ipaddr,sizeof(gui_ipaddr));
        //sprintf(setting.ip_address, "%s", setting.ip_address);
        //api_get_string_option(NETWORK_WAN_L2TP_GUI_NETMASK_OPTION,gui_netmask,sizeof(gui_netmask));
        //sprintf(setting.subnet_mask, "%s", gui_netmask);
        //api_get_string_option(NETWORK_WAN_L2TP_GUI_GATEWAY_OPTION,gui_gw,sizeof(gui_gw));
        sprintf(setting.gui_gw, "%s", setting.gateway);

        strcpy(setting.type,"L2TP");
	}
    else if (strcmp(type,"pptp")==0)
    {
        sprintf(ifName, "pptp-pptp");
        sysutil_get_interface_ipaddr(ifName, setting.ip_address, sizeof(setting.ip_address));
        sysutil_get_interface_netmask(ifName, setting.subnet_mask, sizeof(setting.subnet_mask));
        sysutil_get_interface_gateway(ifName, setting.gateway, sizeof(setting.gateway));

        sysutil_get_interface_dns("pptp", 1, dns1, sizeof(dns1));
        sysutil_get_interface_dns("pptp", 2, dns2, sizeof(dns2));

        sprintf(setting.dns_primary, "%s", dns1);
        sprintf(setting.dns_secondary, "%s", dns2);

        strcpy(setting.type,"PPTP");
        api_get_integer_option(NETWORK_WAN_PPTP_MTU_OPTION,&mtu);
        setting.mtu=mtu;
        api_get_string_option(NETWORK_WAN_PPTP_USERNAME_OPTION,username,sizeof(username));
        sprintf(setting.username, "%s", username);
        api_get_string_option(NETWORK_WAN_PPTP_PASSWORD_OPTION,password,sizeof(password));
        sprintf(setting.password, "%s", password);
        api_get_integer_option(NETWORK_WAN_PPTP_CONNTYPE_OPTION,&connection_type);
        setting.connection_type=connection_type;

        api_get_string_option(NETWORK_WAN_PPTP_GATEWAY_OPTION,setting.servergatewayOpt,sizeof(setting.servergatewayOpt));

        api_get_string_option(NETWORK_WAN_PPTP_GUI_IPADDR_OPTION,setting.gui_ipaddr_opt,sizeof(setting.gui_ipaddr_opt));

        api_get_string_option(NETWORK_WAN_PPTP_GUI_NETMASK_OPTION,setting.gui_netmaskOpt,sizeof(setting.gui_netmaskOpt));

        api_get_string_option(NETWORK_WAN_PPTP_GUI_GATEWAY_OPTION,setting.gui_gwOpt,sizeof(setting.gui_gwOpt));

     //    sprintf(setting.dns_primary, "%s", "");
    	// sprintf(setting.dns_secondary, "%s", "");

        api_get_string_option(NETWORK_WAN_PPTP_IPTYPE_OPTION,iptype,sizeof(iptype));
        sprintf(setting.iptype, "%s", iptype);

        //api_get_string_option(NETWORK_WAN_PPTP_GUI_IPADDR_OPTION,gui_ipaddr,sizeof(gui_ipaddr));
        //sprintf(setting.ip_address, "%s", gui_ipaddr);
        //api_get_string_option(NETWORK_WAN_PPTP_GUI_NETMASK_OPTION,gui_netmask,sizeof(gui_netmask));
        //sprintf(setting.subnet_mask, "%s", gui_netmask);
        //api_get_string_option(NETWORK_WAN_PPTP_GUI_GATEWAY_OPTION,gui_gw,sizeof(gui_gw));
        sprintf(setting.gui_gw, "%s", setting.gateway);

        strcpy(setting.type,"PPTP");
    }    
    else
    {
        strcpy(setting.type,"Static IP");

        api_get_string_option(NETWORK_WAN_IPADDR_OPTION,lanIP,sizeof(lanIP));
        sprintf(setting.ip_address, "%s", lanIP);
        api_get_string_option(NETWORK_WAN_NETMASK_OPTION,netmask,sizeof(netmask));
        sprintf(setting.subnet_mask, "%s", netmask);
        api_get_string_option(NETWORK_WAN_GATEWAY_OPTION,lanGW,sizeof(lanGW));
        sprintf(setting.gateway, "%s", lanGW);
        api_get_string_option(NETWORK_WAN_DNS_OPTION,buf,sizeof(buf));
        sscanf(buf, "%s %s", setting.dns_primary, setting.dns_secondary);
        sprintf(setting.username, "%s", "");
        sprintf(setting.password, "%s", "");
        setting.mtu=0;
        setting.connection_type=connection_type;
    }
#if HAS_AP
    if(pkt->json)
        get_ap_wan_settings_json_cb(pkt, &setting, result);
#endif
    return 0;
}

/*****************************************************************
* NAME:    set_ap_wan_all_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_ap_wan_all_settings_cb(HTTPS_CB *pkt)
{
	char lanAddr[17+1], leaseTime[7+1],dns[128];
	char *return_str=OK_STR;
	bool result=TRUE;
    int autoDetection=0;
	AP_WAN_SETTINGS_T setting;
	char *query_str= get_env(&pkt->envcfg, "QUERY_STRING");

	if(NULL == pkt || 0 == strlen(query_str))
	{
		return_str = ERROR_STR; 
		return FALSE;
	}

#if HAS_WAN_AUTO_DETECTION
    get_json_integer_from_query(query_str, &autoDetection, "AutoDetection");
    APPAGENT_SYSTEM("uci set mesh.wifi.autodetect=%d", autoDetection);
#endif

	memset(&setting, 0, sizeof(setting));
#if HAS_AP
	if(pkt->json)
		result = set_ap_wan_settings_json_cb(query_str, &setting, return_str);
#endif
    if(TRUE != result)
		goto send_pkt;

    if(strcmp(setting.type,"pptp")!=0 && strcmp(setting.type,"l2tp")!=0)
    {
        api_set_string_option(NETWORK_WAN_PROTO_OPTION,setting.type,sizeof(setting.type));
    }

    api_set_string_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,setting.type,sizeof(setting.type));

	if(strcmp(setting.type,"static")==0)
    {
		if(!api_check_ip_addr(setting.ip_address))
        {
            return_str = ERROR_IP_ADDRESS_STR;
            goto send_pkt;
    	}
		if(!api_check_mask_addr(setting.subnet_mask))
		{
            return_str = ERROR_SUBNET_MASK_STR;
            goto send_pkt;
		}
		if(!api_check_ip_addr(setting.gateway))
        {
			return_str = ERROR_GATEWAY_STR;
			goto send_pkt;
		}
		if((strlen(setting.dns_primary) > 0) && !api_check_dns_addr(setting.dns_primary))
        {
			return_str = ERROR_STR;
			goto send_pkt;
		}
		if((strlen(setting.dns_secondary) > 0) && !api_check_dns_addr(setting.dns_secondary))
        {
			return_str = ERROR_STR;
			goto send_pkt;
		}		
        memset(dns,0x0,sizeof(dns));
        if(strlen(setting.dns_secondary) != 0)
        {
		   strcpy(dns,setting.dns_primary);
		   strcat(dns," ");
		   strcat(dns,setting.dns_secondary);
		   api_set_string_option(NETWORK_WAN_DNS_OPTION, dns, sizeof(dns));
           api_set_string_option(NETWORK_WAN_GUI_DNS_OPTION, dns, sizeof(dns));
		}
		else
        {
		   api_set_string_option(NETWORK_WAN_DNS_OPTION, setting.dns_primary, sizeof(setting.dns_primary));
           api_set_string_option(NETWORK_WAN_GUI_DNS_OPTION, setting.dns_primary, sizeof(setting.dns_primary));
        }

        api_set_lan_ipaddr_option(NETWORK_WAN_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
		api_set_lan_netmask_option(NETWORK_WAN_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
		api_set_lan_gateway_option(NETWORK_WAN_GATEWAY_OPTION, setting.gateway,sizeof(setting.gateway));

        api_set_lan_ipaddr_option(NETWORK_WAN_GUI_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
        api_set_lan_netmask_option(NETWORK_WAN_GUI_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
        api_set_lan_gateway_option(NETWORK_WAN_GUI_GATEWAY_OPTION, setting.gateway,sizeof(setting.gateway));
	}
    else
    {
        if (strcmp(setting.type,"pppoe")==0)
        {
            api_set_string_option(NETWORK_WAN_USERNAME_OPTION,setting.username,sizeof(setting.username));
            api_set_string_option(NETWORK_WAN_PASSWORD_OPTION,setting.password,sizeof(setting.password));
            api_set_integer_option(NETWORK_WAN_CONNTYPE_OPTION,setting.connection_type);
        }
        else if(strcmp(setting.type,"l2tp")==0)
        {
            //del pptp proto
            APPAGENT_SYSTEM("uci set network.pptp.proto=''");

            api_set_string_option(NETWORK_WAN_L2TP_USERNAME_OPTION,setting.username,sizeof(setting.username));
            api_set_string_option(NETWORK_WAN_L2TP_PASSWORD_OPTION,setting.password,sizeof(setting.password));            
            api_set_integer_option(NETWORK_WAN_L2TP_CONNTYPE_OPTION,setting.connection_type);
            api_set_string_option(NETWORK_WAN_L2TP_GATEWAY_OPTION,setting.servergateway,sizeof(setting.servergateway));

            if (atoi(setting.iptype) == 0)
            {
                api_set_string_option(NETWORK_WAN_PROTO_OPTION,"dhcp",sizeof("dhcp"));
            }
            else
            {
                api_set_string_option(NETWORK_WAN_PROTO_OPTION,"static",sizeof("static"));
            }
            
            api_set_string_option(NETWORK_WAN_L2TP_GUI_IPADDR_OPTION,setting.ip_address,sizeof(setting.ip_address));
            api_set_string_option(NETWORK_WAN_L2TP_GUI_NETMASK_OPTION,setting.subnet_mask,sizeof(setting.subnet_mask));
            api_set_string_option(NETWORK_WAN_L2TP_GUI_GATEWAY_OPTION,setting.gateway,sizeof(setting.gateway));
            api_set_string_option(NETWORK_WAN_GUI_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
            api_set_string_option(NETWORK_WAN_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
            api_set_string_option(NETWORK_WAN_GATEWAY_OPTION, setting.gateway,sizeof(setting.gateway));
            api_set_string_option(NETWORK_WAN_L2TP_PROTO_OPTION,setting.type,sizeof(setting.type));
            api_set_string_option(NETWORK_WAN_L2TP_IPTYPE_OPTION,setting.iptype,sizeof(setting.iptype));

            api_set_string_option(NETWORK_WAN_NETMASK_OPTION,setting.subnet_mask,sizeof(setting.subnet_mask));
    	}
        else if (strcmp(setting.type,"pptp")==0)
        {
            //del l2tp proto
            APPAGENT_SYSTEM("uci set network.l2tp.proto=''");

            api_set_string_option(NETWORK_WAN_PPTP_USERNAME_OPTION,setting.username,sizeof(setting.username));
            api_set_string_option(NETWORK_WAN_PPTP_PASSWORD_OPTION,setting.password,sizeof(setting.password));            
            api_set_integer_option(NETWORK_WAN_PPTP_CONNTYPE_OPTION,setting.connection_type);
            api_set_string_option(NETWORK_WAN_PPTP_GATEWAY_OPTION,setting.servergateway,sizeof(setting.servergateway));

            if (atoi(setting.iptype) == 0)
            {
                api_set_string_option(NETWORK_WAN_PROTO_OPTION,"dhcp",sizeof("dhcp"));
            }
            else
            {
                api_set_string_option(NETWORK_WAN_PROTO_OPTION,"static",sizeof("static"));
            }

            api_set_string_option(NETWORK_WAN_PPTP_GUI_IPADDR_OPTION,setting.ip_address,sizeof(setting.ip_address));
            api_set_string_option(NETWORK_WAN_PPTP_GUI_NETMASK_OPTION,setting.subnet_mask,sizeof(setting.subnet_mask));
            api_set_string_option(NETWORK_WAN_PPTP_GUI_GATEWAY_OPTION,setting.gateway,sizeof(setting.gateway));
            api_set_string_option(NETWORK_WAN_GUI_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
            api_set_string_option(NETWORK_WAN_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
            api_set_string_option(NETWORK_WAN_GATEWAY_OPTION, setting.gateway,sizeof(setting.gateway));
            api_set_string_option(NETWORK_WAN_PPTP_PROTO_OPTION,setting.type,sizeof(setting.type));
            api_set_string_option(NETWORK_WAN_PPTP_IPTYPE_OPTION,setting.iptype,sizeof(setting.iptype));

            api_set_string_option(NETWORK_WAN_NETMASK_OPTION,setting.subnet_mask,sizeof(setting.subnet_mask));
        }     
    }
    
    if (0 == autoDetection)
    {
        if (setting.opmode==1)
        {
            APPAGENT_SYSTEM("uci set mesh.wifi.man_opmode=ap");
        }
        else
        {
            APPAGENT_SYSTEM("uci set mesh.wifi.man_opmode=ar");
        }
    }
	if(result)
    {
		if(send_simple_response(pkt, OK_STR) == 0)
		{
			system("uci commit");
		}

        /* Clear all login IP such that the client with new IP can log in again. */
        AdminCfg_ClearMultiAdmin();
        APPAGENT_SYSTEM("luci-reload auto network &");

		return 0;
	}

send_pkt:
	/* Send response packet */
	send_simple_response(pkt,ERROR_STR);
	return 0;
}

/*****************************************************************
* NAME:    get_blocked_client_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_blocked_client_list_cb(HTTPS_CB *pkt){

    int i;
    int j =0;
    int block;
    char mac_buf[32];
    BLOCKED_CLIENT_LIST_T setting;
    int enabled ;
    if(NULL == pkt)
    {
        return -1;
    }

    memset(&setting, 0x00, sizeof(BLOCKED_CLIENT_LIST_T));

    setting.enabled = 1;

    for(i = 1; i <= MAX_CLIENT_DEVICE; i++)
    {
            api_get_integer_option2(&block, CLIENT_DEVICE_BLOCK_OPTION, i);

            if ( block == 1) 
            {
                memset(mac_buf, 0x00, sizeof(mac_buf));
                api_get_string_option2(mac_buf, sizeof(mac_buf), CLIENT_DEVICE_MAC_OPTION, i);

                if(0 == strlen(mac_buf))
                {
                    continue;
                }
                else
                {
                   api_get_string_option2(setting.device_name[j], sizeof(setting.device_name[j]), CLIENT_DEVICE_NAME_OPTION, i);
                   sprintf(setting.mac[j], "%s", mac_buf);
                    j++;
                }
            }
            block = 0;
    }
    if(pkt->json)
    {
        get_blocked_client_list_json_cb(pkt, &setting);
    }

  
    return 0;
}

/*****************************************************************
* NAME:    edit_blocked_client_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int edit_blocked_client_list_cb(HTTPS_CB *pkt){

    int i, j, k;
    int existed;
    int empty_index;
    int total_num;
#if 0
    char *query_str="{"
                    " \"Enabled\" : \"true\","
                    " \"IsAllowList\" : \"true\","
                    " \"BlockedClientList\" : ["
                    " {"
                    " \"MacAddress\" : \"1C:BD:B9:E0:32:01\","
                    " \"DeviceName\" : \"ZACK PC 1\""
                    " },"
                    " {"
                    " \"MacAddress\" : \"1C:BD:B9:E0:32:02\","
                    " \"DeviceName\" : \"ZACK PC 2\""
                    " },"
                    " {"
                    " \"MacAddress\" : \"1C:BD:B9:E0:32:03\","
                    " \"DeviceName\" : \"ZACK PC 3\""
                    " },"
                    " {"
                    " \"MacAddress\" : \"1C:BD:B9:E0:32:04\","
                    " \"DeviceName\" : \"ZACK PC 4\""
                    " },"
                    " {"
                    " \"MacAddress\" : \"1C:BD:B9:E0:32:05\","
                    " \"DeviceName\" : \"ZACK PC 5\""
                    " },"
                    " {"
                    " \"MacAddress\" : \"1C:BD:B9:E0:32:06\","
                    " \"DeviceName\" : \"ZACK PC 6\""
                    " },"
                    " {"
                    " \"MacAddress\" : \"1C:BD:B9:E0:32:07\","
                    " \"DeviceName\" : \"ZACK PC 7\""
                    " }"
                    "] "
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif
    char *tmp_pkt;
    char *return_str;
    bool result, new_rule, modified_rule;
    BLOCKED_CLIENT_LIST_T setting;
    char *ptr;
    bool block;
    char buf[32];
    char mac[32];
    char mac_buf[32];
    char name[32];
    char name_buf[256];

    if(NULL == pkt)
    {
        return -1;
    }

    result = FALSE;
    new_rule = FALSE;
    modified_rule = FALSE;
    return_str = ERROR_STR;

    memset(&setting, 0x00, sizeof(BLOCKED_CLIENT_LIST_T));

    if(query_str)
    {
        if(pkt->json)
        {
            result = parse_blocked_client_list_json_cb(query_str, &setting, &total_num);
        }
        if(TRUE == isDuplicateMac(&setting))
        {
            result = FALSE;
            return_str = ERROR_DUPLICATE_MAC_STR;
        }

        if (TRUE == result)
        {
            // prototype for set_mac_filter_cb()
            /* Rule 1 and 2 are default rules. */

            if ( setting.enabled) 
            { 
                for (k = 0; k < total_num;k++) 
                {
                    for (i = 1; i <= MAX_CLIENT_DEVICE; i++)
                    {
                        memset(mac_buf, 0x00, sizeof(mac_buf));
                        memset(name_buf, 0x00, sizeof(name_buf));
                        api_get_string_option2(mac_buf, sizeof(mac_buf), CLIENT_DEVICE_MAC_OPTION, i);
                        api_get_string_option2(name_buf, sizeof(name_buf), CLIENT_DEVICE_NAME_OPTION, i);

                        if(0 == strlen(mac_buf))
                        {
                            continue;
                        }

                        if (0 == strcmp(setting.mac[k], mac_buf) && 0 == strcmp(setting.device_name[k], name_buf) )
                        {
                            existed = 1;
                            APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_BLOCK_OPTION"=1", i);
                        }
                    }
                    if (existed == 0)
                    {
                        for (i = 1; i <= MAX_CLIENT_DEVICE; i++)
                        {
                            memset(mac_buf, 0x00, sizeof(mac_buf));
                            memset(name_buf, 0x00, sizeof(name_buf));
                            api_get_string_option2(mac_buf, sizeof(mac_buf), CLIENT_DEVICE_MAC_OPTION, i);
                            api_get_string_option2(name_buf, sizeof(name_buf), CLIENT_DEVICE_NAME_OPTION, i);

                            if(0 == strlen(mac_buf))
                            {
                                APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_NAME_OPTION"=%s", i, setting.device_name[k]);
                                APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_MAC_OPTION"=%s", i, setting.mac[k]);
                                APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_BLOCK_OPTION"=1", i);
                                break;
                            }
                        }
                    }
                    existed = 0;
                }
                for (i = 1; i <= MAX_CLIENT_DEVICE; i++)
                {
                    memset(mac_buf, 0x00, sizeof(mac_buf));
                    memset(name_buf, 0x00, sizeof(name_buf));
                    api_get_string_option2(mac_buf, sizeof(mac_buf), CLIENT_DEVICE_MAC_OPTION, i);
                    api_get_string_option2(name_buf, sizeof(name_buf), CLIENT_DEVICE_NAME_OPTION, i);

                    if(0 == strlen(mac_buf))
                    {
                        continue;
                    }
                    for (k = 0; k < total_num;k++) 
                    {
                        if (0 == strcmp(setting.mac[k], mac_buf) && 0 == strcmp(setting.device_name[k], name_buf) )
                        {
                            existed = 1;
                        }
                    }
                    if ( existed == 0)
                    {
                        APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_BLOCK_OPTION"=0", i);
                    }
                    existed = 0;
                }
            }


            /* Save new token setting */
            return_str = OK_STR;

            if(OK_STR == return_str)
            {
                APPAGENT_SYSTEM("luci-reload auto &");
            }
        }
    }

    send_simple_response(pkt, return_str);
    return 0;
}

/*****************************************************************
* NAME:    delete_blocked_client_list_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int delete_blocked_client_list_cb(HTTPS_CB *pkt){

    int i;
    char mac_buf[32];

    if(NULL == pkt)
    {
        return -1;
    }
    for (i = 1; i <= MAX_CLIENT_DEVICE; i++)
    {
        memset(mac_buf, 0x00, sizeof(mac_buf));
        api_get_string_option2(mac_buf, sizeof(mac_buf), CLIENT_DEVICE_MAC_OPTION, i);

        if(0 != strlen(mac_buf))
        {
            APPAGENT_SYSTEM("uci set "CLIENT_DEVICE_BLOCK_OPTION"=0", i);
        }
    }

    APPAGENT_SYSTEM("luci-reload auto client &");

    //send the success response
    send_simple_response(pkt, OK_STR);
    return 0;
}
