#include <api_common.h>
#include <api_wireless.h>
#include <variable.h>
#include <api_lan.h>
#include <api_mesh.h>
#include <wireless_tokens.h>
#include <api_response.h>
#include <api_sys.h>
//#include <integer_check.h>
#include <json_object.h>
#include <json_tokener.h>
#include <json_common.h>
#include <api_guest.h>
#include <json_wireless.h>
#include <math.h>
#include <api_vpn.h>

#define ACTION_POST  1
#define ACTION_GET   2
struct device_info
{
    char deviceType[8];
    int  rssi;
    char mac[32];
    char uid[16];
    char deviceName[128];
    char locationName[128];
    char meshRole[16];
    char meshController[16];
    int  tq;
    char lANIPAddress[32];
    char LANMacAddress[32];
    int  wifi_status;
    int  dev_connect_number;
    int  rootHopCount;
    int  led_status;
    char FullFwVersion[32];
    char WANMacAddress[32];
    int  NextHopRssi;
    char NextHopMac[32];
    /* mesh_group_info */
    int  MeshChannel;
    int  Threshold;
    bool Enable;
    char MacAddr[128][32];
    char LearnRssi[128][4];
    char LinkFlags[128][4];
    char LinkMacAddr[64];
    char TqVal[32];
    char Rssi[32];
    char TxRate[128][4];
    char RxRate[128][4];
    char GwMacAddr[64];
    /* mesh_group_info end*/
    MESH_GUEST_CLIENT_INFO_T gClientInfo;
};

#define TMP_CHANNELS_FILE "/tmp/channels_wifi"



static int timeout = 30;
static bool simple_output = false;

int json_patch_sys_info(ResponseEntry *rep, char *query_str)
{
    int ret = 0, i = 0, length = T_NUM_OF_ELEMENTS(api_timezone_table), systime_manual_en = 0, year = 0, month = 0, day = 0, hour = 0, minute = 0, daylight_saving_start_week = 0, daylight_saving_end_week = 0;
    bool  daylight_saving_en = false, led_enable = false, time_zone_auto_detect = false, pw_changed = false, pw_ignore = false, auto_daylight_saving = false;
    char *sysName=NULL, *timezone=NULL, tmp[100]={0}, *time_mode=NULL, *date=NULL, *time=NULL, *ntp_server=NULL, *pch, *modelName;
    char *jobj_time_string=NULL, *jobj_manual_set_string=NULL, *jobj_auto_set_string=NULL, *jobj_daylight_saving_string=NULL;
    char *daylight_saving_start_month=NULL, *daylight_saving_start_day=NULL, *daylight_saving_start_time=NULL, *daylight_saving_end_month=NULL, *daylight_saving_end_day=NULL, *daylight_saving_end_time=NULL;
    struct json_object *jobj = NULL, *jobj_time = NULL, *jobj_manual_set = NULL, *jobj_auto_set = NULL, *jobj_daylight_saving = NULL;

    char month_ary[12][4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    char weekday[7][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    char buf[256] = {0};
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            /* device_name */
            if (json_object_object_get(jobj, "device_name") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "device_name", &sysName);
#if SUPPORT_NETGEAR_FUNCTION
                if (!api_check_string_length(sysName, 1, 15) || !regxMatch("^.*[a-zA-Z]+.*$", sysName) ||
                    regxMatch("^[\\-]", sysName) || regxMatch("[\\-]$", sysName) ||
                    !regxMatch("^([a-zA-Z0-9\\-]+)$", sysName))
                {
                    debug_print("\n%s[%d]===> netgear ap name fail \n", __FUNCTION__, __LINE__);
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DEVICE NAME");
                }
#endif
                if (api_set_system_hostname_option(SYSTEM_SYSTEM_SYSTEMNAME_OPTION, sysName, sizeof(sysName)))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DEVICE NAME");
                api_set_system_hostname_option(DHCP_DNSMASQ_DOMAIN_OPTION, sysName, sizeof(sysName));
                api_set_system_hostname_option(NETWORK_LAN_HOSTNAME_OPTION, sysName, sizeof(sysName));
                api_set_system_hostname_option(SNMPD_SYSTEM_SYSNAME_OPTION, sysName, sizeof(sysName));
            }
            if (json_object_object_get(jobj, "model") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "model", &modelName);
                if (api_set_string_option(SYSPRODUCTINFO_MODEL_MODELNAME_OPTION, modelName, sizeof(modelName)))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MODEL");
            }
            if (json_object_object_get(jobj, "time") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "time", &jobj_time_string);

                if((jobj_time = jsonTokenerParseFromStack(rep, jobj_time_string)))
                {
                    if (json_object_object_get(jobj_time, "mode") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, jobj_time, "mode", &time_mode);
                        if ( strcasecmp(time_mode, "Manual") == 0 )
                        {
                            api_set_system_ntp_enable_server_option(SYSTEM_NTP_ENABLE_SERVER_OPTION, 0);
                            systime_manual_en = 1;
                        }
                        else
                        {
                            api_set_system_ntp_enable_server_option(SYSTEM_NTP_ENABLE_SERVER_OPTION, 1);
                            systime_manual_en = 0;
                        }
                        /* time mode */
                        if (api_set_integer_option(SYSTIME_MANUAL_ENABLE_OPTION, systime_manual_en))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME MODE");
                    }

                    if (json_object_object_get(jobj_time, "Manual_set") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, jobj_time, "Manual_set", &jobj_manual_set_string);

                        if((jobj_manual_set = jsonTokenerParseFromStack(rep, jobj_manual_set_string)))
                        {
                            if (json_object_object_get(jobj_manual_set, "date") != NULL)
                            {
                                senao_json_object_get_and_create_string(rep, jobj_manual_set, "date", &date);
                                if ( sscanf(date, "%d/%d/%d", &year, &month, &day) != 3 )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DATE");
                                ret = API_RC_SUCCESS ;
                                ret |= api_set_integer_option(SYSTIME_YEAR_ENABLE_OPTION, year);
                                ret |= api_set_integer_option(SYSTIME_MONTH_ENABLE_OPTION, month);
                                ret |= api_set_integer_option(SYSTIME_DAY_ENABLE_OPTION, day);

                                if ( ret != API_RC_SUCCESS )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DATE");
                            }
                            if (json_object_object_get(jobj_manual_set, "time") != NULL)
                            {
                                senao_json_object_get_and_create_string(rep, jobj_manual_set, "time", &time);
                                if ( sscanf(time, "%d:%d", &hour, &minute) != 2 )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME");
                                ret = API_RC_SUCCESS ;
                                ret |= api_set_integer_option(SYSTIME_HOUR_ENABLE_OPTION, hour);
                                ret |= api_set_integer_option(SYSTIME_MINUTE_OPTION, minute);

                                if ( ret != API_RC_SUCCESS )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME");
                            }
                        }
                    }

                    if (json_object_object_get(jobj_time, "Auto_set") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, jobj_time, "Auto_set", &jobj_auto_set_string);

                        if((jobj_auto_set = jsonTokenerParseFromStack(rep, jobj_auto_set_string)))
                        {
                            if (json_object_object_get(jobj_auto_set, "ntp_server") != NULL)
                            {
                                senao_json_object_get_and_create_string(rep, jobj_auto_set, "ntp_server", &ntp_server);

                                if ( api_set_system_ntp_server_list(SYSTEM_NTP_SERVER_LIST, ntp_server, sizeof(ntp_server)) )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NTP SERVER");
                            }
                        }
                    }

                    /* time_zone */
                    if (json_object_object_get(jobj_time, "time_zone") != NULL)
                    {
                        senao_json_object_get_boolean(jobj_time, "auto_daylight_saving", &auto_daylight_saving );
                        api_set_integer_option(SYSTEM_SYSTEM_USE_LOCATIOIN_OPTION, auto_daylight_saving?1:0);//default is false

                        senao_json_object_get_and_create_string(rep, jobj_time, "time_zone", &timezone);

                        if ( auto_daylight_saving == true)
                        {
                            ret |= api_set_string_option(SYSTEM_SYSTEM_ZONENAME_OPTION, timezone, sizeof(timezone));
                            if ( ret != API_RC_SUCCESS )
                                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME ZONE");
                        }
                        else
                        {
                            for( i=0 ; i < length; i++)
                            {
                                if (strcasecmp(api_timezone_table[i].zonename, timezone) == 0)
                                {
                                    ret = API_RC_SUCCESS ;
                                    ret |= api_set_string_option(SYSTEM_SYSTEM_ZONENAME_OPTION, api_timezone_table[i].zonename, sizeof(api_timezone_table[i].zonename));
                                    ret |= api_set_string_option(SYSTEM_SYSTEM_TIMEZONE_OPTION, api_timezone_table[i].timezone, sizeof(api_timezone_table[i].timezone));
                                    ret |= api_set_integer_option(SYSTEM_SYSTEM_ZONENUMBER_OPTION, i);

                                    if ( ret != API_RC_SUCCESS )
                                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME ZONE");

                                    break;
                                }
                            }
                            if ( i == length )
                                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME ZONE");
                        }
                    }

                    /* time_zone auto detect */
                    // if (json_object_object_get(jobj_time, "time_zone_auto_detect") != NULL)
                    // {
                    //     senao_json_object_get_boolean(jobj_time, "time_zone_auto_detect", &time_zone_auto_detect);
                        api_set_integer_option("system.@system[0].zone_auto_detect", time_zone_auto_detect?1:0);//default is false

                    //     if ( time_zone_auto_detect == true)
                    //     {
                    //         api_set_string_option(SYSTEM_SYSTEM_ZONENAME_OPTION, "auto", 4);
                    //     }
                    // }

                    /* daylight_saving */
                    if (json_object_object_get(jobj_time, "daylight_saving") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, jobj_time, "daylight_saving", &jobj_daylight_saving_string);

                        if((jobj_daylight_saving = jsonTokenerParseFromStack(rep, jobj_daylight_saving_string)))
                        {
                            if (json_object_object_get(jobj_daylight_saving, "enable") != NULL)
                            {
                                senao_json_object_get_boolean(jobj_daylight_saving, "enable", &daylight_saving_en );
                                if (auto_daylight_saving == true)
                                    daylight_saving_en = false;

                                api_set_system_ntp_day_light_saving_enable_option(NTPCLIENT_DAYLIGHTSAVING_DAYLIGHTENABLE_OPTION, (daylight_saving_en)?1:0);

                            }
                            if (json_object_object_get(jobj_daylight_saving, "start_month") != NULL)
                            {
                                senao_json_object_get_and_create_string(rep, jobj_daylight_saving, "start_month", &daylight_saving_start_month);

                                for (i = 0; i < 12; i++)
                                {
                                    if ( strcasecmp(daylight_saving_start_month, month_ary[i] ) == 0 )
                                    {
                                        break;
                                    }
                                }
                                if ( api_set_system_ntp_day_light_saving_month_option(NTPCLIENT_DAYLIGHTSAVING_STARTMONTH_OPTION, i+1) != API_RC_SUCCESS )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING STARTMONTH");
                            }
                            if (json_object_object_get(jobj_daylight_saving, "start_week") != NULL)
                            {
                                senao_json_object_get_integer(jobj_daylight_saving, "start_week", &daylight_saving_start_week);

                                if ( api_set_system_ntp_day_light_saving_week_option(NTPCLIENT_DAYLIGHTSAVING_STARTWEEK_OPTION, daylight_saving_start_week) != API_RC_SUCCESS )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING STARTWEEK");
                            }
                            if (json_object_object_get(jobj_daylight_saving, "start_day") != NULL)
                            {
                                senao_json_object_get_and_create_string(rep, jobj_daylight_saving, "start_day", &daylight_saving_start_day);

                                for ( i = 0 ; i < 7 ; i++ )
                                {
                                    if ( strcasecmp(daylight_saving_start_day, weekday[i] ) == 0 )
                                    {
                                        break;
                                    }
                                }

                                if ( api_set_system_ntp_day_light_saving_day_option(NTPCLIENT_DAYLIGHTSAVING_STARTDAY_OPTION, i ) != API_RC_SUCCESS )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING STARTDAY");
                            }
                            if (json_object_object_get(jobj_daylight_saving, "start_time") != NULL)
                            {
                                senao_json_object_get_and_create_string(rep, jobj_daylight_saving, "start_time", &daylight_saving_start_time);

                                if ( sscanf(daylight_saving_start_time, "%d:", &hour) != 1 )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING STARTIME");

                                if ( api_set_system_ntp_day_light_saving_hour_option(NTPCLIENT_DAYLIGHTSAVING_STARTHOUR_OPTION, hour) != API_RC_SUCCESS )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING STARTTIME");

                            }
                            if (json_object_object_get(jobj_daylight_saving, "end_month") != NULL)
                            {
                                senao_json_object_get_and_create_string(rep, jobj_daylight_saving, "end_month", &daylight_saving_end_month);

                                for ( i = 0 ; i < 12 ; i++ )
                                {
                                    if ( strcasecmp(daylight_saving_end_month, month_ary[i] ) == 0 )
                                    {
                                        break;
                                    }
                                }

                                if ( api_set_system_ntp_day_light_saving_month_option(NTPCLIENT_DAYLIGHTSAVING_ENDMONTH_OPTION, i+1 ) != API_RC_SUCCESS )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING ENDMONTH");
                            }
                            if (json_object_object_get(jobj_daylight_saving, "end_week") != NULL)
                            {
                                senao_json_object_get_integer(jobj_daylight_saving, "end_week", &daylight_saving_end_week);

                                if ( api_set_system_ntp_day_light_saving_week_option(NTPCLIENT_DAYLIGHTSAVING_ENDWEEK_OPTION, daylight_saving_end_week) != API_RC_SUCCESS )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING ENDWEEK");
                            }
                            if (json_object_object_get(jobj_daylight_saving, "end_day") != NULL)
                            {
                                senao_json_object_get_and_create_string(rep, jobj_daylight_saving, "end_day", &daylight_saving_end_day);

                                for ( i = 0 ; i < 7 ; i++ )
                                {
                                    if ( strcasecmp(daylight_saving_end_day, weekday[i] ) == 0 )
                                    {
                                        break;
                                    }
                                }

                                if ( api_set_system_ntp_day_light_saving_day_option(NTPCLIENT_DAYLIGHTSAVING_ENDDAY_OPTION, i ) != API_RC_SUCCESS )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING ENDDAY");
                            }
                            if (json_object_object_get(jobj_daylight_saving, "end_time") != NULL)
                            {
                                senao_json_object_get_and_create_string(rep, jobj_daylight_saving, "end_time", &daylight_saving_end_time);
                                if ( sscanf(daylight_saving_end_time, "%d:", &hour) != 1 )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING ENDTIME");

                                if ( api_set_system_ntp_day_light_saving_hour_option(NTPCLIENT_DAYLIGHTSAVING_ENDHOUR_OPTION, hour) != API_RC_SUCCESS )
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING ENDTIME");
                            }
                        }
                    } // if (json_object_object_get(jobj_time, "daylight_saving") != NULL)
                }
            }

            /* led_enable */
            if (json_object_object_get(jobj, "led_enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "led_enable", &led_enable);

#if SUPPORT_LED_ONLY_STATUS
                api_set_integer_option(SYSTEM_SYSTEM_LED_STATUS_OPTION, led_enable?1:0);
#else
                api_set_integer_option(SYSTEM_POWER_LED_DEFAULT_OPTION, led_enable?0:1);
                api_set_integer_option(SYSTEM_LAN1_LED_DEFAULT_OPTION, led_enable?0:1);
                api_set_integer_option(SYSTEM_WIFI0_LED_DEFAULT_OPTION, led_enable?0:1);
                api_set_integer_option(SYSTEM_WIFI1_LED_DEFAULT_OPTION, led_enable?0:1);
                api_set_integer_option("system.mesh_led.default", led_enable?0:1);
#endif
            }
            if (json_object_object_get(jobj, "pw_changed") != NULL)
            {
                senao_json_object_get_boolean(jobj, "pw_changed", &pw_changed);
                api_set_integer_option("tmp_wizard.wizard.pw_changed", pw_changed?1:0);
            }
            if (json_object_object_get(jobj, "pw_ignore") != NULL)
            {
		senao_json_object_get_boolean(jobj, "pw_ignore", &pw_ignore);
                api_set_integer_option("tmp_wizard.wizard.pw_ignore", pw_ignore?1:0);
            }
        }
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}


int json_patch_ethernet(ResponseEntry *rep, char *query_str)
{
    char *mode=NULL, *p=NULL, *ip=NULL, *mask=NULL, *gateway=NULL, *pDns=NULL, *sDns=NULL, *ipv4_obj_str=NULL, *ipv6_obj_str=NULL;
    int mode_val = 0, vlanId = 0 ,prefix=0, linkloacl_val=0;
    bool linklocal_en=0;
    char *pDns6=NULL, *sDns6=NULL, *ip6_addr=NULL, *ip6_gateway=NULL, *static_mode=NULL;
    bool lacp_mode = 0, vlanEn = 0;
    struct json_object *jobj=NULL;
    struct json_object *ipv4_obj = NULL;
    struct json_object *ipv6_obj = NULL;
    struct json_object *static_obj = NULL;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if(jobj = jsonTokenerParseFromStack(rep, query_str))
        {
            if (json_object_object_get(jobj, "lacp_mode") != NULL)
            {
                senao_json_object_get_boolean(jobj, "lacp_mode", &lacp_mode);
#if SUPPORT_ETHERNET_BONDING
                /* lacp_mode */
                if (api_set_lan_lacp_option(lacp_mode))
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "LACP_MODE");
#else // set to no use config
                if (api_set_integer_option("network.lan.lacp_mode",lacp_mode))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "LACP_MODE");
#endif
            }
            if (json_object_object_get(jobj, "mgm_vlan_enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "mgm_vlan_enable", &vlanEn);
                /* vlan enable */
                if (api_set_integer_option(NETWORK_SYSTEM_WLANVLANENABLE_OPTION, vlanEn?1:0))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "VLAN ENABLE");                 
            }
            if (json_object_object_get(jobj, "mgm_vlan_id") != NULL)
            {
                senao_json_object_get_integer(jobj, "mgm_vlan_id", &vlanId);
                /* vlan id */
                if (api_set_integer_option(NETWORK_SYSTEM_MANAGEMENTVLANID_OPTION, vlanId))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "VLAN ID");
            }
            if (json_object_object_get(jobj, "mode") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "mode", &mode);
                /* mode */
                for ( p = mode; *p; ++p) *p = tolower(*p);
                mode_val = ((strcmp(mode,"static") == 0)?0:1); 
                if (api_set_lan_proto_option(NETWORK_LAN_PROTO_OPTION, mode_val)) 
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MODE");

#if SUPPORT_IPV6_SETTING
                api_set_integer_option(NETWORK_LAN_ACCEPT_RA_OPTION, mode_val);
#endif

#if SUPPORT_DHCP6C_SETTING
                api_set_integer_option(DHCP6C_BASIC_ENABLED_OPTION, mode_val); 
#endif
            }
            if (json_object_object_get(jobj, "ipv4") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "ipv4", &ipv4_obj_str);

                if((ipv4_obj = jsonTokenerParseFromStack(rep, ipv4_obj_str)))
                {
                    if (json_object_object_get(ipv4_obj, "ip") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, ipv4_obj, "ip", &ip);
                        /* ip */
                        if (api_set_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION, ip, strlen(ip)))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IP");
                    }
                    if (json_object_object_get(ipv4_obj, "mask") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, ipv4_obj, "mask", &mask);
                        /* mask */
                        if (api_set_lan_netmask_option(NETWORK_LAN_NETMASK_OPTION, mask, strlen(mask)))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MASK");
                    }
                    if (json_object_object_get(ipv4_obj, "gateway") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, ipv4_obj, "gateway", &gateway);
                        /* gateway */
                        if (api_set_lan_gateway_option(NETWORK_LAN_GATEWAY_OPTION, gateway, strlen(gateway)))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "GATEWAY");
                    }
                    if (json_object_object_get(ipv4_obj, "primary_dns") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, ipv4_obj, "primary_dns", &pDns);
                        /* primary_dns */
                        if (api_set_lan_dns_option(NETWORK_LAN_DNS_OPTION, 1, pDns, strlen(pDns)))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PRIMARY DNS");
                    }
                    if (json_object_object_get(ipv4_obj, "secondary_dns") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, ipv4_obj, "secondary_dns", &sDns);
                        /* secondary_dns */
                        if (api_set_lan_dns_option(NETWORK_LAN_DNS_OPTION, 2, sDns, strlen(sDns)))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SECONDARY DNS");
                    }
                }

            }
#if SUPPORT_IPV6_SETTING
            if (json_object_object_get(jobj, "ipv6") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "ipv6", &ipv6_obj_str);
                if((ipv6_obj = jsonTokenerParseFromStack(rep, ipv6_obj_str)))
                {
                    if (json_object_object_get(ipv6_obj, "linklocal") != NULL)
                    {
                        senao_json_object_get_boolean(ipv6_obj, "linklocal", &linklocal_en);

                        if (api_set_lan_ipv6_link_local_option(NETWORK_LAN_IPV6_LINK_LOCAL_OPTION, linklocal_en?1:0))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "LINKLOCAL");
                    }
                    if (json_object_object_get(ipv6_obj, "ip") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, ipv6_obj, "ip", &ip6_addr);
                        /* ipv6 addr */
                        if (api_set_lan_ipv6_addr_option(NETWORK_LAN_IPV6_IPADDR_OPTION, ip6_addr, strlen(ip6_addr)))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IPv6 ADDRESS");
                    }
                    if (json_object_object_get(ipv6_obj, "gateway") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, ipv6_obj, "gateway", &ip6_gateway);
                        /* ipv6 gateway */
                        if (api_set_lan_ipv6_gateway_option(NETWORK_LAN_IPV6_GATEWAY_OPTION, ip6_gateway, strlen(ip6_gateway)))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IPv6 GATEWAY");
                    }
                    if (json_object_object_get(ipv6_obj, "primary_dns") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, ipv6_obj, "primary_dns", &pDns6);
                        /* primary_dns v6 */
                        if (api_set_lan_dns_option(NETWORK_LAN_DNS_OPTION, 3, pDns6, strlen(pDns6)))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IPv6 PRIMARY DNS");
                    }
                    if (json_object_object_get(ipv6_obj, "secondary_dns") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, ipv6_obj, "secondary_dns", &sDns6);
                        /* secondary_dns v6 */
                        if (api_set_lan_dns_option(NETWORK_LAN_DNS_OPTION, 4, sDns6, strlen(sDns6)))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IPv6 SECONDARY DNS");
                    }
                    if (json_object_object_get(ipv6_obj, "prefix") != NULL)
                    {
                        senao_json_object_get_integer(ipv6_obj, "prefix", &prefix);
                        /* ipv6 prefix */
                        if (api_set_lan_ipv6_subnet_prefix_length_option(NETWORK_LAN_IPV6_IPADDR_OPTION, prefix))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IPv6 PREFIX");

                    }
                }
            }
#endif
        }
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_patch_net_spanning_tree(ResponseEntry *rep, char *query_str){

    int  hello_time = 0, max_age = 0, forward_delay = 0, priority = 0;
    bool enable = 0;
    struct json_object *jobj;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (json_object_object_get(jobj, "enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "enable", &enable);
                if (api_set_lan_stp_status_option(NETWORK_LAN_STP_STATUS_OPTION, enable))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ENABLE");
            }
            if (json_object_object_get(jobj, "hello_time") != NULL)
            {
                senao_json_object_get_integer(jobj, "hello_time",&(hello_time));
                if (api_set_lan_stp_hello_time_option(NETWORK_LAN_HELLO_TIME_OPTION, hello_time))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HELLO TIME");
            }
            if (json_object_object_get(jobj, "max_age") != NULL)
            {
                senao_json_object_get_integer(jobj, "max_age",&(max_age));
                if (api_set_lan_stp_max_age_option(NETWORK_LAN_MAX_AGE_OPTION, max_age))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MAX AGE");
            }
            if (json_object_object_get(jobj, "forward_delay") != NULL)
            {
                senao_json_object_get_integer(jobj, "forward_delay",&(forward_delay));
                if (api_set_lan_stp_forward_delay_option(NETWORK_LAN_FORWARD_DELAY_OPTION, forward_delay))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "FORWARD DELAY");
            }
            if (json_object_object_get(jobj, "priority") != NULL)
            {
                senao_json_object_get_integer(jobj, "priority",&(priority));
                if (api_set_lan_stp_priority_option(NETWORK_LAN_PRIORITY_OPTION, priority))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PRIORITY");
            }
        }
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_patch_net_proxy_settings(ResponseEntry *rep, char *query_str){

    char *ip=NULL, *username=NULL, *password=NULL;
    int port;
    bool is_auth_en = false;
    char proxy1_enable[10] = {0}, proxy2_enable[10] = {0};
    bool enable = false;
    char use_profile[200] = {0};
    char *exclude_subnet=NULL;
    char buf[32] = {0};
    int value_num = 0, i = 0;
    struct json_object *jobj;
    ResponseStatus *res = rep->res;
    struct json_object *http_obj = NULL;
    struct json_object *https_obj = NULL;
    char *http_obj_str=NULL, *https_obj_str=NULL;

    api_get_string_option(REDSOCKS2_ETHERNET_PROXY_USE_PROFILE, use_profile, sizeof (use_profile));

    sscanf(use_profile, "%s %s", proxy1_enable, proxy2_enable);

    if(NULL != query_str)
    {
        if(jobj = jsonTokenerParseFromStack(rep, query_str))
        {
            if (json_object_object_get(jobj, "https") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "https", &https_obj_str);

                if((https_obj = jsonTokenerParseFromStack(rep, https_obj_str)))
                {
                    if (json_object_object_get(https_obj, "enable") != NULL)
                    {
                        senao_json_object_get_boolean(https_obj, "enable", &enable);
                        if (enable == true)
                            strcpy(proxy1_enable, "proxy1");
                        else
                            strcpy(proxy1_enable, "none");
                    }
                    if (json_object_object_get(https_obj, "ip") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, https_obj, "ip", &ip);
                        if (api_set_redsocks_proxy_ip_https_url_option_by_sectionname(REDSOCKS2_PROXY1_IP, ip, sizeof(ip)))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IP");
                    }
                    if (json_object_object_get(https_obj, "port") != NULL)
                    {
                        senao_json_object_get_integer(https_obj, "port", &port);
                        if (api_set_redsocks_proxy_port_option_by_sectionname(REDSOCKS2_PROXY1_PORT, port))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PORT");
                    }
                    if (json_object_object_get(https_obj, "is_auth_en") != NULL)
                    {
                        senao_json_object_get_boolean(https_obj, "is_auth_en", &is_auth_en);
                        if (api_set_redsocks_proxy_authorization_option_by_sectionname(REDSOCKS2_PROXY1_AUTHORIZATION, is_auth_en))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IS_AUTH_EN");
                    }
                    if (json_object_object_get(https_obj, "username") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, https_obj, "username", &username);
                        if (api_set_redsocks_proxy_username_option_by_sectionname(REDSOCKS2_PROXY1_USERNAME, username, sizeof(username)))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "USERNAME");
                    }
                    if (json_object_object_get(https_obj, "password") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, https_obj, "password", &password);
                    if (api_set_redsocks_proxy_password_option_by_sectionname(REDSOCKS2_PROXY1_PASSWORD, password, sizeof(password)))
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSWORD");
                    }
                }
            }
            
            if (json_object_object_get(jobj, "http") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "http", &http_obj_str);

                if((http_obj = jsonTokenerParseFromStack(rep, http_obj_str)))
                {
                    if (json_object_object_get(http_obj, "enable") != NULL)
                    {
                        senao_json_object_get_boolean(http_obj, "enable", &enable);
                        if (enable == true)
                            strcpy(proxy2_enable, "proxy2");
                        else
                            strcpy(proxy2_enable, "none");
                    }
                    if (json_object_object_get(http_obj, "ip") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, http_obj, "ip", &ip);
                        if (api_set_redsocks_proxy_ip_http_url_option_by_sectionname(REDSOCKS2_PROXY2_IP, ip, sizeof(ip)))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IP");
                    }
                    if (json_object_object_get(http_obj, "port") != NULL)
                    {
                        senao_json_object_get_integer(http_obj, "port", &port);
                        if (api_set_redsocks_proxy_port_option_by_sectionname(REDSOCKS2_PROXY2_PORT, port))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PORT");
                    }
                    if (json_object_object_get(http_obj, "is_auth_en") != NULL)
                    {
                        senao_json_object_get_boolean(http_obj, "is_auth_en", &is_auth_en);
                        if (api_set_redsocks_proxy_authorization_option_by_sectionname(REDSOCKS2_PROXY2_AUTHORIZATION, is_auth_en))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IS_AUTH_EN");
                    }
                    if (json_object_object_get(http_obj, "username") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, http_obj, "username", &username);
                        if (api_set_redsocks_proxy_username_option_by_sectionname(REDSOCKS2_PROXY2_USERNAME, username, sizeof(username)))
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "USERNAME");
                    }
                    if (json_object_object_get(http_obj, "password") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, http_obj, "password", &password);
                    if (api_set_redsocks_proxy_password_option_by_sectionname(REDSOCKS2_PROXY2_PASSWORD, password, sizeof(password)))
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSWORD");
                    }
                }
            }
            api_delete_option(REDSOCKS2_ETHERNET_PROXY_USE_PROFILE, "");
            api_add_list(REDSOCKS2_ETHERNET_PROXY_USE_PROFILE, proxy1_enable, strlen(proxy1_enable));
            api_add_list(REDSOCKS2_ETHERNET_PROXY_USE_PROFILE, proxy2_enable, strlen(proxy2_enable));

            if (json_object_object_get(jobj, "exclude_subnet") != NULL)
            {
                api_delete_option(REDSOCKS2_ETHERNET_EXCLUDE_SUBNET, "");
                senao_json_object_get_and_create_string(rep, jobj, "exclude_subnet", &exclude_subnet);
                memset(buf,0,sizeof(buf));
                sys_interact(buf, sizeof(buf), "echo %s | awk '{print NF}'", exclude_subnet);
                value_num = atoi(buf);

                for ( i = 0 ; i < value_num && i < 2 ; i++ )
                {
                    memset(buf,0,sizeof(buf));
                    sys_interact(buf, sizeof(buf), "echo %s | awk '{print $%d}'", exclude_subnet, i+1);
                    if ( buf[strlen(buf)-1] == '\n' )
                        buf[strlen(buf)-1] = 0;
                    api_add_list(REDSOCKS2_ETHERNET_EXCLUDE_SUBNET, buf, strlen(buf));
                }
            }
        }
    }
}

int json_patch_net_iptable_rules(ResponseEntry *rep, char *query_str){

    char *use_profile=NULL;
    char *exclude_subnet=NULL;
    char buf[16] = {0};
    struct json_object *jobj;
    int value_num = 0, i = 0;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (json_object_object_get(jobj, "use_profile") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "use_profile", &use_profile);
                api_delete_option(REDSOCKS2_ETHERNET_PROXY_USE_PROFILE, "");
                memset(buf,0,sizeof(buf));
                sys_interact(buf, sizeof(buf), "echo %s | awk '{print NF}'", use_profile);
                value_num = atoi(buf);

                for ( i = 0 ; i < value_num && i < 2 ; i++ )
                {
                    memset(buf,0,sizeof(buf));
                    sys_interact(buf, sizeof(buf), "echo %s | awk '{print $%d}'", use_profile, i+1);
                    if ( buf[strlen(buf)-1] == '\n' )
                        buf[strlen(buf)-1] = 0;
                    api_add_list(REDSOCKS2_ETHERNET_PROXY_USE_PROFILE, buf, strlen(buf));
                }

            }
            if (json_object_object_get(jobj, "exclude_subnet") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "exclude_subnet", &exclude_subnet);
                api_delete_option(REDSOCKS2_ETHERNET_EXCLUDE_SUBNET, "");
                memset(buf,0,sizeof(buf));
                sys_interact(buf, sizeof(buf), "echo %s | awk '{print NF}'", exclude_subnet);
                value_num = atoi(buf);

                for ( i = 0 ; i < value_num && i < 2 ; i++ )
                {
                    memset(buf,0,sizeof(buf));
                    sys_interact(buf, sizeof(buf), "echo %s | awk '{print $%d}'", exclude_subnet, i+1);
                    if ( buf[strlen(buf)-1] == '\n' )
                        buf[strlen(buf)-1] = 0;
                    api_add_list(REDSOCKS2_ETHERNET_EXCLUDE_SUBNET, buf, strlen(buf));
                }
            }
        }
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}


int json_patch_wireless_radio(ResponseEntry *rep, char *query_str, char *radio)
{
    char *opmode = NULL, *hwmode = NULL, *htmode = NULL, *wifix, *p, section[32]={0}, *country = NULL;
    char opmode_opt[64]={0}, *enjet_info=NULL, *priority=NULL, *ap_role=NULL, *sta_role=NULL;
    int i = 0, radio_idx = 0, obey = 0, mode = 0, ch = 0, ext_ch = 0, txpower = 0, climit = 0, climitNum = 0, minbrate = 0, rssi = 0, ext_ch_set = 0, disabled = 0, ch_config_interval = 0;
    int tmp_status = 0, encryption = 0, hwmode_val = 0, regular = 0, country_code = 0, max_ctry_num = 0, htMode = 0, outdoor_distance;
    bool enable = 0, green_mode = 0, enjet_enable = 0, hwmode_deny = 0, ax_mode_disable = 0, ch_enable = 0, ch_dynamic_enable = 0;
    char *ch_list=NULL, *p_ch, *ch_ptr=NULL, buf_ch[128]={0};
    int array_str_num = 0, ssid_num = 0;
    struct json_object *jobj = NULL, *jobj_enjet = NULL, *jobj_enjet_ap = NULL, *jobj_enjet_sta = NULL;
    int ap_time_slot = 0, priority_num = 0;
    int txpower_start=8, txpower_end=28;
    int country_id = 0;

    country_code_t *countryTablePtr = NULL;
    wlan_info wlanInfo;
    ResponseStatus *res = rep->res;
    const int extch_array[] = {36,44,52,60,100,108,116,124,132,140,149,157};
    size_t extch_array_str_num = (sizeof extch_array) / (sizeof *extch_array);
    memset(&wlanInfo, 0, sizeof(wlanInfo));

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (strcmp(radio, WIFI_RADIO_NAME_24G) == 0)
            {
                wifix="wifi0";
                wlanInfo.wifix=0;
                api_get_wifi_opmode_option(WIRELESS_WIFI_OPMODE_OPTION, &mode);
                mode = pow(2,mode);  
            }
#if SUPPORT_WLAN_5G_SETTING
            else if (strcmp(radio, WIFI_RADIO_NAME_5G) == 0)
            {
                wifix="wifi1";
                wlanInfo.wifix=1;
                api_get_wifi_opmode_option(WIRELESS_WIFI_5G_OPMODE_OPTION, &mode);
                mode = pow(2,mode);
            }
            else if (strcmp(radio, WIFI_RADIO_NAME_5G_2) == 0)
            {
                wifix="wifi2";
                wlanInfo.wifix=2;
                api_get_wifi_opmode_option(WIRELESS_WIFI_5G_2_OPMODE_OPTION, &mode);
                mode = pow(2,mode);
            }
#endif
            else
                RET_GEN_ERRORMSG(res, API_UNKNOWN_ACTION, NULL);

            api_get_integer_option2(&country_code, "wireless.%s.country", wifix);
            api_get_integer_option2(&obey, "wireless.%s.obeyregpower", wifix);
            green_mode=(obey == 1?true:false);

            if (json_object_object_get(jobj, "opmode") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "opmode", &opmode);
                for (p = opmode; *p; ++p) *p = tolower(*p);
                if (strcmp(opmode, "ap") == 0) 
                    mode = OPM_AP;
                else if (strcmp(opmode, "sta") == 0)
                    mode = OPM_CB;
                else if (strcmp(opmode, "wds_ap") == 0)
                    mode = OPM_WDSAP;
                else if (strcmp(opmode, "wds_bridge") == 0)
                    mode = OPM_WDSB;
                else if (strcmp(opmode, "wds_sta") == 0)
                    mode = OPM_WDSSTA;
                else if (strcmp(opmode, "sta_ap") == 0)
                    mode = OPM_RP;
#if SUPPORT_SCANNING_RADIO_MODE
                else if (strcmp(opmode, "scan") == 0)
                    mode = OPM_SCAN;
#endif

                /*array_str_num = (sizeof opmode_mapping) / (sizeof *opmode_mapping);
                for ( i = 0 ; i < array_str_num ; i++ )
                {
                    if ( strcmp(opmode, opmode_mapping[i])==0 )
                    {
                        int opmodeResult=0;
                        snprintf(opmode_opt, sizeof(opmode_opt), "wireless.%s.opmode", wifix);
                        opmodeResult=api_set_wifi_opmode_option(opmode_opt, i);
                        if(opmodeResult > 0)
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OPMODE");
                        break;
                    }
                }*/
            }
            if(api_get_wifix_section_name(mode, wlanInfo.wifix, section))
            {
                RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "GET SECTION NAME");
            }

            if (json_object_object_get(jobj, "enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "enable", &enable);
                if (mode != OPM_RP)
                {
                    if (mode == OPM_WDSAP)
                    {
                        ssid_num = WDSAP_AP_WIFI_IFACE_NUM;
                    }
                    else if (mode == OPM_AP)
                    {
                        ssid_num = AP_WIFI_IFACE_NUM;
                    }
                    else
                    {
                        ssid_num = 1;
                    }
                    for (i = 0; i < ssid_num; i++)
                    {
                        if (api_get_wifi_ifacex_disabled_option_by_sectionname(mode, section, i+1, &disabled))
                        {
                            RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "GET IFACE DISABLED");
                        }
                        if ((enable == false) && (disabled == 0))
                        {
                            if (api_set_wifi_ifacex_status_option_by_sectionname(mode, section, i+1, disabled?0:1))
                            {
                                RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "SET IFACE STATUS");
                            }
                            if (api_set_wifi_ifacex_disabled_option_by_sectionname(mode, section, i+1, 1))
                            {
                                RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "SET IFACE DISABLED");
                            }
                        }
                        else if ((enable == true) && (disabled == 1))
                        {
                            if (api_get_wifi_ifacex_status_option_by_sectionname(mode, section, i+1, &tmp_status))
                                tmp_status = 0; 
                            if (api_set_wifi_ifacex_disabled_option_by_sectionname(mode, section, i+1, tmp_status?0:1))
                            {
                                RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "GET IFACE DISABLED");
                            }
                            if (api_set_wifi_ifacex_status_option_by_sectionname(mode, section, i+1, 0))
                            {
                                RET_GEN_ERRORMSG(res, API_INTERNAL_ERROR, "SET IFACE STATUS");
                            }
                        }
                    }
                }
            }
            if (json_object_object_get(jobj, "country") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "country", &country);
                country_id = atoi(country);
                if (sys_get_regular_domain_info(&regular))
                {
                    // Get country code
                    switch (regular)
                    {
                        case REG_FCC:
                            countryTablePtr = FCC_CountryCodeTable;
                            max_ctry_num = sizeof(FCC_CountryCodeTable) / sizeof(country_code_t);
                            break;
                        case REG_ETSI:
                            countryTablePtr = ETSI_CountryCodeTable;
                            max_ctry_num = sizeof(ETSI_CountryCodeTable) / sizeof(country_code_t);
                            break;
                        default:
                            countryTablePtr = INT_CountryCodeTable;
                            max_ctry_num = sizeof(INT_CountryCodeTable) / sizeof(country_code_t);
                            break;
                    }
                }

                for (i = 0; i < max_ctry_num; i++)
                {
                    if (strcmp(countryTablePtr[i].name, country)==0 || countryTablePtr[i].code==country_id)
                    {
                        country_code = countryTablePtr[i].code;
                        api_set_integer_option("wireless.wifi0.country", country_code);
                        api_set_integer_option("wireless.wifi1.country", country_code);
            #if SUPPORT_WLAN_5G_2_SETTING
                        api_set_integer_option("wireless.wifi2.country", country_code);
            #endif
                    }
                }

                if( country_code == 0 )
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "COUNTRY");

            }
            if (json_object_object_get(jobj, "green_mode") != NULL)
            {
                senao_json_object_get_boolean(jobj, "green_mode", &green_mode);
                api_set_integer_option2(green_mode, "wireless.wifi0.obeyregpower");
                api_set_integer_option2(green_mode, "wireless.wifi1.obeyregpower");
                api_set_integer_option2(green_mode, "wireless.wifi2.obeyregpower");
            }
            if (json_object_object_get(jobj, "ht_mode") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "ht_mode", &htmode);
                for ( p = htmode; *p; ++p) *p = toupper(*p);
                if (strcmp(htmode, "HT20") == 0)
                    htMode = BANDWIDTH_20MHZ, wlanInfo.htmode = 20;
                else if (strcmp(htmode, "HT40") == 0)
                    htMode = BANDWIDTH_40MHZ, wlanInfo.htmode = 40;
                else if (strcmp(htmode, "HT20_40") == 0)
                    htMode = BANDWIDTH_20MHZ_40MHZ, wlanInfo.htmode = 40;
                else if (strcmp(htmode, "HT80") == 0)
                    htMode = BANDWIDTH_80MHZ, wlanInfo.htmode = 80;
                else
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HTMODE");

                if ( wlanInfo.htmode == 20 && htMode != BANDWIDTH_20MHZ) 
                {
                    htMode = BANDWIDTH_20MHZ;
                    snprintf(htmode, sizeof(htmode), "HT20");
                }

                if (strcmp(radio, WIFI_RADIO_NAME_24G) == 0)
                {
                    if(htMode < BANDWIDTH_20MHZ || htMode > BANDWIDTH_20MHZ_40MHZ)
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HTMODE");
                }
                else if (strcmp(radio, WIFI_RADIO_NAME_5G) == 0 || strcmp(radio, WIFI_RADIO_NAME_5G_2) == 0)
                {
                    if(htMode < BANDWIDTH_20MHZ || htMode > BANDWIDTH_80MHZ || htMode == BANDWIDTH_20MHZ_40MHZ)
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HTMODE");
                }
                if (mode != OPM_CB && mode != OPM_WDSSTA && mode != OPM_RP)
                {
                    api_set_string_option2(htmode, sizeof(htmode), "wireless.%s.htmode", wifix);
                }
            }
#if HWMODE_AX
            if (senao_json_object_get_boolean(jobj, "is_11ax_disable", &ax_mode_disable))
            {
                if(wifix == "wifi0"){
                    api_set_wifi_hwmode_option(WIRELESS_WIFI_HWMODE_OPTION, ax_mode_disable?P24G_IEEE802_11NG:P24G_IEEE802_11AX);
                }
                else if(wifix == "wifi1"){
                    api_set_wifi_5g_hwmode_option(WIRELESS_WIFI_5G_HWMODE_OPTION, ax_mode_disable?P5G_IEEE802_11AC:P5G_IEEE802_11AX);
                }
            }
#endif
            if (json_object_object_get(jobj, "is_legacy_hwmode_deny") != NULL)
            {
                senao_json_object_get_boolean(jobj, "is_legacy_hwmode_deny", &hwmode_deny);
                if (mode == OPM_AP || mode == OPM_WDSAP)
                {
                    api_set_integer_option2(hwmode_deny, "wireless.%s.legacy_hwmode_deny", wifix);
                }
                else
                {
                    if (hwmode_deny == 1)
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "LEGACY_HWMODE_DENY");
                    }
                    else
                    {
                        api_set_integer_option2(hwmode_deny, "wireless.%s.legacy_hwmode_deny", wifix);
                    }
                }
            }
            if (json_object_object_get(jobj, "channel") != NULL)
            {
                senao_json_object_get_integer(jobj, "channel",&(ch));
                if (strcmp(radio, WIFI_RADIO_NAME_24G) == 0)
                {
                    if (!(ch >= 1 && ch <= 14 || ch == 0))
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CHANNEL");
                }
#if SUPPORT_WLAN_5G_SETTING
                else if (strcmp(radio, WIFI_RADIO_NAME_5G) == 0)
                {
#if SUPPORT_WLAN_5G_2_SETTING
                    if (!(ch >= 100 && ch <= 165 || ch == 0))
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CHANNEL");
#else
                    if (!(ch >= 36 && ch <= 165 || ch == 0))
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CHANNEL");
#endif
                }
                else if (strcmp(radio, WIFI_RADIO_NAME_5G_2) == 0)
                {
                    if (!(ch >= 36 && ch <= 64 || ch == 0))
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CHANNEL");
                }
#endif
                wlanInfo.ch = ch;
                
                if (!check_wifix_channel(&wlanInfo, wifix, country_code, green_mode, ch))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CHANNEL");
                    wlanInfo.ch = 0;
                }
                if ((mode == OPM_WDSB || mode == OPM_WDSAP) && ch == 0)
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CHANNEL");
                }
                if (mode != OPM_CB && mode != OPM_WDSSTA && mode != OPM_RP) 
                {
                    (wlanInfo.ch) ? api_set_integer_option2(wlanInfo.ch, "wireless.%s.channel", wifix) : api_set_string_option2("auto", sizeof("auto"), "wireless.%s.channel", wifix);
                    api_set_integer_option2((wlanInfo.ch)?4:1, "wireless.%s.channel_config_status", wifix);
                    api_set_integer_option2(0, "wireless.%s.channel_config_enable", wifix);
                }
            }

            if (json_object_object_get(jobj, "channel_config_enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "channel_config_enable", &ch_enable);
                if (ch_enable==1 && mode!=OPM_AP)
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CHANNEL_CONFIG_ENABLE");
                }
                if (ch_enable==1)
                {
                    if (json_object_object_get(jobj, "channel_config_list") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, jobj, "channel_config_list", &ch_list);
                        sys_interact(buf_ch, sizeof(buf_ch), "echo %s | grep -o \",\" | wc -l", ch_list);
                        if (atoi(buf_ch) < 1)
                        {
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CHANNEL_CONFIG_LIST");
                        }
                        memset(buf_ch, 0, sizeof (buf_ch));
                        sys_interact(buf_ch, sizeof(buf_ch), "echo %s | tr -d \" \"", ch_list);
                        if (buf_ch[strlen(buf_ch)-1] == '\n')
                            buf_ch[strlen(buf_ch)-1] = '\0';
                        sprintf(ch_list, "%s", buf_ch);
                        p_ch = strtok_r(buf_ch, ",", &ch_ptr);
                        while (p_ch != NULL)
                        {
                            wlanInfo.ch = atoi(p_ch);
                            if (!check_wifix_channel(&wlanInfo, wifix, country_code, green_mode, atoi(p_ch)))
                            {
                                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CHANNEL_CONFIG_LIST");
                                wlanInfo.ch = 0;
                            }
                            p_ch = strtok_r(NULL, ",", &ch_ptr);
                        }
                    }
                    else
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CHANNEL_CONFIG_LIST");
                    }
                    api_set_string_option2("auto", sizeof("auto"), "wireless.%s.channel", wifix);
                    api_set_string_option2(ch_list, sizeof(ch_list), "wireless.%s.channel_config_list", wifix);
                    api_set_integer_option2(4, "wireless.%s.channel_config_status", wifix);
                }
                api_set_integer_option2(ch_enable, "wireless.%s.channel_config_enable", wifix);
            }

#if SUPPORT_WATCHGUARD_FUNCTION
            if (json_object_object_get(jobj, "channel_config_dynamic") != NULL)
            {
                senao_json_object_get_boolean(jobj, "channel_config_dynamic", &ch_dynamic_enable);
                api_set_integer_option2(ch_dynamic_enable, "wireless.%s.channel_config_dynamic", wifix);
            }

            if (json_object_object_get(jobj, "channel_config_interval") != NULL)
            {
                senao_json_object_get_integer(jobj, "channel_config_interval",&(ch_config_interval));
                api_set_integer_option2(ch_config_interval, "wireless.%s.channel_config_interval", wifix);
            }
#endif

            if (json_object_object_get(jobj, "opmode") != NULL)
            {
                array_str_num = (sizeof opmode_mapping) / (sizeof *opmode_mapping);
                for ( i = 0 ; i < array_str_num ; i++ )
                {
                    if ( strcmp(opmode, opmode_mapping[i])==0 )
                    {
                        int opmodeResult=0;
                        snprintf(opmode_opt, sizeof(opmode_opt), "wireless.%s.opmode", wifix);
                        opmodeResult=api_set_wifi_opmode_option(opmode_opt, i);
                        if(opmodeResult > 0)
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OPMODE");
                        break;
                    }
                }
            }

            if (json_object_object_get(jobj, "tx_power") != NULL)
            {
                senao_json_object_get_integer(jobj, "tx_power",&(txpower));
                if (strcmp(radio, WIFI_RADIO_NAME_24G) == 0)
                {
                    if (!(txpower == 0 || (txpower >= txpower_start && txpower <= txpower_end)))
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TXPOWER");
                }
#if SUPPORT_WLAN_5G_SETTING
                else if (strcmp(radio, WIFI_RADIO_NAME_5G) == 0)
                {
                    if (!(txpower == 0 || (txpower >= txpower_start && txpower <= txpower_end)))
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TXPOWER");

                }
                else if (strcmp(radio, WIFI_RADIO_NAME_5G_2) == 0)
                {
                    if (!(txpower == 0 || (txpower >= txpower_start && txpower <= txpower_end)))
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TXPOWER");
                }
#endif
                api_get_integer_option("functionlist.vendorlist.TXPOWER_MIN_5G", &txpower_start);
                api_get_integer_option("functionlist.vendorlist.TXPOWER_MAX_5G", &txpower_end);

                if ((txpower < txpower_start) && txpower != 0)
                    txpower = txpower_start ;
                if (txpower > txpower_end)
                    txpower = txpower_end;

#if SUPPORT_NETGEAR_FUNCTION
                if (sys_check_file_existed("/tmp/insight_im_app"))
                   system("touch /etc/insight/.im_tx_power");
#endif

                api_set_integer_option2(txpower, "wireless.%s.txpower", wifix);
            }
            if (json_object_object_get(jobj, "client_limit") != NULL)
            {
                senao_json_object_get_integer(jobj, "client_limit",&(climitNum));
                if (mode == OPM_AP || mode == OPM_WDSAP)
                {
                    if (climitNum > 0 && climitNum < 128)
                    {
                        climit = 1;
                        api_set_integer_option2(climitNum, "wireless.%s.clientlimits_number", wifix);
                    }
                    else if ( climitNum == 0 )
                        climit = 0;
                    else
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT LIMITS");
                        //debug_msg("Client Limits: value is out of range [1 - 127]");
                    api_set_integer_option2(climit, "wireless.%s.clientlimits_enable", wifix);
                }
            }
            if (json_object_object_get(jobj, "min_bitrate") != NULL)
            {
                senao_json_object_get_integer(jobj, "min_bitrate",&(minbrate));
                if (strcmp(radio, WIFI_RADIO_NAME_24G) == 0)
                {
                    if (!(minbrate == 1 || minbrate == 2 || minbrate == 5 || minbrate == 6 || minbrate == 9 || minbrate == 11 || minbrate == 12 || minbrate == 18 || minbrate == 24))
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MINIMUN RATE");
                }
#if SUPPORT_WLAN_5G_SETTING
                else if (strcmp(radio, WIFI_RADIO_NAME_5G) == 0)
                {
                    if (!(minbrate == 6 || minbrate == 9 || minbrate == 12 || minbrate == 18 || minbrate == 24))
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MINIMUN RATE");

                }
                else if (strcmp(radio, WIFI_RADIO_NAME_5G_2) == 0)
                {
                    if (!(minbrate == 6 || minbrate == 9 || minbrate == 12 || minbrate == 18 || minbrate == 24))
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MINIMUN RATE");
                }
#endif
                api_set_integer_option2(minbrate, "wireless.%s.min_rate", wifix);
            }
            if (json_object_object_get(jobj, "rssi_threshold") != NULL)
            {
                senao_json_object_get_integer(jobj, "rssi_threshold",&(rssi));
                if (rssi <= -60 && rssi >= -100)
                {
                    api_set_integer_option2(1, "wireless.%s.fasthandover_status", wifix);
                    api_set_integer_option2(rssi, "wireless.%s.fasthandover_rssi", wifix);
                }
                else if ( rssi == 0 )
                {
                    api_set_integer_option2(0, "wireless.%s.fasthandover_status", wifix);
                }
                else
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "RSSI THRESHOLD");
                }
            }
            if (json_object_object_get(jobj, "distance") != NULL)
            {
                senao_json_object_get_integer(jobj, "distance",&(outdoor_distance));
                outdoor_distance = (outdoor_distance*1000);
#if SUPPORT_NETGEAR_FUNCTION
                if (sys_check_file_existed("/tmp/insight_im_app"))
                   system("touch /etc/insight/.im_distance");
#endif
                api_set_integer_option2(outdoor_distance, "wireless.%s.distance", wifix);
            }
#if SUPPORT_ENJET
            if (json_object_object_get(jobj, "enjet_enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "enjet_enable",&(enjet_enable));
                api_set_integer_option2(enjet_enable, "wireless.%s.qboost_enable", wifix);
                api_set_integer_option2(!enjet_enable, "wireless.%s_enjet.disabled", wifix);
                if (enjet_enable == true)
                    api_set_lan_stp_status_option(NETWORK_LAN_STP_STATUS_OPTION, 1);
            }
            if (json_object_object_get(jobj, "enjet") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "enjet", &enjet_info);
                if (jobj_enjet = jsonTokenerParseFromStack(rep, enjet_info))
                {
                    if (json_object_object_get(jobj_enjet, "AP_role") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, jobj_enjet, "AP_role", &ap_role);

                        if (jobj_enjet_ap = jsonTokenerParseFromStack(rep, ap_role))
                        {
                            if (json_object_object_get(jobj_enjet_ap, "ap_time_slot") != NULL)
                            {
                                senao_json_object_get_integer(jobj_enjet_ap, "ap_time_slot",&(ap_time_slot));
                                if (ap_time_slot >=0 && ap_time_slot <=7 && ap_time_slot !=1) 
                                {
                                    api_set_integer_option2(ap_time_slot, "wireless.%s.aptimeslot", wifix);
                                }
                                else
                                {
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AP_TIME_SLOT");
                                }
                            }
                        }
                    }
                    if (json_object_object_get(jobj_enjet, "Station_role") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, jobj_enjet, "Station_role", &sta_role);
                        if(jobj_enjet_sta = jsonTokenerParseFromStack(rep, sta_role))
                        {
                            if (json_object_object_get(jobj_enjet_sta, "priority") != NULL)
                            {
                                senao_json_object_get_and_create_string(rep,jobj_enjet_sta, "priority", &priority);

                                if (strcmp(priority,"High") == 0) 
                                {
                                    priority_num = 0;
                                }
                                else if (strcmp(priority,"Middle") == 0)
                                {
                                    priority_num = 1;
                                }
                                else if (strcmp(priority,"Low") == 0)
                                {
                                    priority_num = 2;
                                }
                                else
                                {
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PRIORITY");
                                }
                                api_set_integer_option2(priority_num, "wireless.%s.stationpriority", wifix);
                            }
                        }
                    }
                }
            }
#endif
#if SUPPORT_NETGEAR_FUNCTION
                if (sys_check_file_existed("/tmp/insight_im_app"))
                   system("rm /tmp/insight_im_app");
#endif
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_patch_wifi_guest_network(ResponseEntry *rep, char *query_str)
{
    char *ip = NULL, *mask = NULL, *start_ip = NULL, *end_ip = NULL, *wins_server = NULL;
    struct json_object *jobj;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (json_object_object_get(jobj, "ip") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "ip", &ip);
                if (api_set_wifi_guest_ipaddr_option(NETWORK_GUEST_IPADDR_OPTION, ip, sizeof(ip)))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IP");
            }
            if (json_object_object_get(jobj, "mask") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "mask", &mask);
                if (api_set_wifi_guest_netmask_option(NETWORK_GUEST_NETMASK_OPTION, mask, sizeof(mask)))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MASK");
            }
            if (json_object_object_get(jobj, "start") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "start", &start_ip);
                if (api_set_wifi_guest_dhcp_start_option(DHCP_GUEST_START_OPTION, start_ip, sizeof(start_ip)))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "START");
            }
            if (json_object_object_get(jobj, "end") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "end", &end_ip);
                if (api_set_wifi_guest_dhcp_end_option(NULL, end_ip, sizeof(end_ip)))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "END");
            }
            if (json_object_object_get(jobj, "wins_server") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "wins_server", &wins_server);
                if (api_set_wifi_guest_wins_server_option(DHCP_GUEST_DHCP_OPTION_OPTION, wins_server, sizeof(wins_server)))
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "WINS SERVER");
            }
        }
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_patch_sys_syslog(ResponseEntry *rep, char *query_str)
{
    char *log_ip = NULL;
    int log_port=0;
    bool syslog_enable=true,trafficlog_enable=true, remotelog_enable=true;
    struct json_object *jobj;

    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (json_object_object_get(jobj,"syslog_enable") != NULL)
            {
		senao_json_object_get_boolean(jobj, "syslog_enable",&syslog_enable);
                if (api_set_integer_option(SYSTEM_SYSTEM_SYSLOG_ENABLE_OPTION,(syslog_enable?1:0)) != API_RC_SUCCESS)
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "syslog_enable");
            }

            if (json_object_object_get(jobj, "remotelog_enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "remotelog_enable",&remotelog_enable);
                if (api_set_integer_option(SYSTEM_SYSTEM_LOG_REMOTELOG_ENABLE_OPTION,(remotelog_enable?1:0)) != API_RC_SUCCESS)
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "remotelog_enable");
            }

            if (json_object_object_get(jobj, "trafficlog_enable") != NULL)
            {
		senao_json_object_get_boolean(jobj, "trafficlog_enable",&trafficlog_enable);
                if (api_set_integer_option(SYSTEM_SYSTEM_LOG_TRAFFICLOG_ENABLE_OPTION,(trafficlog_enable?1:0)) != API_RC_SUCCESS)
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "trafficlog_enable");
	    }

            if (json_object_object_get(jobj, "log_ip") != NULL)
            {
		senao_json_object_get_and_create_string(rep, jobj, "log_ip", &log_ip);
                if (api_set_system_log_ip_option(SYSTEM_SYSTEM_LOG_IP_OPTION, log_ip, sizeof(log_ip)) != API_RC_SUCCESS)
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "log_ip");
            }

	    if (json_object_object_get(jobj, "log_port")!= NULL)
	    {
		senao_json_object_get_integer(jobj, "log_port",&(log_port));
                if (api_set_system_log_port_option(SYSTEM_SYSTEM_LOG_PORT_OPTION, log_port) != API_RC_SUCCESS)
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "log_port");
	    }
	}
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_patch_wifi_vpn_profile(ResponseEntry *rep, char *query_str)
{
    ResponseStatus *res = rep->res;

    struct json_object *jobj = NULL, *remote_obj = NULL, *tunnel_obj = NULL, *p2proposal_obj = NULL, *profile_obj = NULL;
    struct json_object *p1_jarr = NULL, *p1_jarr_info = NULL, *route_jarr = NULL, *route_jarr_info = NULL;

    char *profileName = NULL, *remote_obj_str = NULL, *tunnel_obj_str = NULL, *type = NULL, *gateway = NULL, *auth_method = NULL;
    char *preShareKey = NULL, *eapIdentity = NULL, *eapPassword = NULL, *p1proposal_str = NULL, *encryption_alg = NULL;
    char *hash_alg = NULL, *dhGroup = NULL, *localSubnet = NULL, *remoteSubnet = NULL, *route_str = NULL, *route = NULL;
    char *p2proposal_str = NULL, *p2_encr_algo = NULL, *p2_auth_algo = NULL;
    char buf[256] = {0};
    bool enable = 0, forceTraffic = 0;
    int arraylen = 0, json_array_idx = 0, j = 0, profile_len = 0, profile_num = 0;

    PROFILE_SETTING profile_data;
    P1_PROPOSAL_SETTING p1_data;
    TUNNEL_SETTING tunnel_data;
    ROUTE_SETTING route_data;
    P2_PROPOSAL_SETTING p2_data;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            profile_len = json_object_array_length(jobj);

            for ( profile_num = 0; profile_num < profile_len; profile_num++ )
            {
                profile_obj = json_object_array_get_idx(jobj, profile_num);
                senao_json_object_get_and_create_string(rep, profile_obj, "vpn_profile_name", &profileName);
                senao_json_object_get_and_create_string(rep, profile_obj, "vpn_remote", &remote_obj_str);
                senao_json_object_get_and_create_string(rep, profile_obj, "vpn_tunnel", &tunnel_obj_str);

                strcpy(profile_data.profile_name, profileName);

                if((remote_obj = jsonTokenerParseFromStack(rep, remote_obj_str)))
                {
                    senao_json_object_get_boolean(remote_obj, "enable", &enable);
                    senao_json_object_get_and_create_string(rep, remote_obj, "type", &type);
                    senao_json_object_get_and_create_string(rep, remote_obj, "gateway", &gateway);
                    senao_json_object_get_and_create_string(rep, remote_obj, "authentication_method", &auth_method);
                    senao_json_object_get_and_create_string(rep, remote_obj, "pre_shared_key", &preShareKey);
                    senao_json_object_get_and_create_string(rep, remote_obj, "identity", &eapIdentity);
                    senao_json_object_get_and_create_string(rep, remote_obj, "password", &eapPassword);
                    senao_json_object_get_and_create_string(rep, remote_obj, "p1_proposal", &p1proposal_str);

                    profile_data.enable=enable;
                    strcpy(profile_data.vpn_type, type);
                    strcpy(profile_data.gateway, gateway);
                    strcpy(profile_data.authentication_method, auth_method);
                    strcpy(profile_data.pre_shared_key, preShareKey);
                    strcpy(profile_data.identity, eapIdentity);
                    strcpy(profile_data.password, eapPassword);
                }

                if((tunnel_obj = jsonTokenerParseFromStack(rep, tunnel_obj_str)))
                {
                    senao_json_object_get_and_create_string(rep, tunnel_obj, "local_subnet", &localSubnet);
                    senao_json_object_get_and_create_string(rep, tunnel_obj, "remote_subnet", &remoteSubnet);
                    senao_json_object_get_boolean(tunnel_obj, "force_traffic", &forceTraffic);
                    senao_json_object_get_and_create_string(rep, tunnel_obj, "route", &route_str);

                    profile_data.force_traffic=forceTraffic;
                    strcpy(tunnel_data.profile_name, profileName);
                    strcpy(tunnel_data.local_subnet, localSubnet);
                    strcpy(tunnel_data.remote_subnet, remoteSubnet);

                    senao_json_object_get_and_create_string(rep, tunnel_obj, "p2_proposal", &p2proposal_str);

                    if((p2proposal_obj = jsonTokenerParseFromStack(rep, p2proposal_str)))
                    {
                        senao_json_object_get_and_create_string(rep, p2proposal_obj, "encryption_algorithm", &p2_encr_algo);
                        senao_json_object_get_and_create_string(rep, p2proposal_obj, "hash_algorithm", &p2_auth_algo);

                        strcpy(p2_data.profile_name, profileName);
                        strcpy(p2_data.encryption_algorithm, p2_encr_algo);
                        strcpy(p2_data.hash_algorithm, p2_auth_algo);
                    }
                }

                api_set_vpn_profile_section(profile_data);

                if(p1_jarr = jsonTokenerParseFromStack(rep, p1proposal_str))
                {
                    arraylen = json_object_array_length(p1_jarr);

                    for (json_array_idx = 0; json_array_idx < arraylen; json_array_idx++)
                    {
                        p1_jarr_info = json_object_array_get_idx(p1_jarr, json_array_idx);

                        senao_json_object_get_and_create_string(rep, p1_jarr_info, "encryption_algorithm", &encryption_alg);
                        senao_json_object_get_and_create_string(rep, p1_jarr_info, "hash_algorithm", &hash_alg);
                        senao_json_object_get_and_create_string(rep, p1_jarr_info, "dh_group", &dhGroup);

                        strcpy(p1_data.profile_name, profileName);
                        strcpy(p1_data.encryption_algorithm, encryption_alg);
                        strcpy(p1_data.hash_algorithm, hash_alg);
                        strcpy(p1_data.dh_group, dhGroup);

                        api_set_vpn_p1_proposal_section(p1_data);
                    }
                }

                api_set_vpn_tunnel_section(tunnel_data);
                api_set_vpn_p2_proposal_section(p2_data);

                if(route_jarr = jsonTokenerParseFromStack(rep, route_str))
                {
                    memset(buf, 0, sizeof(buf));
                    j=0;
                    arraylen = json_object_array_length(route_jarr);

                    for (json_array_idx = 0; json_array_idx < arraylen; json_array_idx++)
                    {
                        route_jarr_info = json_object_array_get_idx(route_jarr, json_array_idx);

                        senao_json_object_get_and_create_string(rep, route_jarr_info, "route", &route);

                        if ((json_array_idx + 1) < arraylen)
                            j+=sprintf(buf+j, "%s ", route);
                        else
                            j+=sprintf(buf+j, "%s", route);

                    }
                    strcpy(route_data.profile_name, profileName);
                    strcpy(route_data.route, buf);
                    api_set_vpn_route_section(route_data);
                }
            }
        }

        RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }


}
