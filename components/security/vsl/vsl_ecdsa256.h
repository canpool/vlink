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

#ifndef __VSL_ECDSA256_H__ /* base sha256 */
#define __VSL_ECDSA256_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CONFIG_ECDSA_PUBKEY_LEN 64 /* public key (X+Y) */
#define CONFIG_ECDSA_PRIKEY_LEN 32 /* private key */
#define CONFIG_ECDSA_SIG_LEN    64 /* raw signature (r+s), not encoded by ANS.1 */

int vsl_ecdsa_gen_keypair(unsigned char public_key[CONFIG_ECDSA_PUBKEY_LEN],
                      unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN]);
int vsl_ecdsa_sign(unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN], unsigned char *data,
               unsigned int data_len, unsigned char out_sig[CONFIG_ECDSA_SIG_LEN]);
int vsl_ecdsa_verify(unsigned char peer_public_key[CONFIG_ECDSA_PUBKEY_LEN], unsigned char *data,
                 unsigned int data_len, unsigned char in_sig[CONFIG_ECDSA_SIG_LEN]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VSL_ECDSA256_H__ */