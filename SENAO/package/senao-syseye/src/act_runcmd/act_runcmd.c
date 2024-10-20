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
#include "act_runcmd.h"

/*
 * Sample System information include
 *
 * */
#include <arpa/inet.h>

/*
 *  Utility
 */


int act_runcmd_main(gdata_t *g, int path_key, void *args)
{
#if 1
    system(args);
#else
    pid_t pid;
    int argc, flags, fd;
    char **argvp;
    char *p = NULL;
    //JsonNode *cfg_ram = g->cfg_ram;
    pid = safe_fork();
    if (pid !=0) // parent or others
        return 0;
    //child

    // check output path or device
    flags = O_WRONLY | O_CREAT;
    if ( (p = strstr(args, ">>")) != NULL){
        flags |= O_APPEND;
        *p = '\0';
        p+=2;
    } else if ((p = strchr(args, '>')) != NULL){
        flags |= O_TRUNC;
        *p = '\0';
        p+=1;
    }
    if (p){
        while (*p ==32 || *p == 9) p++; // 32 is space, 9 is tab
        sedbg("output path:[%s]\n", p);
        if ((fd = open(p, flags, 0644)) < 0)
            perror(p);
        else {
            dup2(fd, STDERR_FILENO);
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
    }

    argc = makeargv(args, " \t", &argvp);
    setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
    execvp(argvp[0], argvp);
    freemakeargv(argvp);
    exit(errno);
#endif
    return 0;
}

/**
 * 
 */

void *act_runcmd(void *action_data)
{
    action_data_t *t = (action_data_t *) action_data;
    JsonNode *cfg_ram;
    JsonNode *gx;
    if (action_data == NULL){
        return NULL;
    }
    gdata_t *gdata = (gdata_t *)t->gdata;
    // TODO: handle delay

    act_runcmd_main(gdata, t->path_key, t->args);
    return NULL;
}
