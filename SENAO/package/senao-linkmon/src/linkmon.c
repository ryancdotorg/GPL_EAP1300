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
#include "linkmon.h"
#include <stdlib.h>
#include <stdio.h>
#include "util.h"

linkmon_t *linkmon_init()
{
	linkmon_t *linkmon;
	linkmon = (linkmon_t *)calloc(1, sizeof(linkmon_t));
	INIT_LIST_HEAD(&(linkmon->list));
	return linkmon;
}
void linkmon_uninit(linkmon_t *linkmon_pool)
{
	if (linkmon_pool == NULL){
		lmdbg("linkmon not initialized\n");
		return;
	}
	linkmon_t *ptr;
	while(!list_empty(&(linkmon_pool->list))) {
		ptr = list_entry((linkmon_pool->list).next, linkmon_t, list);	
		// free what alloc in linkmon_init
		//
		list_del(&(ptr->list));	
		free(ptr);
	}
	free(linkmon_pool);
}
linkmon_t *add_linkmon(linkmon_t *linkmon_pool, char *name, unsigned int sec, unsigned int usec, lm_initfunc init_func, lm_sendfunc send_func, lm_recvfunc recv_func, lm_statusfunc status_func, lm_msgfunc msg_func, lm_uninitfunc uninit_func, void *data)
{
	linkmon_t *tmp;
	tmp = (linkmon_t *)calloc(1, sizeof(linkmon_t));
	if(!tmp) {
		perror("calloc");
		return NULL;
	}
	strcpy(tmp->name, name);
	tmp->sec = sec;
	tmp->usec = usec;
	tmp->lm_initfunc = init_func;
	tmp->lm_sendfunc = send_func;
	tmp->lm_recvfunc = recv_func;
	tmp->lm_statusfunc = status_func;
	tmp->lm_msgfunc = msg_func;
	tmp->lm_uninitfunc = uninit_func;
	tmp->data = data;

	list_add_tail( &(tmp->list), &(linkmon_pool->list));
	return tmp;

}
void remove_linkmon(linkmon_t *linkmon_pool, linkmon_t *linkmon)
{
	if (linkmon_pool == NULL || linkmon == NULL) return;

	linkmon_t *ptr;
	list_for_each_entry(ptr, &(linkmon_pool->list),list) {
		if (!strcmp(ptr->name, linkmon->name)){
			// free what alloc in add_linkmon
			//
			list_del(&(ptr->list));	
			free(ptr);
			break;
		}
	}
}
linkmon_t *find_linkmon_by_name(linkmon_t *linkmon_pool, char *name)
{
	if (linkmon_pool == NULL) 
		return NULL;

	linkmon_t *ptr;
	list_for_each_entry(ptr, &(linkmon_pool->list),list) {
		if (!strcmp(ptr->name, name)){
			return ptr;
		}
	}
	return NULL;
}
void traverse_linkmon(linkmon_t *linkmon)
{
	linkmon_t *ptr;
	list_for_each_entry(ptr, &(linkmon->list),list) {
		lmdbg("traverse_linkmon - name:[%s] fd:[%d] sec:[%u] usec:[%u] data:[%p] ", ptr->name, ptr->fd, ptr->sec, ptr->usec, ptr->data);
		if (ptr->lm_statusfunc && ptr->data)
			lmdbg("status:[%d]\n", ptr->lm_statusfunc(ptr->data));
		else
			lmdbg("\n");
	}
}
/*
int main(int argc, char *argv[])
{
	linkmon_t *linkmon, *linkmon2, *linkmon3, *linkmon4;
	linkmon = linkmon_init();
	linkmon2 = add_linkmon(linkmon, "icmpv4", 0, NULL, NULL, NULL, NULL, NULL);
	linkmon3 = add_linkmon(linkmon, "http", 0, NULL, NULL, NULL, NULL, NULL);
	linkmon4 = add_linkmon(linkmon, "dns", 0, NULL, NULL, NULL, NULL, NULL);
	traverse_linkmon(linkmon);
	printf("============\n");
	remove_linkmon(linkmon, linkmon2);
	traverse_linkmon(linkmon);
	printf("============\n");
	remove_linkmon(linkmon, linkmon3);
	traverse_linkmon(linkmon);
	printf("============\n");
	remove_linkmon(linkmon, linkmon4);
	traverse_linkmon(linkmon);

}
*/
