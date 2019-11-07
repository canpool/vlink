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
/*
 *  TCP/IP or UDP/IP networking functions
 *
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if !defined(MBEDTLS_NET_C)

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdlib.h>
#endif

#include "mbedtls/net_sockets.h"

#include <time.h>
#include <string.h>

#include "sal.h"
#include "vos.h"

/*
 * Initialize a context
 */
void mbedtls_net_init(mbedtls_net_context *ctx)
{
    ctx->fd = -1;
}

/*
 * Initiate a TCP connection with host:port and the given protocol
 */
int mbedtls_net_connect(mbedtls_net_context *ctx, const char *host, const char *port, int proto)
{
    int ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
    struct sockaddr_in addr;

    /**
     * we try use the gethostbyname to get the ip address, the host maybe a domain name
     * eg. sal_gethostbyname("www.baidu.com")
     *          official hostname www.a.shifen.com
	 *                  alials: www.baidu.com
	 *                  address: 180.101.49.12
	 *                  address: 180.101.49.11
     *     sal_gethostbyname("180.101.49.12")
     *          official hostname 180.101.49.12
     *                  address: 180.101.49.12
     */
    struct hostent *entry = sal_gethostbyname(host);
    if (!(entry && entry->h_addr_list[0] && (entry->h_addrtype == AF_INET))) {
        return MBEDTLS_ERR_NET_UNKNOWN_HOST;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr.s_addr, entry->h_addr_list[0], sizeof(addr.sin_addr.s_addr));
    addr.sin_port = htons(atoi(port));

    if (proto == MBEDTLS_NET_PROTO_TCP) {
        ctx->fd = sal_socket(AF_INET, SOCK_STREAM, 0);
    } else { // default is udp
        ctx->fd = sal_socket(AF_INET, SOCK_DGRAM, 0);
    }

    if (ctx->fd < 0) {
        return ret;
    }

    if (sal_connect(ctx->fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        sal_closesocket(ctx->fd);
        ctx->fd = -1;
        return MBEDTLS_ERR_NET_CONNECT_FAILED;
    }
    return 0;
}

/*
 * Create a listening socket on bind_ip:port
 */
int mbedtls_net_bind(mbedtls_net_context *ctx, const char *bind_ip, const char *port, int proto)
{
    int n;
    struct sockaddr_in addr;

    struct hostent *entry = sal_gethostbyname(bind_ip);
    if (!(entry && entry->h_addr_list[0] && (entry->h_addrtype == AF_INET))) {
        return MBEDTLS_ERR_NET_UNKNOWN_HOST;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr.s_addr, entry->h_addr_list[0], sizeof(addr.sin_addr.s_addr));
    addr.sin_port = htons(atoi(port));

    if (proto == MBEDTLS_NET_PROTO_TCP) {
        ctx->fd = sal_socket(AF_INET, SOCK_STREAM, 0);
    } else { // default is udp
        ctx->fd = sal_socket(AF_INET, SOCK_DGRAM, 0);
    }

    if (ctx->fd < 0) {
        return MBEDTLS_ERR_NET_SOCKET_FAILED;
    }

    n = 1;
    if (setsockopt(ctx->fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&n, sizeof(n)) != 0) {
        sal_closesocket(ctx->fd);
        ctx->fd = -1;
        return MBEDTLS_ERR_NET_SOCKET_FAILED;
    }

    if (sal_bind(ctx->fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        sal_closesocket(ctx->fd);
        ctx->fd = -1;
        return MBEDTLS_ERR_NET_BIND_FAILED;
    }
    /* Listen only makes sense for TCP */
    if (proto == MBEDTLS_NET_PROTO_TCP) {
        if (sal_listen(ctx->fd, MBEDTLS_NET_LISTEN_BACKLOG) != 0) {
            sal_closesocket(ctx->fd);
            ctx->fd = -1;
            return MBEDTLS_ERR_NET_LISTEN_FAILED;
        }
    }
    return 0;
}

void mbedtls_net_usleep(unsigned long usec)
{
    // TODO: we only support the ms delay for the system
    vos_task_sleep(usec / 1000);
}

int mbedtls_net_recv(void *ctx, unsigned char *buf, size_t len)
{
    int fd = ((mbedtls_net_context *)ctx)->fd;
    int ret = MBEDTLS_ERR_NET_RECV_FAILED;
    int rcvlen = -1;

    rcvlen = sal_recv(fd, buf, len, 0);
    if (rcvlen == 0) {
        ret = MBEDTLS_ERR_NET_RECV_FAILED;
    } else if (rcvlen < 0) {
        ret = MBEDTLS_ERR_SSL_WANT_READ;
    } else {
        ret = rcvlen;
    }
    return ret;
}

int mbedtls_net_recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout)
{
    int fd = ((mbedtls_net_context *)ctx)->fd;
    struct timeval timedelay;

    timedelay.tv_sec = timeout / 1000;
    timedelay.tv_usec = (timeout % 1000) * 1000;

    if (sal_setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timedelay, sizeof(timedelay)) != 0) {
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }

    return mbedtls_net_recv(ctx, buf, len);
}

int mbedtls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
    int fd = ((mbedtls_net_context *)ctx)->fd;
    int ret = MBEDTLS_ERR_NET_SEND_FAILED;
    int senlen = -1;

    senlen = sal_send(fd, buf, len, 0);
    if (senlen == 0) {
        ret = MBEDTLS_ERR_NET_SEND_FAILED;
    } else if (senlen < 0) {
        ret = MBEDTLS_ERR_SSL_WANT_WRITE;
    } else {
        ret = senlen;
    }

    return ret;
}

void mbedtls_net_free(mbedtls_net_context *ctx)
{
    if (ctx->fd == -1) {
        return;
    }
    sal_shutdown(ctx->fd, 2);
    sal_closesocket(ctx->fd);

    ctx->fd = -1;
}

/*
 * Accept a connection from a remote client
 */
int mbedtls_net_accept(mbedtls_net_context *bind_ctx, mbedtls_net_context *client_ctx,
                       void *client_ip, size_t buf_size, size_t *ip_len)
{
    int ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
    struct sockaddr_in client_addr;

    int type;
    int n = (int)sizeof(client_addr);
    int type_len = (int)sizeof(type);

    /* Is this a TCP or UDP socket? */
    if (sal_getsockopt(bind_ctx->fd, SOL_SOCKET, SO_TYPE, (void *)&type, &type_len) != 0 ||
        (type != SOCK_STREAM && type != SOCK_DGRAM)) {
        return MBEDTLS_ERR_NET_ACCEPT_FAILED;
    }

    if (type == SOCK_STREAM) {
        /* TCP: actual accept() */
        ret = client_ctx->fd = (int)sal_accept(bind_ctx->fd, (struct sockaddr *)&client_addr, &n);
    } else {
        /* UDP: wait for a message, but keep it in the queue */
        char buf[1] = {0};

        ret = (int)sal_recvfrom(bind_ctx->fd, buf, sizeof(buf), MSG_PEEK, (struct sockaddr *)&client_addr, &n);
    }
    if (ret < 0) {
        return MBEDTLS_ERR_NET_ACCEPT_FAILED;
    }
    /* UDP: hijack the listening socket to communicate with the client,
     * then bind a new socket to accept new connections */
    if (type != SOCK_STREAM) {
        struct sockaddr_in local_addr;
        int one = 1;

        if (sal_connect(bind_ctx->fd, (struct sockaddr *)&client_addr, n) != 0)
            return MBEDTLS_ERR_NET_ACCEPT_FAILED;

        client_ctx->fd = bind_ctx->fd;
        bind_ctx->fd = -1; /* In case we exit early */

        n = sizeof(struct sockaddr_in);
        if (sal_getsockname(client_ctx->fd, (struct sockaddr *)&local_addr, &n) != 0 ||
            (bind_ctx->fd = (int)sal_socket(local_addr.sin_family, SOCK_DGRAM, IPPROTO_UDP)) < 0 ||
            sal_setsockopt(bind_ctx->fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(one)) != 0) {
            return MBEDTLS_ERR_NET_SOCKET_FAILED;
        }

        if (sal_bind(bind_ctx->fd, (struct sockaddr *)&local_addr, n) != 0) {
            return MBEDTLS_ERR_NET_BIND_FAILED;
        }
    }

    if (client_ip != NULL) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)&client_addr;
        *ip_len = sizeof(addr4->sin_addr.s_addr);

        if (buf_size < *ip_len) {
            return MBEDTLS_ERR_NET_BUFFER_TOO_SMALL;
        }

        memcpy(client_ip, &client_addr.sin_addr, *ip_len);
    }

    return 0;
}

#endif /* !MBEDTLS_NET_C */