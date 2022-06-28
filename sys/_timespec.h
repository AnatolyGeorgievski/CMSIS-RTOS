/*! \brief C11 threads */
#ifndef SYS_TIMESPEC_H
#define SYS_TIMESPEC_H
#include <sys/_timeval.h>
//typedef uint32_t time_t;

#ifndef _TIMESPEC_DEFINED
#define _TIMESPEC_DEFINED
struct timespec {
	time_t	tv_sec;		/* seconds */
	long	tv_nsec;	/* and nanoseconds */
};
#endif // _TIMESPEC_DEFINED
#endif // SYS_TIMESPEC_H
