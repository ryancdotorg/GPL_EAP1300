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
#include <pthread.h>
#include "act_runlib_snlog.h"
#include <snlog.h>

/*
 * Sample System information include
 *
 * */
#include <arpa/inet.h>

/*
 *  Utility
 */
void *runlib_snlog(void *args)
{
    pthread_detach(pthread_self());
    struct action_args_t *act_args;
    char *cmd, *p_args;
    char *p1, *p2, *pend;

    act_args = (struct action_args_t *)args;
    cmd = act_args->cmd;
    p_args = act_args->args;
    sedbg("cmd:%s arg:[%s]\n", cmd, p_args);
    // FIXME: ugly format ... seperate 2 args

    p1 = p_args +1;
    pend = strchr(p1, '\"');
    *pend = '\0';
     
    p2 = pend + 3; // " space and "
    pend = strrchr(p2, '\"');
    *pend = '\0';
    //sedbg("ident p1:[%s] logstr p2:[%s]\n", p1, p2);

    opensnlog();
    snlog(p1, p2);
    closesnlog();
    
leave:
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
}

int act_runlib_snlog_main(gdata_t *g, int path_key, void *args)
{
    pid_t pid;
    int argc, flags, fd;
    char *c, *a;
    char **argvp;
    char *p = NULL;

    if(!args) {
        sedbg(" empty args, fail return\n");
        return -1;
    }
    //sedbg("args %s\n", args);
    c = args;
    if ((a = strchr(args, ' ')) != NULL){
        *a = '\0';
        a = a + 1;
    }
    //sedbg("cmd:%s arg:[%s]\n", c, a);
    struct action_args_t *act_args = calloc(1, sizeof(struct action_args_t));
    if (c)
        act_args->cmd = strdup(c);
    if (a)
        act_args->args = strdup(a);

#if defined(USE_FORK)
    safe_fork_fn(runlib_snlog, act_args);
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
    if (pthread_create(&thread_id, &attr, runlib_snlog, act_args) != 0){
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

void *act_runlib_snlog(void *action_data)
{
    action_data_t *t = (action_data_t *) action_data;
    JsonNode *cfg_ram;
    JsonNode *gx;
    if (action_data == NULL){
        return NULL;
    }
    gdata_t *gdata = (gdata_t *)t->gdata;
    // TODO: handle delay

    act_runlib_snlog_main(gdata, t->path_key, t->args);
    return NULL;
}
