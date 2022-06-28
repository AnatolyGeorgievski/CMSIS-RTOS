/*! \brief c11 threads */
#ifndef SYS_TIMEVAL_H
#define SYS_TIMEVAL_H
#include <stdint.h>

#ifndef _TIME_T_DEFINED
#define _TIME_T_DEFINED
typedef uint32_t time_t;
#endif // _TIME_T_DEFINED
typedef uint32_t suseconds_t;
#define	_TIME_T_DECLARED
#define	_SUSECONDS_T_DECLARED
#ifndef _TIMEVAL_DEFINED
#define _TIMEVAL_DEFINED
struct timeval {
	time_t		tv_sec;		/* seconds */
	suseconds_t	tv_usec;	/* and microseconds */
};
#endif // _TIMEVAL_DEFINED
#endif // SYS_TIMEVAL_H
