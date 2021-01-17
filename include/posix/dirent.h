/**
 * Copyright (c) [2019] maminjie <canpool@163.com>
 * Copyright (c) [2021] vlink Team. All rights reserved.
 * SPDX-License-Identifier: MulanPSL-2.0
 */

#ifndef __DIRENT_H__
#define __DIRENT_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* File types for `d_type'.  */
enum {
    DT_UNKNOWN = 0,
#define DT_UNKNOWN  DT_UNKNOWN
    DT_FIFO = 1,
#define DT_FIFO     DT_FIFO
    DT_CHR = 2,
#define DT_CHR      DT_CHR
    DT_DIR = 4,
#define DT_DIR      DT_DIR
    DT_BLK = 6,
#define DT_BLK      DT_BLK
    DT_REG = 8,
#define DT_REG      DT_REG
    DT_LNK = 10,
#define DT_LNK      DT_LNK
    DT_SOCK = 12,
#define DT_SOCK     DT_SOCK
    DT_WHT = 14
#define DT_WHT      DT_WHT
};

struct dirent {
    char    d_name[256];
    uint8_t d_type;
    size_t  d_size;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DIRENT_H__ */
