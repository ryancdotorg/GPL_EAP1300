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
#include <stdlib.h>
#include <stdio.h>
#include "montask.h"

montask_t *montask_init()
{
	montask_t *montask;
	montask = (montask_t *)calloc(1, sizeof(montask_t));
	INIT_LIST_HEAD(&(montask->list));
	return montask;
}
void montask_uninit(montask_t *montask_pool)
{
	if (montask_pool == NULL){
		printf("montask not initialized\n");
		return;
	}
	montask_t *ptr;
	while(!list_empty(&(montask_pool->list))) {
		ptr = list_entry((montask_pool->list).next, montask_t, list);	
		// free what alloc in montask_init
		//
		list_del(&(ptr->list));	
		free(ptr);
	}
	free(montask_pool);
}
montask_t *add_montask(montask_t *montask_pool, char *name, unsigned int sec, unsigned int usec, mt_init_func init_func, mt_periodic_func periodic_func, mt_recv_func recv_func, mt_uninit_func uninit_func, void *data)
{
	montask_t *tmp;
	tmp = (montask_t *)calloc(1, sizeof(montask_t));
	if(!tmp) {
		perror("calloc");
		return NULL;
	}
	strcpy(tmp->name, name);
	tmp->sec = sec;
	tmp->usec = usec;
	tmp->mt_init_func = init_func;
	tmp->mt_periodic_func = periodic_func;
	tmp->mt_recv_func = recv_func;
	tmp->mt_uninit_func = uninit_func;
	tmp->data = data;

	list_add_tail( &(tmp->list), &(montask_pool->list));
	return tmp;

}
void remove_montask(montask_t *montask_pool, montask_t *montask)
{
    //TODO: remote it directly, no need to check name
	if (montask_pool == NULL || montask == NULL) return;

	montask_t *ptr;
	list_for_each_entry(ptr, &(montask_pool->list),list) {
		if (!strcmp(ptr->name, montask->name)){
			// free what alloc in add_montask
			//
			list_del(&(ptr->list));	
			free(ptr);
			break;
		}
	}
}
montask_t *find_montask_by_name(montask_t *montask_pool, char *name)
{
	if (montask_pool == NULL) 
		return NULL;

	montask_t *ptr;
	list_for_each_entry(ptr, &(montask_pool->list),list) {
		if (!strcmp(ptr->name, name)){
			return ptr;
		}
	}
	return NULL;
}
void traverse_montask(montask_t *montask)
{
	montask_t *ptr;
	list_for_each_entry(ptr, &(montask->list),list) {
		printf("traverse_montask - name:[%s] sec:[%u] usec:[%u] data:[%p]\n", ptr->name, ptr->sec, ptr->usec, ptr->data);
	}
}
/*
int main(int argc, char *argv[])
{
	montask_t *montask, *montask2, *montask3, *montask4;
	montask = montask_init();
	montask2 = add_montask(montask, "icmpv4", 0, NULL, NULL, NULL, NULL, NULL);
	montask3 = add_montask(montask, "http", 0, NULL, NULL, NULL, NULL, NULL);
	montask4 = add_montask(montask, "dns", 0, NULL, NULL, NULL, NULL, NULL);
	traverse_montask(montask);
	printf("============\n");
	remove_montask(montask, montask2);
	traverse_montask(montask);
	printf("============\n");
	remove_montask(montask, montask3);
	traverse_montask(montask);
	printf("============\n");
	remove_montask(montask, montask4);
	traverse_montask(montask);

}
*/
