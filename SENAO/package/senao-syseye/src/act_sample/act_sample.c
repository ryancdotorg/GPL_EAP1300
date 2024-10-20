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
#include "act_sample.h"

/*
 * Sample System information include
 *
 * */
#include <arpa/inet.h>

/*
 *  Utility
 */

JsonNode *get_keypath(JsonNode *cfg, int path_key)
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

/*
 * Sample system information get function
 *
 * */
int get_if_netinfo(char *iface, uint32_t *gw)
{
	FILE *fd;
	char line[128] , *p , *c, *g, *saveptr;
	int nRet=-1;
	fd = fopen("/proc/net/route" , "r");

	while(fgets(line , sizeof(line) , fd))
	{
		//sedbg("* %s", line);
		p = strtok_r(line , " \t", &saveptr);
		c = strtok_r(NULL , " \t", &saveptr);
		g = strtok_r(NULL , " \t", &saveptr);

		if(p!=NULL && c!=NULL)
		{
			if(strcmp(c , "00000000") == 0)
			{
				strcpy(iface,p);
				if (g)
				{
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

int act_sample_main(gdata_t *g, int path_key, void *args)
{
    JsonNode *cfg_ram = g->cfg_ram;

    JsonNode *out;
    out = get_keypath(g->cfg_ram, path_key);
    // Handle the action and write to key path: out
    char iface[16];
    uint32_t gw;
	struct in_addr addr_gw;
    get_if_netinfo(iface, &gw);
	addr_gw.s_addr=gw;
    js_set_path_str(out, "interface", iface);
    js_set_path_str(out, "addr", inet_ntoa(addr_gw));
    
    return 0;
}

/**
 * 
 */

void *act_sample(void *action_data)
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

    act_sample_main(gdata, t->path_key, t->args);
    return NULL;
}
