/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 *
 * vlink is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *    http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include "jffs2_os.h"
#include "jffs2_debug.h"


int jffs2_mutex_init(jffs2_mutex_t *mutex)
{
    return vos_mutex_init(&mutex->mux);
}

int jffs2_mutex_lock(jffs2_mutex_t *mutex)
{
    return vos_mutex_lock(mutex->mux);
}

int jffs2_mutex_trylock(jffs2_mutex_t *mutex)
{
    return 1;
}

int jffs2_mutex_unlock(jffs2_mutex_t *mutex)
{
    return vos_mutex_unlock(mutex->mux);
}

int jffs2_mutex_release(jffs2_mutex_t *mutex)
{
    int ret = vos_mutex_destroy(&mutex->mux);
    if (ret == 0) {
        mutex->mux = V_MUTEX_INVALID;
    }
    return ret;
}


int jffs2_flag_init(jffs2_flag_t *flag)
{
    return vos_event_init(&flag->event);
}


int jffs2_flag_setbits(jffs2_flag_t *flag, uint32_t value)
{
    return vos_event_send(flag->event, value);
}


int jffs2_flag_maskbits(jffs2_flag_t *flag, uint32_t value)
{
    return vos_event_clear(flag->event, value);
}

uint32_t jffs2_flag_wait(jffs2_flag_t *flag, uint32_t pattern, uint32_t mode)
{
    return jffs2_flag_timed_wait(flag->event, pattern, mode, V_TIMEOUT_FOREVER);
}

uint32_t jffs2_flag_timed_wait(jffs2_flag_t *flag, uint32_t pattern, uint32_t mode, uint32_t timeout)
{
    uint32_t recv = 0;

    switch (mode) {
        case JFFS2_FLAG_WAITMODE_AND:
            mode = V_EVENT_WAITMODE_AND;
            break;
        case JFFS2_FLAG_WAITMODE_CLR:
            mode = V_EVENT_WAITMODE_AND;
            break;
        case JFFS2_FLAG_WAITMODE_OR:
            mode = V_EVENT_WAITMODE_AND;
            break;
        default:
            break;
    }

    vos_event_timedrecv(flag->event, pattern, mode, timeout, &recv);
    return recv;
}

int jffs2_flag_destroy(jffs2_flag_t *flag)
{
    return vos_event_destroy(&flag->event);
}

int jffs2_thread_new(jffs2_thread_t *thread, char *name, jffs2_thread_fn function,
                        void *arg, int stacksize, int prio)
{
    return vos_task_create(&thread->task, name, function, arg, stacksize, prio);
}

int jffs2_thread_delete(jffs2_thread_t *thread)
{
    return vos_task_delete(&thread->task);
}

