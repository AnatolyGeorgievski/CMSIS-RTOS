#ifndef _FCNTL_H_
#define _FCNTL_H_
#define O_RDONLY        0               /* +1 == FREAD */
#define O_WRONLY        1               /* +1 == FWRITE */
#define O_RDWR          2               /* +1 == FREAD|FWRITE */
#define O_EXEC 			4	/*!< Open for execute only (non-directory files). The result is unspecified if this
								flag is applied to a directory */
#define O_DIRECTORY 	8 	/*!< Fail if file is a non-directory file */
#define O_SEARCH 		8	/*!< Open directory for search only. The result is unspecified if this flag is applied
								to a non-directory file */
#define O_CREAT 		0x10 	/*!< Create file if it does not exist. */
#define O_APPEND 		0x20	/*!< Set append mode */
#define O_EXCL          0x40	/*!< Exclusive use flag */
#define O_NONBLOCK 		0x80	/*!< Non-blocking mode. */
#define O_TRUNC 		0x100	/*!<  */

#define O_ACCMODE 		 (O_RDONLY|O_WRONLY|O_RDWR) /*!< Mask for file access modes. */
#define O_SYNC /* Write according to synchronized I/O file integrity completion. */

#define AT_FDCWD		-2
/*!< Use the current working directory to determine the target of relative file paths. */
#define AT_EACCESS 		2
/*!< Check access using effective user and group ID. */
#define AT_SYMLINK_NOFOLLOW 1 /*!< Do not follow symbolic links. */
#define AT_SYMLINK_FOLLOW   0 /*!< Follow symbolic link. */
#define AT_REMOVEDIR 		1 /*!< Remove directory instead of file. */
#include <sys/types.h>

extern int open (const char *, int, ...);
extern int openat(int, const char *, int, ...);
extern int creat(const char *, mode_t);
extern int fcntl(int, int, ...);

/* ADV _POSIX_ADVISORY_INFO */
#define POSIX_FADV_NORMAL		0
/*!< Specifies that the application has no advice to give on its behavior with respect to the
specified data. It is the default characteristic if no advice is given for an open file.*/
#define POSIX_FADV_SEQUENTIAL	1
/*!< Specifies that the application expects to access the specified data sequentially from lower
offsets to higher offsets.*/
#define POSIX_FADV_RANDOM		0
/*!< Specifies that the application expects to access the specified data in a random order.*/
#define POSIX_FADV_WILLNEED		2
/*!< Specifies that the application expects to access the specified data in the near future.*/
#define POSIX_FADV_DONTNEED		0
/*!< Specifies that the application expects that it will not access the specified data in the near
future.*/
#define POSIX_FADV_NOREUSE		0
/*!< Specifies that the application expects to access the specified data once and then not reuse it
thereafter. */

/* ADV */
int posix_fadvise(int, off_t, off_t, int);
/* ADV */
int posix_fallocate(int, off_t, off_t);
#endif//_FCNTL_H_