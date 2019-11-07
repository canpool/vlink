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

#ifdef FORCE_IPV4
#define BIND_IP "0.0.0.0" /* Forces IPv4 */
#else
#define BIND_IP "::"
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
    mbedtls_net_context listen_fd;
    unsigned char buf[1024];
    const char *pers = "dtls_server";
    const char *psk = "010203040506";
    const char *psk_identity = "mbed";
    unsigned char client_ip[16] = {0};
    size_t cliip_len;
    int retry_left = MAX_RETRY;

    vlink_init();
    dtls_init();

    memset(&config, 0, sizeof(dtls_config_info));
    memset(&handshake_info, 0, sizeof(dtls_handshake_info));

    config.auth_type |= DTLS_AUTH_PSK;
    config.proto = MBEDTLS_NET_PROTO_UDP;
    config.endpoint = MBEDTLS_SSL_IS_SERVER;
    config.pers = pers;
    config.psk.psk_key = (unsigned char *)psk;
    config.psk.psk_key_len = strlen(psk);
    config.psk.psk_identity = (unsigned char *)psk_identity;
    config.psk.psk_identity_len = strlen(psk_identity);

    context = dtls_create(&config);
    if (context == NULL) {
        return 0;
    }

    if ((ret = dtls_bind(context, BIND_IP, "4433")) != 0) {
        printf(" failed\n  ! mbedtls_net_bind returned %d\n\n", ret);
        goto exit;
    }

reset:
#ifdef MBEDTLS_ERROR_C
    if (ret != 0) {
        char error_buf[100];
        mbedtls_strerror(ret, error_buf, 100);
        printf("Last error was: 0x%x - %s\n\n", -ret, error_buf);
    }
#endif

    mbedtls_net_free(&context->c_sock);
    mbedtls_ssl_session_reset(&context->ssl);

    /*
     * 3. Wait until a client connects
     */
    printf("  . Waiting for a remote connection ...");
    fflush(stdout);

    if ((ret = dtls_accept(context, client_ip, sizeof(client_ip), &cliip_len)) != 0) {
        printf(" failed\n  ! mbedtls_net_accept returned %d\n\n", ret);
        goto exit;
    }
    /* For HelloVerifyRequest cookies */
    if ((ret = mbedtls_ssl_set_client_transport_id(&context->ssl, client_ip, cliip_len)) != 0) {
        printf(" failed\n  ! "
               "mbedtls_ssl_set_client_transport_id() returned -0x%x\n\n",
               -ret);
        goto exit;
    }

    printf(" ok\n");
    /*
     * 5. Handshake
     */
    printf("  . Performing the DTLS handshake...");
    fflush(stdout);

    handshake_info.timeout = CONFIG_DTLS_HANDSHAKE_TIMEOUT;
    ret = dtls_handshake(context, &handshake_info);
    if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED) {
        printf(" hello verification requested\n");
        ret = 0;
        goto reset;
    } else if (ret != 0) {
        printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
        goto reset;
    }

    printf(" ok\n\n");

    /*
     * 6. Read the echo Request
     */
    printf("  < Read from client:");
    fflush(stdout);

    len = sizeof(buf) - 1;
    memset(buf, 0, sizeof(buf));

    ret = dtls_read(context, buf, len, 0);
    if (ret <= 0) {
        switch (ret) {
        case MBEDTLS_ERR_SSL_TIMEOUT:
            printf(" timeout\n\n");
            goto reset;

        case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
            printf(" connection was closed gracefully\n");
            ret = 0;
            goto close_notify;

        default:
            printf(" mbedtls_ssl_read returned -0x%x\n\n", -ret);
            goto reset;
        }
    }

    len = ret;
    printf(" %d bytes read\n\n%s\n\n", len, buf);

    /*
     * 7. Write the 200 Response
     */
    printf("  > Write to client:");
    fflush(stdout);

    ret = dtls_write(context, buf, len);
    if (ret < 0) {
        printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
        goto exit;
    }

    len = ret;
    printf(" %d bytes written\n\n%s\n\n", len, buf);

    /*
     * 8. Done, cleanly close the connection
     */
close_notify:
    printf("  . Closing the connection...");

    /* No error checking, the connection might be closed already */
    dtls_close_notify(context);
    ret = 0;

    printf(" done\n");

    goto reset;

exit:
    dtls_destroy(context);

    /* Shell can not handle large exit numbers -> 1 for errors */
    if (ret < 0)
        ret = 1;

    return (ret);
}
