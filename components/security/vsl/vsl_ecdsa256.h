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

#include "vsl.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CONFIG_ECDSA_PUBKEY_LEN 64 /* public key (X+Y) */
#define CONFIG_ECDSA_PRIKEY_LEN 32 /* private key */
#define CONFIG_ECDSA_SIG_LEN    64 /* raw signature (r+s), not encoded by ANS.1 */
#define CONFIG_ECDSA_SHRKEY_LEN 64 /* shared key (X+Y) */

#define CONFIG_CURVE_POINT_LEN  32

#ifndef CONFIG_CURVE_NAME_LEN
#define CONFIG_CURVE_NAME_LEN   20
#endif

#ifndef CONFIG_CSR_DERBUF_LEN
#define CONFIG_CSR_DERBUF_LEN   2048
#endif

#define VSL_SECP256R1_NAME  "secp256r1"
#define VSL_PRIME256V1_NAME "prime256v1"
#define VSL_SECP256K1_NAME  "secp256k1"
#define VSL_BP256R1_NAME    "brainpoolP256r1"
#define VSL_FRP256V1_NAME   "FRP256V1"


int vsl_ecdsa_gen_keypair(unsigned char public_key[CONFIG_ECDSA_PUBKEY_LEN],
                          unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN]);
int vsl_ecdsa_sign(unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN], unsigned char *data,
                   unsigned int data_len, unsigned char out_sig[CONFIG_ECDSA_SIG_LEN]);
int vsl_ecdsa_verify(unsigned char peer_public_key[CONFIG_ECDSA_PUBKEY_LEN], unsigned char *data,
                     unsigned int data_len, unsigned char in_sig[CONFIG_ECDSA_SIG_LEN]);
int vsl_ecdsa_gen_shared(unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN],
                         unsigned char peer_public_key[CONFIG_ECDSA_PUBKEY_LEN],
                         unsigned char shared_key[CONFIG_ECDSA_SHRKEY_LEN]);

// support: "secp256r1"="prime256v1", "secp256k1", "brainpoolP256r1", "FRP256V1"
// default: "secp256k1"
int vsl_ecdsa_set_curve(const char *name);

typedef struct __vsl_curve_point {
    unsigned char  p[CONFIG_CURVE_POINT_LEN];
    unsigned char  a[CONFIG_CURVE_POINT_LEN];
    unsigned char  b[CONFIG_CURVE_POINT_LEN];
    unsigned char gx[CONFIG_CURVE_POINT_LEN];
    unsigned char gy[CONFIG_CURVE_POINT_LEN];
    unsigned char  n[CONFIG_CURVE_POINT_LEN];
} vsl_curve_point;

int vsl_ecdsa_get_curve(char *name, unsigned int nlen, vsl_curve_point *points);
int vsl_ecdsa_def_curve(const char *name, vsl_curve_point *points);
int vsl_ecdsa_on_curve(unsigned char public_key[CONFIG_ECDSA_PUBKEY_LEN]);

int vsl_ecdsa_gen_csr(unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN], const char *subject,
                      char *buf, unsigned int *iolen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VSL_ECDSA256_H__ */