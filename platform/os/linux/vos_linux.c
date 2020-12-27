/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
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

#ifdef CONFIG_SWTMR
#include "swtmr.h"
#endif

#ifdef CONFIG_SWQUE
#include "swque.h"
#endif

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

    if (vmutex_init(&s_prot_lock) != 0) {
        return VEMUTEX;
    }

#ifdef CONFIG_SWTMR
    if (swtmr_lib_init() != 0) {
        return VETIMER;
    }
#endif

    return VOK;
}

int vos_exit(void)
{
#ifdef CONFIG_SWTMR
    if (swtmr_lib_exit() != 0) {
        return VETIMER;
    }
#endif
    if (vmutex_destroy(&s_prot_lock) != 0) {
        return VEMUTEX;
    }

    return VOK;
}

int vtask_create(vtask_t *task, const char *name, int (*entry)(uintptr_t arg), uintptr_t arg,
                 int stacksize, int priority)
{
    pthread_t pid;

    if (vmagic_verify(task)) {
        return -1;
    }
    if (pthread_create(&pid, NULL, (pthread_entry)entry, (void *)arg) != 0) {
        return -1;
    }
    pthread_detach(pid);
    vmagic_set(task, pid);
    return 0;
}

int vtask_delete(vtask_t *task)
{
    if (!vmagic_verify(task)) {
        return -1;
    }
    pthread_t pid = (pthread_t)vmagic_get(task);
    if (pthread_cancel(pid) != 0) {
        return -1;
    }
    vmagic_reset(task);
    return 0;
}

int vtask_exit(void)
{
    pthread_exit(NULL);

    return 0;
}

int vtask_resume(vtask_t *task)
{
    return 0;
}

int vtask_suspend(vtask_t *task)
{
    return 0;
}

void vtask_lock(void)
{
    vmutex_lock(&s_prot_lock);
}

void vtask_unlock(void)
{
    vmutex_unlock(&s_prot_lock);
}

void vtask_sleep(uint32_t ms)
{
    usleep(ms * 1000);
}

int vtask_prio_set(vtask_t *task, int prio)
{
    return -1;
}

int vtask_prio_get(vtask_t *task)
{
    return -1;
}

void vtask_stat(void)
{
}

int vtask_waterline(vtask_t *task)
{
    return -1;
}

int vmutex_init(vmutex_t *mutex)
{
    if (vmagic_verify(mutex)) {
        return -1;
    }
    pthread_mutex_t *m = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if (m == NULL) {
        return -1;
    }
    if (pthread_mutex_init(m, NULL) != 0) {
        free(m);
        return -1;
    }
    vmagic_set(mutex, m);
    return 0;
}

int vmutex_destroy(vmutex_t *mutex)
{
    if (!vmagic_verify(mutex)) {
        return -1;
    }
    pthread_mutex_t *m = (pthread_mutex_t *)vmagic_get(mutex);
    if (pthread_mutex_destroy(m) != 0) {
        // return -1;
    }
    free(m);
    vmagic_reset(mutex);
    return 0;
}

int vmutex_timedlock(vmutex_t *mutex, uint32_t timeout)
{
    if (!vmagic_verify(mutex)) {
        return -1;
    }
    struct timespec ts;

    __set_timespec(&ts, timeout);

    return pthread_mutex_timedlock((pthread_mutex_t *)vmagic_get(mutex), &ts);
}

int vmutex_trylock(vmutex_t *mutex)
{
    if (!vmagic_verify(mutex)) {
        return -1;
    }
    return pthread_mutex_trylock((pthread_mutex_t *)vmagic_get(mutex));
}

int vmutex_lock(vmutex_t *mutex)
{
    if (!vmagic_verify(mutex)) {
        return -1;
    }
    return pthread_mutex_lock((pthread_mutex_t *)vmagic_get(mutex));
}

int vmutex_unlock(vmutex_t *mutex)
{
    if (!vmagic_verify(mutex)) {
        return -1;
    }
    if (pthread_mutex_unlock((pthread_mutex_t *)vmagic_get(mutex)) != 0) {
        return -1;
    }
    usleep(1000);
    return 0;
}

int vsem_init(vsem_t *sem, int limit, int value)
{
    if (vmagic_verify(sem)) {
        return -1;
    }
    sem_t *s = (sem_t *)malloc(sizeof(sem_t));
    if (s == NULL) {
        return -1;
    }

    if (sem_init(s, 0, value) != 0) {
        free(s);
        return -1;
    }
    vmagic_set(sem, s);
    return 0;
}

int vsem_destroy(vsem_t *sem)
{
    if (!vmagic_verify(sem)) {
        return -1;
    }
    sem_t *s = (sem_t *)vmagic_get(sem);
    if (sem_destroy(s) != 0) {
        return -1;
    }
    free(s);
    vmagic_reset(sem);
    return 0;
}

int vsem_timedwait(vsem_t *sem, uint32_t timeout)
{
    if (!vmagic_verify(sem)) {
        return -1;
    }
    struct timespec ts;

    __set_timespec(&ts, timeout);

    return sem_timedwait((sem_t *)vmagic_get(sem), &ts);
}

int vsem_trywait(vsem_t *sem)
{
    if (!vmagic_verify(sem)) {
        return -1;
    }
    return sem_trywait((sem_t *)vmagic_get(sem));
}

int vsem_wait(vsem_t *sem)
{
    if (!vmagic_verify(sem)) {
        return -1;
    }
    return sem_wait((sem_t *)vmagic_get(sem));
}

int vsem_post(vsem_t *sem)
{
    if (!vmagic_verify(sem)) {
        return -1;
    }
    return sem_post((sem_t *)vmagic_get(sem));
}

int vevent_init(vevent_t *event)
{
    return -1;
}

int vevent_destroy(vevent_t *event)
{
    return -1;
}

int vevent_timedrecv(vevent_t *event, uint32_t wanted, uint32_t option, uint32_t timeout, uint32_t *recved)
{
    return -1;
}

int vevent_tryrecv(vevent_t *event, uint32_t wanted, uint32_t option, uint32_t *recved)
{
    return -1;
}

int vevent_recv(vevent_t *event, uint32_t wanted, uint32_t option, uint32_t *recved)
{
    return -1;
}

int vevent_send(vevent_t *event, uint32_t value)
{
    return -1;
}

int vevent_clear(vevent_t *event, uint32_t value)
{
    return -1;
}

/* msg queue */

#ifdef CONFIG_SWQUE
int vmq_create(vmq_t *mq, const char *name, size_t msg_size, size_t max_msgs, uint32_t options)
{
    swque_t *q;

    if (name == NULL) {
        return -1;
    }
    if (vmagic_verify(mq)) {
        return -1;
    }

    q = swque_create(name, msg_size, max_msgs, options);
    if (q == NULL) {
        return -1;
    }
    vmagic_set(mq, q);

    return 0;
}

int vmq_delete(vmq_t *mq)
{
    if (!vmagic_verify(mq)) {
        return -1;
    }
    swque_t *q = (swque_t *)vmagic_get(mq);
    if (q == NULL) {
        return -1;
    }
    if (swque_delete(q) != 0) {
        return -1;
    }
    vmagic_reset(mq);

    return 0;
}

int vmq_timedsend(vmq_t *mq, const void *buf, size_t len, uint32_t timeout)
{
    if (!vmagic_verify(mq)) {
        return -1;
    }
    swque_t *q = (swque_t *)vmagic_get(mq);
    if (q == NULL) {
        return -1;
    }
    return swque_push(q, buf, len, timeout);
}

int vmq_trysend(vmq_t *mq, const void *buf, size_t len)
{
    return vmq_timedsend(mq, buf, len, 0);
}

int vmq_send(vmq_t *mq, const void *buf, size_t len)
{
    return vmq_timedsend(mq, buf, len, V_WAIT_FOREVER);
}

int vmq_timedrecv(vmq_t *mq, void *buf, size_t len, uint32_t timeout)
{
    if (!vmagic_verify(mq)) {
        return -1;
    }
    swque_t *q = (swque_t *)vmagic_get(mq);
    if (q == NULL) {
        return -1;
    }
    return swque_pop(q, buf, len, timeout);
}

int vmq_tryrecv(vmq_t *mq, void *buf, size_t len)
{
    return vmq_timedrecv(mq, buf, len, 0);
}

int vmq_recv(vmq_t *mq, void *buf, size_t len)
{
    return vmq_timedrecv(mq, buf, len, V_WAIT_FOREVER);
}

#else

typedef struct vmqueue {
    mqd_t       id;
    const char *name;
} vmqueue_t;

int vmq_create(vmq_t *mq, const char *name, size_t msg_size, size_t max_msgs, uint32_t options)
{
    vmqueue_t       * q;
    struct mq_attr    attr;

    if (name == NULL) {
        return -1;
    }
    if (vmagic_verify(mq)) {
        return -1;
    }

    q = (vmqueue_t *)malloc(sizeof(vmqueue_t));
    if (q == NULL) {
        return -1;
    }

    // attr.mq_maxmsg <= /proc/sys/fs/mqueue/msg_max (10)
    // attr.mq_msgsize <= /proc/sys/fs/mqueue/msgsize_max (8192)
    attr.mq_maxmsg  = max_msgs;
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
    vmagic_set(mq, q);

    return 0;
}

int vmq_delete(vmq_t *mq)
{
    if (!vmagic_verify(mq)) {
        return -1;
    }
    vmqueue_t *q = (vmqueue_t *)vmagic_get(mq);
    if (q == NULL || q->name == NULL) {
        return -1;
    }
    if (mq_close(q->id) != 0) {
        return -1;
    }
    mq_unlink(q->name);
    free(q);
    vmagic_reset(mq);

    return 0;
}

int vmq_timedsend(vmq_t *mq, const void *buf, size_t len, uint32_t timeout)
{
    if (!vmagic_verify(mq)) {
        return -1;
    }
    struct timespec ts;
    vmqueue_t *q = (vmqueue_t *)vmagic_get(mq);

    if (q == NULL) {
        return -1;
    }
    __set_timespec(&ts, timeout);

    return mq_timedsend(q->id, buf, len, 1, &ts);
}

int vmq_trysend(vmq_t *mq, const void *buf, size_t len)
{
    return vmq_timedsend(mq, buf, len, 0);
}

int vmq_send(vmq_t *mq, const void *buf, size_t len)
{
    if (!vmagic_verify(mq)) {
        return -1;
    }
    vmqueue_t *q = (vmqueue_t *)vmagic_get(mq);
    if (q == NULL) {
        return -1;
    }
    return mq_send(q->id, buf, len, 1);
}

int vmq_timedrecv(vmq_t *mq, void *buf, size_t len, uint32_t timeout)
{
    if (!vmagic_verify(mq)) {
        return -1;
    }
    struct timespec ts;
    vmqueue_t *q = (vmqueue_t *)vmagic_get(mq);

    if (q == NULL) {
        return -1;
    }
    __set_timespec(&ts, timeout);

    return mq_timedreceive(q->id, buf, len, NULL, &ts);
}

int vmq_tryrecv(vmq_t *mq, void *buf, size_t len)
{
    return vmq_timedrecv(mq, buf, len, 0);
}

int vmq_recv(vmq_t *mq, void *buf, size_t len)
{
    if (!vmagic_verify(mq)) {
        return -1;
    }
    vmqueue_t *q = (vmqueue_t *)vmagic_get(mq);
    if (q == NULL) {
        return -1;
    }
    return mq_receive(q->id, buf, len, NULL);
}
#endif // SWQUE

int vtimer_create(vtimer_t *timer, uint32_t interval, uint16_t mode,
                     void (*pfn)(uintptr_t arg), uintptr_t arg)
{
#ifdef CONFIG_SWTMR
    uint8_t  mod;
    swtmr_t *tmr;

    if (vmagic_verify(timer)) {
        return -1;
    }

    switch (mode)
    {
    case V_TIMER_MODE_REPEATED:
        mod = SWTMR_MODE_PERIOD;
        break;

    default:
        mod = SWTMR_MODE_ONCE;
        break;
    }
    tmr = swtmr_create("", interval, mod, (swtmr_proc_pfn)pfn, arg);
    if (tmr == NULL) {
        return -1;
    }
    vmagic_set(timer, tmr);
    return 0;
#else
    return -1;
#endif
}

int vtimer_delete(vtimer_t *timer)
{
#ifdef CONFIG_SWTMR
    if (!vmagic_verify(timer)) {
        return -1;
    }
    if (swtmr_delete((swtmr_t *)vmagic_get(timer)) != 0) {
        return -1;
    }
    vmagic_reset(timer);
    return 0;
#else
    return -1;
#endif
}

int vtimer_start(vtimer_t *timer)
{
#ifdef CONFIG_SWTMR
    if (!vmagic_verify(timer)) {
        return -1;
    }
    if (swtmr_start((swtmr_t *)vmagic_get(timer)) != 0) {
        return -1;
    }
    return 0;
#else
    return -1;
#endif
}

int vtimer_stop(vtimer_t *timer)
{
#ifdef CONFIG_SWTMR
    if (!vmagic_verify(timer)) {
        return -1;
    }
    if (swtmr_stop((swtmr_t *)vmagic_get(timer)) != 0) {
        return -1;
    }
    return 0;
#else
    return -1;
#endif
}

int vint_connect(unsigned int irq, unsigned int prio,
                 void (*handler)(uintptr_t), uintptr_t arg)
{
    return -1;
}

int vint_disconnect(unsigned int irq)
{
    return -1;
}

void *vmem_malloc(size_t size)
{
    if (size > 0) {
        return malloc(size);
    }
    return NULL;
}

void vmem_free(void *ptr)
{
    if (ptr != NULL) {
        free(ptr);
    }
}

void *vmem_realloc(void *ptr, size_t newsize)
{
    return (void *)realloc(ptr, newsize);
}

void vmem_stat(void)
{

}

uint64_t mseconds(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return (uint64_t)(tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
