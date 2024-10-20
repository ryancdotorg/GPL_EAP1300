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
#include <ezjson.h>
#include <mio.h>
#include <global.h>
#include "util.h"
#include "act_sitesurvey.h"
#include "libseipc.h"
#include <pthread.h>

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

#define CFG_PATH_LENGTH 512
void *get_sitesurvey(void *args)
{
    struct action_args_t *act_args;
    char *cmd, *p_args;
    int path_key;
    char ifname[16];
	FILE *fp;
	char line[1024], *saveptr;
    char *bssid, *ssid, *len, *mode, *ch, *sig, *enc, *type;
	int count=0;
    char cmdstr[128];
	char cfgpath[CFG_PATH_LENGTH];
	struct seipc_t *handle=NULL;
    JsonNode *setjs, *act, *scan;
    if(!args) {
        sedbg(" empty args, fail return\n");
        return NULL;
    }
    act_args = (struct action_args_t *)args;
    cmd = act_args->cmd;
    p_args = act_args->args;
    path_key = act_args->path_key;
    sedbg("cmd:%s arg:[%s] path_key %d\n", cmd, p_args, path_key);

    memset(ifname, 0, sizeof(ifname));
    strncpy(ifname, p_args, sizeof(ifname));

    sprintf(cmdstr, "iwlist %s scan", ifname);
	fp = popen(cmdstr, "r");
    handle = seipc_create("act_sitesurvey", "/tmp/syseye.unix");
    if (!handle) {
		sedbg("Cannot create seipc handle. Exit!\n");
		return NULL;
	}
    setjs=js_parse_str("{\"CFG_RAM\":{\"Action\":[]}}");
	snprintf(cfgpath, sizeof(cfgpath), "CFG_RAM/Action[key=%d]", path_key);
    act = js_idx_set_path(setjs, cfgpath);
	while(fgets(line , sizeof(line) , fp))
	{
        if (count++ < 2) {
            sedbg("ignore: %s\n", line);
            continue; // ignore title
        }
		//sedbg("(%d) %s\n", strlen(len), line);
		bssid = strtok_r(line , " \t", &saveptr);
		ssid = strtok_r(NULL , " \t", &saveptr);
		if(*ssid == '0'){
			len = "0";
		}
		else
			len = strtok_r(NULL , " \t", &saveptr);
		mode = strtok_r(NULL , " \t", &saveptr);
		ch = strtok_r(NULL , " \t", &saveptr);
		sig = strtok_r(NULL , " \t", &saveptr);
		enc = strtok_r(NULL , " \t", &saveptr);
		type = strtok_r(NULL , " \t", &saveptr);
        snprintf(cfgpath, sizeof(cfgpath), "scan_list[bssid=\"%s\"]", bssid);
        scan = js_idx_set_path(act, cfgpath);
        js_set_path_str(scan, "bssid", bssid);
        js_set_path_str(scan, "ssid", ssid);
        js_set_path_str(scan, "len", len);
        js_set_path_str(scan, "mode", mode);
        js_set_path_str(scan, "ch", ch);
        js_set_path_str(scan, "sig", sig);
        js_set_path_str(scan, "enc", enc);
        js_set_path_str(scan, "type", type);
/*		sedbg("bssid:%s ssid:%s len:%s mode:%s ch:%s sig:%s enc:%s type:%s \n",
				bssid,
				ssid,
				len,
				mode,
				ch,
				sig,
				enc,
				type);
                */
                
	}
//    seipc_debug_on(stdout);
    js_set_path_int(act, "status", 1);
    seipc_set_blk(handle, setjs);
    seipc_close(handle);
    js_free(setjs);
    // TODO: get from parent thread, free in child.
    free(act_args);
	fclose(fp);
}

int act_sitesurvey_main(gdata_t *g, int path_key, void *args)
{
    JsonNode *cfg_ram = g->cfg_ram;
    JsonNode *cfg_runtime = g->cfg_runtime;
    JsonNode *out;
    char *c, *a;

    if(!args) {
        sedbg(" empty args, fail return\n");
        return -1;
    }
    //sedbg("args %s\n", args);
    out = get_keypath(cfg_ram, path_key);
    js_set_path_int(out, "status", 0);  // status 0 not ready

    c = args;
    if ((a = strchr(args, ' ')) != NULL){
        *a = '\0';
        a = a + 1;
    }
    //sedbg("cmd:%s arg:[%s]\n", c, a);
    struct action_args_t *act_args = calloc(1, sizeof(struct action_args_t));
    act_args->cmd = c;
    act_args->args = a;
    act_args->path_key = path_key;

    pthread_attr_t attr;
    pthread_t thread_id;
    if (pthread_attr_init(&attr)!=0){
        sedbg("create thread attr error\n");
        return -1;
    }

    if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)!=0){
        sedbg("thread set attr detach error\n");
        return -1;
    }

    if (pthread_create(&thread_id, &attr, get_sitesurvey, act_args) != 0){
        sedbg("create thread fail\n");
        return -1;
    }

    if (pthread_attr_destroy(&attr) != 0){
        sedbg("destroy thread attr error\n");
        return -1;
    }
    // TODO use join to wait thread exit, then free act_args

    return 0;
}

/**
 * 
 */

void *act_sitesurvey(void *action_data)
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
    act_sitesurvey_main(gdata, t->path_key, t->args);
    return NULL;
}
