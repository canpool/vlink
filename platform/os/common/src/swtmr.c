/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "swtmr.h"
#include "vos.h"

#include <string.h>

#define SWTMR_WAIT_TIME_MAX     V_WAIT_FOREVER
#define SWTMR_DEAD_TIME_MAX     ((uint64_t)-1)

typedef struct swtmr_cb {
    swtmr_t *list;  /**< soft timer list */
    vsem_t   sem;   /**< used for timer wait sync */
    vmutex_t mutex; /**< used for protect timer list */
    vtask_t  task;  /**< module task engine */
} swtmr_cb_t;

static swtmr_cb_t *s_swtmr_cb = NULL;

static void __swtmr_add(swtmr_t *timer)
{
    uint64_t timeleft = timer->timeleft;
    swtmr_t *cur      = s_swtmr_cb->list;
    swtmr_t *prev     = NULL;

    while (cur != NULL) {
        if (cur->timeleft > timeleft) {
            break;
        }
        prev = cur;
        cur  = cur->next;
    }
    if (prev == NULL) {
        timer->next = s_swtmr_cb->list;
        s_swtmr_cb->list = timer;
    } else {
        prev->next  = timer;
        timer->next = cur;
    }
}

static void __swtmr_rm(swtmr_t *timer)
{
    if (timer == s_swtmr_cb->list) {
        s_swtmr_cb->list = timer->next;
    } else {
        swtmr_t *itr = s_swtmr_cb->list;
        while (itr != NULL) {
            if (itr->next == timer) {
                itr->next = timer->next;
                break;
            }
            itr = itr->next;
        }
    }
    timer->next = NULL;
}

static void __swtmr_scan(void)
{
    swtmr_t *timer;
    uint64_t cur_time;

    while (s_swtmr_cb->list != NULL) {
        timer    = s_swtmr_cb->list;
        cur_time = vsystime();

        if (timer->timeleft > cur_time) {
            break;
        }
        // take it
        s_swtmr_cb->list = timer->next;
        timer->next      = NULL;

        // handle
        timer->handler(timer->arg);
        if (timer->mode == SWTMR_MODE_ONCE) {
            timer->timeleft = SWTMR_DEAD_TIME_MAX;
        } else {
            timer->timeleft = cur_time + timer->interval;
        }
        __swtmr_add(timer);
    }
}

static int __swtmr_entry(uintptr_t arg)
{
    uint32_t wait_time = SWTMR_WAIT_TIME_MAX;
    uint64_t cur_time  = 0;

    while (1) {
        vsem_timedwait(&s_swtmr_cb->sem, wait_time);
        if (vmutex_lock(&s_swtmr_cb->mutex) != 0) {
            continue;
        }

        __swtmr_scan();

        cur_time       = vsystime();
        swtmr_t *timer = s_swtmr_cb->list;
        if (timer == NULL || timer->timeleft == SWTMR_DEAD_TIME_MAX) { // no active timer
            wait_time = SWTMR_WAIT_TIME_MAX;
        } else {
            wait_time = timer->timeleft > cur_time ? timer->timeleft - cur_time : 0;
        }

        vmutex_unlock(&s_swtmr_cb->mutex);
    }

    return 0;
}

int swtmr_lib_init(void)
{
    int         ret = VERR;
    swtmr_cb_t *cb  = NULL;

    if (s_swtmr_cb != NULL) {
        return VEPERM;
    }
    cb = (swtmr_cb_t *)vmem_malloc(sizeof(swtmr_cb_t));
    if (cb == NULL) {
        return VENOMEM;
    }
    memset(cb, 0, sizeof(swtmr_cb_t));

    if (vsem_init(&cb->sem, 1, 0) != 0) {
        ret = VESEM;
        goto EXIT_FREE_CB;
    }
    if (vmutex_init(&cb->mutex) != 0) {
        ret = VEMUTEX;
        goto EXIT_DESTROY_SEM;
    }
    if (vtask_create(&cb->task, "user_swtmr", __swtmr_entry, 0, CONFIG_SWTMR_STACK_SIZE, CONFIG_SWTMR_TASK_PRIO) != 0) {
        ret = VETASK;
        goto EXIT_DESTROY_MUTEX;
    }
    s_swtmr_cb = cb;
    return VOK;

EXIT_DESTROY_MUTEX:
    vmutex_destroy(&cb->mutex);
EXIT_DESTROY_SEM:
    vsem_destroy(&cb->sem);
EXIT_FREE_CB:
    vmem_free(cb);

    return ret;
}

int swtmr_lib_exit(void)
{
    if (s_swtmr_cb == NULL) {
        return VERR;
    }

    if (vtask_delete(&s_swtmr_cb->task) != 0) {
        return VETASK;
    }
    vtask_sleep(100);
    if (vsem_destroy(&s_swtmr_cb->sem) != 0) {
        return VESEM;
    }
    if (vmutex_destroy(&s_swtmr_cb->mutex) != 0) {
        return VEMUTEX;
    }

    swtmr_t *itr  = s_swtmr_cb->list;
    swtmr_t *next = NULL;
    while (itr != NULL) {
        next = itr->next;
        vmem_free(itr);
        itr = next;
    }
    s_swtmr_cb->list = NULL;

    vmem_free(s_swtmr_cb);
    s_swtmr_cb = NULL;

    return VOK;
}

swtmr_t *swtmr_create(const char *name, uint32_t interval, uint8_t mode, swtmr_proc_pfn handler, uintptr_t arg)
{
    swtmr_t *timer;

    if (handler == NULL) {
        return NULL;
    }
    timer = (swtmr_t *)vmem_malloc(sizeof(swtmr_t));
    if (timer == NULL) {
        return NULL;
    }
    memset(timer, 0, sizeof(swtmr_t));
    timer->name     = name;
    timer->interval = interval;
    timer->handler  = handler;
    timer->arg      = arg;
    timer->mode     = mode;

    return timer;
}

int swtmr_delete(swtmr_t *timer)
{
    if (timer == NULL) {
        return VEINVAL;
    }
    if (s_swtmr_cb == NULL) {
        return VERR;
    }

    __swtmr_rm(timer);
    vmem_free(timer);

    return VOK;
}

static void __swtmr_start(swtmr_t *timer)
{
    __swtmr_rm(timer);
    timer->timeleft = timer->interval + vsystime();
    __swtmr_add(timer);
    vsem_post(&s_swtmr_cb->sem);
}

static void __swtmr_stop(swtmr_t *timer)
{
    __swtmr_rm(timer);
    timer->timeleft = SWTMR_DEAD_TIME_MAX;
    __swtmr_add(timer);
    vsem_post(&s_swtmr_cb->sem);
}

static int __swtmr_do(void (*job)(swtmr_t *), swtmr_t *timer)
{
    if (timer == NULL) {
        return VEINVAL;
    }
    if (s_swtmr_cb == NULL) {
        return VERR;
    }
    if (vmutex_lock(&s_swtmr_cb->mutex) != 0) {
        return VEMUTEX;
    }
    job(timer);

    vmutex_unlock(&s_swtmr_cb->mutex);

    return VOK;
}

int swtmr_start(swtmr_t *timer)
{
    return __swtmr_do(__swtmr_start, timer);
}

int swtmr_stop(swtmr_t *timer)
{
    return __swtmr_do(__swtmr_stop, timer);
}
