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
#include "act_ws_wifi_clinfo.h"
#include <stdarg.h>
#include "iwinfo.h"

/*
 * wifi_clinfo System information include
 *
 * */
#include <arpa/inet.h>

int sys_interact(char *output, int length, char *fmt, ...)
{
    static char command[255+1];
    FILE *pipe;
    int c;
    int i;
    va_list ap;

    memset(command, 0, sizeof(command));

    va_start(ap, fmt);
    vsnprintf(command, sizeof(command), fmt, ap);
    va_end(ap);

    //  //printf("[%s][%d] command:[%s]\n", __FUNCTION__, __LINE__, command);
#ifdef HAS_CMD_DEBUG_LEVEL
    TRACE(MOD_SYSUTIL|MSG_DBG, "[FUNC %s::%s][LINE %d]: command = %s\n", __FILE__, __FUNCTION__, __LINE__, command);
#endif

    if ((pipe = popen(command, "r")) == NULL)
    {
        goto err;
    }

    for (i = 0; ((c = fgetc(pipe)) != EOF) && (i < length - 1); i++)
    {
        output[i] = (char) c;
    }
    output[i] = '\0';

    pclose(pipe);

    if (strlen(output) == 0)
    {
        goto err;
    }

    return strlen(output);

err:
    strncpy(output, "---", strlen(output));
    return -1;
}

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

/*
 * get wifi client history info 
 *
 * */
static int get_cli_history(JsonNode *obj_wireless, JsonNode *out)
{
    JsonNode *obj_client, *obj_ssid, *obj_traffic;
    JsonNode *out_obj_client, *out_obj_ssid, *out_obj_traffic;
    char path[128];
    char *mac_addr = NULL;
    char *ifname = NULL;
    double up;

    json_foreach(obj_client, js_get_path(obj_wireless, "clients")){
        mac_addr = js_get_path_str(obj_client, "mac_addr");
        if (!mac_addr) continue;
        // create output: entry client
        snprintf(path, sizeof(path), "clients[mac_addr=\"%s\"]", mac_addr);
        out_obj_client = js_set_path(out, path);

        // create output: mac_addr
        js_set_path_str(out_obj_client, "mac_addr", mac_addr);

        // create output: entry ssid
        json_foreach(obj_ssid, js_get_path(obj_client, "ssids")){
            ifname = js_get_path_str(obj_ssid, "ifname");
            if (!ifname) continue;
            // create output: entry ssid_id
            snprintf(path, sizeof(path), "ssids[ifname=\"%s\"]", ifname);
            out_obj_ssid = js_set_path(out_obj_client, path);

            json_foreach(obj_traffic, js_get_path(obj_ssid, "traffic")){
                up  = js_get_path_double(obj_traffic,"up");
                if (up == JS_FAIL) continue;
                // create output: entry traffic
                js_set_path_double(out_obj_ssid, "up", js_get_path_double(obj_traffic,"up"));
                js_set_path_double(out_obj_ssid, "down", js_get_path_double(obj_traffic, "down"));
            }
        }
    }
    
}

int get_ifname(int radio, int idx, int mode, char *ifname, int len)
{
    /* 100000000  enjet flag */
    /* 000000001  AP */
    /* 000000010  CB */
    /* 000000100  WDS AP */
    /* 000001000  WDS STATION */
    /* 000010000  WDS_BRIDGE */
    /* 000100000  REPEATER */

    if ((mode & 256) && (mode & 5)) {    /* enjet + AP (100001) or enjet + WDS AP (100100) */
        snprintf(ifname, len, "enjet%d", radio); /* enjet AP 2.4G: enjet0; 5G : enjet1; 5G-2 : enjet2; */
    }
    else {
        if (mode & 1) {     /* 000000001  AP */
            if ( radio == 2 )
                radio = 4; // 5G-2 are ath4~ath47
            if ( idx == 0 )
                snprintf(ifname, len, "ath%d", radio);
            else
                snprintf(ifname, len, "ath%d%d", radio, idx);
        }
        else if (mode & 2){ /* 000000010  CB */
            snprintf(ifname, len, "ath%d6", (radio==0) ? 2 : (radio==1) ? 5 : 6); /* Client Bridge 2.4G: ath26; 5G : ath56; 5G-2 : ath66; */
        }
        else if (mode & 4){ /* 000000100  WDS AP */
            snprintf(ifname, len, "ath%d%d", (radio==0) ? 2 : (radio==1) ? 5 : 6, idx);
        }
        else if (mode & 8){ /* 000001000  WDS STATION */
            snprintf(ifname, len, "ath%d5", (radio==0) ? 2 : (radio==1) ? 5 : 6); /* WDS Station 2.4G: ath25; 5G : ath55; 5G-2 : ath65; */
        }
        else if (mode & 16){ /* 000010000  WDS BRIDGE */
            snprintf(ifname, len, "ath%d8", (radio==2) ? 4 : radio); /* WDS BRIDGE 2.4G: ath08; 5G : ath18; 5G-2 : ath48; */
        }
        else if (mode & 32){ /* 000100000  REPEATER */
            if (idx)
                snprintf(ifname, len, "ath%d9", (radio==0) ? 2 : (radio==1) ? 5 : 6); /* REPEATER CB 2.4G: ath29; 5G : ath59; 5G-2 : ath69; */
            else
                snprintf(ifname, len, "ath%d8", (radio==0) ? 2 : (radio==1) ? 5 : 6); /* REPEATER AP 2.4G: ath28; 5G : ath58; 5G-2 : ath68; */
        }
    }
    return 0;
}

/*
 * wifi_clinfo system information get function
 *
 * */
static void get_clinfo(int radio, int idx, JsonNode *obj_wireless, JsonNode *out, int mode)
{
    FILE *fp, *fp1;
    char line[1024], *saveptr;
    char *addr = NULL, *aid = NULL, *chan = NULL, *txrate = NULL, *rxrate = NULL, *rssi = NULL, *txbytes = NULL, *rxbytes = NULL;
    char *ipaddr = NULL, *os = NULL, *devName = NULL;
    char *idle, *txseq, *rxseq, *caps, *acaps, *erp, *state_maxrate, *htcaps, *assoctime;
    int count=0, isUP = 0, val = 0, signal = 0;
    char cmd[64], path[128], ifname[8] = {0}, if_bk[8] = {0}, *s, buf[16] = {0};
    double up=0, down=0, up2cloud=0, down2cloud=0;  // tx: up rx: down

    get_ifname(radio, idx, mode, ifname, sizeof(ifname));

    sedbg("get_clinfo radio:[%d] idx:[%d] mode:[%d] ifname:[%s]\n", radio, idx, mode, ifname);

    memset(cmd, 0, sizeof(cmd));
    snprintf(cmd, sizeof(cmd), "wlanconfig %s list sta 2> /dev/null", ifname);
    if ((fp = popen(cmd , "r")) == NULL) {
        sedbg("popen() error!\n");
        perror("socket error");
        return;
    }

    while(fgets(line , sizeof(line) , fp) != NULL)
    {
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
        idle = strtok_r(NULL , " \t", &saveptr);
        txseq = strtok_r(NULL , " \t", &saveptr);
        rxseq = strtok_r(NULL , " \t", &saveptr);
        if(!((mode & 16) || ((mode & 4) && idx > 3)))        /* wds bridge station list and wds ap (bridge) caps is empty */
            caps = strtok_r(NULL , " \t", &saveptr);
        acaps = strtok_r(NULL , " \t", &saveptr);
        erp = strtok_r(NULL , " \t", &saveptr);
        state_maxrate = strtok_r(NULL , " \t", &saveptr);
        htcaps = strtok_r(NULL , " \t", &saveptr);
        assoctime = strtok_r(NULL , " \t", &saveptr);

        if ((s = strstr(txrate,"M"))!=NULL)
            *s = '\0';
        if ((s = strstr(rxrate,"M"))!=NULL)
            *s = '\0';
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
        /* sedbg("addr:%s aid:%s chan:%s txrate:%s rxrate:%s rssi:%s txbytes:%s rxbytes:%s assoctime:%s\n", addr, aid, chan, txrate, rxrate, rssi, txbytes, rxbytes, assoctime); */

        if(atoi(rxrate)==0)
            isUP = 0;
        else{
            if(atoi(idle) < 30)
                isUP = 1;
            else
                isUP = 0;
        }

        if(isUP) {                          /* only show real connected clients */
            // format output entry client
            JsonNode *out_obj_client, *out_obj_ssid;
            snprintf(path, sizeof(path), "clients[mac_addr=\"%s\"]", addr);
            out_obj_client = js_set_path(out, path);
            // create output: mac_addr
            js_set_path_str(out_obj_client, "mac_addr", addr);

            // create output: entry ssid_id
            /* js_set_path_str(out_obj_client, "ifname", ifname); */

            snprintf(path, sizeof(path), "ssids[ifname=\"%s\"]", ifname);
            out_obj_ssid = js_set_path(out_obj_client, path);
            // create output: ssids[]/ssid_id from ifname
            js_set_path_str(out_obj_ssid, "ifname", ifname);
            js_set_path_int(out_obj_ssid, "ssid_id", (mode == 1)?(idx+1):0);

            if ((up = js_get_path_double(out_obj_ssid, "up")) != JS_FAIL)
                up2cloud = up + atof(txbytes)*1024;
            else
                up2cloud = atof(txbytes)*1024; /* KBytes -> Bytes */
            if ((down = js_get_path_double(out_obj_ssid, "down")) != JS_FAIL)
                down2cloud = down + atof(rxbytes)*1024;
            else
                down2cloud = atof(rxbytes)*1024;

            js_set_path_double(out_obj_ssid, "up", up2cloud);
            js_set_path_double(out_obj_ssid, "down", down2cloud);
            js_set_path_int(out_obj_ssid, "txrate", atoi(txrate));
            js_set_path_int(out_obj_ssid, "rxrate", atoi(rxrate));
            js_set_path_int(out_obj_ssid, "rssi", atoi(rssi));
            js_set_path_str(out_obj_ssid, "assoctime", assoctime);
            js_set_path_int(out_obj_ssid, "radio", radio);
            js_set_path_int(out_obj_ssid, "bridge", 0);
        }
    }

    /* CB mode || WDS station || REPEATER CB interface ( ps: if wlanconfig can't get info, use iwinfo get information ) */
    if (((mode & 2) || (mode & 8) || ((mode & 32) && idx)) && (addr == NULL)){
        const struct iwinfo_ops *iw;
        if ((iw = iwinfo_backend(ifname))!=NULL){
            iw->bitrate(ifname, &val);
            if (val != 0) {          /* if bitrate equal 0 , not connected */
                JsonNode *out_obj_client, *out_obj_ssid;
                iw->bssid(ifname, buf);
                snprintf(path, sizeof(path), "clients[mac_addr=\"%s\"]", buf);
                out_obj_client = js_set_path(out, path);
                // create output: mac_addr
                js_set_path_str(out_obj_client, "mac_addr", buf);
                snprintf(path, sizeof(path), "ssids[ifname=\"%s\"]", ifname);
                out_obj_ssid = js_set_path(out_obj_client, path);
                // create output: ssids[]/ssid_id from ifname
                js_set_path_str(out_obj_ssid, "ifname", ifname);
	        js_set_path_int(out_obj_ssid, "ssid_id", 0);
                js_set_path_int(out_obj_ssid, "txrate", val/1000);
                js_set_path_int(out_obj_ssid, "rxrate", 0);
                iw->signal(ifname, &signal);
                js_set_path_int(out_obj_ssid, "rssi", signal);
                js_set_path_int(out_obj_ssid, "radio", radio);
                js_set_path_int(out_obj_ssid, "bridge", 1);
            }
       }
    }
    //sedbg_js(out);
    pclose(fp);

    snprintf(cmd, sizeof(cmd), "/tmp/fingerprint_status_list_%s", ifname);
    if ((fp1 = fopen(cmd , "r")) == NULL) {
        sedbg("fopen() error!\n");
        return;
    }

    //line:[c0:9f:42:7b:3a:af]----addr:[c0:9f:42:7b:3a:af]---ipaddr:[172.27.0.21]---os:[Apple iOS]---devName:[huanggudeiPhone]
    while(fgets(line , sizeof(line) , fp1)!= NULL)
    {
        JsonNode *obj_client_finger;
        addr = strtok_r(line , "|", &saveptr);
        ipaddr = strtok_r(NULL , "|", &saveptr);
        os = strtok_r(NULL , "|", &saveptr);
        devName = strtok_r(NULL , "|", &saveptr);
        snprintf(path, sizeof(path), "clients[mac_addr=\"%s\"]/ssids[ifname=\"%s\"]", addr, ifname);
        obj_client_finger = js_set_path(out, path);
        js_set_path_str(obj_client_finger, "ipaddr", ipaddr);
        js_set_path_str(obj_client_finger, "os", os);
        js_set_path_str(obj_client_finger, "devName", devName);
        //sedbg("fingerprint : line:[%s] addr:[%s] ipaddr:[%s] os:[%s] devName:[%s]\n", line, addr, ipaddr, os, devName);
    }
    fclose(fp1);
    //sedbg_js(out);
}

static void get_result_traffic(JsonNode *out, JsonNode *obj_prev_wireless)
{
    JsonNode *out_obj_client, *out_obj_ssid, *out_obj_traffic;
    JsonNode *obj_prev_traffic;
    char path[128];
    char *mac_addr = NULL;
    char *ifname = NULL;
    double up, down, up_prev, down_prev, up2cloud, down2cloud;

    json_foreach(out_obj_client, js_get_path(out, "clients")){
        mac_addr = js_get_path_str(out_obj_client, "mac_addr");
        json_foreach(out_obj_ssid, js_get_path(out_obj_client, "ssids")){
            ifname = js_get_path_str(out_obj_ssid, "ifname");
            snprintf(path, sizeof(path), "clients[mac_addr=\"%s\"]/ssids[ifname=\"%s\"]", mac_addr, ifname);
            obj_prev_traffic = js_set_path(obj_prev_wireless, path);

            if ( (up = js_get_path_double(out_obj_ssid,"up")) == JS_FAIL)
                up = 0;
            if ( (down = js_get_path_double(out_obj_ssid,"down")) == JS_FAIL)
                down = 0;

            if ( (up_prev = js_get_path_double(obj_prev_traffic,"up")) == JS_FAIL)
                up_prev = 0;
            if ( (down_prev = js_get_path_double(obj_prev_traffic,"down")) == JS_FAIL)
                down_prev = 0;

            up2cloud = up - up_prev;
            down2cloud = down - down_prev;

            js_set_path_double(out_obj_ssid,"up", up2cloud);
            js_set_path_double(out_obj_ssid,"down", down2cloud);

            js_set_path_double(obj_prev_traffic,"up", up_prev + up2cloud);
            js_set_path_double(obj_prev_traffic,"down", down_prev + down2cloud);
        }
    }
    //sedbg_js(out);
    //sedbg_js(obj_prev_traffic);
}
#if SUPPORT_WLAN_5G_2_SETTING
#define OM_RADIO_NUM 3
#else
#define OM_RADIO_NUM 2
#endif
int act_ws_wifi_clinfo_main(gdata_t *g, int path_key, void *args)
{
    JsonNode *cfg_ram = g->cfg_ram;
    JsonNode *cfg_runtime = g->cfg_runtime;
    JsonNode *out, *obj_wireless, *obj_client, *obj_prev_wireless;
    int radio_start = 0, radio_end = OM_RADIO_NUM, idx_start = 0, idx_end = 8, idx = 0, allradio = 1, allssid = 1, mode = 0;
    char *saveptr, buf[16] = {0};
    char *p_radio_start, *p_idx_start;

    strtok_r(args , " ", &saveptr);

    p_radio_start = strtok_r(NULL , " ", &saveptr);
    //  radio  0 : all radio, 1 : 2_4G, 2 : 5G, 4 : 5G-2
    radio_start = p_radio_start == NULL?0:atoi(p_radio_start);

    p_idx_start = strtok_r(NULL , " ", &saveptr);
    // ssid   0 : all ssid, 1 : ssid 1  ~  8 : ssid 8
    idx_start = p_idx_start == NULL?0:atoi(p_idx_start);
    
    /* If do not get all radio, set for loop radio start and end value */
    if ( radio_start != 0 ) {
        allradio = 0;
        radio_start = (radio_start / 2) + 1 ; /* convert Radio  0 : 2_4G, 1 : 5G, 2 : 5G-2 */
        radio_end = radio_start--;
    }
    /* If do not get all ssdi, set for loop ssdi start and end value */
    if ( idx_start != 0 ) {
        allssid = 0;
        idx_end = idx_start--;  /* ex : idx_start:0 , idx_end:1*/
    }

    out = get_keypath(cfg_ram, path_key);
    obj_wireless = js_get_path(cfg_ram, "wireless");
    obj_prev_wireless = js_get_path(cfg_ram, "WebScoket/wireless");

    // Handle the action and write to key path: out
    // get client history
    get_cli_history(obj_wireless, out);

    sedbg("radio_start:[%d] radio_end:[%d] idx_start:[%d] idx_end:[%d]\n", radio_start, radio_end, idx_start, idx_end);
    // get current connected client
    for ( ; radio_start < radio_end ; radio_start++ ) {
        mode = 0, idx = 0;

        /* if radio disabled, continue */
        sys_interact(buf, sizeof(buf), "uci get wireless.wifi%d.disabled 2> /dev/null", radio_start);
        if(atoi(buf))
            continue;

        sys_interact(buf, sizeof(buf), "uci get wireless.wifi%d.qboost_enable 2> /dev/null", radio_start);
        mode |= atoi(buf) << 8;                                 /* set enjet flag */

        memset(buf, 0, sizeof(buf));
        sys_interact(buf, sizeof(buf), "uci get wireless.wifi%d.opmode | tr -d '\n' 2> /dev/null", radio_start);

        if (strcmp(buf, "ap") == 0)
            mode |= 1 << 0;
        else if (strcmp(buf, "sta") == 0)
            mode |= 1 << 1;
        else if (strcmp(buf, "wds_ap") == 0)
            mode |= 1 << 2;
        else if (strcmp(buf, "wds_sta") == 0)
            mode |= 1 << 3;
        else if (strcmp(buf, "wds_bridge") == 0)
            mode |= 1 << 4;
        else if (strcmp(buf, "sta_ap") == 0)
            mode |= 1 << 5;


        if ((mode & 256) && (mode & 5) || (mode & 10) || (mode & 16))          /* enjet + (AP or WDS AP) && STA && WDS Bridge only need to get one interface*/
            get_clinfo(radio_start, idx, obj_wireless, out, mode);
        else if ((mode & 32)) {                                                /* Repeater mode need to get AP & CB interface */
            for ( idx = 0; idx < 2 ; idx++ )
                get_clinfo(radio_start, idx, obj_wireless, out, mode);
        }
        else {                                                  /* AP mode */
            for ( idx = idx_start; idx < idx_end ; idx++ )
                get_clinfo(radio_start, idx, obj_wireless, out, mode);
        }
    }
    // up, down handle: minus previous, update previous
    get_result_traffic(out, obj_prev_wireless);

    return 0;
}

/**
 * 
 */

void *act_ws_wifi_clinfo(void *action_data)
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
    act_ws_wifi_clinfo_main(gdata, t->path_key, t->args);
    return NULL;
}
