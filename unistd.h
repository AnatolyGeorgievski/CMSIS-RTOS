#ifndef _UNISTD_H_
#define _UNISTD_H_
<<<<<<< HEAD
/* Inclusion of the <unistd.h> header may make visible all symbols from the headers <stddef.h>,
<stdint.h>, and <stdio.h>. */
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <errno.h>
/*! \defgroup _posix Portable Operating System Interface (POSIX®)

POSIX определяет набор профилей поддерживаемых в процессе сборки и 
исполнения приложений. Список поддерживаемых в RTOS профилей задается 
в <unistd.h> набором определений. 
	\{ */
#define _POSIX_VERSION							200809L

/*! Группа добавлена для R3v2 чтобы выборочно отключать компиляцию. 
	При этом функционал ядра не меняется. 
	Функции могут быть скомпилированы в прикладной программе или библиотеке. */
#define _POSIX_DEVICE_IO						 1
#define _POSIX_FILE_SYSTEM						 1
#define _POSIX_FILE_SYSTEM_FD					1
#define _POSIX_FILE_ATTRIBUTES					1
#define _POSIX_FILE_ATTRIBUTES_FD				1
#define _POSIX_FD_MGMT							1
=======
#include <sys/types.h>
#include <errno.h>

#define _POSIX_VERSION							200809L

>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
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
#define _POSIX_MEMLOCK                           	-1 
#define _POSIX_MEMLOCK_RANGE                    200809L
#define _POSIX_MEMORY_PROTECTION                200809L
#define _POSIX_MESSAGE_PASSING                  200809L
#define _POSIX_MONOTONIC_CLOCK                  200809L
#define _POSIX_NO_TRUNC                              1
#define _POSIX_PRIORITIZED_IO                    	-1
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
#define _POSIX_SPORADIC_SERVER                   	-1
#define _POSIX_SYNCHRONIZED_IO                  200809L
#define _POSIX_THREAD_ATTR_STACKADDR            200809L
#define _POSIX_THREAD_ATTR_STACKSIZE            200809L
#define _POSIX_THREAD_CPUTIME                   200809L
#define _POSIX_THREAD_PRIO_INHERIT               	-1
#define _POSIX_THREAD_PRIO_PROTECT               	-1
#define _POSIX_THREAD_PRIORITY_SCHEDULING       200809L
#define _POSIX_THREAD_PROCESS_SHARED            200809L
#define _POSIX_THREAD_SAFE_FUNCTIONS            200809L
/* #define _POSIX_THREAD_SPORADIC_SERVER            -1 */
#define _POSIX_THREADS                          200809L
#define _POSIX_TIMEOUTS                         200809L
#define _POSIX_TIMERS                           200809L
#define _POSIX_TRACE                             	-1
#define _POSIX_TRACE_EVENT_FILTER                	-1
#define _POSIX_TRACE_INHERIT                     	-1
#define _POSIX_TRACE_LOG                         	-1
#define _POSIX_TYPED_MEMORY_OBJECTS              	-1
<<<<<<< HEAD
//! \}
// Execution-Time Symbolic Constants:
/*! All of the following values, whether defined as symbolic constants in <unistd.h> or not, may be
queried with respect to a specific file using the pathconf( ) or fpathconf( ) functions: */
#define _POSIX_ASYNC_IO 0
/*!< Asynchronous input or output operations may be performed for the associated file. */
#define _POSIX_PRIO_IO 0
/*!< Prioritized input or output operations may be performed for the associated file. */
#define _POSIX_SYNC_IO 0
/*!< Synchronized input or output operations may be performed for the associated file */

#define _POSIX_TIMESTAMP_RESOLUTION 1
/*!< The resolution in nanoseconds for all file timestamps */

#define SCHED_FIFO 	0 //First in-first out (FIFO) scheduling policy.
#define SCHED_RR 	1 //Round robin scheduling policy. PS|TPS 
#define SCHED_SPORADIC 2// Sporadic server scheduling policy.SS|TSP 
#define SCHED_OTHER 3 //Another scheduling poli PS|TPS 

struct sched_param {
	int sched_priority:8;//!< Process or thread execution scheduling priority
};
typedef struct {
  unsigned short int stacksize;
  struct sched_param schedparam;
  unsigned int detachstate	:1;
  unsigned int inheritsched	:1;
  unsigned int schedpolicy	:2;
//  void *stackaddr;
=======

#define _POSIX_TIMESTAMP_RESOLUTION 1
	//!< The resolution in nanoseconds for all file timestamps


typedef struct {
#if 0
  int is_initialized;
  int contentionscope;
  int inheritsched;
  int schedpolicy;
  struct sched_param schedparam;

  /* P1003.4b/D8, p. 54 adds cputime_clock_allowed attribute.  */
#if defined(_POSIX_THREAD_CPUTIME)
  int  cputime_clock_allowed;  /* see time.h */
#endif
#endif
#if defined(_POSIX_THREAD_ATTR_STACKADDR) && (_POSIX_THREAD_ATTR_STACKADDR>0)
  void *stackaddr;
#endif
#if defined(_POSIX_THREAD_ATTR_STACKSIZE) && (_POSIX_THREAD_ATTR_STACKSIZE>0)
  int stacksize;
#endif
  int  detachstate;
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
} pthread_attr_t;
#if defined(_POSIX_THREAD_PROCESS_SHARED)
#define PTHREAD_PROCESS_SHARED  1/* visible too all processes with access to */
#define PTHREAD_PROCESS_PRIVATE 0/* visible within only the creating process */
#endif

#if defined(_POSIX_THREAD_PRIO_PROTECT) && (_POSIX_THREAD_PRIO_PROTECT>0)
/* Values for blocking protocol. */
#define PTHREAD_PRIO_NONE    0
#define PTHREAD_PRIO_INHERIT 1
#define PTHREAD_PRIO_PROTECT 2
#endif


<<<<<<< HEAD

#define STDERR_FILENO 	2//!< File number of stderr; 2.
#define STDIN_FILENO 	0//!< File number of stdin;  0.
#define STDOUT_FILENO 	1//!< File number of stdout; 1.
=======
#define _POSIX_ASYNC_IO
//!< Asynchronous input or output operations may be performed for the associated file.
#define _POSIX_PRIO_IO
//!< Prioritized input or output operations may be performed for the associated file.
#define _POSIX_SYNC_IO
//!< Synchronized input or output operations may be performed for the associated file
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299

#define STDERR_FILENO 	2//!< File number of stderr; 2.
#define STDIN_FILENO 	0//!< File number of stdin;  0.
#define STDOUT_FILENO 	1//!< File number of stdout; 1.

#define F_OK 0//!< Test for existence of file.
#define R_OK 4//!< Test for read permission.
#define W_OK 2//!< Test for write permission.
#define X_OK 1//!< Test for execute (search) permission
# define        SEEK_SET        0
# define        SEEK_CUR        1
# define        SEEK_END        2

#define _POSIX_V7_ILP32_OFF32	1
//!< The implementation provides a C-language compilation environment with 32-bit int, long, pointer, and off_t types.
#define _POSIX_V7_ILP32_OFFBIG 	-1
//!< The implementation provides a C-language compilation environment with 32-bit int, long, and pointer types and an off_t type using at least 64 bits.
#define _POSIX_V7_LP64_OFF64 	-1
//!< The implementation provides a C-language compilation environment with 32-bit int and 64-bit long, pointer, and off_t types.
#define _POSIX_V7_LPBIG_OFFBIG 	-1
//!< The implementation provides a C-language compilation environment with an int type using at least 32 bits and long, pointer, and off_t types using at least 64 bits.

/*
 *  sysconf values per IEEE Std 1003.1, 2008 Edition
 */
<<<<<<< HEAD
=======

>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
#define _SC_ARG_MAX                       0
#define _SC_CHILD_MAX                     1
#define _SC_CLK_TCK                       2
#define _SC_NGROUPS_MAX                   3
#define _SC_OPEN_MAX                      4
#define _SC_JOB_CONTROL                   5
#define _SC_SAVED_IDS                     6
#define _SC_VERSION                       7
#define _SC_PAGESIZE                      8
#define _SC_PAGE_SIZE                     _SC_PAGESIZE
/* These are non-POSIX values we accidentally introduced in 2000 without
   guarding them.  Keeping them unguarded for backward compatibility. */
#define _SC_NPROCESSORS_CONF              9
#define _SC_NPROCESSORS_ONLN             10
#define _SC_PHYS_PAGES                   11
#define _SC_AVPHYS_PAGES                 12
/* End of non-POSIX values. */
#define _SC_MQ_OPEN_MAX                  13
#define _SC_MQ_PRIO_MAX                  14
#define _SC_RTSIG_MAX                    15
#define _SC_SEM_NSEMS_MAX                16
#define _SC_SEM_VALUE_MAX                17
#define _SC_SIGQUEUE_MAX                 18
#define _SC_TIMER_MAX                    19
#define _SC_TZNAME_MAX                   20
#define _SC_ASYNCHRONOUS_IO              21
#define _SC_FSYNC                        22
#define _SC_MAPPED_FILES                 23
#define _SC_MEMLOCK                      24
#define _SC_MEMLOCK_RANGE                25
#define _SC_MEMORY_PROTECTION            26
#define _SC_MESSAGE_PASSING              27
#define _SC_PRIORITIZED_IO               28
#define _SC_REALTIME_SIGNALS             29
#define _SC_SEMAPHORES                   30
#define _SC_SHARED_MEMORY_OBJECTS        31
#define _SC_SYNCHRONIZED_IO              32
#define _SC_TIMERS                       33
#define _SC_AIO_LISTIO_MAX               34
#define _SC_AIO_MAX                      35
#define _SC_AIO_PRIO_DELTA_MAX           36
#define _SC_DELAYTIMER_MAX               37
#define _SC_THREAD_KEYS_MAX              38
#define _SC_THREAD_STACK_MIN             39
#define _SC_THREAD_THREADS_MAX           40
#define _SC_TTY_NAME_MAX                 41
#define _SC_THREADS                      42
#define _SC_THREAD_ATTR_STACKADDR        43
#define _SC_THREAD_ATTR_STACKSIZE        44
#define _SC_THREAD_PRIORITY_SCHEDULING   45
#define _SC_THREAD_PRIO_INHERIT          46
/* _SC_THREAD_PRIO_PROTECT was _SC_THREAD_PRIO_CEILING in early drafts */
#define _SC_THREAD_PRIO_PROTECT          47
#define _SC_THREAD_PRIO_CEILING          _SC_THREAD_PRIO_PROTECT
#define _SC_THREAD_PROCESS_SHARED        48
#define _SC_THREAD_SAFE_FUNCTIONS        49
#define _SC_GETGR_R_SIZE_MAX             50
#define _SC_GETPW_R_SIZE_MAX             51
#define _SC_LOGIN_NAME_MAX               52
#define _SC_THREAD_DESTRUCTOR_ITERATIONS 53
#define _SC_ADVISORY_INFO                54
#define _SC_ATEXIT_MAX                   55
#define _SC_BARRIERS                     56
#define _SC_BC_BASE_MAX                  57
#define _SC_BC_DIM_MAX                   58
#define _SC_BC_SCALE_MAX                 59
#define _SC_BC_STRING_MAX                60
#define _SC_CLOCK_SELECTION              61
#define _SC_COLL_WEIGHTS_MAX             62
#define _SC_CPUTIME                      63
#define _SC_EXPR_NEST_MAX                64
#define _SC_HOST_NAME_MAX                65
#define _SC_IOV_MAX                      66
#define _SC_IPV6                         67
#define _SC_LINE_MAX                     68
#define _SC_MONOTONIC_CLOCK              69
#define _SC_RAW_SOCKETS                  70
#define _SC_READER_WRITER_LOCKS          71
#define _SC_REGEXP                       72
#define _SC_RE_DUP_MAX                   73
#define _SC_SHELL                        74
#define _SC_SPAWN                        75
#define _SC_SPIN_LOCKS                   76
#define _SC_SPORADIC_SERVER              77
#define _SC_SS_REPL_MAX                  78
#define _SC_SYMLOOP_MAX                  79
#define _SC_THREAD_CPUTIME               80
#define _SC_THREAD_SPORADIC_SERVER       81
#define _SC_TIMEOUTS                     82
#define _SC_TRACE                        83
#define _SC_TRACE_EVENT_FILTER           84
#define _SC_TRACE_EVENT_NAME_MAX         85
#define _SC_TRACE_INHERIT                86
#define _SC_TRACE_LOG                    87
#define _SC_TRACE_NAME_MAX               88
#define _SC_TRACE_SYS_MAX                89
#define _SC_TRACE_USER_EVENT_MAX         90
#define _SC_TYPED_MEMORY_OBJECTS         91
#define _SC_V7_ILP32_OFF32               92
#define _SC_V6_ILP32_OFF32               _SC_V7_ILP32_OFF32
#define _SC_XBS5_ILP32_OFF32             _SC_V7_ILP32_OFF32
#define _SC_V7_ILP32_OFFBIG              93
#define _SC_V6_ILP32_OFFBIG              _SC_V7_ILP32_OFFBIG
#define _SC_XBS5_ILP32_OFFBIG            _SC_V7_ILP32_OFFBIG
#define _SC_V7_LP64_OFF64                94
#define _SC_V6_LP64_OFF64                _SC_V7_LP64_OFF64
#define _SC_XBS5_LP64_OFF64              _SC_V7_LP64_OFF64
#define _SC_V7_LPBIG_OFFBIG              95
#define _SC_V6_LPBIG_OFFBIG              _SC_V7_LPBIG_OFFBIG
#define _SC_XBS5_LPBIG_OFFBIG            _SC_V7_LPBIG_OFFBIG
#define _SC_XOPEN_CRYPT                  96
#define _SC_XOPEN_ENH_I18N               97
#define _SC_XOPEN_LEGACY                 98
#define _SC_XOPEN_REALTIME               99
#define _SC_STREAM_MAX                  100
#define _SC_PRIORITY_SCHEDULING         101
#define _SC_XOPEN_REALTIME_THREADS      102
#define _SC_XOPEN_SHM                   103
#define _SC_XOPEN_STREAMS               104
#define _SC_XOPEN_UNIX                  105
#define _SC_XOPEN_VERSION               106
#define _SC_2_CHAR_TERM                 107
#define _SC_2_C_BIND                    108
#define _SC_2_C_DEV                     109
#define _SC_2_FORT_DEV                  110
#define _SC_2_FORT_RUN                  111
#define _SC_2_LOCALEDEF                 112
#define _SC_2_PBS                       113
#define _SC_2_PBS_ACCOUNTING            114
#define _SC_2_PBS_CHECKPOINT            115
#define _SC_2_PBS_LOCATE                116
#define _SC_2_PBS_MESSAGE               117
#define _SC_2_PBS_TRACK                 118
#define _SC_2_SW_DEV                    119
#define _SC_2_UPE                       120
#define _SC_2_VERSION                   121
#define _SC_THREAD_ROBUST_PRIO_INHERIT  122
#define _SC_THREAD_ROBUST_PRIO_PROTECT  123
#define _SC_XOPEN_UUCP                  124
#define _SC_LEVEL1_ICACHE_SIZE          125
#define _SC_LEVEL1_ICACHE_ASSOC         126
#define _SC_LEVEL1_ICACHE_LINESIZE      127
#define _SC_LEVEL1_DCACHE_SIZE          128
#define _SC_LEVEL1_DCACHE_ASSOC         129
#define _SC_LEVEL1_DCACHE_LINESIZE      130
#define _SC_LEVEL2_CACHE_SIZE           131
#define _SC_LEVEL2_CACHE_ASSOC          132
#define _SC_LEVEL2_CACHE_LINESIZE       133
#define _SC_LEVEL3_CACHE_SIZE           134
#define _SC_LEVEL3_CACHE_ASSOC          135
#define _SC_LEVEL3_CACHE_LINESIZE       136
#define _SC_LEVEL4_CACHE_SIZE           137
#define _SC_LEVEL4_CACHE_ASSOC          138
#define _SC_LEVEL4_CACHE_LINESIZE       139
#define _SC_POSIX_26_VERSION            140
<<<<<<< HEAD

/*
 *  pathconf values per IEEE Std 1003.1, 2008 Edition
 */
#define _PC_LINK_MAX                      0
#define _PC_MAX_CANON                     1
#define _PC_MAX_INPUT                     2
#define _PC_NAME_MAX                      3
#define _PC_PATH_MAX                      4
#define _PC_PIPE_BUF                      5
#define _PC_CHOWN_RESTRICTED              6
#define _PC_NO_TRUNC                      7
#define _PC_VDISABLE                      8
#define _PC_ASYNC_IO                      9
#define _PC_PRIO_IO                      10
#define _PC_SYNC_IO                      11
#define _PC_FILESIZEBITS                 12
#define _PC_2_SYMLINKS                   13
#define _PC_SYMLINK_MAX                  14
#define _PC_ALLOC_SIZE_MIN               15
#define _PC_REC_INCR_XFER_SIZE           16
#define _PC_REC_MAX_XFER_SIZE            17
#define _PC_REC_MIN_XFER_SIZE            18
#define _PC_REC_XFER_ALIGN               19
#define _PC_TIMESTAMP_RESOLUTION         20

=======
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299

int execve(const char *path, char *const argv[], char *const envp[]);
// предполагаем загрузку утилиты с носителя методом mmap
int fexecve(int fd, char *const argv[], char *const envp[]);

int access(const char *, int);
<<<<<<< HEAD
int faccessat(int, const char *, int, int);
=======
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
unsigned alarm(unsigned);
int chdir(const char *);
int chown(const char *, uid_t, gid_t);
int close(int);
size_t confstr(int, char *, size_t);
int fchdir(int);
int fchown(int, uid_t, gid_t);
int fdatasync(int);
long fpathconf(int, int);
<<<<<<< HEAD
/* SIO */
int fsync(int);
/* SIO */
int fdatasync(int);
int  truncate(const char *, off_t);
int ftruncate(int, off_t);

char *getcwd(char *, size_t);
char *getlogin(void);
int getlogin_r(char *, size_t);
int getopt(int, char * const [], const char *);
pid_t getpgid(pid_t);
pid_t getpgrp(void);
pid_t getpid(void);
pid_t getppid(void);
pid_t getsid(pid_t);
uid_t getuid(void);
int isatty(int);
int lchown(const char *, uid_t, gid_t);
int link(const char *, const char *);
int linkat(int, const char *, int, const char *, int);
off_t lseek(int fildes, off_t offset, int whence);
long pathconf(const char *, int);
int pause(void);
int pipe(int [2]);
ssize_t pread(int, void *, size_t, off_t);
ssize_t pwrite(int, const void *, size_t, off_t);
ssize_t read(int, void *, size_t);
void sync(void);
long sysconf(int);
int unlink(const char *);
int unlinkat(int, const char *, int);
ssize_t write(int, const void *, size_t);

int getopt(int argc, char * const argv[], const char *optstring);
extern char **environ;
extern char *optarg;
extern int opterr, optind, optopt;

=======
int fsync(int);
int ftruncate(int, off_t);
char *getcwd(char *, size_t);
long pathconf(const char *, int);
int pause(void);
int pipe(int [2]);
ssize_t pwrite(int, const void *, size_t, off_t);
ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset);
ssize_t read(int fildes, void *buf, size_t nbyte);
off_t lseek(int fildes, off_t offset, int whence);
int unlink(const char *);
ssize_t write(int, const void *, size_t);

>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
#endif//_UNISTD_H_