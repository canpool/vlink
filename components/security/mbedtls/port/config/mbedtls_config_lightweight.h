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

#ifndef __MBEDTLS_CONFIG_LIGHT_WEIGHT_H__
#define __MBEDTLS_CONFIG_LIGHT_WEIGHT_H__

#ifndef MBEDTLS_ENTROPY_MAX_SOURCES
#define MBEDTLS_ENTROPY_MAX_SOURCES         2
#endif

#ifndef MBEDTLS_CTR_DRBG_MAX_SEED_INPUT
#define MBEDTLS_CTR_DRBG_MAX_SEED_INPUT     64  /* strlen(peers) < 32 */
#endif

#endif /* __MBEDTLS_CONFIG_LIGHT_WEIGHT_H__ */