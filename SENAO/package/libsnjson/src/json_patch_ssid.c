#include <api_common.h>
#include <sys_common.h>
#include <api_wireless.h>
#include <variable.h>
#include <api_lan.h>
#include <wireless_tokens.h>
//#include <integer_check.h>
#include <json_object.h>
#include <json_tokener.h>
#include <json_ssid.h>
#include <json_patch_ssid.h>
#include <json_chk_ssid.h>
#include <json_common.h>
#include <variable/api_schedule.h>
#include <unistd.h>
#include <api_vpn.h>


int json_patch_wireless_ssid(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){
    int radio_idx = 0, opmode = ssidCfg_p->opmode,  vlan_id = 0;
    char *ssid=NULL, *enable_bands=NULL, *guest_network = NULL, *client_ip_assignment = NULL, *profile_name = NULL;
    char *section = ssidCfg_p->section;
    char *bs_jsonStr=NULL, *cd_jsonStr=NULL, *tf_jsonStr=NULL, *security_jsonStr=NULL, *cp_jsonStr=NULL, *scheduling_jsonStr=NULL, *l2_acl_jsonStr=NULL, *captive_portal_jsonStr=NULL;
    bool enable = 0, hidden = 0, isolate = 0, l2_isolation = 0, isolation = 0;
    struct json_object *jobj;
    int radio_start = 0, radio_end = 0, i = 0, ret = 0;
    ResponseStatus *res = rep->res;
    int radio_num = 0;
    int idx = ssidCfg_p->idx;
    char gn_status[20] = {0};

    if ( RADIO_MODE % 5 != 2 ) // no wifi0
        radio_start = 1;

    radio_end = radio_start + WIFI_RADIO_NUM - 1;

debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (json_object_object_get(jobj, "enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "enable",&(enable));
                if (enable == false) 
                {
                    if (opmode != OPM_WDSAP){
                        for (i = radio_start; i <= radio_end; i++)
                        {
                            set_wifi_ifacex_disabled(i,idx,1,ssidCfg_p);
                        }
                    }
                    else//disable wds_ap
                    {
                        if (strstr(ssidCfg_p->section,"wifi0") )
                            i = 0;
                        else if (strstr(ssidCfg_p->section,"wifi1") )
                            i = 1;
                        else if (strstr(ssidCfg_p->section,"wifi2"))
                            i = 2;
                        set_wifi_ifacex_disabled(i,idx,1,ssidCfg_p);
                    }
                }
            }
            if (json_object_object_get(jobj, "enable_bands") != NULL || (enable == true && opmode == OPM_WDSAP))
            {
                senao_json_object_get_and_create_string(rep, jobj, "enable_bands", &enable_bands);
                // if (enable == true) 
                if (opmode != OPM_WDSAP)
                {
                    for (i = radio_start; i <= radio_end; i++)
                    {
                        set_wifi_ifacex_disabled(i,idx,1,ssidCfg_p);
                    }
                }
                else
                {
                    if(strcmp(enable_bands , "") == 0)
                    {
                        debug_print("[DEBUG] WDS-AP not support enableband.\n");
                    }
                    else
                    {
                        debug_print("[DEBUG] Error! WDS-AP not support enableband.\n");
                        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
                    }

                    if (strstr(ssidCfg_p->section,"wifi0") )
                        strcpy(enable_bands,"2_4G");
                    else if (strstr(ssidCfg_p->section,"wifi1") )
                        strcpy(enable_bands,"5G");
                    else if (strstr(ssidCfg_p->section,"wifi2"))
                        strcpy(enable_bands,"5G-2");
                }
                if ( strstr(enable_bands,"2_4G") )
                {
                    set_wifi_ifacex_disabled(0,idx,0,ssidCfg_p);
                    radio_num++;
                }

                if ( strstr(enable_bands,"5G-2") )
                {
                    set_wifi_ifacex_disabled(2,idx,0,ssidCfg_p);
                    if ( strstr(enable_bands,"5G|") )
                    {
                        set_wifi_ifacex_disabled(1,idx,0,ssidCfg_p);
                    }
                    radio_num++;
                }
                else if ( strstr(enable_bands,"5G") )
                {
                    set_wifi_ifacex_disabled(1,idx,0,ssidCfg_p);
                    radio_num++;
                }
            }
            if (json_object_object_get(jobj, "ssid_name") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "ssid_name", &ssid);
                if(api_set_wifi_ifacex_ssid_option_by_sectionname(opmode, section, idx, ssid, sizeof ssid))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SSID NAME");
                }
            }
            if (json_object_object_get(jobj, "hidden_ssid") != NULL)
            {
                senao_json_object_get_boolean(jobj, "hidden_ssid",&(hidden));
                if(api_set_wifi_ifacex_hidden_option_by_sectionname(opmode, section, idx, hidden))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HIDDEN SSID");
                }
            }
            if (json_object_object_get(jobj, "client_isolation") != NULL)
            {
                senao_json_object_get_boolean(jobj, "client_isolation",&(isolate));
                if(api_set_wifi_ifacex_isolate_option_by_sectionname(opmode, section, idx, isolate))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT ISOLATION");
                }
            }
            if (json_object_object_get(jobj, "l2_isolation") != NULL)
            {
                senao_json_object_get_boolean(jobj, "l2_isolation",&(l2_isolation));
                if(api_set_wifi_ifacex_l2_isolation_option_by_sectionname(opmode, section, idx, l2_isolation))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "L2 ISOLATION");
                }
            }

#if SUPPORT_SWOS_FUNCTION
            api_get_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, gn_status, sizeof gn_status);

            if (strcmp(gn_status,"Disable") == 0 || strcmp(gn_status,"") == 0)

#endif
            {
                if (json_object_object_get(jobj, "vlan_isolation") != NULL)
                {
                    senao_json_object_get_boolean(jobj, "vlan_isolation",&(isolation));
                    if(api_set_wifi_ifacex_isolation_option_by_sectionname(opmode, section, idx, isolation))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "VLAN ISOLATION");
                    }
                }
                if (json_object_object_get(jobj, "vlan_id") != NULL)
                {
                    senao_json_object_get_integer(jobj, "vlan_id",&(vlan_id));
                    if (api_set_wifi_ifacex_vlan_id_option_by_sectionname(opmode, section, idx, vlan_id))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "VLAN ID");
                    }
                }
            }
            if (json_object_object_get(jobj, "client_ip_assignment") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "client_ip_assignment", &client_ip_assignment);
                debug_print("Jason DEBUG %s[%d]client_ip_assignment[%s]\n", __FUNCTION__, __LINE__, client_ip_assignment);
            }
            if (json_object_object_get(jobj, "wireless_security") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "wireless_security", &security_jsonStr);
                if((ret = json_patch_wireless_security(rep, security_jsonStr, ssidCfg_p)) != API_SUCCESS)
                {
                    return ret;
                }
            }
            if (json_object_object_get(jobj, "band_steering") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "band_steering", &bs_jsonStr);
                if((ret = json_patch_wireless_band_steering(rep, bs_jsonStr, ssidCfg_p)) != API_SUCCESS)
                {
                    return ret;
                }
            }
            if (json_object_object_get(jobj, "traffic_shaping") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "traffic_shaping", &tf_jsonStr);
                if((ret = json_patch_wireless_traffic_shaping(rep, tf_jsonStr, ssidCfg_p)) != API_SUCCESS)
                {
                    return ret;
                }
            }
            if (json_object_object_get(jobj, "guest_network") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "guest_network", &cp_jsonStr);
                if((ret = json_patch_wireless_guest_network(rep, cp_jsonStr, ssidCfg_p)) != API_SUCCESS)
                {
                    return ret;
                }
            }
#if !SUPPORT_SWOS_FUNCTION
            if (json_object_object_get(jobj, "captive_portal") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "captive_portal", &captive_portal_jsonStr);
                if((ret = json_patch_wireless_captive_portal(rep, captive_portal_jsonStr, ssidCfg_p, client_ip_assignment)) != API_SUCCESS)
                {
                    return ret;
                }
            }
            if (json_object_object_get(jobj, "client_dns_server") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "client_dns_server", &cd_jsonStr);
                if((ret = json_patch_client_dns_server(rep, cd_jsonStr, ssidCfg_p, client_ip_assignment)) != API_SUCCESS)
                {
                    return ret;
                }
            }
#endif
            if (json_object_object_get(jobj, "scheduling") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "scheduling", &scheduling_jsonStr);
                if((ret = json_patch_wireless_scheduling_sync(rep, scheduling_jsonStr, ssidCfg_p)) != API_SUCCESS)
                {
                    return ret;
                }
            }
            if (json_object_object_get(jobj, "l2_acl") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "l2_acl", &l2_acl_jsonStr);
                if((ret = json_patch_wireless_l2_acl(rep, l2_acl_jsonStr, ssidCfg_p)) != API_SUCCESS)
                {
                    return ret;
                }
            }
#if SUPPORT_VPN_FUNCTION
            if (json_object_object_get(jobj, "vpn_profile_name") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "vpn_profile_name", &profile_name);
                api_set_snvpn_ssid_profile_name(idx, profile_name, sizeof(profile_name));
                api_set_snvpn_ssid_enable(idx, 1);
            }
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

debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_patch_wireless_guest_network(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){

    struct json_object *jobj, *jobj_nat=NULL;
    char *section = ssidCfg_p->section;
    int opmode = ssidCfg_p->opmode;
    bool enable = 0, nat_enable=0;
    int idx = ssidCfg_p->idx, time;
    char guest_network_en[16] = {0}, guest_network_ori[16]={0};
    char *ip=NULL, *mask=NULL, *start_ip=NULL, *end_ip=NULL, *lease_time=NULL, *nat_jsonStr=NULL, *cd_jsonStr=NULL;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (json_object_object_get(jobj, "enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "enable", &enable);

#if SUPPORT_SWOS_FUNCTION
                strcpy(guest_network_en, enable?"NAT_only":"Disable");
                /*if(enable)
                {
                    if(api_set_wifi_ifacex_isolate_option_by_sectionname(opmode, section, idx, 1))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HIDDEN SSID");
                    }
                }
                else
                {
                    if(api_set_wifi_ifacex_isolate_option_by_sectionname(opmode, section, idx, 0))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HIDDEN SSID");
                    }
                }*/
#else
                strcpy(guest_network_en, enable?"Enable":"Disable");
#endif
                if(api_set_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, guest_network_en, sizeof(guest_network_en)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"GUEST_NETWORK");
                }
            }

#if SUPPORT_SWOS_FUNCTION
            if (json_object_object_get(jobj, "manual_nat") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "manual_nat", &nat_jsonStr);
                if((jobj_nat = jsonTokenerParseFromStack(rep, nat_jsonStr)))
                {
                    api_get_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, guest_network_ori, sizeof(guest_network_ori));

                    /*if (json_object_object_get(jobj_nat, "enable") != NULL)
                    {
                        senao_json_object_get_boolean(jobj_nat, "enable", &nat_enable);
                        if (nat_enable==1 && strcmp(guest_network_ori, "Enable")==0)
                        {
                            strcpy(guest_network_en, "NAT_only");
                        }
                        else
                        {
                            strcpy(guest_network_en, "Disable");
                        }
                        if(api_set_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, guest_network_en, sizeof(guest_network_en)))
                        {
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"GUEST_NETWORK");
                        }
                    }*/
                    if (json_object_object_get(jobj_nat, "ip") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, jobj_nat, "ip", &ip);
                        if(api_set_network_nat_ip_address_option_by_sectionname(section, idx, ip, sizeof(ip)) != API_RC_SUCCESS)
                        {
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IP");
                        }
                    }
                    if (json_object_object_get(jobj_nat, "subnet_mask") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, jobj_nat, "subnet_mask", &mask);
                        if(api_set_network_nat_subnet_mask_option_by_sectionname(section, idx, mask, sizeof(mask)) != API_RC_SUCCESS)
                        {
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SUBNET_MASK");
                        }
                    }
                    if (json_object_object_get(jobj_nat, "start_ip") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, jobj_nat, "start_ip", &start_ip);
                        if(api_set_dhcp_nat_start_ip_option_by_sectionname(section, idx, start_ip, sizeof(start_ip)) != API_RC_SUCCESS)
                        {
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "START_IP");
                        }
                    }
                    if (json_object_object_get(jobj_nat, "end_ip") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, jobj_nat, "end_ip", &end_ip);
                        if(api_set_dhcp_nat_end_ip_option_by_sectionname(section, idx, end_ip, sizeof(end_ip)) != API_RC_SUCCESS)
                        {
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "END_IP");
                        }
                    }
                    if (json_object_object_get(jobj_nat, "client_lease_time") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, jobj_nat, "client_lease_time", &lease_time);
                        sscanf(lease_time, "%dh", &time);
                        if (time<1 || time>24)
                        {
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT LEASE_TIME");
                        }
                        if(api_set_dhcp_nat_lease_time_option_by_sectionname(section, idx, lease_time, sizeof(lease_time)) != API_RC_SUCCESS)
                        {
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT LEASE_TIME");
                        }
                    }
                    if (json_object_object_get(jobj_nat, "client_dns_server") != NULL)
                    {
                        senao_json_object_get_and_create_string(rep, jobj_nat, "client_dns_server", &cd_jsonStr);
                        if (strcmp(guest_network_ori, "NAT_only")==0)
                        {
                            json_patch_client_dns_server(rep, cd_jsonStr, ssidCfg_p, "NAT");
                        }
                        else
                        {
                            json_patch_client_dns_server(rep, cd_jsonStr, ssidCfg_p, "");
                        }
                    }
                }
            }
#endif
        }
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_patch_wireless_captive_portal(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char *option){

    struct json_object *jobj;
    int opmode = ssidCfg_p->opmode, idx = ssidCfg_p->idx;
    int auth_type = 0, session_timeout = 0, idle_timeout = 0;
    bool enable = 0, portal_en = 0;
    char *ext_url=NULL, *walled_garden=NULL, *auth_type_str=NULL;
#if SUPPORT_SWOS_FUNCTION
    char *shared_secret=NULL, *splash_url=NULL;
#endif
    char *section = ssidCfg_p->section;
    char guestnetwork_origin[16] = {0}, client_ip_assignment[16] = {0}, guest_network[16] = {0};
    ResponseStatus *res = rep->res;

#if SUPPORT_CAPTIVE_PORTAL_SETTING
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (option && strlen(option) > 0)
            {
                strcpy(client_ip_assignment, option);
            }
            else
            {
                api_get_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, guestnetwork_origin, sizeof guestnetwork_origin);
                if (strcmp(guestnetwork_origin,"Disable") == 0 || strcmp(guestnetwork_origin,"") == 0 || strcmp(guestnetwork_origin,"Bridge") == 0)
                {
                    strcpy(client_ip_assignment, "Bridge");
                }
                else //NAT
                {
                    strcpy(client_ip_assignment, "NAT");
                }
            }



            if (json_object_object_get(jobj, "enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "enable", &enable);
                if (enable == 0)
                {
                    portal_en = 0;
                    if (strcmp(client_ip_assignment, "NAT") == 0){
                        strcpy(guest_network, "NAT_only");
                    }
                    else if (strcmp(client_ip_assignment, "Bridge") == 0){
                        strcpy(guest_network, "Disable");
                    }
                    api_set_string_option2(guest_network, sizeof(guest_network), "portal.ssid_%d.guest_network", idx);
                }
                else
                {
                    portal_en = 1;
                    strcpy(guest_network, client_ip_assignment);
                    api_set_string_option2(guest_network, sizeof(guest_network), "portal.ssid_%d.guest_network", idx);
                }
                if(api_set_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, guest_network, sizeof(guest_network)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"GUEST_NETWORK");
                }
            }

            if (json_object_object_get(jobj, "auth_type") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "auth_type", &auth_type_str);
                if (strcmp(auth_type_str, "splash") == 0) 
                {
                    auth_type = 1;
                }
                else if (strcmp(auth_type_str, "cloud-radius") == 0) 
                {
                    auth_type = 2;
                }
                else if (strcmp(auth_type_str, "custom-radius") == 0)
                {
                    auth_type = 3;
                }
                if (strcmp(auth_type_str, "click-through") == 0)
                {
                    auth_type = 300;
                }

                if (api_set_portal_auth_type_option_by_sectionname(guest_network, idx, auth_type))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"AUTH_TYPE");
                }
            }
#if SUPPORT_SWOS_FUNCTION
            if (json_object_object_get(jobj, "splash_url") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "splash_url", &splash_url);
                if (api_set_portal_intSplashUrl_option_by_sectionname(guest_network, idx, splash_url, sizeof(splash_url)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"SPLASH_URL");
                }
            }
            if (json_object_object_get(jobj, "shared_secret") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "shared_secret", &shared_secret);
                if (api_set_portal_sharedsecret_option_by_sectionname(guest_network, idx, shared_secret, sizeof(shared_secret)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"SHARED_SECRET");
                }
            }
#else
            if (json_object_object_get(jobj, "external_splash_url") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "external_splash_url", &ext_url);
                if (api_set_portal_extSplashUrl_option_by_sectionname(guest_network, idx, ext_url, sizeof(ext_url)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"EXT_URL");
                }
            }
#endif
            if (json_object_object_get(jobj, "session_timeout") != NULL)
            {
                senao_json_object_get_integer(jobj, "session_timeout", &session_timeout);
                if(api_set_portal_sessionTimeout_option_by_sectionname(guest_network, idx, session_timeout))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"SESSION_TIMEOUT");
                }
                if(api_set_portal_sessionTimeout_enable_option_by_sectionname(guest_network, idx, (session_timeout == 0)?0:1))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"SESSION_TIMEOUT_ENABLE");
                }
            }
            if (json_object_object_get(jobj, "idle_timeout") != NULL)
            {
                senao_json_object_get_integer(jobj, "idle_timeout", &idle_timeout);
                if(api_set_portal_idleTimeout_option_by_sectionname(guest_network, idx, idle_timeout))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"IDLE_TIMEOUT");
                }
                if(api_set_portal_idleTimeout_enable_option_by_sectionname(guest_network, idx, (idle_timeout == 0)?0:1))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"IDLE_TIMEOUT_ENABLE");
                }
            }
            if (json_object_object_get(jobj, "walled_garden") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "walled_garden", &walled_garden);
                if(api_set_portal_walledGardenEnable_option_by_sectionname(guest_network, idx, (strcmp(walled_garden, "") == 0)?0:1))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"WALLED_GARDEN_ENABLE");
                }
                if (api_set_portal_walledGarden_option_by_sectionname(guest_network, idx, walled_garden, sizeof(walled_garden)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"WALLED_GARDEN");
                }
            }
        }
    }
#endif

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_patch_wireless_band_steering(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){

    int opmode = ssidCfg_p->opmode, radio_idx = 0, bandsteer = 0, bandsteerrssi = 0, bandsteerpercent = 0;
    bool bandsteer_en = 0;
    char *steering_type=NULL, *wifix;
    char *section = ssidCfg_p->section;
    struct json_object *jobj;
    ResponseStatus *res = rep->res;
    int idx = ssidCfg_p->idx;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (json_object_object_get(jobj, "enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "enable",&(bandsteer_en));
                if(api_set_integer_option2(bandsteer_en, "%s_%d.%s", section, idx, JSON_WIRELESS_WIFI_BANDSTEER_EN))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BAND STEERING ENABLE");
                }
            }
            if (json_object_object_get(jobj, "steering_type") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "steering_type", &steering_type);
                if (strcmp(steering_type, DESC_BAND_STEER_FORCE_5G) == 0)
                {
                    bandsteer = FORCE_5G;
                }
                else if (strcmp(steering_type, DESC_BAND_STEER_PREFER_5G) == 0)
                {
                    bandsteer = PREFER_5G;
                }
                else if(strcmp(steering_type, DESC_BAND_STEER_BAND_BALANCE) == 0)
                {
                    bandsteer = BAND_BALANCE;
                }

                if(api_set_integer_option2(bandsteer, "%s_%d.%s", section, idx, JSON_WIRELESS_WIFI_BANDSTEER_EN))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "STEERING TYPE");
                    return FALSE;
                }
            }
            if (json_object_object_get(jobj, "5g_rssi_threshold") != NULL)
            {
                senao_json_object_get_integer(jobj, "5g_rssi_threshold",&(bandsteerrssi));
                if(bandsteerrssi >= -80 && bandsteerrssi <= -60)
                {
                    bandsteerrssi += 95;
                    if(api_set_integer_option2(bandsteerrssi, "%s_%d.%s", section, idx, JSON_WIRELESS_WIFI_BANDSTEER_RSSI))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "5GHz RSSI THRESHOLD");
                    }
                }
                else
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "5GHz RSSI THRESHOLD");
                }
            }
            if (json_object_object_get(jobj, "5g_client_percent") != NULL)
            {
                senao_json_object_get_integer(jobj, "5g_client_percent",&(bandsteerpercent));
                if(bandsteerpercent >= 0 && bandsteerpercent <= 100)
                {
                    if(api_set_integer_option2(bandsteerpercent, "%s_%d.%s", section, idx, JSON_WIRELESS_WIFI_BANDSTEER_PERCENT))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "5GHz CLIENT PERCENT");
                    }
                }
                else
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "5GHz CLIENT PERCENT");
                }
            }
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

int json_patch_client_dns_server(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char *option)
{
    struct json_object *jobj;
    bool enable = 0;
    char *primary_dns = NULL, *secondary_dns = NULL;
    char guestnetwork_origin[16] = {0}, client_ip_assignment[16] = {0};
    char *section = ssidCfg_p->section;
    char buf[64] = {0};
    int opmode = ssidCfg_p->opmode, idx = ssidCfg_p->idx;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_boolean(jobj, "enable",&(enable));
            senao_json_object_get_and_create_string(rep, jobj, "primary_dns", &primary_dns);
            senao_json_object_get_and_create_string(rep, jobj, "secondary_dns", &secondary_dns);

            if (option && strlen(option) > 0)
            {
                strcpy(client_ip_assignment, option);
            }
            else
            {
                api_get_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, guestnetwork_origin, sizeof(guestnetwork_origin));
                if (strcmp(guestnetwork_origin,"Disable") == 0 || strcmp(guestnetwork_origin,"") == 0 || strcmp(guestnetwork_origin,"Bridge") == 0)
                {
                    strcpy(client_ip_assignment, "Bridge");
                }
                else //NAT
                {
                    strcpy(client_ip_assignment, "NAT");
                }
            }

            if (strcmp(client_ip_assignment, "NAT") == 0)
            {
                if(enable == 0)
                {
                    strcpy(buf, "");
                }
                else
                {
                    if(strcmp(primary_dns, "") != 0 && strcmp(secondary_dns, "") != 0)
                    {// set all
                        snprintf(buf, sizeof(buf), "6,%s,%s", primary_dns, secondary_dns);
                    }
                    else if(strcmp(primary_dns, "") != 0 && strcmp(secondary_dns, "") == 0)
                    {// set primary_dns in primary_dns
                        snprintf(buf, sizeof(buf), "6,%s", primary_dns);
                    }
                    else if(strcmp(primary_dns, "") == 0 && strcmp(secondary_dns, "") != 0)
                    {// set secondary_dns in primary_dns
                        strcpy(primary_dns, secondary_dns);
                        strcpy(secondary_dns, "");
                        snprintf(buf, sizeof(buf), "6,%s", primary_dns);
                    }
                    else
                    {// set null and disable
                        enable = 0;
                        strcpy(buf, "");
                    }
                }
                if(api_set_dhcp_nat_dns_mode_option_by_sectionname(section, idx, buf, sizeof(buf)) != API_RC_SUCCESS)
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT DNS SERVER DNS");
                }
#if SUPPORT_CAPTIVE_PORTAL_SETTING
                if(api_set_portal_customdns_option_by_sectionname(section, idx, enable) != API_RC_SUCCESS)
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT DNS SERVER ENABLE");
                }
                if(api_set_portal_dns_option_by_sectionname(section, idx, primary_dns, sizeof(primary_dns), 1) != API_RC_SUCCESS)
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PRIMARY DNS");
                }
                if(api_set_portal_dns_option_by_sectionname(section, idx, secondary_dns, sizeof(secondary_dns), 2) != API_RC_SUCCESS)
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SECONDARY DNS");
                }
#endif
            }
            else
            {
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT DNS SERVER");
            }
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

int json_patch_wireless_traffic_shaping(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){
    int opmode = ssidCfg_p->opmode, radio_idx = 0, tc_downlimit = 0, tc_uplimit = 0;
    bool tc_enabled = 0, tc_downperuser = 0, tc_upperuser = 0;
    int downperuser = 0, upperuser = 0;

    char *section = ssidCfg_p->section;
    struct json_object *jobj;
    ResponseStatus *res = rep->res;
    int idx = ssidCfg_p->idx;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            api_get_wifi_ifacex_tc_downperuser_option_by_sectionname(opmode, section, idx, &downperuser);
            api_get_wifi_ifacex_tc_upperuser_option_by_sectionname(opmode, section, idx, &upperuser);
            tc_downperuser = (downperuser == 1)?true:false;
            tc_upperuser = (upperuser == 1)?true:false;

            if (json_object_object_get(jobj, "enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "enable",&(tc_enabled));
                if(api_set_integer_option2(tc_enabled, WIRELESS_WIFI_OPTION_FORMAT_WITH_INDEX, section, idx, "tc_enabled"))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TRAFFIC SHAPING ENABLE");
                }
            }
            if (json_object_object_get(jobj, "download_limit") != NULL)
            {
                senao_json_object_get_integer(jobj, "download_limit", &(tc_downlimit));
                if ( tc_downlimit < 1 || tc_downlimit > 999 )
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DOWNLOAD LIMIT");
                }
                if ( tc_downperuser )
                {
                    if(api_set_wifi_ifacex_tc_downlimit_option_by_sectionname(opmode, section, idx, tc_downlimit))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DOWNLOAD LIMIT");
                    }
                }
                else
                {
                    if(api_set_wifi_ifacex_tc_downmaxlimit_option_by_sectionname(opmode, section, idx, tc_downlimit))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DOWNLOAD LIMIT");
                    }
                }
            }
            if (json_object_object_get(jobj, "upload_limit") != NULL)
            {
                senao_json_object_get_integer(jobj, "upload_limit",&(tc_uplimit));
                if ( tc_uplimit < 1 || tc_uplimit > 999 )
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "UPLOAD LIMIT");
                }

                if ( tc_upperuser )
                {
                    if(api_set_wifi_ifacex_tc_uplimit_option_by_sectionname(opmode, section, idx, tc_uplimit))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "UPLOAD LIMIT");
                    }
                }
                else
                {
                    if(api_set_wifi_ifacex_tc_upmaxlimit_option_by_sectionname(opmode, section, idx, tc_uplimit))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "UPLOAD LIMIT");
                    }
                }
            }
            if (json_object_object_get(jobj, "perclient_download_limit") != NULL)
            {
                senao_json_object_get_boolean(jobj, "perclient_download_limit",&(tc_downperuser));
                if(api_set_wifi_ifacex_tc_downperuser_option_by_sectionname(opmode, section, idx, tc_downperuser))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PERCLIENT DOWNLOAD LIMIT");
                }
            }
            if (json_object_object_get(jobj, "perclient_upload_limit") != NULL)
            {
                senao_json_object_get_boolean(jobj, "perclient_upload_limit",&(tc_upperuser));

                if(api_set_wifi_ifacex_tc_upperuser_option_by_sectionname(opmode, section, idx, tc_upperuser))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PERCLIENT UPLOAD LIMIT");
                }
            }
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
int json_patch_wireless_l2_acl(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){
    int opmode = ssidCfg_p->opmode, radio_idx = 0, mac_filter_num = 0, macfilter_index = 0;
    bool acl_enable = 0;
    char *section = ssidCfg_p->section;
    char *macfilter=NULL, *maclist=NULL, policy[ACL_MACFILTER_LENGTH] = {0};
    struct json_object *jobj;
    ResponseStatus *res = rep->res;
    int idx = ssidCfg_p->idx;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {

            api_get_wifi_ifacex_macfilter_option_by_sectionname(opmode, section,idx, &macfilter_index);
            acl_enable = (macfilter_index == 0)?false:true;

            if (json_object_object_get(jobj, "enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "enable",&(acl_enable));
                if (acl_enable == false) 
                {
                    if(api_set_wifi_ifacex_macfilter_option_by_sectionname(opmode, section, idx, 0))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "POLICY");
                    }
                }
            }
            if (json_object_object_get(jobj, "policy") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "policy", &macfilter);
                if (strcmp(macfilter, DESC_ACL_ALLOW) == 0)
                {
                    macfilter_index = 1;
                    if(api_set_wifi_ifacex_macfilter_option_by_sectionname(opmode, section, idx, 1))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "POLICY");
                    }
                }
                else if(strcmp(macfilter, DESC_ACL_DENY) == 0)
                {
                    macfilter_index = 2;
                    if(api_set_wifi_ifacex_macfilter_option_by_sectionname(opmode, section, idx, 2))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "POLICY");
                    }
                }
            }
            if (json_object_object_get(jobj, "client_mac_list") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "client_mac_list", &maclist);
                if ( maclist && strlen(maclist) > 0 )
                {
                    api_get_integer_option("functionlist.vendorlist.MAX_WLAN_MAC_FILTER_NUMBER", &mac_filter_num);
                    if ( mac_filter_num == 0 )
                        mac_filter_num = 32; // default is 32

                    if ( strlen(maclist) > (mac_filter_num*18-1) )
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "client_mac_list too long");
                }
                if (macfilter_index == 1)
                {
                    //strcpy(policy, "allow");
                    if ( strstr(section,"enjet") )
                    {
                        api_set_string_option2(maclist, sizeof(maclist), "%s.allowmaclist", section);
                    }
                    else
                    {
                        api_set_string_option2(maclist, sizeof(maclist), "%s_%d.allowmaclist", section, idx);
                    }

                }
                else if(macfilter_index == 2)
                {
                    if ( strstr(section,"enjet") )
                    {
                        api_set_string_option2(maclist, sizeof(maclist), "%s.denymaclist", section);
                    }
                    else
                    {
                        api_set_string_option2(maclist, sizeof(maclist), "%s_%d.denymaclist", section, idx);
                    }
                }
            }
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

int json_patch_wireless_scheduling_sync(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p)
{
    int ret = 0;
    int idx = ssidCfg_p->idx;
    ResponseStatus *res = rep->res;

    if ( ssidCfg_p->opmode != OPM_AP && ssidCfg_p->opmode != OPM_WDSAP && ssidCfg_p->opmode != -1)
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "THIS OPMODE NOT SUPPORT WIFI SCHEDULING");

    // always set to wifi0 now
    if ( RADIO_MODE % 5 == 2 )
        ret = ret || json_patch_wireless_scheduling(rep, query_str, ssidCfg_p, WIFI_RADIO_NAME_24G);

    if ( RADIO_MODE / 5  >= 1 )
        ret = ret || json_patch_wireless_scheduling(rep, query_str, ssidCfg_p, WIFI_RADIO_NAME_5G);

    if ( RADIO_MODE / 5 >= 2 )
        ret = ret || json_patch_wireless_scheduling(rep, query_str, ssidCfg_p, WIFI_RADIO_NAME_5G_2);

    return ret;
}

int json_patch_wireless_scheduling(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char* radio)
{
    struct json_object *jobj;
    int opmode = ssidCfg_p->opmode;
    char *days=NULL;
    bool enable = 0;
    int radio_idx = 0;
    char iface[6] = {0};
    int idx = ssidCfg_p->idx;
    int wifitbl_idx = 0;

    if ( strcmp(radio, "2_4G") == 0) 
    {
        radio_idx = 0;
    }
    else if(strcmp(radio, "5G") == 0) 
    {
        radio_idx = 1;
    }
    else
    {
        radio_idx = 4;
    }

    if (opmode == OPM_AP)
    {
        if(idx == 1) // ath0, ath1
        {
            sprintf(iface, "ath%d", radio_idx);
        } 
        else 
        {
            // ath01, ath02 ...
            sprintf(iface, "ath%d%d", radio_idx, idx-1);
        }
    }
    else if (opmode == OPM_WDSAP)
    {
        //If WDS-AP mode, it use 2.4G:ath20~ath23, 5G:ath50~ath53
        if (radio_idx == 1) 
        {
            sprintf(iface, "ath5%d", idx-1);
        }
        else
        {
            sprintf(iface, "ath2%d", idx-1);
        }
    }
    else // enjet
    {
        sprintf(iface, "enjet%d", radio_idx);
    }

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (json_object_object_get(jobj, "enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "enable", &enable);
                api_set_wifi_schedule_enable_option_by_sectionname(enable);
                if ( enable )
                {
                    api_set_wifi_schedule_radio_option(WIFI_SCHEDULE_WIRELESS_RADIO_OPTION,radio_idx);
                    api_set_wifi_schedule_ifname_option_by_sectionname(WIFI_SCHEDULE_WIRELESS_IFACE_OPTION,iface,sizeof(iface));
                    api_set_wifi_schedule_templates_option_by_sectionname(WIFI_SCHEDULE_WIRELESS_TEMPLATES_OPTION, CUSTOMSCHEDULE);
                    if(api_find_wifi_schedule_index_by_ifname(iface, &wifitbl_idx) == API_RC_INTERNAL_ERROR){
                        api_setup_wifi_schedule_templates(CUSTOMSCHEDULE, iface);
                    }
                }
            }
            if (json_object_object_get(jobj, "days") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "days", &days);
                return json_patch_wireless_scheduling_days(rep, days, ssidCfg_p, radio);

            }
        }
    }
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    return 0;
}
int json_patch_wireless_scheduling_days(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char *radio)
{
    struct json_object *jobj;
    bool enable = 0;
    char *days=NULL;
    int i;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            for ( i = 0 ; i < scheduling_days_length ; i++ )
            {
                days = NULL;
                if (json_object_object_get(jobj, scheduling_days[i]) != NULL)
                {
                    senao_json_object_get_and_create_string(rep, jobj, scheduling_days[i], &days);
                    json_patch_wireless_scheduling_day(rep,  days, ssidCfg_p, radio, i);
                }
            }
        }
    }
    return API_RC_SUCCESS;
}

int json_patch_wireless_scheduling_day(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char *radio, int dayIdx)
{
    struct json_object *jobj = NULL;
    bool enable = 0;
    char day[160] = {0};
    char *start=NULL;
    char *end=NULL;
    char iface[6] = {0};
    int i = 0, radio_idx = 0, wifitbl_idx = 0, schedule_idx = 0;
    int ret = API_RC_SUCCESS;
    int idx = ssidCfg_p->idx;
    int opmode = ssidCfg_p->opmode;
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);

    if ( strcmp(radio, "2_4G") == 0) 
    {
        radio_idx = 0;
    }
    else if(strcmp(radio, "5G") == 0) 
    {
        radio_idx = 1;
    }
    else
    {
        radio_idx = 4;
    }

    if (opmode == OPM_AP)
    {
        if(idx == 1) // ath0, ath1
        {
            sprintf(iface, "ath%d", radio_idx);
        }
        else
        {
            // ath01, ath02 ...
            sprintf(iface, "ath%d%d", radio_idx, idx-1);
        }
    }
    else if (opmode == OPM_WDSAP)
    {
        //If WDS-AP mode, it use 2.4G:ath20~ath23, 5G:ath50~ath53
        if (radio_idx == 1)
        {
            sprintf(iface, "ath5%d", idx-1);
        }
        else
        {
            sprintf(iface, "ath2%d", idx-1);
        }
    }
    else // enjet
    {
        sprintf(iface, "enjet%d", radio_idx);
    }

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            api_find_wifi_schedule_index_by_ifname(iface, &wifitbl_idx);
            schedule_idx = wifitbl_idx+dayIdx;

            if (json_object_object_get(jobj, "available") != NULL)
            {
                senao_json_object_get_boolean( jobj, "available", &enable);
                api_set_wifi_schedule_tablex_status_option(schedule_idx, enable);
            }
            if (json_object_object_get(jobj, "start") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "start", &start);
                api_set_wifi_schedule_tablex_start_time_option(schedule_idx, start, strlen(start));
            }
            if (json_object_object_get(jobj, "end") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "end", &end);
                api_set_wifi_schedule_tablex_end_time_option(schedule_idx, end, strlen(end));
            }
        }
    }
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    return ret;
}

int json_patch_wireless_security(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){
    int opmode = ssidCfg_p->opmode, radio_idx = 0, encr_type = 0, security_mode = 0, key_interval = 0, nasPort = 0, ieee80211w = 0;
    bool fastroaming_enable = false, proxyarp_enable = false, nasId_enable = false, nasIp_enable = false, nasPort_enable = false, enterprise = false, suiteb = false;
    char *auth_type=NULL, *encryption=NULL, *jsonStr=NULL, *radius_jsonStr=NULL, *accounting_jsonStr=NULL;
    char *section = ssidCfg_p->section, *nasIp=NULL, *nasid=NULL;
    int idx = ssidCfg_p->idx,ret = 0;
    char server1Ip[IP_LENGTH] = {0}, server1Secret[65] = {0};
    struct json_object *jobj;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (json_object_object_get(jobj, "encryption") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "encryption", &encryption);

                auth_type = "AES";
                if ( strcmp( encryption, "WPA-PSK" ) == 0 )
                {
                    if ( strcmp( auth_type, "AES" ) == 0 )
                        encr_type = WPA_PSK_CCMP;
                    else if ( strcmp( auth_type, "TKIP" ) == 0 )
                        encr_type = WPA_PSK_TKIP;
                    else
                        encr_type = WPA_PSK_TKIP_CCMP;
                }
                else if ( strcmp( encryption, "WPA2-PSK" ) == 0 )
                {
                    if ( strcmp( auth_type, "AES" ) == 0 )
                        encr_type = WPA2_PSK_CCMP;
                    else if ( strcmp( auth_type, "TKIP" ) == 0 )
                        encr_type = WPA2_PSK_TKIP;
                    else
                        encr_type = WPA2_PSK_TKIP_CCMP;
                }
                else if ( strcmp( encryption, "WPA-PSK-Mixed" ) == 0 )
                {
                    if ( strcmp( auth_type, "AES" ) == 0 )
                        encr_type = WPA_PSK_MIXED_CCMP;
                    else if ( strcmp( auth_type, "TKIP" ) == 0 )
                        encr_type = WPA_PSK_MIXED_TKIP;
                    else
                        encr_type = WPA_PSK_MIXED_TKIP_CCMP;
                }
                else if ( strcmp( encryption, "WPA-Enterprise" ) == 0 )
                {
                    if ( strcmp( auth_type, "AES" ) == 0 )
                        encr_type = WPA_EAP_CCMP;
                    else if ( strcmp( auth_type, "TKIP" ) == 0 )
                        encr_type = WPA_EAP_TKIP;
                    else
                        encr_type = WPA_EAP_TKIP_CCMP;
                    enterprise = true;
                }
                else if ( strcmp( encryption, "WPA2-Enterprise" ) == 0 )
                {
                    if ( strcmp( auth_type, "AES" ) == 0 )
                        encr_type = WPA2_EAP_CCMP;
                    else if ( strcmp( auth_type, "TKIP" ) == 0 )
                        encr_type = WPA2_EAP_TKIP;
                    else
                        encr_type = WPA2_EAP_TKIP_CCMP;
                    enterprise = true;
                }
                else if ( strcmp( encryption, "WPA-Mixed-Enterprise" ) == 0 )
                {
                    if ( strcmp( auth_type, "AES" ) == 0 )
                        encr_type = WPA_EAP_MIXED_CCMP;
                    else if ( strcmp( auth_type, "TKIP" ) == 0 )
                        encr_type = WPA_EAP_MIXED_TKIP;
                    else
                        encr_type = WPA_EAP_MIXED_TKIP_CCMP;
                    enterprise = true;
                }
#if SUPPORT_WPA3
                else if ( strcmp( encryption, "WPA3-SAE" ) == 0 )
                {
                    encr_type = WPA3_SAE_CCMP;
                }
                else if ( strcmp( encryption, "WPA3-SAE-Mixed" ) == 0 )
                {
                    encr_type = WPA3_SAE_MIXED_CCMP;
                }
                else if ( strcmp( encryption, "WPA3-Enterprise" ) == 0 )
                {
                    encr_type = WPA3_EAP_CCMP;
                    enterprise = true;
                }
                else if ( strcmp( encryption, "WPA3-Mixed-Enterprise" ) == 0 )
                {
                    encr_type = WPA3_EAP_MIXED_CCMP;
                    enterprise = true;
                }
                else if ( strcmp( encryption, "OWE" ) == 0 )
                {
                    encr_type = ENCRYPTION_OWE_CCMP;
                }
#endif
                else
                {
                    encr_type = ENCRYPTION_NONE;
                }

                if(api_set_wifi_ifacex_encryption_option_by_sectionname(opmode, section, idx, encr_type))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ENCRYPTION");
                }
                if(encr_type == ENCRYPTION_NONE)
                {
                    if(api_set_wifi_ifacex_fastroaming_enable_option_by_sectionname(opmode, section, idx, 0))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "FAST ROAMING");
                    }
                }
#if SUPPORT_WPA3
                if(senao_json_object_get_boolean(jobj, "suiteb_192bits",&(suiteb)))
                {
                    if((encr_type != WPA3_EAP_CCMP) && (suiteb != false))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SUITEB_192BITS");
                    }
                    if(api_set_wifi_ifacex_suiteb_option_by_sectionname(opmode, section, idx, suiteb))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SUITEB_192BITS");
                    }
                }
                else if(encr_type != WPA3_EAP_CCMP)
                {
                    if(api_set_wifi_ifacex_suiteb_option_by_sectionname(opmode, section, idx, suiteb))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SUITEB_192BITS");
                    }
                }
                if(api_set_wifi_ifacex_encryption_wpa_option_by_sectionname(opmode, section, idx, encr_type))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ENCRYPTION");
                }
#endif
            }
#if SUPPORT_WPA3
                if (json_object_object_get(jobj, "ieee80211w") != NULL)
                {
                    senao_json_object_get_integer(jobj, "ieee80211w", &ieee80211w);
                    if(api_set_wifi_ifacex_wpa_ieee80211w_option_by_sectionname(opmode, section, idx, ieee80211w))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IEEE80211W");
                    }
                }
#endif
            if (json_object_object_get(jobj, "auth_type") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "auth_type", &auth_type);
            }
            if (json_object_object_get(jobj, "wpa") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "wpa", &jsonStr);
                if((ret = json_patch_wireless_encryption(rep, jsonStr, ssidCfg_p)) != API_SUCCESS)
                {
                    return ret;
                }
            }
            if (json_object_object_get(jobj, "key_interval") != NULL)
            {
                senao_json_object_get_integer(jobj, "key_interval", &key_interval);
                if(api_set_wifi_ifacex_wpa_group_rekey_option_by_sectionname(opmode, section, idx, key_interval))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "KEY INTERVAL");
                }
            }
            if (json_object_object_get(jobj, "nasId_enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "nasId_enable",&(nasId_enable));
                api_set_wifi_ifacex_nasid_enabled_option_by_sectionname(opmode, section, idx, nasId_enable);
            }
            if (json_object_object_get(jobj, "nasId") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "nasId", &nasid);
                if (api_set_wifi_ifacex_nasid_option_by_sectionname(opmode, section, idx, nasid, sizeof(nasid)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NAS ID");
                }
            }
            if (json_object_object_get(jobj, "nasIp_enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "nasIp_enable",&(nasIp_enable));
                api_set_wifi_ifacex_nasip_enabled_option_by_sectionname(opmode, section, idx, nasIp_enable);

            }
            if (json_object_object_get(jobj, "nasIp") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "nasIp", &nasIp);
                if (nasIp_enable == true) 
                {
                    if(api_set_wifi_ifacex_nasip_option_by_sectionname(opmode, section, idx, nasIp, sizeof(nasIp)))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NAS IP");
                    }
                }
            }
            if (json_object_object_get(jobj, "nasPort_enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "nasPort_enable",&(nasPort_enable));
                api_set_wifi_ifacex_nasport_enabled_option_by_sectionname(opmode, section, idx, nasPort_enable);

            }
            if (json_object_object_get(jobj, "nasPort") != NULL)
            {
                senao_json_object_get_integer(jobj, "nasPort",&(nasPort));
                if (nasPort_enable == true) 
                {
                    if(api_set_wifi_ifacex_nasport_option_by_sectionname(opmode, section, idx, nasPort))
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NAS PORT");
                    }
                }
            }
            if (json_object_object_get(jobj, "proxyarp") != NULL)
            {
                senao_json_object_get_boolean(jobj, "proxyarp",&(proxyarp_enable));
                if(api_set_wifi_ifacex_proxyarp_enable_option_by_sectionname(opmode, section, idx, proxyarp_enable))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "FAST ROAMING");
                }
            }
            if (json_object_object_get(jobj, "fast_roaming") != NULL)
            {
                senao_json_object_get_boolean(jobj, "fast_roaming",&(fastroaming_enable));
                if(api_set_wifi_ifacex_fastroaming_enable_option_by_sectionname(opmode, section, idx, fastroaming_enable))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "FAST ROAMING");
                }
            }
            if (json_object_object_get(jobj, "accounting_server") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "accounting_server", &accounting_jsonStr);
                if((ret = json_patch_wireless_accounting_server(rep, accounting_jsonStr, ssidCfg_p)) != API_SUCCESS)
                {
                    return ret;
                }
            }
            if (json_object_object_get(jobj, "radius_server") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "radius_server", &radius_jsonStr);
                if((ret = json_patch_wireless_radius_server(rep, radius_jsonStr, ssidCfg_p)) != API_SUCCESS)
                {
                    return ret;
                }
            }
            else
            {
                if (enterprise == true)
                {
                    api_get_wifi_ifacex_auth_server_option_by_sectionname(opmode, section, idx, server1Ip, sizeof(server1Ip));
                    api_get_wifi_ifacex_auth_secret_option_by_sectionname(opmode, section, idx, server1Secret, sizeof(server1Secret));
                    if (strcmp(server1Ip, "") == 0) 
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 IP");
                    }
                    if (strcmp(server1Secret, "") == 0) 
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 SECRET");
                    }
                }
            }
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

int json_patch_wireless_accounting_server(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){
    int opmode = ssidCfg_p->opmode, radio_idx = 0, server1Port = 0, server2Port = 0, nasPort = 0, acct_interval = 0;
    bool acct_enable = 0;
    char *server1Ip=NULL, *server1Secret=NULL, *server2Ip=NULL, *server2Secret=NULL;
    char *nasid=NULL;
    char *section = ssidCfg_p->section;
    struct json_object *jobj;
    ResponseStatus *res = rep->res;
    int idx = ssidCfg_p->idx;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (json_object_object_get(jobj, "enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "enable",&(acct_enable));
                if(api_set_wifi_ifacex_acct_enabled_option_by_sectionname(opmode, section, idx, acct_enable))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ACCOUNTING SERVER ENABLE");
                }
            }
            if (json_object_object_get(jobj, "interval") != NULL)
            {
                senao_json_object_get_integer(jobj, "interval",&(acct_interval));
                if(api_set_wifi_ifacex_acct_interval_option_by_sectionname(opmode, section, idx, acct_interval))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ACCT INTERVAL");
                }
            }
            if (json_object_object_get(jobj, "server1Ip") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "server1Ip", &server1Ip);
                if(api_set_wifi_ifacex_acct_server_option_by_sectionname(opmode, section, idx, server1Ip, sizeof(server1Ip)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 IP");
                }
            }
            if (json_object_object_get(jobj, "server1Port") != NULL)
            {
                senao_json_object_get_integer(jobj, "server1Port",&(server1Port));
                if(api_set_wifi_ifacex_acct_port_option_by_sectionname(opmode, section, idx, server1Port))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 PORT");
                }
            }
            if (json_object_object_get(jobj, "server1Secret") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "server1Secret", &server1Secret);
                if(api_set_wifi_ifacex_acct_secret_option_by_sectionname(opmode, section, idx, server1Secret, sizeof(server1Secret)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 SECRET");
                }
            }
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

int json_patch_wireless_encryption(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){
    int opmode = ssidCfg_p->opmode, radio_idx = 0, encr_type = 0;
    char *passphrase=NULL;
    char *section = ssidCfg_p->section;
    char cipherType[CIPERTYPE_LENGTH] = {0};
    struct json_object *jobj;
    ResponseStatus *res = rep->res;
    int idx = ssidCfg_p->idx;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (json_object_object_get(jobj, "passphrase") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "passphrase", &passphrase);
                if(api_set_wifi_ifacex_wpakey_key_option_by_sectionname(opmode, section, idx, passphrase, sizeof(passphrase)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSPHRASE");
                }
            }
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

int json_patch_wireless_radius_server(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){

    int opmode = ssidCfg_p->opmode, radio_idx = 0, retries = 0, server1Port = 0, server2Port = 0, encr_type = 0;
    char *server1Ip=NULL, *server1Secret=NULL, *server2Ip=NULL, *server2Secret=NULL;
    char *section = ssidCfg_p->section;
    struct json_object *jobj;
    ResponseStatus *res = rep->res;
    int idx = ssidCfg_p->idx;
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if(api_get_wifi_ifacex_encryption_option_by_sectionname(opmode, section, idx, &encr_type))
            {
                return FALSE;
            }
            if (json_object_object_get(jobj, "server1Ip") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "server1Ip", &server1Ip);
                if(api_set_wifi_ifacex_auth_server_option_by_sectionname(opmode, section, idx, server1Ip, sizeof(server1Ip)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 IP");
                }
            }
            if (json_object_object_get(jobj, "server1Port") != NULL)
            {
                senao_json_object_get_integer(jobj, "server1Port",&(server1Port));
                if(api_set_wifi_ifacex_auth_port_option_by_sectionname(opmode, section, idx, server1Port))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 PORT");
                }
            }
            if (json_object_object_get(jobj, "server1Secret") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "server1Secret", &server1Secret);
                if(api_set_wifi_ifacex_auth_secret_option_by_sectionname(opmode, section, idx, server1Secret, sizeof(server1Secret)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 SECRET");
                }
            }
            if (encr_type > 11 && server1Ip == NULL) // enterprise
            {
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 IP");
            }
            if (encr_type > 11 && server1Secret == NULL)
            {
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 SECRET");
            }
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

int json_patch_wifi_mgmt(ResponseEntry *rep, char *query_str)
{
    bool enable = false;
    char *ssid, *passphrase, radio[6]={0}, section[32]={0};
    int mgmt_radio = 0,livetime=0;
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

    api_get_integer_option("functionlist.vendorlist.MANAGEMENT_SUPPORT_RADIO", &mgmt_radio);

    switch (mgmt_radio)
    {
        case 1:
            snprintf(radio, sizeof(radio), "wifi0");
            break;
        case 2:
            snprintf(radio, sizeof(radio), "wifi1");
            break;
        case 4:
            snprintf(radio, sizeof(radio), "wifi2");
            break;
        default:
            snprintf(radio, sizeof(radio), "wifi0");
            break;
    }

    snprintf(section, sizeof(section), "wireless.%s_mgmt", radio);

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            if (json_object_object_get(jobj, "enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "enable",&(enable));
                if (api_set_wifi_ifacex_disabled_option_by_sectionname(OPM_ALL, section, 0, (enable?0:1)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ENABLE");
                }
            }
            if (json_object_object_get(jobj, "ssid_name") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "ssid_name", &ssid);
                if(api_set_wifi_ifacex_ssid_option_by_sectionname(OPM_ALL, section, 0, ssid, sizeof ssid))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SSID NAME");
                }
            }
            if (json_object_object_get(jobj, "passphrase") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "passphrase", &passphrase);
                if(api_set_wifi_ifacex_wpakey_key_option_by_sectionname(OPM_ALL, section, 0, passphrase, sizeof passphrase))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSPHRASE");
                }
            }
            if (json_object_object_get(jobj, "livetime") != NULL)
            {
                senao_json_object_get_boolean(jobj, "livetime",&(livetime));
                if (api_set_wifi_mgmt_livetime_option_by_sectionname(OPM_ALL, section, 0,(livetime?15:0)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Livetime");
                }
            }
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


int json_patch_wifi_sta_mode(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p)
{
    int i = 0, prebssid = 0, mgmt_radio = 0, encr_type = 0, eap_type = 0, eap_auth = 0, default_key = 0, ieee80211w = 0;
    bool prebssid_enable = 0;
    char *ssid = NULL, *bssid = NULL, *passphrase = NULL, *auth_identity = NULL, *auth_type = NULL, *input_type = NULL, *key = NULL, *key_length = NULL;
    char *eap_type_mapping = NULL, *eap_auth_mapping = NULL, *auth_password = NULL ,*encryption = NULL, *jobj_wep_str = NULL, *jobj_wpa_str = NULL, *jobj_enterprise_str = NULL;
    char tmp[8] = {0};
    struct json_object *jobj = NULL, *jobj_wpa = NULL, *jobj_enteriprise = NULL, *jobj_wep = NULL;
    ResponseStatus *res = rep->res;
    char *section = ssidCfg_p->section;
    int opmode = ssidCfg_p->opmode;
    char security_str[32] = {0};

#if defined(SUPPORT_AP_RP_SETUP_WIZARD) || defined(SUPPORT_RP_SSID_SETTING)
	int erp_encr_type = 0, erp_encr_sta = 0, erp_default_key = 0;
	bool erp_enable = 0;
	char *erp_ssid = NULL, *erp_passphrase = NULL, *erp_encryption = NULL, *jobj_erp_str = NULL;
	char erp_ssid_sta[SSID_NAME_LENGTH]={0}, erp_key_sta[65]={0};
	struct json_object *jobj_erp = NULL;
#endif

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            api_get_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 2, &encr_type);

            if (json_object_object_get(jobj, "ssid_name") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "ssid_name", &ssid);
                if (opmode == OPM_RP) 
                {
                    api_set_wifi_ifacex_ssid_option_by_sectionname(opmode, section, 1, ssid, strlen(ssid));
                }
                api_set_wifi_ifacex_ssid_option_by_sectionname(opmode, section, 2, ssid, strlen(ssid));
            }
            if (json_object_object_get(jobj, "prebssid_enable") != NULL)
            {
                senao_json_object_get_boolean(jobj, "prebssid_enable",&(prebssid_enable));
                prebssid = (prebssid_enable == true)?1:0;
                api_set_wifi_ifacex_PreferBSSIDEnable_option_by_sectionname(opmode, section, 2, prebssid);
            }
            if (json_object_object_get(jobj, "encryption") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "encryption", &encryption);

                if (strcmp(encryption, "WEP" ) == 0)
                {
                    snprintf(security_str, sizeof(security_str), "wep-open");
                    encr_type = 1;
                    if (json_object_object_get(jobj, "wep") == NULL)
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH TYPE");
                    }
                }
                else if (strcmp(encryption, "WPA-PSK" ) == 0)
                {
                    snprintf(security_str, sizeof(security_str), "psk+ccmp");
                    encr_type = 3; // psk+ccmp or psk+tkip
                    if (json_object_object_get(jobj, "wpa") == NULL)
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH TYPE");
                    }
                }
                else if (strcmp(encryption, "WPA2-PSK" ) == 0)
                {
                    snprintf(security_str, sizeof(security_str), "psk2+ccmp");
                    encr_type = 5; // psk2+ccmp or psk2+tkip
                    ieee80211w = 1;
                    if (json_object_object_get(jobj, "wpa") == NULL)
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH TYPE");
                    }
                }
                else if (strcmp(encryption, "WPA-Enterprise" ) == 0)
                {
                    snprintf(security_str, sizeof(security_str), "wpa");
                    encr_type = 7; // wpa
                }
                else if (strcmp(encryption, "WPA2-Enterprise" ) == 0)
                {
                    snprintf(security_str, sizeof(security_str), "wpa2");
                    encr_type = 8; // wpa2
                    ieee80211w = 1;
                }
                else
                {
                    snprintf(security_str, sizeof(security_str), "none");
                    encr_type = ENCRYPTION_NONE;
                }
                if (opmode == OPM_RP && encr_type != 8) 
                {
                    api_set_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 1, encr_type);
#if SUPPORT_WPA3
                    api_set_wifi_ifacex_wpa3_security_option_by_sectionname(opmode, section, 1, security_str);
#endif
                }
                else if (opmode == OPM_RP)
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ENCRYPTION");
                }
                if (encr_type == 0 || encr_type == 7 || encr_type == 8) 
                {
                    api_set_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 2, encr_type);
#if SUPPORT_WPA3
                    api_set_wifi_ifacex_wpa3_security_option_by_sectionname(opmode, section, 2, security_str);
                    api_set_wifi_ifacex_wpa_ieee80211w_option_by_sectionname(opmode, section, 2, ieee80211w);
#endif
                }
            }
            if (json_object_object_get(jobj, "wep") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "wep", &jobj_wep_str);
                if(NULL != jobj_wep_str)
                {
                    if((jobj_wep = jsonTokenerParseFromStack(rep, jobj_wep_str)))
                    {
                        if (json_object_object_get(jobj_wep, "auth_type") != NULL)
                        {
                            senao_json_object_get_and_create_string(rep, jobj_wep, "auth_type", &auth_type);
                            if (encr_type == 1 || encr_type == 2)
                            {

                                if(strcmp(auth_type, "open") == 0)
                                {
                                    snprintf(security_str, sizeof(security_str), "wep-open");
                                    encr_type = 1;
                                }
                                else if(strcmp(auth_type, "shared") == 0)
                                {
                                    snprintf(security_str, sizeof(security_str), "wep-shared");
                                    encr_type = 2;
                                }
#if SUPPORT_WPA3
                                api_set_wifi_ifacex_wpa3_security_option_by_sectionname(opmode, section, 2, security_str);
#endif
                                api_set_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 2, encr_type);
                                api_set_wifi_ifacex_wpa_ieee80211w_option_by_sectionname(opmode, section, 2, ieee80211w);
                                if (opmode == OPM_RP)
                                {
                                    api_set_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 1, encr_type);
#if SUPPORT_WPA3
                                    api_set_wifi_ifacex_wpa3_security_option_by_sectionname(opmode, section, 1, security_str);
#endif
                                }
                            }
                        }
                        if (json_object_object_get(jobj_wep, "default_key") != NULL)
                        {
                            senao_json_object_get_integer(jobj_wep, "default_key", &default_key);
                            api_set_wifi_ifacex_wepkey_id_option_by_sectionname(opmode, section, 2, default_key);
                            if (opmode == OPM_RP)
                            {
                                api_set_wifi_ifacex_wepkey_id_option_by_sectionname(opmode, section, 1, default_key);
                            }
                        }
                        for ( i=1; i<5; i++) 
                        {
                            sprintf(tmp,"key%d",i);
                            key = "";
                            if (json_object_object_get(jobj_wep, tmp) != NULL)
                            {
                                senao_json_object_get_and_create_string(rep, jobj_wep, tmp, &key);
                                if(api_set_wifi_ifacex_wepkey_keyx_option_by_sectionname(opmode, section, 2, i, key, sizeof key))
                                {
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "KEY");
                                }
                                if (opmode == OPM_RP)
                                {
                                    api_set_wifi_ifacex_wepkey_keyx_option_by_sectionname(opmode, section, 1, i, key, sizeof key);
                                }
                            }
                        }
                    }
                    else
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
                    }
                }
            }
            if (json_object_object_get(jobj, "wpa") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "wpa", &jobj_wpa_str);
                if(NULL != jobj_wpa_str)
                {
                    if((jobj_wpa = jsonTokenerParseFromStack(rep, jobj_wpa_str)))
                    {
                        if (json_object_object_get(jobj_wpa, "passphrase") != NULL)
                        {
                            senao_json_object_get_and_create_string(rep, jobj_wpa, "passphrase", &passphrase);
                            if (opmode == OPM_RP) 
                            {
                                if(api_set_wifi_ifacex_wpakey_key_option_by_sectionname(opmode, section, 1, passphrase, sizeof(passphrase)))
                                {
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSPHRASE");
                                }
                                //api_set_string_option2(passphrase, sizeof(passphrase), "%s_1.%s", section, "key");
                                api_set_string_option2(passphrase, sizeof(passphrase), "%s_2.%s", section, "key");

                            }
                            else
                            {
                                if(api_set_wifi_ifacex_wpakey_key_option_by_sectionname(opmode, section, 1, passphrase, sizeof(passphrase)))
                                {
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSPHRASE");
                                }
                            }
                        }
                        if (json_object_object_get(jobj_wpa, "auth_type") != NULL)
                        {
                            senao_json_object_get_and_create_string(rep, jobj_wpa, "auth_type", &auth_type);
                            if (encr_type == 3 || encr_type == 4) 
                            {
                                if (strcmp(auth_type, "AES") == 0)
                                {
                                    snprintf(security_str, sizeof(security_str), "psk+ccmp");
                                    encr_type = 3;
                                }
                                else if(strcmp(auth_type, "TKIP") == 0)
                                {
                                    snprintf(security_str, sizeof(security_str), "psk+tkip");
                                    encr_type = 4;
                                }
                            }
                            if (encr_type == 5 || encr_type == 6) 
                            {
                                if (strcmp(auth_type, "AES") == 0)
                                {
                                    snprintf(security_str, sizeof(security_str), "psk2+ccmp");
                                    encr_type = 5;
                                }
                                else if(strcmp(auth_type, "TKIP") == 0)
                                {
                                    snprintf(security_str, sizeof(security_str), "psk2+tkip");
                                    encr_type = 6;
                                }
                            }
                            api_set_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 2, encr_type);
#if SUPPORT_WPA3
                            api_set_wifi_ifacex_wpa3_security_option_by_sectionname(opmode, section, 2, security_str);
#endif
                            api_set_wifi_ifacex_wpa_ieee80211w_option_by_sectionname(opmode, section, 2, ieee80211w);
                        }
                    }
                    else
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
                    }
                }
            }
            if (json_object_object_get(jobj, "enterprise") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "enterprise", &jobj_enterprise_str);
                if(NULL != jobj_enterprise_str)
                {
                    if((jobj_enteriprise = jsonTokenerParseFromStack(rep, jobj_enterprise_str)))
                    {
                        api_get_wifi_ifacex_eap_auth_option_by_sectionname(opmode, section, 1, &eap_auth);

                        if (json_object_object_get(jobj_enteriprise, "eap_auth") != NULL)
                        {
                            senao_json_object_get_and_create_string(rep, jobj_enteriprise, "eap_auth", &eap_auth_mapping);
                            if (strcmp(eap_auth_mapping, "MSCHAP") == 0)
                            {
                                eap_auth = 0;
                            }
                            else if (strcmp(eap_auth_mapping, "MSCHAPV2") == 0)
                            {
                                eap_auth = 1;
                            }
                            api_set_wifi_ifacex_eap_auth_option_by_sectionname(opmode, section, 0, eap_auth);

                        }
                        if (json_object_object_get(jobj_enteriprise, "eap_method") != NULL)
                        {
                            senao_json_object_get_and_create_string(rep, jobj_enteriprise, "eap_method", &eap_type_mapping);
                            if (strcmp(eap_type_mapping, "ttls") == 0)
                            {
                                eap_type = 2;
                            }
                            else if (strcmp(eap_type_mapping, "peap") == 0)
                            {
                                eap_type = 1;
                                if (eap_auth == 0) 
                                {
                                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "EAP_AUTH");
                                }
                            }
                            api_set_wifi_ifacex_eap_type_option_by_sectionname(opmode, section, 0, eap_type);

                        }
                        if (json_object_object_get(jobj_enteriprise, "auth_identity") != NULL)
                        {
                            senao_json_object_get_and_create_string(rep, jobj_enteriprise, "auth_identity", &auth_identity);
                            api_set_wifi_ifacex_eap_auth_identity_option_by_sectionname(opmode, section, 0, auth_identity, strlen(auth_identity));

                        }
                        if (json_object_object_get(jobj_enteriprise, "auth_password") != NULL)
                        {
                            senao_json_object_get_and_create_string(rep, jobj_enteriprise, "auth_password", &auth_password);
                            api_set_wifi_ifacex_eap_auth_password_option_by_sectionname(opmode, section, 0, auth_password, strlen(auth_password));
                        }
                    }
                    else
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
                    }
                }
            }
            if (json_object_object_get(jobj, "bssid") != NULL)
            {
                senao_json_object_get_and_create_string(rep, jobj, "bssid", &bssid);
                if(api_set_wifi_ifacex_bssid_option_by_sectionname(opmode, section, 2, bssid, strlen(bssid)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSSID");
                }
            }
            if (strcmp(encryption, "WEP" ) == 0)
            {
                if (json_object_object_get(jobj_wep, "auth_type") == NULL)
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH TYPE");
                }
            }
            else if (strcmp(encryption, "WPA-PSK" ) == 0 || strcmp(encryption, "WPA2-PSK" ) == 0)
            {
                if (json_object_object_get(jobj_wpa, "auth_type") == NULL)
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH TYPE");
                }
            }
#if defined(SUPPORT_AP_RP_SETUP_WIZARD) || defined(SUPPORT_RP_SSID_SETTING)
			api_set_integer_option("system.firmware.first_login", 0);
			if (json_object_object_get(jobj, "change_erp_ssid") != NULL)
			{
				senao_json_object_get_and_create_string(rep, jobj, "change_erp_ssid", &jobj_erp_str);
				if(NULL != jobj_erp_str)
				{
					if((jobj_erp = jsonTokenerParseFromStack(rep, jobj_erp_str)))
					{
						if (json_object_object_get(jobj_erp, "rp_ssid_name") != NULL)
						{
							senao_json_object_get_and_create_string(rep, jobj_erp, "rp_ssid_name", &erp_ssid);
							api_set_wifi_ifacex_ssid_option_by_sectionname(opmode, section, 1, erp_ssid, strlen(erp_ssid));
						}
						if (json_object_object_get(jobj_erp, "encryption") != NULL)
						{
							senao_json_object_get_and_create_string(rep, jobj_erp, "encryption", &erp_encryption);
							if ( strcmp( erp_encryption, "WPA2-PSK" ) == 0 )
							{
								erp_encr_type = 5;
							}
							else
							{
								erp_encr_type = ENCRYPTION_NONE;
							}
							api_set_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 1, erp_encr_type);
						}
						if (json_object_object_get(jobj_erp, "passphrase") != NULL)
						{
							senao_json_object_get_and_create_string(rep, jobj_erp, "passphrase", &erp_passphrase);
							if(api_set_wifi_ifacex_wpakey_key_option_by_sectionname(opmode, section, 1, erp_passphrase, sizeof(erp_passphrase)))
							{
								RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSPHRASE");
							}
							//api_set_string_option2(erp_passphrase, sizeof(erp_passphrase), "%s_1.%s", section, "key");
						}
						if (json_object_object_get(jobj_erp, "enable") != NULL)
						{
							senao_json_object_get_boolean(jobj_erp, "enable", &(erp_enable));
							if (erp_enable == false)
							{
								api_get_wifi_ifacex_ssid_option_by_sectionname(opmode, section, 2, erp_ssid_sta, sizeof erp_ssid_sta);
								api_set_wifi_ifacex_ssid_option_by_sectionname(opmode, section, 1, erp_ssid_sta, strlen(erp_ssid_sta));
								api_get_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 2, &erp_encr_sta);
								api_set_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 1, erp_encr_sta);
								switch(erp_encr_sta)
								{
								case 1: //wep
								case 2:
									api_get_wifi_ifacex_wepkey_id_option_by_sectionname(opmode, section, 2, &erp_default_key);
									api_set_wifi_ifacex_wepkey_id_option_by_sectionname(opmode, section, 1, erp_default_key);
									for ( i=1; i<5; i++)
									{
										api_get_wifi_ifacex_wepkey_keyx_option_by_sectionname(opmode, section, 2, i, erp_key_sta, sizeof erp_key_sta);
										api_set_wifi_ifacex_wepkey_keyx_option_by_sectionname(opmode, section, 1, i, erp_key_sta, sizeof erp_key_sta);
									}
									break;
								default:
									api_get_string_option2(erp_key_sta, sizeof(erp_key_sta), "%s_2.%s", section, "key");
									if (strlen(erp_key_sta)!=0)
									{
										if (api_set_wifi_ifacex_wpakey_key_option_by_sectionname(opmode, section, 1, erp_key_sta, sizeof(erp_key_sta)))
										{
											RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSPHRASE");
										}
										//api_set_string_option2(erp_key_sta, sizeof(erp_key_sta), "%s_1.%s", section, "key");
									}
									break;
								}
							}
						}
					}
					else
					{
						RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
					}
				}
			}
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

    return 0;
}
