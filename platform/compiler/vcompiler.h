/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __V_COMPILER_H__
#define __V_COMPILER_H__

#if   defined (__GNUC__) || defined (__clang__)
#include "vcompiler-gnuc.h"
#elif defined (_MSC_VER)
#include "vcompiler-msvc.h"
#endif

#endif /* __V_COMPILER_H__ */
