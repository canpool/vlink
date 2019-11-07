/**
 * SPDX-License-Identifier: Apache License 2.0
 *
 *  Copyright (C) 2019 maminjie <canpool@163.com>
 *  All rights reserved.
 *  Contact: https://github.com/canpool
 *
 * Change Logs:
 * Date         Author          Notes
 * 2019-09-14   maminjie        Created file
 * 2019-09-20   maminjie        Add structs for sendmsg and recvmsg
 *
 */
#ifndef __LINK_SOCKET_H__
#define __LINK_SOCKET_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Supported address families. */
#define AF_UNSPEC	0
#define AF_UNIX		1	/* Unix domain sockets 		*/
#define AF_LOCAL	1	/* POSIX name for AF_UNIX	*/
#define AF_INET		2	/* Internet IP Protocol 	*/
#define AF_AX25		3	/* Amateur Radio AX.25 		*/
#define AF_IPX		4	/* Novell IPX 			*/
#define AF_APPLETALK	5	/* AppleTalk DDP 		*/
#define AF_NETROM	6	/* Amateur Radio NET/ROM 	*/
#define AF_BRIDGE	7	/* Multiprotocol bridge 	*/
#define AF_ATMPVC	8	/* ATM PVCs			*/
#define AF_X25		9	/* Reserved for X.25 project 	*/
#define AF_INET6	10	/* IP version 6			*/
#define AF_ROSE		11	/* Amateur Radio X.25 PLP	*/
#define AF_DECnet	12	/* Reserved for DECnet project	*/
#define AF_NETBEUI	13	/* Reserved for 802.2LLC project*/
#define AF_SECURITY	14	/* Security callback pseudo AF */
#define AF_KEY		15      /* PF_KEY key management API */
#define AF_NETLINK	16
#define AF_ROUTE	AF_NETLINK /* Alias to emulate 4.4BSD */
#define AF_PACKET	17	/* Packet family		*/
#define AF_ASH		18	/* Ash				*/
#define AF_ECONET	19	/* Acorn Econet			*/
#define AF_ATMSVC	20	/* ATM SVCs			*/
#define AF_RDS		21	/* RDS sockets 			*/
#define AF_SNA		22	/* Linux SNA Project (nutters!) */
#define AF_IRDA		23	/* IRDA sockets			*/
#define AF_PPPOX	24	/* PPPoX sockets		*/
#define AF_WANPIPE	25	/* Wanpipe API Sockets */
#define AF_LLC		26	/* Linux LLC			*/
#define AF_IB		27	/* Native InfiniBand address	*/
#define AF_MPLS		28	/* MPLS */
#define AF_CAN		29	/* Controller Area Network      */
#define AF_TIPC		30	/* TIPC sockets			*/
#define AF_BLUETOOTH	31	/* Bluetooth sockets 		*/
#define AF_IUCV		32	/* IUCV sockets			*/
#define AF_RXRPC	33	/* RxRPC sockets 		*/
#define AF_ISDN		34	/* mISDN sockets 		*/
#define AF_PHONET	35	/* Phonet sockets		*/
#define AF_IEEE802154	36	/* IEEE802154 sockets		*/
#define AF_CAIF		37	/* CAIF sockets			*/
#define AF_ALG		38	/* Algorithm sockets		*/
#define AF_NFC		39	/* NFC sockets			*/
#define AF_VSOCK	40	/* vSockets			*/
#define AF_KCM		41	/* Kernel Connection Multiplexor*/
#define AF_QIPCRTR	42	/* Qualcomm IPC Router          */
#define AF_SMC		43	/* smc sockets: reserve number for
				 * PF_SMC protocol family that
				 * reuses AF_INET address family
				 */

#define AF_MAX		44	/* For now.. */

/* Protocol families, same as address families. */
#define PF_UNSPEC	AF_UNSPEC
#define PF_UNIX		AF_UNIX
#define PF_LOCAL	AF_LOCAL
#define PF_INET		AF_INET
#define PF_AX25		AF_AX25
#define PF_IPX		AF_IPX
#define PF_APPLETALK	AF_APPLETALK
#define	PF_NETROM	AF_NETROM
#define PF_BRIDGE	AF_BRIDGE
#define PF_ATMPVC	AF_ATMPVC
#define PF_X25		AF_X25
#define PF_INET6	AF_INET6
#define PF_ROSE		AF_ROSE
#define PF_DECnet	AF_DECnet
#define PF_NETBEUI	AF_NETBEUI
#define PF_SECURITY	AF_SECURITY
#define PF_KEY		AF_KEY
#define PF_NETLINK	AF_NETLINK
#define PF_ROUTE	AF_ROUTE
#define PF_PACKET	AF_PACKET
#define PF_ASH		AF_ASH
#define PF_ECONET	AF_ECONET
#define PF_ATMSVC	AF_ATMSVC
#define PF_RDS		AF_RDS
#define PF_SNA		AF_SNA
#define PF_IRDA		AF_IRDA
#define PF_PPPOX	AF_PPPOX
#define PF_WANPIPE	AF_WANPIPE
#define PF_LLC		AF_LLC
#define PF_IB		AF_IB
#define PF_MPLS		AF_MPLS
#define PF_CAN		AF_CAN
#define PF_TIPC		AF_TIPC
#define PF_BLUETOOTH	AF_BLUETOOTH
#define PF_IUCV		AF_IUCV
#define PF_RXRPC	AF_RXRPC
#define PF_ISDN		AF_ISDN
#define PF_PHONET	AF_PHONET
#define PF_IEEE802154	AF_IEEE802154
#define PF_CAIF		AF_CAIF
#define PF_ALG		AF_ALG
#define PF_NFC		AF_NFC
#define PF_VSOCK	AF_VSOCK
#define PF_KCM		AF_KCM
#define PF_QIPCRTR	AF_QIPCRTR
#define PF_SMC		AF_SMC
#define PF_MAX		AF_MAX

/* Types of sockets.  */
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3
#define SOCK_RDW        4
#define SOCK_SEQPACKET  5

/* Maximum queue length specifiable by listen.  */
#define SOMAXCONN	128

/* Flags we can use with send/ and recv.
   Added those for 1003.1g not all are supported yet
 */

#define MSG_OOB		1
#define MSG_PEEK	2
#define MSG_DONTROUTE	4
#define MSG_TRYHARD     4       /* Synonym for MSG_DONTROUTE for DECnet */
#define MSG_CTRUNC	8
#define MSG_PROBE	0x10	/* Do not send. Only probe path f.e. for MTU */
#define MSG_TRUNC	0x20
#define MSG_DONTWAIT	0x40	/* Nonblocking io		 */
#define MSG_EOR         0x80	/* End of record */
#define MSG_WAITALL	0x100	/* Wait for a full request */
#define MSG_FIN         0x200
#define MSG_SYN		0x400
#define MSG_CONFIRM	0x800	/* Confirm path validity */
#define MSG_RST		0x1000
#define MSG_ERRQUEUE	0x2000	/* Fetch message from error queue */
#define MSG_NOSIGNAL	0x4000	/* Do not generate SIGPIPE */
#define MSG_MORE	0x8000	/* Sender will send more */
#define MSG_WAITFORONE	0x10000	/* recvmmsg(): block until 1+ packets avail */
#define MSG_SENDPAGE_NOTLAST 0x20000 /* sendpage() internal : not the last page */
#define MSG_BATCH	0x40000 /* sendmmsg(): more messages coming */
#define MSG_EOF         MSG_FIN

#define MSG_ZEROCOPY	0x4000000	/* Use user data in kernel path */
#define MSG_FASTOPEN	0x20000000	/* Send data in TCP SYN */
#define MSG_CMSG_CLOEXEC 0x40000000	/* Set close_on_exec for file
					   descriptor received through
					   SCM_RIGHTS */
#if defined(CONFIG_COMPAT)
#define MSG_CMSG_COMPAT	0x80000000	/* This message needs 32 bit fixups */
#else
#define MSG_CMSG_COMPAT	0		/* We never have 32 bit fixups */
#endif


/* Setsockoptions(2) level. Thanks to BSD these must match IPPROTO_xxx */
#define SOL_IP		0
/* #define SOL_ICMP	1	No-no-no! Due to Linux :-) we cannot use SOL_ICMP=1 */
#define SOL_TCP		6
#define SOL_UDP		17
#define SOL_IPV6	41
#define SOL_ICMPV6	58
#define SOL_SCTP	132
#define SOL_UDPLITE	136     /* UDP-Lite (RFC 3828) */
#define SOL_RAW		255
#define SOL_IPX		256
#define SOL_AX25	257
#define SOL_ATALK	258
#define SOL_NETROM	259
#define SOL_ROSE	260
#define SOL_DECNET	261
#define	SOL_X25		262
#define SOL_PACKET	263
#define SOL_ATM		264	/* ATM layer (cell level) */
#define SOL_AAL		265	/* ATM Adaption Layer (packet level) */
#define SOL_IRDA        266
#define SOL_NETBEUI	267
#define SOL_LLC		268
#define SOL_DCCP	269
#define SOL_NETLINK	270
#define SOL_TIPC	271
#define SOL_RXRPC	272
#define SOL_PPPOL2TP	273
#define SOL_BLUETOOTH	274
#define SOL_PNPIPE	275
#define SOL_RDS		276
#define SOL_IUCV	277
#define SOL_CAIF	278
#define SOL_ALG		279
#define SOL_NFC		280
#define SOL_KCM		281
#define SOL_TLS		282
#define SOL_SOCKET  0xffff


/* IPX options */
#define IPX_TYPE	1

/* Socket-level options for `getsockopt' and `setsockopt'.  */
#define SO_DEBUG        0x0001
#define SO_REUSEADDR    0x0004
#define SO_KEEPALIVE    0x0008
#define SO_DONTROUTE    0x0010
#define SO_BROADCAST    0x0020
#define SO_LINGER       0x0080
#define SO_OOBINLINE    0x0100
#define SO_REUSEPORT    0x0200
#define SO_SNDBUF       0x1001
#define SO_RCVBUF       0x1002
#define SO_SNDBUFFORCE  0x100a
#define SO_RCVBUFFORCE  0x100b
#define SO_SNDLOWAT 0x1003
#define SO_RCVLOWAT 0x1004
#define SO_SNDTIMEO 0x1005
#define SO_RCVTIMEO 0x1006
#define SO_ERROR    0x1007
#define SO_TYPE     0x1008
#define SO_PROTOCOL 0x1028
#define SO_DOMAIN   0x1029
#define SO_PEERNAME 0x2000

#define SO_NO_CHECK     0x400b
#define SO_PRIORITY     0x400c
#define SO_BSDCOMPAT    0x400e
#define SO_PASSCRED     0x4010
#define SO_PEERCRED     0x4011
#define SO_TIMESTAMP    0x4012
#define SO_TIMESTAMPNS  0x4013


#define IP_TOS      1
#define IP_TTL      2
#define IP_HDRINCL  3
#define IP_OPTIONS  4
#define IP_ROUTER_ALERT 5
#define IP_RECVOPTS     6
#define IP_RETOPTS      7
#define IP_PKTINFO      8
#define IP_PKTOPTIONS   9
#define IP_MTU_DISCOVER 10
#define IP_RECVERR  11
#define IP_RECVTTL  12
#define IP_RECVTOS  13
#define IP_MTU      14
#define IP_FREEBIND 15
#define IP_IPSEC_POLICY 16
#define IP_XFRM_POLICY  17
#define IP_PASSSEC      18
#define IP_TRANSPARENT  19

#define TCP_NODELAY     1   /* Turn off Nagle's algorithm. */
#define TCP_MAXSEG      2   /* Limit MSS */
#define TCP_CORK        3   /* Never send partially complete segments */
#define TCP_KEEPIDLE    4   /* Start keeplives after this period */
#define TCP_KEEPINTVL   5   /* Interval between keepalives */
#define TCP_KEEPCNT     6   /* Number of keepalives before death */
#define TCP_SYNCNT      7   /* Number of SYN retransmits */
#define TCP_LINGER2     8   /* Life time of orphaned FIN-WAIT-2 state */
#define TCP_DEFER_ACCEPT    9   /* Wake up listener only when data arrive */
#define TCP_WINDOW_CLAMP    10  /* Bound advertised window */
#define TCP_INFO        11  /* Information about this connection. */
#define TCP_QUICKACK    12  /* Block/reenable quick acks */
#define TCP_CONGESTION  13  /* Congestion control algorithm */
#define TCP_MD5SIG      14  /* TCP MD5 Signature (RFC2385) */
#define TCP_THIN_LINEAR_TIMEOUTS 16      /* Use linear timeouts for thin streams*/
#define TCP_THIN_DUPACK          17      /* Fast retrans. after 1 dupack */
#define TCP_USER_TIMEOUT    18  /* How long for loss retry before timeout */
#define TCP_REPAIR          19  /* TCP sock is under repair right now */
#define TCP_REPAIR_QUEUE    20
#define TCP_QUEUE_SEQ       21
#define TCP_REPAIR_OPTIONS  22
#define TCP_FASTOPEN        23  /* Enable FastOpen on listeners */
#define TCP_TIMESTAMP       24
#define TCP_NOTSENT_LOWAT   25  /* limit number of unsent bytes in write queue */
#define TCP_CC_INFO         26  /* Get Congestion Control (optional) info */
#define TCP_SAVE_SYN        27  /* Record SYN headers for new connections */
#define TCP_SAVED_SYN       28  /* Get SYN headers recorded for connection */
#define TCP_REPAIR_WINDOW   29  /* Get/set window parameters */
#define TCP_FASTOPEN_CONNECT    30  /* Attempt FastOpen with connect */
#define TCP_ULP                 31  /* Attach a ULP to a TCP connection */
#define TCP_MD5SIG_EXT          32  /* TCP MD5 Signature with extensions */
#define TCP_FASTOPEN_KEY        33  /* Set the key for Fast Open (cookie) */
#define TCP_FASTOPEN_NO_COOKIE  34  /* Enable TFO without a TFO cookie */
#define TCP_ZEROCOPY_RECEIVE    35
#define TCP_INQ                 36  /* Notify bytes available to read as a cmsg on read */

///< defines used for shutdown function
#ifndef SHUT_RDWR
#define SHUT_RD   0
#define SHUT_WR   1
#define SHUT_RDWR 2
#endif


/* Structure for scatter/gather I/O.  */
struct iovec
{
    void *iov_base;	/* Pointer to data.  */
    size_t iov_len;	/* Length of data.  */
};

/* Structure describing messages sent by
   `sendmsg' and received by `recvmsg'.  */
struct msghdr
{
    void *msg_name;		/* Address to send to/receive from.  */
    socklen_t msg_namelen;	/* Length of address data.  */

    struct iovec *msg_iov;	/* Vector of data to send/receive into.  */
    int msg_iovlen;		/* Number of elements in the vector.  */

    void *msg_control;		/* Ancillary data (eg BSD filedesc passing). */
    socklen_t msg_controllen;	/* Ancillary data buffer length.  */

    int msg_flags;		/* Flags in received message.  */
};

/* Structure used for storage of ancillary data object information.  */
struct cmsghdr
{
    socklen_t cmsg_len;		/* Length of data in cmsg_data plus length
				   of cmsghdr structure.  */
    int cmsg_level;		/* Originating protocol.  */
    int cmsg_type;		/* Protocol specific type.  */
};

/* Ancillary data object manipulation macros.  */
#define CMSG_DATA(cmsg) ((unsigned char *) ((struct cmsghdr *) (cmsg) + 1))

#define CMSG_NXTHDR(mhdr, cmsg) __cmsg_nxthdr (mhdr, cmsg)

#define CMSG_FIRSTHDR(mhdr) \
  ((size_t) (mhdr)->msg_controllen >= sizeof (struct cmsghdr)		      \
   ? (struct cmsghdr *) (mhdr)->msg_control : (struct cmsghdr *) 0)

#define CMSG_ALIGN(len) (((len) + sizeof (size_t) - 1) \
			   & (size_t) ~(sizeof (size_t) - 1))
#define CMSG_SPACE(len) (CMSG_ALIGN (len) \
			 + CMSG_ALIGN (sizeof (struct cmsghdr)))
#define CMSG_LEN(len)   (CMSG_ALIGN (sizeof (struct cmsghdr)) + (len))

inline struct cmsghdr *
__cmsg_nxthdr (struct msghdr *__mhdr, struct cmsghdr *__cmsg)
{
  if ((size_t) __cmsg->cmsg_len < sizeof (struct cmsghdr))
    /* The kernel header does this so there may be a reason.  */
    return (struct cmsghdr *) 0;

  __cmsg = (struct cmsghdr *) ((unsigned char *) __cmsg
			       + CMSG_ALIGN (__cmsg->cmsg_len));
  if ((unsigned char *) (__cmsg + 1) > ((unsigned char *) __mhdr->msg_control
					+ __mhdr->msg_controllen)
      || ((unsigned char *) __cmsg + CMSG_ALIGN (__cmsg->cmsg_len)
	  > ((unsigned char *) __mhdr->msg_control + __mhdr->msg_controllen)))
    /* No more entries.  */
    return (struct cmsghdr *) 0;
  return __cmsg;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#include "sal.h"

#endif /* __LINK_SOCKET_H__ */

