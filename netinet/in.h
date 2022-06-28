#ifndef NETINET_IN_H
#define NETINET_IN_H
#include <stdint.h>
#include <sys/socket.h>

typedef uint32_t in_addr_t;
typedef uint16_t in_port_t;
struct in_addr {
	in_addr_t s_addr;
};
struct in6_addr {
	uint8_t s6_addr[16];
};
struct sockaddr_in {
	sa_family_t     sin_family;   //!<  AF_INET. 
	in_port_t       sin_port;     //!<  Port number. 
	struct in_addr  sin_addr;     //!< IP address. 
};

struct sockaddr_in6 {
	sa_family_t      sin6_family;    //!< AF_INET6. 
	in_port_t        sin6_port;      //!< Port number. 
	uint32_t         sin6_flowinfo;  //!< IPv6 traffic class and flow information. 
	struct in6_addr  sin6_addr;      //!< IPv6 address. 
	uint32_t         sin6_scope_id;  //!< Set of interfaces for a scope. 
};
extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;

/* Protocols common to RFC 1700, POSIX, and X/Open. */
//#define IPPROTO_RAW
#define	IPPROTO_IP		0		/* dummy for IP */
//#define IPPROTO_IPV6
#define	IPPROTO_ICMP	1		/* control message protocol */
#define	IPPROTO_TCP		6		/* tcp */
#define	IPPROTO_UDP		17		/* user datagram protocol */



#define	INADDR_ANY			((in_addr_t)0x00000000)
#define	INADDR_BROADCAST	((in_addr_t)0xffffffff)	/* must be masked */
#define	INET_ADDRSTRLEN 	16 /* Length of the string form for IP. */ 

#endif // NETINET_IN_H
