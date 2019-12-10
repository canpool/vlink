
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
#define mbedtls_free   free
#define mbedtls_printf printf
#define mbedtls_exit exit
#define MBEDTLS_EXIT_SUCCESS EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE EXIT_FAILURE
#endif /* MBEDTLS_PLATFORM_C */

#define mbedtls_malloc malloc
#define mbedtls_log(format, ...) \
    mbedtls_printf("[%s:%d] " format "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#include "ec256.h"

// dsa
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/sha256.h"

// csr
#include "mbedtls/x509_csr.h"
#include "mbedtls/pem.h"
#include "mbedtls/pk.h"

// md5
#include "mbedtls/md5.h"

#include <string.h>
#include <stdint.h>

#if defined(MBEDTLS_HAVE_INT32)
#define MBEDTLS_CURVE_POINT_LEN     8
#else
#define MBEDTLS_CURVE_POINT_LEN     4
#endif


#define CONFIG_CSR_DERBUF_LEN   2048
#define CONFIG_CURVE_NAME_LEN   20

typedef struct __ta_dsa_context {
    mbedtls_ecdsa_context    ecdsa;
    mbedtls_entropy_context  entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
} ta_dsa_context;

typedef struct __ta_csr_context {
    mbedtls_pk_context       key;
    mbedtls_x509write_csr    req;
} ta_csr_context;


static ta_dsa_context *s_dsa_ctx = NULL;
static ta_csr_context *s_csr_ctx = NULL;
static int s_curve_id = MBEDTLS_ECP_DP_SECP256R1;
static char s_curve_name[CONFIG_CURVE_NAME_LEN] = TA_PRIME256V1_NAME;
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

static void __export_curve_points(uint8 a[32], uint8 b[32], uint8 p[32],
                                  uint8 gx[32], uint8 gy[32], uint8 n[32])
{
    __mpi_uint_to_bytes(s_p,  MBEDTLS_CURVE_POINT_LEN, p,  32);
    __mpi_uint_to_bytes(s_a,  MBEDTLS_CURVE_POINT_LEN, a,  32);
    __mpi_uint_to_bytes(s_b,  MBEDTLS_CURVE_POINT_LEN, b,  32);
    __mpi_uint_to_bytes(s_gx, MBEDTLS_CURVE_POINT_LEN, gx, 32);
    __mpi_uint_to_bytes(s_gy, MBEDTLS_CURVE_POINT_LEN, gy, 32);
    __mpi_uint_to_bytes(s_n,  MBEDTLS_CURVE_POINT_LEN, n,  32);
}

static void __import_curve_points(uint8 a[32], uint8 b[32], uint8 p[32],
                                  uint8 gx[32], uint8 gy[32], uint8 n[32])
{
    __bytes_to_mpi_uint(p,  32, s_p,  MBEDTLS_CURVE_POINT_LEN);
    __bytes_to_mpi_uint(a,  32, s_a,  MBEDTLS_CURVE_POINT_LEN);
    __bytes_to_mpi_uint(b,  32, s_b,  MBEDTLS_CURVE_POINT_LEN);
    __bytes_to_mpi_uint(gx, 32, s_gx, MBEDTLS_CURVE_POINT_LEN);
    __bytes_to_mpi_uint(gy, 32, s_gy, MBEDTLS_CURVE_POINT_LEN);
    __bytes_to_mpi_uint(n,  32, s_n,  MBEDTLS_CURVE_POINT_LEN);
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

uint8 TA_ec256_register_curve(const uint8 *ec_name)
{
    if (ec_name == NULL) {
        return -1;
    }
    const char *name = (const char *)ec_name;
    unsigned int nlen = strlen(name);
    if (nlen >= CONFIG_CURVE_NAME_LEN) {
        mbedtls_log("name is too long");
        return -1;
    }
    if (strcmp(name, TA_PRIME256V1_NAME) == 0) {
        s_curve_id = MBEDTLS_ECP_DP_SECP256R1;
    } else if (strcmp(name, TA_BP256R1_NAME) == 0) {
        s_curve_id = MBEDTLS_ECP_DP_BP256R1;
    } else if (strcmp(name, TA_FRP256V1_NAME) == 0) {
        s_curve_id = MBEDTLS_ECP_DP_FRP256V1;
    } else {
        mbedtls_log("invalid name \"%s\"", name);
        return -1;
    }
    strcpy(s_curve_name, name);
    s_curve_name[nlen] = '\0';
    s_custom_flag = 0;

    return 0;
}

uint8 TA_ec256_register_custom_curve(uint8 *ec_name, uint8 a[32], uint8 b[32], uint8 p[32],
                         uint8 gx[32], uint8 gy[32], uint8 n[32])
{
    if (ec_name == NULL || a == NULL || b == NULL || p == NULL ||
        gx == NULL || gy == NULL || n == NULL) {
        return -1;
    }
    if (TA_ec256_register_curve(ec_name) != 0) {
        return -1;
    }
    __import_curve_points(a, b, p, gx, gy, n);
    s_custom_flag = 1;

    return 0;
}

uint8 TA_ec256_get_curve(uint8 *ec_name, uint8 a[32], uint8 b[32], uint8 p[32],
                         uint8 gx[32], uint8 gy[32], uint8 n[32])
{
    if (ec_name == NULL || a == NULL || b == NULL || p == NULL ||
        gx == NULL || gy == NULL || n == NULL) {
        return -1;
    }
    mbedtls_ecp_group grp;
    mbedtls_ecp_group_init(&grp);

    if (__mbedtls_ecp_group_load(&grp, s_curve_id) != 0) {
        return -1;
    }
    __export_ecp_group(&grp);
    __export_curve_points(a, b, p, gx, gy, n);

    strcpy((char *)ec_name, s_curve_name);

    mbedtls_ecp_group_free(&grp);

    return 0;
}

static int __ta_ecdsa_destroy(void)
{
    if (s_dsa_ctx == NULL) {
        return 0;
    }
    mbedtls_ecdsa_free(&s_dsa_ctx->ecdsa);
    mbedtls_ctr_drbg_free(&s_dsa_ctx->ctr_drbg);
    mbedtls_entropy_free(&s_dsa_ctx->entropy);

    mbedtls_free(s_dsa_ctx);
    s_dsa_ctx = NULL;

    return 0;
}

static int __ta_ecdsa_init(void)
{
    int ret;
    const char pers[] = "ecdsa";

    if (s_dsa_ctx != NULL) {
        return -1;
    }
    s_dsa_ctx = (ta_dsa_context *)mbedtls_malloc(sizeof(ta_dsa_context));
    if (s_dsa_ctx == NULL) {
        return -1;
    }

    mbedtls_ecdsa_init(&s_dsa_ctx->ecdsa);
    mbedtls_ctr_drbg_init(&s_dsa_ctx->ctr_drbg);
    mbedtls_entropy_init(&s_dsa_ctx->entropy);

    if ((ret = mbedtls_ctr_drbg_seed(&s_dsa_ctx->ctr_drbg, mbedtls_entropy_func, &s_dsa_ctx->entropy,
                                     (const unsigned char *)pers, strlen(pers))) != 0) {
        mbedtls_log("mbedtls_ctr_drbg_seed returned %d", ret);
        goto exit;
    }
    return 0;

exit:
    __ta_ecdsa_destroy();
    return -1;
}

uint8 TA_ec256_is_on_curve(uint8 pk[64])
{
    int ret = -1;

    if (pk == NULL) {
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
        mbedtls_log("mbedtls_mpi_lset returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&pt.X, pk, 64 / 2);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&pt.Y, pk + 64 / 2, 64 / 2);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    ret = mbedtls_ecp_check_pubkey(&grp, &pt);
    if (ret != 0) {
        mbedtls_log("mbedtls_ecp_check_pubkey returned %d", ret);
        goto exit;
    }

    ret = 0;

exit:
    return (ret ? -1 : 0);
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

uint8 TA_ec256_create_key_pair(uint8 sk[32], uint8 pk[64])
{
    int ret = -1;

    if (pk == NULL || sk == NULL) {
        mbedtls_log("illegal input param");
        return -1;
    }

    if (__ta_ecdsa_init() != 0) {
        mbedtls_log("__ta_ecdsa_init failed");
        goto exit;
    }

    if ((ret = __mbedtls_ecdsa_genkey(&s_dsa_ctx->ecdsa, s_curve_id, mbedtls_ctr_drbg_random,
                                    &s_dsa_ctx->ctr_drbg)) != 0) {
        mbedtls_log("__mbedtls_ecdsa_genkey returned %d", ret);
        goto exit;
    }

    ret = mbedtls_mpi_write_binary(&s_dsa_ctx->ecdsa.Q.X, pk, 64 / 2);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_write_binary returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_write_binary(&s_dsa_ctx->ecdsa.Q.Y, pk + 64 / 2, 64 / 2);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_write_binary returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_write_binary(&s_dsa_ctx->ecdsa.d, sk, 32);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_write_binary returned %d", ret);
        goto exit;
    }

    ret = 0;

exit:
    __ta_ecdsa_destroy();
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

uint8 TA_ec256_ecdsa_sign(uint8 *dgst, uint16 dgst_len, uint8 sk[32], uint8 sign[64])
{
    int ret = -1;
    unsigned char hash[32] = {0};

    if (dgst == NULL || dgst_len == 0 || sign == NULL) {
        mbedtls_log("illegal input param");
        return -1;
    }
    if (__ta_ecdsa_init() != 0) {
        mbedtls_log("__ta_ecdsa_init failed");
        goto exit;
    }
    __mbedtls_ecp_group_load(&s_dsa_ctx->ecdsa.grp, s_curve_id);

    if ((ret = mbedtls_sha256_ret(dgst, dgst_len, hash, 0)) != 0) {
        mbedtls_log("mbedtls_sha256_ret returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&s_dsa_ctx->ecdsa.d, sk, 32);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    // take care the boundary of sign
    if ((ret = __mbedtls_ecdsa_write_signature(&s_dsa_ctx->ecdsa, MBEDTLS_MD_SHA256, hash, sizeof(hash), sign,
            64, mbedtls_ctr_drbg_random, &s_dsa_ctx->ctr_drbg)) != 0) {
        mbedtls_log("mbedtls_ecdsa_write_signature returned %d", ret);
        goto exit;
    }
    ret = 0;

exit:
    __ta_ecdsa_destroy();
    return (ret ? -1 : 0);
}

uint8 TA_ec256_ecdsa_verify(uint8 *dgst, uint8 dgst_len, uint8 sign[64], uint8 pk[64])
{
    int ret = -1;
    unsigned char hash[32] = {0};

    if (dgst == NULL || dgst_len == 0 || sign == NULL || pk == NULL) {
        mbedtls_log("illegal input param");
        return -1;
    }

    if (__ta_ecdsa_init() != 0) {
        mbedtls_log("__ta_ecdsa_init failed");
        goto exit;
    }
    __mbedtls_ecp_group_load(&s_dsa_ctx->ecdsa.grp, s_curve_id);

    ret = mbedtls_mpi_lset(&s_dsa_ctx->ecdsa.Q.Z, 1);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_lset returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&s_dsa_ctx->ecdsa.Q.X, pk, 64 / 2);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&s_dsa_ctx->ecdsa.Q.Y, pk + 64 / 2, 64 / 2);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    if ((ret = mbedtls_sha256_ret(dgst, dgst_len, hash, 0)) != 0) {
        mbedtls_log("mbedtls_sha256_ret returned %d", ret);
        goto exit;
    }
    // Verify signature
    if ((ret = __mbedtls_ecdsa_read_signature(&s_dsa_ctx->ecdsa, hash, sizeof(hash), sign, 64)) != 0) {
        mbedtls_log("mbedtls_ecdsa_read_signature returned %d", ret);
        goto exit;
    }

    ret = 0;

exit:
    __ta_ecdsa_destroy();
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

uint8 TA_ec256_ecdh(uint8 sk[32], uint8 pk[64], uint8 sab[64])
{
    int ret = -1;

    if (sk == NULL || pk == NULL || sab == NULL) {
        mbedtls_log("illegal input param");
        return -1;
    }
    if (__ta_ecdsa_init() != 0) {
        mbedtls_log("__ta_ecdsa_init failed");
        goto exit;
    }
    __mbedtls_ecp_group_load(&s_dsa_ctx->ecdsa.grp, s_curve_id);

    ret = mbedtls_mpi_lset(&s_dsa_ctx->ecdsa.Q.Z, 1);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_lset returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&s_dsa_ctx->ecdsa.Q.X, pk, 64 / 2);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    ret = mbedtls_mpi_read_binary(&s_dsa_ctx->ecdsa.Q.Y, pk + 64 / 2, 64 / 2);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }

    ret = mbedtls_mpi_read_binary(&s_dsa_ctx->ecdsa.d, sk, 32);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }

    ret = __compute_shared_restartable(&s_dsa_ctx->ecdsa.grp, sab, 64,
        &s_dsa_ctx->ecdsa.Q, &s_dsa_ctx->ecdsa.d, mbedtls_ctr_drbg_random, &s_dsa_ctx->ctr_drbg);
    if (ret != 0) {
        mbedtls_log("__compute_shared_restartable returned %d", ret);
        goto exit;
    }

    ret = 0;

exit:
    __ta_ecdsa_destroy();
    return (ret ? -1 : 0);
}

static int __ta_csr_destroy(void)
{
    if (s_csr_ctx == NULL) {
        return 0;
    }
    mbedtls_x509write_csr_free(&s_csr_ctx->req);
    mbedtls_platform_zeroize(&s_csr_ctx->key, sizeof(mbedtls_pk_context));

    mbedtls_free(s_csr_ctx);
    s_csr_ctx = NULL;

    return 0;
}

static int __ta_csr_init(void)
{
    if (s_csr_ctx != NULL) {
        return -1;
    }
    s_csr_ctx = (ta_csr_context *)mbedtls_malloc(sizeof(ta_csr_context));
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

    output_buf = (unsigned char *)mbedtls_malloc(CONFIG_CSR_DERBUF_LEN);
    if (output_buf == NULL) {
        return -1;
    }
    memset(output_buf, 0, CONFIG_CSR_DERBUF_LEN);

    if ((ret = mbedtls_x509write_csr_der(ctx, output_buf, CONFIG_CSR_DERBUF_LEN,
            f_rng, p_rng)) < 0) {
        mbedtls_log("mbedtls_x509write_csr_der returned %d", ret);
        goto exit;
    }

    if ((ret = mbedtls_pem_write_buffer(PEM_BEGIN_CSR, PEM_END_CSR,
            output_buf + CONFIG_CSR_DERBUF_LEN - ret, ret, buf, *size, &olen)) != 0) {
        mbedtls_log("mbedtls_pem_write_buffer returned %d", ret);
        goto exit;
    }
    *size = olen;

exit:
    mbedtls_free(output_buf);
    return ret;
}

uint8 TA_ec256_create_csr(uint8 sk[32], const uint8 *subject, uint8 *buf, uint16 *iolen)
{
    int ret = -1;

    if (sk == NULL || subject == NULL || buf == NULL || iolen == NULL) {
        mbedtls_log("illegal input param");
        return -1;
    }
    if (__ta_ecdsa_init() != 0) {
        mbedtls_log("__ta_ecdsa_init failed");
        goto exit;
    }
    if (__ta_csr_init() != 0) {
        mbedtls_log("__ta_csr_init failed");
        goto exit;
    }
    __mbedtls_ecp_group_load(&s_dsa_ctx->ecdsa.grp, s_curve_id);
    mbedtls_x509write_csr_set_md_alg(&s_csr_ctx->req, MBEDTLS_MD_SHA256);

    ret = mbedtls_mpi_read_binary(&s_dsa_ctx->ecdsa.d, sk, 32);
    if (ret != 0) {
        mbedtls_log("mbedtls_mpi_read_binary returned %d", ret);
        goto exit;
    }
    // generate public key
    mbedtls_ecp_keypair *eck = &s_dsa_ctx->ecdsa;
    ret = mbedtls_ecp_mul(&eck->grp, &eck->Q, &eck->d, &eck->grp.G,
            mbedtls_ctr_drbg_random, &s_dsa_ctx->ctr_drbg);
    if (ret != 0) {
        mbedtls_log("mbedtls_ecp_mul returned %d", ret);
        goto exit;
    }
    if ((ret = mbedtls_x509write_csr_set_subject_name(&s_csr_ctx->req, (const char *)subject)) != 0) {
        mbedtls_log("mbedtls_x509write_csr_set_subject_name returned %d", ret);
        goto exit;
    }
    s_csr_ctx->key.pk_ctx = &s_dsa_ctx->ecdsa;
    s_csr_ctx->key.pk_info = mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY);
    mbedtls_x509write_csr_set_key(&s_csr_ctx->req, &s_csr_ctx->key);

    if ((ret = __mbedtls_x509write_csr_pem(&s_csr_ctx->req, buf, (size_t *)iolen,
            mbedtls_ctr_drbg_random, &s_dsa_ctx->ctr_drbg)) != 0) {
        mbedtls_log("__mbedtls_x509write_csr_pem returned %d", ret);
        goto exit;
    }

    ret = 0;

exit:
    __ta_ecdsa_destroy();
    __ta_csr_destroy();
    return (ret ? -1 : 0);
}

uint8 TA_sha256(const uint8 *content, uint16 content_len, uint8 md[32])
{
    if (content == NULL || content_len == 0 || md == NULL) {
        return -1;
    }
    return mbedtls_sha256_ret(content, (size_t)content_len, md, 0);
}

uint8 TA_md5(const uint8 *content, uint16 content_len, uint8 md[16])
{
    if (content == NULL || content_len == 0 || md == NULL) {
        return -1;
    }
    return mbedtls_md5_ret(content, (size_t)content_len, md);
}