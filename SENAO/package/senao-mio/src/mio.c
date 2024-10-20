/*******************************************************************************************
 * Copyright (c) 2018 Ryan Hsu                                                             *
 *                                                                                         *
 * This code has to be used in SenaoNetworks internally                                    *
 * Unauthorized copying of this file, via any medium is strictly prohibite                 *
 * Proprietary and confidential                                                            *
 * --------------------------------------------------------------------------------------- *
 * Project:  mio (io multiplexer)                                                          *
 *                                                                                         *
 * @author :  Ryan Hsu (ryan_hsu@hotmail.com.tw)                                           *
 *******************************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include "mio.h"
#include "list.h"

#if 1
#define DBGMSG(fmt, args...) printf("%s(%d): " fmt, __FUNCTION__, __LINE__, ##args)
#else
#define DBGMSG(fmt, args...)
#endif
/**
 * @brief add_event, register an event to event pool
 *
 * add fd, call back function and call back function data to event pool
 *
 * @param *event_pool event pool head
 * @param fd file description of new event
 * @param func call back function of new event
 * @param *data callback function data of new event
 */
mio_event_t *add_event(mio_event_t *event_pool, int fd, event_func func, void *data)
{
	if (fd <= 0){
		printf("fd:[%d] add_event fail. not allowed smaller equal than 0\n", fd);
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

/**
 * @brief remove_event_by_fd, traverse event pool, remove event with same fd
 *
 * remove event from event_pool if the fd is the same as event
 *
 * @param *event_pool event pool head
 * @param *event event to remove
 */
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

/**
 * @brief remove_event, remove event from event_pool list
 *
 * remove event from event_pool list, use event pointer directly
 *
 * @param *me event to remove
 */
void remove_event(mio_event_t *me)
{
	list_del(&(me->list));
	free(me);
}

/**
 * @brief free_event_pool, empty event pool
 *
 * empty event pool, remove all events in the list
 *
 * @param *pool event pool to remove
 */
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

/**
 * @brief event_setfds, FD_SET all events in event_pool
 *
 * Traverse event_pool, get all fds, use FD_SET to set fd in fds
 *
 * @param *event_pool events pool contains fd
 * @param *fds target fdset
 */
static void event_setfds(mio_event_t *event_pool, fd_set *fds)
{
	FD_ZERO(fds);
	if (event_pool == NULL)
		return;

	mio_event_t *ptr;
	list_for_each_entry(ptr, &((*event_pool).list),list) {
		FD_SET(ptr->fd, fds);
	}
}

/**
 * @brief event_dispatch, call fd's callback function if fd is turned on
 *
 * Traverse event_pool, check each fd, if the fd in fdset is turned on
 * (event comming), call its callback function
 *
 * @param *event_pool events pool
 * @param *fds fdset, which contains some fds flag turned on
 */
static void event_dispatch(mio_event_t *event_pool, fd_set *fds)
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

/**
 * @brief add_timer, register an timer to timer pool
 *
 * add a delayed time with seconds + useconds, and the 
 * call back function when the time up to timer pool
 * timer_pool is in ordered list, smaller timer first
 *
 * @param *timer_pool timer pool head
 * @param sec seconds after to call callback function
 * @param usec, useconds after to call callback function
 * @param func callback function of new timer
 * @param *data call back function data of new timer
 * @param repeat decide if the timer run repeatedly after executed
 * @return new added timer point, use when we want to modify/remove it
 */
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
		DBGMSG("timeout seconds overflow\n");
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

/**
 * @brief modify_timer, adjust timer parameter in timer pool
 *
 * adjust timer parameter in timer pool, such as the time, and 
 * repeat
 *
 * @param *timer_pool timer pool head
 * @param *et timer to adjust
 * @param sec seconds after to call callback function
 * @param usec, useconds after to call callback function
 * @param repeat decide if the timer run repeatedly after executed
 * @return new added timer point, use when we want to modify/remove it
 */
mio_timer_t *modify_timer(mio_timer_t *timer_pool, mio_timer_t *et, unsigned int sec, unsigned int usec, int repeat)
{
	mio_timer_t *ptr;
	long now_sec;
	if (timer_pool == NULL || et == NULL){
		printf("data null error\n");
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
		DBGMSG("timeout seconds overflow\n");
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

/**
 * @brief remove_timer, remove timer from timer_pool list
 *
 * remove timer from timer_pool list, use timer pointer directly
 *
 * @param *me event to remove
 */
void remove_timer(mio_timer_t *mt)
{
	list_del(&(mt->list));
	free(mt);
}

/**
 * @brief free_timer_pool, empty timer pool
 *
 * empty timer pool, remove all timers in the list
 *
 * @param *pool timer pool to remove
 */
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

/**
 * @brief traverse_timer, print timer elements in timer pool
 *
 * traverse timer pool, print timer elements in the list
 *
 * @param *et timer pool to traverse and print
 */
void traverse_timer(mio_timer_t *et)
{
	mio_timer_t *ptr;
	list_for_each_entry(ptr, &(et->list),list) {
		printf("sec:[%ld] usec:[%ld] delay_dec:[%ld] delsy_usec:[%ld] repeat:[%d] data:[%p]\n", ptr->sec, ptr->usec, ptr->delay_sec, ptr->delay_usec, ptr->repeat, ptr->data);
	}

}

/**
 * @brief event_settimerr, register timer pool to select timer
 *
 * We need only get first timer in the timer pool list, 
 * register the time to timval of select. When this timer
 * timeup, first time is removed, and the second timer's time 
 *  would be registered.
 *
 * @param *event_timer timer pool to register
 * @param *tv timer for select function
 * @return 0 for success, -1 for fail
 */
static int event_settimer(mio_timer_t *event_timer,struct timeval *tv)
{
	struct timeval now;
	if (list_empty(&(event_timer->list))) {
		return -1;
	}
	mio_timer_t *ptr;
	ptr = list_entry((*event_timer).list.next, mio_timer_t, list);

	//printf("ptr->sec:[%ld] ptr->usec:[%ld]\n", ptr->sec, ptr->usec);
	
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
//	printf("tv->tv_sec:[%ld] tv->tv_usec:[%ld]\n", tv->tv_sec, tv->tv_usec);
	return 0;
}

/**
 * @brief event_timer_exection, get a timer from timer_pool to execute
 *
 * When select time out, check timer pool first timer. If time up,
 * get its callback function execute. re-add to timer pool if repeat 
 * flas is set. The first timer will be removed, the second one will be
 * added to select timer in next loop.
 *
 * @param *event_timer timer pool to check
 */
static void event_timer_execution(mio_timer_t *event_timer, mio_data_t *mio_data)
{
	struct timeval now;
	if (list_empty(&(event_timer->list)))
		return;
	mio_timer_t *ptr, *ptr2;
	ptr = list_entry((*event_timer).list.next, mio_timer_t, list);
	//printf("ptr->sec:[%ld] ptr->usec:[%ld]\n", ptr->sec, ptr->usec);

	if (gettimeofday(&now, NULL) != 0){
		perror("gettimeofday fail:");
		return;
	}
	
	if ((ptr->sec < now.tv_sec) ||
		(ptr->sec == now.tv_sec && ptr->usec < now.tv_usec)){
		ptr->timer_func((void *)mio_data, ptr->data);
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

/**
 * @brief since signal function can only pass signal number 
 * as  parameter, we must make signal_pool global
 */
static mio_signal_t *signal_pool = NULL;

/**
 * @brief signal_handler, callback function when registered signal 
 * caught
 *
 * When registered signal caught, add the registered signal's count 
 * in signal_pool.
 *
 * @param *sig the callback function get what signal number is caught
 */
static void signal_handler(int sig)
{
	if (signal_pool == NULL){
		printf("signal handler not initialized\n");
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

/**
 * @brief signal_dispatch, traverse signal pool, execute the callback
 * function when signal is caught
 *
 * Traverse signal pool, check the count added by signal_handler
 * function, if the count > 0, execute its callback function.
 * Then set the count back to 0.
 *
 * @param *signal_pool signal_pool to check
 */
static void signal_dispatch(mio_signal_t *signal_pool, mio_data_t *mio_data)
{
	mio_signal_t *ptr;
	list_for_each_entry(ptr, &(signal_pool->list),list) {
		if (ptr->count){
			ptr->count=0;
			ptr->signal_func(ptr->sig, (void *)mio_data, ptr->data);
		}
	}
}

/**
 * @brief add_signal, register a signal to signal pool
 *
 * add signal number, call back function and call back 
 * function data to signal pool
 *
 * @param *signal_pool event pool head
 * @param sig signal nunber of new signal
 * @param func call back function of new signal
 * @param *data callback function data of new signal
 * @return the signal number to added or -1 if added failed.
 */
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

/**
 * @brief remove_signal, remove signal from signal_pool list
 *
 * remove signal from signal_pool list, use signal number as key
 *
 * @param *signal_pool signal_pool to search
 * @param sig signal number to remove
 */
void remove_signal(mio_signal_t *signal_pool, int sig)
{
	if (signal_pool == NULL) return;
	mio_signal_t *ptr;
	list_for_each_entry(ptr, &((*signal_pool).list),list) {
		if (ptr->sig == sig){
			// free what alloc in add_signal
			list_del(&(ptr->list));	
			free(ptr);
			break; // TODO: check add twice, resonalbe?
		}
	}
}

/**
 * @brief free_signal_pool, empty signal pool
 *
 * empty signal pool, remove all signals in the list
 *
 * @param *pool signal pool to remove
 */
static void free_signal_pool(mio_signal_t *pool)
{
    if (pool == NULL)
        return;
	mio_signal_t *ptr;
	while(!list_empty(&(pool->list))) {
		ptr = list_entry((pool->list).next, mio_signal_t, list);
		// free what alloc in add_event
		list_del(&(ptr->list));	
        if (ptr->sig != 0) // FIXME: workaround, use list_move in mio_init add one sig=0 invalid entry, free it cause seg fault
            free(ptr);
	}
}

void traverse_signal(mio_signal_t *et)
{
	mio_signal_t *ptr;
	list_for_each_entry(ptr, &(et->list),list) {
		printf("sig:[%d] count:[%d]\n", ptr->sig, ptr->count);
	}

}

/**
 * @brief mio_loop, main loop of mio library.
 *
 * main loop of mio library, call it to block waiting
 *
 * @param *mio_data main mio structure, includes event, timer, 
 * signal pool and terminate flag
 */
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

		//DBGMSG("server waiting\n");
		res = select(FD_SETSIZE, &rfds, &wfds,
				&efds, tres == 0 ? &tv : NULL);
	
		if (res < 0 && errno != EINTR && errno !=0){
			perror("select error:");
			continue;
		}
		signal_dispatch(mio_data->signal_pool, mio_data);
		event_timer_execution(&(mio_data->timer_pool), mio_data);

		if (res <= 0){
		//	printf("timeout timer:[%d]\n", res);
			continue;
		}
		event_dispatch(&(mio_data->read_pool), &rfds);
		event_dispatch(&(mio_data->write_pool), &wfds);
		event_dispatch(&(mio_data->exception_pool), &efds);
	}
}

/**
 * @brief mio_init, initial task of mio
 *
 * initial task, call it to get initialized main mio structure
 *
 * @return initialized main mio structure
 */
mio_data_t *mio_init()
{
	mio_data_t *mio_data;
	mio_data = (mio_data_t *)calloc(1,sizeof(mio_data_t));
	INIT_LIST_HEAD(&((mio_data->read_pool).list));
	INIT_LIST_HEAD(&((mio_data->write_pool).list));
	INIT_LIST_HEAD(&((mio_data->exception_pool).list));
	INIT_LIST_HEAD(&((mio_data->timer_pool).list));
    if (signal_pool == NULL){
        signal_pool = calloc(1, sizeof(mio_signal_t));
	    INIT_LIST_HEAD(&(signal_pool->list));
    }
    mio_data->signal_pool = signal_pool;

	mio_data->terminate=0;
	return mio_data;
}

/**
 * @brief mio_uninit, free task of mio
 *
 * uninitial task, free all allocated memory of mio data in mio_init
 *
 * @param *mio_data initialized mio data initialed by mio_init
 */
void mio_uninit(mio_data_t *mio_data)
{
	if (mio_data == NULL) {
		DBGMSG("not initialed\n");
		return;
	}
	// travesal read write exception timer signal list to free
	free_event_pool(&(mio_data->read_pool));
	free_event_pool(&(mio_data->write_pool));
	free_event_pool(&(mio_data->exception_pool));
	free_timer_pool(&(mio_data->timer_pool));
    free_signal_pool(signal_pool);
    if (signal_pool != NULL)
        free(signal_pool);
    mio_data->signal_pool = NULL;
	signal_pool = NULL;
	free(mio_data);
	mio_data=NULL;
}

