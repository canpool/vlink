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

#if defined(MBEDTLS_ENTROPY_HARDWARE_ALT)

#include "mbedtls/entropy.h"

#include "vmisc.h"

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen)
{
    ((void)data);
    *olen = 0;
    if (0 != v_random(output, len))
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    *olen = len;
    return 0;
}

#endif // MBEDTLS_ENTROPY_HARDWARE_ALT