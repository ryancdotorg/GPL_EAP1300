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
#include "action.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>

action_t *action_init()
{
	action_t *action;
	action = (action_t *)calloc(1, sizeof(action_t));
	INIT_LIST_HEAD(&(action->list));
	return action;
}
void action_uninit(action_t *action_pool)
{
	if (action_pool == NULL){
		printf("action not initialized\n");
		return;
	}
	action_t *ptr;
	while(!list_empty(&(action_pool->list))) {
		ptr = list_entry((action_pool->list).next, action_t, list);	
		// free what alloc in linkmon_init
		//
		list_del(&(ptr->list));	
		free(ptr);
	}
	free(action_pool);
}
action_t *add_action(action_t *action_pool, char *name, action_func func, unsigned int type)
{
	action_t *tmp;
	tmp = (action_t *)calloc(1, sizeof(action_t));
	if(!tmp) {
		perror("calloc");
		return NULL;
	}
	strcpy(tmp->name, name);
	tmp->action_func = func;
	tmp->type = type;

	list_add_tail( &(tmp->list), &(action_pool->list));
	return tmp;

}
void remove_action(action_t *action)
{
	if (action == NULL) return;
    list_del(&(action->list));
    free(action);
    return;
}
void remove_action_by_name(action_t *action_pool, char *name)
{
	if (action_pool == NULL || name == NULL) return;

	action_t *ptr;
	list_for_each_entry(ptr, &(action_pool->list),list) {
		if (!strcmp(ptr->name, name)){
			// free what alloc in add_action
			//
			list_del(&(ptr->list));	
			free(ptr);
			break;
		}
	}
}
action_t *find_action_by_name(action_t *action_pool, char *name)
{
	if (action_pool == NULL) 
		return NULL;

	action_t *ptr;
	list_for_each_entry(ptr, &(action_pool->list),list) {
		if (!strcmp(ptr->name, name)){
			return ptr;
		}
	}
	return NULL;
}
void traverse_action(action_t *action)
{
	action_t *ptr;
	list_for_each_entry(ptr, &(action->list),list) {
		sedbg("supported action - type:[%u] name:[%s] \n", ptr->type, ptr->name);
	}
}
int action_get_path_key()
{
    static int path_key=0;
	path_key = (path_key==65535)?0:path_key+1;
	return path_key;
}

/*
int main(int argc, char *argv[])
{
	action_t *action, *action2, *action3, *action4;
	action = action_init();
	action2 = add_action(action, "ipc_udp", "/tmp/linkmon.unix", NULL, NULL, NULL, NULL, NULL);
	action3 = add_action(action, "ipc_tcp", "1024", NULL, NULL, NULL, NULL, NULL);
	action4 = add_action(action, "ipc_pipe", "/tmp/linkmon.pipe", NULL, NULL, NULL, NULL, NULL);
	traverse_action(action);
	printf("============\n");
	remove_action(action, action2);
	traverse_action(action);
	printf("============\n");
	remove_action(action, action3);
	traverse_action(action);
	printf("============\n");
	remove_action(action, action4);
	traverse_action(action);

}
*/
