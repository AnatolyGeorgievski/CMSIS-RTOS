#ifndef _SYS_LIMITS_H_
#define _SYS_LIMITS_H_
/* 	A conforming implementation shall provide values no larger than these values. 
	A conforming application must not require a smaller value for correct operation. */
#define _POSIX_CLOCKRES_MIN 20000000 /* 20 мс */ 
/* A strictly conforming application must not require a larger value for correct operation */
/*{_POSIX_AIO_LISTIO_MAX}
The number of I/O operations that can be specified in a list I/O call.
Value: 2
{_POSIX_AIO_MAX}
The number of outstanding asynchronous I/O operations.
Value: 1
{_POSIX_ARG_MAX}
Maximum length of argument to the exec functions including environment data.
Value: 4 096
{_POSIX_CHILD_MAX}
Maximum number of simultaneous processes per real user ID.
Value: 25
{_POSIX_DELAYTIMER_MAX}
The number of timer expiration overruns.
Value: 32 */
#define _POSIX_DELAYTIMER_MAX 32
/* {_POSIX_HOST_NAME_MAX}
Maximum length of a host name (not including the terminating null) as returned from the
gethostname( ) function.
Value: 255
{_POSIX_LINK_MAX}
Maximum number of links to a single file.
Value: 8
{_POSIX_LOGIN_NAME_MAX}
The size of the storage required for a login name, in bytes (including the terminating null).
Value: 9
{_POSIX_MAX_CANON}
Maximum number of bytes in a terminal canonical input queue.
Value: 255
{_POSIX_MAX_INPUT}
Maximum number of bytes allowed in a terminal input queue.
Value: 255
MSG {_POSIX_MQ_OPEN_MAX}
The number of message queues that can be open for a single process.
Value: 8
MSG {_POSIX_MQ_PRIO_MAX}
The maximum number of message priorities supported by the implementation.
Value: 32
{_POSIX_NAME_MAX}
Maximum number of bytes in a filename (not including the terminating null of a filename
string).
Value: 14
{_POSIX_NGROUPS_MAX}
Maximum number of simultaneous supplementary group IDs per process.
Value: 8
{_POSIX_OPEN_MAX}
A value one greater than the maximum value that the system may assign to a newly-created
file descriptor.
Value: 20
{_POSIX_PATH_MAX}
Minimum number the implementation will accept as the maximum number of bytes in a
pathname.
Value: 256
{_POSIX_PIPE_BUF}
Maximum number of bytes that is guaranteed to be atomic when writing to a pipe.
Value: 512
{_POSIX_RE_DUP_MAX}
Maximum number of repeated occurrences of a BRE or ERE interval expression; see Section
9.3.6 (on page 186) and Section 9.4.6 (on page 190).
Value: 255
{_POSIX_RTSIG_MAX}
The number of realtime signal numbers reserved for application use.
Value: 8
{_POSIX_SEM_NSEMS_MAX}
The number of semaphores that a process may have.
Value: 256
{_POSIX_SEM_VALUE_MAX}
The maximum value a semaphore may have.
Value: 32 767
{_POSIX_SIGQUEUE_MAX}
The number of queued signals that a process may send and have pending at the receiver(s)
at any time.
Value: 32 */
#define _POSIX_SIGQUEUE_MAX 32
/* {_POSIX_SSIZE_MAX}
The value that can be stored in an object of type ssize_t.
Value: 32 767
SS|TSP {_POSIX_SS_REPL_MAX}
The number of replenishment operations that may be simultaneously pending for a
particular sporadic server scheduler.
Value: 4
{_POSIX_STREAM_MAX}
The number of streams that one process can have open at one time.
Value: 8
*/
#endif//_SYS_LIMITS_H_