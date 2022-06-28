#include <cmsis_os.h>
#include <stdlib.h>
#include "atomic.h"

//  ==== Memory Pool Management Functions ====
#if (defined(osFeature_Pool) && (osFeature_Pool != 0))

/*! \ingroup _system
    \defgroup _memory_pool Memory Pool Management Functions

    В данной реализации мы ориентируемся на блоки размером 2^n
	Предлагается использование статического механизма распределения памяти под пулы

	\{
 */
#define OS_POOL_MAP_BITS (sizeof(unsigned int)*8)
#define OS_POOL_MAP_MASK (OS_POOL_MAP_BITS-1)
struct os_pool_cb {
//    volatile int count; // число элементов, семафор
//	const osPoolDef_t * def;
	void* base;//!< адрес пула, блока памяти
	uint16_t min;	//!< индекс младшей свободнй карты памяти, используется для оптимизации поиска
	uint16_t maps;	//!< число карт памяти
	uint16_t size;	//!< число элементов в пуле
	uint16_t item_aligned_size;//!< размер карты в байтах с выравниванием
	volatile unsigned int map[0];
};

typedef struct os_pool_cb osPool_t;
/*! \brief Создать пул памяти

    \param [in] pool_def статическое описание пула
    \return идентификатор или указатель на пул
 */
osPoolId osPoolCreate (const osPoolDef_t *pool_def)
{
	int count = (pool_def->pool_sz+OS_POOL_MAP_MASK)/OS_POOL_MAP_BITS;
    osPoolId pool;
    if (pool_def->map) {
        pool = pool_def->map;//
    } else {
        pool = malloc(sizeof(struct os_pool_cb)+count*sizeof(unsigned int));
    }
	pool->min  = 0;
	pool->size = pool_def->pool_sz;
	pool->item_aligned_size = (pool_def->item_sz + sizeof(unsigned int)-1)/sizeof(unsigned int);
	pool->maps = count;
	int i; for (i=0;i<count;i++) pool->map[i]=0;
	
    if (pool_def->pool!=NULL) {
        pool->base = pool_def->pool;// адрес определен статически
    } else {
        pool->base = malloc(pool->item_aligned_size * pool->size);
	}
    //printf("Pool %08X %08X %08X\n", (uint32_t)pool, (uint32_t)pool->pool, (uint32_t)(pool_def->pool_sz<<size));

	return pool;
}
void *osPoolAlloc (osPoolId pool)
{
	uint16_t i;
	for (i=pool->min;i<pool->maps; i++) {
		unsigned int mask  = atomic_load(&pool->map[i]);
		while (~mask) {
			int idx = __builtin_ctz(~mask);
			mask = atomic_fetch_or(&pool->map[i], (1UL<<idx));
            if ((mask & (1UL<<idx)) ==0 ) {// была коллизия, бит уже устанвлен
				return pool->base + (idx + i*OS_POOL_MAP_BITS)* pool->item_aligned_size;
			}
		}
	}
#if 0
	const osPoolDef_t *pool_def  = pool->def;
	for (i=pool->min;i<pool->maps; i++){
        volatile int *ptr = (volatile int *)&pool->map[i];
		unsigned int mask  = *ptr;
		while (~mask!= 0) {// есть свободный элемент
			int idx = __builtin_ctz(~mask);
			unsigned int set = (1UL<<idx);
            mask = atomic_fetch_or(ptr, set);// установить бит атомарно, если не удается повторить операцию.
            if ((mask & set) ==0 ) {// была коллизия, бит уже устанвлен
                //unsigned int size  = OS_POOL_MAP_BITS -__builtin_clz(pool_def->item_sz-1);//(pool_def->item_sz + 3) & ~3;// выравнивание данных на слово
//                printf("PoolAlloc %08X %08X %d %d\n", (uint32_t)pool->pool, (mask|set), idx, pool_def->item_sz);
				idx += i*OS_POOL_MAP_BITS;
                return pool->base + idx*pool->item_aligned_size;
            }
		}
	}
#endif
	return NULL;
}
void *osPoolCAlloc (osPoolId pool)
{
	void* ptr = osPoolAlloc(pool);
	if (ptr!=NULL) {
		__builtin_bzero(ptr, pool->item_aligned_size);
	}
	return ptr;
}
#if 0
void osPoolDebug(osPoolId pool)
{
    printf("osPoolDebug:\n");
    int i; 
	for (i=0; i< pool->maps; i++){
        printf(" %08X", pool->map[i]);
    }
    printf("\n");
}
#endif
/*!	\brief освободить элемент пула
    \param pool_id идентификатор пула, ссылка на пул
    \param block указатель на блок данных
	\return
    - \b osOK: the memory block is released.
    - \b osErrorValue: block does not belong to the memory pool.
    - \b osErrorParameter: a parameter is invalid or outside of a permitted range.

 */

osStatus osPoolFree (osPoolId pool, void *block)
{
	unsigned int idx = (block - pool->base)/pool->item_aligned_size; // номер элемента
	if (idx>=pool->size) {
//        printf("osPoolFree: osErrorParameter %p %p", pool->base, block);
        return osErrorParameter; 	 // проверяем, что номер элемента попал в пул
	}
	unsigned int mask = (1UL<<(idx & OS_POOL_MAP_MASK));				// очистить бит в маске
//	pool->min = 
	unsigned int map = atomic_fetch_and(&pool->map[idx/OS_POOL_MAP_BITS], ~mask); // бит атомарно чистим
	if ((map & mask) !=0) {
//		printf("osPoolFree: osErrorResource %p %p", pool->base, block);
		return osErrorResource;
	}
//	tss_set((tss_t*)1, block);
	return osOK;
}
/*! \} */
#endif
