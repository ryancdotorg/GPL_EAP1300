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
#include <json_chk_ssid.h>
#include <json_common.h>
#include <variable/api_schedule.h>
#include <unistd.h>
#include <regx.h>
#include <api_vpn.h>

#define TMP_REALM_FILE "/tmp/realm"
#define TMP_REALM_JSON_FILE "/tmp/json"

int g_client_num = 0;
#if SUPPORT_WATCHGUARD_FUNCTION
unsigned long long convert_to_bytes(char *str);
#endif

int get_wifi_ifacex_disabled(int radio, int index, ssid_cfg_st *ssidCfg_p)
{
    //char section[SECTION_NAME_LENGTH] = {0};
    int opmode = ssidCfg_p->opmode;
    char section[32] = {0};
    int disable = 0;

    strcpy(section, ssidCfg_p->section);

    if (opmode != -1)
    {
        if(api_get_wifix_section_name(opmode, radio, section))
        {
            return disable;
        }
    }

    if(api_get_wifi_ifacex_disabled_option_by_sectionname(opmode, section, index, &disable))
    {
        return disable;
    }

    return disable;
}

int set_wifi_ifacex_disabled(int radio, int index, int disable, ssid_cfg_st *ssidCfg_p)
{
    int opmode = ssidCfg_p->opmode;
    char section[32] = {0};

    strcpy(section, ssidCfg_p->section);

    if (opmode != -1)
    {
        api_get_wifix_section_name(opmode, radio, section);
    }
    api_set_wifi_ifacex_disabled_option_by_sectionname(opmode, section, index, disable);
}

int json_get_wifi_mgmt(ResponseEntry *rep, struct json_object *jobj)
{
    int disabled = 0, mgmt_radio = 0,livetime=0;
    char ssid[SSID_NAME_LENGTH]={0}, passphrase[PASSPHRASE_LENGTH]={0}, radio[6]={0}, section[32]={0};
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

    api_get_wifi_ifacex_disabled_option_by_sectionname(OPM_ALL, section, 0, &disabled);

    api_get_wifi_ifacex_wpakey_key_option_by_sectionname(OPM_ALL, section, 0, passphrase, sizeof passphrase);

    api_get_wifi_ifacex_ssid_option_by_sectionname(OPM_ALL, section, 0, ssid, sizeof ssid);

    api_get_wifi_mgmt_livetime_option_by_sectionname(OPM_ALL, section, 0, &livetime);

    json_object_object_add(jobj, "enable", json_object_new_boolean(!disabled));
    json_object_object_add(jobj, "passphrase", json_object_new_string(passphrase));
    json_object_object_add(jobj, "ssid_name", json_object_new_string(ssid));
    json_object_object_add(jobj, "livetime", json_object_new_boolean(livetime?true:false));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_wifi_mgmt(ResponseEntry *rep, char *query_str)
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
            senao_json_object_get_boolean(jobj, "enable",&(enable));
            senao_json_object_get_and_create_string(rep, jobj, "ssid_name", &ssid);
            senao_json_object_get_and_create_string(rep, jobj, "passphrase", &passphrase);
	        senao_json_object_get_boolean(jobj, "livetime",&(livetime));
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

    if (api_set_wifi_ifacex_disabled_option_by_sectionname(OPM_ALL, section, 0, (enable?0:1)))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ENABLE");
    }

    if ( !enable )
        RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);

    if(api_set_wifi_ifacex_wpakey_key_option_by_sectionname(OPM_ALL, section, 0, passphrase, sizeof passphrase))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSPHRASE");
    }

    if(api_set_wifi_ifacex_ssid_option_by_sectionname(OPM_ALL, section, 0, ssid, sizeof ssid))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SSID NAME");
    }

    if (api_set_wifi_mgmt_livetime_option_by_sectionname(OPM_ALL, section, 0, (livetime?15:0)))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "Livetime");
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_wifi_sta_mode(ResponseEntry *rep, struct json_object *jobj, ssid_cfg_st *ssidCfg_p)
{
    int disabled = 0, mgmt_radio = 0, encr_type = 0, eap_type = 0, eap_auth = 0;
    int prebssid_enable = 0, default_key = 0;
    char ssid[SSID_NAME_LENGTH]={0}, bssid[SSID_NAME_LENGTH]={0}, passphrase[PASSPHRASE_LENGTH]={0}, auth_identity[65]={0};
    char eap_type_mapping[10]={0}, eap_auth_mapping[10]={0}, auth_password[65]={0}, input_type[8]={0}, key_length[32]={0};
    char key1[64]={0}, key2[64]={0}, key3[64]={0}, key4[64]={0}, real_key[64]={0}, wep_auth_type[8]={0}, wpa_auth_type[8]={0};
    char ifname[10] = {0}, rssi[16]={0}, tx_rate[24]={0}, rx_rate[24]={0}, tx_bytes[256]={0}, rx_bytes[256]={0}, iface_result[10] = {0};
    char pstr[256] = {0}, wlanconfig_str[512] = {0}, cur_tx_power[32] = {0}, get_connect[128] = {0};
#if SUPPORT_WATCHGUARD_FUNCTION
    unsigned long long wlan_tx_byte = 0, wlan_rx_byte = 0;
#else
    int wlan_tx_byte = 0, wlan_rx_byte = 0;
#endif
    int idx=0;
    char *encr_desc = NULL;
    struct json_object *jobj_wpa = NULL, *jobj_enteriprise = NULL, *jobj_wep = NULL;
    ResponseStatus *res = rep->res;

#if defined(SUPPORT_AP_RP_SETUP_WIZARD) || defined(SUPPORT_RP_SSID_SETTING)
	int erp_enable = 0, erp_encr_type = 0;
	char erp_ssid[SSID_NAME_LENGTH]={0}, erp_passphrase[PASSPHRASE_LENGTH]={0}, erp_auth_type[8]={0};
	char *erp_encr_desc = NULL;
	struct json_object *jobj_erp = NULL;
	jobj_erp = newObjectFromStack(rep);
#endif

    jobj_wpa = newObjectFromStack(rep);
    jobj_enteriprise = newObjectFromStack(rep);
    jobj_wep = newObjectFromStack(rep);

    char *section = ssidCfg_p->section;
    int opmode = ssidCfg_p->opmode;

    api_get_wifi_ifacex_ssid_option_by_sectionname(opmode, section, 2, ssid, sizeof ssid);
    api_get_wifi_ifacex_PreferBSSIDEnable_option_by_sectionname(opmode, section, 2, &prebssid_enable);
    api_get_wifi_ifacex_bssid_option_by_sectionname(opmode, section, 2, bssid, sizeof bssid);
    api_get_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 2, &encr_type);
    api_get_wifi_ifacex_wepkey_id_option_by_sectionname(opmode, section, 2, &default_key);
    api_get_wifi_ifacex_wepkey_keyx_option_by_sectionname(opmode, section, 2, 1, key1, sizeof key1);
    api_get_wifi_ifacex_wepkey_keyx_option_by_sectionname(opmode, section, 2, 2, key2, sizeof key2);
    api_get_wifi_ifacex_wepkey_keyx_option_by_sectionname(opmode, section, 2, 3, key3, sizeof key3);
    api_get_wifi_ifacex_wepkey_keyx_option_by_sectionname(opmode, section, 2, 4, key4, sizeof key4);

    switch(default_key)
    {
        case 1:
            strcpy(real_key, key1);
            break;
        case 2:
            strcpy(real_key, key2);
            break;
        case 3:
            strcpy(real_key, key3);
            break;
        case 4:
            strcpy(real_key, key4);
            break;
    }

    if (strlen(real_key) == 5 || strlen(real_key) == 13 || strlen(real_key) == 16)
    {
        strcpy(input_type, "ascii");
    }
    else
    {
        strcpy(input_type, "hex");
    }
    if (strlen(real_key) == 5 || strlen(real_key) == 10) 
    {
        strcpy(key_length, "40/60-bit");
    }
    else if (strlen(real_key) == 13 || strlen(real_key) == 26)
    {
        strcpy(key_length, "104/128-bit");
    }
    else
    {
        strcpy(key_length, "128/152-bit");
    }

    if ( opmode != OPM_RP) 
    {
        api_get_string_option2(passphrase, sizeof(passphrase), "%s.%s", section, "key");
        api_get_wifi_ifacex_eap_auth_identity_option_by_sectionname(opmode, section, 1, auth_identity, sizeof auth_identity);
        api_get_wifi_ifacex_eap_auth_password_option_by_sectionname(opmode, section, 1, auth_password, sizeof auth_password);
        api_get_wifi_ifacex_eap_type_option_by_sectionname(opmode, section, 1, &eap_type);
        api_get_wifi_ifacex_eap_auth_option_by_sectionname(opmode, section, 1, &eap_auth);

        if (eap_type == 2)
        {
            strcpy(eap_type_mapping, "ttls");
        }
        else if (eap_type == 1)
        {
            strcpy(eap_type_mapping, "peap");
        }

        if (eap_auth == 0)
        {
            strcpy(eap_auth_mapping, "MSCHAP");
        }
        else if (eap_auth == 1)
        {
            strcpy(eap_auth_mapping, "MSCHAPV2");
        }
    }
    else
    {
        api_get_string_option2(passphrase, sizeof(passphrase), "%s_2.%s", section, "key");
    }

    strcpy(wep_auth_type, "open");
    strcpy(wpa_auth_type, "AES"); // set auth_type default

    switch(encr_type)
    {
        case 0:
            encr_desc = DESC_ENCRYPTION_NONE;
            break;
        case 1:
            encr_desc = "WEP";
            strcpy(wep_auth_type, "open");
            break;
        case 2:
            encr_desc = "WEP";
            strcpy(wep_auth_type, "shared");
            break;
        case 3:
            encr_desc = "WPA-PSK";
            strcpy(wpa_auth_type, "AES");
            break;
        case 4:
            encr_desc = "WPA-PSK";
            strcpy(wpa_auth_type, "TKIP");
            break;
        case 5:
            encr_desc = "WPA2-PSK";
            strcpy(wpa_auth_type, "AES");
            break;
        case 6:
            encr_desc = "WPA2-PSK";
            strcpy(wpa_auth_type, "TKIP");
            break;
        case 7:
            encr_desc = "WPA-Enterprise";
            break;
        case 8:
            encr_desc = "WPA2-Enterprise";
            break;
  default:
            encr_desc = DESC_API_OPTION_ERROR;
    }

    if(api_get_wifi_ifacex_ifname_option_by_sectionname(opmode, section, idx, ifname, sizeof(ifname)))
    {
        //return FALSE;
    }

    sys_interact(iface_result, sizeof(iface_result), "iwconfig 2>/dev/null | grep %s || echo no_iface | tr -d '\n'", ifname);

    if ( strcmp(iface_result, "no_iface") != 0 )
    {
        sys_interact(wlanconfig_str, sizeof(wlanconfig_str), "wlanconfig %s list |tail -n +2", ifname);
        if ( strlen(wlanconfig_str) > 0 )
        {
            wlanconfig_str[strcspn(wlanconfig_str,"\n")] = '\0';

            // get tx rate
            sys_interact(tx_rate, sizeof(tx_rate), "echo %s | awk -F \" \" \'{print $4}\'", wlanconfig_str);
            if ( strlen(tx_rate) > 0 )
            {
                tx_rate[strcspn(tx_rate, "\n")] = '\0';
                if ( strstr(tx_rate, "M") )
                    tx_rate[strcspn(tx_rate, "M")] = '\0';
            }

            // get rx rate
            sys_interact(rx_rate, sizeof(rx_rate), "echo %s | awk -F \" \" \'{print $5}\'", wlanconfig_str);
            if ( strlen(rx_rate) > 0 )
            {
                rx_rate[strcspn(rx_rate, "\n")] = '\0';
                if ( strstr(rx_rate, "M") )
                    rx_rate[strcspn(rx_rate, "M")] = '\0';
            }

            // get rssi
            sys_interact(rssi, sizeof(rssi), "echo %s | awk -F \" \" \'{print $6}\'", wlanconfig_str);
            if ( strlen(rssi) > 0 )
            {
                rssi[strcspn(rssi,"\n")] = '\0';
            }

            // get tx byte
            sys_interact(pstr, sizeof(pstr), "echo %s | awk -F \" \" \'{print $7}\'", wlanconfig_str);
            if ( strlen(pstr) > 0 )
            {
                pstr[strcspn(pstr, "\n")] = '\0';
                wlan_tx_byte = convert_to_bytes(pstr);
#if SUPPORT_WATCHGUARD_FUNCTION
                sprintf(tx_bytes, "%llu", wlan_tx_byte);
#else
                sprintf(tx_bytes, "%d", wlan_tx_byte);
#endif
            }

            // get rx byte
            sys_interact(pstr, sizeof(pstr), "echo %s | awk -F \" \" \'{print $8}\'", wlanconfig_str);
            if ( strlen(pstr) > 0 )
            {
                pstr[strcspn(pstr,"\n")] = '\0';
                wlan_rx_byte = convert_to_bytes(pstr);
#if SUPPORT_WATCHGUARD_FUNCTION
                sprintf(rx_bytes, "%llu", wlan_rx_byte);
#else
                sprintf(rx_bytes, "%d", wlan_rx_byte);
#endif
            }
        }

        sys_interact(get_connect, sizeof(get_connect), "iwconfig %s |grep \"Point:\" | grep \"Not\"", ifname);
        if ( get_connect[strlen(get_connect)-1] == '\n' )
                get_connect[strlen(get_connect)-1] = 0;

        if (strcmp(get_connect, "") == 0)
        {
            sys_interact(cur_tx_power, sizeof(cur_tx_power), "iwconfig %s |grep \"Tx-Power\"|awk -F ' ' '{print $4}'|cut -c 10-", ifname);
            if ( cur_tx_power[strlen(cur_tx_power)-1] == '\n' )
                cur_tx_power[strlen(cur_tx_power)-1] = 0;
        }
        else
        {
            strcpy(cur_tx_power, "N/A");
        }
    }

    json_object_object_add(jobj, "ssid_name", json_object_new_string(ssid));
    json_object_object_add(jobj, "prebssid_enable", json_object_new_boolean(prebssid_enable));
    json_object_object_add(jobj, "bssid", json_object_new_string(bssid));

    json_object_object_add(jobj, "tx_rate", json_object_new_string(tx_rate));
    json_object_object_add(jobj, "rx_rate", json_object_new_string(rx_rate));
    json_object_object_add(jobj, "signal_strength", json_object_new_string(rssi));
    json_object_object_add(jobj, "tx_bytes", json_object_new_string(tx_bytes));
    json_object_object_add(jobj, "rx_bytes", json_object_new_string(rx_bytes));
    json_object_object_add(jobj, "cur_tx_power", json_object_new_string(cur_tx_power));

    json_object_object_add(jobj, "encryption", json_object_new_string(encr_desc));

    json_object_object_add(jobj_wep, "auth_type", json_object_new_string(wep_auth_type));
    json_object_object_add(jobj_wep, "input_type", json_object_new_string(input_type));
    json_object_object_add(jobj_wep, "key_length", json_object_new_string(key_length));
    json_object_object_add(jobj_wep, "default_key", json_object_new_int(default_key));
    json_object_object_add(jobj_wep, "key1", json_object_new_string(key1));
    json_object_object_add(jobj_wep, "key2", json_object_new_string(key2));
    json_object_object_add(jobj_wep, "key3", json_object_new_string(key3));
    json_object_object_add(jobj_wep, "key4", json_object_new_string(key4));
    json_object_object_add(jobj, "wep", jobj_wep);

    json_object_object_add(jobj_wpa, "auth_type", json_object_new_string(wpa_auth_type));
    json_object_object_add(jobj_wpa, "passphrase", json_object_new_string(passphrase));
    json_object_object_add(jobj, "wpa", jobj_wpa);
    if ( opmode != OPM_RP)
    {
        json_object_object_add(jobj_enteriprise, "eap_method", json_object_new_string(eap_type_mapping));
        json_object_object_add(jobj_enteriprise, "eap_auth", json_object_new_string(eap_auth_mapping));
        json_object_object_add(jobj_enteriprise, "auth_identity", json_object_new_string(auth_identity));
        json_object_object_add(jobj_enteriprise, "auth_password", json_object_new_string(auth_password));
        json_object_object_add(jobj, "enterprise", jobj_enteriprise);
    }
#if defined(SUPPORT_AP_RP_SETUP_WIZARD) || defined(SUPPORT_RP_SSID_SETTING)
	if ( opmode == OPM_RP )
	{
		api_get_wifi_ifacex_ssid_option_by_sectionname(opmode, section, 1, erp_ssid, sizeof erp_ssid);
		api_get_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 1, &erp_encr_type);
		strcpy(erp_auth_type, "AES");
		switch(erp_encr_type)
		{
			case 0:
				erp_encr_desc = DESC_ENCRYPTION_NONE;
				break;
			case 1:
				erp_encr_desc = "WEP";
				strcpy(erp_auth_type, "open");
				break;
			case 2:
				erp_encr_desc = "WEP";
				strcpy(erp_auth_type, "shared");
				break;
			case 3:
				erp_encr_desc = "WPA-PSK";
				break;
			case 4:
				erp_encr_desc = "WPA-PSK";
				strcpy(erp_auth_type, "TKIP");
				break;
			case 5:
				erp_encr_desc = "WPA2-PSK";
				break;
			case 6:
				erp_encr_desc = "WPA2-PSK";
				strcpy(erp_auth_type, "TKIP");
				break;
			case 7:
				erp_encr_desc = "WPA-Enterprise";
				break;
			case 8:
				erp_encr_desc = "WPA2-Enterprise";
				break;
			default:
				erp_encr_desc = DESC_API_OPTION_ERROR;
		}
		api_get_string_option2(erp_passphrase, sizeof(erp_passphrase), "%s_1.%s", section, "key");
		if ((strcmp(ssid, erp_ssid) != 0) || (encr_type != erp_encr_type))
		{
			erp_enable = 1;
		}

		json_object_object_add(jobj_erp, "enable", json_object_new_boolean(erp_enable));
		json_object_object_add(jobj_erp, "rp_ssid_name", json_object_new_string(erp_ssid));
		json_object_object_add(jobj_erp, "encryption", json_object_new_string(erp_encr_desc));
		json_object_object_add(jobj_erp, "auth_type", json_object_new_string(erp_auth_type));
		json_object_object_add(jobj_erp, "passphrase", json_object_new_string(erp_passphrase));
		json_object_object_add(jobj, "change_erp_ssid", jobj_erp);
	}
#endif
    return 0;
}

int json_set_wifi_sta_mode(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p)
{
    int prebssid = 0, mgmt_radio = 0, encr_type = 0, eap_type = 0, eap_auth = 0, default_key = 0, i = 0;
    bool prebssid_enable = 0;
    char *ssid = NULL, *bssid = NULL, *passphrase = NULL, *auth_identity = NULL, *auth_type = NULL;
    char *eap_type_mapping = NULL, *eap_auth_mapping = NULL, *auth_password = NULL ,*encryption = NULL, *jobj_wpa_str = NULL, *jobj_enterprise_str = NULL;
    char *jobj_wep_str = NULL, *key_length = NULL, *key = NULL, *input_type = NULL;
    struct json_object *jobj = NULL, *jobj_wpa = NULL, *jobj_enteriprise = NULL , *jobj_wep = NULL;
    ResponseStatus *res = rep->res;
    char *section = ssidCfg_p->section;
    int opmode = ssidCfg_p->opmode;
    char tmp[10] = {0};

#if defined(SUPPORT_AP_RP_SETUP_WIZARD) || defined(SUPPORT_RP_SSID_SETTING)
	int erp_encr_type = 0;
	bool erp_enable = 0;
	char *erp_ssid = NULL, *erp_passphrase = NULL, *erp_encryption = NULL, *jobj_erp_str = NULL;
	struct json_object *jobj_erp = NULL;
#endif

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "ssid_name", &ssid);
            senao_json_object_get_boolean(jobj, "prebssid_enable",&(prebssid_enable));
            senao_json_object_get_and_create_string(rep, jobj, "encryption", &encryption);
            //senao_json_object_get_and_create_string(rep, jobj, "auth_type", &auth_type);
            senao_json_object_get_and_create_string(rep, jobj, "wep", &jobj_wep_str);
            senao_json_object_get_and_create_string(rep, jobj, "wpa", &jobj_wpa_str);
            senao_json_object_get_and_create_string(rep, jobj, "enterprise", &jobj_enterprise_str);
#if defined(SUPPORT_AP_RP_SETUP_WIZARD) || defined(SUPPORT_RP_SSID_SETTING)
            senao_json_object_get_and_create_string(rep, jobj, "change_erp_ssid", &jobj_erp_str);
#endif

            prebssid = (prebssid_enable == true)?1:0;
            if (opmode == OPM_RP) 
            {
                api_set_wifi_ifacex_ssid_option_by_sectionname(opmode, section, 1, ssid, strlen(ssid));
                api_set_wifi_ifacex_ssid_option_by_sectionname(opmode, "wireless.wifi0_ssid", 1, ssid, strlen(ssid));
                api_set_wifi_ifacex_ssid_option_by_sectionname(opmode, "wireless.wifi1_ssid", 1, ssid, strlen(ssid));
            }
            api_set_wifi_ifacex_ssid_option_by_sectionname(opmode, section, 2, ssid, strlen(ssid));
            api_set_wifi_ifacex_PreferBSSIDEnable_option_by_sectionname(opmode, section, 2, prebssid);

            if (prebssid_enable == true) 
            {
                senao_json_object_get_and_create_string(rep, jobj, "bssid", &bssid);
                if(api_set_wifi_ifacex_bssid_option_by_sectionname(opmode, section, 2, bssid, strlen(bssid)))
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSSID");
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

    if (strcmp(encryption, "WEP" ) == 0 )
    {
        encr_type = 1;
    }
    else if (strcmp(encryption, "WPA-PSK" ) == 0)
    {
        encr_type = 3; // psk+ccmp or psk+tkip
    }
    else if (strcmp(encryption, "WPA2-PSK" ) == 0)
    {
        encr_type = 5; // psk2+ccmp or psk2+tkip
    }
    else if (strcmp(encryption, "WPA-Enterprise" ) == 0)
    {
        encr_type = 7; // wpa
    }
    else if (strcmp(encryption, "WPA2-Enterprise" ) == 0)
    {
         encr_type = 8; // wpa2
    }
    else
    {
        encr_type = ENCRYPTION_NONE;
    }

    if (encr_type == 1) 
    {
        if(NULL != jobj_wep_str)
        {
            if((jobj_wep = jsonTokenerParseFromStack(rep, jobj_wep_str)))
            {
                senao_json_object_get_and_create_string(rep, jobj_wep, "auth_type", &auth_type);
                senao_json_object_get_and_create_string(rep, jobj_wep, "input_type", &input_type);
                senao_json_object_get_and_create_string(rep, jobj_wep, "key_length", &key_length);
                senao_json_object_get_integer(jobj_wep, "default_key", &default_key);

                if(strcmp(auth_type, "open") == 0)
                {
                    encr_type = 1;
                }
                else if(strcmp(auth_type, "shared") == 0)
                {
                    encr_type = 2;
                }
                else
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH TYPE");
                }
                api_set_wifi_ifacex_wepkey_id_option_by_sectionname(opmode, section, 0, default_key);

                for ( i=1; i<5; i++) 
                {
                    sprintf(tmp,"key%d",i);
                    key = "";
                    senao_json_object_get_and_create_string(rep, jobj_wep, tmp, &key);
                    if ( strcmp(key, "") != 0) 
                    {
                        if (strcmp(input_type, "hex") == 0)
                        {
                            if (!api_check_hexadecimal(key) || !regxMatch(PATTERN1,key)) 
                            {
                                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "KEY");
                            }
                            if ((strlen(key) == 10 && strcmp(key_length, "40/60-bit") == 0) || (strlen(key) == 26 && strcmp(key_length, "104/128-bit") == 0) ||(strlen(key) == 32 && strcmp(key_length, "128/152-bit") == 0))
                            {
                                ;
                            }
                            else
                            {
                                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "KEY LENGTH");
                            }
                        }
                        else if(strcmp(input_type, "ascii") == 0)
                        {
                            if (!regxMatch(PATTERN1,key))
                            {
                                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "KEY");
                            }
                            if ((strlen(key) == 5 && strcmp(key_length, "40/60-bit") == 0) || (strlen(key) == 13 && strcmp(key_length, "104/128-bit") == 0) ||(strlen(key) == 16 && strcmp(key_length, "128/152-bit") == 0))
                            {
                                ;
                            }
                            else
                            {
                                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "KEY LENGTH");
                            }
                        }
                        else
                        {
                            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "INPUT TYPE");
                        }
                        api_set_wifi_ifacex_wepkey_keyx_option_by_sectionname(opmode, section, 2, i, key, sizeof key);
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
    } 
    else if (encr_type == 3 || encr_type == 5)
    {
        if(NULL != jobj_wpa_str)
        {
            if((jobj_wpa = jsonTokenerParseFromStack(rep, jobj_wpa_str)))
            {
                senao_json_object_get_and_create_string(rep, jobj_wpa, "passphrase", &passphrase);
                senao_json_object_get_and_create_string(rep, jobj_wpa, "auth_type", &auth_type);
                if (encr_type == 5) 
                {
                    if(strcmp(auth_type, "AES") == 0)
                    {
                        encr_type = 5;
                    }
                    else if(strcmp(auth_type, "TKIP") == 0)
                    {
                        encr_type = 6;
                    }
                    else
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH_TYPE");
                    }
                }
                else
                {
                    if(strcmp(auth_type, "AES") == 0)
                    {
                        encr_type = 3;
                    }
                    else if(strcmp(auth_type, "TKIP") == 0)
                    {
                        encr_type = 4;
                    }
                    else
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH_TYPE");
                    }
                }

                if (opmode == OPM_RP)
                {
                    api_set_string_option2(passphrase, sizeof(passphrase), "%s_1.%s", section, "key");
                    api_set_string_option2(passphrase, sizeof(passphrase), "%s_2.%s", section, "key");
                    api_set_string_option2(passphrase, sizeof(passphrase), "%s_1.%s", "wireless.wifi0_ssid", "key");
                    api_set_string_option2(passphrase, sizeof(passphrase), "%s_1.%s", "wireless.wifi1_ssid", "key");

                }
                else
                {
                    api_set_string_option2(passphrase, sizeof(passphrase), "%s.%s", section, "key");
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
    }
    else if ((encr_type == 7 || encr_type == 8) && opmode != OPM_RP) //WAP2
    {
        if(NULL != jobj_enterprise_str)
        {
            if((jobj_enteriprise = jsonTokenerParseFromStack(rep, jobj_enterprise_str)))
            {
                senao_json_object_get_and_create_string(rep, jobj_enteriprise, "eap_method", &eap_type_mapping);
                senao_json_object_get_and_create_string(rep, jobj_enteriprise, "eap_auth", &eap_auth_mapping);
                senao_json_object_get_and_create_string(rep, jobj_enteriprise, "auth_identity", &auth_identity);
                senao_json_object_get_and_create_string(rep, jobj_enteriprise, "auth_password", &auth_password);

                if (strcmp(eap_type_mapping, "ttls") == 0)
                {
                    eap_type = 2;
                }
                else if (strcmp(eap_type_mapping, "peap") == 0)
                {
                    eap_type = 1;
                    if (strcmp(eap_auth_mapping, "MSCHAP") == 0) 
                    {
                        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "EAP_AUTH");
                    }
                }
                if (strcmp(eap_auth_mapping, "MSCHAP") == 0)
                {
                    eap_auth = 0;
                }
                else if (strcmp(eap_auth_mapping, "MSCHAPV2") == 0)
                {
                    eap_auth = 1;
               }

                api_set_wifi_ifacex_eap_auth_identity_option_by_sectionname(opmode, section, 0, auth_identity, strlen(auth_identity));
                api_set_wifi_ifacex_eap_auth_password_option_by_sectionname(opmode, section, 0, auth_password, strlen(auth_password));

                api_set_wifi_ifacex_eap_type_option_by_sectionname(opmode, section, 0, eap_type);
                api_set_wifi_ifacex_eap_auth_option_by_sectionname(opmode, section, 0, eap_auth);
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
    }

    if (opmode == OPM_RP && encr_type != 8) 
    {
        api_set_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 1, encr_type);
        api_set_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, "wireless.wifi0_ssid", 1, encr_type);
        api_set_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, "wireless.wifi1_ssid", 1, encr_type);
    }
    else if (opmode == OPM_RP)
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ENCRYPTION");
    }
    api_set_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 2, encr_type);

#if defined(SUPPORT_AP_RP_SETUP_WIZARD) || defined(SUPPORT_RP_SSID_SETTING)
	if(NULL != jobj_erp_str)
	{
		if (opmode == OPM_RP)
		{
			if((jobj_erp = jsonTokenerParseFromStack(rep, jobj_erp_str)))
			{
				senao_json_object_get_boolean(jobj_erp, "enable", &(erp_enable));
				if (erp_enable == true)
				{
					senao_json_object_get_and_create_string(rep, jobj_erp, "rp_ssid_name", &erp_ssid);
					senao_json_object_get_and_create_string(rep, jobj_erp, "encryption", &erp_encryption);
					if ( strcmp( erp_encryption, "WPA2-PSK" ) == 0 )
					{
						erp_encr_type = 5;
					}
					else
					{
						erp_encr_type = ENCRYPTION_NONE;
					}
					senao_json_object_get_and_create_string(rep, jobj_erp, "passphrase", &erp_passphrase);

					api_set_wifi_ifacex_ssid_option_by_sectionname(opmode, section, 1, erp_ssid, strlen(erp_ssid));
					api_set_wifi_ifacex_sta_encryption_option_by_sectionname(opmode, section, 1, erp_encr_type);
					api_set_string_option2(erp_passphrase, sizeof(erp_passphrase), "%s_1.%s", section, "key");
				}
			}
			else
			{
				RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
			}
		}
	}
#endif

    return 0;
}

int json_get_wifi_wds_link(ResponseEntry *rep, struct json_object *jobj, ssid_cfg_st *ssidCfg_p)
{
    int encryption = 0, opmode = ssidCfg_p->opmode;
    char security[10]={0}, passphrase[128]={0}, wlanwdspeer[256]={0}, mac[32]={0};
    char *section = ssidCfg_p->section;
    char buf[13] = {0};
    struct json_object *jarr_obj = NULL, *jobj_tmp = NULL;
    ResponseStatus *res = rep->res;

    int i, j, length, match, hour, minute, jcount=0;
    char *pch = NULL;

    api_get_wifi_ifacex_nawds_encryption_option_by_sectionname(opmode, section, 0, &encryption);
    api_get_wifi_ifacex_wpakey_key_option_by_sectionname(opmode, section, 0, passphrase, sizeof passphrase);
    api_get_wifi_ifacex_WLANWDSPeer_option_by_sectionname(opmode, section, 0, wlanwdspeer, sizeof wlanwdspeer);
    strcpy(security, (encryption == 0)?"none":"ccmp");
    jarr_obj = json_object_new_array();
    length = strlen(wlanwdspeer) / 13;

    if (length != 0)
    {
        for (i = 1; i <= length; i++)
        {
            jobj_tmp = json_object_new_object();
            strncpy(buf, wlanwdspeer+(i-1)*13, 13);

             buf[13]='\0';
            if ( buf[12] == 'v') 
            {
                json_object_object_add(jobj_tmp, "enable", json_object_new_boolean(1));
            }
            else
            {
                json_object_object_add(jobj_tmp, "enable", json_object_new_boolean(0));
            }
            sprintf(mac,"%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11]);
            json_object_object_add(jobj_tmp, "dev_mac_addr", json_object_new_string(mac));
            json_object_array_add(jarr_obj,jobj_tmp);
        }
    }
    json_object_object_add(jobj, "security", json_object_new_string(security));
    json_object_object_add(jobj, "passphrase", json_object_new_string(passphrase));
    json_object_object_add(jobj, "nawds", jarr_obj);

    return 0;
}

int json_set_wifi_wds_link(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p)
{
    int encryption = 0, opmode = ssidCfg_p->opmode, arraylen = 0, json_array_idx = 0;
    bool enable = 0;
    char *security = NULL, *passphrase = NULL, *nawds_str = NULL, *mac = NULL;
    struct json_object *jobj = NULL, *jarr = NULL, *jarr_info = NULL;
    ResponseStatus *res = rep->res;
    char *section = ssidCfg_p->section;
    char buf[20] = {0}, wlanwdspeer[256] = {0};

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "security", &security);
            senao_json_object_get_and_create_string(rep, jobj, "passphrase", &passphrase);
            senao_json_object_get_and_create_string(rep, jobj, "nawds", &nawds_str);
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

    if (strcmp(security,"none") == 0) 
    {
        encryption = 0;
    }
    else if (strcmp(security,"ccmp") == 0) 
    {
        encryption = 2;
        if(api_set_wifi_ifacex_wpakey_key_option_by_sectionname(opmode, section, 0, passphrase, sizeof passphrase))
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSPHRASE");
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SECURITY");
    }

    api_set_wifi_ifacex_nawds_encryption_option_by_sectionname(opmode, section, 0, encryption);

    if ( jarr = jsonTokenerParseFromStack(rep, nawds_str) )
    {
        arraylen = json_object_array_length(jarr);

        if ( opmode == OPM_WDSAP && arraylen > 4) 
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NAWDS UPPER LIMIT");
        }
        else if (arraylen > 8) 
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NAWDS UPPER LIMIT");
        }
        for (json_array_idx = 0; json_array_idx < arraylen; json_array_idx++)
        {
            jarr_info = json_object_array_get_idx(jarr, json_array_idx);

            senao_json_object_get_boolean(jarr_info,"enable",&enable);
            senao_json_object_get_and_create_string(rep, jarr_info, "dev_mac_addr", &mac);

            if(!api_check_mac_none_colon(mac))
            {
                sys_interact(buf, sizeof(buf), "printf %s|sed 's/://g'",mac);
            
                if (enable == true) 
                {
                    strcat(buf, "v");
                }
                else
                {
                    strcat(buf, "x");
                }
                strcat(wlanwdspeer, buf);
            }
            else
            {
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "MAC");
            }
        }
    }
    if (opmode == OPM_WDSAP) 
    {
        api_set_string_option2(wlanwdspeer, sizeof(wlanwdspeer), "%s_0.WLANWDSPeer", section);
    }
    else if (opmode == OPM_WDSB)
    {
        api_set_string_option2(wlanwdspeer, sizeof(wlanwdspeer), "%s.WLANWDSPeer", section);
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "OPMODE");
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}


int json_get_wireless_ssid(struct json_object *jobj, ssid_cfg_st *ssidCfg_p){

    char *section = ssidCfg_p->section, ssid[SSID_NAME_LENGTH] = {0}, enable_bands[SSID_ENABLE_BANDS] = {0};
    char client_ip_assignment[16]={0}, guestnetwork_origin[16] = {0};
    char guest_network[10] = {0};
    int radio_idx = 0, disable = 1, opmode = ssidCfg_p->opmode,  vlan_id = 0;
    int hidden = 0, isolate = 0, l2_isolation = 0, isolation = 0;
    int radio_start = 0, radio_end = 0, i = 0;
    int idx = ssidCfg_p->idx;
    char ifname[10] = {0}, profile_name[64] = {0};
    char rx_packets[1024] = {0}, tx_packets[1024] = {0}, rx_bytes[1024] = {0}, tx_bytes[1024] = {0}, cur_tx_power[32] = {0};
    char max_rx_data_rate[1024] = {0}, max_tx_data_rate[1024] = {0};
    if ( RADIO_MODE % 5 != 2 ) // no wifi0
        radio_start = 1;

    radio_end = radio_start + WIFI_RADIO_NUM - 1;

    if(NULL == jobj)
    {
        return FALSE;
    }

//    if(_api_get_wifi_opmode_option(&opmode))
//    {
//        return FALSE;
//    }
//
//    if(api_get_wifix_section_name(opmode, radio_idx, section))
//    {
//        return FALSE;
//    }

    for ( i = radio_start ; i <= radio_end ; i++ )
    {
        int tmp_disable = get_wifi_ifacex_disabled(i,idx,ssidCfg_p);
        disable = disable && tmp_disable;

        if ( !tmp_disable )
        {
            if ( strlen(enable_bands) > 0 )
                strcat(enable_bands,"|");

            switch (i)
            {
                case 0:
                    strcat(enable_bands,"2_4G");
                    break;
                case 1:
                    strcat(enable_bands,"5G");
                    break;
                case 2:
                    strcat(enable_bands,"5G-2");
                    break;
            }
        }
    }
    if (opmode == OPM_WDSAP)
    {
        if(api_get_wifi_ifacex_disabled_option_by_sectionname(opmode, section, idx, &disable))
        {
            return FALSE;
        }
    }
    if(api_get_wifi_ifacex_ssid_option_by_sectionname(opmode, section, idx, ssid, sizeof ssid))
    {
        return FALSE;
    }

    if(api_get_wifi_ifacex_hidden_option_by_sectionname(opmode, section, idx, &hidden))
    {
        return FALSE;
    }

    if(api_get_wifi_ifacex_isolate_option_by_sectionname(opmode, section, idx, &isolate))
    {
        return FALSE;
    }

    api_get_wifi_ifacex_l2_isolation_option_by_sectionname(opmode, section, idx, &l2_isolation);

    if(api_get_wifi_ifacex_isolation_option_by_sectionname(opmode, section, idx, &isolation))
    {
        return FALSE;
    }

    if(api_get_wifi_ifacex_vlan_id_option_by_sectionname(opmode, section, idx, &vlan_id))
    {
        return FALSE;
    }

    if(api_get_wifi_ifacex_ifname_option_by_sectionname(opmode, section, idx, ifname, sizeof(ifname)))
    {
        return FALSE;
    }

    sys_interact(max_rx_data_rate, sizeof(max_rx_data_rate), "iwconfig %s |grep \"Rate\"|awk -F ':' '{print $2}'|cut -d ' ' -f1-2", ifname);
    if ( max_rx_data_rate[strlen(max_rx_data_rate)-1] == '\n' )
        max_rx_data_rate[strlen(max_rx_data_rate)-1] = 0;

    sys_interact(rx_packets, sizeof(rx_packets), "ifconfig %s |grep \"RX packets\"|awk -F ' ' '{print $2}'|awk -F ':' '{print $2}'", ifname);
    if ( rx_packets[strlen(rx_packets)-1] == '\n' )
        rx_packets[strlen(rx_packets)-1] = 0;

    sys_interact(tx_packets, sizeof(tx_packets), "ifconfig %s |grep \"TX packets\"|awk -F ' ' '{print $2}'|awk -F ':' '{print $2}'", ifname);
    if ( tx_packets[strlen(tx_packets)-1] == '\n' )
        tx_packets[strlen(tx_packets)-1] = 0;

    strcpy(max_tx_data_rate, max_rx_data_rate);

    sys_interact(rx_bytes, sizeof(rx_bytes), "ifconfig %s |grep \"RX bytes\"|awk -F '(' '{print $2}'|awk -F ')' '{print $1}'", ifname);
    if ( rx_bytes[strlen(rx_bytes)-1] == '\n' )
        rx_bytes[strlen(rx_bytes)-1] = 0;

    sys_interact(tx_bytes, sizeof(tx_bytes), "ifconfig %s |grep \"TX bytes\"|awk -F '(' '{print $3}'|awk -F ')' '{print $1}'", ifname);
    if ( tx_bytes[strlen(tx_bytes)-1] == '\n' )
        tx_bytes[strlen(tx_bytes)-1] = 0;

    sys_interact(cur_tx_power, sizeof(cur_tx_power), "iwconfig %s |grep \"Tx-Power\"|awk -F ' ' '{print $4}'|cut -c 10-", ifname);
    if ( cur_tx_power[strlen(cur_tx_power)-1] == '\n' )
        cur_tx_power[strlen(cur_tx_power)-1] = 0;

#if SUPPORT_VPN_FUNCTION
    api_get_snvpn_ssid_profile_name(idx, profile_name, sizeof(profile_name));
#endif

    /*
    if (opmode != OPM_WDSAP) 
    {
        if (api_get_wifi_ifacex_guestnetwork_enable_option_by_sectionname(opmode, section, idx, guest_network, sizeof guest_network))
        {
            return FALSE;
        }
    }
    */
    if (opmode == OPM_AP)
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

    json_object_object_add(jobj, "enable", json_object_new_boolean(!disable));
    //WDS-AP not support combine ssid
    if (opmode != OPM_WDSAP)
    {
        json_object_object_add(jobj, "enable_bands", json_object_new_string(enable_bands));
    }
    json_object_object_add(jobj, "ssid_name", json_object_new_string(ssid));
    //json_object_object_add(jobj, "guest_network", json_object_new_string(guest_network));
    json_object_object_add(jobj, "hidden_ssid", json_object_new_boolean(hidden));
#if !SUPPORT_SWOS_FUNCTION
    json_object_object_add(jobj, "client_isolation", json_object_new_boolean(isolate));
#endif
    json_object_object_add(jobj, "l2_isolation", json_object_new_boolean(l2_isolation));
    json_object_object_add(jobj, "vlan_isolation", json_object_new_boolean(isolation));
    json_object_object_add(jobj, "vlan_id", json_object_new_int(vlan_id));
    json_object_object_add(jobj, "max_rx_data_rate", json_object_new_string(max_rx_data_rate));
    json_object_object_add(jobj, "rx_bytes", json_object_new_string(rx_bytes));
    json_object_object_add(jobj, "rx_packets", json_object_new_string(rx_packets));
    json_object_object_add(jobj, "max_tx_data_rate", json_object_new_string(max_tx_data_rate));
    json_object_object_add(jobj, "tx_bytes", json_object_new_string(tx_bytes));
    json_object_object_add(jobj, "tx_packets", json_object_new_string(tx_packets));
    json_object_object_add(jobj, "cur_tx_power", json_object_new_string(cur_tx_power));
#if SUPPORT_VPN_FUNCTION
    json_object_object_add(jobj, "vpn_profile_name", json_object_new_string(profile_name));
#endif
    if (opmode == OPM_AP)
    {
        json_object_object_add(jobj, "client_ip_assignment", json_object_new_string(client_ip_assignment));
    }

    return 0;
}

int json_get_wireless_band_steering(struct json_object *jobj, ssid_cfg_st *ssidCfg_p){

    int opmode = ssidCfg_p->opmode, radio_idx = 0, bandsteer = 0, bandsteerrssi = 0, bandsteerpercent = 0;
    int bandsteer_en = 0;
    char *section = ssidCfg_p->section;
    char *wifix, *bandsteer_type;
    int idx = ssidCfg_p->idx;
    if(NULL == jobj)
    {
        return FALSE;
    }

//    if(_api_get_wifi_opmode_option(&opmode))
//    {
//        return FALSE;
//    }
//
//    if(api_get_wifix_section_name(opmode, radio_idx, section))
//    {
//        return FALSE;
//    }

//    if (strcmp(radio, "2_4G") == 0)
//        wifix="wifi0";
//    else if (strcmp(radio, "5G") == 0)
//        wifix="wifi1";
//    else if (strcmp(radio, "5G-2") == 0)
//        wifix="wifi2";
//    else
//        wifix="wifi0";

    if(api_get_integer_option2(&bandsteer_en, "%s_%d.%s", section, idx, JSON_WIRELESS_WIFI_BANDSTEER_EN))
    {
        bandsteer_en = 0;
    }
    if(api_get_integer_option2(&bandsteer, "%s_%d.%s", section, idx, JSON_WIRELESS_WIFI_BANDSTEER_TYPE))
    {
        bandsteer = 0;
    }
    switch(bandsteer_en)
    {
        case FORCE_5G:

            bandsteer_type = DESC_BAND_STEER_FORCE_5G;

            break;
        case PREFER_5G:

            bandsteer_type = DESC_BAND_STEER_PREFER_5G;

            break;
        case BAND_BALANCE:

            bandsteer_type = DESC_BAND_STEER_BAND_BALANCE;

            break;
        default:
            bandsteer_type = DESC_BAND_STEER_PREFER_5G;
    }

    if ( api_get_integer_option2(&bandsteerrssi, "%s_%d.%s", section, idx, JSON_WIRELESS_WIFI_BANDSTEER_RSSI) == API_RC_SUCCESS )
    {
        bandsteerrssi -= 95;
    }

    api_get_integer_option2(&bandsteerpercent, "%s_%d.%s", section, idx, JSON_WIRELESS_WIFI_BANDSTEER_PERCENT);

    json_object_object_add(jobj, "enable", json_object_new_boolean(bandsteer_en));
    json_object_object_add(jobj, "steering_type", json_object_new_string(bandsteer_type));
    json_object_object_add(jobj, "5g_rssi_threshold", json_object_new_int(bandsteerrssi));
    json_object_object_add(jobj, "5g_client_percent", json_object_new_int(bandsteerpercent));

    return 0;
}

int json_get_client_dns_server(struct json_object *jobj, ssid_cfg_st *ssidCfg_p){

    char buf[64] = {0};
    char primary_dns[16] = {0}, secondary_dns[16] = {0};
    char *section = ssidCfg_p->section;
    bool enable = 0;
    int idx = ssidCfg_p->idx;

    if(NULL == jobj)
    {
        return FALSE;
    }

    if(api_get_dhcp_nat_dns_mode_option_by_sectionname(section, idx, buf, sizeof(buf)) == API_RC_SUCCESS)
    {
        if(strcmp(buf,"") == 0)
        {
            enable = 0;
            strcpy(primary_dns, "0.0.0.0");
            strcpy(secondary_dns, "0.0.0.0");
        }
        else
        {
            enable = 1;
            sys_interact(primary_dns, sizeof(primary_dns), "echo %s | awk -F \",\" \'{printf $2}\'", buf);
            sys_interact(secondary_dns, sizeof(secondary_dns), "echo %s | awk -F \",\" \'{printf $3}\'", buf);
        }
    }
    else
    {
        enable = 0;
        strcpy(primary_dns, "0.0.0.0");
        strcpy(secondary_dns, "0.0.0.0");
    }

    json_object_object_add(jobj, "enable", json_object_new_boolean(enable));
    json_object_object_add(jobj, "primary_dns", json_object_new_string(primary_dns));
    json_object_object_add(jobj, "secondary_dns", json_object_new_string(secondary_dns));

    return 0;
}

int json_get_wireless_traffic_shaping(struct json_object *jobj, ssid_cfg_st *ssidCfg_p){

    int opmode = ssidCfg_p->opmode, radio_idx = 0, tc_downlimit = 0, tc_uplimit = 0, tc_downperuser = 0, tc_upperuser = 0;
    int tc_enabled = 0;
    char *section = ssidCfg_p->section;
    int idx = ssidCfg_p->idx;

    if(NULL == jobj)
    {
        return FALSE;
    }

//    if(_api_get_wifi_opmode_option(&opmode))
//    {
//        return FALSE;
//    }
//
//    if(api_get_wifix_section_name(opmode, radio_idx, section))
//    {
//        return FALSE;
//    }

    if(api_get_wifi_ifacex_tc_enabled_option_by_sectionname(opmode, section, idx, &tc_enabled))
    {
        tc_enabled = 0;
    }

    api_get_wifi_ifacex_tc_downperuser_option_by_sectionname(opmode, section, idx, &tc_downperuser);

    if ( tc_downperuser )
    {
        api_get_wifi_ifacex_tc_downlimit_option_by_sectionname(opmode, section, idx, &tc_downlimit);
    }
    else
    {
        api_get_wifi_ifacex_tc_downmaxlimit_option_by_sectionname(opmode, section, idx, &tc_downlimit);
    }

    api_get_wifi_ifacex_tc_upperuser_option_by_sectionname(opmode, section, idx, &tc_upperuser);

    if ( tc_upperuser )
    {
        api_get_wifi_ifacex_tc_uplimit_option_by_sectionname(opmode, section, idx, &tc_uplimit);
    }
    else
    {
        api_get_wifi_ifacex_tc_upmaxlimit_option_by_sectionname(opmode, section, idx, &tc_uplimit);
    }

    json_object_object_add(jobj, "enable", json_object_new_boolean(tc_enabled));
    json_object_object_add(jobj, "download_limit", json_object_new_int(tc_downlimit));
    json_object_object_add(jobj, "upload_limit", json_object_new_int(tc_uplimit));
    json_object_object_add(jobj, "perclient_download_limit", json_object_new_boolean(tc_downperuser));
    json_object_object_add(jobj, "perclient_upload_limit", json_object_new_boolean(tc_upperuser));

    return 0;
}

int json_get_wireless_security(struct json_object *jobj, ssid_cfg_st *ssidCfg_p){

    int opmode = ssidCfg_p->opmode, radio_idx = 0, encr_type = 0, proxyarp_enable = 0, fastroaming_enable = 0, key_interval = 0, ieee80211w = 0;
    char *encr_desc;
    char *section = ssidCfg_p->section;
    char *cipherType;
    int idx = ssidCfg_p->idx,nasPort = 0;
    char nasid[NAS_ID_LENGTH] = {0}, nasIp[IP_LENGTH] = {0};
    int nasId_enable = 0, nasIp_enable = 0, nasPort_enable = 0;
    bool suiteb = false;
    if(NULL == jobj)
    {
        return FALSE;
    }

//    if(_api_get_wifi_opmode_option(&opmode))
//    {
//        return FALSE;
//    }
//
//    if(api_get_wifix_section_name(opmode, radio_idx, section))
//    {
//        return FALSE;
//    }

#if SUPPORT_WPA3
    if(api_get_wifi_ifacex_security_option_by_sectionname(opmode, section, idx, &encr_type) == API_RC_INTERNAL_ERROR)
    {
#endif
        if(api_get_wifi_ifacex_encryption_option_by_sectionname(opmode, section, idx, &encr_type) == API_RC_INTERNAL_ERROR)
        {
            return FALSE;
        }
#if SUPPORT_WPA3
    }
#endif

    switch(encr_type)
    {
        case ENCRYPTION_NONE:
            encr_desc = DESC_ENCRYPTION_NONE;
            break;
        case WEP_OPEN:
        case WEP_SHARED:
            encr_desc = DESC_WEP;
            break;
        case WPA_PSK_TKIP:
        case WPA_PSK_CCMP:
        case WPA_PSK_TKIP_CCMP:
            encr_desc = DESC_WPA_PSK;
            break;
        case WPA2_PSK_TKIP:
        case WPA2_PSK_CCMP:
        case WPA2_PSK_TKIP_CCMP:
            encr_desc = DESC_WPA2_PSK;
            break;
        case WPA_PSK_MIXED_TKIP:
        case WPA_PSK_MIXED_CCMP:
        case WPA_PSK_MIXED_TKIP_CCMP:
            encr_desc = DESC_WPA_PSK_MIXED;
            break;
        case WPA_EAP_TKIP:
        case WPA_EAP_CCMP:
        case WPA_EAP_TKIP_CCMP:
            encr_desc = DESC_WPA_EAP;
            break;
        case WPA2_EAP_TKIP:
        case WPA2_EAP_CCMP:
        case WPA2_EAP_TKIP_CCMP:
            encr_desc = DESC_WPA2_EAP;
            break;
        case WPA_EAP_MIXED_TKIP:
        case WPA_EAP_MIXED_CCMP:
        case WPA_EAP_MIXED_TKIP_CCMP:
            encr_desc = DESC_WPA_EAP_MIXED;
            break;
#if SUPPORT_WPA3
        case WPA3_SAE_CCMP:
            encr_desc = DESC_WPA3_SAE;
            break;
        case WPA3_SAE_MIXED_CCMP:
            encr_desc = DESC_WPA3_SAE_MIXED;
            break;
        case WPA3_EAP_CCMP:
            encr_desc = DESC_WPA3_EAP;
            break;
        case WPA3_EAP_MIXED_CCMP:
            encr_desc = DESC_WPA3_EAP_MIXED;
            break;
        case ENCRYPTION_OWE_CCMP:
            encr_desc = DESC_OWE;
            break;
#endif
        default:
            encr_desc = DESC_API_OPTION_ERROR;
    }

    switch(encr_type)
    {
        case ENCRYPTION_NONE:
            // cipherType = DESC_ENCRYPTION_NONE;
            cipherType = DESC_CCMP;
            break;
        case WEP_OPEN:
            cipherType = DESC_OPEN;
            break;
        case WEP_SHARED:
            cipherType = DESC_SHARED;
            break;
        case WPA_PSK_TKIP:
        case WPA2_PSK_TKIP:
        case WPA_PSK_MIXED_TKIP:
        case WPA_EAP_TKIP:
        case WPA2_EAP_TKIP:
        case WPA_EAP_MIXED_TKIP:
            cipherType = DESC_TKIP;
            break;
        case WPA_PSK_CCMP:
        case WPA2_PSK_CCMP:
        case WPA_PSK_MIXED_CCMP:
        case WPA_EAP_CCMP:
        case WPA2_EAP_CCMP:
        case WPA_EAP_MIXED_CCMP:
            cipherType = DESC_CCMP;
            break;
        case WPA_PSK_TKIP_CCMP:
        case WPA2_PSK_TKIP_CCMP:
        case WPA_PSK_MIXED_TKIP_CCMP:
        case WPA_EAP_TKIP_CCMP:
        case WPA2_EAP_TKIP_CCMP:
        case WPA_EAP_MIXED_TKIP_CCMP:
            cipherType = DESC_TKIP_CCMP;
            break;
#if SUPPORT_WPA3
        case WPA3_SAE_CCMP:
        case WPA3_SAE_MIXED_CCMP:
        case WPA3_EAP_CCMP:
        case WPA3_EAP_MIXED_CCMP:
        case ENCRYPTION_OWE_CCMP:
            cipherType = DESC_CCMP;
            break;
#endif
        default:
            cipherType = DESC_API_OPTION_ERROR;
    }

#if SUPPORT_WPA3
    api_get_wifi_ifacex_suiteb_option_by_sectionname(opmode, section, idx, &suiteb);
    api_get_wifi_ifacex_wpa_ieee80211w_option_by_sectionname(opmode, section, idx, &ieee80211w);
#endif
    api_get_wifi_ifacex_wpa_group_rekey_option_by_sectionname(opmode, section, idx, &key_interval);

    api_get_wifi_ifacex_proxyarp_enable_option_by_sectionname(opmode, section, idx, &proxyarp_enable);

    api_get_wifi_ifacex_fastroaming_enable_option_by_sectionname(opmode, section, idx, &fastroaming_enable);

    api_get_wifi_ifacex_nasid_enabled_option_by_sectionname(opmode, section, idx, &nasId_enable);

    api_get_wifi_ifacex_nasid_option_by_sectionname(opmode, section, idx, nasid, sizeof(nasid));

    api_get_wifi_ifacex_nasip_enabled_option_by_sectionname(opmode, section, idx, &nasIp_enable);

    api_get_wifi_ifacex_nasip_option_by_sectionname(opmode, section, idx, nasIp, sizeof(nasIp));

    api_get_wifi_ifacex_nasport_enabled_option_by_sectionname(opmode, section, idx, &nasPort_enable);

    api_get_wifi_ifacex_nasport_option_by_sectionname(opmode, section, idx, &nasPort);

    json_object_object_add(jobj, "encryption", json_object_new_string(encr_desc));
    json_object_object_add(jobj, "auth_type", json_object_new_string(cipherType));
    json_object_object_add(jobj, "suiteb_192bits", json_object_new_boolean(suiteb));
    json_object_object_add(jobj, "ieee80211w", json_object_new_int(ieee80211w));
    json_object_object_add(jobj, "key_interval", json_object_new_int(key_interval));
    json_object_object_add(jobj, "nasId_enable", json_object_new_boolean(nasId_enable));
    json_object_object_add(jobj, "nasId", json_object_new_string(nasid));
    json_object_object_add(jobj, "nasIp_enable", json_object_new_boolean(nasIp_enable));
    json_object_object_add(jobj, "nasIp", json_object_new_string(nasIp));
    json_object_object_add(jobj, "nasPort_enable", json_object_new_boolean(nasPort_enable));
    json_object_object_add(jobj, "nasPort", json_object_new_int(nasPort));
    json_object_object_add(jobj, "proxyarp", json_object_new_boolean(proxyarp_enable));
    if( strstr(section,"enjet") )
    {
        debug_print("[DEBUG] enjet not support FAST ROAMIN.\n");
    }
    else
    {
        json_object_object_add(jobj, "fast_roaming", json_object_new_boolean(fastroaming_enable));
    }

    return 0;
}

int json_get_wireless_encryption(struct json_object *jobj, ssid_cfg_st *ssidCfg_p){

    int opmode = ssidCfg_p->opmode, radio_idx = 0, encr_type = 0;
    char passphrase[PASSPHRASE_LENGTH] = {0};
    char *section = ssidCfg_p->section;
    int idx = ssidCfg_p->idx;

    if(NULL == jobj)
    {
        return FALSE;
    }

//    if(_api_get_wifi_opmode_option(&opmode))
//    {
//        return FALSE;
//    }
//
//    if(api_get_wifix_section_name(opmode, radio_idx, section))
//    {
//        return FALSE;
//    }

    api_get_wifi_ifacex_encryption_option_by_sectionname(opmode, section, idx, &encr_type);


    api_get_wifi_ifacex_wpakey_key_option_by_sectionname(opmode, section, idx, passphrase, sizeof(passphrase));

    json_object_object_add(jobj, "passphrase", json_object_new_string(passphrase));

    return 0;
}

int json_get_wireless_radius_server(struct json_object *jobj, ssid_cfg_st *ssidCfg_p){

    int opmode = ssidCfg_p->opmode, radio_idx = 0, retries = 0, server1Port = 0, server2Port = 0;
    char server1Ip[IP_LENGTH] = {0}, server1Secret[65] = {0}, *server2Ip, *server2Secret;

    char passphrase[PASSPHRASE_LENGTH] = {0};
    char *section = ssidCfg_p->section;
    int idx = ssidCfg_p->idx;

    if(NULL == jobj)
    {
        return FALSE;
    }

//    if(_api_get_wifi_opmode_option(&opmode))
//    {
//        return FALSE;
//    }
//
//    if(api_get_wifix_section_name(opmode, radio_idx, section))
//    {
//        return FALSE;
//    }

//    if(api_get_wifi_ifacex_retries_option_by_sectionname(opmode, section, idx, &retries))
//    {
//        return FALSE;
//    }


    api_get_wifi_ifacex_auth_server_option_by_sectionname(opmode, section, idx, server1Ip, sizeof(server1Ip));

    api_get_wifi_ifacex_auth_port_option_by_sectionname(opmode, section, idx, &server1Port);

    api_get_wifi_ifacex_auth_secret_option_by_sectionname(opmode, section, idx, server1Secret, sizeof(server1Secret));

//    json_object_object_add(jobj, "retries", json_object_new_int(retries));
    json_object_object_add(jobj, "server1Ip", json_object_new_string(server1Ip));
    json_object_object_add(jobj, "server1Port", json_object_new_int(server1Port));
    json_object_object_add(jobj, "server1Secret", json_object_new_string(server1Secret));
//    json_object_object_add(jobj, "server2Ip", json_object_new_string(server1Ip));
//    json_object_object_add(jobj, "server2Port", json_object_new_int(server1Port));
//    json_object_object_add(jobj, "server2Secret", json_object_new_string(server1Secret));

    return 0;
}

int json_get_wireless_accounting_server(struct json_object *jobj, ssid_cfg_st *ssidCfg_p){

    int opmode = ssidCfg_p->opmode, radio_idx = 0, server1Port = 0, server2Port = 0;
    int acct_enable = 0, acct_interval = 0;
    char server1Ip[IP_LENGTH] = {0}, server1Secret[128] = {0}, *server2Ip, *server2Secret;
    char *section = ssidCfg_p->section;
    int idx = ssidCfg_p->idx;

    if(NULL == jobj)
    {
        return FALSE;
    }

//    if(_api_get_wifi_opmode_option(&opmode))
//    {
//        return FALSE;
//    }
//
//    if(api_get_wifix_section_name(opmode, radio_idx, section))
//    {
//        return FALSE;
//    }

    api_get_wifi_ifacex_acct_enabled_option_by_sectionname(opmode, section, idx, &acct_enable);

    api_get_wifi_ifacex_acct_server_option_by_sectionname(opmode, section, idx, server1Ip, sizeof(server1Ip));

    api_get_wifi_ifacex_acct_port_option_by_sectionname(opmode, section, idx, &server1Port);

    api_get_wifi_ifacex_acct_secret_option_by_sectionname(opmode, section, idx, server1Secret, sizeof(server1Secret));

    api_get_wifi_ifacex_acct_interval_option_by_sectionname(opmode, section, idx, &acct_interval);

    json_object_object_add(jobj, "enable", json_object_new_boolean(acct_enable));
    json_object_object_add(jobj, "interval", json_object_new_int(acct_interval));
    json_object_object_add(jobj, "server1Ip", json_object_new_string(server1Ip));
    json_object_object_add(jobj, "server1Port", json_object_new_int(server1Port));
    json_object_object_add(jobj, "server1Secret", json_object_new_string(server1Secret));
//    json_object_object_add(jobj, "server2Ip", json_object_new_string(server1Ip));
//    json_object_object_add(jobj, "server2Port", json_object_new_int(server1Port));
//    json_object_object_add(jobj, "server2Secret", json_object_new_string(server1Secret));

    return 0;
}

int json_get_wireless_guest_network(struct json_object *jobj, ssid_cfg_st *ssidCfg_p){

    char guest_network[20] = {0};
    char *section = ssidCfg_p->section;
    int disable = 0, opmode = ssidCfg_p->opmode;
    int idx = ssidCfg_p->idx;
    struct json_object *jobj_nat = NULL, *jobj_client_dns_server=NULL;
    char ip[32]={0}, mask[32]={0}, start_ip[32]={0}, end_ip[32]={0}, lease_time[8]={0};

    if(NULL == jobj)
    {
        return FALSE;
    }

    api_get_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, guest_network, sizeof guest_network);

    if (strcmp(guest_network,"Disable") == 0 || strcmp(guest_network,"") == 0)
    {
        disable = 1;
    }
    else //NAT or Bridge
    {
        disable = 0;
    }

    json_object_object_add(jobj, "enable", json_object_new_boolean(!disable));

#if SUPPORT_SWOS_FUNCTION
    jobj_nat = json_object_new_object();

    api_get_network_nat_ip_address_option_by_sectionname(section, idx, ip, sizeof(ip));
    json_object_object_add(jobj_nat, "ip", json_object_new_string(ip));

    api_get_network_nat_subnet_mask_option_by_sectionname(section, idx, mask, sizeof(mask));
    json_object_object_add(jobj_nat, "subnet_mask", json_object_new_string(mask));

    api_get_dhcp_nat_start_ip_option_by_sectionname(section, idx, start_ip, sizeof(start_ip));
    json_object_object_add(jobj_nat, "start_ip", json_object_new_string(start_ip));

    api_get_dhcp_nat_end_ip_option_by_sectionname(section, idx, end_ip, sizeof(end_ip));
    json_object_object_add(jobj_nat, "end_ip", json_object_new_string(end_ip));

    api_get_dhcp_nat_lease_time_option_by_sectionname(section, idx, lease_time, sizeof(lease_time));
    json_object_object_add(jobj_nat, "client_lease_time", json_object_new_string(lease_time));

    jobj_client_dns_server = json_object_new_object();
    json_get_client_dns_server(jobj_client_dns_server, ssidCfg_p);
    json_object_object_add(jobj_nat, "client_dns_server", jobj_client_dns_server);
    json_object_object_add(jobj, "manual_nat", jobj_nat);
#endif

    return 0;
}

int json_get_wireless_captive_portal(struct json_object *jobj, ssid_cfg_st *ssidCfg_p){

    char guest_network[16] = {0};
    char *section = ssidCfg_p->section;
    int disable = 0, opmode = ssidCfg_p->opmode;
    int idx = ssidCfg_p->idx;

    if(NULL == jobj)
    {
        return FALSE;
    }

    api_get_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, guest_network, sizeof guest_network);

    if (strcmp(guest_network,"Disable") == 0 || strcmp(guest_network,"Enable") == 0 || strcmp(guest_network,"") == 0 || strcmp(guest_network,"NAT_only") == 0)
    {
        disable = 1;
    }
    else //NAT or Bridge
    {
        disable = 0;
    }

    json_object_object_add(jobj, "enable", json_object_new_boolean(!disable));

#if SUPPORT_CAPTIVE_PORTAL_SETTING
    char ext_url[SECTION_NAME_LENGTH] = {0}, walled_garden[2048] = {0}, auth_type_str[20] = {0};
#if SUPPORT_SWOS_FUNCTION    
    char shared_secret[128] = {0}, splash_url[256] = {0};
#endif
    int auth_type = 0, session_timeout = 0, idle_timeout = 0;

    api_get_portal_auth_type_option_by_sectionname(NULL, idx, &auth_type);

    if (auth_type == 1)
    {
        strcpy(auth_type_str, "splash");
    }
    else if(auth_type == 2)
    {
        strcpy(auth_type_str, "cloud-radius"); // or social-login
    }
    else if (auth_type == 3)
    {
        strcpy(auth_type_str, "custom-radius");
    }
    else if (auth_type == 300)
    {
        strcpy(auth_type_str, "click-through");
    }
    else
    {
        strcpy(auth_type_str, "");
    }

#if SUPPORT_SWOS_FUNCTION
    if (api_get_portal_intSplashUrl_option_by_sectionname(guest_network, idx, splash_url, sizeof(splash_url)))
    {
        strcpy(splash_url,"");
    }

    api_get_portal_sharedsecret_option_by_sectionname(guest_network, idx, shared_secret, sizeof(shared_secret));
#else
    if (api_get_portal_extSplashUrl_option_by_sectionname(guest_network, idx, ext_url, sizeof(ext_url)))
    {
        strcpy(ext_url,"");
    }
#endif

    api_get_portal_sessionTimeout_option_by_sectionname(guest_network, idx, &session_timeout);

    api_get_portal_idleTimeout_option_by_sectionname(guest_network, idx, &idle_timeout);

    if ( api_get_portal_walledGarden_option_by_sectionname(guest_network, idx, walled_garden, sizeof(walled_garden)))
    {
        strcpy(walled_garden,"");
    }

    json_object_object_add(jobj, "auth_type", json_object_new_string(auth_type_str));
#if SUPPORT_SWOS_FUNCTION
    json_object_object_add(jobj, "splash_url", json_object_new_string(splash_url));
    json_object_object_add(jobj, "shared_secret", json_object_new_string(shared_secret));
#else
    json_object_object_add(jobj, "external_splash_url", json_object_new_string(ext_url));
#endif
    json_object_object_add(jobj, "session_timeout", json_object_new_int(session_timeout));
    json_object_object_add(jobj, "idle_timeout", json_object_new_int(idle_timeout));
    json_object_object_add(jobj, "walled_garden", json_object_new_string(walled_garden));
#endif

    return 0;
}

int json_get_wireless_scheduling(struct json_object *jobj, ssid_cfg_st *ssidCfg_p)
{
    int output = 0, status = 0, radio_idx = 0;
    char templates[50] = {0};
    char iface[8] = {0};
    int idx = ssidCfg_p->idx;

    if(NULL == jobj)
    {
        return FALSE;
    }

    api_get_wifi_schedule_enable_option_by_sectionname(&output);

    json_object_object_add(jobj, "enable", json_object_new_boolean(output));

    return 0;
}

int json_get_wireless_scheduling_days(struct json_object *jobj, ssid_cfg_st *ssidCfg_p, int day)
{
    char time_Start[20] = "00:00", time_End[20] = "24:00";
    char iface[8];
    int  radio_idx = 0, wifitbl_idx = 0, schedule_idx = 0;
    int status = 1;
    int idx = ssidCfg_p->idx;
    char *section = ssidCfg_p->section;
    int opmode = ssidCfg_p->opmode;

    if(NULL == jobj)
    {
        return FALSE;
    }

    if ( strstr(section, "wifi0") ) 
    {
        radio_idx = 0;
    }
    else if( strstr(section, "wifi1") ) 
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

    if(api_find_wifi_schedule_index_by_ifname(iface, &wifitbl_idx) == API_RC_SUCCESS){
        schedule_idx = wifitbl_idx+day;
        api_get_wifi_schedule_tablex_status_option(schedule_idx, &status);
        api_get_wifi_schedule_tablex_start_time_option(schedule_idx, time_Start, sizeof (time_Start));
        api_get_wifi_schedule_tablex_end_time_option(schedule_idx, time_End, sizeof (time_End));
    }

    json_object_object_add(jobj, "available", json_object_new_boolean(status));
    json_object_object_add(jobj, "start", json_object_new_string(time_Start));
    json_object_object_add(jobj, "end", json_object_new_string(time_End));

    return 0;
}

int json_get_wireless_l2_acl(struct json_object *jobj, ssid_cfg_st *ssidCfg_p){

    int opmode = ssidCfg_p->opmode, radio_idx = 0, acl_enable = 0, macfilter_index = 0;
    char *section = ssidCfg_p->section;
    char macfilter[ACL_MACFILTER_LENGTH], maclist[ACL_MACLIST_LENGTH], *policy;
    int idx = ssidCfg_p->idx;

    if(NULL == jobj)
    {
        return FALSE;
    }

    api_get_wifi_ifacex_macfilter_option_by_sectionname(opmode, section,idx, &macfilter_index);


    // api_get_string_option2(macfilter, sizeof(macfilter), "%s_%d.%s", section, idx, JSON_WIRELESS_WIFI_ACL_MACFILTER);

    if(macfilter_index == 1)
    {
        policy = DESC_ACL_ALLOW;
        acl_enable = 1;
        if ( strstr(section,"enjet") )
        {
            api_get_string_option2(maclist, sizeof(maclist), "%s.allowmaclist", section);
        }
        else
        {
            api_get_string_option2(maclist, sizeof(maclist), "%s_%d.allowmaclist", section, idx);
        }
    }
    else if(macfilter_index == 2)
    {
        policy = DESC_ACL_DENY;
        acl_enable = 1;
        if ( strstr(section,"enjet") )
        {
            api_get_string_option2(maclist, sizeof(maclist), "%s.denymaclist", section);
        }
        else
        {
            api_get_string_option2(maclist, sizeof(maclist), "%s_%d.denymaclist", section, idx);
        }
    }
    else
    {
        policy = "";
        acl_enable = 0;
        strcpy(maclist, "");
    }

    json_object_object_add(jobj, "enable", json_object_new_boolean(acl_enable));
    json_object_object_add(jobj, "client_mac_list", json_object_new_string(maclist));
    json_object_object_add(jobj, "policy", json_object_new_string(policy));

    return 0;
}

int json_get_wireless_traffic_info(struct json_object *jobj, ssid_cfg_st *ssidCfg_p)
{
    struct json_object *jobj_array = NULL, *jobj_client = NULL;
    char *section = ssidCfg_p->section;
    int idx = ssidCfg_p->idx, opmode = ssidCfg_p->opmode;
    char ifname[12] = {0};
    char pstr[256] = {0}, wlanconfig_str[512] = {0};
    int client_num = 0, client_idx = 0;
    char wlan_mac[24] = {0}, rssi[12] = {0}, tx_bytes[256] = {0}, rx_bytes[256] = {0}, tx_rate[24] = {0}, rx_rate[24] = {0};
#if SUPPORT_WATCHGUARD_FUNCTION
    unsigned long long wlan_tx_byte = 0, wlan_rx_byte = 0;
#else
    int wlan_tx_byte = 0, wlan_rx_byte = 0;
#endif

    jobj_array = json_object_new_array();

    if(api_get_wifi_ifacex_ifname_option_by_sectionname(opmode, section, idx, ifname, sizeof(ifname)))
    {
        return FALSE;
    }
    debug_print("%s[%d]===>ifname[%s]\n", __FUNCTION__, __LINE__,ifname);

    sys_interact(pstr, sizeof(pstr), "wlanconfig %s list sta | tail -n +2 | wc -l", ifname);

    if ( sscanf( pstr, "%d", &client_num ) == 1 && client_num != 0 )
    {
        for ( client_idx = 0 ; client_idx < client_num ; client_idx++ )
        {
            jobj_client = json_object_new_object();

            memset(wlanconfig_str, 0, sizeof(wlanconfig_str));
            memset(wlan_mac, 0, sizeof(wlan_mac));

            sys_interact(wlanconfig_str, sizeof(wlanconfig_str), "wlanconfig %s list sta | tail -n +2 | sed -n \"%d,%dp\"", ifname, client_idx+1, client_idx+1);
            if ( strlen(wlanconfig_str) > 0 )
                wlanconfig_str[strcspn(wlanconfig_str, "\n")] = '\0';
            else
                continue;

            // get mac
            sys_interact(pstr, sizeof(pstr), "echo %s | awk -F \" \" \'{print $1}\'", wlanconfig_str);
            if ( strlen(pstr) > 0 )
            {
                pstr[strcspn(pstr, "\n")] = '\0';
                snprintf(wlan_mac, sizeof(wlan_mac)-1, "%s", pstr);
            }
            else
                continue;

            // get tx rate
            sys_interact(tx_rate, sizeof(tx_rate), "echo %s | awk -F \" \" \'{print $4}\'", wlanconfig_str);
            if ( strlen(tx_rate) > 0 )
            {
                tx_rate[strcspn(tx_rate, "\n")] = '\0';
                if ( strstr(tx_rate, "M") )
                    tx_rate[strcspn(tx_rate, "M")] = '\0';
            }
            else
                continue;

            // get rx rate
            sys_interact(rx_rate, sizeof(rx_rate), "echo %s | awk -F \" \" \'{print $5}\'", wlanconfig_str);
            if ( strlen(rx_rate) > 0 )
            {
                rx_rate[strcspn(rx_rate, "\n")] = '\0';
                if ( strstr(rx_rate, "M") )
                    rx_rate[strcspn(rx_rate, "M")] = '\0';
            }
            else
                continue;

            // get rssi
            sys_interact(rssi, sizeof(rssi), "echo %s | awk -F \" \" \'{print $6}\'", wlanconfig_str);
            if ( strlen(rssi) > 0 )
            {
                rssi[strcspn(rssi,"\n")] = '\0';
            }
            else
                continue;

            // get tx byte
            sys_interact(pstr, sizeof(pstr), "echo %s | awk -F \" \" \'{print $7}\'", wlanconfig_str);
            if ( strlen(pstr) > 0 )
            {
                pstr[strcspn(pstr, "\n")] = '\0';
                wlan_tx_byte = convert_to_bytes(pstr);
#if SUPPORT_WATCHGUARD_FUNCTION
                sprintf(tx_bytes, "%llu", wlan_tx_byte);
#else
                sprintf(tx_bytes, "%d", wlan_tx_byte);
#endif
            }
            else
                continue;

            // get rx byte
            sys_interact(pstr, sizeof(pstr), "echo %s | awk -F \" \" \'{print $8}\'", wlanconfig_str);
            if ( strlen(pstr) > 0 )
            {
                pstr[strcspn(pstr,"\n")] = '\0';
                wlan_rx_byte = convert_to_bytes(pstr);
#if SUPPORT_WATCHGUARD_FUNCTION
                sprintf(rx_bytes, "%llu", wlan_rx_byte);
#else
                sprintf(rx_bytes, "%d", wlan_rx_byte);
#endif
            }
            else
                continue;

            json_object_object_add(jobj_client, "mac_addr", json_object_new_string((wlan_mac[0]==0)? "00:00:00:00:00:00" : wlan_mac));
            json_object_object_add(jobj_client, "tx_rate", json_object_new_string(tx_rate));
            json_object_object_add(jobj_client, "rx_rate", json_object_new_string(rx_rate));
            json_object_object_add(jobj_client, "signal_strength", json_object_new_string(rssi));
            json_object_object_add(jobj_client, "tx_bytes", json_object_new_string(tx_bytes));
            json_object_object_add(jobj_client, "rx_bytes", json_object_new_string(rx_bytes));

            json_object_array_add(jobj_array, jobj_client);
        }
        json_object_object_add(jobj, "connection_list", jobj_array);
    }

    return 0;
}

int json_set_wireless_ssid(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){
    int radio_idx = 0, opmode = ssidCfg_p->opmode,  vlan_id = 0;
    char *ssid=NULL, *enable_bands=NULL, *guest_network = NULL, *client_ip_assignment = NULL, *profile_name = NULL;
    char *section = ssidCfg_p->section;
    char *bs_jsonStr=NULL, *tf_jsonStr=NULL, *cd_jsonStr=NULL, *security_jsonStr=NULL, *cp_jsonStr=NULL, *scheduling_jsonStr=NULL, *l2_acl_jsonStr=NULL, *captive_portal_jsonStr=NULL;
    bool enable = 0, hidden = 0, isolate = 0, l2_isolation = 0, isolation = 0;
    struct json_object *jobj;
    int radio_start = 0, radio_end = 0, i = 0;
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
            senao_json_object_get_boolean(jobj, "enable",&(enable));
            senao_json_object_get_and_create_string(rep, jobj, "enable_bands", &enable_bands);
            senao_json_object_get_and_create_string(rep, jobj, "ssid_name", &ssid);
            //senao_json_object_get_and_create_string(rep, jobj, "guest_network", &guest_network);
            senao_json_object_get_boolean(jobj, "hidden_ssid",&(hidden));
            senao_json_object_get_boolean(jobj, "client_isolation",&(isolate));
            senao_json_object_get_boolean(jobj, "l2_isolation",&(l2_isolation));
            senao_json_object_get_boolean(jobj, "vlan_isolation",&(isolation));
            senao_json_object_get_integer(jobj, "vlan_id",&(vlan_id));
            senao_json_object_get_integer(jobj, "client_ip_assignment",&(client_ip_assignment));
            senao_json_object_get_and_create_string(rep, jobj, "wireless_security", &security_jsonStr);
            senao_json_object_get_and_create_string(rep, jobj, "band_steering", &bs_jsonStr);
            senao_json_object_get_and_create_string(rep, jobj, "traffic_shaping", &tf_jsonStr);
            senao_json_object_get_and_create_string(rep, jobj, "guest_network", &cp_jsonStr);
#if !SUPPORT_SWOS_FUNCTION
            senao_json_object_get_and_create_string(rep, jobj, "captive_portal", &captive_portal_jsonStr);
            senao_json_object_get_and_create_string(rep, jobj, "client_dns_server", &cd_jsonStr);
#endif
            senao_json_object_get_and_create_string(rep, jobj, "scheduling", &scheduling_jsonStr);
            senao_json_object_get_and_create_string(rep, jobj, "l2_acl", &l2_acl_jsonStr);
#if SUPPORT_VPN_FUNCTION
            senao_json_object_get_and_create_string(rep, jobj, "vpn_profile_name", &profile_name);
#endif
        }
        else
        {
            //RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
        }
    }
    else
    {
        //RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
    }
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);

//    if(_api_get_wifi_opmode_option(&opmode))
//    {
//        return FALSE;
//    }
//
//    if(api_get_wifix_section_name(opmode, radio_idx, section))
//    {
//        return FALSE;
//    }

    // close all radio first
    for ( i = radio_start ; i <= radio_end ; i++ )
    {
        set_wifi_ifacex_disabled(i,idx,1,ssidCfg_p);
    }

debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    // enable need radio second
    if ( enable )
    {
        if (opmode == OPM_WDSAP)
        {
            if(strcmp(enable_bands , "") == 0)
            {
                debug_print("[DEBUG] WDS-AP not support enableband.\n");
            }
            else
            {
                debug_print("[DEBUG] Error! WDS-AP not support enableband.\n");
                //RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
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

        if(api_set_wifi_ifacex_ssid_option_by_sectionname(opmode, section, idx, ssid, sizeof ssid))
        {
            //RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SSID NAME");
        }
        /*
        if ( opmode == OPM_AP) 
        {
            if (api_set_wifi_ifacex_guestnetwork_enable_option_by_sectionname(opmode, section, idx, guest_network, sizeof guest_network))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "GUEST NETWORK");
            }
        }*/
        if(api_set_wifi_ifacex_hidden_option_by_sectionname(opmode, section, idx, hidden))
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HIDDEN SSID");
        }

        if(api_set_wifi_ifacex_isolate_option_by_sectionname(opmode, section, idx, isolate))
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "HIDDEN SSID");
        }

        if(api_set_wifi_ifacex_l2_isolation_option_by_sectionname(opmode, section, idx, l2_isolation))
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "L2 ISOLATION");
        }

#if SUPPORT_SWOS_FUNCTION
        api_get_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, gn_status, sizeof gn_status);

        if (strcmp(gn_status,"Disable") == 0 || strcmp(gn_status,"") == 0)
        
#endif
        {
            if(api_set_wifi_ifacex_isolation_option_by_sectionname(opmode, section, idx, isolation))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "VLAN ISOLATION");
            }
        }

debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
        if (isolation == 1)
        {
            if (api_set_wifi_ifacex_vlan_id_option_by_sectionname(opmode, section, idx, vlan_id))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "VLAN ID");
            }
        }

        if ( ssidCfg_p->opmode == OPM_AP )
        {
            if (radio_num == 1)
            {
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
                return  json_set_wireless_traffic_shaping(rep, tf_jsonStr, ssidCfg_p)
                    || json_set_wireless_security(rep, security_jsonStr, ssidCfg_p)
                    || json_set_wireless_guest_network(rep, cp_jsonStr, ssidCfg_p)
#if !SUPPORT_SWOS_FUNCTION
                    || json_set_wireless_captive_portal(rep, captive_portal_jsonStr, ssidCfg_p, client_ip_assignment)
                    || json_set_client_dns_server(rep, cd_jsonStr, ssidCfg_p, client_ip_assignment)
#endif
                    || json_set_wireless_scheduling_sync(rep, scheduling_jsonStr, ssidCfg_p)
                    || json_set_wireless_l2_acl(rep, l2_acl_jsonStr, ssidCfg_p);
    debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
            }
            else
            {
                return  json_set_wireless_band_steering(rep, bs_jsonStr, ssidCfg_p)
                    || json_set_wireless_traffic_shaping(rep, tf_jsonStr, ssidCfg_p)
                    || json_set_wireless_security(rep, security_jsonStr, ssidCfg_p)
                    || json_set_wireless_guest_network(rep, cp_jsonStr, ssidCfg_p)
#if !SUPPORT_SWOS_FUNCTION
                    || json_set_wireless_captive_portal(rep, captive_portal_jsonStr, ssidCfg_p, client_ip_assignment)
                    || json_set_client_dns_server(rep, cd_jsonStr, ssidCfg_p, client_ip_assignment)
#endif
                    || json_set_wireless_scheduling_sync(rep, scheduling_jsonStr, ssidCfg_p)
                    || json_set_wireless_l2_acl(rep, l2_acl_jsonStr, ssidCfg_p);
            }
        }
        else if (ssidCfg_p->opmode == OPM_WDSAP) 
        {
            return json_set_wireless_traffic_shaping(rep, tf_jsonStr, ssidCfg_p) 
                || json_set_wireless_security(rep, security_jsonStr, ssidCfg_p)
                || json_set_wireless_scheduling_sync(rep, scheduling_jsonStr, ssidCfg_p)
                || json_set_wireless_l2_acl(rep, l2_acl_jsonStr, ssidCfg_p);
        } else
        {
            return  json_set_wireless_security(rep, security_jsonStr, ssidCfg_p)
                || json_set_wireless_scheduling_sync(rep, scheduling_jsonStr, ssidCfg_p)
                || json_set_wireless_l2_acl(rep, l2_acl_jsonStr, ssidCfg_p);
        }
#if SUPPORT_VPN_FUNCTION
        api_set_snvpn_ssid_profile_name(idx, profile_name, sizeof(profile_name));
        api_set_snvpn_ssid_enable(idx, 1);
#endif
    }
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_wireless_band_steering(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){

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
            senao_json_object_get_boolean(jobj, "enable",&(bandsteer_en));
            senao_json_object_get_and_create_string(rep, jobj, "steering_type", &steering_type);
            senao_json_object_get_integer(jobj, "5g_rssi_threshold",&(bandsteerrssi));
            senao_json_object_get_integer(jobj, "5g_client_percent",&(bandsteerpercent));
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
//    if (strcmp(radio, WIFI_RADIO_NAME_24G) == 0)
//        wifix="wifi0";
//    else if (strcmp(radio, WIFI_RADIO_NAME_5G) == 0)
//        wifix="wifi1";
//    else if (strcmp(radio, WIFI_RADIO_NAME_5G_2) == 0)
//        wifix="wifi2";
//    else
//        wifix="wifi0";

    //GET_SECTION_NAME(radio_idx, opmode, section, res);

    if(api_set_integer_option2(bandsteer_en, "%s_%d.%s", section, idx, JSON_WIRELESS_WIFI_BANDSTEER_EN))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BAND STEERING ENABLE");
    }

	if (bandsteer_en==0)
	{
		RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
	}

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

	if (bandsteer == FORCE_5G)
	{
		//if bandsteer type is force 5G, no need to set RSSI and percent.
		RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
	}

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

	if (bandsteer == PREFER_5G)
	{
		//if bandsteer type is prefer 5G, no need to set percent.
		RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
	}

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


    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_client_dns_server(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char *option)
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

    if (strcmp(client_ip_assignment, "NAT") == 0){
        if(enable == 0)
        {
            strcpy(buf, "");
        }
        else
        {
            snprintf(buf, sizeof(buf), "6,%s,%s", primary_dns, secondary_dns);
        }
        if(api_set_dhcp_nat_dns_mode_option_by_sectionname(section, idx, buf, sizeof(buf)) != API_RC_SUCCESS)
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT DNS SERVER");
        }
#if SUPPORT_CAPTIVE_PORTAL_SETTING
        if(api_set_portal_customdns_option_by_sectionname(section, idx, enable) != API_RC_SUCCESS)
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT DNS SERVER ENABLE");
        }
        if(api_set_portal_dns_option_by_sectionname(section, idx, primary_dns, sizeof(primary_dns), 1) != API_RC_SUCCESS)
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT DNS SERVER");
        }
        if(api_set_portal_dns_option_by_sectionname(section, idx, secondary_dns, sizeof(secondary_dns), 2) != API_RC_SUCCESS)
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT DNS SERVER");
        }
#endif
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT DNS SERVER");
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_wireless_traffic_shaping(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){
    int opmode = ssidCfg_p->opmode, radio_idx = 0, tc_downlimit = 0, tc_uplimit = 0;
    bool tc_enabled = 0, tc_downperuser = 0, tc_upperuser = 0;
    char *section = ssidCfg_p->section;
    struct json_object *jobj;
    ResponseStatus *res = rep->res;
    int idx = ssidCfg_p->idx;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_boolean(jobj, "enable",&(tc_enabled));
            senao_json_object_get_integer(jobj, "download_limit", &(tc_downlimit));
            senao_json_object_get_integer(jobj, "upload_limit",&(tc_uplimit));
            senao_json_object_get_boolean(jobj, "perclient_download_limit",&(tc_downperuser));
            senao_json_object_get_boolean(jobj, "perclient_upload_limit",&(tc_upperuser));
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

    //GET_SECTION_NAME(radio_idx, opmode, section, res);

//    if(_api_get_wifi_opmode_option(&opmode))
//    {
//        return FALSE;
//    }
//
//    if(api_get_wifix_section_name(opmode, radio_idx, section))
//    {
//        return FALSE;
//    }

    if(api_set_integer_option2(tc_enabled, WIRELESS_WIFI_OPTION_FORMAT_WITH_INDEX, section, idx, "tc_enabled"))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TRAFFIC SHAPING ENABLE");
    }

    if ( !tc_enabled )
    {
        RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
    }

    if(api_set_wifi_ifacex_tc_downperuser_option_by_sectionname(opmode, section, idx, tc_downperuser))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PERCLIENT DOWNLOAD LIMIT");
    }

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

    if(api_set_wifi_ifacex_tc_upperuser_option_by_sectionname(opmode, section, idx, tc_upperuser))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PERCLIENT UPLOAD LIMIT");
    }

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

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_wireless_security(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){
    int opmode = ssidCfg_p->opmode, radio_idx = 0, encr_type = 0, security_mode = 0, key_interval = 0, nasPort = 0;
    bool fastroaming_enable = false, nasId_enable = false, nasIp_enable = false, nasPort_enable = false;
    char *auth_type=NULL, *encryption=NULL, *jsonStr=NULL, *radius_jsonStr=NULL, *accounting_jsonStr=NULL;
    char *section = ssidCfg_p->section, *nasIp=NULL, *nasid=NULL;
    int idx = ssidCfg_p->idx;
 
    struct json_object *jobj;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "encryption", &encryption);
            senao_json_object_get_and_create_string(rep, jobj, "auth_type", &auth_type);
            senao_json_object_get_integer(jobj, "key_interval", &key_interval);
            senao_json_object_get_boolean(jobj, "nasId_enable",&(nasId_enable));
            senao_json_object_get_and_create_string(rep, jobj, "nasId", &nasid);
            senao_json_object_get_boolean(jobj, "nasIp_enable",&(nasIp_enable));
            senao_json_object_get_and_create_string(rep, jobj, "nasIp", &nasIp);
            senao_json_object_get_boolean(jobj, "nasPort_enable",&(nasPort_enable));
            senao_json_object_get_integer(jobj, "nasPort",&(nasPort));
            senao_json_object_get_boolean(jobj, "fast_roaming",&(fastroaming_enable));
            senao_json_object_get_and_create_string(rep, jobj, "accounting_server", &accounting_jsonStr);
            senao_json_object_get_and_create_string(rep, jobj, "radius_server", &radius_jsonStr);
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

    //GET_SECTION_NAME(radio_idx, opmode, section, res);

    if ( strcmp( encryption, "WPA-PSK" ) == 0 )
    {
        security_mode = 1;
        if ( strcmp( auth_type, "AES" ) == 0 )
            encr_type = WPA_PSK_CCMP;
        else if ( strcmp( auth_type, "TKIP" ) == 0 )
            encr_type = WPA_PSK_TKIP;
        else
            encr_type = WPA_PSK_TKIP_CCMP;
    }
    else if ( strcmp( encryption, "WPA2-PSK" ) == 0 )
    {
        security_mode = 1;
        if ( strcmp( auth_type, "AES" ) == 0 )
            encr_type = WPA2_PSK_CCMP;
        else if ( strcmp( auth_type, "TKIP" ) == 0 )
            encr_type = WPA2_PSK_TKIP;
        else
            encr_type = WPA2_PSK_TKIP_CCMP;
    }
    else if ( strcmp( encryption, "WPA-PSK Mixed" ) == 0 )
    {
        security_mode = 1;
        if ( strcmp( auth_type, "AES" ) == 0 )
            encr_type = WPA_PSK_MIXED_CCMP;
        else if ( strcmp( auth_type, "TKIP" ) == 0 )
            encr_type = WPA_PSK_MIXED_TKIP;
        else
            encr_type = WPA_PSK_MIXED_TKIP_CCMP;
    }
    else if ( strcmp( encryption, "WPA-Enterprise" ) == 0 )
    {
        security_mode = 2;
        if ( strcmp( auth_type, "AES" ) == 0 )
            encr_type = WPA_EAP_CCMP;
        else if ( strcmp( auth_type, "TKIP" ) == 0 )
            encr_type = WPA_EAP_TKIP;
        else
            encr_type = WPA_EAP_TKIP_CCMP;
    }
    else if ( strcmp( encryption, "WPA2-Enterprise" ) == 0 )
    {
        security_mode = 2;
        if ( strcmp( auth_type, "AES" ) == 0 )
            encr_type = WPA2_EAP_CCMP;
        else if ( strcmp( auth_type, "TKIP" ) == 0 )
            encr_type = WPA2_EAP_TKIP;
        else
            encr_type = WPA2_EAP_TKIP_CCMP;
    }
    else if ( strcmp( encryption, "WPA Mixed-Enterprise" ) == 0 )
    {
        security_mode = 2;
        if ( strcmp( auth_type, "AES" ) == 0 )
            encr_type = WPA_EAP_MIXED_CCMP;
        else if ( strcmp( auth_type, "TKIP" ) == 0 )
            encr_type = WPA_EAP_MIXED_TKIP;
        else
            encr_type = WPA_EAP_MIXED_TKIP_CCMP;
    }
    else
    {
        security_mode = 0;
        encr_type = ENCRYPTION_NONE;
    }

    if(api_set_wifi_ifacex_encryption_option_by_sectionname(opmode, section, idx, encr_type))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ENCRYPTION");
    }

    if(strstr(encryption, "WPA"))
    {
        senao_json_object_get_and_create_string(rep, jobj, "wpa", &jsonStr);

        if(api_set_wifi_ifacex_wpa_group_rekey_option_by_sectionname(opmode, section, idx, key_interval))
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "KEY INTERVAL");
        }
    }
    else
    {
        ;//RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "AUTH TYPE");
    }

    api_set_wifi_ifacex_nasid_enabled_option_by_sectionname(opmode, section, idx, nasId_enable);

    if (nasId_enable == true) 
    {
        if (api_set_wifi_ifacex_nasid_option_by_sectionname(opmode, section, idx, nasid, sizeof(nasid)))
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NAS ID");
        }
    }

    api_set_wifi_ifacex_nasip_enabled_option_by_sectionname(opmode, section, idx, nasIp_enable);

    if (nasIp_enable == true) 
    {
        if(api_set_wifi_ifacex_nasip_option_by_sectionname(opmode, section, idx, nasIp, sizeof(nasIp)))
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NAS IP");
        }
    }

    api_set_wifi_ifacex_nasport_enabled_option_by_sectionname(opmode, section, idx, nasPort_enable);

    if (nasPort_enable == true) 
    {
        if(api_set_wifi_ifacex_nasport_option_by_sectionname(opmode, section, idx, nasPort))
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NAS PORT");
        }
    }

    if (security_mode == 0) 
    {
        return json_set_wireless_accounting_server(rep, accounting_jsonStr, ssidCfg_p);
    }
    else if (security_mode == 1) 
    {
        if(api_set_wifi_ifacex_fastroaming_enable_option_by_sectionname(opmode, section, idx, fastroaming_enable))
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "FAST ROAMING");
        }

        return json_set_wireless_encryption(rep, jsonStr, ssidCfg_p)
            || json_set_wireless_accounting_server(rep, accounting_jsonStr, ssidCfg_p);
    }
    else if (security_mode == 2) 
    {
        if(api_set_wifi_ifacex_fastroaming_enable_option_by_sectionname(opmode, section, idx, fastroaming_enable))
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "FAST ROAMING");
        }

        return json_set_wireless_radius_server(rep, radius_jsonStr, ssidCfg_p)
            || json_set_wireless_accounting_server(rep, accounting_jsonStr, ssidCfg_p);
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_wireless_encryption(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){
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
            senao_json_object_get_and_create_string(rep, jobj, "passphrase", &passphrase);
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

    //GET_SECTION_NAME(radio_idx, opmode, section, res);


    if(api_set_wifi_ifacex_wpakey_key_option_by_sectionname(opmode, section, idx, passphrase, sizeof(passphrase)))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "PASSPHRASE");
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_wireless_radius_server(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){

    int opmode = ssidCfg_p->opmode, radio_idx = 0, retries = 0, server1Port = 0, server2Port = 0;
    char *server1Ip=NULL, *server1Secret=NULL, *server2Ip=NULL, *server2Secret=NULL;
    char *section = ssidCfg_p->section;
    struct json_object *jobj;
    ResponseStatus *res = rep->res;
    int idx = ssidCfg_p->idx;
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
//            senao_json_object_get_integer(jobj, "retries",&(retries));
            senao_json_object_get_and_create_string(rep, jobj, "server1Ip", &server1Ip);
            senao_json_object_get_integer(jobj, "server1Port",&(server1Port));
            senao_json_object_get_and_create_string(rep, jobj, "server1Secret", &server1Secret);
//            senao_json_object_get_and_create_string(rep, jobj, "server2Ip", &server2Ip);
//            senao_json_object_get_integer(jobj, "server2Port",&(server2Port));
//            senao_json_object_get_and_create_string(rep, jobj, "server2Secret", &server2Secret);
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

    //GET_SECTION_NAME(radio_idx, opmode, section, res);

//    if(api_set_wifi_ifacex_retries_option_by_sectionname(opmode, section, idx, retries))
//    {
//        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "RETRIES");
//    }


    if(api_set_wifi_ifacex_auth_server_option_by_sectionname(opmode, section, idx, server1Ip, sizeof(server1Ip)))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 IP");
    }

    if(api_set_wifi_ifacex_auth_port_option_by_sectionname(opmode, section, idx, server1Port))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 PORT");
    }

    if(api_set_wifi_ifacex_auth_secret_option_by_sectionname(opmode, section, idx, server1Secret, sizeof(server1Secret)))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 SECRET");
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_wireless_accounting_server(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){
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
            senao_json_object_get_boolean(jobj, "enable",&(acct_enable));
            senao_json_object_get_integer(jobj, "interval",&(acct_interval));
            senao_json_object_get_and_create_string(rep, jobj, "server1Ip", &server1Ip);
            senao_json_object_get_integer(jobj, "server1Port",&(server1Port));
            senao_json_object_get_and_create_string(rep, jobj, "server1Secret", &server1Secret);
//            senao_json_object_get_and_create_string(rep, jobj, "server2Ip", &server2Ip);
//            senao_json_object_get_integer(jobj, "server2Port",&(server2Port));
//            senao_json_object_get_and_create_string(rep, jobj, "server2Secret", &server2Secret);
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


    //GET_SECTION_NAME(radio_idx, opmode, section, res);

    if(api_set_wifi_ifacex_acct_enabled_option_by_sectionname(opmode, section, idx, acct_enable))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ACCOUNTING SERVER ENABLE");
    }

    if ( !acct_enable )
    {
        RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
    }

    if(api_set_wifi_ifacex_acct_server_option_by_sectionname(opmode, section, idx, server1Ip, sizeof(server1Ip)))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 IP");
    }

    if(api_set_wifi_ifacex_acct_port_option_by_sectionname(opmode, section, idx, server1Port))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 PORT");
    }

    if(api_set_wifi_ifacex_acct_secret_option_by_sectionname(opmode, section, idx, server1Secret, sizeof(server1Secret)))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SERVER 1 SECRET");
    }

    if(api_set_wifi_ifacex_acct_interval_option_by_sectionname(opmode, section, idx, acct_interval))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "ACCT INTERVAL");
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_wireless_guest_network(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){

    struct json_object *jobj, *jobj_nat = NULL;
    char *section = ssidCfg_p->section;
    int opmode = ssidCfg_p->opmode;
    bool enable = 0, nat_enable=0;
    int idx = ssidCfg_p->idx, time;
    char guest_network_en[16] = {0};
    char *ip=NULL, *mask=NULL, *start_ip=NULL, *end_ip=NULL, *lease_time=NULL, *nat_jsonStr=NULL, *cd_jsonStr=NULL;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_boolean(jobj, "enable", &enable);
        }
    }

    strcpy(guest_network_en, enable?"Enable":"Disable");

#if SUPPORT_SWOS_FUNCTION
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

    if(enable)
    {
        if (json_object_object_get(jobj, "manual_nat") != NULL)
        {
            senao_json_object_get_and_create_string(rep, jobj, "manual_nat", &nat_jsonStr);
            if((jobj_nat = jsonTokenerParseFromStack(rep, nat_jsonStr)))
            {
                strcpy(guest_network_en, "NAT_only");

                senao_json_object_get_and_create_string(rep, jobj_nat, "ip", &ip);
                if(api_set_network_nat_ip_address_option_by_sectionname(section, idx, ip, sizeof(ip)) != API_RC_SUCCESS)
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "IP");
                }

                senao_json_object_get_and_create_string(rep, jobj_nat, "subnet_mask", &mask);
                if(api_set_network_nat_subnet_mask_option_by_sectionname(section, idx, mask, sizeof(mask)) != API_RC_SUCCESS)
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "SUBNET_MASK");
                }

                senao_json_object_get_and_create_string(rep, jobj_nat, "start_ip", &start_ip);
                if(api_set_dhcp_nat_start_ip_option_by_sectionname(section, idx, start_ip, sizeof(start_ip)) != API_RC_SUCCESS)
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "START_IP");
                }

                senao_json_object_get_and_create_string(rep, jobj_nat, "end_ip", &end_ip);
                if(api_set_dhcp_nat_end_ip_option_by_sectionname(section, idx, end_ip, sizeof(end_ip)) != API_RC_SUCCESS)
                {
                    RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "END_IP");
                }

                senao_json_object_get_and_create_string(rep, jobj_nat, "client_dns_server", &cd_jsonStr);
                json_set_client_dns_server(rep, cd_jsonStr, ssidCfg_p, "NAT");

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
        }
    }
#endif

    if(api_set_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, guest_network_en, sizeof(guest_network_en)))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"GUEST_NETWORK");
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_wireless_captive_portal(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char *option){

    struct json_object *jobj;
    int auth_type = 0, session_timeout = 0, idle_timeout = 0;
    int opmode = ssidCfg_p->opmode, idx = ssidCfg_p->idx;
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
            senao_json_object_get_boolean(jobj, "enable", &enable);
            senao_json_object_get_and_create_string(rep, jobj, "auth_type", &auth_type_str);
#if SUPPORT_SWOS_FUNCTION
            senao_json_object_get_and_create_string(rep, jobj, "splash_url", &splash_url);
            senao_json_object_get_and_create_string(rep, jobj, "shared_secret", &shared_secret);
#else
            senao_json_object_get_and_create_string(rep, jobj, "external_splash_url", &ext_url);
#endif
            senao_json_object_get_integer(jobj, "session_timeout", &session_timeout);
            senao_json_object_get_integer(jobj, "idle_timeout", &idle_timeout);
            senao_json_object_get_and_create_string(rep, jobj, "walled_garden", &walled_garden);
        }
    }
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
    if (enable == 0)
    {
        portal_en = 0;
        if (strcmp(client_ip_assignment, "NAT") == 0){
            strcpy(guest_network, "NAT_only");
        }
        else if (strcmp(client_ip_assignment, "Bridge") == 0){
            strcpy(guest_network, "Disable");
        }
        if(api_set_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, guest_network, sizeof(guest_network)))
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"GUEST_NETWORK");
        }
    }
    else
    {
        strcpy(guest_network, client_ip_assignment);
        if (strcmp(guest_network, "NAT") == 0 || strcmp(guest_network, "Bridge") == 0)
        {
            portal_en = 1;
            api_set_string_option2(guest_network, sizeof(guest_network), "portal.ssid_%d.guest_network", idx);

            if(api_set_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, guest_network, sizeof(guest_network)))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"GUEST_NETWORK");
            }
        }
        else if (strcmp(guest_network, "Disable") == 0)//bridge
        {
            portal_en = 0;
            api_set_string_option2(guest_network, sizeof(guest_network), "portal.ssid_%d.guest_network", idx);

            strcpy(guest_network, "Enable");

            if(api_set_wifi_ifacex_guest_network_option_by_sectionname(opmode, section, idx, guest_network, sizeof(guest_network)))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"GUEST_NETWORK");
            }
        }
    }

    debug_print("Jason DEBUG %s[%d] [%s]\n", __FUNCTION__, __LINE__, guest_network);

    // guest_network
    if (enable == 0 || portal_en == 0)
    {
        RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
    }

    // auth_type
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
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"GUEST_NETWORK");
    }

    // external_splash_url
#if SUPPORT_SWOS_FUNCTION
    if (api_set_portal_intSplashUrl_option_by_sectionname(guest_network, idx, splash_url, sizeof(splash_url)))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"SPLASH_URL");
    }
    if (api_set_portal_sharedsecret_option_by_sectionname(guest_network, idx, shared_secret, sizeof(shared_secret)))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"SHARED_SECRET");
    }
#else
    if (api_set_portal_extSplashUrl_option_by_sectionname(guest_network, idx, ext_url, sizeof(ext_url)))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"EXT_URL");
    }
#endif

    // session_timeout
    if(api_set_portal_sessionTimeout_option_by_sectionname(guest_network, idx, session_timeout))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"SESSION_TIMEOUT");
    }
    if(api_set_portal_sessionTimeout_enable_option_by_sectionname(guest_network, idx, (session_timeout == 0)?0:1))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"SESSION_TIMEOUT_ENABLE");
    }
    // idle_timeout
    if(api_set_portal_idleTimeout_option_by_sectionname(guest_network, idx, idle_timeout))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"IDLE_TIMEOUT");
    }
    if(api_set_portal_idleTimeout_enable_option_by_sectionname(guest_network, idx, (idle_timeout == 0)?0:1))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"IDLE_TIMEOUT_ENABLE");
    }
    // walled_garden
    if(api_set_portal_walledGardenEnable_option_by_sectionname(guest_network, idx, (strcmp(walled_garden, "") == 0)?0:1))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"WALLED_GARDEN_ENABLE");
    }

    if (api_set_portal_walledGarden_option_by_sectionname(guest_network, idx, walled_garden, sizeof(walled_garden)))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS,"WALLED_GARDEN");
    }
#endif
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_wireless_scheduling_sync(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p)
{
    int ret = 0;
    int idx = ssidCfg_p->idx;
    ResponseStatus *res = rep->res;

    if ( ssidCfg_p->opmode != OPM_AP && ssidCfg_p->opmode != OPM_WDSAP && ssidCfg_p->opmode != -1)
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "THIS OPMODE NOT SUPPORT WIFI SCHEDULING");

    // always set to wifi0 now
    if ( RADIO_MODE % 5 == 2 )
        ret = ret || json_set_wireless_scheduling(rep, query_str, ssidCfg_p, WIFI_RADIO_NAME_24G);

    if ( RADIO_MODE / 5  >= 1 )
        ret = ret || json_set_wireless_scheduling(rep, query_str, ssidCfg_p, WIFI_RADIO_NAME_5G);

    if ( RADIO_MODE / 5 >= 2 )
        ret = ret || json_set_wireless_scheduling(rep, query_str, ssidCfg_p, WIFI_RADIO_NAME_5G_2);

    return ret;
}

int json_set_wireless_scheduling(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char* radio)
{
    struct json_object *jobj;
    int opmode = ssidCfg_p->opmode;
    char *days=NULL;
    bool enable = 0;
    int radio_idx = 0;
    char iface[6] = {0};
    int idx = ssidCfg_p->idx;

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
            senao_json_object_get_boolean(jobj, "enable", &enable);
            senao_json_object_get_and_create_string(rep, jobj, "days", &days);

            api_set_wifi_schedule_enable_option_by_sectionname(enable);

            if ( enable )
            {
                api_set_wifi_schedule_radio_option(WIFI_SCHEDULE_WIRELESS_RADIO_OPTION,radio_idx);
                api_set_wifi_schedule_ifname_option_by_sectionname(WIFI_SCHEDULE_WIRELESS_IFACE_OPTION,iface,sizeof(iface));
                api_set_wifi_schedule_templates_option_by_sectionname(WIFI_SCHEDULE_WIRELESS_TEMPLATES_OPTION, CUSTOMSCHEDULE);
                api_setup_wifi_schedule_templates(CUSTOMSCHEDULE, iface);
                return json_set_wireless_scheduling_days(rep, days, ssidCfg_p, radio);
            }
        }
    }
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    return 0;
}

int json_set_wireless_scheduling_days(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char *radio)
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
                senao_json_object_get_and_create_string(rep, jobj, scheduling_days[i], &days);
                json_set_wireless_scheduling_day(rep,  days, ssidCfg_p, radio, i);
            }
        }
    }
    return API_RC_SUCCESS;
}

int json_set_wireless_scheduling_day(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p, char *radio, int dayIdx)
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

            senao_json_object_get_boolean( jobj, "available", &enable);
            senao_json_object_get_and_create_string(rep, jobj, "start", &start);
            senao_json_object_get_and_create_string(rep, jobj, "end", &end);

            api_set_wifi_schedule_tablex_status_option(schedule_idx, enable);
            api_set_wifi_schedule_tablex_start_time_option(schedule_idx, start, strlen(start));
            api_set_wifi_schedule_tablex_end_time_option(schedule_idx, end, strlen(end));
        }
    }
debug_print("Jason DEBUG %s[%d]\n", __FUNCTION__, __LINE__);
    return ret;
}

int json_set_wireless_l2_acl(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){
    int opmode = ssidCfg_p->opmode, radio_idx = 0, mac_filter_num = 0;
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
            senao_json_object_get_boolean(jobj, "enable",&(acl_enable));
            senao_json_object_get_and_create_string(rep, jobj, "client_mac_list", &maclist);
            senao_json_object_get_and_create_string(rep, jobj, "policy", &macfilter);
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


    //GET_SECTION_NAME(radio_idx, opmode, section, res);
/*
    if(api_set_integer_option2(acl_enable, "%s_%d.%s", section, idx, JSON_WIRELESS_WIFI_ACL_ENABLE))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "L2 ACL ENABLE");
    }
    if(api_set_string_option2(maclist, sizeof(maclist), "%s_%d.%s", section, idx, JSON_WIRELESS_WIFI_ACL_MACLIST))
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT MAC LIST");
    }
*/
    if (acl_enable == true) 
    {

        if ( maclist && strlen(maclist) > 0 )
        {
            api_get_integer_option("functionlist.vendorlist.MAX_WLAN_MAC_FILTER_NUMBER", &mac_filter_num);
            if ( mac_filter_num == 0 )
                mac_filter_num = 32; // default is 32

            if ( strlen(maclist) > (mac_filter_num*18-1) )
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "client_mac_list too long");
        }
        if (strcmp(macfilter, DESC_ACL_ALLOW) == 0)
        {
            //strcpy(policy, "allow");

            if(api_set_wifi_ifacex_macfilter_option_by_sectionname(opmode, section, idx, 1))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "POLICY");
            }
            if ( strstr(section,"enjet") )
            {
                api_set_string_option2(maclist, sizeof(maclist), "%s.allowmaclist", section);
            }
            else
            {
                api_set_string_option2(maclist, sizeof(maclist), "%s_%d.allowmaclist", section, idx);
            }
            /*if(api_add_wifi_ifacex_allowmaclist_option_by_sectionname(opmode, section, idx, maclist, sizeof maclist))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "CLIENT MAC LIST");
            }*/
        }
        else if(strcmp(macfilter, DESC_ACL_DENY) == 0)
        {
            //strcpy(policy, "deny");

            if(api_set_wifi_ifacex_macfilter_option_by_sectionname(opmode, section, idx, 2))
            {
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "POLICY");

            }
            if ( strstr(section,"enjet") )
            {
                api_set_string_option2(maclist, sizeof(maclist), "%s.denymaclist", section);
            }
            else
            {
                api_set_string_option2(maclist, sizeof(maclist), "%s_%d.denymaclist", section, idx);
            }
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "POLICY");
        }
    }
    else
    {
        if(api_set_wifi_ifacex_macfilter_option_by_sectionname(opmode, section, idx, 0))
        {
            RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "POLICY");
        }
    }
       
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
int json_set_wireless_kick(ResponseEntry *rep, char *query_str, ssid_cfg_st *ssidCfg_p){
    int opmode = ssidCfg_p->opmode, radio_idx = 0, macfilter_index = 0, addspace = 0, same_mac = 0;
    bool acl_enable = 0;
    char *section = ssidCfg_p->section, *pch, maclist_kick[ACL_MACLIST_LENGTH] = {0};
    char *macfilter=NULL, *kick_mac=NULL, policy[ACL_MACFILTER_LENGTH] = {0}, iface[10] = {0}, buf[256] = {0}, maclist[ACL_MACLIST_LENGTH] = {0};
    char *saveptr=NULL;
    struct json_object *jobj;
    ResponseStatus *res = rep->res;
    int idx = ssidCfg_p->idx, length = 0;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "kick_mac", &kick_mac);
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

    if ( strstr(section, "wifi0") ) 
    {
        radio_idx = 0;
    }
    else if( strstr(section, "wifi1") ) 
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
    api_get_wifi_ifacex_macfilter_option_by_sectionname(opmode, section,idx, &macfilter_index);


    if(macfilter_index == 1)
    {

        sys_interact(buf, sizeof(buf), "iwpriv %s maccmd 1", iface);
        sys_interact(buf, sizeof(buf), "iwpriv %s delmac %s",iface, kick_mac);
        sys_interact(buf, sizeof(buf), "iwpriv %s kickmac %s",iface, kick_mac);

        //policy = DESC_ACL_ALLOW;
        //acl_enable = 1;
        if ( strstr(section,"enjet") )
        {
            api_get_string_option2(maclist, sizeof(maclist), "%s.allowmaclist", section);
        }
        else
        {
            api_get_string_option2(maclist, sizeof(maclist), "%s_%d.allowmaclist", section, idx);
        }

        api_set_wifi_ifacex_macfilter_option_by_sectionname(opmode, section, idx, 1);

        pch = strtok_r(maclist, " ", &saveptr);

        while(pch !=NULL)
        {
            addspace = 0;
            if (strcmp(pch, kick_mac) != 0) 
            {
                strcat(maclist_kick, pch);
                addspace = 1;
            }
            pch = strtok_r(NULL, " ", &saveptr);
            if (pch != NULL && addspace == 1) 
            {
                strcat(maclist_kick, " ");
            }
            else if (addspace == 0 && pch == NULL)
            {
                maclist_kick[strlen(maclist_kick)-1] = 0;
            }
        }
        if ( strstr(section,"enjet") )
        {
            api_set_string_option2(maclist_kick, sizeof(maclist_kick), "%s.allowmaclist", section);
        }
        else
        {
            api_set_string_option2(maclist_kick, sizeof(maclist_kick), "%s_%d.allowmaclist", section, idx);
        }
    }
    else
    {

        sys_interact(buf, sizeof(buf), "iwpriv %s maccmd 2", iface);
        sys_interact(buf, sizeof(buf), "iwpriv %s addmac %s",iface, kick_mac);
        sys_interact(buf, sizeof(buf), "iwpriv %s kickmac %s",iface, kick_mac);

        if ( strstr(section,"enjet") )
        {
            api_get_string_option2(maclist, sizeof(maclist), "%s.denymaclist", section);
        }
        else
        {
            api_get_string_option2(maclist, sizeof(maclist), "%s_%d.denymaclist", section, idx);
        }

        api_set_wifi_ifacex_macfilter_option_by_sectionname(opmode, section, idx, 2);
        strcpy(maclist_kick, maclist);

        pch = strtok_r(maclist, " ", &saveptr);

        while(pch !=NULL)
        {
            if (strcmp(pch, kick_mac) == 0) 
            {
                same_mac = 1;
            }
            pch = strtok_r(NULL, " ", &saveptr);
        }

        if (same_mac == 0) {
            if (strcmp(maclist_kick, "") != 0)
            {
                strcat(maclist_kick, " ");
            }
            strcat(maclist_kick, kick_mac);
        }
        
        if ( strstr(section,"enjet") )
        {
            api_set_string_option2(maclist_kick, sizeof(maclist_kick), "%s.denymaclist", section);
        }
        else
        {
            api_set_string_option2(maclist_kick, sizeof(maclist_kick), "%s_%d.denymaclist", section, idx);
        }
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

#if SUPPORT_WATCHGUARD_FUNCTION
unsigned long long convert_to_bytes(char *str)
{
    unsigned long long result = 0;

    if ( sscanf(str, "%llu", &result) != 1 )
        return 0;

    if ( strstr(str,"K") || strstr(str,"k") )
        result = result * 1024;
    else if ( strstr(str,"M") || strstr(str,"m") )
        result = result * 1024 * 1024;
    else if ( strstr(str,"G") || strstr(str,"g") )
        result = result * 1024 *1024 *1024;

    return result;
}
#else
int convert_to_bytes(char *str)
{
    int result = 0;

    if ( sscanf(str, "%d", &result) != 1 )
        return 0;

    if ( strstr(str,"K") || strstr(str,"k") )
        result = result * 1024;
    else if ( strstr(str,"M") || strstr(str,"m") )
        result = result * 1024 * 1024;
    else if ( strstr(str,"G") || strstr(str,"g") )
        result = result * 1024 *1024 *1024;

    return result;
}
#endif

int map_hwmode(char *str)
{
	if(strcmp(str, "IEEE80211_MODE_11AXA_HE80_80")==0
	|| strcmp(str, "IEEE80211_MODE_11AXA_HE160")==0
	|| strcmp(str, "IEEE80211_MODE_11AXA_HE80")==0
	|| strcmp(str, "IEEE80211_MODE_11AXG_HE40")==0
	|| strcmp(str, "IEEE80211_MODE_11AXA_HE40")==0
	|| strcmp(str, "IEEE80211_MODE_11AXG_HE40MINUS")==0
	|| strcmp(str, "IEEE80211_MODE_11AXG_HE40PLUS")==0
	|| strcmp(str, "IEEE80211_MODE_11AXA_HE40MINUS")==0
	|| strcmp(str, "IEEE80211_MODE_11AXA_HE40PLUS")==0
	|| strcmp(str, "IEEE80211_MODE_11AXG_HE20")==0
	|| strcmp(str, "IEEE80211_MODE_11AXA_HE20")==0
	) {
		sprintf(str, "802.11ax");
	}
	else if(strcmp(str, "IEEE80211_MODE_11AC_VHT80_80")==0
	|| strcmp(str, "IEEE80211_MODE_11AC_VHT160")==0
	|| strcmp(str, "IEEE80211_MODE_11AC_VHT80")==0
	|| strcmp(str, "IEEE80211_MODE_11AC_VHT40")==0
	|| strcmp(str, "IEEE80211_MODE_11AC_VHT40MINUS")==0
	|| strcmp(str, "IEEE80211_MODE_11AC_VHT40PLUS")==0
	|| strcmp(str, "IEEE80211_MODE_11AC_VHT20")==0
	) {
		sprintf(str, "802.11ac");
	}
	else if(strcmp(str, "IEEE80211_MODE_11NA_HT40")==0
	|| strcmp(str, "IEEE80211_MODE_11NA_HT40MINUS")==0
	|| strcmp(str, "IEEE80211_MODE_11NA_HT40PLUS")==0
	|| strcmp(str, "IEEE80211_MODE_11NA_HT20")==0
	) {
		sprintf(str, "802.11na");
	}
	else if(strcmp(str, "IEEE80211_MODE_11NG_HT40")==0
	|| strcmp(str, "IEEE80211_MODE_11NG_HT40MINUS")==0
	|| strcmp(str, "IEEE80211_MODE_11NG_HT40PLUS")==0
	|| strcmp(str, "IEEE80211_MODE_11NG_HT20")==0
	) {
		sprintf(str, "802.11ng");
	}
	else if(strcmp(str, "IEEE80211_MODE_11A") == 0
	|| strcmp(str, "IEEE80211_MODE_TURBO_STATIC_A") == 0
	|| strcmp(str, "IEEE80211_MODE_TURBO_A") == 0
	) {
		sprintf(str, "802.11a");
	}
	else if(strcmp(str, "IEEE80211_MODE_TURBO_G") == 0
	|| strcmp(str, "IEEE80211_MODE_11G") == 0
	) {
		sprintf(str, "802.11g");
	}
	else if(strcmp(str, "IEEE80211_MODE_11B") == 0 ) {
		sprintf(str, "802.11b");
	}
	else {
		sprintf(str, "802.11n");
	}
	return 0;
}

struct json_object* get_clients_info_by_radio( int radio )
{
    struct json_object *jobj = NULL, *jobj_client = NULL;
    char ifname[6] = {0};
    char fprint_mac[20], fprint_ip[20] = {0}, fprint_os[33] = {0}, fprint_device_name[32] = {0};
    char wlan_mac[20] = {0}, wlan_assoc_time[16] = {0}, bssid[20] = {0};
    int client_num = 0, client_idx = 0, clientinfo_idx = 0, vap_idx = 0;
    int client_exist = 0, vap_exist = 0;
    int ssid_id = -1, ssid = 0;
    char pstr[256] = {0}, wlanconfig_str[256] = {0};
#if SUPPORT_WATCHGUARD_FUNCTION
    int wlan_chan = 0, wlan_rssi = 0, wlan_tx_rate = 0, wlan_rx_rate = 0;
    unsigned long long wlan_tx_byte = 0, wlan_rx_byte = 0;
    char cmd[128]={0}, aid[64]={0}, identity[128]={0};
#else
    int wlan_chan = 0, wlan_rssi = 0, wlan_tx_byte = 0, wlan_rx_byte = 0, wlan_tx_rate = 0, wlan_rx_rate = 0;
#endif
    int idx = 0, ct = 0;
#if 1
    char ssid_name[128]={0}, capability[32]={0}, ssid_encr[32]={0};
#endif


    jobj = json_object_new_array();

    char buf[256] = {0};

    if ( radio == 2 )
        radio = 4; // 5G-2 are ath4~ath47

    for ( idx = 0 ; idx < 8 ; idx++ )
    {
        if ( idx == 0 )
            snprintf(ifname, sizeof(ifname), "ath%d", radio);
        else
            snprintf(ifname, sizeof(ifname), "ath%d%d", radio, idx);

        sys_interact(pstr, sizeof(pstr), "wlanconfig %s list sta | tail -n +2 | wc -l", ifname);

        sys_interact(bssid, sizeof(bssid), "iwconfig %s |grep 'Access Point' |awk -F ' ' '{print $6}'", ifname);
        bssid[strcspn(bssid,"\n")] = '\0';

        if ( sscanf( pstr, "%d", &client_num ) != 1 || client_num ==0 )
            continue;

        for ( client_idx = 0 ; client_idx < client_num ; client_idx++ )
        {
            jobj_client = json_object_new_object();

            memset(wlan_mac, 0, sizeof(wlan_mac));
            memset(wlan_assoc_time, 0, sizeof(wlan_assoc_time));

            sys_interact(wlanconfig_str, sizeof(wlanconfig_str), "wlanconfig %s list sta | tail -n +2 | sed -n \"%d,%dp\"", ifname, client_idx+1, client_idx+1);

            if ( strlen(wlanconfig_str) > 0 )
                wlanconfig_str[strcspn(wlanconfig_str,"\n")] = '\0';
            else
                continue;

#if 1
            sys_interact(ssid_name, sizeof(ssid_name), "uci get wireless.wifi%d_ssid_%d.ssid | tr -d '\n'", radio, idx+1);
            sys_interact(ssid_encr, sizeof(ssid_encr), "uci get wireless.wifi%d_ssid_%d.encryption | tr -d '\n'", radio, idx+1);

            if(strcmp(ssid_encr, "none") == 0) //If SSID enable security, the output format of "wlanconfig athx list sta" will be changed
            {
                sys_interact(capability, sizeof(capability), "echo '%s' | awk {'print $20'} | tr -d '\n'", wlanconfig_str);
            }
            else
            {
                sys_interact(capability, sizeof(capability), "echo '%s' | awk {'print $21'} | tr -d '\n'", wlanconfig_str);
            }

            if(strlen(capability)>0)
            {
                map_hwmode(capability);
            }
#endif

            // get mac
            sys_interact(pstr, sizeof(pstr), "echo %s | awk -F \" \" \'{print $1}\'", wlanconfig_str);

            if ( strlen(pstr) > 0 )
            {
                pstr[strcspn(pstr,"\n")] = '\0';
                snprintf(wlan_mac,sizeof(wlan_mac)-1,"%s",pstr);
            }
            else
                continue;

#if SUPPORT_WATCHGUARD_FUNCTION
            //get aid
            sprintf(cmd, "wlanconfig %s find_snassocid %s", ifname, wlan_mac);
            system(cmd);
            sys_interact(aid, sizeof(aid), "iwpriv %s get_snassocid |awk -F':' '{print $2}'", ifname);
            aid[strcspn(aid,"\n")] = '\0';

            //get identity of radius server
            sys_interact(identity, sizeof(identity), "hostapd_cli -i %s -p /var/run/hostapd-phy0/ sta %s |grep dot1xAuthSessionUserName |awk -F '=' '{print $2}'", ifname, wlan_mac);
            identity[strcspn(identity,"\n")] = '\0';
#endif

            // get chan
            sys_interact(pstr, sizeof(pstr), "echo %s | awk -F \" \" \'{print $3}\'", wlanconfig_str);

            if ( strlen(pstr) > 0 )
            {
                pstr[strcspn(pstr,"\n")] = '\0';
                sscanf(pstr,"%d",&wlan_chan);
            }
            else
                continue;

            // get tx rate
            sys_interact(pstr, sizeof(pstr), "echo %s | awk -F \" \" \'{print $4}\'", wlanconfig_str);

            if ( strlen(pstr) > 0 )
            {
                pstr[strcspn(pstr,"\n")] = '\0';
                wlan_tx_rate = convert_to_bytes(pstr);
            }
            else
                continue;

            // get rx rate
            sys_interact(pstr, sizeof(pstr), "echo %s | awk -F \" \" \'{print $5}\'", wlanconfig_str);

            if ( strlen(pstr) > 0 )
            {
                pstr[strcspn(pstr,"\n")] = '\0';
                wlan_rx_rate = convert_to_bytes(pstr);
            }
            else
                continue;

            // get rssi
            sys_interact(pstr, sizeof(pstr), "echo %s | awk -F \" \" \'{print $6}\'", wlanconfig_str);

            if ( strlen(pstr) > 0 )
            {
                pstr[strcspn(pstr,"\n")] = '\0';
                sscanf(pstr,"%d",&wlan_rssi);
            }
            else
                continue;

            // get tx byte
            sys_interact(pstr, sizeof(pstr), "echo %s | awk -F \" \" \'{print $7}\'", wlanconfig_str);

            if ( strlen(pstr) > 0 )
            {
                pstr[strcspn(pstr,"\n")] = '\0';
                wlan_tx_byte = convert_to_bytes(pstr);
            }
            else
                continue;

            // get rx byte
            sys_interact(pstr, sizeof(pstr), "echo %s | awk -F \" \" \'{print $8}\'", wlanconfig_str);

            if ( strlen(pstr) > 0 )
            {
                pstr[strcspn(pstr,"\n")] = '\0';
                wlan_rx_byte = convert_to_bytes(pstr);
            }
            else
                continue;
            for (ct = 16; ct <20;ct++) // different driver
            {
                sys_interact(pstr, sizeof(pstr), "echo %s | awk -F \" \" \'{print $%d}\'", wlanconfig_str, ct);
                // get assoc time
                if ( strstr(pstr, ":"))
                {
                    if ( ( pstr != NULL ) && ( strcmp(pstr,"") != 0 ) )
                    {
                        pstr[strcspn(pstr,"\n")] = '\0';
                        snprintf(wlan_assoc_time,sizeof(wlan_assoc_time)-1,"%s",pstr);
                    }
                    else
                        continue;
                    ct = 20;
                }
            }

            memset(fprint_mac, 0, sizeof(fprint_mac));
            memset(fprint_ip, 0, sizeof(fprint_ip));
            memset(fprint_os, 0, sizeof(fprint_os));
            memset(fprint_device_name, 0, sizeof(fprint_device_name));

            sys_interact(pstr, sizeof(pstr), "cat /tmp/fingerprint_status_list_%s | grep %s", ifname, wlan_mac);
            sscanf(pstr, "%[^|]|%[^|]|%[^|]|%s", fprint_mac, fprint_ip, fprint_os, fprint_device_name);

            sys_interact(pstr, sizeof(pstr), "uci get wireless.wifi%d_ssid_%d.id", radio, idx+1);
            if( strlen(pstr) > 0)
                sscanf(pstr, "%d", &ssid_id);
            else
                ssid_id = -1;

            json_object_object_add(jobj_client, "device_name", json_object_new_string((fprint_device_name[0]==0)? "unknown" : fprint_device_name));
            json_object_object_add(jobj_client, "mac_addr", json_object_new_string((wlan_mac[0]==0)? "00:00:00:00:00:00" : wlan_mac));
            json_object_object_add(jobj_client, "ip_addr", json_object_new_string((fprint_ip[0]==0)? "0.0.0.0" : fprint_ip));
            json_object_object_add(jobj_client, "os_info", json_object_new_string((fprint_os[0]==0)? "unknown" : fprint_os));
            json_object_object_add(jobj_client, "channel", json_object_new_int(wlan_chan));
            json_object_object_add(jobj_client, "ssid_id", json_object_new_int(ssid_id));
#if 1
            json_object_object_add(jobj_client, "ssid_name", json_object_new_string(ssid_name));
#endif
            json_object_object_add(jobj_client, "bssid", json_object_new_string(bssid));
            json_object_object_add(jobj_client, "rssi", json_object_new_int(wlan_rssi));
            json_object_object_add(jobj_client, "tx_rate", json_object_new_int(wlan_tx_rate));
            json_object_object_add(jobj_client, "rx_rate", json_object_new_int(wlan_rx_rate));
#if SUPPORT_WATCHGUARD_FUNCTION
            json_object_object_add(jobj_client, "tx_byte", json_object_new_uint64(wlan_tx_byte));
            json_object_object_add(jobj_client, "rx_byte", json_object_new_uint64(wlan_rx_byte));
            json_object_object_add(jobj_client, "aid", json_object_new_string(aid));
            json_object_object_add(jobj_client, "identity", json_object_new_string(identity));
#else
            json_object_object_add(jobj_client, "tx_byte", json_object_new_int(wlan_tx_byte));
            json_object_object_add(jobj_client, "rx_byte", json_object_new_int(wlan_rx_byte));
#endif
            json_object_object_add(jobj_client, "connection_time", json_object_new_string((wlan_assoc_time[0]==0)? "00:00:00" : wlan_assoc_time));
#if 1
            json_object_object_add(jobj_client, "capability", json_object_new_string(capability));
#endif
            json_object_array_add(jobj,jobj_client);
        }
    }

    return jobj;
}

int json_get_client_info(ResponseEntry *rep, struct json_object *jobj)
{
    struct json_object *jobj_dev = NULL;
    int radio_start = 0, idx = 0;
    int radio_num = 0;
    ResponseStatus *res = rep->res;

#if SUPPORT_WLAN_5G_SETTING
    radio_start=1;
    radio_num++;
#if SUPPORT_WLAN_5G_2_SETTING
    radio_num++;
#endif
#endif

#if SUPPORT_WLAN_24G_SETTING
    radio_start=0;
    radio_num++;
#endif

    if(NULL == jobj)
    {
        return FALSE;
    }

    char buf[256] = {0};

    system("killall -SIGUSR1 dhcrelay_fp");
    sleep(5);
    for ( idx = radio_start ; idx < radio_start+radio_num ; idx++ )
    {
        jobj_dev = get_clients_info_by_radio(idx);

        switch (idx)
        {
            case 0:
                json_object_object_add(jobj, "2_4G", jobj_dev);
                break;
            case 1:
                json_object_object_add(jobj, "5G", jobj_dev);
                break;
            case 2:
                json_object_object_add(jobj, "5G-2", jobj_dev);
                break;
        }
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

#define SITE_SURVEY_24G_RESULT_FILE "/tmp/site_24g_survey_result"
#define SITE_SURVEY_5G_RESULT_FILE "/tmp/site_5g_survey_result"
#define SITE_SURVEY_5G_2_RESULT_FILE "/tmp/site_5g_2_survey_result"

const char* site_surery_files[] = {
#if SUPPORT_WLAN_24G_SETTING
    SITE_SURVEY_24G_RESULT_FILE,
#endif
#if SUPPORT_WLAN_5G_SETTING
    SITE_SURVEY_5G_RESULT_FILE,
#if SUPPORT_WLAN_5G_2_SETTING
    SITE_SURVEY_5G_2_RESULT_FILE
#endif
#endif
};
const char* radio_name[] = {
#if SUPPORT_WLAN_24G_SETTING
    "2_4G",
#endif
#if SUPPORT_WLAN_5G_SETTING
    "5G",
#if SUPPORT_WLAN_5G_2_SETTING
    "5G-2"
#endif
#endif
};
int json_wifi_site_survey(ResponseEntry *rep, char *query_str)
{
    char ifname[128]={0}, cmd[128]={'\0'};
    struct json_object *jobj;
    int support_fast_scan = 0;
    api_get_integer_option("functionlist.functionlist.SUPPORT_WLAN_FAST_SCAN", &support_fast_scan);
    char* scan_method = support_fast_scan?"displayscan":"scan";
    int i, len = 0;
    ResponseStatus *res = rep->res;

    for(i = 0; i < T_NUM_OF_ELEMENTS(site_surery_files); ++i)
    {
        memset(ifname, 0, sizeof(ifname));
        if ( strcmp(radio_name[i],"2_4G") == 0 )
        {
            sys_interact(ifname, sizeof(ifname), "wifi_mac=$(cat /sys/class/net/wifi0/address); grep ${wifi_mac#*:} /sys/class/net/*/address | grep -Ev 'wifi' | head -n 1 | awk -F'/' '{print $5}'");
        }
        else if ( strcmp(radio_name[i],"5G") == 0 )
        {
            sys_interact(ifname, sizeof(ifname), "wifi_mac=$(cat /sys/class/net/wifi1/address); grep ${wifi_mac#*:} /sys/class/net/*/address | grep -Ev 'wifi' | head -n 1 | awk -F'/' '{print $5}'");
        }
        else if ( strcmp(radio_name[i],"5G-2") == 0 )
        {
            sys_interact(ifname, sizeof(ifname), "wifi_mac=$(cat /sys/class/net/wifi2/address); grep ${wifi_mac#*:} /sys/class/net/*/address | grep -Ev 'wifi' | head -n 1 | awk -F'/' '{print $5}'");
        }

        len = strlen(ifname);
        if (len > 0 && ifname[len-1] == '\n') ifname[len-1] = '\0';

        debug_print("DEBUG %s:%d sitesurvey radio[%d] ifname[%s] \n", __FUNCTION__, __LINE__, i, ifname);

        if ( strlen(ifname) == 0 )
            continue;

        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "iwlist %s %s 2>&1 >> %s &", ifname, "scan", site_surery_files[i]);
        unlink(site_surery_files[i]);
        system(cmd);
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);

}

int json_get_wifi_site_survey(ResponseEntry *rep, struct json_object *jobj)
{
    char buf[1024]={0}, dev_mac_addr[18]={0}, ssid_name[64]={0}, encryption[16]={0};
    int rssi=0, ssid_len = 0;
    struct json_object* jobj_tmp;
    struct json_object* jarr;
    FILE *fp = NULL;
    int i;
    unsigned long status = 0;
    ResponseStatus *res = rep->res;

    sys_interact(buf, sizeof(buf), "pgrep -o iwlist 2>&1");
    status = atol(buf);
    memset(buf, 0, sizeof(buf));
    if(status > 0 ){
       RET_GEN_ERRORMSG(rep->res, API_PROCESSING, "PROCESSING"); 
    }

    for(i = 0; i < T_NUM_OF_ELEMENTS(site_surery_files); ++i){
        if(access(site_surery_files[i], F_OK) != -1){
            fp = fopen(site_surery_files[i], "r");
            if(fp){
                jarr = newObjectArrayFromStack(rep);
                while(fgets(buf, sizeof(buf), fp) != NULL)
                {
                    memset(dev_mac_addr, 0, sizeof(dev_mac_addr));
                    memset(ssid_name, 0, sizeof(ssid_name));
                    memset(encryption, 0, sizeof(encryption));
                    ssid_len = 0;
                    rssi = 0;

                    if(sscanf(buf,"%17s%*c%32[^\n]\t%d%*s%*s%d%s%*s", dev_mac_addr, ssid_name, &ssid_len, &rssi, encryption) == 5)
                    {
                        ssid_name[ssid_len] = 0;
                    }
                    else
                    {
                        continue;
                    }

                    // debug_print("%s:%d dev_mac_addr[%s] ssid_name[%s] ssid_len[%d] rssi[%d] encryption[%s] ###\n", __FUNCTION__, __LINE__, dev_mac_addr, ssid_name, ssid_len, rssi, encryption);

                    if ( ssid_len > 0 )
                    {
                        jobj_tmp = newObjectFromStack(rep);
                        json_object_object_add(jobj_tmp, "dev_mac_addr", json_object_new_string(dev_mac_addr));
                        json_object_object_add(jobj_tmp, "ssid_name", json_object_new_string(ssid_name));
                        json_object_object_add(jobj_tmp, "rssi", json_object_new_int(rssi));
                        json_object_object_add(jobj_tmp, "encryption", json_object_new_string(encryption));
                        json_object_array_add(jarr, jobj_tmp);
                    }
                }
                json_object_object_add(jobj, radio_name[i], jarr);
            }
        }else{
            RET_GEN_ERRORMSG(rep->res, API_INVALID_DATA_TYPE, "No site survey result");
        }
    }
    RET_GEN_ERRORMSG(rep->res, API_SUCCESS, NULL);
}

int json_get_hotspot20(ssid_cfg_st *ssidCfg_p, char *query_str, int str_size, int error_code, char* msg)
{
    struct json_object *jobj = NULL, *jobj_tmp = NULL, *jarr_operator_name = NULL, *jarr_domain = NULL, *jarr_realm = NULL;
    struct json_object *jarr_roaming_consortium = NULL, *jarr_anqp_3gpp_cell_net = NULL, *jarr_connection_capability = NULL;
    char hessid[128] = {0}, operator_name[1024] = {0}, domain[1024] = {0}, roaming_consortium[1024] = {0};
    char buf[1024] = {0}, filename[32] = {0}, anqp_3gpp_cell_net[128] = {0}, connection_capability[128] = {0}, realm[1024] = {0};
    char *section = ssidCfg_p->section, venue_string[1024] = {0}, osu_string[1024] = {0}, wan_metrics_string[1024] = {0}, network_string[1024] = {0};
    char *pstr = NULL, *pstr2 = NULL, *saveptr = NULL, *saveptr2 = NULL, *delim = ",", *delim2 = ";", *delim3 = ":", *delim4 = "+";
    int i = 0, enable = 0, internet_access = 0, operating_class = 0, result = 0, conn_capab = 0, anqp = 0;
    int opmode = ssidCfg_p->opmode, idx = ssidCfg_p->idx;

    jobj = json_object_new_object();

    api_get_wifi_ifacex_hs20_enable_option_by_sectionname(opmode, section, idx, &enable);
    api_get_wifi_ifacex_hs20_hessid_option_by_sectionname(opmode, section, idx, hessid, sizeof(hessid));
    api_get_wifi_ifacex_hs20_operator_name_option_by_sectionname(opmode, section, idx, operator_name, sizeof(operator_name));
    api_get_wifi_ifacex_hs20_domain_name_option_by_sectionname(opmode, section, idx, domain, sizeof (domain));
    api_get_wifi_ifacex_hs20_roaming_consortium_option_by_sectionname(opmode, section, idx, roaming_consortium, sizeof (roaming_consortium));
    json_get_hotspot20_venue(opmode, idx, section, venue_string, sizeof venue_string, &error_code, msg);
    json_get_hotspot20_network(opmode, idx, section, network_string, sizeof network_string, &error_code, msg);
    json_get_hotspot20_osu(opmode, idx, section, osu_string, sizeof osu_string, &error_code, msg);

    if(strlen(operator_name) > 0){
        jarr_operator_name = json_object_new_array();
        pstr = strtok_r(operator_name, delim, &saveptr);
        while ( pstr != NULL )
        {
            pstr2 = strtok_r(pstr, delim3, &saveptr2);
            jobj_tmp = json_object_new_object();
            i = 0;
            while( pstr2 != NULL )
            {
                if(i == 0)
                    json_object_object_add(jobj_tmp, "language", json_object_new_string(pstr2));
                else
                    json_object_object_add(jobj_tmp, "name", json_object_new_string(pstr2));
                pstr2 = strtok_r(NULL, delim3, &saveptr2);
                i++;
            }
            json_object_array_add(jarr_operator_name, jobj_tmp);
            pstr = strtok_r(NULL, delim, &saveptr);
        }
        json_object_object_add(jobj, "operator_friendly_name", jarr_operator_name);
    }

    else
        json_object_object_add(jobj, "operator_friendly_name", json_object_new_string(""));

    if(strlen(domain) > 0){
        pstr = strtok_r(domain, delim, &saveptr);
        jarr_domain = json_object_new_array();
        while ( pstr != NULL )
        {
            json_object_array_add(jarr_domain, json_object_new_string(pstr));
            pstr = strtok_r(NULL, delim, &saveptr);
        }
        json_object_object_add(jobj, "domain", jarr_domain);
    }
    else
       json_object_object_add(jobj, "domain", json_object_new_string(""));

    if(strlen(roaming_consortium) > 0){
        pstr = strtok_r(roaming_consortium, delim, &saveptr);
        jarr_roaming_consortium = json_object_new_array();
        while ( pstr != NULL )
        {
            json_object_array_add(jarr_roaming_consortium, json_object_new_string(pstr));
            pstr = strtok_r(NULL, delim, &saveptr);
        }
        json_object_object_add(jobj, "roaming_consortium", jarr_roaming_consortium);
    }
    else
        json_object_object_add(jobj, "roaming_consortium", json_object_new_string(""));

    json_object_object_add(jobj, "enable", json_object_new_int(enable));

    if(strlen(venue_string) > 0)
        json_object_object_add(jobj, "venue", json_tokener_parse(venue_string));
    else
        json_object_object_add(jobj, "venue", json_object_new_string(""));

    if(strlen(network_string) > 0)
        json_object_object_add(jobj, "network", json_tokener_parse(network_string));
    else
        json_object_object_add(jobj, "network", json_object_new_string(""));

    json_object_object_add(jobj, "hessid", json_object_new_string(hessid));

    if(json_tokener_parse(osu_string)!= NULL)
        json_object_object_add(jobj, "osu", json_tokener_parse(osu_string));

#if 0
//****************************************************FOR HOTSPOT2.0 PHASE 2******************************************************************************
    api_get_wifi_ifacex_hs20_internetworking_enable_option_by_sectionname(opmode, section, idx, &internet_access);
    api_get_wifi_ifacex_hs20_operating_class_option_by_sectionname(opmode, section, idx, &operating_class);
    api_get_wifi_ifacex_hs20_anqp_3gpp_cell_net_option_by_sectionname(opmode, section, idx, anqp_3gpp_cell_net, sizeof anqp_3gpp_cell_net);
    api_get_wifi_ifacex_hs20_connection_capability_option_by_sectionname(opmode, section, idx, connection_capability, sizeof connection_capability);
    api_get_wifi_ifacex_hs20_nai_realm_option_by_sectionname(opmode, section, idx, realm, sizeof realm);
    json_get_hotspot20_wan_metrics(opmode, idx, section, wan_metrics_string, sizeof wan_metrics_string, &error_code, msg)
    jarr_anqp_3gpp_cell_net = json_object_new_array();
    pstr = strtok_r(anqp_3gpp_cell_net, delim2, &saveptr);
    while ( pstr != NULL )
    {
        pstr2 = strtok_r(pstr, delim, &saveptr2);
        jobj_tmp = json_object_new_object();
        i = 0;
        while ( pstr2 != NULL)
        {
            anqp = atoi(pstr2);
            if(i == 0)
                json_object_object_add(jobj_tmp, "mcc", json_object_new_int(anqp));
            else
                json_object_object_add(jobj_tmp, "mnc", json_object_new_int(anqp));
            pstr2 = strtok_r(NULL, delim, &saveptr2);
            i++;
        }
        json_object_array_add(jarr_anqp_3gpp_cell_net, jobj_tmp);
        pstr = strtok_r(NULL, delim2, &saveptr);
    }

    jarr_connection_capability = json_object_new_array();
    pstr = strtok_r(connection_capability, delim, &saveptr);
    while( pstr != NULL )
    {
        pstr2 = strtok_r(pstr, delim3, &saveptr2);
        jobj_tmp = json_object_new_object();
        i = 0;
        while ( pstr2 != NULL )
        {
            conn_capab = atoi(pstr2);
            if(i == 0)
                json_object_object_add(jobj_tmp, "protocol", json_object_new_int(conn_capab));
            else if(i ==1)
                json_object_object_add(jobj_tmp, "port", json_object_new_int(conn_capab));
            else
                json_object_object_add(jobj_tmp, "status", json_object_new_int(conn_capab));
            pstr2 = strtok_r(NULL, delim3, &saveptr2);
            i++;
        }
        json_object_array_add(jarr_connection_capability, jobj_tmp);
        pstr = strtok_r(NULL, delim, &saveptr);
    }

    memset(filename, 0, sizeof(filename));
    snprintf(filename, sizeof(filename), "%s", TMP_REALM_JSON_FILE);
    jarr_realm = json_object_new_array();
    debug_print("%s:%d %s###\n", __FUNCTION__, __LINE__, filename);
    debug_print("%s:%d %s###\n", __FUNCTION__, __LINE__, realm);
    pstr = strtok_r(realm, delim4, &saveptr);
    while ( pstr != NULL )
    {
        debug_print("%s:%d %s###\n", __FUNCTION__, __LINE__, pstr);
        FILE* fp = fopen(filename, "w");
        if( NULL == fp )
        {
            debug_print("%s open failure", filename);
        }
        else
        {
            result = fputs(pstr, fp);
            if ( result == EOF )
                debug_print("write %s fail", filename);
            fclose(fp);
        }

        sys_interact(buf, sizeof(buf), "sh /sbin/convert_realm_to_json.sh %s" TMP_REALM_JSON_FILE);
        json_object_array_add(jarr_realm, json_tokener_parse(buf));
        pstr = strtok_r(NULL, delim4, &saveptr);
    }

    json_object_object_add(jobj, "anqp_3gpp_cell_net", jarr_anqp_3gpp_cell_net);
    json_object_object_add(jobj, "wan_metrics", json_tokener_parse(wan_metrics_string));
    json_object_object_add(jobj, "operating_class", json_object_new_int(operating_class));
    json_object_object_add(jobj, "realm", jarr_realm);
    json_object_object_add(jobj, "connection_capability", jarr_connection_capability);
    json_object_object_add(jobj, "internet_access", json_object_new_int(internet_access));
#endif

    snprintf(query_str, str_size, "%s", json_object_to_json_string(jobj));
    debug_print("Jason DEBUG %s:%d opmode: %d secion: %s idx: %d\n", __FUNCTION__, __LINE__, opmode, section, idx);
    json_object_put(jobj);
}

int json_set_hotspot20(ssid_cfg_st *ssidCfg_p, char *query_str, int error_code, char* msg)
{
    struct json_object *hotspot20 = NULL, *jobj = NULL, *jarr_operator_name = NULL, *jarr_domain = NULL;
    struct json_object *jarr_roaming_consortium = NULL, *jarr_anqp_3gpp_cell_net = NULL, *jarr_realm = NULL, *jarr_connection_capability = NULL;
    struct json_object *jarr_operator_name_info = NULL, *jarr_domain_info = NULL, *jarr_roaming_consortium_info = NULL, *jarr_anqp_3gpp_cell_net_info = NULL;
    struct json_object *jarr_connection_capability_info = NULL, *jarr_realm_info = NULL;
    int opmode = ssidCfg_p->opmode, result = 0, enable = 0, internet_access = 0, json_array_idx = 0, array_length = 0;
    int mcc = 0, mnc = 0, operating_class = 0, protocol = 0, port = 0, status = 0;
    char venue[1024] = {0}, osu[1024] = {0}, hessid[128] = {0}, domain[1024] = {0}, roaming_consortium[1024] = {0}, network[1024] = {0}, language[128] = {0};
    char anqp_3gpp_cell_net[1024] = {0}, realm[1024] = {0}, wan_metrics[1024] = {0}, connection_capability[1024] = {0}, name[128] = {0}, operator_name[1024] = {0};
    char buf[1024] = {0}, filename[32] = {0}, anqp_3gpp_cell_net_string[128] = {0}, mcc_mnc[10] = {0}, operator_name_string[1024] = {0}, language_string[128] = {0};
    char domain_string[1024] = {0}, roaming_consortium_string[1024] = {0}, conn_capab_string[1024] = {0}, realm_string[1024] = {0}, name_string[128] = {0};
    char *section = ssidCfg_p->section;
    int idx = ssidCfg_p->idx;

    debug_print("Jason DEBUG %s:%d opmode: %d secion: %s idx: %d\n", __FUNCTION__, __LINE__, opmode, section, idx);
    if((hotspot20= json_tokener_parse(query_str)) != NULL){
        if(!senao_json_object_get_integer(hotspot20, "enable", &enable))
            senao_json_object_get_integer(hotspot20, "is_enable", &enable);
        senao_json_object_get_string(hotspot20, "venue", venue);
        senao_json_object_get_string(hotspot20, "hessid", hessid);
        senao_json_object_get_string(hotspot20, "network", network);
        senao_json_object_get_string(hotspot20, "osu", osu);
        jarr_operator_name = json_object_object_get(hotspot20, "operator_friendly_name");
        jarr_domain = json_object_object_get(hotspot20, "domain");
        jarr_roaming_consortium = json_object_object_get(hotspot20, "roaming_consortium");

#if 0
//****************************************************FOR HOTSPOT2.0 PHASE 2******************************************************************************

        senao_json_object_get_integer(hotspot20, "internet_access", &internet_access);
        senao_json_object_get_integer(hotspot20, "operating_class", &operating_class);
        senao_json_object_get_string(hotspot20, "wan_metrics", wan_metrics);
        jarr_anqp_3gpp_cell_net = json_object_object_get(hotspot20, "anqp_3gpp_cell_net");
        jarr_connection_capability = json_object_object_get(hotspot20, "connection_capability");
        jarr_realm = json_object_object_get(hotspot20, "realm");

//*********************************************************************************************************************************************************
#endif
    }

    api_set_wifi_ifacex_hs20_enable_option_by_sectionname(opmode, section, idx, enable);
    api_set_wifi_ifacex_hs20_hessid_option_by_sectionname(opmode, section, idx, hessid, sizeof hessid);
    json_set_hotspot20_venue(opmode, idx, section, venue, &error_code, msg);
    json_set_hotspot20_network(opmode, idx, section, network, &error_code, msg);

    if(strlen(osu) > 0)
        json_set_hotspot20_osu(opmode, idx, section, osu, &error_code, msg);

    else
        api_delet_wifi_ifacex_hs20_osu_option_by_sectionname(opmode, section, idx);

    if(jarr_operator_name != NULL){
        array_length = json_object_array_length(jarr_operator_name);
        for (json_array_idx = 0; json_array_idx < array_length; json_array_idx++){
            jarr_operator_name_info = json_object_array_get_idx(jarr_operator_name, json_array_idx);
            senao_json_object_get_string(jarr_operator_name_info, "language", language);
            senao_json_object_get_string(jarr_operator_name_info, "name", name);
            snprintf(operator_name, sizeof(operator_name), "%s:%s", language, name);
            if(json_array_idx == 0)
                snprintf(operator_name_string, sizeof(operator_name_string), "%s", operator_name);
            else{
                strncat(operator_name_string, ",", sizeof(operator_name_string) - strlen(operator_name_string) - 1);
                strncat(operator_name_string, operator_name, sizeof(operator_name_string) - strlen(operator_name_string) - 1);
                //snprintf(operator_name_string, sizeof(operator_name_string), "%s,%s", operator_name_string, operator_name);
            }
        }
        api_set_wifi_ifacex_hs20_operator_name_option_by_sectionname(opmode, section, idx, operator_name_string, sizeof operator_name_string);
    }

    if(jarr_domain != NULL){
        array_length = json_object_array_length(jarr_domain);
        for (json_array_idx = 0; json_array_idx < array_length; json_array_idx++){
            jarr_domain_info = json_object_array_get_idx(jarr_domain, json_array_idx);
            snprintf(domain, sizeof(domain), "%s", json_object_get_string(jarr_domain_info));
            if(json_array_idx == 0)
                snprintf(domain_string, sizeof(domain_string), "%s", domain);
            else{
                strncat(domain_string, ",", sizeof(domain_string) - strlen(domain_string) - 1);
                strncat(domain_string, domain, sizeof(domain_string) - strlen(domain_string) - 1);
                //snprintf(domain_string, sizeof(domain_string), "%s,%s", domain_string, domain);
            }
        }
        api_set_wifi_ifacex_hs20_domain_name_option_by_sectionname(opmode, section, idx, domain_string, sizeof domain_string);
    }

    if(jarr_roaming_consortium != NULL){
        array_length = json_object_array_length(jarr_roaming_consortium);
        for (json_array_idx = 0; json_array_idx < array_length; json_array_idx++){
            jarr_roaming_consortium_info = json_object_array_get_idx(jarr_roaming_consortium, json_array_idx);
            snprintf(roaming_consortium, sizeof(roaming_consortium), "%s", json_object_get_string(jarr_roaming_consortium_info));
            if(json_array_idx == 0)
                snprintf(roaming_consortium_string, sizeof(roaming_consortium_string), "%s", roaming_consortium);
            else{
                strncat(roaming_consortium_string, ",", sizeof(roaming_consortium_string) - strlen(roaming_consortium_string) - 1);
                strncat(roaming_consortium_string, roaming_consortium, sizeof(roaming_consortium_string) - strlen(roaming_consortium_string) - 1);
                //snprintf(roaming_consortium_string, sizeof(roaming_consortium_string), "%s,%s", roaming_consortium_string, roaming_consortium);
            }
        }
        api_set_wifi_ifacex_hs20_roaming_consortium_option_by_sectionname(opmode, section, idx, roaming_consortium_string, sizeof roaming_consortium_string);
    }

#if 0
//****************************************************FOR HOTSPOT2.0 PHASE 2******************************************************************************
    api_set_wifi_ifacex_hs20_internetworking_enable_option_by_sectionname(opmode, section, idx, internet_access);
    api_set_wifi_ifacex_hs20_operating_class_option_by_sectionname(opmode, section, idx, operating_class);
    json_set_hotspot20_wan_metrics(opmode, idx, section, wan_metrics, &error_code, msg);
    if(jarr_anqp_3gpp_cell_net != NULL){
        array_length = json_object_array_length(jarr_anqp_3gpp_cell_net);
        debug_print("%s:%d array_length[%d] ###\n", __FUNCTION__, __LINE__, array_length);
        for (json_array_idx = 0; json_array_idx < array_length; json_array_idx++){
           jarr_anqp_3gpp_cell_net_info = json_object_array_get_idx(jarr_anqp_3gpp_cell_net, json_array_idx);
            senao_json_object_get_integer(jarr_anqp_3gpp_cell_net_info, "mcc", &mcc);
            senao_json_object_get_integer(jarr_anqp_3gpp_cell_net_info, "mnc", &mnc);
            snprintf(mcc_mnc, sizeof(mcc_mnc), "%d,%d", mcc, mnc);
            if(json_array_idx == 0)
                snprintf(anqp_3gpp_cell_net_string, sizeof(anqp_3gpp_cell_net_string), "%s", mcc_mnc);
            else{
                strncat(anqp_3gpp_cell_net_string, ";", sizeof(anqp_3gpp_cell_net_string) - strlen(anqp_3gpp_cell_net_string) - 1);
                strncat(anqp_3gpp_cell_net_string, mcc_mnc, sizeof(anqp_3gpp_cell_net_string) - strlen(anqp_3gpp_cell_net_string) - 1);
                //snprintf(anqp_3gpp_cell_net_string, sizeof(anqp_3gpp_cell_net_string), "%s;%s", anqp_3gpp_cell_net_string, mcc_mnc);
            }
            debug_print("%s:%d %s###\n", __FUNCTION__, __LINE__, anqp_3gpp_cell_net_string);
        }
        api_set_wifi_ifacex_hs20_anqp_3gpp_cell_net_option_by_sectionname(opmode, section, idx, anqp_3gpp_cell_net_string, sizeof anqp_3gpp_cell_net_string);
    }

    if(jarr_connection_capability != NULL){
        array_length = json_object_array_length(jarr_connection_capability);
        for (json_array_idx = 0; json_array_idx < array_length; json_array_idx++){
            jarr_connection_capability_info = json_object_array_get_idx(jarr_connection_capability, json_array_idx);
            senao_json_object_get_integer(jarr_connection_capability_info, "protocol", &protocol);
            senao_json_object_get_integer(jarr_connection_capability_info, "port", &port);
            senao_json_object_get_integer(jarr_connection_capability_info, "status", &status);
            snprintf(connection_capability, sizeof(connection_capability), "%d:%d:%d", protocol, port, status);
            debug_print("%s:%d %s###\n", __FUNCTION__, __LINE__, connection_capability);
            if(json_array_idx == 0)
                snprintf(conn_capab_string, sizeof(conn_capab_string), "%s", connection_capability);
            else{
                strncat(conn_capab_string, ",", sizeof(conn_capab_string) - strlen(conn_capab_string) - 1);
                strncat(conn_capab_string, connection_capability, sizeof(conn_capab_string) - strlen(conn_capab_string) - 1);
                //snprintf(conn_capab_string, sizeof(conn_capab_string), "%s,%s", conn_capab_string, connection_capability);
            }
        }
        api_set_wifi_ifacex_hs20_connection_capability_option_by_sectionname(opmode, section, idx, conn_capab_string, sizeof conn_capab_string);
    }

    memset(filename, 0, sizeof(filename));
    snprintf(filename, sizeof(filename), "%s", TMP_REALM_FILE);
    if(jarr_realm != NULL){
        array_length = json_object_array_length(jarr_realm);
        for (json_array_idx = 0; json_array_idx < array_length; json_array_idx++){
            jarr_realm_info = json_object_array_get_idx(jarr_realm, json_array_idx);
            strcpy(realm, json_object_get_string(jarr_realm_info));
            FILE* fp = fopen(filename, "w");
            if( NULL == fp )
            {
                debug_print("%s open failure", filename);
            }
            else
            {
                result = fputs(realm, fp);
                if ( result == EOF )
                    debug_print("write %s fail", filename);
                fclose(fp);
            }

            sys_interact(buf, sizeof(buf), "sh /sbin/convert_json_to_realm.sh %s" TMP_REALM_FILE);
            debug_print("%s:%d %s###\n", __FUNCTION__, __LINE__, buf);
            if(json_array_idx == 0)
                snprintf(realm_string, sizeof(realm_string), "%s", buf);
            else{
                strncat(realm_string, " ", sizeof(realm_string) - strlen(realm_string) - 1);
                strncat(realm_string, buf, sizeof(realm_string) - strlen(realm_string) - 1);
                //snprintf(realm_string, sizeof(realm_string), "%s %s", realm_string, buf);
            }
        }
        api_set_wifi_ifacex_hs20_nai_realm_option_by_sectionname(opmode, section, idx, realm_string, sizeof realm_string);
    }
#endif

    json_object_put(hotspot20);
    debug_print("Jason DEBUG %s:%d Set config end\n", __FUNCTION__, __LINE__);
}

int json_get_hotspot20_venue(int opmode, int idx, char* section, char *query_str, int str_size, int* error_code, char* msg)
{
    struct json_object *jobj = NULL, *jarr_venue_name = NULL, *jobj_tmp = NULL;
    int venue_group = 0, venue_type = 0, i = 0;
    char venue_name[1024] = {0};
    char *pstr = NULL, *saveptr = NULL, *delim = ",", *pstr2 = NULL, *saveptr2 = NULL, *delim2 = ":";

    jobj = json_object_new_object();

    api_get_wifi_ifacex_hs20_venue_group_option_by_sectionname(opmode, section, idx, &venue_group);
    api_get_wifi_ifacex_hs20_venue_type_option_by_sectionname(opmode, section, idx, &venue_type);
    api_get_wifi_ifacex_hs20_venue_name_option_by_sectionname(opmode, section, idx, venue_name, sizeof venue_name);

    jarr_venue_name = json_object_new_array();
    pstr = strtok_r(venue_name, delim, &saveptr);
    while ( pstr != NULL )
    {
        pstr2 = strtok_r(pstr, delim2, &saveptr2);
        jobj_tmp = json_object_new_object();
        i = 0;
        while( pstr2 != NULL )
        {
            if(i == 0)
                json_object_object_add(jobj_tmp, "language", json_object_new_string(pstr2));
            else
                json_object_object_add(jobj_tmp, "name", json_object_new_string(pstr2));
            pstr2 = strtok_r(NULL, delim2, &saveptr2);
            i++;
        }
        json_object_array_add(jarr_venue_name, jobj_tmp);
        pstr = strtok_r(NULL, delim, &saveptr);
    }

    json_object_object_add(jobj, "group", json_object_new_int(venue_group));
    json_object_object_add(jobj, "type", json_object_new_int(venue_type));
    json_object_object_add(jobj, "desc", jarr_venue_name);
    snprintf(query_str, str_size, "%s", json_object_to_json_string(jobj));

    json_object_put(jobj);
}

int json_set_hotspot20_venue(int opmode, int idx, char* section, char *query_str, int* error_code, char* msg)
{
    struct json_object *jobj = NULL, *jarr_desc = NULL, *jarr_info = NULL;
    int group = 0, type = 0, array_length = 0, json_array_idx = 0;
    char desc[1024] = {0}, desc_string[1024] = {0}, language[128] = {0}, name[128] = {0};

    if ((jobj = json_tokener_parse(query_str)) != NULL){
        senao_json_object_get_integer(jobj, "group", &group);
        senao_json_object_get_integer(jobj, "type", &type);
        jarr_desc = json_object_object_get(jobj, "desc");
    }
    else
        return false;

    api_set_wifi_ifacex_hs20_venue_group_option_by_sectionname(opmode, section, idx, group);
    api_set_wifi_ifacex_hs20_venue_type_option_by_sectionname(opmode, section, idx, type);

    if(jarr_desc != NULL){
        array_length = json_object_array_length(jarr_desc);
        for (json_array_idx = 0; json_array_idx < array_length; json_array_idx++){
            jarr_info = json_object_array_get_idx(jarr_desc, json_array_idx);
            senao_json_object_get_string(jarr_info, "language", language);
            senao_json_object_get_string(jarr_info, "name", name);
            snprintf(desc, sizeof(desc), "%s:%s", language, name);
            if(json_array_idx == 0)
                snprintf(desc_string, sizeof(desc_string), "%s", desc);
            else{
                strncat(desc_string, ",", sizeof(desc_string) - strlen(desc_string) -1);
                strncat(desc_string, desc, sizeof(desc_string) - strlen(desc_string) -1);
                //snprintf(desc_string, sizeof(desc_string), "%s,%s", desc_string, desc);
            }
        }
        api_set_wifi_ifacex_hs20_venue_name_option_by_sectionname(opmode, section, idx, desc_string, sizeof desc_string);
    }
    json_object_put(jobj);
}

int json_get_hotspot20_network(int opmode, int idx, char* section, char *query_str, int str_size, int* error_code, char* msg)
{
    struct json_object *jobj = NULL;
    int type = 0, auth_type = 0, i = 0;
    char auth_url[128] = {0}, auth[130] = {0}, authtype[3] = {0};

    jobj = json_object_new_object();

    api_get_wifi_ifacex_hs20_network_type_option_by_sectionname(opmode, section, idx, &type);
    api_get_wifi_ifacex_hs20_auth_type_option_by_sectionname(opmode, section, idx, auth, sizeof auth);

    authtype[0] = auth[0];
    authtype[1] = auth[1];
    auth_type = atoi(authtype);

    for(i = 0; i < sizeof auth_url; i++)
        auth_url[i] = auth[i+2];

    json_object_object_add(jobj, "type" , json_object_new_int(type));
    json_object_object_add(jobj, "auth_type", json_object_new_int(auth_type));
    json_object_object_add(jobj, "auth_url", json_object_new_string(auth_url));
    snprintf(query_str, str_size, "%s", json_object_to_json_string(jobj));
    json_object_put(jobj);
}

int json_set_hotspot20_network(int opmode, int idx, char* section, char *query_str, int* error_code, char* msg)
{
    struct json_object *jobj = NULL;
    int type = 0, auth_type = 0;
    char auth_url[512] = {0}, auth[512] = {0};

    if((jobj = json_tokener_parse(query_str)) != NULL){
        senao_json_object_get_integer(jobj, "type", &type);
        senao_json_object_get_integer(jobj, "auth_type", &auth_type);
        senao_json_object_get_string(jobj, "auth_url", auth_url);
    }
    else{
        return false;
    }
    snprintf(auth, sizeof(auth), "0%d%s", auth_type, auth_url);
    api_set_wifi_ifacex_hs20_network_type_option_by_sectionname(opmode, section, idx, type);
    api_set_wifi_ifacex_hs20_auth_type_option_by_sectionname(opmode, section, idx, auth, sizeof auth);

    json_object_put(jobj);
}


//****************************************************FOR HOTSPOT2.0 PHASE 2******************************************************************************

int json_get_hotspot20_osu(int opmode, int idx, char* section, char *query_str, int str_size, int* error_code, char* msg)
{
    struct json_object *jobj = NULL, *jobj_tmp = NULL, *jarr_osu_provider = NULL, *jarr_provider_desc = NULL;
    struct json_object *jobj_tmp_provider_desc = NULL, *jobj_tmp_operator_friendly_name = NULL, *jobj_tmp_service_desc = NULL;
    char osu_ssid[1024] = {0}, osu_server_uri[1024] = {0}, osu_friendly_name[1024] = {0}, osu_nai[1024] = {0}, osu_method[1024] = {0}, osu_service_desc[1024] = {0};
    char *pstr = NULL, *saveptr = NULL, *pstr2 = NULL, *saveptr2 = NULL, *pstr3 = NULL, *saveptr3 = NULL;
    char *pstr4 = NULL, *saveptr4 = NULL, *pstr5 = NULL, *saveptr5 = NULL, *pstr6 = NULL, *savepstr6 = NULL, *pstr7 = NULL, *savepstr7 = NULL;
    char *delim = ",", *delim2 = ":";
    int i = 0;

    jobj = json_object_new_object();
    jarr_osu_provider = json_object_new_array();
    jarr_provider_desc = json_object_new_array();
    jobj_tmp = json_object_new_object();

    api_get_wifi_ifacex_hs20_osu_ssid_option_by_sectionname(opmode, section, idx, osu_ssid, sizeof osu_ssid);
    api_get_wifi_ifacex_hs20_osu_server_uri_option_by_sectionname(opmode, section, idx, osu_server_uri, sizeof osu_server_uri);
    api_get_wifi_ifacex_hs20_osu_friendly_name_option_by_sectionname(opmode, section, idx, osu_friendly_name, sizeof osu_friendly_name);
    api_get_wifi_ifacex_hs20_osu_nai_option_by_sectionname(opmode, section, idx, osu_nai, sizeof osu_nai);
    api_get_wifi_ifacex_hs20_osu_method_option_by_sectionname(opmode, section, idx, osu_method, sizeof osu_method);
    api_get_wifi_ifacex_hs20_osu_service_desc_option_by_sectionname(opmode, section, idx, osu_service_desc, sizeof osu_service_desc);
    pstr = strtok_r(osu_server_uri, delim, &saveptr);
    pstr2 = strtok_r(osu_nai, delim, &saveptr2);
    pstr3 = strtok_r(osu_method, delim, &saveptr3);
    pstr4 = strtok_r(osu_friendly_name, delim, &saveptr4);
    pstr5 = strtok_r(osu_service_desc, delim, &saveptr5);
    if(pstr == NULL){
        snprintf(query_str, str_size, "%s", "");
    }
    else{
        while ( pstr != NULL && pstr2 != NULL && pstr3 != NULL && pstr4 != NULL && pstr5 != NULL )
        {
            json_object_object_add(jobj_tmp, "server_uri", json_object_new_string(pstr));
            json_object_object_add(jobj_tmp, "nai", json_object_new_string(pstr2));
            json_object_object_add(jobj_tmp, "method", json_object_new_string(pstr3));

            pstr6 = strtok_r(pstr4, delim2, &savepstr6);
            pstr7 = strtok_r(pstr5, delim2, &savepstr7);

            jobj_tmp_provider_desc = json_object_new_object();
            jobj_tmp_operator_friendly_name = json_object_new_object();
            jobj_tmp_service_desc = json_object_new_object();
            i = 0;
            while (pstr6 != NULL && pstr7 != NULL )
            {
                if(i == 0){
                    json_object_object_add(jobj_tmp_operator_friendly_name, "language", json_object_new_string(pstr6));
                    json_object_object_add(jobj_tmp_service_desc, "language", json_object_new_string(pstr7));
                }
                else{
                    json_object_object_add(jobj_tmp_operator_friendly_name, "name", json_object_new_string(pstr6));
                    json_object_object_add(jobj_tmp_service_desc, "desc", json_object_new_string(pstr7));
                }
                pstr6 = strtok_r(NULL, delim2, &savepstr6);
                pstr7 = strtok_r(NULL, delim2, &savepstr7);
                i++;
            }
            json_object_object_add(jobj_tmp_provider_desc, "operator_friendly_name", jobj_tmp_operator_friendly_name);
            json_object_object_add(jobj_tmp_provider_desc, "service_desc", jobj_tmp_service_desc);
            json_object_array_add(jarr_provider_desc, jobj_tmp_provider_desc);
            json_object_object_add(jobj_tmp, "provider_desc", jarr_provider_desc);
            json_object_array_add(jarr_osu_provider, jobj_tmp);
            pstr = strtok_r(NULL, delim, &saveptr);
            pstr2 = strtok_r(NULL, delim, &saveptr2);
            pstr3 = strtok_r(NULL, delim, &saveptr3);
            pstr4 = strtok_r(NULL, delim, &saveptr4);
            pstr5 = strtok_r(NULL, delim, &saveptr5);
        }

        json_object_object_add(jobj, "ssid", json_object_new_string(osu_ssid));
        json_object_object_add(jobj, "provider", jarr_osu_provider);
        snprintf(query_str, str_size, "%s", json_object_to_json_string(jobj));
    }

    json_object_put(jobj);
}

int json_set_hotspot20_osu(int opmode, int idx, char* section, char *query_str, int* error_code, char* msg)
{
    struct json_object *jobj = NULL, *jarr_provider = NULL, *jarr_provider_desc = NULL, *jarr_friendly_name = NULL, *jarr_service_desc = NULL;
    struct json_object *jarr_provider_info = NULL, *jarr_desc_info = NULL;
    char ssid[128] = {0}, provider[1024] = {0}, server_uri[128] = {0}, nai[128] = {0}, method[128] = {0}, provider_desc[512] = {0};
    char friendly_name[256] = {0}, service_desc[256] = {0}, friendly_name_language[128] = {0}, friendly_name_name[128] = {0}, service_desc_language[128] = {0};
    char service_desc_name[128] = {0};
    int provider_array_length = 0, provider_desc_array_length = 0, json_array_idx_1 = 0, json_array_idx_2 = 0;

    if ((jobj = json_tokener_parse(query_str)) != NULL){
        senao_json_object_get_string(jobj, "ssid", ssid);
        jarr_provider = json_object_object_get(jobj, "provider");
    }

    api_set_wifi_ifacex_hs20_osu_ssid_option_by_sectionname(opmode, section, idx, ssid, sizeof ssid);

    if(jarr_provider != NULL) {
        provider_array_length = json_object_array_length(jarr_provider);
        for (json_array_idx_1 = 0; json_array_idx_1 < provider_array_length; json_array_idx_1++){
            jarr_provider_info = json_object_array_get_idx(jarr_provider, json_array_idx_1);
            senao_json_object_get_string(jarr_provider_info, "server_uri", server_uri);
            senao_json_object_get_string(jarr_provider_info, "nai", nai);
            senao_json_object_get_string(jarr_provider_info, "method", method);
            jarr_provider_desc = json_object_object_get(jarr_provider_info, "provider_desc");
            if(jarr_provider_desc != NULL){
                provider_desc_array_length = json_object_array_length(jarr_provider_desc);
                for (json_array_idx_2 = 0; json_array_idx_2 < provider_desc_array_length; json_array_idx_2++){
                    jarr_desc_info = json_object_array_get_idx(jarr_provider_desc, json_array_idx_2);
                    jarr_friendly_name = json_object_object_get(jarr_desc_info, "operator_friendly_name");
                    jarr_service_desc = json_object_object_get(jarr_desc_info, "service_desc");
                    if(jarr_friendly_name != NULL){
                        senao_json_object_get_string(jarr_friendly_name, "language", friendly_name_language);
                        senao_json_object_get_string(jarr_friendly_name, "name", friendly_name_name);
                        snprintf(friendly_name, sizeof(friendly_name), "%s:%s", friendly_name_language, friendly_name_name);
                        api_set_wifi_ifacex_hs20_osu_friendly_name_option_by_sectionname(opmode, section, idx, friendly_name, sizeof friendly_name);
                    }
                    if(jarr_service_desc != NULL){
                        senao_json_object_get_string(jarr_service_desc, "language", service_desc_language);
                        senao_json_object_get_string(jarr_service_desc, "desc", service_desc_name);
                        snprintf(service_desc, sizeof(service_desc), "%s:%s", service_desc_language, service_desc_name);
                        api_set_wifi_ifacex_hs20_osu_service_desc_option_by_sectionname(opmode, section, idx, service_desc, sizeof service_desc);
                    }
                }
            }
        }
        api_set_wifi_ifacex_hs20_osu_server_uri_option_by_sectionname(opmode, section, idx, server_uri, sizeof server_uri);
        api_set_wifi_ifacex_hs20_osu_nai_option_by_sectionname(opmode, section, idx, nai, sizeof nai);
        api_set_wifi_ifacex_hs20_osu_method_option_by_sectionname(opmode, section, idx, method, sizeof method);
    }

    json_object_put(jobj);
}

int json_get_hotspot20_wan_metrics(int opmode, int idx, char* section, char *query_str, int str_size, int* error_code, char* msg)
{
    struct json_object *jobj = NULL, *jarr_wan_metrics = NULL;
    char wan_metrics[128] = {0}, tmp[10] = {0}, wan_info_char[2] = {0};
    char *pstr = NULL, *saveptr = NULL, *delim = ":";
    int i =0, wan_info = 0, downlink_speed = 0, uplink_speed = 0, downlink_load = 0, uplink_load = 0, load_measurement_duration = 0;

    jobj = json_object_new_object();

    api_get_wifi_ifacex_hs20_wan_metrics_option_by_sectionname(opmode, section, idx, wan_metrics, sizeof wan_metrics);

    pstr = strtok_r(wan_metrics, delim, &saveptr);
        while ( pstr != NULL )
        {
            if(i == 0){
                snprintf(tmp, sizeof tmp, "%s", pstr);
                wan_info_char[0] = tmp[1];
                wan_info = atoi(wan_info_char);
                json_object_object_add(jobj, "wan_info", json_object_new_int(wan_info));
            }
            else if(i == 1){
                downlink_speed = atoi(pstr);
                json_object_object_add(jobj, "downlink_speed", json_object_new_int(downlink_speed));
            }
            else if(i == 2){
                uplink_speed = atoi(pstr);
                json_object_object_add(jobj, "uplink_speed", json_object_new_int(uplink_speed));
            }
            else if(i == 3){
                downlink_load = atoi(pstr);
                json_object_object_add(jobj, "downlink_load", json_object_new_int(downlink_load));
            }
            else if(i == 4){
                uplink_load = atoi(pstr);
                json_object_object_add(jobj, "uplink_load", json_object_new_int(uplink_load));
            }
            else{
                load_measurement_duration = atoi(pstr);
                json_object_object_add(jobj, "load_measurement_duration", json_object_new_int(load_measurement_duration));
            }
            i++;
            pstr = strtok_r(NULL, delim, &saveptr);
        }
    snprintf(query_str, str_size, "%s", json_object_to_json_string(jobj));
    json_object_put(jobj);
}

int json_set_hotspot20_wan_metrics(int opmode, int idx, char* section, char *query_str, int* error_code, char* msg)
{
    struct json_object *jobj = NULL;
    int wan_info = 0, downlink_speed = 0, uplink_speed = 0, downlink_load = 0, uplink_load = 0, load_measurement_duration = 0;
    char wan_metrics[128] = {0};

    if((jobj = json_tokener_parse(query_str)) != NULL){
      senao_json_object_get_integer(jobj, "wan_info", &wan_info);
      senao_json_object_get_integer(jobj, "downlink_speed", &downlink_speed);
      senao_json_object_get_integer(jobj, "uplink_speed", &uplink_speed);
      senao_json_object_get_integer(jobj, "downlink_load", &downlink_load);
      senao_json_object_get_integer(jobj, "uplink_load", &uplink_load);
      senao_json_object_get_integer(jobj, "load_measurement_duration", &load_measurement_duration);
    }
    snprintf(wan_metrics, sizeof(wan_metrics), "0%d:%d:%d:%d:%d:%d", wan_info, downlink_speed, uplink_speed, downlink_load, uplink_load, load_measurement_duration);
    api_set_wifi_ifacex_hs20_wan_metrics_option_by_sectionname(opmode, section, idx, wan_metrics, sizeof wan_metrics);
}
