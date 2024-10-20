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
#include "global.h"
#include "mio.h"
#include "nlc.h"
#include "nl_cmdparser.h"
#include "ezjson.h"
#include "util.h"
#include "util_socket.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void nl_cmdhandler(void *data, int sockfd, void *content, int content_len)
{
    gdata_t *gdata; 
    mio_data_t *mdata;
    JsonNode *js_data_zone;
    nl_cmd_t *nl_cmd;
    nl_set_data_t *nl_set_data;
    nl_act_data_t *nl_act_data;
    nl_prune_data_t *nl_prune_data;
    cmd_type_t type = CMD_NONE;
    char *path, *value, *s, *e;
    storage_t storage;

    if (data == NULL || content == NULL){
        sedbg("data error\n");
        return;
    }
    gdata = (gdata_t *)data; 
    mdata = gdata->mio_data;

    nl_cmd = (nl_cmd_t *)content;

    sedbg("[netlink cmd] %s(%d) type:%d\n", nl_cmd->func, nl_cmd->line, nl_cmd->type);

    switch(nl_cmd->type){
        case CMD_SET:
            nl_set_data = (nl_set_data_t *) nl_cmd->data;
            sedbg("[netlink cmd set] storage:[%d] path:[%s] value:[%s]\n", nl_set_data->storage, nl_set_data->path, nl_set_data->value);
            storage = nl_set_data->storage;	    
            path = nl_set_data->path;
            value = nl_set_data->value;

            if (storage == CFG_RUNTIME)
                js_data_zone = gdata->cfg_runtime;
            else if (storage == CFG_SAVED)
                js_data_zone = gdata->cfg_saved;
	    else if (storage == CFG_RAM)
                js_data_zone = gdata->cfg_ram;

	    if (strlen(value) == 0){ // path only
              	js_idx_set_path(js_data_zone, path);
	    } else if ( (s = strchr(value, '\"')) && (e = strrchr(value,'\"'))  && s!=e ){ // string
                value = s+1;
		*e = '\0';
		js_idx_set_path_str(js_data_zone, path, value);
            }else{ // integer
                js_idx_set_path_int(js_data_zone, path, atoi(value));
	    }
            break;
        case CMD_GET:
            break;
        case CMD_GET_NUM_ARRAY:
            break;
        case CMD_PRUNE:
            nl_prune_data = (nl_prune_data_t *) nl_cmd->data;
            sedbg("[netlink cmd prune] storage:[%d] path:[%s]\n", nl_prune_data->storage, nl_prune_data->path);
            storage = nl_prune_data->storage;	    
            path = nl_prune_data->path;

            if (storage == CFG_RUNTIME)
                js_data_zone = gdata->cfg_runtime;
            else if (storage == CFG_SAVED)
                js_data_zone = gdata->cfg_saved;
	    else if (storage == CFG_RAM)
                js_data_zone = gdata->cfg_ram;
            js_free_path(js_data_zone, path);
            break;
        case CMD_PRUNE_FIRST_ELEMENT:
            nl_prune_data = (nl_prune_data_t *) nl_cmd->data;
            sedbg("[netlink cmd prune] storage:[%d] path:[%s]\n", nl_prune_data->storage, nl_prune_data->path);
            storage = nl_prune_data->storage;	    
            path = nl_prune_data->path;

            if (storage == CFG_RUNTIME)
                js_data_zone = gdata->cfg_runtime;
            else if (storage == CFG_SAVED)
                js_data_zone = gdata->cfg_saved;
	    else if (storage == CFG_RAM)
                js_data_zone = gdata->cfg_ram;
   
            js_free(json_first_child(js_get_path(js_data_zone, path)));
            break;
        case CMD_UPDATE_ELEMENT: // move element to the tail
	    {
            JsonNode *t, *n;		    
            nl_prune_data = (nl_prune_data_t *) nl_cmd->data;
            sedbg("[netlink cmd prune] storage:[%d] path:[%s]\n", nl_prune_data->storage, nl_prune_data->path);
            storage = nl_prune_data->storage;	    
            path = nl_prune_data->path;

            if (storage == CFG_RUNTIME)
                js_data_zone = gdata->cfg_runtime;
            else if (storage == CFG_SAVED)
                js_data_zone = gdata->cfg_saved;
	    else if (storage == CFG_RAM)
                js_data_zone = gdata->cfg_ram;
            t = js_get_path(js_data_zone, path);
            if ((n = t->parent) == NULL){
                sedbg("update element: parent not found\n");		    
                goto fmterr;
            }
            json_remove_from_parent(t);
            json_append_element(n, t);
            break;
	    }
        case CMD_RECONF:
            break;
        case CMD_COMMIT:
            break;
        case CMD_TREEVIEW:
            break;
        case CMD_ACTION:
	    {
            action_t *act;
            action_data_t action_data;
            nl_act_data = (nl_act_data_t *) nl_cmd->data;
            sedbg("[netlink cmd action] name:[%s] delay:[%d] command:[%s]\n", nl_act_data->name, nl_act_data->delay, nl_act_data->command);
            memset(&action_data, 0, sizeof(action_data_t));
            action_data.name = nl_act_data->name;
            action_data.args = nl_act_data->command;
            action_data.delay = nl_act_data->delay;
            action_data.path_key = 0; // not used
            action_data.gdata = (void *)gdata;
            act = find_action_by_name(gdata->action, action_data.name);
            if (!act){
                sedbg("action %s not found\n", action_data.name);
                goto fmterr;
            }
            act->action_func((void *)&action_data);
            break;
	    }
        case CMD_ACTIONS:
            break;
        case CMD_LISTACTION:
            break;
        default:
            goto fmterr;
    }
    return;
fmterr: 
    sedbg("pkt format parse error\n");
    return;
}
