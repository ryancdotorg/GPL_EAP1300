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
#include "cmdipc.h"
#include <stdlib.h>
#include <stdio.h>
#include "util.h"

cmdipc_t *cmdipc_init()
{
	cmdipc_t *cmdipc;
	cmdipc = (cmdipc_t *)calloc(1, sizeof(cmdipc_t));
	INIT_LIST_HEAD(&(cmdipc->list));
	return cmdipc;
}
void cmdipc_uninit(cmdipc_t *cmdipc_pool)
{
	if (cmdipc_pool == NULL){
		lmdbg("cmdipc not initialized\n");
		return;
	}
	cmdipc_t *ptr;
	while(!list_empty(&(cmdipc_pool->list))) {
		ptr = list_entry((cmdipc_pool->list).next, cmdipc_t, list);	
		// free what alloc in linkmon_init
		//
		list_del(&(ptr->list));	
		free(ptr);
	}
	free(cmdipc_pool);
}
cmdipc_t *add_cmdipc(cmdipc_t *cmdipc_pool, char *name, char *access_point, ipc_sockfunc sock_func, ipc_recvfunc recv_func, ipc_sendfunc send_func, ipc_closesock_func closesock_func, ipc_cmdfunc cmd_func, void *data)
{
	cmdipc_t *tmp;
	tmp = (cmdipc_t *)calloc(1, sizeof(cmdipc_t));
	if(!tmp) {
		perror("calloc");
		return NULL;
	}
	strcpy(tmp->name, name);
	strcpy(tmp->access_point, access_point);
	tmp->ipc_sockfunc = sock_func;
	tmp->ipc_recvfunc = recv_func;
	tmp->ipc_sendfunc = send_func;
	tmp->ipc_closesockfunc = closesock_func;
	tmp->ipc_cmdfunc = cmd_func;
	tmp->data = data;

	list_add_tail( &(tmp->list), &(cmdipc_pool->list));
	return tmp;

}
void remove_cmdipc(cmdipc_t *cmdipc_pool, cmdipc_t *cmdipc)
{
	if (cmdipc_pool == NULL || cmdipc == NULL) return;

	cmdipc_t *ptr;
	list_for_each_entry(ptr, &(cmdipc_pool->list),list) {
		if (!strcmp(ptr->name, cmdipc->name)){
			// free what alloc in add_cmdipc
			//
			list_del(&(ptr->list));	
			free(ptr);
			break;
		}
	}
}
cmdipc_t *find_cmdipc_by_name(cmdipc_t *cmdipc_pool, char *name)
{
	if (cmdipc_pool == NULL) 
		return NULL;

	cmdipc_t *ptr;
	list_for_each_entry(ptr, &(cmdipc_pool->list),list) {
		if (!strcmp(ptr->name, name)){
			return ptr;
		}
	}
	return NULL;
}
void traverse_cmdipc(cmdipc_t *cmdipc)
{
	cmdipc_t *ptr;
	list_for_each_entry(ptr, &(cmdipc->list),list) {
		lmdbg("traverse_cmdipc - name:[%s] fd:[%d] access_point:[%s] data:[%p]\n", ptr->name, ptr->fd, ptr->access_point, ptr->data);
	}
}
/*
int main(int argc, char *argv[])
{
	cmdipc_t *cmdipc, *cmdipc2, *cmdipc3, *cmdipc4;
	cmdipc = cmdipc_init();
	cmdipc2 = add_cmdipc(cmdipc, "ipc_udp", "/tmp/linkmon.unix", NULL, NULL, NULL, NULL, NULL);
	cmdipc3 = add_cmdipc(cmdipc, "ipc_tcp", "1024", NULL, NULL, NULL, NULL, NULL);
	cmdipc4 = add_cmdipc(cmdipc, "ipc_pipe", "/tmp/linkmon.pipe", NULL, NULL, NULL, NULL, NULL);
	traverse_cmdipc(cmdipc);
	printf("============\n");
	remove_cmdipc(cmdipc, cmdipc2);
	traverse_cmdipc(cmdipc);
	printf("============\n");
	remove_cmdipc(cmdipc, cmdipc3);
	traverse_cmdipc(cmdipc);
	printf("============\n");
	remove_cmdipc(cmdipc, cmdipc4);
	traverse_cmdipc(cmdipc);

}
*/
