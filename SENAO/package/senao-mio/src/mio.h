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
/**
 * @brief event data strucute call back function
 *
 */
typedef void (*event_func)(void *data); 
/**
 * @brief Event data structure
 *
 */
typedef struct mio_event_t {
	struct list_head list;
	int fd;                         ///< file descriptor to listen
	void (*event_func)(void *data); ///< call back function, execute when event is triggered
	void *data;                     ///< data for callbacl function
}mio_event_t;
/**
 * @brief timer data strucute call back function
 *
 */
typedef void (*timer_func)(void *mio_data, void *data); 
/**
 * @brief timer data structure
 */
typedef struct mio_timer_t {
	struct list_head list;
	long sec;                       ///< system time after, current time + seconds
	long usec;                      ///< system time after, current time + useconds
	long delay_sec;                 ///< execute after seconds
	long delay_usec;                ///< execute after useconds
	int repeat;                     ///< if thie timer repeat or just execute once
	void (*timer_func)(void *mio_data,void *data); ///< call back function, execute when timeup
	void *data;                     ///< data for callback function
}mio_timer_t;
/**
 * @brief signal data strucute call back function
 *
 */
typedef void (*signal_func)(int sig, void *mio_data, void *data); 
/**
 * @brief signal data structure
 */
typedef struct mio_signal_t {
	struct list_head list;
	int sig;
	int count;
	void (*signal_func)(int sig, void *mio_data, void *data);
	void *data;
}mio_signal_t;

/**
 * mio main data structure
 */
typedef struct mio_data_t {
	mio_event_t read_pool, write_pool, exception_pool; ///< read, write, and exception event pools
	mio_timer_t timer_pool;                            ///< timer pool
	mio_signal_t *signal_pool;                          ///< signal pool
	int terminate;                                     ///< terminate flag for end condition of loop
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
