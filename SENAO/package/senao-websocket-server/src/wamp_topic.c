#include <utility/sys_common.h>
#include <wamp.h>
#include <variable/api_ipv6.h>

#include <libseipc.h>
#define APP_NAME "websocket"
#define SYSEYE_PATH "/tmp/syseye.unix"

typedef struct
{
	unsigned int total;
	unsigned int used;
} memoryInfo;

int getMemoryInfo(memoryInfo *memInfo)
{
    char buf[256]={0}, *c;
    FILE *fp;

    fp = fopen("/proc/meminfo", "r");
    if(!fp)
    {
        return false;
    }

    while(fgets(buf, sizeof(buf), fp) != NULL)
    {
        if(strstr(buf,"MemTotal:"))
        {
            /* MemTotal */
            c = strchr(buf, ':');
            if(!c)
            {
                goto error_exit;
            }

            do
            {
                c++;
            }
            while(!isdigit(*c));

            memInfo->total = strtoul(c, NULL, 10);
        }
        else if(strstr(buf,"MemFree:"))
        {
            /* MemFree */
            c = strchr(buf, ':');
            if(!c)
            {
                goto error_exit;
            }

            do
            {
                c++;
            }
            while(!isdigit(*c));

            memInfo->used = memInfo->total - strtoul(c, NULL, 10);
        }
        else if(strstr(buf,"Buffers:"))
        {
            /* Buffers */
            c = strchr(buf, ':');
            if(!c)
            {
                goto error_exit;
            }

            do
            {
                c++;
            }
            while(!isdigit(*c));

            memInfo->used -= strtoul(c, NULL, 10);
        }
        else if(strstr(buf,"Cached:"))
        {
            /* Cached */
            c = strchr(buf, ':');
            if(!c)
            {
                goto error_exit;
            }

            do
            {
                c++;
            }
            while(!isdigit(*c));

            memInfo->used -= strtoul(c, NULL, 10);
        }
    }

    fclose(fp);
    return true;

error_exit:
    fclose(fp);
    return false;
}

int get_mem_usage(WAMP_Node_t *wNode, char *info)
{
	int mem_usage=0;
	memoryInfo memInfo;
	float used=0,total=0;

	if (getMemoryInfo(&memInfo)==false)
		mem_usage = 0;
	else
	{
		used=memInfo.used;
		total=memInfo.total;
		mem_usage = (used/total)*100;
	}

	sprintf(info, "{ \"memory\": \"%d\" }", mem_usage);

	return SUCCESS;
}

int get_cpu_usage(WAMP_Node_t *wNode, char *info)
{
	char buf[5+1]={0};
	int cpu_idle = 50;

	if (sys_interact(buf, sizeof(buf), "mpstat -P ALL | sed -n '4p' | awk '{print $11}'") > 0)
		sscanf(buf, "%d%%", &cpu_idle);

	sprintf(info, "{ \"cpu\": \"%d\" }", (100-cpu_idle));

	return SUCCESS;
}

int get_ping_result(WAMP_Node_t *wNode, char *info)
{
	char *dst_ip_addr, cmd[1024]={'\0'}, buf[128]={0};
	int packet_size=0, number_of_ping=0, num = 0, len, i = 0, use_domain = 0;
	struct json_object *jobj;
	char tmp[12][64] = {0}, *pch = NULL, *saveptr = NULL, *sp = NULL, *ep = NULL, *delim = " =";
	int packet_size_max=20480, packet_size_min=64, num_of_ping_max=200, num_of_ping_min=1;
	JsonNode *obj_para;

	num = SHOW_PING_COUNT(wNode->interval) >> 16;
	wNode->interval = SHOW_PURE_INTERVAL(wNode->interval);

	if(NULL != wNode->para)
	{
		if(obj_para = js_parse_str(wNode->para))
		{
			dst_ip_addr = js_get_path_str(obj_para, "dst_ip_addr");
			packet_size = js_get_path_int(obj_para, "packet_size");
			number_of_ping = js_get_path_int(obj_para, "number_of_ping");

			if(packet_size < packet_size_min || packet_size > packet_size_max)
				return FAIL;
			if(number_of_ping < num_of_ping_min || number_of_ping > num_of_ping_max)
				return FAIL;

			memset(cmd, 0, sizeof(cmd));
			if(num < number_of_ping) {
				if(api_check_ip_addr(dst_ip_addr)){
					sprintf(cmd, "ping -c 1 -W 1 -s %d -I br-lan %s | sed '2!d' | tr -d '\r\n'", packet_size, dst_ip_addr);
				}
				else if(api_check_ipv6_addr(dst_ip_addr) || strncmp(dst_ip_addr, "fe80", strlen("fe80")) == 0){
					sprintf(cmd, "ping6 -c 1 -W 1 -s %d %s | sed '2!d' | tr -d '\r\n'", packet_size, dst_ip_addr);
				}
				else if(api_check_domain_name(dst_ip_addr)){
					use_domain = 1;
					sprintf(cmd, "ping -c 1 -W 1 -s %d -I br-lan %s | sed '2!d' | tr -d '\r\n'", packet_size, dst_ip_addr);
				}
				else{
					return FAIL;
				}
				memset(buf, 0, sizeof(buf));
				sys_interact(buf, sizeof(buf), cmd);
				pch = strtok_r(buf, delim, &saveptr);
				while (pch != NULL )
				{
					sprintf(tmp[i], "%s", pch);
					pch = strtok_r(NULL, delim, &saveptr);
					i++;
				}
				if(strlen(tmp[0]) != 0 && strcmp(tmp[0],"From")!=0){
                                    	if (strcmp(tmp[10], "ms") == 0) 
                                        	use_domain = 0;
					sp = ep = (use_domain == 1) ? tmp[4] : tmp[3];
					while(ep != NULL) {
						if (*ep == '(')
							sp++;
						if(*ep == ':' || *ep == ')'){
							*ep = '\0';
							break;
						}
						ep++;
					}
					if(use_domain)
						sprintf(info, "{ \"seq\": %d, \"ip\": \"%s\", \"ttl\": \"%s\", \"packet_size\": \"%s\", \"time\": \"%s\" }", num + 1, sp, tmp[8], tmp[0], tmp[10]);
					else
						sprintf(info, "{ \"seq\": %d, \"ip\": \"%s\", \"ttl\": \"%s\", \"packet_size\": \"%s\", \"time\": \"%s\" }", num + 1, sp, tmp[7], tmp[0], tmp[9]);
				}
				else{
					sprintf(info, "{ \"seq\": %d, \"ip\": \"%s\", \"ttl\": \"\", \"packet_size\": \"\", \"time\": \"\" }", num + 1, dst_ip_addr);
				}
				num++;
			}
			else {
				num = 0;
				len = strlen(wNode->para) + 1;
				memset(wNode->para, 0, len);
			}
			wNode->interval = (wNode->interval | (num << 16));
			free(obj_para);
		}
	}

	return SUCCESS;
}

int get_station_list(WAMP_Node_t *wNode, char *info)
{
	struct seipc_t *handle=NULL;
	char *mac_addr, *ifname, *assoctime, *ipaddr, *os, *devName, *result_str;
	JsonNode *obj_para = NULL, *obj_recv = NULL, *obj_client = NULL;
	JsonNode *obj_ssid, *obj_traffic, *result_ssid = NULL;
	int radio = 0, ssid = 0, txrate = 0, rxrate = 0, rssi = 0, bridge = 0, ssid_id = 0;
	double up, down;
	char path[128];
	char buf[8];
	JsonNode *result = js_parse_str("{}");

	handle = seipc_create(APP_NAME, SYSEYE_PATH);

	if (!handle) {
		fprintf(stderr, "Cannot create seipc handle. Exit!\n");
		return -1;
	}

	if(NULL != wNode->para){
		if(obj_para = js_parse_str(wNode->para))
		{
			//js_print_hr(obj_para);
			radio = js_get_path_int(obj_para, "radio"); /* radio  0 : all radio, 1 : 2_4G, 2 : 5G, 4 : 5G-2 */
			ssid = js_get_path_int(obj_para, "ssid");  /* ssid   0 : all ssid, 1 : ssid 1  ~  8 : ssid 8 */
		}
	}

	/* get info from syseye */
	snprintf(path, sizeof(path), "act_ws_wifi_clinfo %d %d", radio, ssid);
	obj_recv = seipc_action(handle, "act_ws_wifi_clinfo", path);

    if(obj_recv != NULL) { 
        /* js_print_hr(obj_recv); */
        json_foreach(obj_client, js_get_path(obj_recv, "clients")){
            mac_addr = js_get_path_str(obj_client, "mac_addr");
            json_foreach(obj_ssid, js_get_path(obj_client, "ssids")){
                ifname = js_get_path_str(obj_ssid, "ifname");
                ssid_id = js_get_path_int(obj_ssid, "ssid_id");

                assoctime = js_get_path_str(obj_ssid,"assoctime");
                bridge = js_get_path_int(obj_ssid,"bridge");
                if((assoctime != NULL) || (bridge == 1)) {          /* only send current data || bridge mode */
                    up = js_get_path_double(obj_ssid,"up");
                    down = js_get_path_double(obj_ssid,"down");
                    txrate = js_get_path_int(obj_ssid,"txrate");
                    rxrate = js_get_path_int(obj_ssid,"rxrate");
                    rssi = js_get_path_int(obj_ssid,"rssi");
                    radio = js_get_path_int(obj_ssid,"radio");
                    ipaddr = js_get_path_str(obj_ssid,"ipaddr");
                    os = js_get_path_str(obj_ssid,"os");
                    devName = js_get_path_str(obj_ssid,"devName");
                    debug_print("[SWD][DEBUG] mac_addr:%s ifname:%s ssid_id:%d txrate:%d MB rxrate:%d MB up:%f MB down:%f MB rssi:%d assoctime:%s \n", mac_addr, ifname, ssid_id, txrate, rxrate, ((up/1024)/1024), ((down/1024)/1024), rssi, assoctime);
                    debug_print("[SWD][DEBUG] ipaddr:[%s] os:[%s] devName:[%s] \n", ipaddr, os, devName);

                    /* return Json */
                    snprintf(path, sizeof(path), "%s[mac_addr=\"%s\"]", (radio==0) ? "2_4G" : (radio==1) ? "5G" : "5G-2" , mac_addr);
                    result_ssid = js_set_path(result, path);
                    js_set_path_int(result_ssid, "ssid_id", ssid_id);
                    js_set_path_str(result_ssid, "device_name", (devName==NULL) ? "" : devName);
                    js_set_path_str(result_ssid, "product_name", (os==NULL) ? "" : os);
                    js_set_path_str(result_ssid, "ipaddress", (ipaddr==NULL) ? "" : ipaddr);
                    js_set_path_int(result_ssid, "tx_rate", txrate);
                    js_set_path_int(result_ssid, "rx_rate", rxrate);
                    js_set_path_double(result_ssid, "tx_byte", ((down/1024)/1024)); // Bytes ->  MBytes
                    js_set_path_double(result_ssid, "rx_byte", ((up/1024)/1024));
                    js_set_path_int(result_ssid, "rssi", rssi);
                    js_set_path_str(result_ssid, "connection_time", (assoctime==NULL) ? "" : assoctime);
                    js_set_path_int(result_ssid, "bridge", bridge);
                }
            }
        }
        //js_print_hr(result);
        result_str = js_to_str(result);
        sprintf(info, "%s", result_str);
        if(result_ssid!= NULL)
            js_free(result_ssid);
        free(result_str);
    }
    else
        sprintf(info, "");
	/* resize the buffer in case of the size over NORMAL_TX_FRAME_SIZE */
	//memcpy(info, result_str, strlen(result_str)+1);

    if(obj_recv != NULL)
        js_free(obj_recv);
    if(result != NULL)
		js_free(result);
    if(obj_para != NULL)
		js_free(obj_para);
	seipc_close(handle);
}

int get_antenna_result(WAMP_Node_t *wNode, char *info)
{
    char *wlan_mac, *lan_mac, *delim=" \t", *pch = NULL, *saveptr = NULL;
    int radio = 0, ssid = 0, rxrate = 0, rssi = 0, i = 0;
    char mac_addr[18], ipv6_addr[64], cmd[128], buf[256];
    JsonNode *obj_para;

    if(NULL != wNode->para){
        if(obj_para = js_parse_str(wNode->para)) {
            //js_print_hr(obj_para);
            radio = js_get_path_int(obj_para, "radio"); /* radio  1 : 2_4G, 2 : 5G, 4 : 5G-2 */
            ssid = js_get_path_int(obj_para, "ssid");   /* ssid   1 : ssid 1  ~  8 : ssid 8 */
            wlan_mac = js_get_path_str(obj_para, "dst_wlan_mac");   /* destination wlan mac address */
            lan_mac = js_get_path_str(obj_para, "dst_lan_mac");   /* destination lan mac address */
        }
    }
    sprintf(mac_addr, "%s", lan_mac);

    /* mac to ipv6 */
    api_get_ipv6_link_local_by_mac(mac_addr, ipv6_addr);

    /* iperf3 -6 -c ipv6_addr%br-lan -p 60001 -b 10M -R -t 3600 -i 60 */
    if (sys_interact(buf, sizeof(buf), "ps w | grep '%s' | grep -v grep", ipv6_addr) == -1) {
        sprintf(cmd, "iperf3 -6 -c %s%%br-lan -p 60001 -b 10M -R -t 3600 -i 60 > /dev/null 2>&1 &", ipv6_addr);
        system(cmd);
    }

    memset(buf, 0, sizeof(buf));

    /* sys_interact(buf, sizeof(buf), "wlanconfig ath%d%d list sta | grep %s", radio - 1, ssid - 1, mac); */
    if ( (ssid - 1 ) == 0 )
        sys_interact(buf, sizeof(buf), "wlanconfig ath%d list sta | grep %s", radio/2, wlan_mac);
    else
        sys_interact(buf, sizeof(buf), "wlanconfig ath%d%d list sta | grep %s", radio/2 , ssid - 1, wlan_mac);

    /* get RX RATE and RSSI */
    pch = strtok_r(buf, delim, &saveptr);
    while ( pch != NULL ) {
        if (i == 4) {   /* RX rate */
            rxrate = atoi(pch);
        }

        if (i == 5) {   /* RSSI */
            rssi = atoi(pch);
        }

        pch = strtok_r(NULL, delim, &saveptr);
        i++;
    }

    sprintf(info, "{ \"rx_rate\": %d, \"rssi\": %d }", rxrate, rssi);

    js_free(obj_para);
}
