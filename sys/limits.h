#ifndef _SYS_LIMITS_H_
#define _SYS_LIMITS_H_
//Pathname Variable Values
/* The values in the following list may be constants within an implementation or may vary from
one pathname to another. For example, file systems or directories may have different
characteristics. */
/*
{FILESIZEBITS}
Minimum number of bits needed to represent, as a signed integer value, the maximum size
of a regular file allowed in the specified directory.
Minimum Acceptable Value: 32 */
#define FILESIZEBITS 32
/* 
{LINK_MAX}
Maximum number of links to a single file.
Minimum Acceptable Value: {_POSIX_LINK_MAX} */
#define LINK_MAX _POSIX_LINK_MAX
/* {MAX_CANON}
Maximum number of bytes in a terminal canonical input line.
Minimum Acceptable Value: {_POSIX_MAX_CANON}
{MAX_INPUT}
Minimum number of bytes for which space is available in a terminal input queue; therefore,
the maximum number of bytes a conforming application may require to be typed as input
before reading them.
Minimum Acceptable Value: {_POSIX_MAX_INPUT}
{NAME_MAX}
Maximum number of bytes in a filename (not including the terminating null of a filename
string).
Minimum Acceptable Value: {_POSIX_NAME_MAX}
XSI Minimum Acceptable Value: {_XOPEN_NAME_MAX}
{PATH_MAX}
Maximum number of bytes the implementation will store as a pathname in a user-supplied
buffer of unspecified size, including the terminating null character. Minimum number the
implementation will accept as the maximum number of bytes in a pathname.
Minimum Acceptable Value: {_POSIX_PATH_MAX}
XSI Minimum Acceptable Value: {_XOPEN_PATH_MAX}*/
#define PATH_MAX _POSIX_PATH_MAX
/*{PIPE_BUF}
Maximum number of bytes that is guaranteed to be atomic when writing to a pipe.
Minimum Acceptable Value: {_POSIX_PIPE_BUF}
ADV {POSIX_ALLOC_SIZE_MIN}
Minimum number of bytes of storage actually allocated for any portion of a file.
Minimum Acceptable Value: Not specified.
ADV {POSIX_REC_INCR_XFER_SIZE}
Recommended increment for file transfer sizes between the
{POSIX_REC_MIN_XFER_SIZE} and {POSIX_REC_MAX_XFER_SIZE} values.
Minimum Acceptable Value: Not specified.
ADV {POSIX_REC_MAX_XFER_SIZE}
Maximum recommended file transfer size.
Minimum Acceptable Value: Not specified.
ADV {POSIX_REC_MIN_XFER_SIZE}
Minimum recommended file transfer size.
Minimum Acceptable Value: Not specified.
ADV {POSIX_REC_XFER_ALIGN}
Recommended file transfer buffer alignment.
Minimum Acceptable Value: Not specified.
{SYMLINK_MAX}
Maximum number of bytes in a symbolic link.
Minimum Acceptable Value: {_POSIX_SYMLINK_MAX}
*/
#define SYMLINK_MAX _POSIX_SYMLINK_MAX


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
Value: 8 */
#define _POSIX_LINK_MAX 8
/* {_POSIX_LOGIN_NAME_MAX}
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
Value: 14 */
#define _POSIX_NAME_MAX 14
/* {_POSIX_NGROUPS_MAX}
Maximum number of simultaneous supplementary group IDs per process.
Value: 8
{_POSIX_OPEN_MAX}
A value one greater than the maximum value that the system may assign to a newly-created
file descriptor.
Value: 20*/
#define _POSIX_NAME_MAX 20
/*{_POSIX_PATH_MAX}
Minimum number the implementation will accept as the maximum number of bytes in a
pathname.
Value: 256 */
#define _POSIX_PATH_MAX 256
/*{_POSIX_PIPE_BUF}
Maximum number of bytes that is guaranteed to be atomic when writing to a pipe.
Value: 512 */
/* {_POSIX_RE_DUP_MAX}
Maximum number of repeated occurrences of a BRE or ERE interval expression; see Section
9.3.6 (on page 186) and Section 9.4.6 (on page 190).
Value: 255
{_POSIX_RTSIG_MAX}
The number of realtime signal numbers reserved for application use.
Value: 8 */
#define _POSIX_RTSIG_MAX 8
/* {_POSIX_SEM_NSEMS_MAX}
The number of semaphores that a process may have.
Value: 256 */
/*{_POSIX_SEM_VALUE_MAX}
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
#define _POSIX_STREAM_MAX 8
#endif//_SYS_LIMITS_H_