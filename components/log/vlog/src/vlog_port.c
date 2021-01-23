/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "vlog.h"

#include "uprintf.h"
#include "vos.h"

#ifdef CONFIG_VLOG

v_weak int vlog_print(const char *format, ...)
{
    int     ret;
    va_list valist;

    va_start(valist, format);
    ret = vuprintf(format, valist);
    va_end(valist);

    return ret;
}

v_weak const char *vlog_get_time(void)
{
    static char t[21] = {0};
    (void)snprintf(t, sizeof(t), "%lu", mseconds());
    return t;
}

#endif
