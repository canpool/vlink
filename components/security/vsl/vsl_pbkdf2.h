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

#ifndef __VSL_PBKDF2_H__
#define __VSL_PBKDF2_H__

#include "vsl.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// pkcs5_pbkdf2_hmac
int vsl_pbkdf2(const unsigned char *pwd, unsigned int plen,
               const unsigned char *salt, unsigned int slen,
               unsigned int iter_cnt,
               unsigned int key_len, unsigned char *output);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VSL_PBKDF2_H__ */