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
#define AT_FDCWD		-2
#include <sys/types.h>

extern int open (const char *, int, ...);
extern int creat(const char *, mode_t);
extern int fcntl(int, int, ...);

#endif//_FCNTL_H_