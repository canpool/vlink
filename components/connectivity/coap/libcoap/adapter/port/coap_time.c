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
#include "libcoap.h"
#include "coap_time.h"

static coap_tick_t coap_clock_offset = 0;

void coap_clock_init(void)
{
    coap_clock_offset = vos_sys_time() / 1000;
}

/**
 * Sets @p t to the internal time with COAP_TICKS_PER_SECOND resolution.
 */
void coap_ticks(coap_tick_t *t)
{
    unsigned long long ms = vos_sys_time();

    *t += (ms / 1000 - coap_clock_offset) * COAP_TICKS_PER_SECOND +
          (ms % 1000) * COAP_TICKS_PER_SECOND / 1000;
}

/**
 * Helper function that converts coap ticks to wallclock time. On POSIX, this
 * function returns the number of seconds since the epoch. On other systems, it
 * may be the calculated number of seconds since last reboot or so.
 *
 * @param t Internal system ticks.
 *
 * @return  The number of seconds that has passed since a specific reference
 *          point (seconds since epoch on POSIX).
 */
coap_time_t coap_ticks_to_rt(coap_tick_t t)
{
    return coap_clock_offset + (t / COAP_TICKS_PER_SECOND);
}

/**
* Helper function that converts coap ticks to POSIX wallclock time in us.
*
* @param t Internal system ticks.
*
* @return  The number of seconds that has passed since a specific reference
*          point (seconds since epoch on POSIX).
*/
uint64_t coap_ticks_to_rt_us(coap_tick_t t)
{
    return (uint64_t)coap_clock_offset * 1000000 + (uint64_t)t * 1000000 / COAP_TICKS_PER_SECOND;
}

/**
* Helper function that converts POSIX wallclock time in us to coap ticks.
*
* @param t POSIX time is us
*
* @return  coap ticks
*/
coap_tick_t coap_ticks_from_rt_us(uint64_t t)
{
    return (coap_tick_t)((t - (uint64_t)coap_clock_offset * 1000000) * COAP_TICKS_PER_SECOND / 1000000);
}

