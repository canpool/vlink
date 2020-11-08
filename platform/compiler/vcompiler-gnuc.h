/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VCOMIPLER_GNUC_H__
#define __VCOMIPLER_GNUC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef  __KEIL__
#define SECTION_NAME_PREFIX
#define SECTION_SYM_PREFIX
#define SECTION_SYM_START               $$Base
#define SECTION_SYM_END                 $$Limit
#else
#define SECTION_NAME_PREFIX             "."
#define SECTION_SYM_PREFIX              __
#define SECTION_SYM_START               _start__
#define SECTION_SYM_END                 _end__
#endif

/**
 * v_inline - create no static inlined function
 */
#define v_inline                    static inline

/**
 * v_always_inline - create always static inlined function
 */
#define v_always_inline             __attribute__((always_inline)) static inline

/**
 * v_noreturn - create a non-return routine
 */
#define v_noreturn                  __attribute__ ((__noreturn__))

/**
 * v_used - create a "used" symbol
 */
#define v_used                      __attribute__((used))

/**
 * v_unused - create a "unused" symbol
 */
#define v_unused                    __attribute__((unused))

/**
 * v_weak - create a weak symbol
 */
#define v_weak                      __attribute__((weak))

/**
 * likely - a condition is very likely to be true.
 * @exp: the bool exp
 */
#ifndef likely
#define likely(exp)                 __builtin_expect(!!(exp), 1)
#endif

/**
 * unlikely - a condition is very unlikely to be true.
 * @exp: the bool exp.
 */
#ifndef unlikely
#define unlikely(exp)               __builtin_expect(!!(exp), 0)
#endif

/**
 * v_section - place a symbol in a specific section
 * @name: the section name.
 */
#define v_section(name)             \
    __attribute__ ((section (SECTION_NAME_PREFIX V_CVTSTR (name))))

/**
 * v_section_start - import to the start of a section
 * @name: the section name.
 */
#define v_section_start(name)       \
    V_CONCAT (V_CONCAT (SECTION_SYM_PREFIX, name), SECTION_SYM_START)

/**
 * v_section_end - import to the end of a section
 * @name: the section name.
 */
#define v_section_end(name)         \
    V_CONCAT (V_CONCAT (SECTION_SYM_PREFIX, name), SECTION_SYM_END)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VCOMIPLER_GNUC_H__ */
