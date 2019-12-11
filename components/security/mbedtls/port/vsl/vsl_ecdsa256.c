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

#if defined(MBEDTLS_HAVE_INT32)
#define MBEDTLS_CURVE_POINT_LEN     8
#else
#define MBEDTLS_CURVE_POINT_LEN     4
#endif


typedef struct __vsl_ecdsa256_context {
    mbedtls_ecdsa_context    ecdsa;
    mbedtls_entropy_context  entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
} vsl_ecdsa256_context;

static vsl_ecdsa256_context *s_ctx = NULL;
static int s_curve_id = MBEDTLS_ECP_DP_SECP256K1;
static char s_curve_name[CONFIG_CURVE_NAME_LEN] = VSL_SECP256K1_NAME;
static char s_custom_flag = 0;


static mbedtls_mpi_uint  s_p[MBEDTLS_CURVE_POINT_LEN] = {0};
static mbedtls_mpi_uint  s_a[MBEDTLS_CURVE_POINT_LEN] = {0};
static mbedtls_mpi_uint  s_b[MBEDTLS_CURVE_POINT_LEN] = {0};
static mbedtls_mpi_uint s_gx[MBEDTLS_CURVE_POINT_LEN] = {0};
static mbedtls_mpi_uint s_gy[MBEDTLS_CURVE_POINT_LEN] = {0};
static mbedtls_mpi_uint  s_n[MBEDTLS_CURVE_POINT_LEN] = {0};

/*
 * Create an MPI from embedded constants
 * (assumes len is an exact multiple of sizeof mbedtls_mpi_uint)
 */
static inline void __ecp_mpi_load( mbedtls_mpi *X, const mbedtls_mpi_uint *p, size_t len )
{
    X->s = 1;
    X->n = len / sizeof( mbedtls_mpi_uint );
    X->p = (mbedtls_mpi_uint *) p;
}

/*
 * Set an MPI to static value 1
 */
static inline void __ecp_mpi_set1( mbedtls_mpi *X )
{
    static mbedtls_mpi_uint one[] = { 1 };
    X->s = 1;
    X->n = 1;
    X->p = one;
}

static void __bytes_to_mpi_uint(const unsigned char *b, int b_cnt,
    mbedtls_mpi_uint *m, int m_cnt)
{
    int i, j, k;
#if defined(MBEDTLS_HAVE_INT32)
    int blk = 4;
#else
    int blk = 8;
#endif

    memset(m, 0, m_cnt);

    for (i = 0; i < m_cnt; ++i) {
        for (j = b_cnt - 1 - (i * blk), k = 0; j >= (b_cnt - blk - (i * blk)); --j, ++k) {
            m[i] |= ((mbedtls_mpi_uint)b[j] << (k << 3));
        }
    }
}

static void __mpi_uint_to_bytes(const mbedtls_mpi_uint *m, int m_cnt,
    unsigned char *b, int b_cnt)
{
    int i, j, k;
#if defined(MBEDTLS_HAVE_INT32)
    int blk = 4;
#else
    int blk = 8;
#endif

    memset(b, 0, b_cnt);

    for (i = 0; i < m_cnt; ++i) {
        for (j = b_cnt - 1 - (i * blk), k = 0; j >= (b_cnt - blk - (i * blk)); --j, ++k) {
            b[j] = (unsigned char)(m[i] >> (k << 3));
        }
    }
}

static void __export_curve_points(vsl_curve_point *points)
{
    __mpi_uint_to_bytes(s_p,  MBEDTLS_CURVE_POINT_LEN, points->p,  CONFIG_CURVE_POINT_LEN);
    __mpi_uint_to_bytes(s_a,  MBEDTLS_CURVE_POINT_LEN, points->a,  CONFIG_CURVE_POINT_LEN);
    __mpi_uint_to_bytes(s_b,  MBEDTLS_CURVE_POINT_LEN, points->b,  CONFIG_CURVE_POINT_LEN);
    __mpi_uint_to_bytes(s_gx, MBEDTLS_CURVE_POINT_LEN, points->gx, CONFIG_CURVE_POINT_LEN);
    __mpi_uint_to_bytes(s_gy, MBEDTLS_CURVE_POINT_LEN, points->gy, CONFIG_CURVE_POINT_LEN);
    __mpi_uint_to_bytes(s_n,  MBEDTLS_CURVE_POINT_LEN, points->n,  CONFIG_CURVE_POINT_LEN);
}

static void __import_curve_points(vsl_curve_point *points)
{
    __bytes_to_mpi_uint(points->p,  CONFIG_CURVE_POINT_LEN, s_p,  MBEDTLS_CURVE_POINT_LEN);
    __bytes_to_mpi_uint(points->a,  CONFIG_CURVE_POINT_LEN, s_a,  MBEDTLS_CURVE_POINT_LEN);
    __bytes_to_mpi_uint(points->b,  CONFIG_CURVE_POINT_LEN, s_b,  MBEDTLS_CURVE_POINT_LEN);
    __bytes_to_mpi_uint(points->gx, CONFIG_CURVE_POINT_LEN, s_gx, MBEDTLS_CURVE_POINT_LEN);
    __bytes_to_mpi_uint(points->gy, CONFIG_CURVE_POINT_LEN, s_gy, MBEDTLS_CURVE_POINT_LEN);
    __bytes_to_mpi_uint(points->n,  CONFIG_CURVE_POINT_LEN, s_n,  MBEDTLS_CURVE_POINT_LEN);
}

#define __EXPORT_GROUP(d, s, l) \
    if (s) { \
        memcpy(d, s, l); \
    } else { \
        memset(d, 0, l); \
    }

static void __export_ecp_group(mbedtls_ecp_group *grp)
{
    __EXPORT_GROUP(s_p,  grp->P.p,   sizeof(s_p));
    __EXPORT_GROUP(s_a,  grp->A.p,   sizeof(s_a));
    __EXPORT_GROUP(s_b,  grp->B.p,   sizeof(s_b));
    __EXPORT_GROUP(s_gx, grp->G.X.p, sizeof(s_gx));
    __EXPORT_GROUP(s_gy, grp->G.Y.p, sizeof(s_gy));
    __EXPORT_GROUP(s_n,  grp->N.p,   sizeof(s_n));
}

static int __ecp_group_load(mbedtls_ecp_group *grp)
{
    __ecp_mpi_load( &grp->P, s_p, sizeof(s_p) );
    __ecp_mpi_load( &grp->A, s_a, sizeof(s_a) );
    __ecp_mpi_load( &grp->B, s_b, sizeof(s_b) );
    __ecp_mpi_load( &grp->N, s_n, sizeof(s_n) );

    __ecp_mpi_load( &grp->G.X, s_gx, sizeof(s_gx) );
    __ecp_mpi_load( &grp->G.Y, s_gy, sizeof(s_gy) );
    __ecp_mpi_set1( &grp->G.Z );

    grp->pbits = mbedtls_mpi_bitlen( &grp->P );
    grp->nbits = mbedtls_mpi_bitlen( &grp->N );

    grp->h = 1;

    return( 0 );
}

static int __mbedtls_ecp_group_load(mbedtls_ecp_group *grp, int id)
{
    if (s_custom_flag) {
        mbedtls_ecp_group_free(grp);
        grp->id = id;
        __ecp_group_load(grp);
    } else {
        return mbedtls_ecp_group_load(grp, id);
    }
    return 0;
}

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

static int __mbedtls_ecdsa_genkey( mbedtls_ecdsa_context *ctx, mbedtls_ecp_group_id gid,
                  int (*f_rng)(void *, unsigned char *, size_t), void *p_rng )
{
    int ret = 0;

    ret = __mbedtls_ecp_group_load( &ctx->grp, gid );
    if( ret != 0 )
        return( ret );

   return( mbedtls_ecp_gen_keypair( &ctx->grp, &ctx->d,
                                    &ctx->Q, f_rng, p_rng ) );
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

    if ((ret = __mbedtls_ecdsa_genkey(&s_ctx->ecdsa, s_curve_id, mbedtls_ctr_drbg_random,
                                    &s_ctx->ctr_drbg)) != 0) {
        vlog_error("__mbedtls_ecdsa_genkey returned %d", ret);
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
    __mbedtls_ecp_group_load(&s_ctx->ecdsa.grp, s_curve_id);

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
    __mbedtls_ecp_group_load(&s_ctx->ecdsa.grp, s_curve_id);

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
    __mbedtls_ecp_group_load(&s_ctx->ecdsa.grp, s_curve_id);

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
    unsigned int nlen = strlen(name);
    if (nlen >= CONFIG_CURVE_NAME_LEN) {
        vlog_error("name is too long");
        return -1;
    }

    if (strcmp(name, VSL_SECP256R1_NAME) == 0 ||
        strcmp(name, VSL_PRIME256V1_NAME) == 0) {
        s_curve_id = MBEDTLS_ECP_DP_SECP256R1;
    } else if (strcmp(name, VSL_SECP256K1_NAME) == 0) {
        s_curve_id = MBEDTLS_ECP_DP_SECP256K1;
    } else if (strcmp(name, VSL_BP256R1_NAME) == 0) {
        s_curve_id = MBEDTLS_ECP_DP_BP256R1;
    } else if (strcmp(name, VSL_FRP256V1_NAME) == 0) {
        s_curve_id = MBEDTLS_ECP_DP_FRP256V1;
    } else {
        vlog_error("invalid name \"%s\"", name);
        return -1;
    }
    strcpy(s_curve_name, name);
    s_curve_name[nlen] = '\0';
    s_custom_flag = 0;

    return 0;
}

int vsl_ecdsa_get_curve(char *name, unsigned int nlen, vsl_curve_point *points)
{
    if (name == NULL || points == NULL) {
        return -1;
    }

    unsigned int name_len = strlen(s_curve_name);
    if (name_len == 0 || name_len >= nlen) {
        vlog_error("current name len: %u, nlen: %u", name_len, nlen);
        return -1;
    }
    mbedtls_ecp_group grp;
    mbedtls_ecp_group_init(&grp);

    if (__mbedtls_ecp_group_load(&grp, s_curve_id) != 0) {
        return -1;
    }
    __export_ecp_group(&grp);
    __export_curve_points(points);
    strcpy(name, s_curve_name);

    mbedtls_ecp_group_free(&grp);

    return 0;
}

int vsl_ecdsa_def_curve(const char *name, vsl_curve_point *points)
{
    if (name == NULL || points == NULL) {
        return -1;
    }
    if (vsl_ecdsa_set_curve(name) != 0) {
        return -1;
    }
    __import_curve_points(points);
    s_custom_flag = 1;

    return 0;
}

int vsl_ecdsa_on_curve(unsigned char public_key[CONFIG_ECDSA_PUBKEY_LEN])
{
    int ret = -1;

    if (public_key == NULL) {
        return -1;
    }
    mbedtls_ecp_group grp;
    mbedtls_ecp_group_init(&grp);

    if (__mbedtls_ecp_group_load(&grp, s_curve_id) != 0) {
        return -1;
    }

    mbedtls_ecp_point pt;
    mbedtls_ecp_point_init(&pt);

    ret = mbedtls_mpi_lset(&pt.Z, 1);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_lset returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&pt.X, public_key, CONFIG_ECDSA_PUBKEY_LEN / 2);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&pt.Y, public_key + CONFIG_ECDSA_PUBKEY_LEN / 2,
        CONFIG_ECDSA_PUBKEY_LEN / 2);
    if (ret != 0) {
        vlog_error("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    ret = mbedtls_ecp_check_pubkey(&grp, &pt);
    if (ret != 0) {
        vlog_error("mbedtls_ecp_check_pubkey returned %d", ret);
        goto exit;
    }

    ret = 0;

exit:
    return (ret ? -1 : 0);
}

#ifdef CONFIG_ECDSA256_CSR_MBEDTLS

#include "mbedtls/x509_csr.h"
#include "mbedtls/pem.h"
#include "mbedtls/pk.h"
#include "mbedtls/oid.h"
#include "mbedtls/asn1write.h"
#include "mbedtls/platform_util.h"

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

static int __mbedtls_x509write_csr_der( mbedtls_x509write_csr *ctx, unsigned char *tmp_buf,
                       unsigned char *buf, size_t size,
                       int (*f_rng)(void *, unsigned char *, size_t),
                       void *p_rng )
{
    int ret;
    const char *sig_oid;
    size_t sig_oid_len = 0;
    unsigned char *c, *c2;
    unsigned char hash[64];
    unsigned char sig[MBEDTLS_MPI_MAX_SIZE];
    size_t pub_len = 0, sig_and_oid_len = 0, sig_len;
    size_t len = 0;
    mbedtls_pk_type_t pk_alg;

    /*
     * Prepare data to be signed in tmp_buf
     */
    c = tmp_buf + size;

    MBEDTLS_ASN1_CHK_ADD( len, mbedtls_x509_write_extensions( &c, tmp_buf, ctx->extensions ) );

    if( len )
    {
        MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_len( &c, tmp_buf, len ) );
        MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_tag( &c, tmp_buf, MBEDTLS_ASN1_CONSTRUCTED |
                                                        MBEDTLS_ASN1_SEQUENCE ) );

        MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_len( &c, tmp_buf, len ) );
        MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_tag( &c, tmp_buf, MBEDTLS_ASN1_CONSTRUCTED |
                                                        MBEDTLS_ASN1_SET ) );

        MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_oid( &c, tmp_buf, MBEDTLS_OID_PKCS9_CSR_EXT_REQ,
                                          MBEDTLS_OID_SIZE( MBEDTLS_OID_PKCS9_CSR_EXT_REQ ) ) );

        MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_len( &c, tmp_buf, len ) );
        MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_tag( &c, tmp_buf, MBEDTLS_ASN1_CONSTRUCTED |
                                                        MBEDTLS_ASN1_SEQUENCE ) );
    }

    MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_len( &c, tmp_buf, len ) );
    MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_tag( &c, tmp_buf, MBEDTLS_ASN1_CONSTRUCTED |
                                                    MBEDTLS_ASN1_CONTEXT_SPECIFIC ) );

    MBEDTLS_ASN1_CHK_ADD( pub_len, mbedtls_pk_write_pubkey_der( ctx->key,
                                                tmp_buf, c - tmp_buf ) );
    c -= pub_len;
    len += pub_len;

    /*
     *  Subject  ::=  Name
     */
    MBEDTLS_ASN1_CHK_ADD( len, mbedtls_x509_write_names( &c, tmp_buf, ctx->subject ) );

    /*
     *  Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
     */
    MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_int( &c, tmp_buf, 0 ) );

    MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_len( &c, tmp_buf, len ) );
    MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_tag( &c, tmp_buf, MBEDTLS_ASN1_CONSTRUCTED |
                                                    MBEDTLS_ASN1_SEQUENCE ) );

    /*
     * Prepare signature
     */
    mbedtls_md( mbedtls_md_info_from_type( ctx->md_alg ), c, len, hash );

    if( ( ret = mbedtls_pk_sign( ctx->key, ctx->md_alg, hash, 0, sig, &sig_len,
                                 f_rng, p_rng ) ) != 0 )
    {
        return( ret );
    }

    if( mbedtls_pk_can_do( ctx->key, MBEDTLS_PK_RSA ) )
        pk_alg = MBEDTLS_PK_RSA;
    else if( mbedtls_pk_can_do( ctx->key, MBEDTLS_PK_ECDSA ) )
        pk_alg = MBEDTLS_PK_ECDSA;
    else
        return( MBEDTLS_ERR_X509_INVALID_ALG );

    if( ( ret = mbedtls_oid_get_oid_by_sig_alg( pk_alg, ctx->md_alg,
                                                &sig_oid, &sig_oid_len ) ) != 0 )
    {
        return( ret );
    }

    /*
     * Write data to output buffer
     */
    c2 = buf + size;
    MBEDTLS_ASN1_CHK_ADD( sig_and_oid_len, mbedtls_x509_write_sig( &c2, buf,
                                        sig_oid, sig_oid_len, sig, sig_len ) );

    if( len > (size_t)( c2 - buf ) )
        return( MBEDTLS_ERR_ASN1_BUF_TOO_SMALL );

    c2 -= len;
    memcpy( c2, c, len );

    len += sig_and_oid_len;
    MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_len( &c2, buf, len ) );
    MBEDTLS_ASN1_CHK_ADD( len, mbedtls_asn1_write_tag( &c2, buf, MBEDTLS_ASN1_CONSTRUCTED |
                                                 MBEDTLS_ASN1_SEQUENCE ) );

    return( (int) len );
}

#define PEM_BEGIN_CSR           "-----BEGIN CERTIFICATE REQUEST-----\n"
#define PEM_END_CSR             "-----END CERTIFICATE REQUEST-----\n"

static int __mbedtls_x509write_csr_pem(mbedtls_x509write_csr *ctx, unsigned char *buf, size_t *size,
                                       int (*f_rng)(void *, unsigned char *, size_t), void *p_rng)
{
    int ret;
    unsigned char *output_buf = NULL;
    unsigned char *tmp_buf = NULL;
    size_t olen = 0;

    output_buf = (unsigned char *)vos_malloc(CONFIG_CSR_DERBUF_LEN);
    if (output_buf == NULL) {
        return -1;
    }
    tmp_buf = (unsigned char *)mbedtls_malloc(CONFIG_CSR_DERBUF_LEN);
    if (tmp_buf == NULL) {
        goto exit;
    }
    memset(output_buf, 0, CONFIG_CSR_DERBUF_LEN);
    memset(tmp_buf, 0, CONFIG_CSR_DERBUF_LEN);

    if ((ret = __mbedtls_x509write_csr_der(ctx, tmp_buf, output_buf, CONFIG_CSR_DERBUF_LEN,
            f_rng, p_rng)) < 0) {
        vlog_error("mbedtls_x509write_csr_der returned %d", ret);
        mbedtls_free(tmp_buf);
        goto exit;
    }
    mbedtls_free(tmp_buf);

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
    size_t len = 0;

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
    __mbedtls_ecp_group_load(&s_ctx->ecdsa.grp, s_curve_id);
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

    len = *iolen;
    if ((ret = __mbedtls_x509write_csr_pem(&s_csr_ctx->req, buf, &len,
            mbedtls_ctr_drbg_random, &s_ctx->ctr_drbg)) != 0) {
        vlog_error("__mbedtls_x509write_csr_pem returned %d", ret);
        goto exit;
    }
    *iolen = (unsigned int)len;
    ret = 0;

exit:
    vsl_ecdsa_destroy();
    __vsl_csr_destroy();
    return (ret ? -1 : 0);
}
#endif
