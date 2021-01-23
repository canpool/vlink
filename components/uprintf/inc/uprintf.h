/**
 * Copyright (c) [2019-2020] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __UPRINTF_H__
#define __UPRINTF_H__

#include "vtypes.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int  uputc(int ch);
int  ugetc(void);

int  uprintf(const char *format, ...); // user-printf instead of printf, kprintf
int  vuprintf(const char *format, va_list args);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UPRINTF_H__ */
