/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VCOMPILER_MSVC_H__
#define __VCOMPILER_MSVC_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * v_inline - create no static inlined function
 */
#define v_inline                    static inline

/*
 * v_always_inline - create always static inlined function
 */
#define v_always_inline             static inline

/*
 * v_noreturn - create a non-return routine
 */
#define v_noreturn

/*
 * v_used - create a "used" symbol
 */
#define v_used

/*
 * v_unused - create a "unused" symbol
 */
#define v_unused

/*
 * v_weak - create a weak symbol
 */
#define v_weak

/*
 * likely - a condition is very likely to be true.
 * @exp: the bool exp
 */
#ifndef likely
#define likely(exp)                 (exp)
#endif

/*
 * unlikely - a condition is very unlikely to be true.
 * @exp: the bool exp.
 */
#ifndef unlikely
#define unlikely(exp)                (exp)
#endif

/*
 * v_section - place a symbol in a specific section
 * @name: the section name.
 */
#define v_section(name)

/*
 * v_section_start - import to the start of a section
 * @name: the section name.
 */
#define v_section_start(name)

/*
 * v_section_end - import to the end of a section
 * @name: the section name.
 */
#define v_section_end(name)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VCOMPILER_MSVC_H__ */
