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

#ifndef __TESTFS_UTILS_H__
#define __TESTFS_UTILS_H__

#include "vfs.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int  create_file(const char *path);
extern void list_stat(const char *mp_path, const char *d_name);
extern void print_dir(const char *d_name, int level);
extern void print_stat(const char *name, struct stat *st);
extern char *sperm(int mode);
extern void waiting(unsigned int second);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TESTFS_UTILS_H__ */
