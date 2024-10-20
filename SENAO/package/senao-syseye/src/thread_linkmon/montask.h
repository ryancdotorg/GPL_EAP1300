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
#ifndef _MONTASK_H_
#define _MONTASK_H_
#include <string.h>
#include "list.h"
typedef struct montask_t {
	struct list_head list;
	char name[15];
	unsigned int sec;
	unsigned int usec;
	int (*mt_init_func)(void *); // xxx_data_t
	void (*mt_periodic_func)(void *mio_data, void *data);
	void (*mt_recv_func)(void *);
	void (*mt_uninit_func)(void *);
    void *mt_timer;
	void *data;
}montask_t;

typedef int (*mt_init_func)(void *); // xxx_data_t 
typedef void (*mt_periodic_func)(void *, void *); 
typedef void (*mt_recv_func)(void *); 
typedef void (*mt_uninit_func)(void *); 

montask_t *montask_init(void);
void montask_uninit(montask_t *montask_pool);
montask_t *add_montask(montask_t *montask_pool, char *name, unsigned int sec, unsigned int usec, mt_init_func init_func, mt_periodic_func periodic_func, mt_recv_func recv_func, mt_uninit_func uninit_func, void *data);
void remove_montask(montask_t *montask_pool, montask_t *montask);
montask_t *find_montask_by_name(montask_t *montask_pool, char *name);
void traverse_montask(montask_t *montask);
#endif
