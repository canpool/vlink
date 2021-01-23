/**
 * Copyright (c) [2019-2020] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VPRINTF_H__
#define __VPRINTF_H__

#include "vtypes.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int  xprintf(const char *format, va_list args,
             int (*output)(uintptr_t, unsigned char), uintptr_t outarg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VPRINTF_H__ */
