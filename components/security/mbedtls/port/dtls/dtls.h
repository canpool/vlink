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

#ifndef __DTLS_MBEDTLS_H__
#define __DTLS_MBEDTLS_H__

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls_config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_printf     printf
#endif

#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ssl_cookie.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/timing.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CONFIG_DTLS_BUF_SIZE
#define CONFIG_DTLS_BUF_SIZE            512
#endif

#ifndef CONFIG_DTLS_HANDSHAKE_TIMEOUT
#define CONFIG_DTLS_HANDSHAKE_TIMEOUT   1000
#endif

typedef enum {
    DTLS_AUTH_PSK   = 0x01,
    DTLS_AUTH_CA    = 0x02
} dtls_auth_type;

typedef struct {
    unsigned char   *psk_identity;
    size_t           psk_identity_len;
    unsigned char   *psk_key;
    size_t           psk_key_len;
} dtls_psk_info;

typedef struct {
    unsigned char   *ca_cert;
    size_t           ca_cert_len;
} dtls_ca_info;

typedef struct {
    dtls_psk_info   psk;
    dtls_ca_info    ca;
    dtls_auth_type  auth_type;

    int             endpoint;   // MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_IS_SERVER
    int             proto;      // MBEDTLS_NET_PROTO_TCP, MBEDTLS_NET_PROTO_UDP

    const char     *pers;       // device-specific identifiers. Can be NULL
} dtls_config_info;

typedef struct {
    uint32_t    timeout;

    void (*step)  (void *arg);   // step notify
    void (*finish)(void *arg);   // finish notify
    void *arg;
} dtls_handshake_info;

typedef struct {
    mbedtls_ssl_context ssl;
    mbedtls_net_context s_sock; // server/bind socket
    mbedtls_net_context c_sock; // client socket

    dtls_auth_type      auth_type;
    int                 proto;

    char               *buf;
    size_t              buf_len;
} dtls_context;

void            dtls_init(void);
dtls_context  * dtls_create(dtls_config_info *info);
void            dtls_destroy(dtls_context *ctx);
int             dtls_handshake(dtls_context *ctx, dtls_handshake_info *info);
int             dtls_write(dtls_context *ctx, const char *buf, size_t len);
int             dtls_read(dtls_context *ctx, char *buf, size_t len, uint32_t timeout);
int             dtls_close_notify(dtls_context *ctx);
/* client */
int             dtls_connect(dtls_context *ctx, const char *host, const char *port);
/* server */
int             dtls_bind(dtls_context *ctx, const char *bind_ip, const char *port);
int             dtls_accept(dtls_context *ctx, void *client_ip, size_t buf_size, size_t *ip_len);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DTLS_MBEDTLS_H__ */