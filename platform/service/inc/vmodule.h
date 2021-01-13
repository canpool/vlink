/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VMODULE_H__
#define __VMODULE_H__

#include "vlist.h"
#include "vpool.h"
#include "vconfig.h"
#include "vprintf.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* defines */

/* module group or type */
typedef enum {
    VMODULE_CPU = 0,
    VMODULE_BOARD,
    VMODULE_OS,
    VMODULE_HAL,
    VMODULE_BUS,
    VMODULE_PREDRIVER,
    VMODULE_DRIVER,
    VMODULE_POSTDRIVER,
    VMODULE_LIB,
    VMODULE_USER,
    VMODULE_MAX,
} vmoudle_e;

/* typedefs */

typedef int (*init_pfn)(void);
typedef int (*exit_pfn)(void);

typedef struct vmodule {
    init_pfn    init;
    exit_pfn    exit;
    vslist_t    list;
} vmodule_t;

/* macros */

#define __VMODULE_DEF(type, module, tag)                                            \
    void __attribute__((constructor, used))                                         \
    V_CONCAT(type##_##module##_, tag)(void) {                                       \
        vmodule_add(type, &module);                                                 \
    }

#ifdef CONFIG_VMODULE
#define VMODULE_DEF(type, init, exit)                                               \
    static vmodule_t s_##init##_##exit = {                                          \
        init, exit,                                                                 \
    };                                                                              \
    __VMODULE_DEF(type, s_##init##_##exit, __LINE__)
#else
#define VMODULE_DEF(type, init, exit)
#endif // CONFIG_VMODULE

/* externs */

void vmodule_add(vmoudle_e type, vmodule_t *module);

void vmodule_init(void);
void vmodule_exit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VMODULE_H__ */