//#include <sys/socket.h>
#ifndef SYS_SOCKET_H
#define SYS_SOCKET_H
#include <sys/types.h> // size_t and ssize_t types as described in <sys/types.h>.

enum {
//SOCK_RAW,		//!< [RS] Raw Protocol Interface.
SOCK_DGRAM,		//!< Datagram socket.
SOCK_STREAM,	//!< Byte-stream socket
SOCK_SEQPACKET,	//!< Sequenced-packet socket.
};
enum {// protocol|address family 
AF_UNSPEC=0,//!< Unspecified. 
AF_INET,    //!< Internet domain sockets for use with IPv4 addresses.
AF_INET6,   //!< [IP6] [Option Start] Internet domain sockets for use with IPv6 addresses. [Option End]
AF_UNIX,    //!< UNIX domain sockets.
};
//  flags parameter in recv(), recvfrom(), recvmsg(), send(), sendmsg(), or sendto() calls:
enum {
MSG_CTRUNC=1, 	//!< Control data truncated.
MSG_DONTROUTE=2,	//!< Send without using routing tables.
MSG_EOR=4,		//!< Terminates a record (if supported by the protocol).
MSG_OOB=8,		//!< Out-of-band data.
MSG_NOSIGNAL=16,	//!< No SIGPIPE generated when an attempt to send is made on a stream-oriented socket that is no longer connected.
MSG_PEEK=32,		//!< Leave received data in queue.
MSG_TRUNC=64,		//!< Normal data truncated.
MSG_WAITALL=128,	//!< Attempt to fill the read buffer. 	
MSG_ZEROCOPY=256,	//!<
};
#define SOL_SOCKET 0 //!<  Options to be accessed at socket level, not protocol level,  of setsockopt() and getsockopt().
enum {
SO_ACCEPTCONN,	    //!< Socket is accepting connections.
SO_BROADCAST,	    //!< Transmission of broadcast messages is supported.
SO_DEBUG,		    //!< Debugging information is being recorded.
SO_DONTROUTE,	    //!< Bypass normal routing.
SO_ERROR,		   	//!< Socket error status.
SO_KEEPALIVE,	    //!< Connections are kept alive with periodic messages.
SO_LINGER,		    //!< Socket lingers on close.
SO_OOBINLINE,	    //!< Out-of-band data is transmitted in line.
SO_RCVBUF,		    //!< Receive buffer size.
SO_RCVLOWAT,	    //!< Receive ``low water mark''.
SO_RCVTIMEO,	    //!< Receive timeout.
SO_REUSEADDR,	    //!< Reuse of local addresses is supported.
SO_SNDBUF,		    //!< Send buffer size.
SO_SNDLOWAT,	    //!< Send ``low water mark''.
SO_SNDTIMEO,	    //!< Send timeout.
SO_TYPE,		    //!< Socket type. 
};

#define SHUT_RD 	1	//!< Disables further receive operations.
#define SHUT_WR		2	//!< Disables further send operations. 
#define SHUT_RDWR 	3	//!< Disables further send and receive operations.

#define SOMAXCONN 4	//!< The maximum backlog queue length.
#define SOMINCONN 1

typedef size_t socklen_t;
typedef uint32_t sa_family_t;

/* The sockaddr structure is used to define a socket address which is used in the bind(), connect(), getpeername(), getsockname(), recvfrom(), and sendto() functions. */
struct sockaddr {
	sa_family_t  sa_family;	//!<  Address family. 
	char         sa_data[0];//!<  Socket address (variable-length data). 
};
struct sockaddr_storage {
	sa_family_t   ss_family;
};
#if 0
#define _SS_PAD1SIZE (_SS_ALIGNSIZE - sizeof(sa_family_t))
#define _SS_PAD2SIZE (_SS_MAXSIZE - (sizeof(sa_family_t)+ \
                      _SS_PAD1SIZE + _SS_ALIGNSIZE))
struct sockaddr_storage {
    sa_family_t  ss_family;  /* Address family. */
/*
 *  Following fields are implementation-defined.
 */
    char _ss_pad1[_SS_PAD1SIZE];
        /* 6-byte pad; this is to make implementation-defined
           pad up to alignment field that follows explicit in
           the data structure. */
    int64_t _ss_align;  /* Field to force desired structure
                           storage alignment. */
    char _ss_pad2[_SS_PAD2SIZE];
        /* 112-byte pad to achieve desired size,
           _SS_MAXSIZE value minus size of ss_family
           __ss_pad1, __ss_align fields is 112. */
};
#endif
int setsockopt(int socket, int level, int option_name, 
		const void *option_value, socklen_t option_len);
int getsockopt(int socket, int level, int option_name, 
		void *restrict option_value, socklen_t *restrict option_len);

int socket (int domain, int type, int protocol);
int connect(int socket, const struct sockaddr *address, socklen_t address_len);
int bind   (int socket, const struct sockaddr *address, socklen_t address_len);
int accept (int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);
int listen (int socket, int backlog);
int shutdown(int socket, int how);
void closesocket(int socket);
ssize_t recvfrom(int socket, void *restrict buffer, size_t length,
       int flags, struct sockaddr *restrict address,
       socklen_t *restrict address_len);
ssize_t sendto(int socket, const void *message, size_t length,
       int flags, const struct sockaddr *dest_addr,
       socklen_t dest_len);
ssize_t recv(int socket, void *buffer, size_t length, int flags);
ssize_t send(int socket, const void *buffer, size_t length, int flags);


struct iodesc* socket_iodesc(int socket);
#endif // SYS_SOCKET_H
