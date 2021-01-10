/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VLOG_H__
#define __VLOG_H__

#include "vpool.h"
#include "vconfig.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* tag */
#ifndef VLOG_TAG
#define VLOG_TAG        ""
#endif

/* priority or level */
#define VLOG_EMERG      0   /* system is unusable */
#define VLOG_ALERT      1   /* action must be taken immediately */
#define VLOG_CRIT       2   /* critical conditions */
#define VLOG_ERR        3   /* error conditions */
#define VLOG_WARNING    4   /* warning conditions */
#define VLOG_NOTICE     5   /* normal but significant condition */
#define VLOG_INFO       6   /* informational */
#define VLOG_DEBUG      7   /* debug-level messages */
#define VLOG_MAX        8

#ifdef CONFIG_VLOG

int vlog_print(const char *format, ...);

void vlog_set_level(int level);
int vlog_get_level(void);

const char *vlog_get_name(int level);
const char *vlog_get_time(void);

// TIME TAG[LEVEL][FUNCTION:LINE] MESSAGE
#define vlog_x(level, format, ...)                                          \
    do {                                                                    \
        if ((level) <= vlog_get_level())  {                                 \
            (void)vlog_print("%s %s[%s][%s:%d] " format "\r\n",             \
            vlog_get_time(), VLOG_TAG, vlog_get_name((level)),              \
            __FUNCTION__, __LINE__, ##__VA_ARGS__);                         \
        }                                                                   \
    } while (0)

#define vlog_buffer(buf, len)    vprint_buffer(stdout, buf, len, 0)

#else

#define vlog_x(level, format, ...)
#define vlog_buffer(buf, len)

#endif // CONFIG_VLOG

#define vlog_debug(format, ...)   vlog_x(VLOG_DEBUG,    format, ##__VA_ARGS__)
#define vlog_info(format, ...)    vlog_x(VLOG_INFO,     format, ##__VA_ARGS__)
#define vlog_notice(format, ...)  vlog_x(VLOG_NOTICE,   format, ##__VA_ARGS__)
#define vlog_warning(format, ...) vlog_x(VLOG_WARNING,  format, ##__VA_ARGS__)
#define vlog_error(format, ...)   vlog_x(VLOG_ERR,      format, ##__VA_ARGS__)
#define vlog_crit(format, ...)    vlog_x(VLOG_CRIT,     format, ##__VA_ARGS__)
#define vlog_alert(format, ...)   vlog_x(VLOG_ALERT     format, ##__VA_ARGS__)
#define vlog_emerg(format, ...)   vlog_x(VLOG_EMERG,    format, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VLOG_H__ */