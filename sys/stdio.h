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

// Устройство должно обладать номером и правами доступа... 
typedef struct _Device Device_t;
struct _Device {
	dev_t  dev_id:8; 
	ino_t  ino	:24;
	mode_t mode	:16;// 4 старшие бита содержат класс устройства

	uid_t 	uid:8;		// User ID of file.
	gid_t 	gid:8;		// Group ID of file.

	nlink_t nlink;
};
struct _stat {
	struct _Device d;
};
struct _fpos {
	off_t 		offset;
//	mbstate_t 	mbstate;
};

// Системно - зависимые определения: 
// Есть несколько определений: Device, File, File Descriptor (fildes), Open File Description


typedef struct _OpenFileDescription FILE;
// \see <aio.h>
struct _OpenFileDescription {
	dev_t  dev_id:8;
	int fildes:8;
//	int8_t 	prio;	// Request priority offset.
	
//	uint32_t 	nbytes; // Length of transfer.
	off_t	offset;	// File offset.
//	volatile void *  buf;	// Location of buffer.
	struct _File *file; 	// Это не обязательно файл
#if defined(_POSIX_FILE_LOCKING) && (_POSIX_FILE_LOCKING>0)
// Thread-Safe Stdio Locking
	volatile int lock;		// блокировка доступа flockfile, рекурсивный мьютекс
	volatile pthread_t owner;	// владелец блокировки
#endif//_POSIX_FILE_LOCKING 
};
struct _File {
	void* 	phandle;// на носителе
	off_t	size;	// размер на носителе
	struct timespec mtim;// Last data modification timestamp.
};


struct _DeviceClass {
	struct {
		int xfer_block:4;// размер блока в битах
		int xfer_align:4;// выравнивание в битах
		blkcnt_t  xfer_min:8;//!< число байт в одном трансфере 2^N
		blkcnt_t  xfer_max:8;//!< максимальное число баит в трансфере
	} adv;//!< Advisory Info \see pathconf()
	uint8_t alloc_size;//!< размер данных при выделении объекта
//	const dev_t dev_id;// dev_t rdev
//	struct _DeviceClass* next;
	const void* 	phandle;
//	volatile int* unique_id;
	// методы класса
	void* 	(*open) (void*, unsigned long, int);
	ssize_t	(*read) (void*, void*, size_t);
	ssize_t	(*write)(void*, const void*, size_t);
	off_t 	(*seek )(void*, off_t offset, int whence);
	off_t 	(*trunc)(void*, off_t);
	int 	(*close)(void*);
	// mmap, ioctl
};

// системно-зависимые функции:
extern Device_t* dtree_path	 (Device_t* , const char* path, const char**name);
//extern FILE* dtree_openat(FILE* , const char* name, int oflag, mode_t mode);
extern Device_t* dtree_mknodat(Device_t* fp, const char* name, mode_t mode, dev_t dev_id);
extern void  dtree_insert(Device_t* dirp, const char* name, Device_t *dev);
extern int 	 dtree_unref (Device_t* );
extern int 	 dtree_nlink (Device_t* );
// преобразует системные объекты в дескрипторы 
int  device_flag_alloc(Device_t* data);
void device_flag_free (int fildes);

// Известные иентификаторы типов системных объектов
#define DEV_FIL         0 // regular file
#define DEV_DIR         1 // directory
#define DEV_LNK         2 // symlink
#define DEV_FIFO        3 // pipe

#define DEV_SHM         4 // process shared memory
#define DEV_SEM         5 // semaphores
#define DEV_MSG         6 // message queue
#define DEV_SОС         7 // socket

#define DEV_TYM         8 // posix typed memory
#define DEV_FLA         9 // mountpoint, файловая система крепится к директории через устройство VFS

extern Device_t* _devices[];// per thread?
extern const struct _DeviceClass* dev_classes[];
#define DEV_PTR(fildes) _devices[fildes+2]
#define DEV_ID(dev)  	((dev)->dev_id)
#define DEV_CLASS(dev)  dev_classes[DEV_ID(dev)]
#define FILE_CLASS(f)  dev_classes[DEV_ID(f)]


#endif//_SYS_STDIO_H_