#include <unistd.h>
//long sysconf(int name); // значение системной переменной по идентификатору _SC_PAGESIZE
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "pshared.h"

typedef struct _shm {
	uint32_t oid;	
	void* 	phandle;
	mode_t mode;
} shm_t;

#if defined(_POSIX_MAPPED_FILES) && (_POSIX_MAPPED_FILES>0)
/*! \brief преобразует идентификатор объекта в указатель в памяти процесса
    \param len длина сегмента данных
    \param fildes идентификатор объекта
	\return адрес в памяти процесса (pa)
	
Чтобы было проще понять явление, надо представить что addr - некоторый адрес в памяти сложно доступной, 
например во флеш, QSPI.
Открытие файлов с загрузкой содержимого выглядит таким побразом

fd = open(path, O_RDONLY);
buff = NULL;
buff = mmap(buf, len, PROT_READ, MAP_FIXED,  fd, offset);
-- если асинхронный доступ, то вероятно надо еще подожать дескриптор методом 
-- доступ к файлам может быть по блокам
poll или pselect. 
munmap(buf, len) -- освобождает буфер, если он выделен понятным способом
 */
void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off) {
	void* pa = NULL;
	void* dev = _devices[fildes & INO_MASK];
	switch (DEV_ID(fildes)){
	case DEV_FIL:
		// если файл открыт на запись
		if (prot & PROT_WRITE) {
			// записать данные во флеш, не блокирующий вызов.
		} else {
			//pa = shm->phandle + off;// чтение напрямую из внутренней флеш памяти
		}
		break;
	case DEV_SHM:
		//pa = shm->phandle + off;// располагается в SHARED сегменте. 
		break;
	default: break;
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

#if defined(FLASH_BASE)
	if (SEGMENT_ID(addr)==FLASH_BASE) {
		
	} else
#endif
#if defined(SRAM_BASE)
	if (SEGMENT_ID(addr)==SRAM_BASE) {// 
		free()
	} else 
#endif
	{}
	return 0;
}
#endif //_POSIX_MAPPED_FILES

static int tym_unique_id=0;
static const SharedClass_t tym_class = 
	{"shm", DEV_TYM, sizeof(shm_t), &tym_unique_id};
// возвращает дескриптор файла (номер) и тип памяти
int posix_typed_mem_open(const char *name, int oflag, int tflag)
{
	void *tym = shared_object_open(name, oflag, &tym_class);
	return device_flag_alloc(tym);
}
static int shm_unique_id=0;
static const SharedClass_t shm_class = 
	{"shm", DEV_SHM, sizeof(shm_t), &shm_unique_id};
/*! 
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
	void *shm = shared_object_open(path, oflags, &shm_class);// рождается заблокированным и незримым
	if (shm!=NULL && (oflags & (O_CREAT))) {
		shared_object_mode(shm, mode);// разрешить доступ и назначить права
	}
	return device_flag_alloc(shm);
}
int    shm_unlink(const char * path){
	return shared_object_unlink(path, &shm_class);
}
