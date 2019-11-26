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

#ifndef __VSL_CURVE25519_H__
#define __VSL_CURVE25519_H__

#include "vsl.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* public key */
#ifndef CONFIG_CURVE_PUBKEY_LEN
#define CONFIG_CURVE_PUBKEY_LEN 32
#endif

/* private key */
#ifndef CONFIG_CURVE_PRIKEY_LEN
#define CONFIG_CURVE_PRIKEY_LEN 32
#endif

/* shared key */
#ifndef CONFIG_CURVE_SHRKEY_LEN
#define CONFIG_CURVE_SHRKEY_LEN 32
#endif

int vsl_curve25519_gen_keypair(unsigned char public_key[CONFIG_CURVE_PUBKEY_LEN],
                               unsigned char private_key[CONFIG_CURVE_PRIKEY_LEN]);
int vsl_curve25519_gen_shared(unsigned char private_key[CONFIG_CURVE_PRIKEY_LEN],
                              unsigned char peer_public_key[CONFIG_CURVE_PUBKEY_LEN],
                              unsigned char shared_key[CONFIG_CURVE_SHRKEY_LEN]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VSL_CURVE25519_H__ */