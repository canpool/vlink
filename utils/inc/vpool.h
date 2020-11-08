/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VPOOL_H__ /* share pool */
#define __VPOOL_H__

#include "vcompiler.h"
#include "vtypes.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief caculate the offset for a member in a struct
 *
 * @param t [IN] the type of the container struct this is embedded in
 * @param m [IN] the name of the member within the struct
 */
#ifndef offset_of
#define offset_of(t, m)         ((uintptr_t)&((t *)0)->m)
#endif

/**
 * @brief cast a member of a structure out to the containing structure
 *
 * @param p [IN] the pointer to the member
 * @param t [IN] the type of the container struct this is embedded in
 * @param m [IN] the name of the member within the struct
 */
#ifndef container_of
#define container_of(p, t, m)   ((t *)(((char *)(p)) - (offset_of(t, m))))
#endif

/**
 * @brief make a value to be aligned forward
 *
 * @param x [IN] the value to be aligned
 * @param a [IN] the alignment value
 */
#ifndef round_up
#define round_up(x, a)          (((uintptr_t)(x) + ((uintptr_t)(a) - 1)) & ~((uintptr_t)(a) - 1))
#endif

/**
 * @brief make a value to be aligned backward
 *
 * @param x [IN] the value to be aligned
 * @param a [IN] the alignment value
 */
#ifndef round_down
#define round_down(x, a)        ((uintptr_t)(x) & ~((uintptr_t)(a) - 1))
#endif

/**
 * @brief check if a value is aligned
 *
 * @param x [IN] the value to be checked
 * @param a [IN] the alignment value
 */
#define aligned_at(x, a)        (((uintptr_t)(x) & ((uintptr_t)(a) - 1)) == 0)

/**
 * @brief get the min value of two
 *
 * @param x [IN] the 1st value
 * @param y [IN] the 2nd value
 */
#ifndef min
#define min(x, y)               ((x) < (y) ? (x) : (y))
#endif

/**
 * @brief get the max value of two
 *
 * @param x [IN] the 1st value
 * @param y [IN] the 2nd value
 */
#ifndef max
#define max(x, y)               ((x) > (y) ? (x) : (y))
#endif

/**
 * @brief constrain the range of a number
 *
 * @param l [IN] the low value
 * @param v [IN] the real value
 * @param h [IN] the high value
 *
 * eg:
 *  bound(2, 1, 3) => 2
 *  bound(2, 5, 3) => 3
 *  bound(1, 3, 5) => 3
 */
#ifndef bound
#define bound(l, v, h)          max(l, min(v, h))
#endif

/**
 * @brief raw free
 *
 * @param p [IN] the pointer to valid memory
 * @param f [IN] the free routine (function)
 */
#ifndef free_raw
#define free_raw(p, f)          do { if ((p) != NULL) { (f)((p)); (p) = NULL; } } while (0)
#endif

/**
 * @brief get the number of elements for an array
 *
 * @param a [IN] the array name
 */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)           (sizeof(a) / sizeof((a)[0]))
#endif

/**
 * @brief make a new symbol by join two sub-strings
 *
 * @param s1 [IN] string 1
 * @param s2 [IN] string 2
 *
 * eg:
 *  V_CONCAT(can, pool) ==> canpool
 */
#define __CONCAT_RAW(s1, s2)    s1 ## s2

#ifndef V_CONCAT
#define V_CONCAT(s1, s2)        __CONCAT_RAW(s1, s2)
#endif

/**
 * @brief convert symbol as string
 *
 * @param s [IN] the input symbol
 *
 * eg:
 *  V_CVTSTR(canpool) ==> "canpool"
 */
#define __CVTSTR_RAW(s)         #s

#ifndef V_CVTSTR
#define V_CVTSTR(s)             __CVTSTR_RAW(s)
#endif

/* character operation */

#ifndef ISDIGIT
#define ISDIGIT(c)              ('0' <= (c) && (c) <= '9')
#endif

#ifndef ISXDIGIT
#define ISXDIGIT(c)             (((c) - '0' < 10) || (((c) | 0x20) - 'a' < 6))
#endif

#ifndef ISLOWER
#define ISLOWER(c)              ('a' <= (c) && (c) <= 'z')
#endif

#ifndef TOUPPER
#define TOUPPER(c)              (ISLOWER(c) ? 'A' + ((c) - 'a') : (c))
#endif

#ifndef V_UNUSED
#define V_UNUSED(x)             (void)(x)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VPOOL_H__ */
