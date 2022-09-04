#include <unistd.h>
#include <sys/stdio.h>
#include <fcntl.h>
#include <string.h>
#include <atomic.h>
#include <r3_slice.h>

#include "pshared.h"

typedef struct _dentry dentry_t;
struct _dentry {
	uint32_t  key;// ключ для сравнения
	dentry_t* next;
	FILE* dev;
	const char*d_name;
};
// контекст процесса, специальный сегмент памяти:
FILE *dentry_root=NULL;
FILE *dentry_dev =NULL;// device tree
FILE *dentry_cwd =NULL;
mode_t umask = ~0;//0777;

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
static void dtree_insert(FILE * parent, uint32_t key, const char* name, FILE *dev)
{
	dentry_t *dentry = g_slice_alloc(sizeof(dentry_t));
	dentry->dev  = dev;
	dentry->d_name = name;
	dentry->key = key;
	void* node;
	volatile void** ptr = (volatile void**)&parent->phandle;
	do {
		dentry->next = node = atomic_pointer_get(ptr);
		atomic_mb();// fence
	} while (!atomic_pointer_compare_and_exchange(ptr, node, dentry));
}
static int path_token(const char* name){
	int len=0;
	while (name[len]!='/' && name[len]!='\0') len++;
	return len;
}
/*! \brief объект типа файл или директория, на который ссылается путь
	\return NULL или объект типа FILE или DIR, на который ссылается путь
 */
FILE* dtree_path  (FILE* fp, const char* path, const char**tail)
{
	if (path[0]=='/') fp = dentry_root, path++;
	if (fp==NULL)  fp = dentry_cwd;
	int nlen;
	while(fp!=NULL && (nlen=path_token(path))) {
		dentry_t * dentry = fp->phandle;
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
FILE* dtree_openat(FILE* fp, const char* name, int oflag, mode_t mode){
	FILE* f = NULL;
	if (name!=NULL && (oflag&O_CREAT) && (fp->dev == DEV_DIR)) {
		f = g_slice_alloc(sizeof(FILE));
		f->dev  = DEV_FIL;
		f->ino  = atomic_fetch_add(&FILE_unique_id, 1);
		f->mode = mode & umask;
		f->nlink = 0;
		uint32_t key = dtree_hash(name, 256);// ключик для хеширования
		dtree_insert(fp, key, name, f);
	}
	return f;
}
int   dtree_unref (FILE* f){
	int count = atomic_fetch_sub(&f->nlink, 1);
	if (count == 0)
		g_slice_free1(sizeof(FILE), f);
	return count;
}
