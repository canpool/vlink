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

#ifndef __VFS_MOCKFS_H__
#define __VFS_MOCKFS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int mockfs_init(void);
int mockfs_mount(const char *path, void *data);
int mockfs_unmount(const char *path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VFS_MOCKFS_H__ */
