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

#ifndef __MBEDTLS_CONFIG_ECDSA_H__
#define __MBEDTLS_CONFIG_ECDSA_H__

/* mbed TLS modules */
#ifndef MBEDTLS_AES_C
#define MBEDTLS_AES_C
#endif

#ifndef MBEDTLS_ENTROPY_C
#define MBEDTLS_ENTROPY_C
#endif

#ifndef MBEDTLS_ENTROPY_C
#define MBEDTLS_ENTROPY_C
#endif

#ifndef MBEDTLS_SHA256_C
#define MBEDTLS_SHA256_C
#endif

#ifndef MBEDTLS_ECDSA_C
#define MBEDTLS_ECDSA_C
#endif

#ifndef MBEDTLS_ECP_C
#define MBEDTLS_ECP_C
#endif

#ifndef MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_PARSE_C
#endif

#ifndef MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_ASN1_WRITE_C
#endif

#ifndef MBEDTLS_BIGNUM_C
#define MBEDTLS_BIGNUM_C
#endif

#ifndef MBEDTLS_ECDH_LEGACY_CONTEXT
#define MBEDTLS_ECDH_LEGACY_CONTEXT
#endif

#ifndef MBEDTLS_ECP_DP_SECP256K1_ENABLED
#define MBEDTLS_ECP_DP_SECP256K1_ENABLED
#endif

#ifndef MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#endif

#ifndef MBEDTLS_ECP_DP_BP256R1_ENABLED
#define MBEDTLS_ECP_DP_BP256R1_ENABLED
#endif

#ifndef MBEDTLS_CTR_DRBG_C
#define MBEDTLS_CTR_DRBG_C
#endif

#include "mbedtls/check_config.h"

#endif // __MBEDTLS_CONFIG_ECDSA_H__