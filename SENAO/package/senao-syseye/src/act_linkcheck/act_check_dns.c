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
#include <arpa/inet.h>
#include <signal.h>
#include <ezjson.h>
#include <mio.h>
#include <global.h>
#include "util.h"
#include "act_linkcheck.h"
#include "libseipc.h"
#include <pthread.h>
#include "phylink.h"
#include "arpdup.h"
#include "icmp.h"
#include "dns.h"
#include "http.h"
#include "statuscode.h"

/*
 *  Utility
 */

static JsonNode *get_keypath(JsonNode *cfg, int path_key)
{
    char path[32];
    JsonNode *gx;

    memset(path, 0, sizeof(path));
    snprintf(path, sizeof(path), "Actions[key=%d]/key", path_key);

    gx = js_get_path(cfg, path);
    if (gx == NULL){
        js_set_path(cfg, path);
    }

    snprintf(path, sizeof(path), "Actions[key=%d]", path_key);
    gx = js_get_path(cfg, path);
    return gx;
}

/*
static int get_dns_addr(char *dns_serv)
{
    char line[64], *name, *value, *saveptr;
	FILE *fp;
    int ret = -1;
    if ((fp = fopen("/tmp/resolv.conf.auto", "r")) == NULL){
        ret = -1;
        return ret;
    }

	while(fgets(line , sizeof(line) , fp)){
		name = strtok_r(line , " \t", &saveptr);
		value = strtok_r(NULL , " \t\n", &saveptr);
        if (name != NULL && !strcmp(name, "nameserver")){
            sedbg("nameserver:%s\n", value);
            strcpy(dns_serv, value);
            ret = 0;
            break;
        }
    }
    fclose(fp);
    return ret;
}
*/

static int get_dns_addr(char *dns_serv)
{
    int fd, ret = -1, i = 0;
    char line[128], *name, *value, *saveptr;

    if ((fd = open("/tmp/resolv.conf.auto" , O_RDONLY)) == -1){
        sedbg("resolv.conf.auto open fail\n");
        return -1;
    }

    while (read(fd, &line[i], 1) == 1){
        if (line[i] == '\n' || line[i] == 0x0){
	    line[i] = 0;
	    sedbg("line:%s\n", line);
            name = strtok_r(line , " \t", &saveptr);
            value = strtok_r(NULL , " \t", &saveptr);
            if (name != NULL && !strcmp(name, "nameserver")){
                sedbg("nameserver:%s\n", value);
                strcpy(dns_serv, value);
                ret = 0;
                break;
            }
	    i = 0;
	    continue;
        }
        i++;
    }
    close(fd);
    return ret;
}

#define CFG_PATH_LENGTH 512
void *check_dns(void *args)
{
    pthread_detach(pthread_self());
    struct action_args_t *act_args;
    char *cmd, *p_args;
    int path_key, ret, i;
    char cfgpath[CFG_PATH_LENGTH];
    struct seipc_t *handle=NULL;
    JsonNode *setjs, *act, *conn;
    JsonNode *js_linkcheck;

    char dns_serv[16];
    char *dns_query_host;
    int retry_dns= 3;

    act_args = (struct action_args_t *)args;
    cmd = act_args->cmd;
    p_args = act_args->args;
    path_key = act_args->path_key;
    handle = seipc_create("act_linkcheck", "/tmp/syseye.unix");

    // Get configuration
    js_linkcheck = seipc_get_blk(handle, "CFG_RUNTIME/linkcheck");
    dns_query_host = js_get_path_str(js_linkcheck, "dns/query_host");
    retry_dns = js_get_path_int(js_linkcheck, "dns/retry");
    //

    setjs=js_parse_str("{\"CFG_RAM\":{\"Actions\":[]}}");
    snprintf(cfgpath, sizeof(cfgpath), "CFG_RAM/Actions[key=%d]", path_key);
    act = js_idx_set_path(setjs, cfgpath);

    snprintf(cfgpath, sizeof(cfgpath), "Connectivity[protocol=\"%s\"]", "dns");
    conn = js_idx_set_path(act, cfgpath);

    // check dns
    if (get_dns_addr(dns_serv) != 0){
        sedbg("dns server detect fail!\n");
        ret = STATUS_FAIL_DEV_DNS;
    } else {
        sedbg(" ========== \n");
        sedbg(" dns server: %s\n", dns_serv);
        sedbg(" dns_query_host %s, retry %d\n",dns_query_host, retry_dns);
        sedbg(" ========== \n");
	i=0;
	do {
		ret = check_dns_status(dns_serv, dns_query_host);
		i++;
		if (ret != STATUS_OK) {
			sedbg("dns status:%d retry %d times\n", ret, i);
		}
	} while(i < retry_dns && ret != STATUS_OK);
    }
    js_set_path_int(conn, "status", ret);

    seipc_set_blk(handle, setjs);
    seipc_close(handle);
    js_free(setjs);
    js_free(js_linkcheck);
    // TODO: get from parent thread, free in child.
    if (act_args){
	if (act_args->cmd){
	    free(act_args->cmd);
	    act_args->cmd = NULL;
	}
	if (act_args->args){
	    free(act_args->args);
	    act_args->args = NULL;
	}
        free(act_args);
        act_args = NULL;
    }
    pthread_exit((void*)1);
}

int act_check_dns_main(gdata_t *g, int path_key, void *args)
{
    JsonNode *cfg_ram = g->cfg_ram;
    char *c, *a;
    char cfgpath[CFG_PATH_LENGTH];

    if(!args) {
        sedbg(" empty args, fail return\n");
        return -1;
    }

    snprintf(cfgpath, sizeof(cfgpath), "Actions[key=%d]/Connectivity[protocol=\"%s\"]/status", path_key, "dns");
    js_set_path_int(cfg_ram, cfgpath, 0);

    //sedbg("args %s\n", args);

    c = args;
    if ((a = strchr(args, ' ')) != NULL){
        *a = '\0';
        a = a + 1;
    }
    sedbg("cmd:%s arg:[%s]\n", c, a);
    struct action_args_t *act_args = calloc(1, sizeof(struct action_args_t));
    if (c)
        act_args->cmd = strdup(c);
    if (a)
        act_args->args = strdup(a);
    act_args->path_key = path_key;
    
#if defined(USE_FORK)
    safe_fork_fn(check_dns, act_args);
    if (act_args){
	if (act_args->cmd){
	    free(act_args->cmd);
	    act_args->cmd = NULL;
	}
	if (act_args->args){
	    free(act_args->args);
	    act_args->args = NULL;
	}
        free(act_args);
        act_args = NULL;
    }
#else
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

    if (pthread_create(&thread_id, &attr, check_dns, act_args) != 0){
        sedbg("create thread fail\n");
        return -1;
    }

    if (pthread_attr_destroy(&attr) != 0){
        sedbg("destroy thread attr error\n");
        return -1;
    }
    // TODO use join to wait thread exit, then free act_args
#endif
    return 0;
}

/**
 * 
 */

void *act_check_dns(void *action_data)
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
    act_check_dns_main(gdata, t->path_key, t->args);
    return NULL;
}
