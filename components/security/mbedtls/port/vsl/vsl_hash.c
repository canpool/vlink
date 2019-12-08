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

#include "mbedtls/md5.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"

int vsl_sha256(const unsigned char *input, unsigned int ilen, unsigned char output[32])
{
    if (input == NULL || ilen == 0 || output == NULL) {
        return -1;
    }
    return mbedtls_sha256_ret(input, (size_t)ilen, output, 0);
}

int vsl_sha512(const unsigned char *input, unsigned int ilen, unsigned char output[64])
{
    if (input == NULL || ilen == 0 || output == NULL) {
        return -1;
    }
    return mbedtls_sha512_ret(input, (size_t)ilen, output, 0);
}

int vsl_md5(const unsigned char *input, unsigned int ilen, unsigned char output[16])
{
    if (input == NULL || ilen == 0 || output == NULL) {
        return -1;
    }
    return mbedtls_md5_ret(input, (size_t)ilen, output);
}