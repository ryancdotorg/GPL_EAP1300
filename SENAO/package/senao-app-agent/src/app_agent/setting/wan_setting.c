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
;    File    : wan_setting.c
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
#include "wan_setting.h"
#include "app_agent.h"
#include "utility/sys_addr.h"
#include "json_setting.h"
#include "api_tokens.h"
#include "variable/api_wan.h"
#if SUPPORT_IPV6_SETTING
#include "variable/api_ipv6.h"
#endif
#include "sysCore.h"
#include "sysFile.h"
#include "sysAddr.h"

#include <senao-sysutil/regx.h>
/*--------------------------------------------------------------------------*/
/*                                PARAMETER                                 */
/*--------------------------------------------------------------------------*/
WAN_CONNECT_TYPE wanConnectTypeArr[] =
{
    {0, "Static IP Address"},
    {1, "Dynamic IP Address"},
    {2, "DS-Lite"},
    {3, "PPTP"},
    {4, "PPP over Ethernet"},
    {5, "L2TP"},
    {6, "NULL"}
};

#if SUPPORT_IPV6_SETTING
struct _wanIpv6ConnectType {
    IPV6_WAN_CONNECTION_TYPE connection_type;
    char *setting_name;
    char *display_name;
    char *interface_name;
};
struct _wanIpv6ConnectType wanIpv6ConnectTypeArr[] =
{
    {IPV6_WAN_STATIC,                   "static",           "Static IPv6 Address",        WAN_IF},
    {IPV6_WAN_PPPOE,                    "pppoe",            "PPP over Ethernet",          "pppoe-wan6"},
    {IPV6_WAN_AUTOCONFIGURATION,        "dhcpv6",           "Autoconfiguration",          WAN_IF},
    {IPV6_WAN_6RD,                      "6rd",              "6RD",                        "6rd-wan6"},
    {IPV6_WAN_LINK_LOCAL_ONLY,          "linklocal",        "Link-Local only",            WAN_IF},
    {IPV6_WAN_TYPE_MAX,                 NULL,               "NULL",                       NULL}
};
#endif

#define MIN_MTU 512
#define MAX_MTU 1400
#define MIN_IDLE_TIME 1
#define MAX_IDLE_TIME 1000
#define AMOUNT_WAN_BRIDGE_PORT 4

int wanConnectTypeArrSize = sizeof(wanConnectTypeArr)/sizeof(wanConnectTypeArr[0]);

/*--------------------------------------------------------------------------*/
/*                            FUNCTION PROTOTYPE                            */
/*--------------------------------------------------------------------------*/
/*****************************************************************
* NAME:    get_ipv4_wan_status_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/

int get_ipv4_wan_status_cb(HTTPS_CB *pkt){

    if(NULL == pkt)
    {
        return -1;
    }

    int i;
    char *result;
    char *wan_status_str = "";

    WAN_SETTINGS_T setting;

    T_UINT32 status = 0;
    int wanTypeIdx = 0;
    int wanProto,wanProtoDisplay;
    char wanIp[15+1];
    char wanMask[15+1];
    char wanGateway[15+1];
    char wanStatus[15+1];
    char wanMac[17+1];
    char dns1[15+1], dns2[15+1];

    memset(&setting, 0x00, sizeof(WAN_SETTINGS_T));

    /* TYPE */
    api_get_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,&wanProtoDisplay);
    for(i=0;i<wanConnectTypeArrSize;i++)
    {
        if(wanConnectTypeArr[i].type==wanProtoDisplay)
		{
			strcpy(setting.type, wanConnectTypeArr[i].name);
			setting.typeIdx=wanConnectTypeArr[i].type;
			break;
		}
    }

    /* WAN Physic Status */

    memset(wanStatus,0,sizeof(wanStatus));
    sysutil_get_wan_status(wanStatus,sizeof(wanStatus));
    if (strcmp(wanStatus,"0") == 0 )
        wan_status_str = "DISCONNECTED";
    else
        wan_status_str = "CONNECTED";

    /* IP */
    memset(wanIp,0,sizeof(wanIp));
    sysutil_get_wan_ipaddr(wanIp, sizeof(wanIp));

    /* Mask */
    sysutil_get_wan_netmask(wanMask, sizeof(wanMask));

    /* Gateway */
    sysutil_get_wan_gateway(wanGateway,sizeof(wanGateway));

    /* MacAddress */
    sysutil_get_wan_mac(wanMac, sizeof(wanMac));
    /* DNS */

    memset(dns1, 0, sizeof(dns1));
    memset(dns2, 0, sizeof(dns2));
    sysutil_get_wan_dns(1, dns1, sizeof(dns1));
    sysutil_get_wan_dns(2, dns2, sizeof(dns2));
    if(strlen(dns1) == 0)
        snprintf(dns1,sizeof(dns1),"%s","0.0.0.0");
    if(strlen(dns2) == 0)
        snprintf(dns2,sizeof(dns2),"%s","0.0.0.0");

    sprintf(setting.ip_address, "%s", wanIp);
    sprintf(setting.subnet_mask, "%s", wanMask);
    sprintf(setting.gateway, "%s", wanGateway);
    sprintf(setting.mac_address, "%s", wanMac);
    sprintf(setting.dns_primary, "%s", dns1);
    sprintf(setting.dns_secondary, "%s", dns2);

    result = OK_STR;

    if(pkt->json)
    {
        get_ipv4_wan_status_json_cb(pkt, &setting, wan_status_str, result);
    }
}
/*****************************************************************
* NAME:    get_wan_status_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/

int get_wan_status_cb(HTTPS_CB *pkt){

    if(NULL == pkt)
    {
        return -1;
    }

    int i;
    char *result;
    char *wan_status_str = "";

    WAN_SETTINGS_T setting;

    T_UINT32 status = 0;
    int wanTypeIdx = 0;
    int wanProto,wanProtoDisplay;
    char wanIp[15+1];
    char wanMask[15+1];
    char wanGateway[15+1];
    char wanStatus[15+1];
    char wanMac[17+1];
    char dns1[15+1], dns2[15+1];
    char wan_interface[5]={0};
    char opmode[3]={0};

    memset(&setting, 0x00, sizeof(WAN_SETTINGS_T));

    /* TYPE */
    api_get_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,&wanProtoDisplay);
    for(i=0;i<wanConnectTypeArrSize;i++)
    {
        if(wanConnectTypeArr[i].type==wanProtoDisplay)
		{
			strcpy(setting.type, wanConnectTypeArr[i].name);
			setting.typeIdx=wanConnectTypeArr[i].type;
			break;
		}
    }

    /* WAN Physic Status */

    memset(wanStatus,0,sizeof(wanStatus));
    sysutil_get_wan_status(wanStatus,sizeof(wanStatus));
    if (strcmp(wanStatus,"0") == 0 )
        wan_status_str = "DISCONNECTED";
    else
        wan_status_str = "CONNECTED";
#if APP_AGENT_SUPPORT_ENSHARE
    api_get_string_option("system.@system[0].opmode", opmode, sizeof(opmode));

    if ( strcmp(opmode,"ar") == 0) 
    {
        api_get_string_option("network.wan.ifname", wan_interface, sizeof(wan_interface));
    }
    else
    {
        sysutil_interact(wan_interface,sizeof(wan_interface), "cat /tmp/wandev");
    }
    memset(wanIp,0x0,sizeof(wanIp));
    sysutil_get_interface_ipaddr(wan_interface, wanIp, sizeof(wanIp));
    //sysutil_get_wan_ipaddr(wanIP, sizeof(wanIP)); 

    memset(wanMask,0x0,sizeof(wanMask));
    //sysutil_get_wan_netmask(wanMask, sizeof(wanMask));
    sysutil_get_interface_netmask(wan_interface, wanMask, sizeof(wanMask));

    memset(wanGateway,0x0,sizeof(wanGateway));
    //sysutil_get_wan_netmask(wanMask, sizeof(wanMask));
    sysutil_get_interface_gateway(wan_interface, wanGateway, sizeof(wanGateway));

    memset(wanMac,0x0,sizeof(wanMac));
    sysutil_get_interface_mac(wan_interface, wanMac, sizeof(wanMac));

    /* DNS */

    memset(dns1, 0, sizeof(dns1));
    memset(dns2, 0, sizeof(dns2));
    sysutil_get_interface_dns(wan_interface, 1, dns1, sizeof(dns1));
    sysutil_get_interface_dns(wan_interface, 2, dns2, sizeof(dns2));

#else
    /* IP */
    memset(wanIp,0,sizeof(wanIp));
    sysutil_get_wan_ipaddr(wanIp, sizeof(wanIp));

    /* Mask */
    sysutil_get_wan_netmask(wanMask, sizeof(wanMask));

    /* Gateway */
    sysutil_get_wan_gateway(wanGateway,sizeof(wanGateway));

    /* MacAddress */
    sysutil_get_wan_mac(wanMac, sizeof(wanMac));
    /* DNS */

    memset(dns1, 0, sizeof(dns1));
    memset(dns2, 0, sizeof(dns2));
    sysutil_get_wan_dns(1, dns1, sizeof(dns1));
    sysutil_get_wan_dns(2, dns2, sizeof(dns2));
#endif

    if(strlen(dns1) == 0)
        snprintf(dns1,sizeof(dns1),"%s","0.0.0.0");
    if(strlen(dns2) == 0)
        snprintf(dns2,sizeof(dns2),"%s","0.0.0.0");

    sprintf(setting.ip_address, "%s", wanIp);
    sprintf(setting.subnet_mask, "%s", wanMask);
    sprintf(setting.gateway, "%s", wanGateway);
    sprintf(setting.mac_address, "%s", wanMac);
    sprintf(setting.dns_primary, "%s", dns1);
    sprintf(setting.dns_secondary, "%s", dns2);

    result = OK_STR;

    if(pkt->json)
    {
        get_ipv4_wan_status_json_cb(pkt, &setting, wan_status_str, result);
    }
}


/*****************************************************************
* NAME:    get_ipv4_wan_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_ipv4_wan_settings_cb(HTTPS_CB *pkt){

    char *result;
    int i=0,j;
	int wanTypeIdx = 0;
    int wanProto,wanProtoDisplay;
    char wanIp[15+1]={0};
    char wanMask[15+1]={0};
    char wanGateway[15+1]={0};
    char dns1[15+1], dns2[15+1];
    char hostname[31+1]={0};
    int mtu,connectionType,idletime,connectionid;
    char service[15+1];
    char username[31+1],password[31+1],wanMac[31+1]={0},wanMactmp[31+1]={0};
    char defaultgateway[31+1],l2tpgateway[31+1],pptpgateway[31+1];
#if SUPPORT_IPV6_SETTING
    char AFTRIPv6Address[31+1]={0},Ipv6WanAddress[31+1]={0},Ipv6WanDefaultGateway[31+1]={0};
#endif

    WAN_SETTINGS_T setting;

    if(NULL == pkt)
    {
        return -1;
    }

    memset(&setting, 0x00, sizeof(WAN_SETTINGS_T));

    /* TYPE */
    api_get_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,&wanProtoDisplay);
    for(i=0;i<wanConnectTypeArrSize;i++)
    {
        if(wanConnectTypeArr[i].type==wanProtoDisplay)
		{
			strcpy(setting.type, wanConnectTypeArr[i].name);
			setting.typeIdx=wanConnectTypeArr[i].type;
			break;
		}
    }

    /* IP */
    sysutil_get_wan_ipaddr(wanIp, sizeof(wanIp));

    /* Mask */
    sysutil_get_wan_netmask(wanMask, sizeof(wanMask));

    /* Gateway */
    sysutil_get_wan_gateway(wanGateway,sizeof(wanGateway));

    /* MacAddress */
    api_get_string_option(NETWORK_WAN_MACADDR_OPTION, wanMac, sizeof(wanMac));
    sysutil_lower2upper_mac(wanMac);

    if(strlen(wanMac)!=0){
        for(i=0,j=0;i<strlen(wanMac);i++)
        {
            if (wanMac[i]!=':')
            {
               wanMactmp[j++]=wanMac[i]; 
            }
        }
        sprintf(setting.mac_address, "%s", wanMactmp);
    }
    else{
        sprintf(setting.mac_address, "%s", "000000000000");
    }
        
    /* DNS */

    memset(dns1, 0, sizeof(dns1));
    memset(dns2, 0, sizeof(dns2));
    sysutil_get_wan_dns(1, dns1, sizeof(dns1));
    sysutil_get_wan_dns(2, dns2, sizeof(dns2));
    if(strlen(dns1) == 0)
        snprintf(dns1,sizeof(dns1),"%s","0.0.0.0");
    if(strlen(dns2) == 0)
        snprintf(dns2,sizeof(dns2),"%s","0.0.0.0");

    /* Hostname */ 
    api_get_wan_hostname_option(NETWORK_WAN_HOSTNAME_OPTION,hostname,sizeof(hostname));

    /* CloneMac */   
    if(strcmp((char *)api_cloneMacAddress(pkt->fd),setting.mac_address) == 0){
        setting.clonedMacStatus = 1;
    }
    else 
        setting.clonedMacStatus = 0;

    /* MTU, Username , Password , ConnectionType, IdleTime*/

	switch (setting.typeIdx)
	{
#if SUPPORT_IPV6_SETTING
    case WAN_DSLITE:
        /* AFTRIPv6Address */
        api_get_wan_AFTRIPv6Address_option(NETWORK_WAN_AFTRIPV6ADDRESS_OPTION,AFTRIPv6Address,sizeof(AFTRIPv6Address));
        sprintf(setting.ipv6_ds_aftr_ip6, "%s", AFTRIPv6Address); 
        /* Ipv6WanAddress */
        sysutil_get_wan_Ipv6WanAddress(Ipv6WanAddress,sizeof(Ipv6WanAddress));
        sprintf(setting.ipv6_wan_address, "%s", Ipv6WanAddress);          
        /* Ipv6WanDefaultGateway */
        sysutil_get_wan_Ipv6WanDefaultGateway(Ipv6WanDefaultGateway,sizeof(Ipv6WanDefaultGateway));
        sprintf(setting.ipv6_wan_default_gw, "%s", Ipv6WanDefaultGateway);    
#endif
    case WAN_PPTP:
        api_get_wan_mtu_option(NETWORK_WAN_PPTP_MTU_OPTION,&mtu);
        api_get_wan_username_option(NETWORK_WAN_PPTP_USERNAME_OPTION,username,sizeof(username));
        api_get_wan_password_option(NETWORK_WAN_PPTP_PASSWORD_OPTION,password,sizeof(password));
        api_get_wan_connectionType_option(NETWORK_WAN_PPTP_CONNTYPE_OPTION,&connectionType);
        api_get_wan_idletime_option(NETWORK_WAN_PPTP_IDLETIME_OPTION,&idletime);
        /* ConnectionId */
        api_get_wan_connectionid_option(NETWORK_WAN_PPTP_CONNECTIONID,&connectionid);
        setting.conId=connectionid;

        /* PPTPGateway */
        api_get_wan_pptpgateway_option(NETWORK_WAN_PPTP_GATEWAY_OPTION,pptpgateway,sizeof(pptpgateway));
        sprintf(setting.service_ip, "%s", pptpgateway);        
        break;
    case WAN_PPPOE:
        api_get_wan_mtu_option(NETWORK_WAN_MTU_OPTION,&mtu);
        api_get_wan_username_option(NETWORK_WAN_USERNAME_OPTION,username,sizeof(username));
        api_get_wan_password_option(NETWORK_WAN_PASSWORD_OPTION,password,sizeof(password));
        api_get_wan_connectionType_option(NETWORK_WAN_CONNTYPE_OPTION,&connectionType);
        api_get_wan_idletime_option(NETWORK_WAN_IDLETIME_OPTION,&idletime);
        /* Service */
        api_get_wan_service_option(NETWORK_WAN_SERVICE_OPTION,service,sizeof(service));
        sprintf(setting.service_name, "%s", service);
        break;
    case WAN_L2TP:
        api_get_wan_mtu_option(NETWORK_WAN_L2TP_MTU_OPTION,&mtu);
        api_get_wan_username_option(NETWORK_WAN_L2TP_USERNAME_OPTION,username,sizeof(username));
        api_get_wan_password_option(NETWORK_WAN_L2TP_PASSWORD_OPTION,password,sizeof(password));
        api_get_wan_connectionType_option(NETWORK_WAN_L2TP_CONNTYPE_OPTION,&connectionType);
        api_get_wan_idletime_option(NETWORK_WAN_L2TP_IDLETIME_OPTION,&idletime);
        /* L2TPGateway */
        api_get_wan_l2tpgateway_option(NETWORK_WAN_L2TP_GATEWAY_OPTION,l2tpgateway,sizeof(l2tpgateway));
        sprintf(setting.service_ip, "%s", l2tpgateway);
        break;
    default:
        break;
    }

    /* DefaultGateway */
    api_get_wan_defaultgateway_option(NETWORK_WAN_GATEWAY_OPTION,defaultgateway, sizeof(defaultgateway));

    /* IsIPDynamic */
    api_get_wan_proto_option(NETWORK_WAN_PROTO_OPTION,&wanProto);

    sprintf(setting.ip_address, "%s", wanIp);
    sprintf(setting.subnet_mask, "%s", wanMask);
    sprintf(setting.gateway, "%s", wanGateway);
    sprintf(setting.dns_primary, "%s", dns1);
    sprintf(setting.dns_secondary, "%s", dns2);
    sprintf(setting.hostname, "%s", hostname);
    sprintf(setting.user_name, "%s", username);
    sprintf(setting.password, "%s", password);
    setting.mtu=mtu;
    setting.connType=connectionType;
    setting.max_idle_time=idletime;
    sprintf(setting.gateway, "%s", defaultgateway);
    setting.IsIPDynamic=wanProto;

    result = OK_STR;


    if(pkt->json)
    {
        get_ipv4_wan_settings_json_cb(pkt, &setting, result);
    }
}

/*****************************************************************
* NAME:    get_wan_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_wan_settings_cb(HTTPS_CB *pkt){

    char *result;
    int i=0,j;
	int wanTypeIdx = 0;
    int wanProto,wanProtoDisplay;
    char wanIp[15+1]={0};
    char wanMask[15+1]={0};
    char wanGateway[15+1]={0};
    char dns1[15+1], dns2[15+1];
    char hostname[31+1]={0};
    int mtu,connectionType,idletime,connectionid;
    char service[15+1];
    char username[31+1],password[31+1],wanMac[31+1]={0},wanMactmp[31+1]={0};
    char defaultgateway[31+1],l2tpgateway[31+1],pptpgateway[31+1];
    char wan_interface[5]={0};
    char opmode[3]={0};

#if SUPPORT_IPV6_SETTING
    char AFTRIPv6Address[31+1]={0},Ipv6WanAddress[31+1]={0},Ipv6WanDefaultGateway[31+1]={0};
#endif

    WAN_SETTINGS_T setting;

    if(NULL == pkt)
    {
        return -1;
    }

    memset(&setting, 0x00, sizeof(WAN_SETTINGS_T));

    /* TYPE */
    api_get_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,&wanProtoDisplay);
    for(i=0;i<wanConnectTypeArrSize;i++)
    {
        if(wanConnectTypeArr[i].type==wanProtoDisplay)
		{
			strcpy(setting.type, wanConnectTypeArr[i].name);
			setting.typeIdx=wanConnectTypeArr[i].type;
			break;
		}
    }

    api_get_string_option("system.@system[0].opmode", opmode, sizeof(opmode));

    if ( strcmp(opmode,"ar") == 0) 
    {
        api_get_string_option("network.wan.ifname", wan_interface, sizeof(wan_interface));
    }
    else
    {
        sysutil_interact(wan_interface,sizeof(wan_interface), "cat /tmp/wandev");
    }

    memset(wanIp,0x0,sizeof(wanIp));
    sysutil_get_interface_ipaddr(wan_interface, wanIp, sizeof(wanIp));
    //sysutil_get_wan_ipaddr(wanIP, sizeof(wanIP)); 

    memset(wanMask,0x0,sizeof(wanMask));
    //sysutil_get_wan_netmask(wanMask, sizeof(wanMask));
    sysutil_get_interface_netmask(wan_interface, wanMask, sizeof(wanMask));

    memset(wanGateway,0x0,sizeof(wanGateway));
    //sysutil_get_wan_netmask(wanMask, sizeof(wanMask));
    sysutil_get_interface_gateway(wan_interface, wanGateway, sizeof(wanGateway));

    memset(wanMac,0x0,sizeof(wanMac));
    sysutil_get_interface_mac(wan_interface, wanMac, sizeof(wanMac));

    /* IP */
    // sysutil_get_wan_ipaddr(wanIp, sizeof(wanIp));

    /* Mask */
    // sysutil_get_wan_netmask(wanMask, sizeof(wanMask));

    /* Gateway */
    // sysutil_get_wan_gateway(wanGateway,sizeof(wanGateway));

    /* MacAddress */
    // api_get_string_option(NETWORK_WAN_MACADDR_OPTION, wanMac, sizeof(wanMac));

    sysutil_lower2upper_mac(wanMac);

    if(strlen(wanMac)!=0){
        for(i=0,j=0;i<strlen(wanMac);i++)
        {
            if (wanMac[i]!=':')
            {
               wanMactmp[j++]=wanMac[i]; 
            }
        }
        sprintf(setting.mac_address, "%s", wanMactmp);
    }
    else{
        sprintf(setting.mac_address, "%s", "000000000000");
    }
        
    /* DNS */

    memset(dns1, 0, sizeof(dns1));
    memset(dns2, 0, sizeof(dns2));
    sysutil_get_interface_dns(wan_interface, 1, dns1, sizeof(dns1));
    sysutil_get_interface_dns(wan_interface, 2, dns2, sizeof(dns2));
    if(strlen(dns1) == 0)
        snprintf(dns1,sizeof(dns1),"%s","0.0.0.0");
    if(strlen(dns2) == 0)
        snprintf(dns2,sizeof(dns2),"%s","0.0.0.0");

    /* Hostname */ 
    api_get_wan_hostname_option(NETWORK_WAN_HOSTNAME_OPTION,hostname,sizeof(hostname));

    /* CloneMac */   
    if(strcmp((char *)api_cloneMacAddress(pkt->fd),setting.mac_address) == 0){
        setting.clonedMacStatus = 1;
    }
    else 
        setting.clonedMacStatus = 0;

    /* MTU, Username , Password , ConnectionType, IdleTime*/

	switch (setting.typeIdx)
	{
#if SUPPORT_IPV6_SETTING
    case WAN_DSLITE:
        /* AFTRIPv6Address */
        api_get_wan_AFTRIPv6Address_option(NETWORK_WAN_AFTRIPV6ADDRESS_OPTION,AFTRIPv6Address,sizeof(AFTRIPv6Address));
        sprintf(setting.ipv6_ds_aftr_ip6, "%s", AFTRIPv6Address); 
        /* Ipv6WanAddress */
        sysutil_get_wan_Ipv6WanAddress(Ipv6WanAddress,sizeof(Ipv6WanAddress));
        sprintf(setting.ipv6_wan_address, "%s", Ipv6WanAddress);          
        /* Ipv6WanDefaultGateway */
        sysutil_get_wan_Ipv6WanDefaultGateway(Ipv6WanDefaultGateway,sizeof(Ipv6WanDefaultGateway));
        sprintf(setting.ipv6_wan_default_gw, "%s", Ipv6WanDefaultGateway);    
#endif
    case WAN_PPTP:
        api_get_wan_mtu_option(NETWORK_WAN_PPTP_MTU_OPTION,&mtu);
        api_get_wan_username_option(NETWORK_WAN_PPTP_USERNAME_OPTION,username,sizeof(username));
        api_get_wan_password_option(NETWORK_WAN_PPTP_PASSWORD_OPTION,password,sizeof(password));
        api_get_wan_connectionType_option(NETWORK_WAN_PPTP_CONNTYPE_OPTION,&connectionType);
        api_get_wan_idletime_option(NETWORK_WAN_PPTP_IDLETIME_OPTION,&idletime);
        /* ConnectionId */
        api_get_wan_connectionid_option(NETWORK_WAN_PPTP_CONNECTIONID,&connectionid);
        setting.conId=connectionid;

        /* PPTPGateway */
        api_get_wan_pptpgateway_option(NETWORK_WAN_PPTP_GATEWAY_OPTION,pptpgateway,sizeof(pptpgateway));
        sprintf(setting.service_ip, "%s", pptpgateway);        
        break;
    case WAN_PPPOE:
        api_get_wan_mtu_option(NETWORK_WAN_MTU_OPTION,&mtu);
        api_get_wan_username_option(NETWORK_WAN_USERNAME_OPTION,username,sizeof(username));
        api_get_wan_password_option(NETWORK_WAN_PASSWORD_OPTION,password,sizeof(password));
        api_get_wan_connectionType_option(NETWORK_WAN_CONNTYPE_OPTION,&connectionType);
        api_get_wan_idletime_option(NETWORK_WAN_IDLETIME_OPTION,&idletime);
        /* Service */
        api_get_wan_service_option(NETWORK_WAN_SERVICE_OPTION,service,sizeof(service));
        sprintf(setting.service_name, "%s", service);
        break;
    case WAN_L2TP:
        api_get_wan_mtu_option(NETWORK_WAN_L2TP_MTU_OPTION,&mtu);
        api_get_wan_username_option(NETWORK_WAN_L2TP_USERNAME_OPTION,username,sizeof(username));
        api_get_wan_password_option(NETWORK_WAN_L2TP_PASSWORD_OPTION,password,sizeof(password));
        api_get_wan_connectionType_option(NETWORK_WAN_L2TP_CONNTYPE_OPTION,&connectionType);
        api_get_wan_idletime_option(NETWORK_WAN_L2TP_IDLETIME_OPTION,&idletime);
        /* L2TPGateway */
        api_get_wan_l2tpgateway_option(NETWORK_WAN_L2TP_GATEWAY_OPTION,l2tpgateway,sizeof(l2tpgateway));
        sprintf(setting.service_ip, "%s", l2tpgateway);
        break;
    default:
        break;
    }

    /* DefaultGateway */
    api_get_wan_defaultgateway_option(NETWORK_WAN_GATEWAY_OPTION,defaultgateway, sizeof(defaultgateway));

    /* IsIPDynamic */
    api_get_wan_proto_option(NETWORK_WAN_PROTO_OPTION,&wanProto);

    sprintf(setting.ip_address, "%s", wanIp);
    sprintf(setting.subnet_mask, "%s", wanMask);
    sprintf(setting.gateway, "%s", wanGateway);
    sprintf(setting.dns_primary, "%s", dns1);
    sprintf(setting.dns_secondary, "%s", dns2);
    sprintf(setting.hostname, "%s", hostname);
    sprintf(setting.user_name, "%s", username);
    sprintf(setting.password, "%s", password);
    setting.mtu=mtu;
    setting.connType=connectionType;
    setting.max_idle_time=idletime;
    sprintf(setting.gateway, "%s", defaultgateway);
    setting.IsIPDynamic=wanProto;

    result = OK_STR;


    if(pkt->json)
    {
        get_ipv4_wan_settings_json_cb(pkt, &setting, result);
    }
}

/*****************************************************************
* NAME:    set_ipv4_wan_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_ipv4_wan_settings_cb(HTTPS_CB *pkt){

    bool result = TRUE;
    char *return_str;
    int isReboot=0;
    int i, j;
    char mac_tmp_input[20] = {0};
    char dns[127+1]={0};
    char lanIP[15+1] = {0},lanMask[15+1] = {0};
#if 0
    char *query_str="{\n" \
                    "  \"Type\" : \"Static IP\",\n" \
                    "  \"IPAddress\" : \"172.1.1.1\",\n" \
                    "  \"SubnetMask\" : \"255.255.255.0\",\n" \
                    "  \"Gateway\" : \"172.1.1.254\",\n" \
                    "  \"PrimaryDNS\" : \"\",\n" \
                    "  \"SecondaryDNS\" : \"\" \n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif

    WAN_SETTINGS_T setting;

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if(NULL == pkt || 0 == strlen(query_str))
    {
        goto send_pkt;
    }

    memset(&setting, 0x00, sizeof(WAN_SETTINGS_T));

    if(pkt->json)
    {
        result &= parse_ipv4_wan_settings_json_cb(query_str, &setting, &return_str);
    }

    if(TRUE != result) goto send_pkt;

    /* Check value */
    for(i = 0; i < T_NUM_OF_ELEMENTS(wanConnectTypeArr); i++){
         if(strcmp(wanConnectTypeArr[i].name,setting.type) == 0){
             break;
         }
         else if(i == T_NUM_OF_ELEMENTS(wanConnectTypeArr)-1){
            return_str = ERROR_TYPE_NOT_SUPPORTED_STR;
            goto send_pkt;
         }
    }

#if 1//HAS_MAC_CLONE
    if(setting.clonedMacStatus){
        strcpy(setting.mac_address, (char *)api_cloneMacAddress(pkt->fd));
    }
    sysutil_add_colon_to_mac(setting.mac_address,setting.mac_address);
#endif
    switch(setting.typeIdx)
    {
    case WAN_STATIC:
//Check
        memset(lanIP, 0, sizeof(lanIP));
        memset(lanMask, 0, sizeof(lanMask));
        sysutil_get_lan_ipaddr(lanIP, sizeof(lanIP));
        sysutil_get_lan_netmask(lanMask, sizeof(lanMask));

        if(!api_check_ip_addr(setting.ip_address)){
            return_str = ERROR_IP_ADDRESS_STR;
            goto send_pkt;
        }
        if(!api_check_mask_addr(setting.subnet_mask)){
            return_str = ERROR_SUBNET_MASK_STR;
            goto send_pkt;
        }
        if(!api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.ip_address,1)){
            return_str = ERROR_IP_ADDRESS_STR;
            goto send_pkt;
        }
        if(api_check_is_same_subnet(setting.ip_address, lanIP, lanMask)){
            return_str = ERROR_IP_ADDRESS_STR;
            goto send_pkt;
        }
        if(!api_check_ip_addr(setting.gateway)){
            return_str = ERROR_GATEWAY_STR;
            goto send_pkt;
        }
        if(!api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.gateway,1)){
            return_str = ERROR_GATEWAY_STR;
            goto send_pkt;
        }
        if(strlen(setting.dns_primary) != 0 && !api_check_ip_addr(setting.dns_primary)){
            return_str = ERROR_DNS1_IP_ADDRESS_STR;
            goto send_pkt;
        }
        if(strlen(setting.dns_secondary) != 0 && !api_check_ip_addr(setting.dns_secondary)){
            return_str = ERROR_DNS2_IP_ADDRESS_STR;
            goto send_pkt;
        }
//Set
        /* 
        if(0 != strcmp(setting.ip_address,getWanIP())){
            isReboot = 1;
        } 
        */ 
        api_set_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,WAN_STATIC);          //proto_display
        api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_STATIC);                  //proto
        api_set_wan_ipaddr_option(NETWORK_WAN_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
        api_set_wan_ipaddr_option(NETWORK_WAN_GUI_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
        api_set_wan_netmask_option(NETWORK_WAN_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
        api_set_wan_netmask_option(NETWORK_WAN_GUI_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
        api_set_wan_gateway_option(NETWORK_WAN_GATEWAY_OPTION, setting.gateway, sizeof(setting.gateway));
        api_set_wan_gateway_option(NETWORK_WAN_GUI_GATEWAY_OPTION, setting.gateway, sizeof(setting.gateway));

        if(strlen(setting.dns_secondary) != 0){
           strcpy(dns,setting.dns_primary);
           strcat(dns," ");
           strcat(dns,setting.dns_secondary);
           api_set_wan_dns_option(NETWORK_WAN_DNS_OPTION, dns, sizeof(dns));
        }
        else
        {
           api_set_wan_dns_option(NETWORK_WAN_DNS_OPTION, setting.dns_primary, sizeof(setting.dns_primary));
        }
       
        break;

    case WAN_DHCP:
//Check
        if(strlen(setting.hostname) != 0){
            if(!api_check_checkHostNameString(setting.hostname)){
                return_str = ERROR_HOSTNAME_STR;
                goto send_pkt;
            }
        }
		if(!api_check_mac(setting.mac_address)){
            return_str = ERROR_MAC_ADDRESS_STR;
            goto send_pkt;
        }
//Set
        api_set_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,WAN_DHCP);          //proto_display
        api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_DHCP);                  //proto
        api_set_wan_hostname_option(NETWORK_WAN_HOSTNAME_OPTION,setting.hostname,sizeof(setting.hostname));
        api_set_wan_mac_option(NETWORK_WAN_MAC_OPTION,setting.mac_address,sizeof(setting.mac_address));
        break;

    case WAN_PPTP:
//Check
        memset(lanIP, 0, sizeof(lanIP));
        memset(lanMask, 0, sizeof(lanMask));
        sysutil_get_lan_ipaddr(lanIP, sizeof(lanIP));
        sysutil_get_lan_netmask(lanMask, sizeof(lanMask));

        if (setting.IsIPDynamic)
        {
            if(strlen(setting.hostname) != 0){
        		if(!api_check_checkHostNameString(setting.hostname)){
                    return_str = ERROR_HOSTNAME_STR;
                    goto send_pkt;
                }
            }
    		if(!api_check_mac(setting.mac_address)){
                return_str = ERROR_MAC_ADDRESS_STR;
                goto send_pkt;
            }
        }
        else
        {
            if(!api_check_ip_addr(setting.ip_address)){
                return_str = ERROR_IP_ADDRESS_STR;
                goto send_pkt;
            }
            if(!api_check_mask_addr(setting.subnet_mask)){
                return_str = ERROR_SUBNET_MASK_STR;
                goto send_pkt;
            }
            if(api_check_is_same_subnet(setting.ip_address, lanIP, lanMask)){
                return_str = ERROR_IP_ADDRESS_STR;
                goto send_pkt;
            }
            if(!api_check_ip_addr(setting.service_ip)){
                return_str = ERROR_PPTP_GATEWAY_STR;
                goto send_pkt;
            }

            if(strcmp(setting.gateway, "") == 0 || strcmp(setting.gateway, "0.0.0.0") == 0 || strcmp(setting.gateway, "NULL") == 0)
            {
                if(!api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.service_ip,1)){
                    return_str = ERROR_PPTP_GATEWAY_STR;
                    goto send_pkt;
                }
            }
            else{
                if(!api_check_ip_addr(setting.gateway)){
                    return_str = ERROR_GATEWAY_STR;
                    goto send_pkt;
                }
                if(!api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.gateway,1)){
                    return_str = ERROR_GATEWAY_STR;
                    goto send_pkt;
                }
                if(api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.service_ip,1)){
                    return_str = ERROR_PPTP_GATEWAY_STR;
                    goto send_pkt;
                }
            }
        }
        if(strcmp(setting.user_name, "") == 0){
            return_str = ERROR_USER_NAME_STR;
            goto send_pkt;
        }
        for(i = 0; setting.user_name[i] != 0; i++){
            if(!isprint(setting.user_name[i])){
                return_str = ERROR_USER_NAME_STR;
                goto send_pkt;
            }
        }
        if(strcmp(setting.password, "") == 0) goto send_pkt;
        for (j = 0; setting.password[j] != 0; j++){
            if(!isprint(setting.password[j])){
                goto send_pkt;
            }
        }
        if(!api_check_ip_addr(setting.service_ip)){
            return_str = ERROR_PPTP_GATEWAY_STR;
            goto send_pkt;
        }
        if(api_check_is_same_subnet(setting.service_ip, lanIP, lanMask)){
            return_str = ERROR_PPTP_GATEWAY_STR;
            goto send_pkt;
        }
        if((int *)setting.conId != NULL){
            if(!api_check_digit_range(setting.conId, 0, 65535)){
                goto send_pkt;
            }
        }
        if(!api_check_digit_range(setting.mtu, MIN_MTU, MAX_MTU))
        {
            return_str = ERROR_MTU_STR;
            goto send_pkt;
        }
        if (setting.connType == 1){
            if(!api_check_digit_range(setting.max_idle_time, MIN_IDLE_TIME, MAX_IDLE_TIME))
            {
                return_str = ERROR_IDLE_TIME_STR;
                goto send_pkt;
            }
        }

//Set
        api_set_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,WAN_PPTP); //proto_display
        if (setting.IsIPDynamic)                                             // dynamic IP
        {
            api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_DHCP);                //proto
            api_set_integer_option(NETWORK_WAN_PPTP_IPTYPE_OPTION, 0);                  //IsIPDynamic
            //apCfgSetIntValue(WAN1_PPTP_TYPE_TOK,WAN_CONNECTION_TYPE_DHCP);
            api_set_wan_hostname_option(NETWORK_WAN_HOSTNAME_OPTION,setting.hostname,sizeof(setting.hostname));
            api_set_wan_mac_option(NETWORK_WAN_MAC_OPTION,setting.mac_address,sizeof(setting.mac_address));
        }
        else                                                                    // static IP
        {
            //if(0 != strcmp(setting.ip_address,getWanIP())){
            //    isReboot = 1;
            //}
            api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_STATIC);                  //proto
            api_set_integer_option(NETWORK_WAN_PPTP_IPTYPE_OPTION, 1);                      //IsIPDynamic
            api_set_wan_ipaddr_option(NETWORK_WAN_PPTP_GUI_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
            api_set_wan_ipaddr_option(NETWORK_WAN_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
            api_set_wan_netmask_option(NETWORK_WAN_PPTP_GUI_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
            api_set_wan_netmask_option(NETWORK_WAN_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
            api_set_wan_gateway_option(NETWORK_WAN_PPTP_GUI_GATEWAY_OPTION, setting.gateway, sizeof(setting.gateway));
            api_set_wan_gateway_option(NETWORK_WAN_GATEWAY_OPTION, setting.gateway, sizeof(setting.gateway));
        }
        api_set_wan_user_option(NETWORK_WAN_PPTP_USERNAME_OPTION, setting.user_name, sizeof(setting.user_name));
        api_set_wan_password_option(NETWORK_WAN_PPTP_PASSWORD_OPTION, setting.password, sizeof(setting.password));
        api_set_string_option(NETWORK_WAN_PPTP_GATEWAY_OPTION, setting.service_ip, sizeof(setting.service_ip));
        api_set_integer_option(NETWORK_WAN_PPTP_CONNECTIONID, setting.conId);
        api_set_integer_option(NETWORK_WAN_PPTP_MTU_OPTION, setting.mtu);
        api_set_integer_option(NETWORK_WAN_PPTP_CONNTYPE_OPTION, setting.connType);

        if (setting.connType == CONN_AUTO)
        {
            api_set_integer_option(NETWORK_WAN_PPTP_IDLETIME_OPTION, setting.max_idle_time);
        }
        break;

    case WAN_PPPOE:                             //PPPOE
//Check
        if(strcmp(setting.user_name, "") == 0) {
            return_str = ERROR_USER_NAME_STR;
            goto send_pkt;
        }
        for(i = 0; setting.user_name[i] != 0; i++){
            if(!isprint(setting.user_name[i])){
                return_str = ERROR_USER_NAME_STR;
                goto send_pkt;
            }
        }
        if(strcmp(setting.password, "") == 0) goto send_pkt;
        for (j = 0; setting.password[j] != 0; j++){
            if(!isprint(setting.password[j])){
                goto send_pkt;
            }
        }
        if(!api_check_digit_range(setting.mtu, MIN_MTU, 1492)){
            return_str = ERROR_MTU_STR;
            goto send_pkt;
        }
        if (setting.connType == 1){
            if(!api_check_digit_range(setting.max_idle_time, MIN_IDLE_TIME, MAX_IDLE_TIME)){
                return_str = ERROR_IDLE_TIME_STR;
                goto send_pkt;
            }
        }
//Set
        api_set_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,WAN_PPPOE);          //proto_display
        api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_PPPOE);                  //proto
        api_set_string_option(NETWORK_WAN_USERNAME_OPTION, setting.user_name, sizeof(setting.user_name));
        api_set_string_option(NETWORK_WAN_PASSWORD_OPTION, setting.password, sizeof(setting.password));
        api_set_string_option(NETWORK_WAN_SERVICE_OPTION, setting.service_name, sizeof(setting.service_name));
        api_set_integer_option(NETWORK_WAN_MTU_OPTION, setting.mtu);
        api_set_integer_option(NETWORK_WAN_CONNTYPE_OPTION, setting.connType);
        api_set_integer_option(NETWORK_WAN_IDLETIME_OPTION, setting.max_idle_time);
        break;

    case WAN_L2TP:
//Check
        memset(lanIP, 0, sizeof(lanIP));
        memset(lanMask, 0, sizeof(lanMask));
        sysutil_get_lan_ipaddr(lanIP, sizeof(lanIP));
        sysutil_get_lan_netmask(lanMask, sizeof(lanMask));

        if (setting.IsIPDynamic)
        {
            if(strlen(setting.hostname) != 0){
                if(!api_check_checkHostNameString(setting.hostname)){
                    return_str = ERROR_HOSTNAME_STR;
                    goto send_pkt;
                }
            }
            if(!api_check_mac(setting.mac_address)){
                return_str = ERROR_MAC_ADDRESS_STR;
                goto send_pkt;
            }
        }
        else
        {
            if(!api_check_ip_addr(setting.ip_address)){
                return_str = ERROR_IP_ADDRESS_STR;
                goto send_pkt;
            }
            if(!api_check_mask_addr(setting.subnet_mask)){
                return_str = ERROR_SUBNET_MASK_STR;
                goto send_pkt;
            }
            if(api_check_is_same_subnet(setting.ip_address, lanIP, lanMask)){
                return_str = ERROR_IP_ADDRESS_STR;
                goto send_pkt;
            }
            if(!api_check_ip_addr(setting.service_ip)){
                return_str = ERROR_L2TP_GATEWAY_STR;
                goto send_pkt;
            }
            if(strcmp(setting.gateway, "") == 0 || strcmp(setting.gateway, "0.0.0.0") == 0 || strcmp(setting.gateway, "NULL") == 0)
            {
                if(!api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.service_ip,1)){
                    return_str = ERROR_L2TP_GATEWAY_STR;
                    goto send_pkt;
                }
            }
            else{
                if(!api_check_ip_addr(setting.gateway)){
                    return_str = ERROR_GATEWAY_STR;
                    goto send_pkt;
                }
                if(!api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.gateway,1)){
                    return_str = ERROR_GATEWAY_STR;
                    goto send_pkt;
                }
                if(api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.service_ip,1)){
                    return_str = ERROR_L2TP_GATEWAY_STR;
                    goto send_pkt;
                }
            }
        }

        if(strcmp(setting.user_name, "") == 0){
            return_str = ERROR_USER_NAME_STR;
            goto send_pkt;
        }
        for(i = 0; setting.user_name[i] != 0; i++){
            if(!isprint(setting.user_name[i])){
                return_str = ERROR_USER_NAME_STR;
                goto send_pkt;
            }
        }
        if(strcmp(setting.password, "") == 0) goto send_pkt;
        for (j = 0; setting.password[j] != 0; j++){
            if(!isprint(setting.password[j])){
                goto send_pkt;
            }
        }
        if(!api_check_ip_addr(setting.service_ip)){
            return_str = ERROR_L2TP_GATEWAY_STR;
            goto send_pkt;
        }
        if(api_check_is_same_subnet(setting.service_ip, lanIP, lanMask)){
            return_str = ERROR_L2TP_GATEWAY_STR;
            goto send_pkt;
        }
        if(!api_check_digit_range(setting.mtu, MIN_MTU, MAX_MTU))
        {
            return_str = ERROR_MTU_STR;
            goto send_pkt;
        }
        if (setting.connType == 1){
            if(!api_check_digit_range(setting.max_idle_time, MIN_IDLE_TIME, MAX_IDLE_TIME))
            {
                return_str = ERROR_IDLE_TIME_STR;
                goto send_pkt;
            }
        }

//Set

        api_set_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,WAN_L2TP);            //proto_display
        if (setting.IsIPDynamic)
        {           
            api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_DHCP);                //proto
            api_set_integer_option(NETWORK_WAN_L2TP_IPTYPE_OPTION, 0);                  //IsIPDynamic
            //apCfgSetIntValue(WAN1_PPTP_TYPE_TOK,WAN_CONNECTION_TYPE_DHCP);
            api_set_wan_hostname_option(NETWORK_WAN_HOSTNAME_OPTION,setting.hostname,sizeof(setting.hostname));
            api_set_wan_mac_option(NETWORK_WAN_MAC_OPTION,setting.mac_address,sizeof(setting.mac_address));
        }
        else
        {
            //if(0 != strcmp(setting.ip_address,getWanIP())){
            //    isReboot = 1;
            //}

            api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_STATIC);                  //proto
            api_set_integer_option(NETWORK_WAN_L2TP_IPTYPE_OPTION, 1);                      //IsIPDynamic
            api_set_wan_ipaddr_option(NETWORK_WAN_L2TP_GUI_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
            api_set_wan_ipaddr_option(NETWORK_WAN_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
            api_set_wan_netmask_option(NETWORK_WAN_L2TP_GUI_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
            api_set_wan_netmask_option(NETWORK_WAN_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
            api_set_wan_gateway_option(NETWORK_WAN_L2TP_GUI_GATEWAY_OPTION, setting.gateway, sizeof(setting.gateway));
            api_set_wan_gateway_option(NETWORK_WAN_GATEWAY_OPTION, setting.gateway, sizeof(setting.gateway));
        }

        api_set_wan_user_option(NETWORK_WAN_L2TP_USERNAME_OPTION, setting.user_name, sizeof(setting.user_name));
        api_set_wan_password_option(NETWORK_WAN_L2TP_PASSWORD_OPTION, setting.password, sizeof(setting.password));
        api_set_string_option(NETWORK_WAN_L2TP_GATEWAY_OPTION, setting.service_ip, sizeof(setting.service_ip));
        api_set_integer_option(NETWORK_WAN_L2TP_CONNECTIONID, setting.conId);
        api_set_integer_option(NETWORK_WAN_L2TP_MTU_OPTION, setting.mtu);
        api_set_integer_option(NETWORK_WAN_L2TP_CONNTYPE_OPTION, setting.connType);

        if (setting.connType == CONN_AUTO)
        {
            api_set_integer_option(NETWORK_WAN_L2TP_IDLETIME_OPTION, setting.max_idle_time);
        }
        break;

#if SUPPORT_IPV6_SETTING
    case WAN_DSLITE:
//Check
        if(!regxMatch(IPV6_REGX, setting.ipv6_ds_aftr_ip6))
        {
            return_str = ERROR_AFTER_IPV6_ADDRESS_STR;
            goto send_pkt;
        }

        if(!regxMatch(IPV6_REGX, setting.ipv6_ds_b4_ip4))
        {
            return_str = ERROR_B4_IPV6_ADDRESS_STR;
            goto send_pkt;
        }
//Set
        api_set_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,WAN_DSLITE);          //proto_display
        api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_DSLITE);                  //proto
        api_set_string_option(NETWORK_WAN_AFTRIPV6ADDRESS_OPTION, setting.ipv6_ds_aftr_ip6, sizeof(setting.ipv6_ds_aftr_ip6));
        api_set_string_option(NETWORK_WAN_IPV6WANADDRESS_OPTION, setting.ipv6_ds_b4_ip4, sizeof(setting.ipv6_ds_b4_ip4));
        break; 
#endif
    }

    /* Save new token value */

    if(result)
    {
        if(isReboot){
            APPAGENT_SYSTEM("uci commit");
            APPAGENT_SYSTEM("reboot &");
        }    
        else{
            APPAGENT_SYSTEM("rm -f /tmp/resolv.conf.auto");
            APPAGENT_SYSTEM("uci commit");
            APPAGENT_SYSTEM("ubus call network reload &");
        }

        return_str = REBOOT_STR;
    }

send_pkt:
    /* Send response packet */
    send_simple_response(pkt, return_str);
    return 0;

}

/*****************************************************************
* NAME:    set_wan_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_wan_settings_cb(HTTPS_CB *pkt){

    bool result = TRUE;
    char *return_str;
    int isReboot=0;
    int i, j;
    char mac_tmp_input[20] = {0};
    char dns[127+1]={0};
    char lanIP[15+1] = {0},lanMask[15+1] = {0};
#if 0
    char *query_str="{\n" \
                    "  \"Type\" : \"Static IP\",\n" \
                    "  \"IPAddress\" : \"172.1.1.1\",\n" \
                    "  \"SubnetMask\" : \"255.255.255.0\",\n" \
                    "  \"Gateway\" : \"172.1.1.254\",\n" \
                    "  \"PrimaryDNS\" : \"\",\n" \
                    "  \"SecondaryDNS\" : \"\" \n" \
                    "}";
#else
    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
#endif

    WAN_SETTINGS_T setting;

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if(NULL == pkt || 0 == strlen(query_str))
    {
        goto send_pkt;
    }

    memset(&setting, 0x00, sizeof(WAN_SETTINGS_T));

    if(pkt->json)
    {
        result &= parse_wan_settings_json_cb(query_str, &setting, &return_str);
    }
    if(TRUE != result) goto send_pkt;

    /* Check value */
    for(i = 0; i < T_NUM_OF_ELEMENTS(wanConnectTypeArr); i++){
         if(strcmp(wanConnectTypeArr[i].name,setting.type) == 0){
             break;
         }
         else if(i == T_NUM_OF_ELEMENTS(wanConnectTypeArr)-1){
            return_str = ERROR_TYPE_NOT_SUPPORTED_STR;
            goto send_pkt;
         }
    }

#if 1//HAS_MAC_CLONE
    if(setting.clonedMacStatus){
        strcpy(setting.mac_address, (char *)api_cloneMacAddress(pkt->fd));
    }
    sysutil_add_colon_to_mac(setting.mac_address,setting.mac_address);
#endif
    switch(setting.typeIdx)
    {
    case WAN_STATIC:
//Check
        memset(lanIP, 0, sizeof(lanIP));
        memset(lanMask, 0, sizeof(lanMask));
        sysutil_get_lan_ipaddr(lanIP, sizeof(lanIP));
        sysutil_get_lan_netmask(lanMask, sizeof(lanMask));

        if(!api_check_ip_addr(setting.ip_address)){
            return_str = ERROR_IP_ADDRESS_STR;
            goto send_pkt;
        }
        if(!api_check_mask_addr(setting.subnet_mask)){
            return_str = ERROR_SUBNET_MASK_STR;
            goto send_pkt;
        }
        if(!api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.ip_address,1)){
            return_str = ERROR_IP_ADDRESS_STR;
            goto send_pkt;
        }
        if(api_check_is_same_subnet(setting.ip_address, lanIP, lanMask)){
            return_str = ERROR_IP_ADDRESS_STR;
            goto send_pkt;
        }
        if(!api_check_ip_addr(setting.gateway)){
            return_str = ERROR_GATEWAY_STR;
            goto send_pkt;
        }
        if(!api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.gateway,1)){
            return_str = ERROR_GATEWAY_STR;
            goto send_pkt;
        }
        if(strlen(setting.dns_primary) != 0 && !api_check_ip_addr(setting.dns_primary)){
            return_str = ERROR_DNS1_IP_ADDRESS_STR;
            goto send_pkt;
        }
        if(strlen(setting.dns_secondary) != 0 && !api_check_ip_addr(setting.dns_secondary)){
            return_str = ERROR_DNS2_IP_ADDRESS_STR;
            goto send_pkt;
        }
//Set
        /* 
        if(0 != strcmp(setting.ip_address,getWanIP())){
            isReboot = 1;
        } 
        */ 
        api_set_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,WAN_STATIC);          //proto_display
        api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_STATIC);                  //proto
        api_set_wan_ipaddr_option(NETWORK_WAN_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
        api_set_wan_ipaddr_option(NETWORK_WAN_GUI_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
        api_set_wan_netmask_option(NETWORK_WAN_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
        api_set_wan_netmask_option(NETWORK_WAN_GUI_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
        api_set_wan_gateway_option(NETWORK_WAN_GATEWAY_OPTION, setting.gateway, sizeof(setting.gateway));
        api_set_wan_gateway_option(NETWORK_WAN_GUI_GATEWAY_OPTION, setting.gateway, sizeof(setting.gateway));

        if(strlen(setting.dns_secondary) != 0){
           strcpy(dns,setting.dns_primary);
           strcat(dns," ");
           strcat(dns,setting.dns_secondary);
           api_set_wan_dns_option(NETWORK_WAN_DNS_OPTION, dns, sizeof(dns));
        }
        else
        {
           api_set_wan_dns_option(NETWORK_WAN_DNS_OPTION, setting.dns_primary, sizeof(setting.dns_primary));
        }
       
        break;

    case WAN_DHCP:
//Check
        if(strlen(setting.hostname) != 0){
            if(!api_check_checkHostNameString(setting.hostname)){
                return_str = ERROR_HOSTNAME_STR;
                goto send_pkt;
            }
        }
		if(!api_check_mac(setting.mac_address)){
            return_str = ERROR_MAC_ADDRESS_STR;
            goto send_pkt;
        }
//Set
        api_set_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,WAN_DHCP);          //proto_display
        api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_DHCP);                  //proto
        api_set_wan_hostname_option(NETWORK_WAN_HOSTNAME_OPTION,setting.hostname,sizeof(setting.hostname));
        api_set_wan_mac_option(NETWORK_WAN_MAC_OPTION,setting.mac_address,sizeof(setting.mac_address));
        break;

    case WAN_PPTP:
//Check
        memset(lanIP, 0, sizeof(lanIP));
        memset(lanMask, 0, sizeof(lanMask));
        sysutil_get_lan_ipaddr(lanIP, sizeof(lanIP));
        sysutil_get_lan_netmask(lanMask, sizeof(lanMask));

        if (setting.IsIPDynamic)
        {
            if(strlen(setting.hostname) != 0){
        		if(!api_check_checkHostNameString(setting.hostname)){
                    return_str = ERROR_HOSTNAME_STR;
                    goto send_pkt;
                }
            }
    		if(!api_check_mac(setting.mac_address)){
                return_str = ERROR_MAC_ADDRESS_STR;
                goto send_pkt;
            }
        }
        else
        {
            if(!api_check_ip_addr(setting.ip_address)){
                return_str = ERROR_IP_ADDRESS_STR;
                goto send_pkt;
            }
            if(!api_check_mask_addr(setting.subnet_mask)){
                return_str = ERROR_SUBNET_MASK_STR;
                goto send_pkt;
            }
            if(api_check_is_same_subnet(setting.ip_address, lanIP, lanMask)){
                return_str = ERROR_IP_ADDRESS_STR;
                goto send_pkt;
            }
            if(!api_check_ip_addr(setting.service_ip)){
                return_str = ERROR_PPTP_GATEWAY_STR;
                goto send_pkt;
            }

            if(strcmp(setting.gateway, "") == 0 || strcmp(setting.gateway, "0.0.0.0") == 0 || strcmp(setting.gateway, "NULL") == 0)
            {
                if(!api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.service_ip,1)){
                    return_str = ERROR_PPTP_GATEWAY_STR;
                    goto send_pkt;
                }
            }
            else{
                if(!api_check_ip_addr(setting.gateway)){
                    return_str = ERROR_GATEWAY_STR;
                    goto send_pkt;
                }
                if(!api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.gateway,1)){
                    return_str = ERROR_GATEWAY_STR;
                    goto send_pkt;
                }
                if(api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.service_ip,1)){
                    return_str = ERROR_PPTP_GATEWAY_STR;
                    goto send_pkt;
                }
            }
        }
        if(strcmp(setting.user_name, "") == 0){
            return_str = ERROR_USER_NAME_STR;
            goto send_pkt;
        }
        for(i = 0; setting.user_name[i] != 0; i++){
            if(!isprint(setting.user_name[i])){
                return_str = ERROR_USER_NAME_STR;
                goto send_pkt;
            }
        }
        if(strcmp(setting.password, "") == 0) goto send_pkt;
        for (j = 0; setting.password[j] != 0; j++){
            if(!isprint(setting.password[j])){
                goto send_pkt;
            }
        }
        if(!api_check_ip_addr(setting.service_ip)){
            return_str = ERROR_PPTP_GATEWAY_STR;
            goto send_pkt;
        }
        if(api_check_is_same_subnet(setting.service_ip, lanIP, lanMask)){
            return_str = ERROR_PPTP_GATEWAY_STR;
            goto send_pkt;
        }
        if((int *)setting.conId != NULL){
            if(!api_check_digit_range(setting.conId, 0, 65535)){
                goto send_pkt;
            }
        }
        if(!api_check_digit_range(setting.mtu, MIN_MTU, MAX_MTU))
        {
            return_str = ERROR_MTU_STR;
            goto send_pkt;
        }
        if (setting.connType == 1){
            if(!api_check_digit_range(setting.max_idle_time, MIN_IDLE_TIME, MAX_IDLE_TIME))
            {
                return_str = ERROR_IDLE_TIME_STR;
                goto send_pkt;
            }
        }

//Set
        api_set_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,WAN_PPTP); //proto_display
        if (setting.IsIPDynamic)                                             // dynamic IP
        {
            api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_DHCP);                //proto
            api_set_integer_option(NETWORK_WAN_PPTP_IPTYPE_OPTION, 0);                  //IsIPDynamic
            //apCfgSetIntValue(WAN1_PPTP_TYPE_TOK,WAN_CONNECTION_TYPE_DHCP);
            api_set_wan_hostname_option(NETWORK_WAN_HOSTNAME_OPTION,setting.hostname,sizeof(setting.hostname));
            api_set_wan_mac_option(NETWORK_WAN_MAC_OPTION,setting.mac_address,sizeof(setting.mac_address));
        }
        else                                                                    // static IP
        {
            //if(0 != strcmp(setting.ip_address,getWanIP())){
            //    isReboot = 1;
            //}
            api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_STATIC);                  //proto
            api_set_integer_option(NETWORK_WAN_PPTP_IPTYPE_OPTION, 1);                      //IsIPDynamic
            api_set_wan_ipaddr_option(NETWORK_WAN_PPTP_GUI_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
            api_set_wan_ipaddr_option(NETWORK_WAN_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
            api_set_wan_netmask_option(NETWORK_WAN_PPTP_GUI_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
            api_set_wan_netmask_option(NETWORK_WAN_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
            api_set_wan_gateway_option(NETWORK_WAN_PPTP_GUI_GATEWAY_OPTION, setting.gateway, sizeof(setting.gateway));
            api_set_wan_gateway_option(NETWORK_WAN_GATEWAY_OPTION, setting.gateway, sizeof(setting.gateway));
        }
        api_set_wan_user_option(NETWORK_WAN_PPTP_USERNAME_OPTION, setting.user_name, sizeof(setting.user_name));
        api_set_wan_password_option(NETWORK_WAN_PPTP_PASSWORD_OPTION, setting.password, sizeof(setting.password));
        api_set_string_option(NETWORK_WAN_PPTP_GATEWAY_OPTION, setting.service_ip, sizeof(setting.service_ip));
        api_set_integer_option(NETWORK_WAN_PPTP_CONNECTIONID, setting.conId);
        api_set_integer_option(NETWORK_WAN_PPTP_MTU_OPTION, setting.mtu);
        api_set_integer_option(NETWORK_WAN_PPTP_CONNTYPE_OPTION, setting.connType);

        if (setting.connType == CONN_AUTO)
        {
            api_set_integer_option(NETWORK_WAN_PPTP_IDLETIME_OPTION, setting.max_idle_time);
        }
        break;

    case WAN_PPPOE:                             //PPPOE
//Check
        if(strcmp(setting.user_name, "") == 0) {
            return_str = ERROR_USER_NAME_STR;
            goto send_pkt;
        }
        for(i = 0; setting.user_name[i] != 0; i++){
            if(!isprint(setting.user_name[i])){
                return_str = ERROR_USER_NAME_STR;
                goto send_pkt;
            }
        }
        if(strcmp(setting.password, "") == 0) goto send_pkt;
        for (j = 0; setting.password[j] != 0; j++){
            if(!isprint(setting.password[j])){
                goto send_pkt;
            }
        }
        if(!api_check_digit_range(setting.mtu, MIN_MTU, 1492)){
            return_str = ERROR_MTU_STR;
            goto send_pkt;
        }
        if (setting.connType == 1){
            if(!api_check_digit_range(setting.max_idle_time, MIN_IDLE_TIME, MAX_IDLE_TIME)){
                return_str = ERROR_IDLE_TIME_STR;
                goto send_pkt;
            }
        }
//Set
        api_set_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,WAN_PPPOE);          //proto_display
        api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_PPPOE);                  //proto
        api_set_string_option(NETWORK_WAN_USERNAME_OPTION, setting.user_name, sizeof(setting.user_name));
        api_set_string_option(NETWORK_WAN_PASSWORD_OPTION, setting.password, sizeof(setting.password));
        api_set_string_option(NETWORK_WAN_SERVICE_OPTION, setting.service_name, sizeof(setting.service_name));
        api_set_integer_option(NETWORK_WAN_MTU_OPTION, setting.mtu);
        api_set_integer_option(NETWORK_WAN_CONNTYPE_OPTION, setting.connType);
        api_set_integer_option(NETWORK_WAN_IDLETIME_OPTION, setting.max_idle_time);
        break;

    case WAN_L2TP:
//Check
        memset(lanIP, 0, sizeof(lanIP));
        memset(lanMask, 0, sizeof(lanMask));
        sysutil_get_lan_ipaddr(lanIP, sizeof(lanIP));
        sysutil_get_lan_netmask(lanMask, sizeof(lanMask));

        if (setting.IsIPDynamic)
        {
            if(strlen(setting.hostname) != 0){
                if(!api_check_checkHostNameString(setting.hostname)){
                    return_str = ERROR_HOSTNAME_STR;
                    goto send_pkt;
                }
            }
            if(!api_check_mac(setting.mac_address)){
                return_str = ERROR_MAC_ADDRESS_STR;
                goto send_pkt;
            }
        }
        else
        {
            if(!api_check_ip_addr(setting.ip_address)){
                return_str = ERROR_IP_ADDRESS_STR;
                goto send_pkt;
            }
            if(!api_check_mask_addr(setting.subnet_mask)){
                return_str = ERROR_SUBNET_MASK_STR;
                goto send_pkt;
            }
            if(api_check_is_same_subnet(setting.ip_address, lanIP, lanMask)){
                return_str = ERROR_IP_ADDRESS_STR;
                goto send_pkt;
            }
            if(!api_check_ip_addr(setting.service_ip)){
                return_str = ERROR_L2TP_GATEWAY_STR;
                goto send_pkt;
            }
            if(strcmp(setting.gateway, "") == 0 || strcmp(setting.gateway, "0.0.0.0") == 0 || strcmp(setting.gateway, "NULL") == 0)
            {
                if(!api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.service_ip,1)){
                    return_str = ERROR_L2TP_GATEWAY_STR;
                    goto send_pkt;
                }
            }
            else{
                if(!api_check_ip_addr(setting.gateway)){
                    return_str = ERROR_GATEWAY_STR;
                    goto send_pkt;
                }
                if(!api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.gateway,1)){
                    return_str = ERROR_GATEWAY_STR;
                    goto send_pkt;
                }
                if(api_check_subnetIPwithNetAndBroadcast(setting.ip_address, setting.subnet_mask, setting.service_ip,1)){
                    return_str = ERROR_L2TP_GATEWAY_STR;
                    goto send_pkt;
                }
            }
        }

        if(strcmp(setting.user_name, "") == 0){
            return_str = ERROR_USER_NAME_STR;
            goto send_pkt;
        }
        for(i = 0; setting.user_name[i] != 0; i++){
            if(!isprint(setting.user_name[i])){
                return_str = ERROR_USER_NAME_STR;
                goto send_pkt;
            }
        }
        if(strcmp(setting.password, "") == 0) goto send_pkt;
        for (j = 0; setting.password[j] != 0; j++){
            if(!isprint(setting.password[j])){
                goto send_pkt;
            }
        }
        if(!api_check_ip_addr(setting.service_ip)){
            return_str = ERROR_L2TP_GATEWAY_STR;
            goto send_pkt;
        }
        if(api_check_is_same_subnet(setting.service_ip, lanIP, lanMask)){
            return_str = ERROR_L2TP_GATEWAY_STR;
            goto send_pkt;
        }
        if(!api_check_digit_range(setting.mtu, MIN_MTU, MAX_MTU))
        {
            return_str = ERROR_MTU_STR;
            goto send_pkt;
        }
        if (setting.connType == 1){
            if(!api_check_digit_range(setting.max_idle_time, MIN_IDLE_TIME, MAX_IDLE_TIME))
            {
                return_str = ERROR_IDLE_TIME_STR;
                goto send_pkt;
            }
        }

//Set

        api_set_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,WAN_L2TP);            //proto_display
        if (setting.IsIPDynamic)
        {           
            api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_DHCP);                //proto
            api_set_integer_option(NETWORK_WAN_L2TP_IPTYPE_OPTION, 0);                  //IsIPDynamic
            //apCfgSetIntValue(WAN1_PPTP_TYPE_TOK,WAN_CONNECTION_TYPE_DHCP);
            api_set_wan_hostname_option(NETWORK_WAN_HOSTNAME_OPTION,setting.hostname,sizeof(setting.hostname));
            api_set_wan_mac_option(NETWORK_WAN_MAC_OPTION,setting.mac_address,sizeof(setting.mac_address));
        }
        else
        {
            //if(0 != strcmp(setting.ip_address,getWanIP())){
            //    isReboot = 1;
            //}

            api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_STATIC);                  //proto
            api_set_integer_option(NETWORK_WAN_L2TP_IPTYPE_OPTION, 1);                      //IsIPDynamic
            api_set_wan_ipaddr_option(NETWORK_WAN_L2TP_GUI_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
            api_set_wan_ipaddr_option(NETWORK_WAN_IPADDR_OPTION, setting.ip_address, sizeof(setting.ip_address));
            api_set_wan_netmask_option(NETWORK_WAN_L2TP_GUI_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
            api_set_wan_netmask_option(NETWORK_WAN_NETMASK_OPTION, setting.subnet_mask, sizeof(setting.subnet_mask));
            api_set_wan_gateway_option(NETWORK_WAN_L2TP_GUI_GATEWAY_OPTION, setting.gateway, sizeof(setting.gateway));
            api_set_wan_gateway_option(NETWORK_WAN_GATEWAY_OPTION, setting.gateway, sizeof(setting.gateway));
        }

        api_set_wan_user_option(NETWORK_WAN_L2TP_USERNAME_OPTION, setting.user_name, sizeof(setting.user_name));
        api_set_wan_password_option(NETWORK_WAN_L2TP_PASSWORD_OPTION, setting.password, sizeof(setting.password));
        api_set_string_option(NETWORK_WAN_L2TP_GATEWAY_OPTION, setting.service_ip, sizeof(setting.service_ip));
        api_set_integer_option(NETWORK_WAN_L2TP_CONNECTIONID, setting.conId);
        api_set_integer_option(NETWORK_WAN_L2TP_MTU_OPTION, setting.mtu);
        api_set_integer_option(NETWORK_WAN_L2TP_CONNTYPE_OPTION, setting.connType);

        if (setting.connType == CONN_AUTO)
        {
            api_set_integer_option(NETWORK_WAN_L2TP_IDLETIME_OPTION, setting.max_idle_time);
        }
        break;

#if SUPPORT_IPV6_SETTING
    case WAN_DSLITE:
//Check
        if(!regxMatch(IPV6_REGX, setting.ipv6_ds_aftr_ip6))
        {
            return_str = ERROR_AFTER_IPV6_ADDRESS_STR;
            goto send_pkt;
        }

        if(!regxMatch(IPV6_REGX, setting.ipv6_ds_b4_ip4))
        {
            return_str = ERROR_B4_IPV6_ADDRESS_STR;
            goto send_pkt;
        }
//Set
        api_set_wan_proto_option(NETWORK_WAN_PROTO_DISPLAY_OPTION,WAN_DSLITE);          //proto_display
        api_set_wan_proto_option(NETWORK_WAN_PROTO_OPTION,WAN_DSLITE);                  //proto
        api_set_string_option(NETWORK_WAN_AFTRIPV6ADDRESS_OPTION, setting.ipv6_ds_aftr_ip6, sizeof(setting.ipv6_ds_aftr_ip6));
        api_set_string_option(NETWORK_WAN_IPV6WANADDRESS_OPTION, setting.ipv6_ds_b4_ip4, sizeof(setting.ipv6_ds_b4_ip4));
        break; 
#endif
    }

    /* Save new token value */

    if(result)
    {
        if(isReboot){
            APPAGENT_SYSTEM("uci commit");
            APPAGENT_SYSTEM("reboot &");
        }    
        else{
            APPAGENT_SYSTEM("rm -f /tmp/resolv.conf.auto");
            APPAGENT_SYSTEM("uci commit");
            APPAGENT_SYSTEM("ubus call network reload &");
        }

        return_str = REBOOT_STR;
    }

send_pkt:
    /* Send response packet */
    send_simple_response(pkt, return_str);
    return 0;

}
#if SUPPORT_IPV6_SETTING
/*****************************************************************
* NAME:    get_ipv6_wan_status_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   ynyang
* Modify:   
******************************************************************/
int get_ipv6_wan_status_cb(HTTPS_CB *pkt){
    int  i, rval;
    char type[16];
    char wan_if[16];
    char buf[128];
    char *ptr;
    char *result;
    WAN_IPV6_SETTINGS_T setting;

    if(NULL == pkt)
        return -1;

    memset(&setting, 0x00, sizeof(WAN_IPV6_SETTINGS_T));

    memset(type, 0x00, sizeof(type));
    api_get_string_option(NETWORK_WAN6_PROTO_OPTION, type, sizeof(type));

    i = 0;
    while((NULL != wanIpv6ConnectTypeArr[i].setting_name) && (0 != strcmp(wanIpv6ConnectTypeArr[i].setting_name, type)))
    {
        i++;
    }

    if(IPV6_WAN_TYPE_MAX == i)
    {
        result = ERROR_TYPE_NOT_SUPPORTED_STR;
        goto send_pkt;
    }

    setting.type_index = i;
    sprintf(setting.type, wanIpv6ConnectTypeArr[i].display_name);
    sprintf(wan_if, wanIpv6ConnectTypeArr[i].interface_name);

    memset(buf, 0x00, sizeof(buf));
    rval = sys_interact(buf, sizeof(buf), "ifconfig %s | grep \"Scope:Global\"", wan_if);
    sprintf(setting.status, "%s", (0 == strlen(buf))?DISCONNECTED_STR:CONNECTED_STR);

    memset(buf, 0x00, sizeof(buf));
    rval = sys_interact(buf, sizeof(buf), "ifconfig %s | grep \"/128 Scope:Global\" | awk -F \" \" '{print $3}'", wan_if);
    buf[strcspn(buf, "\n")] = '\0';
    sprintf(setting.ipv6_addr, "%s", buf);

    if(IPV6_WAN_6RD == i)
    {
        memset(buf, 0x00, sizeof(buf));
        rval = sys_interact(buf, sizeof(buf), "ifconfig 6rd-wan6 | grep \"Scope:Global\" | awk -F \" \" '{print $3}'");
        buf[strcspn(buf, "\n")] = '\0';
        sprintf(setting.tunnel_link_local, "%s", buf);
    }

    memset(buf, 0x00, sizeof(buf));
    rval = sys_interact(buf, sizeof(buf), "ifconfig %s | grep \"Scope:Link\" | awk -F \" \" '{print $3}'", wan_if);
    buf[strcspn(buf, "\n")] = '\0';
    sprintf(setting.default_gw, "%s", buf);

    memset(buf, 0x00, sizeof(buf));
    rval = sys_interact(buf, sizeof(buf), "ifconfig %s | grep \"Scope:Global\" | awk -F \" \" '{print $3}'", BRG_DEV);
    buf[strcspn(buf, "\n")] = '\0';
    sprintf(setting.lan_ipv6_addr, "%s", buf);

    memset(buf, 0x00, sizeof(buf));
    rval = sys_interact(buf, sizeof(buf), "ifconfig %s | grep \"Scope:Link\" | awk -F \" \" '{print $3}'", BRG_DEV);
    buf[strcspn(buf, "\n")] = '\0';
    sprintf(setting.lan_link_local, "%s", buf);

    if(sysutil_check_file_existed("/tmp/resolv.conf.auto"))
    {
        memset(buf, 0x00, sizeof(buf));
        rval = sys_interact(buf, sizeof(buf), "cat /tmp/resolv.conf.auto | grep nameserver | awk -F \" \" '{print $2}'");
        ptr = strchr(buf, '\n');
        if(NULL != ptr)
        {
            ptr++;
            snprintf(setting.primary_dns, (ptr - buf), "%s", buf);

            if(NULL != strchr(ptr, '\n'))
            {
                ptr[strcspn(ptr, "\n")] = '\0';
                sprintf(setting.secondary_dns, "%s", ptr);
            }
        }
    }

    api_get_bool_option(NETWORK_LAN_DHCPPD_OPTION, (int *)&setting.enable_dhcpPD);

    memset(buf, 0x00, sizeof(buf));
    rval = sys_interact(buf, sizeof(buf), "ifconfig %s | grep \"Scope:Link\" | awk -F \" \" '{print $3}'", wan_if);
    buf[strcspn(buf, "\n")] = '\0';
    sprintf(setting.wan_link_local, "%s", buf);

    result = OK_STR;

send_pkt:
    if(pkt->json)
        get_ipv6_wan_status_json_cb(pkt, &setting, result);

    return 0;
}

/*****************************************************************
* NAME:    get_ipv6_wan_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int get_ipv6_wan_settings_cb(HTTPS_CB *pkt){
    int  i, rval;
    int  flag_rdnss, flag_prefix, flag_adv_managed, flag_adv_other_config;
    char type[16];
    char wan_if[16];
    char buf[128];
    char *ptr;
    char *result;
    WAN_IPV6_SETTINGS_T setting;

    if(NULL == pkt)
        return -1;

    memset(&setting, 0x00, sizeof(WAN_IPV6_SETTINGS_T));

    memset(type, 0x00, sizeof(type));
    api_get_string_option(NETWORK_WAN6_PROTO_OPTION, type, sizeof(type));

    i = 0;
    while((NULL != wanIpv6ConnectTypeArr[i].setting_name) && (0 != strcmp(wanIpv6ConnectTypeArr[i].setting_name, type)))
    {
        i++;
    }

    if(IPV6_WAN_TYPE_MAX == i)
    {
        result = ERROR_TYPE_NOT_SUPPORTED_STR;
        goto send_pkt;
    }

    setting.type_index = i;
    sprintf(setting.type, wanIpv6ConnectTypeArr[i].display_name);
    sprintf(wan_if, wanIpv6ConnectTypeArr[i].interface_name);

    // TODO : Not supported.
    // api_get_bool_option(NETWORK_WAN6_USELINKLOCAL_OPTION, (int *)&setting.use_link_local);

    memset(buf, 0x00, sizeof(buf));
    rval = sys_interact(buf, sizeof(buf), "ifconfig %s | grep \"/128 Scope:Global\" | awk -F \" \" '{print $3}'", wan_if);
    buf[strcspn(buf, "\n")] = '\0';
    sprintf(setting.ipv6_addr, "%s", buf);

    ptr = strchr(buf, '/');
    if(NULL != ptr)
    {
        setting.prefix_length = atoi(ptr+1);
    }

    memset(buf, 0x00, sizeof(buf));
    rval = sys_interact(buf, sizeof(buf), "ifconfig %s | grep \"Scope:Link\" | awk -F \" \" '{print $3}'", wan_if);
    buf[strcspn(buf, "\n")] = '\0';
    sprintf(setting.default_gw, "%s", buf);

    if(sysutil_check_file_existed("/tmp/resolv.conf.auto"))
    {
        memset(buf, 0x00, sizeof(buf));
        rval = sys_interact(buf, sizeof(buf), "cat /tmp/resolv.conf.auto | grep nameserver | awk -F \" \" '{print $2}'");
        ptr = strchr(buf, '\n');
        if(NULL != ptr)
        {
            ptr++;
            snprintf(setting.primary_dns, (ptr - buf), "%s", buf);

            if(NULL != strchr(ptr, '\n'))
            {
                ptr[strcspn(ptr, "\n")] = '\0';
                sprintf(setting.secondary_dns, "%s", ptr);
            }
        }
    }

    memset(buf, 0x00, sizeof(buf));
    rval = sys_interact(buf, sizeof(buf), "ifconfig %s | grep \"Scope:Global\" | awk -F \" \" '{print $3}'", BRG_DEV);
    buf[strcspn(buf, "\n")] = '\0';
    sprintf(setting.lan_ipv6_addr, "%s", buf);

#if SUPPORT_DHCP6S_SETTING
    api_get_bool_option(DHCP6S_BASIC_ENABLED_OPTION, (int *)&setting.enable_auto_assignment);
#else
    setting.enable_auto_assignment = false;
#endif

    // RDNSS or stateless or stateful of LAN
    api_get_bool_option(RADVD_RDNSS_IGNORE_OPTION, (int *)&flag_rdnss);
    api_get_bool_option(RADVD_PREFIX_IGNORE_OPTION, (int *)&flag_prefix);
    api_get_bool_option(RADVD_INTERFACE_ADVMANAGEDFLAG_OPTION, (int *)&flag_adv_managed);
    api_get_bool_option(RADVD_INTERFACE_ADVOTHERCONFIGFLAG_OPTION, (int *)&flag_adv_other_config);

    if(flag_rdnss)
    {
        if(flag_prefix && flag_adv_managed && (0 == flag_adv_other_config))
        {
            setting.auto_config_type = 1;
        }
        else if((0 == flag_prefix) && (0 == flag_adv_managed) && flag_adv_other_config)
        {
            setting.auto_config_type = 2;
        }
    }
    else
    {
        if((0 == flag_prefix) && (0 == flag_adv_managed) && (0 == flag_adv_other_config))
        {
            setting.auto_config_type = 0;
        }
    }

#if SUPPORT_DHCP6S_SETTING
    api_get_string_option(DHCP6S_BASIC_IP_START_OPTION, setting.start, sizeof(setting.start));
    api_get_string_option(DHCP6S_BASIC_IP_END_OPTION, setting.end, sizeof(setting.end));
#endif

    api_get_integer_option(RADVD_PREFIX_ADVVALIDLIFETIME_OPTION, &setting.ra_life_time);

    setting.dhcpv6_life_time = 3600; // TODO : Hard code 3600

    // TODO : setting.is_ip_dynamic // Not supported.

    if(IPV6_WAN_PPPOE == i)
    {
        api_get_string_option(NETWORK_WAN6_USERNAME_OPTION, setting.username, sizeof(setting.username));
        api_get_string_option(NETWORK_WAN6_PASSWORD_OPTION, setting.password, sizeof(setting.password));
        api_get_string_option(NETWORK_WAN6_SERVICE_OPTION, setting.service, sizeof(setting.service));
    }

    // TODO : setting.reconnect_mode // Not supported.

    api_get_integer_option(NETWORK_WAN6_MTU_OPTION, &setting.mtu);
    api_get_bool_option(NETWORK_WAN6_DNS_OPTION, (int *)&setting.enable_auto_dns);
    api_get_bool_option(NETWORK_LAN_DHCPPD_OPTION, (int *)&setting.enable_dhcpPD);

    // TODO : setting._6rd_type // Not supported.

    api_get_string_option(NETWORK_WAN6_IP6PREFIX_OPTION, setting.prefix, sizeof(setting.prefix));
    api_get_integer_option(NETWORK_WAN6_IP6PREFIXLEN_OPTION, &setting.mask_length);

    api_get_string_option(NETWORK_WAN6_PEERADDR_OPTION, setting.relay, sizeof(setting.relay));

    memset(buf, 0x00, sizeof(buf));
    rval = sys_interact(buf, sizeof(buf), "ifconfig %s | grep \"Scope:Link\" | awk -F \" \" '{print $3}'", wan_if);
    buf[strcspn(buf, "\n")] = '\0';
    sprintf(setting.wan_link_local, "%s", buf);

    memset(buf, 0x00, sizeof(buf));
    rval = sys_interact(buf, sizeof(buf), "ifconfig %s | grep \"Scope:Link\" | awk -F \" \" '{print $3}'", BRG_DEV);
    buf[strcspn(buf, "\n")] = '\0';
    sprintf(setting.lan_link_local, "%s", buf);

    memset(buf, 0x00, sizeof(buf));
    rval = sys_interact(buf, sizeof(buf), "ifconfig %s | grep \"inet addr\" | awk -F \" \" '{print $2}' | cut -d \":\" -f 2", WAN_IF);
    buf[strcspn(buf, "\n")] = '\0';
    sprintf(setting.ipv4_addr, "%s", buf);

    result = OK_STR;

send_pkt:
    if(pkt->json)
        get_ipv6_wan_settings_json_cb(pkt, &setting, result);

    return 0;
}

/*****************************************************************
* NAME:    check_ipv6_wan_dns
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool check_ipv6_wan_dns(WAN_IPV6_SETTINGS_T *setting, char *error_log)
{
    bool result = TRUE, checked = FALSE;

    if(0 != strlen(setting->primary_dns))
    {
        checked = TRUE;

        if(!regxMatch(IPV6_REGX, setting->primary_dns))
        {
            result = FALSE;
            error_log = ERROR_DNS1_IP_ADDRESS_STR;
        }
    }

    if((0 != strlen(setting->secondary_dns)) && (!regxMatch(IPV6_REGX, setting->secondary_dns)))
    {
        result = FALSE;
        error_log = ERROR_DNS2_IP_ADDRESS_STR;
    }

    if(FALSE == checked) result = FALSE;

    return result;
}

/*****************************************************************
* NAME:    check_ipv6_wan_auto_assignment
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
bool check_ipv6_wan_auto_assignment(WAN_IPV6_SETTINGS_T *setting, char *error_log, int lifetime_min, int lifetime_max)
{
    bool result = TRUE;

    if(setting->enable_auto_assignment)
    {
        if(IPV6_WAN_AUTO_CONFIG_STATEFUL_DHCPV6 == setting->auto_config_type)
        {
            if(atoi(setting->start) > atoi(setting->end))
            {
                result = FALSE;
                error_log = ERROR_DHCP_START_ADDRESS_STR;
            }

            if(!api_check_digit_range(setting->dhcpv6_life_time, lifetime_min, lifetime_max))
            {
                result = FALSE;
                error_log = ERROR_LIFETIME_STR;
            }
        }
        else
        {
            if(!api_check_digit_range(setting->ra_life_time, lifetime_min, lifetime_max))
            {
                result = FALSE;
                error_log = ERROR_LIFETIME_STR;
            }
        }
    }

    return result;
}

/*****************************************************************
* NAME:    set_ipv6_wan_settings_cb
* ---------------------------------------------------------------
* FUNCTION: 
* INPUT:    
* OUTPUT:   
* Author:   
* Modify:   
******************************************************************/
int set_ipv6_wan_settings_cb(HTTPS_CB *pkt){
    int i;
    bool result = TRUE;
    char *return_str;
    char wan_ip[32];

    char *query_str=get_env(&pkt->envcfg, "QUERY_STRING");
    WAN_IPV6_SETTINGS_T setting;

    // The default value in return packet is ERROR.
    return_str = ERROR_STR;

    if(NULL == pkt || 0 == strlen(query_str))
    {
        goto send_pkt;
    }

    memset(&setting, 0x00, sizeof(WAN_IPV6_SETTINGS_T));

    if(pkt->json)
    {
        result &= parse_ipv6_wan_settings_json_cb(query_str, &setting, &return_str);
    }

    if(TRUE != result) goto send_pkt;

    i = 0;
    while((NULL != wanIpv6ConnectTypeArr[i].setting_name) && (0 != strcmp(wanIpv6ConnectTypeArr[i].display_name, setting.type)))
    {
        i++;
    }

    // Set to Auto-configuration if the setting type is unknown.
    if(IPV6_WAN_TYPE_MAX == i)
    {
        return_str = ERROR_TYPE_NOT_SUPPORTED_STR;
        goto send_pkt;
    }

    setting.type_index = i;
    memset(setting.type, 0x00, sizeof(setting.type));
    sprintf(setting.type, wanIpv6ConnectTypeArr[i].setting_name);

    switch(i)
    {
        case IPV6_WAN_STATIC:
            /* WLR-9100 does not support LAN IPv6 address. */
            if(setting.use_link_local) goto send_pkt;

            if(!regxMatch(IPV6_REGX, setting.ipv6_addr))
            {
                return_str = ERROR_IP_ADDRESS_STR;
                goto send_pkt;
            }

            if(!api_check_digit_range(setting.prefix_length, 2, 128))
            {
                return_str = ERROR_PREFIX_LENGTH_STR;
                goto send_pkt;
            }

            if(!regxMatch(IPV6_REGX, setting.default_gw))
            {
                return_str = ERROR_GATEWAY_STR;
                goto send_pkt;
            }

            result &= check_ipv6_wan_dns(&setting, return_str);

            if(FALSE == result) goto send_pkt;

#if 0
            /* WLR-9100 does not support LAN IPv6 address. */
            if(!regxMatch(IPV6_REGX, setting.lan_ipv6_addr))
            {
                return_str = ERROR_LAN_IP_ADDRESS_STR;
                goto send_pkt;
            }
#endif

            result &= check_ipv6_wan_auto_assignment(&setting, return_str, 180, 43200);

            if(FALSE == result) goto send_pkt;

            if(api_set_ipv6_wan_static(&setting)) goto send_pkt;

            break;
        case IPV6_WAN_PPPOE:
#if 0
            if(setting->is_ip_dynamic)
            {
                if(!regxMatch(IPV6_REGX, setting.ipv6_addr))
                {
                    return_str = ERROR_IP_ADDRESS_STR;
                    goto send_pkt;
                }
            }
#else
            /* WLR-9100 does not support "Manual IPv6 address" */
            if(setting.is_ip_dynamic) goto send_pkt;
#endif

            if((0 == strlen(setting.username))  || (!api_check_string_isprint(setting.username)))
            {
                return_str = ERROR_USER_NAME_STR;
                goto send_pkt;
            }

            if((0 == strlen(setting.password))  || (!api_check_string_isprint(setting.password)))
            {
                goto send_pkt;
            }

            if((0 == strlen(setting.service))  || (!api_check_checkHostNameString(setting.service)))
            {
                return_str = ERROR_HOSTNAME_STR;
                goto send_pkt;
            }

            if(!api_check_digit_range(setting.mtu, 1280, 1492))
            {
                return_str = ERROR_MTU_STR;
                goto send_pkt;
            }

#if 0
            if(!setting.enable_auto_dns)
            {
                result &= check_ipv6_wan_dns(&setting, return_str);

                if(FALSE == result) goto send_pkt;
            }
#else
            /* WLR-9100 IPv6 PPPoE does not support "Manual DNS setting" */
            if(!setting.enable_auto_dns) goto send_pkt;
#endif

            if(!setting.enable_dhcpPD)
            {
                if(!regxMatch(IPV6_REGX, setting.lan_ipv6_addr))
                {
                    return_str = ERROR_LAN_IP_ADDRESS_STR;
                    goto send_pkt;
                }
            }

            result &= check_ipv6_wan_auto_assignment(&setting, return_str, 180, 43200);

            if(FALSE == result) goto send_pkt;

            if(api_set_ipv6_wan_pppoe(&setting)) goto send_pkt;

            break;
        case IPV6_WAN_AUTOCONFIGURATION:
            if(!setting.enable_auto_dns)
            {
                result &= check_ipv6_wan_dns(&setting, return_str);

                if(FALSE == result) goto send_pkt;
            }

            if(!setting.enable_dhcpPD)
            {
                if(!regxMatch(IPV6_REGX, setting.lan_ipv6_addr))
                {
                    return_str = ERROR_LAN_IP_ADDRESS_STR;
                    goto send_pkt;
                }
            }

            result &= check_ipv6_wan_auto_assignment(&setting, return_str, 180, 43200);

            if(FALSE == result) goto send_pkt;

            if(api_set_ipv6_wan_dhcpv6(&setting)) goto send_pkt;

            break;
        case IPV6_WAN_6RD:
            if(sysutil_get_wan_ipaddr(wan_ip, sizeof(wan_ip)))
            {
                if(!regxMatch(IP_REGX, setting.relay))
                {
                    goto send_pkt;
                }

                if(!regxMatch(IPV6_REGX, setting.prefix))
                {
                    return_str = ERROR_PREFIX_STR;
                    goto send_pkt;
                }

                if(!api_check_digit_range(setting.prefix_length, 1, 63))
                {
                    return_str = ERROR_PREFIX_LENGTH_STR;
                    goto send_pkt;
                }

                if(!api_check_digit_range(setting.mask_length, (setting.prefix_length - 32), 31))
                {
                    return_str = ERROR_SUBNET_MASK_STR;
                    goto send_pkt;
                }
            }
            else
            {
                return_str = ERROR_IPV4_IP_ADDRESS_STR;
                goto send_pkt;
            }

            if(api_set_ipv6_wan_6rd(&setting)) goto send_pkt;

            break;
        case IPV6_WAN_LINK_LOCAL_ONLY:
            if(api_set_ipv6_wan_link_local(&setting)) goto send_pkt;

            break;
        default:
            break;
    }

    if(IPV6_WAN_LINK_LOCAL_ONLY != i)
    {
        if(setting.enable_auto_assignment)
        {
            api_set_ipv6_wan_auto_config_type(&setting);
        }
    }

    APPAGENT_SYSTEM("uci commit");
    APPAGENT_SYSTEM("ubus call network reload &");

    return_str = OK_STR;

send_pkt:
    send_simple_response(pkt, return_str);

    return 0;
}
#endif
