/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VOS_H__
#define __VOS_H__

#include "verrno.h"
#include "vmagic.h"
#include "vpool.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define V_WAIT_FOREVER 0xffffffff

typedef vmagic_t vtask_t;
typedef vmagic_t vmutex_t;
typedef vmagic_t vevent_t;
typedef vmagic_t vsem_t;
typedef vmagic_t vmq_t;
typedef vmagic_t vtimer_t;

/* module */
int vos_init(void);
int vos_exit(void);

/* task */
int  vtask_create(vtask_t *task, const char *name, int (*entry)(uintptr_t arg),
                  uintptr_t arg, int stacksize, int priority);
int  vtask_delete(vtask_t *task);
int  vtask_exit(void);
int  vtask_resume(vtask_t *task);
int  vtask_suspend(vtask_t *task);
void vtask_lock(void);
void vtask_unlock(void);
void vtask_sleep(uint32_t ms);
int  vtask_prio_set(vtask_t *task, int prio);
int  vtask_prio_get(vtask_t *task);
void vtask_stat(void);
int  vtask_waterline(vtask_t *task);

/* mutex */
int vmutex_init(vmutex_t *mutex);
int vmutex_destroy(vmutex_t *mutex);
int vmutex_timedlock(vmutex_t *mutex, uint32_t timeout);
int vmutex_trylock(vmutex_t *mutex);
int vmutex_lock(vmutex_t *mutex);
int vmutex_unlock(vmutex_t *mutex);

/* event */
#define V_EVENT_WAITMODE_AND 4 /* all bits must be set */
#define V_EVENT_WAITMODE_OR  2 /* any bit must be set  */
#define V_EVENT_WAITMODE_CLR 1 /* clear when satisfied */

int vevent_init(vevent_t *event);
int vevent_destroy(vevent_t *event);
int vevent_timedrecv(vevent_t *event, uint32_t wanted, uint32_t option, uint32_t timeout, uint32_t *recved);
int vevent_tryrecv(vevent_t *event, uint32_t wanted, uint32_t option, uint32_t *recved);
int vevent_recv(vevent_t *event, uint32_t wanted, uint32_t option, uint32_t *recved);
int vevent_send(vevent_t *event, uint32_t value);
int vevent_clear(vevent_t *event, uint32_t value);

/* semaphore */
int vsem_init(vsem_t *sem, unsigned int limit, unsigned int value);
int vsem_destroy(vsem_t *sem);
int vsem_timedwait(vsem_t *sem, uint32_t timeout);
int vsem_trywait(vsem_t *sem);
int vsem_wait(vsem_t *sem);
int vsem_post(vsem_t *sem);
int vsem_getvalue(vsem_t *sem, unsigned int *val);

/* message queue */
typedef struct vmq_buf {
    uint8_t *buf;
    size_t   len;
} vmq_buf_t;

int vmq_create(vmq_t *mq, const char *name, size_t msg_size, size_t max_msgs, uint32_t options);
int vmq_delete(vmq_t *mq);
int vmq_timedsend(vmq_t *mq, const void *buf, size_t len, uint32_t timeout);
int vmq_trysend(vmq_t *mq, const void *buf, size_t len);
int vmq_send(vmq_t *mq, const void *buf, size_t len);
int vmq_timedrecv(vmq_t *mq, void *buf, size_t len, uint32_t timeout);
int vmq_tryrecv(vmq_t *mq, void *buf, size_t len);
int vmq_recv(vmq_t *mq, void *buf, size_t len);

/* software timer */
#define V_TIMER_MODE_ONESHOT  0
#define V_TIMER_MODE_REPEATED 1

int vtimer_create(vtimer_t *timer, uint32_t interval, uint16_t mode,
                  void (*pfn)(uintptr_t arg), uintptr_t arg);
int vtimer_delete(vtimer_t *timer);
int vtimer_start(vtimer_t *timer);
int vtimer_stop(vtimer_t *timer);

/* interrupt */
int vint_connect(unsigned int irq, unsigned int prio, void (*handler)(uintptr_t), uintptr_t arg);
int vint_disconnect(unsigned int irq);

/* memory */
void *vmem_malloc(size_t size);
void *vmem_zalloc(size_t size);
void *vmem_realloc(void *ptr, size_t size);
void *vmem_calloc(size_t n, size_t size);
void  vmem_free(void *ptr);
void  vmem_stat(void);

#ifndef vmem_sfree
#define vmem_sfree(p) free_raw(p, vmem_free)
#endif

/* time */
uint64_t mseconds(void);
void     msleep(uint32_t ms);

#define vsystime() mseconds()


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VOS_H__ */
