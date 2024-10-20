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
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "liblmipc.h"

#define SERVER_SOCK "/tmp/linkmon.unix"
#define CLIENT_SOCK_PREFIX "/tmp/linkmon"
#define RETRY 3
ipc_handle_t *ipc_open()
{
	ipc_handle_t *ih;
	pid_t pid = getpid();
	ih = calloc(1, sizeof(ipc_handle_t));
	if (ih == NULL){
		perror("calloc:");
		return NULL;
	}

	if ( (ih->sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1){
		perror("socket:");
		goto openerr;
	}

	(ih->addr_cli).sun_family = AF_UNIX;
	sprintf((ih->addr_cli).sun_path, "%s.%d", CLIENT_SOCK_PREFIX, pid);
	unlink((ih->addr_cli).sun_path);

	if (bind(ih->sockfd, (struct sockaddr *) &(ih->addr_cli), sizeof(ih->addr_cli)) == -1){
		perror("bind:");
		goto openerr;
	}

	(ih->addr_srv).sun_family = AF_UNIX;
	strcpy((ih->addr_srv).sun_path, SERVER_SOCK);
	if (connect(ih->sockfd, (struct sockaddr *)&(ih->addr_srv), sizeof(ih->addr_srv)) == -1){
		perror("connect:");
		unlink((ih->addr_cli).sun_path);
		goto openerr;
	}
	return ih;
openerr:
	free(ih);
	return NULL;
}
int ipc_send(ipc_handle_t *ih, char *bufsend, int slen, char *bufrcv, int rlen)
{
	if (ih == NULL){
		printf("error handler");
		return -1;
	}
	int byteSent, byteReceived;
	int retry = 0;
	byteSent = sendto(ih->sockfd, bufsend, slen, 0, (struct sockaddr *) &(ih->addr_srv), sizeof(ih->addr_srv));

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec= 0;
	if (setsockopt(ih->sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
		perror("Error");
		return -1;
	}
	do {
		if ( (byteReceived = recvfrom (ih->sockfd, bufrcv, rlen, 0,
				(struct sockaddr *) &(ih->addr_srv), &(ih->addr_srv_len)) == -1)){
			perror("recvfrom error:");
			return -1;
		}else {
			break;
		}
		retry++;
	} while(retry < RETRY);
	return 0;
	
}
void ipc_close(ipc_handle_t *ih)
{
	if(ih == NULL)
		return;
	if ((ih->addr_cli).sun_path != NULL)
		unlink((ih->addr_cli).sun_path);
	if (ih->sockfd > 0)
		close(ih->sockfd);
	free(ih);
}


lm_handle_t *lm_open()
{
	lm_handle_t *lh;
	lh = calloc(1, sizeof(lm_handle_t));
	if (lh == NULL){
		perror("calloc:");
		return NULL;
	}
	lh->rdata = calloc(1, MAX_RCV_BUFFER_SIZE);
	lh->rdata_len = MAX_RCV_BUFFER_SIZE;
	lh->ih = ipc_open();
	return lh;
}

void lm_close(lm_handle_t *lh)
{
	if(lh == NULL)
		return;
	ipc_close(lh->ih);
	if (lh->rdata != NULL)
		free(lh->rdata);
	free(lh);
}

//char json_text[] = "{ \"Commands\": [ { \"Type\": \"GetConnectivity\", \"Data\": { \"DeviceStatus\": { \"Connectivity\": [ { \"protocol\": \"icmp\", \"destination\": \"8.8.8.8\" } ] } } } ] }";
char json_text[] = "{ \"Commands\": [ { \"Type\": \"GetConnectivity\", \"Data\": { \"DeviceStatus\": { \"Connectivity\": []} } } ] }";

json_object *json_object_get_path(json_object *jo, char *path)
{
	if (jo == NULL || path == NULL)
		return NULL;
	char buffer[128];
	json_object *subobj = NULL;
	strcpy(buffer, path);

	char *objkey, *subpath=NULL, *start, *end;
	objkey = buffer;
	end = strchr(buffer, '/');
	if (end == NULL){
		
	} else {
		*end = '\0';
		subpath = end+1;
	}
		
	char *s, *e, *idx = NULL, *s2 = NULL, *k = NULL, *v = NULL;
	if ( (s = strchr(objkey, '[')) && (e = strchr(objkey,']'))){ // array
		*s = '\0';
		if ((s2 = strchr(s+1, '='))){ // key value member
			*s2 = '\0';
			k = s+1;
			v = s2+1;
			//printf("key to search:[%s] value to match:[%s]\n", k,v);
		}else{ // index
			idx = s+1;
		}
		*e = '\0';
	}
	//printf("objkey:[%s], subpath:[%s]\n", objkey, subpath);
	struct json_object *arr_obj, *tmp, *key_obj;
	int i;
	json_object_object_foreach(jo,key,val){
		if(!strcmp(key, objkey)){
			if (idx != NULL){ // has index
				subobj = json_object_array_get_idx(val, atoi(idx));
			}else if (k!=NULL && v!=NULL)   { // has sub element key value
				// check what the index is in sub elements, k(ey), v(alue)
				json_object_object_get_ex(jo, key, &arr_obj);

				for (i = 0; i < json_object_array_length(arr_obj); i++) {
					tmp = json_object_array_get_idx(arr_obj, i);
					json_object_object_get_ex(tmp, k, &key_obj);
					//printf("protocol [%d] = %s\n", i,  json_object_get_string(key_obj));
					//printf("v:[%s] key_obj:[%s]\n", v,  json_object_get_string(key_obj));
					if (!strcmp(v, json_object_get_string(key_obj))){
						subobj = tmp;
						break;
					}
				}

			}
			else // no index
				subobj = val;
		}
	}	
	if (subobj == NULL)
		return NULL; // not found
	if(subpath != NULL)
		return json_object_get_path(subobj, subpath);
	else
		return subobj;
}	

lm_handle_t *lm_getobj(lm_handle_t *lh, char *path)
{
	if (lh == NULL || path == NULL)
		return NULL;
//	if (lh->robj == NULL) TODO: save bandwitdh
	if (ipc_send(lh->ih, json_text, sizeof(json_text), lh->rdata, lh->rdata_len) != 0)
		return NULL;
	lh->robj = json_tokener_parse(lh->rdata);
	json_object *query = NULL, *result = NULL;
	// TODO: just move down, should back to root?
	if (lh->subobj)
		query = lh->subobj;
	else
		query = lh->robj;
	
	result = json_object_get_path(query, path);

	json_object_put(lh->subobj);
	lh->subobj = result;
	return lh;
}

json_object *lm_getjobj(lm_handle_t *lh, char *path)
{
	lm_handle_t *res = lm_getobj(lh, path);
	if (res == NULL) 
		return NULL;
	return res->subobj;
}

int lm_getint(lm_handle_t *lh, char *path)
{
	lm_handle_t *res;
	res = lm_getobj(lh, path);
//	printf("new_obj.to_string()=%s\n", json_object_to_json_string(res->subobj));
	if (res->subobj)
		return json_object_get_int(res->subobj);
	else 
		return -1;
}
int lm_getbool(lm_handle_t *lh, char *path)
{
	lm_handle_t *res;
	res = lm_getobj(lh, path);
//	printf("new_obj.to_string()=%s\n", json_object_to_json_string(res->subobj));
	if (res->subobj)
		return json_object_get_boolean(res->subobj);
	else 
		return -1;
}
void lm_show_obj(lm_handle_t *lh, char *path)
{
	lm_handle_t *res;
	int val_type;
	res = lm_getobj(lh, path);
	if (res == NULL){
		return;
	}
	val_type = json_object_get_type(res->subobj);
	switch (val_type) {
		case json_type_null:
			printf("null\n");
			break;
		case json_type_boolean:
			printf("%d\n",json_object_get_boolean(res->subobj));
			break;
		case json_type_double:
			printf("%f\n",json_object_get_double(res->subobj));
			break;
		case json_type_int:
			printf("%d\n",json_object_get_int(res->subobj));
			break;
		case json_type_string:
			printf("%s\n",json_object_get_string(res->subobj));
			break;
		case json_type_object:
			break;
		case json_type_array:	
			break;
	}
}
