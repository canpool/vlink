/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VMODULE_H__
#define __VMODULE_H__

#include "vlist.h"
#include "vpool.h"
#include "vconfig.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* typedefs */

typedef int (*init_pfn)(void);
typedef int (*exit_pfn)(void);

#ifdef CONFIG_VMODULE

#ifdef CONFIG_CONSTRUCTOR

/* module group */

typedef enum {
    VMODULE_CPU = 0,
#define vmodule_cpu             VMODULE_CPU
    VMODULE_BOARD,
#define vmodule_board           VMODULE_BOARD
    VMODULE_OS,
#define vmodule_os              VMODULE_OS
    VMODULE_HAL,
#define vmodule_hal             VMODULE_HAL
    VMODULE_BUS,
#define vmodule_bus             VMODULE_BUS
    VMODULE_PREDRIVER,
#define vmodule_predriver       VMODULE_PREDRIVER
    VMODULE_DRIVER,
#define vmodule_driver          VMODULE_DRIVER
    VMODULE_POSTDRIVER,
#define vmodule_postdriver      VMODULE_POSTDRIVER
    VMODULE_LIB,
#define vmodule_lib             VMODULE_LIB
    VMODULE_POSTLIB,
#define vmodule_postlib         VMODULE_POSTLIB
    VMODULE_USER,
#define vmodule_user            VMODULE_USER
    VMODULE_MAX,
} vmoudle_e;

typedef struct vmodule {
    init_pfn    init;
    exit_pfn    exit;
    vslist_t    node;
} vmodule_t;

/* macros */

#define __VMODULE_DEF(group, module, tag)                           \
    void __attribute__((constructor, used))                         \
    V_CONCAT(__vm_, V_CONCAT(module, tag))(void) {                  \
        vmodule_add(vmodule_##group, &module);                      \
    }

#define VMODULE_DEF(group, init, exit)                              \
    static vmodule_t V_CONCAT(group, init) = {                      \
        init, exit,                                                 \
    };                                                              \
    __VMODULE_DEF(group, V_CONCAT(group, init), __LINE__)

void vmodule_add(vmoudle_e group, vmodule_t *module);

#else // CONFIG_CONSTRUCTOR

typedef struct vmodule {
    init_pfn    init;
    exit_pfn    exit;
} vmodule_t;

#define __VMODULE_DEF(group, init, exit, tag)                           \
    const vmodule_t __vm##group##_##init##tag v_section(vm_##group) = { \
        init, exit                                                      \
    }
/*
 * VMODULE_DEF(group, init, exit) - add a module routine
 * @group: the module group name
 * @init:  the module init routine
 * @exit:  the module exit routine
 *
 * supported groups for now:
 *
 *     *) cpu
 *     *) board
 *     *) os
 *     *) hal
 *     *) bus
 *     *) predriver
 *     *) driver
 *     *) postdriver
 *     *) lib
 *     *) postlib
 *     *) user
 */
#define VMODULE_DEF(group, init, exit)                                  \
    __VMODULE_DEF(group, init, exit, __LINE__)

#endif // CONFIG_CONSTRUCTOR

#else
#define VMODULE_DEF(group, init, exit)
#endif // CONFIG_VMODULE

/* externs */

void vmodule_init(void);
void vmodule_exit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VMODULE_H__ */