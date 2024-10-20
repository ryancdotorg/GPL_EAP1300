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
#include <stdio.h>
#include <dirent.h> 
#include <sys/stat.h>
//#include <string.h>
//#include <stdlib.h>
#include <unistd.h>
#include <mio.h>
#include <global.h>
#include "util.h"
#include "mt_sample.h"

static int mt_init(void *data);
static void mt_periodic(void *mio_data, void *mt_sample_data);
static void mt_uninit(void *data);
static void mt_recv(void *data);

static mt_sample_t mt_sample =
{
	.name = "sample",
	.sec = 5,
	.usec = 0,
	.mt_init_func = mt_init,
	.mt_periodic_func = mt_periodic,
	.mt_recv_func = mt_recv,
    .mt_uninit_func = mt_uninit,
};

static int mt_init(void *data)
{   
    mt_sample_data_t *mt_data;
	if (data == NULL){
		printf("%s(%d) init error\n", __func__, __LINE__);
		return -1;
	}
    mt_data = (mt_sample_data_t *)data;
    mio_data_t *mdata = (mio_data_t *)mt_data->mdata;
}
static void mt_periodic(void *mio_data,  void *data)
{
    mt_sample_data_t *mt_data;
	if (data == NULL){
		printf("phylink_close, data not initialized\n");
		return;
	}
    mt_data = (mt_sample_data_t *)data;
    mio_data_t *mdata = (mio_data_t *)mio_data;
    sedbg("periodic function\n");
}

static void mt_uninit(void *data)
{
    mt_sample_data_t *mt_data;
	if (data == NULL){
		printf("phylink_close, data not initialized\n");
		return;
	}
    mt_data = (mt_sample_data_t *)data;
    mio_data_t *mdata = (mio_data_t *)mt_data->mdata;
}

static void mt_recv(void *data)
{
    mt_sample_data_t *mt_data;
	if (data == NULL){
		printf("phylink_close, data not initialized\n");
		return;
	}
    mt_data = (mt_sample_data_t *)data;
    mio_data_t *mdata = (mio_data_t *)mt_data->mdata;
    printf("%s(%d)\n", __func__,__LINE__);
}

void init_mt_sample(montask_t *montask_pool, void *mdata)
{
	mt_sample_data_t *data;
	data = (mt_sample_data_t *)calloc(1,sizeof(mt_sample_data_t));
	if(!data) {
		perror("calloc");
		return;
	}
    data->sec = mt_sample.sec;
    data->usec = mt_sample.usec;
    data->mdata = mdata;

	add_montask(montask_pool, mt_sample.name, 
           mt_sample.sec,
           mt_sample.usec,
           mt_sample.mt_init_func,
           mt_sample.mt_periodic_func,
           mt_sample.mt_recv_func,
           mt_sample.mt_uninit_func,
           (void *)data);
}

void uninit_mt_sample(montask_t *montask_pool)
{
	montask_t *montask;
	if (!montask_pool) 
		return;

	montask = find_montask_by_name(montask_pool, mt_sample.name); 
	if (montask == NULL)
		return;

	if (montask->data != NULL){ // mt_sample_data_t
		free(montask->data);
		montask->data = NULL;
	}
	remove_montask(montask_pool, montask);
}
