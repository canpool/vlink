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

#ifndef __VLOG_H__
#define __VLOG_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef vprintf
#define vprintf printf
#endif

typedef enum {
    VLOG_DEBUG = 0,  /**< Debug */
    VLOG_INFO,       /**< Information */
    VLOG_NOTICE,     /**< Notice */
    VLOG_WARNING,    /**< Warning */
    VLOG_ERR,        /**< Error */
    VLOG_CRIT,       /**< Critical */
    VLOG_ALERT,      /**< Alert */
    VLOG_EMERG,      /**< Emergency */
    VLOG_MAX
} vlog_e;

void   vlog_set_level(vlog_e level);
vlog_e vlog_get_level(void);

#ifdef CONFIG_VLOG
const char *vlog_get_name(vlog_e level);

#define vlog(level, format, ...) \
    do { \
        if ((level) >= vlog_get_level())  { \
            (void)vprintf("[%s][%s:%d] " format "\r\n", vlog_get_name((level)), \
            __FILE__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)
#else
#define vlog(level, format, ...)
#endif

#define vlog_debug(format, ...)   vlog(VLOG_DEBUG,    format, ##__VA_ARGS__)
#define vlog_info(format, ...)    vlog(VLOG_INFO,     format, ##__VA_ARGS__)
#define vlog_notice(format, ...)  vlog(VLOG_NOTICE,   format, ##__VA_ARGS__)
#define vlog_warning(format, ...) vlog(VLOG_WARNING,  format, ##__VA_ARGS__)
#define vlog_error(format, ...)   vlog(VLOG_ERR,      format, ##__VA_ARGS__)
#define vlog_crit(format, ...)    vlog(VLOG_CRIT,     format, ##__VA_ARGS__)
#define vlog_alert(format, ...)   vlog(VLOG_ERR,      format, ##__VA_ARGS__)
#define vlog_emerg(format, ...)   vlog(VLOG_ERR,      format, ##__VA_ARGS__)


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __VLOG_H__ */
