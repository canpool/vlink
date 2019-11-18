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

#ifndef __LINK_SAL_H__
#define __LINK_SAL_H__

#include <stdint.h>
#include <stddef.h>

#include "netinet/in.h"
#include "sys/socket.h"
#include "netdb.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int     sal_init(int socknum);

int     sal_socket(int domain, int type, int protocol);
int     sal_bind(int sockfd, const struct sockaddr *myaddr, socklen_t addrlen);
int     sal_listen(int sockfd, int backlog);
int     sal_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int     sal_connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen);
int     sal_getsockname(int sockfd, struct sockaddr *localaddr, socklen_t *addrlen);
int     sal_getpeername(int sockfd, struct sockaddr *peeraddr, socklen_t *addrlen);
int     sal_getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
int     sal_setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
ssize_t sal_send(int sockfd, const void *msg, size_t len, int flags);
ssize_t sal_recv(int sockfd, void *buf, size_t len, int flags);
ssize_t sal_sendto(int sockfd, const void *msg, size_t len, int flags, const struct sockaddr *dst_addr, socklen_t addrlen);
ssize_t sal_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *fromlen);
ssize_t sal_sendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t sal_recvmsg(int sockfd, struct msghdr *msg, int flags);
int     sal_shutdown(int sockfd, int how);
int     sal_closesocket(int sockfd);
int     sal_ioctl(int sockfd, unsigned long cmd, void *args);
struct hostent *sal_gethostbyname(const char *name);
int     sal_getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
void    sal_freeaddrinfo(struct addrinfo *ai);

ssize_t sal_timedsend(int sockfd, const void *msg, size_t len, int flags, uint32_t timeout);
ssize_t sal_timedrecv(int sockfd, void *msg, size_t len, int flags, uint32_t timeout);
int     sal_sockconnect(int domain, int type, int protocol, const char *host, const char *port);

/* network interface opreations */
struct net_ops {
    /* network socket */
    int     (*socket)     (int domain, int type, int protocol);
    int     (*bind)       (int s, const struct sockaddr *myaddr, socklen_t addrlen);
    int     (*listen)     (int s, int backlog);
    int     (*accept)     (int s, struct sockaddr *addr, socklen_t *addrlen);
    int     (*connect)    (int s, const struct sockaddr *serv_addr, socklen_t addrlen);
    int     (*getsockname)(int s, struct sockaddr *localaddr, socklen_t *addrlen);
    int     (*getpeername)(int s, struct sockaddr *peeraddr, socklen_t *addrlen);
    int     (*setsockopt) (int s, int level, int optname, const void *optval, socklen_t optlen);
    int     (*getsockopt) (int s, int level, int optname, void *optval, socklen_t *optlen);
    ssize_t (*send)       (int s, const void *msg, size_t len, int flags);
    ssize_t (*sendmsg)    (int s, const struct msghdr *msg, int flags);
    ssize_t (*sendto)     (int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
    ssize_t (*recv)       (int s, void *buf, size_t len, int flags);
    ssize_t (*recvmsg)    (int s, struct msghdr *msg, int flags);
    ssize_t (*recvfrom)   (int s, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
    int     (*shutdown)   (int s, int how);
    int     (*ioctl)      (int s, unsigned long cmd, ...);
    int     (*closesocket)(int s);

    /* network database name resolving */
    struct hostent *(*gethostbyname)  (const char *name);
    int             (*getaddrinfo)    (const char *nodename, const char *servname,
                                       const struct addrinfo *hints, struct addrinfo **res);
    void            (*freeaddrinfo)   (struct addrinfo *ai);
};

struct net_domain {
    const char              *name;
    int                      domain;    ///< AF_INET, AF_AT, AF_WIZ and so on
    const struct net_ops    *ops;
};

int sal_install(const struct net_domain *domain);


#if SAL_COMPAT_SOCKETS
#define accept(s,addr,addrlen)                    sal_accept(s,addr,addrlen)
#define bind(s,addr,addrlen)                      sal_bind(s,addr,addrlen)
#define shutdown(s,how)                           sal_shutdown(s,how)
#define getpeername(s,addr,addrlen)               sal_getpeername(s,addr,addrlen)
#define getsockname(s,addr,addrlen)               sal_getsockname(s,addr,addrlen)
#define setsockopt(s,level,optname,opval,optlen)  sal_getsockopt(s,level,optname,opval,optlen)
#define getsockopt(s,level,optname,opval,optlen)  sal_getsockopt(s,level,optname,opval,optlen)
#define closesocket(s)                            sal_closesocket(s)
#define connect(s,addr,addrlen)                   sal_connect(s,addr,addrlen)
#define listen(s,backlog)                         sal_listen(s,backlog)
#define recv(s,mem,len,flags)                     sal_recv(s,mem,len,flags)
#define recvmsg(s,message,flags)                  sal_recvmsg(s,message,flags)
#define recvfrom(s,mem,len,flags,from,fromlen)    sal_recvfrom(s,mem,len,flags,from,fromlen)
#define send(s,dataptr,size,flags)                sal_send(s,dataptr,size,flags)
#define sendmsg(s,message,flags)                  sal_sendmsg(s,message,flags)
#define sendto(s,dataptr,size,flags,to,tolen)     sal_sendto(s,dataptr,size,flags,to,tolen)
#define socket(domain,type,protocol)              sal_socket(domain,type,protocol)
#define ioctlsocket(s,cmd,arg)                    sal_ioctl(s,cmd,arg)

#define gethostbyname(name)                       sal_gethostbyname(name)
#define freeaddrinfo(addrinfo)                    sal_freeaddrinfo(addrinfo)
#define getaddrinfo(nodname, servname, hints, res) sal_getaddrinfo(nodname, servname, hints, res)
#endif /* SAL_COMPAT_SOCKETS */

// <arpa/inet.h>
in_addr_t inet_addr(const char *cp);
char *inet_ntoa(struct in_addr in);
int inet_aton(const char *strip, struct in_addr *addr);
int inet_pton(int af, const char *src, void *dst);
const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LINK_SAL_H__ */
