#ifndef _SYS_STDIO_H_
#define _SYS_STDIO_H_
#include <sys/types.h>
#include <sys/_timespec.h>

# define        SEEK_SET        0
# define        SEEK_CUR        1
# define        SEEK_END        2
#define 	BUFSIZ 		128
#define 	FOPEN_MAX  	20 
/*!< Number of streams which the implementation guarantees can be open
simultaneously. The value is at least eight.*/
#define FILENAME_MAX    256
/*!< Maximum size in bytes of the longest pathname that the implementation
	guarantees can be opened. */
#define EOF 			(-1)	/*!< End-of-file return value */
// Системно - зависимые определения
typedef struct _Device FILE;
struct _Device {
	const struct _DeviceClass* dev_class;
	void* 	phandle;
// структура повторяет собой struct stat <sys/stat.h>
	dev_t 	dev :8; 
	ino_t 	ino	:24; 
	mode_t 	mode:16;
	dev_t  	rdev:8;		// идентификатор блочного устройства DeviceClass_t
	int		fildes:8;	// 

	nlink_t nlink;
	
	uid_t 	st_uid;		// User ID of file.
	gid_t 	st_gid;		// Group ID of file.

	off_t	size;
	
	off_t	offset;
	
	struct timespec st_mtim;// Last data modification timestamp.
	struct timespec st_ctim;// Last file status change timestamp.
};
struct _DeviceClass {
	struct _DeviceClass* next;
	void* 	phandle;
	volatile int* unique_id;
	const dev_t dev_id;// dev_t rdev
	const char* prefix;
	void* 	(*open) (unsigned long, int);
	ssize_t	(*read) (void*, void*, size_t);
	ssize_t	(*write)(void*, const void*, size_t);
	off_t 	(*seek) (void*, off_t offset, int whence);
	int 	(*close)(void*);
};

#endif//_SYS_STDIO_H_