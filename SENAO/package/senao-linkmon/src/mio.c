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
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include "mio.h"
#include "list.h"
#include "util.h"

mio_event_t *add_event(mio_event_t *event_pool, int fd, event_func func, void *data)
{
	if (fd <= 0){
		lmdbg("fd:[%d] add_event fail. not allowed smaller equal than 0\n", fd);
		return NULL;
	}
	mio_event_t *tmp;
	tmp = (mio_event_t *)calloc(1, sizeof(mio_event_t));
	if(!tmp) {
		perror("calloc");
		return NULL;
	}
	tmp->fd = fd;
	tmp->event_func = func;
	tmp->data = data;
	
	list_add_tail( &(tmp->list), &((*event_pool).list));
	return tmp;
}
void remove_event_by_fd(mio_event_t *event_pool, mio_event_t *event)
{
	if (event_pool == NULL || event == NULL) return;

	mio_event_t *ptr;
	list_for_each_entry(ptr, &((*event_pool).list),list) {
		if (ptr->fd == event->fd &&
			ptr->event_func == event->event_func &&
			ptr->data == event->data){
			// free what alloc in add_event
			list_del(&(ptr->list));	
			free(ptr);
			break;
		}
	}
}
void remove_event(mio_event_t *me)
{
	list_del(&(me->list));
	free(me);
}

static void free_event_pool(mio_event_t *pool)
{
	mio_event_t *ptr;
	while(!list_empty(&(pool->list))) {
		ptr = list_entry((pool->list).next, mio_event_t, list);
		// free what alloc in add_event
		list_del(&(ptr->list));	
		free(ptr);
	}
}

void event_setfds(mio_event_t *event_pool, fd_set *fds)
{
	FD_ZERO(fds);
	if (event_pool == NULL)
		return;

	mio_event_t *ptr;
	list_for_each_entry(ptr, &((*event_pool).list),list) {
		FD_SET(ptr->fd, fds);
	}
}

void event_dispatch(mio_event_t *event_pool, fd_set *fds)
{
	if (event_pool == NULL)
		return;

	mio_event_t *ptr;
	list_for_each_entry(ptr, &((*event_pool).list),list) {
		if (FD_ISSET(ptr->fd, fds)){
			if (ptr->event_func)
				ptr->event_func(ptr->data);
			break;
		}
	}
}

mio_timer_t *add_timer(mio_timer_t *timer_pool, unsigned int sec, unsigned int usec, timer_func func, void *data, int repeat){
	mio_timer_t *tmp, *ptr;
	long now_sec;

	struct timeval tv;
	// caculate seconds and useconds
	if (gettimeofday(&tv, NULL) != 0){
		perror("gettimeofday fail:");
		return NULL;
	}
	tmp = (mio_timer_t *)calloc(1, sizeof(mio_timer_t));
	if(!tmp) {
		perror("calloc");
		return NULL;
	}

	tmp->delay_sec = sec;
	tmp->delay_usec = usec;
	tmp->repeat = repeat;

	now_sec = tv.tv_sec;
	tmp->sec = tv.tv_sec + sec;
	if ( tmp->sec < now_sec){ // overflow
		lmdbg("timeout seconds overflow\n");
		free(tmp);
		return NULL;
	}
	tmp->usec = tv.tv_usec + usec;
	while (tmp->usec >= 1000000) {
		tmp->sec++;
		tmp->usec -= 1000000;
	}

	tmp->data = data;
	tmp->timer_func = func;
	// add ordered	
	list_for_each_entry(ptr, &(timer_pool->list),list) {
		if ((tmp->sec < ptr->sec) ||
			(tmp->sec == ptr->sec && tmp->usec < ptr->usec)){
			list_add_tail( &(tmp->list) , &((*ptr).list));
			return tmp;
		}
	}
	// biggest
	list_add_tail( &(tmp->list), &((*ptr).list));
	return tmp;
}
mio_timer_t *modify_timer(mio_timer_t *timer_pool, mio_timer_t *et, unsigned int sec, unsigned int usec, int repeat)
{
	mio_timer_t *ptr;
	long now_sec;
	if (timer_pool == NULL || et == NULL){
		lmdbg("data null error\n");
		return NULL;
	}

	list_del(&(et->list));
		
	struct timeval tv;
	if (gettimeofday(&tv, NULL) != 0){
		perror("gettimeofday fail:");
		return et;
	}
	et->delay_sec = sec;
	et->delay_usec = usec;
	et->repeat = repeat;

	now_sec = tv.tv_sec;
	et->sec = tv.tv_sec + sec;
	if ( et->sec < now_sec){ // overflow
		lmdbg("timeout seconds overflow\n");
		if (et->data){
			free(et->data);
			et->data = NULL;
		}
		free(et);
		return NULL;
	}
	et->usec = tv.tv_usec + usec;
	while (et->usec >= 1000000) {
		et->sec++;
		et->usec -= 1000000;
	}

	// add ordered	
	list_for_each_entry(ptr, &(timer_pool->list),list) {
		if ((et->sec < ptr->sec) ||
			(et->sec == ptr->sec && et->usec < ptr->usec)){
			list_add_tail( &(et->list) , &((*ptr).list));
			return et;
		}
	}
	// biggest
	list_add_tail( &(et->list), &((*ptr).list));
	return et;
}

void remove_timer(mio_timer_t *mt)
{
	list_del(&(mt->list));
	free(mt);
}

static void free_timer_pool(mio_timer_t *pool)
{
	mio_timer_t *ptr;
	while(!list_empty(&(pool->list))) {
		ptr = list_entry((pool->list).next, mio_timer_t, list);
		// free what alloc in add_event
		list_del(&(ptr->list));	
		free(ptr);
	}
}

void traverse_timer(mio_timer_t *et)
{
	mio_timer_t *ptr;
	list_for_each_entry(ptr, &(et->list),list) {
		lmdbg("sec:[%ld] usec:[%ld] delay_dec:[%ld] delsy_usec:[%ld] repeat:[%d] data:[%p]\n", ptr->sec, ptr->usec, ptr->delay_sec, ptr->delay_usec, ptr->repeat, ptr->data);
	}

}

int event_settimer(mio_timer_t *event_timer,struct timeval *tv)
{
	struct timeval now;
	if (list_empty(&(event_timer->list))) {
		return -1;
	}
	mio_timer_t *ptr;
	ptr = list_entry((*event_timer).list.next, mio_timer_t, list);

	//lmdbg("ptr->sec:[%ld] ptr->usec:[%ld]\n", ptr->sec, ptr->usec);
	
	if (gettimeofday(&now, NULL) != 0){
		perror("gettimeofday fail:");
		return -1;
	}

	if ((now.tv_sec < ptr->sec) ||
		(now.tv_sec == ptr->sec && now.tv_usec < ptr->usec)){
		tv->tv_sec = ptr->sec - now.tv_sec;
		tv->tv_usec = ptr->usec - now.tv_usec;
		if (tv->tv_usec < 0){
			tv->tv_sec--;
			tv->tv_usec += 1000000;
		}
	}
	else{
		tv->tv_sec = 0;
		tv->tv_usec = 0;
	
	}
//	lmdbg("tv->tv_sec:[%ld] tv->tv_usec:[%ld]\n", tv->tv_sec, tv->tv_usec);
	return 0;
}

void event_timer_execution(mio_timer_t *event_timer)
{
	struct timeval now;
	if (list_empty(&(event_timer->list)))
		return;
	mio_timer_t *ptr, *ptr2;
	ptr = list_entry((*event_timer).list.next, mio_timer_t, list);
	//lmdbg("ptr->sec:[%ld] ptr->usec:[%ld]\n", ptr->sec, ptr->usec);

	if (gettimeofday(&now, NULL) != 0){
		perror("gettimeofday fail:");
		return;
	}
	
	if ((ptr->sec < now.tv_sec) ||
		(ptr->sec == now.tv_sec && ptr->usec < now.tv_usec)){
		ptr->timer_func(ptr->data);
		if (ptr->repeat){
			ptr->sec = now.tv_sec + ptr->delay_sec;
			ptr->usec =  now.tv_usec + ptr->delay_usec;
			list_del(&(ptr->list));
			// re-add ordered	
			list_for_each_entry(ptr2, &(event_timer->list),list) {
				if ((ptr->sec < ptr2->sec) ||
					(ptr->sec == ptr2->sec && ptr->usec < ptr2->usec)){
					list_add_tail( &(ptr->list) , &((*ptr2).list));
					return;
				}
			}
			// biggest
			list_add_tail( &(ptr->list), &((*ptr2).list));
		}
		else 
			remove_timer(ptr);
	}
}
static mio_signal_t *signal_pool = NULL;
static void signal_handler(int sig)
{
	if (signal_pool == NULL){
		lmdbg("signal handler not initialized\n");
		return;
	}
	mio_signal_t *ptr;
	list_for_each_entry(ptr, &(signal_pool->list),list) {
		if (ptr->sig == sig){
			ptr->count++;
			break;
		}
	}
}
void signal_dispatch(mio_signal_t *signal_pool)
{
	mio_signal_t *ptr;
	list_for_each_entry(ptr, &(signal_pool->list),list) {
		if (ptr->count){
			ptr->count=0;
			ptr->signal_func(ptr->sig, ptr->data);
		}
	}
}


int add_signal(mio_signal_t *signal_pool,int sig, signal_func func, void *data)
{
	mio_signal_t *tmp;

	tmp = (mio_signal_t *)calloc(1, sizeof(mio_signal_t));
	if(!tmp) {
		perror("calloc");
		return -1;
	}
	// TODO: check add twice, resonable?
	tmp->sig = sig;
	tmp->data = data;
	tmp->signal_func = func;	
	signal(sig, signal_handler);
	list_add_tail( &(tmp->list) , &((*signal_pool).list));
	return sig;
}
void remove_signal(mio_signal_t *signal_pool, int sig)
{
	if (signal_pool == NULL) return;
	mio_signal_t *ptr;
	list_for_each_entry(ptr, &((*signal_pool).list),list) {
		if (ptr->sig == sig){
			// free what alloc in add_signal
			//
			list_del(&(ptr->list));	
			free(ptr);
			break; // TODO: check add twice, resonalbe?
		}
	}
}

static void free_signal_pool(mio_signal_t *pool)
{
	mio_signal_t *ptr;
	while(!list_empty(&(pool->list))) {
		ptr = list_entry((pool->list).next, mio_signal_t, list);
		// free what alloc in add_event
		list_del(&(ptr->list));	
		free(ptr);
	}
}

void mio_loop(mio_data_t *mio_data)
{
	int res,tres;
	fd_set rfds, wfds, efds;
	struct timeval tv;
	while(!mio_data->terminate){
		int fd;

		event_setfds(&(mio_data->read_pool), &rfds);
		event_setfds(&(mio_data->write_pool), &wfds);
		event_setfds(&(mio_data->exception_pool), &efds);
		tres = event_settimer(&(mio_data->timer_pool), &tv);

		//lmdbg("server waiting\n");
		res = select(FD_SETSIZE, &rfds, &wfds,
				&efds, tres == 0 ? &tv : NULL);

		if (res < 0 && errno != EINTR && errno !=0){
			perror("select error:");
			continue;
		}
		signal_dispatch(&(mio_data->signal_pool));
		event_timer_execution(&(mio_data->timer_pool));

		if (res <= 0){
		//	lmdbg("timeout timer:[%d]\n", res);
			continue;
		}
		event_dispatch(&(mio_data->read_pool), &rfds);
		event_dispatch(&(mio_data->write_pool), &wfds);
		event_dispatch(&(mio_data->exception_pool), &efds);
	}
}

mio_data_t *mio_init()
{
	mio_data_t *mio_data;
	mio_data = (mio_data_t *)calloc(1,sizeof(mio_data_t));
	INIT_LIST_HEAD(&((mio_data->read_pool).list));
	INIT_LIST_HEAD(&((mio_data->write_pool).list));
	INIT_LIST_HEAD(&((mio_data->exception_pool).list));
	INIT_LIST_HEAD(&((mio_data->timer_pool).list));
	INIT_LIST_HEAD(&((mio_data->signal_pool).list));
	// add global link of signal pool to be used in not-able-to-pass-parameter signal function
	signal_pool = &(mio_data->signal_pool);
	mio_data->terminate=0;
	return mio_data;
}
void mio_uninit(mio_data_t *mio_data)
{
	if (mio_data == NULL) {
		lmdbg("not initialed\n");
		return;
	}
	// travesal read write exception timer signal list to free
	free_event_pool(&(mio_data->read_pool));
	free_event_pool(&(mio_data->write_pool));
	free_event_pool(&(mio_data->exception_pool));
	free_timer_pool(&(mio_data->timer_pool));
	free_signal_pool(&(mio_data->signal_pool));
	// disconnect the global link of signal pool
	signal_pool = NULL;
	free(mio_data);
	mio_data=NULL;
}

