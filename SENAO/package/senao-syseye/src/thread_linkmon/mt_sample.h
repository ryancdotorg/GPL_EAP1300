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
#ifndef _MT_SAMPLE_H_
#define _MT_SAMPLE_H_
#include "montask.h"
typedef struct mt_sample_data_t {
	char name[16];
	unsigned int sec;
	unsigned int usec;
    void *mdata;
} mt_sample_data_t;

typedef struct mt_sample_t {
	char name[16];
	unsigned int sec;
	unsigned int usec;
	int (*mt_init_func)(void *mt_sample_data);
    void (*mt_periodic_func)(void *mio_data, void *mt_sample_data); // mio timer function
	void (*mt_recv_func)(void *);	
	void (*mt_uninit_func)(void *);
} mt_sample_t;

void init_mt_sample(montask_t *montask_pool, void *mdata);
void uninit_mt_sample(montask_t *montask_pool);
#endif
