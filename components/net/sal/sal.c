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

#if CONFIG_NET

#include "sal.h"
#include "vos.h"

#include <time.h>
#include <string.h>
#include <stdlib.h>

struct net_cb {
    const struct net_ops   *ops;
    int                     idx;    /* index of cb */
    int                     fd;     /* sockfd */
};

struct domain_cb {
    const struct net_domain    *domain;
    int                         net_cb_num;
    vmutex_t                    net_cb_mutex;
    void                      **net_cb_tab;
};

/**
 * This variable is initialized by the system to contain the wildcard IPv6 address.
 */
const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;

static struct domain_cb s_domain_cb;

static struct net_cb *__sal_netcb_malloc(void)
{
    struct net_cb *cb = vos_malloc(sizeof(struct net_cb));
    if (cb == NULL) {
        return NULL;
    }

    if (vos_mutex_lock(s_domain_cb.net_cb_mutex) == -1) {
        vos_free(cb);
        return NULL;
    }

    int i;
    for (i = 0; i < s_domain_cb.net_cb_num; ++i) {
        if (s_domain_cb.net_cb_tab[i] == NULL) {
            cb->ops = s_domain_cb.domain->ops;
            cb->idx = i;
            cb->fd = -1;
            s_domain_cb.net_cb_tab[i] = cb;
            break;
        }
    }
    (void)vos_mutex_unlock(s_domain_cb.net_cb_mutex);
    if (i == s_domain_cb.net_cb_num) {
        vos_free(cb);
        return NULL;
    }
    return cb;
}

static void __sal_netcb_free(int idx)
{
    if (idx < 0 || idx >= s_domain_cb.net_cb_num) {
        return;
    }
    if (vos_mutex_lock(s_domain_cb.net_cb_mutex) == -1) {
        return;
    }

    struct net_cb *cb = (struct net_cb *)s_domain_cb.net_cb_tab[idx];
    s_domain_cb.net_cb_tab[idx] = NULL;
    (void)vos_mutex_unlock(s_domain_cb.net_cb_mutex);
    vos_free(cb);
    return;
}

static struct net_cb *__sal_netcb_get(int sockfd)
{
    struct net_cb *cb = NULL;
    int i;

    for (i = 0; i < s_domain_cb.net_cb_num; ++i) {
        cb = (struct net_cb *)s_domain_cb.net_cb_tab[i];
        if (cb != NULL && cb->fd == sockfd) {
            return cb;
        }
    }
    return NULL;
}

int sal_init(int socknum)
{
    if (socknum <= 0) {
        return -1;
    }
    if (s_domain_cb.net_cb_tab != NULL) {
        return -1;
    }

    if (vos_mutex_init(&s_domain_cb.net_cb_mutex) != 0) {
        return -1;
    }

    s_domain_cb.net_cb_tab = vos_malloc(socknum * sizeof(void *));
    if (s_domain_cb.net_cb_tab == NULL) {
        vos_mutex_destroy(&s_domain_cb.net_cb_mutex);
        return -1;
    }

    memset(s_domain_cb.net_cb_tab, 0, socknum * sizeof(void *));
    s_domain_cb.net_cb_num = socknum;
    s_domain_cb.domain = NULL;

    return 0;
}

int sal_install(const struct net_domain *domain)
{
    if (s_domain_cb.domain != NULL || domain == NULL) {
        return -1;
    }
    s_domain_cb.domain = domain;
    return 0;
}

int sal_socket(int domain, int type, int protocol)
{
    struct net_cb *cb = (struct net_cb *)__sal_netcb_malloc();
    if (cb == NULL) {
        return -1;
    }

    int fd = -1;
    if (s_domain_cb.domain != NULL &&
        s_domain_cb.domain->ops != NULL &&
        s_domain_cb.domain->ops->socket != NULL) {
        fd = s_domain_cb.domain->ops->socket(domain, type, protocol);
    }
    if (fd != -1) {
        cb->fd = fd;
    } else {
        __sal_netcb_free(cb->idx);
    }
    return fd;
}

int sal_bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen)
{
    int ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->bind != NULL) {
        ret = cb->ops->bind(cb->fd, myaddr, addrlen);
    }
    return ret;
}

int sal_listen(int sockfd, int backlog)
{
    int ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->listen != NULL) {
        ret = cb->ops->listen(cb->fd, backlog);
    }
    return ret;
}

int sal_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int sockfd_client = -1;
    struct net_cb *sockcb_client = NULL;
    struct net_cb *sockcb_server = NULL;

    sockcb_server = __sal_netcb_get(sockfd);
    if (sockcb_server == NULL) {
        return sockfd_client;
    }

    sockcb_client = __sal_netcb_malloc();
    if (sockcb_client != NULL) {
        if ((sockcb_server->ops != NULL) && (sockcb_server->ops->accept != NULL)) {
            sockfd_client = sockcb_server->ops->accept(sockcb_server->fd, addr, addrlen);
        }
        if (sockfd_client != -1) {
            sockcb_client->fd = sockfd_client;
        } else {
            __sal_netcb_free(sockcb_client->idx);
        }
    }

    return sockfd_client;
}

int sal_connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
    int ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->connect != NULL) {
        ret = cb->ops->connect(cb->fd, serv_addr, addrlen);
    }
    return ret;
}

int sal_getsockname(int sockfd, struct sockaddr *localaddr, socklen_t *addrlen)
{
    int ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->getsockname != NULL) {
        ret = cb->ops->getsockname(cb->fd, localaddr, addrlen);
    }
    return ret;
}

int sal_getpeername(int sockfd, struct sockaddr *peeraddr, socklen_t *addrlen)
{
    int ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->getpeername != NULL) {
        ret = cb->ops->getpeername(cb->fd, peeraddr, addrlen);
    }
    return ret;
}

int sal_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
    int ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->getsockopt != NULL) {
        ret = cb->ops->getsockopt(cb->fd, level, optname, optval, optlen);
    }
    return ret;
}

int sal_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
{
    int ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->setsockopt != NULL) {
        ret = cb->ops->setsockopt(cb->fd, level, optname, optval, optlen);
    }
    return ret;
}

ssize_t sal_send(int sockfd, const void *msg, size_t len, int flags)
{
    ssize_t ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->send != NULL) {
        ret = cb->ops->send(cb->fd, msg, len, flags);
    }
    return ret;
}

ssize_t sal_recv(int sockfd, void *buf, size_t len, int flags)
{
    ssize_t ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->recv != NULL) {
        ret = cb->ops->recv(cb->fd, buf, len, flags);
    }
    return ret;
}

ssize_t sal_sendto(int sockfd, const void *msg, size_t len, int flags, const struct sockaddr *dst_addr, socklen_t addrlen)
{
    ssize_t ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->sendto != NULL) {
        ret = cb->ops->sendto(cb->fd, msg, len, flags, dst_addr, addrlen);
    }
    return ret;
}

ssize_t sal_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *fromlen)
{
    ssize_t ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->recvfrom != NULL) {
        ret = cb->ops->recvfrom(cb->fd, buf, len, flags, src_addr, fromlen);
    }
    return ret;
}

ssize_t sal_sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
    ssize_t ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->sendmsg != NULL) {
        ret = cb->ops->sendmsg(cb->fd, msg, flags);
    }
    return ret;
}

ssize_t sal_recvmsg(int sockfd, struct msghdr *msg, int flags)
{
    ssize_t ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->recvmsg != NULL) {
        ret = cb->ops->recvmsg(cb->fd, msg, flags);
    }
    return ret;
}

int sal_shutdown(int sockfd, int how)
{
    int ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->shutdown != NULL) {
        ret = cb->ops->shutdown(cb->fd, how);
    }
    return ret;
}

int sal_closesocket(int sockfd)
{
    int ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->closesocket != NULL) {
        ret = cb->ops->closesocket(cb->fd);
    }
    if (ret == 0) {
        __sal_netcb_free(cb->fd);
    }
    return ret;
}

int sal_ioctl(int sockfd, unsigned long cmd, void *arg)
{
    int ret = -1;
    struct net_cb *cb = __sal_netcb_get(sockfd);
    if (cb == NULL) {
        return ret;
    }
    if (cb->ops != NULL && cb->ops->ioctl != NULL) {
        ret = cb->ops->ioctl(cb->fd, cmd, arg);
    }
    return ret;
}

struct hostent *sal_gethostbyname(const char *name)
{
    if (s_domain_cb.domain != NULL && s_domain_cb.domain->ops != NULL &&
        s_domain_cb.domain->ops->gethostbyname != NULL) {
        return (struct hostent *)s_domain_cb.domain->ops->gethostbyname(name);
    }
    return NULL;
}

int sal_getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res)
{
    if (s_domain_cb.domain != NULL && s_domain_cb.domain->ops != NULL &&
        s_domain_cb.domain->ops->getaddrinfo != NULL) {
        return s_domain_cb.domain->ops->getaddrinfo(nodename, servname, hints, res);
    }
    return -1;
}

void sal_freeaddrinfo(struct addrinfo *ai)
{
    if (s_domain_cb.domain != NULL && s_domain_cb.domain->ops != NULL &&
        s_domain_cb.domain->ops->freeaddrinfo != NULL) {
        return s_domain_cb.domain->ops->freeaddrinfo(ai);
    }
}

ssize_t sal_timedsend(int sockfd, const void *msg, size_t len, int flags, uint32_t timeout)
{
    struct timeval timedelay;

    timedelay.tv_sec = timeout / 1000;
    timedelay.tv_usec = (timeout % 1000) * 1000;

    if (sal_setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timedelay, sizeof(timedelay)) != 0) {
        return -1;
    }

    return sal_send(sockfd, msg, len, flags);
}

ssize_t sal_timedrecv(int sockfd, void *msg, size_t len, int flags, uint32_t timeout)
{
    struct timeval timedelay;

    timedelay.tv_sec = timeout / 1000;
    timedelay.tv_usec = (timeout % 1000) * 1000;

    if (sal_setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timedelay, sizeof(timedelay)) != 0) {
        return -1;
    }

    return sal_recv(sockfd, msg, len, flags);
}

/**
 * domain   : AF_INET, AF_INET6, AF_LOCAL(AF_UNIX), AF_ROUTE
 * type     : SOCK_STREAM, SOCK_DGRAM, SOCK_RAW, SOCK_PACKET, SOCK_SEQPACKET
 * protocol : IPPROTO_TCP, IPPROTO_UDP, IPPROTO_SCTP, IPPROTO_TIPC, 0
 */
int sal_sockconnect(int domain, int type, int protocol, const char *host, const char *port)
{
    int fd = -1;
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
    if (!(entry && entry->h_addr_list[0] && (entry->h_addrtype == domain))) {
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = domain;
    memcpy(&addr.sin_addr.s_addr, entry->h_addr_list[0], sizeof(addr.sin_addr.s_addr));
    addr.sin_port = htons(atoi(port));

    if ((fd = sal_socket(domain, type, protocol)) < 0) {
        return -1;
    }

    if (sal_connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        sal_closesocket(fd);
        return -1;
    }
    return fd;
}

#endif // CONFIG_NET
