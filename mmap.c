#include <unistd.h>
#include <stdlib.h>
//long sysconf(int name); // значение системной переменной по идентификатору _SC_PAGESIZE
#include <sys/types.h>
#include <sys/stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
typedef struct _DeviceClass DeviceClass_t;

#define MAPPED_DEVICE(a) (((Mapped_t*)(a) -1)->dev)
#define MAPPED_ADDR(a) 	  ((Mapped_t*)(a) -1)
typedef struct _Mapped Mapped_t;
struct _Mapped {
	int fildes;
	void* 		adr;
	off_t 		off;
	off_t 		len;
};
#if defined(_POSIX_MAPPED_FILES) && (_POSIX_MAPPED_FILES>0)
/*! \defgroup POSIX_MAPPED_FILES POSIX: Memory Mapped Files
	\ingroup _posix

mmap( ), munmap( ) 
	\{ */

/*! \brief преобразует идентификатор объекта в указатель в памяти процесса
    \param addr адрес для выделения методом MAP_FIXED
    \param prot доступ
		\arg PROT_READ Data can be read.
		\arg PROT_WRITE Data can be written.
		\arg PROT_EXEC Data can be executed.
		\arg PROT_NONE Data cannot be accessed
    \param flags флаги
		\arg MAP_SHARED Changes are shared.
		\arg MAP_PRIVATE Changes are private.
		\arg MAP_FIXED Interpret addr exactly
    \param fildes идентификатор объекта
	\return адрес в памяти процесса (pa)

работа с общей памятью:
\code{.c}
fd = shm_open(path, O_RDWR|O_CREAT, 0644 );
ftruncate(fd, size);
buff = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED,  fd, offset);
if (MAP_FAILED == buff)  ...
. . .
munmap(buff, size);
close(fd);
shm_unlink(fd);
\endcode


Чтобы было проще понять явление, надо представить что addr - некоторый адрес в памяти сложно доступной, 
например во флеш, QSPI.
Открытие файлов с загрузкой содержимого выглядит таким побразом

fd = open(path, O_RDONLY, 0644 );
ftruncate(fd, size);
buff = mmap(NULL, size, PROT_READ, MAP_FIXED,  fd, offset);
-- если асинхронный доступ, то вероятно надо еще подожать дескриптор методом 
-- доступ к файлам может быть по блокам
poll или pselect. 
munmap(buf, len) -- освобождает буфер, если он выделен понятным способом
Непонятный способ выделения MAP_FIXED
 */
void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off) {
	void* pa = NULL;
	Device_t* dev = DEV_PTR(fildes);
	if (flags == MAP_SHARED) {
		void** ptr = (void**)(dev +1);// указатель на области памяти
		pa = (*ptr) + off;
	} else
	if (flags == MAP_FIXED) {
		pa = addr + off;
	}
	return pa;
}
/*!

The munmap() function shall remove any mappings for those entire pages containing
any part of the address space of the process starting at addr and continuing for len bytes.
Further references to these pages shall result in the generation of a SIGSEGV signal to the process.
If there are no mappings in the specified address range, then munmap() has no effect.

The implementation may require that addr be a multiple of the page size as returned by sysconf().
 */
int munmap(void *addr, size_t len){
	return 0;
}
/*! \breif  synchronize memory with physical storage 

	The msync( ) function shall write all modified data to permanent storage locations, if any, in
	those whole pages containing any part of the address space of the process starting at address
	addr and continuing for len bytes. If no such storage exists, msync( ) need not have any effect. If
	requested, the msync( ) function shall then invalidate cached copies of data.
 */
int msync(void *data, size_t size, int flag)
{
#if 0
	if (flag & MS_SYNC) {// wait for op complete
		Device_t* dev = MAPPED_DEVICE(data);
		const DeviceClass_t* dev_class = DEV_CLASS(dev);
		dev_class->write(dev+1, data, size);
		if (flag & MS_INVALIDATE) {// invalidate cached copies of data
			
		}
	}
#endif
}
	//!\}
#endif
/*! \defgroup POSIX_MEMORY_PROTECTION POSIX: Memory Protection
	\ingroup _posix
mprotect( )
	\{ */
int mprotect(void *, size_t, int);
	//!\}
// возвращает дескриптор файла номер и тип памяти

int posix_typed_mem_open(const char *path, int oflags, int tflag){
	Device_t *dev = dtree_path(NULL, path, &path);
	if (dev==NULL) return -1;
	if (dev!=NULL && (oflags & (O_CREAT))) {
		dev = dtree_mknodat(dev, path, 0666, DEV_TYM);// разрешить доступ и назначить права
	}
	return device_flag_alloc(dev);

}
int posix_mem_offset(const void *restrict addr, size_t len,
	off_t *restrict off, size_t *restrict contig_len, int *restrict fildes)
{
	Mapped_t* map = MAPPED_ADDR(addr);
	if(fildes) *fildes = map->fildes;//map->fildes;
	if(off) *off = map->off;
	if(contig_len) *contig_len = map->len;
	return 0;
}
int posix_typed_mem_get_info(int fildes, struct posix_typed_mem_info * info){
	Device_t* dev = DEV_PTR(fildes);
	
	return 0;
}
/*! \defgroup POSIX_SHARED_MEMORY_OBJECTS POSIX: Shared Memory Objects (SHM)
	\ingroup _posix
	\{ */
/*! \brief 
	\param oflags 
		\arg O_RDONLY 	Open for read access only.
		\arg O_RDWR 	Open for read or write access
		\arg O_CREAT
		\arg O_EXCL

	\note O_TRUNC, O_CREAT, O_EXCL могут идти в комбинации
	Upon successful completion, the shm_open( ) function shall return a non-negative integer
representing the file descriptor
 */
int    shm_open(const char *path, int oflags, mode_t mode){
	Device_t *dev = dtree_path(NULL, path, &path);
	if (dev==NULL) return -1;
	if (path!=NULL && (oflags & (O_CREAT))) {
		dev = dtree_mknodat(dev, path, mode, DEV_SHM);// разрешить доступ и назначить права
	}
	return device_flag_alloc(dev);
}
int    shm_unlink(const char * path){
	return unlink(path);
}
	//!\}