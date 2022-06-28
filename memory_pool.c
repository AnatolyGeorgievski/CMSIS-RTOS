#include "cmsis_os2.h"
#include <stdlib.h>
#include "atomic.h"
typedef struct _osMemoryPool osMemoryPool_t;
typedef struct _List List_t;
struct _List {
    List_t* next;
};
static inline void atomic_list_push(volatile void** top, List_t* elem)
{
	do {
		elem->next = atomic_pointer_get(top);
		atomic_mb();
	} while(!atomic_pointer_compare_and_exchange(top, elem->next, elem));
}
static inline void* atomic_list_pop(volatile void** top)
{
	List_t* ref;
	do {
		ref = atomic_pointer_get(top);
		if(ref==NULL) break;
	} while(!atomic_pointer_compare_and_exchange(top, ref, ref->next));
	return ref;
}



struct _osMemoryPool {
    volatile int count;// счетчик семафора
    List_t *pool;
//    uint16_t block_size;
};

osMemoryPoolId_t osMemoryPoolNew 	(uint32_t  	block_count, uint32_t  	block_size,	const osMemoryPoolAttr_t * attr)
{

    block_size = (block_size+3)>>2;// выравнивание >>2;
    osMemoryPool_t *mp = malloc(block_count*block_size + sizeof(osMemoryPool_t));
    mp->pool = (void*)mp + sizeof(osMemoryPool_t);

    List_t* elem = mp->pool;
    while (--block_count) {
        elem->next = (List_t*)((uint32_t*)elem + block_size);
        elem = elem->next;
    }
    elem->next = NULL;
    return mp;
}
void * osMemoryPoolAlloc(osMemoryPoolId_t mp_id, uint32_t timeout)
{
    osMemoryPool_t *mp = mp_id;
	return atomic_list_pop((volatile void**)&mp->pool);
}
osStatus_t osMemoryPoolFree(osMemoryPoolId_t mp_id, void* block)
{
    osMemoryPool_t *mp = mp_id;
	(void)atomic_list_push((volatile void**)&mp->pool, (List_t*)block);
	return osOK;
}
