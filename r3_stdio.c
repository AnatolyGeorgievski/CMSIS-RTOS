
#include <unistd.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>

#include <r3_slice.h>/* В версии C2x использовать free_sized() */
<<<<<<< HEAD
/*
теория 
Мы используем всего три вызова которые привязаны к системе
dtree_path
dtree_mknodat
dtree_unref

Файловая система - это некоторый отдельный процесс который заполняет два дерева:
Директория dtee_* поиск по пути и есть возможность находить объекты по идентификаторам (dev, ino). 
Основаня возможность - это отображение прямо в память без копирования, с использованием функций mmap

Each wide-oriented stream has an associated mbstate_t object that stores the current parse state
of the stream. A successful call to fgetpos( ) shall store a representation of the value of this
mbstate_t object as part of the value of the fpos_t object. A later successful call to fsetpos( ) using
the same stored fpos_t value shall restore the value of the associated mbstate_t object as well as
the position within the controlled stream

*/

/*!	\defgroup _libc C Library (libc)
 */
/*!	\defgroup _stdio <stdio.h> Стандартный ввод-вывод 
	\ingroup _libc
 */

typedef struct _Device Device_t;
typedef struct _DeviceClass DeviceClass_t;
/*!	\brief 
	\ingroup POSIX_DEVICE_IO _libc _stdio

*/
int 	fclose	(FILE *f){
	Device_t * dev = (Device_t*)f -1;
	const DeviceClass_t* dev_class = DEV_CLASS(dev);
	int rc = dev_class->close(f);
	dtree_unref(dev);
	return rc;
}
#define O_BINARY 0
/*! \brief
	\ingroup POSIX_DEVICE_IO _libc _stdio

	\param [in] mods 
	\arg r or rb 		Open file for reading.
	\arg w or wb 		Truncate to zero length or create file for writing.
	\arg a or ab 		Append; open or create file for writing at end-of-file.
	\arg r+ or rb+ or r+b Open file for update (reading and writing).
	\arg w+ or wb+ or w+b Truncate to zero length or create file for update.
	\arg a+ or ab+ or a+b Append; open or create file for update, writing at end-of-file.
*/
FILE *	fopen	(const char *restrict path, const char *restrict mods)
{
	int oflag=0;
	mode_t mode = 0666;//  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
	if (mods[0]=='r') oflag |= O_RDONLY;
	else
	if (mods[0]=='w') oflag |= O_WRONLY|O_CREAT|O_TRUNC;
	else
	if (mods[0]=='a') oflag |= O_WRONLY|O_CREAT|O_APPEND;
	mods++;
	if (mods[0]=='b') oflag |= O_BINARY, mods++;//
	if (mods[0]=='+') {
		oflag &=~(O_RDONLY|O_WRONLY); 
		oflag |= O_RDWR;
		mods++;
	}
	if (mods[0]=='b') oflag |= O_BINARY;//
	
	const char* name;
	Device_t* dev = dtree_path(NULL, path, &name);// follow symlink? [EACCES] ELOOP ENXIO
	if (dev==NULL) {// выйти путь не найден
		return NULL;
	} else
	if (name==NULL){// путь разобран
		if (dev->dev_id!=DEV_FIL)  return NULL;// EISDIR
		if (dtree_nlink(dev)<0) return NULL;// ENOENT
/*		if (oflag & O_APPEND) dev->offset = dev->size;
		else
		if (oflag & O_TRUNC ) dev->offset = 0; */
	} else {
		if (dev->dev_id!=DEV_DIR) return NULL;// ENOTDIR
		if ((oflag & O_CREAT)==0) return NULL;
		dev = dtree_mknodat(dev, name, mode, DEV_FIL);// EMFILE ENOMEM EROFS
	}
	return (FILE*)(dev+1);
}
/*! \brief
	\ingroup POSIX_DEVICE_IO _libc _stdio
 */	
int 	vfprintf(FILE *restrict f, const char *restrict format, va_list ap)
{
	// Если определено POSIX MEMORY MAPPING
	int fildes = fileno(f);// может вернуть -1
	char* str = mmap(0, BUFSIZ, PROT_WRITE, MAP_PRIVATE, fildes, f->offset);
	int rc = vsnprintf(str, BUFSIZ, format, ap);
	f->size += rc;
	fdatasync(fildes);
	munmap(str, BUFSIZ);
	return rc;
}
/*! \brief
	\ingroup POSIX_DEVICE_IO _libc _stdio
 */	
=======
// системно-зависимые функции:
extern FILE* dtree_path	 (FILE* f, const char* path, const char**name);
extern FILE* dtree_openat(FILE* f, const char* name, mode_t mode);
extern int 	 dtree_unref (FILE* f);

int 	fclose	(FILE *f){
	int rc = f->dev_class->close(f->phandle);
	if(dtree_unref(f)==0)
		g_slice_free(FILE, f);
	return rc;
}
#define O_BINARY 0
/*
r or rb 		Open file for reading.
w or wb 		Truncate to zero length or create file for writing.
a or ab 		Append; open or create file for writing at end-of-file.
r+ or rb+ or r+b Open file for update (reading and writing).
w+ or wb+ or w+b Truncate to zero length or create file for update.
a+ or ab+ or a+b Append; open or create file for update, writing at end-of-file.
*/
FILE *	fopen	(const char *restrict path, const char *restrict mods)
{
	mode_t mode=0;
	if (mods[0]=='r') mode |= O_RDONLY;
	else
	if (mods[0]=='w') mode |= O_WRONLY|O_CREAT|O_TRUNC;
	else
	if (mods[0]=='a') mode |= O_WRONLY|O_CREAT|O_APPEND;
	mods++;
	if (mods[0]=='b') mode |= O_BINARY, mods++;//
	if (mods[0]=='+') {
		mode &=~(O_RDONLY|O_WRONLY); 
		mode |= O_RDWR;
		mods++;
	}
	if (mods[0]=='b') mode |= O_BINARY;//
	
	const char* name = NULL;
	FILE* dev = dtree_path(NULL, path, &name);
	if (dev==NULL) return NULL;
	if (name!=NULL && (mode & O_CREAT)==0) return NULL;
	dev = dtree_openat(dev, name, mode);
	return dev;
}
int 	vfprintf(FILE *restrict f, const char *restrict format, va_list ap)
{
	// Если определено POSIX MEMORY MAPPING
	char* str = mmap(0, BUFSIZ, PROT_WRITE, MAP_PRIVATE, f->fildes, f->offset);
	int rc = vsnprintf(str, BUFSIZ, format, ap);
	f->size += rc;
	fdatasync(f->fildes);
	munmap(str, BUFSIZ);
	return rc;
}
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
int 	fprintf	(FILE *restrict f, const char *restrict format, ...)
{
	va_list ap;
	va_start(ap, format);
	int rc = vfprintf(f, format, ap);
	va_end(ap);
	return rc;
}
/*! \breaf put a string on a stream
<<<<<<< HEAD
	\ingroup POSIX_DEVICE_IO _libc _stdio
=======
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
 	\return Upon successful completion, fputs( ) shall return a non-negative number. Otherwise, it shall

[CX] return EOF, set an error indicator for the stream
*/
<<<<<<< HEAD
int fputs	(const char *restrict str, FILE *restrict f) {
	Device_t* dev = (Device_t*)f -1;
	const DeviceClass_t* dev_class = DEV_CLASS(dev);
	return dev_class->write(f, str, strlen(str));
}
/*! \brief
	\ingroup POSIX_DEVICE_IO _libc _stdio
 */	
size_t fread	(      void *restrict ptr, size_t size, size_t nitems, FILE* restrict f){
	Device_t* dev = (Device_t*)f -1;
	const DeviceClass_t* dev_class = DEV_CLASS(dev);
	return dev_class->read (f, ptr, size*nitems);
}
/*! \brief
	\ingroup POSIX_DEVICE_IO _libc _stdio
 */	
size_t fwrite	(const void *restrict ptr, size_t size, size_t nitems, FILE* restrict f){
	Device_t* dev = (Device_t*)f -1;
	const DeviceClass_t* dev_class = DEV_CLASS(dev);
	return dev_class->write(f, ptr, size*nitems);
}
/*! \brief
	\ingroup POSIX_DEVICE_IO _libc _stdio
 */	
int 	feof	(FILE *f){
	return (f->offset == f->size);
}
/*! \brief
	\ingroup POSIX_FD_MGMT _libc _stdio
 */	
int fgetpos	(FILE *restrict f, fpos_t *restrict pos) {
	pos->offset = f->offset;
	return 0;
}
/*! \brief
	\ingroup POSIX_FD_MGMT _libc _stdio
 */	
int fsetpos(FILE * f, const fpos_t * pos){
	f->offset = pos->offset;
	return 0;
}
#if 0
int fileno(FILE* restrict f){// тут есть неопределенность чем закрыть close или fclose
	return -1;
/*	if (f->fildes<0)
		f->fildes = device_flag_alloc((Device_t*)f -1);
	return f->fildes; */
}
#endif
//!\}
=======
int 	fputs	(const char *restrict str, FILE *restrict f) {
	return f->dev_class->write(f->phandle, str, strlen(str));
}
FILE *	tmpfile	(void)
{
	return NULL;
}
>>>>>>> 70f57831c2d5e46eb0d6195ba6a29572a4c13299
