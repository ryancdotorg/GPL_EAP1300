#include "common.h"
#include "sysAddr.h"
#include "deviceinfo.h"

#include <api_sys.h>

int json_get_sys_sys_info(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "ipcammgr_cli getfunc sys_info");
	sys_interact_long(json_reply, MAX_JSON_REPLY_LEN, cmd);

	debug_print("--- JSON DEBUG %s[%d]: #.%s.#\n", __FUNCTION__, __LINE__, json_reply);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int get_wan_json_cb(struct json_object *jobj, WAN_SETTINGS_T *settings)
{
	struct json_object *jobj_wan_settings;
	struct json_object *jstr_dns;

	jobj_wan_settings = json_object_new_object();

	/* Construct the packet content in json format. */
	//jobj = json_object_new_object();

	//jstr_result = json_object_new_string(result);
	//json_object_object_add(jobj, "GetWanSettingsResult", jstr_result);

	json_object_object_add(jobj_wan_settings, "Type", json_object_new_string(settings->type));

	json_object_object_add(jobj_wan_settings, "IPAddress", json_object_new_string(settings->ip_address));
	json_object_object_add(jobj_wan_settings, "SubnetMask", json_object_new_string(settings->subnet_mask));
	json_object_object_add(jobj_wan_settings, "Gateway", json_object_new_string(settings->gateway));
	json_object_object_add(jobj_wan_settings, "SubnetMask", json_object_new_string(settings->subnet_mask));
	json_object_object_add(jobj_wan_settings, "MACAddress", json_object_new_string(settings->mac_address));

	jstr_dns = json_object_new_object();

	json_object_object_add(jstr_dns, "Primary", json_object_new_string(settings->dns_primary));
	json_object_object_add(jstr_dns, "Secondary", json_object_new_string(settings->dns_secondary));

	json_object_object_add(jobj_wan_settings, "DNS", jstr_dns);

	json_object_object_add(jobj, "wan", jobj_wan_settings);

	return 0;
}

int json_get_sys_wan(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;
	char cmd[256] = {0};

	char lanIP[17+1], netmask[17+1], lanGW[17+1], dns1[15+1], dns2[15+1], type[10], username[128], password[128], gateway[32], service[128], wantype[128], iptype[8];
	char opMode[8], ifName[16], buf[32];

	WAN_SETTINGS_T settings;

	memset(&settings, 0, sizeof(settings));

	api_get_string_option(NETWORK_WAN_PROTO_OPTION, type, sizeof(type));

	debug_print("--- JSON DEBUG %s[%d] type: #.%s.#\n", __FUNCTION__, __LINE__, type);

	if(strcmp(type, "dhcp") == 0)
	{
		sprintf(settings.type, "%s", "dhcp");

		api_get_string_option("system.@system[0].opmode", opMode, sizeof(opMode));

		if(0 == strcmp(opMode, "ar"))
		{
			api_get_string_option(NETWORK_WAN_IFNAME_OPTION, ifName, sizeof(ifName));

			if(0 != strlen(ifName))
			{
				sysutil_get_interface_ipaddr(ifName, lanIP, sizeof(lanIP));
				sprintf(settings.ip_address, "%s", lanIP);

				sysutil_get_interface_netmask(ifName, netmask, sizeof(netmask));
				sprintf(settings.subnet_mask, "%s", netmask);

				sysutil_get_interface_gateway(ifName, lanGW, sizeof(lanGW));
				sprintf(settings.gateway, "%s", lanGW);

	#if SUPPORT_WAN_SETTING
				debug_print("--- JSON DEBUG %s[%d] SUPPORT_WAN_SETTING\n", __FUNCTION__, __LINE__);
				sysutil_get_wan_dns(1, dns1, sizeof(dns1));
				sysutil_get_wan_dns(2, dns2, sizeof(dns2));
	#else
				sysutil_get_interface_dns(ifName, 1, dns1, sizeof(dns1));
				sysutil_get_interface_dns(ifName, 2, dns2, sizeof(dns2));
	#endif
				sprintf(settings.dns_primary, "%s", dns1);
				sprintf(settings.dns_secondary, "%s", dns2);

				sys_interact_long(buf, sizeof(buf), "setconfig -g 6 | tr -d '\n'");
				sprintf(settings.mac_address, "%s", buf);
			}
		}
	}
	else
	{
		strcpy(settings.type, "static");

		api_get_string_option(NETWORK_WAN_IPADDR_OPTION, lanIP, sizeof(lanIP));
		sprintf(settings.ip_address, "%s", lanIP);
		api_get_string_option(NETWORK_WAN_NETMASK_OPTION, netmask, sizeof(netmask));
		sprintf(settings.subnet_mask, "%s", netmask);
		api_get_string_option(NETWORK_WAN_GATEWAY_OPTION, lanGW, sizeof(lanGW));
		sprintf(settings.gateway, "%s", lanGW);
		api_get_string_option(NETWORK_WAN_DNS_OPTION, buf, sizeof(buf));
		sscanf(buf, "%s %s", settings.dns_primary, settings.dns_secondary);

		sys_interact_long(buf, sizeof(buf), "setconfig -g 6 | tr -d '\n'");
		sprintf(settings.mac_address, "%s", buf);
	}

	get_wan_json_cb(jobj, &settings);
	//memcpy(json_reply, json_object_to_json_string(jobj), MAX_JSON_REPLY_LEN);
	//debug_print("--- JSON DEBUG %s[%d]: #.%s.#\n", __FUNCTION__, __LINE__, json_reply);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

bool parse_json_wan(char *query_str, WAN_SETTINGS_T *settings)
{
	struct json_object *jobj, *jobj_wan_settings;
	struct json_object *jobj_type, *jobj_ip, *jobj_subnetmask, *jobj_gateway, *jobj_mac;
	struct json_object *jobj_dns, *jobj_primary, *jobj_secondary;

	ResponseEntry *rep = Response_create();
	char *WanSettings = NULL;
	int i;

	if((jobj = jsonTokenerParseFromStack(rep, query_str)))
	{
		senao_json_object_get_and_create_string(rep, jobj, "wan", &WanSettings);

		if(jobj_wan_settings = jsonTokenerParseFromStack(rep, WanSettings))
		{
			if((jobj_type = json_object_object_get(jobj_wan_settings, "Type")))
			{
				//result = senao_json_object_get_and_create_string(rep, jobj_wan_settings, "Type", setting->type);
				sprintf(settings->type, "%s", json_object_get_string(jobj_type));
				json_object_put(jobj_type);
			}

			if(strcmp(settings->type, "static") == 0)
			{
				if((jobj_ip = json_object_object_get(jobj_wan_settings, "IPAddress")))
				{
					sprintf(settings->ip_address, "%s", json_object_get_string(jobj_ip));
					json_object_put(jobj_ip);
				}

				if((jobj_subnetmask = json_object_object_get(jobj_wan_settings, "SubnetMask")))
				{
					sprintf(settings->subnet_mask, "%s", json_object_get_string(jobj_subnetmask));
					json_object_put(jobj_subnetmask);
				}

				if((jobj_gateway = json_object_object_get(jobj_wan_settings, "Gateway")))
				{
					sprintf(settings->gateway, "%s", json_object_get_string(jobj_gateway));
					json_object_put(jobj_gateway);
				}

				jobj_dns = json_object_object_get(jobj_wan_settings, "DNS");
				if((jobj_primary = json_object_object_get(jobj_dns, "Primary")))
				{
					sprintf(settings->dns_primary, "%s", json_object_get_string(jobj_primary));
					json_object_put(jobj_primary);
				}

				if((jobj_secondary = json_object_object_get(jobj_dns, "Secondary")))
				{
					sprintf(settings->dns_secondary, "%s", json_object_get_string(jobj_secondary));
					json_object_put(jobj_secondary);
				}

				json_object_put(jobj_dns);
			}

			if((jobj_mac = json_object_object_get(jobj_wan_settings, "MACAddress")))
			{
				sprintf(settings->mac_address, "%s", json_object_get_string(jobj_mac));
				for(i = 0; i < sizeof(settings->mac_address); i++)
				{
					settings->mac_address[i] = toupper(settings->mac_address[i]);
					//debug_print("settings->mac_address[%d]: %c\n", i, settings->mac_address[i]);
				}
				json_object_put(jobj_mac);
			}
		}
	}

	Response_destroy(rep);
}

int json_set_sys_wan(ResponseEntry *rep, char *query_str, char *json_reply)
{
	char cmd[MAX_JSON_REPLY_LEN] = {0};
	char buf[512] = {0};

	ResponseStatus *res = rep->res;
	WAN_SETTINGS_T settings;
	char dns[128];
	char ori_mac_address[32];
	int i;

	char ifName[16], lanIP[17+1];

	memset(&settings, 0, sizeof(settings));
	parse_json_wan(query_str, &settings);

	api_set_string_option(NETWORK_WAN_PROTO_OPTION, settings.type, sizeof(settings.type));

	if(strcmp(settings.type, "static") == 0)
	{
		if(!api_check_ip_addr(settings.ip_address))
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, ERROR_IP_ADDRESS_STR);
		}
		if(!api_check_mask_addr(settings.subnet_mask))
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, ERROR_SUBNET_MASK_STR);
		}
		if(!api_check_ip_addr(settings.gateway))
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, ERROR_GATEWAY_STR);
		}
		if((strlen(settings.dns_primary) > 0) && !api_check_dns_addr(settings.dns_primary))
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, ERROR_IP_ADDRESS_STR); // ERROR_STR
		}
		if((strlen(settings.dns_secondary) > 0) && !api_check_dns_addr(settings.dns_secondary))
		{
			RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, ERROR_IP_ADDRESS_STR);
		}

		memset(dns, 0x0, sizeof(dns));
		if(strlen(settings.dns_secondary) != 0)
		{
			strcpy(dns, settings.dns_primary);
			strcat(dns, " ");
			strcat(dns, settings.dns_secondary);
			api_set_string_option(NETWORK_WAN_DNS_OPTION, dns, sizeof(dns));
		}
		else
		{
			api_set_string_option(NETWORK_WAN_DNS_OPTION, settings.dns_primary, sizeof(settings.dns_primary));
		}

		api_set_lan_ipaddr_option(NETWORK_WAN_IPADDR_OPTION, settings.ip_address, sizeof(settings.ip_address));
		api_set_lan_netmask_option(NETWORK_WAN_NETMASK_OPTION, settings.subnet_mask, sizeof(settings.subnet_mask));
		api_set_lan_gateway_option(NETWORK_WAN_GATEWAY_OPTION, settings.gateway, sizeof(settings.gateway));
	}
	else
	{
		api_get_string_option(NETWORK_WAN_IFNAME_OPTION, ifName, sizeof(ifName));

		if(0 != strlen(ifName))
		{
			sysutil_get_interface_ipaddr(ifName, lanIP, sizeof(lanIP));
			sprintf(settings.ip_address, "%s", lanIP);
			debug_print("settings.ip_address: %s\n", settings.ip_address);
		}
	}

	sys_interact_long(ori_mac_address, sizeof(ori_mac_address), "setconfig -g 6 | tr -d '\n'");

	for(i = 0; i < sizeof(ori_mac_address); i++)
	{
		ori_mac_address[i] = toupper(ori_mac_address[i]);
		//debug_print("ori_mac_address[%d]: %c\n", i, ori_mac_address[i]);
	}

	debug_print("Jason DEBUG %s[%d], ori_mac_address [%s]\n", __FUNCTION__, __LINE__, ori_mac_address);
	debug_print("Jason DEBUG %s[%d], settings.mac_address [%s]\n", __FUNCTION__, __LINE__, settings.mac_address);

	if(strcmp(ori_mac_address, settings.mac_address) != 0)
	{
		api_set_wan_mac_option(NETWORK_WAN_MACADDR_OPTION, settings.mac_address, sizeof(settings.mac_address));

		debug_print("setconfig!\n");
		system("setconfig -a 1"); // generate /tmp/uboot_config
		SYSTEM("setconfig -a 2 -s 6 -d \"%s\"", settings.mac_address); // /tmp/uboot_config -> ethaddr=00:aa:e8:c5:da:30
		system("setconfig -a 5"); // writing from <stdin> to /dev/mtd8
		system("rm -rf /tmp/uboot_config");
	}

	snprintf(cmd, sizeof(cmd), "uci commit network");
	sys_interact_long(buf, sizeof(buf), cmd);

	SYSTEM("(sleep 2; /sbin/luci-reload auto network; sleep 1; arping -U %s -c 3) &", settings.ip_address);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int get_time_json_cb(struct json_object *jobj, TIME_SETTINGS_T *settings)
{
	/* Construct the packet content in json format. */
	//jobj = json_object_new_object();

	//jstr_result = json_object_new_string(result);
	//json_object_object_add(jobj, "GetWanSettingsResult", jstr_result);

	//json_object_object_add(jobj_time_settings, "Type", json_object_new_string(settings->type));

	/*json_object_object_add(jobj_time_settings, "NtpUsedIndex", json_object_new_int(settings->ntp_used_index));
	json_object_object_add(jobj_time_settings, "SubnetMask", json_object_new_string(settings->subnet_mask));
	json_object_object_add(jobj_time_settings, "Gateway", json_object_new_string(settings->gateway));
	json_object_object_add(jobj_time_settings, "SubnetMask", json_object_new_string(settings->subnet_mask));

	jstr_dns = json_object_new_object();

	json_object_object_add(jstr_dns, "Primary", json_object_new_string(settings->dns_primary));
	json_object_object_add(jstr_dns, "Secondary", json_object_new_string(settings->dns_secondary));

	json_object_object_add(jobj_wan_settings, "DNS", jstr_dns);
	json_object_object_add(jobj, "wan", jobj_wan_settings);


	ADD_JSON_OBJECT_NEW_INT(jobj, "SupportMethod", 3);
	ADD_JSON_OBJECT_NEW_INT(jobj, "NtpUsedIndex", settings->);
	ADD_JSON_OBJECT_NEW_INT(jobj, "TimeZoneID", settings->timezone_id);
	ADD_JSON_OBJECT_NEW_STRING(jobj, "NtpServer", settings->ntp_server_addr);

	#if HAS_WAN_AUTO_DETECTION
	api_get_string_option("system.ntp.auto_detect", buf, sizeof(buf));
	ADD_JSON_OBJECT_NEW_INT(jobj, "AutoDetection", atoi(buf));
	#endif
*/
	/*json_object_object_add(jobj_time_settings, "Year", json_object_new_int(settings->year));
	json_object_object_add(jobj_time_settings, "Month", json_object_new_int(settings->month));
	json_object_object_add(jobj_time_settings, "Day", json_object_new_int(settings->day));
	json_object_object_add(jobj_time_settings, "Hour", json_object_new_int(settings->hour));
	json_object_object_add(jobj_time_settings, "Minute", json_object_new_int(settings->minute));
	json_object_object_add(jobj_time_settings, "Second", json_object_new_int(settings->sec));*/
	//json_object_object_add(jobj_time_settings, "CurrentSysTime", settings->current_sys_time));

	//json_object_object_add(jobj_time_settings, "DayLightSavingEn", settings->daylight_saving_en);

	/*json_object_object_add(jobj_current_time, "date", json_object_new_string(current_date));
	json_object_object_add(jobj_current_time, "time", json_object_new_string(current_time));
	json_object_object_add(jobj_time, "current_time", jobj_current_time);
	json_object_object_add(jobj_time, "mode", json_object_new_string(time_mode));
	json_object_object_add(jobj_manual_set, "date", json_object_new_string(systime_date));
	json_object_object_add(jobj_manual_set, "time", json_object_new_string(systime_time));
	json_object_object_add(jobj_time, "Manual_set", jobj_manual_set);

	json_object_object_add(jobj_auto_set, "ntp_server", json_object_new_string(ntp_server));
	json_object_object_add(jobj_time, "Auto_set", jobj_auto_set);

	json_object_object_add(jobj_time, "time_zone", json_object_new_string(api_timezone_table[zone_number].zonename));
	json_object_object_add(jobj_time, "cur_time_zone", json_object_new_string(curtimezone));
	json_object_object_add(jobj_time, "time_zone_auto_detect", json_object_new_boolean(zone_auto_detect));*/

	//json_object_object_add(jobj, "time", jobj_time_settings);

	return 0;
}

int json_get_sys_time(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;

	struct json_object *jobj_time_settings;

	jobj_time_settings = json_object_new_object();

	int systime_manual_en = 0;
	int zone_auto_detect, zone_number;
	char timezone[128]={0}, curtimezone[128]={0}, tzname[64] = {0}, date[32] = {0}, time_mode[7] = {0}, systime_date[11] = {0}, systime_time[6] = {0}, current_date[11] = {0}, current_time[6] = {0};
	int systime_year = 0, systime_month = 0, systime_day = 0, systime_hour = 0, systime_minute = 0;
	int daylight_en = 0, daylight_start_month = 0, daylight_start_week, daylight_start_day = 0, daylight_start_hour = 0, daylight_end_month = 0, daylight_end_week, daylight_end_day = 0, daylight_end_hour = 0;
	char ntp_server[256] = {0}, daylight_start_hour_string[6] = {0}, daylight_end_hour_string[6] = {0}, buf[256] = {0}, dhcp[256] = {0};

	struct json_object *jobj_time = NULL, *jobj_current_time = NULL, *jobj_manual_set = NULL, *jobj_auto_set = NULL, *jobj_daylight_saving = NULL;

	char month[12][4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
	char weekday[7][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

	jobj_time = json_object_new_object();
	jobj_current_time = json_object_new_object();
	jobj_manual_set = json_object_new_object();
	jobj_auto_set = json_object_new_object();
	jobj_daylight_saving = json_object_new_object();

	/* time_zone */
	api_get_integer_option(SYSTEM_SYSTEM_ZONENUMBER_OPTION, &zone_number);
	api_get_system_timezone_option(SYSTEM_SYSTEM_ZONENAME_OPTION, timezone, sizeof(timezone));

	/* time_zone auto detect*/
	strcpy(curtimezone, "N/A");  //https://maps.googleapis.com is not available, so the zonename is incorrect
	api_get_integer_option("system.@system[0].zone_auto_detect", &zone_auto_detect);

	strcpy(curtimezone, api_timezone_table[zone_number].zonename);

	/* current time */
	sys_interact(current_date, sizeof(current_date), "%s", "date +%Y-%m-%d");
	sys_interact(current_time, sizeof(current_time), "%s", "date +%H:%M");

	/* time_mode */
	api_get_integer_option(SYSTIME_MANUAL_ENABLE_OPTION, &systime_manual_en);
	memset(time_mode, 0, sizeof(time_mode));
	if (systime_manual_en)
		sprintf(time_mode, "manual");
	else
		sprintf(time_mode, "auto");

	/* date */
	api_get_integer_option(SYSTIME_YEAR_ENABLE_OPTION, &systime_year);
	api_get_integer_option(SYSTIME_MONTH_ENABLE_OPTION, &systime_month);
	api_get_integer_option(SYSTIME_DAY_ENABLE_OPTION, &systime_day);
	memset(systime_date, 0, sizeof(systime_date));
	sprintf(systime_date, "%d/%d/%d", systime_year, systime_month, systime_day);

	/* time */
	api_get_integer_option(SYSTIME_HOUR_ENABLE_OPTION, &systime_hour);
	api_get_integer_option(SYSTIME_MINUTE_OPTION, &systime_minute);
	memset(systime_time, 0, sizeof(systime_time));

	if (systime_hour > 9 && systime_minute != 0)
	{
		sprintf(systime_time, "%d:%d", systime_hour, systime_minute);
	}
	else if (systime_hour <= 9 && systime_minute != 0)
	{
		sprintf(systime_time, "0%d:%d", systime_hour, systime_minute);
	}
	else if (systime_hour > 9 && systime_minute == 0)
	{
		sprintf(systime_time, "%d:00", systime_hour);
	}
	else
	{
		sprintf(systime_time, "0%d:00", systime_hour);
	}

	/* ntp_server */
	api_get_system_ntp_server_list(SYSTEM_NTP_SERVER_LIST, ntp_server, sizeof(ntp_server));

	/* daylight saving */
	api_get_integer_option(NTPCLIENT_DAYLIGHTSAVING_DAYLIGHTENABLE_OPTION, &daylight_en);
	api_get_integer_option(NTPCLIENT_DAYLIGHTSAVING_STARTMONTH_OPTION, &daylight_start_month);
	api_get_integer_option(NTPCLIENT_DAYLIGHTSAVING_STARTWEEK_OPTION, &daylight_start_week);
	api_get_integer_option(NTPCLIENT_DAYLIGHTSAVING_STARTDAY_OPTION, &daylight_start_day);
	api_get_integer_option(NTPCLIENT_DAYLIGHTSAVING_STARTHOUR_OPTION, &daylight_start_hour);
	api_get_integer_option(NTPCLIENT_DAYLIGHTSAVING_ENDMONTH_OPTION, &daylight_end_month);
	api_get_integer_option(NTPCLIENT_DAYLIGHTSAVING_ENDWEEK_OPTION, &daylight_end_week);
	api_get_integer_option(NTPCLIENT_DAYLIGHTSAVING_ENDDAY_OPTION, &daylight_end_day);
	api_get_integer_option(NTPCLIENT_DAYLIGHTSAVING_ENDHOUR_OPTION, &daylight_end_hour);
	sprintf(daylight_start_hour_string, "%d:00", daylight_start_hour );
	sprintf(daylight_end_hour_string, "%d:00", daylight_end_hour );

	json_object_object_add(jobj_current_time, "date", json_object_new_string(current_date));
	json_object_object_add(jobj_current_time, "time", json_object_new_string(current_time));
	json_object_object_add(jobj_time, "current_time", jobj_current_time);
	json_object_object_add(jobj_time, "mode", json_object_new_string(time_mode));
	json_object_object_add(jobj_manual_set, "date", json_object_new_string(systime_date));
	json_object_object_add(jobj_manual_set, "time", json_object_new_string(systime_time));
	json_object_object_add(jobj_time, "manual_set", jobj_manual_set);

	json_object_object_add(jobj_auto_set, "ntp_server", json_object_new_string(ntp_server));
	json_object_object_add(jobj_time, "auto_set", jobj_auto_set);

	json_object_object_add(jobj_time, "time_zone", json_object_new_string(api_timezone_table[zone_number].zonename));
	json_object_object_add(jobj_time, "cur_time_zone", json_object_new_string(curtimezone));
	json_object_object_add(jobj_time, "time_zone_auto_detect", json_object_new_boolean(zone_auto_detect));

	json_object_object_add(jobj_daylight_saving, "enable", json_object_new_boolean(daylight_en));
	json_object_object_add(jobj_daylight_saving, "start_month", json_object_new_string(month[daylight_start_month-1]));
	json_object_object_add(jobj_daylight_saving, "start_week", json_object_new_int(daylight_start_week));
	json_object_object_add(jobj_daylight_saving, "start_day", json_object_new_string(weekday[daylight_start_day]));
	json_object_object_add(jobj_daylight_saving, "start_time", json_object_new_string(daylight_start_hour_string));
	json_object_object_add(jobj_daylight_saving, "end_month", json_object_new_string(month[daylight_end_month-1]));
	json_object_object_add(jobj_daylight_saving, "end_week", json_object_new_int(daylight_end_week));
	json_object_object_add(jobj_daylight_saving, "end_day", json_object_new_string(weekday[daylight_end_day]));
	json_object_object_add(jobj_daylight_saving, "end_time", json_object_new_string(daylight_end_hour_string));
	json_object_object_add(jobj_time, "daylight_saving", jobj_daylight_saving);

	json_object_object_add(jobj, "time", jobj_time);
	//memcpy(json_reply, json_object_to_json_string(jobj), MAX_JSON_REPLY_LEN);
	debug_print("--- JSON DEBUG %s[%d]: #.%s.#\n", __FUNCTION__, __LINE__, json_reply);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_sys_time(ResponseEntry *rep, char *query_str, char *json_reply)
{
	int ret = 0;
	ResponseStatus *res = rep->res;

	char cmd[MAX_JSON_REPLY_LEN] = {0};
	char buf[256] = {0};

	struct json_object *jobj = NULL, *jobj_time = NULL, *jobj_manual_set = NULL, *jobj_auto_set = NULL, *jobj_daylight_saving = NULL;
	char *jobj_time_string=NULL, *jobj_manual_set_string=NULL, *jobj_auto_set_string=NULL, *jobj_daylight_saving_string=NULL;
	int i = 0, length = T_NUM_OF_ELEMENTS(api_timezone_table);
	bool daylight_saving_en = false, time_zone_auto_detect = false;
	char *time_mode=NULL, *date=NULL, *time=NULL, *ntp_server=NULL;
	int systime_manual_en = 0, year = 0, month = 0, day = 0, hour = 0, minute = 0, daylight_saving_start_week = 0, daylight_saving_end_week = 0;
	char *daylight_saving_start_month=NULL, *daylight_saving_start_day=NULL, *daylight_saving_start_time=NULL, *daylight_saving_end_month=NULL, *daylight_saving_end_day=NULL, *daylight_saving_end_time=NULL;
	char daylight_start_hour_string[6] = {0}, daylight_end_hour_string[6] = {0};

	char month_ary[12][4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
	char weekday[7][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

	if((jobj = jsonTokenerParseFromStack(rep, query_str)))
	{
		senao_json_object_get_and_create_string(rep, jobj, "time", &jobj_time_string);
debug_print("--- JSON DEBUG %s[%d] jobj_time_string: #.%s.#\n", __FUNCTION__, __LINE__, jobj_time_string);
		if(jobj_time = jsonTokenerParseFromStack(rep, jobj_time_string))
		{
			senao_json_object_get_and_create_string(rep, jobj_time, "mode", &time_mode);
debug_print("--- JSON DEBUG %s[%d] time_mode: #.%s.#\n", __FUNCTION__, __LINE__, time_mode);
			if (strcasecmp(time_mode, "manual") == 0)
			{
				systime_manual_en = 1;
				senao_json_object_get_and_create_string(rep, jobj_time, "manual_set", &jobj_manual_set_string);

				if((jobj_manual_set = jsonTokenerParseFromStack(rep, jobj_manual_set_string)))
				{
					senao_json_object_get_and_create_string(rep, jobj_manual_set, "date", &date);
					senao_json_object_get_and_create_string(rep, jobj_manual_set, "time", &time);

					if ( sscanf(date, "%d/%d/%d", &year, &month, &day) != 3 )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DATE");

debug_print("--- JSON DEBUG %s[%d] DATE: #.%d/%d/%d.#\n", __FUNCTION__, __LINE__, year, month, day);

					if ( sscanf(time, "%d:%d", &hour, &minute) != 2 )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME");
debug_print("--- JSON DEBUG %s[%d] TIME: #.%d:%d.#\n", __FUNCTION__, __LINE__, hour, minute);

					api_set_system_ntp_enable_server_option(SYSTEM_NTP_ENABLE_SERVER_OPTION, 0);

					ret = API_RC_SUCCESS ;
					ret |= api_set_integer_option(SYSTIME_YEAR_ENABLE_OPTION, year);
					ret |= api_set_integer_option(SYSTIME_MONTH_ENABLE_OPTION, month);
					ret |= api_set_integer_option(SYSTIME_DAY_ENABLE_OPTION, day);

					if ( ret != API_RC_SUCCESS )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DATE");

					ret = API_RC_SUCCESS ;
					ret |= api_set_integer_option(SYSTIME_HOUR_ENABLE_OPTION, hour);
					ret |= api_set_integer_option(SYSTIME_MINUTE_OPTION, minute);

					if ( ret != API_RC_SUCCESS )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME");
				}
			}
			else
			{
				systime_manual_en = 0;
				senao_json_object_get_and_create_string(rep, jobj_time, "auto_set", &jobj_auto_set_string);

				if((jobj_auto_set = jsonTokenerParseFromStack(rep, jobj_auto_set_string)))
				{
					senao_json_object_get_and_create_string(rep, jobj_auto_set, "ntp_server", &ntp_server);

					api_set_system_ntp_enable_server_option(SYSTEM_NTP_ENABLE_SERVER_OPTION, 1);

					if (api_set_system_ntp_server_list(SYSTEM_NTP_SERVER_LIST, ntp_server, sizeof(ntp_server)) )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "NTP SERVER");
				}
			}

			if (api_set_integer_option(SYSTIME_MANUAL_ENABLE_OPTION, systime_manual_en))
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME MODE");

			 /* time mode */
			if (api_set_integer_option(SYSTIME_MANUAL_ENABLE_OPTION, systime_manual_en))
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME MODE");

			/* time_zone */
			senao_json_object_get_and_create_string(rep, jobj_time, "time_zone", &timezone);

			for(i = 0 ; i < length; i++)
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

			/* time_zone auto detect */
			// senao_json_object_get_boolean(jobj_time, "time_zone_auto_detect", &time_zone_auto_detect);
			api_set_integer_option("system.@system[0].zone_auto_detect", time_zone_auto_detect?1:0);//default is false

			// if ( time_zone_auto_detect == true)
			// {
			//     api_set_string_option(SYSTEM_SYSTEM_ZONENAME_OPTION, "auto", 4);
			// }

			/* daylight_saving */
			senao_json_object_get_and_create_string(rep, jobj_time, "daylight_saving", &jobj_daylight_saving_string);

			if((jobj_daylight_saving = jsonTokenerParseFromStack(rep, jobj_daylight_saving_string)))
			{
				senao_json_object_get_boolean(jobj_daylight_saving, "enable", &daylight_saving_en );

				senao_json_object_get_and_create_string(rep, jobj_daylight_saving, "start_month", &daylight_saving_start_month);
				senao_json_object_get_integer(jobj_daylight_saving, "start_week", &daylight_saving_start_week);
				senao_json_object_get_and_create_string(rep, jobj_daylight_saving, "start_day", &daylight_saving_start_day);
				senao_json_object_get_and_create_string(rep, jobj_daylight_saving, "start_time", &daylight_saving_start_time);

				senao_json_object_get_and_create_string(rep, jobj_daylight_saving, "end_month", &daylight_saving_end_month);
				senao_json_object_get_integer(jobj_daylight_saving, "end_week", &daylight_saving_end_week);
				senao_json_object_get_and_create_string(rep, jobj_daylight_saving, "end_day", &daylight_saving_end_day);
				senao_json_object_get_and_create_string(rep, jobj_daylight_saving, "end_time", &daylight_saving_end_time);

				api_set_system_ntp_day_light_saving_enable_option(NTPCLIENT_DAYLIGHTSAVING_DAYLIGHTENABLE_OPTION, (daylight_saving_en)?1:0);
				if (daylight_saving_en == true) 
				{
					for (i = 0; i < 12; i++)
					{
						if ( strcasecmp(daylight_saving_start_month, month_ary[i] ) == 0 )
						{
							break;
						}
					}

					if ( api_set_system_ntp_day_light_saving_month_option(NTPCLIENT_DAYLIGHTSAVING_STARTMONTH_OPTION, i+1) != API_RC_SUCCESS )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING STARTMONTH");

					if ( api_set_system_ntp_day_light_saving_week_option(NTPCLIENT_DAYLIGHTSAVING_STARTWEEK_OPTION, daylight_saving_start_week) != API_RC_SUCCESS )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING STARTWEEK");

					for ( i = 0 ; i < 7 ; i++ )
					{
						if ( strcasecmp(daylight_saving_start_day, weekday[i] ) == 0 )
						{
							break;
						}
					}

					if ( api_set_system_ntp_day_light_saving_day_option(NTPCLIENT_DAYLIGHTSAVING_STARTDAY_OPTION, i ) != API_RC_SUCCESS )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING STARTDAY");

					if ( sscanf(daylight_saving_start_time, "%d:", &hour) != 1 )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING STARTIME");

					if ( api_set_system_ntp_day_light_saving_hour_option(NTPCLIENT_DAYLIGHTSAVING_STARTHOUR_OPTION, hour) != API_RC_SUCCESS )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING STARTTIME");

					for ( i = 0 ; i < 12 ; i++ )
					{
						if ( strcasecmp(daylight_saving_end_month, month_ary[i] ) == 0 )
						{
							break;
						}
					}

					if ( api_set_system_ntp_day_light_saving_month_option(NTPCLIENT_DAYLIGHTSAVING_ENDMONTH_OPTION, i+1 ) != API_RC_SUCCESS )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING ENDMONTH");

					if ( api_set_system_ntp_day_light_saving_week_option(NTPCLIENT_DAYLIGHTSAVING_ENDWEEK_OPTION, daylight_saving_end_week) != API_RC_SUCCESS )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING ENDWEEK");

					for ( i = 0 ; i < 7 ; i++ )
					{
						if ( strcasecmp(daylight_saving_end_day, weekday[i] ) == 0 )
						{
							break;
						}
					}

					if ( api_set_system_ntp_day_light_saving_day_option(NTPCLIENT_DAYLIGHTSAVING_ENDDAY_OPTION, i ) != API_RC_SUCCESS )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING ENDDAY");

					if ( sscanf(daylight_saving_end_time, "%d:", &hour) != 1 )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING ENDTIME");

					if ( api_set_system_ntp_day_light_saving_hour_option(NTPCLIENT_DAYLIGHTSAVING_ENDHOUR_OPTION, hour) != API_RC_SUCCESS )
						RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DAYLIGHTSAVING ENDTIME");
				}
			}
		}
	}
	else
	{
		debug_print("--- JSON DEBUG %s[%d] query_str: #.%s.#\n", __FUNCTION__, __LINE__, query_str);
	}

	snprintf(cmd, sizeof(cmd), "uci commit system");
	sys_interact_long(buf, sizeof(buf), cmd);

	snprintf(cmd, sizeof(cmd), "uci commit systime");
	sys_interact_long(buf, sizeof(buf), cmd);

	snprintf(cmd, sizeof(cmd), "luci-reload auto system");
	sys_interact_long(buf, sizeof(buf), cmd);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_sys_timezone(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	int zone_number = 0, zone_auto_detect = 0;
	int i = 0, length = T_NUM_OF_ELEMENTS(api_timezone_table);
	char curtimezone[128]={0}, tzname[64] = {0}, *tzbuffer;

	struct json_object *jobj_timezone;

	jobj_timezone = json_object_new_object();

	/* time_zone */
	api_get_integer_option(SYSTEM_SYSTEM_ZONENUMBER_OPTION, &zone_number);
	//api_get_system_timezone_option(SYSTEM_SYSTEM_ZONENAME_OPTION, timezone, sizeof(timezone));

	/* time_zone auto detect*/

	strcpy(curtimezone, "N/A"); //https://maps.googleapis.com is not available, so the zonename is incorrect
	api_get_integer_option("system.@system[0].zone_auto_detect", &zone_auto_detect);

#if 0
	if(zone_auto_detect == 1)
	{
		sys_interact(tzname, sizeof(tzname), "uci get system.@system[0].zonename");
		if (tzname[strlen(tzname)-1] == '\n')
			tzname[strlen(tzname)-1] = 0;

		if (strcmp(tzname, "auto")==0 || strcmp(tzname, "")==0)
		{
			strcpy(curtimezone, "N/A"); //https://maps.googleapis.com is not available, so the zonename is incorrect
		}
		else
		{
			for(i = 0 ; i < length; i++)
			{
				tzbuffer = strstr(api_timezone_table[i].autotimezonename, tzname);

				if (tzbuffer != NULL)
				{
					strcpy(curtimezone, api_timezone_table[i].zonename);
					break;
				}
			}
		}
	}
	else
	{
		strcpy(curtimezone, api_timezone_table[zone_number].zonename);
		debug_print("--- JSON DEBUG %s[%d] zonename: #.%s.#\n", __FUNCTION__, __LINE__, curtimezone);
	}
#endif

	json_object_object_add(jobj_timezone, "zonename", json_object_new_string(curtimezone));
	json_object_object_add(jobj_timezone, "time_zone_auto_detect", json_object_new_boolean(zone_auto_detect));

	json_object_object_add(jobj, "timezone", jobj_timezone);
}

int json_set_sys_timezone(ResponseEntry *rep, char *query_str, char *json_reply)
{
	int i, length = T_NUM_OF_ELEMENTS(api_timezone_table), ret;
	bool time_zone_auto_detect = false;
	char *TimezoneSettings = NULL;
	char *zonename = NULL;

	char cmd[MAX_JSON_REPLY_LEN] = {0};
	char buf[256] = {0};

	ResponseStatus *res = rep->res;
	struct json_object *jobj = NULL, *jobj_time_zone = NULL;
#if 0
	if (NULL != query_str)
	{
		if ((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_and_create_string(rep, jobj, "timezone", &TimezoneSettings);

			if(jobj_time_zone = jsonTokenerParseFromStack(rep, TimezoneSettings))
			{
				senao_json_object_get_and_create_string(rep, jobj_time_zone, "zonename", &zonename);
				debug_print("--- JSON DEBUG %s[%d] zonename: #.%s.#\n", __FUNCTION__, __LINE__, zonename);
// api_timezone_table
// timezone / zonename / autotimezonename
/*
{ "UTC3", "UTC-03:00 Guyana", "America/Cayenne, America/Paramaribo" },
{ "UTC2", "UTC-02:00 Mid-Atlantic", "America/Miquelon, America/Noronha, Atlantic/South_Georgia" },
{ "STD1", "UTC-01:00 Azores", "Atlantic/Cape_Verde" },
{ "UTC0", "UTC+00:00 Gambia, Liberia, Morocco", "Africa/Abidjan, Africa/Accra, Africa/Bamako, Africa/Banjul, Africa/Bissau, Africa/Conakry, Africa/Dakar, Africa/Freetown, Africa/Lome, Africa/Monrovia, Africa/Nouakchott, Africa/Ouagadougou, Africa/Sao_Tome, Atlantic/Azores, Atlantic/St_Helena" },
{ "GMT0", "UTC+00:00 England", "America/Danmarkshavn, America/Scoresbysund, Atlantic/Reykjavik" },
{ "STD-1", "UTC+01:00 France, Germany, Italy", "Atlantic/Faroe, Atlantic/Madeira, Europe/Dublin, Europe/Guernsey, Europe/Isle_of_Man, Europe/Jersey, Europe/Lisbon, Europe/London"},
{ "STD-2", "UTC+02:00 Greece, Ukraine, Romania, Turkey", "Arctic/Longyearbyen, Europe/Amsterdam, Europe/Andorra, Europe/Belgrade, Europe/Berlin, Europe/Bratislava, Europe/Brussels, Europe/Budapest, Europe/Copenhagen, Europe/Gibraltar, Europe/Kaliningrad, Europe/Ljubljana, Europe/Luxembourg, Europe/Madrid, Europe/Malta, Europe/Monaco, Europe/Oslo, Europe/Paris, Europe/Podgorica, Europe/Prague, Europe/Rome, Europe/San_Marino, Europe/Sarajevo, Europe/Skopje, Europe/Stockholm, Europe/Tirane, Europe/Vaduz, Europe/Vatican, Europe/Vienna, Europe/Warsaw, Europe/Zagreb, Europe/Zurich" },
{ "UTC-3", "UTC+03:00 Iraq, Jordan, Kuwait", "Africa/Addis_Ababa, Africa/Asmara, Africa/Dar_es_Salaam, Africa/Djibouti, Africa/Kampala, Africa/Khartoum, Africa/Mogadishu, Africa/Nairobi, Asia/Aden, Asia/Baghdad, Asia/Bahrain, Asia/Beirut, Asia/Damascus, Asia/Gaza, Asia/Kuwait, Asia/Nicosia, Asia/Qatar, Asia/Riyadh, Indian/Antananarivo, Indian/Comoro, Indian/Mayotte" },
*/
				for(i = 0; i < length; i++)
				{
					// UTC+00:00 England from restfultimezone
					if (strcasecmp(api_timezone_table[i].zonename, zonename) == 0)
					{
						ret = API_RC_SUCCESS;

						// system.@system[0].zonename
						ret |= api_set_string_option(SYSTEM_SYSTEM_ZONENAME_OPTION, api_timezone_table[i].zonename, sizeof(api_timezone_table[i].zonename));

						// system.@system[0].timezone
						ret |= api_set_string_option(SYSTEM_SYSTEM_TIMEZONE_OPTION, api_timezone_table[i].timezone, sizeof(api_timezone_table[i].timezone));

						//snprintf(cmd, sizeof(cmd), "echo %s > /etc/TZ", api_timezone_table[i].timezone);
						//sys_interact_long(buf, sizeof(buf), cmd);

						// system.@system[0].zonenumber
						ret |= api_set_integer_option(SYSTEM_SYSTEM_ZONENUMBER_OPTION, i);

						debug_print("--- JSON DEBUG %s[%d] SYSTEM_SYSTEM_ZONENUMBER_OPTION %d\n", __FUNCTION__, __LINE__, i);

						if (ret != API_RC_SUCCESS)
							RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME ZONE");

						break;
					}
				}
				if (i == length)
					RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "TIME ZONE");
			}
		}
		api_set_integer_option("system.@system[0].zone_auto_detect", time_zone_auto_detect ? 1 : 0); // default is false
	}

	snprintf(cmd, sizeof(cmd), "uci commit system");
	sys_interact_long(buf, sizeof(buf), cmd);

	snprintf(cmd, sizeof(cmd), "luci-reload system");
	sys_interact_long(buf, sizeof(buf), cmd);
#endif
	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
