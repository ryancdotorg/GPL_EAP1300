/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu                                                             *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                     *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  mio (io multiplexer)                                                          *
 *                                                                                         *
 * Author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                            *
 *******************************************************************************************/
#ifndef _MIO_H
#define _MIO_H
#include <stdlib.h>
#include <stdio.h>
#include "list.h"
// Private data type
typedef void (*event_func)(void *data); 
typedef struct mio_event_t {
	struct list_head list;
	int fd;
	void (*event_func)(void *data);
	void *data;
}mio_event_t;
typedef void (*timer_func)(void *data); 
typedef struct mio_timer_t {
	struct list_head list;
	long sec;	// system sec to execute, current time + sec
	long usec;	// 
	long delay_sec; // sec to execute 
	long delay_usec;
	int repeat;
	void (*timer_func)(void *data);
	void *data;
}mio_timer_t;
typedef void (*signal_func)(int sig, void *data); 
typedef struct mio_signal_t {
	struct list_head list;
	int sig;
	int count;
	void (*signal_func)(int sig, void *data);
	void *data;
}mio_signal_t;
typedef struct mio_data_t {
	mio_event_t read_pool,write_pool,exception_pool;
	mio_timer_t timer_pool;
	mio_signal_t signal_pool;
	int terminate;
}mio_data_t;

// Public function
void mio_loop(mio_data_t *mio_data);
mio_data_t *mio_init();
void mio_uninit(mio_data_t *mio_data);
mio_event_t *add_event(mio_event_t *, int, event_func func, void *data);
void remove_event_by_fd(mio_event_t *event_pool, mio_event_t *event);
void remove_event(mio_event_t *me);
mio_timer_t *add_timer(mio_timer_t *, unsigned int sec, unsigned int usec, timer_func func, void *data, int repeat);
mio_timer_t *modify_timer(mio_timer_t *, mio_timer_t *et, unsigned int sec, unsigned int usec, int repeat);
void traverse_timer(mio_timer_t *et);
void remove_timer(mio_timer_t *);
int add_signal(mio_signal_t *, int sig, signal_func func, void *data);
void remove_signal(mio_signal_t *, int);
#endif //_MIO_H
