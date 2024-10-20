#include "WTPBoardApiNetwork.h"
#include "WTPBoardApiCommon.h"
#include <ctype.h>
#include <sysWlan.h>
#include <api_tokens.h>
#include <variable/variable.h>
#include <variable/api_lan.h>
#include <variable/api_dhcp.h>
#include <gconfig.h>

#ifdef DMALLOC
#include "../dmalloc-5.5.0/dmalloc.h"
#endif

int CWWTPBoardGetMaxLanPortNum()
{
#ifdef VLAN_PORT_MAX
    return VLAN_PORT_MAX;
#else
    return 0;
#endif
}

CWBool CWWTPBoardGetIPv4Cfg(CWIPv4Cfg *cfg)
{
    char val[15+1];

    memset(val, 0, sizeof(val));

    if(api_get_string_option(NETWORK_LAN_PROTO_OPTION, val, sizeof(val)))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if(!strcmp(val, "dhcp"))
    {
        cfg->address = 0;
        cfg->mask = 0;
        cfg->gateway = 0;
    }
    else /*if(!strcmp(val, "static"))*/
    {
        memset(val, 0, sizeof(val));

        api_get_string_option(NETWORK_LAN_IPADDR_OPTION, val, sizeof(val));

        cfg->address = inet_addr(val);

        memset(val, 0, sizeof(val));

        api_get_string_option(NETWORK_LAN_NETMASK_OPTION, val, sizeof(val));

        cfg->mask = inet_addr(val);

        memset(val, 0, sizeof(val));

        api_get_string_option(NETWORK_LAN_GATEWAY_OPTION, val, sizeof(val));

        cfg->gateway = inet_addr(val);

    }

    CWDebugLog("%s %u.%u.%u.%u/%u.%u.%u.%u/%u.%u.%u.%u", __FUNCTION__,
               CW_IPV4_PRINT_LIST(cfg->address),
               CW_IPV4_PRINT_LIST(cfg->mask),
               CW_IPV4_PRINT_LIST(cfg->gateway)
              );

    return CW_TRUE;
}

CWBool CWWTPBoardSetIPv4Cfg(CWIPv4Cfg *cfg)
{
    char val[15+1];
    char dns[31+1];
    char ipaddr[15+1], mask[15+1], gateway[15+1];

    memset(dns, 0, sizeof(dns));

    strcpy(dns, "/tmp/resolv.conf.auto");

    CWDebugLog("%s %u.%u.%u.%u/%u.%u.%u.%u/%u.%u.%u.%u", __FUNCTION__,
               CW_IPV4_PRINT_LIST(cfg->address),
               CW_IPV4_PRINT_LIST(cfg->mask),
               CW_IPV4_PRINT_LIST(cfg->gateway)
              );

    if((cfg->address == 0)/* && (cfg->mask == 0) && (cfg->gateway == 0)*/)
    {
        memset(val, 0, sizeof(val));

        strncpy(val, "dhcp", sizeof(val));

#if SUPPORT_DHCP6C_SETTING
        api_set_integer_option(DHCP6C_BASIC_ENABLED_OPTION,1);
#endif

    }
    else
    {
        memset(val, 0, sizeof(val));

        strcpy(val, "static");

#if SUPPORT_DHCP6C_SETTING  
        api_set_integer_option(DHCP6C_BASIC_ENABLED_OPTION,0);
#endif        

        snprintf(ipaddr, sizeof(ipaddr), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(cfg->address));
        api_set_string_option(NETWORK_LAN_IPADDR_OPTION, ipaddr, sizeof(ipaddr));

        snprintf(mask, sizeof(mask), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(cfg->mask));
        api_set_string_option(NETWORK_LAN_NETMASK_OPTION, mask, sizeof(mask));

        snprintf(gateway, sizeof(gateway), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(cfg->gateway));
        api_set_string_option(NETWORK_LAN_GATEWAY_OPTION, gateway, sizeof(gateway));
    }

    api_set_string_option(NETWORK_LAN_PROTO_OPTION, val, sizeof(val));
    api_set_string_option(DHCP_DNSMASQ_RESOLVFILE_OPTION, dns, sizeof(dns));

    return CW_TRUE;
}

CWBool CWWTPBoardGetDns1Cfg(unsigned int *addr)
{
    char val[64] = {0};
    char ip[32] = {0};

    if(api_get_string_option(NETWORK_LAN_DNS_OPTION, val, sizeof(val)))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, val);

    *addr = 0;

    if(val[0] != '\0')
    {
        if ( sscanf(val, "%s", ip) == 1 )
        {
            CWDebugLog("%s %d %s", __FUNCTION__, __LINE__, ip);
            *addr = inet_addr(ip);
        }
    }

    CWDebugLog("%s %d Dns1:[%u.%u.%u.%u]", __FUNCTION__, __LINE__, CW_IPV4_PRINT_LIST(*addr));

    return CW_TRUE;
}

CWBool CWWTPBoardSetDns1Cfg(unsigned int addr)
{
    unsigned int dns2_addr;
    char dns[31+1];

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

    if(!CWWTPBoardGetDns2Cfg(&dns2_addr))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    snprintf(dns, sizeof(dns), "%u.%u.%u.%u %u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr), CW_IPV4_PRINT_LIST(dns2_addr));
    api_set_string_option(NETWORK_LAN_DNS_OPTION, dns, sizeof(dns));

    return CW_TRUE;
}

CWBool CWWTPBoardGetDns2Cfg(unsigned int *addr)
{
    char *c;
    char val[64];

    if(api_get_string_option(NETWORK_LAN_DNS_OPTION, val, sizeof(val)))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if(val[0] != '\0')
    {
        c = val;
        while(*c != ' ' && *c != '\0')
        {
            c++;
        }
        if(*c == '\0')
        {
            *addr = 0;
        }
        else
        {
            c++;
            *addr = inet_addr(c);
        }
    }
    else
    {
        *addr = 0;
    }

    CWDebugLog("%s %d Dns2:[%u.%u.%u.%u]", __FUNCTION__, __LINE__, CW_IPV4_PRINT_LIST(*addr));

    return CW_TRUE;
}

CWBool CWWTPBoardSetDns2Cfg(unsigned int addr)
{
    unsigned int dns1_addr;
    char dns[31+1];

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

    if(!CWWTPBoardGetDns1Cfg(&dns1_addr))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    snprintf(dns, sizeof(dns), "%u.%u.%u.%u %u.%u.%u.%u", CW_IPV4_PRINT_LIST(dns1_addr), CW_IPV4_PRINT_LIST(addr));
    api_set_string_option(NETWORK_LAN_DNS_OPTION, dns, sizeof(dns));

    return CW_TRUE;
}

CWBool CWWTPBoardGetCurrentIPv4(CWIPv4Cfg *cfg)
{
    char ipaddr[15+1], mask[15+1], gateway[15+1];

    memset(ipaddr, 0, sizeof(ipaddr));

    if(!sys_get_lan_ipaddr(ipaddr, sizeof(ipaddr)))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if(ipaddr[0] != '\0')
    {
        cfg->address = inet_addr(ipaddr);
    }
    else
    {
        cfg->address = 0;
    }
    
    memset(mask, 0, sizeof(mask));

    if(!sys_get_lan_netmask(mask, sizeof(mask)))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if(mask[0] != '\0')
    {
        cfg->mask = inet_addr(mask);
    }
    else
    {
        cfg->mask = 0;
    }

    memset(gateway, 0, sizeof(gateway));

    sys_get_lan_gateway(gateway, sizeof(gateway));

    if(gateway[0] != '\0')
    {
        cfg->gateway = inet_addr(gateway);
    }
    else
    {
        cfg->gateway = 0;
    }

    CWDebugLog("%s %u.%u.%u.%u %u.%u.%u.%u %u.%u.%u.%u", __FUNCTION__,
               CW_IPV4_PRINT_LIST(cfg->address), CW_IPV4_PRINT_LIST(cfg->mask), CW_IPV4_PRINT_LIST(cfg->gateway));

    return CW_TRUE;
}

CWBool CWWTPBoardGetCurrentDns(unsigned int *dns1, unsigned int *dns2)
{
    char val[15+1];

    memset(val, 0 ,sizeof(val));

    sys_get_lan_dns(1, val, sizeof(val));

    if(val[0] != '\0')
    {
        *dns1 = inet_addr(val);
    }
    else
    {
        *dns1 = 0;
    }

    memset(val, 0 ,sizeof(val));

    sys_get_lan_dns(2, val, sizeof(val));

    if(val[0] != '\0')
    {
        *dns2 = inet_addr(val);
    }
    else
    {
        *dns2 = 0;
    }

    CWDebugLog("%s %u.%u.%u.%u %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(*dns1), CW_IPV4_PRINT_LIST(*dns2));

    return CW_TRUE;
}

CWBool CWWTPBoardGetLanPortEnableCfg(int port, int *enable)
{
    char *val=NULL;

#ifdef VLAN_PORT_MAX
    if((port > VLAN_PORT_MAX) || (port < (VLAN_PORT_MIN -1)))
    {
        CWDebugLog("no such lan port setting");
        return CW_TRUE;
    }
#else
	return CW_TRUE;
#endif
#if SUPPORT_VLAN_PORT_LAN2_TO_LAN4
    port++;
#endif
    if(!(val = CWCreateStringByUci("network.lan_index%d.enable", port)))
        *enable =0;
    else
        *enable = atoi(val);

    CW_FREE_OBJECT(val);

    CWDebugLog("%s %d [%d]", __FUNCTION__, __LINE__, *enable);

    return CW_TRUE;
}

CWBool CWWTPBoardSetLanPortEnableCfg(int port, int enable)
{
    char option[64]={0};

#ifdef VLAN_PORT_MAX
#if SUPPORT_VLAN_PORT_ONLY_LAN2
    if(port != 1)
    {
        CWDebugLog("only support lan2 port setting");
        return CW_TRUE;
    }
#endif
    if((port > VLAN_PORT_MAX) || (port < (VLAN_PORT_MIN -1)))
    {
        CWDebugLog("no such lan port setting");
        return CW_TRUE;
    }
#else
	return CW_TRUE;
#endif

#if SUPPORT_VLAN_PORT_LAN2_TO_LAN4
    port++;
#endif

    snprintf(option, sizeof(option), "network.lan_index%d.enable",port);
    api_set_integer_option(option,enable);

    CWDebugLog("%s %d %s:[%d]", __FUNCTION__, __LINE__,option,enable);

    return CW_TRUE;
}

CWBool CWWTPBoardGetLanPortEnableVLANCfg(int port, int *mode)
{
    char *val=NULL;

#ifdef VLAN_PORT_MAX
    if((port > VLAN_PORT_MAX) || (port < (VLAN_PORT_MIN -1)))
    {
        CWDebugLog("no such lan port setting");
        return CW_TRUE;
    }
#else
	return CW_TRUE;
#endif

#if SUPPORT_VLAN_PORT_LAN2_TO_LAN4
    port++;
#endif

    if(!(val = CWCreateStringByUci("network.lan_index%d.enableVLAN",port)))
        *mode =0;
    else
        *mode = atoi(val);

    CW_FREE_OBJECT(val);

    CWDebugLog("%s %d [%d]", __FUNCTION__, __LINE__, *mode);

    return CW_TRUE;
}

CWBool CWWTPBoardSetLanPortEnableVLANCfg(int port, int mode)
{
    char option[64]={0};

#ifdef VLAN_PORT_MAX
#if SUPPORT_VLAN_PORT_ONLY_LAN2
    if(port != 1)
    {
        CWDebugLog("only support lan2 port setting");
        return CW_TRUE;
    }
#endif
    if((port > VLAN_PORT_MAX) || (port < (VLAN_PORT_MIN -1)))
    {
        CWDebugLog("no such lan port setting");
        return CW_TRUE;
    }
#else
        return CW_TRUE;
#endif

#if SUPPORT_VLAN_PORT_LAN2_TO_LAN4
    port++;
#endif

    snprintf(option, sizeof(option), "network.lan_index%d.enableVLAN",port);
    api_set_integer_option(option,mode);

    CWDebugLog("%s %d %s:[%d]", __FUNCTION__, __LINE__,option,mode);

    return CW_TRUE;
}

/*begin,20160503,andy,merge capwap v4*/

CWBool CWWTPBoardGetLanPortVlanModeCfg(int port, int *mode)
{
    char *val=NULL;

#ifdef VLAN_PORT_MAX
    if ((port > VLAN_PORT_MAX) || (port < (VLAN_PORT_MIN -1)))
        return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "no such lan port setting");
#else
    return CW_TRUE;
#endif

#if SUPPORT_VLAN_PORT_LAN2_TO_LAN4
    port++;
#endif

    if (!(val = CWCreateStringByUci("network.lan_index%d.enableVLAN",port)))
        *mode = 0;
    else
        *mode = atoi(val);

    CW_FREE_OBJECT(val);

    CWDebugLog("%s %d port_%d:[%d]", __FUNCTION__, __LINE__, port, *mode);

    return CW_TRUE;
}

CWBool CWWTPBoardSetLanPortVlanModeCfg(int port, int mode)
{
    char option[64]={0};

    CWDebugLog("%s %d port_%d:[%d]", __FUNCTION__, __LINE__,port,mode);
#ifdef VLAN_PORT_MAX
#if SUPPORT_VLAN_PORT_ONLY_LAN2
    if(port != 1)
    {
        CWDebugLog("only support lan2 port setting");
        return CW_TRUE;
    }
#endif
    if ((port > VLAN_PORT_MAX) || (port < (VLAN_PORT_MIN -1)))
        return CWErrorRaise(CW_ERROR_NOT_SUPPORTED, "no such lan port setting");
#else
    return CW_TRUE;
#endif

#if SUPPORT_VLAN_PORT_LAN2_TO_LAN4
    port++;
#endif

    if (mode < 0 || mode > 2)
        return CWErrorRaise(CW_ERROR_WRONG_ARG, "Invalid VLAN MODE");

    snprintf(option, sizeof(option), "network.lan_index%d.enableVLAN", port);
    api_set_integer_option(option, mode);

    return CW_TRUE;
}
/*end,20160503,andy,merge capwap v4*/

CWBool CWWTPBoardGetLanPortVlanIdCfg(int port, int *vid)
{
    char *val=NULL;
#ifdef VLAN_PORT_MAX
    if((port > VLAN_PORT_MAX) || (port < (VLAN_PORT_MIN -1)))
    {
        CWDebugLog("no such lan port setting");
        return CW_TRUE;
    }
#else
        return CW_TRUE;
#endif

#if SUPPORT_VLAN_PORT_LAN2_TO_LAN4
    port++;
#endif

    if(!(val = CWCreateStringByUci("network.lan_index%d.vlanId",port)))
        *vid =0;
    else
        *vid = atoi(val);

    CW_FREE_OBJECT(val);

    CWDebugLog("%s %d [%d]", __FUNCTION__, __LINE__, *vid);

    return CW_TRUE;
}

CWBool CWWTPBoardSetLanPortVlanIdCfg(int port, int vid)
{
    char option[64]={0};

#ifdef VLAN_PORT_MAX
#if SUPPORT_VLAN_PORT_ONLY_LAN2
    if(port != 1)
    {
        CWDebugLog("only support lan2 port setting");
        return CW_TRUE;
    }
#endif
    if((port > VLAN_PORT_MAX) || (port < (VLAN_PORT_MIN -1)))
    {
        CWDebugLog("no such lan port setting");
        return CW_TRUE;
    }
#else
        return CW_TRUE;
#endif

    if(vid < 0 || vid > 4095)
    {
        CWDebugLog("Invalid VLAN ID");
        return CW_TRUE;
    }

#if SUPPORT_VLAN_PORT_LAN2_TO_LAN4
    port++;
#endif

    snprintf(option, sizeof(option), "network.lan_index%d.vlanId",port);
    api_set_integer_option(option,vid);

    CWDebugLog("%s %d %s:[%d]", __FUNCTION__, __LINE__,option,vid);

    return CW_TRUE;
}

CWBool CWWTPBoardSetManageVlanCfg(int vlan)
{
    if (vlan == 0)
    {
        api_set_management_vlan_enable_option(NETWORK_SYSTEM_WLANVLANENABLE_OPTION, 0);
    }
    else
    {
        if (!api_set_management_vlan_enable_option(NETWORK_SYSTEM_WLANVLANENABLE_OPTION, 1))
        {
            if (api_set_management_vlan_id_option(NETWORK_SYSTEM_MANAGEMENTVLANID_OPTION, vlan))
            {
                CWDebugLog("Set configuration fail");
                return CW_TRUE;
            }
        }
    }

    return CW_TRUE;
}

CWBool CWWTPBoardGetManageVlanCfg(int *vlan)
{
    int val, enable;

    if(api_get_management_vlan_id_option(NETWORK_SYSTEM_MANAGEMENTVLANID_OPTION, &val))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    *vlan = val;

    api_get_management_vlan_enable_option(NETWORK_SYSTEM_WLANVLANENABLE_OPTION, &enable);

    if(enable == 0)
    {
        *vlan = 0;
    }

    CWDebugLog("%s %d ManageVlan:[%d]", __FUNCTION__, __LINE__, *vlan);

    return CW_TRUE;
}

CWBool CWWTPBoardGetGuestNetworkAddressCfg(unsigned int *addr)
{
    char val[15+1]={0};

    if (api_get_string_option(NETWORK_GUEST_IPADDR_OPTION, val, sizeof(val)))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    CWDebugLog("%s val = %s", __FUNCTION__, val);

    if (val[0] == '\0')
    {
        *addr = 0;
    }
    else
    {
        *addr = inet_addr(val);
    }

    CWDebugLog("%s addr = %lu", __FUNCTION__, *addr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetGuestNetworkAddressCfg(unsigned int addr)
{
    char ipaddr[15+1]={0};

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

    snprintf(ipaddr, sizeof(ipaddr), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
    api_set_string_option(NETWORK_GUEST_IPADDR_OPTION, ipaddr, sizeof(ipaddr));

    return CW_TRUE;
}

CWBool CWWTPBoardGetGuestNetworkMaskCfg(unsigned int *addr)
{
    char val[15+1]={0};

    if (api_get_string_option(NETWORK_GUEST_NETMASK_OPTION, val, sizeof(val)))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if (val[0] == '\0')
    {
        *addr = 0;
    }
    else
    {
        *addr = inet_addr(val);
    }

    CWDebugLog("%s addr = %lu", __FUNCTION__, *addr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetGuestNetworkMaskCfg(unsigned int addr)
{
    char ipaddr[15+1];

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

    snprintf(ipaddr, sizeof(ipaddr), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
    api_set_string_option(NETWORK_GUEST_NETMASK_OPTION, ipaddr, sizeof(ipaddr));

    return CW_TRUE;
}

CWBool CWWTPBoardGetGuestNetworkDhcpStartCfg(unsigned int *addr)
{
    char val[15+1];

    if (api_get_string_option(DHCP_GUEST_START_OPTION, val, sizeof(val)))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if (val[0] == '\0')
    {
        *addr = 0;
    }
    else
    {
        *addr = inet_addr(val);
    }

    CWDebugLog("%s addr = %lu", __FUNCTION__, *addr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetGuestNetworkDhcpStartCfg(unsigned int addr)
{
    char ipaddr[15+1]={0};

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

    if(addr != 0)
    {
        snprintf(ipaddr, sizeof(ipaddr), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));
    }

    {
        char start_ip[15+1]={0}, end_ip[15+1]={0};
        char netmask[15+1]="255.255.0.0";
        int limit, new_limit;

        // get start ip
        api_get_string_option(DHCP_GUEST_START_OPTION, start_ip, sizeof(start_ip));
        // get netmask
        api_get_string_option(NETWORK_GUEST_NETMASK_OPTION, netmask, sizeof(netmask));
        // get old limit
        api_get_integer_option(DHCP_GUEST_LIMIT_OPTION, &limit);
        // get old end ip
        api_get_dhcp_end_addr(start_ip, netmask, limit, end_ip, sizeof(end_ip));

        // use new start ip -> get new limit
        // check new start ip & old end ip vaild
        if(!api_get_dhcp_limit(end_ip, ipaddr, netmask, &new_limit))
        {
            // set new limit
            if(limit != new_limit)
            {
                api_set_integer_option(DHCP_GUEST_LIMIT_OPTION, new_limit);
            }
        }
    }

    api_set_string_option(DHCP_GUEST_START_OPTION, ipaddr, sizeof(ipaddr));

    return CW_TRUE;
}

CWBool CWWTPBoardGetGuestNetworkDhcpEndCfg(unsigned int *addr)
{
    char start_ip[15+1]={0}, end_ip[15+1]={0};
    char netmask[15+1]="255.255.0.0";
    int limit;

    if (api_get_string_option(DHCP_GUEST_START_OPTION, start_ip, sizeof(start_ip)))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    // Don't use current netmask, maybe netmask config just be set.
    // You will get wrong limit.
    // if (api_get_string_option(NETWORK_GUEST_NETMASK_OPTION, netmask, sizeof(netmask)))
    // {
    //     return CW_FALSE;
    // }

    if (api_get_integer_option(DHCP_GUEST_LIMIT_OPTION, &limit))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    api_get_dhcp_end_addr(start_ip, netmask, limit, end_ip, sizeof(end_ip));

    *addr = inet_addr(end_ip);

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(*addr));

    return CW_TRUE;
}

CWBool CWWTPBoardSetGuestNetworkDhcpEndCfg(unsigned int addr)
{
    char start_ip[15+1]={0}, end_ip[15+1]={0}, netmask[15+1]={0};
    int limit;

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

    snprintf(end_ip, sizeof(end_ip), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));

    if (api_get_string_option(DHCP_GUEST_START_OPTION, start_ip, sizeof(start_ip)))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    if (api_get_string_option(NETWORK_GUEST_NETMASK_OPTION, netmask, sizeof(netmask)))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    api_get_dhcp_limit(end_ip, start_ip, netmask, &limit);

    api_set_integer_option(DHCP_GUEST_LIMIT_OPTION, limit);

    return CW_TRUE;
}

CWBool CWWTPBoardGetGuestNetworkWinsServerCfg(unsigned int *addr)
{
    char val[15+1];

    if(api_get_wifi_guest_wins_server_option(DHCP_GUEST_DHCP_OPTION_OPTION, val, sizeof(val)))
    {
        CWDebugLog("Get configuration fail");
        return CW_TRUE;
    }

    *addr = inet_addr(val);

    CWDebugLog("%s addr = %lu", __FUNCTION__, *addr);

    return CW_TRUE;
}

CWBool CWWTPBoardSetGuestNetworkWinsServerCfg(unsigned int addr)
{
    char ipaddr[15+1];

    CWDebugLog("%s %u.%u.%u.%u", __FUNCTION__, CW_IPV4_PRINT_LIST(addr));

    snprintf(ipaddr, sizeof(ipaddr), "%u.%u.%u.%u", CW_IPV4_PRINT_LIST(addr));

    api_set_wifi_guest_wins_server_option(DHCP_GUEST_DHCP_OPTION_OPTION, ipaddr, sizeof(ipaddr));

    return CW_TRUE;
}

CWBool CWWTPBoardGetAcAddressWithDhcpOption(char **acAddr)
{
    char dhcpfile[]="/tmp/dhcp_option";
    int acAddrLen = 0;

    if(!CWCreateStringByFile(dhcpfile,acAddr))
    {
        CWDebugLog("Failed :Get ac address By dhcp option43");
        return CW_FALSE;
    }

    acAddrLen = strlen(*acAddr);

    if((*acAddr)[acAddrLen-1] == '\n')
    {
        (*acAddr)[acAddrLen-1] = '\0';
    }

    CWDebugLog("%s dhcp addr=%s#", __FUNCTION__, *acAddr);

    return CW_TRUE;
}
