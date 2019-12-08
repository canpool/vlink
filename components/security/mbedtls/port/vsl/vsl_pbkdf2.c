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

#include "vsl_pbkdf2.h"

#include "mbedtls/pkcs5.h"
#include "mbedtls/md.h"

int vsl_pbkdf2(const unsigned char *pwd, unsigned int plen,
               const unsigned char *salt, unsigned int slen,
               unsigned int iter_cnt,
               unsigned int key_len, unsigned char *output)
{
    int ret;
    mbedtls_md_context_t mbedtls_md_ctx;
    const mbedtls_md_info_t *md_info;

    if (pwd == NULL || salt == NULL || output == NULL) {
        vlog_error("illegal input");
        return MBEDTLS_ERR_MD_BAD_INPUT_DATA;
    }

    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (md_info == NULL) {
        vlog_error("mbedtls_md_info_from_type() failed");
        return MBEDTLS_ERR_MD_BAD_INPUT_DATA;
    }

    mbedtls_md_init(&mbedtls_md_ctx);

    if ((ret = mbedtls_md_setup(&mbedtls_md_ctx, md_info, 1)) != 0) {
        vlog_error("mbedtls_md_setup() returned -0x%04x", -ret);
        goto exit;
    }

    if ((ret = mbedtls_pkcs5_pbkdf2_hmac(&mbedtls_md_ctx,
        pwd, plen, salt, slen, iter_cnt, key_len, output)) != 0) {
        vlog_error("mbedtls_pkcs5_pbkdf2_hmac() returned -0x%04x", -ret);
        goto exit;
    }

exit:
    mbedtls_md_free(&mbedtls_md_ctx);

    return ret;
}