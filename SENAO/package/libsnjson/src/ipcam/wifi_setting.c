#include "common.h"
#include "wifi_setting.h"

int get_scan_ap_list_count()
{
	char cmd[128] = {0};
	char buf[128] = {0};
	int count;

	snprintf(cmd, sizeof(cmd), "wc -l < %s | awk {'printf $1'} | tr -d '\n'", SCAN_AP_LIST);
	sys_interact_long(buf, sizeof(buf), cmd);

	count = atoi(buf) - 1;
	debug_print("ap_list_count: %d\n", count);

	return count;
}

int read_one_line(char *line, FILE *fptr)
{
	int ch; // the return value from getc() is an int
	int count = 0;

	memset(line, 0, 512);

	while(1)
	{
		ch = getc(fptr);
		if(ch == EOF)
			return - 1;

		line[count] = ch;

		if(line[count] == '\n')
			break;

		count++;
	}
}

void disconnect_ap()
{
	SYSTEM("wpa_cli disconnect");
	SYSTEM("killall -9 wpa_supplicant");
	SYSTEM("rm -rf %s", WPA_SUPPLICANT_RUN);
	SYSTEM("killall -9 udhcpc");
}

void connect_ap(int useDefaultConf)
{
	FILE *fptr;
	char buf[256];

	disconnect_ap();

	if(useDefaultConf)
	{
		SYSTEM("wpa_supplicant -B -d -i wlan0 -D nl80211 -c %s", WPA_SUPPLICANT_DEFAULT_CONFIG_FILE);
		SYSTEM("sleep 5");
	}
	else
	{
		SYSTEM("wpa_supplicant -B -d -i wlan0 -D nl80211 -c %s", WPA_SUPPLICANT_CONFIG_FILE);
		debug_print("after wpa_supplicant...\n");
		SYSTEM("sleep 5");
		//SYSTEM("udhcpc -i wlan0"); system will get IP automatically
		//debug_print("udhcpc OK!\n");

		// disconnect eth0
		//SYSTEM("ifconfig eth0 0.0.0.0");
	}
}

void write_scan_ap_list_info()
{
	char cmd[128] = {0};
	char buf[128] = {0};

	unsigned int process;

	sys_find_proc("wpa_supplicant", &process);

	if(process > 0)
	{
		/*if(uci.get.wireless.enable == 0)
		{
			// wpa_supplicant -i wlan0 -c /etc/wpa_supp_conf
		}*/
	}
	else
	{
		connect_ap(1);
	}

	SYSTEM("wpa_cli -i %s scan", WLAN_G_DEV);
	SYSTEM("sleep 6");
	SYSTEM("wpa_cli -i %s scan_results > %s", WLAN_G_DEV, SCAN_AP_LIST);
}

int read_scan_ap_list_info(struct json_object *jobj)
{
	struct json_object* jobj_tmp;

	char cmd[256] = {0};
	char buf[256] = {0};

	FILE *fptr;

	int i, idx;
	int total_nums;

	char bssid[20] = {0};
	char essid[256] = {0};
	char signal_strength[5];
	int quality_percent = 0;
	char encStr[128];
	char frequency[10];

	int scan_count = 0;
	int found = 0;

	char line[512] = {0};
	int encList_size = sizeof(encList)/sizeof(encList[0]);

	total_nums = get_scan_ap_list_count();

	if(total_nums == 0)
	{
		return 0;
	}

	if ((fptr = fopen(SCAN_AP_LIST, "r")) == NULL)
	{
		debug_print("fopen failed.\n");
		return -1;
	}
/*
bssid / frequency / signal level / flags / ssid
88:dc:96:34:3b:3e	2412	-81	[ESS]	EnGenius343B3E
8e:dc:96:8b:13:b2	0	-63	[WPA2-PSK-CCMP][ESS]	SNWL
*/
	read_one_line(line, fptr); // read title

	for(i = 0; i < total_nums; i++)
	{
		scan_count = 0;
		found = 0;

		read_one_line(line, fptr);
		scan_count = sscanf(line, "%[^\t]%*1[\t]%[^\t]%*1[\t]%[^\t]%*1[\t]%[^\t]%*1[\t]%[^\n]", bssid, frequency, signal_strength, encStr, essid);

		// The cfg80211 wext compat layer assumes a signal range of -110 dBm to -40 dBm
		quality_percent = (atoi(signal_strength) + 110) * 10 / 7;

		// block unknown ssid
		if(scan_count != 5)
		{
			//sprintf(essid, "unknown");
			continue;
		}

		if((strlen(encStr) == 5) && (strcmp(encStr, "[ESS]") == 0)) // no encryption
		{
			idx = 0;
			found = 1;
		}
		else if((strlen(encStr) == 10) && (strcmp(encStr, "[WPS][ESS]") == 0)) // no encryption
		{
			idx = 1;
			found = 1;
		}
		else
		{
			for(idx = 2; idx < encList_size; idx++)
			{
				if(strstr(encStr, encList[idx].encString) != NULL)
				{
					found = 1;
					break;
				}
			}
		}

		jobj_tmp = json_object_new_object();
		json_object_object_add(jobj_tmp, "bssid", json_object_new_string(bssid));
		json_object_object_add(jobj_tmp, "ssid", json_object_new_string(essid));
		if(found)
		{
			json_object_object_add(jobj_tmp, "auth", json_object_new_string(encList[idx].auth));
			json_object_object_add(jobj_tmp, "enc", json_object_new_string(encList[idx].enc));
		}
		else
		{
			json_object_object_add(jobj_tmp, "auth", json_object_new_string("-"));
			json_object_object_add(jobj_tmp, "enc", json_object_new_string("-"));
		}
		json_object_object_add(jobj_tmp, "signal", json_object_new_int(quality_percent));
		json_object_array_add(jobj, jobj_tmp);
	}

	fclose(fptr);
	return 0;
}

int json_get_wifi_site_survey(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;

	struct json_object *jarr;
	struct json_object *json_node = NULL;

	write_scan_ap_list_info();

	jarr = newObjectArrayFromStack(rep);
	read_scan_ap_list_info(jarr);
	json_object_object_add(jobj, "site_survey_list", jarr);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int get_ap_profile_count()
{
	char cmd[256] = {0};
	char buf[256] = {0};

	// add a '$' at the end of pattern to match the end of the line
	snprintf(cmd, sizeof(cmd), "grep -w \"config ap-profile$\" -c %s", CONFIG_AP_PROFILE_LIST);
	sys_interact(buf, sizeof(buf), cmd);

	return atoi(buf);
}

int find_ap_profile_idx(char *bssid)
{
	char cmd[256] = {0};
	char buf[256] = {0};
	int i;
	int total_nums;

	total_nums = get_ap_profile_count();

	for(i = 0; i < total_nums; i++)
	{
		snprintf(cmd, sizeof(cmd), "uci get ap-profile-list.@ap-profile[%d].bssid | tr -d \"\\n\"", i);
		sys_interact(buf, sizeof(buf), cmd);

		if(strcmp(buf, bssid) == 0)
		{
			return i;
		}
	}

	return -1;
}

int json_get_wifi_ap_profile_list(ResponseEntry *rep, struct json_object *jobj, char *json_reply)
{
	ResponseStatus *res = rep->res;

	char cmd[1024] = {0};
	char buf[1024] = {0};

	char bssid[20] = {0};
	char ssid[256] = {0};
	char auth[17] = {0};
	char enc[12] = {0};
	char password[64] = {0};

	struct json_object *jarr_AP_table, *jobj_AP_table;
	jarr_AP_table = json_object_new_array();

	int total_nums, idx;

	total_nums = get_ap_profile_count();

	for(idx = 0; idx < total_nums; idx++)
	{
		snprintf(cmd, sizeof(cmd), "uci get ap-profile-list.@ap-profile[%d].bssid | tr -d \"\\n\"", idx);
		sys_interact(bssid, sizeof(bssid), cmd);

		snprintf(cmd, sizeof(cmd), "uci get ap-profile-list.@ap-profile[%d].ssid | tr -d \"\\n\"", idx);
		sys_interact(ssid, sizeof(ssid), cmd);

		snprintf(cmd, sizeof(cmd), "uci get ap-profile-list.@ap-profile[%d].auth | tr -d \"\\n\"", idx);
		sys_interact(auth, sizeof(auth), cmd);

		snprintf(cmd, sizeof(cmd), "uci get ap-profile-list.@ap-profile[%d].enc | tr -d \"\\n\"", idx);
		sys_interact(enc, sizeof(enc), cmd);

		snprintf(cmd, sizeof(cmd), "uci get ap-profile-list.@ap-profile[%d].password | tr -d \"\\n\"", idx);
		sys_interact(password, sizeof(password), cmd);

		jobj_AP_table = json_object_new_object();
		json_object_object_add(jobj_AP_table, "bssid", json_object_new_string(bssid));
		json_object_object_add(jobj_AP_table, "ssid", json_object_new_string(ssid));
		json_object_object_add(jobj_AP_table, "auth", json_object_new_string(auth));
		json_object_object_add(jobj_AP_table, "enc", json_object_new_string(enc));
		json_object_object_add(jobj_AP_table, "password", json_object_new_string(password));
		json_object_array_add(jarr_AP_table, jobj_AP_table);

		debug_print("idx: %d, bssid: %s.\n", idx, bssid);
		debug_print("idx: %d, ssid: %s.\n", idx, ssid);
		debug_print("idx: %d, auth: %s.\n", idx, auth);
		debug_print("idx: %d, enc: %s.\n", idx, enc);
		debug_print("idx: %d, password: %s.\n", idx, password);
	}

	json_object_object_add(jobj, "ap_profile_list", jarr_AP_table);

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

void remove_ap_profile_from_list(int idx)
{
	char cmd[256] = {0};
	char buf[256] = {0};

	snprintf(cmd, sizeof(cmd), "uci delete ap-profile-list.@ap-profile[%d]", idx);
	sys_interact(buf, sizeof(buf), cmd);
}

int adjust_ap_profile_order(int idx, char *action)
{
	char cmd[256] = {0};
	char buf[256] = {0};

	int total_nums;

	if(strcmp(action, "up") == 0)
	{
		if(idx == 0)
			return -5;
		snprintf(cmd, sizeof(cmd), "uci reorder ap-profile-list.@ap-profile[%d]=%d", idx, idx-1);
		sys_interact(buf, sizeof(buf), cmd);
	}
	else if(strcmp(action, "down") == 0)
	{
		total_nums = get_ap_profile_count();

		if(idx == (total_nums-1))
			return -5;
		snprintf(cmd, sizeof(cmd), "uci reorder ap-profile-list.@ap-profile[%d]=%d", idx, idx+1);
		sys_interact(buf, sizeof(buf), cmd);
	}

	return 0;
}

int set_ap_profile(char *action, char *ssid, char *bssid, char *auth, char *enc, char *password)
{
	char cmd[1024] = {0};
	char buf[256] = {0};

	int idx, total_nums;
	int result;

	idx = find_ap_profile_idx(bssid);
	debug_print("Jason DEBUG %s[%d] bssid: %s, idx: %d\n", __FUNCTION__, __LINE__, bssid, idx);

	if(strcmp(action, "add") == 0)
	{
		total_nums = get_ap_profile_count();

		if(total_nums == 8) // full
			return -2;

		if(idx != -1) // duplicate case -> just update
		{
			debug_print("duplicate: %d.\n", idx);
		}
		else // normal case -> add ap profile at last postion of list
		{
			snprintf(cmd, sizeof(cmd), "uci add ap-profile-list ap-profile");
			sys_interact(buf, sizeof(buf), cmd);
			idx = total_nums;
			debug_print("add: %d, bssid %s.\n", idx, bssid);
		}

		snprintf(cmd, sizeof(cmd), "uci set ap-profile-list.@ap-profile[%d].ssid=\"%s\"", idx, ssid);
		sys_interact(buf, sizeof(buf), cmd);

		snprintf(cmd, sizeof(cmd), "uci set ap-profile-list.@ap-profile[%d].bssid=%s", idx, bssid);
		sys_interact(buf, sizeof(buf), cmd);

		snprintf(cmd, sizeof(cmd), "uci set ap-profile-list.@ap-profile[%d].auth=%s", idx, auth);
		sys_interact(buf, sizeof(buf), cmd);

		snprintf(cmd, sizeof(cmd), "uci set ap-profile-list.@ap-profile[%d].enc=%s", idx, enc);
		sys_interact(buf, sizeof(buf), cmd);

		// uci does not allow empty string as an option value
		snprintf(cmd, sizeof(cmd), "uci set ap-profile-list.@ap-profile[%d].password=\"%s\"", idx, password);
		sys_interact(buf, sizeof(buf), cmd);
	}
	else if(strcmp(action, "remove") == 0)
	{
		if(idx != -1)
		{
			debug_print("remove: %d, bssid %s.\n", idx, bssid);
			remove_ap_profile_from_list(idx);
		}
		else
		{
			return -3;
		}
	}
	else if(strcmp(action, "up") == 0)
	{
		if(idx != -1)
		{
			debug_print("up: %d, bssid %s.\n", idx, bssid);
			result = adjust_ap_profile_order(idx, "up");
			if(result != 0)
				return result;
		}
		else
		{
			return -3;
		}
	}
	else if(strcmp(action, "down") == 0)
	{
		if(idx != -1)
		{
			debug_print("down: %d, bssid %s.\n", idx, bssid);
			result = adjust_ap_profile_order(idx, "down");
			if(result != 0)
				return result;
		}
		else
		{
			return -3;
		}
	}
	else
	{
		return -4;
	}

	snprintf(cmd, sizeof(cmd), "uci commit ap-profile-list");
	sys_interact(buf, sizeof(buf), cmd);

	return 0;
}

int json_set_wifi_ap_profile(ResponseEntry *rep, char *query_str, char *json_reply)
{
	char *action = NULL;
	char *ssid = NULL;
	char *bssid = NULL;
	char *auth = NULL;
	char *enc = NULL;
	char *password = NULL;

	int status;

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			//senao_json_object_get_integer(jobj, "op", &bind);
			senao_json_object_get_and_create_string(rep, jobj, "action", &action);
			senao_json_object_get_and_create_string(rep, jobj, "ssid", &ssid);
			senao_json_object_get_and_create_string(rep, jobj, "bssid", &bssid);
			senao_json_object_get_and_create_string(rep, jobj, "auth", &auth);
			senao_json_object_get_and_create_string(rep, jobj, "enc", &enc);
			senao_json_object_get_and_create_string(rep, jobj, "password", &password);

			status = set_ap_profile(action, ssid, bssid, auth, enc, password);

			if(status == -1)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
			}
			else if(status == -2)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "The number of AP profiles should be smaller than 8.");
			}
			else if(status == -3)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSSID no found.");
			}
			else if(status == -4)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "This action is not supported.");
			}
			else if(status == -5)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "No change.");
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

int write_wpasupp_config_body(int idx, int priority, FILE *fptr)
{
	int len;

	char ssid[256] = {0};
	char auth[17] = {0};
	char enc[12] = {0};
	char password[64] = {0};

	char cmd[1024] = {0};
	char buf[256] = {0};

	snprintf(cmd, sizeof(cmd), "uci get ap-profile-list.@ap-profile[%d].ssid | tr -d \"\\n\"", idx);
	sys_interact(ssid, sizeof(ssid), cmd);

	snprintf(cmd, sizeof(cmd), "uci get ap-profile-list.@ap-profile[%d].auth | tr -d \"\\n\"", idx);
	sys_interact(auth, sizeof(auth), cmd);

	snprintf(cmd, sizeof(cmd), "uci get ap-profile-list.@ap-profile[%d].enc | tr -d \"\\n\"", idx);
	sys_interact(enc, sizeof(enc), cmd);

	snprintf(cmd, sizeof(cmd), "uci get ap-profile-list.@ap-profile[%d].password | tr -d \"\\n\"", idx);
	sys_interact(password, sizeof(password), cmd);

	debug_print("idx: %d, ssid: %s.\n", idx, ssid);
	debug_print("idx: %d, auth: %s.\n", idx, auth);
	debug_print("idx: %d, enc: %s.\n", idx, enc);
	debug_print("idx: %d, password: %s.\n", idx, password);

	if(strcmp(auth, "OPEN") == 0)
	{
		fprintf(fptr, WPASUPP_CONFIG_NONE_ENC, ssid, priority); //"EnGenius343B3E"
	}
	else if(strcmp(auth, "WPA-PSK") == 0 || strcmp(auth, "WPA2-PSK") == 0 || strcmp(auth, "WPA-PSK/WPA2-PSK") == 0)
	{
		fprintf(fptr, WPASUPP_CONFIG_WPA_PSK, ssid, password, priority);
	}
	else if(strcmp(auth, "WEP") == 0)
	{
		len = strlen(password);

		if(len == 5 || len == 13) // ASCII
			fprintf(fptr, WPASUPP_CONFIG_WEP_ASCII, ssid, password, priority);
		else if(len == 10 || len == 26) // HEX
			fprintf(fptr, WPASUPP_CONFIG_WEP_HEX, ssid, password, priority);
		else
			return -4;
	}
	else
	{
		cprintf("write_wpasupp_config_body - no support");
	}
}

int set_ap_connection(char *action, char *bssid)
{
	int i, idx, total_nums;

	FILE *fptr;
	int priority = 20;

	if(strcmp(action, "connect") == 0)
	{
		idx = find_ap_profile_idx(bssid);
		debug_print("Jason DEBUG %s[%d] bssid: %s, idx: %d\n", __FUNCTION__, __LINE__, bssid, idx);

		if(idx == -1)
		{
			return -2;
		}

		// write wpa config

		if ((fptr = fopen(WPA_SUPPLICANT_CONFIG_FILE, "w")) == NULL)
		{
			printf("fopen failed.");
			return -1;
		}

		fprintf(fptr, WPASUPP_CONFIG_HEAD, WPA_SUPPLICANT_RUN);

		// write first AP config bady
		write_wpasupp_config_body(idx, priority, fptr);
		priority--;

		total_nums = get_ap_profile_count();

		// write the other AP config bady
		for(i = 0; i < total_nums; i++)
		{
			if(i != idx)
			{
				write_wpasupp_config_body(i, priority, fptr);
				priority--;
			}
		}

		fclose(fptr);

		connect_ap(0);
	}
	else if(strcmp(action, "disconnect") == 0)
	{
		disconnect_ap();
	}
	else
	{
		return -3;
	}

	return 0;
}

// operation for specified AP that already existed in the AP profile list
int json_set_wifi_ap_connection(ResponseEntry *rep, char *query_str, char *json_reply)
{
	char *action = NULL;
	char *bssid = NULL;

	int status;

	struct json_object *jobj = NULL;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			//senao_json_object_get_integer(jobj, "op", &bind);
			senao_json_object_get_and_create_string(rep, jobj, "action", &action);
			senao_json_object_get_and_create_string(rep, jobj, "bssid", &bssid);

			status = set_ap_connection(action, bssid);

			if(status == -1)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "JSON");
			}
			else if(status == -2)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "BSSID no found.");
			}
			else if(status == -3)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "This action is not supported.");
			}
			else if(status == -4)
			{
				RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "WEP key length error.");
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
