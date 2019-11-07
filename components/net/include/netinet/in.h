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

#ifndef __LINK_IN_H__
#define __LINK_IN_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef socklen_t
#define socklen_t int
#endif

//#if !defined (ssize_t) && !defined(__ssize_t_defined) && !defined(_SSIZE_T)
//#define ssize_t   int
//#endif

#ifndef sa_family_t
#define sa_family_t  uint16_t
#endif

#ifndef in_port_t
#define in_port_t  uint16_t
#endif

#ifndef in_addr_t
#define in_addr_t uint32_t
#endif

/* Standard well-defined IP protocols.  */
enum
{
    IPPROTO_IP = 0,         /* Dummy protocol for TCP.  */
#define IPPROTO_IP          IPPROTO_IP
    IPPROTO_ICMP = 1,       /* Internet Control Message Protocol.  */
#define IPPROTO_ICMP        IPPROTO_ICMP
    IPPROTO_IGMP = 2,       /* Internet Group Management Protocol. */
#define IPPROTO_IGMP        IPPROTO_IGMP
    IPPROTO_IPIP = 4,       /* IPIP tunnels (older KA9Q tunnels use 94).  */
#define IPPROTO_IPIP        IPPROTO_IPIP
    IPPROTO_TCP = 6,        /* Transmission Control Protocol.  */
#define IPPROTO_TCP         IPPROTO_TCP
    IPPROTO_EGP = 8,        /* Exterior Gateway Protocol.  */
#define IPPROTO_EGP         IPPROTO_EGP
    IPPROTO_PUP = 12,       /* PUP protocol.  */
#define IPPROTO_PUP         IPPROTO_PUP
    IPPROTO_UDP = 17,       /* User Datagram Protocol.  */
#define IPPROTO_UDP         IPPROTO_UDP
    IPPROTO_IDP = 22,       /* XNS IDP protocol.  */
#define IPPROTO_IDP         IPPROTO_IDP
    IPPROTO_TP = 29,        /* SO Transport Protocol Class 4.  */
#define IPPROTO_TP          IPPROTO_TP
    IPPROTO_DCCP = 33,      /* Datagram Congestion Control Protocol.  */
#define IPPROTO_DCCP        IPPROTO_DCCP
    IPPROTO_IPV6 = 41,      /* IPv6 header.  */
#define IPPROTO_IPV6        IPPROTO_IPV6
    IPPROTO_RSVP = 46,      /* Reservation Protocol.  */
#define IPPROTO_RSVP        IPPROTO_RSVP
    IPPROTO_GRE = 47,       /* General Routing Encapsulation.  */
#define IPPROTO_GRE         IPPROTO_GRE
    IPPROTO_ESP = 50,       /* encapsulating security payload.  */
#define IPPROTO_ESP         IPPROTO_ESP
    IPPROTO_AH = 51,        /* authentication header.  */
#define IPPROTO_AH          IPPROTO_AH
    IPPROTO_MTP = 92,       /* Multicast Transport Protocol.  */
#define IPPROTO_MTP         IPPROTO_MTP
    IPPROTO_BEETPH = 94,    /* IP option pseudo header for BEET.  */
#define IPPROTO_BEETPH      IPPROTO_BEETPH
    IPPROTO_ENCAP = 98,     /* Encapsulation Header.  */
#define IPPROTO_ENCAP       IPPROTO_ENCAP
    IPPROTO_PIM = 103,      /* Protocol Independent Multicast.  */
#define IPPROTO_PIM         IPPROTO_PIM
    IPPROTO_COMP = 108,     /* Compression Header Protocol.  */
#define IPPROTO_COMP        IPPROTO_COMP
    IPPROTO_SCTP = 132,     /* Stream Control Transmission Protocol.  */
#define IPPROTO_SCTP        IPPROTO_SCTP
    IPPROTO_UDPLITE = 136,  /* UDP-Lite protocol.  */
#define IPPROTO_UDPLITE     IPPROTO_UDPLITE
    IPPROTO_MPLS = 137,     /* MPLS in IP.  */
#define IPPROTO_MPLS        IPPROTO_MPLS
    IPPROTO_RAW = 255,      /* Raw IP packets.  */
#define IPPROTO_RAW         IPPROTO_RAW
    IPPROTO_MAX
};


/* Definitions of the bits in an Internet address integer.

   On subnets, host and network parts are found according to
   the subnet mask, not these masks.  */

#define IN_CLASSA(a)        ((((in_addr_t)(a)) & 0x80000000) == 0)
#define IN_CLASSA_NET       0xff000000
#define IN_CLASSA_NSHIFT    24
#define IN_CLASSA_HOST      (0xffffffff & ~IN_CLASSA_NET)
#define IN_CLASSA_MAX       128

#define IN_CLASSB(a)        ((((in_addr_t)(a)) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET       0xffff0000
#define IN_CLASSB_NSHIFT    16
#define IN_CLASSB_HOST      (0xffffffff & ~IN_CLASSB_NET)
#define IN_CLASSB_MAX       65536

#define IN_CLASSC(a)        ((((in_addr_t)(a)) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET       0xffffff00
#define IN_CLASSC_NSHIFT    8
#define IN_CLASSC_HOST      (0xffffffff & ~IN_CLASSC_NET)

#define IN_CLASSD(a)        ((((in_addr_t)(a)) & 0xf0000000) == 0xe0000000)
#define IN_MULTICAST(a)     IN_CLASSD(a)

#define IN_EXPERIMENTAL(a)  ((((in_addr_t)(a)) & 0xe0000000) == 0xe0000000)
#define IN_BADCLASS(a)      ((((in_addr_t)(a)) & 0xf0000000) == 0xf0000000)

/* Address to accept any incoming messages.  */
#define INADDR_ANY          ((in_addr_t) 0x00000000)
/* Address to send to all hosts.  */
#define INADDR_BROADCAST    ((in_addr_t) 0xffffffff)
/* Address indicating an error return.  */
#define INADDR_NONE         ((in_addr_t) 0xffffffff)

/* Network number for local host loopback.  */
#define IN_LOOPBACKNET      127
/* Address to loopback in software to local host.  */
#ifndef INADDR_LOOPBACK
# define INADDR_LOOPBACK    ((in_addr_t) 0x7f000001) /* Inet 127.0.0.1.  */
#endif

/* Defines for Multicast INADDR.  */
#define INADDR_UNSPEC_GROUP     ((in_addr_t) 0xe0000000) /* 224.0.0.0 */
#define INADDR_ALLHOSTS_GROUP   ((in_addr_t) 0xe0000001) /* 224.0.0.1 */
#define INADDR_ALLRTRS_GROUP    ((in_addr_t) 0xe0000002) /* 224.0.0.2 */
#define INADDR_MAX_LOCAL_GROUP  ((in_addr_t) 0xe00000ff) /* 224.0.0.255 */


struct sockaddr {
    sa_family_t     sa_family;      /* address family, AF_xxx   */
    char            sa_data[14];    /* 14 bytes of protocol address */
};

struct linger {
    int l_onoff;    /* Linger active            */
    int l_linger;   /* How long to linger for   */
};

/* Internet address.  */
struct in_addr {
    in_addr_t s_addr;                    /* IPv4 address */
};

/* Structure describing an Internet socket address.  */
struct sockaddr_in {
    sa_family_t     sin_family;         /* AF_INET */
    in_port_t       sin_port;           /* Port number.  */
    struct in_addr  sin_addr;           /* Internet address.  */
    char            sin_zero[8];        /* Pad to size of `struct sockaddr'.  */
};

/* IPv6 address */
struct in6_addr {
    union {
        uint8_t     u6_addr8[16];
        uint16_t    u6_addr16[8];
        uint32_t    u6_addr32[4];
    } in6_u;

#define s6_addr     in6_u.u6_addr8
#define s6_addr16   in6_u.u6_addr16
#define s6_addr32   in6_u.u6_addr32
};

extern const struct in6_addr in6addr_any;        /* :: */
extern const struct in6_addr in6addr_loopback;   /* ::1 */
#define IN6ADDR_ANY_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } } }
#define IN6ADDR_LOOPBACK_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 } } }

#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

/* Ditto, for IPv6.  */
struct sockaddr_in6 {
    sa_family_t     sin6_family;    /* AF_INET6 */
    in_port_t       sin6_port;      /* Transport layer port # */
    uint32_t        sin6_flowinfo;  /* IPv6 flow information */
    struct in6_addr sin6_addr;      /* IPv6 address */
    uint32_t        sin6_scope_id;  /* scope id (new in RFC2553) */
};

/* IPv6 packet information.  */
struct in6_pktinfo
{
    struct in6_addr ipi6_addr;      /* src/dst IPv6 address */
    unsigned int ipi6_ifindex;      /* send/recv interface index */
};

struct in_pktinfo {
  int ipi_ifindex;
  struct in_addr ipi_spec_dst;
  struct in_addr ipi_addr;
};

/* IPv6 MTU information.  */
struct ip6_mtuinfo
{
    struct sockaddr_in6 ip6m_addr; /* dst address including zone ID */
    uint32_t ip6m_mtu;             /* path MTU in host byte order */
};

#define IN6_IS_ADDR_V4MAPPED(a) \
    ((((const uint32_t *) (a))[0] == 0)                       \
     && (((const uint32_t *) (a))[1] == 0)                    \
     && (((const uint32_t *) (a))[2] == htonl (0xffff)))

#define IN6_IS_ADDR_MULTICAST(a) (((const uint8_t *) (a))[0] == 0xff)


// define the normal addres function
#define swaps(value) ((((value) & ((uint16_t)0xff00)) >> 8) | (((value) & ((uint16_t)0x00ff)) << 8))

#define swapl(value)  ((((value) & ((uint32_t)0xff000000)) >> 24) | (((value) & ((uint32_t)0xff0000)) >> 8) | \
                      (((value) & ((uint32_t)0xff00)) << 8) |  (((value) & ((uint32_t)0xff)) << 24))

// le - little endian
// be - big endian
#if CONFIG_ENDIAN_BIG
// host is big endian
#define htoles(value)  swaps(value)
#define htolel(value)  swapl(value)
#define htobes(value)  (value)
#define htobel(value)  (value)
#else
// host is little endian
#define htoles(value)  (value)
#define htolel(value)  (value)
#define htobes(value)  swaps(value)
#define htobel(value)  swapl(value)
#endif

#ifndef htons
#define htons      htobes      //translate the host endian to network endian (2 Bytes)
#endif

#ifndef htonl
#define htonl      htobel      //translate the host endian to network endian (4 Bytes)
#endif

#ifndef ntohs
#define ntohs      htobes      //translate the network endian to host endian (2 Bytes)
#endif

#ifndef ntohl
#define ntohl      htobel      //translate the network endian to host endian (4 Bytes)
#endif

/*
 *  IPV6 socket options
 */
//#if SAL_DEF_IPV6_OPTIONS
#define IPV6_ADDRFORM       1
#define IPV6_2292PKTINFO    2
#define IPV6_2292HOPOPTS    3
#define IPV6_2292DSTOPTS    4
#define IPV6_2292RTHDR      5
#define IPV6_2292PKTOPTIONS 6
#define IPV6_CHECKSUM       7
#define IPV6_2292HOPLIMIT   8
#define IPV6_NEXTHOP        9
#define IPV6_AUTHHDR        10  /* obsolete */
#define IPV6_FLOWINFO       11

#define IPV6_UNICAST_HOPS   16
#define IPV6_MULTICAST_IF   17
#define IPV6_MULTICAST_HOPS 18
#define IPV6_MULTICAST_LOOP 19
#define IPV6_ADD_MEMBERSHIP 20
#define IPV6_DROP_MEMBERSHIP    21
#define IPV6_ROUTER_ALERT   22
#define IPV6_MTU_DISCOVER   23
#define IPV6_MTU            24
#define IPV6_RECVERR        25
#define IPV6_V6ONLY         26
#define IPV6_JOIN_ANYCAST   27
#define IPV6_LEAVE_ANYCAST  28

/* IPV6_MTU_DISCOVER values */
#define IPV6_PMTUDISC_DONT  0
#define IPV6_PMTUDISC_WANT  1
#define IPV6_PMTUDISC_DO    2
#define IPV6_PMTUDISC_PROBE 3
/* same as IPV6_PMTUDISC_PROBE, provided for symetry with IPv4
 * also see comments on IP_PMTUDISC_INTERFACE
 */
#define IPV6_PMTUDISC_INTERFACE 4
/* weaker version of IPV6_PMTUDISC_INTERFACE, which allows packets to
 * get fragmented if they exceed the interface mtu
 */
#define IPV6_PMTUDISC_OMIT  5

/* Flowlabel */
#define IPV6_FLOWLABEL_MGR  32
#define IPV6_FLOWINFO_SEND  33

#define IPV6_IPSEC_POLICY   34
#define IPV6_XFRM_POLICY    35
#define IPV6_HDRINCL        36
//#endif

/*
 * Multicast:
 * Following socket options are shared between IPv4 and IPv6.
 *
 * MCAST_JOIN_GROUP         42
 * MCAST_BLOCK_SOURCE       43
 * MCAST_UNBLOCK_SOURCE     44
 * MCAST_LEAVE_GROUP        45
 * MCAST_JOIN_SOURCE_GROUP  46
 * MCAST_LEAVE_SOURCE_GROUP 47
 * MCAST_MSFILTER           48
 */

/*
 * Advanced API (RFC3542) (1)
 *
 * Note: IPV6_RECVRTHDRDSTOPTS does not exist. see net/ipv6/datagram.c.
 */

#define IPV6_RECVPKTINFO    49
#define IPV6_PKTINFO        50
#define IPV6_RECVHOPLIMIT   51
#define IPV6_HOPLIMIT       52
#define IPV6_RECVHOPOPTS    53
#define IPV6_HOPOPTS        54
#define IPV6_RTHDRDSTOPTS   55
#define IPV6_RECVRTHDR      56
#define IPV6_RTHDR          57
#define IPV6_RECVDSTOPTS    58
#define IPV6_DSTOPTS        59
#define IPV6_RECVPATHMTU    60
#define IPV6_PATHMTU        61
#define IPV6_DONTFRAG       62
#if 0   /* not yet */
#define IPV6_USE_MIN_MTU    63
#endif

/*
 * Netfilter (1)
 *
 * Following socket options are used in ip6_tables;
 * see include/linux/netfilter_ipv6/ip6_tables.h.
 *
 * IP6T_SO_SET_REPLACE / IP6T_SO_GET_INFO           64
 * IP6T_SO_SET_ADD_COUNTERS / IP6T_SO_GET_ENTRIES   65
 */

/*
 * Advanced API (RFC3542) (2)
 */
#define IPV6_RECVTCLASS     66
#define IPV6_TCLASS         67

/*
 * Netfilter (2)
 *
 * Following socket options are used in ip6_tables;
 * see include/linux/netfilter_ipv6/ip6_tables.h.
 *
 * IP6T_SO_GET_REVISION_MATCH   68
 * IP6T_SO_GET_REVISION_TARGET  69
 * IP6T_SO_ORIGINAL_DST         80
 */

#define IPV6_AUTOFLOWLABEL      70
/* RFC5014: Source address selection */
#define IPV6_ADDR_PREFERENCES   72

#define IPV6_PREFER_SRC_TMP     0x0001
#define IPV6_PREFER_SRC_PUBLIC  0x0002
#define IPV6_PREFER_SRC_PUBTMP_DEFAULT  0x0100
#define IPV6_PREFER_SRC_COA     0x0004
#define IPV6_PREFER_SRC_HOME    0x0400
#define IPV6_PREFER_SRC_CGA     0x0008
#define IPV6_PREFER_SRC_NONCGA  0x0800

/* RFC5082: Generalized Ttl Security Mechanism */
#define IPV6_MINHOPCOUNT        73

#define IPV6_ORIGDSTADDR        74
#define IPV6_RECVORIGDSTADDR    IPV6_ORIGDSTADDR
#define IPV6_TRANSPARENT        75
#define IPV6_UNICAST_IF         76
#define IPV6_RECVFRAGSIZE       77
#define IPV6_FREEBIND           78

/*
 * Multicast Routing:
 * see include/uapi/linux/mroute6.h.
 *
 * MRT6_BASE            200
 * ...
 * MRT6_MAX
 */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LINK_IN_H__ */
