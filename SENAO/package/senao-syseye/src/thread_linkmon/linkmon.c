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
#include <signal.h>
#include <util.h>
#include <pthread.h>
#include <mio.h>
#include <global.h>
#include "linkmon.h"
#include "mt_sample.h"

static void sig_exit_func(int sig, void *mio_data, void *data)
{
    mio_data_t *mdata = (mio_data_t *) mio_data;
	sedbg("[linkmon]got signal:[%d]\n", sig);
	mdata->terminate = 1;
}

void add_monitors(montask_t *montask_pool, mio_data_t *mdata)
{
    init_mt_sample(montask_pool, (void *)mdata);
}
void remove_monitors(montask_t *montask_pool)
{
    uninit_mt_sample(montask_pool);
}
void init_monitors(mio_data_t *mio_data, montask_t *montask_pool)
{
    montask_t *ptr;
	list_for_each_entry(ptr, &(montask_pool->list),list) {
		if (ptr->mt_init_func){
		    ptr->mt_init_func(ptr->data);
        }
    }

    if (!mio_data) 
        return;
	list_for_each_entry(ptr, &(montask_pool->list),list) {
		if (ptr->mt_periodic_func){
            ptr->mt_timer = (void *)
		    add_timer(&(mio_data->timer_pool), ptr->sec, ptr->usec, ptr->mt_periodic_func, ptr->data, 1);
        }
    }
    // TODO: any event request?
}

void uninit_monitors(mio_data_t *mio_data, montask_t *montask_pool)
{
    montask_t *ptr;
	list_for_each_entry(ptr, &(montask_pool->list),list) {
		if (ptr->mt_init_func){
		    ptr->mt_uninit_func(ptr->data);
        }
    }

	list_for_each_entry(ptr, &(montask_pool->list),list) {
		if (ptr->mt_timer){
            remove_timer((mio_timer_t *)ptr->mt_timer);
            ptr->mt_timer = NULL;
        }
    }
}

int linkmon_main(void *data)
{
    mio_data_t *g_mio_data = (mio_data_t *) data;
    montask_t *montask;

    if (data == NULL){
        printf("%s(%d) data null\n", __func__, __LINE__);
        return -1;
    }

	mio_data_t *mio_data = mio_init();

	montask = montask_init();
    add_signal(mio_data->signal_pool, SIGUSR1, sig_exit_func, NULL);
	add_monitors(montask, mio_data);
	init_monitors(mio_data, montask);
	mio_loop(mio_data);
	uninit_monitors(mio_data, montask);
	remove_monitors(montask);
	montask_uninit(montask);
	mio_uninit(mio_data);
}

void *linkmon_start(void *thread_data)
{
//    int s;
//    s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
//    if (s != 0)
//        sedbg_en(s, "pthread_setcancelstate");

    linkmon_main(thread_data); // mio_data
    pthread_detach(pthread_self());
    return NULL;
}
