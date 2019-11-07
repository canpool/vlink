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

#include "sal_linux.h"
#include "sal.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>

static const struct net_ops s_linux_socket_ops = {
    .socket = socket,
    .bind = bind,
    .listen = listen,
    .connect = connect,
    .accept = accept,
    .send = send,
    .sendmsg = sendmsg,
    .sendto = sendto,
    .recv = recv,
    .recvmsg = recvmsg,
    .recvfrom = recvfrom,
    .setsockopt = setsockopt,
    .getsockopt = getsockopt,
    .shutdown = shutdown,
    .ioctl = ioctl,
    .closesocket = close,
    .getpeername = getpeername,
    .getsockname = getsockname,
    .gethostbyname = gethostbyname,
    .getaddrinfo = getaddrinfo,
    .freeaddrinfo = freeaddrinfo
};

static const struct net_domain s_linux_domain = {
    .name = "linux_socket",
    .domain = AF_INET,
    .ops = &s_linux_socket_ops,
};

int sal_install_linux(void)
{
    return sal_install(&s_linux_domain);
}
