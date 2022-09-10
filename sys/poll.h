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
#define POLLIN 		1
/*!< Data other than high-priority data may be read without blocking. */
#define POLLRDNORM 	1
/*!< POLLRDNORM Normal data may be read without blocking. */
#define POLLRDBAND 	2
/*!< POLLRDBAND Priority data may be read without blocking. */
#define POLLPRI 	4
/*!< POLLPRI High-priority data may be read without blocking. */
#define POLLOUT		2//!< Normal data may be written without blocking
#define POLLWRNORM 	2//!< Equivalent to POLLOUT.
#define POLLWRBAND 	2//!< Priority data may be written.
#define POLLERR		4
/*!< An error has occurred on the device or stream. This flag is only valid in the
revents bitmask; it shall be ignored in the events member. */
#define POLLHUP		4
/*!< A device has been disconnected, or a pipe or FIFO has been closed by the last
process that had it open for writing. */
#define POLLNVAL 	8/*!< The specified fd value is invalid. */

/*! 
Есть два ограничения 20 дескрипторов и 8 потоков надо уложить в счетное количество бит.
 */
struct pollfd {
	unsigned short  fd	   :8;   // The following descriptor being polled. 
	unsigned short  events :3;   // The input event flags (see below). 
	unsigned short  revents:5;   // The output event flags (see below). 
};

typedef unsigned int nfds_t;
int poll(struct pollfd *, nfds_t, int);

/* int ppoll(struct pollfd *fds, nfds_t nfds,
    const struct timespec *tmo_p, const sigset_t *sigmask);
*/
#endif // _SYS_POLL_H_
