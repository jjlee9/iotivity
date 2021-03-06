//******************************************************************
//
// Copyright 2014 Samsung Electronics All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



#define _BSD_SOURCE

#include "iotivity_config.h"
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <stdio.h>

#include "octimer.h"

#define SECOND (1)

#define TIMEOUTS 10

#define TIMEOUT_USED   1
#define TIMEOUT_UNUSED  2

#ifndef WITH_ARDUINO
pthread_t thread_id = 0; // 0: initial thread id (meaningless)
#endif

struct timelist_t
{
    int timeout_state;
    time_t timeout_seconds;
    time_t timeout_time;
    TimerCallback cb;
} timeout_list[TIMEOUTS];

time_t timespec_diff(const time_t after, const time_t before)
{
    return after - before;
}

void timespec_add(time_t *to, const time_t seconds)
{
    if (to && seconds > 0)
    {
        (*to) += seconds;
    }
}

#ifndef WITH_ARDUINO

long int getSeconds(struct tm *tp)
{
    long int nInterval = 0;

    nInterval = (tp->tm_hour * SECS_PER_HOUR);
    nInterval += (tp->tm_min * SECS_PER_MIN);
    nInterval += (tp->tm_sec * SECOND);

    printf("%ld", nInterval);

    return nInterval;
}

long int getRelativeSecondsOfDayofweek(int ia, int ib)
{
    if (ia > ib)
        return (((long int)(7 - (ib - ia))) * SECS_PER_DAY);

    return (((long int)((ib - ia))) * SECS_PER_DAY);
}

time_t getRelativeIntervalOfWeek(struct tm *tp)
{
    time_t current_time;
    struct tm *current, *midnight;
    time_t delayed_time = 0;

    time(&current_time);
    current = localtime(&current_time);

    if (current == NULL)
    {
        printf("ERROR; Getting local time fails\n");
        return 0;
    }

    midnight = (struct tm *)malloc(sizeof(struct tm));

    if (midnight == NULL)
    {
        printf("ERROR; Memory allocation fails\n");
        return 0;
    }

    memcpy(midnight, current, sizeof(struct tm));

    midnight->tm_hour = 0;
    midnight->tm_min = 0;
    midnight->tm_sec = 0;

    // Seconds.
    // Seconds from midnight.
    delayed_time = current_time - mktime(midnight);
    delayed_time = getRelativeSecondsOfDayofweek(current->tm_wday, tp->tm_wday) - delayed_time;
    delayed_time = delayed_time + getSeconds(tp);

    free(midnight);

    return delayed_time;
}

time_t getSecondsFromAbsTime(struct tm *tp)
{
    time_t current_time;
    time_t delayed_time = 0;

    time(&current_time);
    localtime(&current_time);

    delayed_time = mktime(tp) - current_time;

    return delayed_time;
}

time_t registerTimer(const time_t seconds, int *id, TimerCallback cb)
{
    time_t now, then;
    time_t next;
    int i, idx;

    if (0 == thread_id)
    {
        initThread();
    }

    if (seconds <= 0)
        return -1 ;

    // get the current time
    time(&now);

    for (idx = 0; idx < TIMEOUTS; ++idx)
        if (!((timeout_list[idx].timeout_state & TIMEOUT_USED) & TIMEOUT_USED))
            break;

    if (TIMEOUTS == idx) // reach to end of timer list
        return -1;

    // idx th timeout will be used.
    // Reset and set state of the timer
    timeout_list[idx].timeout_state = 0;
    timeout_list[idx].timeout_state |= TIMEOUT_USED;

    // calculate when the timeout should fire
    then = now;
    timespec_add(&then, seconds);

    timeout_list[idx].timeout_time = then;
    timeout_list[idx].timeout_seconds = seconds;

    // printf( "\nbefore timeout_list[idx].cb = %X\n", timeout_list[idx].cb);
    timeout_list[idx].cb = cb;
    // printf( " after timeout_list[idx].cb = %X\n", timeout_list[idx].cb);

    // How long till the next timeout?
    next = seconds;
    for (i = 0; i < TIMEOUTS; i++)
    {
        if ((timeout_list[i].timeout_state & (TIMEOUT_USED | TIMEOUT_UNUSED)) == TIMEOUT_USED)
        {
            const time_t secs = timespec_diff(timeout_list[i].timeout_time, now);

            if (secs >= 0 && secs < next)
                next = secs;
        }
    }

    *id = idx;
    /* Return the timeout number. */
    return timeout_list[idx].timeout_time;
}

void unregisterTimer(int idx)
{
    if (0 <= idx && idx < TIMEOUTS)
        timeout_list[idx].timeout_state = TIMEOUT_UNUSED;
}

void checkTimeout()
{
    time_t now;
    int i;

    time(&now);

    /* Check all timeouts that are used and armed, but not passed yet. */
    for (i = 0; i < TIMEOUTS; i++)
    {
        if ((timeout_list[i].timeout_state & (TIMEOUT_USED | TIMEOUT_UNUSED)) == TIMEOUT_USED)
        {
            const time_t seconds = timespec_diff(timeout_list[i].timeout_time, now);

            if (seconds <= 0)
            {
                /* timeout [i] fires! */
                timeout_list[i].timeout_state = TIMEOUT_UNUSED;
                if (timeout_list[i].cb)
                {
                    timeout_list[i].cb();
                }
            }
        }
    }
}

void *loop(void *threadid)
{
    (void)threadid;
    for (;;)
    {
        sleep(SECOND);
        checkTimeout();
    }
}

int initThread()
{
    int res = pthread_create(&thread_id, NULL, loop, NULL);

    if (res)
    {
        printf("ERROR; return code from pthread_create() is %d\n", res);
        return -1;
    }

    return 0;
}
#else   // WITH_ARDUINO
time_t timeToSecondsFromNow(tmElements_t *t_then)
{
    time_t t, then;

    t = now();
    then = makeTime((*t_then));

    return (time_t) (then - t);
}

time_t registerTimer(const time_t seconds, int *id, TimerCallback cb)
{
    time_t t, then;
    time_t next;
    int i, idx;

    if (seconds <= 0)
        return -1;

    // get the current time
    t = now();

    for (idx = 0; idx < TIMEOUTS; ++idx)
        if (!((timeout_list[idx].timeout_state & TIMEOUT_USED) & TIMEOUT_USED))
            break;

    if (TIMEOUTS == idx)// reach to end of timer list
        return -1;

    // idx th timeout will be used.
    // Reset and set state of the timer
    timeout_list[idx].timeout_state = 0;
    timeout_list[idx].timeout_state |= TIMEOUT_USED;

    // calculate when the timeout should fire
    then = t;
    timespec_add(&then, seconds);

    timeout_list[idx].timeout_time = then;
    timeout_list[idx].timeout_seconds = seconds;

    // printf( "\nbefore timeout_list[idx].cb = %X\n", timeout_list[idx].cb);
    timeout_list[idx].cb = cb;
    // printf( " after timeout_list[idx].cb = %X\n", timeout_list[idx].cb);

    // How long till the next timeout?
    next = seconds;
    for (i = 0; i < TIMEOUTS; i++)
    {
        if ((timeout_list[i].timeout_state & (TIMEOUT_USED | TIMEOUT_UNUSED))
            == TIMEOUT_USED)
        {
            const time_t secs = timespec_diff(timeout_list[i].timeout_time,
                                              t);

            if (secs >= 0 && secs < next)
                next = secs;
        }
    }

    *id = idx;
    /* Return the timeout number. */
    return timeout_list[idx].timeout_time;
}

void unregisterTimer(int idx)
{
    if (0 <= idx && idx < TIMEOUTS)
        timeout_list[idx].timeout_state = TIMEOUT_UNUSED;
}

void checkTimeout()
{
    time_t t;
    int i;

    t = now();

    /* Check all timeouts that are used and armed, but not passed yet. */
    for (i = 0; i < TIMEOUTS; i++)
    {
        if ((timeout_list[i].timeout_state & (TIMEOUT_USED | TIMEOUT_UNUSED))
            == TIMEOUT_USED)
        {
            const time_t seconds = timespec_diff(timeout_list[i].timeout_time,
                                                 t);

            if (seconds <= 0)
            {
                /* timeout [i] fires! */
                timeout_list[i].timeout_state = TIMEOUT_UNUSED;
                if (timeout_list[i].cb)
                {
                    timeout_list[i].cb();
                }
            }
        }
    }
}

#endif
