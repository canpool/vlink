/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VMAGIC_H__
#define __VMAGIC_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define V_MAGIC_INVALID_ID        (uintptr_t)0xffffffffU

typedef struct vmagic {
    uintptr_t magic;
    uintptr_t id;
} vmagic_t;

#define vmagic_verify(p)        (((uintptr_t)(p) != 0) && ((p)->magic == (uintptr_t)(p)))
#define vmagic_reset(p)         do {(p)->id = V_MAGIC_INVALID_ID; (p)->magic = 0;} while(0)
#define vmagic_set(p, v)        do {(p)->id = (uintptr_t)(v);(p)->magic = (uintptr_t)(p);} while(0)
#define vmagic_get(p)           ((p)->id)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VMAGIC_H__ */
