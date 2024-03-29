#ifndef SYS_SELECT_H
#define SYS_SELECT_H
#include <signal.h>
#include <time.h>
//#include <sys/types.h>

//#ifndef FD_SETSIZE
// _POSIX_STREAM_MAX = 8 _POSIX_OPEN_MAX=20 
#define FD_SETSIZE 32
//#endif
/*
The <sys/select.h> header shall define the sigset_t type as described in <signal.h>.
The <sys/select.h> header shall define the timespec structure as described in <time.h>.
The <sys/select.h> header shall define the fd_set type as a structure.
*/
typedef struct _fd_set fd_set;
struct _fd_set {
	uint32_t fds_bits[FD_SETSIZE/32];
};

/*! \addgroup POSIX_DEVICE_IO 
	\{ */
int pselect(int nfds, fd_set *restrict readfds,
       fd_set *restrict writefds, fd_set *restrict errorfds,
       const struct timespec *restrict timeout,
       const sigset_t *restrict sigmask);
// select может быть устарел?
int select(int nfds, fd_set *restrict readfds,
       fd_set *restrict writefds, fd_set *restrict errorfds,
       const struct timeval *restrict timeout);
static inline void FD_CLR  (int fd, fd_set *fdset)
{
	fdset->fds_bits[fd>>5] &= ~(1UL<<fd);
}
static inline int  FD_ISSET(int fd, fd_set *fdset)
{
	return (fdset->fds_bits[fd>>5] & (1UL<<fd))!=0;
}
static inline void FD_SET  (int fd, fd_set *fdset)
{
	fdset->fds_bits[fd>>5] |= (1UL<<fd);
}
static inline void FD_ZERO(fd_set *fdset)
{
	int i;
	for (i=0; i<(FD_SETSIZE/32); i++)
		fdset->fds_bits[i] = 0;
}
/*! \} */
#endif // SYS_SELECT_H