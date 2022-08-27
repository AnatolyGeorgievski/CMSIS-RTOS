/* POSIX_DEVICE_IO: Device Input and Output */
#include <atomic.h>
//#include <fcntl.h>// O_CREAT
#include <signal.h>
#include <unistd.h>
#include <r3_slice.h>
#include <r3_tree.h>

#define O_CREAT 1// 

static inline void device_flag_free (uint32_t *flags, int fd){
	flags[0] &= ~(1UL<<fd);
}
static inline int device_flag_alloc(uint32_t *flags){
	if (flags[0]==~0) return -1;
	int fd = __builtin_ctz(~flags[0]);
	flags[0] |= 1UL<<fd;
	return fd;
}
static inline int has_prefix(const char* name, const char* prefix){
	return *(uint32_t*)name == *(uint32_t*)prefix;
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

typedef struct _DeviceClass {
	struct _DeviceClass* next;
	void* 	phandle;
	volatile int* unique_id;
	const uint32_t type_id;
	const char* prefix;
	void* 	(*open) (uint32_t, int);
	ssize_t	(*read) (void*, void*, size_t);
	ssize_t	(*write)(void*, void*, size_t);
	void* 	(*close)(void*);
} DeviceClass_t;
typedef struct {
	void* 	phandle;
	uint32_t oid;
	volatile int refcount;
	const DeviceClass_t* dev_class;
} Device_t;
static DeviceClass_t * device_classes=NULL;

/*! \brief 

при небольшом количестве устройств нет смысла заводить хеш табицу
*/
static const DeviceClass_t * device_class_get(const char* name, char** tail){
	DeviceClass_t * drv = device_classes;
	while (drv) {
		if (has_prefix(name, drv->prefix)) break;
		drv = drv->next;
	}
	return drv;
}
/* Дескирипторы файлов сделал глобальными, их можно перенсти в треды и наследовать */
static uint32_t device_flags[1]= {0};// per thread?
static Device_t* _devices[1];// per thread?
static tree_t * device_tree=NULL;
#define OID(t, n) (((t)<<22)|(n))
#define ERR(e) (-1)
int 	open(const char *name, int oflags, ...)
{
	int fd = device_flag_alloc(device_flags);
	if (fd<0) return -1;
	if (!has_prefix(name, "/dev")) return -1;
	const DeviceClass_t *dev_class = device_class_get(name+4, NULL);
	if (dev_class == NULL) return -1;
	uint32_t dev_id = a2bcd(name+8, NULL);// 0 - undefined
	uint32_t oid = OID(dev_class->type_id, dev_id);
	Device_t * dev = tree_lookup(device_tree, oid);
	if (dev==NULL) {
		if (oflags & (O_CREAT)) {
			dev_id = atomic_fetch_add(dev_class->unique_id, 1);// ссылка на переменную unique_id
			oid = OID(dev_class->type_id, dev_id);
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
		tree_t* node = g_slice_alloc(sizeof(tree_t));// shared memory
		if (node==NULL) return ERR(ENOMEM);
		tree_init(node, oid, dev);
		node = tree_insert_tree (&device_tree, node);
		if (node) g_slice_free(tree_t, node);// такого тоже не должно быть
	} else {// объект существует
		int count = atomic_fetch_add(&dev->refcount, 1);// проверить? может tree_lookup само будет считать ссылки
		if (count<=0) return -1;
		if (dev_class->open) dev->phandle = dev_class->open(oid, oflags);
		else dev->phandle = dev_class->phandle;
	}
	return fd;
}
ssize_t read(int fildes, void *buf, size_t nbyte){
	Device_t * dev = _devices[fildes];
	return dev->dev_class->read(dev->phandle, buf, nbyte);
}
ssize_t write(int fildes, void *buf, size_t nbyte){
	Device_t * dev = _devices[fildes];
	return dev->dev_class->write(dev->phandle, buf, nbyte);
}
int 	close(int fd)
{
	Device_t * dev = _devices[fd];
	_devices[fd] = NULL;
	device_flag_free(device_flags, fd);
	const DeviceClass_t *dev_class = dev->dev_class;
	if (dev_class->close) dev_class->close(dev->phandle);
	
	int count = atomic_fetch_sub(&dev->refcount, 1);
	if (count==0) {
		dev = tree_replace_data(device_tree, dev->oid, NULL);
		g_slice_free(Device_t, dev);
	}
}
int 	unlink(const char *name)
{
	if (!has_prefix(name, "/dev")) return -1;
	const DeviceClass_t *dev_class = device_class_get(name+4, NULL);
	if (dev_class == NULL) return -1;
	uint32_t dev_id = a2bcd(name+8, NULL);// 0 - undefined
	uint32_t oid = OID(dev_class->type_id, dev_id);
	Device_t * dev = tree_lookup(device_tree, oid);
	if (dev==NULL) return -1;
	int count = atomic_fetch_sub(&dev->refcount, 1);
	if (count==0) {
		dev = tree_replace_data(device_tree, oid, NULL);
		g_slice_free(Device_t, dev);
	}
	return 0;
}
#endif// _POSIX_DEVICE_IO