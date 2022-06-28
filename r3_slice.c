#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "atomic.h"

/*!

Основаная идея реализации, мы не возвращаем выделенные блоки,
Мы храним один блок данных, его выделяем подряд из массива, единица измерения при выделении памяти 64-128-256-512 байт
Выделяем безвозвратно.

освободивщиеся блоки ставим в очередь с атомарной выборкой
*/
typedef struct _List List_t;
struct _List {
	List_t *next;
};
/*! добавить элемент в стек с вершины. Тут есть один прикол из любого фрагмента памяти размером более 4 байт можно
сделать указатьель List_t, при освобождении памяти все слайсы - элементы списка. Память на слайсы не расходуется. */
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


static inline int bitn_mask_idx(size_t size)
{
    return 8*sizeof(uint32_t)-__builtin_clz((size-1)>>2);
}
static inline uint32_t bitn_first_zero(uint32_t word) {
    return __builtin_ctz(~word);
}

#define SLICE_POOL_NUM 5 // размеры 2 и 4 слова.
// минимальное выравнивание на должно быть на sizeof(void*)
#define SLICE_BLOCK_ALIGN 8
#define SLICE_SIZE(idx)         (  1<<(idx+2))
#define SLICE_BLOCK_SIZE(idx)   ((sizeof(uint32_t)*32)<<idx)
#define SLICE_OFFSET(pool_idx, idx)       (idx<<(pool_idx+2))
typedef struct _SlicePool SlicePool;
struct _SlicePool {
	volatile void* free_list;//!< стек для свободных слайсов
    volatile int count;
};
static SlicePool slice_pools[SLICE_POOL_NUM] = {{NULL}};


void g_slice_free1(size_t size, void* data)
{
    int pool_idx = bitn_mask_idx(size);
	SlicePool* pool = &slice_pools[pool_idx];
	atomic_list_push(&pool->free_list, (List_t*)data);
#if 0
	int value = atomic_fetch_sub(&pool->count,1);
	printf("SLICE: free %d-%d (%p)\n", pool_idx, value, data);
#endif // 1
}
// Изначально слайсы можно добывать в любых местах, в том числе на стеке
void g_slice_init(SlicePool* pool, size_t size, void* array, size_t length)
{	// нарезать и затолкать в стек
	List_t* top = array;
	List_t* elem;
	do {
		elem = array;
		array+=size;
		elem->next = array;
	} while (--length);// -1?
	do {
		elem->next = atomic_pointer_get(&pool->free_list);
		atomic_mb();
	} while(!atomic_pointer_compare_and_exchange(&pool->free_list,elem->next, top));
}
void* g_slice_alloc(size_t size)
{
    // каким количеством бит описывается размер
    int pool_idx = bitn_mask_idx(size);
    SlicePool* pool = &slice_pools[pool_idx];
	void* data = NULL;
	do {
		data = atomic_list_pop(&pool->free_list);

		if (data!=NULL) {
#if 0
            int value = atomic_fetch_add(&pool->count,1);
		    printf("SLICE: Pop %d-%d (%p)\n", pool_idx, value+1, data);
#endif
            return data;
		}
		// взять в системе блок и нарезать его, заталкать в стек пустых
		// можно запретить выделять слайсы
		int qty = 32>>pool_idx;
		void* mem_block = malloc(32*4);// 128b
#ifdef __WIN32
		if (1) {
			static uint32_t total_size=0;
            total_size +=32*4;
		printf("SLICE: Get mem block %d (%dB)\n", pool_idx, total_size);
		}

		if (mem_block==NULL) _Exit(10);
#endif // __win__
		g_slice_init (pool, 4<<pool_idx, mem_block, qty);
	} while(1);
	return NULL;
}

