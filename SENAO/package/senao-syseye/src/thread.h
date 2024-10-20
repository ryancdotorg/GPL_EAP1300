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
#ifndef _THREAD_H_
#define _THREAD_H_
#include <pthread.h>
#include <string.h>
#include "list.h"

typedef struct thread_t {
	struct list_head list;
	char name[15];
	void *(*thread_start_func)(void *);
    pthread_t thread_id;
	void *data;
}thread_t;

typedef void *(*thread_start_func)(void *);

thread_t *thread_init(void);
void thread_uninit(thread_t *thread_pool);
thread_t *add_thread(thread_t *thread_pool, char *name, thread_start_func tstart, void *data);
void remove_thread(thread_t *thread_pool, thread_t *thread);
thread_t *find_thread_by_name(thread_t *thread_pool, char *name);
void traverse_thread(thread_t *thread);
#endif
