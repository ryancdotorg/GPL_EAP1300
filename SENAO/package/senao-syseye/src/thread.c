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
#include "thread.h"
#include <stdlib.h>
#include <stdio.h>

thread_t *thread_init()
{
	thread_t *thread;
	thread = (thread_t *)calloc(1, sizeof(thread_t));
	INIT_LIST_HEAD(&(thread->list));
	return thread;
}
void thread_uninit(thread_t *thread_pool)
{
	if (thread_pool == NULL){
		printf("thread not initialized\n");
		return;
	}
	thread_t *ptr;
	while(!list_empty(&(thread_pool->list))) {
		ptr = list_entry((thread_pool->list).next, thread_t, list);	
		// free what alloc in linkmon_init
		//
		list_del(&(ptr->list));	
		free(ptr);
	}
	free(thread_pool);
}
thread_t *add_thread(thread_t *thread_pool, char *name, thread_start_func tstart, void *data)
{
	thread_t *tmp;
	tmp = (thread_t *)calloc(1, sizeof(thread_t));
	if(!tmp) {
		perror("calloc");
		return NULL;
	}
	strcpy(tmp->name, name);
	tmp->thread_start_func = tstart;
	tmp->data = data;

	list_add_tail( &(tmp->list), &(thread_pool->list));
	return tmp;

}
void remove_thread(thread_t *thread_pool, thread_t *thread)
{
	if (thread_pool == NULL || thread == NULL) return;

	thread_t *ptr;
	list_for_each_entry(ptr, &(thread_pool->list),list) {
		if (!strcmp(ptr->name, thread->name)){
			// free what alloc in add_thread
			//
			list_del(&(ptr->list));	
			free(ptr);
			break;
		}
	}
}
thread_t *find_thread_by_name(thread_t *thread_pool, char *name)
{
	if (thread_pool == NULL) 
		return NULL;

	thread_t *ptr;
	list_for_each_entry(ptr, &(thread_pool->list),list) {
		if (!strcmp(ptr->name, name)){
			return ptr;
		}
	}
	return NULL;
}
void traverse_thread(thread_t *thread)
{
	thread_t *ptr;
	list_for_each_entry(ptr, &(thread->list),list) {
		printf("traverse_thread - name:[%s] data:[%p]\n", ptr->name, ptr->data);
	}
}
/*
int main(int argc, char *argv[])
{
	thread_t *thread, *thread2, *thread3, *thread4;
	thread = thread_init();
	thread2 = add_thread(thread, "ipc_udp", "/tmp/linkmon.unix", NULL, NULL, NULL, NULL, NULL);
	thread3 = add_thread(thread, "ipc_tcp", "1024", NULL, NULL, NULL, NULL, NULL);
	thread4 = add_thread(thread, "ipc_pipe", "/tmp/linkmon.pipe", NULL, NULL, NULL, NULL, NULL);
	traverse_thread(thread);
	printf("============\n");
	remove_thread(thread, thread2);
	traverse_thread(thread);
	printf("============\n");
	remove_thread(thread, thread3);
	traverse_thread(thread);
	printf("============\n");
	remove_thread(thread, thread4);
	traverse_thread(thread);

}
*/
