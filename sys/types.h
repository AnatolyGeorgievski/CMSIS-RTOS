#ifndef SYS_TYPES_H
#define SYS_TYPES_H
#include <sys/_types.h>
typedef void* timer_t;// POSIX Timer

typedef unsigned short int gid_t;
typedef unsigned short int uid_t;
// Мы используем уникальный идентификатор объекта OID (dev_t:10 | ino_t:22)
typedef unsigned short int dev_t;// идентификатор типа(класса) устройства
typedef   int ino_t;// идентификатор порядковый номер объекта 
typedef unsigned short int mode_t;
typedef volatile int nlink_t;// число ссылок на объект
typedef int blksize_t;// размер блока на устройстве
typedef int blkcnt_t;// число блоков занятых объектом
typedef struct _fpos fpos_t;// сохраняет контекст для IO stream, включая разбор utf8 mbstate_t
typedef struct _mbstate mbstate_t;
#endif
