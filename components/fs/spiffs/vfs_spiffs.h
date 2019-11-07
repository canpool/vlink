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

#ifndef __VFS_SPIFFS_H__
#define __VFS_SPIFFS_H__

#include "spiffs.h"
#include "spiffs_config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int spiffs_init(void);
int spiffs_mount(const char *path, spiffs_config *config);
int spiffs_unmount(const char *path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VFS_SPIFFS_H__ */
