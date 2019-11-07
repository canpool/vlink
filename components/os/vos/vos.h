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

#ifndef __VOS_H__
#define __VOS_H__

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "vpool.h"
#include "vlog.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define V_TIMEOUT_FOREVER 0xffffffff

typedef uintptr_t vtask_t;
typedef uintptr_t vmutex_t;
typedef uintptr_t vsem_t;
typedef uintptr_t vevent_t;
typedef uintptr_t vmq_t;

#define V_TASK_INVALID  ((vtask_t)0xffffffff)
#define V_MUTEX_INVALID ((vmutex_t)0xffffffff)
#define V_SEM_INVALID   ((vmutex_t)0xffffffff)
#define V_EVENT_INVALID ((vmutex_t)0xffffffff)

#define V_EVENT_WAITMODE_AND    4  /* all bits must be set */
#define V_EVENT_WAITMODE_OR     2  /* any bit must be set  */
#define V_EVENT_WAITMODE_CLR    1  /* clear when satisfied */

int vos_init(void);

int vos_task_create(vtask_t *task, const char *name, int (*entry)(uintptr_t arg), uintptr_t arg,
                    int stacksize, int priority);
int vos_task_delete(vtask_t task);
int vos_task_resume(vtask_t task);
int vos_task_suspend(vtask_t task);
void vos_task_lock(void);
void vos_task_unlock(void);
void vos_task_sleep(uint32_t ms);

int vos_mutex_init(vmutex_t *mutex);
int vos_mutex_destroy(vmutex_t mutex);
int vos_mutex_timedlock(vmutex_t mutex, uint32_t timeout);
int vos_mutex_trylock(vmutex_t mutex);
int vos_mutex_lock(vmutex_t mutex);
int vos_mutex_unlock(vmutex_t mutex);

int vos_sem_init(vsem_t *sem, int limit, int value);
int vos_sem_destroy(vsem_t sem);
int vos_sem_timedwait(vsem_t sem, uint32_t timeout);
int vos_sem_trywait(vsem_t sem);
int vos_sem_wait(vsem_t sem);
int vos_sem_post(vsem_t sem);

int vos_event_init(vevent_t *event);
int vos_event_destroy(vevent_t event);
int vos_event_timedrecv(vevent_t event, uint32_t wanted, uint32_t option, uint32_t timeout, uint32_t *recved);
int vos_event_tryrecv(vevent_t event, uint32_t wanted, uint32_t option, uint32_t *recved);
int vos_event_recv(vevent_t event, uint32_t wanted, uint32_t option, uint32_t *recved);
int vos_event_send(vevent_t event, uint32_t value);
int vos_event_clear(vevent_t event, uint32_t value);

int vos_mq_create(vmq_t *mq, const char *name, size_t msg_size, size_t max_msgs, uint32_t options);
int vos_mq_delete(vmq_t mq);
int vos_mq_timedsend(vmq_t mq, const void *buf, size_t len, uint32_t timeout);
int vos_mq_trysend(vmq_t mq, const void *buf, size_t len);
int vos_mq_send(vmq_t mq, const void *buf, size_t len);
int vos_mq_timedrecv(vmq_t mq, void *buf, size_t len, uint32_t timeout);
int vos_mq_tryrecv(vmq_t mq, void *buf, size_t len);
int vos_mq_recv(vmq_t mq, void *buf, size_t len);

void *vos_malloc(size_t size);
void vos_free(void *ptr);
void *vos_zalloc(size_t size);
void *vos_realloc(void *ptr, size_t size);
void *vos_calloc(size_t n, size_t size);

uint64_t vos_sys_time(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VOS_H__ */
