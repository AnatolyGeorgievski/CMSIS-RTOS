/* POSIX_DEVICE_IO: Device Input and Output */
#include <atomic.h>
//#include <fcntl.h>// O_CREAT
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <r3_slice.h>
#include <r3_tree.h>
#include <atomic.h>

#include "pshared.h"


typedef struct _node dtree_t;
struct _node {
	uint32_t key;
	volatile int refcount;
	void * data;
	dtree_t *next;
	dtree_t *prev;
};
struct _node * device_tree=NULL;

void* dev_tree_open(dtree_t *node, uint32_t key)
{
    while(node) {
		int32_t cmp = key - node->key;
		if(cmp==0) {
			int refcount;
			void* data;
			volatile int * ptr = &node->refcount;
			do {
				refcount = atomic_int_get(ptr);
				if (refcount<0) {
					data = NULL;
					break;
				} else
					data = (volatile void**)(&node->data);
			} while(!atomic_int_compare_and_exchange(ptr, refcount, refcount+1));
			return data;
		}
        node = (cmp < 0)? node->prev: node->next;
    }
    return NULL;
}
/* облегченный вариант дерева, удаление происходит снаружи 
	\return NULL или ссылка на объект
*/
void* dev_tree_unref(dtree_t *node, uint32_t key)
{
    while(node) {
		int32_t cmp = key - node->key;
		if(cmp==0) {
			int count = atomic_fetch_sub(&node->refcount,1);
			return (count == 0)? atomic_exchange(&node->data, NULL) : NULL;
		}
        node = (cmp < 0)? node->prev: node->next;
    }
    return NULL;
}
int dev_tree_insert(dtree_t** prev, uint32_t key, void* data)
{
	dtree_t *elem = g_slice_alloc(sizeof(dtree_t));
	dtree_t *node;
	do {
		while((node=atomic_pointer_get((volatile void**)prev))!=NULL) {
			const int32_t cmp = key - node->key;
			if (cmp==0){ // добавляем без изменения структуры дерева методом замены значения
				//data = atomic_exchange(&node->value, elem->value);
				atomic_free();
				return -1;// не является уникальным
			}
			prev = (cmp<0)? &node->prev : &node->next;
		}
	} while(!atomic_pointer_compare_and_exchange(prev, node, elem));
	return 0;
}


typedef struct _shm {
	uint32_t oid;	
	void* 	phandle;
	mode_t mode;
} shm_t;

static inline void _flag_free (uint32_t *flags, int fd){
	flags[0] &= ~(1UL<<fd);
}
static inline int _flag_alloc(uint32_t *flags){
	if (flags[0]==~0) return -1;
	int fd = __builtin_ctz(~flags[0]);
	flags[0] |= 1UL<<fd;
	return fd;
}
static inline void atomic_flag_free (uint32_t *flags, int fd){
	atomic_fetch_and(&flags[fd/32], ~(1UL<<(fd%32)));
}
static inline int atomic_flag_alloc(uint32_t *flags){
	int fildes = 0;
	uint32_t map;
	volatile int *ptr = (volatile int *)flags;
	do {
		map = atomic_int_get(ptr);
		if (map==~0U) return -1;
		fildes = __builtin_ctz(~map);
	} while(!atomic_int_compare_and_exchange(ptr, map, map | (1U<<fildes)));
	return fildes;
}

#define MASK(len) ((~0UL)>>(4 - len%4))
static inline int has_prefix(const char* name, const char* prefix, size_t len){
	return (*(uint32_t*)name ^ *(uint32_t*)prefix) & MASK(len);
}
static inline int a2bcd(const char* s, char **tail){
	int n = 0;
	while ('0' <= s[0] && s[0] <= '9')
		n = (n<<4) + (*s++ - '0');
	if (tail)*tail = (char *)s;
	return n;
}

#if 1 // subprofile POSIX_DEVICE_IO
/*
POSIX_DEVICE_IO: Device Input and Output
FD_CLR(+), FD_ISSET(+), FD_SET(+), FD_ZERO(+), clearerr( ), close(+), fclose( ), fdopen( ), feof( ),
ferror( ), fflush( ), fgetc( ), fgets( ), fileno( ), fopen( ), fprintf( ), fputc( ), fputs( ), fread( ), freopen( ),
fscanf( ), fwrite( ), getc( ), getchar( ), gets( ), open(+), perror( ), poll( ), printf(+), pread( ), pselect(+),
putc( ), putchar( ), puts(+), pwrite( ), read(+), scanf( ), select( ), setbuf( ), setvbuf( ), stderr, stdin,
stdout, ungetc( ), vfprintf( ), vfscanf( ), vprintf( ), vscanf( ), write(+)
*/
/* POSIX_FILE_SYSTEM: File System
access( ), chdir( ), closedir( ), creat(+), fchdir( ), fpathconf( ), fstat( ), fstatvfs( ), getcwd( ), link( ),
mkdir( ), mkstemp( ), opendir( ), pathconf( ), readdir( ), remove( ), rename( ), rewinddir( ), rmdir( ),
stat( ), statvfs( ), tmpfile( ), tmpnam( ), truncate( ), unlink(+), utime( )
*/
typedef struct _DeviceClass DeviceClass_t;
struct _DeviceClass {
	struct _DeviceClass* next;
	void* 	phandle;
	volatile int* unique_id;
	const dev_t dev_id;// dev_t
	const char* prefix;
	void* 	(*open) (uint32_t, int);
	ssize_t	(*read) (void*, void*, size_t);
	ssize_t	(*write)(void*, void*, size_t);
	void* 	(*close)(void*);
};
typedef struct _Device Device_t;
struct _Device {
	uint32_t oid;
	void* 	phandle;
	mode_t mode;
	volatile int refcount;
	const DeviceClass_t* dev_class;
};
static DeviceClass_t * device_classes=NULL;
#if 0
struct _DeviceClass SYMLINK_class;
struct _DeviceClass FIFO_class;
struct _DeviceClass DIR_class = {
	.open = d_open,
};
struct _DeviceClass FILE_class = {
	.open = f_open,
	.read = f_read,
	.write= f_write,
	.close= f_close,
};
/* каждый файл(коллекция или устройсво) имеет два номера dev_t и ino_t вместе они образуют oid
В системе каждый файл имеет уникальный идентификатор
*/
#endif

/*! \brief 

при небольшом количестве устройств нет смысла заводить хеш табицу
*/
static const DeviceClass_t * device_class_get(const char* name, char** tail){
	DeviceClass_t * drv = device_classes;
	while (drv) {
		if (has_prefix(name, drv->prefix,3)) break;
		drv = drv->next;
	}
	return drv;
}
static const DeviceClass_t * device_class_id(dev_t drv_id){
	DeviceClass_t * drv = device_classes;
	while (drv) {
		if (drv_id == drv->dev_id) break;
		drv = drv->next;
	}
	return drv;
}
/* Дескирипторы файлов сделал глобальными, их можно перенсти в треды и наследовать */
static uint32_t device_flags[1]= {0};// per thread?
Device_t* _devices[32];// per thread?
//static dtree_t * device_tree=NULL;

#define ERR(e) (-1)
mode_t umask(mode_t cmask);
int device_flag_alloc(void* data){
	
	int fildes = atomic_flag_alloc(device_flags);
	if (fildes>=0)
		_devices[fildes] = data;
	return fildes;
}
void device_flag_free(int fildes){
	_devices[fildes] = NULL;
	atomic_flag_free(device_flags, fildes);
}
int mknod (const char *path, mode_t mode, dev_t dev_id)
{

	const DeviceClass_t *dev_class = device_class_id(dev_id);
	if (dev_class==NULL) return -1;
	Device_t * dev = g_slice_alloc(sizeof(Device_t));// shared memory
	ino_t ino_id = atomic_fetch_add(dev_class->unique_id, 1);
	uint32_t oid = OID(dev_class->dev_id, ino_id);
	dev->oid = oid;
	dev->dev_class = dev_class;
	dev->refcount = 0;
	dev->mode = mode;
	
	dev_tree_insert(&device_tree, dev->oid, dev);

	return device_flag_alloc(dev);
}
int mkdir (const char *, mode_t);
int mkfifo(const char *, mode_t);
//int chmod(const char *path, mode_t mode);
int fchmod(int fildes, mode_t mode) {
	Device_t* dev = DEV_PTR(fildes);
	dev->mode = mode;
	return 0;
}
int creat (const char *path, mode_t mode) {
    return open(path, O_WRONLY|O_CREAT|O_TRUNC, mode);
}
/*! \brief синхронизация */
int fsync(int fildes){// если операция не завершена, ждать завершения
	return 0;
}

int open(const char *name, int oflags, ...)
{
	if (!has_prefix(name, "/dev", 4)) return -1;
	const DeviceClass_t *dev_class = device_class_get(name+4, NULL);
	if (dev_class == NULL) return -1;
	ino_t ino_id = a2bcd(name+8, NULL);// 0 - undefined
	uint32_t oid = OID(dev_class->dev_id, ino_id);
	Device_t * dev = dev_tree_open(device_tree, oid);
	if (dev==NULL) {
		if (oflags & (O_CREAT)) {
			ino_id = atomic_fetch_add(dev_class->unique_id, 1);// ссылка на переменную unique_id
			oid = OID(dev_class->dev_id, ino_id);
			dev = g_slice_alloc(sizeof(Device_t));// shared memory
			if (dev==NULL) return -1;
			dev->refcount = 0;
			dev->oid = oid;
			dev->dev_class= dev_class;
			if (dev_class->open) 
				dev->phandle = dev_class->open(oid, oflags);
			else 
				dev->phandle = dev_class->phandle;
		} else 
			return -1;
		dev_tree_insert(&device_tree, oid, dev);
	} else {// объект существует
		if (dev_class->open) dev->phandle = dev_class->open(oid, oflags);
		else dev->phandle = dev_class->phandle;
	}
	return device_flag_alloc(dev);
}
ssize_t read(int fildes, void *buf, size_t nbyte){
	Device_t * dev = DEV_PTR(fildes);//_devices[fildes];
	return dev->dev_class->read(dev->phandle, buf, nbyte);
}
ssize_t write(int fildes, void *buf, size_t nbyte){
	Device_t * dev = DEV_PTR(fildes);//_devices[fildes];
	return dev->dev_class->write(dev->phandle, buf, nbyte);
}
int 	close(int fildes)
{
	Device_t * dev = _devices[fildes];
	device_flag_free(fildes);

	dev = dev_tree_unref(device_tree, dev->oid);
	if (dev!=NULL) {
		const DeviceClass_t *dev_class = dev->dev_class;
		if (dev_class->close) dev_class->close(dev->phandle);
		g_slice_free(Device_t, dev);// может другой размер?
	}
}
// предопределенные идентификаторы типа устройств
int 	unlink(const char *name)
{
	const DeviceClass_t *dev_class = NULL;
	Device_t * dev = NULL;
	if (has_prefix(name, "/dev", 4)) {// null tty console random shm sem dsk msg i2c gpio stdio stdout stderr
		dev_class = device_class_get(name+4, NULL);
		if (dev_class == NULL) return -1;
		ino_t ino_id = a2bcd(name+8, NULL);// 0 - undefined, -1 undefined
		uint32_t oid = OID(dev_class->dev_id, ino_id);
		dev = dev_tree_open(device_tree, oid);
		if (dev==NULL) return -1;
	} else 
	//if (has_prefix(name, "/")) 
	{
#if 0
typedef struct _Entry Entry_t;
struct _Entry {// надо чтобы совпадало со способом хранения записей в файловой системе
	uint32_t oid;
	char * name;
};
struct _Dir {
	struct _Dir *parent;
	void* htable;
};
		dev = dev_open(path);

		DIR_t* dir = (name[0]=='/')?root: cwd;//curret working directory '~' user '.'
		switch (name[0]) {
		case '/': dir = root; name++; break;// root
		case '.': dir = cwd;  name++; break;// current working directory
		case '~': dir = home; name++; break;// user home
		default:  dir = cwd; // относительный путь
			break;
		}
		while (tail!='\0' && (name = strtok_r(NULL, "/", &tail))!=NULL) {// коллекция типа DIR
			Entry_t *entry = dir_lookup(dir, name);// заменить на итератор в дереве или хеш таблицу
			if (entry == NULL) return -1;
			if (DEV_ID(entry->oid) == DEV_DIR && tail!='\0')
				dir = tree_lookup(device_tree, entry->oid);// можно сузить поиск если хранить отдельно точку входа DIR
			else if (tail == '\0') {
				dev = tree_lookup(device_tree, entry->oid);
				break;
			} else {
				return -1;
			}
		}
#endif
		if (dev==NULL) return -1;
		dev_class = dev->dev_class;// Коллекция или файл. 
		if (dev_class == NULL) return -1;
	}
	dev = dev_tree_unref(device_tree, dev->oid);
	if (dev!=NULL) {
		g_slice_free(Device_t, dev);// может другой размер?
	}
	return 0;
}
#endif// _POSIX_DEVICE_IO