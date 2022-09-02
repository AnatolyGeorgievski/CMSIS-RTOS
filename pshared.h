#ifndef _PSHARED_H_
#define _PSHARED_H_

#include <sys/types.h>

typedef struct _Device Device_t;
typedef struct _SharedClass SharedClass_t;
struct _SharedClass {
	const char* prefix;
	int type_id;// dev_id
	int size;
	int *unique_id;// уникальный идентификатор позволяет создавать устройства с префиксом.
};
// базовый интерфейс 
void  shared_object_mode(void* , mode_t);
void* shared_object_open(const char*, int, const SharedClass_t*);
int   shared_object_close(void*, const SharedClass_t*);
int   shared_object_unlink(const char*, const SharedClass_t*);
// выделение дескрипторов устройств
int device_flag_alloc(void* data);
void device_flag_free(int fildes);

extern Device_t* _devices[];
#define INO_BITS 22
#define INO_MASK ((1UL<<INO_BITS)-1)
#define OID(t,ino) (((uint32_t)(t)<<INO_BITS) | ((ino)&INO_MASK))
#define DEV_PTR(fildes) _devices[fildes & 0x1F]
#define SHM_PTR(fildes) (shm_t*)(_devices[fildes])
#define SHM_DES(shm) (int)(shm->oid)// переделать
#define DEV_ID(fd) ((fd)>>INO_BITS)// идентификатор типа устройства
// Известные иентификаторы
#define DEV_FIL         0
#define DEV_DIR         1
#define DEV_SEM         2 // semaphores 
#define DEV_SHM         3 // process shared memory
#define DEV_TYM         4 // posix typed memory
#define DEV_MSG         5 // posix message passing
#endif// _PSHARED_H_