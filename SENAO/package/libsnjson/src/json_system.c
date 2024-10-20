#include <api_common.h>
#include <sys_common.h>
#include <api_wireless.h>
#include <variable.h>
#include <api_lan.h>
#include <wireless_tokens.h>
//#include <integer_check.h>
#include <json_object.h>
#include <json_tokener.h>
#include <json_system.h>
#include <json_common.h>
#include <unistd.h>
#include <dirent.h>

int json_get_version(ResponseEntry *rep, struct json_object *jobj)
{
    int major = 0, minor = 0, release = 0;
    char patch[16] = {0};
    char buf_server[128] = {0}, buf_module[128] = {0}, buf_libapi[128] = {0}, buf_libsnjson[128] = {0};
    struct json_object *jobj_server;
    struct json_object *jobj_module;
    struct json_object *jobj_library;
    struct json_object *jobj_libapi;
    struct json_object *jobj_libsnjson;
    ResponseStatus *res = rep->res;

    jobj_server = json_object_new_object();
    jobj_module = json_object_new_object();
    jobj_library = json_object_new_object();
    jobj_libapi = json_object_new_object();
    jobj_libsnjson = json_object_new_object();

    sys_interact(buf_server, sizeof(buf_server), "opkg list senao-openapi-server |awk '{printf $3}'");
    sscanf(buf_server, "%d.%d-%d-g%s", &major, &minor, &release, patch);
    json_object_object_add(jobj_server, "major", json_object_new_int(major));
    json_object_object_add(jobj_server, "minor", json_object_new_int(minor));
    json_object_object_add(jobj_server, "release", json_object_new_int(release));
    json_object_object_add(jobj_server, "patch", json_object_new_string(patch));

    major = 0;
    minor = 0;
    release = 0;
    memset(patch, 0, sizeof(patch));

    sys_interact(buf_module, sizeof(buf_module), "opkg list senao-ap-api-module |awk '{printf $3}'");
    sscanf(buf_module, "%d.%d-%d-g%s", &major, &minor, &release, patch);
    json_object_object_add(jobj_module, "major", json_object_new_int(major));
    json_object_object_add(jobj_module, "minor", json_object_new_int(minor));
    json_object_object_add(jobj_module, "release", json_object_new_int(release));
    json_object_object_add(jobj_module, "patch", json_object_new_string(patch));

    major = 0;
    minor = 0;
    release = 0;
    memset(patch, 0, sizeof(patch));

    sys_interact(buf_libsnjson, sizeof(buf_libsnjson), "opkg list libsnjson |awk '{printf $3}'");
    sscanf(buf_libsnjson, "%d.%d-%d-g%s", &major, &minor, &release, patch);
    json_object_object_add(jobj_libsnjson, "major", json_object_new_int(major));
    json_object_object_add(jobj_libsnjson, "minor", json_object_new_int(minor));
    json_object_object_add(jobj_libsnjson, "release", json_object_new_int(release));
    json_object_object_add(jobj_libsnjson, "patch", json_object_new_string(patch));
    major = 0;
    minor = 0;
    release = 0;
    memset(patch, 0, sizeof(patch));

    sys_interact(buf_libapi, sizeof(buf_libapi), "opkg list libapi |awk '{printf $3}'");
    sscanf(buf_libapi, "%d.%d-%d-g%s", &major, &minor, &release, patch);
    json_object_object_add(jobj_libapi, "major", json_object_new_int(major));
    json_object_object_add(jobj_libapi, "minor", json_object_new_int(minor));
    json_object_object_add(jobj_libapi, "release", json_object_new_int(release));
    json_object_object_add(jobj_libapi, "patch", json_object_new_string(patch));

    json_object_object_add(jobj_library, "libapi", jobj_libapi);
    json_object_object_add(jobj_library, "libsnjson", jobj_libsnjson);
    json_object_object_add(jobj, "server", jobj_server);
    json_object_object_add(jobj, "module", jobj_module);
    json_object_object_add(jobj, "library", jobj_library);

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_sys_revert(ResponseEntry *rep, char *query_str)
{
    ResponseStatus *res = rep->res;

    char section[32] = {0};
    char cmd[128] = {0};

    sys_interact(section, sizeof(section), "uci changes | head -n 1 | awk -F '.' '{print $1}' | tr -d '\n'");
    while ( strlen(section) > 0 )
    {
        sprintf(cmd, "uci revert %s", section);
        system(cmd);
        sys_interact(section, sizeof(section), "uci changes | head -n 1 | awk -F '.' '{print $1}' | tr -d '\n'");
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_mgm_fw_upgrade(ResponseEntry *rep, char *query_str)
{
    char *upgrade_from_server=NULL, *mode=NULL;
    // bool doReset = 0;
    struct json_object *jobj;
    ResponseStatus *res = rep->res;

#if SUPPORT_SYSTEM_LOG
    system("echo upgrade>/mnt/rebootType");
#endif

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "mode", &mode);
            // senao_json_object_get_boolean(jobj, "do_reset_after_upgrade", &doReset);
            if (strcmp(mode, "Upgrade_from_server") == 0)
            {
                senao_json_object_get_and_create_string(rep, jobj, "upgrade_from_server", &upgrade_from_server);
                return json_fw_upgrade_fromserver(rep, upgrade_from_server);
            }
            else if (strcmp(mode, "Upgrade_locally") == 0)
            {
                burn_fw();
            }
        }
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);

}

int json_fw_upgrade_fromserver(ResponseEntry *rep, char *query_str)
{
    struct json_object *jobj;
    bool enable = 0;
    char *upgrade_url=NULL, buf[128]={0}, buf2[128]={0};
    int led, senaowrt;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "upgrade_url", &upgrade_url);
        }
    }
    if (sys_check_file_existed("/tmp/fw_download") == true)
        system("rm /tmp/fw_download");

    sys_interact(buf, sizeof(buf), "curl --connect-timeout 6 --speed-limit 1 --speed-time 5 -k %s --output /tmp/firmware.img && echo success >/tmp/fw_download || echo falied >/tmp/fw_download &", upgrade_url);

    if (sys_check_file_existed("/tmp/fw_download") == true)
    {
        sys_interact(buf2, sizeof(buf2), "cat /tmp/fw_download");
        if (strstr(buf2, "success") != NULL)
        {
            if (sys_check_file_existed("/tmp/firmware.img"))
            {
                burn_fw();
                RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
            }
            else
            {
                RET_GEN_ERRORMSG(res, API_FILE_NOT_EXIST, NULL);
            }
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_FILE_NOT_EXIST, NULL);
        }
    }
    //return 0;
}
int burn_fw()
{
    int led = 0, senaowrt = 0;
    char buf[10] = {0};
    
    api_get_integer_option("functionlist.functionlist.SUPPORT_LED_MODULE_NAME", &led);
    api_get_integer_option("functionlist.functionlist.SUPPORT_SENAOWRT_IMAGE", &senaowrt);

    memset(buf,0,sizeof(buf));
    sys_interact(buf, sizeof(buf), "pgrep avahi-daemon");
    if ( strlen(buf) > 0 )
        system("/etc/init.d/avahi-daemon stop");

    if (led == 1) 
    {
        if (senaowrt == 1)
        {
            system("sleep 1; killall dropbear uhttpd lighttpd ; sleep 1; echo timer > /sys/class/leds/power1_led/trigger;/etc/fwupgrade.sh /tmp/firmware.img &");
        }
        else
        {
            system("sleep 1; killall dropbear uhttpd lighttpd ; sleep 1; echo timer > /sys/class/leds/power1_led/trigger;/sbin/sysupgrade /tmp/firmware.img &");
        }
    }
    else
    {
        if (senaowrt == 1)
        {
            system("sleep 1; killall dropbear uhttpd lighttpd ; sleep 1; echo timer > /sys/class/leds/power/trigger;/etc/fwupgrade.sh /tmp/firmware.img &");
        }
        else
        {
            system("sleep 1; killall dropbear uhttpd lighttpd ; sleep 1; echo timer > /sys/class/leds/power/trigger;/sbin/sysupgrade /tmp/firmware.img &");
        }
    }
    return 0;
}

int json_mgm_fw_upgrade_only(ResponseEntry *rep, char *query_str)
{
    ResponseStatus *res = rep->res;

#if SUPPORT_SYSTEM_LOG
    system("echo upgrade>/mnt/rebootType");
#endif

    if (sys_check_file_existed("/tmp/firmware.img"))
    {
        burn_fw_only();
        RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_FILE_NOT_EXIST, NULL);
    }

}

int burn_fw_only()
{
    system("sleep 1; killall dropbear uhttpd lighttpd ; sleep 1; echo timer > /sys/class/leds/power/trigger;/sbin/sysupgrade -k /tmp/firmware.img");
    return 0;
}

int json_mgm_localupgradeimage(ResponseEntry *rep, struct json_object *jobj)
{
    char buf[128] = {0}, fw_md5sum[128] = {0};
    int fw_size;
    ResponseStatus *res = rep->res;

    if (sys_check_file_existed("/tmp/firmware.img"))
    {
        sys_interact(buf, sizeof(buf), "ls -al /tmp/firmware.img |awk {'printf $5'}");
        sys_interact(fw_md5sum, sizeof(fw_md5sum), "md5sum /tmp/firmware.img |awk {'printf $1'}");
        fw_size = atoi(buf);
    }
    else
    {
        fw_size = 0;
        strcpy(fw_md5sum, "");
    }

    json_object_object_add(jobj, "image_size", json_object_new_int(fw_size));
    json_object_object_add(jobj, "image_checksum", json_object_new_string(fw_md5sum));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_mgm_restoreimage(ResponseEntry *rep, struct json_object *jobj)
{
    char buf[128] = {0}, restore_md5sum[128] = {0};
    int restore_size;
    ResponseStatus *res = rep->res;

    if (sys_check_file_existed("/tmp/restore.gz"))
    {
        sys_interact(buf, sizeof(buf), "ls -al /tmp/restore.gz |awk {'printf $5'}");
        sys_interact(restore_md5sum, sizeof(restore_md5sum), "md5sum /tmp/restore.gz |awk {'printf $1'}");
        restore_size = atoi(buf);
    }
    else
    {
        restore_size = 0;
        strcpy(restore_md5sum, "");
    }

    json_object_object_add(jobj, "image_size", json_object_new_int(restore_size));
    json_object_object_add(jobj, "image_checksum", json_object_new_string(restore_md5sum));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_auth_check(ResponseEntry *rep, struct json_object *jobj)
{
    ResponseStatus *res = rep->res;
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

unsigned int countSetBits(unsigned int n)
{
    unsigned int count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}

int json_get_sys_dev_capability(ResponseEntry *rep, struct json_object *jobj)
{
    char buf[128] = {0}, reg_domain_str[5] = {0}, mgm_radio[10] = {0};
    int reg_domain = 0, dfs_fcc = 0, dfs_etsi = 0, dfs_int = 0, ssid_num_ap = 0, ssid_num_wds_ap = 0, ssid_num_enjet = 0, nawds_num_wds_ap = 0, nawds_num_wds_bridge = 0, mac_filter_num = 0;
    bool support_mesh, support_enjet, outdoor, support_gps, support_bandsteering, support_captive_portal, support_ezmcloud, support_wpa3, support_hwmode_ax, support_rp_ssid_setting = false, support_repeater_module = false;
    struct json_object *jarr_mode = NULL, *jobj_dfs_certified = NULL, *jobj_ssid_num = NULL, *jobj_nawds_num = NULL;
    int mgm_radio_num = 0, txpower_start = 0, txpower_end = 0;
    ResponseStatus *res = rep->res;
    DIR *dir = NULL;
    FILE *fp_mac = NULL;
    struct dirent *ent;
    struct json_object *jobj_ifaces_info = NULL, *jobj_iface_info = NULL, *jobj_antenna_num = NULL;
    char mac_addr_file[128] = {0}, iface_mac[18] = {0};
    int eth_port_num = 0,support_wallmount = 0, ssid_profile_port = 0, support_mesh_led = 0 , support_scan_radio = 0, antenna_24g = 0, antenna_5g = 0;

    jarr_mode = json_object_new_array();
    jobj_dfs_certified = json_object_new_object();
    jobj_ssid_num = json_object_new_object();
    jobj_nawds_num = json_object_new_object();
    jobj_antenna_num = json_object_new_object();

    sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.SUPPORT_AP_MODE_ONLY 2>/dev/null");

    if (atoi(buf) == 1) 
    {
        json_object_array_add(jarr_mode, json_object_new_string("ap"));
        json_object_object_add(jobj, "support_opmode", jarr_mode);
    }
    else
    {
        json_object_array_add(jarr_mode, json_object_new_string("ap"));
        sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.SUPPORT_CLIENTBRIDGE_MODULE 2>/dev/null");
        if (atoi(buf) == 1) 
        {
            json_object_array_add(jarr_mode, json_object_new_string("sta"));
        }
        sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.NOT_SUPPORT_WDS_AP_MODE 2>/dev/null");
        if (atoi(buf) == 0 || strcmp(buf, "") == 0) 
        {
            json_object_array_add(jarr_mode, json_object_new_string("wds_ap"));
        }
        sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.NOT_SUPPORT_WDS_BRIDGE_MODE 2>/dev/null");
        if (atoi(buf) == 0 || strcmp(buf, "") == 0) 
        {
            json_object_array_add(jarr_mode, json_object_new_string("wds_bridge"));
        }
        sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.NOT_SUPPORT_WDS_STATION_MODE 2>/dev/null");
        if (atoi(buf) == 0 || strcmp(buf, "") == 0) 
        {
            json_object_array_add(jarr_mode, json_object_new_string("wds_sta"));
        }
        sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.SUPPORT_REPEATER_MODULE 2>/dev/null");
        if (atoi(buf) == 1) 
        {
            json_object_array_add(jarr_mode, json_object_new_string("sta_ap"));
            support_repeater_module = true;
	    support_rp_ssid_setting = true;
        }
        json_object_object_add(jobj, "support_opmode", jarr_mode);
    }

    jarr_mode = json_object_new_array();
    sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.SUPPORT_WLAN24G 2>/dev/null");
    if (atoi(buf) == 1) 
    {
        json_object_array_add(jarr_mode, json_object_new_string("2_4G"));
        json_object_object_add(jobj_antenna_num, "2_4G", json_object_new_int(countSetBits(sys_get_txchainmask(0))));

    }
    sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.SUPPORT_WLAN5G 2>/dev/null");
    if (atoi(buf) == 1) 
    {
        json_object_array_add(jarr_mode, json_object_new_string("5G"));
        json_object_object_add(jobj_antenna_num, "5G", json_object_new_int(countSetBits(sys_get_txchainmask(1))));
    }
    sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.SUPPORT_WLAN5G_2 2>/dev/null");
    if (atoi(buf) == 1) 
    {
        json_object_array_add(jarr_mode, json_object_new_string("5G-2"));
        json_object_object_add(jobj_antenna_num, "5G-2", json_object_new_int(countSetBits(sys_get_txchainmask(2))));
    }
    json_object_object_add(jobj, "support_radio", jarr_mode);
    json_object_object_add(jobj, "antenna_num", jobj_antenna_num);

    sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.SUPPORT_MESH_SETTING 2>/dev/null");
    support_mesh = (atoi(buf) == 1)?true:false;
    sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.SUPPORT_ENJET 2>/dev/null");
    support_enjet = (atoi(buf) == 1)?true:false;
    sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.SUPPORT_WPA3 2>/dev/null");
    support_wpa3 = (atoi(buf) == 1)?true:false;
    sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.HWMODE_AX 2>/dev/null");
    support_hwmode_ax = (atoi(buf) == 1)?true:false;
    sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.SUPPORT_BANDSTEER_SELECT_LIST 2>/dev/null");
    support_bandsteering = (atoi(buf) == 1)?true:false;
    sys_interact(buf, sizeof(buf), "uci get sysProductInfo.model.outdoor 2>/dev/null");
    outdoor = (atoi(buf) == 1)?true:false;
    sys_interact(buf, sizeof(buf), "setconfig -g 4 2>/dev/null");
    reg_domain = atoi(buf);
    if ( api_get_integer_option("sysProductInfo.model.fccDfsCertified", &dfs_fcc) != API_RC_SUCCESS )
    {
        dfs_fcc = 0;
    }
    if ( api_get_integer_option("sysProductInfo.model.etsiDfsCertified", &dfs_etsi) != API_RC_SUCCESS )
    {
        dfs_etsi = 0;
    }
    if ( api_get_integer_option("sysProductInfo.model.intDfsCertified", &dfs_int) != API_RC_SUCCESS )
    {
        dfs_int = 0;
    }

    switch (reg_domain)
    {
        case 0:
            sprintf(reg_domain_str,"FCC");
            break;
        case 1:
            sprintf(reg_domain_str,"ETSI");
            break;
        case 2:
            sprintf(reg_domain_str,"INT");
            break;
        default:
            sprintf(reg_domain_str,"INT");
            break;
    }
    support_gps = sys_check_file_existed("/etc/config/gpsd")?true:false;
    support_ezmcloud = sys_check_file_existed("/etc/config/ezmcloud")?true:false;
    api_get_integer_option("functionlist.vendorlist.AP_WIFI_IFACE_NUM", &ssid_num_ap);
    api_get_integer_option("functionlist.vendorlist.WDSAP_AP_WIFI_IFACE_NUM", &ssid_num_wds_ap);
    api_get_integer_option("functionlist.vendorlist.WDSAP_AP_WIFI_IFACE_NUM", &nawds_num_wds_ap);
    api_get_integer_option("functionlist.vendorlist.WDS_MAC_ADDR_NUM", &nawds_num_wds_bridge);
    api_get_integer_option("functionlist.vendorlist.TXPOWER_MIN_5G", &txpower_start);
    api_get_integer_option("functionlist.vendorlist.TXPOWER_MAX_5G", &txpower_end);
    api_get_integer_option("functionlist.vendorlist.ETHER_PORT_NUM", &eth_port_num);
    api_get_integer_option("functionlist.functionlist.SUPPORT_SSID_PROFILE", &support_wallmount);
    api_get_integer_option("functionlist.vendorlist.SSID_PROFILE_PORT", &ssid_profile_port);
    api_get_integer_option("functionlist.vendorlist.HAS_MESH_LED", &support_mesh_led);
    api_get_integer_option("functionlist.vendorlist.HAS_SCAN_RADIO", &support_scan_radio);

    ssid_num_enjet = 1;

    sys_interact(buf, sizeof(buf), "uci get apcontroller.capwap.enable 2>/dev/null");
    support_captive_portal = (atoi(buf) == 1)?true:false;

    api_get_integer_option("functionlist.vendorlist.MAX_WLAN_MAC_FILTER_NUMBER", &mac_filter_num);
    if ( mac_filter_num == 0 )
        mac_filter_num = 32; // default is 32

    api_get_integer_option("functionlist.vendorlist.MANAGEMENT_SUPPORT_RADIO", &mgm_radio_num);

    if (mgm_radio_num == 1) 
        strcpy(mgm_radio, "wifi0");
    else if (mgm_radio_num == 8)
        strcpy(mgm_radio, "wifi2");
    else
        strcpy(mgm_radio, "wifi1");

    if(support_rp_ssid_setting == true)
    {
	sys_interact(buf, sizeof(buf), "uci get functionlist.functionlist.SUPPORT_RP_SSID_SETTING 2>/dev/null");
	support_rp_ssid_setting = (atoi(buf) == 1)?true:false;
    }
    json_object_object_add(jobj, "eth_port_num", json_object_new_int(eth_port_num));
    json_object_object_add(jobj, "support_wallmount", json_object_new_int(support_wallmount));
    json_object_object_add(jobj, "ssid_profile_port", json_object_new_int(ssid_profile_port));
    json_object_object_add(jobj, "support_mesh_led", json_object_new_int(support_mesh_led));
    json_object_object_add(jobj, "support_scan_radio", json_object_new_int(support_scan_radio));
    json_object_object_add(jobj, "support_mesh", json_object_new_boolean(support_mesh));
#if SUPPORT_ENJET
    json_object_object_add(jobj, "support_enjet", json_object_new_boolean(support_enjet));
#endif
    json_object_object_add(jobj, "support_wpa3", json_object_new_boolean(support_wpa3));
    json_object_object_add(jobj, "support_hwmode_ax", json_object_new_boolean(support_hwmode_ax));
    json_object_object_add(jobj, "support_band_steering", json_object_new_boolean(support_bandsteering));
    json_object_object_add(jobj, "support_captive_portal", json_object_new_boolean(support_captive_portal));
#if !SUPPORT_SWOS_FUNCTION
    json_object_object_add(jobj, "support_ezmcloud", json_object_new_boolean(support_ezmcloud));
#endif
    json_object_object_add(jobj, "support_repeater_module", json_object_new_boolean(support_repeater_module));
    json_object_object_add(jobj, "support_rp_ssid_setting", json_object_new_boolean(support_rp_ssid_setting));
    json_object_object_add(jobj, "outdoor", json_object_new_boolean(outdoor));
    json_object_object_add(jobj, "reg_domain", json_object_new_string(reg_domain_str));
    json_object_object_add(jobj_dfs_certified, "fcc", json_object_new_int(dfs_fcc));
    json_object_object_add(jobj_dfs_certified, "etsi", json_object_new_int(dfs_etsi));
    json_object_object_add(jobj_dfs_certified, "int", json_object_new_int(dfs_int));
    json_object_object_add(jobj, "dfs_certified", jobj_dfs_certified);
#if !SUPPORT_SWOS_FUNCTION
    json_object_object_add(jobj, "support_gps", json_object_new_boolean(support_gps));
#endif
    json_object_object_add(jobj_ssid_num, "ap", json_object_new_int(ssid_num_ap));
    json_object_object_add(jobj_ssid_num, "wds_ap", json_object_new_int(ssid_num_wds_ap));
#if SUPPORT_ENJET
    json_object_object_add(jobj_ssid_num, "enjet", json_object_new_int(ssid_num_enjet));
#endif
    json_object_object_add(jobj, "ssid_num", jobj_ssid_num);
    json_object_object_add(jobj_nawds_num, "wds_ap", json_object_new_int(nawds_num_wds_ap));
    json_object_object_add(jobj_nawds_num, "wds_bridge", json_object_new_int(nawds_num_wds_bridge));
    json_object_object_add(jobj, "nawds_num", jobj_nawds_num);
    json_object_object_add(jobj, "mac_filter_num", json_object_new_int(mac_filter_num));
#if !SUPPORT_SWOS_FUNCTION
    json_object_object_add(jobj, "mgm_radio", json_object_new_string(mgm_radio));
#endif
    json_object_object_add(jobj, "tx_power_min", json_object_new_int(txpower_start));
    json_object_object_add(jobj, "tx_power_max", json_object_new_int(txpower_end));

    jobj_ifaces_info = json_object_new_array();
    if ((dir = opendir("/sys/class/net")) != NULL)
    {
        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL)
        {
            if ( strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, "bonding_masters") == 0 )
                continue;

            memset(iface_mac, 0, sizeof(iface_mac));
            jobj_iface_info = json_object_new_object();

            json_object_object_add(jobj_iface_info, "ifname", json_object_new_string(ent->d_name));

            snprintf(mac_addr_file, sizeof(mac_addr_file), "/sys/class/net/%s/address", ent->d_name);
            if ( ( fp_mac = fopen(mac_addr_file, "r") ) != NULL )
            {
                if ( fgets(iface_mac, 18, fp_mac) != NULL && strlen(iface_mac) == 17 )
                    json_object_object_add(jobj_iface_info, "mac", json_object_new_string(iface_mac));

                fclose(fp_mac);
            }
            json_object_array_add(jobj_ifaces_info, jobj_iface_info);
        }
        closedir(dir);
    }
    json_object_object_add(jobj, "iface_info", jobj_ifaces_info);

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

#if HAS_SENAO_PACKETEER
int json_get_sys_benu_tunnel_addr(ResponseEntry *rep, struct json_object *jobj)
{
    char buf[16] = {0};
    char benu_tunnel_addr[16] = {0};
    ResponseStatus *res = rep->res;

    sys_interact(buf, sizeof(buf), "uci get benu.benu.peeraddr 2>/dev/null");

    if (strlen(buf) > 0)
        sscanf(buf, "%s", benu_tunnel_addr);

    json_object_object_add(jobj, "benu_tunnel_addr", json_object_new_string(benu_tunnel_addr));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_sys_benu_tunnel_addr(ResponseEntry *rep, char *query_str)
{
    char *benu_tunnel_addr=NULL;
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

    if (NULL != query_str)
    {
        if ((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "benu_tunnel_addr", &benu_tunnel_addr);

        }
    }

    if (api_set_string_option("benu.benu.peeraddr", benu_tunnel_addr, sizeof(benu_tunnel_addr)) != API_RC_SUCCESS)
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BENU PEER ADDR");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
#endif
int json_post_sys_reload(ResponseEntry *rep, char *query_str)
{

    struct json_object *jobj;

    ResponseStatus *res = rep->res;

    if (sys_check_file_existed("/var/run/luci-reload-status") == true)
    {
        RET_GEN_ERRORMSG(res, API_PROCESSING, "PROCESSING");
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_SUCCESS, NULL); 
    }

}

int json_get_lxc_ezm_backup_info(ResponseEntry *rep, struct json_object *jobj)
{
    char date[32];
    ResponseStatus *res = rep->res;

    memset(date, 0x00, sizeof(date));
    sys_interact(date, sizeof(date), "ls -e /mnt/mmcblk0p4/ezmaster.tar.gz | awk '{print $6,$7,$8,$9,$10}' | tr -d '\n' 2>/dev/null");

    json_object_object_add(jobj, "date", json_object_new_string(date));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_lxc_ezm_backup_method(ResponseEntry *rep, struct json_object *jobj)
{
    int manual;
    char buf[16];
    ResponseStatus *res = rep->res;

    manual = 0;
    memset(buf, 0x00, sizeof(buf));

    api_get_integer_option("lxc.lqsdk.backup_manual", &manual);
    api_get_string_option("lxc.lqsdk.backup_time", buf, sizeof (buf));

    json_object_object_add(jobj, "manualbackup", json_object_new_boolean(manual));
    json_object_object_add(jobj, "backuptime", json_object_new_string(buf));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_lxc_ezm_backup_method(ResponseEntry *rep, char *query_str)
{
    bool manual;
    int ret;
    int hour;
    int minute;
    char *backup_time=NULL;
    char buf[64];
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

    if (NULL != query_str)
    {
        memset(buf, 0x00, sizeof(buf));
        sys_interact(buf, sizeof(buf), "ps | grep tar | grep ezmaster.tar.gz | grep -v grep");

        if (0 != strlen(buf))
        {
            RET_GEN_ERRORMSG(res, API_PROCESSING, "BACKUP IN PROCESSING");
        }

        if ((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_boolean(jobj, "manualbackup", &manual);

            api_set_bool_option("lxc.lqsdk.backup_manual", manual);
            system("uci commit");

            // Remove original backup time.
            system("sed -i '/backup_ezmaster/d' /etc/crontabs/root");

            if (TRUE == manual)
            {
                system("/usr/shc/backup_ezmaster &");
            }
            else
            {
                senao_json_object_get_and_create_string(rep, jobj, "backuptime", &backup_time);

                ret = sscanf(backup_time, "%d:%d", &hour, &minute);

                if (2 == ret && (0 <= hour && 24 > hour) && (0 <= minute && 60 > minute))
                {
                    api_set_string_option("lxc.lqsdk.backup_time", backup_time, strlen(backup_time));
                    system("uci commit");

                    // Add new backup time.
                    sprintf(buf,
                            "echo \"%d %d * * * /usr/shc/backup_ezmaster & \" >> /etc/crontabs/root",
                            minute, hour);
                    system(buf);
                }
                else
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BACKUP TIME");
                }
            }
        }
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_lxc_ezm_restore(ResponseEntry *rep, struct json_object *jobj)
{
    ResponseStatus *res = rep->res;

    if (sys_check_file_existed("/mnt/mmcblk0p4/ezmaster.tar.gz"))
    {
        system("/usr/shc/restore_ezmaster &");
        RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_FILE_NOT_EXIST, "EZMASTER BACKUP");
    }
}

int json_mgm_drop_caches(ResponseEntry *rep, char *query_str)
{
    struct json_object *jobj;

    ResponseStatus *res = rep->res;

    system("echo 3 > /proc/sys/vm/drop_caches");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_sys_first_login(ResponseEntry *rep, struct json_object *jobj)
{
    int pw_changed = 0, pw_ignore = 0;
    ResponseStatus *res = rep->res;

    api_get_integer_option("tmp_wizard.wizard.pw_changed", &pw_changed);
    api_get_integer_option("tmp_wizard.wizard.pw_ignore", &pw_ignore);

    json_object_object_add(jobj, "pw_changed", json_object_new_boolean(pw_changed));
    json_object_object_add(jobj, "pw_ignore", json_object_new_boolean(pw_ignore));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_sys_first_login(ResponseEntry *rep, char *query_str)
{
    bool pw_changed = false, pw_ignore = false;
    struct json_object *jobj = NULL;
    ResponseStatus *res = rep->res;

    if (NULL != query_str)
    {
        if ((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
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
#if SUPPORT_LXC_EZMASTER_ACCOUNT_SETTING
            if ( pw_changed || pw_ignore )
            {
                system("EZMCFG=/mnt/mmcblk0p3/ezmaster/rootfs/usr/share/ezmaster/conf/ezmcfg; grep -q pwd_alert ${EZMCFG} && sed -i '/pwd_alert/c\\pwd_alert = 0' ${EZMCFG} || echo \"pwd_alert = 0\" >> ${EZMCFG}");
            }
#endif
            system("uci commit tmp_wizard");
        }
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
