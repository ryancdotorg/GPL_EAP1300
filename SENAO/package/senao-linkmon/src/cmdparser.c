/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu @ SenaoNetworks - Taiwan                                    *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                     *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  linkmon                                                                       *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#include <stdio.h>
#include <string.h>
#include "cmdparser.h"
#include "ipc_udp.h"
#include "json.h"
#include "util.h"

int getconnectivity(json_object *dataobj, struct linkmon_t *linkmon, json_object *res_dataobj)
{
	int i;

	if (dataobj == NULL || res_dataobj == NULL){
		lmdbg("invalid data\n");
		return -1;
	}
	//lmdbg("data obj=%s\n", json_object_to_json_string(dataobj));

	json_object *devstatus_obj = json_object_object_get(dataobj, "DeviceStatus");
	if (devstatus_obj == NULL){
		lmdbg("invalid device status\n");
		return -1;
	}
	json_object *res_devstatus_obj = json_object_new_object(); 
	json_object_object_add(res_dataobj, "DeviceStatus", res_devstatus_obj);


	json_object *conn_objs = json_object_object_get(devstatus_obj, "Connectivity");
	json_object *conn_obj = NULL;
	if (conn_objs == NULL){
		lmdbg("invalid connectivity status\n");
		return -1;
	}
	json_object *res_conn_objs = json_object_new_array();
	json_object_object_add(res_devstatus_obj, "Connectivity", res_conn_objs);
	json_object *conn_i;
	int arrLen = json_object_array_length(conn_objs);

	if (arrLen > 0) { // get specific protocols
		for (i=0; i < arrLen; i++){
			conn_obj = json_object_array_get_idx(conn_objs, i);
			const char *proto, *dest;
			conn_i = json_object_new_object();
			json_object_object_foreach(conn_obj,key,val){
				if(!strcmp(key,"protocol")){
					proto = json_object_get_string(val);
					json_object_object_add(conn_i, key, val);
				}
			//	if(!strcmp(key,"destination")){
			//		dest = json_object_get_string(val);
			//		json_object_object_add(conn_i, key, val);
			//	}
			}
			struct linkmon_t *ptr;
			list_for_each_entry(ptr, &(linkmon->list),list) {
				if (!strcmp(ptr->name, proto)){
					if (ptr->lm_statusfunc && ptr->data){
						lmdbg("status:[%d]\n", ptr->lm_statusfunc(ptr->data));
						json_object_object_add(conn_i, "status", json_object_new_int(ptr->lm_statusfunc(ptr->data)));
						json_object_object_add(conn_i, "msg", json_object_new_string(ptr->lm_msgfunc(ptr->data)));
						break;
					}
				}
			}
			json_object_array_add(res_conn_objs,conn_i);
		}
	}
	else { // get all protocols
		struct linkmon_t *ptr;
		list_for_each_entry(ptr, &(linkmon->list),list) {
			if (ptr->lm_statusfunc && ptr->data){
				if (ptr->name){
					conn_i = json_object_new_object();
					json_object_object_add(conn_i, "protocol", json_object_new_string(ptr->name));
					json_object_object_add(conn_i, "status", json_object_new_int(ptr->lm_statusfunc(ptr->data)));
					json_object_object_add(conn_i, "msg", json_object_new_string(ptr->lm_msgfunc(ptr->data)));
					json_object_array_add(res_conn_objs,conn_i);
					lmdbg("protocol:[%s] status:[%d] msg:[%s]\n", ptr->name,ptr->lm_statusfunc(ptr->data), ptr->lm_msgfunc(ptr->data));
				}
				//break;
			}
		}
	}
}
int handlecmd(json_object *cmdobj, struct linkmon_t *linkmon, json_object *res)
{
	int ret=0;
	if(cmdobj == NULL || res == NULL){
		lmdbg("nothing to do\n");
		return -1;
	}
	const char *type;
	json_object *dataobj = NULL, *res_dataobj = NULL;
	json_object_object_foreach(cmdobj,key,val){
		if(!strcmp(key,"Type")){
			type = json_object_get_string(val);
			json_object_object_add(res, key, val);
		}
		if(!strcmp(key,"Data")){
			dataobj = val;
			res_dataobj = json_object_new_object();
			json_object_object_add(res, key, res_dataobj);
		}
	}
	
	if (!strcmp(type, "GetConnectivity") && dataobj != NULL){
		ret = getconnectivity(dataobj, linkmon, res_dataobj);
	}
	return ret;
}

void cmdparser(void *udata, void *data)
{
	json_object *rootobj = NULL, *p;
	json_object *resobj = NULL;
	enum json_type type;
	int len = 0;
	int i;
	if (udata == NULL || data == NULL){
		lmdbg("invalid data\n");
		return;
	}
	struct udp_data_t *udpdata = (struct udp_data_t *)udata;
	struct linkmon_t *linkmon = (struct linkmon_t *)udpdata->linkmon;
	*(udpdata->terminate_ignore) = 1; // extend terminate time

	len = strlen(data);
	rootobj = json_tokener_parse(data);
	resobj = json_object_new_object(); 
	//lmdbg("%s(%d) got data=%s\n",__func__,__LINE__,json_object_to_json_string(rootobj));
	
	json_object *resobjs = json_object_new_array();
	json_object_object_add(resobj, "Response", resobjs);
	json_object *res_i;
	json_object_object_foreach(rootobj,key,val){
		type = json_object_get_type(val);
		if (!strcmp(key, "Commands")){
			json_object *cmdobjs = json_object_object_get(rootobj, key);
			json_object *cmdobj;
			int arrLen = json_object_array_length(cmdobjs);
			for (i=0; i< arrLen; i++){
				res_i = json_object_new_object();
				cmdobj = json_object_array_get_idx(cmdobjs, i);
				handlecmd(cmdobj, linkmon, res_i);
				json_object_array_add(resobjs,res_i);
			}

		}
	}
	//lmdbg("%s(%d) receive ok prepare to send\n",__func__,__LINE__);
	const char *sendbuf = json_object_to_json_string(resobj);

	udpdata->ipc_sendfunc((void *)udpdata, (void *)sendbuf);
	lmdbg("new_obj.to_string()=%s\n", json_object_to_json_string_ext(resobj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));

	json_object_put(rootobj);
	json_object_put(resobj);
}

