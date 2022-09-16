#ifndef _NET_IF_H_
#define _NET_IF_H_

/* First released in XNS Issue 5.2. */
#define IF_NAMESIZE 4 //!< interface name length

struct if_nameindex {
	unsigned int if_index;	//!< numeric index of the interface
	char * 		 if_name;	//!< null-terminated name of the interface
};

void 					if_freenameindex(struct if_nameindex *);
char *					if_indextoname(unsigned, char *);
struct 	if_nameindex *	if_nameindex(void);
unsigned 				if_nametoindex(const char *);
#endif//_NET_IF_H_