#ifndef __EC_256_H__
#define __EC_256_H__


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// tmp
typedef unsigned char  uint8;
typedef unsigned short uint16;

#define TA_PRIME256V1_NAME "prime256v1" // default
#define TA_BP256R1_NAME    "brainpoolP256r1"
#define TA_FRP256V1_NAME   "FRP256V1"

uint8 TA_ec256_register_curve(const uint8 *ec_name);
uint8 TA_ec256_register_custom_curve(uint8 *ec_name, uint8 a[32], uint8 b[32], uint8 p[32],
                                     uint8 gx[32], uint8 gy[32], uint8 n[32]);
uint8 TA_ec256_get_curve(uint8 *ec_name, uint8 a[32], uint8 b[32], uint8 p[32],
                         uint8 gx[32], uint8 gy[32], uint8 n[32]);
uint8 TA_ec256_create_key_pair(uint8 sk[32], uint8 pk[64]);
uint8 TA_ec256_ecdsa_sign(uint8 *dgst, uint16 dgst_len, uint8 sk[32], uint8 sign[64]);
uint8 TA_ec256_ecdsa_verify(uint8 *dgst, uint8 dgst_len, uint8 sign[64], uint8 pk[64]);
uint8 TA_ec256_ecdh(uint8 sk[32], uint8 pk[64], uint8 sab[64]);
uint8 TA_ec256_create_csr(uint8 sk[32], const uint8 *subject, uint8 *buf, uint16 *iolen);

uint8 TA_sha256(const uint8 *content, uint16 content_len, uint8 md[32]);
uint8 TA_md5(const uint8 *content, uint16 content_len, uint8 md[16]);

#ifdef __cplusplus
}
#endif

#endif