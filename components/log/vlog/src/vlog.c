/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#include "vlog.h"

#ifdef CONFIG_VLOG

static int g_vlog_level = VLOG_INFO;

void vlog_set_level(int level)
{
    g_vlog_level = bound(0, level, VLOG_MAX - 1);
}

int vlog_get_level(void)
{
    return g_vlog_level;
}

static const char *g_log_names[] = {
    "EMRG", "ALRT", "CRIT", "ERR", "WARN", "NOTE", "INFO", "DEBUG"
};

const char *vlog_get_name(int level)
{
    if (level < 0 || level >= VLOG_MAX) {
        return "UNKOWN";
    }

    return g_log_names[level];
}

#endif // CONFIG_VLOG
