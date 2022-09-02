#ifndef _UNISTD_H_
#define _UNISTD_H_
#include <sys/types.h>
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
typedef struct {
#if 0
  int is_initialized;
  void *stackaddr;
  int stacksize;
  int contentionscope;
  int inheritsched;
  int schedpolicy;
  struct sched_param schedparam;

  /* P1003.4b/D8, p. 54 adds cputime_clock_allowed attribute.  */
#if defined(_POSIX_THREAD_CPUTIME)
  int  cputime_clock_allowed;  /* see time.h */
#endif
#endif
  int  detachstate;
} pthread_attr_t;
#if defined(_POSIX_THREAD_PROCESS_SHARED)
#define PTHREAD_PROCESS_SHARED  1/* visible too all processes with access to */
#define PTHREAD_PROCESS_PRIVATE 0/* visible within only the creating process */
// относится к системе
/*
extern int shared_object_close(void*);
extern void* shared_object_open (const char*, int);
extern int shared_object_unlink(const char*);
extern int shared_object_mode (void*, mode_t);
*/
#endif

#if defined(_POSIX_THREAD_PRIO_PROTECT)
/* Values for blocking protocol. */
#define PTHREAD_PRIO_NONE    0
#define PTHREAD_PRIO_INHERIT 1
#define PTHREAD_PRIO_PROTECT 2
#endif


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

#endif//_UNISTD_H_