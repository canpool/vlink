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

#ifndef __VPOOL_H__ /* share pool */
#define __VPOOL_H__

#include "compiler/compiler.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * offset_of - caculate the offset for a member in a struct
 * @t: the type of the container struct this is embedded in
 * @m: the name of the member within the struct
 */
#ifndef offset_of
#define offset_of(t, m)         ((uintptr_t)&((t *)0)->m)
#endif

/*
 * container_of - cast a member of a structure out to the containing structure
 * @p: the pointer to the member
 * @t: the type of the container struct this is embedded in
 * @m: the name of the member within the struct
 */
#ifndef container_of
#define container_of(p, t, m)   ((t *)(((char *)(p)) - (offset_of(t, m))))
#endif

/*
 * round_up - make a value to be aligned forward
 * @x: the value to be aligned
 * @a: the alignment value
 */
#ifndef round_up
#define round_up(x, a)          (((uintptr_t)(x) + ((uintptr_t)(a) - 1)) & ~((uintptr_t)(a) - 1))
#endif

/*
 * round_down - make a value to be aligned backward
 * @x: the value to be aligned
 * @a: the alignment value
 */
#ifndef round_down
#define round_down(x, a)        ((uintptr_t)(x) & ~((uintptr_t)(a) - 1))
#endif

#ifndef min
#define min(x, y)               ((x) < (y) ? (x) : (y))
#endif

#ifndef max
#define max(x, y)               ((x) > (y) ? (x) : (y))
#endif

#ifndef bound
#define bound(l, v, h)          max(l, min(v, h))
#endif


#ifndef safe_free
#define safe_free_raw(p, f)     do { if ((p) != NULL) { (f)((p)); (p) = NULL; } } while (0)
#define safe_free(p)            safe_free_raw(p, vos_free)
#endif

/*
 * ARRAY_SIZE - get the number of elements for an array
 * @a: the array name
 */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)           (sizeof(a) / sizeof((a)[0]))
#endif

/*
 * __CONCAT - make a new symbol by join two sub-strings
 * @s1: string 1
 * @s2: string 2
 */
#ifndef __CONCAT
#define __CONCAT_RAW(s1, s2)    s1##s2
#define __CONCAT(s1, s2)        __CONCAT_RAW(s1, s2)
#endif

/*
 * __CVTSTR - create a string as string -> "string"
 * @s: the input string
 */
#ifndef __CVTSTR
#define __CVTSTR_RAW(s)         #s
#define __CVTSTR(s)             __CVTSTR_RAW(s)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VPOOL_H__ */
