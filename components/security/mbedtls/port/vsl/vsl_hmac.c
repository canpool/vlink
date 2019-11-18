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

#ifdef CONFIG_HMAC

#include "vsl_hmac.h"

#include "mbedtls/md.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/platform.h"

#include "mbedtls/md_internal.h"

#define VSL_HMAC_TYPE(len) ((len == 64) ? MBEDTLS_MD_SHA512 : MBEDTLS_MD_SHA256)

int vsl_hmac(const unsigned char *key, size_t keylen, const unsigned char *input,
             size_t ilen, unsigned char output[CONFIG_MACH_LEN])
{
    int ret;
    mbedtls_md_context_t mbedtls_md_ctx;
    const mbedtls_md_info_t *md_info;
    mbedtls_md_type_t type = VSL_HMAC_TYPE(CONFIG_MACH_LEN);

    if (key == NULL || input == NULL || output == NULL) {
        vlog_error("illegal input");
        return MBEDTLS_ERR_MD_BAD_INPUT_DATA;
    }

    md_info = mbedtls_md_info_from_type(type);
    if (md_info == NULL || md_info->size > CONFIG_MACH_LEN) {
        vlog_error("mbedtls_md_info_from_type() failed");
        return MBEDTLS_ERR_MD_BAD_INPUT_DATA;
    }

    mbedtls_md_init(&mbedtls_md_ctx);

    ret = mbedtls_md_setup(&mbedtls_md_ctx, md_info, 1);
    if ((ret = mbedtls_md_setup(&mbedtls_md_ctx, md_info, 1)) != 0) {
        vlog_error("mbedtls_md_setup() returned -0x%04x", -ret);
        goto exit;
    }

    (void)mbedtls_md_hmac_starts(&mbedtls_md_ctx, key, keylen);
    (void)mbedtls_md_hmac_update(&mbedtls_md_ctx, input, ilen);
    (void)mbedtls_md_hmac_finish(&mbedtls_md_ctx, output);

exit:
    mbedtls_md_free(&mbedtls_md_ctx);

    return ret;
}

#endif // CONFIG_HMAC
