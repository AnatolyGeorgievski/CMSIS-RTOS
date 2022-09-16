/*! \file signal.h */
#ifndef _SIGNAL_H_
#define _SIGNAL_H_
#include <unistd.h>
#include <sys/types.h>
#include <sys/_timespec.h>
#define _SIG_ATOMIC_T_DEFINED
typedef __SIG_ATOMIC_TYPE__ sig_atomic_t; // 
typedef unsigned long sigset_t;

    //!< Integer or structure type of an object used to represent sets of signals.
extern void __aeabi_SIG_DFL(int);
#define SIG_DFL (__aeabi_SIG_DFL)
extern void __aeabi_SIG_IGN(int);
#define SIG_IGN (__aeabi_SIG_IGN)
extern void __aeabi_SIG_ERR(int);
#define SIG_ERR (__aeabi_SIG_ERR)
void (*signal(int sig, void (*func)(int)))(int);
#ifdef _AEABI_PORTABLE

extern const int __aeabi_SIGABRT;// = 6;
#define SIGABRT (__aeabi_SIGABRT)
extern const int __aeabi_SIGFPE;//  = 8;
#define SIGFPE  (__aeabi_SIGFPE)
extern const int __aeabi_SIGILL;//  = 4;
#define SIGILL  (__aeabi_SIGILL)
extern const int __aeabi_SIGINT;//  = 2;
#define SIGINT  (__aeabi_SIGINT) // receipt of an interactive attention signal
extern const int __aeabi_SIGSEGV;// = 11;
#define SIGSEGV (__aeabi_SIGSEGV)
extern const int __aeabi_SIGTERM;// = 15;
#define SIGTERM (__aeabi_SIGTERM)

#else // !_AEABI_PORTABLE
#define SIGABRT 6
#define SIGFPE	8
#define SIGILL	4
#define SIGINT	2
#define SIGSEGV	11
#define SIGTERM	15
#endif
/* The <signal.h> header shall declare the SIGRTMIN and SIGRTMAX macros, 
	which shall expand to positive integer expressions with type int, 
	but which need not be constant expressions. These macros specify 
	a range of signal numbers that are reserved for application use 
	and for which the realtime signal behavior specified in this volume of POSIX.1-2017 is supported. 

	The range SIGRTMIN through SIGRTMAX inclusive shall include at least {RTSIG_MAX} signal numbers.
*/
#define SIGRTMIN 16
#define SIGRTMAX 31
#define RTSIG_MAX (SIGRTMAX-SIGRTMIN+1)
#define NSIG 32

#define _POSIX_SIGQUEUE_MAX 32

union sigval {
	int    sival_int;   //!< Integer signal value. 
	void  *sival_ptr;   //!< Pointer signal value. 
};
/* si_code values */
#define SI_USER    1    /* Sent by a user. kill(), abort(), etc */
#define SI_QUEUE   2    /* Sent by sigqueue() */
#define SI_TIMER   3    /* Sent by expiration of a timer_settime() timer */
#define SI_ASYNCIO 4    /* Indicates completion of asycnhronous IO */
#define SI_MESGQ   5    /* Indicates arrival of a message at an empty queue */
typedef struct {
	int          si_code;     /* Cause of the signal */
	int          si_signo;    /* Signal number */
	union sigval si_value;    /* Signal value */
} siginfo_t;
// таймерная функция использует структуру sigevent при создании таймера
struct sigevent {
	int          sigev_notify;               /* Notification type */
	int          sigev_signo;                /* Signal number */
	union sigval sigev_value;                /* Signal value */
	void           (*sigev_notify_function)( union sigval );
// могут быть дополнительные параметры \see _POSIX_THREADS
	pthread_attr_t *sigev_notify_attributes;// Notification attributes.
};
struct sigaction {
  int         sa_flags;   	/* Special flags to affect behavior of signal. */
  sigset_t    sa_mask;		/* Additional set of signals to be blocked
								during execution of signal-catching function */
  union {
    void  (*_handler)(int);	/* Pointer to a signal-catching function or one of the macros SIG_IGN or SIG_DFL */
#if defined(_POSIX_REALTIME_SIGNALS)
    void  (*_sigaction)(int, siginfo_t *, void * );/* Pointer to a signal-catching function. */
#endif
  } _signal_handlers;
};
#define sa_handler    _signal_handlers._handler
#if defined(_POSIX_REALTIME_SIGNALS)
#define sa_sigaction  _signal_handlers._sigaction
#endif


/* sigev_notify values
   NOTE: P1003.1c/D10, p. 34 adds SIGEV_THREAD.  */

#define SIGEV_NONE   1  /* No asynchronous notification shall be delivered */
                        /*   when the event of interest occurs. */
#define SIGEV_SIGNAL 2  /* A queued signal, with an application defined */
                        /*  value, shall be delivered when the event of */
                        /*  interest occurs. */
#define SIGEV_THREAD 3  /* A notification function shall be called to */
                        /*   perform notification. */


void (* signal(int sig, void (*func)(int)))(int);
int     raise(int);
// POSIX.1-2017

#define sigaddset(what,sig) (*(what) |=  (1<<(sig)), 0)
#define sigdelset(what,sig) (*(what) &= ~(1<<(sig)), 0)
#define sigemptyset(what)   (*(what) =   0 , 0)
#define sigfillset(what)    (*(what) = ~(0), 0)
#define sigismember(what,sig) (((*(what)) & (1<<(sig))) != 0)

int sigqueue(pid_t, int, union sigval);
int sigtimedwait(const sigset_t *restrict, siginfo_t *restrict,
           const struct timespec *restrict);
int	sigwaitinfo(const sigset_t *restrict set, siginfo_t *restrict info);
#define SIG_SETMASK 0   /* set mask with sigprocmask() */
#define SIG_BLOCK 	1   /* set of signals to block */
#define SIG_UNBLOCK 2   /* set of signals to unblock */

int sigprocmask (int, const sigset_t *, sigset_t *);

#endif//_SIGNAL_H_