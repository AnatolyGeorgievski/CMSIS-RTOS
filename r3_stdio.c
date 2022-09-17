
#include <unistd.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <sys/mman.h>

#include <r3_slice.h>/* В версии C2x использовать free_sized() */
/*
теория 
Мы используем всего три вызова которые привязаны к системе
dtree_path
dtree_mknodat
dtree_unref

Файловая система - это некоторый отдельный процесс, который заполняет два дерева:
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
	device_flag_free(f->fildes);// освободить дескриптор файла
	const DeviceClass_t* dev_class = FILE_CLASS(f);
	int rc = dev_class->close(f);
	Device_t* dev = (Device_t*)f->file -1;
	g_slice_free(sizeof(FILE), f);// удалить OpenFileDescription Object
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
	
	int fildes = device_flag_alloc((void*)NULL);

	const char* name=NULL;
	Device_t* dev = dtree_path(NULL, path, &name);// follow symlink? [EACCES] ELOOP ENXIO
	if (dev==NULL) {// выйти путь не найден
		return NULL;
	} else
	if (name==NULL){// путь разобран
		if (dev->dev_id!=DEV_FIL)  return NULL;// EISDIR
	} else {
		if (dev->dev_id!=DEV_DIR) return NULL;// ENOTDIR
		if ((oflag & O_CREAT)==0) return NULL;
		dev = dtree_mknodat(dev, name, mode, DEV_FIL);// EMFILE ENOMEM EROFS
	}
	if (dtree_nlink(dev)<0) return NULL;
	
	struct _OpenFileDescription * f= g_slice_alloc(sizeof(struct _OpenFileDescription));// open file description
	f->dev_id = DEV_FIL;
	f->fildes = fildes;
	f->offset = 0;
	f->file = (struct _File *)(dev +1);
	DEV_PTR(fildes) = (Device_t*)f;
	return f;
}
/*! \brief
	\ingroup POSIX_DEVICE_IO _libc _stdio
 */	
int 	vfprintf(FILE *restrict f, const char *restrict format, va_list ap) {
	return vdprintf(f->fildes, format, ap);
}
/*! \brief
	\ingroup POSIX_DEVICE_IO _libc _stdio
 */	
int 	fprintf	(FILE *restrict f, const char *restrict format, ...)
{
	va_list ap;
	va_start(ap, format);
	int rc = vfprintf(f, format, ap);
	va_end(ap);
	return rc;
}
/*! \breaf put a string on a stream
	\ingroup POSIX_DEVICE_IO _libc _stdio
 	\return Upon successful completion, fputs( ) shall return a non-negative number. Otherwise, it shall

[CX] return EOF, set an error indicator for the stream
*/
int fputs	(const char *restrict str, FILE *restrict f) {
	const DeviceClass_t* dev_class = FILE_CLASS(f);
	return dev_class->write(f, str, strlen(str));
}
/*! \brief
	\ingroup POSIX_DEVICE_IO _libc _stdio
 */	
inline size_t fread	(      void *restrict ptr, size_t size, size_t nitems, FILE* restrict f){
	const DeviceClass_t* dev_class = FILE_CLASS(f);
	return dev_class->read (f, ptr, size*nitems);
}
/*! \brief
	\ingroup POSIX_DEVICE_IO _libc _stdio
 */	
inline size_t fwrite	(const void *restrict ptr, size_t size, size_t nitems, FILE* restrict f){
	const DeviceClass_t* dev_class = FILE_CLASS(f);
	return dev_class->write(f, ptr, size*nitems);
}
/*! \brief
	\ingroup POSIX_DEVICE_IO _libc _stdio
 */	
inline int 	feof	(FILE *f){
	return (f->offset == f->file->size);
}
/*! \brief
	\ingroup POSIX_FD_MGMT _libc _stdio
 */	
inline int fgetpos	(FILE *restrict f, fpos_t *restrict pos) {
	pos->offset = f->offset;
	return 0;
}
/*! \brief
	\ingroup POSIX_FD_MGMT _libc _stdio
 */	
inline int fsetpos(FILE * f, const fpos_t * pos){
	f->offset = pos->offset;
	return 0;
}
/*! \brief
	\ingroup POSIX_DEVICE_IO _libc _stdio
 */	
void clearerr(FILE *f) {
	
}
inline void rewind(FILE *f ){
	f->offset = 0;
}
int fflush(FILE *f){
	return 0;
}
/*! \brief
	\ingroup POSIX_FD_MGMT _libc
 */	
off_t 	ftello	(FILE *f){
	return f->offset;
}
/*! \brief
	\ingroup POSIX_FD_MGMT _libc 
 */	
int     fseeko 	(FILE *f, off_t offset, int whence){
	const DeviceClass_t* dev_class = FILE_CLASS(f);
	dev_class->seek(f, offset, whence);
	return 0;
}
inline int fileno(FILE* restrict f){// тут есть неопределенность чем закрыть close или fclose
	return f->fildes; 
}
int remove(const char *path){
	Device_t* dev = dtree_path(NULL, path, &path);
	dtree_unref(dev);
	return 0;
}
int rename(const char *old, const char *new){
	///
	return 0;
}
//!\}