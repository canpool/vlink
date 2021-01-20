/**
 * Copyright (c) [2020] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __HAL_H__
#define __HAL_H__

#include "vconfig.h"
#include "vtypes.h"
#include "vlist.h"
#include "verrno.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* typedefs */

typedef struct hal_dev hal_dev_t;

typedef struct hal_dev_ops {
    int     (*open)     (hal_dev_t *dev, int oflag, va_list args);
    int     (*close)    (hal_dev_t *dev);
    size_t  (*read)     (hal_dev_t *dev, int pos, void *buf, size_t len);
    size_t  (*write)    (hal_dev_t *dev, int pos, const void *buf, size_t len);
    int     (*ioctl)    (hal_dev_t *dev, int cmd, va_list args);
} hal_dev_ops_t;

typedef struct hal_dev {
    vdlist_t              node;
    const char          * name;
    const hal_dev_ops_t * ops;
    int                   flags;
} hal_dev_t;

/* externs */

hal_dev_t * hal_open        (const char *name, int oflag, ...);
int         hal_close       (hal_dev_t *dev);
size_t      hal_read        (hal_dev_t *dev, int pos, void *buf, size_t len);
size_t      hal_write       (hal_dev_t *dev, int pos, const void *buf, size_t len);
int         hal_ioctl       (hal_dev_t *dev, int cmd, ...);

hal_dev_t * hal_find        (const char *name);

int         hal_init        (void);
int         hal_exit        (void);

int         hal_register    (hal_dev_t *dev);
int         hal_unregister  (hal_dev_t *dev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __HAL_H__ */
