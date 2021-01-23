/**
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VBUG_H__
#define __VBUG_H__

#include "vpool.h"
#include "vconfig.h"

#ifdef CONFIG_UPRINTF
#include "uprintf.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * VWARN - print a warning message
 *
 * usage: VWARN(format, args), arguments is the same as uprintf
 */
#ifdef CONFIG_DEBUG
#define VWARN(...)                                                              \
    do {                                                                        \
        uprintf("\r\nWARNING, " __VA_ARGS__);                                   \
        uprintf("\r\n@ (file : %s, line : %d)\r\n", __FILE__, __LINE__);        \
    } while (0)
#else
#define VWARN(...)
#endif // CONFIG_DEBUG

/*
 * VWARN_ON - executive code and print a warning message when the result is true
 *
 * code:   the code to executive, note, it will be executived always regardless
 *         to the macro of CONFIG_DEBUG, if the code is comperation only, compiler
 *         can do correct optimization
 * action: action when the result is true, can be a code block or more than one
 *         sentence without ',' and can be empty (do nothing)
 *
 * examples:
 *
 *     VWARN_ON(xxx != 0,
 *              return,                         // action
 *              "xxx is %d", xxx);
 *
 *     VWARN_ON(xxx != 0, , "xxx is %d", xxx);  // action is empty
 *
 *     VWARN_ON(xxx == 0,
 *              errno = ERR_XXX; return -1,     // action is more than one
 *              "xxx is not supported");
 *
 *     VWARN_ON(xxx == 0,
 *              {
 *              errno = ERR_XXX;
 *              return -1;
 *              },
 *              "xxx is not supported");
 */
#define VWARN_ON(code, action, ...)                                             \
    do {                                                                        \
        if (unlikely(code)) {                                                   \
            VWARN("\"" V_CVTSTR(code) "\" unexpected!\r\n" __VA_ARGS__);        \
            action;                                                             \
        }                                                                       \
    } while (0)

/*
 * VBUG - enter bug state
 *
 * usage: VBUG(format, args), arguments is the same as uprintf
 */
#ifdef CONFIG_DEBUG
#define VBUG(...)                                                               \
    do {                                                                        \
        uprintf("\r\nBUG, " __VA_ARGS__);                                       \
        uprintf("\r\n@ (file : %s, line : %d)\r\n", __FILE__, __LINE__);        \
    } while (0)
#else
#define VBUG(...)
#endif // CONFIG_DEBUG

/*
 * VBUG_ON - executive code and enter to bug state when the result is true
 *
 * code:   the code to executive, note, it will be executived always regardless
 *         to the macro of CONFIG_DEBUG, if the code is comperation only, compiler
 *         can do correct optimization
 *
 * examples:
 *
 *     VBUG_ON(xxx != 0, "xxx is %d", xxx);
 *
 *     VBUG_ON(xxx == 0, "xxx is not supported");
 */
#define VBUG_ON(code, ...)                                                      \
    do {                                                                        \
        if ((code)) {                                                           \
            VBUG("\"" V_CVTSTR(code) "\" unexpected!\n" __VA_ARGS__);           \
            vbug("");                                                           \
        }                                                                       \
    } while (0)

/* externs */

extern void vbug(const char *info);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VBUG_H__ */
