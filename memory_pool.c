#include "cmsis_os2.h"
#include "semaphore.h"
#include "atomic.h"
#include <stdlib.h>
typedef struct _osMemoryPool osMemoryPool_t;
typedef struct _List List_t;
struct _List {
    List_t* next;
};
struct _osMemoryPool {
    volatile int count;// счетчик семафора
    List_t *pool;// Свободные блоки памяти заложены в стек.
//    osMemoryPoolAttr_t* attr;
//    uint16_t block_size;
};
/*! \brief атомарно пихает в стек */
static inline void atomic_list_push(volatile void** top, List_t* elem)
{
	do {
		elem->next = atomic_pointer_get(top);
		atomic_mb();
	} while(!atomic_pointer_compare_and_exchange(top, elem->next, elem));
}
/*! \brief атомарно выталкивает со стека */
static inline void* atomic_list_pop(volatile void** top)
{
	List_t* ref;
	do {
		ref = atomic_pointer_get(top);
		if(ref==NULL){
			atomic_free();
			break;
		}
	} while(!atomic_pointer_compare_and_exchange(top, ref, ref->next));
	return ref;
}


osMemoryPoolId_t osMemoryPoolNew 	(uint32_t  	block_count, uint32_t  	block_size,	const osMemoryPoolAttr_t * attr)
{

    block_size = (block_size+3)>>2;// выравнивание >>2;
    osMemoryPool_t *mp=NULL;
    if (attr!=NULL) {
        mp = attr->cb_mem;
    }
    if (mp==NULL){
		mp = malloc(sizeof(osMemoryPool_t));
        mp->pool = NULL;
    }
    if (attr!=NULL)
        mp->pool = attr->mp_mem;
    if (mp->pool == NULL){
		mp->pool = calloc(block_count, block_size<<2);
    }
//	mp->block_size = block_size<<2;
//	mp->attr = attr;

	semaphore_init(&mp->count, block_count);

    List_t* elem = mp->pool;
    while (--block_count) {// запихать в стек все блоки
        elem->next = (List_t*)((uint32_t*)elem + block_size);
        elem = elem->next;
    }
    elem->next = NULL;
    return mp;
}
void * osMemoryPoolAlloc(osMemoryPoolId_t mp_id, uint32_t timeout)
{
    osMemoryPool_t *mp = mp_id;
    int count = semaphore_enter(&mp->count);
    if (count==0) {
		osEvent_t event = {.status = osEventSemaphore, .value={.p = (void*)&mp->count}};
		osEventWait(&event, timeout);
		if (event.status != osEventSemaphore){
			return NULL;
		}
	}
	return atomic_list_pop((volatile void**)&mp->pool);
}
osStatus_t osMemoryPoolFree(osMemoryPoolId_t mp_id, void* block)
{
    osMemoryPool_t *mp = mp_id;
	atomic_list_push((volatile void**)&mp->pool, (List_t*)block);
	semaphore_leave (&mp->count);
	return osOK;
}
