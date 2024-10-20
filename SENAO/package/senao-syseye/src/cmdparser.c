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
#include "cmdparser.h"
#include "ezjson.h"
#include "util.h"
#include "util_socket.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef enum {
    STATUS_OK,
    STATUS_FAIL,
    STATUS_ACT_NOT_FOUND,
    STATUS_ACT_NOT_FOUND_PARTIAL
}status_code_t;

static const char pkt_data_template[] =
"{"
"   \"Header\": {"
"   	\"source\": \"syseye\","
"   	\"type\":99\n"
"   },"
"   \"Data\": {"
"     \"Response\": {"
"   	  \"status\": 0,"
"   	  \"message\": \"\""
"     }"
"   }"
"}";

typedef struct status_msg_t{
    status_code_t status_code;
    char *status_msg;
}status_msg_t;

struct status_msg_t status_msgs[]={
    {STATUS_OK, "status is ok"},
    {STATUS_FAIL, "status has error"},
    {STATUS_ACT_NOT_FOUND, "failed, action not found"},
    {STATUS_ACT_NOT_FOUND_PARTIAL, "warning, some actions not found"}
};
int count_status_msg = sizeof(status_msgs)/sizeof(status_msg_t);

static void js_remove_js(void *mio_data,  void *data)
{
    char *path;
	if (mio_data == NULL || data == NULL){
		sedbg("data cannot empty\n");
		return;
	}
    mio_data_t *mdata = (mio_data_t *)mio_data;
    JsonNode *j = (JsonNode *) data;
    sedbg("==delay remove:===\n");
    sedbg_js(j);
    js_free(j);
}

static char *get_status_msg(status_code_t status_code)
{
    int i;
    for (i = 0; i < count_status_msg; ++i){
        if (status_code == status_msgs[i].status_code)
            return status_msgs[i].status_msg;
    }
    return NULL;
} 

static JsonNode* set_pkt_status(JsonNode *js, status_code_t status_code)
{
    JsonNode *t;
    t = js_get_path(js, "Data/Response");
    js_set_path_int(t, "status", status_code);
    js_set_path_str(t, "message", get_status_msg(status_code));
    return js;
}

JsonNode *data_pkt_new(char *pktfmt)
{
    JsonNode *j1, *j2;
    j1 = js_parse_str(pktfmt);

    if (!j1)
        return NULL;

    j2 = js_dup(j1);
    js_free(j1);

    return j2;
}

static JsonNode* set_pkt_data(JsonNode *js, JsonNode *data)
{
    JsonNode *t;
    t = js_get_path(js, "Data");
    if (t){
        js_union(t, data);
    }
    return js;
}
static JsonNode* set_pkt_status_data(status_code_t status_code, JsonNode *data)
{
    char fmt[512];
    JsonNode *js;
    snprintf(fmt, sizeof(fmt), pkt_data_template);

    js = data_pkt_new(fmt);
    if (!js)
        return NULL;
    js = set_pkt_status(js, status_code);
    if(data){
        js = set_pkt_data(js, data);
    }
    return js;
}

static void 
js_write(int sockfd, JsonNode *js)
{
	char *out;
	out = js_to_str(js);
	write_chunked(sockfd, out, strlen(out));
	free(out);
	write_chunked_terminate(sockfd);
}

void cmdhandler(void *data, int sockfd, void *content, int content_len)
{
    gdata_t *gdata; 
    mio_data_t *mdata;
    JsonNode *req, *cfg_runtime, *cfg_saved, *cfg_ram, *datapkt, *cfg_data = NULL, *t, *n, *act_data= NULL, *js_data_zone, *req_data;
     char *source = NULL, path[64], datazone[16];
    cmd_type_t type = CMD_NONE;
    char cfg_path[128];
    int count;
    if (data == NULL || content == NULL){
        sedbg("data error\n");
        return;
    }
    gdata = (gdata_t *)data; 
    mdata = gdata->mio_data;

    req = js_parse_str(content);

    if (!req){
        sedbg("parse json error\n");
        goto fmterr;
    }

    if (!(source = js_get_path_str(req, "Header/source")))
        goto fmterr;

    if ((type = js_get_path_int(req, "Header/type"))==CMD_NONE)
        goto fmterr;

    sedbg("source [%s] type:[%d], req:\n", source, type);
    sedbg_js(req);

    cfg_runtime = js_get_path(req, "Data/CFG_RUNTIME");
    cfg_saved = js_get_path(req, "Data/CFG_SAVED");
    cfg_ram = js_get_path(req, "Data/CFG_RAM");

    switch(type){
        case CMD_SET:
            if (cfg_runtime){
                js_data_zone = gdata->cfg_runtime;
                req_data = cfg_runtime;
                js_idx_union(js_data_zone, req_data);
            }
            if (cfg_saved){
                js_data_zone = gdata->cfg_saved;
                req_data = cfg_saved;
                js_idx_union(js_data_zone, req_data);
            }
            if (cfg_ram){
                js_data_zone = gdata->cfg_ram;
                req_data = cfg_ram;
                js_idx_union(js_data_zone, req_data);
            }
            datapkt = set_pkt_status_data(STATUS_OK, NULL);
            sedbg("source [%s] type:[%d], response:\n", source, type);
            sedbg_js(datapkt);
            js_write(sockfd, datapkt);
            js_free(datapkt);
            break;
        case CMD_GET:
            if (cfg_runtime){
                strcpy(datazone, "CFG_RUNTIME");
                js_data_zone = gdata->cfg_runtime;
                req_data = cfg_runtime;
            }
            if (cfg_saved){
                strcpy(datazone, "CFG_SAVED");
                js_data_zone = gdata->cfg_saved;
                req_data = cfg_saved;
            }
            if (cfg_ram){
                strcpy(datazone, "CFG_RAM");
                js_data_zone = gdata->cfg_ram;
                req_data = cfg_ram;
            }
            cfg_data =  js_parse_str("{}");
            t = js_set_path(cfg_data, datazone);
            js_union(t, req_data);
            js_join(t, js_data_zone);

            datapkt = set_pkt_status_data(STATUS_OK, cfg_data);
            if (cfg_data) {
                js_free(cfg_data);
                cfg_data = NULL;
            }
            sedbg("source [%s] type:[%d], response:\n", source, type);
            sedbg_js(datapkt);
            js_write(sockfd, datapkt);
            js_free(datapkt);
            break;
        case CMD_GET_NUM_ARRAY:
            cfg_data =  js_parse_str("{}");
            if (cfg_runtime)
            {
                strcpy(datazone, "CFG_RUNTIME");
                js_data_zone = gdata->cfg_runtime;
                req_data = cfg_runtime;
            }
            if (cfg_saved){
                strcpy(datazone, "CFG_SAVED");
                js_data_zone = gdata->cfg_saved;
                req_data = cfg_saved;
            }
            if (cfg_ram){
                strcpy(datazone, "CFG_RAM");
                js_data_zone = gdata->cfg_ram;
                req_data = cfg_ram;
            }
            // locate the target in gdata->cfg_xxx
            t = js_get_js(js_data_zone, req_data);
            // check the count
            count = 0;
            json_foreach(n, t){
                count++;
            }
            // format result, use queried cfg_xxx as based
            t = js_set_path(cfg_data, datazone);
            js_union(t, req_data);
            // locate the target in result
            t = js_get_js(t, req_data);
            // if query array key is abc, result written as num_abc
            if (t->parent && t->key){
                snprintf(path, sizeof(path), "num_%s", t->key);
                js_set_path_int(t->parent, path, count);
            }
            // write the same layer as queried array, remove the queried array copied from cfg_ram
            js_free(t);
            datapkt = set_pkt_status_data(STATUS_OK, cfg_data);
            sedbg("source [%s] type:[%d], response:\n", source, type);
            sedbg_js(datapkt);
            js_write(sockfd, datapkt);
            if (cfg_data) {
                js_free(cfg_data);
                cfg_data = NULL;
            }
            js_free(datapkt);
            break;
        case CMD_PRUNE:
            if (cfg_runtime){
                js_data_zone = gdata->cfg_runtime;
                req_data = cfg_runtime;
            }
            if (cfg_saved){
                js_data_zone = gdata->cfg_saved;
                req_data = cfg_saved;
            }
            if (cfg_ram){
                js_data_zone = gdata->cfg_ram;
                req_data = cfg_ram;
            }
            js_free_js(js_data_zone, req_data);
            datapkt = set_pkt_status_data(STATUS_OK, NULL);
            sedbg("source [%s] type:[%d], response:\n", source, type);
            sedbg_js(datapkt);
            js_write(sockfd, datapkt);
            js_free(datapkt);
            break;
        case CMD_PRUNE_FIRST_ELEMENT:
            if (cfg_runtime){
                js_data_zone = gdata->cfg_runtime;
                req_data = cfg_runtime;
            }
            if (cfg_saved){
                js_data_zone = gdata->cfg_saved;
                req_data = cfg_saved;
            }
            if (cfg_ram){
                js_data_zone = gdata->cfg_ram;
                req_data = cfg_ram;
            }
            t = js_get_js(js_data_zone, req_data);
            js_free(json_first_child(t));
            datapkt = set_pkt_status_data(STATUS_OK, NULL);
            sedbg("source [%s] type:[%d], response:\n", source, type);
            sedbg_js(datapkt);
            js_write(sockfd, datapkt);
            js_free(datapkt);
            break;
        case CMD_UPDATE_ELEMENT:
            if (cfg_runtime){
                js_data_zone = gdata->cfg_runtime;
                req_data = cfg_runtime;
            }
            if (cfg_saved){
                js_data_zone = gdata->cfg_saved;
                req_data = cfg_saved;
            }
            if (cfg_ram){
                js_data_zone = gdata->cfg_ram;
                req_data = cfg_ram;
            }
            t = js_get_js(js_data_zone, req_data);
            if ((n = t->parent) == NULL){
                datapkt = set_pkt_status_data(STATUS_ACT_NOT_FOUND, NULL);
                js_write(sockfd, datapkt);
                js_free(datapkt);
                goto fmterr;
            }
            json_remove_from_parent(t);
            json_append_element(n, t);
            datapkt = set_pkt_status_data(STATUS_OK, NULL);
            sedbg("source [%s] type:[%d], response:\n", source, type);
            sedbg_js(datapkt);
            js_write(sockfd, datapkt);
            js_free(datapkt);
            break;
        case CMD_RECONF:
            js_free(gdata->cfg_saved);
            gdata->cfg_saved =  js_dup(gdata->cfg_runtime);
            datapkt = set_pkt_status_data(STATUS_OK, NULL);
            sedbg("source [%s] type:[%d], response:\n", source, type);
            sedbg_js(datapkt);
            js_write(sockfd, datapkt);
            js_free(datapkt);
            break;
        case CMD_COMMIT:
            sprintf(cfg_path, "%s/%s", gdata->storage_path, "cfg_saved.json");
            sedbg("commit to path:%s\n", cfg_path);
            js_to_file_hr(gdata->cfg_saved, cfg_path);
            datapkt = set_pkt_status_data(STATUS_OK, NULL);
            sedbg("source [%s] type:[%d], response:\n", source, type);
            sedbg_js(datapkt);
            js_write(sockfd, datapkt);
            js_free(datapkt);
            break;
        case CMD_TREEVIEW:
            if (cfg_runtime){
                js_data_zone = gdata->cfg_runtime;
                req_data = cfg_runtime;
            }
            if (cfg_saved){
                js_data_zone = gdata->cfg_saved;
                req_data = cfg_saved;
            }
            if (cfg_ram){
                js_data_zone = gdata->cfg_ram;
                req_data = cfg_ram;
            }
            sedbg_js(json_first_child(req_data) == NULL ? js_data_zone: js_get_js(js_data_zone, req_data));
            datapkt = set_pkt_status_data(STATUS_OK, NULL);
            sedbg("source [%s] type:[%d], response:\n", source, type);
            sedbg_js(datapkt);
            js_write(sockfd, datapkt);
            js_free(datapkt);
            break;
        case CMD_ACTION:
            {
            JsonNode *jx, *cx, *action;
            action_t *act;
            action_data_t action_data;
            if ((jx = js_get_path(req, "Data/Action")) == NULL){
			    datapkt = set_pkt_status_data(STATUS_FAIL, NULL);
                js_write(sockfd, datapkt);
                js_free(datapkt);
                goto fmterr;
            }
            memset(&action_data, 0, sizeof(action_data_t));
            action_data.name = js_get_path_str(jx, "name");
            action_data.args = js_get_path_str(jx, "command");
            action_data.delay = js_get_path_int(jx, "delay");
            action_data.path_key = action_get_path_key();
            action_data.gdata = (void *)gdata;
            
            act = find_action_by_name(gdata->action, action_data.name);
            if (!act){
                datapkt = set_pkt_status_data(STATUS_ACT_NOT_FOUND, NULL);
                js_write(sockfd, datapkt);
                js_free(datapkt);
                goto fmterr;
            }
            act->action_func((void *)&action_data);

            act_data =  js_parse_str("{\"Action\":{}}");
            if (act->type == ACT_QUICK){
                snprintf(path, sizeof(path), "Action[key=%d]", action_data.path_key);
                t = js_get_path(gdata->cfg_ram, path);
                js_join(js_get_path(act_data,"Action"), t);

                datapkt = set_pkt_status_data(STATUS_OK, act_data);
                sedbg("source [%s] type:[%d], response:\n", source, type);
                sedbg_js(datapkt);
                js_write(sockfd, datapkt);
                // free path_key data
                js_free(t);
            }
            else { // ACT_NORMAL
                js_set_path_str(act_data, "Action/name", action_data.name);
                js_set_path_int(act_data, "Action/resultPath", action_data.path_key);

                datapkt = set_pkt_status_data(STATUS_OK, act_data);
                sedbg("source [%s] type:[%d], response:\n", source, type);
                sedbg_js(datapkt);
                js_write(sockfd, datapkt);
                // free path_key data
                snprintf(path, sizeof(path), "Action[key=%d]", action_data.path_key);
		add_timer(&(mdata->timer_pool), 40, 0, js_remove_js, js_get_path(gdata->cfg_ram, path), 0);
            }
            js_free(act_data);
            js_free(datapkt);
            break;
            }
        case CMD_ACTIONS:
            {
            JsonNode *jx, *cx, *action, *j;
            action_t *act;
            action_data_t action_data;
	    int path_key, action_count = 0, fail_action_count=0;
            if ((jx = js_get_path(req, "Data/Actions")) == NULL){
                datapkt = set_pkt_status_data(STATUS_FAIL, NULL);
                js_write(sockfd, datapkt);
                js_free(datapkt);
                goto fmterr;
            }
            path_key = action_get_path_key();
            json_foreach(j, jx){
                action_count++;
                memset(&action_data, 0, sizeof(action_data_t));
                action_data.name = js_get_path_str(j, "name");
                action_data.args = js_get_path_str(j, "command");
                action_data.delay = js_get_path_int(j, "delay");
                action_data.path_key = path_key;
                action_data.gdata = (void *)gdata;
                act = find_action_by_name(gdata->action, action_data.name);
                if (act)
                    act->action_func((void *)&action_data);
                else
                    fail_action_count++;
            }
            if (fail_action_count > 0 && fail_action_count == action_count){
                datapkt = set_pkt_status_data(STATUS_ACT_NOT_FOUND, NULL);
                js_write(sockfd, datapkt);
                js_free(datapkt);
                goto fmterr;
            }
            // set information to result
            memset(path, 0, sizeof(path));
            snprintf(path, sizeof(path), "Actions[key=%d]", path_key);
            t = js_set_path(gdata->cfg_ram, path);

            js_set_path_str(t, "source", source);
            js_set_path_int(t, "status", 0);  // status 0 not ready

            act_data =  js_parse_str("{\"Actions\":{}}");
            js_set_path_int(act_data, "Actions/resultPath", path_key);
            if (fail_action_count > 0)
                datapkt = set_pkt_status_data(STATUS_ACT_NOT_FOUND_PARTIAL, act_data);
            else
                datapkt = set_pkt_status_data(STATUS_OK, act_data);

            sedbg("source [%s] type:[%d], response:\n", source, type);
            sedbg_js(datapkt);
            js_write(sockfd, datapkt);

            // free path_key data
            add_timer(&(mdata->timer_pool), 40, 0, js_remove_js, t, 0);

            js_free(act_data);
            js_free(datapkt);
            break;
            }
        case CMD_LISTACTION:
            traverse_action(gdata->action);
            datapkt = set_pkt_status_data(STATUS_OK, NULL);
            js_write(sockfd, datapkt);
            js_free(datapkt);
            break;
        default:
            goto fmterr;
    }
    js_free(req);
    return;
fmterr: 
    sedbg("pkt format parse error\n");
    js_free(req); 
	return;
}
