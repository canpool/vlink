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

#ifndef __JFFS2_TYPES_H__
#define __JFFS2_TYPES_H__

#include <stdint.h>
#include <stdio.h>	// for SEEK_SET SEEK_CUR SEEK_END
#include <string.h>

#include "jffs2_compiler.h"
#include "jffs2_errno.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if (CONFIG_LINUX_ENABLE || CONFIG_MACOS_ENABLE)
#include "sys/types.h"
#else
typedef unsigned short  nlink_t;
typedef long            off_t;
typedef unsigned short  gid_t;
typedef unsigned short  uid_t;
typedef unsigned int    ino_t;
typedef int             pid_t;
typedef int             ssize_t;
typedef unsigned long   mode_t;
typedef unsigned int    dev_t;
typedef int             time_t;
typedef unsigned char   u_char;
#endif

#define loff_t          off_t
#define kvec            iovec



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __JFFS2_TYPES_H__ */

