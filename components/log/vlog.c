/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 *
 * vlink is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *    http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include "vlog.h"

static vlog_e g_vlog_level = VLOG_INFO;

void vlog_set_level(vlog_e level)
{
    g_vlog_level = level;
}

vlog_e vlog_get_level(void)
{
    return g_vlog_level;
}

#ifdef CONFIG_VLOG

static const char *g_log_names[] = {
    "DEBUG", "INFO", "NOTE", "WARN", "ERR", "CRIT", "ALRT", "EMRG",
};

const char *vlog_get_name(vlog_e log_level)
{
    if (log_level >= VLOG_MAX) {
        return "UNKOWN";
    }

    return g_log_names[log_level];
}

#endif
