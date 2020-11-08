/**
 * Copyright (c) [2020] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __VTYPES_H__
#define __VTYPES_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* typedefs */

typedef struct vstring {
    size_t        len;      /**< length of string */
    const char  * data;     /**< string data */
} vstring_t;

typedef struct vbinary {
    size_t        len;      /**< length of binary data */
    uint8_t     * data;     /**< binary data */
} vbinary_t;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VTYPES_H__ */
