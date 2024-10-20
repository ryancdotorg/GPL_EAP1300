#include <api_common.h>
#include <sys_common.h>
#include <api_wireless.h>
#include <variable.h>
#include <api_lan.h>
#include <wireless_tokens.h>
//#include <integer_check.h>
#include <json_object.h>
#include <json_tokener.h>
#include <json_mgm.h>
#include <json_common.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

typedef struct trigger_day 
{
    char *day;
    char *counter;
} trigger_day;

trigger_day day_table[] = {
    { "SUN", "0" },
    { "MON", "1" },
    { "TUE", "2" },
    { "WED", "3" },
    { "THU", "4" },
    { "FRI", "5" },
    { "SAT", "6" }
};

int json_get_mgm_tools_iperf(ResponseEntry *rep, struct json_object *jobj)
{
    int upload_rate, upload_bytes, download_rate, download_bytes = 0;
    char buf[100] = {0};
    ResponseStatus *res = rep->res;

    sys_interact(buf, sizeof(buf), "ps -w | grep iperf3 |grep /tmp/diag_iperf |grep -v grep");
    if(strlen(buf) != 0)
    {
        RET_GEN_ERRORMSG(res, API_PROCESSING, "PROCESSING");
    }
    if (sys_check_file_existed("/tmp/diag_iperf"))
    {
        sys_interact(buf, sizeof(buf), "cat /tmp/diag_iperf |grep error");
        if(strlen(buf) != 0){
            RET_GEN_ERRORMSG(res, API_INTERNET_ERROR, "INTERNET ERROR"); 
        }
        sys_interact(buf, sizeof(buf), "cat /tmp/diag_iperf |grep sender |awk {'printf $7'}");
        upload_rate = atoi(buf);
        sys_interact(buf, sizeof(buf), "cat /tmp/diag_iperf |grep sender |awk {'printf $5'}");
        upload_bytes = atoi(buf) *1024*1024;
        sys_interact(buf, sizeof(buf), "cat /tmp/diag_iperf |grep receiver |awk {'printf $7'}");
        download_rate = atoi(buf);
        sys_interact(buf, sizeof(buf), "cat /tmp/diag_iperf |grep receiver |awk {'printf $5'}");
        download_bytes = atoi(buf) *1024*1024;
        json_object_object_add(jobj, "upload_rate", json_object_new_int(upload_rate));
        json_object_object_add(jobj, "upload_bytes", json_object_new_int(upload_bytes));
        json_object_object_add(jobj, "download_rate", json_object_new_int(download_rate));
        json_object_object_add(jobj, "download_bytes", json_object_new_int(download_bytes));
    }
    else
    {

        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "NO iperf result");
    }


    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_mgm_tools_iperf(ResponseEntry *rep, char *query_str)
{
    struct json_object *jobj;
    char *dst_ip=NULL, *bandwidth_limit=NULL;
    char cmd[1000] = {0}, buf[100] = {0};
    int time_period, check_interval;
    bool direction_reverse = false;

    ResponseStatus *res = rep->res;

    sys_interact(buf, sizeof(buf), "ps -w | grep iperf3 |grep /tmp/diag_iperf |grep -v grep");
    if(strlen(buf) != 0)
    {
        RET_GEN_ERRORMSG(res, API_PROCESSING, "PROCESSING");
    }
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "dst_ip_addr", &dst_ip);
            senao_json_object_get_integer(jobj, "time_period", &time_period);
            senao_json_object_get_integer(jobj, "check_interval", &check_interval);
            senao_json_object_get_boolean(jobj, "direction_reverse", &direction_reverse);
            senao_json_object_get_and_create_string(rep, jobj, "bandwidth_limit", &bandwidth_limit);
        }
    }

    if (time_period < 1 || time_period > 9999)
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE,"TIME_PERIOD");
    }
    if (check_interval < 1 || check_interval > 60)
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE,"CHECK_INTERVAL");
    }

    system("rm /tmp/diag_iperf");

    if ( strcmp(bandwidth_limit, "") !=0)
    {
        sprintf(buf,"-b %s", bandwidth_limit);
    }
    if (direction_reverse == true) 
    {
        strcat(buf, " -R");
    }

    if(api_check_ip_addr(dst_ip)){
        sprintf(cmd,"iperf3 -f bytes -c %s -t %d -i %d -p 5201 %s --logfile /tmp/diag_iperf &", dst_ip, time_period, check_interval, buf);
    }
    else if(api_check_ipv6_addr(dst_ip) || strncmp(dst_ip, "fe80", strlen("fe80")) == 0){
        sprintf(cmd,"iperf3 -f bytes -c %s -t %d -i %d -p 60001 %s --logfile /tmp/diag_iperf &", dst_ip, time_period, check_interval, buf);
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "DST_IP");
    }
    system(cmd);

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
int json_get_mgm_backup_config(ResponseEntry *rep, struct json_object *jobj)
{
    char hostName[31+1], localTime[15+1], filePath[127+1],lanIP[32],type[10],http_filePath[255+1];
    char *dhcp_ip=NULL;
    ResponseStatus *res = rep->res;
    char path[128] ={0}, path_tmp[128] ={0}, path2[128] ={0};

    api_get_string_option("luci.main.mediaurlbase", path_tmp, sizeof(path_tmp));
    sprintf(path, "/www%s/backup_config.tar.gz", path_tmp);
    sprintf(path2, "%s/backup_config.tar.gz", path_tmp);

    memset(hostName,0x0, sizeof(hostName));
    memset(localTime,0x0, sizeof(localTime));
    memset(filePath,0x0, sizeof(filePath));
    memset(lanIP,0x0,sizeof(lanIP));
    memset(type,0x0,sizeof(type));

    if(!sys_check_file_existed(path))
    {
       RET_GEN_ERRORMSG(res, API_PROCESSING, "PROCESSING"); 
    }

    api_get_string_option(NETWORK_LAN_PROTO_OPTION,type,sizeof(type));
    
    if(!strcmp(type,"dhcp"))
    {
        sys_interact(lanIP, sizeof(lanIP), "cat /tmp/dhcp_addr |awk {'printf $2'}");
    }
    else
    {
        api_get_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION,lanIP,sizeof(lanIP));
    }

    strcat(lanIP, path2);
    json_object_object_add(jobj, "backup_file_path", json_object_new_string(path));
    json_object_object_add(jobj, "backup_file", json_object_new_string(lanIP));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_mgm_backup_config(ResponseEntry *rep, char *query_str)
{
    ResponseStatus *res = rep->res;
    struct json_object *jobj;
    char *backup_setting=NULL;
    char buf[50] = {0};
    char path[128] ={0}, path_tmp[128] ={0}, rm_path[128] ={0}, upgrade[128] ={0};

    api_get_string_option("luci.main.mediaurlbase", path_tmp, sizeof(path_tmp));
    sprintf(path, "/www%s/backup_config.tar.gz", path_tmp);
    sprintf(rm_path, "rm %s", path);
    sprintf(upgrade, "sysupgrade --create-backup %s", path);

    if(sys_check_file_existed(path))
    {
        system(rm_path);
    }

    system(upgrade);

/*
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "backup_setting", &backup_setting);
        }
    }

    if (strcmp(backup_setting, "user") == 0) 
    {
        system("sh /etc/cfg_user_save.sh");
    }
    else if (strcmp(backup_setting, "factory") == 0)
    {
        if(sys_check_file_existed("/www/luci-static/web_ECB_FANCY/backup_config.tar.gz"))
        {
            system("rm /www/luci-static/web_ECB_FANCY/backup_config.tar.gz");
        }

        system("sysupgrade --create-backup /www/luci-static/web_ECB_FANCY/backup_config.tar.gz");

    }
*/
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_mgm_restore_config(ResponseEntry *rep, char *query_str)
{
    ResponseStatus *res = rep->res;
    struct json_object *jobj;
    char *backup_setting=NULL;
    char buf[50] = {0};

#if SUPPORT_SYSTEM_LOG
    system("echo restore>/mnt/rebootType");
#endif

    system("sh /etc/cfgrestore.sh;sleep 1;reboot &");
/*
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "backup_setting", &backup_setting);
        }
    }

    if (strcmp(backup_setting, "user") == 0) 
    {
        system("sh /etc/cfg_user_restore.sh;sleep 1;reboot &");
    }
    else if (strcmp(backup_setting, "factory_setting") == 0)
    {
        system("sh /etc/cfgrestore.sh;sleep 1;reboot &");
    }
*/
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
int json_set_mgm_reboot(ResponseEntry *rep, char *query_str)
{
    ResponseStatus *res = rep->res;

#if SUPPORT_SYSTEM_LOG
    system("echo reboot>/mnt/rebootType");
#endif

    //system("reboot &");
	system("echo 'sleep 5; reboot &' > /tmp/reboot.sh; sh /tmp/reboot.sh &");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_mgm_auto_reboot_cfg(ResponseEntry *rep, char *query_str)
{
    bool enable = 0;
    char *time=NULL, day_list[50] = {0};
    struct json_object *jobj, *jarr, *jarr_obj;
    int i, j, length, match, hour, minute, jcount=0;
    char day[10][10];
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_boolean(jobj, "enable", &enable);
            if (enable == true) 
            {
                jarr = json_object_object_get(jobj, "trigger_day");
                jcount = json_object_array_length(jarr) ;
                for (i = 0; i < jcount; i++) 
                {
                    jarr_obj = json_object_array_get_idx(jarr, i);
                    sprintf(day[i], "%s", json_object_get_string(jarr_obj));
                }
                senao_json_object_get_and_create_string(rep, jobj, "trigger_time", &time);
            }
        }
    }
    if (enable == true) 
    {
        for (j = 0; j < jcount; j++)
        {   
            match = 0;
            for (i =0; i < 7; i++) 
            {
                if (strcmp(day[j], day_table[i].day) == 0) 
                {

                    strcat(day_list, day_table[i].counter);
                    if (j+1 < jcount) 
                    {
                        strcat(day_list, ",");
                    }
                    match = 1;
                }
            }
            if (match == 0) 
            {
                RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE,"TRIGGER_DAY");
            }
        }

        sscanf(time, "%d:%d", &hour, &minute);
         
        if ( hour < 0 || hour > 23 || minute < 0 || minute >59) 
        {
            RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE,"TRIGGER_TIME");
        }

        api_set_string_option(WIFI_SCHEDULE_REBOOT_DAY_OPTION, day_list, sizeof(day_list));
        api_set_string_option(WIFI_SCHEDULE_REBOOT_TIME_OPTION, time, sizeof(time));
    }

    api_set_bool_option(WIFI_SCHEDULE_REBOOT_STATUS_OPTION, enable);

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_mgm_auto_reboot_cfg(ResponseEntry *rep, struct json_object *jobj)
{
    int enable = 0;
    char time[10] = {0}, day_list[50] = {0}, day[10] ={0};
    struct json_object *jarr, *jarr_obj;
    int i, j, length, match, hour, minute, jcount=0;
    // char day[10][10];
    char *pch = NULL, *saveptr = NULL;
    char *week[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

    ResponseStatus *res = rep->res;

    api_get_string_option(WIFI_SCHEDULE_REBOOT_DAY_OPTION, day_list, sizeof(day_list));
    api_get_string_option(WIFI_SCHEDULE_REBOOT_TIME_OPTION, time, sizeof(time));
    api_get_integer_option(WIFI_SCHEDULE_REBOOT_STATUS_OPTION, &enable);

    jarr_obj = json_object_new_array();

    pch = strtok_r(day_list, ",", &saveptr);

    while(pch !=NULL)
    {
        strcpy(day, week[atoi(pch)]);

        json_object_array_add(jarr_obj,json_object_new_string(day));
        pch = strtok_r(NULL, ",", &saveptr);
    }


    //json_object_object_add(jarr_obj, "trigger_day", json_object_new_string("1"));
    //json_object_object_add(jarr_obj, "trigger_day", json_object_new_string("2"));

    json_object_object_add(jobj, "enable", json_object_new_boolean(enable));
    json_object_object_add(jobj, "trigger_day", jarr_obj);
    json_object_object_add(jobj, "trigger_time", json_object_new_string(time));


    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
int json_set_mgm_reset_to_default(ResponseEntry *rep, char *query_str)
{
    char reset_script[50] = {0}, buf[50] = {0};
    ResponseStatus *res = rep->res;

    api_get_string_option("functionlist.vendorlist.SUPPORT_RESET_FACTORY_SCRIPT", reset_script, sizeof(reset_script));

    if ( strlen(reset_script) > 0 )
    {
        sys_interact(buf, sizeof(buf), "echo 'sleep 5;sh %s' > /etc/reset_api.sh", reset_script);
        system("sh /etc/reset_api.sh &");
        //sys_interact(buf, sizeof(buf), "sh %s &", reset_script);
    }
    else
    {
        DIR* dir = opendir("/overlay/upper");
        if (dir)
        {
            closedir(dir);
            system("rm /overlay/upper/* -rf;reboot &");
        }
        else
        {
            system("rm /overlay/* -rf;reboot &");
        }
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_mgm_reset_with_key(ResponseEntry *rep, char *query_str)
{
    char reset_script[50] = {0}, buf[50] = {0}, *key = NULL, mac_md5[33] = {0};
    struct json_object *jobj;
    bool key_match = false;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "key", &key);
            if ( key != NULL && strlen(key) == 32 )
            {
                sys_interact(mac_md5, sizeof(mac_md5), "ifconfig br-lan | grep HWaddr | awk {'printf toupper($5)'} | md5sum | awk {'print $1'} | tr -d '\n'");

                if ( strcmp(key, mac_md5) == 0 )
                    key_match = true;
            }
        }
    }

    if (!key_match)
        RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "key");

    api_get_string_option("functionlist.vendorlist.SUPPORT_RESET_FACTORY_SCRIPT", reset_script, sizeof(reset_script));

    if ( strlen(reset_script) > 0 )
    {
        sys_interact(buf, sizeof(buf), "echo 'sleep 5;sh %s' > /etc/reset_api.sh", reset_script);
        system("sh /etc/reset_api.sh &");
        //sys_interact(buf, sizeof(buf), "sh %s &", reset_script);
    }
    else
    {
        DIR* dir = opendir("/overlay/upper");
        if (dir)
        {
            closedir(dir);
            system("rm /overlay/upper/* -rf;reboot &");
        }
        else
        {
            system("rm /overlay/* -rf;reboot &");
        }
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

#define PING_RESULT_FILE "/tmp/ping_result"
#define TRACEROUTE_RESULT_FILE "/tmp/traceroute_result"
#define NSLOOKUP_RESULT_FILE "/tmp/nslookup_result"
int json_mgm_ping(ResponseEntry *rep, char *query_str)
{
    char *dst_ip_addr=NULL, cmd[128]={'\0'};
    int packet_size=0, number_of_ping=0;
    struct json_object *jobj;
    int packet_size_max=20480, packet_size_min=64;
    int num_of_ping_max=200, num_of_ping_min=1;
    ResponseStatus *res = rep->res;

    api_get_integer_option("functionlist.vendorlist.PING_SIZE_MAX", &num_of_ping_max);
    api_get_integer_option("functionlist.vendorlist.PING_SIZE_MIN", &num_of_ping_min);
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "dst_ip_addr", &dst_ip_addr);
            senao_json_object_get_integer(jobj, "packet_size", &packet_size);
            senao_json_object_get_integer(jobj, "number_of_ping", &number_of_ping);
            if(packet_size < packet_size_min || packet_size > packet_size_max){
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "packet_size");
            }
            if(number_of_ping < num_of_ping_min || number_of_ping > num_of_ping_max){
                RET_GEN_ERRORMSG(res, API_INVALID_ARGUMENTS, "number_of_ping");
            }
            if(api_check_ip_addr(dst_ip_addr)){
                sprintf(cmd, "ping -c %d -W 1 -s %d -I br-lan %s 2>&1 >> %s &", number_of_ping, packet_size, dst_ip_addr, PING_RESULT_FILE);
            }
            else if(api_check_ipv6_addr(dst_ip_addr) || strncmp(dst_ip_addr, "fe80", strlen("fe80")) == 0){
                sprintf(cmd, "ping6 -c %d -W 1 -s %d  %s 2>&1 >> %s &", number_of_ping, packet_size, dst_ip_addr, PING_RESULT_FILE);
            }else{
                sprintf(cmd, "ping -c %d -W 1 -s %d -I br-lan %s 2>&1 >> %s &", number_of_ping, packet_size, dst_ip_addr, PING_RESULT_FILE);
            }
            unlink(PING_RESULT_FILE);
            system(cmd);
        }
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);

}

int json_get_mgm_led_list(ResponseEntry *rep, struct json_object *jobj)
{
    ResponseStatus *res = rep->res;
    char buf[256]={0};
    char *pch = NULL;
    char *saveptr = NULL;
    struct json_object *jarr_led_list;
    jarr_led_list = json_object_new_array();
    sys_interact(buf, sizeof(buf), "ls /sys/class/leds");

    if(strlen(buf) > 0) {

        pch = strtok_r(buf, "\n", &saveptr);
        while(pch != NULL)
        {
            json_object_array_add(jobj, json_object_new_string(pch));
            pch = strtok_r(NULL, "\n", &saveptr);
        }
    }else{
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "No LED available.");
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_mgm_led_status(ResponseEntry *rep, struct json_object *jobj)
{
    ResponseStatus *res = rep->res;
    char buf[256]={0}, buf2[128]={0};
    char *pch = NULL;
    char *saveptr = NULL;
    struct json_object *jobj_led_status;
    sys_interact(buf, sizeof(buf), "ls /sys/class/leds");

    if(strlen(buf) > 0) {

        pch = strtok_r(buf, "\n", &saveptr);
        while(pch != NULL)
        {
            jobj_led_status = json_object_new_object();

            json_object_object_add(jobj_led_status, "led", json_object_new_string(pch));
            sys_interact(buf2, sizeof(buf2), "cat /sys/class/leds/%s/trigger | awk -F] {'print $1'} | awk -F[ {'printf $2'}", pch);
            if(strcmp(buf2, "timer") == 0)
            {
                json_object_object_add(jobj_led_status, "action", json_object_new_string("blinking"));
                sys_interact(buf2, sizeof(buf2), "cat /sys/class/leds/%s/delay_on", pch);
                json_object_object_add(jobj_led_status, "blinking_on", json_object_new_int(atoi(buf2)));
                sys_interact(buf2, sizeof(buf2), "cat /sys/class/leds/%s/delay_off", pch);
                json_object_object_add(jobj_led_status, "blinking_off", json_object_new_int(atoi(buf2)));
            }
            else
            {
                sys_interact(buf2, sizeof(buf2), "cat /sys/class/leds/%s/brightness", pch);
                json_object_object_add(jobj_led_status, "action", json_object_new_string(atoi(buf2) > 0 ? "on" : "off"));

                json_object_object_add(jobj_led_status, "blinking_on", json_object_new_int(0));
                json_object_object_add(jobj_led_status, "blinking_off", json_object_new_int(0));
            }

            json_object_array_add(jobj, jobj_led_status);
            pch = strtok_r(NULL, "\n", &saveptr);
        }
    }else{
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "No LED available.");
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

#define CHANGE_LED_SCRIPT "/usr/sbin/change_led.sh"
int json_set_mgm_led_cfg(ResponseEntry *rep, char *query_str)
{
	char path[128]={0}, cmd[128]={0}, script_cmd[128]={0};
	struct json_object *jobj;
	char *led=NULL, *trigger=NULL;
	bool brightness=false;
	int delay_on=0, delay_off=0;
	ResponseStatus *res = rep->res;

	if(NULL != query_str)
	{
		if((jobj = jsonTokenerParseFromStack(rep, query_str)))
		{
			senao_json_object_get_and_create_string(rep, jobj, "led", &led);
			senao_json_object_get_and_create_string(rep, jobj, "action", &trigger);
			senao_json_object_get_integer(jobj, "blinking_on", &delay_on);
			senao_json_object_get_integer(jobj, "blinking_off", &delay_off);

			debug_print("%s %s delay %d %d brightness %d\n", led, trigger, delay_on, delay_off, brightness);

			if(access(CHANGE_LED_SCRIPT, F_OK) != -1)
			{
				sprintf(script_cmd, "sh %s %s %s %d %d", CHANGE_LED_SCRIPT, led, trigger, delay_on, delay_off);
				system(script_cmd);
			}
			else
			{
				sprintf(path, "/sys/class/leds/%s", led);
				if(access(path, F_OK) != -1){
					if(strcmp(trigger, "blinking") == 0)
					{
						sprintf(cmd, "echo timer > %s/trigger", path);
						system(cmd);

						if(access(path, F_OK) != -1){
							sprintf(cmd, "echo %d > %s/delay_on", delay_on, path);
							system(cmd);
							sprintf(cmd, "echo %d > %s/delay_off", delay_off, path);
							system(cmd);
						}
					}
					else
					{
						sprintf(cmd, "echo none > %s/trigger", path);
						system(cmd);

						if(strcmp(trigger, "on")==0)
						{
							sprintf(cmd, "echo `cat %s/max_brightness` > %s/brightness", path, path);
						}
						else
						{
							sprintf(cmd, "echo 0 > %s/brightness", path);
						}
						system(cmd);
					}
				}else{
					sprintf(cmd, "Target led [%s] not avaiable.", led);
					RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, cmd);
				}
			}
	}else{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "Cannot get data from query string.");
	}
	}else{
		RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "No query string.");
	}

	RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_mgm_ping(ResponseEntry *rep, struct json_object *jobj)
{
    char buf[1024]={0};
    unsigned long status = 0;
    ResponseStatus *res = rep->res;

    sys_interact(buf, sizeof(buf), "pgrep -o ping 2>&1");
    status = atol(buf);
    memset(buf, 0, sizeof(buf));
    if(status > 0 ){
       RET_GEN_ERRORMSG(res, API_PROCESSING, "PROCESSING"); 
    }
    if(access(PING_RESULT_FILE, F_OK) != -1){
        sys_interact(buf, sizeof(buf), "cat %s 2>&1", PING_RESULT_FILE);
        json_object_object_add(jobj, "result", json_object_new_string(buf));
    }else{
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "NO ping result");
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_mgm_traceroute(ResponseEntry *rep, char *query_str)
{
    char *dst_ip_addr=NULL, cmd[128]={'\0'}, buf[64]={0};
    unsigned long status = 0;
    struct json_object *jobj;
    ResponseStatus *res = rep->res;

    sys_interact(buf, sizeof(buf), "pgrep -o traceroute 2>&1");
    status = atol(buf);
    memset(buf, 0, sizeof(buf));
    if(status > 0 ){
       RET_GEN_ERRORMSG(res, API_PROCESSING, "PROCESSING");
    }
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "dst_ip_addr", &dst_ip_addr);
            if(api_check_ip_addr(dst_ip_addr)){
                sprintf(cmd, "traceroute -q 1 -w 1 -n %s 2>&1 >> %s &", dst_ip_addr, TRACEROUTE_RESULT_FILE);
            }
            else if(api_check_ipv6_addr(dst_ip_addr)){
                sprintf(cmd, "traceroute6 -q 1 -w 2 -n %s  2>&1 >> %s &", dst_ip_addr, TRACEROUTE_RESULT_FILE);
            }
            else if(strncmp(dst_ip_addr,"fe80", strlen("fe80")) == 0){
                sprintf(cmd, "traceroute6 -q 1 -w 2 -n %s -i br-lan 2>&1 >> %s &", dst_ip_addr, TRACEROUTE_RESULT_FILE);
            }else{
                sprintf(cmd, "traceroute -q 1 -w 1 -n %s 2>&1 >> %s &", dst_ip_addr, TRACEROUTE_RESULT_FILE);
            }
            unlink(TRACEROUTE_RESULT_FILE);
            system(cmd);
        }
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);

}

int json_get_mgm_traceroute(ResponseEntry *rep, struct json_object *jobj)
{
    char buf[1024]={0};
    unsigned long status = 0;
    ResponseStatus *res = rep->res;

    sys_interact(buf, sizeof(buf), "pgrep -o traceroute 2>&1");
    status = atol(buf);
    memset(buf, 0, sizeof(buf));
    if(status > 0 ){
       RET_GEN_ERRORMSG(res, API_PROCESSING, "PROCESSING"); 
    }
    if(access(TRACEROUTE_RESULT_FILE, F_OK) != -1){
        sys_interact(buf, sizeof(buf), "cat %s 2>&1", TRACEROUTE_RESULT_FILE);
        json_object_object_add(jobj, "result", json_object_new_string(buf));
    }else{
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "No traceroute result");
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_mgm_nslookup(ResponseEntry *rep, char *query_str)
{
    char *dst_ip_addr=NULL, cmd[128]={'\0'};
    struct json_object *jobj;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "dst_ip_addr", &dst_ip_addr);
            if(strlen(dst_ip_addr) == 0){
                RET_GEN_ERRORMSG(res, API_INVALID_TOKEN, "dst_ip_addr");
            }
            unlink(NSLOOKUP_RESULT_FILE);
            sprintf(cmd, "nslookup  %s 2>&1 >> %s &", dst_ip_addr, NSLOOKUP_RESULT_FILE);
            system(cmd);
        }
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);

}

int json_get_mgm_nslookup(ResponseEntry *rep, struct json_object *jobj)
{
    char buf[1024]={0};
    unsigned long status = 0;
    ResponseStatus *res = rep->res;

    sys_interact(buf, sizeof(buf), "pgrep -o nslookup 2>&1");
    status = atol(buf);
    memset(buf, 0, sizeof(buf));
    if(status > 0 ){
       RET_GEN_ERRORMSG(res, API_PROCESSING, "PROCESSING"); 
    }
    if(access(NSLOOKUP_RESULT_FILE, F_OK) != -1){
        sys_interact(buf, sizeof(buf), "cat %s 2>&1", NSLOOKUP_RESULT_FILE);
        json_object_object_add(jobj, "result", json_object_new_string(buf));
    }else{
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "No nslookup result");
    }
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_mgm_device_discovery(ResponseEntry *rep, char *query_str)
{
    ResponseStatus *res = rep->res;
    system("rm /tmp/device_list;killall discover;/usr/sbin/discover &");
    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_mgm_device_discovery(ResponseEntry *rep, struct json_object *jobj)
{
    char buf[2048]={0}, buf2[2048]={0}, buf3[2048]={0}, buf4[2048]={0}, buf5[2048]={0} , buf6[2048]={0}, tmp[20]={0};
    int i, num;
    char *pch, *pch2, *pch3, *pch4, *pch5, *pch6;
    char *saveptr1, *saveptr2, *saveptr3, *saveptr4, *saveptr5 , *saveptr6;
    struct json_object *jarr_device, *jobj_device_info;
    jobj_device_info = json_object_new_object();
    unsigned long status = 0;
    ResponseStatus *res = rep->res;

    sys_interact(buf, sizeof(buf), "pgrep -o discover 2>&1");
    status = atol(buf);
    memset(buf, 0, sizeof(buf));
    if(status > 0 ){
       RET_GEN_ERRORMSG(res, API_PROCESSING, "PROCESSING"); 
    }

    if (sys_check_file_existed("/tmp/device_list"))
    {
        sys_interact(buf, sizeof(buf), "cat /tmp/device_list |tail -n +2|awk '{print $1}'");
        sys_interact(buf2, sizeof(buf2), "cat /tmp/device_list |tail -n +2|awk '{print $2}'");
        sys_interact(buf3, sizeof(buf3), "cat /tmp/device_list |tail -n +2|awk '{print $3}'");
        sys_interact(buf4, sizeof(buf4), "cat /tmp/device_list |tail -n +2|awk '{print $4}'");
        sys_interact(buf5, sizeof(buf5), "cat /tmp/device_list |tail -n +2|awk '{print $5}'");
        sys_interact(buf6, sizeof(buf6), "cat /tmp/device_list |tail -n +2|awk '{print $6}'");
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_INVALID_DATA_TYPE, "NO device discovery result");
    }
    pch = strtok_r(buf, "\n", &saveptr1);
    pch2 = strtok_r(buf2, "\n", &saveptr2);
    pch3 = strtok_r(buf3, "\n", &saveptr3);
    pch4 = strtok_r(buf4, "\n", &saveptr4);
    pch5 = strtok_r(buf5, "\n", &saveptr5);
    pch6 = strtok_r(buf6, "\n", &saveptr6);

    while (pch != NULL)
    {
        jobj_device_info = json_object_new_object();
        json_object_object_add(jobj_device_info, "device_name", json_object_new_string(pch));
        if ( strcmp(pch2, "WDS") == 0) 
        {
            strcpy(tmp, "");
            sprintf(tmp, "%s %s",pch2,pch3);

            json_object_object_add(jobj_device_info, "opmode", json_object_new_string(tmp));
            json_object_object_add(jobj_device_info, "ip_addr", json_object_new_string(pch4));
            json_object_object_add(jobj_device_info, "mac_addr", json_object_new_string(pch5));
            json_object_object_add(jobj_device_info, "fw_version", json_object_new_string(pch6));
            json_object_array_add(jobj, jobj_device_info);
        }
        else
        {
            json_object_object_add(jobj_device_info, "opmode", json_object_new_string(pch2));
            json_object_object_add(jobj_device_info, "ip_addr", json_object_new_string(pch3));
            json_object_object_add(jobj_device_info, "mac_addr", json_object_new_string(pch4));
            json_object_object_add(jobj_device_info, "fw_version", json_object_new_string(pch5));
            json_object_array_add(jobj, jobj_device_info);
        }
        pch = strtok_r(NULL, "\n", &saveptr1);
        pch2 = strtok_r(NULL, "\n", &saveptr2);
        pch3 = strtok_r(NULL, "\n", &saveptr3);
        pch4 = strtok_r(NULL, "\n", &saveptr4);
        pch5 = strtok_r(NULL, "\n", &saveptr5);
        pch6 = strtok_r(NULL, "\n", &saveptr6);
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_mgm_gps(ResponseEntry *rep, char *query_str)
{
    char *gps_usr_defined = NULL, *latitude = NULL, *longitude = NULL, cmd[128]={'\0'};
    bool gps_enable = false;
    struct json_object *jobj, *jobj_usr_defined;
    ResponseStatus *res = rep->res;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "gps_usr_defined", &gps_usr_defined);
        }
    }

    if (jobj_usr_defined = jsonTokenerParseFromStack(rep, gps_usr_defined))
    {
        senao_json_object_get_and_create_string(rep, jobj_usr_defined, "latitude", &latitude);
        senao_json_object_get_and_create_string(rep, jobj_usr_defined, "longitude", &longitude);
        api_set_string_option2(latitude, sizeof(latitude), "system.@system[0].gps_usr_def_latitude");
        api_set_string_option2(longitude, sizeof(longitude), "system.@system[0].gps_usr_def_longitude");
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_mgm_gps(ResponseEntry *rep, struct json_object *jobj)
{
    ResponseStatus *res = rep->res;

    char gps_log[2048] = {0}, longitude[32] = {0}, latitude[32] = {0};
    struct json_object *jobj_usr_defined;
    int gps_enable = 0;

    jobj_usr_defined = json_object_new_object();

    if(sys_check_file_existed("/tmp/gps_json.log"))
    {
        sys_interact(gps_log, sizeof(gps_log), "cat /tmp/gps_json.log |tr -d '\n'");

    }

    api_get_string_option("system.@system[0].gps_usr_def_longitude", longitude, sizeof (longitude));
    api_get_string_option("system.@system[0].gps_usr_def_latitude", latitude, sizeof (latitude));

    json_object_object_add(jobj_usr_defined, "longitude", json_object_new_string(longitude));
    json_object_object_add(jobj_usr_defined, "latitude", json_object_new_string(latitude));
    json_object_object_add(jobj, "gps_log", json_object_new_string(gps_log));
    json_object_object_add(jobj, "gps_usr_defined", jobj_usr_defined);

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_post_mgm_fw_check(ResponseEntry *rep, char *query_str)
{
    char buf[1024]={0}, cmd[256]={0};
    unsigned long status = 0;
    ResponseStatus *res = rep->res;
    struct json_object *jobj;

    memset(buf, 0, sizeof(buf));
    char *fw_url = NULL;

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_and_create_string(rep, jobj, "fw_url", &fw_url);
        }
    }

    sys_interact(buf, sizeof(buf), "ps |grep getLatestFWInfo |grep -v grep|awk '{print $1}'");

    if(strlen(buf))
    {
        status = atol(buf);
    }

    if(status > 0 ){
       RET_GEN_ERRORMSG(res, API_PROCESSING, "PROCESSING");
    }

    if(sys_check_file_existed("/sbin/getLatestFWInfo.sh"))
    {
        sprintf(cmd,"sh /sbin/getLatestFWInfo.sh %s &", fw_url);
        system(cmd);
    }

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_mgm_fw_check(ResponseEntry *rep, struct json_object *jobj)
{
    char buf[1024] = {0}, buf2[1024] = {0}, *model=NULL, *ver=NULL, *file_size=NULL, *release_date=NULL, *change_log=NULL, *sku=NULL, *comment=NULL, *type=NULL, *md5sum=NULL, *file_name=NULL;
    int id;
    unsigned long status = 0;
    ResponseStatus *res = rep->res;
    struct json_object *jobj_fw_info;

    memset(buf2, 0, sizeof(buf2));

    sys_interact(buf2, sizeof(buf2), "ps |grep getLatestFWInfo |grep -v grep|awk '{print $1}'");

    if(strlen(buf2))
    {
        status = atol(buf2);
    }

    if(status > 0 ){
       RET_GEN_ERRORMSG(res, API_PROCESSING, "PROCESSING");
    }

    if(sys_check_file_existed("/tmp/new_fw_info_ori"))
    {
        sys_interact(buf, sizeof(buf), "cat /tmp/new_fw_info_ori");
        if(strlen(buf) > 1)
        {
            if((jobj_fw_info = jsonTokenerParseFromStack(rep, buf)))
            {
                json_object_object_add(jobj, "FW_info", jobj_fw_info);
                RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
            }
        }
        else
        {
            RET_GEN_ERRORMSG(res, API_FILE_SIZE_NOT_EQUAL, NULL);
        }
    }
    else
    {
        //ret no internet
        //RET_GEN_ERRORMSG(res, API_INTERNET_ERROR, NULL);
        RET_GEN_ERRORMSG(res, API_FILE_NOT_EXIST, NULL);
    }
}

int json_post_mgm_fw_download(ResponseEntry *rep, char *query_str)
{
    char cmd[128]={0}, buf[1024]={0};
    struct json_object *jobj;
    int id=0;
    unsigned long status = 0;
    ResponseStatus *res = rep->res;
    char *fw_url = NULL;

    memset(buf, 0, sizeof(buf));
    sys_interact(buf, sizeof(buf), "ps |grep doDownloadFW |grep -v grep|awk '{print $1}'");
    if(strlen(buf))
    {
        status = atol(buf);
    }

    if(status > 0 ){
       RET_GEN_ERRORMSG(res, API_PROCESSING, "PROCESSING");
    }

    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_integer(jobj, "id", &id);
            senao_json_object_get_and_create_string(rep, jobj, "fw_url", &fw_url);
        }
    }

    if(sys_check_file_existed("/tmp/firmware.img"))
    {
        system("rm /www/firmware.img");
        system("rm /tmp/firmware.img");
    }

    if(sys_check_file_existed("/tmp/newfwmd5sum"))
    {
        if(sys_check_file_existed("/sbin/doDownloadFW.sh"))
        {
            sprintf(cmd,"sh /sbin/doDownloadFW.sh %d %s &", id, fw_url);
            system(cmd);
            RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
        }
    }
    else
    {
        RET_GEN_ERRORMSG(res, API_FILE_NOT_EXIST, NULL);
    }

    //error code
    //RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_get_mgm_fw_download(ResponseEntry *rep, struct json_object *jobj)
{
    char url[512]={0}, fw_md5_info[512]={0}, fw_md5_cur[512]={0},lanIP[32],buf[1024], type[16]={0};
    int id;
    unsigned long status = 0;
    ResponseStatus *res = rep->res;

    if(sys_check_file_existed("/tmp/newfwmd5sum"))

    {
        sys_interact(fw_md5_info, sizeof(fw_md5_info), "cat /tmp/newfwmd5sum| tr -d '\n'");
    }

    memset(type,0x0,sizeof(type));
    api_get_string_option(NETWORK_LAN_PROTO_OPTION,type,sizeof(type));

    if(!strcmp(type,"dhcp"))
    {
        sys_interact(lanIP, sizeof(lanIP), "cat /tmp/dhcp_addr |awk {'printf $2'}");
    }
    else
    {
        api_get_lan_ipaddr_option(NETWORK_LAN_IPADDR_OPTION,lanIP,sizeof(lanIP));
    }

    if(sys_check_file_existed("/tmp/firmware.img"))
    {
        sys_interact(fw_md5_cur, sizeof(fw_md5_cur), "md5sum /tmp/firmware.img |awk '{print $1}' | tr -d '\n'");

        if(strcmp(fw_md5_cur, fw_md5_info) == 0)
        {
            if(!sys_check_file_existed("/www/firmware.img"))
            {
                json_object_object_add(jobj, "url", json_object_new_string(url));
                RET_GEN_ERRORMSG(res, API_FILE_NOT_EXIST, NULL);
            }
            else
            {
                strcat(lanIP,"/firmware.img");
                json_object_object_add(jobj, "url", json_object_new_string(lanIP));
                json_object_object_add(jobj, "md5sum", json_object_new_string(fw_md5_cur));
                RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
            }
        }
        else
        {
            memset(buf, 0, sizeof(buf));
            sys_interact(buf, sizeof(buf), "ps |grep doDownloadFW |grep -v grep|awk '{print $1}'");
            if(strlen(buf))
            {
                status = atol(buf);
            }
            if(status > 0 )
            {
                RET_GEN_ERRORMSG(res, API_PROCESSING, "PROCESSING");
            }
            else
            {
                json_object_object_add(jobj, "url", json_object_new_string(url));
                RET_GEN_ERRORMSG(res,  API_FILE_MD5SUM_ERROR, NULL);
            }
        }
    }
    else
    {
        json_object_object_add(jobj, "url", json_object_new_string(url));
        RET_GEN_ERRORMSG(res, API_FILE_NOT_EXIST, NULL);
    }
}

#if SUPPORT_NETGEAR_FUNCTION
#define INSIGHT_ANGENT_FILE "/etc/insight/.insight_onboard"
int json_get_mgm_mode(ResponseEntry *rep, struct json_object *jobj)
{
    ResponseStatus *res = rep->res;
    int enable = 0;
    bool insight_mode, insight_onboard=false;


    api_get_integer_option("apcontroller.capwap.enable", &enable);

    insight_mode=(enable == 1?true:false);

    if(access(INSIGHT_ANGENT_FILE, F_OK) != -1)
        insight_onboard = true;
    
    json_object_object_add(jobj, "insight_mode", json_object_new_boolean(insight_mode));
    json_object_object_add(jobj, "insight_onboard", json_object_new_boolean(insight_onboard));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_mgm_mode(ResponseEntry *rep, char *query_str)
{
    ResponseStatus *res = rep->res;
    bool enable = 0;

    struct json_object *jobj;
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_boolean(jobj, "insight_mode", &enable);
        }
    }

    api_set_integer_option("apcontroller.capwap.enable", enable?1:0);
    system("sh /sbin/notify_insight_agent.sh");

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
#endif

int json_get_ssh_setting(ResponseEntry *rep, struct json_object *jobj)
{
    ResponseStatus *res = rep->res;
    int enable = 0;
    char status[8]={0};

    api_get_string_option(DROPBEAR_DROPBEAR_ENABLE_OPTION, status, sizeof(status));
    enable=((strcmp(status, "on")==0)?true:false);

    json_object_object_add(jobj, "ssh_enable", json_object_new_boolean(enable));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}

int json_set_ssh_setting(ResponseEntry *rep, char *query_str)
{
    ResponseStatus *res = rep->res;
    bool enable = 0;
    char status[8]={0};

    struct json_object *jobj;
    if(NULL != query_str)
    {
        if((jobj = jsonTokenerParseFromStack(rep, query_str)))
        {
            senao_json_object_get_boolean(jobj, "ssh_enable", &enable);
        }
    }

    api_set_string_option(DROPBEAR_DROPBEAR_ENABLE_OPTION, ((enable)?"on":"off"), sizeof(((enable)?"on":"off")));

    RET_GEN_ERRORMSG(res, API_SUCCESS, NULL);
}
