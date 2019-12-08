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

#ifndef __VSL_HASH_H__
#define __VSL_HASH_H__

#include "vsl.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int vsl_sha256(const unsigned char *input, unsigned int ilen, unsigned char output[32]);
int vsl_sha512(const unsigned char *input, unsigned int ilen, unsigned char output[64]);
int vsl_md5(const unsigned char *input, unsigned int ilen, unsigned char output[16]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VSL_HASH_H__ */