/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef ssize_t
#define ssize_t         int
#endif

#ifndef off_t
#define off_t           long
#endif

#ifndef clock_t
#define clock_t         long
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SYS_TYPES_H__ */
