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

#ifndef __MQTT_OS_H__
#define __MQTT_OS_H__

#include <stdint.h>

#include "vos.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MQTT_TASK 1

typedef struct Timer
{
	unsigned long long end_time;
} Timer;

void TimerInit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);

typedef vmutex_t  Mutex;
#define MutexInit(mutex)        vos_mutex_init((mutex))
#define MutexLock(mutex)        vos_mutex_lock(*(mutex))
#define MutexUnlock(mutex)      vos_mutex_unlock(*(mutex))
#define MutexDestory(mutex)     vos_mutex_destroy(mutex)


typedef struct
{
	void * no_used;
} Thread;
int ThreadStart(Thread *thread, void (*fn)(void *), void *arg);


typedef struct Network
{
    uintptr_t ctx; // context or fd
    uintptr_t arg;
    int (*mqttread) (struct Network*, unsigned char*, int, int);
    int (*mqttwrite) (struct Network*, unsigned char*, int, int);
} Network;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MQTT_OS_H__ */