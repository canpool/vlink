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

#include "vos.h"

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <mqueue.h>
#include <errno.h>

typedef void *(*pthread_entry)(void *);

static vmutex_t s_prot_lock; // protect lock

static void __set_timespec(struct timespec *ts, uint32_t timeout)
{
    clock_gettime(CLOCK_REALTIME, ts);

    if (timeout == 0) return;

    ts->tv_sec += (timeout / 1000);
    ts->tv_nsec += ((timeout % 1000) * 1000);
}

int vos_init(void)
{
    sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);

    vos_mutex_init(&s_prot_lock);

    return 0;
}

int vos_task_create(vtask_t *task, const char *name, int (*entry)(uintptr_t arg), uintptr_t arg,
                    int stacksize, int priority)
{
    pthread_t pid;

    if (pthread_create(&pid, NULL, (pthread_entry)entry, (void *)arg) != 0) {
        *task = V_TASK_INVALID;
        return -1;
    }
    pthread_detach(pid);
    *task = (vtask_t)pid;
    return 0;
}

int vos_task_delete(vtask_t *task)
{
    pthread_t pid = (pthread_t)(*task);

    if (*task == V_TASK_INVALID) {
        return -1;
    }

    if (pthread_cancel(pid) != 0) {
        return -1;
    }
    *task = V_TASK_INVALID;
    return 0;
}

int vos_task_resume(vtask_t task)
{
    return 0;
}

int vos_task_suspend(vtask_t task)
{
    return 0;
}

void vos_task_lock(void)
{
    vos_mutex_lock(s_prot_lock);
}

void vos_task_unlock(void)
{
    vos_mutex_unlock(s_prot_lock);
}

void vos_task_sleep(uint32_t ms)
{
    usleep(ms * 1000);
}

int vos_mutex_init(vmutex_t *mutex)
{
    pthread_mutex_t *m = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if (m == NULL) {
        return -1;
    }
    if (pthread_mutex_init(m, NULL) != 0) {
        free(m);
        return -1;
    }
    *mutex = (vmutex_t)m;
    return 0;
}

int vos_mutex_destroy(vmutex_t *mutex)
{
    pthread_mutex_t *m = (pthread_mutex_t *)(*mutex);

    if (*mutex == V_MUTEX_INVALID) {
        return -1;
    }

    if (pthread_mutex_destroy(m) != 0) {
        return -1;
    }
    free(m);
    *mutex = V_MUTEX_INVALID;
    return 0;
}

int vos_mutex_timedlock(vmutex_t mutex, uint32_t timeout)
{
    struct timespec ts;

    __set_timespec(&ts, timeout);

    return pthread_mutex_timedlock((pthread_mutex_t *)mutex, &ts);
}

int vos_mutex_trylock(vmutex_t mutex)
{
    return pthread_mutex_trylock((pthread_mutex_t *)mutex);
}

int vos_mutex_lock(vmutex_t mutex)
{
    return pthread_mutex_lock((pthread_mutex_t *)mutex);
}

int vos_mutex_unlock(vmutex_t mutex)
{
    if (pthread_mutex_unlock((pthread_mutex_t *)mutex) != 0) {
        return -1;
    }
    usleep(1000);
    return 0;
}

int vos_sem_init(vsem_t *sem, int limit, int value)
{
    sem_t *s = (sem_t *)malloc(sizeof(sem_t));
    if (s == NULL) {
        return -1;
    }

    if (sem_init(s, 0, value) != 0) {
        free(s);
        return -1;
    }
    *sem = (vsem_t)s;
    return 0;
}

int vos_sem_destroy(vsem_t *sem)
{
    sem_t *s = (sem_t *)(*sem);

    if (*sem == V_SEM_INVALID) {
        return -1;
    }

    if (sem_destroy(s) != 0) {
        return -1;
    }
    free(s);
    *sem = V_SEM_INVALID;
    return 0;
}

int vos_sem_timedwait(vsem_t sem, uint32_t timeout)
{
    struct timespec ts;

    __set_timespec(&ts, timeout);

    return sem_timedwait((sem_t *)sem, &ts);
}

int vos_sem_trywait(vsem_t sem)
{
    return sem_trywait((sem_t *)sem);
}

int vos_sem_wait(vsem_t sem)
{
    return sem_wait((sem_t *)sem);
}

int vos_sem_post(vsem_t sem)
{
    return sem_post((sem_t *)sem);
}

int vos_event_init(vevent_t *event)
{
    return -1;
}

int vos_event_destroy(vevent_t *event)
{
    return -1;
}

int vos_event_timedrecv(vevent_t event, uint32_t wanted, uint32_t option, uint32_t timeout, uint32_t *recved)
{
    return -1;
}

int vos_event_tryrecv(vevent_t event, uint32_t wanted, uint32_t option, uint32_t *recved)
{
    return -1;
}

int vos_event_recv(vevent_t event, uint32_t wanted, uint32_t option, uint32_t *recved)
{
    return -1;
}

int vos_event_send(vevent_t event, uint32_t value)
{
    return -1;
}

int vos_event_clear(vevent_t event, uint32_t value)
{
    return -1;
}

/* msg queue */

typedef struct vmqueue {
    mqd_t id;
    const char *name;
} vmqueue_t;

int vos_mq_create(vmq_t *mq, const char *name, size_t msg_size, size_t max_msgs, uint32_t options)
{
    vmqueue_t *q;
    struct mq_attr attr;

    if (name == NULL) {
        return -1;
    }

    q = (vmqueue_t *)malloc(sizeof(vmqueue_t));
    if (q == NULL) {
        return -1;
    }

    // attr.mq_maxmsg <= /proc/sys/fs/mqueue/msg_max (10)
    // attr.mq_msgsize <= /proc/sys/fs/mqueue/msgsize_max (8192)
    attr.mq_maxmsg = max_msgs;
    attr.mq_msgsize = msg_size;

    q->id = mq_open(name, O_RDWR | O_CREAT | O_EXCL, 0666, &attr); // name format is "/xxxx"
    if (q->id < 0) {
        if (errno == EEXIST) {
            mq_unlink(name);
        }
        free(q);
        return -1;
    }
    q->name = name; // name is global
    *mq = (vmq_t)q;

    return 0;
}

int vos_mq_delete(vmq_t *mq)
{
    vmqueue_t *q = (vmqueue_t *)(*mq);

    if (*mq == V_MQ_INVALID) {
        return -1;
    }

    if (q == NULL || q->name == NULL) {
        return -1;
    }
    if (mq_close(q->id) != 0) {
        return -1;
    }
    mq_unlink(q->name);
    free(q);
    *mq = V_MQ_INVALID;

    return 0;
}

int vos_mq_timedsend(vmq_t mq, const void *buf, size_t len, uint32_t timeout)
{
    struct timespec ts;
    vmqueue_t *q = (vmqueue_t *)mq;

    if (q == NULL) {
        return -1;
    }
    __set_timespec(&ts, timeout);

    return mq_timedsend(q->id, buf, len, 1, &ts);
}

int vos_mq_trysend(vmq_t mq, const void *buf, size_t len)
{
    return vos_mq_timedsend(mq, buf, len, 0);
}

int vos_mq_send(vmq_t mq, const void *buf, size_t len)
{
    vmqueue_t *q = (vmqueue_t *)mq;
    if (q == NULL) {
        return -1;
    }
    return mq_send(q->id, buf, len, 1);
}

int vos_mq_timedrecv(vmq_t mq, void *buf, size_t len, uint32_t timeout)
{
    struct timespec ts;
    vmqueue_t *q = (vmqueue_t *)mq;

    if (q == NULL) {
        return -1;
    }
    __set_timespec(&ts, timeout);

    return mq_timedreceive(q->id, buf, len, NULL, &ts);
}

int vos_mq_tryrecv(vmq_t mq, void *buf, size_t len)
{
    return vos_mq_timedrecv(mq, buf, len, 0);
}

int vos_mq_recv(vmq_t mq, void *buf, size_t len)
{
    vmqueue_t *q = (vmqueue_t *)mq;
    if (q == NULL) {
        return -1;
    }
    return mq_receive(q->id, buf, len, NULL);
}

void *vos_malloc(size_t size)
{
    if (size > 0) {
        return malloc(size);
    }
    return NULL;
}

void vos_free(void *ptr)
{
    if (ptr != NULL) {
        free(ptr);
    }
}

void *vos_realloc(void *ptr, size_t newsize)
{
    return (void *)realloc(ptr, newsize);
}

uint64_t vos_sys_time(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return (uint64_t)(tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
