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
#include <pthread.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "util.h"
#include "act_selftest.h"

#define NETLINK_SYSEYE NETLINK_USERSOCK
//#define NETLINK_SYSEYE 30
#define SENLC_SELFTEST 20
#define MAX_PAYLOAD 1024

/*
 * Sample System information include
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
static void safe_mem_free(void **buf){
    if (buf!=NULL && *buf !=NULL){
        free(*buf);
        *buf = NULL;	
    }
}
#define safe_free(pointer) safe_mem_free((void **) &(pointer))

void *selftest(void *args)
{
    struct action_args_t *act_args;
    char *cmd, *p_args;
    int path_key;

    int fd;
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;

    pthread_detach(pthread_self());
    act_args = (struct action_args_t *)args;
    cmd = act_args->cmd;
    p_args = act_args->args;
    path_key = act_args->path_key;

    if ((fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_SYSEYE)) < 0){
        perror("create socket:");
        goto leave;
    }
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* kernel will input process pid, self pid */

    bind(fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;   /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)calloc(1, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_type |= SENLC_SELFTEST;
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags |= NLM_F_REQUEST;
    sprintf(NLMSG_DATA(nlh), "%s %s", cmd, p_args);

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;

    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    sendmsg(fd,&msg,0);
    safe_free(nlh);	    
    close(fd);
leave:
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

int act_selftest_main(gdata_t *g, int path_key, void *args)
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
    safe_fork_fn(selftest, act_args);
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
    if (pthread_create(&thread_id, &attr, selftest, act_args) != 0){
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

void *act_selftest(void *action_data)
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

    act_selftest_main(gdata, t->path_key, t->args);
    return NULL;
}
