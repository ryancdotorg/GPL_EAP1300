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
#include "arpdup.h"
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
static int get_if_waninfo(char *iface, uint32_t *gw)
{
    FILE *fd;
    char line[128] , *p , *c, *g, *saveptr;
    int nRet=-1;
    fd = fopen("/proc/net/route" , "r");
    while(fgets(line , sizeof(line) , fd))
    {
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
                    nRet=0;
                }
                break;
            }
        }
    }
    fclose(fd);
    return nRet;
}
*/

static int get_if_waninfo(char *iface, uint32_t *gw)
{
    int fd, ret = -1, i = 0;
    char line[128] , *p , *c, *g, *saveptr;
    fd = open("/proc/net/route" , O_RDONLY);
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

#define CFG_PATH_LENGTH 512
void *check_arpdup(void *args)
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

    JsonNode *setjs, *act, *conn;

    act_args = (struct action_args_t *)args;
    cmd = act_args->cmd;
    p_args = act_args->args;
    path_key = act_args->path_key;

    handle = seipc_create("act_check_arpdup", "/tmp/syseye.unix");

    // Get configuration
    //

    setjs=js_parse_str("{\"CFG_RAM\":{\"Actions\":[]}}");
    snprintf(cfgpath, sizeof(cfgpath), "CFG_RAM/Actions[key=%d]", path_key);
    act = js_idx_set_path(setjs, cfgpath);

    snprintf(cfgpath, sizeof(cfgpath), "Connectivity[protocol=\"%s\"]", "arpdup");
    conn = js_idx_set_path(act, cfgpath);

    // check arpdup
    if (get_if_waninfo(iface, &gw) == -1){
        sedbg("interface and gateway detect fail!\n");
	ret = STATUS_FAIL_DEV_IF_GW;
    } else {
        sedbg(" ========== \n");
        sedbg(" gateway interface: %s \n", iface);
        sedbg(" ========== \n");
        addr_gw.s_addr = gw;
        ret = check_arpdup_status(iface);
    }
    js_set_path_int(conn, "status", ret);
    seipc_set_blk(handle, setjs);
    js_free(setjs);
    seipc_close(handle);

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
    return (void *)1;
}

int act_check_arpdup_main(gdata_t *g, int path_key, void *args)
{
    JsonNode *cfg_ram = g->cfg_ram;
    char *c, *a;
    char cfgpath[CFG_PATH_LENGTH];

    if(!args) {
        sedbg(" empty args, fail return\n");
        return -1;
    }

    snprintf(cfgpath, sizeof(cfgpath), "Actions[key=%d]/Connectivity[protocol=\"%s\"]/status", path_key, "arpdup");
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
    safe_fork_fn(check_arpdup, act_args);
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

    if (pthread_create(&thread_id, &attr, check_arpdup, act_args) != 0){
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

void *act_check_arpdup(void *action_data)
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
    act_check_arpdup_main(gdata, t->path_key, t->args);
    return NULL;
}
