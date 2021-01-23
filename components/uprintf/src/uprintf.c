/**
 * Copyright (c) [2019-2020] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "uprintf.h"
#include "vpool.h"

#include <ctype.h>
#include <limits.h>
#include <string.h>

int uprintf(const char *format, ...)
{
    int     ret;
    va_list valist;

    va_start(valist, format);
    ret = vuprintf(format, valist);
    va_end(valist);

    return ret;
}

v_weak int vuprintf(const char *format, va_list args)
{
    return vprintf(format, args);
}

v_weak int uputc(int ch)
{
    return putchar(ch);
}

v_weak int ugetc(void)
{
    return getchar();
}
