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

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_printf printf
#define mbedtls_exit exit
#define MBEDTLS_EXIT_SUCCESS EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE EXIT_FAILURE
#endif /* MBEDTLS_PLATFORM_C */

#include "vsl_curve25519.h"
#include "vos.h"

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/entropy.h"

typedef struct __vsl_curve25519_context {
    mbedtls_ecdh_context ecdh;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
} vsl_curve25519_context;

static vsl_curve25519_context *s_ctx = NULL;

int vsl_curve25519_destroy(void)
{
    if (s_ctx == NULL) {
        return 0;
    }
    mbedtls_ecdh_free(&s_ctx->ecdh);
    mbedtls_ctr_drbg_free(&s_ctx->ctr_drbg);
    mbedtls_entropy_free(&s_ctx->entropy);

    vos_free(s_ctx);
    s_ctx = NULL;

    return 0;
}

int vsl_curve25519_init(void)
{
    int ret;
    const char pers[] = "ecdh";

    if (s_ctx != NULL) {
        return -1;
    }
    s_ctx = (vsl_curve25519_context *)vos_malloc(sizeof(vsl_curve25519_context));
    if (s_ctx == NULL) {
        return -1;
    }

    mbedtls_ecdh_init(&s_ctx->ecdh);
    mbedtls_ctr_drbg_init(&s_ctx->ctr_drbg);
    mbedtls_entropy_init(&s_ctx->entropy);

    if ((ret = mbedtls_ctr_drbg_seed(&s_ctx->ctr_drbg, mbedtls_entropy_func, &s_ctx->entropy,
                                     (const unsigned char *)pers, sizeof(pers))) != 0) {
        vlog_error("mbedtls_ctr_drbg_seed returned %d", ret);
        goto exit;
    }
    if ((ret = mbedtls_ecp_group_load(&s_ctx->ecdh.grp, MBEDTLS_ECP_DP_CURVE25519)) != 0) {
        vlog_error("mbedtls_ecp_group_load returned %d", ret);
        goto exit;
    }
    return 0;

exit:
    vsl_curve25519_destroy();
    return -1;
}

int vsl_curve25519_gen_keypair(unsigned char public_key[CONFIG_CURVE_PUBKEY_LEN],
                               unsigned char private_key[CONFIG_CURVE_PRIKEY_LEN])
{
    int ret = -1;

    if (public_key == NULL || private_key == NULL) {
        vlog_error("illegal input param");
        return -1;
    }
    if (vsl_curve25519_init() != 0) {
        vlog_error("vsl_curve25519_init failed");
        goto exit;
    }
    ret = mbedtls_ecdh_gen_public(&s_ctx->ecdh.grp, &s_ctx->ecdh.d, &s_ctx->ecdh.Q,
                                  mbedtls_ctr_drbg_random, &s_ctx->ctr_drbg);
    if (ret != 0) {
        vlog_error("mbedtls_ecdh_gen_public returned %d", ret);
        goto exit;
    }

    ret = mbedtls_mpi_write_binary(&s_ctx->ecdh.Q.X, public_key, CONFIG_CURVE_PUBKEY_LEN);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_write_binary returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_write_binary(&s_ctx->ecdh.d, private_key, CONFIG_CURVE_PRIKEY_LEN);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_write_binary returned %d", ret);
        goto exit;
    }
    ret = 0;

exit:
    vsl_curve25519_destroy();
    return (ret ? -1 : 0);
}

int vsl_curve25519_gen_shared(unsigned char private_key[CONFIG_CURVE_PRIKEY_LEN],
                              unsigned char peer_public_key[CONFIG_CURVE_PUBKEY_LEN],
                              unsigned char shared_key[CONFIG_CURVE_SHRKEY_LEN])
{
    int ret = -1;

    if (private_key == NULL || peer_public_key == NULL || shared_key == NULL) {
        vlog_error("illegal input param");
        return -1;
    }
    if (vsl_curve25519_init() != 0) {
        vlog_error("vsl_curve25519_init failed");
        goto exit;
    }

    ret = mbedtls_mpi_lset(&s_ctx->ecdh.Qp.Z, 1);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_lset returned %d", ret);
        goto exit;
    }

    ret = mbedtls_mpi_read_binary(&s_ctx->ecdh.d, private_key, CONFIG_CURVE_PRIKEY_LEN);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }

    ret = mbedtls_mpi_read_binary(&s_ctx->ecdh.Qp.X, peer_public_key, CONFIG_CURVE_PUBKEY_LEN);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }

    ret = mbedtls_ecdh_compute_shared(&s_ctx->ecdh.grp, &s_ctx->ecdh.z, &s_ctx->ecdh.Qp, &s_ctx->ecdh.d,
                                      mbedtls_ctr_drbg_random, &s_ctx->ctr_drbg);
    if (ret != 0) {
        vlog_error("mbedtls_ecdh_compute_shared returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_write_binary(&s_ctx->ecdh.z, shared_key, CONFIG_CURVE_SHRKEY_LEN);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_write_binary returned %d", ret);
        goto exit;
    }

    ret = 0;

exit:
    vsl_curve25519_destroy();
    return (ret ? -1 : 0);
}
