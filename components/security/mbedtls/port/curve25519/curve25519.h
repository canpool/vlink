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

#ifndef __CURVE25519_H__
#define __CURVE25519_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CONFIG_PUBLICK_KEY_LEN
#define CONFIG_PUBLICK_KEY_LEN 32
#endif

#ifndef CONFIG_PRIVATE_KEY_LEN
#define CONFIG_PRIVATE_KEY_LEN 32
#endif

#ifndef CONFIG_SHARED_KEY_LEN
#define CONFIG_SHARED_KEY_LEN 32
#endif

int curve25519_init(void);
int curve25519_destroy(void);
int curve25519_gen_keypair(unsigned char public_key[CONFIG_PUBLICK_KEY_LEN],
                           unsigned char private_key[CONFIG_PRIVATE_KEY_LEN]);
int curve25519_compute_shared(unsigned char private_key[CONFIG_PRIVATE_KEY_LEN],
                              unsigned char peer_public_key[CONFIG_PUBLICK_KEY_LEN],
                              unsigned char shared_key[CONFIG_SHARED_KEY_LEN]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CURVE25519_H__ */