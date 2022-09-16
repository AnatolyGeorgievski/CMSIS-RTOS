#ifndef _ARPA_INET_H_
#define _ARPA_INET_H_
#include <stdint.h>
#include <netinet/in.h>
/* The <arpa/inet.h> header shall define the in_port_t and in_addr_t types as described in
<netinet/in.h>. */
/* The <arpa/inet.h> header shall define the in_addr structure as described in <netinet/in.h>. */
/* The <arpa/inet.h> header shall define the INET_ADDRSTRLEN and INET6_ADDRSTRLEN
macros as described in <netinet/in.h>. */
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)
#define htonl(a) __builtin_bswap32(a)
#define htons(a) __builtin_bswap16(a)
#define ntohl(a) __builtin_bswap32(a)
#define ntohs(a) __builtin_bswap16(a)
#else
#define htonl(a) (a)
#define htons(a) (a)
#define ntohl(a) (a)
#define ntohs(a) (a)
#endif

in_addr_t inet_addr(const char *);
char *inet_ntoa(struct in_addr);
const char *inet_ntop(int, const void *restrict, char *restrict, socklen_t);
int inet_pton(int, const char *restrict, void *restrict);

#endif//_ARPA_INET_H_
