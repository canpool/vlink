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
#include "vlink.h"

#include <stdio.h>
#include <string.h>

/* Uncomment out the following line to default to IPv4 and disable IPv6 */
#define FORCE_IPV4

#define SERVER_PORT "4433"
#define SERVER_NAME "localhost"

#ifdef FORCE_IPV4
#define SERVER_ADDR "127.0.0.1" /* Forces IPv4 */
#else
#define SERVER_ADDR "::1"
#endif

#define MESSAGE "Echo this"

#define READ_TIMEOUT_MS 1000
#define MAX_RETRY 5

int main(int argc, char *argv)
{
    int ret, len;
    dtls_config_info config;
    dtls_handshake_info handshake_info;
    dtls_context *context = NULL;
    uint32_t flags;
    unsigned char buf[1024];
    const char *pers = "dtls_client";
    const char *psk = "010203040506";
    const char *psk_identity = "mbed";
    int retry_left = MAX_RETRY;

    vlink_init();
    dtls_init();

    memset(&config, 0, sizeof(dtls_config_info));
    memset(&handshake_info, 0, sizeof(dtls_handshake_info));

    config.auth_type |= DTLS_AUTH_PSK;
    config.proto = MBEDTLS_NET_PROTO_UDP;
    config.endpoint = MBEDTLS_SSL_IS_CLIENT;
    config.pers = pers;
    config.psk.psk_key = (unsigned char *)psk;
    config.psk.psk_key_len = strlen(psk);
    config.psk.psk_identity = (unsigned char *)psk_identity;
    config.psk.psk_identity_len = strlen(psk_identity);

    context = dtls_create(&config);
    if (context == NULL) {
        return 0;
    }

    if ((ret = dtls_connect(context, SERVER_ADDR, SERVER_PORT)) != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_net_connect returned %d\n\n", ret);
        goto exit;
    }

    handshake_info.timeout = CONFIG_DTLS_HANDSHAKE_TIMEOUT;
    ret = dtls_handshake(context, &handshake_info);
    if (ret != 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
        goto exit;
    }
    /*
     * 6. Write the echo request
     */
send_request:
    mbedtls_printf("  > Write to server:");
    fflush(stdout);

    len = sizeof(MESSAGE) - 1;

    ret = dtls_write(context, MESSAGE, len);
    if (ret < 0) {
        mbedtls_printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
        goto exit;
    }

    len = ret;
    mbedtls_printf(" %d bytes written\n\n%s\n\n", len, MESSAGE);

    /*
     * 7. Read the echo response
     */
    mbedtls_printf("  < Read from server:");
    fflush(stdout);

    len = sizeof(buf) - 1;
    memset(buf, 0, sizeof(buf));

    ret = dtls_read(context, buf, len, 0);
    if (ret <= 0) {
        switch (ret) {
        case MBEDTLS_ERR_SSL_TIMEOUT:
            mbedtls_printf(" timeout\n\n");
            if (retry_left-- > 0)
                goto send_request;
            goto exit;

        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
            mbedtls_printf(" connection was closed gracefully\n");
            ret = 0;
            goto close_notify;

        default:
            mbedtls_printf(" mbedtls_ssl_read returned -0x%x\n\n", -ret);
            goto exit;
        }
    }

    len = ret;
    mbedtls_printf(" %d bytes read\n\n%s\n\n", len, buf);

    /*
     * 8. Done, cleanly close the connection
     */
close_notify:
    mbedtls_printf("  . Closing the connection...");

    /* No error checking, the connection might be closed already */
    dtls_close_notify(context);
    ret = 0;

    mbedtls_printf(" done\n");

exit:
#ifdef MBEDTLS_ERROR_C
    if (ret != 0) {
        char error_buf[100];
        mbedtls_strerror(ret, error_buf, 100);
        mbedtls_printf("Last error was: -0x%x - %s\n\n", -ret, error_buf);
    }
#endif

    dtls_destroy(context);

    /* Shell can not handle large exit numbers -> 1 for errors */
    if (ret < 0)
        ret = 1;

    return (ret);
}
