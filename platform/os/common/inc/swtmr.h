/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __SOFTWARE_TIMER_H__
#define __SOFTWARE_TIMER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* config */

#ifndef CONFIG_SWTMR_STACK_SIZE
#define CONFIG_SWTMR_STACK_SIZE     0x800
#endif

#ifndef CONFIG_SWTMR_TASK_PRIO
#define CONFIG_SWTMR_TASK_PRIO      10
#endif

/* enum */
enum swtmr_type {
    SWTMR_MODE_ONCE,                /**< One-off software timer */
    SWTMR_MODE_PERIOD,              /**< Periodic software timer */
};

/* typedef */

typedef void (*swtmr_proc_pfn)(uintptr_t arg);

typedef struct swtmr {
    struct swtmr  * next;
    const char    * name;
    uint32_t        interval;       /**< Timer interval (ms) */
    swtmr_proc_pfn  handler;
    uintptr_t       arg;
    uint64_t        timeleft;       /**< Timer timeleft (ms) */
    uint8_t         mode;
} swtmr_t;

/* module */

int swtmr_lib_init(void);
int swtmr_lib_exit(void);

/* externs */

swtmr_t *swtmr_create(const char *name, uint32_t interval, uint8_t mode,
                      swtmr_proc_pfn handler, uintptr_t arg);
int swtmr_delete(swtmr_t *timer);
int swtmr_start (swtmr_t *timer);
int swtmr_stop  (swtmr_t *timer);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SOFTWARE_TIMER_H__ */