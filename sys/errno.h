#ifndef _SYS_ERRNO_H_
#define _SYS_ERRNO_H_

#if defined _AEABI_PORTABILITY_LEVEL && _AEABI_PORTABILITY_LEVEL != 0 && !defined _AEABI_PORTABLE
# define _AEABI_PORTABLE
#endif

#define	EPERM 	1	/* Not owner */
#define	ENOENT 	2	/* No such file or directory */
#define	ESRCH 	3	/* No such process */
#define	EINTR 	4	/* Interrupted system call */
#define	EIO 	5	/* I/O error */
#define	ENXIO 	6	/* No such device or address */
#define	E2BIG 	7	/* Arg list too long */
#define	ENOEXEC 8	/* Exec format error */
#define	EBADF 	9	/* Bad file number */
#define	ECHILD 10	/* No children */
#define	EAGAIN 11	/* No more processes */
#define	ENOMEM 12	/* Not enough space */
#define	EACCES 13	/* Permission denied */
#define	EFAULT 14	/* Bad address */


volatile int *__aeabi_errno_addr(void);
#define errno (*__aeabi_errno_addr())

#ifdef _AEABI_PORTABLE
extern const int __aeabi_EDOM;
#define EDOM (__aeabi_EDOM)
extern const int __aeabi_ERANGE;
#define ERANGE (__aeabi_ERANGE)
extern const int __aeabi_EILSEQ;
#define ERANGE (__aeabi_EILSEQ)

#else // _AEABI_PORTABLE
#define EDOM  	33
#define ERANGE  34
#define EILSEQ  47

#endif

#endif
