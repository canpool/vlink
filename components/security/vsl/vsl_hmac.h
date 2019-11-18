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

#ifndef __VSL_HMAC_H__
#define __VSL_HMAC_H__

#include "vsl.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CONFIG_MACH_LEN
#define CONFIG_MACH_LEN     32
#endif

int vsl_hmac(const unsigned char *key, size_t keylen, const unsigned char *input,
             size_t ilen, unsigned char output[CONFIG_MACH_LEN]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VSL_HMAC_H__ */