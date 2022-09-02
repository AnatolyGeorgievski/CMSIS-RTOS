/* pshared */
#include <unistd.h>
#include <fcntl.h>
#include "atomic.h"
#include "pshared.h"
#include "r3_slice.h"

typedef struct _node dtree_t;
extern struct _node *device_tree;
extern int   dev_tree_insert(dtree_t **, uint32_t, void*);
extern void* dev_tree_open (dtree_t *, uint32_t);
extern void* dev_tree_unref(dtree_t *, uint32_t);

// шапка объекта \sa Device_t
typedef struct _shm  Shared_t;
struct _shm {
	uint32_t oid;
	mode_t mode;
};
#define MASK(len) ((~0UL)>>(4 - len%4))
static inline int has_prefix(const char* name, const char* prefix, size_t len){
	return (*(uint32_t*)name ^ *(uint32_t*)prefix) & MASK(len);
}

void* shared_object_open (const char *path, int oflag, const SharedClass_t* sh_class)
{
	if (!has_prefix(path, sh_class->prefix, 3)) return NULL;
	ino_t ino_id = atoi(path+2);
	Shared_t* sh;
	if (oflag & O_CREAT) {
		sh = g_slice_alloc(sizeof(Shared_t)+sh_class->size);// shared memory
		ino_id = atomic_fetch_add(sh_class->unique_id, 1);
		sh->oid = OID(sh_class->type_id, ino_id);
		sh->mode= 0;// Надо создавать так чтобы объект был заблокирован пока не настроен
		//sh->refcount = 0; -- свойство дерева
		dev_tree_insert(&device_tree, sh->oid, sh);
	} else 
		sh = dev_tree_open(device_tree, sh->oid);
	
	return (void*)(sh +1);
}
void shared_object_mode (void* data, mode_t mode){
	Shared_t* shm = (Shared_t*)data -1;
	shm->mode = mode;
}
static int shared_object_unref(uint32_t oid)
{
	Shared_t* shm = dev_tree_unref(device_tree, shm->oid);
	if (shm) {// удалить объект
		
	}
	return 0;
}
int shared_object_close (void* data, const SharedClass_t* sh_class){
	Shared_t* shm = (Shared_t*)data -1;
	return shared_object_unref(shm->oid);
}
int shared_object_unlink(const char *path, const SharedClass_t* sh_class)
{
	if (!has_prefix(path, sh_class->prefix,3)) return -1;
	ino_t ino_id = atoi(path+2);
	uint32_t oid = OID(sh_class->type_id, ino_id);
	return shared_object_unref(oid);
}