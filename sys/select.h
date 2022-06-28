#ifndef SYS_SELECT_H
#define SYS_SELECT_H
#include <sys/time.h>
#define FD_SETSIZE 32
/*
The <sys/select.h> header shall define the sigset_t type as described in <signal.h>.
The <sys/select.h> header shall define the timespec structure as described in <time.h>.
The <sys/select.h> header shall define the fd_set type as a structure.
*/
typedef struct _fd_set fd_set;
int pselect(int nfds, fd_set *restrict readfds,
       fd_set *restrict writefds, fd_set *restrict errorfds,
       const struct timespec *restrict timeout,
       const sigset_t *restrict sigmask);
int select(int nfds, fd_set *restrict readfds,
       fd_set *restrict writefds, fd_set *restrict errorfds,
       struct timeval *restrict timeout);
static inline void FD_CLR  (int fd, fd_set *fdset)
{
	fdset[0] &= ~(1<<fd);
}
static inline int  FD_ISSET(int fd, fd_set *fdset)
{
	return (fdset[0] & (1<<fd))!=0;
}
static inline void FD_SET  (int fd, fd_set *fdset)
{
	fdset[0] |= (1<<fd);
}
void FD_ZERO(fd_set *fdset)
{
	fdset[0] = 0;
}

#endif // SYS_SELECT_H