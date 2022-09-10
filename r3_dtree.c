#include <unistd.h>
#include <sys/stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <atomic.h>
#include <r3_slice.h>

//#include "pshared.h"
typedef struct _Device Device_t;
typedef struct _DeviceClass DeviceClass_t;

typedef struct _dentry dentry_t;
struct _dentry {// общий для dirent.c
	const char* 	d_name;
	uint32_t  key;// ключ для сравнения
	dentry_t* next;
	Device_t* dev;
};

static Device_t _dentry_root = {.dev_id=DEV_DIR, .mode=0755};

// контекст процесса, специальный сегмент памяти:
Device_t *dentry_root=&_dentry_root;
Device_t *dentry_cwd =&_dentry_root;
mode_t _umask = 0777;//0777;
/*! \brief создает структуру директории */
void dtree_init(){
	Device_t* dev = dentry_root;
	dtree_mknodat(dentry_root, "dev", 0744, DEV_DIR);
	dtree_mknodat(dentry_root, "bin", 0755, DEV_DIR);
//	dev = dtree_mknodat(dentry_root, "tmp", 0755, DEV_DIR);
//	dtree_mount(dentry_root, "bin", dsk);
	// dev = dtree_mknodat(dev, "null", 0755, DEV_NUL);
	// dev = dtree_mknodat(dev, "console", 0755, DEV_TTY);
}

const DeviceClass_t *dev_classes[4]={NULL};

volatile int FILE_unique_id=0;
/*! Функция выбрана исходя из того, что используются маленькие буквы, 
	спец символы и цифры в кодировке ASCII или UTF8
 */
static uint32_t dtree_hash(const char* name, int len){
	uint32_t hash = 0;
	do {
		hash = ((hash<<6) | (hash>>26)) ^ (uint8_t)name[0];// ROR(hash, 26)
		name++;
	} while (name[0]!='\0' && --len);
	return __REV(hash);
}
static dentry_t* dentry_push(Device_t *parent, Device_t *dev, const char* name, int nlen)
{
	dentry_t *dentry;
	dentry = g_slice_alloc(sizeof(dentry_t));
	dentry->dev  = dev;
	dentry->d_name = name;
	dentry->key = dtree_hash(name, nlen);
#if 0 // если не нужна атомарность
	dentry->next= parent->phandle;
	parent->phandle = dentry;
#else
	void* node;
	volatile void** ptr = (volatile void**)(dentry_t**)(parent+1);
	do {
		dentry->next = node = atomic_pointer_get(ptr);
		atomic_mb();// fence
	} while (!atomic_pointer_compare_and_exchange(ptr, node, dentry));
#endif
	return dentry;
}
static inline int path_token(const char* name){
	int len=0;
	while (name[len]!='/' && name[len]!='\0') len++;
	return len;
}

/*! \defgroup _dtree Device Structured View (Directory)
	\brief Базовый интерфейс для поиска объектов
	\ingroup _system
	\{
 */
/*! \brief увеличивает число ссылок на объект.
	\return число ссылок на объект.
	Объект может быть в заблокированном состоянии, тогда возвращается значение меньше нуля.
 */
int dtree_nlink(Device_t * dev)
{
	int count;
	volatile int* ptr = (volatile int*)&dev->nlink;
	do {
		count = atomic_int_get(ptr);
	} while (!atomic_int_compare_and_exchange(ptr, count, count>-0? count+1: count));
	return count;
}
/*! \brief Устанавливает связь между элементами структуры
	\param dirp parent directory object
	Увеличивает число ссылок на объект 
 */
void dtree_insert(Device_t * dirp, const char* name, Device_t *dev)
{
	if (dirp->dev_id == DEV_DIR && dtree_nlink(dirp)>=0) {
		dentry_push(dirp, dev, name, 16);
		if (dev->dev_id == DEV_DIR) {
			dentry_push(dev, dev, 	 ".",  1);
			dentry_push(dev, dirp, "..", 2);
		}
	}
}
/*! \brief объект типа файл или директория, на который ссылается путь
	\return NULL или объект типа FILE или DIR, на который ссылается путь
 */
Device_t* dtree_path  (Device_t* fp, const char* path, const char**tail)
{
	if (path[0]=='/') fp = dentry_root, path++;
//	else
//	if (path[0]=='~') fp = dentry_home, path++;
	else
	if (fp==NULL)  fp = dentry_cwd;
	int nlen;
	while(fp!=NULL && (nlen=path_token(path))) {
		dentry_t * dentry = *(dentry_t **)(fp+1);
		uint32_t key = dtree_hash(path, nlen);// можно считать хеш внутри path_token
		while (dentry) {// поиск по списку
			if (dentry->key == key && strncmp(dentry->d_name, path, nlen) == 0){
				path += nlen;
				if (path[0]=='/') path++;
				fp = dentry->dev;
				break;
			}
			dentry = dentry->next;
		}
		if (dentry==NULL) 
			return NULL;
	}
	*tail = path;
	return fp;
}
int   dtree_unref (Device_t* dev){
	int count = atomic_fetch_sub(&dev->nlink, 1 );
	if (count == 0){
		const DeviceClass_t* dev_class = DEV_CLASS(dev);
		g_slice_free1(dev_class->alloc_size, dev);
	}
	return count;
}
static volatile int FILE_uid=0;
Device_t* dtree_mknodat(Device_t* fp, const char* name, mode_t mode, dev_t dev_id)
{
	const DeviceClass_t* dev_class = DEV_CLASS(fp);
	int alloc_size= dev_class->alloc_size;
	volatile int * unique_id = &FILE_uid;// dev_class->unique_id;
	Device_t* f;
	f = g_slice_alloc(sizeof(Device_t)+alloc_size);
	f->dev_id  = dev_id;
	f->ino  = atomic_fetch_add(unique_id, 1);
	f->mode = mode & _umask;
	f->uid = ~0; 
	f->gid = ~0;
	// назначить права uid и gid
	f->nlink = 0;
	dtree_insert(fp, name, f);	
	return f;
}
//!\}