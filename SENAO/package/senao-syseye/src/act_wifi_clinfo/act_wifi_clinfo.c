/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                    *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  syseye                                                                        *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <action_data.h>
#include <ezjson.h>
#include <mio.h>
#include <global.h>
#include "util.h"
#include "act_wifi_clinfo.h"

/*
 * wifi_clinfo System information include
 *
 * */
#include <arpa/inet.h>

/*
 *  Utility
 */

static JsonNode *get_keypath(JsonNode *cfg, int path_key)
{
    char path[32];
    JsonNode *gx;

    snprintf(path, sizeof(path), "Action[key=%d]", path_key);

    gx = js_get_path(cfg, path);
    if (gx == NULL){
        snprintf(path, sizeof(path), "Action[key=%d]", path_key);
        gx = js_set_path(cfg, path);
    }
    return gx;
}

static int get_ssid_id(JsonNode *ssid_map, char *ifname)
{
    JsonNode *e;
    if (!ssid_map || !ifname) return 0;

    json_foreach(e, ssid_map){
        if (!strcmp(ifname, js_get_path_str(e, "ifname")))
            return js_get_path_int(e, "id");
    }
    return 0;
}

static char *get_ssid_ifname(JsonNode *ssid_map, int id)
{
    JsonNode *e;
    if (!ssid_map) return NULL;

    json_foreach(e, ssid_map){
        if (id == js_get_path_int(e, "id"))
            return js_get_path_str(e, "ifname");
    }
    return NULL;
}

/*
 * get wifi client history 
 *
 * */
static int get_cli_history(JsonNode *obj_wireless, JsonNode *obj_ssid_map, JsonNode *out)
{
    JsonNode *obj_client, *obj_ssid, *obj_traffic;
    JsonNode *out_obj_client, *out_obj_ssid, *out_obj_traffic;
    char path[128];
    char *mac_addr = NULL;
    char *ifname = NULL;
    int id;

    json_foreach(obj_client, js_get_path(obj_wireless, "clients")){
        mac_addr = js_get_path_str(obj_client, "mac_addr");
        if (!mac_addr) continue;
        // create output: entry client
        snprintf(path, sizeof(path), "clients[mac_addr=\"%s\"]", mac_addr);
        out_obj_client = js_set_path(out, path);

        // create output: mac_addr
        js_set_path_str(out_obj_client, "mac_addr", mac_addr);
        // create output: ssid_id 
        ifname = js_get_path_str(obj_client, "ifname");
        id = get_ssid_id(obj_ssid_map, ifname);
        js_set_path_int(out_obj_client, "ssid_id", id);

        // create output: entry ssid
        json_foreach(obj_ssid, js_get_path(obj_client, "ssids")){
            ifname = js_get_path_str(obj_ssid, "ifname");
            if (!ifname) continue;
            // create output: entry ssid_id
            id = get_ssid_id(obj_ssid_map, ifname);
            snprintf(path, sizeof(path), "ssids[id=%d]", id);
            out_obj_ssid = js_set_path(out_obj_client, path);
            // create output: ssids[]/ssid_id from ifname
            js_set_path_int(out_obj_ssid, "id", id);
            
            json_foreach(obj_traffic, js_get_path(obj_ssid, "traffic")){
                id  = js_get_path_int(obj_traffic, "id");
                if (id == JS_FAIL) continue;
                // create output: entry traffic
                snprintf(path, sizeof(path), "traffic[id=%d]", id);
                out_obj_traffic = js_set_path(out_obj_ssid, path);

                // create output: ssids[]/traffic[]/id
                js_set_path_int(out_obj_traffic, "id", id);
                // create output: ssids[]/traffic[]/up
                js_set_path_double(out_obj_traffic, "up", js_get_path_double(obj_traffic,"up"));
                // create output: ssids[]/traffic[]/down
                js_set_path_double(out_obj_traffic, "down", js_get_path_double(obj_traffic, "down"));
            }
        }
    }
}

/*
 * wifi_clinfo system information get function
 *
 * */
static void get_clinfo(int radio, int idx, JsonNode *obj_wireless, JsonNode *obj_ssid_map, JsonNode *out)
{
	FILE *fp;
	char line[1024], *saveptr;
    char *addr, *aid, *chan, *txrate, *rxrate, *rssi, *txbytes, *rxbytes;
	int nRet=-1, count=0, is_first = 1;
// format interface TODO: fix the rule
    char cmd[32];
    char path[128];
    char ifname[6] = {0};
    char *s;
    double up=0, down=0, up2cloud=0, down2cloud=0;  // tx: up rx: down
    int id = 0;
    if ( radio == 2 )
        radio = 4; // 5G-2 are ath4~ath47

    if ( idx == 0 )
        snprintf(ifname, sizeof(ifname), "ath%d", radio);
    else
        snprintf(ifname, sizeof(ifname), "ath%d%d", radio, idx);
//
    snprintf(cmd, sizeof(cmd), "wlanconfig %s list sta", ifname);
	fp = popen(cmd , "r");
    
	while(fgets(line , sizeof(line) , fp))
	{
        JsonNode *obj_client, *obj_ssid, *obj_traffic;
        JsonNode *obj_ezm_client, *obj_ezm_ssid, *obj_ezm_traffic;
        JsonNode *out_obj_client, *out_obj_ssid, *out_obj_traffic;
        if (count++ == 0) continue; // ignore title
//		printf("%s", line);
		addr = strtok_r(line , " \t", &saveptr);
		aid = strtok_r(NULL , " \t", &saveptr);
		chan = strtok_r(NULL , " \t", &saveptr);
		txrate = strtok_r(NULL , " \t", &saveptr);
		rxrate = strtok_r(NULL , " \t", &saveptr);
		rssi = strtok_r(NULL , " \t", &saveptr);
		txbytes = strtok_r(NULL , " \t", &saveptr);
		rxbytes = strtok_r(NULL , " \t", &saveptr);
        if ((s = strstr(txbytes,"Kb"))!=NULL)
            *s = '\0';
        if ((s = strstr(rxbytes,"Kb"))!=NULL)
            *s = '\0';
/*        printf("addr:%s aid:%s chan:%s txrate:%s rxrate:%s rssi:%s txbytes:%s rxbytes:%s\n",
                addr,
                aid,
                chan,
                txrate,
                rxrate,
                rssi,
                txbytes,
                rxbytes);
*/        
        // format output entry client
        snprintf(path, sizeof(path), "clients[mac_addr=\"%s\"]", addr);
        out_obj_client = js_set_path(out, path);
        // create output: mac_addr
        js_set_path_str(out_obj_client, "mac_addr", addr);

        // create output: entry ssid_id
        id = get_ssid_id(obj_ssid_map, ifname);
        js_set_path_int(out_obj_client, "ssid_id", id);

        // create output: entry ssid
        snprintf(path, sizeof(path), "ssids[id=%d]", id);
        out_obj_ssid = js_set_path(out_obj_client, path);
        // create output: ssids[]/ssid_id from ifname
        js_set_path_int(out_obj_ssid, "id", id);
        snprintf(path, sizeof(path), "traffic[id=%d]", 0); // now id always 0
        out_obj_traffic = js_set_path(out_obj_ssid, path);
        if ((up = js_get_path_double(out_obj_traffic, "up")) != JS_FAIL)
            up2cloud = up + atof(txbytes)*1024;
        else 
            up2cloud = atof(txbytes)*1024;
        if ((down = js_get_path_double(out_obj_traffic, "down")) != JS_FAIL)
            down2cloud = down + atof(rxbytes)*1024;
        else 
            down2cloud = atof(rxbytes)*1024;

        sedbg("txbytes:%f rxbytes:%f up:%f %f down:%f %f\n", atof(txbytes), atof(rxbytes), up, up*1024, down, down*1024);
        js_set_path_double(out_obj_traffic, "up", up2cloud); 
        js_set_path_double(out_obj_traffic, "down", down2cloud); 

    }
	fclose(fp);
}
void ezm_previous_modify(JsonNode *out, JsonNode *obj_ssid_map, JsonNode *obj_ezm_wireless)
{
    JsonNode *out_obj_client, *out_obj_ssid, *out_obj_traffic;
    JsonNode *obj_ezm_traffic;
    char path[128];
    char *mac_addr = NULL;
    char *ifname = NULL;
    int id, traffic_id;
    double up, down, up_prev, down_prev, up2cloud, down2cloud;
    json_foreach(out_obj_client, js_get_path(out, "clients")){
        mac_addr = js_get_path_str(out_obj_client, "mac_addr");
        json_foreach(out_obj_ssid, js_get_path(out_obj_client, "ssids")){
            id = js_get_path_int(out_obj_ssid, "id");
            ifname = get_ssid_ifname(obj_ssid_map, id);
            json_foreach(out_obj_traffic, js_get_path(out_obj_ssid, "traffic")){
                traffic_id = js_get_path_int(out_obj_traffic, "id");

                snprintf(path, sizeof(path), "clients[mac_addr=\"%s\"]/ssids[id=%d]/traffic[id=%d]", mac_addr, id, traffic_id);
                obj_ezm_traffic = js_set_path(obj_ezm_wireless, path);
                if ( (up = js_get_path_double(out_obj_traffic,"up")) == JS_FAIL)
                    up = 0;
                if ( (down = js_get_path_double(out_obj_traffic,"down")) == JS_FAIL)
                    down = 0;

                if ( (up_prev = js_get_path_double(obj_ezm_traffic,"up")) == JS_FAIL)
                    up_prev = 0;
                if ( (down_prev = js_get_path_double(obj_ezm_traffic,"down")) == JS_FAIL)
                    down_prev = 0;
                up2cloud = up - up_prev;
                down2cloud = down - down_prev;

                js_set_path_double(out_obj_traffic,"up", up2cloud);
                js_set_path_double(out_obj_traffic,"down", down2cloud);

                js_set_path_double(obj_ezm_traffic,"up", up_prev + up2cloud);
                js_set_path_double(obj_ezm_traffic,"down", down_prev + down2cloud);
            }
        }
    }
}

#define OM_RADIO_NUM 2
int act_wifi_clinfo_main(gdata_t *g, int path_key, void *args)
{
    JsonNode *cfg_ram = g->cfg_ram;
    JsonNode *cfg_runtime = g->cfg_runtime;
    int radio = 0, idx = 0;

    JsonNode *out, *obj_wireless, *obj_client, *obj_ezm_wireless, *obj_ssid_map;
    out = get_keypath(cfg_ram, path_key);
    obj_wireless = js_get_path(cfg_ram, "wireless");
    obj_ezm_wireless = js_get_path(cfg_ram, "EZMCloud/wireless");
    obj_ssid_map = js_get_path(cfg_runtime, "EZMCloud/wireless/ssid_map");
    // Handle the action and write to key path: out

    // get client history
    get_cli_history(obj_wireless, obj_ssid_map, out);

    // get current connected client
    for ( radio = 0 ; radio < OM_RADIO_NUM ; radio++ )
    {
        for ( idx = 0 ; idx < 8 ; idx++ )
        {
            get_clinfo(radio, idx, obj_wireless, obj_ssid_map, out);
        }
    }
    // up, down handle: minus previous, update previous
    ezm_previous_modify(out, obj_ssid_map, obj_ezm_wireless);

    return 0;
}

/**
 * 
 */

void *act_wifi_clinfo(void *action_data)
{
    action_data_t *t = (action_data_t *) action_data;
    JsonNode *cfg_ram;
    char path[64];
    JsonNode *gx;
    if (action_data == NULL){
        return NULL;
    }
    gdata_t *gdata = (gdata_t *)t->gdata;
    // TODO: handle delay
    act_wifi_clinfo_main(gdata, t->path_key, t->args);
    return NULL;
}
