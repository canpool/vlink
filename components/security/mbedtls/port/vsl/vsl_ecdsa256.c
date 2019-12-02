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

#include "vsl_ecdsa256.h"
#include "vos.h"

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/sha256.h"

#include <string.h>


typedef struct __vsl_ecdsa256_context {
    mbedtls_ecdsa_context    ecdsa;
    mbedtls_entropy_context  entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
} vsl_ecdsa256_context;

static vsl_ecdsa256_context *s_ctx = NULL;
static int s_curve_id = MBEDTLS_ECP_DP_SECP256K1;

int vsl_ecdsa_destroy(void)
{
    if (s_ctx == NULL) {
        return 0;
    }
    mbedtls_ecdsa_free(&s_ctx->ecdsa);
    mbedtls_ctr_drbg_free(&s_ctx->ctr_drbg);
    mbedtls_entropy_free(&s_ctx->entropy);

    vos_free(s_ctx);
    s_ctx = NULL;

    return 0;
}

int vsl_ecdsa_init(void)
{
    int ret;
    const char pers[] = "ecdsa";

    if (s_ctx != NULL) {
        return -1;
    }
    s_ctx = (vsl_ecdsa256_context *)vos_malloc(sizeof(vsl_ecdsa256_context));
    if (s_ctx == NULL) {
        return -1;
    }

    mbedtls_ecdsa_init(&s_ctx->ecdsa);
    mbedtls_ctr_drbg_init(&s_ctx->ctr_drbg);
    mbedtls_entropy_init(&s_ctx->entropy);

    if ((ret = mbedtls_ctr_drbg_seed(&s_ctx->ctr_drbg, mbedtls_entropy_func, &s_ctx->entropy,
                                     (const unsigned char *)pers, strlen(pers))) != 0) {
        vlog_error("mbedtls_ctr_drbg_seed returned %d", ret);
        goto exit;
    }
    return 0;

exit:
    vsl_ecdsa_destroy();
    return -1;
}

int vsl_ecdsa_gen_keypair(unsigned char public_key[CONFIG_ECDSA_PUBKEY_LEN],
                      unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN])
{
    int ret = -1;
    size_t len;

    if (public_key == NULL || private_key == NULL) {
        vlog_error("illegal input param");
        return -1;
    }

    if (vsl_ecdsa_init() != 0) {
        vlog_error("vsl_ecdsa_init failed");
        goto exit;
    }

    if ((ret = mbedtls_ecdsa_genkey(&s_ctx->ecdsa, s_curve_id, mbedtls_ctr_drbg_random,
                                    &s_ctx->ctr_drbg)) != 0) {
        vlog_error("mbedtls_ecdsa_genkey returned %d", ret);
        goto exit;
    }

    ret = mbedtls_mpi_write_binary(&s_ctx->ecdsa.Q.X, public_key, CONFIG_ECDSA_PUBKEY_LEN / 2);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_write_binary returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_write_binary(&s_ctx->ecdsa.Q.Y, public_key + CONFIG_ECDSA_PUBKEY_LEN / 2,
                                   CONFIG_ECDSA_PUBKEY_LEN / 2);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_write_binary returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_write_binary(&s_ctx->ecdsa.d, private_key, CONFIG_ECDSA_PRIKEY_LEN);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_write_binary returned %d", ret);
        goto exit;
    }

    ret = 0;

exit:
    vsl_ecdsa_destroy();
    return (ret ? -1 : 0);
}

#include "mbedtls/platform_util.h"
/* Parameter validation macros based on platform_util.h */
#define ECDSA_VALIDATE_RET( cond )    \
    MBEDTLS_INTERNAL_VALIDATE_RET( cond, MBEDTLS_ERR_ECP_BAD_INPUT_DATA )
/*
 * Compute and write signature
 */
static int __mbedtls_ecdsa_write_signature( mbedtls_ecdsa_context *ctx,
                                 mbedtls_md_type_t md_alg,
                                 const unsigned char *hash, size_t hlen,
                                 unsigned char *sig, size_t slen,
                                 int (*f_rng)(void *, unsigned char *, size_t),
                                 void *p_rng )
{
    int ret;
    mbedtls_mpi r, s;
    ECDSA_VALIDATE_RET( ctx  != NULL );
    ECDSA_VALIDATE_RET( hash != NULL );
    ECDSA_VALIDATE_RET( sig  != NULL );

    mbedtls_mpi_init( &r );
    mbedtls_mpi_init( &s );

    (void) md_alg;

    MBEDTLS_MPI_CHK( mbedtls_ecdsa_sign( &ctx->grp, &r, &s, &ctx->d,
                         hash, hlen, f_rng, p_rng ) );

    // take care the boundary of sig
    MBEDTLS_MPI_CHK( mbedtls_mpi_write_binary( &r, sig, slen / 2 ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_write_binary( &s, sig + slen / 2, slen / 2 ) );

cleanup:
    mbedtls_mpi_free( &r );
    mbedtls_mpi_free( &s );

    return( ret );
}

/*
 * Restartable read and check signature
 */
static int __mbedtls_ecdsa_read_signature( mbedtls_ecdsa_context *ctx,
                          const unsigned char *hash, size_t hlen,
                          const unsigned char *sig, size_t slen)
{
    int ret;
    mbedtls_mpi r, s;
    ECDSA_VALIDATE_RET( ctx  != NULL );
    ECDSA_VALIDATE_RET( hash != NULL );
    ECDSA_VALIDATE_RET( sig  != NULL );

    mbedtls_mpi_init( &r );
    mbedtls_mpi_init( &s );

    // take care the boundary of sig
    if( ( ret = mbedtls_mpi_read_binary( &r, sig, slen / 2 ) ) != 0 ||
        ( ret = mbedtls_mpi_read_binary( &s, sig + slen / 2, slen / 2 ) ) != 0 )
    {
        ret += MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
        goto cleanup;
    }
    if( ( ret = mbedtls_ecdsa_verify( &ctx->grp, hash, hlen,
                                      &ctx->Q, &r, &s ) ) != 0 ) {
        goto cleanup;
    }

cleanup:
    mbedtls_mpi_free( &r );
    mbedtls_mpi_free( &s );

    return( ret );
}

int vsl_ecdsa_sign(unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN], unsigned char *data,
               unsigned int data_len, unsigned char out_sig[CONFIG_ECDSA_SIG_LEN])
{
    int ret = -1;
    unsigned char hash[32] = {0};

    if (data == NULL || data_len == 0 || out_sig == NULL) {
        vlog_error("illegal input param");
        return -1;
    }
    if (vsl_ecdsa_init() != 0) {
        vlog_error("vsl_ecdsa_init failed");
        goto exit;
    }
    mbedtls_ecp_group_load(&s_ctx->ecdsa.grp, s_curve_id);

    if ((ret = mbedtls_sha256_ret(data, data_len, hash, 0)) != 0) {
        vlog_error("mbedtls_sha256_ret returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&s_ctx->ecdsa.d, private_key, CONFIG_ECDSA_PRIKEY_LEN);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    // take care the boundary of out_sig
    if ((ret = __mbedtls_ecdsa_write_signature(&s_ctx->ecdsa, MBEDTLS_MD_SHA256, hash, sizeof(hash), out_sig,
            CONFIG_ECDSA_SIG_LEN, mbedtls_ctr_drbg_random, &s_ctx->ctr_drbg)) != 0) {
        vlog_error("mbedtls_ecdsa_write_signature returned %d", ret);
        goto exit;
    }
    ret = 0;

exit:
    vsl_ecdsa_destroy();
    return (ret ? -1 : 0);
}

int vsl_ecdsa_verify(unsigned char peer_public_key[CONFIG_ECDSA_PUBKEY_LEN], unsigned char *data,
                 unsigned int data_len, unsigned char in_sig[CONFIG_ECDSA_SIG_LEN])
{
    int ret = -1;
    unsigned char hash[32] = {0};
    size_t sig_len;

    if (peer_public_key == NULL || data == NULL || data_len == 0 || in_sig == NULL) {
        vlog_error("illegal input param");
        return -1;
    }

    if (vsl_ecdsa_init() != 0) {
        vlog_error("vsl_ecdsa_init failed");
        goto exit;
    }
    mbedtls_ecp_group_load(&s_ctx->ecdsa.grp, s_curve_id);

    ret = mbedtls_mpi_lset(&s_ctx->ecdsa.Q.Z, 1);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_lset returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&s_ctx->ecdsa.Q.X, peer_public_key, CONFIG_ECDSA_PUBKEY_LEN / 2);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&s_ctx->ecdsa.Q.Y, peer_public_key + CONFIG_ECDSA_PUBKEY_LEN / 2,
                                  CONFIG_ECDSA_PUBKEY_LEN / 2);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    if ((ret = mbedtls_sha256_ret(data, data_len, hash, 0)) != 0) {
        vlog_error("mbedtls_sha256_ret returned %d", ret);
        goto exit;
    }
    // Verify signature
    if ((ret = __mbedtls_ecdsa_read_signature(&s_ctx->ecdsa, hash, sizeof(hash), in_sig, CONFIG_ECDSA_SIG_LEN)) != 0) {
        vlog_error("mbedtls_ecdsa_read_signature returned %d", ret);
        goto exit;
    }

    ret = 0;

exit:
    vsl_ecdsa_destroy();
    return (ret ? -1 : 0);
}

static int __compute_shared_restartable( mbedtls_ecp_group *grp,
                         unsigned char *z, int zlen,
                         const mbedtls_ecp_point *Q, const mbedtls_mpi *d,
                         int (*f_rng)(void *, unsigned char *, size_t),
                         void *p_rng )
{
    int ret;
    mbedtls_ecp_point P;

    mbedtls_ecp_point_init( &P );

    MBEDTLS_MPI_CHK( mbedtls_ecp_mul_restartable( grp, &P, d, Q,
                                                  f_rng, p_rng, NULL ) );

    if( mbedtls_ecp_is_zero( &P ) )
    {
        ret = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
        goto cleanup;
    }

    MBEDTLS_MPI_CHK( mbedtls_mpi_write_binary( &P.X, z, zlen / 2 ) );
    MBEDTLS_MPI_CHK( mbedtls_mpi_write_binary( &P.Y, z + zlen / 2, zlen / 2 ) );

cleanup:
    mbedtls_ecp_point_free( &P );

    return( ret );
}

int vsl_ecdsa_gen_shared(unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN],
                         unsigned char peer_public_key[CONFIG_ECDSA_PUBKEY_LEN],
                         unsigned char shared_key[CONFIG_ECDSA_SHRKEY_LEN])
{
    int ret = -1;

    if (private_key == NULL || peer_public_key == NULL || shared_key == NULL) {
        vlog_error("illegal input param");
        return -1;
    }
    if (vsl_ecdsa_init() != 0) {
        vlog_error("vsl_ecdsa_init failed");
        goto exit;
    }
    mbedtls_ecp_group_load(&s_ctx->ecdsa.grp, s_curve_id);

    ret = mbedtls_mpi_lset(&s_ctx->ecdsa.Q.Z, 1);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_lset returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&s_ctx->ecdsa.Q.X, peer_public_key, CONFIG_ECDSA_PUBKEY_LEN / 2);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&s_ctx->ecdsa.Q.Y, peer_public_key + CONFIG_ECDSA_PUBKEY_LEN / 2,
                                  CONFIG_ECDSA_PUBKEY_LEN / 2);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }

    ret = mbedtls_mpi_read_binary(&s_ctx->ecdsa.d, private_key, CONFIG_ECDSA_PRIKEY_LEN);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }

    ret = __compute_shared_restartable(&s_ctx->ecdsa.grp, shared_key, CONFIG_ECDSA_SHRKEY_LEN,
        &s_ctx->ecdsa.Q, &s_ctx->ecdsa.d, mbedtls_ctr_drbg_random, &s_ctx->ctr_drbg);
    if (ret != 0) {
        vlog_error("__compute_shared_restartable returned %d", ret);
        goto exit;
    }

    ret = 0;

exit:
    vsl_ecdsa_destroy();
    return (ret ? -1 : 0);
}


int vsl_ecdsa_set_curve(const char *name)
{
    if (name == NULL) {
        return -1;
    }

    if (strcmp(name, "secp256r1") == 0) {
        s_curve_id = MBEDTLS_ECP_DP_SECP256R1;
    } else if (strcmp(name, "secp256k1") == 0) {
        s_curve_id = MBEDTLS_ECP_DP_SECP256K1;
    } else if (strcmp(name, "brainpoolP256r1") == 0) {
        s_curve_id = MBEDTLS_ECP_DP_BP256R1;
    } else {
        return -1;
    }
    return 0;
}

#ifdef CONFIG_ECDSA256_CSR_MBEDTLS

#include "mbedtls/x509_csr.h"
#include "mbedtls/pem.h"
#include "mbedtls/pk.h"

typedef struct __vsl_csr_context {
    mbedtls_pk_context       key;
    mbedtls_x509write_csr    req;
} vsl_csr_context;

static vsl_csr_context *s_csr_ctx = NULL;

static int __vsl_csr_destroy(void)
{
    if (s_csr_ctx == NULL) {
        return 0;
    }
    mbedtls_x509write_csr_free(&s_csr_ctx->req);
    mbedtls_platform_zeroize(&s_csr_ctx->key, sizeof(mbedtls_pk_context));

    vos_free(s_csr_ctx);
    s_csr_ctx = NULL;

    return 0;
}

static int __vsl_csr_init(void)
{
    if (s_csr_ctx != NULL) {
        return -1;
    }
    s_csr_ctx = (vsl_csr_context *)vos_malloc(sizeof(vsl_csr_context));
    if (s_csr_ctx == NULL) {
        return -1;
    }

    mbedtls_x509write_csr_init(&s_csr_ctx->req);
    mbedtls_pk_init(&s_csr_ctx->key);

    return 0;
}

#define PEM_BEGIN_CSR           "-----BEGIN CERTIFICATE REQUEST-----\n"
#define PEM_END_CSR             "-----END CERTIFICATE REQUEST-----\n"

static int __mbedtls_x509write_csr_pem(mbedtls_x509write_csr *ctx, unsigned char *buf, size_t *size,
                                       int (*f_rng)(void *, unsigned char *, size_t), void *p_rng)
{
    int ret;
    unsigned char *output_buf = NULL;
    size_t olen = 0;

    output_buf = (unsigned char *)vos_malloc(CONFIG_CSR_DERBUF_LEN);
    if (output_buf == NULL) {
        return -1;
    }
    memset(output_buf, 0, CONFIG_CSR_DERBUF_LEN);

    if ((ret = mbedtls_x509write_csr_der(ctx, output_buf, CONFIG_CSR_DERBUF_LEN,
            f_rng, p_rng)) < 0) {
        vlog_error("mbedtls_x509write_csr_der returned %d", ret);
        goto exit;
    }

    if ((ret = mbedtls_pem_write_buffer(PEM_BEGIN_CSR, PEM_END_CSR,
            output_buf + CONFIG_CSR_DERBUF_LEN - ret, ret, buf, *size, &olen)) != 0) {
        vlog_error("mbedtls_pem_write_buffer returned %d", ret);
        goto exit;
    }
    *size = olen;

exit:
    vos_free(output_buf);
    return ret;
}

int vsl_ecdsa_gen_csr(unsigned char private_key[CONFIG_ECDSA_PRIKEY_LEN], const char *subject,
                      char *buf, unsigned int *iolen)
{
    int ret = -1;

    if (private_key == NULL || subject == NULL || buf == NULL || iolen == NULL) {
        vlog_error("illegal input param");
        return -1;
    }
    if (vsl_ecdsa_init() != 0) {
        vlog_error("vsl_ecdsa_init failed");
        goto exit;
    }
    if (__vsl_csr_init() != 0) {
        vlog_error("__vsl_csr_init failed");
        goto exit;
    }
    mbedtls_ecp_group_load(&s_ctx->ecdsa.grp, s_curve_id);
    mbedtls_x509write_csr_set_md_alg(&s_csr_ctx->req, MBEDTLS_MD_SHA256);

    ret = mbedtls_mpi_read_binary(&s_ctx->ecdsa.d, private_key, CONFIG_ECDSA_PRIKEY_LEN);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    // generate public key
    mbedtls_ecp_keypair *eck = &s_ctx->ecdsa;
    ret = mbedtls_ecp_mul(&eck->grp, &eck->Q, &eck->d, &eck->grp.G,
            mbedtls_ctr_drbg_random, &s_ctx->ctr_drbg);
    if (ret != 0) {
        vlog_error("mbedtls_ecp_mul returned %d", ret);
        goto exit;
    }
    if ((ret = mbedtls_x509write_csr_set_subject_name(&s_csr_ctx->req, subject)) != 0) {
        vlog_error("mbedtls_x509write_csr_set_subject_name returned %d", ret);
        goto exit;
    }
    s_csr_ctx->key.pk_ctx = &s_ctx->ecdsa;
    s_csr_ctx->key.pk_info = mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY);
    mbedtls_x509write_csr_set_key(&s_csr_ctx->req, &s_csr_ctx->key);

    if ((ret = __mbedtls_x509write_csr_pem(&s_csr_ctx->req, buf, (size_t *)iolen,
            mbedtls_ctr_drbg_random, &s_ctx->ctr_drbg)) != 0) {
        vlog_error("__mbedtls_x509write_csr_pem returned %d", ret);
        goto exit;
    }

    ret = 0;

exit:
    vsl_ecdsa_destroy();
    __vsl_csr_destroy();
    return (ret ? -1 : 0);
}
#endif
