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
#ifndef _ACTION_H_
#define _ACTION_H_
#include <string.h>
#include "action_data.h"
#include "list.h"

typedef struct action_t {
	struct list_head list;
    unsigned int type; // quick action, normal action
	char name[32];
	void *(*action_func)(void *args);
}action_t;

typedef enum {
   ACT_QUICK = 0,
   ACT_NORMAL,
   ACT_NORET
}action_type_t;

/*
 * action information get function
 *
 * */
struct action_args_t {
    char *cmd;
    char *args;
    int path_key;
};

typedef void *(*action_func)(void *args);

action_t *action_init(void);
void action_uninit(action_t *action_pool);
action_t *add_action(action_t *action_pool, char *name, action_func func, unsigned int type);
void remove_action_by_name(action_t *action_pool, char *name);
action_t *find_action_by_name(action_t *action_pool, char *name);
void traverse_action(action_t *action);
int action_get_path_key();
#endif
