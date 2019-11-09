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

#include "dtls.h"
#include "vos.h"

#include <string.h>

static inline uint32_t dtls_get_time(void)
{
    return (uint32_t)(vos_sys_time() / 10);
}

void dtls_init(void)
{
    (void)mbedtls_platform_set_calloc_free(vos_calloc, vos_free);
    (void)mbedtls_platform_set_snprintf(snprintf);
    (void)mbedtls_platform_set_printf(printf);
}

dtls_context *dtls_create(dtls_config_info *info)
{
    int ret;
    dtls_context                    *context = NULL;
    mbedtls_ssl_config              *conf = NULL;
    mbedtls_entropy_context         *entropy = NULL;
    mbedtls_ctr_drbg_context        *ctr_drbg = NULL;
    mbedtls_timing_delay_context    *timer = NULL;
#if defined(MBEDTLS_SSL_SRV_C)
    mbedtls_ssl_cookie_ctx          *cookie_ctx = NULL;
#endif
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    mbedtls_x509_crt *cacert;
#endif

    if (info == NULL) {
        return NULL;
    }
    const char *pers = info->pers;

    context     = mbedtls_calloc(1, sizeof(dtls_context));
    conf        = mbedtls_calloc(1, sizeof(mbedtls_ssl_config));
    entropy     = mbedtls_calloc(1, sizeof(mbedtls_entropy_context));
    ctr_drbg    = mbedtls_calloc(1, sizeof(mbedtls_ctr_drbg_context));
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    cacert      = mbedtls_calloc(1, sizeof(mbedtls_x509_crt));
#endif

    if (context == NULL || conf == NULL || entropy == NULL || ctr_drbg == NULL
#if defined(MBEDTLS_X509_CRT_PARSE_C)
        || cacert == NULL
#endif
    ) {
        goto EXIT_FAIL;
    }

    if (info->proto == MBEDTLS_NET_PROTO_UDP) {
        timer = mbedtls_calloc(1, sizeof(mbedtls_timing_delay_context));
        if (timer == NULL) {
            goto EXIT_FAIL;
        }
    }

    context->buf_len = CONFIG_DTLS_BUF_SIZE;
    if (CONFIG_DTLS_BUF_SIZE <= 0 || CONFIG_DTLS_BUF_SIZE > 2048) {
        context->buf_len = 512;
    }
    context->buf = (unsigned char *)mbedtls_calloc(1, context->buf_len);
    if (context->buf == NULL) {
        goto EXIT_FAIL;
    }

#if defined(MBEDTLS_SSL_SRV_C)
    if (info->endpoint == MBEDTLS_SSL_IS_SERVER) {
        cookie_ctx = mbedtls_calloc(1, sizeof(mbedtls_ssl_cookie_ctx));
        if (cookie_ctx == NULL) {
            goto EXIT_FAIL;
        }
    }
    mbedtls_ssl_cookie_init(cookie_ctx);
#endif

    mbedtls_net_init(&context->s_sock);
    mbedtls_net_init(&context->c_sock);
    mbedtls_ssl_init(&context->ssl);
    mbedtls_ssl_config_init(conf);
    mbedtls_ctr_drbg_init(ctr_drbg);
    mbedtls_entropy_init(entropy);

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    if (info->auth_type & DTLS_AUTH_CA) {
        mbedtls_x509_crt_init(cacert);
    }
#endif

    if ((ret = mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy, (const unsigned char *)pers,
                                     strlen(pers))) != 0) {
        vlog_error("mbedtls_ctr_drbg_seed failed: -0x%x", -ret);
        goto EXIT_FAIL;
    }

    vlog_info("setting up the SSL structure");

    if (info->proto == MBEDTLS_NET_PROTO_UDP) {
        ret = mbedtls_ssl_config_defaults(conf, info->endpoint, MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT);
    } else {
        ret = mbedtls_ssl_config_defaults(conf, info->endpoint, MBEDTLS_SSL_TRANSPORT_STREAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT);
    }

    if (ret != 0) {
        vlog_error("mbedtls_ssl_config_defaults failed: -0x%x", -ret);
        goto EXIT_FAIL;
    }

    mbedtls_ssl_conf_authmode(conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_rng(conf, mbedtls_ctr_drbg_random, ctr_drbg);

    if (info->proto == MBEDTLS_NET_PROTO_TCP) {
        mbedtls_ssl_conf_read_timeout(conf, CONFIG_DTLS_HANDSHAKE_TIMEOUT);
    }

#if defined(MBEDTLS_KEY_EXCHANGE__SOME__PSK_ENABLED)
    if (info->auth_type & DTLS_AUTH_PSK) {
        if ((ret = mbedtls_ssl_conf_psk(conf, info->psk.psk_key, info->psk.psk_key_len, info->psk.psk_identity,
                                        info->psk.psk_identity_len)) != 0) {
            vlog_error("mbedtls_ssl_conf_psk failed: -0x%x", -ret);
            goto EXIT_FAIL;
        }
    }
#endif

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    if (info->auth_type & DTLS_AUTH_CA) {
        ret = mbedtls_x509_crt_parse(cacert, info->ca.ca_cert, info->ca.ca_cert_len);
        if (ret < 0) {
            vlog_error("mbedtls_x509_crt_parse failed -0x%x", -ret);
            goto EXIT_FAIL;
        }
        mbedtls_ssl_conf_authmode(conf, MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_ca_chain(conf, cacert, NULL);
    }
#endif
#if defined(MBEDTLS_SSL_SRV_C)
    if (info->endpoint == MBEDTLS_SSL_IS_SERVER) {
        if ((ret = mbedtls_ssl_cookie_setup(cookie_ctx, mbedtls_ctr_drbg_random, ctr_drbg)) != 0) {
            vlog_error("mbedtls_ssl_cookie_setup faild -0x%x", -ret);
            goto EXIT_FAIL;
        }

        mbedtls_ssl_conf_dtls_cookies(conf, mbedtls_ssl_cookie_write, mbedtls_ssl_cookie_check, cookie_ctx);
    }
#endif
    // TODO: test for no check the ca_rt--only for the debug
    mbedtls_ssl_conf_authmode(conf, MBEDTLS_SSL_VERIFY_REQUIRED);

    if ((ret = mbedtls_ssl_setup(&context->ssl, conf)) != 0) {
        vlog_error("mbedtls_ssl_setup failed: -0x%x", -ret);
        goto EXIT_FAIL;
    }

    if (info->proto == MBEDTLS_NET_PROTO_UDP) {
        mbedtls_ssl_set_timer_cb(&context->ssl, timer, mbedtls_timing_set_delay, mbedtls_timing_get_delay);
    }

    vlog_error("set SSL structure succeed");

    context->auth_type = info->auth_type;
    context->proto = info->proto;

    return context;

EXIT_FAIL:
    if (conf) {
        mbedtls_ssl_config_free(conf);
        mbedtls_free(conf);
    }

    if (ctr_drbg) {
        mbedtls_ctr_drbg_free(ctr_drbg);
        mbedtls_free(ctr_drbg);
    }

    if (entropy) {
        mbedtls_entropy_free(entropy);
        mbedtls_free(entropy);
    }

    if (timer) {
        mbedtls_free(timer);
    }

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    if (cacert) {
        mbedtls_x509_crt_free(cacert);
        mbedtls_free(cacert);
    }
#endif

    if (context) {
        mbedtls_ssl_free(&context->ssl);
        if (context->buf) {
            mbedtls_free(context->buf);
        }
        mbedtls_free(context);
    }
    return NULL;
}

void dtls_destroy(dtls_context *ctx)
{
    mbedtls_ssl_config           *conf = NULL;
    mbedtls_ctr_drbg_context     *ctr_drbg = NULL;
    mbedtls_entropy_context      *entropy = NULL;
    mbedtls_timing_delay_context *timer = NULL;
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    mbedtls_x509_crt *cacert = NULL;
#endif

    if (ctx == NULL) {
        return;
    }

    conf    = (mbedtls_ssl_config *)ctx->ssl.conf;
    timer   = (mbedtls_timing_delay_context *)ctx->ssl.p_timer;
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    cacert  = (mbedtls_x509_crt *)conf->ca_chain;
#endif

    mbedtls_net_free(&ctx->c_sock);

    if (conf) {
        ctr_drbg = conf->p_rng;
        if (ctr_drbg) {
            entropy = ctr_drbg->p_entropy;
        }
    }

    if (conf) {
        mbedtls_ssl_config_free(conf);
        mbedtls_free(conf);
        ctx->ssl.conf = NULL; //  need by mbedtls_debug_print_msg(), see mbedtls_ssl_free(ssl)
    }

    if (ctr_drbg) {
        mbedtls_ctr_drbg_free(ctr_drbg);
        mbedtls_free(ctr_drbg);
    }

    if (entropy) {
        mbedtls_entropy_free(entropy);
        mbedtls_free(entropy);
    }

    if (timer) {
        mbedtls_free(timer);
    }

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    if (cacert) {
        mbedtls_x509_crt_free(cacert);
        mbedtls_free(cacert);
    }
#endif

    mbedtls_ssl_free(&ctx->ssl);
    mbedtls_free(ctx);
}

int dtls_connect(dtls_context *ctx, const char *host, const char *port)
{
    return mbedtls_net_connect(&ctx->c_sock, host, port, ctx->proto);
}

int dtls_bind(dtls_context *ctx, const char *bind_ip, const char *port)
{
    return mbedtls_net_bind(&ctx->s_sock, bind_ip, port, ctx->proto);
}

int dtls_handshake(dtls_context *ctx, dtls_handshake_info *info)
{
    int ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
    uint32_t change_value = 0;
    mbedtls_ssl_context *ssl = &ctx->ssl;
    mbedtls_net_context *sock = &ctx->c_sock;
    uint32_t max_value;
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    unsigned int flags;
#endif

    mbedtls_ssl_set_bio(ssl, sock, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    vlog_info("performing the SSL/TLS handshake");

    max_value =
        ((ssl->conf->endpoint == MBEDTLS_SSL_IS_SERVER || ctx->proto == MBEDTLS_NET_PROTO_UDP)
             ? (dtls_get_time() + info->timeout)
             : 50);

    do {
        ret = mbedtls_ssl_handshake(ssl);

        if (ssl->conf->endpoint == MBEDTLS_SSL_IS_CLIENT && ctx->proto == MBEDTLS_NET_PROTO_TCP) {
            change_value++;
        } else {
            change_value = dtls_get_time();
        }

        if (info->step) {
            info->step(info->arg);
        }
    } while ((ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE ||
              (ret == MBEDTLS_ERR_SSL_TIMEOUT && ctx->proto == MBEDTLS_NET_PROTO_TCP)) &&
             (change_value < max_value));

    if (info->finish) {
        info->finish(info->arg);
    }

    if (ret != 0) {
        vlog_error("mbedtls_ssl_handshake failed: -0x%x", -ret);
        goto EXIT_FAIL;
    }

#if defined(MBEDTLS_X509_CRT_PARSE_C)
    if (ctx->auth_type & DTLS_AUTH_CA) {
        if ((flags = mbedtls_ssl_get_verify_result(ssl)) != 0) {
            mbedtls_x509_crt_verify_info(ctx->buf, ctx->buf_len, "  ! ", flags);
            vlog_error("cert verify failed: %s", ctx->buf);
            goto EXIT_FAIL;
        } else {
            vlog_info("cert verify succeed");
        }
    }
#endif
    vlog_info("handshake succeed");

    return 0;

EXIT_FAIL:
    return ret;
}

int dtls_accept(dtls_context *ctx, void *client_ip, size_t buf_size, size_t *ip_len)
{
    return mbedtls_net_accept(&ctx->s_sock, &ctx->c_sock, client_ip, buf_size, ip_len);
}

int dtls_write(dtls_context *ctx, const char *buf, size_t len)
{
    int ret;

    do {
        ret = mbedtls_ssl_write(&ctx->ssl, (unsigned char *)buf, len);
    } while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    return ret;
}

int dtls_read(dtls_context *ctx, char *buf, size_t len, uint32_t timeout)
{
    int ret;

    mbedtls_ssl_conf_read_timeout((mbedtls_ssl_config *)ctx->ssl.conf, timeout);

    do {
        ret = mbedtls_ssl_read(&ctx->ssl, (unsigned char *)buf, len);
    } while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    return ret;
}


int dtls_close_notify(dtls_context *ctx)
{
    int ret;

    do {
        ret = mbedtls_ssl_close_notify(&ctx->ssl);
    } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    return ret;
}