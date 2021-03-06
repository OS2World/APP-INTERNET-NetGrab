#ifndef __SOCKET_32H
#define __SOCKET_32H

/*
 * Copyright (c) 1982, 1985, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 *      @(#)socket.h    7.2 (Berkeley) 12/30/87
 */

/*
 * Definitions related to sockets: types, address families, options.
 */

/*
 * Types
 */
#define SOCK_STREAM     1               /* stream socket */
#define SOCK_DGRAM      2               /* datagram socket */
#define SOCK_RAW        3               /* raw-protocol interface */
#define SOCK_RDM        4               /* reliably-delivered message */
#define SOCK_SEQPACKET  5               /* sequenced packet stream */

/*
 * Option flags per-socket.
 */
#define SO_DEBUG        0x0001          /* turn on debugging info recording */
#define SO_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SO_REUSEADDR    0x0004          /* allow local address reuse */
#define SO_KEEPALIVE    0x0008          /* keep connections alive */
#define SO_DONTROUTE    0x0010          /* just use interface addresses */
#define SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SO_USELOOPBACK  0x0040          /* bypass hardware when possible */
#define SO_LINGER       0x0080          /* linger on close if data present */
#define SO_OOBINLINE    0x0100          /* leave received OOB data in line */

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
#define SO_ERROR        0x1007          /* get error status and clear */
#define SO_TYPE         0x1008          /* get socket type */

/*
 * Structure used for manipulating linger option.
 */
struct  linger {
        int     l_onoff;                /* option on/off */
        int     l_linger;               /* linger time */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define SOL_SOCKET      0xffff          /* options for socket level */

/*
 * Address families.
 */
#define AF_UNSPEC       0               /* unspecified */
#define AF_UNIX         1               /* local to host (pipes, portals) */
#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define AF_IMPLINK      3               /* arpanet imp addresses */
#define AF_PUP          4               /* pup protocols: e.g. BSP */
#define AF_CHAOS        5               /* mit CHAOS protocols */
#define AF_NS           6               /* XEROX NS protocols */
#define AF_NBS          7               /* nbs protocols */
#define AF_ECMA         8               /* european computer manufacturers */
#define AF_DATAKIT      9               /* datakit protocols */
#define AF_CCITT        10              /* CCITT protocols, X.25 etc */
#define AF_SNA          11              /* IBM SNA */
#define AF_DECnet       12              /* DECnet */
#define AF_DLI          13              /* Direct data link interface */
#define AF_LAT          14              /* LAT */
#define AF_HYLINK       15              /* NSC Hyperchannel */
#define AF_APPLETALK    16              /* Apple Talk */

#define AF_OS2          AF_UNIX

#define AF_NB           17              /* Netbios */
#define AF_NETBIOS      AF_NB

#define AF_MAX          18


/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sockaddr {
        unsigned short sa_family;              /* address family */
        char    sa_data[14];            /* up to 14 bytes of direct address */
};

/*
 * Structure used by kernel to pass protocol
 * information in raw sockets.
 */
struct sockproto {
        unsigned short sp_family;              /* address family */
        unsigned short sp_protocol;            /* protocol */
};

/*
 * Protocol families, same as address families for now.
 */
#define PF_UNSPEC       AF_UNSPEC
#define PF_UNIX         AF_UNIX
#define PF_INET         AF_INET
#define PF_IMPLINK      AF_IMPLINK
#define PF_PUP          AF_PUP
#define PF_CHAOS        AF_CHAOS
#define PF_NS           AF_NS
#define PF_NBS          AF_NBS
#define PF_ECMA         AF_ECMA
#define PF_DATAKIT      AF_DATAKIT
#define PF_CCITT        AF_CCITT
#define PF_SNA          AF_SNA
#define PF_DECnet       AF_DECnet
#define PF_DLI          AF_DLI
#define PF_LAT          AF_LAT
#define PF_HYLINK       AF_HYLINK
#define PF_APPLETALK    AF_APPLETALK
#define PF_NETBIOS      AF_NB
#define PF_NB           AF_NB
#define PF_OS2          PF_UNIX
#define PF_MAX          AF_MAX

/*
 * Maximum queue length specifiable by listen.
 */
#define SOMAXCONN       5

struct iovec {
        char *  iov_base;
        int     iov_len;
};

/*
 * Message header for recvmsg and sendmsg calls.
 */
struct msghdr {
        char * msg_name;               /* optional address */
        int msg_namelen;            /* size of address */
        struct  iovec *  msg_iov;         /* scatter/gather array */
        int   msg_iovlen;             /* # elements in msg_iov */
        char *  msg_accrights;          /* access rights sent/received */
        int   msg_accrightslen;
};

struct uio {
        struct  iovec *uio_iov;
        int     uio_iovcnt;
        off_t   uio_offset;
        int     uio_segflg;
        int     uio_resid;
};
enum    uio_rw { UIO_READ, UIO_WRITE };
#define FREAD  1
#define FWRITE 2

#define MSG_OOB         0x1             /* process out-of-band data */
#define MSG_PEEK        0x2             /* peek at incoming message */
#define MSG_DONTROUTE   0x4             /* send without using routing tables */
#define MSG_FULLREAD    0x8             /* send without using routing tables */

#define MSG_MAXIOVLEN   16

int _System accept( int, struct sockaddr *, int * );
int _System bind( int, struct sockaddr *, int );
int _System connect( int, struct sockaddr *, int );
int _System gethostid(void);
int _System getpeername( int, struct sockaddr *, int * );
int _System getsockname( int, struct sockaddr *, int * );
int _System getsockopt( int, int, int, char *, int * );
int _System ioctl(int, int, char *, int);
int _System listen( int, int);
int _System recvmsg( int, struct msghdr * , int);
int _System recv( int, char *, int, int );
int _System recvfrom(int, char *, int, int, struct sockaddr *, int * );
#ifndef BSD_SELECT
int _System select( int *, int, int, int, long );
#endif
int _System send( int, char *, int, int );
int _System sendmsg( int, struct msghdr * , int);
int _System sendto( int, char *, int, int, struct sockaddr *, int);
int _System setsockopt( int, int, int, char *, int );
int _System sock_init( void );
int _System sock_errno( void );
void _System psock_errno( char * );
int _System socket( int, int, int );
int _System soclose( int );
int _System soabort(int);
int _System so_cancel(int);
int _System readv(int, struct iovec * , int);
int _System writev(int, struct iovec* , int);
int _System shutdown(int, int);
int _System getinetversion(char *);

#endif /* __SOCKET_32H */
