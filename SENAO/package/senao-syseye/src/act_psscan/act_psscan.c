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
#include <time.h>
#include <limits.h>
#include <action_data.h>
#include <ezjson.h>
#include <mio.h>
#include <global.h>
#include "util.h"
#include "act_psscan.h"

/*
 * Sample System information include
 *
 * */
#include <arpa/inet.h>

/*
 *  Utility
 */
int add_cli(gdata_t *g, char *mac, int count, int min_signal, int max_signal, int avg_signal, int last_seen_signal, int associated, int channel)
{
    JsonNode *obj_wireless, *obj_psscan;
    char path[128];
    int num_psscan, max_psscan;
    int seconds;
    int count_saved;
    int min_signal_saved;
    int max_signal_saved;
    int avg_signal_saved;
    JsonNode *cfg_ram = g->cfg_ram;
    JsonNode *cfg_runtime = g->cfg_runtime;

    max_psscan = js_get_path_int(cfg_runtime, "wireless/max_psscan");

    obj_wireless = js_get_path(cfg_ram, "wireless");
    num_psscan = js_get_path_int(obj_wireless, "num_psscan");
    if (num_psscan == JS_FAIL)
        num_psscan = 0;

    snprintf(path, sizeof(path), "psscan[mac=\"%s\"]", mac);

    obj_psscan = js_get_path(obj_wireless, path);
    seconds = time((time_t *) NULL);
    if (obj_psscan == NULL){ // add new
        sedbg("new mac, add\n");
        num_psscan++;
        obj_psscan = js_set_path(obj_wireless, path);
        js_set_path_double(obj_psscan, "first_seen", seconds);
        min_signal_saved = min_signal;
        max_signal_saved = max_signal;
        avg_signal_saved = avg_signal;
        if (associated == -1)
            associated = 0;
	count_saved = 0;
    }
    else {
        sedbg("exist mac, update\n");
        min_signal_saved = js_get_path_int(obj_psscan, "min_signal");
        max_signal_saved = js_get_path_int(obj_psscan, "max_signal");
        avg_signal_saved = js_get_path_int(obj_psscan, "avg_signal");
	count_saved = js_get_path_int(obj_psscan, "count");
    }
    min_signal = (min_signal_saved < min_signal) ? min_signal_saved : min_signal;
    max_signal = (max_signal_saved > max_signal) ? max_signal_saved : max_signal;
    avg_signal = (count_saved*avg_signal_saved + avg_signal*count)/(count+count_saved);

    js_set_path_int(obj_psscan, "count", count+count_saved);
    js_set_path_int(obj_psscan, "min_signal", min_signal);
    js_set_path_int(obj_psscan, "max_signal", max_signal);
    js_set_path_int(obj_psscan, "avg_signal", avg_signal);
    js_set_path_int(obj_psscan, "last_seen_signal", last_seen_signal);
    if (associated != -1)
        js_set_path_int(obj_psscan, "associated", associated);
    js_set_path_int(obj_psscan, "channel", channel);

    js_set_path_double(obj_psscan, "last_seen", seconds);

    if (num_psscan > max_psscan){
        // delete oldest entry
	time_t oldest_last_seen = LONG_MAX , last_seen;
	JsonNode *obj_oldest_psscan;
        json_foreach(obj_psscan, js_get_path(obj_wireless, "psscan")){
            last_seen = js_get_path_double(obj_psscan, "last_seen");
	    if (oldest_last_seen > last_seen){
               oldest_last_seen = last_seen;
               obj_oldest_psscan = obj_psscan;
            }
	}
        js_free(obj_oldest_psscan);
	num_psscan--;
    }
    js_set_path_int(obj_wireless, "num_psscan", num_psscan);
    return 0;
}

int cli_info(gdata_t *g, char *c)
{
    char *p, *saveptr;
    int i;
    char mac[18];
    int count;
    int min_signal;
    int max_signal;
    int avg_signal;
    int last_seen_signal;
    int associated; // (0:false; 1:true)
    int channel; // (2.4G:1-11; 5G:36+)
    sedbg("cli:%s\n", c);
    i=0;
    if ((p = strtok_r(c, ",", &saveptr)) != NULL){
	i++;
        strcpy(mac, p);
        while ((p = strtok_r(NULL, ",", &saveptr)) != NULL){
            if (i==1) count=atoi(p);
            if (i==2) min_signal=atoi(p);
            if (i==3) max_signal=atoi(p);
            if (i==4) avg_signal=atoi(p);
            if (i==5) last_seen_signal=atoi(p);
            if (i==6) associated=atoi(p);
            if (i==7) channel=atoi(p);
            if (i>=8) {
                sedbg("error:format error\n");
                return -1;
            }
            i++;
        }
    }
    sedbg("mac:%s count:%d min_signal:%d max_signal:%d avg_signal:%d last_seen_signal:%d associated: %d channel:%d\n", mac, count, min_signal, max_signal, avg_signal, last_seen_signal, associated, channel);
    add_cli(g, mac, count, min_signal, max_signal, avg_signal, last_seen_signal, associated, channel);
    return 0;
}

int act_psscan_main(gdata_t *g, int path_key, void *args)
{
    JsonNode *cfg_ram = g->cfg_ram;
    char *c, *a;
    char *p, *saveptr;

    c = args;
    if ((a = strchr(args, ' ')) != NULL){
        *a = '\0';
        a = a + 1;
    }
    sedbg("cmd:[%s] args:[%s]\n", c, a);
    if ((p = strtok_r(a, "\n", &saveptr)) != NULL){
        cli_info(g, p);
        while ((p = strtok_r(NULL, "\n", &saveptr)) != NULL){
            cli_info(g, p);           
        }
    }

    return 0;
}

/**
 * 
 */

void *act_psscan(void *action_data)
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

    act_psscan_main(gdata, t->path_key, t->args);
    return NULL;
}
