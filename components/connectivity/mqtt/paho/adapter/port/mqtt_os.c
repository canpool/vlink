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

#include "mqtt_os.h"

void TimerInit(Timer* timer)
{
	timer->end_time = (unsigned long long)vos_sys_time();
}

char TimerIsExpired(Timer* timer)
{
    unsigned long long now = (unsigned long long)vos_sys_time();
    return now >= timer->end_time;
}

void TimerCountdownMS(Timer* timer, unsigned int timeout)
{
	unsigned long long now = (unsigned long long)vos_sys_time();
    timer->end_time = now + timeout;
}

void TimerCountdown(Timer* timer, unsigned int timeout)
{
	TimerCountdownMS(timer, timeout * 1000);
}

int TimerLeftMS(Timer* timer)
{
	unsigned long long now = (unsigned long long)vos_sys_time();
    return (timer->end_time <= now) ? 0 : (timer->end_time - now);
}

int ThreadStart(Thread *thread, void (*fn)(void *), void *arg)
{
    return -1;
}