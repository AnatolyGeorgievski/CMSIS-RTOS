/*! \file pthread.h */
#ifndef _PTHREAD_H_
#define _PTHREAD_H_
#include <unistd.h>
// [POSIX] D.3.4 Configuration Options (System Interfaces)
// unistd.h  standard symbolic constants and types 
//  −1, the option is not supported for compilation −1, 0, or 200809L.
//  >0  the option shall be supported for compilation
#define _POSIX_ADVISORY_INFO                    200809L
#define _POSIX_ASYNCHRONOUS_IO                  200809L
#define _POSIX_BARRIERS                         200809L
#define _POSIX_CHOWN_RESTRICTED                      1
#define _POSIX_CLOCK_SELECTION                  200809L
#define _POSIX_CPUTIME                          200809L
#define _POSIX_FSYNC                            200809L
#define _POSIX_IPV6                             200809L
#define _POSIX_JOB_CONTROL                           1
#define _POSIX_MAPPED_FILES                     200809L
/* #define _POSIX_MEMLOCK                           -1 */
#define _POSIX_MEMLOCK_RANGE                    200809L
#define _POSIX_MEMORY_PROTECTION                200809L
#define _POSIX_MESSAGE_PASSING                  200809L
#define _POSIX_MONOTONIC_CLOCK                  200809L
#define _POSIX_NO_TRUNC                              1
/* #define _POSIX_PRIORITIZED_IO                    -1 */
#define _POSIX_PRIORITY_SCHEDULING              200809L
#define _POSIX_RAW_SOCKETS                      200809L
#define _POSIX_READER_WRITER_LOCKS              200809L
#define _POSIX_REALTIME_SIGNALS                 200809L
#define _POSIX_REGEXP                                1
#define _POSIX_SAVED_IDS                             1
#define _POSIX_SEMAPHORES                       200809L
#define _POSIX_SHARED_MEMORY_OBJECTS            200809L
#define _POSIX_SHELL                                 1
#define _POSIX_SPAWN                            200809L
#define _POSIX_SPIN_LOCKS                       200809L
/* #define _POSIX_SPORADIC_SERVER                   -1 */
#define _POSIX_SYNCHRONIZED_IO                  200809L
#define _POSIX_THREAD_ATTR_STACKADDR            200809L
#define _POSIX_THREAD_ATTR_STACKSIZE            200809L
#define _POSIX_THREAD_CPUTIME                   200809L
/* #define _POSIX_THREAD_PRIO_INHERIT               -1 */
/* #define _POSIX_THREAD_PRIO_PROTECT               -1 */
#define _POSIX_THREAD_PRIORITY_SCHEDULING       200809L
#define _POSIX_THREAD_PROCESS_SHARED            200809L
#define _POSIX_THREAD_SAFE_FUNCTIONS            200809L
/* #define _POSIX_THREAD_SPORADIC_SERVER            -1 */
#define _POSIX_THREADS                          200809L
#define _POSIX_TIMEOUTS                         200809L
#define _POSIX_TIMERS                           200809L
/* #define _POSIX_TRACE                             -1 */
/* #define _POSIX_TRACE_EVENT_FILTER                -1 */
/* #define _POSIX_TRACE_INHERIT                     -1 */
/* #define _POSIX_TRACE_LOG                         -1 */
/* #define _POSIX_TYPED_MEMORY_OBJECTS              -1 */

#define _POSIX_TIMESTAMP_RESOLUTION 1

#define PTHREAD_PROCESS_SHARED  1/* visible too all processes with access to */
#define PTHREAD_PROCESS_PRIVATE 0/* visible within only the creating process */
//!< The resolution in nanoseconds for all file timestamps
#define _POSIX_ASYNC_IO
//!< Asynchronous input or output operations may be performed for the associated file.
#define _POSIX_PRIO_IO
//!< Prioritized input or output operations may be performed for the associated file.
#define _POSIX_SYNC_IO
//!< Synchronized input or output operations may be performed for the associated file

#define F_OK 0//!< Test for existence of file.
#define R_OK 4//!< Test for read permission.
#define W_OK 2//!< Test for write permission.
#define X_OK 1//!< Test for execute (search) permission
# define        SEEK_SET        0
# define        SEEK_CUR        1
# define        SEEK_END        2

#define _POSIX_V7_ILP32_OFF32	1
//!< The implementation provides a C-language compilation environment with 32-bit int, long, pointer, and off_t types.
//#define _POSIX_V7_ILP32_OFFBIG
//!< The implementation provides a C-language compilation environment with 32-bit int, long, and pointer types and an off_t type using at least 64 bits.
//#define _POSIX_V7_LP64_OFF64
//!< The implementation provides a C-language compilation environment with 32-bit int and 64-bit long, pointer, and off_t types.
//#define _POSIX_V7_LPBIG_OFFBIG
//!< The implementation provides a C-language compilation environment with an int type using at least 32 bits and long, pointer, and off_t types using at least 64 bits.


extern char **environ;
int execve(const char *path, char *const argv[], char *const envp[]);
// предполагаем загрузку утилиты с носителя методом mmap
int fexecve(int fd, char *const argv[], char *const envp[]);
#include <sys/types.h>
#include <time.h>
// системно зависимые типы отправить в <sys/types.h>
typedef struct _thread *pthread_t;
typedef void* pthread_attr_t;
typedef struct _barrier pthread_barrier_t;
typedef void* pthread_barrierattr_t;
typedef struct _cond pthread_cond_t;
typedef void* pthread_condattr_t;
typedef void* pthread_key_t;
typedef struct _mtx  pthread_mutex_t;
typedef void* pthread_mutexattr_t;
typedef int   pthread_once_t;
typedef void* pthread_rwlock_t;
typedef void* pthread_rwlockattr_t;
typedef struct _lock pthread_spinlock_t;

#define PTHREAD_BARRIER_SERIAL_THREAD
#define PTHREAD_CANCEL_ASYNCHRONOUS
#define PTHREAD_CANCEL_ENABLE
#define PTHREAD_CANCEL_DEFERRED
#define PTHREAD_CANCEL_DISABLE
#define PTHREAD_CANCELED
#define PTHREAD_CREATE_DETACHED
#define PTHREAD_CREATE_JOINABLE
// TPS PTHREAD_EXPLICIT_SCHED
#define PTHREAD_INHERIT_SCHED
#define PTHREAD_MUTEX_DEFAULT
#define PTHREAD_MUTEX_ERRORCHECK
#define PTHREAD_MUTEX_NORMAL
#define PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_ROBUST
#define PTHREAD_MUTEX_STALLED
#define PTHREAD_ONCE_INIT 0
// RPI|TPI PTHREAD_PRIO_INHERIT
// MC1 PTHREAD_PRIO_NONE
// RPP|TPP PTHREAD_PRIO_PROTECT
#define PTHREAD_PROCESS_SHARED
#define PTHREAD_PROCESS_PRIVATE
// TPS PTHREAD_SCOPE_PROCESS
#define PTHREAD_SCOPE_SYSTEM

#define PTHREAD_COND_INITIALIZER	((pthread_cond_t)0)
#define PTHREAD_MUTEX_INITIALIZER 	((pthread_mutex_t)1)
#define PTHREAD_RWLOCK_INITIALIZER 	((pthread_rwlock_t)0)
struct sched_param;
/* The following shall be declared as functions and may also be defined as macros. Function
prototypes shall be provided */
int pthread_atfork(void (*)(void), void (*)(void), void(*)(void));
int pthread_attr_destroy(pthread_attr_t *);
int pthread_attr_getdetachstate(const pthread_attr_t *, int *);
int pthread_attr_getguardsize(const pthread_attr_t *restrict, size_t *restrict);
/* TPS */int pthread_attr_getinheritsched(const pthread_attr_t *restrict, int *restrict);
int pthread_attr_getschedparam(const pthread_attr_t *restrict, struct sched_param *restrict);
/* TPS */int pthread_attr_getschedpolicy(const pthread_attr_t *restrict, int *restrict);
int pthread_attr_getscope(const pthread_attr_t *restrict, int *restrict);
/* TSA TSS */int pthread_attr_getstack(const pthread_attr_t *restrict, void **restrict, size_t *restrict);
/* TSS */int pthread_attr_getstacksize(const pthread_attr_t *restrict, size_t *restrict);
int pthread_attr_init(pthread_attr_t *);
int pthread_attr_setdetachstate(pthread_attr_t *, int);
int pthread_attr_setguardsize(pthread_attr_t *, size_t);
/* TPS */int pthread_attr_setinheritsched(pthread_attr_t *, int);
int pthread_attr_setschedparam(pthread_attr_t *restrict,
const struct sched_param *restrict);
/* TPS */int pthread_attr_setschedpolicy(pthread_attr_t *, int);
int pthread_attr_setscope(pthread_attr_t *, int);
/* TSA TSS */int pthread_attr_setstack(pthread_attr_t *, void *, size_t);
/* TSS */int pthread_attr_setstacksize(pthread_attr_t *, size_t);
int pthread_barrier_destroy(pthread_barrier_t *);
int pthread_barrier_init(pthread_barrier_t *restrict, const pthread_barrierattr_t *restrict, unsigned);
int pthread_barrier_wait(pthread_barrier_t *);
int pthread_barrierattr_destroy(pthread_barrierattr_t *);
/* TSH */int pthread_barrierattr_getpshared(const pthread_barrierattr_t *restrict, int *restrict);
int pthread_barrierattr_init(pthread_barrierattr_t *);
/* TSH */int pthread_barrierattr_setpshared(pthread_barrierattr_t *, int);
int pthread_cancel(pthread_t);
int pthread_cond_broadcast(pthread_cond_t *);
int pthread_cond_destroy(pthread_cond_t *);
int pthread_cond_init(pthread_cond_t *restrict, const pthread_condattr_t *restrict);
int pthread_cond_signal(pthread_cond_t *);
int pthread_cond_timedwait(pthread_cond_t *restrict, pthread_mutex_t *restrict, const struct timespec *restrict);
int pthread_cond_wait(pthread_cond_t *restrict, pthread_mutex_t *restrict);
int pthread_condattr_destroy(pthread_condattr_t *);
int pthread_condattr_getclock(const pthread_condattr_t *restrict, clockid_t *restrict);
/* TSH */int pthread_condattr_getpshared(const pthread_condattr_t *restrict, int *restrict);
int pthread_condattr_init(pthread_condattr_t *);
int pthread_condattr_setclock(pthread_condattr_t *, clockid_t);
/* TSH */int pthread_condattr_setpshared(pthread_condattr_t *, int);
int pthread_create(pthread_t *restrict, const pthread_attr_t *restrict, void *(*)(void*), void *restrict);
int pthread_detach(pthread_t);
int pthread_equal(pthread_t, pthread_t);
void pthread_exit(void *);
//OB XSI int pthread_getconcurrency(void);
/* TCT */int pthread_getcpuclockid(pthread_t, clockid_t *);
/* TPS */int pthread_getschedparam(pthread_t, int *restrict, struct sched_param *restrict);
void *pthread_getspecific(pthread_key_t);
int pthread_join(pthread_t, void **);
int pthread_key_create(pthread_key_t *, void (*)(void*));
int pthread_key_delete(pthread_key_t);
int pthread_mutex_consistent(pthread_mutex_t *);
int pthread_mutex_destroy(pthread_mutex_t *);
/* RPP|TPP */int pthread_mutex_getprioceiling(const pthread_mutex_t *restrict, int *restrict);
int pthread_mutex_init(pthread_mutex_t *restrict, const pthread_mutexattr_t *restrict);
int pthread_mutex_lock(pthread_mutex_t *);
/* RPP|TPP */int pthread_mutex_setprioceiling(pthread_mutex_t *restrict, int, int *restrict);
int pthread_mutex_timedlock(pthread_mutex_t *restrict, const struct timespec *restrict);
int pthread_mutex_trylock(pthread_mutex_t *);
int pthread_mutex_unlock(pthread_mutex_t *);
int pthread_mutexattr_destroy(pthread_mutexattr_t *);
/* RPP|TPP */int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *restrict, int *restrict);
/* MC1 */int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *restrict, int *restrict);
/* TSH */int pthread_mutexattr_getpshared(const pthread_mutexattr_t *restrict, int *restrict);
int pthread_mutexattr_getrobust(const pthread_mutexattr_t *restrict, int *restrict);
int pthread_mutexattr_gettype(const pthread_mutexattr_t *restrict, int *restrict);
int pthread_mutexattr_init(pthread_mutexattr_t *);
/* RPP|TPP */int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *, int);
/* MC1 */int pthread_mutexattr_setprotocol(pthread_mutexattr_t *, int);
/* TSH */int pthread_mutexattr_setpshared(pthread_mutexattr_t *, int);
int pthread_mutexattr_setrobust(pthread_mutexattr_t *, int);
int pthread_mutexattr_settype(pthread_mutexattr_t *, int);
int pthread_once(pthread_once_t *, void (*)(void));
int pthread_rwlock_destroy(pthread_rwlock_t *);
int pthread_rwlock_init(pthread_rwlock_t *restrict, const pthread_rwlockattr_t *restrict);
int pthread_rwlock_rdlock(pthread_rwlock_t *);
int pthread_rwlock_timedrdlock(pthread_rwlock_t *restrict, const struct timespec *restrict);
int pthread_rwlock_timedwrlock(pthread_rwlock_t *restrict, const struct timespec *restrict);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *);
int pthread_rwlock_trywrlock(pthread_rwlock_t *);
int pthread_rwlock_unlock(pthread_rwlock_t *);
int pthread_rwlock_wrlock(pthread_rwlock_t *);
int pthread_rwlockattr_destroy(pthread_rwlockattr_t *);
/* TSH */int pthread_rwlockattr_getpshared(
const pthread_rwlockattr_t *restrict, int *restrict);
int pthread_rwlockattr_init(pthread_rwlockattr_t *);
/* TSH */int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *, int);
pthread_t pthread_self(void);
int pthread_setcancelstate(int, int *);
int pthread_setcanceltype(int, int *);
// OB XSI int pthread_setconcurrency(int);
/* TPS */int pthread_setschedparam(pthread_t, int, const struct sched_param *);
int pthread_setschedprio(pthread_t, int);
int pthread_setspecific(pthread_key_t, const void *);
int pthread_spin_destroy(pthread_spinlock_t *);
int pthread_spin_init(pthread_spinlock_t *, int);
int pthread_spin_lock(pthread_spinlock_t *);
int pthread_spin_trylock(pthread_spinlock_t *);
int pthread_spin_unlock(pthread_spinlock_t *);
void pthread_testcancel(void);
/* The following may be declared as functions, or defined as macros, or both. If functions are
declared, function prototypes shall be provided.
pthread_cleanup_pop( )
pthread_cleanup_push( ) 
*/
#endif