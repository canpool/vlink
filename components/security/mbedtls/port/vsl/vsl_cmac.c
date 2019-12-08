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

#include "vsl_cmac.h"

#include "mbedtls/cmac.h"

int vsl_cmac(const unsigned char *key, unsigned int key_len,
             const unsigned char *input, unsigned int in_len,
             unsigned char output[16])
{
    if (key == NULL || key_len == 0 ||
        input == NULL || in_len == 0 || output == NULL) {
        return -1;
    }
    return mbedtls_aes_cmac_prf_128(key, key_len, input, in_len, output);
}