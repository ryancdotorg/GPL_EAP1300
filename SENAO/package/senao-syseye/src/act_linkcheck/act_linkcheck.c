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
#include "https.h"
#include "statuscode.h"

/*
 *  Utility
 */

static JsonNode *get_keypath(JsonNode *cfg, int path_key)
{
    char path[32];
    JsonNode *gx;

    memset(path, 0, sizeof(path));
    snprintf(path, sizeof(path), "Action[key=%d]/key", path_key);

    gx = js_get_path(cfg, path);
    if (gx == NULL){
        js_set_path(cfg, path);
    }

    snprintf(path, sizeof(path), "Action[key=%d]", path_key);
    gx = js_get_path(cfg, path);
    return gx;
}

static int get_if_waninfo(char *iface, uint32_t *gw)
{
    int fd, ret = -1, i = 0;
    char line[128] , *p , *c, *g, *saveptr;
    if ((fd = open("/proc/net/route" , O_RDONLY)) == -1 )
        return -1;

    while (read(fd, &line[i], 1) == 1){
        if (line[i] == '\n' || line[i] == 0x0){
	    line[i] = 0;

            //fprintf(stderr, ">>>> %s\n", line);

            p = strtok_r(line , " \t", &saveptr);
            c = strtok_r(NULL , " \t", &saveptr);
            g = strtok_r(NULL , " \t", &saveptr);
            if(p!=NULL && c!=NULL)
            {
                if(strcmp(c , "00000000") == 0)
                {
                    strcpy(iface,p);
                    if (g){
                        long ng;
                        struct in_addr addr;
                        sscanf(g, "%lx", &ng);
                        addr.s_addr=ng;
                        *gw = ng;
                        ret=0;
                    }
                    break;
                }
            }
	    i = 0;
	    continue;
	}
	i++;
    }
    close(fd);
    return ret;
}


static int get_dns_addr(char *dns_serv)
{
    int fd, ret = -1, i = 0;
    char line[128], *name, *value, *saveptr;

    if ((fd = open("/tmp/resolv.conf.auto" , O_RDONLY)) == -1 )
        return -1;

    while (read(fd, &line[i], 1) == 1){
        if (line[i] == '\n' || line[i] == 0x0){
	    line[i] = 0;

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
void *linkcheck(void *args)
{
    pthread_detach(pthread_self());
    struct action_args_t *act_args;
    char *cmd, *p_args;
    int path_key, ret, i;
	char cfgpath[CFG_PATH_LENGTH];
	struct seipc_t *handle=NULL;

    char iface[16];
    uint32_t gw;
    struct in_addr addr_gw;
    char dns_serv[16];
    char *dns_query_host;
    char *http_query_host;
    char *http_query_path;
    char *https_query_host;
    char *https_query_path;

    int retry_icmp = 3;
    int retry_dns= 3;
    int retry_http= 2;
    int retry_https= 2;

    act_args = (struct action_args_t *)args;
    cmd = act_args->cmd;
    p_args = act_args->args;
    path_key = act_args->path_key;
    handle = seipc_create("act_linkcheck", "/tmp/syseye.unix");
    JsonNode *js_linkcheck;
    JsonNode *setjs, *act, *conn;
    // Get configuration
    js_linkcheck = seipc_get_blk(handle, "CFG_RUNTIME/linkcheck");
    dns_query_host = js_get_path_str(js_linkcheck, "dns/query_host");
    http_query_host = js_get_path_str(js_linkcheck, "http/query_host");
    http_query_path = js_get_path_str(js_linkcheck, "http/query_path");
    https_query_host = js_get_path_str(js_linkcheck, "https/query_host");
    https_query_path = js_get_path_str(js_linkcheck, "https/query_path");
    retry_icmp = js_get_path_int(js_linkcheck, "icmp/retry");
    retry_dns = js_get_path_int(js_linkcheck, "dns/retry");
    retry_http = js_get_path_int(js_linkcheck, "http/retry");
    retry_https = js_get_path_int(js_linkcheck, "https/retry");
    //

    setjs=js_parse_str("{\"CFG_RAM\":{\"Action\":[]}}");
    snprintf(cfgpath, sizeof(cfgpath), "CFG_RAM/Action[key=%d]", path_key);
    act = js_idx_set_path(setjs, cfgpath);

    sedbg(" ========== \n");
    sedbg(" icmp retry %d\n", retry_icmp);
    sedbg(" dns_query_host %s, retry %d\n",dns_query_host, retry_dns);
    sedbg(" http_query_host %s retry %d\n",http_query_host, retry_http);
    sedbg(" https_query_host %s retry %d\n",https_query_host, retry_https);
    sedbg(" ========== \n");

    // check phylink
    ret = check_phy_status();
    snprintf(cfgpath, sizeof(cfgpath), "Connectivity[protocol=\"%s\"]", "phylink");
    conn = js_idx_set_path(act, cfgpath);
    js_set_path_int(conn, "status", ret);

    // check arpdup
    snprintf(cfgpath, sizeof(cfgpath), "Connectivity[protocol=\"%s\"]", "arpdup");
    conn = js_idx_set_path(act, cfgpath);
    if (get_if_waninfo(iface, &gw) != -1){
        addr_gw.s_addr = gw;
        ret = check_arpdup_status(iface);
    } else {
        addr_gw.s_addr = 0;
        sedbg("interface and gateway detect fail!\n");
        ret = STATUS_FAIL_DEV_IF_GW;
    }
    js_set_path_int(conn, "status", ret);

    // check icmp
    snprintf(cfgpath, sizeof(cfgpath), "Connectivity[protocol=\"%s\"]", "icmp");
    conn = js_idx_set_path(act, cfgpath);
    if (get_if_waninfo(iface, &gw) != -1){
        addr_gw.s_addr = gw;
        sedbg(" icmp gateway: %s\n", inet_ntoa(addr_gw));
	i=0;
	do {
		ret = check_icmp_status(inet_ntoa(addr_gw));
		i++;
		if (ret != STATUS_OK) {
			sedbg("icmp status:%d retry %d times\n", ret, i);
		}
	} while(i <= retry_icmp && ret != STATUS_OK);
    } else {
        addr_gw.s_addr = 0;
        sedbg("interface and gateway detect fail!\n");
        ret = STATUS_FAIL_DEV_IF_GW;
    }
    js_set_path_int(conn, "status", ret);
    // check dns
    //
    snprintf(cfgpath, sizeof(cfgpath), "Connectivity[protocol=\"%s\"]", "dns");
    conn = js_idx_set_path(act, cfgpath);
    if (ret == STATUS_OK){
        if (get_dns_addr(dns_serv) == 0){
            sedbg(" dns server: %s\n", dns_serv);
	    i=0;
	    do {
	        ret = check_dns_status(dns_serv, dns_query_host);
	        i++;
		if (ret != STATUS_OK) {
		    sedbg("dns status:%d retry %d times\n", ret, i);
		}
	    } while(i <= retry_dns && ret != STATUS_OK);
	} else {
            sedbg("dns server detect fail!\n");
            ret = STATUS_FAIL_DEV_DNS;
        }
    }
    js_set_path_int(conn, "status", ret);

    // check http
    //
    snprintf(cfgpath, sizeof(cfgpath), "Connectivity[protocol=\"%s\"]", "http");
    conn = js_idx_set_path(act, cfgpath);
    if (ret == STATUS_OK){
        i=0;
        do {
            ret = check_http_status(http_query_host, http_query_path);
            i++;
            if (ret != STATUS_OK) {
                sedbg("http status:%d retry %d times\n", ret, i);
            }
        } while(i <= retry_http && ret != STATUS_OK);
    }
    js_set_path_int(conn, "status", ret);
    // check https
    snprintf(cfgpath, sizeof(cfgpath), "Connectivity[protocol=\"%s\"]", "https");
    conn = js_idx_set_path(act, cfgpath);
    if (ret == STATUS_OK){
        i=0;
        do {
            ret = check_https_status(https_query_host, https_query_path);
            i++;
            if (ret != STATUS_OK) {
                sedbg("https status:%d retry %d times\n", ret, i);
            }
        } while(i <= retry_https && ret != STATUS_OK);
    }
    js_set_path_int(conn, "status", ret);
    js_set_path_int(act, "status", 1);
leave:
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

int act_linkcheck_main(gdata_t *g, int path_key, void *args)
{
    JsonNode *cfg_ram = g->cfg_ram;
    JsonNode *out;
    char *c, *a;

    if(!args) {
        sedbg(" empty args, fail return\n");
        return -1;
    }

    //sedbg("args %s\n", args);
    out = get_keypath(g->cfg_ram, path_key);
    js_set_path_int(out, "status", 0);  // status 0 not ready
    js_set_path_str(out, "act_name", "act_linkcheck");

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
    safe_fork_fn(linkcheck, act_args);
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
    if (pthread_create(&thread_id, &attr, linkcheck, act_args) != 0){
        sedbg("create thread fail\n");
        return -1;
    }
    if (pthread_attr_destroy(&attr) != 0){
        sedbg("destroy thread attr error\n");
        return -1;
    }
#endif

    return 0;
}

/**
 * 
 */

void *act_linkcheck(void *action_data)
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
    act_linkcheck_main(gdata, t->path_key, t->args);
    return NULL;
}
