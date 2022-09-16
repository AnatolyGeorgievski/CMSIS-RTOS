#ifndef _SYS_UN_H_
#define _SYS_UN_H_
#include <sys/socket.h>
//  definitions for UNIX domain sockets
struct sockaddr_un {
	sa_family_t sun_family;	//!< Address family.
	char sun_path[];		//!< Socket pathname.
};

#endif//_SYS_UN_H_