#ifndef _SYS_POLL_H_
#define _SYS_POLL_H_
/* The <poll.h> header shall define the following symbolic constants, zero or more of which may be OR'ed together to form the events or revents members in the pollfd structure:

POLLIN
    Data other than high-priority data may be read without blocking.
POLLRDNORM
    Normal data may be read without blocking.
POLLRDBAND
    Priority data may be read without blocking.
POLLPRI
    High priority data may be read without blocking.
POLLOUT
    Normal data may be written without blocking.
POLLWRNORM
    Equivalent to POLLOUT.
POLLWRBAND
    Priority data may be written.
POLLERR
    An error has occurred (revents only).
POLLHUP
    Device has been disconnected (revents only).
POLLNVAL
    Invalid fd member (revents only).

	\see https://pubs.opengroup.org/onlinepubs/9699919799/

 */
#include <sys/types.h>// sigset_t 
#define POLLIN 	1
#define POLLOUT	2
#define POLLERR	4
struct pollfd {
	int    fd;       // The following descriptor being polled. 
	short  events;   // The input event flags (see below). 
	short  revents;  // The output event flags (see below). 
};

typedef unsigned int nfds_t;
int poll(struct pollfd *, nfds_t, int);
int ppoll(struct pollfd *fds, nfds_t nfds,
    const struct timespec *tmo_p, const sigset_t *sigmask);

#endif // _SYS_POLL_H_
