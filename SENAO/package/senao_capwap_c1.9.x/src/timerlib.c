/*
 * Copyright (C) 2008
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *
 * Author(s): Mauro Bisson
 */

#include <stdio.h> /* printf */
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <errno.h> /* ETIMEDOUT */
#include <signal.h>
#include <string.h> /* memcpy */
#include "timerlib.h"
#include "CWCommon.h"

/* time to wait the init of ticker thread in timer_init() function */
#define INIT_WAIT_TIME 5

/* for conversion purpose */
#define NANO_PER_MICRO 1000
#define MICRO_PER_SEC  1000000
#define NANO_PER_SEC   1000000000

/* max number of microsecond in a timeval struct */
#define MAX_USEC    999999

/* max number of nanosecond in a timespec struct */
#define MAX_NANO    999999999

/* microseconds in a millisecond */
#define ALMOST_NOW   1000

#define TIMER_KIND   ITIMER_REAL
#define SIG_TO_WAIT  SIGALRM

#define MAX_TIMER_ID 0x7ffffffe

/*
 * evals true if struct timeval t1 defines a time strictly before t2;
 * false otherwise.
 */
#define TV_LESS_THAN(t1,t2) \
                (((t1).tv_sec < (t2).tv_sec) ||\
                (((t1).tv_sec == (t2).tv_sec) && ((t1).tv_nsec < (t2).tv_nsec)))

/*
 * Assign struct timeval tgt the time interval between the absolute
 * times t1 and t2.
 * IT IS ASSUMED THAT t1 > t2, use TV_LESS_THAN macro to test it.
 */
#define TV_MINUS(t1,t2,tgt) \
                if ((t1).tv_nsec >= (t2).tv_nsec) {\
                    (tgt).tv_sec = (t1).tv_sec - (t2).tv_sec;\
                    (tgt).tv_usec = ((t1).tv_nsec - (t2).tv_nsec) / NANO_PER_MICRO;\
                } else {\
                    (tgt).tv_sec = (t1).tv_sec - (t2).tv_sec - 1;\
                    (tgt).tv_usec = ((t1).tv_nsec + (NANO_PER_SEC - (t2).tv_nsec)) / NANO_PER_MICRO;\
                }

typedef struct timer tl_timer_t;

static void timer_free(tl_timer_t * /*t*/);
static void timer_dequeue(tl_timer_t * /*t*/);
static void timer_start(struct timespec * /*abs_to*/);
static void *cronometer(void * /*arg*/);

struct timer
{
    struct timespec timeout;
    void(*handler)(void *);
    void *handler_arg;
    int         id;
    int         in_use;
    int         cancelled;
#ifdef CW_DEBUGGING_TIMER
    time_t      add_time;
    int         sec;
    int         usec;
#endif
    tl_timer_t  *next;
    tl_timer_t  *prev;
};

static struct timer_queue
{
    tl_timer_t      *first;
    tl_timer_t      *last;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    pthread_t       ticker;
    int             cur_id;
} timerq;

static void timer_free(tl_timer_t *t)
{
    if(t)
    {
        CWMemFree(t);
    }
    return;
}

static void timer_dequeue(tl_timer_t *t)
{
    if(t == NULL)
    {
        return;
    }

    if(t->prev == NULL)
    /* t is the first of the queue */
    {
        timerq.first = t->next;
    }
    else
    {
        t->prev->next = t->next;
    }

    if(t->next == NULL)
    /* t is the last of the queue */
    {
        timerq.last = t->prev;
    }
    else
    {
        t->next->prev = t->prev;
    }

    timer_free(t);
    return;
}

/*
 * Set a timer with setititmer syscall to expire on abs_to time.
 * The parameter abs_to is an absolute time as those returned with
 * gettimeofday (since epoch).
 * If abs_to refers to a time in the past the timer will be set to
 * expire in 1 millisecond.
 * This function has to be called in a critical section.
 */
static void timer_start(struct timespec *abs_to)
{
    struct itimerval relative = {{0, 0}, {0, 0}};
    struct timespec  abs_now;
    int              rv;

    if(abs_to == NULL)
    {
        return;
    }

    /* absolute to relative time */
    clock_gettime(CLOCK_MONOTONIC, &abs_now);

    if(TV_LESS_THAN(abs_now, *abs_to))
    {
        /* ok, timeout is in the future */
        TV_MINUS(*abs_to, abs_now, relative.it_value);

        /* if timeout is less than 1 usec, let's set it
         * to a very near future value. */
        if(relative.it_value.tv_sec == 0 && relative.it_value.tv_usec == 0)
        {
            relative.it_value.tv_usec = ALMOST_NOW;
        }
    }
    else
    {
        /*
         * ouch, timeout is in the past! Let's set it
         * to a very near future value.
         */
        relative.it_value.tv_sec = 0;
        relative.it_value.tv_usec = ALMOST_NOW;
    }

    rv = setitimer(TIMER_KIND, &relative, NULL);
    if(rv == -1)
    {
        /* This should never happen but I dont trust myself */
        perror("setitimer");
        exit(1);
    }

    return;
}

static void *cronometer(void *arg)
{
    void     (*hdl)(void *);
    void     *hdl_arg = arg; /* keep gcc silent */
    sigset_t mask;
    int      sig;

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGUSR1); /* used to notify there is a timer that is shorter than current one */

    /*
     * Set this thread to be cancelled only in the cancellation
     * points (pthread_cond_wait() and sigwait()).
     */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    /* signal creator thread of succesful init */
    pthread_mutex_lock(&timerq.mutex);
    pthread_cond_signal(&timerq.cond);
    /* printf("\tTicker thread: inizializzaione compleata\n"); */
    pthread_mutex_unlock(&timerq.mutex);

    while(1)
    {
        pthread_mutex_lock(&timerq.mutex);

        while(timerq.first == NULL)
        {
            pthread_cond_wait(&timerq.cond, &timerq.mutex);
        }

        /* SENAO Jonse 2014/1/8: Prevent starting a cancelled timer */
        if(timerq.first->cancelled == 1)
        {
            timer_dequeue(timerq.first);
            pthread_mutex_unlock(&timerq.mutex);
            continue;
        }

        timer_start(&(timerq.first->timeout));
        timerq.first->in_use = 1;

        pthread_mutex_unlock(&timerq.mutex);

        /* wait for a pending SIGALRM */
        if(sigwait(&mask, &sig) != 0)
        {
            perror("sigwait");
            continue;
        }

        /* SENAO Jonse 2014/03/21: SIGUSR1 means we have a new timer shorter than current one */
        if(sig == SIGUSR1)
        {
            continue;
        }

        pthread_mutex_lock(&timerq.mutex);

        /* check the timer is not cancelled */
        if(timerq.first->cancelled == 1)
        {
            timer_dequeue(timerq.first);
            pthread_mutex_unlock(&timerq.mutex);
            continue;
        }
        /*
         * we cannot be cancelled while freeing memory and executing
         * an handler.
         */
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        hdl = timerq.first->handler;
        hdl_arg = timerq.first->handler_arg;
#ifdef CW_DEBUGGING_TIMER
        CWDebugLog("timer %d %d.%d expire passed time %u\n",
                    timerq.first->id,
                    timerq.first->sec,
                    timerq.first->usec,
                    (unsigned int) (time(NULL) - timerq.first->add_time));
#endif
        timer_dequeue(timerq.first);
        /*
         * unlock the mutex before executing an handler to avoid
         * possible deadlock inside it.
         */
        pthread_mutex_unlock(&timerq.mutex);

        hdl(hdl_arg);
        /* reset cancel state (cancel type has not  been changed) */
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    }

    return NULL;
}

/*
 * Initialize the timerq struct, spwan the ticker thread,
 * wait its initialization and return.
 */
int timer_init()
{
    int rv;
    struct timespec	ts;
    struct timeval	tv;

    rv = pthread_mutex_init(&timerq.mutex, NULL);
    if(rv != 0)
    {
        return 0;
    }

    rv = pthread_cond_init(&timerq.cond, NULL);
    if(rv != 0)
    {
        pthread_mutex_destroy(&timerq.mutex);
        return 0;
    }

    timerq.first = timerq.last = NULL;
    timerq.cur_id = 0;

    pthread_mutex_lock(&timerq.mutex);

    rv = pthread_create(&timerq.ticker, NULL, cronometer, NULL);
    if(rv != 0)
    {
        pthread_mutex_unlock(&timerq.mutex);
        pthread_mutex_destroy(&timerq.mutex);
        pthread_cond_destroy(&timerq.cond);
        return 0;
    }

    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec + INIT_WAIT_TIME;
    ts.tv_nsec = tv.tv_usec * NANO_PER_MICRO;

    rv = pthread_cond_timedwait(&timerq.cond, &timerq.mutex, &ts);
    if(rv != 0)
    {
        if(rv == ETIMEDOUT)
        {
            pthread_cancel(timerq.ticker);
            pthread_join(timerq.ticker, NULL);
        }
        pthread_mutex_destroy(&timerq.mutex);
        pthread_cond_destroy(&timerq.cond);
        return 0;
    }

    /*
         * BUG.TRL02
         * Here we have to detach the thread in order to avoid memory leakage.
         *
         * 16/10/2009 - Donato Capitella
         */
    pthread_detach(timerq.ticker);

    pthread_mutex_unlock(&timerq.mutex);

    return 1;
}

void timer_destroy()
{
    tl_timer_t *t;

    pthread_cancel(timerq.ticker);
    pthread_join(timerq.ticker, NULL);

    t = timerq.last;
    while(t != NULL)
    {
        timerq.last = t->prev;
        timer_free(t);
        t = timerq.last;
    }

    pthread_mutex_destroy(&timerq.mutex);
    pthread_cond_destroy(&timerq.cond);

    return;
}

int timer_add(int sec, int usec, void(*hndlr)(void *), void *hndlr_arg)
{
    tl_timer_t *tmp = NULL;
    tl_timer_t *app = NULL;
    int        id = 0;
    int        nsec;

    if(hndlr == NULL)
    {
        return -1;
    }

    /* ensure timeout is in the future */
    if((sec < 0) || (usec < 0) || ((sec == 0) && (usec == 0)))
    {
        return -1;
    }

    pthread_mutex_lock(&timerq.mutex);

    app = (tl_timer_t *) CWMemAlloc(sizeof(tl_timer_t));
    if(app == NULL)
    {
        pthread_mutex_unlock(&timerq.mutex);
        return -1;
    }

#ifdef CW_DEBUGGING_TIMER
    app->add_time = time(NULL);
    app->sec = sec;
    app->usec = usec;
#endif

    /* relative to absolute time for timer */
    clock_gettime(CLOCK_MONOTONIC, &app->timeout);

    nsec = usec * NANO_PER_MICRO; /* to nanoseconds */
    app->timeout.tv_sec += sec + ((app->timeout.tv_nsec + nsec) / NANO_PER_SEC);
    app->timeout.tv_nsec = (app->timeout.tv_nsec + nsec) % NANO_PER_SEC;

    app->handler = hndlr;
    app->handler_arg = hndlr_arg;

    id = timerq.cur_id++;
    if(timerq.cur_id > MAX_TIMER_ID)
    {
        timerq.cur_id = 0;
    }

    app->id = id;
    app->in_use = 0;
    app->cancelled = 0;

    if(timerq.first == NULL)
    {
        /* timer queue empty */
        timerq.first = app;
        timerq.last = app;
        app->prev = NULL;
        app->next = NULL;
        pthread_cond_signal(&timerq.cond);
        pthread_mutex_unlock(&timerq.mutex);
        return id;
    }

    /* there is at least a timer in the queue */
    tmp = timerq.first;

    /* find the first timer that expires before app */
    while(tmp != NULL)
    {
        if(TV_LESS_THAN(app->timeout, tmp->timeout))
        {
            break;
        }

        tmp = tmp->next;
    }

    if(tmp == NULL)
    {
        /* app is the longest timer */
        app->prev = timerq.last;
        app->next = NULL;
        timerq.last->next = app;
        timerq.last = app;

        pthread_mutex_unlock(&timerq.mutex);
        return id;
    }

    if(tmp->prev == NULL)
    {
        /* app is the shoprtest timer */
        app->prev = NULL;
        app->next = tmp;
        tmp->prev = app;
        timerq.first = app;

        /* start app timer */
        tmp->in_use = 0;

        /* notify timer thread that a short timer is added */
        pthread_kill(timerq.ticker, SIGUSR1);
    }
    else
    {
        app->prev = tmp->prev;
        app->next = tmp;
        tmp->prev->next = app;
        tmp->prev = app;
    }

    pthread_mutex_unlock(&timerq.mutex);
    return id;
}

void timer_rem(int id, void(* free_arg)(void *))
{
    tl_timer_t *t;

    if(id < 0)
    {
        return;
    }

    pthread_mutex_lock(&timerq.mutex);
    t = timerq.first;
    /* look for timer id */
    while(t != NULL)
    {
        if(t->id == id)
        {
            break;
        }
        t = t->next;
    }

    if(t == NULL)
    {
        /* timer id is not in queue (maybe empty) */
        pthread_mutex_unlock(&timerq.mutex);
        return;
    }

    if(t->in_use == 1)
    {
        /*
         * timer id has been already activated, so just set
         * its "cancelled" flag (ticker thread will remove it)
         */

        /* LE-03-02-2010.01
         * We have to set handler_arg to NULL, otherwise we
         * risk to free it more than one time.
         */
        if(free_arg)
        {
            free_arg(t->handler_arg);
            t->handler_arg = NULL;
        }
        t->cancelled = 1;
        pthread_mutex_unlock(&timerq.mutex);
        return;
    }

    /* timer id is in the queue and has not been activated */
    if(free_arg)
    {
        free_arg(t->handler_arg);
    }
    timer_dequeue(t);

    pthread_mutex_unlock(&timerq.mutex);
    return;
}
